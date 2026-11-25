# Glossary

Each term in a sentence or two, with the chapter that explains it.

**16.16 fixed point**: an integer whose upper 16 bits are whole units and lower 16 bits fractions, so 65,536 stands for 1 °C. OpenSMU and Apple's drivers keep temperatures in it. See [OpenSMU](../synthesis/opensmu.md).

**`9A581`, `9L30`**: Apple's build numbers for Mac OS X 10.5.0 and 10.5.8. The project builds the versions of `9L30`. See [Leopard 9L30](../method/sources-and-roots.md).

**ADC**: analog-to-digital converter, which turns a voltage into a number. The SMU's ADC reads the CPU diode. See [Step 7](../steps/step-7-fans.md).

**AppleFWOHCI**: Apple's driver of the FireWire controller, a plug-in of IOFireWireFamily that no published source holds. Without it, or our OpenFWOHCI in its place, no FireWire device appears. See [Step 9](../steps/step-9-disks-and-devices.md) and [Step 12](../steps/step-12-firewire.md).

**AppleGPIO**: the driver of the general-purpose input and output lines of PowerPC Macs. On the iMac G5 it rings the SMU's doorbell. See [Step 5](../steps/step-5-clock-and-power.md).

**AppleK2**: the driver of the K2 I/O controller on G5 Macs. On the iMac G5 it also hands the SMU the address of its command buffer. See [Step 5](../steps/step-5-clock-and-power.md).

**AppleMacRISC4PE**: the platform driver of G5 Macs such as the iMac G5: it knows the machine's buses, interrupts and processors. See [Step 1, part 2](../steps/step-1-drivers-and-bootx.md).

**AppleOnboardAudio**: the driver of Apple's built-in sound, with AppleDBDMAAudio as its engine and one plug-in per codec. See [Step 9](../steps/step-9-disks-and-devices.md).

**AppleOnyxAudio, OpenOnyx**: Apple's plug-in for the iMac G5's PCM3052 codec, published only as a stub, and the project's open stand-in for it ([D94](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d94)). See [Step 9](../steps/step-9-disks-and-devices.md).

**apple-oss-distributions**: Apple's GitHub organisation, created on 4 October 2021, with one repository per open-source project and one tag per version. See [Leopard 9L30](../method/sources-and-roots.md) and [Darwin's history](../background/history.md).

**Apple Partition Map**: the partition table of PowerPC Macs' disks, read by Open Firmware. See [the boot chain](../background/boot-chain.md).

**ApplePMU, AppleSMU**: Apple's closed drivers for the PMU and the SMU. Leopard ships them as binaries only, so the image has neither; OpenPMU and OpenSMU stand in for them. See [What stays closed](../synthesis/what-stays-closed.md).

**APSL**: the Apple Public Source License 2.0, the licence of most of Darwin's projects. See [What Darwin is](../background/what-darwin-is.md).

**`asr`**: Apple Software Restore, Leopard's tool that copies a disk image onto a partition. See [How testing is done](../method/testing.md).

**ASPM, L0s, L1**: Active State Power Management, by which each end of a PCI Express link puts its transmitter into a low-power state while it has nothing to send: L0s, left within a fraction of a microsecond, or the deeper L1. Software turns it on in each end's Link Control register. See [Step 13](../steps/step-13-gpu-idle-heat.md).

**ATAPI**: SCSI commands carried over an ATA bus, the way CD and DVD drives are driven. In Darwin, IOATAPIProtocolTransport carries them. See [Step 1, part 2](../steps/step-1-drivers-and-bootx.md).

**`auto-boot?`**: the NVRAM variable that says whether Open Firmware boots at power-on (`true`) or stops at its prompt. See [the boot chain](../background/boot-chain.md).

**`AutomountDisksWithoutUserLogin`**: the setting in `/Library/Preferences/SystemConfiguration/autodiskmount.plist` with which DiskArbitration mounts removable and external disks without a console user, as on Apple's servers. `make-test-image` writes it. See [Step 9](../steps/step-9-disks-and-devices.md).

**backport**: a fix taken from a later Apple release of the same project and applied to the `9L30` version, the second choice of rule 2. See [The rules](../method/rules.md).

**BCD**: binary-coded decimal, a way of storing a number with each decimal digit in four bits, so that 59 is stored as the byte `0x59`. The SMU keeps its clock this way. See [Step 5](../steps/step-5-clock-and-power.md).

**bison, `y.tab.c`**: bison turns a grammar (`parse.y`) into a C parser (`y.tab.c`). `make` makes the parser again only when the grammar is newer, which D20 relies on. See [Step 1, part 1](../steps/step-1-kernel-and-userland.md).

**`bitrot`**: the kind of deviation for what worked when Apple built Leopard and no longer does: a dead URL, or a tool that has changed. See [The rules](../method/rules.md).

**`bless`**: the command that blesses a folder, puts BootX in it and sets `boot-device`. See [the boot chain](../background/boot-chain.md).

**blessed folder**: the folder of an HFS+ volume that holds its boot loader, named by its ID in the volume header's Finder information. On Mac OS X it is `/System/Library/CoreServices`. See [the boot chain](../background/boot-chain.md).

**`boot-args`**: the NVRAM variable holding the kernel's arguments, such as `-v -s`. See [the boot chain](../background/boot-chain.md).

**BootCache**: a kext and a tool, `BootCacheControl`, that record which disk blocks a boot reads, in the playlist `/var/db/BootCache.playlist`, and read them ahead at the next boot. See [Step 8](../steps/step-8-clean-boot.md).

**`boot-device`**: the NVRAM variable holding the device path Open Firmware boots from, such as `hd:,\\:tbxi`. See [the boot chain](../background/boot-chain.md).

**boot loader**: the small program that firmware loads from a disk, which loads the kernel and what it needs, describes the machine to it and jumps to it. On PowerPC Macs it is BootX. See [Hardware and boot basics](../background/hardware-basics.md).

**boot ROM**: the memory chip that keeps its content with the power off and holds the firmware. On the iMac G5 it is a flash chip that also holds NVRAM. See [Hardware and boot basics](../background/hardware-basics.md).

**bootstrap namespace**: the set of service names a process can look up through its bootstrap port. launchd keeps a root namespace for the system and subsets for logins. See [Step 6](../steps/step-6-securityd.md).

**bootstrap port**: the Mach port, inherited from the parent, through which a process asks launchd to look up a service by name. See [Step 6](../steps/step-6-securityd.md).

**bootstrap subset**: a child namespace that sees every name of its parent, while names registered in it are seen only inside it. See [Step 6](../steps/step-6-securityd.md).

**`boot-uuid`**: the root volume's UUID, which BootX writes into the device tree's `/chosen` node and the kernel uses to find its root partition. See [the boot chain](../background/boot-chain.md).

**BootX**: the boot loader of Mac OS X and Darwin on PowerPC Macs. Open Firmware starts it, and it loads the kernel and the driver cache. See [the boot chain](../background/boot-chain.md).

**`bootx.bootinfo`**: BootX as Open Firmware loads it: a `CHRP-BOOT` text header with a Forth script, in front of the XCOFF program, copied into the blessed folder as `BootX`. See [the boot chain](../background/boot-chain.md) and [Step 1, part 2](../steps/step-1-drivers-and-bootx.md).

**bridge**: a chip that joins two buses and passes traffic between them, such as a PCI-to-PCI bridge. See [Hardware and boot basics](../background/hardware-basics.md).

**BSD layer**: the part of xnu, mostly from FreeBSD, that gives programs processes, files, sockets and the POSIX system calls. See [What Darwin is](../background/what-darwin-is.md).

**`bug`**: the kind of deviation for a fix to Apple's source or build. See [The rules](../method/rules.md).

**build alias**: a build-list entry that builds part of another project's source, such as `Libc_debug` from Libc or `launchd_libs` from launchd. See [Leopard 9L30](../method/sources-and-roots.md).

**build order**: `manifest/build-order`, every darwinbuild run and `prepare-buildroot` step of an image, in the order `build-image` runs them. See [darwinbuild and the build machine](../method/darwinbuild.md).

**build configuration**: a named set of Xcode settings for one project, chosen with `xcodebuild -configuration`. Security's `Darwin` configuration builds it without its closed parts ([D51](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d51)). See [Kinds of deviation](../synthesis/kinds-of-deviation.md).

**`build-env`**: the kind of deviation that changes the build environment and no source. See [The rules](../method/rules.md).

**build list**: darwinbuild's property list for one build: each project's version, dependencies and build settings. Each later list names only what changed and inherits the rest from the one before, down to `9A581`. See [Leopard 9L30](../method/sources-and-roots.md).

**build machine**: the PowerPC Mac running Leopard 10.5.8, Xcode 3.1.4 and darwinbuild that builds every project and makes the image. See [darwinbuild and the build machine](../method/darwinbuild.md).

**build number**: Apple's name for one build of Mac OS X, such as `9L30`: the major release, a letter for each update, and a count of builds. See [Leopard 9L30](../method/sources-and-roots.md).

**BuildRoot**: the disk image that darwinbuild builds each project in, used as a chroot. See [darwinbuild and the build machine](../method/darwinbuild.md).

**bus**: a shared path between chips, with its own rules for addresses and data, such as PCI, I²C or USB. See [Hardware and boot basics](../background/hardware-basics.md).

**bus reset**: on FireWire, the moment after a device is plugged in or removed when every node forgets the bus's tree and finds it again; each PHY then sends a self-ID packet. See [Step 12](../steps/step-12-firewire.md).

**byte order**: the order in which a processor stores the bytes of a number. PowerPC Macs are big-endian (most significant byte first), x86 is little-endian. See [Hardware and boot basics](../background/hardware-basics.md).

**`ByteRangeLocking.framework`**: a private, closed framework of Leopard's that Apple's Samba links; its `darwin_streams` module calls it only with `darwin_streams:brlm = yes` ([D107](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d107), [D108](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d108)). See [Step 10](../steps/step-10-working-unix.md).

**cache flush**: writing the processor's cached copy of some memory back to memory and dropping it, so that a device reading memory directly sees what the processor wrote, and the processor then reads what the device wrote. See [Hardware and boot basics](../background/hardware-basics.md).

**`callPlatformFunction`**: IOKit's call by which one driver asks another to perform a platform function. See [Step 5](../steps/step-5-clock-and-power.md).

**CarbonHeaders**: the project of `AvailabilityMacros.h`, `TargetConditionals.h`, `MacTypes.h` and other headers that many others include. Its `9L30` version has no source ([D10](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d10)); version 18.1, of Mac OS X 10.6, is on GitHub, and the image gets its nine headers ([D111](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d111)). See [Step 10](../steps/step-10-working-unix.md).

**cause**: why a deviation was needed, as against its kind, what sort of change it is. Chapter 25 sorts every row by both. See [Kinds of deviation](../synthesis/kinds-of-deviation.md).

**cctools, ld64**: Apple's project of the assembler and binary tools (`as`, `ar`, `nm`, `otool`, `lipo`), and Apple's linker for Leopard, `ld`. See [Step 10](../steps/step-10-working-unix.md).

**CDSA, CSSM**: the Common Data Security Architecture that Security.framework follows: a core, CSSM, with plug-in modules for cryptography, storage, certificates and trust. See [Step 3, part 1](../steps/step-3-security-and-daemons.md).

**`CFBundleVersion`**: a bundle's version in its `Info.plist`. For most of Apple's drivers it also gives the project version, `1.7.2f1` for 172.3.1. See [Leopard 9L30](../method/sources-and-roots.md).

**CFFileDescriptor**: a CoreFoundation run-loop source that calls a function when a file descriptor is readable. Only Apple's closed CoreFoundation has it ([D79](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d79)). See [Step 3, part 2](../steps/step-3-network-and-ssh.md).

**CF-Lite**: the open part of CoreFoundation, Apple's C library of strings, collections and property lists. Leopard's own CoreFoundation has more, and is closed. See [What Darwin is](../background/what-darwin-is.md).

**CFNotificationCenter**: CoreFoundation's notifications between processes. Only Apple's closed CoreFoundation has it, so every caller is cut out or left unbuilt. See [Step 3, part 1](../steps/step-3-security-and-daemons.md).

**CFOpenDirectory, OpenDirectory**: Leopard's interface to directory services (accounts, groups, passwords). Its C part, CFOpenDirectory, includes a private header of the closed CoreFoundation, so nothing built on it can be built here, such as `passwd`, `chpass` and `pam_securityserver`. See [Step 2](../steps/step-2-console-login.md).

**chroot**: a folder entered with `chroot`, so that programs inside see it as `/` and can load nothing outside it. The first userland was tried this way before it was booted. See [Step 1, part 1](../steps/step-1-kernel-and-userland.md).

**clock gating**: stopping the clock of a chip's block while the block is idle, which saves power without changing what software sees. An ATI GPU gates each block unless a force bit keeps its clock on. See [Step 13](../steps/step-13-gpu-idle-heat.md).

**closed, open, ours**: the book's three kinds of code: Apple's binary only; built here from Apple's published source; written by the project. See [What Darwin is](../background/what-darwin-is.md).

**`closed-dependency`**: the kind of deviation that turns off a feature because it needs closed Apple code. See [The rules](../method/rules.md).

**`closed-replacement`**: the kind of deviation that stands in for a closed Apple binary, such as OpenSMU. See [The rules](../method/rules.md).

**codec**: the chip that turns digital samples into an analog signal and back. On the iMac G5 it is a Texas Instruments PCM3052. See [Step 9](../steps/step-9-disks-and-devices.md).

**`com.apple.Boot.plist`**: the property list from which BootX reads the kernel's name and flags. See [the boot chain](../background/boot-chain.md).

**combined mode**: an I²C transfer that writes a register number, then reads from that register, in one go. See [OpenSMU](../synthesis/opensmu.md).

**command page**: the page of memory into which OpenSMU writes a command for the SMU, and the SMU writes its reply. See [OpenSMU](../synthesis/opensmu.md) and [Step 5](../steps/step-5-clock-and-power.md).

**compiler driver, `cc1`**: `cc` (or `gcc`) is a driver that runs `cc1`, the compiler proper, then `as` and `ld`. See [Step 10](../steps/step-10-working-unix.md).

**config ROM**: the block at a fixed address of each FireWire node that says who made it and which units it holds, such as an SBP-2 disk. See [Step 12](../steps/step-12-firewire.md).

**configd, dynamic store**: the System Configuration daemon and its table of the machine's current network state, filled by its plug-ins and read by `scutil`. See [Step 3, part 1](../steps/step-3-security-and-daemons.md).

**configuration space**: the standard registers of a PCI device that say what it is (vendor, device, class) and where its registers are. See [Hardware and boot basics](../background/hardware-basics.md).

**`configure`, autoconf**: the script with which many open projects test the system before they build, by compiling and linking small programs, often with `-g -O2`; autoconf generates it. See [Step 10](../steps/step-10-working-unix.md).

**console user**: the user logged in at the graphical console, whom loginwindow registers. DiskArbitration waits for one before it mounts removable or external disks. See [Step 9](../steps/step-9-disks-and-devices.md).

**CoreAudio**: Leopard's closed sound framework, through which programs play and record sound. See [Step 9](../steps/step-9-disks-and-devices.md).

**CoreFoundation**: Apple's C library of strings, collections, property lists and run loops. Darwin has its open part, CF-Lite. See [What Darwin is](../background/what-darwin-is.md).

**CPU diode**: a diode on the processor chip whose voltage changes with temperature, read through the SMU's ADC and calibrated with a slope and an offset. See [Step 7](../steps/step-7-fans.md).

**`crt1.o`, Csu**: the start-up code linked into every program, which calls `main`, and the project that builds it. See [Step 10](../steps/step-10-working-unix.md).

**CSP**: the cryptographic service provider, the module of Security that holds its cryptographic algorithms. See [Step 3, part 1](../steps/step-3-security-and-daemons.md).

**Cuda**: the power and keyboard controller of older PowerPC Macs, before the PMU. No Mac that Leopard supports has one. See [PowerPC Macs](../background/powerpc-macs.md).

**DAC**: digital-to-analog converter, the part of a codec that turns samples into a voltage for the speakers. See [Step 9](../steps/step-9-disks-and-devices.md).

**DART**: the IOMMU in the U3 and U4 north bridges, which translates the addresses devices use for DMA. AppleDART drives it. See [PowerPC Macs](../background/powerpc-macs.md).

**Darwin**: the open core of Mac OS X: xnu and the Unix userland around it. Darwin 9.8 is Leopard 10.5.8's. See [What Darwin is](../background/what-darwin-is.md).

**darwinbuild**: the tool that builds one Darwin project at a time from Apple's sources, in a BuildRoot, from a build list of exact versions. See [darwinbuild and the build machine](../method/darwinbuild.md).

**DarwinTools**: a project of Darwin 8.0's build list, `8A428`, that holds Apple's open `sw_vers` and `startupfiletool`; the image's `sw_vers` is built from it ([D121](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d121)). See [Step 10](../steps/step-10-working-unix.md).

**darwinxref**: darwinbuild's database tool, which loads the build lists and answers each project's version, dependencies and settings. See [darwinbuild and the build machine](../method/darwinbuild.md).

**`deferred`**: the kind of deviation that turns off a feature until the open-source project it needs is built. See [The rules](../method/rules.md).

**deployment target**: the oldest Mac OS X version a binary is built to run on, set with `MACOSX_DEPLOYMENT_TARGET`; Leopard's headers choose some symbols by it ([D17](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d17)). See [Kinds of deviation](../synthesis/kinds-of-deviation.md).

**deviation**: any difference from Apple's sources or from Apple's build: a patch, a change to the build environment, or a replacement. Each has an ID such as D81 and a row in DEVIATIONS.md. See [The rules](../method/rules.md) and [Kinds of deviation](../synthesis/kinds-of-deviation.md).

**device alias**: Open Firmware's short name for a device path, such as `hd` or `sata`. `devalias` lists them. See [the boot chain](../background/boot-chain.md).

**device path**: Open Firmware's name for a device, a partition and a file, such as `sata/k2-sata@0/@0:4,\\:tbxi`. See [the boot chain](../background/boot-chain.md).

**device tree**: Open Firmware's description of the machine's hardware, a tree of nodes with properties, which the kernel receives at boot. IOKit turns each node into an object that drivers can match. See [Hardware and boot basics](../background/hardware-basics.md).

**DHCP**: the protocol by which a machine gets its address, router and name servers from the network. On Darwin, bootp's IPConfiguration speaks it. See [Step 3, part 2](../steps/step-3-network-and-ssh.md).

**DirectoryService**: Leopard's daemon that serves accounts and groups from the local directory (`/var/db/dslocal`) and from network directories. Libinfo asks it whenever it runs. See [Step 3, part 2](../steps/step-3-network-and-ssh.md).

**DiskArbitration, diskarbitrationd**: the framework and daemon that watch for disks, mount their volumes and tell other programs, such as kextd. See [Step 3, part 1](../steps/step-3-security-and-daemons.md).

**`ditto`**: Mac OS X's copying tool, which keeps owners, modes and links. `make-test-image` copies with it. Apple's is closed, and darwinbuild puts a shell-script stand-in for it in the build environment ([D99](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d99)). See [Making the image](../method/making-the-image.md).

**DMA**: direct memory access, a device reading or writing the computer's memory by itself, without the processor. See [Hardware and boot basics](../background/hardware-basics.md).

**doorbell**: a signal by which a driver tells a device that work waits for it in memory. On the iMac G5, the SMU's doorbell is a GPIO line. See [Step 5](../steps/step-5-clock-and-power.md).

**driver cache**: one file, `Extensions.mkext`, holding the kexts needed to boot, made by kextcache and loaded by BootX with the kernel. See [the boot chain](../background/boot-chain.md).

**driver stack**: drivers layered on each other, each the provider of the one above, from a controller to a file system. See [Hardware and boot basics](../background/hardware-basics.md).

**`dropped`**: the kind of deviation no longer applied, whose row keeps its ID, which commits and the book still name. See [The rules](../method/rules.md) and [Step 6](../steps/step-6-securityd.md).

**DSP**: digital signal processing, code that changes sound samples on the processor before they reach the sound hardware. AppleDBDMAAudio's DSP sources are published empty. See [Step 9](../steps/step-9-disks-and-devices.md).

**`DSTROOT`**: the empty folder a project installs into during a build. darwinbuild keeps it as the project's Root. See [darwinbuild and the build machine](../method/darwinbuild.md).

**dsymutil, `.dSYM`**: Xcode's closed tool that collects a binary's debugging information into a separate `.dSYM` bundle; gcc runs it after a one-step `-g` build. It crashes in the build environment ([D7](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d7)), and on the image a script that exits 0 stands in for it ([D112](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d112)). See [Step 1, part 1](../steps/step-1-kernel-and-userland.md) and [Step 10](../steps/step-10-working-unix.md).

**DTrace, probe**: Leopard's tracing framework, and a marker compiled into a program that DTrace can switch on. `dtrace -h` turns a `.d` file into a header of probe macros. DTrace's library needs closed frameworks, so Libc's probes and `plockstat` are off, and the probes of DirectoryService, Perl, Python and Ruby compile to nothing ([D97](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d97), [D103](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d103)). See [Step 1, part 1](../steps/step-1-kernel-and-userland.md) and [Step 10](../steps/step-10-working-unix.md).

**dyld**: the dynamic linker, which loads a program's libraries when it starts. See [Step 1, part 1](../steps/step-1-kernel-and-userland.md).

**ECC**: error-correcting memory, checked by the U3 and U4 north bridges. See [Step 1, part 2](../steps/step-1-drivers-and-bootx.md).

**EHCI, OHCI**: the kinds of USB host controller, the chips that run a USB bus: EHCI for USB 2.0, OHCI for USB 1.1. See [Hardware and boot basics](../background/hardware-basics.md).

**ExpansionData, reserved slot**: Apple's ways of keeping an IOKit class's layout stable across versions: a pointer to a separate structure for members added later, and spare vtable slots for virtual functions added later. See [Step 9](../steps/step-9-disks-and-devices.md).

**expat, libffi**: an XML parser and a library for calling C functions described at run time, which Python's `pyexpat` and `ctypes` use. See [Step 10](../steps/step-10-working-unix.md).

**exports file**: a list of the only symbols a linked program or library keeps visible. Xcode's Jam-based targets apply it with `nmedit`. See [Step 6](../steps/step-6-securityd.md).

**fail-safe**: OpenSMU's rule that every fan runs at full speed when a sensor is missing, unreadable or 5 °C past its zone. See [Step 7](../steps/step-7-fans.md).

**family (IOKit)**: a kext holding the code shared by a whole class of hardware, such as IOPCIFamily, IOATAFamily or IOStorageFamily. See [Step 1, part 2](../steps/step-1-drivers-and-bootx.md).

**fan zone**: in OpenSMU, one sensor tied to one fan and two temperatures, between which the fan runs from its minimum to its maximum speed. In the version Step 7 tested, two zones drove two fans each. See [Step 7](../steps/step-7-fans.md) and [OpenSMU](../synthesis/opensmu.md).

**fat binary**: a Mach-O file holding the same program built for several processors, such as `ppc` and `i386`. See [darwinbuild and the build machine](../method/darwinbuild.md).

**file-system bundle**: a bundle in `/System/Library/Filesystems`, such as `hfs.fs`, `msdos.fs` or `cd9660.fs`, through which DiskArbitration recognises, names and mounts a volume. See [Step 9](../steps/step-9-disks-and-devices.md).

**FireWire**: IEEE 1394, a serial bus for disks and other devices, which the iMac G5 has beside USB. See [Step 9](../steps/step-9-disks-and-devices.md) and [Step 12](../steps/step-12-firewire.md).

**firmware**: the program stored on the machine itself, which runs at power-on, tests and describes the hardware and starts a boot loader. On PowerPC Macs it is Open Firmware. See [Hardware and boot basics](../background/hardware-basics.md).

**`fixincludes`**: the step of gcc's build that copies the system's headers into gcc's own folder, repairing what it needs to. See [Step 10](../steps/step-10-working-unix.md).

**Forth**: Open Firmware's stack-based language, in which its prompt, its scripts and the FCode drivers of expansion cards are written. See [the boot chain](../background/boot-chain.md).

**four-character code**: a 32-bit number written as four characters, such as the file type `tbxi`. Its bytes read differently in each byte order. See [Hardware and boot basics](../background/hardware-basics.md).

**framework**: a bundle in `/System/Library/Frameworks` holding a library with its headers and resources. See [What Darwin is](../background/what-darwin-is.md).

**gcc_select, gnumake**: the project that makes `cc`, `gcc`, `c++` and the other links to the default compiler, gcc 4.0, and the project of GNU Make 3.81, `make`. See [Step 10](../steps/step-10-working-unix.md).

**getty**: the program that opens a terminal, prints `login:` and runs `login`. launchd starts it from system_cmds' `com.apple.getty.plist`. See [Step 2](../steps/step-2-console-login.md).

**GPIO**: general-purpose input and output, a single wire that a driver can set or read. See [Hardware and boot basics](../background/hardware-basics.md).

**GUID partition map**: the partition table of current Macs. Open Firmware reads the Apple Partition Map instead. See [Step 14](../steps/step-14-usb-boot.md).

**`hdiutil`**: Mac OS X's tool that creates, attaches and converts disk images. See [Making the image](../method/making-the-image.md).

**headers Root**: a Root holding only the headers a project installs, made with `darwinbuild -headers` or downloaded as `<project>.hdrs.tar.gz`. See [darwinbuild and the build machine](../method/darwinbuild.md).

**HFS+**: the file system of Mac OS X. See [the boot chain](../background/boot-chain.md).

**HFSX**: the variant of HFS+ that tells upper-case from lower-case names. darwinbuild's BuildRoot is an HFSX volume. See [darwinbuild and the build machine](../method/darwinbuild.md).

**`hfs.util`**: the helper in `hfs.fs` that DiskArbitration runs to read an HFS+ volume's name and UUID. See [Step 8](../steps/step-8-clean-boot.md).

**HID**: human interface device, the USB class of keyboards and mice, handled by IOHIDFamily. See [Step 1, part 2](../steps/step-1-drivers-and-bootx.md).

**HyperTransport**: a fast point-to-point link between chips that software sees as PCI. On the iMac G5 it joins the U4 to the K2. See [Hardware and boot basics](../background/hardware-basics.md).

**I²C**: a slow two-wire bus that connects small chips, such as temperature sensors, to a controller. See [Hardware and boot basics](../background/hardware-basics.md) and [Step 7](../steps/step-7-fans.md).

**install name**: the path a library or framework records for itself, which programs linked against it then look for ([D25](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d25)). See [Kinds of deviation](../synthesis/kinds-of-deviation.md).

**I²S**: a serial bus that carries audio samples between the sound DMA and the codec. See [Step 9](../steps/step-9-disks-and-devices.md).

**imports, exports**: the symbols a Mach-O file needs from other files, and those it offers to them, as `nm -u` and `nm -g` list them. See [How testing is done](../method/testing.md).

**`installhdrs`**: the build step that installs only a project's headers. Some projects install some headers only in their full install ([D31](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d31), [D38](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d38)). See [Kinds of deviation](../synthesis/kinds-of-deviation.md).

**InterfaceNamer**: the configd plug-in that names network interfaces (`en0`), after waiting up to 60 seconds for IOKit to go quiet. See [Step 3, part 2](../steps/step-3-network-and-ssh.md) and [Step 8](../steps/step-8-clean-boot.md).

**interrupt**: a signal from a device that makes the processor stop and run the kernel's handler. See [Hardware and boot basics](../background/hardware-basics.md).

**interrupt controller**: the chip that collects the devices' interrupt lines and signals the processor. On PowerPC Macs it is the MPIC. See [Hardware and boot basics](../background/hardware-basics.md).

**IOATABlockStorage**: the driver that turns block reads and writes into ATA commands for a hard disk. Leopard's 200.1.8 is unpublished, so the image has Panther's 130.3.1 ([D40](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d40)). See [Step 1, part 2](../steps/step-1-drivers-and-bootx.md).

**IOAudioFamily, IOAudioEngine**: the IOKit family for sound, and its object for one engine moving samples through a DMA buffer, which programs reach through a user client. See [Step 9](../steps/step-9-disks-and-devices.md).

**`IOHDIXController`**: Leopard's closed driver of disk images. See [What stays closed](../synthesis/what-stays-closed.md).

**IOKit**: xnu's framework for drivers, written in a subset of C++. Drivers are objects that match the hardware they drive and are kept in the IORegistry. See [What Darwin is](../background/what-darwin-is.md).

**IOKit busy, quiet**: a nub is busy while IOKit matches drivers to it and their `start` runs, and so are the entries above it. IOKit is quiet when nothing is busy, which `IOKitWaitQuiet` waits for. See [Step 8](../steps/step-8-clean-boot.md).

**IOKit resource**: a name that a driver publishes with `publishResource` and that other drivers wait for with `resourceMatching`, such as `IORTC`. Resources hang on `IOResources`, the registry entry IOKit keeps for them. See [Step 5](../steps/step-5-clock-and-power.md) and [OpenSMU](../synthesis/opensmu.md).

**IOMMU**: a translator, like the MMU but for devices, between the addresses a device uses and physical memory. On the iMac G5 it is the DART. See [Hardware and boot basics](../background/hardware-basics.md).

**IONetworkingFamily**: the IOKit family that turns a network driver into an interface for the kernel's BSD layer. See [Step 3, part 2](../steps/step-3-network-and-ssh.md).

**`IOPMU`**: the IOKit resource published by the driver of a Mac's power controller. AppleMacRISC4PE's processor driver waits for it. See [Step 8](../steps/step-8-clean-boot.md).

**IORegistry**: IOKit's live tree of the machine's devices and their drivers, which `ioreg` prints. See [What Darwin is](../background/what-darwin-is.md).

**`IORTC`**: the IOKit resource published by a clock driver. ApplePlatformExpert waits up to 30 seconds for it before it reads the time. See [Step 5](../steps/step-5-clock-and-power.md).

**IOSCSIArchitectureModelFamily**: the IOKit family that turns a device answering SCSI commands into a disk or an optical drive. Leopard's 200.3.6 was never published, so the image has Panther's 139.0.2 ([D85](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d85)). See [Step 9](../steps/step-9-disks-and-devices.md).

**IOUSBMassStorageClass**: the driver of USB disks and keys, the transport for USB's mass-storage class. Its `OSBundleRequired` is `Local-Root`. See [Step 9](../steps/step-9-disks-and-devices.md) and [Step 14](../steps/step-14-usb-boot.md).

**IPConfiguration**: the configd plug-in that gives interfaces their addresses, by DHCP among others. `ipconfig waitall` waits until the interfaces it found are configured. See [Step 3, part 2](../steps/step-3-network-and-ssh.md) and [Step 8](../steps/step-8-clean-boot.md).

**IPMonitor**: the configd plug-in that sets the default route and the name servers from the interfaces' state. See [Step 3, part 2](../steps/step-3-network-and-ssh.md).

**K2**: the I/O controller of G5 Macs: SATA, ATA, Ethernet, FireWire, GPIO, I²C and audio. See [PowerPC Macs](../background/powerpc-macs.md).

**`Kernel.framework`**: the SDK's folder of headers for code that runs in the kernel, which kexts compile against. See [Step 1, part 2](../steps/step-1-drivers-and-bootx.md) and [Kinds of deviation](../synthesis/kinds-of-deviation.md).

**kernel mode, user mode**: the processor's two modes, which PowerPC calls supervisor and problem state. Programs run in user mode and enter the kernel by system calls. See [Hardware and boot basics](../background/hardware-basics.md).

**kext**: a kernel extension, a bundle holding a driver or other kernel code and an `Info.plist` that says what it matches and needs. See [the boot chain](../background/boot-chain.md).

**kextcache**: the tool that makes the driver cache. See [Making the image](../method/making-the-image.md).

**kextd**: the kext daemon, which keeps the driver caches current and holds off cache updates during a reboot. It links DiskArbitration. See [Step 2](../steps/step-2-console-login.md).

**`kextload -n`**: links a kext against the running kernel without loading it, which shows that every symbol it imports exists. See [How testing is done](../method/testing.md).

**KeyLargo, UniNorth**: the I/O controller and the north bridge of G4 Macs, and of QEMU's `mac99`. See [PowerPC Macs](../background/powerpc-macs.md).

**kit**: `PowerDarwin-kit.tar`, the repo with every source, Root and generated file the build machine needs, which `download-kit` makes on a current computer. See [darwinbuild and the build machine](../method/darwinbuild.md).

**`kmod_info`, `KMOD_EXPLICIT_DECL`**: the structure naming a kext's identifier, version and start and stop routines, which the kernel's loader reads, and the macro that makes it. See [OpenSMU](../synthesis/opensmu.md).

**kqueue**: the BSD kernel's event queue, through which a program waits for events such as a file becoming readable. See [Step 3, part 2](../steps/step-3-network-and-ssh.md).

**launchd**: process 1 of Mac OS X since 10.4, which starts and watches every daemon. See [Step 2](../steps/step-2-console-login.md).

**LaunchDaemon, job**: a property list in `/System/Library/LaunchDaemons` or `/Library/LaunchDaemons` describing a program launchd starts, with keys such as `KeepAlive`, `Sockets` and `SessionCreate`. See [Step 2](../steps/step-2-console-login.md).

**launchproxy**: launchd's helper for inetd-style jobs such as sshd. It accepts each connection, calls `SessionCreate` for it and runs the job's program on it. See [Step 3, part 2](../steps/step-3-network-and-ssh.md) and [Step 6](../steps/step-6-securityd.md).

**Leopard**: Mac OS X 10.5, here 10.5.8, the system whose open parts the project builds and whose binaries it compares with. See [What Darwin is](../background/what-darwin-is.md).

**Libc, Libsyscall, Libinfo**: libSystem's C library, its system-call stubs (from xnu's source), and its user, group and host lookups. See [Step 1, part 1](../steps/step-1-kernel-and-userland.md).

**`Libc_headers`**: the build-list project that installs Libc's headers. Built in Step 10, for the image's `/usr/include`; until then the build environment used a downloaded headers Root. See [Step 10](../steps/step-10-working-unix.md).

**libkern**: xnu's C++ runtime and kernel library, on which IOKit's classes are built. See [What Darwin is](../background/what-darwin-is.md).

**`libmd`**: a static library of message digests (MD5, SHA and others) that ntp links. See [Step 4](../steps/step-4-everyday-commands.md).

**`libSystem`**: Darwin's C library and system interface, made of Libc, Libm, Libinfo and others linked into one library. See [Step 1, part 1](../steps/step-1-kernel-and-userland.md).

**`lipo`**: the tool that lists and splits the slices of a fat binary. See [How testing is done](../method/testing.md).

**LM75, MAX6690**: small temperature-sensor chips on an I²C bus. The iMac G5 has two of each, for the disk and the optical drive, and for the GPU and the north bridge. See [Step 7](../steps/step-7-fans.md).

**local channel, remote channel**: the MAX6690's two temperatures, its own and that of a diode in another chip. `GPU Temp` and `NB Temp` are remote channels. See [OpenSMU](../synthesis/opensmu.md).

**`Local-Root`**: the `OSBundleRequired` value of a kext needed to boot from a local disk, such as IOUSBMassStorageClass. `kextcache -l` puts such kexts in the driver cache. See [Step 14](../steps/step-14-usb-boot.md).

**loginwindow**: Leopard's closed graphical login program. See [Step 9](../steps/step-9-disks-and-devices.md).

**MAC framework**: xnu's mandatory access control framework, from TrustedBSD, through which policy modules decide what a process may do. See [What Darwin is](../background/what-darwin-is.md).

**Mach-O**: Darwin's file format for programs, libraries and kexts. Its header says which processor the code is for. See [Hardware and boot basics](../background/hardware-basics.md).

**Mac OS Forge**: Apple's hosting site for open-source projects such as darwinbuild and MacPorts, with `src.macosforge.org`, which served darwinbuild's Roots. It shut down in November 2016. See [Darwin's history](../background/history.md).

**`mac99`**: QEMU's model of a G4 Power Mac (`PowerMac3,1`), started with a PMU by `boot-qemu`. See [PowerPC Macs](../background/powerpc-macs.md).

**Mach**: the part of xnu that manages tasks, threads, memory and messages between them. See [What Darwin is](../background/what-darwin-is.md).

**Mach service**: a named port registered with launchd, through which programs send messages to a daemon, such as kextd's. See [Step 2](../steps/step-2-console-login.md).

**MacIOGPIO**: the driver of the K2's block of GPIO lines, from the AppleGPIO project, through which AppleGPIO reads and writes a line. See [OpenSMU](../synthesis/opensmu.md).

**MacRISC2, MacRISC3, MacRISC4**: the platform names in the device tree's root `compatible` property: MacRISC2 and MacRISC3 for G3 and G4 Macs on UniNorth and KeyLargo, MacRISC4 for G5 Macs on U3 or U4 and K2. See [PowerPC Macs](../background/powerpc-macs.md).

**`master.passwd`**: the BSD file of accounts with their password hashes, which Libinfo reads when DirectoryService is not running. See [Step 2](../steps/step-2-console-login.md).

**matching**: IOKit's choice of a driver for a nub, by comparing the nub with the personalities in each kext's `Info.plist` and their probe scores. See [Hardware and boot basics](../background/hardware-basics.md).

**memory-mapped register**: a device register that answers at a physical address, so that the processor reads and writes it with ordinary loads and stores. See [Hardware and boot basics](../background/hardware-basics.md).

**MIG**: the Mach Interface Generator, which makes the code that packs and unpacks Mach messages from `.defs` files. See [What Darwin is](../background/what-darwin-is.md).

**MMU**: the processor's memory management unit, which translates virtual addresses into physical ones, page by page, from the kernel's page tables. See [Hardware and boot basics](../background/hardware-basics.md).

**model identifier**: Apple's name for a Mac model in its device tree, such as `PowerMac12,1`. `sysctl hw.model` prints it. See [PowerPC Macs](../background/powerpc-macs.md).

**MPIC**: the multiprocessor interrupt controller of PowerPC Macs, an OpenPIC, driven by AppleMPIC. See [Hardware and boot basics](../background/hardware-basics.md).

**msdosfs, `cd9660`, cddafs**: the FAT file system with its tools, the bundle for ISO 9660 data CDs, and the file system for audio CDs. See [Step 9](../steps/step-9-disks-and-devices.md).

**`nmedit`**: a cctools tool that edits a binary's symbol table, run after linking to apply an exports file. See [Step 6](../steps/step-6-securityd.md).

**north bridge**: the chip that joins the processor to memory and the fastest devices: the U4 on the iMac G5, UniNorth on G4 Macs and in QEMU. See [Hardware and boot basics](../background/hardware-basics.md).

**notifyd**: the server behind `notify(3)`, for cheap named notifications between processes. See [Step 3, part 1](../steps/step-3-security-and-daemons.md).

**`ntpd-wrapper`**: the script launchd runs for ntpd: it waits for the network, runs `ntpdate` once, then starts `ntpd`. See [Step 4](../steps/step-4-everyday-commands.md) and [Step 8](../steps/step-8-clean-boot.md).

**nub**: an IOKit object that stands for a device, such as a device-tree node or a disk on a SATA port, published by the driver behind it and matched by the drivers above. See [Hardware and boot basics](../background/hardware-basics.md).

**NVRAM**: memory that keeps its content with the power off. On PowerPC Macs it holds Open Firmware's variables, such as `boot-device` and `boot-args`, and the PRAM. See [the boot chain](../background/boot-chain.md).

**OHCI-1394**: the 1394 Open Host Controller Interface, the register layout and DMA model that most FireWire controllers follow, not to be confused with USB's OHCI. See [Step 12](../steps/step-12-firewire.md).

**OpenBIOS**: the open-source Open Firmware that QEMU runs. See [PowerPC Macs](../background/powerpc-macs.md).

**OpenDarwin**: the community Darwin project founded in April 2002 by the Internet Software Consortium and Apple. It released bootable Darwins 6 and 7 for PowerPC and Intel, and shut down in July 2006. See [Darwin's history](../background/history.md).

**`opendirectory`**: an `smb.conf` setting of Apple's Samba, on by default, that hands passwords to directory services; with it, `smbpasswd` stores no NT hash. The image sets it to `no`. See [Step 10](../steps/step-10-working-unix.md).

**Open Firmware**: the firmware of PowerPC Macs. It describes the hardware in the device tree, offers a Forth prompt, and boots from a device path. See [the boot chain](../background/boot-chain.md).

**`openplay`**: the project's command that plays a tone or a WAV file through IOAudioFamily's engine interface, without CoreAudio ([D95](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d95)). See [Step 9](../steps/step-9-disks-and-devices.md).

**OpenFWOHCI**: this project's link driver for the iMac G5's FireWire controller, in place of Apple's closed `AppleFWOHCI` ([D119](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d119)). See [Step 12](../steps/step-12-firewire.md).

**OpenPMU**: this project's open driver for the PMU, in place of Apple's closed ApplePMU: clock, restart and power-off. Its class derives from one named `ApplePMU`, for which AppleMacRISC2PE's processor driver waits. See [Step 11](../steps/step-11-pmu.md).

**OpenRadeonPM**: this project's kext that sets the idle power state of the iMac G5's ATI RV370, its clock gating and its link's L0s state, in place of that part of Apple's closed `ATIRadeon9700` ([D117](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d117)). See [Step 13](../steps/step-13-gpu-idle-heat.md).

**OpenSMU**: this project's open driver for the iMac G5's SMU, in place of Apple's closed AppleSMU: clock, restart, power-off and fans. See [OpenSMU](../synthesis/opensmu.md).

**order file**: a list of functions in the order the linker should place them. Kerberos links with one Apple did not publish ([D70](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d70)). See [Kinds of deviation](../synthesis/kinds-of-deviation.md).

**`OSBundleLibraries`**: the `Info.plist` key naming what a kext links against, such as `com.apple.kernel.6.0`. See [OpenSMU](../synthesis/opensmu.md).

**`OSBundleRequired`**: the `Info.plist` key saying when a kext is needed. `Root` means it is needed to reach the root disk, so it goes into the driver cache and loads at boot. See [the boot chain](../background/boot-chain.md).

**`OSvKernDSPLib`**: Apple's closed kernel library of vector arithmetic. IOAudioFamily's one call into it is replaced by a loop ([D87](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d87)). See [Step 9](../steps/step-9-disks-and-devices.md).

**`otool`**: Apple's tool that prints the load commands, sections and disassembly of a Mach-O file. See [How testing is done](../method/testing.md).

**page**: the unit of virtual memory, 4 KB on PowerPC. See [Hardware and boot basics](../background/hardware-basics.md).

**PAM**: pluggable authentication modules, the library through which `login` and `sshd` check passwords (`pam_unix`), accounts and sessions (`pam_launchd`), with one policy file per service in `/etc/pam.d`. See [Step 2](../steps/step-2-console-login.md).

**`pam_launchd`**: the PAM session module that makes a login's namespace a launchd session of the user, a `StandardIO` session for root's logins. See [Step 6](../steps/step-6-securityd.md).

**Panther**: Mac OS X 10.3 (Darwin 7), whose IOATABlockStorage and SCSI family, the last that Apple published, the image builds in place of Leopard's unpublished ones. See [Darwin's history](../background/history.md).

**PC/SC**: the standard interface to smart-card readers. On Darwin it is SmartCardServices' `PCSC.framework` and `pcscd`. See [Step 6](../steps/step-6-securityd.md).

**PCI, PCI Express**: the main bus of PowerPC Macs, and its later serial form, with the same configuration space. See [Hardware and boot basics](../background/hardware-basics.md).

**personality**: a dictionary in a kext's `Info.plist` that describes what the driver matches (`IOProviderClass`, `IONameMatch`, `IOPCIMatch`, `IOProbeScore`). See [Hardware and boot basics](../background/hardware-basics.md).

**phandle**: Open Firmware's number for a device-tree node, kept by IOKit as the node's `AAPL,phandle` property. See [Step 5](../steps/step-5-clock-and-power.md).

**physical address**: where a byte sits in the machine's memory, as devices see it. Programs and the kernel use virtual addresses, which the processor's MMU translates. See [Hardware and boot basics](../background/hardware-basics.md).

**physical request**: a FireWire read or write of the host's memory that the controller answers by itself, for the nodes that its physical request filter lets through. See [Step 12](../steps/step-12-firewire.md).

**PID loop**: a controller that sets its output from the error between a reading and a target, the error's sum over time and its rate of change. Apple's fan control runs one per sensor. See [OpenSMU](../synthesis/opensmu.md).

**`platform-do-` property**: a device-tree property that describes a platform function as a short program of commands, such as writing a register, a GPIO line or an I²C transfer, with flags for when it runs. See [OpenSMU](../synthesis/opensmu.md).

**platform driver**: the kext that matches the root of the device tree and knows one family of boards: AppleMacRISC2PE for G4 Macs, AppleMacRISC4PE for G5 Macs. See [PowerPC Macs](../background/powerpc-macs.md).

**platform expert**: the part of xnu, with the machine's platform driver, that knows how this model reads its clock, restarts and powers off. See [Step 5](../steps/step-5-clock-and-power.md).

**platform function**: a named operation that one driver performs for others, such as `platform-doorbell-req`, listed in the device tree and called with `callPlatformFunction`. See [Step 5](../steps/step-5-clock-and-power.md).

**platform plug-in**: a kext of `IOPlatformPluginFamily` that runs one G5 model's fans from a thermal profile, such as `PowerMac12_1_PlatformPlugin`. Leopard's are closed; Tiger's AppleMacRISC4PE published the iMac G5's. See [PowerPC Macs](../background/powerpc-macs.md) and [Step 7](../steps/step-7-fans.md).

**PLL**: phase-locked loop, the circuit that makes a clock of a given frequency from a reference clock. An ATI GPU's PLL registers, reached through `CLOCK_CNTL_INDEX` and `CLOCK_CNTL_DATA`, also hold its clock-gating bits. See [Step 13](../steps/step-13-gpu-idle-heat.md).

**PMU**: power management unit, the chip of many earlier PowerPC Macs, and of QEMU's `mac99`, that keeps the clock and switches the power. The kernel talks to it one byte at a time through a VIA. See [PowerPC Macs](../background/powerpc-macs.md) and [Step 11](../steps/step-11-pmu.md).

**polling**: reading a status again and again until it changes, instead of waiting for an interrupt. See [Hardware and boot basics](../background/hardware-basics.md).

**port**: a Mach message queue in the kernel, to which tasks hold send or receive rights. See [What Darwin is](../background/what-darwin-is.md).

**`PowerMac12,1`**: Apple's model identifier of the iMac G5 the project uses as its reference machine. See [PowerPC Macs](../background/powerpc-macs.md).

**PowerPC 970FX**: the G5 processor of the iMac G5, IBM's 64-bit PowerPC, here running a 32-bit kernel. See [PowerPC Macs](../background/powerpc-macs.md).

**PRAM, XPRAM**: parameter RAM, a small area of NVRAM inherited from the classic Mac OS. Its extended part, XPRAM, holds the time-zone offset that the kernel applies to the hardware clock. See [Step 5](../steps/step-5-clock-and-power.md).

**PreferencesMonitor**: the configd plug-in that writes the default network configuration when none exists. See [Step 3, part 2](../steps/step-3-network-and-ssh.md).

**project**: one source tree that Apple publishes with its own version, such as `xnu` 1228.15.4. A release of Mac OS X is a list of project versions. See [Leopard 9L30](../method/sources-and-roots.md).

**property list**: Apple's format for structured data: nested dictionaries, lists, strings and numbers, written as XML, in NeXT's older text form, or in binary. `Info.plist` files, launchd jobs and darwinbuild's build lists are property lists. See [Leopard 9L30](../method/sources-and-roots.md).

**PureDarwin**: a community project making usable systems from Darwin's sources: Nano (2008, Darwin 9.2 from Apple's prebuilt parts, Intel) and rafirafi's 1.3 beta (2012, Darwin 9.8, i386 only). See [Darwin's history](../background/history.md).

**QEMU**: the open-source emulator that runs the image as `mac99`. See [PowerPC Macs](../background/powerpc-macs.md).

**QEMU monitor**: QEMU's command line, which `boot-qemu` opens on a Unix socket, through which a script presses keys, takes screenshots or plugs in disks. See [How testing is done](../method/testing.md).

**quarantine, sandbox**: two closed libraries linked into Leopard's libSystem. The image has stubs that always fail ([D13](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d13), [D15](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d15)). See [Step 1, part 1](../steps/step-1-kernel-and-userland.md).

**`rbconfig.rb`**: Ruby's record of the flags and paths it was built with, which Apple's build edits with `rbconfig.diff` ([D104](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d104), [D105](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d105)). See [Step 10](../steps/step-10-working-unix.md).

**`RC_ARCHS`**: the build setting naming the processors a project is built for: `ppc`, or `ppc ppc64` where Leopard's binaries have 64-bit slices ([D115](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d115)). Apple's build lists say `ppc i386`, and four architectures for some projects. See [darwinbuild and the build machine](../method/darwinbuild.md).

**`rc.common`, `hostconfig`**: shell files from launchd's source that boot scripts such as `rc.audit` read for settings. See [Step 8](../steps/step-8-clean-boot.md).

**real-time clock (RTC)**: a clock that keeps counting while the machine is off, on standby power or a battery. On the iMac G5 it is part of the SMU. See [Step 5](../steps/step-5-clock-and-power.md).

**ReportCrash**: Leopard's crash reporter, closed and not on the image. launchd and configd log failed lookups of it. See [Step 6](../steps/step-6-securityd.md) and [Step 8](../steps/step-8-clean-boot.md).

**reset vector**: the fixed address at which a processor starts after power-on or reset, which the board maps to the boot ROM. See [Hardware and boot basics](../background/hardware-basics.md).

**Root**: in darwinbuild, a project's built files, as installed into the BuildRoot. Apple's prebuilt Roots bootstrap the build. See [Leopard 9L30](../method/sources-and-roots.md).

**`root~N`**: darwinbuild's numbering of one project's build attempts. The latest Root has the highest `N`. See [darwinbuild and the build machine](../method/darwinbuild.md).

**rubber band**: Apple's `rubber-band-connection`, a floor on one fan's target set by another's: the CPU fan's target is at least the ODD fan's less 500 rpm. See [OpenSMU](../synthesis/opensmu.md) and [Step 7](../steps/step-7-fans.md).

**safe boot** (`-x`): a boot with Shift held, which ignores the driver cache and loads kexts one at a time. See [the boot chain](../background/boot-chain.md).

**Samba, SMB1**: the open file server for Windows' file-sharing protocol, here 3.0.25b, which speaks that protocol's first version only. See [Step 10](../steps/step-10-working-unix.md).

**SATA, ATA**: the serial and parallel disk interfaces. The iMac G5's disk is on SATA, its optical drive on ATA. See [Hardware and boot basics](../background/hardware-basics.md).

**SBP-2**: the protocol by which FireWire disks carry SCSI commands. The disk reads its commands, called ORBs, from the host's memory, and moves its data there, through physical requests. See [Step 9](../steps/step-9-disks-and-devices.md) and [Step 12](../steps/step-12-firewire.md).

**SCSI**: a command set for storage devices, older than the buses that carry it. CD and DVD drives, USB disks and FireWire disks all speak it. See [Step 9](../steps/step-9-disks-and-devices.md).

**SDB partition**: a numbered block of the SMU's own data. Partition `0x18` holds the CPU diode's calibration. See [Step 7](../steps/step-7-fans.md).

**SDK**: Xcode's `MacOSX10.5.sdk`, the headers and link stubs of Leopard's frameworks, closed ones included, which the build may use and the image never holds. See [The rules](../method/rules.md).

**SecurityFoundation**: a closed framework of Security's, left unbuilt as it needs Apple's closed DiskImages. See [What stays closed](../synthesis/what-stays-closed.md).

**security session**: securityd's state (keychains, authorisations, tokens) for one bootstrap namespace: a root session for the system, a dynamic session for each login. See [Step 6](../steps/step-6-securityd.md).

**securityd**: the Security Server, the daemon behind Security.framework's shared state, started by launchd as `com.apple.SecurityServer`. See [Step 6](../steps/step-6-securityd.md).

**SecurityTokend**: the library of tokend, the daemons that present a smart card to securityd as a keychain. See [Step 6](../steps/step-6-securityd.md).

**self-ID packet**: the packet each FireWire PHY sends after a bus reset, with the node number it took, its ports and its speed. See [Step 12](../steps/step-12-firewire.md).

**service access list**: DirectoryService's list of users allowed to use a service such as SSH, which Apple's sshd checks through `mbr_user_name_to_uuid`. See [Step 3, part 2](../steps/step-3-network-and-ssh.md).

**`SessionCreate`**: Security's call that makes a bootstrap subset, moves the process into it and asks securityd for a security session. See [Step 6](../steps/step-6-securityd.md).

**shared cache (dyld)**: one file holding the common libraries, already bound together, mapped into every process. `update_dyld_shared_cache` builds it from the lists in `/var/db/dyld/shared_region_roots/`. See [Step 4](../steps/step-4-everyday-commands.md).

**single-user** (`-s`): a boot in which launchd starts a root shell on the read-only root volume, before the multi-user start-up. See [the boot chain](../background/boot-chain.md).

**slice**: one processor's build of a program inside a fat binary: `ppc`, `ppc7400` (G4 and later), `ppc64` (64-bit, G5), `i386`, `x86_64`. See [PowerPC Macs](../background/powerpc-macs.md).

**`smb.conf`, `smbpasswd`**: Samba's configuration file, which `make-test-image` writes ([D113](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d113)), and the command that sets an account's Samba password. See [Step 10](../steps/step-10-working-unix.md).

**SMU**: system management unit, a small processor on the logic board of G5 Macs such as the iMac G5, that keeps the clock, switches the power, reads sensors and drives fans. See [Step 5](../steps/step-5-clock-and-power.md).

**Snow Leopard**: Mac OS X 10.6 (Darwin 10), August 2009, the first Mac OS X for Intel Macs only. See [Darwin's history](../background/history.md).

**Sorbet Leopard**: a community update of Leopard 10.5.8 for PowerPC Macs, which the project's build machine runs; its kernel and drivers carry the versions of 10.5.8's build list. The comparisons with Leopard's binaries are provisional until they are repeated against an unmodified Leopard 10.5.8. See [How testing is done](../method/testing.md).

**source export**: the step that turns Apple's internal source into the published one. It removed blocks marked `#ifndef __OPEN_SOURCE__` but not code using them ([D35](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d35), [D39](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d39)). See [Step 1, part 2](../steps/step-1-drivers-and-bootx.md).

**S/PDIF**: a digital audio output. AppleOnboardAudio mutes the digital stream while no S/PDIF cable is in. See [Step 9](../steps/step-9-disks-and-devices.md).

**SPNEGO**: the exchange in which an SMB client and server agree how to authenticate. Samba 3.0.25b rejects the `mechListMIC` field that today's Mac client sends in it, so the image's `smb.conf` turns it off ([D113](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d113)). See [Step 10](../steps/step-10-working-unix.md).

**Startup Manager**: the boot menu that appears when Option is held at power-on, listing the volumes a Mac can start from. See [the boot chain](../background/boot-chain.md).

**static archive**: a `.a` file of object files that the linker copies into each program at link time, so that their code lives on in the program. See [Step 4](../steps/step-4-everyday-commands.md).

**`Still waiting for root device`**: the kernel's message when no driver has published the root partition after 60 seconds. See [the boot chain](../background/boot-chain.md).

**`sungem`**: QEMU's model of Apple's GMAC Ethernet chip, the iMac G5's own, driven by AppleGMACEthernet. See [PowerPC Macs](../background/powerpc-macs.md).

**`sw_vers`**: the command that prints the system's name, version and build, read from `SystemVersion.plist`; Perl's build calls it, and a stand-in prints 10.5.8 and `9L30` in the build environment ([D98](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d98)). Leopard's comes from a closed project; the image's is built from DarwinTools 1, Apple's open `sw_vers` of Darwin 8, and prints Darwin, 9.8 and `9L30` ([D121](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d121)). See [Step 10](../steps/step-10-working-unix.md).

**swift-corelibs-foundation**: Apple's open Foundation for Swift, with a CoreFoundation of its own, from which CFStringTransform comes ([D56](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d56)). See [Step 3, part 1](../steps/step-3-security-and-daemons.md).

**Symbolication**: a private, closed framework of Leopard's that `libdtrace.dylib` needs. See [Step 10](../steps/step-10-working-unix.md) and [What stays closed](../synthesis/what-stays-closed.md).

**`synchronize-preferences`**: Samba's tool, built from its `prefsync` source, that launchd runs whenever the SMB server's preferences file changes, and that turns file sharing on or off as Leopard's Sharing pane set it; the image removes the job that runs it ([D109](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d109), [D113](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d113)). See [Step 10](../steps/step-10-working-unix.md).

**syslogd**: the daemon that collects programs' and the kernel's messages and writes them to the files `/etc/syslog.conf` names. See [Step 4](../steps/step-4-everyday-commands.md).

**system call**: the instruction by which a program enters the kernel to ask for a service. See [Hardware and boot basics](../background/hardware-basics.md).

**SystemConfiguration.framework**: the framework through which configd's clients and plug-ins use the dynamic store and network preferences. See [Step 3, part 1](../steps/step-3-security-and-daemons.md).

**System session, `StandardIO` session**: launchd's session types for its root namespace and for a terminal or SSH login. See [Step 6](../steps/step-6-securityd.md).

**SystemStubs**: Apple's static archive of `$LDBLStub` functions and `___stub_getrealaddr`, which pick at run time the `long double` version of a `libSystem` function on PowerPC. gcc links every program with it, so the image holds it since Step 10 ([D110](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d110)). See [Step 4](../steps/step-4-everyday-commands.md).

**`SystemVersion.plist`**: the file naming the system's product, version and build. launchctl sets `kern.osversion` from it. See [Step 8](../steps/step-8-clean-boot.md).

**task, thread**: Mach's address space with its resources, and the thing that runs in it. See [What Darwin is](../background/what-darwin-is.md).

**`tbxi`**: the HFS file type of BootX, by which Open Firmware finds it in the blessed folder. See [the boot chain](../background/boot-chain.md).

**`tdbsam`**: Samba's local database of accounts and password hashes. See [Step 10](../steps/step-10-working-unix.md).

**thermal model**: the SMU's byte, in SDB partition `0x25`, that tells the 17-inch iMac G5 (2) from the 20-inch (3). OpenSMU drives the fans only on the 17-inch. See [OpenSMU](../synthesis/opensmu.md) and [Step 7](../steps/step-7-fans.md).

**thermal profile**: the data that gives a platform plug-in its sensors, fans and target temperatures. Leopard's `PowerMac12_1_ThermalProfile` is closed; Tiger's AppleMacRISC4PE 180.0.34 to 186.3.1 publish version 1.8.6f1. See [Step 7](../steps/step-7-fans.md).

**Tiger**: Mac OS X 10.4 (Darwin 8), whose AppleMacRISC4PE 186.3.1 publishes the iMac G5's platform plug-in and thermal profile. See [Darwin's history](../background/history.md).

**toolbox glue**: Python's Mac modules, such as `MacOS` and `_Res`, built on the closed Carbon; the image's Python is built without them ([D102](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d102)). See [Step 10](../steps/step-10-working-unix.md).

**toolchain**: the compiler, assembler, linker and their tools. Rule 4 names the release's own: gcc 5465, gcc_42 5531, cctools 667.3, ld64 77.1. See [Step 10](../steps/step-10-working-unix.md).

**transport driver**: a driver that carries SCSI commands over one bus: IOATAPIProtocolTransport over ATA, IOUSBMassStorageClass over USB, IOFireWireSerialBusProtocolTransport over FireWire. See [Step 9](../steps/step-9-disks-and-devices.md).

**tz database, `zic`, `/etc/localtime`**: the time-zone data (TimeZoneData), the compiler that turns it into `/usr/share/zoneinfo`, and the link that picks the local zone. See [Step 8](../steps/step-8-clean-boot.md).

**U3**: the north bridge of earlier G5 Macs, driven, like the U4, by AppleMacRISC4PE. See [Step 1, part 2](../steps/step-1-drivers-and-bootx.md).

**U4**: the iMac G5's north bridge, which joins the processor, the memory and the I/O controller. See [PowerPC Macs](../background/powerpc-macs.md).

**UDF**: the file system of many DVDs. The `9L30` build list has no UDF project. See [What's next](../steps/whats-next.md).

**umbrella header**: a framework's main header, which includes all its others, such as `CoreServices/CoreServices.h` ([D67](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d67)). See [Kinds of deviation](../synthesis/kinds-of-deviation.md).

**unit address**: the part of a device-tree node's name after `@`, its place on its parent bus, such as `f8000000` in `u4@0,f8000000`. See [Hardware and boot basics](../background/hardware-basics.md).

**verbose boot** (`-v`): a boot that shows the kernel's and launchd's messages on the screen. See [the boot chain](../background/boot-chain.md).

**virtual control**: Apple's `PowerMac12_1VirtualControl`, which gives a fan the highest of several loops' requests and applies the rubber band and the power floor. See [OpenSMU](../synthesis/opensmu.md).

**vtable**: the table of function pointers through which C++ calls a class's virtual functions. A subclass built against one version of a family calls its functions at fixed slots, so the slots must match in the version it runs with. See [Step 9](../steps/step-9-disks-and-devices.md).

**Wayback Machine**: the Internet Archive's copy of old web pages, the only remaining source of darwinbuild's Roots and of some of Apple's old tarballs. See [Leopard 9L30](../method/sources-and-roots.md).

**work loop**: the thread on which an IOKit driver handles its interrupts and requests. See [Hardware and boot basics](../background/hardware-basics.md).

**XCOFF**: the program format that Open Firmware loads and runs, which BootX is built in. See [the boot chain](../background/boot-chain.md).

**`/XCD/SY`, `/XCD/loper`**: where darwinbuild puts its copy of Xcode in the BuildRoot, `/System` and `/Developer` renamed to paths of the same length. See [darwinbuild and the build machine](../method/darwinbuild.md).

**Xcode 3.1.4**: Apple's developer tools for Leopard, closed, needed on the build machine for `xcodebuild`. See [darwinbuild and the build machine](../method/darwinbuild.md).

**xnu**: Darwin's kernel: Mach, a BSD layer and IOKit. See [What Darwin is](../background/what-darwin-is.md).
