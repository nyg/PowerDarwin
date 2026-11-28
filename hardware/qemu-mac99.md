# QEMU mac99

QEMU's `mac99` machine, run by [scripts/boot-qemu](../scripts/boot-qemu), models a PowerPC G4 Mac (`PowerMac3,1`) with a KeyLargo I/O controller, two ATA buses, a `sungem` Ethernet card (Apple's GMAC) and USB.

Tested: the image boots to a single-user shell and to a console `login:`, brings up `en0` by DHCP and accepts SSH logins, runs the everyday commands, starts securityd, mounts CDs and HFS+ and FAT32 USB disks plugged in while it runs, and runs the commands built after Step 10, from `awk`, `tar` and `curl` to `diff`, `patch`, `man`, `rsync`, `zip`, `screen` and `nano`. With OpenPMU (Step 11), it reads the clock from the PMU at boot, `reboot` restarts the machine, `shutdown -h now` ends QEMU, and `en0` is named without the 60-second wait.

What differs from a real Mac:

- The platform driver is `AppleMacRISC2PE`, not the iMac G5's `AppleMacRISC4PE`.
- The power manager is a PMU, reached through a VIA (`via-pmu`, which our `AppleVIA` drives), not an SMU. Its driver `ApplePMU` is closed; our `OpenPMU` ([D118](../DEVIATIONS.md#d118)) stands in for its clock, restart and power-off, and its class derives from one named `ApplePMU`, for which AppleMacRISC2PE's CPU driver waits. Not replaced: sleep, processor speed changes and the PMU's interrupts (ADB, buttons, battery, lid), none of which QEMU's model needs.
- USB disks work on QEMU's EHCI controller; its OHCI controller stops about 90 seconds after boot.
