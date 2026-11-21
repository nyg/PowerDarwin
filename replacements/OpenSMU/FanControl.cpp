// SPDX-License-Identifier: MIT

#include "OpenSMU.h"
#include <IOKit/IOLib.h>
#include <IOKit/IODeviceTreeSupport.h>
#include <IOI2CDefs.h>

enum {
   kCommandFan = 0x4a,
   kFanOldSetRPM = 0x00,
   kFanOldGetRPM = 0x01,
   kFanSetRPM = 0x30,
   kFanGetRPM = 0x31,
   kCommandADC = 0xd8,
   kCommandPartition = 0x3e,
   kPartitionLatest = 0x01,
   kCommandMisc = 0xee,
   kMiscGetData = 0x02
};

enum {
   kSensorNone,
   kSensorADC,
   kSensorLM75,
   kSensorMAX6690
};

enum {
   kLoopMilliseconds = 2000,
   kBindingCycles = 30,
   kLimitMarginCelsius = 5,
   kDiodePartition = 0x18,
   kDiodePartitionLength = 8,
   kOldFanCommandLength = 14,
   kI2CRetries = 2
};

static const UInt8 kMAX6690ExtendedRegister[] = { 0x11, 0x10 };

// Leopard's thermal profile drives the HDD fan from the disk and GPU, the ODD fan from the
// optical drive and the north bridge, and keeps the CPU fan within 500 rpm of the ODD fan.
static const OpenSMUZone kPowerMac12_1Zones[] = {
   { "CPU T-Diode", { "CPU Fan" }, 65, 80 },
   { "HD Temp", { "HDD Fan" }, 50, 60 },
   { "GPU Temp", { "HDD Fan" }, 85, 95 },
   { "ODD Temp", { "ODD Fan", "CPU Fan" }, 45, 55 },
   { "NB Temp", { "ODD Fan", "CPU Fan" }, 85, 95 }
};

static const struct {
   const char *model;
   const OpenSMUZone *zones;
   UInt32 count;
} kModels[] = {
   { "PowerMac12,1", kPowerMac12_1Zones, sizeof(kPowerMac12_1Zones) / sizeof(kPowerMac12_1Zones[0]) }
};

static bool dataEquals(OSObject *object, const char *string)
{
   OSData *data = OSDynamicCast(OSData, object);
   UInt32 length = strlen(string);
   const char *bytes;

   if (!data || data->getLength() < length)
      return false;
   bytes = (const char *)data->getBytesNoCopy();
   return !strncmp(bytes, string, length) && (data->getLength() == length || bytes[length] == 0);
}

static bool dataNumber(OSObject *object, UInt32 *value)
{
   OSData *data = OSDynamicCast(OSData, object);

   if (!data || data->getLength() != sizeof(*value))
      return false;
   *value = *(const UInt32 *)data->getBytesNoCopy();
   return true;
}

static IORegistryEntry *childNamed(IORegistryEntry *parent, const char *name)
{
   OSIterator *children = parent->getChildIterator(gIODTPlane);
   IORegistryEntry *child, *found = 0;

   if (!children)
      return 0;
   while (!found && (child = OSDynamicCast(IORegistryEntry, children->getNextObject())))
      if (!strcmp(child->getName(gIODTPlane), name))
         found = child;
   children->release();
   return found;
}

static UInt32 zoneRPM(const OpenSMUZone *zone, SInt32 temperature, const OpenSMUFan *fan)
{
   SInt32 above = temperature / 256 - zone->minCelsius * 256;
   SInt32 span = (zone->maxCelsius - zone->minCelsius) * 256;

   if (above <= 0)
      return fan->minRPM;
   if (above >= span)
      return fan->maxRPM;
   return fan->minRPM + (fan->maxRPM - fan->minRPM) * (UInt32)above / (UInt32)span;
}

IOReturn OpenSMU::readPartition(UInt8 id, UInt8 *buffer, UInt8 length)
{
   UInt8 partition[] = { kPartitionLatest, id };
   UInt8 address[2];
   IOReturn result;

   result = sendCommand(kCommandPartition, partition, sizeof(partition), address, sizeof(address));
   if (result != kIOReturnSuccess)
      return result;
   UInt8 request[] = { kMiscGetData, 4, 0, 0, address[0], address[1], length };
   result = sendCommand(kCommandMisc, request, sizeof(request), buffer, length);
   if (result == kIOReturnSuccess && buffer[0] != id)
      result = kIOReturnNotFound;
   return result;
}

bool OpenSMU::startFanControl(IOService *provider)
{
   IORegistryEntry *root = IORegistryEntry::fromPath("/", gIODTPlane);
   IORegistryEntry *fans;
   OSIterator *children;
   IORegistryEntry *child;
   UInt8 partition[kDiodePartitionLength];
   OpenSMUFan *fan;
   UInt32 i, j, k, reg, length;

   for (i = 0; root && i < sizeof(kModels) / sizeof(kModels[0]); i++)
      if (dataEquals(root->getProperty("model"), kModels[i].model)) {
         fZones = kModels[i].zones;
         fZoneCount = kModels[i].count;
      }
   if (root)
      root->release();
   if (!fZones) {
      IOLog("OpenSMU: no fan zones for this model, the SMU keeps the fans\n");
      return false;
   }

   if (readPartition(kDiodePartition, partition, sizeof(partition)) != kIOReturnSuccess) {
      IOLog("OpenSMU: no CPU diode calibration, the SMU keeps the fans\n");
      return false;
   }
   fDiodeSlope = (partition[4] << 8) | partition[5];
   fDiodeOffset = (SInt16)((partition[6] << 8) | partition[7]);

   fans = childNamed(provider, "fans");
   if (!fans)
      fans = childNamed(provider, "rpm-fans");
   children = fans ? fans->getChildIterator(gIODTPlane) : 0;
   while (children && fFanCount < kMaxFans && (child = OSDynamicCast(IORegistryEntry, children->getNextObject()))) {
      fan = &fFans[fFanCount];
      OSData *location = OSDynamicCast(OSData, child->getProperty("location"));
      if (!dataEquals(child->getProperty("device_type"), "fan-rpm-control") || !location
         || !dataNumber(child->getProperty("reg"), &reg) || reg >= kMaxFans
         || !dataNumber(child->getProperty("min-value"), &fan->minRPM)
         || !dataNumber(child->getProperty("max-value"), &fan->maxRPM))
         continue;
      length = min(location->getLength(), sizeof(fan->location) - 1);
      bcopy(location->getBytesNoCopy(), fan->location, length);
      fan->location[length] = 0;
      fan->reg = reg;
      fFanCount++;
   }
   if (children)
      children->release();

   for (i = 0; i < fZoneCount; i++)
      for (j = 0; j < kMaxZoneFans && fZones[i].fans[j]; j++) {
         for (k = 0; k < fFanCount && strcmp(fFans[k].location, fZones[i].fans[j]); k++)
            ;
         if (k == fFanCount) {
            IOLog("OpenSMU: no %s in the device tree, the SMU keeps the fans\n", fZones[i].fans[j]);
            return false;
         }
      }

   fReadI2CFunction = OSSymbol::withCString(kReadI2Cbus);
   if (!fReadI2CFunction || !IOCreateThread(fanThread, this))
      return false;
   IOLog("OpenSMU: %lu fans, %lu zones, CPU diode slope 0x%04lx offset %ld\n", (unsigned long)fFanCount,
      (unsigned long)fZoneCount, (unsigned long)fDiodeSlope, (long)fDiodeOffset);
   return true;
}

void OpenSMU::fanThread(void *smu)
{
   ((OpenSMU *)smu)->controlFans();
}

bool OpenSMU::bindSensor(OpenSMUSensor *sensor, const char *location)
{
   IORegistryIterator *entries = IORegistryIterator::iterateOver(gIODTPlane, kIORegistryIterateRecursively);
   IORegistryEntry *entry, *parent;
   IOService *nub;
   UInt32 reg, address;

   if (!entries)
      return false;
   while (sensor->kind == kSensorNone && (entry = entries->getNextObject())) {
      if (!dataEquals(entry->getProperty("device_type"), "temp-sensor")
         || !dataEquals(entry->getProperty("location"), location)
         || !dataNumber(entry->getProperty("reg"), &reg)
         || !(parent = entry->getParentEntry(gIODTPlane)))
         continue;
      if (dataEquals(parent->getProperty("compatible"), "smu-sensors")) {
         sensor->kind = kSensorADC;
         sensor->reg = reg;
         continue;
      }
      nub = OSDynamicCast(IOService, parent);
      if (!nub || !dataNumber(parent->getProperty("reg"), &address))
         continue;
      if (dataEquals(parent->getProperty("compatible"), "lm75"))
         sensor->kind = kSensorLM75;
      else if (dataEquals(parent->getProperty("compatible"), "max6690") && reg < sizeof(kMAX6690ExtendedRegister))
         sensor->kind = kSensorMAX6690;
      else
         continue;
      nub->retain();
      sensor->nub = nub;
      sensor->address = address & 0xff;
      sensor->reg = reg;
   }
   entries->release();
   return sensor->kind != kSensorNone;
}

bool OpenSMU::bindSensors(void)
{
   bool bound = true;
   UInt32 i;

   for (i = 0; i < fZoneCount; i++)
      if (fSensors[i].kind == kSensorNone && !bindSensor(&fSensors[i], fZones[i].sensor))
         bound = false;
   return bound;
}

IOReturn OpenSMU::readI2C(OpenSMUSensor *sensor, UInt8 subAddress, UInt8 *buffer, UInt32 count)
{
   IOI2CCommand command;

   bzero(&command, sizeof(command));
   command.command = kI2CCommand_Read;
   command.address = sensor->address;
   command.subAddress = subAddress;
   command.buffer = buffer;
   command.count = count;
   command.mode = kI2CMode_Combined;
   command.retries = kI2CRetries;
   return sensor->nub->callPlatformFunction(fReadI2CFunction, false, &command,
      (void *)kIOI2C_CLIENT_KEY_DEFAULT, 0, 0);
}

bool OpenSMU::readSensor(OpenSMUSensor *sensor)
{
   UInt8 data[2];

   switch (sensor->kind) {
   case kSensorADC:
      if (sendCommand(kCommandADC, &sensor->reg, 1, data, sizeof(data)) != kIOReturnSuccess)
         return false;
      sensor->temperature = (SInt32)(((UInt64)((data[0] << 8) | data[1]) * fDiodeSlope) >> 3) * 2
         + fDiodeOffset * 1024;
      return true;
   case kSensorLM75:
      if (readI2C(sensor, 0, data, sizeof(data)) != kIOReturnSuccess)
         return false;
      sensor->temperature = (SInt16)(((data[0] << 8) | data[1]) & 0xff80) * 256;
      return true;
   case kSensorMAX6690:
      if (readI2C(sensor, sensor->reg, &data[0], 1) != kIOReturnSuccess
         || readI2C(sensor, kMAX6690ExtendedRegister[sensor->reg], &data[1], 1) != kIOReturnSuccess)
         return false;
      sensor->temperature = (SInt8)data[0] * 65536 + ((data[1] & 0xe0) << 8);
      return true;
   }
   return false;
}

IOReturn OpenSMU::setFanRPM(OpenSMUFan *fan, UInt32 rpm)
{
   UInt8 data[kOldFanCommandLength];
   IOReturn result;

   if (!fOldFanCommands) {
      data[0] = kFanSetRPM;
      data[1] = fan->reg;
      data[2] = rpm >> 8;
      data[3] = rpm;
      if (sendCommand(kCommandFan, data, 4) == kIOReturnSuccess)
         return kIOReturnSuccess;
   }
   bzero(data, sizeof(data));
   data[0] = kFanOldSetRPM;
   data[1] = 1 << fan->reg;
   data[2 + fan->reg * 2] = rpm >> 8;
   data[3 + fan->reg * 2] = rpm;
   result = sendCommand(kCommandFan, data, sizeof(data));
   if (result == kIOReturnSuccess && !fOldFanCommands) {
      fOldFanCommands = true;
      IOLog("OpenSMU: the SMU takes the old fan commands\n");
   }
   return result;
}

IOReturn OpenSMU::getFanRPM(OpenSMUFan *fan, UInt32 *rpm)
{
   UInt8 data[3 + 2 * kMaxFans];
   IOReturn result;

   if (!fOldFanCommands) {
      data[0] = kFanGetRPM;
      data[1] = fan->reg;
      result = sendCommand(kCommandFan, data, 2, data, 2);
      if (result == kIOReturnSuccess)
         *rpm = (data[0] << 8) | data[1];
      return result;
   }
   data[0] = kFanOldGetRPM;
   result = sendCommand(kCommandFan, data, 1, data, 3 + fan->reg * 2);
   if (result == kIOReturnSuccess)
      *rpm = (data[1 + fan->reg * 2] << 8) | data[2 + fan->reg * 2];
   return result;
}

void OpenSMU::publishState(void)
{
   OSArray *sensors = OSArray::withCapacity(fZoneCount);
   OSArray *fans = OSArray::withCapacity(fFanCount);
   OSDictionary *entry;
   OSNumber *number;
   OSString *location;
   UInt32 i;

   for (i = 0; sensors && i < fZoneCount; i++) {
      if (!fSensors[i].read || !(entry = OSDictionary::withCapacity(2)))
         continue;
      if ((location = OSString::withCString(fZones[i].sensor))) {
         entry->setObject("location", location);
         location->release();
      }
      if ((number = OSNumber::withNumber((UInt32)fSensors[i].temperature, 32))) {
         entry->setObject("current-value", number);
         number->release();
      }
      sensors->setObject(entry);
      entry->release();
   }
   for (i = 0; fans && i < fFanCount; i++) {
      if (!(entry = OSDictionary::withCapacity(3)))
         continue;
      if ((location = OSString::withCString(fFans[i].location))) {
         entry->setObject("location", location);
         location->release();
      }
      if ((number = OSNumber::withNumber(fFans[i].targetRPM, 32))) {
         entry->setObject("target-value", number);
         number->release();
      }
      if ((number = OSNumber::withNumber(fFans[i].currentRPM, 32))) {
         entry->setObject("current-value", number);
         number->release();
      }
      fans->setObject(entry);
      entry->release();
   }
   if (sensors) {
      setProperty("Sensors", sensors);
      sensors->release();
   }
   if (fans) {
      setProperty("Fans", fans);
      fans->release();
   }
   setProperty("Failsafe", fFailsafe);
}

void OpenSMU::controlFans(void)
{
   const OpenSMUZone *zone;
   OpenSMUFan *fan;
   const char *failure, *where;
   UInt32 cycle, i, j, k, rpm;
   bool bound;

   for (cycle = 0; ; cycle++) {
      failure = where = 0;
      bound = bindSensors();
      for (i = 0; i < fZoneCount; i++) {
         fSensors[i].read = fSensors[i].kind != kSensorNone && readSensor(&fSensors[i]);
         if (failure)
            continue;
         if (fSensors[i].kind == kSensorNone) {
            if (cycle >= kBindingCycles)
               failure = "not found";
         } else if (!fSensors[i].read)
            failure = "unreadable";
         else if (fSensors[i].temperature >= (fZones[i].maxCelsius + kLimitMarginCelsius) * 65536)
            failure = "over its limit";
         if (failure)
            where = fZones[i].sensor;
      }
      if (!bound && !failure) {
         publishState();
         IOSleep(kLoopMilliseconds);
         continue;
      }
      if (bound && !fSensorsBound) {
         fSensorsBound = true;
         IOLog("OpenSMU: found the %lu zone sensors, the fans follow them\n", (unsigned long)fZoneCount);
      }

      for (k = 0; k < fFanCount; k++)
         fFans[k].targetRPM = failure ? fFans[k].maxRPM : fFans[k].minRPM;
      for (i = 0; !failure && i < fZoneCount; i++) {
         zone = &fZones[i];
         for (j = 0; j < kMaxZoneFans && zone->fans[j]; j++)
            for (k = 0; k < fFanCount; k++) {
               fan = &fFans[k];
               if (strcmp(fan->location, zone->fans[j]))
                  continue;
               rpm = zoneRPM(zone, fSensors[i].temperature, fan);
               if (rpm > fan->targetRPM)
                  fan->targetRPM = rpm;
            }
      }

      if (failure && !fFailsafe)
         IOLog("OpenSMU: fans at full speed: %s %s\n", where, failure);
      else if (!failure && fFailsafe)
         IOLog("OpenSMU: fans back under zone control\n");
      fFailsafe = failure != 0;

      for (k = 0; k < fFanCount; k++) {
         setFanRPM(&fFans[k], fFans[k].targetRPM);
         if (getFanRPM(&fFans[k], &rpm) == kIOReturnSuccess)
            fFans[k].currentRPM = rpm;
      }
      publishState();
      IOSleep(kLoopMilliseconds);
   }
}
