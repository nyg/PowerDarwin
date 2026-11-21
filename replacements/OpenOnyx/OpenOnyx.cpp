// SPDX-License-Identifier: MIT AND BSD-2-Clause

#include "OpenOnyx.h"
#include <IOKit/IOLib.h>

#define super AudioHardwareObjectInterface

OSDefineMetaClassAndStructors(OpenOnyxAudio, AudioHardwareObjectInterface)

enum {
   kOnyxLeftAttenuation = 0,
   kOnyxRightAttenuation = 1,
   kOnyxDACControl = 3,
   kOnyxADCControl = 7
};

enum {
   kOnyxDACMuteBits = 0x03,
   kOnyxADCGainMask = 0x0f,
   kOnyxAttenuationBase = 0x80,
   kOnyxMaximumVolume = 127,
   kOnyxDefaultVolume = 100,
   kOnyxGainOffsetdB = 8,
   kOnyxMaximumGain = 15,
   kOnyxDefaultGain = 7,
   kOnyxMinimumSampleRate = 32000,
   kOnyxMaximumSampleRate = 96000
};

// The codec's registers and their first values, from FreeBSD's onyx.c (Copyright 2012 by Andreas Tobler, BSD-2-Clause: see LICENSE).
static const UInt8 kOnyxRegisterAddress[kOnyxRegisterCount] = { 65, 66, 67, 68, 69, 70, 71, 72, 75, 77, 78, 79, 80 };
static const UInt8 kOnyxRegisterDefault[kOnyxRegisterCount] = { 0x80, 0x80, 0xc0, 0x00, 0x00, 0x04, 0x00, 0x07, 0x04, 0x04, 0x02, 0x00, 0x01 };

bool OpenOnyxAudio::start(IOService *provider)
{
   if (!provider || !super::start(provider)) {
      return false;
   }
   return provider->open(this, kFamilyOption_OpenMultiple);
}

void OpenOnyxAudio::initPlugin(PlatformInterface *inPlatformObject)
{
   mPlatformInterface = inPlatformObject;
   bcopy(kOnyxRegisterDefault, mRegisters, sizeof(mRegisters));
   mActiveInput = 0;
   mMuted = false;
}

bool OpenOnyxAudio::preDMAEngineInit()
{
   IOReturn result = writeAllRegisters();
   if (result != kIOReturnSuccess) {
      IOLog("OpenOnyx: cannot reach the PCM3052 codec (0x%x)\n", result);
      return false;
   }
   IOLog("OpenOnyx: PCM3052 codec set up\n");
   return true;
}

IOReturn OpenOnyxAudio::writeRegister(UInt32 index, UInt8 value)
{
   mRegisters[index] = value;
   if (!mPlatformInterface) {
      return kIOReturnNotReady;
   }
   return mPlatformInterface->writeCodecRegister(kCodec_PCM3052, kOnyxRegisterAddress[index], &mRegisters[index], 1);
}

IOReturn OpenOnyxAudio::writeAllRegisters()
{
   for (UInt32 index = 0; index < kOnyxRegisterCount; index++) {
      IOReturn result = writeRegister(index, mRegisters[index]);
      if (result != kIOReturnSuccess) {
         return result;
      }
   }
   return kIOReturnSuccess;
}

IOReturn OpenOnyxAudio::writeMute()
{
   UInt8 value = mRegisters[kOnyxDACControl] & ~kOnyxDACMuteBits;
   return writeRegister(kOnyxDACControl, mMuted ? value | kOnyxDACMuteBits : value);
}

IOReturn OpenOnyxAudio::setCodecMute(bool muteState)
{
   mMuted = muteState;
   return writeMute();
}

IOReturn OpenOnyxAudio::setCodecMute(bool muteState, UInt32 streamType)
{
   return setCodecMute(muteState);
}

bool OpenOnyxAudio::hasAnalogMute()
{
   return true;
}

UInt32 OpenOnyxAudio::getMaximumdBVolume()
{
   return (UInt32)(-0x8000);
}

UInt32 OpenOnyxAudio::getMinimumdBVolume()
{
   return (UInt32)(-64 << 16);
}

UInt32 OpenOnyxAudio::getMaximumVolume()
{
   return kOnyxMaximumVolume;
}

UInt32 OpenOnyxAudio::getMinimumVolume()
{
   return 0;
}

UInt32 OpenOnyxAudio::getDefaultOutputVolume()
{
   return kOnyxDefaultVolume;
}

bool OpenOnyxAudio::setCodecVolume(UInt32 leftVolume, UInt32 rightVolume)
{
   if (leftVolume > kOnyxMaximumVolume || rightVolume > kOnyxMaximumVolume) {
      return false;
   }
   mVolLeft = leftVolume;
   mVolRight = rightVolume;
   return writeRegister(kOnyxLeftAttenuation, kOnyxAttenuationBase + leftVolume) == kIOReturnSuccess
      && writeRegister(kOnyxRightAttenuation, kOnyxAttenuationBase + rightVolume) == kIOReturnSuccess;
}

UInt32 OpenOnyxAudio::getMaximumdBGain()
{
   return (UInt32)((kOnyxMaximumGain - kOnyxGainOffsetdB) << 16);
}

UInt32 OpenOnyxAudio::getMinimumdBGain()
{
   return (UInt32)(-kOnyxGainOffsetdB << 16);
}

UInt32 OpenOnyxAudio::getMaximumGain()
{
   return kOnyxMaximumGain;
}

UInt32 OpenOnyxAudio::getMinimumGain()
{
   return 0;
}

UInt32 OpenOnyxAudio::getDefaultInputGain()
{
   return kOnyxDefaultGain;
}

IOReturn OpenOnyxAudio::setInputGain(UInt32 leftGain, UInt32 rightGain)
{
   if (leftGain > kOnyxMaximumGain) {
      return kIOReturnBadArgument;
   }
   mGainLeft = leftGain;
   mGainRight = rightGain;
   return writeRegister(kOnyxADCControl, (mRegisters[kOnyxADCControl] & ~kOnyxADCGainMask) | leftGain);
}

UInt32 OpenOnyxAudio::getActiveInput()
{
   return mActiveInput;
}

IOReturn OpenOnyxAudio::setActiveInput(UInt32 input)
{
   mActiveInput = input;
   return kIOReturnSuccess;
}

IOReturn OpenOnyxAudio::setPlayThrough(bool playthroughState)
{
   return playthroughState ? kIOReturnUnsupported : kIOReturnSuccess;
}

IOReturn OpenOnyxAudio::performSetPowerState(UInt32 currentPowerState, UInt32 pendingPowerState)
{
   IOReturn result = kIOReturnSuccess;
   if (pendingPowerState == kIOAudioDeviceSleep) {
      UInt8 value = mRegisters[kOnyxDACControl];
      result = writeRegister(kOnyxDACControl, value | kOnyxDACMuteBits);
      mRegisters[kOnyxDACControl] = value;
   } else if (pendingPowerState == kIOAudioDeviceActive && currentPowerState != kIOAudioDeviceActive) {
      result = writeAllRegisters();
      if (result == kIOReturnSuccess) {
         result = writeMute();
      }
   }
   mPluginCurrentPowerState = pendingPowerState;
   return result;
}

IOReturn OpenOnyxAudio::setSampleRate(UInt32 sampleRate)
{
   return sampleRate >= kOnyxMinimumSampleRate && sampleRate <= kOnyxMaximumSampleRate ? kIOReturnSuccess : kIOReturnBadArgument;
}

IOReturn OpenOnyxAudio::setSampleDepth(UInt32 sampleDepth)
{
   return sampleDepth == 16 || sampleDepth == 24 ? kIOReturnSuccess : kIOReturnBadArgument;
}

IOReturn OpenOnyxAudio::recoverFromFatalError(FatalRecoverySelector selector)
{
   IOReturn result = writeAllRegisters();
   return result == kIOReturnSuccess ? writeMute() : result;
}

IOReturn OpenOnyxAudio::requestSleepTime(UInt32 *microsecondsUntilComplete)
{
   return microsecondsUntilComplete ? kIOReturnSuccess : kIOReturnBadArgument;
}

IOReturn OpenOnyxAudio::getPluginState(HardwarePluginDescriptorPtr outState)
{
   if (!outState) {
      return kIOReturnBadArgument;
   }
   outState->hardwarePluginType = getPluginType();
   outState->registerCacheSize = kOnyxRegisterCount;
   bcopy(mRegisters, outState->registerCache, kOnyxRegisterCount);
   outState->recoveryRequest = 0;
   return kIOReturnSuccess;
}

IOReturn OpenOnyxAudio::setPluginState(HardwarePluginDescriptorPtr inState)
{
   if (!inState || inState->registerCacheSize != kOnyxRegisterCount) {
      return kIOReturnBadArgument;
   }
   bcopy(inState->registerCache, mRegisters, kOnyxRegisterCount);
   return writeAllRegisters();
}

HardwarePluginType OpenOnyxAudio::getPluginType()
{
   return kCodec_PCM3052;
}
