// SPDX-License-Identifier: MIT

#ifndef _OPENFWOHCI_H
#define _OPENFWOHCI_H

#define FIREWIREPRIVATE

#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IODMACommand.h>
#include <IOKit/IOFilterInterruptEventSource.h>
#include <IOKit/firewire/IOFireWireLink.h>
#include <IOKit/firewire/IOFWWorkLoop.h>
#include <IOKit/pci/IOPCIDevice.h>

enum {
   kOpenFWOHCIATRequestSlots = 64,
   kOpenFWOHCIATResponseSlots = 32,
   kOpenFWOHCIATMaxSlots = 64,
   kOpenFWOHCIATSlotDescriptorBytes = 64,
   kOpenFWOHCIATPayloadBytes = 2048,
   kOpenFWOHCIARBuffers = 16,
   kOpenFWOHCIARBufferBytes = 4096,
   kOpenFWOHCIPacketBytes = 4096 + 64,
   kOpenFWOHCISelfIDBytes = 2048,
   kOpenFWOHCIROMBytes = 1024,
   kOpenFWOHCINodes = 64
};

struct OpenFWOHCIBuffer {
   IOBufferMemoryDescriptor *memory;
   IODMACommand *dma;
   UInt8 *bytes;
   UInt32 address;
   UInt32 size;
};

struct OpenFWOHCIATContext {
   const char *name;
   UInt32 registers;
   UInt32 slots;
   OpenFWOHCIBuffer descriptors;
   OpenFWOHCIBuffer payloads;
   IOFWAsyncCommand *commands[kOpenFWOHCIATMaxSlots];
   UInt8 statusWord[kOpenFWOHCIATMaxSlots];
   UInt8 branchWord[kOpenFWOHCIATMaxSlots];
   UInt32 head;
   UInt32 count;
   bool expires;
   bool running;
   bool completing;
};

struct OpenFWOHCIARContext {
   const char *name;
   UInt32 registers;
   OpenFWOHCIBuffer descriptors;
   OpenFWOHCIBuffer data;
   UInt8 *packet;
   UInt32 position;
   UInt32 last;
   bool draining;
};

class OpenFWOHCI : public IOFireWireLink
{
   OSDeclareDefaultStructors(OpenFWOHCI)

public:
   virtual bool start(IOService *provider);
   virtual void stop(IOService *provider);
   virtual void free(void);
   virtual IOReturn setPowerState(unsigned long state, IOService *device);

   virtual IOReturn setLinkPowerState(unsigned long state);
   virtual IOPMPowerState *getPowerStateTable(unsigned long *count);
   virtual void enableAllInterrupts(void);
   virtual void handleInterrupts(IOInterruptEventSource *source, int count);

   virtual CSRNodeUniqueID getGUID(void);
   virtual UInt32 getBusCharacteristics(void);
   virtual UInt32 getMaxSendLog(void);
   virtual UInt16 getNodeID(void);
   virtual IOFWSpeed getPhySpeed(void);
   virtual UInt32 *getPingTimes(void);

   virtual IOReturn resetBus(bool useIBR = false);
   virtual IOReturn sendPHYPacket(UInt32 quad);
   virtual IOReturn setContender(bool state);
   virtual IOReturn setRootHoldOff(bool state);
   virtual IOReturn setCycleMaster(bool state);
   virtual IOReturn updateROM(const OSData *rom);

   virtual IOReturn getCycleTime(UInt32 &cycleTime);
   virtual IOReturn getBusCycleTime(UInt32 &busTime, UInt32 &cycleTime);
   virtual IOReturn setBusTime(UInt32 busTime);

   virtual IOReturn asyncRead(UInt16 nodeID, UInt16 addrHi, UInt32 addrLo, int speed, int label, int size, IOFWAsyncCommand *cmd, IOFWReadFlags flags);
   virtual IOReturn asyncWrite(UInt16 nodeID, UInt16 addrHi, UInt32 addrLo, int speed, int label, IOMemoryDescriptor *buf, IOByteCount offset, int size, IOFWAsyncCommand *cmd, IOFWWriteFlags flags);
   virtual IOReturn asyncLock(UInt16 nodeID, UInt16 addrHi, UInt32 addrLo, int speed, int label, int type, IOMemoryDescriptor *buf, IOByteCount offset, int length, IOFWAsyncCommand *cmd);
   virtual IOReturn asyncReadQuadResponse(UInt16 nodeID, int speed, int label, int rcode, UInt32 data);
   virtual IOReturn asyncReadResponse(UInt16 nodeID, int speed, int label, int rcode, IOMemoryDescriptor *buf, IOByteCount offset, int len, IODMACommand *dmaCommand);
   virtual IOReturn asyncWriteResponse(UInt16 nodeID, int speed, int label, int rcode, UInt16 addrHi);
   virtual IOReturn asyncLockResponse(UInt16 nodeID, int speed, int label, int rcode, int type, void *data, int len);
   virtual IOReturn asyncStreamTransmit(UInt32 channel, int speed, UInt32 sync, UInt32 tag, IOMemoryDescriptor *buf, IOByteCount offset, int length, IOFWAsyncStreamCommand *cmd);
   virtual IOReturn handleAsyncTimeout(IOFWAsyncCommand *cmd);
   virtual IOReturn handleAsyncCompletion(IOFWCommand *cmd, IOReturn status);
   virtual void flushWaitingPackets(void);

   virtual IODCLProgram *createDCLProgram(bool talking, DCLCommand *opcodes, IOFireWireController::DCLTaskInfo *info, UInt32 startEvent, UInt32 startState, UInt32 startMask);

   virtual void setNodeIDPhysicalFilter(UInt16 nodeID, bool state);
   virtual void setNodeFlags(UInt16 nodeID, UInt32 flags);
   virtual bool isPhysicalAccessEnabledForNodeID(UInt16 nodeID);
   virtual void setSecurityMode(IOFWSecurityMode mode);
   virtual IOReturn clipMaxRec2K(bool clipMaxRec);

private:
   IOPCIDevice *fPCI;
   IOMemoryMap *fRegisterMap;
   volatile UInt8 *fRegisters;
   IOFilterInterruptEventSource *fInterruptSource;
   volatile UInt32 fPendingEvents;
   bool fLinkOn;
   bool fInterruptsOn;

   CSRNodeUniqueID fGUID;
   UInt32 fBusOptions;
   UInt32 fMaxRec;
   IOFWSpeed fPhySpeed;
   UInt32 fPorts;
   bool fPhy1394a;
   UInt32 fPingTimes[kOpenFWOHCINodes];
   UInt32 fNodeFlags[kOpenFWOHCINodes];
   IOFWSecurityMode fSecurityMode;
   UInt32 fBusPeriods;

   OpenFWOHCIBuffer fSelfIDBuffer;
   UInt32 fSelfIDs[kOpenFWOHCISelfIDBytes / 4];
   UInt32 fOwnSelfIDs[8];
   OpenFWOHCIBuffer fROM[2];
   UInt32 fCurrentROM;
   bool fROMPending;

   OpenFWOHCIATContext fATRequest;
   OpenFWOHCIATContext fATResponse;
   OpenFWOHCIARContext fARRequest;
   OpenFWOHCIARContext fARResponse;
   UInt32 fPHYPacketSlot;
   bool fPHYPacketPending;

   UInt32 readRegister(UInt32 offset);
   void writeRegister(UInt32 offset, UInt32 value);
   bool enableClock(void);
   bool readPhy(UInt32 reg, UInt8 *value);
   bool writePhy(UInt32 reg, UInt8 value);
   bool updatePhy(UInt32 reg, UInt8 clear, UInt8 set);
   void probePhy(void);
   void publishIdentity(void);
   bool resetController(void);
   void writeInitialROM(void);

   static bool interruptFilter(OSObject *owner, IOFilterInterruptEventSource *source);
   static void interruptHandler(OSObject *owner, IOInterruptEventSource *source, int count);
   static IOReturn powerDownAction(OSObject *owner, void *arg0, void *arg1, void *arg2, void *arg3);
   bool collectInterrupts(void);
   void processInterrupts(void);
   void handleSelfIDComplete(void);
   int synthesizeOwnSelfID(UInt32 phyID);
   void handleUnrecoverableError(void);
   void handleCycle64Seconds(void);

   bool allocateBuffer(OpenFWOHCIBuffer *buffer, UInt32 size);
   void freeBuffer(OpenFWOHCIBuffer *buffer);
   bool allocateContexts(void);
   void freeContexts(void);

   void initATContext(OpenFWOHCIATContext *context);
   void stopContext(UInt32 registers);
   void flushATContext(OpenFWOHCIATContext *context);
   IOReturn queuePacket(OpenFWOHCIATContext *context, const UInt32 *header, UInt32 headerBytes, bool quadletData, IOMemoryDescriptor *buf, IOByteCount offset, const void *bytes, UInt32 payloadBytes, IOFWAsyncCommand *cmd);
   void completeATContext(OpenFWOHCIATContext *context);

   void initARContext(OpenFWOHCIARContext *context);
   UInt32 filledARBytes(OpenFWOHCIARContext *context, UInt32 buffer);
   UInt32 availableARBytes(OpenFWOHCIARContext *context);
   void copyARBytes(OpenFWOHCIARContext *context, UInt32 position, UInt8 *to, UInt32 length);
   void recycleARBuffer(OpenFWOHCIARContext *context, UInt32 buffer);
   void drainARContext(OpenFWOHCIARContext *context);
};

#endif
