# Hardware

One page per machine PowerDarwin has been tried on: what works there, and the drivers it needs, with the project that builds each one and whether its source is published.

| Machine | Model | Status | Page |
|---|---|---|---|
| iMac G5 (iSight) | `PowerMac12,1` | Tested: Steps 1 to 10 | [powermac12-1.md](powermac12-1.md) |
| QEMU `mac99` | emulated `PowerMac3,1` | Tested: Steps 1 to 4 and 6, CDs and USB disks | [qemu-mac99.md](qemu-mac99.md) |

Other PowerPC Macs that Leopard supports are untested. Most of what they need is built, since Darwin 9 shares its drivers across models; what usually differs is the platform plug-in, the power or system management controller, the sound codec and the graphics.

A page for a new machine lists the drivers Leopard loads on it (`kextstat` under Leopard), the 9L30 project that builds each one, and whether that project's source is published, with any stand-in listed in [DEVIATIONS.md](../DEVIATIONS.md).
