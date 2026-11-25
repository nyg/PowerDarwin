// SPDX-License-Identifier: MIT

#ifndef _OPENRADEONPM_H
#define _OPENRADEONPM_H

#include <IOKit/IOService.h>
#include <IOKit/pci/IOPCIDevice.h>

class OpenRadeonPM : public IOService
{
   OSDeclareDefaultStructors(OpenRadeonPM);

public:
   virtual bool start(IOService *provider);

private:
   volatile UInt8 *fRegisters;

   UInt32 readRegister(UInt32 offset);
   void writeRegister(UInt32 offset, UInt32 value);
   UInt32 readPLL(UInt8 index);
   void writePLL(UInt8 index, UInt32 value);
   bool waitForIdle();
   void enableDynamicClocks();
   bool enableLinkL0s(IOPCIDevice *device);
};

#endif
