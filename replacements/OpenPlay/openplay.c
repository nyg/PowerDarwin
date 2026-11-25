/* SPDX-License-Identifier: MIT */

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/audio/IOAudioDefines.h>
#include <IOKit/audio/IOAudioTypes.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <math.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum {
   kChunkFrames = 1024,
   kLeadFrames = 4096,
   kInputMarginFrames = 2048,
   kBufferSetID = 0,
   kPollMicroseconds = 5000,
   kLevelsPerSecond = 2
};

typedef struct {
   io_service_t service;
   io_connect_t connect;
   volatile IOAudioEngineStatus *status;
   UInt32 bufferFrames;
   UInt32 rate;
   UInt32 channels;
   UInt32 streamID;
   double framesPerTick;
   UInt64 ticksPerFrame;
} Engine;

typedef struct {
   const UInt8 *data;
   UInt32 frames;
   UInt32 channels;
   UInt32 rate;
   double position;
   double phase;
   double frequency;
   UInt32 toneFrames;
   UInt32 toneDone;
} Source;

static volatile sig_atomic_t gStop;

static void stop(int signal)
{
   (void)signal;
   gStop = 1;
}

static SInt32 numberValue(CFTypeRef value, SInt32 fallback)
{
   SInt32 number = fallback;

   if (value && CFGetTypeID(value) == CFNumberGetTypeID())
      CFNumberGetValue(value, kCFNumberSInt32Type, &number);
   return number;
}

static SInt32 numberProperty(io_registry_entry_t entry, CFStringRef key, SInt32 fallback)
{
   CFTypeRef value = IORegistryEntryCreateCFProperty(entry, key, kCFAllocatorDefault, 0);
   SInt32 number = numberValue(value, fallback);

   if (value)
      CFRelease(value);
   return number;
}

static SInt32 dictionaryNumber(io_registry_entry_t entry, CFStringRef key, CFStringRef field, SInt32 fallback)
{
   CFTypeRef value = IORegistryEntryCreateCFProperty(entry, key, kCFAllocatorDefault, 0);
   SInt32 number = fallback;

   if (value && CFGetTypeID(value) == CFDictionaryGetTypeID())
      number = numberValue(CFDictionaryGetValue(value, field), fallback);
   if (value)
      CFRelease(value);
   return number;
}

static int findStream(Engine *engine, SInt32 direction)
{
   io_iterator_t children;
   io_registry_entry_t child;
   int found = 0;

   if (IORegistryEntryGetChildIterator(engine->service, kIOServicePlane, &children) != KERN_SUCCESS)
      return 0;
   while (!found && (child = IOIteratorNext(children))) {
      if (IOObjectConformsTo(child, "IOAudioStream") &&
          numberProperty(child, CFSTR(kIOAudioStreamDirectionKey), -1) == direction) {
         engine->streamID = numberProperty(child, CFSTR(kIOAudioStreamIDKey), 0);
         engine->channels = dictionaryNumber(child, CFSTR(kIOAudioStreamFormatKey), CFSTR(kIOAudioStreamNumChannelsKey), 2);
         found = 1;
      }
      IOObjectRelease(child);
   }
   IOObjectRelease(children);
   return found;
}

static int findEngine(Engine *engine, SInt32 direction)
{
   io_iterator_t engines;
   io_service_t service;

   if (IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching("IOAudioEngine"), &engines) != KERN_SUCCESS)
      return 0;
   while ((service = IOIteratorNext(engines))) {
      engine->service = service;
      if (findStream(engine, direction))
         break;
      IOObjectRelease(service);
      engine->service = IO_OBJECT_NULL;
   }
   IOObjectRelease(engines);
   return engine->service != IO_OBJECT_NULL;
}

static void setNumber(io_registry_entry_t entry, SInt32 value)
{
   CFNumberRef number = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
   kern_return_t result = IORegistryEntrySetCFProperty(entry, CFSTR(kIOAudioControlValueKey), number);

   CFRelease(number);
   if (result != KERN_SUCCESS)
      fprintf(stderr, "openplay: cannot set a control (0x%x)\n", result);
}

static void setLevels(const Engine *engine, SInt32 usage, const char *name, int percent)
{
   io_iterator_t children;
   io_registry_entry_t child;

   if (IORegistryEntryGetChildIterator(engine->service, kIOServicePlane, &children) != KERN_SUCCESS)
      return;
   while ((child = IOIteratorNext(children))) {
      if (numberProperty(child, CFSTR(kIOAudioControlUsageKey), 0) == usage) {
         SInt32 type = numberProperty(child, CFSTR(kIOAudioControlTypeKey), 0);
         SInt32 subType = numberProperty(child, CFSTR(kIOAudioControlSubTypeKey), 0);
         SInt32 value = numberProperty(child, CFSTR(kIOAudioControlValueKey), 0);

         if (type == kIOAudioControlTypeLevel && subType == kIOAudioLevelControlSubTypeVolume) {
            SInt32 minimum = numberProperty(child, CFSTR(kIOAudioLevelControlMinValueKey), 0);
            SInt32 maximum = numberProperty(child, CFSTR(kIOAudioLevelControlMaxValueKey), 0);
            SInt32 level = minimum + (maximum - minimum) * percent / 100;

            printf("%s level %d -> %d (range %d to %d)\n", name, (int)value, (int)level, (int)minimum, (int)maximum);
            setNumber(child, level);
         } else if (type == kIOAudioControlTypeToggle && subType == kIOAudioToggleControlSubTypeMute) {
            printf("%s mute %d -> 0\n", name, (int)value);
            setNumber(child, 0);
         }
      }
      IOObjectRelease(child);
   }
   IOObjectRelease(children);
}

static int openEngine(Engine *engine)
{
   mach_timebase_info_data_t timebase;
   vm_address_t address = 0;
   vm_size_t size = 0;
   kern_return_t result;

   engine->bufferFrames = numberProperty(engine->service, CFSTR(kIOAudioEngineNumSampleFramesPerBufferKey), 0);
   engine->rate = dictionaryNumber(engine->service, CFSTR(kIOAudioSampleRateKey), CFSTR(kIOAudioSampleRateWholeNumberKey), 0);
   if (engine->bufferFrames == 0 || engine->rate == 0) {
      fprintf(stderr, "openplay: the audio engine publishes no buffer size or sample rate\n");
      return 0;
   }
   mach_timebase_info(&timebase);
   engine->framesPerTick = (double)engine->rate * timebase.numer / timebase.denom / 1e9;
   engine->ticksPerFrame = (UInt64)(1e9 * timebase.denom / timebase.numer / engine->rate);

   result = IOServiceOpen(engine->service, mach_task_self(), 0, &engine->connect);
   if (result != KERN_SUCCESS) {
      fprintf(stderr, "openplay: cannot open the audio engine (0x%x)\n", result);
      return 0;
   }
   result = IOConnectMapMemory(engine->connect, kIOAudioStatusBuffer, mach_task_self(), &address, &size, kIOMapAnywhere);
   if (result != KERN_SUCCESS) {
      fprintf(stderr, "openplay: cannot map the engine status (0x%x)\n", result);
      return 0;
   }
   engine->status = (volatile IOAudioEngineStatus *)address;
   return 1;
}

static UInt64 currentFrame(const Engine *engine)
{
   UInt32 loops;
   UInt64 lastLoop;
   UInt64 now;

   do {
      loops = engine->status->fCurrentLoopCount;
      memcpy(&lastLoop, (const void *)&engine->status->fLastLoopTime, sizeof lastLoop);
   } while (loops != engine->status->fCurrentLoopCount);
   now = mach_absolute_time();
   if (now < lastLoop)
      now = lastLoop;
   return (UInt64)loops * engine->bufferFrames + (UInt64)((now - lastLoop) * engine->framesPerTick);
}

static float littleEndianSample(const UInt8 *bytes)
{
   SInt16 sample = (SInt16)(bytes[0] | (bytes[1] << 8));

   return sample / 32768.0f;
}

static int fill(Source *source, const Engine *engine, float *samples, UInt32 frames)
{
   UInt32 frame;
   UInt32 channel;
   int more = 1;

   for (frame = 0; frame < frames; frame++) {
      float *out = samples + frame * engine->channels;

      if (source->data) {
         UInt32 index = (UInt32)source->position;

         if (index >= source->frames) {
            more = 0;
            for (channel = 0; channel < engine->channels; channel++)
               out[channel] = 0.0f;
            continue;
         }
         for (channel = 0; channel < engine->channels; channel++) {
            UInt32 from = channel < source->channels ? channel : source->channels - 1;

            out[channel] = littleEndianSample(source->data + 2 * (index * source->channels + from));
         }
         source->position += (double)source->rate / engine->rate;
      } else {
         float value = 0.0f;

         if (source->toneDone < source->toneFrames) {
            value = 0.25f * (float)sin(source->phase);
            source->phase += 2.0 * M_PI * source->frequency / engine->rate;
            if (source->phase > 2.0 * M_PI)
               source->phase -= 2.0 * M_PI;
            source->toneDone++;
         } else {
            more = 0;
         }
         for (channel = 0; channel < engine->channels; channel++)
            out[channel] = value;
      }
   }
   return more;
}

static UInt32 littleEndian32(const UInt8 *bytes)
{
   return bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | ((UInt32)bytes[3] << 24);
}

static UInt16 littleEndian16(const UInt8 *bytes)
{
   return bytes[0] | (bytes[1] << 8);
}

static int readWave(const char *path, Source *source)
{
   FILE *file = fopen(path, "rb");
   UInt8 *bytes;
   long length;
   long offset = 12;
   int haveFormat = 0;

   if (!file) {
      perror(path);
      return 0;
   }
   fseek(file, 0, SEEK_END);
   length = ftell(file);
   fseek(file, 0, SEEK_SET);
   bytes = malloc(length);
   if (!bytes || fread(bytes, 1, length, file) != (size_t)length) {
      fprintf(stderr, "openplay: cannot read %s\n", path);
      fclose(file);
      return 0;
   }
   fclose(file);
   if (length < 12 || memcmp(bytes, "RIFF", 4) || memcmp(bytes + 8, "WAVE", 4)) {
      fprintf(stderr, "openplay: %s is not a WAVE file\n", path);
      return 0;
   }
   while (offset + 8 <= length) {
      UInt32 size = littleEndian32(bytes + offset + 4);
      const UInt8 *chunk = bytes + offset + 8;

      if (size > (unsigned long)(length - offset - 8))
         size = length - offset - 8;
      if (!memcmp(bytes + offset, "fmt ", 4) && size >= 16) {
         if (littleEndian16(chunk) != 1 || littleEndian16(chunk + 14) != 16) {
            fprintf(stderr, "openplay: %s is not 16-bit PCM\n", path);
            return 0;
         }
         source->channels = littleEndian16(chunk + 2);
         source->rate = littleEndian32(chunk + 4);
         haveFormat = source->channels > 0 && source->rate > 0;
      } else if (!memcmp(bytes + offset, "data", 4) && haveFormat) {
         source->data = chunk;
         source->frames = size / (2 * source->channels);
         return 1;
      }
      offset += 8 + size + (size & 1);
   }
   fprintf(stderr, "openplay: %s has no PCM data\n", path);
   return 0;
}

static void putLittleEndian(UInt8 *bytes, UInt32 value, int size)
{
   int index;

   for (index = 0; index < size; index++)
      bytes[index] = (UInt8)(value >> (8 * index));
}

static int writeWaveHeader(FILE *file, UInt32 rate, UInt32 channels, UInt32 frames)
{
   UInt8 header[44];
   UInt32 dataBytes = frames * channels * 2;

   memcpy(header, "RIFF", 4);
   putLittleEndian(header + 4, 36 + dataBytes, 4);
   memcpy(header + 8, "WAVEfmt ", 8);
   putLittleEndian(header + 16, 16, 4);
   putLittleEndian(header + 20, 1, 2);
   putLittleEndian(header + 22, channels, 2);
   putLittleEndian(header + 24, rate, 4);
   putLittleEndian(header + 28, rate * channels * 2, 4);
   putLittleEndian(header + 32, channels * 2, 2);
   putLittleEndian(header + 34, 16, 2);
   memcpy(header + 36, "data", 4);
   putLittleEndian(header + 40, dataBytes, 4);
   return fseek(file, 0, SEEK_SET) == 0 && fwrite(header, 1, sizeof header, file) == sizeof header;
}

static double decibels(float peak)
{
   return peak > 0.0f ? 20.0 * log10(peak) : -999.0;
}

static int record(Engine *engine, double seconds, const char *path)
{
   size_t header = offsetof(IOAudioBufferDataDescriptor, fData);
   size_t bytes = kChunkFrames * engine->channels * sizeof(float);
   UInt32 levelFrames = engine->rate / kLevelsPerSecond;
   UInt64 total = (UInt64)(seconds * engine->rate);
   IOAudioBufferDataDescriptor *buffer = calloc(1, header + bytes);
   SInt16 *samples = calloc(kChunkFrames * engine->channels, sizeof(SInt16));
   FILE *file = fopen(path, "wb");
   uint64_t arguments[4];
   UInt64 first;
   UInt64 next;
   UInt64 now;
   UInt64 done = 0;
   UInt32 levelDone = 0;
   UInt32 late = 0;
   UInt32 firstLoop;
   UInt32 loops;
   float peak = 0.0f;
   float totalPeak = 0.0f;
   kern_return_t result;

   if (!file) {
      perror(path);
      return 1;
   }
   if (!buffer || !samples || !writeWaveHeader(file, engine->rate, engine->channels, 0)) {
      fprintf(stderr, "openplay: cannot write %s\n", path);
      return 1;
   }
   arguments[0] = engine->streamID;
   arguments[1] = (uintptr_t)buffer;
   arguments[2] = header + bytes;
   arguments[3] = kBufferSetID;
   result = IOConnectCallScalarMethod(engine->connect, kIOAudioEngineCallRegisterClientBuffer, arguments, 4, NULL, NULL);
   if (result != KERN_SUCCESS) {
      fprintf(stderr, "openplay: cannot register the input buffer (0x%x)\n", result);
      return 1;
   }
   result = IOConnectCallScalarMethod(engine->connect, kIOAudioEngineCallStart, NULL, 0, NULL, NULL);
   if (result != KERN_SUCCESS) {
      fprintf(stderr, "openplay: cannot start the audio engine (0x%x)\n", result);
      return 1;
   }

   signal(SIGINT, stop);
   signal(SIGTERM, stop);
   firstLoop = engine->status->fCurrentLoopCount;
   first = next = currentFrame(engine);
   while (!gStop && done < total) {
      now = currentFrame(engine);
      if (now > next + engine->bufferFrames - kChunkFrames) {
         late++;
         next = now - kInputMarginFrames - kChunkFrames;
      }
      while (done < total && next + kChunkFrames + kInputMarginFrames <= now) {
         const float *in = (const float *)buffer->fData;
         UInt32 frames = total - done < kChunkFrames ? (UInt32)(total - done) : kChunkFrames;
         UInt32 frame;
         UInt32 index;

         buffer->fActualNumSampleFrames = kChunkFrames;
         buffer->fActualDataByteSize = bytes;
         buffer->fTotalDataByteSize = bytes;
         buffer->fNominalDataByteSize = bytes;
         result = IOConnectTrap6(engine->connect, kIOAudioEngineTrapPerformClientIO,
                                 (uintptr_t)(next % engine->bufferFrames), (uintptr_t)(next / engine->bufferFrames), 1, kBufferSetID,
                                 (uintptr_t)(engine->ticksPerFrame >> 32), (uintptr_t)(engine->ticksPerFrame & 0xffffffff));
         if (result != KERN_SUCCESS)
            late++;
         for (frame = 0; frame < frames; frame++) {
            for (index = frame * engine->channels; index < (frame + 1) * engine->channels; index++) {
               float value = in[index];
               float magnitude = value < 0.0f ? -value : value;

               if (value > 1.0f)
                  value = 1.0f;
               else if (value < -1.0f)
                  value = -1.0f;
               putLittleEndian((UInt8 *)&samples[index], (UInt32)(SInt32)(value * 32767.0f), 2);
               if (magnitude > peak)
                  peak = magnitude;
            }
            if (++levelDone == levelFrames) {
               printf("%5.1f s  peak %7.1f dBFS\n", (double)(done + frame + 1) / engine->rate, decibels(peak));
               if (peak > totalPeak)
                  totalPeak = peak;
               peak = 0.0f;
               levelDone = 0;
            }
         }
         if (fwrite(samples, 2 * engine->channels, frames, file) != frames) {
            fprintf(stderr, "openplay: cannot write %s\n", path);
            gStop = 1;
            break;
         }
         done += frames;
         next += kChunkFrames;
      }
      usleep(kPollMicroseconds);
   }
   loops = engine->status->fCurrentLoopCount - firstLoop;

   IOConnectCallScalarMethod(engine->connect, kIOAudioEngineCallStop, NULL, 0, NULL, NULL);
   arguments[0] = (uintptr_t)buffer;
   arguments[1] = kBufferSetID;
   IOConnectCallScalarMethod(engine->connect, kIOAudioEngineCallUnregisterClientBuffer, arguments, 2, NULL, NULL);
   IOServiceClose(engine->connect);
   if (peak > totalPeak)
      totalPeak = peak;
   if (!writeWaveHeader(file, engine->rate, engine->channels, (UInt32)done) || fclose(file) != 0) {
      fprintf(stderr, "openplay: cannot write %s\n", path);
      return 1;
   }
   printf("%u frames from frame %llu, %u late, %u buffer loops, peak %.1f dBFS\n",
          (unsigned)done, (unsigned long long)first, (unsigned)late, (unsigned)loops, decibels(totalPeak));
   if (loops == 0) {
      fprintf(stderr, "openplay: the audio engine did not move: no DMA\n");
      return 1;
   }
   return late ? 1 : 0;
}

static void usage(void)
{
   fprintf(stderr, "usage: openplay [-f hertz] [-t seconds] [-v percent] [file.wav]\n"
                   "       openplay -r file.wav [-t seconds] [-g percent]\n");
   exit(2);
}

int main(int argc, char *argv[])
{
   Engine engine = { 0 };
   Source source = { 0 };
   double seconds = 2.0;
   int volume = -1;
   int gain = -1;
   const char *recording = NULL;
   int option;
   size_t header = offsetof(IOAudioBufferDataDescriptor, fData);
   size_t bytes;
   IOAudioBufferDataDescriptor *buffer;
   uint64_t arguments[4];
   UInt64 next;
   UInt64 now;
   UInt32 late = 0;
   UInt32 chunks = 0;
   UInt32 firstLoop;
   UInt32 loops;
   int more = 1;
   kern_return_t result;

   source.frequency = 440.0;
   while ((option = getopt(argc, argv, "f:g:r:t:v:")) != -1) {
      switch (option) {
      case 'f': source.frequency = atof(optarg); break;
      case 'g': gain = atoi(optarg); break;
      case 'r': recording = optarg; break;
      case 't': seconds = atof(optarg); break;
      case 'v': volume = atoi(optarg); break;
      default: usage();
      }
   }
   if (argc - optind > 1 || seconds <= 0 || source.frequency <= 0 || volume > 100 || gain > 100)
      usage();
   if (recording && (argc - optind > 0 || volume >= 0))
      usage();
   if (!recording && gain >= 0)
      usage();
   if (argc - optind == 1 && !readWave(argv[optind], &source))
      return 1;

   if (!findEngine(&engine, recording ? kIOAudioStreamDirectionInput : kIOAudioStreamDirectionOutput)) {
      fprintf(stderr, "openplay: no audio engine with an %s stream\n", recording ? "input" : "output");
      return 1;
   }
   if (!openEngine(&engine))
      return 1;
   if (recording) {
      if (gain >= 0)
         setLevels(&engine, kIOAudioControlUsageInput, "input", gain);
      printf("%u Hz, %u channels, %u-frame buffer, input stream %u\n",
             (unsigned)engine.rate, (unsigned)engine.channels, (unsigned)engine.bufferFrames, (unsigned)engine.streamID);
      return record(&engine, seconds, recording);
   }
   if (volume >= 0)
      setLevels(&engine, kIOAudioControlUsageOutput, "output", volume);
   source.toneFrames = (UInt32)(seconds * engine.rate);
   printf("%u Hz, %u channels, %u-frame buffer, stream %u\n",
          (unsigned)engine.rate, (unsigned)engine.channels, (unsigned)engine.bufferFrames, (unsigned)engine.streamID);

   bytes = kChunkFrames * engine.channels * sizeof(float);
   buffer = calloc(1, header + bytes);
   arguments[0] = engine.streamID;
   arguments[1] = (uintptr_t)buffer;
   arguments[2] = header + bytes;
   arguments[3] = kBufferSetID;
   result = IOConnectCallScalarMethod(engine.connect, kIOAudioEngineCallRegisterClientBuffer, arguments, 4, NULL, NULL);
   if (result != KERN_SUCCESS) {
      fprintf(stderr, "openplay: cannot register the sample buffer (0x%x)\n", result);
      return 1;
   }
   result = IOConnectCallScalarMethod(engine.connect, kIOAudioEngineCallStart, NULL, 0, NULL, NULL);
   if (result != KERN_SUCCESS) {
      fprintf(stderr, "openplay: cannot start the audio engine (0x%x)\n", result);
      return 1;
   }

   signal(SIGINT, stop);
   signal(SIGTERM, stop);
   firstLoop = engine.status->fCurrentLoopCount;
   next = currentFrame(&engine) + kLeadFrames;
   while (!gStop && more) {
      now = currentFrame(&engine);
      if (next <= now) {
         late++;
         next = now + kLeadFrames;
      }
      while (more && next < now + kLeadFrames + kChunkFrames) {
         more = fill(&source, &engine, (float *)buffer->fData, kChunkFrames);
         buffer->fActualNumSampleFrames = kChunkFrames;
         buffer->fActualDataByteSize = bytes;
         buffer->fTotalDataByteSize = bytes;
         buffer->fNominalDataByteSize = bytes;
         result = IOConnectTrap6(engine.connect, kIOAudioEngineTrapPerformClientIO,
                                 (uintptr_t)(next % engine.bufferFrames), (uintptr_t)(next / engine.bufferFrames), 0, kBufferSetID,
                                 (uintptr_t)(engine.ticksPerFrame >> 32), (uintptr_t)(engine.ticksPerFrame & 0xffffffff));
         if (result != KERN_SUCCESS)
            late++;
         chunks++;
         next += kChunkFrames;
      }
      usleep(kPollMicroseconds);
   }
   while (!gStop && currentFrame(&engine) < next)
      usleep(kPollMicroseconds);
   loops = engine.status->fCurrentLoopCount - firstLoop;

   IOConnectCallScalarMethod(engine.connect, kIOAudioEngineCallStop, NULL, 0, NULL, NULL);
   arguments[0] = (uintptr_t)buffer;
   arguments[1] = kBufferSetID;
   IOConnectCallScalarMethod(engine.connect, kIOAudioEngineCallUnregisterClientBuffer, arguments, 2, NULL, NULL);
   IOServiceClose(engine.connect);
   printf("%u chunks of %u frames, %u late, %u buffer loops\n", (unsigned)chunks, (unsigned)kChunkFrames, (unsigned)late, (unsigned)loops);
   if (loops == 0) {
      fprintf(stderr, "openplay: the audio engine did not move: no DMA\n");
      return 1;
   }
   return late ? 1 : 0;
}
