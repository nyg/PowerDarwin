# PowerDarwin

Darwin 9.8 for PowerPC Macs: the open core of Mac OS X 10.5.8 Leopard (build `9L30`), built from the sources Apple published, with the fewest changes that make it boot and run, each one documented.

The book in [docs/](docs/src/README.md), published at [nyg.github.io/PowerDarwin](https://nyg.github.io/PowerDarwin/), explains the project from the hardware up: what Darwin is, how it is built, and how each step was reached.

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

[RESOURCES.md](RESOURCES.md) lists everyone else's work this project draws on.

## Status

Steps 1 to 10 and 13 are reached: on an iMac G5, the image boots to a multi-user system with Ethernet, SSH, fan control, disks, USB, sound, its own compiler, Perl, Python, Ruby and Samba, keeps its GPU and north bridge cooler at idle than Leopard does, and restarts and powers off. Step 11 is reached in QEMU `mac99`, where an open driver for its PMU keeps the clock, restarts and powers off, and the network comes up without a 60-second wait. Not working yet: Wi-Fi, graphics acceleration and sleep. FireWire has an open driver in the image, which brings the bus up, but no FireWire device has been tested on it yet. `awk`, `tar`, `gzip`, `file`, `lsof`, `top` and `curl`, built after Step 10, run on the iMac and in QEMU, and `curl`, like Leopard's, reaches over HTTPS only the servers that still accept TLS 1.0. Other everyday commands, such as `cmp`, `diff` and `man`, are not built yet.

Every binary of the build is compared with Leopard 10.5.8's at the same path. Of the 833 that Leopard also has, 713 import and export the same symbols as Leopard's, 91 export the same symbols and import others, and 29 export other symbols, among them the stand-ins for closed or unpublished parts ([DEVIATIONS.md](DEVIATIONS.md)).

## Hardware

- iMac G5 (iSight), `PowerMac12,1`: tested, Steps 1 to 10 and 13.
- QEMU `mac99`: tested, Steps 1 to 4, 6 and 11, the CDs and USB disks of Step 9, and the everyday tools built after Step 10.
- Other PowerPC Macs that Leopard supports: untested.

[hardware/](hardware/) has a page per machine, with the drivers it needs and which of them are closed.

## Rules

1. **Apple's release, as published.** Projects and versions are those of darwinbuild's `9L30` build list ([manifest/9L30.tsv](manifest/9L30.tsv)). Sources are fetched as Apple published them, checked by their SHA-256 ([manifest/9L30.sha256](manifest/9L30.sha256)), and never edited in place. A project changes version only to take Apple's own fix from a later release, or, when Apple never published 9L30's version, to build the nearest one it did publish.
2. **The smallest fix, in this order.** Change the build environment; failing that, take Apple's fix from another release; only then write a new patch, as small as possible, with no reformatting or unrelated change. Where the missing part is closed, leave out the feature that needs it, or write an open replacement in `replacements/`.
3. **Every change documented.** Every patch, build-environment workaround and replacement is listed in [DEVIATIONS.md](DEVIATIONS.md). The files that `scripts/make-test-image` writes or edits on the image are listed in the book's [Making the image](docs/src/method/making-the-image.md).
4. **The release's own toolchain.** Builds use `gcc` 5465, `gcc_42` 5531, `cctools` 667.3 and `ld64` 77.1.
5. **Open source only in the image.** The image holds only what is built from open source or written by this repo's scripts: no Apple binary, and no prebuilt binary that a source carries. Closed tools may run the build.

## Steps

- ✅ **Step 1.** A disk image whose files all come from this repo's builds boots to a single-user root shell (`-v -s`) in QEMU `mac99` and on an iMac G5.
- ✅ **Step 2.** The same image, booted without `-s`, runs launchd's multi-user startup to a `login:` prompt on the console, where root logs in.
- ✅ **Step 3.** On the iMac G5, the image brings up its Ethernet port and accepts SSH logins.
- ✅ **Step 4.** Over SSH, everyday commands work: `grep`, `ps`, `less` and `vi` run, syslogd writes `/var/log/system.log`, and ntpd sets the clock from the network.
- ✅ **Step 5.** `reboot` restarts the machine, `shutdown -h now` switches it off, and the clock is read from the hardware at boot.
- ✅ **Step 6.** securityd runs: SSH and console logins get their own security session, and launchctl and login use Security as Apple built them.
- ✅ **Step 7.** OpenSMU drives the fans from the SMU's temperature sensors, and the machine is as quiet at idle as under Leopard.
- ✅ **Step 8.** The image boots cleanly: the network comes up without a 60-second wait, `ntpdate` sets the clock at boot, the local time zone applies, BootCache runs, and `/var/log/system.log` shows no errors.
- ✅ **Step 9.** DiskArbitration mounts the internal partitions, USB disks and CDs as they appear (CDs tested on the iMac through a USB drive), `bless` makes a partition the startup disk, and the built-in audio plays sound.
- ✅ **Step 10.** The image is a working Unix system: its own compiler builds and links a program, Perl, Python and Ruby run, and Samba shares a folder with another machine.
- ✅ **Step 11.** In QEMU `mac99`, an open driver for the PMU reads the clock from the hardware at boot, `reboot` restarts the machine, `shutdown -h now` switches it off, and the network comes up without a 60-second wait.
- ⬜ **Step 12.** On the iMac G5, a FireWire disk mounts as it is plugged in, through an open driver for the FireWire controller.
- ✅ **Step 13.** On the iMac G5, the GPU and the north bridge are as cool at idle as under Leopard.
- ⬜ **Step 14.** A USB key holding the image boots the iMac G5 to the same system, with no internal disk involved.

## Build it

PowerDarwin is built on a PowerPC Mac running Leopard 10.5.8 with Xcode 3.1.4, the Mac it is then installed on, from the sources Apple published. Leopard cannot reach GitHub or the Wayback Machine over TLS, so a current computer downloads everything first, into one file, the kit. The build takes about 10 hours on an iMac G5 and needs 40 GB free. From a kit made on a fresh clone, `build-image` ran from an empty folder to the end in one go, in 9 hours 41 minutes, and the image, installed with `install-image`, started on the same iMac.

1. On a current computer, macOS or Linux, with `git`, `curl` and Java 6 or later, clone this repo and run `scripts/download-kit`. It downloads Apple's sources from [apple-oss-distributions](https://github.com/apple-oss-distributions) and the Wayback Machine, darwinbuild's from [macosforge/darwinbuild](https://github.com/macosforge/darwinbuild), and the Roots, Apple's builds of each project, which stand in for the projects not built yet, from [nyg/PowerDarwin-roots](https://github.com/nyg/PowerDarwin-roots), a copy of them on GitHub, and the six it lacks from the Wayback Machine. It checks each file against its SHA-256 in `manifest/`, makes [D49](DEVIATIONS.md#d49)'s parser with Java, and packs it all with the repo into `PowerDarwin-kit.tar`, about 2.2 GB. The Wayback Machine refuses connections for a few minutes after many requests: `download-kit` then waits and tries again, for up to an hour per file. Run it again if a download still fails: it keeps what it has.
2. Download Xcode 3.1.4, `xcode314_2809_developerdvd.dmg`, from [Apple's developer downloads](https://developer.apple.com/download/all/) (a free Apple account is enough), and install it on the PowerPC Mac.
3. Copy the kit to the PowerPC Mac: on a USB key, or over HTTP, with `python3 -m http.server` in the kit's folder on the current computer and `curl -O http://<its address>:8000/PowerDarwin-kit.tar` on the Mac. That server cannot resume a transfer: if `curl` stops with `transfer closed`, run it again.
4. On the PowerPC Mac, in Terminal, run `tar -xf PowerDarwin-kit.tar`, `cd PowerDarwin` and `sudo scripts/build-image`. It checks the kit's hashes again, installs darwinbuild into `/usr/local` unless it is there, makes the build folder `~/9L30` (`--folder=<path>` for another), stages the sources and Roots, and runs the 283 steps of [manifest/build-order](manifest/build-order): each darwinbuild build, in order, and each build-environment workaround of DEVIATIONS.md where the builds need it. Each build runs in UTC, under the host name `powerdarwin.invalid` and the login name `root`, so that no program records the Mac's own names or its user's ([D125](DEVIATIONS.md#d125)): the Mac takes that host name while darwinbuild runs and gets its own back after, as it does at its next start in any case. Then it makes the image, `/private/var/tmp/PowerDarwin.dmg`, and checks every file on it with `scripts/check-image`. Set Computer sleep to Never in Energy Saver first: a sleeping Mac pauses the build.
5. If a step fails, `build-image` stops and names darwinbuild's log. Run it again: it goes on from that step.

### Working on it

Changing a project means running the same steps one at a time. From a current Mac, the scripts reach the build machine over SSH, through its host name; on the build machine itself, `--here=<build folder>` takes the host's place.

1. `scripts/fetch-sources` downloads into `sources/` every source whose hash `manifest/9L30.sha256` records, and `scripts/fetch-roots` the Roots into `roots/`; `download-kit` runs both.
2. Copy `manifest/plists/*.plist` into `~/9L30/.build/`, then run `darwinbuild -init 9L30` in `~/9L30` ([D2](DEVIATIONS.md#d2), [D25](DEVIATIONS.md#d25)).
3. `scripts/stage-sources <host> <project>...` unpacks each project's source into `~/9L30/Sources` as `<project>-<version>`, applies its patches and touches the files listed in its `touch` file. `<project>=<version>` stages another version, or a project from `replacements/`.
4. `scripts/stage-roots <host>` copies the chosen Roots into `~/9L30/Roots/.DownloadCache`; `manifest/9L30-roots-used.tsv` says which build each came from.
5. `scripts/prepare-buildroot <host>` puts the build-environment workarounds of DEVIATIONS.md into the build environment, one step per workaround. A step already in place is left alone, a step whose input is not built yet says so and waits, and `--check` only reports. From a current Mac, D49's parser is made there, with Java 6 or later; on the build machine, it is taken from the kit's `inputs/antlr2`. darwinbuild mounts the build environment at its first build, so run step 6 once, for any project, before this one.
6. On the build machine, `cd ~/9L30 && sudo /usr/local/bin/darwinbuild <project>`, or `sudo scripts/run-darwinbuild ~/9L30 <project>`, which builds it as `build-image` does, under neutral names ([D125](DEVIATIONS.md#d125)). Self-built Roots land in `~/9L30/Roots/<project>/` and take precedence over downloaded ones.
7. A failure is fixed by rule 2 and listed in DEVIATIONS.md. A new build, or a new step of `prepare-buildroot`, goes into `manifest/build-order` where the builds need it.

## Install it

Each release is a page under [Releases](https://github.com/nyg/PowerDarwin/releases) with three files: `PowerDarwin-<name>.img.gz`, the image, a raw disk with an Apple Partition Map, gzipped; `PowerDarwin-<name>-source.tar`, the source of every binary on it; and `SHA256SUMS`. To have the same from your own build, make the image (above) again with `make-test-image --release=<name>`, which makes it as a release has it: root has no password, and the time zone is UTC. `/private/var/tmp/PowerDarwin.cdr.gz` on the build machine is then the same raw image, gzipped.

### In QEMU

On a current computer, macOS or Linux, with QEMU's `qemu-system-ppc` (tested with QEMU 11.1.3) and a clone of this repo:

1. Check the files with `shasum -a 256 -c SHA256SUMS`, or `sha256sum -c SHA256SUMS`, and unpack the image with `gunzip PowerDarwin-<name>.img.gz`.
2. Run `scripts/boot-qemu PowerDarwin-<name>.img -v`. `-v` starts the system verbosely, up to the login prompt; without it, `boot-qemu` starts the single-user shell (`-v -s`). The guest's SSH port is forwarded to port 2222 of `127.0.0.1`.
3. At the first boot, the console asks for root's password, twice, of which only the first 8 characters count. Then it makes the SSH host keys, in about a minute in QEMU, turns SSH on and shows `login:`. Later boots go straight to `login:`.
4. Log in as root on the console, or over SSH with `ssh -p 2222 -o HostKeyAlgorithms=+ssh-rsa -o KexAlgorithms=+diffie-hellman-group14-sha1 root@127.0.0.1`.
5. `shutdown -h now` switches it off.

A test image, which `make-test-image` makes without `--release`, has root's password already when the build folder holds `root-password.hash`, its DES hash (`openssl passwd -crypt -stdin`), and the build machine's time zone and NTP server.

### On a PowerPC Mac, from Leopard

1. On a current computer, check and unpack the image as above, then copy it, with a clone of this repo, to the PowerPC Mac: Leopard cannot reach GitHub over TLS. On the build machine itself, `install-image` takes `/private/var/tmp/PowerDarwin.dmg` unless `--image` names another.
2. Run `sudo scripts/install-image --image=PowerDarwin-<name>.img`. It asks where PowerDarwin goes: onto a new partition taken from the end of a Journaled HFS+ volume, which keeps its files (Leopard's own volume will do); onto an existing partition, which is erased; or onto a whole disk, which is erased. It needs 2 GB, and offers 10 GB. Then it asks for the time zone and the NTP server, offering Leopard's own, for root's password, and whether to make PowerDarwin the startup disk, and it shows what it will do before it changes anything. Options answer without asking: `--shrink=disk0s3 --size=10G`, `--partition=disk0s4` or `--disk=disk1`, `--time-zone=America/New_York`, `--ntp-server=pool.ntp.org`, `--startup-disk`, and `--yes`.
3. Start it: hold Option at power-on and choose it in the Startup Manager, or restart, if it is the startup disk. Its console is a text console, seen in a verbose start: from Open Firmware, `setenv boot-args -v` then `boot sata/k2-sata@0/@0:<partition number>,\\:tbxi` starts it verbosely on an iMac G5.
4. Log in as root on the console, or over SSH, which the first boot turns on and which takes root's password: keep the Mac on a network you trust. A current OpenSSH client needs `-o HostKeyAlgorithms=+ssh-rsa -o KexAlgorithms=+diffie-hellman-group14-sha1`.
5. `bless --mount /Volumes/<Leopard volume> --setOF` and `reboot` go back to Leopard.

An image put onto a Mac another way, with `asr` by hand, has no root password, as in QEMU: its console asks for one at the first boot, in a verbose start, and SSH stays off until then. As on any Mac without a firmware password, whoever is at the console can also start the single-user shell, `-v -s`, which asks for no password.

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

## Page views

The published book counts its page views with [GoatCounter](https://powerdarwin.goatcounter.com), through a Cloudflare Worker, so that blockers listing GoatCounter's domains do not drop them. `docs/src/count.js` is an unmodified copy of `https://gc.zgo.at/count.v4.js`, served with the book, and `docs/theme/head.hbs` points it at the Worker's `/hit` endpoint. The Worker in `worker/` accepts only `POST /hit` from `https://nyg.github.io`, and forwards the hit to GoatCounter's API with the visitor's IP address, user agent and language. Publishing the book does not deploy the Worker:

```sh
cd worker
pnpm install

# the GoatCounter API token, never committed (use worker/.dev.vars for local runs)
pnpm wrangler secret put GOATCOUNTER_TOKEN

pnpm run deploy
```

## Licence

- What this project writes is under the MIT licence, in [LICENSE](LICENSE): the scripts, the files of `overlay/`, the files of `env/` but the two below, the replacements in `replacements/` (each with its own copy of the licence, OpenOnyx's with the BSD notice of the FreeBSD table it copies, and OpenRadeonPM's with the MIT notices of the X.Org files it follows), the book in `docs/`, the Worker in `worker/` and the other documents.
- `env/ditto` is darwinbuild's `ditto.sh` with one change, under darwinbuild's BSD licence from Apple, and `env/quarantine.h` holds the stubs of copyfile's `copyfile.c` with four additions, under the APSL 2.0. Each file says so in its first lines.
- `docs/src/count.js` is GoatCounter's script, under the ISC licence, as its first lines state.
- Each patch in `patches/` changes another project's source, so its changes are under the licence of the files it changes, as that project's source states: the APSL, version 2.0 or 1.1, for most of Apple's projects, and the licences of bash, gcc, Kerberos, ntp, Python, Ruby, Samba and the others for theirs. Its first line gives its deviation and the date of the change. A file that a patch adds is this project's, under the MIT licence, unless its notice says otherwise: the one that `patches/CF-476.19/0003-string-transform-and-forced-preferences.patch` adds comes from swift-corelibs-foundation, under the Apache License 2.0 with Runtime Library Exception.
- [LICENSES/](LICENSES/) holds the texts of the APSL 2.0 and 1.1, and of the Apache License 2.0 with Swift's Runtime Library Exception.
- `manifest/plists/` holds darwinbuild's Darwin 9 build lists, under darwinbuild's BSD licence from Apple, in [manifest/plists/LICENSE](manifest/plists/LICENSE), with this project's changes to `9A581.plist`.
- `reference/` holds listings and logs captured from Leopard, kept as records of what Apple's system does.
- The image holds, in `/usr/share/doc/PowerDarwin`, a notice saying where the source of its binaries is, the texts of the APSL 2.0 and 1.1 and of the Apache License 2.0, and one file per project built with the licence files and notices of its source, which `scripts/collect-notices` gathers; Apple's builds of some projects add theirs in `/usr/local/OpenSourceLicenses`. A release holds, beside its image, the source of every binary on it ([Making the image](docs/src/method/making-the-image.md#releases)).
- The book quotes short excerpts of Apple's and others' sources, each under its own licence, with the project, version and file it comes from.

PowerDarwin is an independent project, not affiliated with, endorsed or sponsored by Apple Inc. Apple, Mac, Mac OS, iMac and Power Mac are trademarks of Apple Inc., and PowerPC is a trademark of IBM; the other names belong to their owners.
