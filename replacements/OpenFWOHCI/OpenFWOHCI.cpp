// SPDX-License-Identifier: MIT

#include "OpenFWOHCI.h"
#include "OHCI.h"
#include <IOKit/IOLib.h>
#include <IOKit/firewire/IOFWUtils.h>
#include <libkern/OSAtomic.h>
#include <libkern/OSByteOrder.h>

#define super IOFireWireLink
OSDefineMetaClassAndStructors(OpenFWOHCI, IOFireWireLink)

enum {
   kClockTimeoutSeconds = 10,
   kPhyAccessTries = 1000,
   kSoftResetTries = 100,
   kLPSTries = 3,
   kPHYPacketWaitTries = 500,
   kMaxRec2K = 10,
   kATRetries = (200 << 16) | (3 << 8) | (3 << 4) | 3,
   kBusOptionsCapabilities = 0xE0000000,
   kBusOptionsBusManager = 0x18000000,
   kBusOptionsMaxRecShift = 12,
   kBusOptionsMaxRecMask = 0xF << 12,
   kInterruptMask = kOHCIIntReqTxComplete | kOHCIIntRespTxComplete | kOHCIIntRQPkt | kOHCIIntRSPkt |
      kOHCIIntPostedWriteErr | kOHCIIntLockRespErr | kOHCIIntSelfIDComplete | kOHCIIntBusReset |
      kOHCIIntRegAccessFail | kOHCIIntCycle64Seconds | kOHCIIntUnrecoverableError
};

static IOPMPowerState sPowerStates[2] = {
   { kIOPMPowerStateVersion1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
   { kIOPMPowerStateVersion1, kIOPMDeviceUsable, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0 }
};

UInt32 OpenFWOHCI::readRegister(UInt32 offset)
{
   return OSReadLittleInt32((void *)fRegisters, offset);
}

void OpenFWOHCI::writeRegister(UInt32 offset, UInt32 value)
{
   OSWriteLittleInt32((void *)fRegisters, offset, value);
   OSSynchronizeIO();
}

bool OpenFWOHCI::enableClock(void)
{
   const OSSymbol *function = OSSymbol::withCString("EnableFireWireClock");
   mach_timespec_t timeout = { kClockTimeoutSeconds, 0 };
   IOService *resources = waitForService(resourceMatching(function), &timeout);
   IOService *performer = resources ? OSDynamicCast(IOService, resources->getProperty(function)) : 0;
   IOReturn status = kIOReturnNotFound;

   if (performer)
      status = performer->callPlatformFunction(function, false, (void *)true, (void *)fPCI, 0, 0);
   function->release();
   if (status != kIOReturnSuccess) {
      IOLog("OpenFWOHCI: cannot enable the FireWire clock (0x%x)\n", status);
      return false;
   }
   return true;
}

bool OpenFWOHCI::readPhy(UInt32 reg, UInt8 *value)
{
   writeRegister(kOHCIPhyControl, kOHCIPhyControlRdReg | (reg << kOHCIPhyControlRegAddrShift));
   for (UInt32 i = 0; i < kPhyAccessTries; i++) {
      UInt32 control = readRegister(kOHCIPhyControl);
      if ((control & kOHCIPhyControlRdDone) && ((control >> kOHCIPhyControlRdAddrShift) & 0xF) == reg) {
         *value = (control >> kOHCIPhyControlRdDataShift) & 0xFF;
         return true;
      }
      IODelay(10);
   }
   IOLog("OpenFWOHCI: PHY register %u read timed out\n", (unsigned)reg);
   return false;
}

bool OpenFWOHCI::writePhy(UInt32 reg, UInt8 value)
{
   writeRegister(kOHCIPhyControl, kOHCIPhyControlWrReg | (reg << kOHCIPhyControlRegAddrShift) | value);
   for (UInt32 i = 0; i < kPhyAccessTries; i++) {
      if (!(readRegister(kOHCIPhyControl) & kOHCIPhyControlWrReg))
         return true;
      IODelay(10);
   }
   IOLog("OpenFWOHCI: PHY register %u write timed out\n", (unsigned)reg);
   return false;
}

bool OpenFWOHCI::updatePhy(UInt32 reg, UInt8 clear, UInt8 set)
{
   UInt8 value;

   if (reg == kPhyRegisterExtended)
      clear |= kPhyExtendedStatusBits & ~set;
   if (!readPhy(reg, &value))
      return false;
   return writePhy(reg, (value & ~clear) | set);
}

void OpenFWOHCI::probePhy(void)
{
   UInt8 ports = 0, speed = 0;

   readPhy(kPhyRegisterPorts, &ports);
   fPhy1394a = (ports & kPhyPortsExtended) == kPhyPortsExtended;
   fPorts = ports & 0x1F;
   if (fPhy1394a) {
      readPhy(kPhyRegisterSpeed, &speed);
      fPhySpeed = (IOFWSpeed)(speed >> 5);
   } else {
      fPhySpeed = (IOFWSpeed)((ports >> 6) & 3);
   }
   if (fPhySpeed > kFWSpeed400MBit)
      fPhySpeed = kFWSpeed400MBit;
}

void OpenFWOHCI::publishIdentity(void)
{
   UInt8 vendor[3] = { 0, 0, 0 }, product[3] = { 0, 0, 0 };
   UInt32 version = readRegister(kOHCIVersion);

   if (fPhy1394a && writePhy(kPhyRegisterPageSelect, 1 << 5)) {
      for (int i = 0; i < 3; i++) {
         readPhy(kPhyRegisterVendorID0 + i, &vendor[i]);
         readPhy(kPhyRegisterProductID0 + i, &product[i]);
      }
      writePhy(kPhyRegisterPageSelect, 0);
   }
   setProperty("OHCI Version", version, 32);
   setProperty("PHY Vendor_ID", (vendor[0] << 16) | (vendor[1] << 8) | vendor[2], 32);
   setProperty("PHY Device_ID", (product[0] << 16) | (product[1] << 8) | product[2], 32);
   setProperty("RcvPhyPkt", 0ULL, 32);
   IOLog("OpenFWOHCI: OHCI %u.%u, PHY %06x:%06x, %s, %u port%s, S%u\n",
      (unsigned)((version >> 16) & 0xFF), (unsigned)(version & 0xFF),
      (unsigned)((vendor[0] << 16) | (vendor[1] << 8) | vendor[2]),
      (unsigned)((product[0] << 16) | (product[1] << 8) | product[2]),
      fPhy1394a ? "1394a" : "1394-1995", (unsigned)fPorts, fPorts == 1 ? "" : "s", 100U << fPhySpeed);
}

bool OpenFWOHCI::start(IOService *provider)
{
   if (!super::start(provider))
      return false;
   fPCI = OSDynamicCast(IOPCIDevice, provider);
   if (!fPCI)
      return false;
   if (!enableClock())
      return false;
   fPCI->setMemoryEnable(true);
   fPCI->setBusMasterEnable(true);
   fRegisterMap = fPCI->mapDeviceMemoryWithRegister(kIOPCIConfigBaseAddress0);
   if (!fRegisterMap || fRegisterMap->getLength() < kOHCIRegisterSpaceSize) {
      IOLog("OpenFWOHCI: cannot map the OHCI registers\n");
      return false;
   }
   fRegisters = (volatile UInt8 *)fRegisterMap->getVirtualAddress();

   if (!resetController())
      return false;
   OSData *localGUID = OSDynamicCast(OSData, provider->getProperty("local-guid"));
   fGUID = ((UInt64)readRegister(kOHCIGUIDHi) << 32) | readRegister(kOHCIGUIDLo);
   if ((fGUID == 0 || fGUID == ~0ULL) && localGUID && localGUID->getLength() == 8)
      fGUID = OSReadBigInt64(localGUID->getBytesNoCopy(), 0);
   if (fGUID == 0 || fGUID == ~0ULL) {
      IOLog("OpenFWOHCI: no GUID in the controller or the device tree\n");
      return false;
   }
   probePhy();
   publishIdentity();

   if (!allocateContexts()) {
      IOLog("OpenFWOHCI: cannot allocate DMA memory\n");
      return false;
   }

   fWorkLoop = createWorkLoop();
   if (!fWorkLoop)
      return false;
   fIsocWorkloop = fWorkLoop;
   fIsocWorkloop->retain();
   fInterruptSource = IOFilterInterruptEventSource::filterInterruptEventSource(this,
      interruptHandler, interruptFilter, provider, 0);
   if (!fInterruptSource || fWorkLoop->addEventSource(fInterruptSource) != kIOReturnSuccess) {
      IOLog("OpenFWOHCI: cannot attach the interrupt\n");
      return false;
   }

   PMinit();
   provider->joinPMtree(this);
   registerPowerDriver(this, sPowerStates, 2);
   changePowerStateTo(1);

   fControl = createController();
   if (!fControl || !fControl->attach(this)) {
      IOLog("OpenFWOHCI: cannot create the FireWire controller\n");
      PMstop();
      return false;
   }
   if (!fControl->start(this)) {
      IOLog("OpenFWOHCI: the FireWire controller did not start\n");
      fControl->detach(this);
      setLinkPowerState(0);
      PMstop();
      return false;
   }
   return true;
}

IOReturn OpenFWOHCI::powerDownAction(OSObject *owner, void *arg0, void *arg1, void *arg2, void *arg3)
{
   ((OpenFWOHCI *)owner)->setLinkPowerState(0);
   return kIOReturnSuccess;
}

void OpenFWOHCI::stop(IOService *provider)
{
   if (fInterruptSource)
      fInterruptSource->disable();
   if (fWorkLoop)
      fWorkLoop->runAction(powerDownAction, this);
   if (fControl) {
      fControl->release();
      fControl = NULL;
   }
   PMstop();
   super::stop(provider);
}

void OpenFWOHCI::free(void)
{
   if (fControl)
      fControl->release();
   if (fInterruptSource) {
      if (fInterruptSource->getWorkLoop())
         fInterruptSource->getWorkLoop()->removeEventSource(fInterruptSource);
      fInterruptSource->release();
   }
   if (fIsocWorkloop)
      fIsocWorkloop->release();
   if (fWorkLoop)
      fWorkLoop->release();
   freeContexts();
   if (fRegisterMap)
      fRegisterMap->release();
   super::free();
}

IOReturn OpenFWOHCI::setPowerState(unsigned long state, IOService *device)
{
   return IOPMAckImplied;
}

IOPMPowerState *OpenFWOHCI::getPowerStateTable(unsigned long *count)
{
   *count = 2;
   return sPowerStates;
}

bool OpenFWOHCI::resetController(void)
{
   writeRegister(kOHCIHCControlSet, kOHCIHCControlSoftReset);
   for (UInt32 i = 0; readRegister(kOHCIHCControlSet) & kOHCIHCControlSoftReset; i++) {
      if (i == kSoftResetTries) {
         IOLog("OpenFWOHCI: the controller did not leave its soft reset\n");
         return false;
      }
      IOSleep(1);
   }
   for (UInt32 i = 0; !(readRegister(kOHCIHCControlSet) & kOHCIHCControlLPS); i++) {
      if (i == kLPSTries) {
         IOLog("OpenFWOHCI: the link power status did not come on\n");
         return false;
      }
      writeRegister(kOHCIHCControlSet, kOHCIHCControlLPS);
      IOSleep(50);
   }
   writeRegister(kOHCIIntMaskClear, 0xFFFFFFFF);
   writeRegister(kOHCIIsoXmitIntMaskClear, 0xFFFFFFFF);
   writeRegister(kOHCIIsoRecvIntMaskClear, 0xFFFFFFFF);
   return true;
}

void OpenFWOHCI::writeInitialROM(void)
{
   UInt32 *rom = (UInt32 *)fROM[0].bytes;

   bzero(rom, kOpenFWOHCIROMBytes);
   rom[1] = kFWBIBBusName;
   rom[2] = fBusOptions;
   rom[3] = fGUID >> 32;
   rom[4] = fGUID & 0xFFFFFFFF;
   rom[0] = (4 << 24) | (4 << 16) | FWComputeCRC16(&rom[1], 4);
   fCurrentROM = 0;
   fROMPending = false;
   writeRegister(kOHCIConfigROMHeader, rom[0]);
   writeRegister(kOHCIBusOptions, fBusOptions);
   writeRegister(kOHCIConfigROMMap, fROM[0].address);
}

IOReturn OpenFWOHCI::setLinkPowerState(unsigned long state)
{
   if (state == 0) {
      if (!fLinkOn)
         return IOPMAckImplied;
      fLinkOn = false;
      fInterruptsOn = false;
      writeRegister(kOHCIIntMaskClear, 0xFFFFFFFF);
      stopContext(fARRequest.registers);
      stopContext(fARResponse.registers);
      stopContext(fATRequest.registers);
      stopContext(fATResponse.registers);
      flushATContext(&fATRequest);
      flushATContext(&fATResponse);
      writeRegister(kOHCILinkControlClear, kOHCILinkControlCycleMaster | kOHCILinkControlCycleTimerEnable | kOHCILinkControlRcvSelfID);
      writeRegister(kOHCIHCControlClear, kOHCIHCControlLinkEnable | kOHCIHCControlBIBImageValid);
      return IOPMAckImplied;
   }
   if (fLinkOn)
      return IOPMAckImplied;
   if (!resetController())
      return kIOReturnNotResponding;

   writeRegister(kOHCIHCControlClear, kOHCIHCControlNoByteSwapData | kOHCIHCControlPostedWriteEnable);
   if (fPhy1394a && (readRegister(kOHCIHCControlSet) & kOHCIHCControlProgramPhyEnable)) {
      updatePhy(kPhyRegisterExtended, 0, kPhyExtendedAccelArbitration | kPhyExtendedMultiSpeed);
      writeRegister(kOHCIHCControlSet, kOHCIHCControlAPhyEnhanceEnable);
      writeRegister(kOHCIHCControlClear, kOHCIHCControlProgramPhyEnable);
   }

   fBusOptions = readRegister(kOHCIBusOptions);
   fMaxRec = (fBusOptions & kBusOptionsMaxRecMask) >> kBusOptionsMaxRecShift;
   if (fMaxRec > kMaxRec2K)
      fMaxRec = kMaxRec2K;
   fBusOptions = (fBusOptions & ~(kBusOptionsMaxRecMask | kBusOptionsBusManager)) | kBusOptionsCapabilities |
      (fMaxRec << kBusOptionsMaxRecShift);
   if ((readRegister(kOHCIGUIDHi) | readRegister(kOHCIGUIDLo)) == 0) {
      writeRegister(kOHCIGUIDHi, fGUID >> 32);
      writeRegister(kOHCIGUIDLo, fGUID & 0xFFFFFFFF);
   }
   writeInitialROM();
   writeRegister(kOHCISelfIDBuffer, fSelfIDBuffer.address);
   writeRegister(kOHCIATRetries, kATRetries);
   writeRegister(kOHCILinkControlClear, kOHCILinkControlRcvPhyPkt | kOHCILinkControlCycleMaster);
   writeRegister(kOHCILinkControlSet, kOHCILinkControlRcvSelfID | kOHCILinkControlCycleTimerEnable);
   writeRegister(kOHCIAsyncRequestFilterHiSet, 0x80000000);
   writeRegister(kOHCIPhysRequestFilterHiClear, 0xFFFFFFFF);
   writeRegister(kOHCIPhysRequestFilterLoClear, 0xFFFFFFFF);

   initATContext(&fATRequest);
   initATContext(&fATResponse);
   initARContext(&fARRequest);
   initARContext(&fARResponse);

   fPendingEvents = 0;
   writeRegister(kOHCIIntEventClear, 0xFFFFFFFF);
   writeRegister(kOHCIIntMaskSet, kInterruptMask);
   writeRegister(kOHCIHCControlSet, kOHCIHCControlLinkEnable | kOHCIHCControlBIBImageValid);
   fLinkOn = true;
   if (fInterruptsOn)
      writeRegister(kOHCIIntMaskSet, kOHCIIntMasterEnable);
   return IOPMAckImplied;
}

void OpenFWOHCI::enableAllInterrupts(void)
{
   fInterruptsOn = true;
   fInterruptSource->enable();
   writeRegister(kOHCIIntMaskSet, kOHCIIntMasterEnable);
}

bool OpenFWOHCI::interruptFilter(OSObject *owner, IOFilterInterruptEventSource *source)
{
   return ((OpenFWOHCI *)owner)->collectInterrupts();
}

void OpenFWOHCI::interruptHandler(OSObject *owner, IOInterruptEventSource *source, int count)
{
   ((OpenFWOHCI *)owner)->processInterrupts();
}

bool OpenFWOHCI::collectInterrupts(void)
{
   UInt32 events = readRegister(kOHCIIntEventClear);

   if (events == 0 || events == 0xFFFFFFFF)
      return false;
   writeRegister(kOHCIIntEventClear, events & ~kOHCIIntBusReset);
   if (events & kOHCIIntBusReset)
      writeRegister(kOHCIIntMaskClear, kOHCIIntBusReset);
   OSBitOrAtomic(events, (UInt32 *)&fPendingEvents);
   return true;
}

void OpenFWOHCI::handleInterrupts(IOInterruptEventSource *source, int count)
{
   if (fLinkOn)
      collectInterrupts();
   processInterrupts();
}

void OpenFWOHCI::processInterrupts(void)
{
   UInt32 events = OSBitAndAtomic(0, (UInt32 *)&fPendingEvents);

   if (!fLinkOn || events == 0)
      return;
   if (events & kOHCIIntUnrecoverableError)
      handleUnrecoverableError();
   if (events & kOHCIIntBusReset)
      processBusReset();
   if (events & kOHCIIntSelfIDComplete)
      handleSelfIDComplete();
   if (events & kOHCIIntReqTxComplete)
      completeATContext(&fATRequest);
   if (events & kOHCIIntRespTxComplete)
      completeATContext(&fATResponse);
   if (events & kOHCIIntRSPkt)
      drainARContext(&fARResponse);
   if (events & kOHCIIntRQPkt) {
      drainARContext(&fARRequest);
      handleARxReqIntComplete();
   }
   if (events & kOHCIIntCycle64Seconds)
      handleCycle64Seconds();
   if (events & kOHCIIntPostedWriteErr)
      IOLog("OpenFWOHCI: posted write error\n");
   if (events & kOHCIIntLockRespErr)
      IOLog("OpenFWOHCI: lock response error\n");
   if (events & kOHCIIntRegAccessFail)
      IOLog("OpenFWOHCI: register access failed\n");
}

void OpenFWOHCI::handleUnrecoverableError(void)
{
   OpenFWOHCIATContext *at[2] = { &fATRequest, &fATResponse };
   OpenFWOHCIARContext *ar[2] = { &fARRequest, &fARResponse };

   for (int i = 0; i < 2; i++) {
      if (readRegister(at[i]->registers + kOHCIContextControlSet) & kOHCIContextDead) {
         IOLog("OpenFWOHCI: %s context dead (0x%08x), restarting it\n", at[i]->name,
            (unsigned)readRegister(at[i]->registers + kOHCIContextControlSet));
         stopContext(at[i]->registers);
         flushATContext(at[i]);
         initATContext(at[i]);
      }
      if (readRegister(ar[i]->registers + kOHCIContextControlSet) & kOHCIContextDead) {
         IOLog("OpenFWOHCI: %s context dead (0x%08x), restarting it\n", ar[i]->name,
            (unsigned)readRegister(ar[i]->registers + kOHCIContextControlSet));
         stopContext(ar[i]->registers);
         initARContext(ar[i]);
      }
   }
}

void OpenFWOHCI::handleSelfIDComplete(void)
{
   UInt32 count = readRegister(kOHCISelfIDCount);
   UInt32 generation = (count >> kOHCISelfIDCountGenerationShift) & 0xFF;
   UInt32 quadlets = (count >> kOHCISelfIDCountSizeShift) & kOHCISelfIDCountSizeMask;
   const UInt8 *buffer = fSelfIDBuffer.bytes;
   UInt32 nodeID, phyID, others = 0;
   int own = 0;

   stopContext(fATRequest.registers);
   stopContext(fATResponse.registers);
   flushATContext(&fATRequest);
   flushATContext(&fATResponse);
   writeRegister(kOHCIIntEventClear, kOHCIIntBusReset);
   writeRegister(kOHCIIntMaskSet, kOHCIIntBusReset);
   writeRegister(kOHCIAsyncRequestFilterHiSet, 0x80000000);

   if (fROMPending) {
      const UInt32 *rom = (const UInt32 *)fROM[fCurrentROM ^ 1].bytes;
      fCurrentROM ^= 1;
      fROMPending = false;
      writeRegister(kOHCIConfigROMHeader, rom[0]);
      writeRegister(kOHCIBusOptions, rom[2]);
   }

   if (count & kOHCISelfIDCountError) {
      IOLog("OpenFWOHCI: self-ID error, generation %u\n", (unsigned)generation);
      return;
   }
   if (quadlets < 1 || ((OSReadLittleInt32(buffer, 0) >> 16) & 0xFF) != generation)
      return;
   nodeID = readRegister(kOHCINodeID);
   if (!(nodeID & kOHCINodeIDValid))
      return;
   phyID = nodeID & kOHCINodeIDNodeNumberMask;

   for (UInt32 i = 1; i + 1 < quadlets; i += 2) {
      UInt32 id = OSReadLittleInt32(buffer, i * 4);
      UInt32 inverse = OSReadLittleInt32(buffer, (i + 1) * 4);
      if (((id >> 24) & 0x3F) == phyID) {
         if (own < 8) {
            fOwnSelfIDs[own++] = id;
            fOwnSelfIDs[own++] = inverse;
         }
      } else if (others + 2 <= kOpenFWOHCISelfIDBytes / 4) {
         fSelfIDs[others++] = id;
         fSelfIDs[others++] = inverse;
      }
   }
   if (((readRegister(kOHCISelfIDCount) >> kOHCISelfIDCountGenerationShift) & 0xFF) != generation)
      return;
   if (own == 0)
      own = synthesizeOwnSelfID(phyID);
   processSelfIDs(fSelfIDs, others / 2, fOwnSelfIDs, own / 2);
}

int OpenFWOHCI::synthesizeOwnSelfID(UInt32 phyID)
{
   UInt8 reset = 0, control = 0, speed = 0, port;
   UInt32 id = 0x80000000 | (phyID << 24);

   readPhy(kPhyRegisterReset, &reset);
   readPhy(kPhyRegisterControl, &control);
   if (fPhy1394a)
      readPhy(kPhyRegisterSpeed, &speed);
   if (control & kPhyControlLinkActive)
      id |= 1 << 22;
   id |= (reset & 0x3F) << 16;
   id |= (UInt32)fPhySpeed << 14;
   if (control & kPhyControlContender)
      id |= 1 << 11;
   id |= (control & 0x7) << 8;
   for (UInt32 i = 0; i < fPorts && i < 3; i++) {
      UInt32 status = 0;
      if (fPhy1394a && writePhy(kPhyRegisterPageSelect, i) && readPhy(8, &port))
         status = (port & 0x04) ? ((port & 0x08) ? 3 : 2) : 1;
      id |= status << (6 - 2 * i);
   }
   fOwnSelfIDs[0] = id;
   fOwnSelfIDs[1] = ~id;
   IOLog("OpenFWOHCI: the controller left out its own self-ID, using 0x%08x\n", (unsigned)id);
   return 2;
}

void OpenFWOHCI::handleCycle64Seconds(void)
{
   if (!(readRegister(kOHCICycleTimer) & 0x80000000))
      fBusPeriods++;
   processCycle64Int();
}

CSRNodeUniqueID OpenFWOHCI::getGUID(void)
{
   return fGUID;
}

UInt32 OpenFWOHCI::getBusCharacteristics(void)
{
   return fBusOptions;
}

UInt32 OpenFWOHCI::getMaxSendLog(void)
{
   return 9 + fPhySpeed;
}

UInt16 OpenFWOHCI::getNodeID(void)
{
   return readRegister(kOHCINodeID) & kOHCINodeIDMask;
}

IOFWSpeed OpenFWOHCI::getPhySpeed(void)
{
   return fPhySpeed;
}

UInt32 *OpenFWOHCI::getPingTimes(void)
{
   return fPingTimes;
}

IOReturn OpenFWOHCI::resetBus(bool useIBR)
{
   bool done;

   if (!fLinkOn)
      return kIOReturnOffline;
   if (fPHYPacketPending) {
      const UInt8 *descriptor = fATRequest.descriptors.bytes + fPHYPacketSlot * kOpenFWOHCIATSlotDescriptorBytes;
      for (UInt32 i = 0; i < kPHYPacketWaitTries && !OSReadLittleInt32(descriptor, 12); i++)
         IODelay(10);
      fPHYPacketPending = false;
   }
   if (useIBR || !fPhy1394a)
      done = updatePhy(kPhyRegisterReset, 0, kPhyResetInitiate);
   else
      done = updatePhy(kPhyRegisterExtended, 0, kPhyExtendedShortReset);
   return done ? kIOReturnSuccess : kIOReturnTimeout;
}

IOReturn OpenFWOHCI::sendPHYPacket(UInt32 quad)
{
   UInt32 header[3] = { kFWTCodePHYPacket << kOHCIAsyncTCodeShift, quad, ~quad };
   UInt32 slot = (fATRequest.head + fATRequest.count) % fATRequest.slots;
   IOReturn status = queuePacket(&fATRequest, header, sizeof(header), false, 0, 0, 0, 0, 0);

   if (status == kIOReturnSuccess) {
      fPHYPacketSlot = slot;
      fPHYPacketPending = true;
   }
   return status;
}

IOReturn OpenFWOHCI::setContender(bool state)
{
   return updatePhy(kPhyRegisterControl, kPhyControlContender, state ? kPhyControlContender : 0) ? kIOReturnSuccess : kIOReturnTimeout;
}

IOReturn OpenFWOHCI::setRootHoldOff(bool state)
{
   return updatePhy(kPhyRegisterReset, kPhyResetRootHoldOff | kPhyResetInitiate, state ? kPhyResetRootHoldOff : 0) ? kIOReturnSuccess : kIOReturnTimeout;
}

IOReturn OpenFWOHCI::setCycleMaster(bool state)
{
   writeRegister(state ? kOHCILinkControlSet : kOHCILinkControlClear, kOHCILinkControlCycleMaster);
   return kIOReturnSuccess;
}

IOReturn OpenFWOHCI::updateROM(const OSData *rom)
{
   OpenFWOHCIBuffer *next = &fROM[fCurrentROM ^ 1];

   if (!rom || rom->getLength() > kOpenFWOHCIROMBytes)
      return kIOReturnBadArgument;
   bzero(next->bytes, kOpenFWOHCIROMBytes);
   bcopy(rom->getBytesNoCopy(), next->bytes, rom->getLength());
   __asm__ __volatile__("sync" : : : "memory");
   fROMPending = true;
   writeRegister(kOHCIConfigROMMap, next->address);
   return kIOReturnSuccess;
}

IOReturn OpenFWOHCI::getCycleTime(UInt32 &cycleTime)
{
   cycleTime = readRegister(kOHCICycleTimer);
   return kIOReturnSuccess;
}

IOReturn OpenFWOHCI::getBusCycleTime(UInt32 &busTime, UInt32 &cycleTime)
{
   cycleTime = readRegister(kOHCICycleTimer);
   busTime = (fBusPeriods << 7) | (cycleTime >> 25);
   return kIOReturnSuccess;
}

IOReturn OpenFWOHCI::setBusTime(UInt32 busTime)
{
   fBusPeriods = busTime >> 7;
   return kIOReturnSuccess;
}

IODCLProgram *OpenFWOHCI::createDCLProgram(bool talking, DCLCommand *opcodes, IOFireWireController::DCLTaskInfo *info, UInt32 startEvent, UInt32 startState, UInt32 startMask)
{
   return 0;
}

IOReturn OpenFWOHCI::asyncStreamTransmit(UInt32 channel, int speed, UInt32 sync, UInt32 tag, IOMemoryDescriptor *buf, IOByteCount offset, int length, IOFWAsyncStreamCommand *cmd)
{
   return kIOReturnUnsupported;
}

void OpenFWOHCI::setNodeIDPhysicalFilter(UInt16 nodeID, bool state)
{
   if (nodeID == kIOFWAllPhysicalFilters) {
      writeRegister(state ? kOHCIPhysRequestFilterLoSet : kOHCIPhysRequestFilterLoClear, 0xFFFFFFFF);
      writeRegister(state ? kOHCIPhysRequestFilterHiSet : kOHCIPhysRequestFilterHiClear, 0x7FFFFFFF);
   } else if (nodeID < 32) {
      writeRegister(state ? kOHCIPhysRequestFilterLoSet : kOHCIPhysRequestFilterLoClear, 1U << nodeID);
   } else if (nodeID < 63) {
      writeRegister(state ? kOHCIPhysRequestFilterHiSet : kOHCIPhysRequestFilterHiClear, 1U << (nodeID - 32));
   }
}

bool OpenFWOHCI::isPhysicalAccessEnabledForNodeID(UInt16 nodeID)
{
   nodeID &= kOHCINodeIDNodeNumberMask;
   if (nodeID < 32)
      return (readRegister(kOHCIPhysRequestFilterLoSet) >> nodeID) & 1;
   if (nodeID < 63)
      return (readRegister(kOHCIPhysRequestFilterHiSet) >> (nodeID - 32)) & 1;
   return false;
}

void OpenFWOHCI::setNodeFlags(UInt16 nodeID, UInt32 flags)
{
   nodeID &= kOHCINodeIDNodeNumberMask;
   if (nodeID < kOpenFWOHCINodes)
      fNodeFlags[nodeID] = flags;
}

void OpenFWOHCI::setSecurityMode(IOFWSecurityMode mode)
{
   fSecurityMode = mode;
}

IOReturn OpenFWOHCI::clipMaxRec2K(bool clipMaxRec)
{
   return kIOReturnSuccess;
}
