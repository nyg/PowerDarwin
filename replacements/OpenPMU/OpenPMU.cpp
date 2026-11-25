// SPDX-License-Identifier: MIT

#include "OpenPMU.h"
#include <IOKit/IOPlatformExpert.h>
#include <IOKit/IOLib.h>
#include <libkern/OSAtomic.h>
#include <pexpert/pexpert.h>

extern "C" boolean_t ml_set_interrupts_enabled(boolean_t enable);

enum {
   kRegisterB = 0x0000,
   kRegisterShift = 0x1400,
   kRegisterAuxControl = 0x1600
};

enum {
   kPortBAck = 0x08,
   kPortBRequest = 0x10,
   kShiftOut = 0x1c,
   kShiftIn = 0x0c,
   kShiftModeMask = 0x1c
};

enum {
   kCommandSetRTC = 0x30,
   kCommandReadRTC = 0x38,
   kCommandShutdown = 0x7e,
   kCommandI2C = 0x9a,
   kCommandResetCPU = 0xd0,
   kCommandReadSwitches = 0xdc
};

enum {
   kAckTimeoutMicroseconds = 100000,
   kPollMicroseconds = 10,
   kLengthByte = -1
};

struct OpenPMUCommand {
   UInt8 command;
   SInt8 sendLength;
   SInt8 replyLength;
};

static const OpenPMUCommand gCommands[] = {
   { kCommandSetRTC, 4, 0 },
   { kCommandReadRTC, 0, 4 },
   { kCommandShutdown, 4, 1 },
   { kCommandI2C, kLengthByte, kLengthByte },
   { kCommandResetCPU, 0, 0 },
   { kCommandReadSwitches, 0, 1 }
};

struct SendMiscCommandParameterBlock {
   UInt32 command;
   IOByteCount sendLength;
   UInt8 *sendBuffer;
   IOByteCount *readLength;
   UInt8 *readBuffer;
};

OSDefineMetaClassAndStructors(ApplePMU, IOService)

#define super ApplePMU
OSDefineMetaClassAndStructors(OpenPMU, ApplePMU)

static OpenPMU *gPMU;

static const OpenPMUCommand *findCommand(UInt8 command)
{
   for (UInt32 i = 0; i < sizeof(gCommands) / sizeof(gCommands[0]); i++)
      if (gCommands[i].command == command)
         return &gCommands[i];
   return 0;
}

// ApplePlatformExpert reads and writes local time in seconds since 1904, past
// the range of a signed long since 1972; it only adds and subtracts offsets.
static int readWriteTimeOfDay(unsigned int options, long *secs)
{
   UInt8 clock[4];
   IOByteCount length = sizeof(clock);
   UInt32 seconds;

   if (options == kPEReadTOD) {
      if (gPMU->sendCommand(kCommandReadRTC, 0, 0, clock, &length) != kIOReturnSuccess || length != sizeof(clock))
         return -1;
      *secs = (long)((UInt32)clock[0] << 24 | (UInt32)clock[1] << 16 | (UInt32)clock[2] << 8 | clock[3]);
      return 0;
   }
   if (options == kPEWriteTOD) {
      seconds = (UInt32)*secs;
      clock[0] = seconds >> 24;
      clock[1] = seconds >> 16;
      clock[2] = seconds >> 8;
      clock[3] = seconds;
      return gPMU->sendCommand(kCommandSetRTC, clock, sizeof(clock), 0, 0) == kIOReturnSuccess ? 0 : -1;
   }
   return -1;
}

static int haltRestart(unsigned int type)
{
   static const UInt8 signature[4] = { 'M', 'A', 'T', 'T' };
   UInt8 reply;
   IOByteCount length = sizeof(reply);

   if (type == kPEHaltCPU)
      return gPMU->sendCommand(kCommandShutdown, signature, sizeof(signature), &reply, &length) == kIOReturnSuccess ? 0 : -1;
   if (type == kPERestartCPU)
      return gPMU->sendCommand(kCommandResetCPU, 0, 0, 0, 0) == kIOReturnSuccess ? 0 : -1;
   return -1;
}

UInt8 OpenPMU::readRegister(UInt32 offset)
{
   UInt8 value = fVIA[offset];
   OSSynchronizeIO();
   return value;
}

void OpenPMU::writeRegister(UInt32 offset, UInt8 value)
{
   fVIA[offset] = value;
   OSSynchronizeIO();
}

bool OpenPMU::waitForAck(bool asserted)
{
   UInt32 waited = 0;

   while (((readRegister(kRegisterB) & kPortBAck) == 0) != asserted) {
      if (waited >= kAckTimeoutMicroseconds)
         return false;
      IODelay(kPollMicroseconds);
      waited += kPollMicroseconds;
   }
   return true;
}

bool OpenPMU::sendByte(UInt8 value)
{
   writeRegister(kRegisterAuxControl, (readRegister(kRegisterAuxControl) & ~kShiftModeMask) | kShiftOut);
   writeRegister(kRegisterShift, value);
   writeRegister(kRegisterB, readRegister(kRegisterB) & ~kPortBRequest);
   if (!waitForAck(true))
      return false;
   writeRegister(kRegisterB, readRegister(kRegisterB) | kPortBRequest);
   return waitForAck(false);
}

bool OpenPMU::receiveByte(UInt8 *value)
{
   writeRegister(kRegisterAuxControl, (readRegister(kRegisterAuxControl) & ~kShiftModeMask) | kShiftIn);
   (void)readRegister(kRegisterShift);
   writeRegister(kRegisterB, readRegister(kRegisterB) & ~kPortBRequest);
   if (!waitForAck(true))
      return false;
   writeRegister(kRegisterB, readRegister(kRegisterB) | kPortBRequest);
   if (!waitForAck(false))
      return false;
   *value = readRegister(kRegisterShift);
   return true;
}

IOReturn OpenPMU::sendCommand(UInt8 command, const UInt8 *data, IOByteCount length, UInt8 *reply, IOByteCount *replyLength)
{
   const OpenPMUCommand *entry = findCommand(command);
   IOByteCount capacity = replyLength ? *replyLength : 0;
   IOByteCount stored = 0;
   UInt8 count, value;
   boolean_t interrupts;
   bool ok;

   if (!entry)
      return kIOReturnUnsupported;
   if (entry->sendLength == kLengthByte ? (length > 255) : (length != (IOByteCount)entry->sendLength))
      return kIOReturnBadArgument;

   IOLockLock(fLock);
   interrupts = ml_set_interrupts_enabled(FALSE);
   ok = waitForAck(false) && sendByte(command);
   if (ok && entry->sendLength == kLengthByte)
      ok = sendByte((UInt8)length);
   for (IOByteCount i = 0; ok && i < length; i++)
      ok = sendByte(data[i]);
   count = entry->replyLength == kLengthByte ? 0 : (UInt8)entry->replyLength;
   if (ok && entry->replyLength == kLengthByte)
      ok = receiveByte(&count);
   for (UInt32 i = 0; ok && i < count; i++) {
      ok = receiveByte(&value);
      if (ok && reply && stored < capacity)
         reply[stored++] = value;
   }
   ml_set_interrupts_enabled(interrupts);
   IOLockUnlock(fLock);

   if (replyLength)
      *replyLength = stored;
   if (!ok) {
      IOLog("OpenPMU: command 0x%02x got no answer\n", command);
      return kIOReturnTimeout;
   }
   return kIOReturnSuccess;
}

IOReturn OpenPMU::callPlatformFunction(const OSSymbol *functionName, bool waitForFunction,
   void *param1, void *param2, void *param3, void *param4)
{
   SendMiscCommandParameterBlock *block = (SendMiscCommandParameterBlock *)param1;

   if (!functionName->isEqualTo(fSendMiscCommand))
      return kIOReturnUnsupported;
   if (!block || block->command > 0xff)
      return kIOReturnBadArgument;
   return sendCommand((UInt8)block->command, block->sendBuffer, block->sendLength, block->readBuffer, block->readLength);
}

bool OpenPMU::start(IOService *provider)
{
   UInt8 clock[4];
   IOByteCount length = sizeof(clock);

   if (!super::start(provider))
      return false;
   fMap = provider->mapDeviceMemoryWithIndex(0);
   fLock = IOLockAlloc();
   fSendMiscCommand = OSSymbol::withCString("sendMiscCommand");
   if (!fMap || !fLock || !fSendMiscCommand)
      return false;
   fVIA = (volatile UInt8 *)fMap->getVirtualAddress();
   if (sendCommand(kCommandReadRTC, 0, 0, clock, &length) != kIOReturnSuccess || length != sizeof(clock))
      return false;
   gPMU = this;
   PE_halt_restart = haltRestart;
   PE_read_write_time_of_day = readWriteTimeOfDay;
   publishResource("IORTC", this);
   // MacRISC4CPU::start waits for IOPMU; we handle none of its sleep or speed calls.
   publishResource("IOPMU", this);
   registerService();
   IOLog("OpenPMU: PMU at 0x%08lx answers\n", (unsigned long)fMap->getPhysicalAddress());
   return true;
}

void OpenPMU::free(void)
{
   if (fMap)
      fMap->release();
   if (fLock)
      IOLockFree(fLock);
   if (fSendMiscCommand)
      fSendMiscCommand->release();
   super::free();
}
