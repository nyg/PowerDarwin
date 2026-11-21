# PowerDarwin

Darwin 9.8 (Mac OS X 10.5.8, build `9L30`) built for PowerPC from Apple's published sources, with the smallest documented fixes.

## Step 1

A disk image whose files all come from this repo's builds boots to a single-user root shell (`-v -s`) in QEMU `mac99` and on an iMac G5.

## Step 2

The same image, booted without `-s`, runs launchd's multi-user startup to a `login:` prompt on the console, where root logs in, in QEMU `mac99` and on an iMac G5.

## Step 3

Booted on the iMac G5, the image brings up its Ethernet port and accepts SSH logins.

## Step 4

Logged in over SSH on the iMac G5, everyday commands work: `grep`, `ps`, `less` and `vi` run, syslogd writes `/var/log/system.log`, and ntpd sets the clock from the network.

## Step 5

On the iMac G5, `reboot` restarts the machine, `shutdown -h now` switches it off, and the clock is read from the hardware at boot.

## Step 6

On the iMac G5, securityd runs: SSH and console logins get their own security session, and launchctl and login use Security as Apple built them.

## Step 7

On the iMac G5, OpenSMU drives the fans from the SMU's temperature sensors, and the machine is as quiet at idle as under Leopard.

## Step 8

On the iMac G5, the image boots cleanly: the network comes up without a 60-second wait, `ntpdate` sets the clock at boot, the local time zone applies, BootCache runs, and `/var/log/system.log` shows no errors.

## Rules

1. Project versions are exactly those in [manifest/9L30.tsv](manifest/9L30.tsv), taken from darwinbuild's 9L30 build list. A project moves to another version only to backport Apple's own fix.
2. Sources stay as Apple published them: fetched by tag from [apple-oss-distributions](https://github.com/apple-oss-distributions), their SHA-256 recorded in [manifest/9L30.sha256](manifest/9L30.sha256), never edited in place. The 14 projects marked `archive` have no matching tag there and come from the archived opensource.apple.com tarballs, where those exist: `IOATABlockStorage` 200.1.8 and `IOSCSIArchitectureModelFamily` 200.3.6 were never published.
3. A fix changes the build environment first. Failing that, it backports Apple's fix from a later Apple release. Only then is it a new patch, as small as possible, with no reformatting or unrelated change.
4. Every patch in [patches/](patches/), every workaround in [env/](env/) and every replacement in [replacements/](replacements/) is listed in [DEVIATIONS.md](DEVIATIONS.md).
5. Builds use the toolchain of the release: `gcc` 5465, `gcc_42` 5531, `cctools` 667.3, `ld64` 77.1.
6. The final image holds only what is built from open source or written by this repo's scripts: no Apple binary, and no prebuilt binary that a source carries. Closed tools may run the build: the host's Leopard and Xcode, darwinbuild's Roots and the host's SDK.

## Layout

- `manifest/`: project versions, their source, and the hashes of the fetched tarballs.
- `patches/<project>-<version>/`: one patch file per fix.
- `env/`: build-environment workarounds, each with the error it fixes.
- `replacements/`: new code standing in for a closed Apple library or driver, each listed in DEVIATIONS.md.
- `scripts/fetch-sources [project...]`: downloads the `github` sources, or the named projects including `archive` ones from the Wayback Machine, into `sources/` (not committed), records the hash of each new tarball and checks them all.
- `scripts/resolve-buildlist`, `scripts/fetch-roots`, `scripts/stage-sources`, `scripts/stage-roots`: work out each project's version and build, fetch darwinbuild's Roots from the Wayback Machine, and copy sources and Roots to the build machine.
- `scripts/prepare-buildroot <host>`: puts the build-environment workarounds of DEVIATIONS.md into the build machine's build environment, or with `--check` reports which are in place.
- `scripts/make-test-image` (run on the build machine) and `scripts/boot-qemu`: make an Apple-partitioned HFS+ test image and boot it in QEMU `mac99`, optionally with a CD image, with a `sungem` Ethernet card (Apple's GMAC) on QEMU's user network and the guest's port 22 forwarded to port 2222 on the host (`QEMU_SSH_PORT`). The image's root password is the DES hash in `~/9L30/root-password.hash` on the build machine (`openssl passwd -crypt -stdin`); without that file, root cannot log in.
- `reference/`: what the reference system runs, and a description of Andrew-R's darwinbuild patch.
- [RESOURCES.md](RESOURCES.md): other people's work, tools, and where Apple's sources and binaries live.

## Building

A current Mac downloads the sources and Roots, since Leopard cannot reach GitHub or archive.org over TLS. A PowerPC Mac running Leopard with Xcode 3.1.4 and darwinbuild's `Darwin9` branch builds, in the build folder `~/9L30`.

1. Copy `manifest/plists/` into `~/9L30/.build/`, then run `darwinbuild -init 9L30` in `~/9L30` ([D2](DEVIATIONS.md), [D25](DEVIATIONS.md)).
2. `scripts/stage-sources <host> <project>...` unpacks the GitHub tarballs as `<project>-<version>` (`<project>=<version>` for a replacement at another version, [D40](DEVIATIONS.md), or for a project in `replacements/`, which it copies, [D81](DEVIATIONS.md)) into `~/9L30/Sources` ([D1](DEVIATIONS.md)), applies the project's patches and touches the files listed in its `touch` file ([D20](DEVIATIONS.md)).
3. `scripts/stage-roots <host>` copies the chosen Roots into `~/9L30/Roots/.DownloadCache`; `manifest/9L30-roots-used.tsv` says which build each came from.
4. `scripts/prepare-buildroot <host>` puts the build-environment workarounds in place (below).
5. On the build machine, `cd ~/9L30 && sudo /usr/local/bin/darwinbuild <project>` (Leopard's sudo has no `/usr/local/bin` in its path). Self-built Roots land in `~/9L30/Roots/<project>/` and take precedence over downloaded ones.
6. A failure is fixed by rule 3 and listed in DEVIATIONS.md.

The build environment (`~/9L30/BuildRoot`, a sparse disk image) needs files that darwinbuild does not put there. `scripts/prepare-buildroot <host> [--check] [step...]` places them, one step per deviation: [D4](DEVIATIONS.md), [D5](DEVIATIONS.md), [D6](DEVIATIONS.md), [D7](DEVIATIONS.md), [D8](DEVIATIONS.md), [D9](DEVIATIONS.md), [D10](DEVIATIONS.md), [D13](DEVIATIONS.md), [D14](DEVIATIONS.md), [D15](DEVIATIONS.md), [D18](DEVIATIONS.md), [D19](DEVIATIONS.md), [D24](DEVIATIONS.md), [D30](DEVIATIONS.md), [D36](DEVIATIONS.md), [D37](DEVIATIONS.md), [D40](DEVIATIONS.md), [D49](DEVIATIONS.md), [D59](DEVIATIONS.md), [D60](DEVIATIONS.md), [D64](DEVIATIONS.md), [D67](DEVIATIONS.md), [D70](DEVIATIONS.md), [D73](DEVIATIONS.md), [D82](DEVIATIONS.md); its `plist` step copies `manifest/plists/9A581.plist` into `~/9L30/.build/` and reloads it with `darwinxref loadIndex`. A step already in place is left alone, and a step whose input is not built or staged yet says so and waits, so the script is rerun after the build that unblocks it; `--check` only reports. D49's parser is made on the current Mac, which needs Java 6 or later. The 9L30 `IOKitUser.hdrs` and `AppleDisplays` Roots left in the build environment by the first IOKitUser attempts are not part of it.

## Status

Built from source so far: `bootstrap_cmds`, `file_cmds`, `shell_cmds`, the kernel (`xnu`, booted to a single-user shell on the iMac G5), `libSystem` with all its parts, `dyld`, `launchd`, `bash`, `ncurses`, `libiconv`, `libgcc`, `bsm`, `libutil`, `libedit`, `diskdev_cmds`, `bzip2`, `IOKitUser`, `CF`, `ICU`, `Security` and its parts, `DirectoryServiceMIG`, `configd` (SystemConfiguration, the `configd` daemon with its plug-ins, `scutil`), `notify` (`notifyd`), `IOSerialFamily`, `DiskArbitration`, `network_cmds`, `libresolv`, `ipsec` (`libipsec`), `tcp_wrappers`, `OpenSSL`, `bootp` (the DHCP client), `Kerberos`, `OpenSSH`, `tcl`, `SQLite`, `passwordserver_sasl` (Cyrus SASL), `DSPasswordServerFramework`, `LDAPFramework` (OpenLDAP's LDAP.framework), `DirectoryService`, `SmartCardServices` (PCSC.framework), `SecurityTokend`, `securityd`, `IOKitTools` (`ioreg`), `objc4`, `autozone`, `libstdcxx`, `zlib`, `kext_tools`, `BootX`, `files`, `system_cmds`, `text_cmds`, `pam`, `pam_modules`, `grep`, `less`, `adv_cmds` (`ps`), `syslog`, `ntp`, `vim`, `libmd`, `SystemStubs`, and the drivers `IOPCIFamily`, `IOStorageFamily`, `IOATAFamily`, `IOI2CFamily`, `AppleMacRiscPCI`, `AppleMacRISC2PE`, `AppleKeyLargo`, `AppleMPIC`, `AppleCore99NVRAM`, `AppleVIA`, `AppleGPIO`, `KeyLargoATA`, `AppleFileSystemDriver`, `IOHIDFamily`, `IOUSBFamily`, `IOATAPIProtocolTransport`, `IOCDStorageFamily`, `IODVDStorageFamily`, `IOBDStorageFamily`, `AppleMediaBay`, `AppleKauaiATA`, `AppleK2SATARoot`, `AppleK2SATA`, `IOGraphics`, `AppleMacRISC4PE`, `AppleFlashNVRAM`, `AppleI2C`, `IONetworkingFamily`, `AppleGMACEthernet` and, Panther's version, `IOATABlockStorage`, and `OpenSMU`, our own driver in place of the closed `AppleSMU` ([D81](DEVIATIONS.md)). In a chroot holding only our files, our shell, `ls`, `mount`, `fsck_hfs` and `launchctl` run. A QEMU `mac99` image with this userland and our kernel boots to a single-user shell, with its driver cache made by our kextcache. Every file on it comes from our builds, the USB keyboard driver and BootX included; `IOATABlockStorage`, through which the root disk is reached, is Panther's 130.3.1, since Leopard's 200.1.8 was never published ([D40](DEVIATIONS.md)). The same image, restored onto a partition of the iMac G5's internal disk, boots the iMac to the same shell through our platform, PCI, SATA and disk drivers. Step 1 is reached. Booted without `-s`, the image runs launchd's multi-user startup to getty's `login:` on the console, where root logs in: PAM's `pam_unix` checks the password against `/etc/master.passwd`, into which `make-test-image` writes the hash of a test password, since neither `passwd` ([D44](DEVIATIONS.md)) nor DirectoryService is built. The iMac G5 boots to the same prompt from its partition, and root logs in there too. Step 2 is reached. `Security`, `configd` with its daemon and plug-ins, `notifyd` and `DiskArbitration` are built, Security in Apple's own `Darwin` configuration without the closed CryptKit ([D51](DEVIATIONS.md)); with them kextd, configd, notifyd and diskarbitrationd run, and `reboot` syncs the disks and stops at `MACH Reboot`, in QEMU and on the iMac G5, since only the closed ApplePMU and AppleSMU restart or power off these Macs. With `IONetworkingFamily` and `AppleGMACEthernet`, the iMac G5's Ethernet driver, QEMU's model of that chip comes up as `en0` and gets its address by DHCP; with `DirectoryService` built, root logs in over SSH from another machine, its password checked by PAM through DirectoryService's local directory, about three minutes after power-on. On the iMac G5, its own Ethernet port comes up at gigabit speed and gets its address and name server from the home router by DHCP, and root logs in over SSH from another Mac on the network. Step 3 is reached in QEMU and on the iMac G5. With `grep`, `less`, `adv_cmds`, `syslog`, `ntp` and `vim` built, `grep`, `ps`, `less` and `vi` run over SSH, syslogd writes `/var/log/system.log`, and ntpd sets the clock from the network a few minutes after boot, in QEMU and on the iMac G5. Step 4 is reached. With `OpenSMU` driving the iMac G5's system management unit, `reboot` restarts the iMac G5, `shutdown -h now` switches it off, and the kernel reads the clock from the hardware at boot and writes it when the time is set, in a form Leopard's own driver reads back. Step 5 is reached. With `SmartCardServices`, `SecurityTokend` and `securityd` built, launchd starts securityd at boot on the iMac G5 and in QEMU, and `launchctl`, `launchproxy` and `login` use Security and `pam_launchd` as Apple built them: each SSH login gets its own security session from `launchproxy`, without the `SessionCreate() failed` it logged before, and root logs in on the console with `pam_launchd` required. Step 6 is reached. OpenSMU also drives the iMac G5's three fans: it reads the CPU diode through the SMU, calibrated by the SMU's own data, and the disk, optical drive, GPU and north bridge sensors through our IOI2CFamily on the U4's I²C bus, and sets each fan's speed through the SMU, in zones that work as NetBSD's do, with the sensors and temperatures of Leopard's thermal profile; if a sensor fails or a temperature passes its limit, every fan runs at full speed. At idle the fans run at Leopard's speeds, within about 50 rpm, and the iMac is as quiet; under load the CPU fan speeds up, and slows down again after. Step 7 is reached. Without a GPU driver the GPU and north bridge run some 5 to 9 °C warmer at idle than under Leopard. The vendors' prebuilt smart-card reader drivers that SmartCardServices' source carries are left out of the image. sshd's account phase uses `pam_unix` instead of `pam_securityserver`, which needs OpenDirectory's Objective-C framework ([D80](DEVIATIONS.md)). Without Apple's `ApplePMU` its clock starts at 1 January 1970, and without `IOSCSIArchitectureModelFamily` it has no CD; with that family added, a CD reads through our drivers. `IOATABlockStorage` 200.1.8, `IOSCSIArchitectureModelFamily` 200.3.6, `ApplePMU`, `XsanFilter` and the security policy kexts have no published source. `AppleMacRISC4PE`, the iMac G5's platform driver, builds with 66 register constants that a patch defines ([D39](DEVIATIONS.md)); the published source leaves parts of five of its functions out, of which only a model-name line in panic logs applies to the iMac G5.

The reference system is an iMac G5 running Sorbet Leopard, a community-modified Leopard that reports 10.5.9 build `9L31b` on Apple's 10.5.8 kernel. Results gathered on it stay provisional until they are checked against a Leopard installed from a retail disc.
