# Resources

Earlier work, tools and sources for building Darwin on PowerPC, as found on 2026-11-18.

## People working on it now

- [Building with darwinbuild for Darwin9/ppc in 2026 [WIP]](https://forums.macrumors.com/threads/building-with-darwinbuild-for-darwin9-ppc-in-2026-wip.2485638/) (MacRumors, Andrew-R, 20–22 July 2026): darwinbuild's `Darwin9` branch on a Leopard PowerPC Mac, about 90 Darwin 9 projects built including `xnu-1228.15.4`, never assembled into a bootable system. His patch is in [reference/andrew-r/](reference/andrew-r/README.md).
- [Reviving Darwin Build?](https://forums.macrumors.com/threads/reviving-darwin-build.2490592/) (MacRumors, from 27 September 2026): alex_free wants a full mirror of Apple's open-source releases; Andrew-R gave up on 10.6.8 on PowerPC; barracuda156 finds darwinbuild "too messy and painful" for single components.
- [barracuda156](https://github.com/barracuda156) (Sergey Fedorov) and the [macos-powerpc](https://github.com/macos-powerpc) organisation: [macos-powerpc/darwinbuild](https://github.com/macos-powerpc/darwinbuild), whose `Darwin10` branch adds ppc to `10A432` and replaces dead URLs (December 2024); build lists at `macos-powerpc.org/darwinbuild/plists/` (its `Roots/` folder was empty on 2026-11-18); forks of PureDarwin's Libsystem and XPC; [apple-libdispatch-powerpc](https://github.com/barracuda156/apple-libdispatch-powerpc); [powerpc-ports](https://github.com/macos-powerpc/powerpc-ports).
- [alex-free](https://github.com/alex-free): tigerports, SDL2 for Panther and Leopard.
- [10.6 Snow Leopard PowerPC – Development](https://forums.macrumors.com/threads/10-6-snow-leopard-powerpc-development.2439769/) (MacRumors): the group that built a Snow Leopard kernel for PowerPC from `xnu-1504.9.37`; summary in [LowEndMac](https://lowendmac.com/2024/mac-os-x-snow-leopard-10-6-10-6-8-for-powerpc/).
- [Sorbet Leopard](https://forums.macrumors.com/threads/sorbet-leopard-10-5-reinvigorated-updated.2300924/): a modified Leopard reporting `10.5.9` build `9L31b`; it is what the iMac G5 runs.

## Earlier attempts

- Apple's last bootable Darwin, 8.0.1 for PowerPC, still served by Apple at `https://opensource.apple.com/static/iso/darwinppc-801.cdr.gz` (428 MB), with its [release notes](https://opensource.apple.com/static/iso/release-notes-8.0.1.txt).
- [HOWTO: Building Darwin 10.4.7 for PPC using builddarwin](https://lists.apple.com/archives/darwin-dev/2006/Nov/msg00000.html) (Craig Harman, 2006) and Graham Lee's 2006 boot of a G4 from an open-source build, both on Apple's darwin-dev list.
- [PureDarwin](https://github.com/PureDarwin/PureDarwin): Darwin 9 "Nano" and rafirafi's 2012 Darwin 9.8 build, Intel only; [PureDarwin/darwinbuild](https://github.com/PureDarwin/darwinbuild).
- [steventroughtonsmith/BootX](https://github.com/steventroughtonsmith/BootX): BootX-34 patched to boot Mac OS X Server 1.0 in `qemu-system-ppc`, with the QEMU options it needs.
- [neozeed/Darwin_0.1](https://github.com/neozeed/Darwin_0.1) and [Rebuilding Darwin from source](https://virtuallyfun.com/2024/04/22/rebuilding-darwin-from-source-part-1-qemu/) (virtuallyfun, 2024): Darwin 0.1 rebuilt on x86 in QEMU.

## Tools

- [macosforge/darwinbuild](https://github.com/macosforge/darwinbuild), archived; this project uses its `Darwin9` branch at `4848b7f` unchanged.
- [wam-slop/llvm-powerpc-apple-darwin](https://github.com/wam-slop/llvm-powerpc-apple-darwin) (from 12 November 2026, AI-assisted): LLVM with `powerpc-apple-darwin8/9` restored, ARC and blocks on Tiger and Leopard. Not Apple's toolchain, so not for this project's builds; useful for later Cocoa work.
- [darwin-xtools](https://github.com/barracuda156/darwin-xtools): updated cctools and ld64 that still target ppc.
- [VariantXYZ/gcc-powerpc-apple-darwin8](https://github.com/VariantXYZ/gcc-powerpc-apple-darwin8): Docker cross toolchain for Tiger on PowerPC.
- [Scottcjn/darwin-ppc-gpu](https://github.com/Scottcjn/darwin-ppc-gpu) (2026): IOKit drivers for modern AMD GPUs on PowerPC Tiger and Leopard, mostly design documents so far.
- NetBSD's macppc [`smu.c`](https://github.com/NetBSD/src/blob/trunk/sys/arch/macppc/dev/smu.c) and [`pmu.c`](https://github.com/NetBSD/src/blob/trunk/sys/arch/macppc/dev/pmu.c) (BSD licence): the clock, power-off and restart commands of the SMU and the PMU, for a stand-in for Apple's closed AppleSMU and ApplePMU, and in `smu.c` the SMU's fan, sensor and SDB partition commands. OpenSMU ([D81](DEVIATIONS.md)) speaks the same SMU protocol, and its fan zones work as those of `smu.c` and [`fancontrol.c`](https://github.com/NetBSD/src/blob/trunk/sys/arch/macppc/dev/fancontrol.c) (Michael Lorenz, BSD licence) do, without their code. Linux's `drivers/macintosh/smu.c` and `via-pmu.c` describe the same, under GPL-2.0-only.
- FreeBSD's [`onyx.c`](https://github.com/freebsd/freebsd-src/blob/main/sys/dev/sound/macio/onyx.c) (BSD licence): the PCM3052 (Onyx) codec's registers and the values it sets them to, for OpenOnyx ([D94](DEVIATIONS.md)), our stand-in for Apple's AppleOnyxAudio, published only as a stub.
- Howard Hinnant's [date algorithms](https://howardhinnant.github.io/date_algorithms.html) (public domain): the day counts with which OpenSMU converts the SMU's dates.

## Apple's sources and binaries

- [apple-oss-distributions](https://github.com/apple-oss-distributions) (since 4 October 2021): one repo per project, one tag per version; 372 of the 386 versioned 9L30 projects have their exact tag. Index: [opensource.apple.com/releases](https://opensource.apple.com/releases/) ("Mac OS X 10.5.8", 369 projects).
- The old `opensource.apple.com/release/mac-os-x-1058/` pages and tarballs, now only on the Wayback Machine.
- darwinbuild's Roots, `src.macosforge.org/Roots/<build>/`, only on the Wayback Machine, which refuses connections after a burst of requests; `scripts/fetch-roots` paces itself. [darwinbuild-dev, July 2010](https://lists.macosforge.org/pipermail/darwinbuild-dev/2010-July/000641.html) confirms where they were served from.
- Later Apple releases that fixes are backported from: CF-550 and DirectoryService-621 (Mac OS X 10.6), CF-635 (Mac OS X 10.7), Security-55471 (OS X 10.9), whose `libsecurity_codesigning/antlr2` holds ANTLR 2.7.7, and [swiftlang/swift-corelibs-foundation](https://github.com/swiftlang/swift-corelibs-foundation), Apple's later open CoreFoundation, under Apache 2.0 with Runtime Library Exception.
- Xcode 3.1.4 (`xcode314_2809_developerdvd.dmg`), from developer.apple.com/download/all with a free Apple account.
