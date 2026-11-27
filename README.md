# PowerDarwin

PowerDarwin is a bootable Darwin 9.8 for PowerPC Macs: the open core of Mac OS X 10.5.8 Leopard (build `9L30`), built from the sources Apple published. Every file on its disk image is built from source by the project or written by its scripts; nothing is copied from Apple's own installation of Leopard. The book, at [nyg.github.io/PowerDarwin](https://nyg.github.io/PowerDarwin/), explains how, from the hardware up.

## What makes it different

Apple's last bootable Darwin was 8.0.1, in 2005; after that, Apple published sources only. The earlier Darwin 9 builds we know of either target Intel machines or stop before a bootable system. PowerDarwin:

- builds every file on the image from Apple's published sources, at the exact versions of Mac OS X 10.5.8;
- changes those sources as little as possible, and lists every change with its reason in [DEVIATIONS.md](DEVIATIONS.md);
- writes open drivers where Apple's are closed and the system cannot do without them, such as [OpenSMU](docs/src/synthesis/opensmu.md) for the iMac G5's system management unit;
- keeps the scripts that repeat the build.

The rules below give the details.

## Rules

1. **Apple's release, as published.** Projects and versions are those of darwinbuild's `9L30` build list ([manifest/9L30.tsv](manifest/9L30.tsv)). Sources are fetched as Apple published them, checked by their SHA-256 ([manifest/9L30.sha256](manifest/9L30.sha256)), and never edited in place. A project changes version only to take Apple's own fix from a later release, or, when Apple never published 9L30's version, to build the nearest one it did publish.
2. **The smallest fix, in this order.** Change the build environment; failing that, take Apple's fix from another release; only then write a new patch, as small as possible, with no reformatting or unrelated change. Where the missing part is closed, leave out the feature that needs it, or write an open replacement in `replacements/`.
3. **Every change documented.** Every patch, build-environment workaround and replacement is listed in [DEVIATIONS.md](DEVIATIONS.md). The files that `scripts/make-test-image` writes or edits on the image are listed in the book's [Making the image](docs/src/method/making-the-image.md).
4. **The release's own toolchain.** Builds use `gcc` 5465, `gcc_42` 5531, `cctools` 667.3 and `ld64` 77.1.
5. **Open source only in the image.** The image holds only what is built from open source or written by this repo's scripts: no Apple binary, and no prebuilt binary that a source carries. Closed tools may run the build.

## Features

- Boots to a `login:` prompt on the console, or to a single-user shell with `-v -s`, and takes SSH logins over Ethernet; `passwd` changes passwords.
- Everyday Unix commands, among them `grep`, `ps`, `less`, `vi`, `awk`, `tar`, `gzip`, `file`, `lsof`, `top` and `curl`, with syslogd writing `/var/log/system.log` and ntpd keeping the clock.
- Its own compilers, built from source, which build and link C and C++ programs, with Perl, Python, Ruby and Samba.
- Mounts internal partitions, USB disks and CDs as they appear, and `bless` sets the startup disk.
- Reads the clock from the hardware at boot, restarts and powers off, through open drivers for the iMac G5's SMU and QEMU's PMU.
- On the iMac G5: plays sound through the built-in audio, drives the fans from the temperature sensors as quietly as Leopard, and keeps the GPU and the north bridge cooler at idle than Leopard does.
- Boots cleanly: the network comes up without a 60-second wait, the clock is set at boot, and `/var/log/system.log` shows no errors.

Not yet: Wi-Fi, graphics acceleration, sleep, FireWire devices (an open driver brings the bus up), booting from a USB key, and commands such as `cmp`, `diff` and `man`.

## Credits

- Apple, for publishing Darwin's sources ([apple-oss-distributions](https://github.com/apple-oss-distributions)) and darwinbuild ([macosforge/darwinbuild](https://github.com/macosforge/darwinbuild)), whose `Darwin9` branch builds every project here, unchanged.
- The [Internet Archive](https://web.archive.org/), whose Wayback Machine still serves darwinbuild's Roots and the opensource.apple.com tarballs that GitHub lacks, and ArchiveBot, whose 2014 crawl of src.macosforge.org holds those Roots.
- Andrew-R, whose [darwinbuild work on Darwin 9 for PowerPC](https://forums.macrumors.com/threads/building-with-darwinbuild-for-darwin9-ppc-in-2026-wip.2485638/) built 100 of its projects on a Leopard PowerPC Mac in 2026.
- [barracuda156](https://github.com/barracuda156) and the [macos-powerpc](https://github.com/macos-powerpc) organisation, who keep darwinbuild and Apple's projects building for PowerPC.
- NetBSD's macppc drivers, [`smu.c`](https://github.com/NetBSD/src/blob/trunk/sys/arch/macppc/dev/smu.c), [`fancontrol.c`](https://github.com/NetBSD/src/blob/trunk/sys/arch/macppc/dev/fancontrol.c) and [`pmu.c`](https://github.com/NetBSD/src/blob/trunk/sys/arch/macppc/dev/pmu.c), whose SMU protocol and fan zones OpenSMU follows, and whose PMU protocol OpenPMU follows, FreeBSD's [`onyx.c`](https://github.com/freebsd/freebsd-src/blob/main/sys/dev/sound/macio/onyx.c), whose codec settings OpenOnyx uses, and Linux's [`smu.c`](https://github.com/torvalds/linux/blob/master/drivers/macintosh/smu.c) and [`windfarm_pm121.c`](https://github.com/torvalds/linux/blob/master/drivers/macintosh/windfarm_pm121.c), read for facts about the SMU and the fans.
- X.Org's [xf86-video-ati](https://gitlab.freedesktop.org/xorg/driver/xf86-video-ati), whose clock-gating sequence for ATI's RV350 family OpenRadeonPM follows, and Linux's [`radeonfb`](https://github.com/torvalds/linux/tree/master/drivers/video/fbdev/aty), read for facts about the same chips.
- The 1394 Open Host Controller Interface specification and FreeBSD's [`fwohci.c`](https://github.com/freebsd/freebsd-src/blob/main/sys/dev/firewire/fwohci.c), from which OpenFWOHCI takes its registers and DMA model, and Linux's [`firewire-ohci`](https://github.com/torvalds/linux/blob/master/drivers/firewire/ohci.c), read for facts about the same controllers.
- [PureDarwin](https://github.com/PureDarwin/PureDarwin), which built bootable Darwin 9 systems for Intel machines.
- [QEMU](https://www.qemu.org/) and [OpenBIOS](https://github.com/openbios/openbios), whose `mac99` machine tests each image before it reaches a real Mac.

## Supported hardware

- iMac G5 (iSight), `PowerMac12,1`.
- QEMU `mac99`.

[hardware/](hardware/README.md) has a page per machine, with the drivers it needs and which of them are closed, and says how to [add another Mac](hardware/README.md#adding-your-mac).

## Build it

PowerDarwin is built on a PowerPC Mac running Leopard 10.5.8 with Xcode 3.1.4. Leopard cannot reach GitHub over TLS, so a current computer first downloads everything into one file, the kit. The build takes about 10 hours on an iMac G5 and needs 40 GB free.

1. On a current computer, macOS or Linux, with `git`, `curl` and Java 6 or later, clone this repo and run `scripts/download-kit`. It checks every file against its SHA-256 and packs them into `PowerDarwin-kit.tar`, about 2.2 GB. If a download fails, run it again: it keeps what it has.
2. Install Xcode 3.1.4, `xcode314_2809_developerdvd.dmg` from [Apple's developer downloads](https://developer.apple.com/download/all/), on the PowerPC Mac.
3. Copy the kit to the PowerPC Mac, on a USB key or over HTTP (`python3 -m http.server` on the current computer, `curl -O` on the Mac).
4. On the PowerPC Mac, set Computer sleep to Never in Energy Saver, then run `tar -xf PowerDarwin-kit.tar`, `cd PowerDarwin` and `sudo scripts/build-image`. It builds every project in the order of [manifest/build-order](manifest/build-order), makes the image, `/private/var/tmp/PowerDarwin.dmg`, and checks every file on it. If a build fails, it stops; run it again to go on from there.

The book's Method chapters explain each script, and how to build one project at a time.

## Install it

Each [release](https://github.com/nyg/PowerDarwin/releases) has the image, `PowerDarwin-<name>.img.gz`, a raw disk, gzipped; `PowerDarwin-<name>-source.tar`, the source of every binary on it; and `SHA256SUMS`. Check and unpack them with `shasum -a 256 -c SHA256SUMS` and `gunzip PowerDarwin-<name>.img.gz`.

### In QEMU

With QEMU's `qemu-system-ppc` and a clone of this repo, run `scripts/boot-qemu PowerDarwin-<name>.img -v`. At the first boot, the console asks for root's password, of which only the first 8 characters count, then turns SSH on; `passwd` changes it later, with every character counting. The guest's SSH port is port 2222 of `127.0.0.1`:

```sh
ssh -p 2222 -o HostKeyAlgorithms=+ssh-rsa -o KexAlgorithms=+diffie-hellman-group14-sha1 root@127.0.0.1
```

### On a PowerPC Mac

From Leopard, with a clone of this repo, run `sudo scripts/install-image --image=PowerDarwin-<name>.img`. It puts PowerDarwin on a new partition taken from a Journaled HFS+ volume, Leopard's own included, on an existing partition or on a whole disk; asks for the time zone, the NTP server and root's password; and shows what it will do before it changes anything. Hold Option at power-on to start PowerDarwin; `bless --mount /Volumes/<Leopard volume> --setOF` and `reboot` go back to Leopard. SSH takes root's password: keep the Mac on a network you trust.

## Layout

- `manifest/`: project versions, their source, the hashes of the fetched tarballs and Roots, darwinbuild's build lists in `plists/`, and `build-order`, the builds and build-environment steps in the order `build-image` runs them.
- `patches/<project>-<version>/`: one patch file per fix.
- `env/`: build-environment workarounds, each with the error it fixes.
- `replacements/`: new code that stands in for a closed Apple library or driver.
- `LICENSES/`: the texts of the licences that the patches and the copied files are under.
- `overlay/`: files that `make-test-image` puts onto the image at the same path, such as `usr/libexec/first-boot`, which sets root's password and makes the SSH host keys at the first boot.
- `scripts/`: the three that build and install PowerDarwin (`download-kit`, `build-image`, `install-image`); fetching, staging and building (`fetch-sources`, `fetch-roots`, `wayback-fetch`, `resolve-buildlist`, `stage-sources`, `stage-roots`, `prepare-buildroot`, `run-darwinbuild`), making, checking and publishing the image (`make-test-image`, `collect-notices`, `check-image`, `cmp-binaries`, `cmp-disassembly`, `check-family-imports`, `make-release`, `publish-release`), running it (`boot-qemu`, `qemu-type`, `ssh-root`), and the book's indexes (`make-book-indexes`). The book's Method chapters explain each one.
- `hardware/`: one page per machine, with the drivers it needs.
- `reference/`: listings and logs from Leopard, against which the image is compared, and a description of Andrew-R's darwinbuild patch.
- `docs/`: the book, an mdBook.
- `worker/`: a Cloudflare Worker that passes the book's page views on to GoatCounter.
- [DEVIATIONS.md](DEVIATIONS.md): every change from Apple's sources and build.
- [RESOURCES.md](RESOURCES.md): other people's work, tools, and where Apple's sources and binaries live.
- [AGENTS.md](AGENTS.md): a guide for coding agents working in this repo.

## Licence

What this project writes is under the MIT licence, in [LICENSE](LICENSE), unless a file or its folder says otherwise. Each patch is under the licence of the files it changes, the APSL 2.0 for most of Apple's projects. The image holds the licences and notices of every project built in `/usr/share/doc/PowerDarwin`, and each release holds the source of every binary on it. The book's [What the licences ask](docs/src/method/making-the-image.md#what-the-licences-ask) gives the details.

PowerDarwin is not affiliated with, endorsed or sponsored by Apple Inc. Apple, Mac, Mac OS, iMac and Power Mac are trademarks of Apple Inc., and PowerPC is a trademark of IBM.
