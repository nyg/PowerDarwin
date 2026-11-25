// SPDX-License-Identifier: MIT

#ifndef _OPENFWOHCI_OHCI_H
#define _OPENFWOHCI_OHCI_H

enum {
   kOHCIVersion = 0x000,
   kOHCIGUIDROM = 0x004,
   kOHCIATRetries = 0x008,
   kOHCIConfigROMHeader = 0x018,
   kOHCIBusID = 0x01C,
   kOHCIBusOptions = 0x020,
   kOHCIGUIDHi = 0x024,
   kOHCIGUIDLo = 0x028,
   kOHCIConfigROMMap = 0x034,
   kOHCIVendorID = 0x040,
   kOHCIHCControlSet = 0x050,
   kOHCIHCControlClear = 0x054,
   kOHCISelfIDBuffer = 0x064,
   kOHCISelfIDCount = 0x068,
   kOHCIIntEventSet = 0x080,
   kOHCIIntEventClear = 0x084,
   kOHCIIntMaskSet = 0x088,
   kOHCIIntMaskClear = 0x08C,
   kOHCIIsoXmitIntEventClear = 0x094,
   kOHCIIsoXmitIntMaskClear = 0x09C,
   kOHCIIsoRecvIntEventClear = 0x0A4,
   kOHCIIsoRecvIntMaskClear = 0x0AC,
   kOHCILinkControlSet = 0x0E0,
   kOHCILinkControlClear = 0x0E4,
   kOHCINodeID = 0x0E8,
   kOHCIPhyControl = 0x0EC,
   kOHCICycleTimer = 0x0F0,
   kOHCIAsyncRequestFilterHiSet = 0x100,
   kOHCIAsyncRequestFilterHiClear = 0x104,
   kOHCIAsyncRequestFilterLoSet = 0x108,
   kOHCIAsyncRequestFilterLoClear = 0x10C,
   kOHCIPhysRequestFilterHiSet = 0x110,
   kOHCIPhysRequestFilterHiClear = 0x114,
   kOHCIPhysRequestFilterLoSet = 0x118,
   kOHCIPhysRequestFilterLoClear = 0x11C,
   kOHCIATRequestContext = 0x180,
   kOHCIATResponseContext = 0x1A0,
   kOHCIARRequestContext = 0x1C0,
   kOHCIARResponseContext = 0x1E0,
   kOHCIIsoXmitContextControlClear = 0x204,
   kOHCIIsoRecvContextControlClear = 0x404,
   kOHCIRegisterSpaceSize = 0x800
};

enum {
   kOHCIContextControlSet = 0x0,
   kOHCIContextControlClear = 0x4,
   kOHCIContextCommandPtr = 0xC
};

enum {
   kOHCIHCControlBIBImageValid = 1U << 31,
   kOHCIHCControlNoByteSwapData = 1 << 30,
   kOHCIHCControlProgramPhyEnable = 1 << 23,
   kOHCIHCControlAPhyEnhanceEnable = 1 << 22,
   kOHCIHCControlLPS = 1 << 19,
   kOHCIHCControlPostedWriteEnable = 1 << 18,
   kOHCIHCControlLinkEnable = 1 << 17,
   kOHCIHCControlSoftReset = 1 << 16
};

enum {
   kOHCIIntReqTxComplete = 1 << 0,
   kOHCIIntRespTxComplete = 1 << 1,
   kOHCIIntRQPkt = 1 << 4,
   kOHCIIntRSPkt = 1 << 5,
   kOHCIIntPostedWriteErr = 1 << 8,
   kOHCIIntLockRespErr = 1 << 9,
   kOHCIIntSelfIDComplete = 1 << 16,
   kOHCIIntBusReset = 1 << 17,
   kOHCIIntRegAccessFail = 1 << 18,
   kOHCIIntCycle64Seconds = 1 << 21,
   kOHCIIntCycleInconsistent = 1 << 23,
   kOHCIIntUnrecoverableError = 1 << 24,
   kOHCIIntMasterEnable = 1U << 31
};

enum {
   kOHCILinkControlCycleSource = 1 << 22,
   kOHCILinkControlCycleMaster = 1 << 21,
   kOHCILinkControlCycleTimerEnable = 1 << 20,
   kOHCILinkControlRcvPhyPkt = 1 << 10,
   kOHCILinkControlRcvSelfID = 1 << 9
};

enum {
   kOHCINodeIDValid = 1U << 31,
   kOHCINodeIDRoot = 1 << 30,
   kOHCINodeIDCablePowerStatus = 1 << 27,
   kOHCINodeIDNodeNumberMask = 0x3F,
   kOHCINodeIDMask = 0xFFFF
};

enum {
   kOHCIPhyControlRdDone = 1U << 31,
   kOHCIPhyControlRdAddrShift = 24,
   kOHCIPhyControlRdDataShift = 16,
   kOHCIPhyControlRdReg = 1 << 15,
   kOHCIPhyControlWrReg = 1 << 14,
   kOHCIPhyControlRegAddrShift = 8
};

enum {
   kOHCISelfIDCountError = 1U << 31,
   kOHCISelfIDCountGenerationShift = 16,
   kOHCISelfIDCountSizeShift = 2,
   kOHCISelfIDCountSizeMask = 0x1FF
};

enum {
   kOHCIContextRun = 1 << 15,
   kOHCIContextWake = 1 << 12,
   kOHCIContextDead = 1 << 11,
   kOHCIContextActive = 1 << 10,
   kOHCIContextSpeedShift = 5,
   kOHCIContextSpeedMask = 0x7,
   kOHCIContextEventMask = 0x1F
};

enum {
   kOHCIDescriptorOutputMore = 0U << 28,
   kOHCIDescriptorOutputLast = 1U << 28,
   kOHCIDescriptorInputMore = 2U << 28,
   kOHCIDescriptorStatus = 1 << 27,
   kOHCIDescriptorKeyImmediate = 2 << 24,
   kOHCIDescriptorInterruptAlways = 3 << 20,
   kOHCIDescriptorBranchAlways = 3 << 18
};

enum {
   kOHCIEventMissingAck = 0x03,
   kOHCIEventBusReset = 0x09,
   kOHCIEventTimeout = 0x0A,
   kOHCIEventFlushed = 0x0F,
   kOHCIEventAckComplete = 0x11,
   kOHCIEventAckPending = 0x12,
   kOHCIEventAckBusyX = 0x14,
   kOHCIEventAckBusyA = 0x15,
   kOHCIEventAckBusyB = 0x16,
   kOHCIEventAckTardy = 0x1B,
   kOHCIEventAckDataError = 0x1D,
   kOHCIEventAckTypeError = 0x1E
};

enum {
   kOHCIAsyncSourceBusID = 1 << 23,
   kOHCIAsyncSpeedShift = 16,
   kOHCIAsyncTLabelShift = 10,
   kOHCIAsyncRetryShift = 8,
   kOHCIAsyncTCodeShift = 4
};

enum {
   kPhyRegisterID = 0,
   kPhyRegisterReset = 1,
   kPhyRegisterPorts = 2,
   kPhyRegisterSpeed = 3,
   kPhyRegisterControl = 4,
   kPhyRegisterExtended = 5,
   kPhyRegisterPageSelect = 7,
   kPhyRegisterVendorID0 = 10,
   kPhyRegisterProductID0 = 13
};

enum {
   kPhyResetRootHoldOff = 1 << 7,
   kPhyResetInitiate = 1 << 6,
   kPhyPortsExtended = 7 << 5,
   kPhyControlLinkActive = 1 << 7,
   kPhyControlContender = 1 << 6,
   kPhyExtendedShortReset = 1 << 6,
   kPhyExtendedStatusBits = 0x1C,
   kPhyExtendedAccelArbitration = 1 << 1,
   kPhyExtendedMultiSpeed = 1 << 0
};

#endif
