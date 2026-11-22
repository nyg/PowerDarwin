# Hardware

One page per machine PowerDarwin has been tried on: what works there, and the drivers it needs, with the project that builds each one and whether its source is published.

| Machine | Model | Status | Page |
|---|---|---|---|
| iMac G5 (iSight) | `PowerMac12,1` | Tested: Steps 1 to 10 | [powermac12-1.md](powermac12-1.md) |
| QEMU `mac99` | emulated `PowerMac3,1` | Tested: Steps 1 to 4 and 6, CDs and USB disks | [qemu-mac99.md](qemu-mac99.md) |

Other PowerPC Macs that Leopard supports are untested. Most of what they need is built, since Darwin 9 shares its drivers across models; what usually differs is the platform plug-in, the power or system management controller, the sound codec and the graphics.

## Adding your Mac

A page for another PowerPC Mac tells the project what that model needs, and is the first step to supporting it. You need the Mac running Leopard 10.5.8.

1. Find its model: `sysctl hw.model` prints it, such as `PowerMac7,3`.
2. List the drivers Leopard loads on it: `kextstat -k -l > kextstat.txt`.
3. For each driver, find the 9L30 project that builds it in [manifest/9L30.tsv](../manifest/9L30.tsv), usually the driver's own name or its family's. The driver's `CFBundleVersion`, in its `Info.plist` under `/System/Library/Extensions`, gives the project version, as [powermac12-1.md](powermac12-1.md) explains. A driver that no project builds is closed: check [DEVIATIONS.md](../DEVIATIONS.md) for a stand-in.
4. Write `hardware/<model>.md` in lower case, with the comma as a hyphen (`powermac7-3.md`), laid out as [powermac12-1.md](powermac12-1.md): the drivers needed for a single-user shell first, then the others, and the closed ones.
5. If you try PowerDarwin's image on it ([Install it](../README.md#install-it)), say which steps work and where it stops, with the verbose boot's last lines.
6. Open a pull request with the page, a row in the table above, and `reference/<model>/kextstat.txt`, or open an issue with them if that is easier.

Keep out of what you send: serial numbers, UUIDs, MAC addresses (mask them after the vendor prefix, as `00:14:51:xx:xx:xx`), and the output of `ioreg -l`, `nvram -p` and `system_profiler`, which hold them.
