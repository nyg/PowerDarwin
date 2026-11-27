# A graphical interface: the options

## What you will learn

- What the image already has for a screen and for input, and what a window server would ask of it.
- Which graphical sources Apple published for Leopard 10.5.8, which of them need closed code, and which the project builds.
- Six ways to a graphical interface, each checked against the five rules: its sources, licences, the work it takes, what a user would see, and where it can be tested.
- Which options fit the rules best, and the decisions the others need from the project's owner.

The image has no graphical interface: its screen shows the kernel's text console and getty's `login:` prompt ([Step 2](step-2-console-login.md)). Leopard's graphical system, from WindowServer and Quartz to the frameworks and applications above them, is closed ([What Darwin is](../background/what-darwin-is.md)), so rule 5 keeps it off the image. This chapter looks at what could take its place within the rules. It is a study, not a step: nothing in it is built. Each finding says how it was checked, from Apple's sources and binaries, the project's own image or other projects' records; what was not checked is said so, and the open questions are listed at the end.

## Words used here

The X Window System, X for short, is the graphical system of most Unix systems. An X server owns a screen, a keyboard and a mouse; X clients are the programs that draw windows on it, and they reach it through a protocol that works as well over a network as on one machine. A window manager is an X client that places, frames and moves the other clients' windows. A framebuffer is the memory whose bytes are the screen's pixels. An NDRV is the display driver that a graphics card's firmware carries, for Mac OS 9, which IOKit's IONDRVSupport runs on Darwin. VNC is a protocol that sends a screen's pixels to a viewer on another computer and the viewer's keys and mouse back.

## What the image has

### The screen

The image has IOGraphics 305.14: IOGraphicsFamily, the family of framebuffers and of display power, and IONDRVSupport, which drives a screen through its NDRV. Neither uses a graphics chip's acceleration. On an iMac G5 (`PowerMac12,1`) the NDRV is `ATY,Aphrodite2`, which sets the screen up and lets the kernel draw into the GPU's memory, and Leopard's own driver for the chip, `ATIRadeon9700`, is closed ([Step 13](step-13-gpu-idle-heat.md)). Any graphical interface on the image therefore draws every pixel with the processor.

Checked in QEMU's `mac99`, with the Step 9 image started to multi-user: kextd loads IOGraphicsFamily and IONDRVSupport, `IONDRVFramebuffer` attaches to `.Display_Video_QemuVGA`, and the console fills the 1024 by 768 screen that `scripts/boot-qemu` asks for (`-g 1024x768x32`). The framebuffer is never opened: it stays in power state 0, with no display connection below it. In IOGraphics 305.14, `IOFramebuffer::newUserClient` calls `open` only when a program asks for the framebuffer's server connection, `kIOFBServerConnectType` (`IOGraphicsFamily/IOFramebuffer.cpp`), which only a window server does; until then the kernel draws its console where the firmware left the screen. So which resolutions and depths the two NDRVs offer is not known: no program on the image has asked for the list.

### Input

The image has IOHIDFamily 258.14 and IOUSBFamily 349.4.3, so USB keyboards and mice reach IOHIDSystem, which hands keys to the console while no program reads its events ([Step 2](step-2-console-login.md)). Checked in QEMU, in the same start: QEMU's USB keyboard and mouse attach as `IOHIDKeyboard` and `IOHIDPointing` under `IOHIDSystem`. On the iMac the USB keyboard types at the console; a USB mouse has had no use there and was not looked at. The ADB drivers of the build list, IOADBFamily 6, AppleADBKeyboard 239.1 and AppleADBMouse 212, are not built, and on a PMU Mac the ADB bus goes through ApplePMU, which is closed: OpenPMU has no ADB ([Step 11](step-11-pmu.md)).

### What a window server needs from it

A window server on Darwin opens the framebuffer and IOHIDSystem as their server, maps the screen's memory, and reads input from a queue it shares with IOHIDSystem. Leopard's sources hold every piece of that, and the image has them built:

- IOGraphics 305.14's `IOFramebufferUserClient` maps the shared cursor, the framebuffer's memory (`kIOFBVRAMMemory`) and its apertures into the server's address space (`IOGraphicsFamily/IOFramebufferUserClient.cpp`).
- IOHIDFamily 258.14's IOHIDSystem accepts a server connection (`kIOHIDServerConnectType`), and `setEventsEnable` turns its events from the console to that queue (`IOHIDSystem/IOHIDSystem.cpp`, `IOHIDSystem/IOHIDUserClient.cpp`).
- IOKitUser 388.53.30 wraps both in C functions: `IOFramebufferOpen`, `IOFBGetDisplayModes`, `IOFBSetDisplayModeAndDepth`, `IOFBGetFramebufferInformationForAperture` and `IOFBCreateSharedCursor` (`graphics.subproj/IOGraphicsLib.c`), `IOHIDCreateSharedMemory` and `IOHIDSetEventsEnable` (`hidsystem.subproj/IOHIDLib.c`). Checked with `nm` on the Step 9 image: our IOKit.framework exports all of them.
- IOHIDFamily's keyboard driver carries a built-in US key map and publishes it in the registry as `kIOHIDKeyMappingKey` (`IOHIDFamily/IOHIDKeyboard.cpp`, `IOHIDSystem/IOHIKeyboard.cpp`), from which a server can read its key map.

## What Apple published for 9L30

The 9L30 build list ([manifest/9L30.tsv](https://github.com/nyg/PowerDarwin/blob/master/manifest/9L30.tsv)) holds these graphical projects. "Closed links" are the closed frameworks and libraries that Apple's own binaries link, checked with `otool -L` on the PowerPC slices of darwinbuild's archived Roots and on Leopard 10.5.8's installed `/usr/X11`:

| Project | What it holds | Closed links | Built here |
|---|---|---|---|
| IOGraphics 305.14 | IOGraphicsFamily, IONDRVSupport | none | yes, on the image |
| IOKitUser 388.53.30 | IOKit.framework, with IOGraphicsLib, IODisplayLib and IOHIDLib | CoreFoundation, which the image has as CF-Lite | yes, on the image |
| X11proto 15.5 | X's protocol headers, pkg-config, FreeType 2.3.9, fontconfig 2.6.0 | FreeType: CoreServices, ApplicationServices and CoreFoundation, for its Mac font support; fontconfig and the `fc-*` tools: ApplicationServices | no |
| X11libs 17.6 | about 50 libraries: libX11, Xt, Xaw, Xft, xcb, pixman, libpng 1.2.35 and others | none | no |
| X11apps 14.3 | about 80 programs: xterm, twm, xdm, xauth, xclock, xcalc, xman, xkbcomp, `xwd`, `glxgears` | `mkfontscale` and `fonttosfnt`: ApplicationServices, through FreeType; `glxgears` and `glxinfo`: OpenGL, through `libGL` | no |
| X11fonts 5.1 | bitmap, Type 1 and TrueType fonts, among them Bitstream Vera | none (fonts only) | no |
| X11misc 6 | imake, makedepend, lndir | none | no |
| X11server 48.7 | xorg-server 1.4.2-apple33, built twice; Mesa 6.5.2 and 7.0.4; xinit 1.1.1 | see below | no |
| tcl 64 | Tcl 8.4.7, and Tk 8.4.7 built for Aqua | Tk: the Aqua frameworks ([D71](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d71)) | Tcl only |
| emacs 70.1, vim 39 | Emacs and Vim, configured `--without-x` | none | vim only |
| wxWidgets 11 | wxWidgets for the Mac, `--with-mac` | Carbon, Cocoa, QuickTime, WebKit and others | no |
| AppleDisplays 1151.0.0 | per-display override lists, which IODisplayLib reads from `/System/Library/Displays/Overrides`, and the closed `AppleOnboardDisplay` kext | `AppleOnboardDisplay` is a binary | no |

`X11server.plist` gives the MIT licence for X11server's four imports, Mesa 6.5.2 and 7.0.4, xorg-server and xinit. FreeType 2.3.9 is under the FreeType License or the GPL-2.0, at the user's choice (`docs/LICENSE.TXT`), and fontconfig under Keith Packard's MIT-style notice (`COPYING`). The licences of the many packages in X11libs and X11apps were not read one by one.

### X11server: two X servers

X11server 48.7's `build` script builds xorg-server 1.4.2-apple33 twice. The first build is Xquartz, X11.app's server, which draws through Xplugin, a closed library that hands its windows to WindowServer (`hw/xquartz/xpr/`); Apple's binary links Carbon, Cocoa, AppKit, Foundation, ApplicationServices, CoreServices, CoreAudio and OpenGL. The second build is configured with `--disable-xquartz --enable-kdrive --enable-xfake --enable-xephyr --enable-xnest --enable-xvfb`, and gives four servers that need no WindowServer: Xvfb, whose screen is only memory; Xnest and Xephyr, which show their screen as a window of another X server; and Xfake, kdrive's test server. Leopard 10.5.8 installs all four in `/usr/X11/bin`, and they link only libraries in `/usr/lib` and `/usr/X11/lib`. None of them draws on a real screen. kdrive's own framebuffer server, Xfbdev, is in the source tree, but it is written for Linux's framebuffer device and includes `linux/fb.h` (`hw/kdrive/fbdev/fbdev.h`).

In Leopard 10.5.8, X11server's `libGL` re-exports the closed OpenGL.framework's, its `libOSMesa` links `libGL` and the closed `libXplugin`, and its `xinit` links CoreServices. Leopard 10.5.8's `/usr/X11` takes about 180 MB, with its Intel code.

### The IOKit mode, published before Leopard

Apple's X server could once run on Darwin alone. Apple's X11 project for Mac OS X 10.3.4 to 10.3.9 (`X11-0.30.4`) and 10.4 to 10.4.8 (`X11-0.40`, `X11-0.40.2`, and `X11-0.40.80` for Intel Macs) holds XDarwin's IOKit mode in `xc/programs/Xserver/hw/darwin/`, in an `iokit/` folder from `X11-0.40` on. In `X11-0.40.2`, XFree86 4.4.0, its manual page says that XDarwin can be "run from the Darwin text console in IOKit mode" (`hw/darwin/XDarwin.man`), and with `DarwinQuartzSupport` off, `hw/darwin/Imakefile` builds only `iokit` and `utils`, with no Quartz. The IOKit mode's three files, `xfIOKit.c`, `xfIOKitCursor.c` and `xfIOKitStartup.c`, under Torrey T. Lyons's MIT-style notice, use the calls listed above: they open the framebuffer's server connection, read and set its mode and depth, map its memory and cursor, and read events from IOHIDSystem's shared queue. Apple's X11 for 10.4.9 to 10.4.11 (`X11-0.46.4`), based on X.Org, and Leopard's X11server 48.7, whose server directory is `hw/xquartz`, have none. Which release holds which X11 version was checked on Apple's [release list](https://opensource.apple.com/releases/), and the folders on Apple's GitHub.

X.Org's own xorg-server 1.4.2, the release that Apple's 1.4.2-apple33 starts from, still has it, as `hw/darwin/iokit`, and its `hw/darwin/Makefile.am` builds it as a program named `XDarwin`, beside Xquartz. X.Org deleted `hw/darwin/iokit` from its main branch in November 2007 (commit `bcbaf2a0`, "Darwin: Dead code removal"), so xorg-server 1.5 and later have no IOKit mode. In 1.4.2, the part of `hw/darwin` without Quartz is about 5,300 lines: `darwin.c`, `darwinEvents.c`, `darwinKeyboard.c`, `darwinXinput.c` and the three IOKit files.

Others have run it on Darwin 9. PureDarwin's wiki records XFree86 4.7.0's IOKit mode on Darwin 9 for Intel, in a VMware virtual machine with VMware's framebuffer driver: built through MacPorts with `DarwinQuartzSupport NO` and FreeType without its Mac font support, it ran twm, xterm, xeyes, WindowMaker, fluxbox and Qt 3 programs. This study found no record of the IOKit mode on Darwin 9 for PowerPC.

## The options

### Option 1: X programs on the image, shown on another computer

X is a network protocol, so a program on the image can draw on the screen of another computer that runs an X server, such as XQuartz on a current Mac or a Linux desktop, through SSH's X11 forwarding. The image needs X's libraries and clients, `xauth`, and `X11Forwarding yes` in `/etc/sshd_config`, which OpenSSH 95.1.5's file leaves commented out (`#X11Forwarding no`).

| | |
|---|---|
| Source | Apple's 9L30: X11misc 6, X11proto 15.5, X11libs 17.6, X11apps 14.3, and X11fonts 5.1 for the fonts that clients draw themselves through Xft |
| Licence | X.Org's MIT-style licences; FreeType's own or the GPL-2.0; fontconfig's MIT-style notice |
| Rules | all five kept, with deviations of known kinds: FreeType built without `--with-old-mac-fonts` and fontconfig without the two frameworks of its `conf_flags` (`closed-dependency`); `glxgears` and `glxinfo` left out, or Mesa's software `libGL` built in place of Apple's (`closed-dependency`); build dependencies for X11proto and X11libs, which darwinbuild's 9A581 list gives none (`bug`) |
| Display and input | none on the image: the other computer's |
| What a user sees | xterm, xclock, xcalc, xman and the other X11apps programs as windows on their own desktop, each running on the PowerPC Mac |
| Work | four or five projects in the build order; three to five deviations; one more file that `make-test-image` edits; the image grows by part of Leopard's 180 MB, not measured for PowerPC alone |
| Where it is tested | fully in QEMU, whose SSH port is forwarded to the computer running it; the same on the iMac |
| Checked | the sources, the closed links of Apple's binaries, `sshd_config`; nothing built |

### Option 2: Xvfb on the image, seen through VNC

Xvfb is an X server whose screen is only memory. Alone, it serves programs that need an X server and no screen, and `xwd`, in X11apps, saves its screen to a file, which would let a test in QEMU see an X program's windows without any display driver. To see it and use it, a VNC server must send Xvfb's screen to a viewer and pass keys and the mouse back: x11vnc, built on LibVNCServer, which reads a running X server's screen, or TigerVNC's Xvnc, which is an X server of its own. No VNC server is in the 9L30 build list.

| | |
|---|---|
| Source | Xvfb from Apple's X11server 48.7, its second build; the VNC server from outside Apple |
| Licence | MIT for Xvfb; GPL-2.0 for x11vnc, LibVNCServer and TigerVNC |
| Rules | Xvfb keeps all five; the VNC server breaks rule 1, as a project outside the 9L30 build list |
| Display and input | no display or input driver: a VNC viewer's, on another computer |
| What a user sees | a whole X desktop, twm and xterm, in a VNC viewer's window: a remote desktop |
| Work | option 1, then X11server (Apple's script builds both servers and both Mesas in one run), then one or two outside projects |
| Where it is tested | fully in QEMU, with one more forwarded port; the same on the iMac |
| Checked | Xvfb in X11server's build script and in Leopard 10.5.8, its links; the VNC servers' licences on their repositories; nothing built |

### Option 3: an X server on the Mac's own screen, through IOKit

The IOKit mode is the one existing code that puts X on the Mac's own screen without WindowServer. There are three ways to get it:

- (a) A patch to X11server 48.7 whose second build also makes `XDarwin`, from the `hw/darwin` of X.Org's xorg-server 1.4.2, the release Apple's own tree starts from.
- (b) X.Org's xorg-server 1.4.2 as a project of its own, configured with `--disable-quartz`.
- (c) Apple's Tiger X11, `X11-0.40.2`, XFree86 4.4.0, as a whole: a second, older X11 with its own libraries and clients, beside Leopard's.

| | |
|---|---|
| Source | (a), (b) X.Org's xorg-server 1.4.2; (c) Apple's Mac OS X 10.4.3 to 10.4.8 release |
| Licence | (a), (b) X.Org's MIT, and Torrey T. Lyons's MIT-style notice on the IOKit files; (c) the XFree86 Licence 1.1 as well |
| Rules | (a) keeps rule 1, X11server's version unchanged, but its patch would import about 5,300 lines that neither Apple nor the project wrote, far from rule 2's smallest fix; (b) breaks rule 1; (c) bends it: rule 1 lets a project change version only when Apple's own version is not published, and Leopard's X11 is |
| Display and input | IONDRVFramebuffer through the NDRV: QEMU's and the iMac's attach (checked in QEMU, [Step 13](step-13-gpu-idle-heat.md) on the iMac), with their mode lists unknown; USB keyboard and mouse through IOHIDSystem (checked in QEMU); the key map from the kernel, IOHIDKeyboard's built-in US layout (`appleUSAFKeyMap`), or from a `.keymapping` file given with `-keymap` (X.Org 1.4.2's `hw/darwin/darwin.c`), whose open source for other layouts was not looked for |
| What a user sees | from the console, `xinit` or `startx` switches the screen to X, with twm, xterm, xclock and the other X11apps programs; or `xdm` as a graphical login; everything drawn by the processor, with no OpenGL beyond Mesa's software renderer |
| Work | option 1, then X11server, then the patch or project; besides option 1's deviations, `xinit`'s CoreServices link (`closed-dependency`); a way to start it from the console |
| Where it is tested | first in QEMU, whose NDRV, USB keyboard and mouse the image already drives; then on the iMac, for the ATI NDRV's modes and depths, the console's return when X quits, and the display's power ([Step 2](step-2-console-login.md), [D116](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d116)) |
| Checked | the IOKit mode's sources in Apple's Panther and Tiger X11 and in X.Org 1.4.2, the calls it makes against Leopard's IOGraphics, IOHIDFamily and IOKitUser, the IOKit side in QEMU; whether X.Org 1.4.2's copy builds and runs on Leopard is open |

### Option 4: new code straight on the framebuffer

If the IOKit mode cannot be brought back, the same two pieces, a screen through IOFramebuffer's server connection and input from IOHIDSystem's queue, could be written anew. One form is a kdrive server: X11server 48.7 already builds kdrive, and its Linux framebuffer server and its fake server are under 2,000 lines of C together (`hw/kdrive/fbdev`, `hw/kdrive/fake`). Another is a graphical toolkit without X, such as LVGL (MIT) or SDL, with a display and an input driver of its own; then only programs written for that toolkit run, and none of the X programs of the build list. No such toolkit is in the build list, and the kernel's text console is the only code on the image that draws on the framebuffer.

| | |
|---|---|
| Source | the project's own code, in `replacements/`, on kdrive or a toolkit |
| Licence | MIT, as the project's other replacements |
| Rules | a kdrive server of the project's own keeps all five, as an open replacement for the closed window server (rule 2's last clause); a toolkit from outside Apple breaks rule 1 |
| Display and input | the same as option 3 |
| What a user sees | with kdrive, the same as option 3; with a toolkit, only its own programs |
| Work | new code, its size not estimated; then option 1 for the X programs |
| Where it is tested | first in QEMU, then on the iMac |
| Checked | kdrive's sources in X11server 48.7; nothing written |

### Option 5: desktop software on top of X

Options 1 to 4 each give an X server or a screen for X programs. What runs on it:

- In the 9L30 build list, built from Apple's sources: twm, the window manager; xdm, the graphical login; xterm, xclock, xcalc, xman, xmh, bitmap and the other X11apps programs. Tk 8.4.7, in tcl 64, has an X11 port in its `unix/` folder beside its Aqua one; built for X11, `wish` and Python's `_tkinter` would run. Emacs (emacs 70.1) and Vim (vim 39) are configured `--without-x` by Apple's Makefiles, and both have X front ends. Each of these is a build flag and a deviation; none was tried.
- From outside Apple: WindowMaker (GPL-2.0), a window manager in the style of NeXTSTEP; GNUstep, an open implementation of the OpenStep and Cocoa interfaces, whose libraries are under the LGPL-2.1. GNUstep's [README.Darwin](https://github.com/gnustep/tools-make/blob/master/Documentation/README.Darwin), last updated in December 2007, builds it on Darwin for PowerPC with Apple's compiler from Xcode 2.5 on, in the `gnu-gnu-gnu` combination, which needs the GNU Objective-C runtime in place of Apple's objc4, libffi, which the image has (libffi 10), libtiff, which the build list lacks, and an X server.

Software from the build list keeps all five rules; software from outside breaks rule 1. On Darwin 9 for Intel, PureDarwin ran WindowMaker and fluxbox on XFree86's IOKit mode, and Xfce through TightVNC, all built with MacPorts.

### Option 6: Apple's own graphical system

WindowServer, Quartz, CoreGraphics, Xplugin, OpenGL.framework, loginwindow and the Finder are closed, and Xquartz needs them. quartz-wm, the window manager of Leopard's X11, is in none of the X11 projects: X11server holds only the script that starts it (`xinitrc.d/99-quartz-wm.sh`). Rule 5 keeps all of them off the image, so this is not an option.

Two ideas this chapter started from do not exist for this release: Leopard's X11 sources have no IOKit mode, only Panther's and Tiger's have, and the 9L30 build list has no framebuffer console or toolkit.

## How they compare

| Option | On the Mac's own screen | Rules broken | Code to write | QEMU | iMac |
|---|---|---|---|---|---|
| 1. X programs, shown elsewhere | no | none | none | all of it | all of it |
| 2. Xvfb with VNC | no, in a viewer | rule 1, by the VNC server | none | all of it | all of it |
| 3. IOKit mode | yes | (a) none, with a large patch; (b) rule 1; (c) rule 1, bent | a port of existing code | first | modes, depths, console, power |
| 4. kdrive server or toolkit | yes | none for a kdrive server of the project's own; rule 1 for a toolkit | a new server or drivers | first | modes, depths, console, power |
| 5. Desktop software | on 1 to 4 | none from the build list; rule 1 from outside | build flags | with 1 to 4 | with 1 to 4 |
| 6. Apple's own | yes | rule 5 | none | no | no |

## Recommendation

1. Option 1 first, with Xvfb: build X11misc, X11proto, X11libs, X11apps and X11fonts, and X11server's second build for Xvfb, all from Apple's 9L30 sources. Every rule holds, and the deviations are of kinds the project already has. It gives X programs from the image on any desktop over SSH, a way for QEMU tests to see X windows through Xvfb and `xwd`, and the libraries every other option needs.
2. Then option 3 (a): XDarwin added to X11server 48.7 from X.Org's 1.4.2, the release Apple's own tree comes from. It is the only route to the Mac's own screen with code that exists, and it uses only interfaces that Leopard's open sources publish. It needs the owner's decision on its form (below). If it does not run on Leopard's IOGraphics, a kdrive server of the project's own (option 4) keeps every rule.
3. On the desktop: twm, xterm and xdm from the build list; Tk, Emacs and Vim with X as build-flag deviations. WindowMaker, GNUstep and VNC only after the owner's decision on projects from outside Apple.

## Decisions for the owner

1. Whether a graphical interface becomes a step in README.md, such as "On the iMac G5, an X server shows twm and xterm on the screen, with a USB keyboard and mouse", or stays an idea in [What's next](whats-next.md).
2. For option 3, which form, if any: (a) a patch to X11server 48.7 that imports about 5,300 lines of X.Org's `hw/darwin`, recorded in DEVIATIONS.md, though rule 2 asks for the smallest patch, or the same code kept in `replacements/` as an open replacement for the closed window server, though the project did not write it; (b) X.Org's xorg-server 1.4.2 as a project outside the build list, which rule 1 forbids today; or (c) Apple's Tiger X11 beside Leopard's.
3. Whether rule 1 admits open projects from outside Apple's release, which options 2, 4 with a toolkit, and 5 need for a VNC server, WindowMaker, or GNUstep with its runtime and libtiff; and if so, how they are listed apart from the 9L30 projects.
4. Whether X11 goes onto every image, or onto a larger image of its own: Leopard's `/usr/X11` takes about 180 MB.
5. For OpenGL: whether to leave it out, or to build Mesa's software `libGL` in place of Apple's, which re-exports the closed OpenGL.framework.

## Open questions

- Which mode the iMac's firmware leaves its screen in, which resolutions and depths the iMac's ATI NDRV and QEMU's NDRV offer, and whether `IOFBSetDisplayModeAndDepth` can change them without the closed ATI driver.
- Whether X.Org 1.4.2's `hw/darwin` builds against Leopard's headers and runs with IOGraphics 305.14 and IOHIDFamily 258.14.
- How the console, IOHIDSystem and IODisplayWrangler behave while a server owns the framebuffer and the input queue, and after it quits.
- Whether X11server's second build can run without its first, and whether X11proto and X11libs build with darwinbuild once given their dependencies.
- How long the X11 projects take to build, and how much the image grows for PowerPC alone.
- Whether a GNUstep of 2008 or 2009, with the GNU runtime, builds with the release's own compilers.

## Where to look

- [manifest/9L30.tsv](https://github.com/nyg/PowerDarwin/blob/master/manifest/9L30.tsv): the X11 projects and their versions; [manifest/plists/9A581.plist](https://github.com/nyg/PowerDarwin/blob/master/manifest/plists/9A581.plist): their build dependencies.
- In X11server-48.7: `build`, `X11server.plist`, `kdrive/xorg-server-1.4.2-apple33/hw/kdrive/fbdev/fbdev.h`, `Xquartz/xorg-server-1.4.2-apple33/hw/xquartz/xpr/`, `xinitrc.d/99-quartz-wm.sh`. In X11proto-15.5: `build`, `freetype/conf_flags`, `fontconfig/conf_flags`.
- In X11-0.40.2: `xc/programs/Xserver/hw/darwin/`, with `Imakefile`, `XDarwin.man` and `iokit/`. In X.Org's xorg-server 1.4.2: `hw/darwin/Makefile.am` and `hw/darwin/iokit/`.
- In IOGraphics-305.14: `IOGraphicsFamily/IOFramebuffer.cpp` (`newUserClient`, `open`) and `IOGraphicsFamily/IOFramebufferUserClient.cpp`. In IOHIDFamily-258.14: `IOHIDSystem/IOHIDSystem.cpp`, `IOHIDSystem/IOHIDUserClient.cpp`, `IOHIDFamily/IOHIDKeyboard.cpp`. In IOKitUser-388.53.30: `graphics.subproj/IOGraphicsLib.c`, `hidsystem.subproj/IOHIDLib.c`.
- In OpenSSH-95.1.5: `openssh/sshd_config`.
- [RESOURCES.md](https://github.com/nyg/PowerDarwin/blob/master/RESOURCES.md): Apple's X11 releases, X.Org's xorg-server 1.4.2, PureDarwin's X11 pages, GNUstep's README.Darwin, the VNC servers.
- [What stays closed](../synthesis/what-stays-closed.md), [What's next](whats-next.md).
