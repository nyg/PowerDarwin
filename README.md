# PowerDarwin

Darwin 9.8 for PowerPC Macs: the open core of Mac OS X 10.5.8 Leopard (build `9L30`), built from the sources Apple published, with the fewest changes that make it boot and run, each one documented.

The book in [docs/](docs/src/README.md), published at [nyg.github.io/PowerDarwin](https://nyg.github.io/PowerDarwin/), explains the project from the hardware up: what Darwin is, how it is built, and how each step was reached.

## Credits

- Apple, for publishing Darwin's sources ([apple-oss-distributions](https://github.com/apple-oss-distributions)) and darwinbuild ([macosforge/darwinbuild](https://github.com/macosforge/darwinbuild)), whose `Darwin9` branch builds every project here, unchanged.
- The [Internet Archive](https://web.archive.org/), whose Wayback Machine still serves darwinbuild's Roots and the opensource.apple.com tarballs that GitHub lacks.
- Andrew-R, whose [darwinbuild work on Darwin 9 for PowerPC](https://forums.macrumors.com/threads/building-with-darwinbuild-for-darwin9-ppc-in-2026-wip.2485638/) built 100 of its projects on a Leopard PowerPC Mac in 2026.
- [barracuda156](https://github.com/barracuda156) and the [macos-powerpc](https://github.com/macos-powerpc) organisation, who keep darwinbuild and Apple's projects building for PowerPC.
- NetBSD's macppc drivers, [`smu.c`](https://github.com/NetBSD/src/blob/trunk/sys/arch/macppc/dev/smu.c) and [`fancontrol.c`](https://github.com/NetBSD/src/blob/trunk/sys/arch/macppc/dev/fancontrol.c), whose SMU protocol and fan zones OpenSMU follows, FreeBSD's [`onyx.c`](https://github.com/freebsd/freebsd-src/blob/main/sys/dev/sound/macio/onyx.c), whose codec settings OpenOnyx uses, and Linux's [`smu.c`](https://github.com/torvalds/linux/blob/master/drivers/macintosh/smu.c) and [`windfarm_pm121.c`](https://github.com/torvalds/linux/blob/master/drivers/macintosh/windfarm_pm121.c), read for facts about the SMU and the fans.
- [PureDarwin](https://github.com/PureDarwin/PureDarwin), which built bootable Darwin 9 systems for Intel machines.
- [QEMU](https://www.qemu.org/) and [OpenBIOS](https://github.com/openbios/openbios), whose `mac99` machine tests each image before it reaches a real Mac.

[RESOURCES.md](RESOURCES.md) lists everyone else's work this project draws on.

## Status

Steps 1 to 10 are reached: on an iMac G5, the image boots to a multi-user system with Ethernet, SSH, fan control, disks, USB, sound, its own compiler, Perl, Python, Ruby and Samba, and restarts and powers off. Not working yet: FireWire, the microphone, Wi-Fi, graphics acceleration and sleep. The image also lacks `awk`, `tar`, `gzip`, `file`, `lsof`, `top` and `curl`, which no step has built yet.

The comparisons with Leopard are provisional: the reference system is Sorbet Leopard, a community update of Leopard 10.5.8 ([reference/imacg5/](reference/imacg5/README.md)), until they are repeated against an unmodified Leopard 10.5.8.

## Hardware

- iMac G5 (iSight), `PowerMac12,1`: tested, Steps 1 to 10.
- QEMU `mac99`: tested, Steps 1 to 4 and 6, and the CDs and USB disks of Step 9.
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
- ✅ **Step 9.** DiskArbitration mounts the internal partitions, USB disks and CDs as they appear (CDs tested in QEMU only), `bless` makes a partition the startup disk, and the built-in audio plays sound.
- ✅ **Step 10.** The image is a working Unix system: its own compiler builds and links a program, Perl, Python and Ruby run, and Samba shares a folder with another machine.
- ⬜ **Step 11.** A USB key holding the image boots the iMac G5 to the same system, with no internal disk involved.

## Build it

A current Mac downloads the sources and Roots, since Leopard cannot reach GitHub or archive.org over TLS. Sources come from [apple-oss-distributions](https://github.com/apple-oss-distributions) by tag; the 14 projects with no tag there come, when a step needs one, from the archived opensource.apple.com tarballs. A PowerPC Mac running Leopard with Xcode 3.1.4 and darwinbuild's `Darwin9` branch builds, in the build folder `~/9L30`.

1. `scripts/fetch-sources` downloads the 372 projects that have a tag into `sources/`, and `scripts/fetch-sources autozone SQLite` the two archived ones built so far. Four more tarballs are downloaded by hand, as [the book](docs/src/method/sources-and-roots.md) tells; until they are there, the script's last check names them. `scripts/fetch-roots` downloads the Roots into `roots/`. Both check the hashes in `manifest/`.
2. Copy `manifest/plists/` into `~/9L30/.build/`, then run `darwinbuild -init 9L30` in `~/9L30` ([D2](DEVIATIONS.md#d2), [D25](DEVIATIONS.md#d25)).
3. `scripts/stage-sources <host> <project>...` unpacks each project's source into `~/9L30/Sources` as `<project>-<version>`, applies its patches and touches the files listed in its `touch` file. `<project>=<version>` stages another version, or a project from `replacements/`.
4. `scripts/stage-roots <host>` copies the chosen Roots into `~/9L30/Roots/.DownloadCache`; `manifest/9L30-roots-used.tsv` says which build each came from.
5. `scripts/prepare-buildroot <host>` puts the build-environment workarounds of DEVIATIONS.md into the build environment, one step per workaround. A step already in place is left alone, a step whose input is not built yet says so and waits, and `--check` only reports. D49's parser is made on the current Mac, which needs Java 6 or later. darwinbuild mounts the build environment at its first build, so run step 6 once, for any project, before this one.
6. On the build machine, `cd ~/9L30 && sudo /usr/local/bin/darwinbuild <project>`. Self-built Roots land in `~/9L30/Roots/<project>/` and take precedence over downloaded ones.
7. A failure is fixed by rule 2 and listed in DEVIATIONS.md.

## Install it

No image is published yet: build one, then install it.

1. On the build machine, `scripts/make-test-image` writes the image as `/private/var/tmp/darwin9-step1.dmg`, an Apple-partitioned HFS+ disk of 1.6 GB, and a raw copy as `darwin9-step1.cdr`. Root's password is the DES hash in `~/9L30/root-password.hash` (`openssl passwd -crypt -stdin`); without that file, root cannot log in.
2. To try it in QEMU, copy the raw image to the current Mac and run `scripts/boot-qemu <image> [boot-args]`. Without boot-args it boots to the single-user shell (`-v -s`); with `-v` it boots to the login prompt, and the guest's SSH port is forwarded to port 2222.
3. To install it on a PowerPC Mac, make an empty HFS+ partition of at least 2 GB, then, from Leopard on the same Mac:
   ```
   hdiutil convert /private/var/tmp/darwin9-step1.dmg -format UDZO -o darwin9-asr.dmg
   sudo asr imagescan --source darwin9-asr.dmg
   sudo asr restore --source darwin9-asr.dmg --target /Volumes/<partition> --erase --noprompt
   ```
4. Boot it: hold Option at power-on and choose it in the Startup Manager, or run `sudo bless --mount /Volumes/<partition> --setOF` and restart. From Open Firmware, `setenv boot-args -v` then `boot sata/k2-sata@0/@0:<partition number>,\\:tbxi` boots it verbosely on an iMac G5.
5. Log in as root on the console or over SSH. `bless --mount /Volumes/<Leopard volume> --setOF` and `reboot` go back to Leopard.

## Layout

- `manifest/`: project versions, their source, the hashes of the fetched tarballs and Roots, and darwinbuild's build lists in `plists/`.
- `patches/<project>-<version>/`: one patch file per fix.
- `env/`: build-environment workarounds, each with the error it fixes.
- `replacements/`: new code that stands in for a closed Apple library or driver.
- `LICENSES/`: the texts of the licences that the patches and the copied files are under.
- `scripts/`: fetching, staging and building (`fetch-sources`, `fetch-roots`, `resolve-buildlist`, `stage-sources`, `stage-roots`, `prepare-buildroot`), making and checking the image (`make-test-image`, `check-image`, `cmp-binaries`, `cmp-disassembly`, `check-family-imports`), running it (`boot-qemu`, `qemu-type`, `ssh-root`), and the book's indexes (`make-book-indexes`). The book's Method chapters explain each one.
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

- What this project writes is under the MIT licence, in [LICENSE](LICENSE): the scripts, the files of `env/` but the two below, the replacements in `replacements/` (each with its own copy of the licence, and OpenOnyx's with the BSD notice of the FreeBSD table it copies), the book in `docs/`, the Worker in `worker/` and the other documents.
- `env/ditto` is darwinbuild's `ditto.sh` with one change, under darwinbuild's BSD licence from Apple, and `env/quarantine.h` holds the stubs of copyfile's `copyfile.c` with three additions, under the APSL 2.0. Each file says so in its first lines.
- `docs/src/count.js` is GoatCounter's script, under the ISC licence, as its first lines state.
- Each patch in `patches/` changes another project's source, so its changes are under the licence of the files it changes, as that project's source states: the APSL, version 2.0 or 1.1, for most of Apple's projects, and the licences of bash, gcc, Kerberos, ntp, Python, Ruby, Samba and the others for theirs. Its first line gives its deviation and the date of the change. A file that a patch adds is this project's, under the MIT licence, unless its notice says otherwise: the one that `patches/CF-476.19/0003-string-transform-and-forced-preferences.patch` adds comes from swift-corelibs-foundation, under the Apache License 2.0 with Runtime Library Exception.
- [LICENSES/](LICENSES/) holds the texts of the APSL 2.0 and 1.1, and of the Apache License 2.0 with Swift's Runtime Library Exception.
- `manifest/plists/` holds darwinbuild's Darwin 9 build lists, under darwinbuild's BSD licence from Apple, in [manifest/plists/LICENSE](manifest/plists/LICENSE), with this project's changes to `9A581.plist`.
- `reference/` holds listings and logs captured from Leopard, kept as records of what Apple's system does.
- The book quotes short excerpts of Apple's and others' sources, each under its own licence, with the project, version and file it comes from.

PowerDarwin is an independent project, not affiliated with, endorsed or sponsored by Apple Inc. Apple, Mac, Mac OS, iMac and Power Mac are trademarks of Apple Inc., and PowerPC is a trademark of IBM; the other names belong to their owners.
