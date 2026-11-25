// SPDX-License-Identifier: MIT

#include "OpenRadeonPM.h"
#include <IOKit/IOLib.h>
#include <IOKit/IOLocks.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOKit/pci/IOPCIBridge.h>
#include <libkern/OSByteOrder.h>
#include <pexpert/pexpert.h>

#define super IOService
OSDefineMetaClassAndStructors(OpenRadeonPM, IOService)

// Registers and bits from X.Org's xf86-video-ati 6.14.6, radeon_reg.h (Copyright 2000 ATI Technologies Inc.
// and VA Linux Systems Inc.: see LICENSE).
enum {
   kClockControlIndex = 0x0008,
   kClockControlData = 0x000c,
   kRBBMStatus = 0x0e40
};

enum {
   kPLLIndexMask = 0x3f,
   kPLLWriteEnable = 1 << 7,
   kRBBMActive = 1U << 31
};

enum {
   kVCLK_ECP_CNTL = 0x08,
   kSCLK_CNTL = 0x0d,
   kMCLK_CNTL = 0x12,
   kSCLK_CNTL2 = 0x1e,
   kMCLK_MISC = 0x1f,
   kPIXCLKS_CNTL = 0x2d,
   kSCLK_MORE_CNTL = 0x35
};

enum {
   kSCLKForceMask = 0xfeff8000,
   kSCLKDynStopLatency = 0x00007ff8,
   kSCLK2MaxDynStopLatency = (1 << 10) | (1 << 11) | (1 << 12),
   kSCLK2ForceMask = (1 << 13) | (1 << 14) | (1 << 15),
   kSCLKMoreMaxDynStopLatency = 0x0007,
   kSCLKMoreForceOn = 0x0700,
   kVCLKDynamic = (1 << 6) | (1 << 7),
   kPIXCLKSDynamic = (1 << 6) | (1 << 7) | (1 << 9) | (1 << 10) | (1 << 11) | (1 << 12) | (1 << 13) | (1 << 14) | (1 << 15) | (1 << 16) | (1 << 17) | (1 << 18) | (1 << 19),
   kMCLKMiscDynamic = (1 << 14) | (1 << 15),
   kMCLKForceOnMCLK = (1 << 16) | (1 << 17),
   kMCLKForceOnYCLKAndMC = (1 << 18) | (1 << 19) | (1 << 20)
};

enum {
   kPCICapabilityPointer = 0x34,
   kPCIeLinkCapabilities = 0x0c,
   kPCIeLinkControl = 0x10,
   kPCIeASPMSupportsL0s = 1 << 10,
   kPCIeASPML0s = 1 << 0,
   kMaxCapabilities = 48
};

enum {
   kLevelOff = 0,
   kLevelClocks = 1,
   kLevelClocksAndLink = 2
};

enum {
   kIdlePollMicroseconds = 10,
   kIdleTimeoutMicroseconds = 100000
};

static bool dataEquals(OSObject *object, const char *string)
{
   OSData *data = OSDynamicCast(OSData, object);
   return data && data->getLength() == strlen(string) + 1 && !strncmp((const char *)data->getBytesNoCopy(), string, data->getLength());
}

bool OpenRadeonPM::start(IOService *provider)
{
   IOPCIDevice *device = OSDynamicCast(IOPCIDevice, provider);
   IORegistryEntry *root;
   IOMemoryMap *map;
   bool model, linkL0s;
   int level = kLevelClocksAndLink;

   if (!device || !super::start(provider))
      return false;
   PE_parse_boot_arg("openradeonpm", &level);
   if (level <= kLevelOff) {
      IOLog("OpenRadeonPM: off by the boot argument openradeonpm=0\n");
      return false;
   }
   root = IORegistryEntry::fromPath("/", gIODTPlane);
   model = root && dataEquals(root->getProperty("model"), "PowerMac12,1");
   if (root)
      root->release();
   if (!model)
      return false;

   map = device->mapDeviceMemoryWithRegister(kIOPCIConfigBaseAddress2, kIOMapInhibitCache);
   if (!map) {
      IOLog("OpenRadeonPM: cannot map the GPU's registers\n");
      return false;
   }
   fRegisters = (volatile UInt8 *)map->getVirtualAddress();
   if (!waitForIdle()) {
      IOLog("OpenRadeonPM: the GPU's engine stays busy, clocks left as they are\n");
      map->release();
      return false;
   }
   enableDynamicClocks();
   map->release();
   fRegisters = 0;
   setProperty("DynamicClocks", true);
   linkL0s = level >= kLevelClocksAndLink && enableLinkL0s(device);
   setProperty("LinkL0s", linkL0s);
   IOLog("OpenRadeonPM: dynamic clocks on, PCIe L0s %s\n", linkL0s ? "on" : "off");
   registerService();
   return true;
}

static UInt8 findExpressCapability(IOPCIBridge *bridge, IOPCIAddressSpace space)
{
   UInt8 offset = bridge->configRead8(space, kPCICapabilityPointer);
   for (UInt32 i = 0; offset && i < kMaxCapabilities; i++) {
      if (bridge->configRead8(space, offset) == kIOPCIPCIExpressCapability)
         return offset;
      offset = bridge->configRead8(space, offset + 1);
   }
   return 0;
}

// ASPM L0s on both ends of the GPU's link, the port above it first. Neither end's firmware sets it, and
// IOPCIFamily only restores what the firmware set.
bool OpenRadeonPM::enableLinkL0s(IOPCIDevice *device)
{
   IOPCIBridge *bridge = OSDynamicCast(IOPCIBridge, device->getProvider());
   IOPCIAddressSpace space;
   UInt8 deviceCapability = 0, portCapability;

   if (!bridge || !device->findPCICapability(kIOPCIPCIExpressCapability, &deviceCapability))
      return false;
   space = bridge->getBridgeSpace();
   portCapability = findExpressCapability(bridge, space);
   if (!portCapability
       || !(bridge->configRead32(space, portCapability + kPCIeLinkCapabilities) & kPCIeASPMSupportsL0s)
       || !(device->configRead32(deviceCapability + kPCIeLinkCapabilities) & kPCIeASPMSupportsL0s))
      return false;
   bridge->configWrite16(space, portCapability + kPCIeLinkControl, bridge->configRead16(space, portCapability + kPCIeLinkControl) | kPCIeASPML0s);
   device->configWrite16(deviceCapability + kPCIeLinkControl, device->configRead16(deviceCapability + kPCIeLinkControl) | kPCIeASPML0s);
   return true;
}

UInt32 OpenRadeonPM::readRegister(UInt32 offset)
{
   UInt32 value = OSReadLittleInt32((void *)fRegisters, offset);
   OSSynchronizeIO();
   return value;
}

void OpenRadeonPM::writeRegister(UInt32 offset, UInt32 value)
{
   OSWriteLittleInt32((void *)fRegisters, offset, value);
   OSSynchronizeIO();
}

UInt32 OpenRadeonPM::readPLL(UInt8 index)
{
   fRegisters[kClockControlIndex] = index & kPLLIndexMask;
   OSSynchronizeIO();
   return readRegister(kClockControlData);
}

void OpenRadeonPM::writePLL(UInt8 index, UInt32 value)
{
   fRegisters[kClockControlIndex] = (index & kPLLIndexMask) | kPLLWriteEnable;
   OSSynchronizeIO();
   writeRegister(kClockControlData, value);
}

bool OpenRadeonPM::waitForIdle()
{
   for (UInt32 waited = 0; waited < kIdleTimeoutMicroseconds; waited += kIdlePollMicroseconds) {
      if (!(readRegister(kRBBMStatus) & kRBBMActive))
         return true;
      IODelay(kIdlePollMicroseconds);
   }
   return false;
}

// The RV350 and later branch of LegacySetClockGating() in xf86-video-ati 6.14.6, radeon_pm.c (Copyright 2009
// Advanced Micro Devices, Inc.: see LICENSE). The firmware's display driver also uses the PLL index, so the
// sequence runs with interrupts off and puts the index back.
void OpenRadeonPM::enableDynamicClocks()
{
   IOSimpleLock *lock = IOSimpleLockAlloc();
   IOInterruptState state = IOSimpleLockLockDisableInterrupt(lock);
   UInt32 index = readRegister(kClockControlIndex);
   UInt32 value;

   value = readPLL(kSCLK_CNTL2);
   value &= ~kSCLK2ForceMask;
   value |= kSCLK2MaxDynStopLatency;
   writePLL(kSCLK_CNTL2, value);

   value = readPLL(kSCLK_CNTL);
   value &= ~kSCLKForceMask;
   value |= kSCLKDynStopLatency;
   writePLL(kSCLK_CNTL, value);

   value = readPLL(kSCLK_MORE_CNTL);
   value &= ~kSCLKMoreForceOn;
   value |= kSCLKMoreMaxDynStopLatency;
   writePLL(kSCLK_MORE_CNTL, value);

   writePLL(kVCLK_ECP_CNTL, readPLL(kVCLK_ECP_CNTL) | kVCLKDynamic);
   writePLL(kPIXCLKS_CNTL, readPLL(kPIXCLKS_CNTL) | kPIXCLKSDynamic);
   writePLL(kMCLK_MISC, readPLL(kMCLK_MISC) | kMCLKMiscDynamic);

   value = readPLL(kMCLK_CNTL);
   value |= kMCLKForceOnMCLK;
   value &= ~kMCLKForceOnYCLKAndMC;
   writePLL(kMCLK_CNTL, value);

   writeRegister(kClockControlIndex, index);
   IOSimpleLockUnlockEnableInterrupt(lock, state);
   IOSimpleLockFree(lock);
}
