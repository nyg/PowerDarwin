// SPDX-License-Identifier: MIT

#ifndef _OPENSMU_H
#define _OPENSMU_H

#include <IOKit/IOService.h>
#include <IOKit/IOLocks.h>

enum {
   kMaxFans = 4,
   kMaxZones = 6,
   kLocationLength = 32
};

struct OpenSMUZone {
   const char *sensor;
   const char *fan;
   SInt32 minCelsius;
   SInt32 maxCelsius;
};

struct OpenSMURubberBand {
   const char *fan;
   const char *reference;
   UInt32 belowRPM;
};

struct OpenSMUFan {
   char location[kLocationLength];
   UInt8 reg;
   UInt32 minRPM;
   UInt32 maxRPM;
   UInt32 targetRPM;
   UInt32 currentRPM;
};

struct OpenSMUSensor {
   UInt8 kind;
   UInt8 reg;
   UInt32 address;
   IOService *nub;
   SInt32 temperature;
   bool read;
};

class OpenSMU : public IOService
{
   OSDeclareDefaultStructors(OpenSMU)

public:
   virtual bool start(IOService *provider);
   virtual void free(void);
   IOReturn sendCommand(UInt8 command, const void *data, UInt8 length, void *reply = 0, UInt8 replyLength = 0);

private:
   IOLock *fLock;
   volatile UInt8 *fCommand;
   UInt32 fCommandAddress;
   const OSSymbol *fBufferFunction;
   const OSSymbol *fRequestFunction;
   const OSSymbol *fAckFunction;
   IOService *fBufferPerformer;
   IOService *fRequestPerformer;
   IOService *fAckPerformer;

   const OpenSMUZone *fZones;
   UInt32 fZoneCount;
   const OpenSMURubberBand *fRubberBands;
   UInt32 fRubberBandCount;
   OpenSMUSensor fSensors[kMaxZones];
   OpenSMUFan fFans[kMaxFans];
   UInt32 fFanCount;
   UInt32 fDiodeSlope;
   SInt32 fDiodeOffset;
   const OSSymbol *fReadI2CFunction;
   bool fOldFanCommands;
   bool fSensorsBound;
   bool fFailsafe;

   IOService *findFunction(const char *name, UInt32 phandle, const OSSymbol **function);
   bool commandDone(void);

   IOReturn readPartition(UInt8 id, UInt8 *buffer, UInt8 length);
   bool startFanControl(IOService *provider);
   OpenSMUFan *findFan(const char *location);
   static void fanThread(void *smu);
   void controlFans(void);
   bool bindSensors(void);
   bool bindSensor(OpenSMUSensor *sensor, const char *location);
   bool readSensor(OpenSMUSensor *sensor);
   IOReturn readI2C(OpenSMUSensor *sensor, UInt8 subAddress, UInt8 *buffer, UInt32 count);
   IOReturn setFanRPM(OpenSMUFan *fan, UInt32 rpm);
   IOReturn getFanRPM(OpenSMUFan *fan, UInt32 *rpm);
   void enterFailsafe(const char *reason, const char *location);
   void publishState(void);
};

#endif
