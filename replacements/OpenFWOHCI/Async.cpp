// SPDX-License-Identifier: MIT

#include "OpenFWOHCI.h"
#include "OHCI.h"
#include <IOKit/IOLib.h>
#include <libkern/OSByteOrder.h>

enum {
   kContextStopTries = 1000,
   kARRingBytes = kOpenFWOHCIARBuffers * kOpenFWOHCIARBufferBytes
};

static inline void syncMemory(void)
{
   __asm__ __volatile__("sync" : : : "memory");
}

static inline void putWord(UInt8 *base, UInt32 word, UInt32 value)
{
   OSWriteLittleInt32(base, word * 4, value);
}

static inline UInt32 getWord(const UInt8 *base, UInt32 word)
{
   return OSReadLittleInt32(base, word * 4);
}

static int ackFromEvent(UInt32 event)
{
   switch (event) {
   case kOHCIEventAckComplete:
      return kFWAckComplete;
   case kOHCIEventAckPending:
      return kFWAckPending;
   case kOHCIEventAckBusyX:
   case kOHCIEventAckTardy:
      return kFWAckBusyX;
   case kOHCIEventAckBusyA:
      return kFWAckBusyA;
   case kOHCIEventAckBusyB:
      return kFWAckBusyB;
   case kOHCIEventAckDataError:
      return kFWAckDataError;
   case kOHCIEventAckTypeError:
      return kFWAckTypeError;
   default:
      return kFWAckTimeout;
   }
}

static UInt32 asyncHeader(int tcode, int speed, int label)
{
   return ((speed & 7) << kOHCIAsyncSpeedShift) | ((label & 0x3F) << kOHCIAsyncTLabelShift) |
      (1 << kOHCIAsyncRetryShift) | (tcode << kOHCIAsyncTCodeShift);
}

bool OpenFWOHCI::allocateBuffer(OpenFWOHCIBuffer *buffer, UInt32 size)
{
   IODMACommand::Segment32 segment;
   UInt64 offset = 0;
   UInt32 segments = 1;

   size = (size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
   buffer->memory = IOBufferMemoryDescriptor::withOptions(kIODirectionInOut | kIOMemoryPhysicallyContiguous, size, PAGE_SIZE);
   if (!buffer->memory)
      return false;
   buffer->bytes = (UInt8 *)buffer->memory->getBytesNoCopy();
   buffer->size = size;
   bzero(buffer->bytes, size);
   buffer->dma = IODMACommand::withSpecification(kIODMACommandOutputHost32, 32, size, IODMACommand::kMapped, size, PAGE_SIZE);
   if (!buffer->dma)
      return false;
   if (buffer->dma->setMemoryDescriptor(buffer->memory) != kIOReturnSuccess) {
      buffer->dma->release();
      buffer->dma = 0;
      return false;
   }
   if (buffer->dma->gen32IOVMSegments(&offset, &segment, &segments) != kIOReturnSuccess || segments != 1 || segment.fLength != size)
      return false;
   buffer->address = segment.fIOVMAddr;
   return true;
}

void OpenFWOHCI::freeBuffer(OpenFWOHCIBuffer *buffer)
{
   if (buffer->dma) {
      buffer->dma->clearMemoryDescriptor();
      buffer->dma->release();
   }
   if (buffer->memory)
      buffer->memory->release();
   bzero(buffer, sizeof(*buffer));
}

bool OpenFWOHCI::allocateContexts(void)
{
   fATRequest.name = "AT request";
   fATRequest.registers = kOHCIATRequestContext;
   fATRequest.slots = kOpenFWOHCIATRequestSlots;
   fATResponse.name = "AT response";
   fATResponse.registers = kOHCIATResponseContext;
   fATResponse.slots = kOpenFWOHCIATResponseSlots;
   fARRequest.name = "AR request";
   fARRequest.registers = kOHCIARRequestContext;
   fARResponse.name = "AR response";
   fARResponse.registers = kOHCIARResponseContext;

   return allocateBuffer(&fSelfIDBuffer, kOpenFWOHCISelfIDBytes) &&
      allocateBuffer(&fROM[0], kOpenFWOHCIROMBytes) &&
      allocateBuffer(&fROM[1], kOpenFWOHCIROMBytes) &&
      allocateBuffer(&fATRequest.descriptors, fATRequest.slots * kOpenFWOHCIATSlotDescriptorBytes) &&
      allocateBuffer(&fATRequest.payloads, fATRequest.slots * kOpenFWOHCIATPayloadBytes) &&
      allocateBuffer(&fATResponse.descriptors, fATResponse.slots * kOpenFWOHCIATSlotDescriptorBytes) &&
      allocateBuffer(&fATResponse.payloads, fATResponse.slots * kOpenFWOHCIATPayloadBytes) &&
      allocateBuffer(&fARRequest.descriptors, kOpenFWOHCIARBuffers * 16) &&
      allocateBuffer(&fARRequest.data, kARRingBytes) &&
      allocateBuffer(&fARResponse.descriptors, kOpenFWOHCIARBuffers * 16) &&
      allocateBuffer(&fARResponse.data, kARRingBytes);
}

void OpenFWOHCI::freeContexts(void)
{
   freeBuffer(&fSelfIDBuffer);
   freeBuffer(&fROM[0]);
   freeBuffer(&fROM[1]);
   freeBuffer(&fATRequest.descriptors);
   freeBuffer(&fATRequest.payloads);
   freeBuffer(&fATResponse.descriptors);
   freeBuffer(&fATResponse.payloads);
   freeBuffer(&fARRequest.descriptors);
   freeBuffer(&fARRequest.data);
   freeBuffer(&fARResponse.descriptors);
   freeBuffer(&fARResponse.data);
}

void OpenFWOHCI::stopContext(UInt32 registers)
{
   if (!fRegisters)
      return;
   writeRegister(registers + kOHCIContextControlClear, kOHCIContextRun);
   for (UInt32 i = 0; i < kContextStopTries; i++) {
      if (!(readRegister(registers + kOHCIContextControlSet) & kOHCIContextActive))
         return;
      IODelay(10);
   }
   IOLog("OpenFWOHCI: context 0x%03x did not stop\n", (unsigned)registers);
}

void OpenFWOHCI::initATContext(OpenFWOHCIATContext *context)
{
   context->head = 0;
   context->count = 0;
   context->running = false;
   context->completing = false;
   bzero(context->commands, sizeof(context->commands));
   if (context->descriptors.bytes)
      bzero(context->descriptors.bytes, context->descriptors.size);
}

void OpenFWOHCI::flushATContext(OpenFWOHCIATContext *context)
{
   IOFWAsyncCommand *flushed[kOpenFWOHCIATMaxSlots];
   UInt32 count = 0;

   for (UInt32 i = 0; i < context->count; i++) {
      UInt32 slot = (context->head + i) % context->slots;
      if (context->commands[slot])
         flushed[count++] = context->commands[slot];
   }
   initATContext(context);
   for (UInt32 i = 0; i < count; i++)
      flushed[i]->gotAck(kFWAckTimeout);
}

IOReturn OpenFWOHCI::queuePacket(OpenFWOHCIATContext *context, const UInt32 *header, UInt32 headerBytes, bool quadletData, IOMemoryDescriptor *buf, IOByteCount offset, const void *bytes, UInt32 payloadBytes, IOFWAsyncCommand *cmd)
{
   UInt32 slot, address, z;
   UInt8 *descriptor, *immediate;

   if (!fLinkOn)
      return kIOReturnOffline;
   if (payloadBytes > kOpenFWOHCIATPayloadBytes)
      return kIOReturnBadArgument;
   if (context->count == context->slots) {
      IOLog("OpenFWOHCI: %s queue full\n", context->name);
      return kIOReturnNoResources;
   }
   slot = (context->head + context->count) % context->slots;
   descriptor = context->descriptors.bytes + slot * kOpenFWOHCIATSlotDescriptorBytes;
   address = context->descriptors.address + slot * kOpenFWOHCIATSlotDescriptorBytes;
   immediate = descriptor + 16;
   bzero(descriptor, kOpenFWOHCIATSlotDescriptorBytes);
   for (UInt32 i = 0; i < headerBytes / 4; i++) {
      if (quadletData && i == 3)
         bcopy(&header[3], immediate + 12, 4);
      else
         putWord(immediate, i, header[i]);
   }
   if (payloadBytes == 0) {
      putWord(descriptor, 0, kOHCIDescriptorOutputLast | kOHCIDescriptorKeyImmediate | kOHCIDescriptorInterruptAlways |
         kOHCIDescriptorBranchAlways | headerBytes);
      context->statusWord[slot] = 3;
      context->branchWord[slot] = 2;
      z = 2;
   } else {
      UInt8 *payload = context->payloads.bytes + slot * kOpenFWOHCIATPayloadBytes;
      if (buf) {
         if (buf->readBytes(offset, payload, payloadBytes) != payloadBytes)
            return kIOReturnUnderrun;
      } else {
         bcopy(bytes, payload, payloadBytes);
      }
      putWord(descriptor, 0, kOHCIDescriptorOutputMore | kOHCIDescriptorKeyImmediate | headerBytes);
      putWord(descriptor, 8, kOHCIDescriptorOutputLast | kOHCIDescriptorInterruptAlways | kOHCIDescriptorBranchAlways | payloadBytes);
      putWord(descriptor, 9, context->payloads.address + slot * kOpenFWOHCIATPayloadBytes);
      context->statusWord[slot] = 11;
      context->branchWord[slot] = 10;
      z = 3;
   }
   context->commands[slot] = cmd;
   context->count++;
   syncMemory();
   if (context->running) {
      UInt32 previous = (slot + context->slots - 1) % context->slots;
      putWord(context->descriptors.bytes + previous * kOpenFWOHCIATSlotDescriptorBytes, context->branchWord[previous], address | z);
      syncMemory();
      writeRegister(context->registers + kOHCIContextControlSet, kOHCIContextWake);
   } else {
      writeRegister(context->registers + kOHCIContextCommandPtr, address | z);
      writeRegister(context->registers + kOHCIContextControlSet, kOHCIContextRun);
      context->running = true;
   }
   return kIOReturnSuccess;
}

void OpenFWOHCI::completeATContext(OpenFWOHCIATContext *context)
{
   if (context->completing)
      return;
   context->completing = true;
   while (context->count) {
      UInt32 slot = context->head;
      UInt32 status = getWord(context->descriptors.bytes + slot * kOpenFWOHCIATSlotDescriptorBytes, context->statusWord[slot]) >> 16;
      IOFWAsyncCommand *cmd;

      if (status == 0)
         break;
      cmd = context->commands[slot];
      context->commands[slot] = 0;
      context->head = (slot + 1) % context->slots;
      context->count--;
      if (cmd)
         cmd->gotAck(ackFromEvent(status & kOHCIContextEventMask));
   }
   context->completing = false;
}

IOReturn OpenFWOHCI::handleAsyncCompletion(IOFWCommand *cmd, IOReturn status)
{
   for (UInt32 i = 0; i < fATRequest.count; i++) {
      UInt32 slot = (fATRequest.head + i) % fATRequest.slots;
      if (fATRequest.commands[slot] == cmd)
         fATRequest.commands[slot] = 0;
   }
   return kIOReturnSuccess;
}

IOReturn OpenFWOHCI::handleAsyncTimeout(IOFWAsyncCommand *cmd)
{
   return kIOReturnSuccess;
}

void OpenFWOHCI::flushWaitingPackets(void)
{
   if (!fLinkOn)
      return;
   completeATContext(&fATRequest);
   completeATContext(&fATResponse);
   drainARContext(&fARResponse);
   drainARContext(&fARRequest);
}

IOReturn OpenFWOHCI::asyncRead(UInt16 nodeID, UInt16 addrHi, UInt32 addrLo, int speed, int label, int size, IOFWAsyncCommand *cmd, IOFWReadFlags flags)
{
   bool block = size != 4 || (flags & kIOFWReadBlockRequest);
   UInt32 header[4];

   header[0] = asyncHeader(block ? kFWTCodeReadBlock : kFWTCodeReadQuadlet, speed, label);
   header[1] = (nodeID << 16) | addrHi;
   header[2] = addrLo;
   header[3] = size << 16;
   return queuePacket(&fATRequest, header, block ? 16 : 12, false, 0, 0, 0, 0, cmd);
}

IOReturn OpenFWOHCI::asyncWrite(UInt16 nodeID, UInt16 addrHi, UInt32 addrLo, int speed, int label, IOMemoryDescriptor *buf, IOByteCount offset, int size, IOFWAsyncCommand *cmd, IOFWWriteFlags flags)
{
   bool block = size != 4 || (flags & kIOFWWriteBlockRequest);
   UInt32 header[4];

   header[0] = asyncHeader(block ? kFWTCodeWriteBlock : kFWTCodeWriteQuadlet, speed, label);
   header[1] = (nodeID << 16) | addrHi;
   header[2] = addrLo;
   if (block) {
      header[3] = size << 16;
      return queuePacket(&fATRequest, header, 16, false, buf, offset, 0, size, cmd);
   }
   if (buf->readBytes(offset, &header[3], 4) != 4)
      return kIOReturnUnderrun;
   return queuePacket(&fATRequest, header, 16, true, 0, 0, 0, 0, cmd);
}

IOReturn OpenFWOHCI::asyncLock(UInt16 nodeID, UInt16 addrHi, UInt32 addrLo, int speed, int label, int type, IOMemoryDescriptor *buf, IOByteCount offset, int length, IOFWAsyncCommand *cmd)
{
   UInt32 header[4];

   header[0] = asyncHeader(kFWTCodeLock, speed, label);
   header[1] = (nodeID << 16) | addrHi;
   header[2] = addrLo;
   header[3] = (length << 16) | (type & 0xFFFF);
   return queuePacket(&fATRequest, header, 16, false, buf, offset, 0, length, cmd);
}

IOReturn OpenFWOHCI::asyncReadQuadResponse(UInt16 nodeID, int speed, int label, int rcode, UInt32 data)
{
   UInt32 header[4];

   header[0] = asyncHeader(kFWTCodeReadQuadletResponse, speed, label);
   header[1] = (nodeID << 16) | ((rcode & 0xF) << 12);
   header[2] = 0;
   header[3] = data;
   return queuePacket(&fATResponse, header, 16, true, 0, 0, 0, 0, 0);
}

IOReturn OpenFWOHCI::asyncReadResponse(UInt16 nodeID, int speed, int label, int rcode, IOMemoryDescriptor *buf, IOByteCount offset, int len, IODMACommand *dmaCommand)
{
   UInt32 header[4];

   if (rcode != kFWResponseComplete)
      len = 0;
   header[0] = asyncHeader(kFWTCodeReadBlockResponse, speed, label);
   header[1] = (nodeID << 16) | ((rcode & 0xF) << 12);
   header[2] = 0;
   header[3] = len << 16;
   return queuePacket(&fATResponse, header, 16, false, len ? buf : 0, offset, 0, len, 0);
}

IOReturn OpenFWOHCI::asyncWriteResponse(UInt16 nodeID, int speed, int label, int rcode, UInt16 addrHi)
{
   UInt32 header[3];

   header[0] = asyncHeader(kFWTCodeWriteResponse, speed, label);
   header[1] = (nodeID << 16) | ((rcode & 0xF) << 12);
   header[2] = 0;
   return queuePacket(&fATResponse, header, 12, false, 0, 0, 0, 0, 0);
}

IOReturn OpenFWOHCI::asyncLockResponse(UInt16 nodeID, int speed, int label, int rcode, int type, void *data, int len)
{
   UInt32 header[4];

   if (rcode != kFWResponseComplete)
      len = 0;
   header[0] = asyncHeader(kFWTCodeLockResponse, speed, label);
   header[1] = (nodeID << 16) | ((rcode & 0xF) << 12);
   header[2] = 0;
   header[3] = (len << 16) | (type & 0xFFFF);
   return queuePacket(&fATResponse, header, 16, false, 0, 0, data, len, 0);
}

void OpenFWOHCI::initARContext(OpenFWOHCIARContext *context)
{
   for (UInt32 i = 0; i < kOpenFWOHCIARBuffers; i++) {
      UInt8 *descriptor = context->descriptors.bytes + i * 16;
      putWord(descriptor, 0, kOHCIDescriptorInputMore | kOHCIDescriptorStatus | kOHCIDescriptorBranchAlways | kOpenFWOHCIARBufferBytes);
      putWord(descriptor, 1, context->data.address + i * kOpenFWOHCIARBufferBytes);
      putWord(descriptor, 2, i + 1 < kOpenFWOHCIARBuffers ? (context->descriptors.address + (i + 1) * 16) | 1 : 0);
      putWord(descriptor, 3, kOpenFWOHCIARBufferBytes);
   }
   context->position = 0;
   context->last = kOpenFWOHCIARBuffers - 1;
   context->draining = false;
   syncMemory();
   writeRegister(context->registers + kOHCIContextCommandPtr, context->descriptors.address | 1);
   writeRegister(context->registers + kOHCIContextControlSet, kOHCIContextRun);
}

UInt32 OpenFWOHCI::filledARBytes(OpenFWOHCIARContext *context, UInt32 buffer)
{
   UInt32 residual = getWord(context->descriptors.bytes + buffer * 16, 3) & 0xFFFF;

   return residual > kOpenFWOHCIARBufferBytes ? 0 : kOpenFWOHCIARBufferBytes - residual;
}

UInt32 OpenFWOHCI::availableARBytes(OpenFWOHCIARContext *context)
{
   UInt32 buffer = context->position / kOpenFWOHCIARBufferBytes;
   UInt32 offset = context->position % kOpenFWOHCIARBufferBytes;
   UInt32 total = 0;

   for (UInt32 n = 0; n < kOpenFWOHCIARBuffers; n++) {
      UInt32 filled = filledARBytes(context, buffer);
      if (filled < offset)
         break;
      total += filled - offset;
      if (filled < kOpenFWOHCIARBufferBytes || buffer == context->last)
         break;
      offset = 0;
      buffer = (buffer + 1) % kOpenFWOHCIARBuffers;
   }
   return total;
}

void OpenFWOHCI::copyARBytes(OpenFWOHCIARContext *context, UInt32 position, UInt8 *to, UInt32 length)
{
   UInt32 first = kARRingBytes - position;

   if (first > length)
      first = length;
   bcopy(context->data.bytes + position, to, first);
   if (first < length)
      bcopy(context->data.bytes, to + first, length - first);
}

void OpenFWOHCI::recycleARBuffer(OpenFWOHCIARContext *context, UInt32 buffer)
{
   UInt8 *descriptor = context->descriptors.bytes + buffer * 16;

   putWord(descriptor, 2, 0);
   putWord(descriptor, 3, kOpenFWOHCIARBufferBytes);
   syncMemory();
   putWord(context->descriptors.bytes + context->last * 16, 2, (context->descriptors.address + buffer * 16) | 1);
   context->last = buffer;
   syncMemory();
   writeRegister(context->registers + kOHCIContextControlSet, kOHCIContextWake);
}

void OpenFWOHCI::drainARContext(OpenFWOHCIARContext *context)
{
   if (context->draining || !fLinkOn)
      return;
   context->draining = true;
   for (;;) {
      UInt32 available = availableARBytes(context);
      UInt32 tcode, headerBytes, swapWords, payloadBytes = 0, total, trailer, speed, before, after;

      if (available < 4)
         break;
      copyARBytes(context, context->position, fPacket, 4);
      tcode = (getWord(fPacket, 0) >> kOHCIAsyncTCodeShift) & 0xF;
      switch (tcode) {
      case kFWTCodeWriteResponse:
      case kFWTCodeReadQuadlet:
      case kFWTCodePHYPacket:
         headerBytes = 12;
         swapWords = 3;
         break;
      case kFWTCodeWriteQuadlet:
      case kFWTCodeReadQuadletResponse:
         headerBytes = 16;
         swapWords = 3;
         break;
      case kFWTCodeReadBlock:
      case kFWTCodeWriteBlock:
      case kFWTCodeReadBlockResponse:
      case kFWTCodeLock:
      case kFWTCodeLockResponse:
         headerBytes = 16;
         swapWords = 4;
         break;
      default:
         IOLog("OpenFWOHCI: %s: unknown tcode %u, restarting the context\n", context->name, (unsigned)tcode);
         stopContext(context->registers);
         initARContext(context);
         goto done;
      }
      if (available < headerBytes)
         break;
      copyARBytes(context, context->position, fPacket, headerBytes);
      if (tcode == kFWTCodeWriteBlock || tcode == kFWTCodeReadBlockResponse || tcode == kFWTCodeLock || tcode == kFWTCodeLockResponse)
         payloadBytes = ((getWord(fPacket, 3) >> 16) + 3) & ~3;
      total = headerBytes + payloadBytes + 4;
      if (total > kOpenFWOHCIPacketBytes) {
         IOLog("OpenFWOHCI: %s: %u-byte packet, restarting the context\n", context->name, (unsigned)total);
         stopContext(context->registers);
         initARContext(context);
         goto done;
      }
      if (available < total)
         break;
      copyARBytes(context, context->position, fPacket, total);
      before = context->position / kOpenFWOHCIARBufferBytes;
      context->position = (context->position + total) % kARRingBytes;
      after = context->position / kOpenFWOHCIARBufferBytes;
      while (before != after) {
         recycleARBuffer(context, before);
         before = (before + 1) % kOpenFWOHCIARBuffers;
      }
      if (tcode == kFWTCodePHYPacket)
         continue;
      trailer = getWord(fPacket, (headerBytes + payloadBytes) / 4) >> 16;
      speed = (trailer >> kOHCIContextSpeedShift) & kOHCIContextSpeedMask;
      for (UInt32 i = 0; i < swapWords; i++)
         ((UInt32 *)fPacket)[i] = getWord(fPacket, i);
      processRcvPacket((UInt32 *)fPacket, (headerBytes + payloadBytes) / 4, (IOFWSpeed)(speed > kFWSpeed400MBit ? kFWSpeed400MBit : speed));
   }
done:
   context->draining = false;
}
