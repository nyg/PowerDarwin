# Step 12: FireWire

> Step 12 is not reached yet. The image now has an open driver for the iMac G5's FireWire controller, and with no device attached the FireWire bus comes up cleanly, but no FireWire disk has been tested. This chapter tells what was built and what has been shown so far.

## What you will learn

- What a FireWire link driver does, and why the iMac G5 (`PowerMac12,1`) had none under Darwin 9.
- How an OHCI-1394 controller is driven: bus resets, self-ID packets, asynchronous DMA contexts and physical requests.
- How OpenFWOHCI fills in IOFireWireFamily's `IOFireWireLink` for the iMac's controller.
- Why IOFireWireFamily itself did not load, and the class that its published source leaves out.

## The goal

README.md sets it as: "On the iMac G5, a FireWire disk mounts as it is plugged in, through an open driver for the FireWire controller."

[Step 9](step-9-disks-and-devices.md) built IOFireWireFamily, IOFireWireSBP2 and IOFireWireSerialBusProtocolTransport, but left FireWire disks out: the iMac's controller needs `AppleFWOHCI`, a closed driver. "Mounts as it is plugged in" asks for the whole chain, from the controller to a mounted volume, with no command typed.

## Background: a bus and its controller

### FireWire

FireWire, IEEE 1394, is a serial bus for disks, cameras and audio devices. Up to 63 nodes share a bus, joined by cables in a tree, and each node has a PHY, which drives the cables and arbitrates for the bus, and usually a link, which sends and receives packets through its PHY. Whenever a device is plugged in or removed, the bus resets: every PHY forgets the tree, the nodes find it again, and each PHY sends a self-ID packet that says what node number it took, how many ports it has and how fast it can talk. After a reset every node knows every other, and the software above starts over from the new list.

Nodes talk in two ways. Asynchronous transactions read, write or lock a 48-bit address on another node, with a request and a response that each carry a transaction label. Isochronous streams send data at a fixed rate every 125 µs, for audio and video. Every node also publishes a config ROM at a fixed address, which says who made it and what units it holds; a FireWire disk's unit says that it speaks SBP-2, which carries SCSI commands over the bus. An SBP-2 disk reads its commands, called ORBs, from the host's memory, and writes its data there, by sending asynchronous requests to the host's own addresses.

### OHCI-1394

Most FireWire controllers of the 2000s follow the 1394 Open Host Controller Interface, OHCI-1394 (not to be confused with USB's OHCI): one register layout and one DMA model for any maker's chip. The host gives the controller chains of descriptors in memory, and four DMA contexts carry asynchronous packets, one each to send requests, send responses, receive requests and receive responses. A controller can also answer requests for host memory by itself: physical requests, which reach the host's memory directly when the node that sends them is let through the controller's physical request filter. That is how SBP-2 disks reach their ORBs and data.

The iMac's controller is part of the K2 I/O controller ([PowerPC Macs](../background/powerpc-macs.md)): the device tree's `firewire@E`, compatible `pci106b,52`, PCI class `0c0010`, with 4 KB of registers and interrupt `0x27`. Its `Version` register reads OHCI 1.0, and its PHY says it is a 1394a PHY with two ports and a top speed of S400, 400 Mbit/s. A driver turns its clock on through `EnableFireWireClock`, a platform function that our AppleK2 publishes.

### The family's link layer

IOFireWireFamily splits a FireWire stack in two. The family's `IOFireWireController` does everything that does not depend on the chip: it scans the bus after each reset, reads config ROMs, publishes a nub for each device and unit, retries transactions and manages the bus. A link driver, a subclass of `IOFireWireLink`, does everything that does: it resets the chip, turns self-ID packets into the family's list, sends and receives packets, and sets the physical request filter. `IOFireWireLink` has 33 pure virtual functions, which a subclass must write. The link creates the controller with `createController`, attaches it and starts it; the controller then calls back through those functions.

Before writing anything, the project looked for Apple's own link driver, as rule 2 asks. Apple never published `AppleFWOHCI`, nor `AppleLynx`, its driver for older controllers: none of IOFireWireFamily's 54 tags on GitHub, from 37 to 492, nor xnu's tags, nor the 2015 index of opensource.apple.com's tarballs, nor the Darwin 0.x to 1.4 project lists holds one. Darwin 1.0's xnu has the family in `iokit/Families/IOFireWire/` and no controller driver. Darwin 1.0's and 1.2.1's release notes list FireWire devices as unsupported, and darwinbuild's list for Mac OS X 10.4 has AppleFWOHCI 244.4.0 among its binary drivers.

## OpenFWOHCI

[replacements/OpenFWOHCI/](https://github.com/nyg/PowerDarwin/blob/master/replacements/OpenFWOHCI/) is a new kext, under the MIT licence ([D119](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d119)), about 1,600 lines of C++ in `OpenFWOHCI.cpp` (the chip, its interrupts and its power), `Async.cpp` (the DMA contexts) and `OHCI.h` (the register layout). The registers, bits and descriptor formats come from the OHCI 1.1 specification; FreeBSD's `fwohci.c`, under the BSD licence, and Linux's `firewire-ohci`, under the GPL, were read for facts about real chips. No code comes from either.

### Matching and start

Its personality matches an `IOPCIDevice` with `IOPCIMatch` `0x0052106b`, the K2's controller only. `start` turns the clock on through `EnableFireWireClock`, maps the registers, resets the chip and waits until its link power status bit reads back as set, before it touches any other register. It reads the controller's GUID, the node's 64-bit unique ID, from the chip or from the device tree's `local-guid`, probes the PHY, allocates its DMA buffers and its work loop, joins power management with the family's two power states, and creates and starts the controller. When the family turns the link's power on, `setLinkPowerState` programs the chip: the bus options, the first config ROM, the self-ID buffer, the retry limits, the DMA contexts, the interrupts, and last the link enable bit, which lets the chip take part in bus resets.

It logs one line:

```text
OpenFWOHCI: OHCI 1.0, PHY 001018:035462, 1394a, 2 ports, S400
```

The four numbers are published as `OHCI Version`, `PHY Vendor_ID` and `PHY Device_ID` in the IORegistry.

### Bus resets and self-IDs

The interrupt filter, at the K2's interrupt, reads and clears the chip's events and leaves the bus reset event masked until the self-IDs arrive; the work loop handles the rest. On a bus reset it tells the family, stops both send contexts and fails what they held, so that the family retries it. When the chip has written the self-ID packets into its buffer, the driver checks their generation before and after copying them, splits its own packets from the others, builds its own from the PHY's registers if the chip did not store them, and hands both lists to `processSelfIDs`. A new config ROM given by `updateROM` takes effect at the next reset, as OHCI requires.

### Asynchronous packets

Each send context has a ring of slots, each with room for up to three descriptors and a 2 KB payload: a packet's header goes in an immediate descriptor, and its payload, copied from the family's memory descriptor, in the next one. Responses carry the time after which the chip drops them, which OHCI asks of every response. The driver builds each header from the family's call in host order and writes it little-endian, as OHCI expects; payloads stay in bus order. When a slot's status comes back, the driver turns the chip's event code into the acknowledgement the family expects, `kFWAckComplete` or `kFWAckPending` for instance, and calls `gotAck`.

Each receive context is a ring of sixteen 4 KB buffers that the chip fills one packet after another, with a trailer giving the speed and the event. The driver copies each packet out across buffer ends, turns its header to host order and passes it to `processRcvPacket`, then gives the emptied buffers back to the chip.

PHY packets go out on the request context. The family sends them, such as the one that sets every PHY's gap count, and the driver waits for a queued one to leave before it starts a bus reset.

### Physical requests

SBP-2's buffers are `IOFWPhysicalAddressSpace` objects, which the family maps through the U4's DART, the iMac's I/O memory map ([PowerPC Macs](../background/powerpc-macs.md)), so that each has a 32-bit bus address. `setNodeIDPhysicalFilter` opens or closes the chip's physical request filter for one node or for all, and the chip then answers that node's reads and writes of those addresses by itself.

### What it leaves out

OpenFWOHCI has no isochronous DMA: `createDCLProgram` returns none and `asyncStreamTransmit` returns `kIOReturnUnsupported`, so FireWire audio and video devices cannot stream. It does not drive other controllers, and it has no sleep or wake handling, which the image does not support anyway.

## What broke

### A class left out of IOFireWireFamily

The first image with the four FireWire kexts booted cleanly, but logged:

```text
kextd[22]: a link/load error occured for kernel extension /System/Library/Extensions/OpenFWOHCI.kext
```

Loading the family by hand showed that it was the family itself that failed:

```text
Can't find superclass for 40IOFWSimpleContiguousPhysicalAddressSpace : 30IOFWSimplePhysicalAddressSpace
```

Our IOFireWireFamily binary had `IOFWSimpleContiguousPhysicalAddressSpace`, which SBP-2 uses for its ORBs, but not its superclass `IOFWSimplePhysicalAddressSpace`: Leopard's own family defines it, and ours left its vtable and metaclass undefined. The build had compiled the class's source file, `IOFireWireFamily.kmodproj/IOFWSimplePhysicalAddressSpace.cpp`, which in 349.4.0 holds only a copyright line. So does it in every release from 344.4.0 to 492. In 338.4.0 the file is complete, but its code sits inside `#ifndef _OPEN_SOURCE_` and it has no licence notice, so the project does not use it.

The class is small, and its header and its subclass's source are published. [D120](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d120) writes it anew in a patch to 349.4.0: `init` allocates the address space's buffer with `IOBufferMemoryDescriptor`, physically contiguous when asked, and hands it to `IOFWPhysicalAddressSpace::initWithDesc`, which maps it; `getVirtualAddress` returns it; `free` releases it. With it, `kextload -n -t` on the build machine says that the family, IOFireWireSBP2 and IOFireWireSerialBusProtocolTransport "appear to be loadable".

### The wrong kernel interface

The same check then failed on OpenFWOHCI itself, with two undefined symbols, `IODMACommand::withSpecification` and `IODMACommand::OutputHost32`. Like OpenSMU, it named `com.apple.kernel.6.0` as its kernel library, the interface of Mac OS X 10.2, which predates `IODMACommand`. It now names the 9.0 versions of `com.apple.kpi.iokit`, `com.apple.kpi.libkern` and `com.apple.kpi.mach`, which export it.

### A cycle at unload

`kextunload` of OpenFWOHCI then took its instance out of the IORegistry but left the kext loaded, with `OpenFWOHCI has 1 instance(s)`. The family's controller retains its link in `init` and releases it in `free`, and OpenFWOHCI released the controller only in its own `free`, so neither object could be freed. OpenFWOHCI now releases the controller in `stop`, once the link is powered down. Termination runs apart from `kextunload`, so the first `kextunload` after a load still finds the instance, and the second unloads the kext.

## How it was proved

### Builds

darwinbuild built OpenFWOHCI from its Makefile in 7 seconds, and the patched IOFireWireFamily in 1 minute 51 seconds, with the one warning that the unpatched family also gives (`'newGap' may be used uninitialized`). The image with the four FireWire kexts holds 28,312 files, 14 more than [Step 13](step-13-gpu-idle-heat.md)'s, and `check-image` finds the same 23 not from the builds.

### On the iMac, with no FireWire device

Three boots of the image ran on the iMac, with nothing plugged into its FireWire port. The second, with the patched family, loaded the family (version 3.4.9, from 349.4.0) and OpenFWOHCI 4 seconds after `kextd` catalogued its 109 personalities. The registry then showed one bus reset handled, `FireWire Generation ID` 1, the iMac's own self-ID packet alone, `0x807fc052`, the local node at `FireWire Node ID` 65472 (`0xffc0`, node 0 of the local bus), and `FireWire Speed` 2 (S400). The generation was still 1 a minute after boot, so no further reset had happened. `system.log` held only the two lines that [Step 8](step-8-clean-boot.md) accepts and OpenFWOHCI's own line.

The third boot ran the build with the fix for the cycle at unload, copied into the image by hand. It loaded the same way. `kextunload` and `kextload` of OpenFWOHCI then ran twice in a row: each time the kext unloaded, the family's controller, link and work loop were freed, as the kernel's instance counts showed, and the new instance brought the bus up again, at generation 1 with the same self-ID. The build in the repo is that build, made again by darwinbuild.

## What is still missing

- No FireWire device has been tested: reading a device's config ROM, SBP-2 logins, physical requests, a disk mounting read-only and read-write, unplugging it and plugging it in again, and a restart with a disk attached. The step is reached when those pass with a disk that holds nothing of value.
- No isochronous DMA: FireWire audio and video devices cannot stream.
- OpenFWOHCI drives one controller on one machine. Other Macs' FireWire controllers are untested and not matched.
- No sleep or wake handling.
- `kextunload` needs two attempts after a load, as above.

## Where to look

- [replacements/OpenFWOHCI/](https://github.com/nyg/PowerDarwin/blob/master/replacements/OpenFWOHCI/), and [D119](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d119), its row.
- [patches/IOFireWireFamily-349.4.0/0001-simple-physical-address-space.patch](https://github.com/nyg/PowerDarwin/blob/master/patches/IOFireWireFamily-349.4.0/0001-simple-physical-address-space.patch) and [D120](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d120).
- In IOFireWireFamily-349.4.0: `IOFireWireFamily.kmodproj/IOFireWireLink.h`, the link's interface; `IOFireWireController.cpp`, whose `init` retains the link and `free` releases it; `IOFWPhysicalAddressSpace.cpp` and `IOFWSimpleContiguousPhysicalAddressSpace.cpp`, the classes around the missing one.
- In AppleKeyLargo-172.3.1: AppleK2's `EnableFireWireClock` platform function.
- The 1394 Open Host Controller Interface specification, release 1.1; FreeBSD's [`sys/dev/firewire/fwohci.c`](https://github.com/freebsd/freebsd-src/blob/main/sys/dev/firewire/fwohci.c); Linux's [`drivers/firewire/ohci.c`](https://github.com/torvalds/linux/blob/master/drivers/firewire/ohci.c), read for facts only.
- [Step 9](step-9-disks-and-devices.md), which built the family, SBP-2 and its SCSI transport.
- [hardware/powermac12-1.md](https://github.com/nyg/PowerDarwin/blob/master/hardware/powermac12-1.md): the FireWire row.
