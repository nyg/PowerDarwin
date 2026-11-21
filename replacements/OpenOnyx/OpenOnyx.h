// SPDX-License-Identifier: MIT

#ifndef _OPENONYX_H
#define _OPENONYX_H

#include "AudioHardwareObjectInterface.h"

enum {
   kOnyxRegisterCount = 13
};

class OpenOnyxAudio : public AudioHardwareObjectInterface
{
   OSDeclareDefaultStructors(OpenOnyxAudio);

public:
   virtual bool start(IOService *provider);
   virtual void initPlugin(PlatformInterface *inPlatformObject);
   virtual bool preDMAEngineInit();

   virtual IOReturn setCodecMute(bool muteState);
   virtual IOReturn setCodecMute(bool muteState, UInt32 streamType);
   virtual bool hasAnalogMute();

   virtual UInt32 getMaximumdBVolume();
   virtual UInt32 getMinimumdBVolume();
   virtual UInt32 getMaximumVolume();
   virtual UInt32 getMinimumVolume();
   virtual UInt32 getDefaultOutputVolume();
   virtual bool setCodecVolume(UInt32 leftVolume, UInt32 rightVolume);

   virtual UInt32 getMaximumdBGain();
   virtual UInt32 getMinimumdBGain();
   virtual UInt32 getMaximumGain();
   virtual UInt32 getMinimumGain();
   virtual UInt32 getDefaultInputGain();
   virtual IOReturn setInputGain(UInt32 leftGain, UInt32 rightGain);

   virtual UInt32 getActiveInput();
   virtual IOReturn setActiveInput(UInt32 input);
   virtual IOReturn setPlayThrough(bool playthroughState);

   virtual IOReturn performSetPowerState(UInt32 currentPowerState, UInt32 pendingPowerState);
   virtual IOReturn setSampleRate(UInt32 sampleRate);
   virtual IOReturn setSampleDepth(UInt32 sampleDepth);
   virtual IOReturn recoverFromFatalError(FatalRecoverySelector selector);

   virtual IOReturn requestSleepTime(UInt32 *microsecondsUntilComplete);
   virtual IOReturn getPluginState(HardwarePluginDescriptorPtr outState);
   virtual IOReturn setPluginState(HardwarePluginDescriptorPtr inState);
   virtual HardwarePluginType getPluginType();

private:
   IOReturn writeRegister(UInt32 index, UInt8 value);
   IOReturn writeAllRegisters();
   IOReturn resetCodec();
   IOReturn startCodec();
   IOReturn writeMute();

   UInt8 mRegisters[kOnyxRegisterCount];
   UInt32 mActiveInput;
   bool mMuted;
};

#endif
