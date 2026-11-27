# Wi-Fi: what it would take

## What you will learn

- Which Wi-Fi chip the iMac G5 has, read from the machine, and which chips the other PowerPC Macs of its time have.
- Which parts of Leopard's Wi-Fi Apple published, which are closed, and what the image leaves out because of them.
- Which open drivers, firmware and 802.11 code exist for the iMac's chip, under which licences, and what joining a WPA2 network asks of them.
- Four ways to Wi-Fi, each checked against the five rules: its sources, licences, size, what can be tested where, and its risks.
- Whether Wi-Fi is feasible under the rules, the decisions it needs from the project's owner, and the stages it could be built in.

The image has no Wi-Fi: its network is Ethernet ([Step 3](../steps/step-3-network-and-ssh.md)). Leopard's Wi-Fi drivers, its 802.11 framework and the daemons that join networks are closed, so rule 5 keeps them off the image ([What stays closed](what-stays-closed.md)). This chapter looks at what could take their place within the rules. It is a study, not a step: nothing in it is built. Each finding says how it was checked: on the iMac under Leopard, in Apple's published sources, or in the sources and documents of other projects. What was not checked is said so, and the open questions are listed at the end.

## Words used here

Wi-Fi is the trade name of IEEE 802.11, the standard for wireless local networks. Its 802.11b and 802.11g forms, the two that the iMac's chip speaks, use the 2.4 GHz band at up to 11 and 54 Mbit/s. A station, here the Mac, joins an access point, which announces its network by a name, the SSID. A Wi-Fi chip's PHY and radio send and receive the signal; its MAC layer handles frames, acknowledgements and retries. A chip whose own processor runs only the timing-critical part of the MAC, and leaves scanning, joining and keys to the host, is called SoftMAC; the host's part is its 802.11 stack. The program on the chip's processor is its firmware, which the host's driver loads at every power-up. WPA2 protects a network with keys derived from a passphrase, the pre-shared key (PSK): a four-way handshake between the station and the access point derives the session keys, and CCMP, built on AES, encrypts each frame. WPA, its predecessor, uses TKIP, built on RC4. A supplicant is the code that runs the handshake on the station's side.

## The hardware

### The iMac's chip

An iMac G5 (`PowerMac12,1`) has one Wi-Fi device, which Apple sells as AirPort Extreme. Read from the iMac under Leopard, in the device tree and the registry (`ioreg`, `system_profiler SPAirPortDataType`):

| | |
|---|---|
| Chip | Broadcom BCM4318, `vendor-id` `0x14e4`, `device-id` `0x4318`, `revision-id` 2 |
| Board | Apple's: `subsystem-vendor-id` `0x106b`, `subsystem-id` `0x4318`; node name `pci106b,4318`, `compatible` `pci14e4,4318` |
| Class | `0x028000`, a network controller of no listed kind |
| Bus | PCI, device 1 of the K2's second PCI bus, behind the HyperTransport bridge `pci106b,54` that also holds `mac-io` and the USB controllers; one interrupt line and one 8 KB register window |
| Built in | yes: the bridge's `built-in-names` property lists `WIRELESS` beside `MAC-IO` and `USB2` |
| Radio | 802.11b and 802.11g, 2.4 GHz only |

Leopard's driver publishes more in the `IO80211HardwareVersion` property of its interface: chip revision 2, 802.11 core revision 9, radio `0x82050000`, board revision `0xa`, and the firmware it loads, `4.170.25.8`. An open driver reads the same values from the chip itself, through the backplane described below, so they are listed here only to say which revisions an open driver must handle.

### Other PowerPC Macs

Leopard runs on PowerPC Macs with a G4 of 867 MHz or more, or a G5. The Macs of those years carry three kinds of Wi-Fi, by Apple's technical notes and the device lists of Linux's drivers ([RESOURCES.md](https://github.com/nyg/PowerDarwin/blob/master/RESOURCES.md)):

| Wi-Fi | Chip and IDs | In which Macs | Open drivers | Firmware |
|---|---|---|---|---|
| AirPort, 802.11b | Lucent's Hermes, on the `mac-io` bus | the earlier ones: by Debian's PowerPC notes, the G3 laptops, the titanium PowerBook G4s and the Power Macs before the FireWire 800 models, of which Leopard runs on the faster G4s only | `wi` in OpenBSD's and NetBSD's macppc kernels; Linux's `airport`, removed from Linux in 2023 | on the card; Linux's driver runs on it when it has no file to load |
| AirPort Extreme card, 802.11b/g, Mini-PCI | Broadcom BCM4306, `14e4:4320`, Apple's subsystem `106b:004e` | Macs from 2003 that take the card, fitted when built or later; its FCC filing of February 2003 holds installation guides for the iBook, the PowerBook and the iMac | b43 (or b43legacy for the earliest revisions), bwn, bwi | loaded by the driver: Broadcom's, or OpenFWWF, which lists the BCM4306 |
| AirPort Extreme built in, 802.11b/g | Broadcom BCM4318, `14e4:4318`, Apple's `106b:4318`, which b43's list calls "Airport Extreme 2" | the iMac G5 (iSight); users report it in a Mac mini G4 and a PowerBook G4 | b43, bwn, bwi | as for the BCM4306 |

Apple's AirPort developer note says that in the iMac G5 of October 2005 the module sits on the I/O controller's 33 MHz PCI bus, and in the Power Mac G5 of the same date on its internal 33 MHz PCI bus. Which Broadcom chip each 2005 model carries is not stated by Apple; the rows above give what sources state, and the model lists are not complete. b43 and bwn drive both Broadcom chips with one code base, so a driver for the BCM4318 could take the AirPort Extreme cards whose BCM4306 has an 802.11 core of revision 5 or later too; the earliest BCM4306, which Linux leaves to b43legacy, and the first AirPort card would need drivers of their own.

## Apple's side

### What Leopard runs

Under Leopard 10.5.8 on the iMac, these are the pieces of Wi-Fi, listed from the iMac's disk and its registry, with the version each carries:

| Piece | What it does | Source |
|---|---|---|
| `IO80211Family.kext` 216.1 | the 802.11 family: classes `IO80211Controller` and `IO80211Interface`, subclasses of IONetworkingFamily's `IOEthernetController` and `IOEthernetInterface` | closed |
| `AppleAirPortBrcm4311.kext` 366.91.21, a plug-in of the family | the driver of Broadcom's chips, class `AirPort_Brcm43xx`, which loads Broadcom's firmware into the chip | closed |
| `AirPortAtheros.kext` 320.16.2 and `AppleAirPort.kext`, plug-ins of the family | the drivers of Atheros chips and of the first AirPort card | closed |
| `Apple80211.framework` 528.1, private, with its `airport` tool | the programs' interface to Wi-Fi: open an interface, scan, associate, read the SSID, set keys | closed |
| `airportd`, `Apple80211Monitor.bundle` 528.1 (a configd plug-in), `AirPort Base Station Agent`, `networksetup` | join and remember networks, and the command `networksetup -setairportnetwork` | closed |
| eap8021x 49.9: `EAP8021X.framework`, `EAPOLController.bundle` with `eapolclient` | 802.1X, the authentication of enterprise networks (WPA2-Enterprise) and of some wired ones | open, in the 9L30 build list, not built here |
| bootp 170.1: `IPConfiguration.bundle` | DHCP, and on Wi-Fi it reads the network's SSID | open, built with `NO_WIRELESS` ([D69](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d69)) |
| configd 212.2: SystemConfiguration, InterfaceNamer, PreferencesMonitor | names interfaces, keeps the network preferences, with AirPort's own keys | open, built |
| IONetworkingFamily 47.3 | the network family, Ethernet's classes | open, built |

Of the 9L30 build list's 386 projects, only eap8021x is about Wi-Fi, and of the 509 repositories of Apple's [apple-oss-distributions](https://github.com/apple-oss-distributions) on GitHub, eap8021x is the only one whose name says Wi-Fi, 802.11 or AirPort: no release publishes the family, a driver, the framework or the daemons. Checked with `grep` on [manifest/9L30.tsv](https://github.com/nyg/PowerDarwin/blob/master/manifest/9L30.tsv) and darwinbuild's build lists, and with GitHub's list of the organisation's repositories.

### The headers Apple did publish

Leopard's `Kernel.framework`, on the iMac's disk and in the 10.5 SDK, holds the family's headers in `Headers/IOKit/apple80211/`: `IO80211Controller.h`, `IO80211Interface.h`, `IO80211WorkLoop.h`, `apple80211_ioctl.h`, `apple80211_var.h` and `apple80211_wps.h`, each under the APSL 1.1. `apple80211_ioctl.h` defines the two ioctls through which user programs reach a Wi-Fi interface, `SIOCSA80211` and `SIOCGA80211`, with a request structure and 64 request types, `APPLE80211_IOC_SSID`, `APPLE80211_IOC_SCAN_REQ` and the others. The headers describe the interface, not the code: the user-space framework's own header, `Apple80211API.h`, is not among them, which is why bootp did not build as published ([D69](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d69)).

### What the open projects ask of the closed ones

- bootp 170.1: `ipconfigd.c` reads the SSID with four calls into Apple80211 (`Apple80211Open`, `Apple80211BindToInterface`, `Apple80211Get` with `APPLE80211_IOC_SSID`, `Apple80211Close`), only when an interface reports the media type `IFM_IEEE80211` (`bootplib/interfaces.c`), and uses it only to skip the router check of a renewed address on networks listed as excluded. With `NO_WIRELESS`, bootp's own switch, the function returns no SSID, and every interface is treated as Ethernet.
- eap8021x 49.9: `eapolclient.tproj/wireless.c` includes `Apple80211API.h`, so the project does not build as published either. It scans, associates and sets keys through Apple80211, and for WPA it hands the driver only the master key (`APPLE80211_CIPHER_PMK`): the four-way handshake is not in its code. Its wired 802.1X uses no Apple80211.
- configd 212.2: `SCNetworkInterface.c` gives an Ethernet interface the type `IEEE80211`, AirPort, only when its controller's class conforms to `IO80211Controller`, `AirPortPCI` or `AirPortDriver`; any other is Ethernet. PreferencesMonitor writes a default configuration, with DHCP on every interface, only when no network preferences exist, that is, at the first boot (`Plugins/PreferencesMonitor/prefsmon.c`). The agent that offers to configure an interface that appears later, SCMonitor, is not built ([D61](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d61)).
- xnu 1228.15.4 has no 802.11 code: `bsd/net/` defines the media type `IFM_IEEE80211` and nothing else of 802.11. It exports to kexts the ciphers an 802.11 stack needs: `SHA1Init`, `SHA1Update`, `SHA1Final`, MD5 (`config/Libkern.exports`), `aes_encrypt_key128` and `aes_encrypt_cbc`, and `rc4_init` and `rc4_crypt` (`config/Unsupported.exports`), with AES built for PowerPC in `bsd/crypto/aes/ppc/`.
- IONetworkingFamily 47.3: a driver that subclasses `IOEthernetController` gets an Ethernet interface, `en1` beside the Ethernet port's `en0`; `IOEthernetInterface::performCommand` passes the ioctls it does not handle to its superclass, so a subclass can take ioctls of its own, as `IO80211Interface` does (`IOEthernetInterface.cpp`, and the published `IO80211Interface.h`).

## The open side

### Drivers for the BCM4318

Three open drivers know the chip, one of them in three systems. Each was checked in its source tree and manual page, which [RESOURCES.md](https://github.com/nyg/PowerDarwin/blob/master/RESOURCES.md) lists:

| Driver | System | Licence | The BCM4318 | Firmware it loads | State |
|---|---|---|---|---|---|
| b43, on the ssb bus driver | Linux | GPL-2.0-or-later (b43), GPL (ssb) | `b43_pci_bridge.c` matches `14e4:4318`; `main.c` takes 802.11 core revision 9, and a G-PHY only with radio 2050 | Broadcom's, or OpenFWWF (below) | in mainline, marked orphan in `MAINTAINERS`, still receiving fixes |
| bwn, on bhnd | FreeBSD | BSD-2-Clause; only its N-PHY code, which this chip does not use, is GPL, in `sys/gnu/dev/bwn` | `bwn.4` lists "Apple Airport Extreme, BCM4318, PCI" | Broadcom's 410.2160, from the port `net/bwn-firmware-kmod`, or an open one under `bwn-open_v4_` names | in the tree, built on every architecture, not in powerpc's `GENERIC` kernels |
| bwi | OpenBSD, NetBSD | BSD-3-Clause, from DragonFly | in both PCI tables and manual pages; MAC revision 9 supported | Broadcom's older "v3" firmware | in both trees; in macppc's `GENERIC` kernels |
| bwi | FreeBSD | BSD-3-Clause | `bwi.4` lists the same Apple card | Broadcom's v3 firmware, port `net/bwi-firmware-kmod` | in the tree |

The chip is SoftMAC: b43 and bwn run on their system's 802.11 stack, mac80211 in Linux and net80211 in FreeBSD, which scans, joins and handles keys, while the chip's firmware sends, acknowledges and retries frames. The chip is also several cores on one internal bus, the backplane, the Sonics Silicon Backplane that gives Linux's ssb its name: a PCI core, a common core with the chip's clocks and its SPROM, the small memory that holds the board's MAC address, calibration and limits, and the 802.11 core. A driver reaches each core through one window of the PCI register space, which it moves from core to core; Linux's ssb and FreeBSD's bhnd do that part.

Linux's b43 is known to run on this hardware: a Debian user reported in September 2024 that the Wi-Fi of an iMac G5 with a BCM4318 works once b43-fwcutter has installed Broadcom's firmware, and b43's code has big-endian paths and Apple's board quirks (`main.c`). A FreeBSD report of 2016 says an iMac G5's BCM4318 works there too, without naming the driver. b43 tests at run time whether the chip's DMA engine reaches 30 or 32 bits of address (`dma.c`, `b43_engine_type`); a Fedora report of 2009 shows an iMac G5 whose BCM4306 failed on the 30-bit limit, since fixed in Linux.

Sizes, counted with `wc -l`: b43's C files hold 15,303 lines, of which `main.c` 5,876, `phy_g.c` 3,057, `dma.c` 1,768 and `lo.c` 1,003; the ssb files a PCI card needs hold about 4,700. bwn's G-PHY path holds about 14,300 with its headers, of which `if_bwn.c` 7,744 and `if_bwn_phy_g.c` 3,622, plus bhnd's bus code, which is much larger because it serves every Broadcom chip.

### Firmware

The chip has no firmware of its own in flash or ROM: at every power-up its driver loads the 802.11 core's microcode and its initial register values. b43 and bwn load three files for core revisions 5 to 10: `ucode5`, `b0g0initvals5` and `b0g0bsinitvals5`, and `pcm5` for hardware encryption (b43 `main.c`; bwn `if_bwn.c`). There are two sources:

- Broadcom's, cut out of Broadcom's own drivers by b43-fwcutter (BSD-2-Clause): from `broadcom-wl-4.150.10.5` (firmware 410.2160), `5.10.56.27.3`, `5.100.138` and `6.30.163.46`. Debian's `firmware-b43-installer` picks 5.100.138 for a BCM4318 of revision 2, and downloads it at install time, since its package carries "sourceless proprietary firmware blobs". Linux's b43 documentation states that distributions may not redistribute the extracted firmware, and FreeBSD's ports mark both firmware ports `NO_PACKAGE`. It is closed, and Broadcom's.
- OpenFWWF, the Open FirmWare for WiFi networks, written at the University of Brescia, copyright the university, Lorenzo Nava, Francesco Gringoli and Michael Büsch, under the GPL-2.0. Its last release, 5.2 of 21 July 2009, holds the microcode and both initial-value files as assembly source (`ucode5.asm`, `initvals.asm`), and is built with `b43-asm`, the assembler of b43-tools (GPL-2.0, C with flex and bison, still maintained). It has no `pcm5`. The release is kept by the Wayback Machine, with the same SHA-256 as the copies in Fedora's and OpenWrt's build files; RHEL 6 shipped it as `b43-openfwwf`, and OpenWrt still offers it. It works as a station, an access point and in ad hoc networks, for 802.11b/g, and its own pages list what it lacks: hardware encryption, QoS, RTS/CTS. It reports itself to the driver as firmware 410 with the date `0xFFFF`, which b43 and bwn both take as open firmware: they then turn hardware encryption and QoS off and encrypt in software (b43 `main.c`, bwn `if_bwn.c`).

OpenFWWF's Debian notes say it has been tested only on "core revision 5" variants of the BCM4306, BCM4311, BCM4318 and BCM4320, and warn that on other revisions the radio's behaviour is not validated and could damage the hardware or break spectrum rules. The iMac's 802.11 core is revision 9. Whether OpenFWWF runs on it is the largest open question of this study. b43 and bwn load the same `ucode5` files for revisions 5 to 10, and the assembler's `%arch 5` names that family of cores, but no source found says that OpenFWWF was tried on a revision 9 core.

No other open firmware for these chips was found: Michael Büsch's earlier `b43-ucode` (GPL-2.0, last changed in 2008) is unfinished, and OpenFWWF's derivatives are research variants of it. bwi, in FreeBSD and the other BSDs, loads Broadcom's older v3 firmware, whose interface differs from the v4 one that OpenFWWF speaks, so it cannot load OpenFWWF.

### The 802.11 stack

An open driver for a SoftMAC chip needs an 802.11 stack under it, and xnu has none. Two open ones fit:

- OpenBSD's `sys/net80211`, 24,703 lines under the ISC and BSD licences. It runs WPA and WPA2 with a pre-shared key in the kernel: the four-way handshake (`ieee80211_pae_input.c`, `ieee80211_pae_output.c`), the key derivation (`ieee80211_crypto.c`), CCMP and TKIP in software, on 3,146 lines of `sys/crypto` (AES, RC4, HMAC, key wrap, Michael, MD5, SHA-1). `ifconfig` derives the key from the passphrase with PBKDF2 in user space and hands it to the kernel with `SIOCS80211WPAPSK`; a user joins with `ifconfig bwi0 join home wpakey <passphrase>`. Only WPA2-Enterprise needs a supplicant, wpa_supplicant.
- FreeBSD's `sys/net80211`, 65,020 lines under BSD licences, which leaves WPA's handshake to wpa_supplicant (BSD licence since 2012), through `src/drivers/driver_bsd.c`.

OpenBSD's stack has been ported into IOKit before. OpenIntelWireless's itlwm (GPL-2.0, for Intel chips, macOS 10.13 to 14) carries 27,592 lines of OpenBSD's net80211 and presents the card as an `IOEthernetController`, as this chapter proposes; its sibling AirportItlwm subclasses Apple's `IO80211Controller` instead. Voodoo80211 (BSD-2-Clause) ported the same stack onto `IO80211Controller`, built against Mac OS X 10.7's headers. Both are for Intel Macs on later systems, not for PowerPC or Leopard.

### What joining a network takes

Joining an open network takes a scan (probe requests, and the access points' beacons and probe responses), open-system authentication and association, then data frames turned to and from Ethernet frames. WPA2 with a passphrase adds:

- the PSK, by PBKDF2-HMAC-SHA1, which the image's OpenSSL already has;
- the four-way handshake, EAPOL-Key frames signed with HMAC-SHA1, whose keys come from a pseudo-random function built on HMAC-SHA1, and the group key, unwrapped with AES key wrap;
- CCMP on every data frame, AES in counter mode with a CBC-MAC.

xnu exports AES, RC4, SHA-1 and MD5 to kexts (above), so a driver would write only HMAC, the key wrap, CCMP's two modes, and for WPA, TKIP's Michael. At 54 Mbit/s at most, software AES on the 1.9 GHz G5 is not expected to be the limit; this was not measured. WPA3, whose SAE handshake replaces the PSK, was not looked at: a network that accepts only WPA3 refuses a WPA2 station, and one in WPA2/WPA3 transition mode accepts it.

## How it would fit the image

### Into the network stack

The image's network path from driver to address is the one [Step 3](../steps/step-3-network-and-ssh.md) built for Ethernet, and a Wi-Fi driver can join it at its first link:

1. The driver subclasses IONetworkingFamily's `IOEthernetController` and hands the stack Ethernet frames: it turns each outgoing frame into an 802.11 data frame, with the 802.2 header that carries the frame's type, and each incoming data frame back. The kernel, `ifconfig`, BPF and the DHCP client then see `en1` as one more Ethernet interface.
2. InterfaceNamer names it `en1`. At the first boot, PreferencesMonitor's default configuration gives it DHCP, as it gives `en0`, if the driver has published the interface before IOKit goes quiet. On an image whose preferences already exist, `en1` has no network service until something adds one, since SCMonitor is not built: the join tool below would add it, through SystemConfiguration's `SCNetworkService` calls.
3. Once the station has joined, and for WPA2 once the handshake has set the keys, the driver reports the link as up with `setLinkStatus`. IONetworkingFamily turns that into the kernel event `KEV_DL_LINK_ON`, which configd's KernelEventMonitor posts in the dynamic store, and IPConfiguration starts DHCP on `en1`. When the station leaves the network, the link goes down, and IPConfiguration drops the address.
4. bootp's `NO_WIRELESS` ([D69](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d69)) costs nothing more: IPConfiguration treats `en1` as Ethernet, and its one use of the SSID is the list of networks on which it does not check a cached address by asking the router.
5. configd types the interface as Ethernet, not as AirPort, since the driver's class is not Apple's `IO80211Controller`; the AirPort keys of the network preferences (`kSCEntNetAirPort`, with `PreferredNetwork` and `JoinMode`) stay unused, as nothing on the image reads them.

### What a user would type

Leopard joins a network with `networksetup -setairportnetwork <network> [password]` or with Apple80211's `airport` tool, both closed. The image would need a tool of its own, in `replacements/` beside `openplay`, that talks to the driver through ioctls on `en1`. A session could look like this, with a tool named `wifi` here for the example:

```sh
wifi scan
wifi join home
wifi status
wifi forget home
```

`wifi join` asks for the passphrase, derives the 256-bit PSK from it with PBKDF2-HMAC-SHA1, 4,096 rounds salted with the SSID, as 802.11i specifies, through the image's own OpenSSL 0.9.7l (`PKCS5_PBKDF2_HMAC_SHA1`, checked in OpenSSL 46.11's `crypto/evp/p5_crpt2.c`), and hands the driver the SSID and the PSK. A launchd job would join the remembered networks at boot from a file readable by root only, such as `/etc/wifi.conf`, since the image has no keychain agent. OpenBSD's `ifconfig` takes the same two values, `nwid` and `wpakey`; Apple's `ifconfig`, in network_cmds 307.1.1, has no 802.11 options (`ifconfig.tproj` holds `ifconfig.c`, `ifmedia.c`, `ifbond.c` and `ifvlan.c` only).

## The routes

### Route 1: the built-in chip, an open driver and OpenFWWF

A new kext in `replacements/`, as OpenSMU, OpenOnyx and OpenFWOHCI are, that drives the BCM4318 through its backplane, loads OpenFWWF, and carries a station's 802.11 stack with WPA2; a tool and a launchd job to join networks; OpenFWWF assembled from its source in the build.

| | |
|---|---|
| Source | the driver and the stack: either written by the project, with the hardware's facts from bwn (BSD) and b43 (GPL, read for facts only), as OpenFWOHCI took FreeBSD's and Linux's, or ported from bwn's G-PHY code and OpenBSD's net80211 under their licences; the firmware: OpenFWWF 5.2, from outside Apple; the assembler: b43-tools |
| Licence | MIT for the project's code; BSD and ISC for ported code, whose notices would go with it, as FreeBSD's does in OpenOnyx; GPL-2.0 for the firmware and the assembler, whose source the release's source archive would hold |
| Rules | rule 5 kept: the image holds only code built from source, the firmware included. Rule 2's last clause covers the driver and the stack as an open replacement for closed ones. Rule 1 does not cover OpenFWWF: it is not Apple's, and the project did not write it. Rule 4 is kept for every PowerPC binary; the firmware runs on the chip's own processor, which none of the release's compilers targets, so b43-asm, built with the release's compiler and the build list's flex 20.3 and bison 13, runs in the build. Rule 3: a `closed-replacement` row each for the driver, the firmware and the tool |
| Work | the largest of the routes: a driver of about 10,000 to 15,000 lines, going by b43's G-PHY code with ssb (about 20,000 lines) and bwn's (about 14,300 lines without bhnd), against about 1,600 for OpenFWOHCI; a station's part of an 802.11 stack, which OpenBSD's 24,703 lines include among an access point's, ad hoc networks and 802.11n; the ciphers above xnu's; a tool of a few hundred lines |
| Where it is tested | in QEMU, nothing of the driver: QEMU has no Broadcom Wi-Fi model, and the kext would simply not match. Testable without the chip: the build, the link check with `kextload -n` on the build machine, and the stack's handshake, ciphers and frame code built as a user program against the test vectors of IEEE 802.11 and RFC 6070 (PBKDF2), in QEMU or on the build machine; OpenFWWF's assembly. On the iMac: everything that touches the chip |
| Risks | OpenFWWF on a revision 9 core is untried and, by its own notes, unvalidated; the G-PHY's radio set-up and calibration (`phy_g.c`, `lo.c`) is the most delicate part of b43; the firmware's last release is from 2009; no hardware encryption, so every frame is encrypted by the processor; the DMA engine's address limit (30 bits, if b43's test finds that) against the iMac's DART; channels and power must follow the SPROM's limits and the country's rules |
| Checked | the drivers' sources, licences and manual pages, OpenFWWF's source, licence, notes and release, the image's network path in Apple's sources; nothing built, nothing run on the chip |

### Route 2: the built-in chip, an open driver and Broadcom's firmware

The same driver and stack, with Broadcom's own firmware in place of OpenFWWF.

| | |
|---|---|
| Source | the project's driver and stack, as in route 1; the firmware: Broadcom's, cut out of a Broadcom driver archive by b43-fwcutter |
| Licence | Broadcom's: the firmware may not be redistributed, so no image or release could hold it, and each user would cut it from an archive they fetch, as Debian's installer does |
| Rules | rule 5 broken on every machine that installs it, though not on the image: a closed binary from outside Apple would run on the chip. Rule 1 does not cover b43-fwcutter either |
| Work | route 1's driver and stack, less the firmware's build; an install step that fetches and cuts the firmware |
| Where it is tested | as route 1 |
| Risks | fewer on the chip than route 1: this firmware is the one b43 and bwn were written for, with hardware encryption and QoS, and it is reported working on an iMac G5 under Linux. The download sites are other people's |
| Checked | b43-fwcutter's list, Debian's installer, the drivers' firmware code; nothing built |

### Route 3: a USB adapter with open firmware

A USB Wi-Fi adapter with an Atheros AR9271 chip, whose firmware is open: Qualcomm Atheros published it as `open-ath9k-htc-firmware`. OpenBSD's athn(4) drives it over USB and, since February 2018, loads only that open firmware; Linux's `ath9k_htc` loads it too. The adapter would plug into the iMac's USB 2.0 port, through our IOUSBFamily, and into any PowerPC Mac's.

| | |
|---|---|
| Source | the driver and the stack: written by the project, or ported from OpenBSD's `if_athn_usb.c` (ISC, 2,761 lines) with the parts of `athn.c`, `ar5008.c`, `ar9285.c` and `ar9280.c` it uses (about 13,000 lines with their headers) and OpenBSD's net80211; the firmware: `open-ath9k-htc-firmware` 1.4.0 |
| Licence | ISC for OpenBSD's code; for the firmware, by its README, the ClearBSD licence for Qualcomm Atheros' code, MIT for Tensilica's, and the GPL-2.0 for three files of eCos |
| Rules | rule 5 kept: firmware and driver built from source. Rule 1 does not cover the firmware, nor athn's code if ported. The firmware runs on the chip's Xtensa processor and is built with GCC 10.2 and binutils 2.35 for Xtensa, which its `make toolchain` builds from GNU's sources: a second compiler from outside the release, beside rule 4's |
| Work | about the same driver and stack as route 1; a cross compiler and the firmware's build; no built-in hardware is used |
| Where it is tested | the only route that QEMU might run, by passing the adapter through to the guest's USB 2.0 controller, as `boot-qemu` already adds one for disks; QEMU's documentation calls USB passthrough experimental and describes it for Linux hosts, and on macOS libusb needs root or an entitlement to take a device from its driver. Untried. On the iMac, and on any PowerPC Mac with USB 2.0 |
| Risks | the adapter must be found second-hand: models change chips between versions, as TP-Link's TL-WN722N, an AR9271 in its first version and a Realtek chip in its second and third; the firmware's last tag is 1.4.0 of 2015; the cross compiler's build on the build machine is untried |
| Checked | the firmware's README and Makefile, OpenBSD's athn(4) sources, manual page, macppc kernel configuration and firmware port, QEMU's USB documentation and source; nothing built |

### Route 4: Apple's own

IO80211Family, `AppleAirPortBrcm4311`, Apple80211, `airportd` and Apple80211Monitor are closed, and Broadcom's firmware inside Apple's driver is closed too, so rule 5 keeps all of them off the image. Not an option.

### Without new code

An Ethernet-to-Wi-Fi bridge, such as a small router set as a Wi-Fi client, plugged into the Mac's Ethernet port, gives the image a wireless network today with no change and every rule kept. It is not Wi-Fi in PowerDarwin, and is listed only as the baseline the other routes are measured against.

## How they compare

| Route | Hardware | Rules broken | Code to write | Firmware | QEMU | iMac |
|---|---|---|---|---|---|---|
| 1. Built-in chip, OpenFWWF | the iMac's own | none of 2 to 5; rule 1 does not cover OpenFWWF | driver, stack, tool | open, built from source | stack and ciphers only | everything on the chip |
| 2. Built-in chip, Broadcom's firmware | the iMac's own | rule 5, on each installed machine | driver, stack, tool, install step | closed, cut at install | stack and ciphers only | everything on the chip |
| 3. USB adapter, open firmware | an adapter with an AR9271 | none of 2 to 5; rule 1 does not cover the firmware | driver, stack, tool | open, built with an Xtensa GCC | possibly, by USB passthrough | everything on the adapter |
| 4. Apple's own | the iMac's own | rule 5 | none | closed | no | no |

## Is it feasible under the rules?

Not entirely as the rules stand. A driver, an 802.11 stack, WPA2 and a tool to join networks can all be the project's own code, built with the release's compilers, which every rule allows, as it allowed OpenSMU and OpenFWOHCI. The chip cannot run without firmware, though, and no firmware for it is Apple's or the project's: Broadcom's is closed, which rule 5 forbids, and the one open firmware, OpenFWWF, is a project from outside Apple's release, which rule 1 does not admit today. So Wi-Fi on the iMac's own chip needs one decision of the owner: whether OpenFWWF may be built into the image. With it, route 1 keeps rules 2 to 5 and needs no closed binary at all. Route 3 needs the same kind of decision, for its firmware and its cross compiler, and an adapter bought for it.

Feasible in principle is not the same as proved. The one fact that could stop route 1, whether OpenFWWF runs on the iMac's revision 9 core, can be checked only on the iMac, early and at small cost, before the rest is written.

## Recommendation

1. Route 1, in the stages below, with the first stage as a test of OpenFWWF on the chip before any other work.
2. The driver and the station's stack as the project's own code in `replacements/`, under the MIT licence, written from bwn's and b43's facts and OpenBSD's net80211 design, rather than ports of them: the result is smaller, fits IOKit's classes rather than another kernel's, and keeps the rule that the project's replacements are its own code. If writing is too slow, OpenBSD's net80211 and bwn's G-PHY code can be ported under their BSD and ISC licences, which needs the owner's word, as OpenFWWF does.
3. WPA2 with CCMP in the kernel, as OpenBSD does, so that no supplicant daemon is needed for home networks; TKIP only for WPA2 networks whose group key is still TKIP.
4. Route 2 not at all: it would put a closed binary on every machine that installs PowerDarwin. Route 3 later, if Macs without a Broadcom card or 802.11n matter, and only if the owner admits outside open firmware for route 1 anyway.

## Decisions for the owner

1. Whether Wi-Fi becomes a step in README.md, such as "On the iMac G5, PowerDarwin joins a WPA2 network through its built-in AirPort Extreme and gets an address by DHCP", or stays an idea in [What's next](../steps/whats-next.md).
2. For the firmware: whether OpenFWWF 5.2, open under the GPL-2.0 but from outside Apple's release, may be assembled from its source and put on the image, and where it is kept: in `replacements/` as an open replacement for Broadcom's closed firmware, though the project did not write it, or listed apart as an outside project with its version and hash, as the 9L30 projects are in `manifest/`. Without it, the iMac's own chip has no Wi-Fi under rule 5.
3. For the assembler: whether b43-tools, open under the GPL-2.0 and from outside the release, may run in the build, as the closed Xcode and the build machine's Ruby do, to make the firmware for a processor that the release's own compilers do not target.
4. For the driver and the stack: written by the project, or ported from bwn and OpenBSD's net80211 under their licences, which is again code from outside the release in `replacements/`.
5. Whether to take Broadcom's firmware at install, as route 2 does. Recommended: no.
6. Whether a USB adapter that the user buys, as in route 3, is an acceptable way to Wi-Fi for PowerDarwin, beside or instead of the built-in chip.

## Stages

Each stage is useful on its own and proves something the next one needs. Sizes are rough, from the line counts above, and nothing below has been tried.

| Stage | What it gives | What it takes | Where it is proved |
|---|---|---|---|
| 0. Firmware | OpenFWWF's three files, assembled from source by the build | b43-tools built on the build machine; a build step; a check that the files match those that OpenWrt's and Fedora's builds make from the same source | the build machine, or any computer |
| 1. The chip awake | the kext matches `pci14e4,4318`, reaches the cores through the backplane, reads the SPROM's MAC address, resets the 802.11 core, loads OpenFWWF and sees it run; `en1` appears; a scan lists the networks around | the backplane, SPROM, core reset, firmware loading, interrupts and DMA rings, the G-PHY and radio set-up: most of the driver | the iMac only; this stage answers the open question on core revision 9 |
| 2. Open networks | `wifi join` on a network without a key; DHCP on `en1` through IPConfiguration, as on `en0` | the station's state machine: scan, authentication, association, data frames to and from Ethernet, link status | the iMac, on a guest network or one set up open for the test |
| 3. WPA2 | the iMac on a WPA2 home network | PBKDF2 in the tool, the four-way and group-key handshakes, CCMP in software; OpenBSD's files for them (`ieee80211_pae_input.c`, `ieee80211_pae_output.c`, `ieee80211_crypto.c`, `ieee80211_crypto_ccmp.c`, `ieee80211_crypto_tkip.c`) hold 3,800 lines, an access point's side included | the handshake and the ciphers first as a user program against the standard's test vectors, in QEMU or on the build machine; then the iMac |
| 4. Every day | networks joined at boot from `/etc/wifi.conf`, rejoined after a loss, a `wifi status` with the signal; channels and power by country; TKIP group keys | a launchd job, roaming between access points of one network, the regulatory limits | the iMac, over several days |
| 5. Later, if wanted | bootp's SSID back, and WPA2-Enterprise | an open library with the four Apple80211 calls that bootp makes, written to bootp's use of them, which would drop [D69](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d69); then eap8021x 49.9, whose `eapolclient` would need the rest of Apple80211's calls | the iMac, on an 802.1X network |

## Open questions

- Whether OpenFWWF 5.2 runs on the iMac's 802.11 core revision 9, and whether its radio set-up stays within the board's limits: the question stage 1 answers first.
- Which PHY revision the iMac's chip has, and whether bwn's or b43's G-PHY code covers it unchanged.
- Whether the chip's DMA engine reaches 30 or 32 bits of address, and how IOKit's DMA through the iMac's DART meets that limit; with 30 bits, buffers must lie in the first gigabyte of the address range the engine sees.
- Whether b43-asm runs on a big-endian build machine, or must run on the current computer, as ANTLR does for [D49](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d49).
- How fast software CCMP is on the G5, and how much heat a radio driven without the firmware's power saving adds.
- Whether Leopard's AirPort driver can be unloaded on the build machine, so that the new kext can be tried there under Leopard's debugging tools before PowerDarwin boots with it.
- Whether the chip's country code in the SPROM, which Leopard's driver shows as `AU`, matters for the channels the driver allows.
- For route 3: whether QEMU can pass an AR9271 adapter through to `mac99`, and whether the Xtensa compiler builds on the build machine or only on the current computer.

## Where to look

- In bootp-170.1: `IPConfiguration.bproj/ipconfigd.c` (`S_copy_ssid`, `service_should_do_router_arp`), `bootplib/interfaces.c`, and `bootp.xcodeproj/project.pbxproj` for `NO_WIRELESS`. In eap8021x-49.9: `eapolclient.tproj/wireless.c`. In configd-212.2: `SystemConfiguration.fproj/SCNetworkInterface.c`, `Plugins/PreferencesMonitor/prefsmon.c`, `Plugins/KernelEventMonitor/eventmon.c`. In IONetworkingFamily-47.3: `IONetworkController.cpp` (`setLinkStatus`), `IONetworkInterface.cpp`, `IOEthernetInterface.cpp`. In xnu-1228.15.4: `config/Libkern.exports`, `config/Unsupported.exports`, `bsd/crypto/aes/ppc/`, `bsd/net/if_media.h`. In network_cmds-307.1.1: `ifconfig.tproj/`. In OpenSSL-46.11: `openssl/crypto/evp/p5_crpt2.c`.
- On Leopard: `/System/Library/Frameworks/Kernel.framework/Headers/IOKit/apple80211/`.
- [DEVIATIONS.md](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md): [D61](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d61) and [D69](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d69); the `closed-replacement` rows of OpenSMU, OpenOnyx and OpenFWOHCI ([D81](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d81), [D94](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d94), [D119](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d119)), for how a replacement is written and credited.
- [hardware/powermac12-1.md](https://github.com/nyg/PowerDarwin/blob/master/hardware/powermac12-1.md) and [reference/imacg5/](https://github.com/nyg/PowerDarwin/blob/master/reference/imacg5/): Leopard's AirPort drivers on the iMac.
- [RESOURCES.md](https://github.com/nyg/PowerDarwin/blob/master/RESOURCES.md): b43 and ssb, bwn and bwi, OpenFWWF and b43-tools, b43-fwcutter, OpenBSD's and FreeBSD's net80211, wpa_supplicant, itlwm and Voodoo80211.
- [What stays closed](what-stays-closed.md), [What's next](../steps/whats-next.md), [Step 3](../steps/step-3-network-and-ssh.md).
