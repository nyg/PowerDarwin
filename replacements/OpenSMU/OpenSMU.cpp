// SPDX-License-Identifier: MIT

#include "OpenSMU.h"
#include <IOKit/IOPlatformExpert.h>
#include <IOKit/IOLib.h>
#include <pexpert/pexpert.h>

extern "C" {
   extern pmap_t kernel_pmap;
   ppnum_t pmap_find_phys(pmap_t pmap, addr64_t va);
   void flush_dcache(vm_offset_t va, unsigned length, boolean_t phys);
}

enum {
   kCommandRTC = 0x8e,
   kRTCSetDateTime = 0x80,
   kRTCGetDateTime = 0x81,
   kCommandPower = 0xaa
};

enum {
   kHeaderLength = 2,
   kMaxReplyLength = 255,
   kPollMicroseconds = 100,
   kCommandTimeoutMicroseconds = 2000000,
   kFunctionTimeoutSeconds = 30
};

enum {
   kSecondsPerDay = 86400,
   kDaysFrom1904To1970 = 24107,
   kDaysFromYear0To1970 = 719468,
   kWeekdayOf1904 = 5
};

#define super IOService
OSDefineMetaClassAndStructors(OpenSMU, IOService)

static OpenSMU *gSMU;

static UInt32 fromBCD(UInt8 value)
{
   return (value >> 4) * 10 + (value & 0x0f);
}

static UInt8 toBCD(UInt32 value)
{
   return (UInt8)(((value / 10) << 4) | (value % 10));
}

static UInt32 daysSince1904(UInt32 year, UInt32 month, UInt32 day)
{
   UInt32 shiftedYear = month > 2 ? year : year - 1;
   UInt32 era = shiftedYear / 400;
   UInt32 yearOfEra = shiftedYear - era * 400;
   UInt32 dayOfYear = (153 * (month > 2 ? month - 3 : month + 9) + 2) / 5 + day - 1;
   UInt32 dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
   return era * 146097 + dayOfEra - kDaysFromYear0To1970 + kDaysFrom1904To1970;
}

static void dateFromDaysSince1904(UInt32 days, UInt32 *year, UInt32 *month, UInt32 *day)
{
   UInt32 shifted = days + kDaysFromYear0To1970 - kDaysFrom1904To1970;
   UInt32 era = shifted / 146097;
   UInt32 dayOfEra = shifted - era * 146097;
   UInt32 yearOfEra = (dayOfEra - dayOfEra / 1460 + dayOfEra / 36524 - dayOfEra / 146096) / 365;
   UInt32 dayOfYear = dayOfEra - (365 * yearOfEra + yearOfEra / 4 - yearOfEra / 100);
   UInt32 shiftedMonth = (5 * dayOfYear + 2) / 153;
   *day = dayOfYear - (153 * shiftedMonth + 2) / 5 + 1;
   *month = shiftedMonth < 10 ? shiftedMonth + 3 : shiftedMonth - 9;
   *year = yearOfEra + era * 400 + (*month <= 2);
}

// ApplePlatformExpert reads and writes local time in seconds since 1904, past
// the range of a signed long since 1972; it only adds and subtracts offsets.
static int readWriteTimeOfDay(unsigned int options, long *secs)
{
   UInt8 date[7];
   UInt32 seconds, days, year, month, day;

   if (options == kPEReadTOD) {
      UInt8 request = kRTCGetDateTime;
      if (gSMU->sendCommand(kCommandRTC, &request, sizeof(request), date, sizeof(date)) != kIOReturnSuccess)
         return -1;
      month = fromBCD(date[5]);
      day = fromBCD(date[4]);
      if (month < 1 || month > 12 || day < 1 || day > 31)
         return -1;
      seconds = daysSince1904(2000 + fromBCD(date[6]), month, day) * kSecondsPerDay
         + fromBCD(date[2]) * 3600 + fromBCD(date[1]) * 60 + fromBCD(date[0]);
      *secs = (long)seconds;
      return 0;
   }
   if (options == kPEWriteTOD) {
      seconds = (UInt32)*secs;
      days = seconds / kSecondsPerDay;
      seconds %= kSecondsPerDay;
      dateFromDaysSince1904(days, &year, &month, &day);
      UInt8 request[8] = {
         kRTCSetDateTime, toBCD(seconds % 60), toBCD(seconds / 60 % 60), toBCD(seconds / 3600),
         toBCD((days + kWeekdayOf1904) % 7), toBCD(day), toBCD(month), toBCD(year % 100)
      };
      return gSMU->sendCommand(kCommandRTC, request, sizeof(request)) == kIOReturnSuccess ? 0 : -1;
   }
   return -1;
}

static int haltRestart(unsigned int type)
{
   static const char shutdown[] = "SHUTDOWN";
   static const char restart[] = "RESTART";

   if (type == kPEHaltCPU)
      return gSMU->sendCommand(kCommandPower, shutdown, sizeof(shutdown)) == kIOReturnSuccess ? 0 : -1;
   if (type == kPERestartCPU)
      return gSMU->sendCommand(kCommandPower, restart, sizeof(restart)) == kIOReturnSuccess ? 0 : -1;
   return -1;
}

IOService *OpenSMU::findFunction(const char *name, UInt32 phandle, const OSSymbol **function)
{
   char fullName[64];
   mach_timespec_t timeout = { kFunctionTimeoutSeconds, 0 };
   IOService *resources;
   IOService *performer = 0;

   snprintf(fullName, sizeof(fullName), "platform-%s-%08lx", name, (unsigned long)phandle);
   *function = OSSymbol::withCString(fullName);
   resources = waitForService(resourceMatching(*function), &timeout);
   if (resources)
      performer = OSDynamicCast(IOService, resources->getProperty(*function));
   if (!performer)
      IOLog("OpenSMU: %s did not show up\n", fullName);
   return performer;
}

bool OpenSMU::commandDone(void)
{
   UInt32 ack = 0;

   fAckPerformer->callPlatformFunction(fAckFunction, false, &ack, 0, 0, 0);
   return ack != 0;
}

IOReturn OpenSMU::sendCommand(UInt8 command, const void *data, UInt8 length, void *reply, UInt8 replyLength)
{
   IOReturn result = kIOReturnSuccess;
   UInt32 waited = 0;
   bool done;

   IOLockLock(fLock);
   fCommand[0] = command;
   fCommand[1] = length;
   bcopy(data, (void *)(fCommand + kHeaderLength), length);
   flush_dcache((vm_offset_t)fCommand, kHeaderLength + length, FALSE);
   fBufferPerformer->callPlatformFunction(fBufferFunction, false, (void *)fCommandAddress, 0, 0, 0);
   fRequestPerformer->callPlatformFunction(fRequestFunction, false, (void *)1, 0, 0, 0);
   while (!(done = commandDone()) && waited < kCommandTimeoutMicroseconds) {
      IODelay(kPollMicroseconds);
      waited += kPollMicroseconds;
   }
   flush_dcache((vm_offset_t)fCommand, kHeaderLength + kMaxReplyLength, FALSE);
   if (!done)
      result = kIOReturnTimeout;
   else if (fCommand[0] != (command ^ 0xff))
      result = kIOReturnIOError;
   else if (fCommand[1] < replyLength)
      result = kIOReturnUnderrun;
   else if (reply)
      bcopy((const void *)(fCommand + kHeaderLength), reply, replyLength);
   if (result != kIOReturnSuccess)
      IOLog("OpenSMU: command 0x%02x failed with 0x%08x, buffer holds 0x%02x 0x%02x\n",
         command, result, fCommand[0], fCommand[1]);
   IOLockUnlock(fLock);
   return result;
}

bool OpenSMU::start(IOService *provider)
{
   OSData *phandleData;
   UInt32 phandle;
   ppnum_t page;

   if (!super::start(provider))
      return false;
   phandleData = OSDynamicCast(OSData, provider->getProperty("AAPL,phandle"));
   if (!phandleData || phandleData->getLength() != sizeof(phandle))
      return false;
   phandle = *(const UInt32 *)phandleData->getBytesNoCopy();
   fBufferPerformer = findFunction("doorbell-buff", phandle, &fBufferFunction);
   fRequestPerformer = findFunction("doorbell-req", phandle, &fRequestFunction);
   fAckPerformer = findFunction("doorbell-ack", phandle, &fAckFunction);
   if (!fBufferPerformer || !fRequestPerformer || !fAckPerformer)
      return false;
   fLock = IOLockAlloc();
   fCommand = (volatile UInt8 *)IOMallocAligned(PAGE_SIZE, PAGE_SIZE);
   if (!fLock || !fCommand)
      return false;
   page = pmap_find_phys(kernel_pmap, (addr64_t)(uintptr_t)fCommand);
   if (!page || page >> (32 - PAGE_SHIFT))
      return false;
   fCommandAddress = page << PAGE_SHIFT;
   gSMU = this;
   PE_halt_restart = haltRestart;
   PE_read_write_time_of_day = readWriteTimeOfDay;
   publishResource("IORTC", this);
   // MacRISC4CPU::start waits for IOPMU; we handle none of its sleep or speed calls.
   publishResource("IOPMU", this);
   registerService();
   IOLog("OpenSMU: command buffer at 0x%08lx\n", (unsigned long)fCommandAddress);
   startFanControl(provider);
   return true;
}

void OpenSMU::free(void)
{
   if (fCommand)
      IOFreeAligned((void *)fCommand, PAGE_SIZE);
   if (fLock)
      IOLockFree(fLock);
   if (fBufferFunction)
      fBufferFunction->release();
   if (fRequestFunction)
      fRequestFunction->release();
   if (fAckFunction)
      fAckFunction->release();
   if (fReadI2CFunction)
      fReadI2CFunction->release();
   super::free();
}
