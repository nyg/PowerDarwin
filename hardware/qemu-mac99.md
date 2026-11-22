# QEMU mac99

QEMU's `mac99` machine, run by [scripts/boot-qemu](../scripts/boot-qemu), models a PowerPC G4 Mac (`PowerMac3,1`) with a KeyLargo I/O controller, two ATA buses, a `sungem` Ethernet card (Apple's GMAC) and USB.

Tested: the image boots to a single-user shell and to a console `login:`, brings up `en0` by DHCP and accepts SSH logins, runs the everyday commands, starts securityd, and mounts CDs and HFS+ and FAT32 USB disks plugged in while it runs.

What differs from a real Mac:

- The platform driver is `AppleMacRISC2PE`, not the iMac G5's `AppleMacRISC4PE`.
- The power manager is a PMU, whose driver `ApplePMU` is closed and has no stand-in: `reboot` stops at `MACH Reboot`, the clock starts at 1 January 1970, and configd waits 60 seconds for IOKit to go quiet before naming `en0`, since AppleMacRISC2PE's CPU driver waits for `ApplePMU`.
- USB disks work on QEMU's EHCI controller; its OHCI controller stops about 90 seconds after boot.
