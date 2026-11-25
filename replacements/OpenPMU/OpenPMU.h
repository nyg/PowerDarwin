// SPDX-License-Identifier: MIT

#ifndef _OPENPMU_H
#define _OPENPMU_H

#include <IOKit/IOService.h>
#include <IOKit/IOLocks.h>

// AppleMacRISC2PE, IOGraphics, IOI2CFamily and AppleI2C look the PMU driver up
// with serviceMatching("ApplePMU"), which matches this class and its subclasses.
class ApplePMU : public IOService
{
   OSDeclareDefaultStructors(ApplePMU)
};

class OpenPMU : public ApplePMU
{
   OSDeclareDefaultStructors(OpenPMU)

public:
   virtual bool start(IOService *provider);
   virtual void free(void);
   virtual IOReturn callPlatformFunction(const OSSymbol *functionName, bool waitForFunction,
      void *param1, void *param2, void *param3, void *param4);
   IOReturn sendCommand(UInt8 command, const UInt8 *data, IOByteCount length, UInt8 *reply, IOByteCount *replyLength);

private:
   IOMemoryMap *fMap;
   volatile UInt8 *fVIA;
   IOLock *fLock;
   const OSSymbol *fSendMiscCommand;

   UInt8 readRegister(UInt32 offset);
   void writeRegister(UInt32 offset, UInt8 value);
   bool waitForAck(bool asserted);
   bool sendByte(UInt8 value);
   bool receiveByte(UInt8 *value);
};

#endif
