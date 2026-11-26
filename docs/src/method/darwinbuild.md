# darwinbuild and the build machine

## What you will learn

- What darwinbuild does to build one project: from the build list, through a BuildRoot filled with the Roots the project depends on, to a new Root of its own.
- How darwinxref's database knows each project's version, dependencies and build settings, and how the options `-headers` and `-target` and another version change a build.
- Why the build machine is a PowerPC Mac running Leopard and Xcode 3.1.4, and what darwinbuild does to its copy of Xcode.
- What a build records of the machine that runs it, and how `run-darwinbuild` keeps the machine's names and time zone out of the binaries.
- How the project's scripts stage sources and Roots on the build machine, and how `prepare-buildroot` turns every change to the build environment into a step that can be checked and repeated.
- How `build-image` runs every build in the order of `manifest/build-order`, and how that order was found.

## Three machines

The work is split between two Macs and tested on a third. A current Mac holds the repository, downloads the sources and Roots ([the previous chapter](sources-and-roots.md)), and runs QEMU. The build machine is a PowerPC Mac running Leopard 10.5.8 with Xcode 3.1.4 and darwinbuild, which builds every project in a build folder, `~/9L30`, and makes the test image in `/private/var/tmp`. The project's own runs Sorbet Leopard, a community update of Leopard 10.5.8 whose kernel and drivers carry the versions of 10.5.8's build list. The scripts that link the two run on the current Mac and reach the build machine over SSH; each takes the build machine's SSH host name as its first argument. So the current Mac needs an SSH login on the build machine, and, since `prepare-buildroot` runs `sudo` there over SSH without a terminal, `sudo` on the build machine must not ask for a password.

The image is tested in QEMU on the current Mac, and on a real PowerPC Mac, the test Mac, which keeps Leopard on one partition of its disk and starts the image from another. The project's test Mac is an iMac G5 (`PowerMac12,1`), the reference machine on which every step is proved ([How testing is done](testing.md)). Any PowerPC Mac with Leopard 10.5.8 and Xcode 3.1.4 can be the build machine.

```mermaid
flowchart LR
    subgraph mac [current Mac]
        F["fetch-sources<br/>fetch-roots"]
        Q["boot-qemu<br/>QEMU mac99"]
    end
    subgraph bm [build machine, Leopard]
        D[darwinbuild]
        M[make-test-image]
    end
    I["test Mac<br/>Darwin9 partition"]
    F -->|"stage-sources<br/>stage-roots<br/>prepare-buildroot"| D
    D -->|Roots| M
    M -->|"image, copied back"| Q
    M -->|"image, restored<br/>with asr"| I
```

### Why a PowerPC Mac with Leopard

darwinbuild runs only on Mac OS X. Many projects are Xcode projects, built with `xcodebuild`, which is closed, and darwinbuild takes it from the Xcode installed on the host.

The build environment shares the host's kernel and daemons. The notes at the top of darwinbuild's `darwinbuild.in` warn that "items in the changed root still need to be compatible with the system daemons that are running". The host must therefore run the release being built: Leopard.

The programs a build makes run on the build machine as soon as they are installed into the build environment, since later builds use them: our `/bin/sh` runs the build scripts of the projects built after bash, and Apple's `tar` runs on our libSystem. So the build machine must run PowerPC binaries natively.

Rule 4 asks for the release's own toolchain: `gcc` 5465, `gcc_42` 5531, `cctools` 667.3 and `ld64` 77.1, the compilers, assembler and linkers of 10.5.8. They are projects of the build list like any other, and darwinbuild installs them into each build environment from their Roots: Apple's until Step 10, ours since, after a check that ours and Apple's give the same output for the same input ([Step 10](../steps/step-10-working-unix.md)). Xcode 3.1.4 is the last Xcode for Leopard, not the one Apple built 9L30 with ([D23](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d23) shows one difference); the build uses only its `xcodebuild` and closed helpers, which darwinbuild points at the toolchain's own compilers (below).

The build machine does not download. Leopard's TLS is too old for GitHub and archive.org, so the current Mac fetches and checks everything, and copies it across. To build PowerDarwin rather than work on it, no SSH is needed: [scripts/download-kit](https://github.com/nyg/PowerDarwin/blob/master/scripts/download-kit), on any current computer, packs the repo, the sources, the Roots and D49's parser into one file, the kit, and on the build machine [scripts/build-image](https://github.com/nyg/PowerDarwin/blob/master/scripts/build-image) stages them and runs every build there ([The build order](#the-build-order)).

## What darwinbuild does

darwinbuild is a set of shell scripts and a small database tool, written by Apple and kept on GitHub as [macosforge/darwinbuild](https://github.com/macosforge/darwinbuild), now archived. The project uses its `Darwin9` branch at commit `4848b7f`, unchanged. It builds one project at a time, as root: `sudo /usr/local/bin/darwinbuild <project>`, with the full path because Leopard's `sudo` leaves `/usr/local/bin` out of its search path. Every build is run in `~/9L30`, since darwinbuild takes its working directory as the build folder.

### The build folder

`darwinbuild -init 9L30`, run once in `~/9L30`, which then holds only `.build/` and the nine build lists copied into it, makes the build folder:

| Path | Holds |
|---|---|
| `.build/` | the build lists and darwinxref's database, `xref.db` |
| `Sources/` | the sources, as tarballs or unpacked folders |
| `Roots/` | the Roots darwinbuild makes, and `Roots/.DownloadCache/`, the downloaded ones |
| `Headers/` | the headers Roots darwinbuild makes |
| `Symbols/` | each build's unstripped binaries |
| `Logs/` | one log per build |
| `BuildRoot` | a link to the mounted build environment |

The build environment, the BuildRoot, is a sparse disk image, `.build/buildroot.sparsebundle`: an HFSX volume, the variant of HFS+ that tells upper-case from lower-case names, that may grow to 1 TB but takes only the room its files use. darwinbuild mounts it as `/Volumes/BuildRoot_9L30_` followed by the date and time it was made.

`-init` then loads the build list `9L30` into the database, reads its `inherits`, loads `9J61`, and so on down to `9A581`. It would download each list from `svn.macosforge.org` if the file were not already in `.build/`; since that host is gone, the lists are copied there first ([D2](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d2)), and darwinbuild finds them.

### darwinxref

darwinxref is darwinbuild's database tool, a C program over an SQLite file. Once the build lists are loaded, it answers every question darwinbuild asks about a project: `darwinxref version xnu` prints `xnu-1228.15.4`, `darwinxref dependencies -build xnu` the projects xnu needs to build, `darwinxref environment xnu` the settings to build it with. It also records which files each Root installed. When a build list changes, `darwinxref loadIndex` reloads it.

`darwinxref environment` also builds the compiler flags: `RC_CFLAGS` is `RC_NONARCH_CFLAGS` followed by one `-arch` for each processor of `RC_ARCHS`, and darwinxref rebuilds it so even when a build list sets `RC_CFLAGS` itself. Apple's build put the flags in the other order, which a diff in Ruby's build expects ([D105](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d105)).

A project's entry in a build list names its dependencies by kind. This is part of xnu's, from `9A581.plist`:

```text
xnu = {
	dependencies = {
		build = (
			adv_cmds,
			awk,
			bash,
			bootstrap_cmds,
			cctools,
			...
		);
		...
	};
	version = 1228;
};
```

`build` names the tools the build runs: the shell, `make`, the compilers. `header` names the projects whose headers it includes. `lib` and `staticlib` name the dynamic and static libraries it links. `run` names what it needs at run time. darwinbuild installs the full Root of every `build`, `run`, `lib` and `staticlib` dependency, and of the `lib` and `run` dependencies of those in turn, and only the headers Root of a `header` dependency.

Apple's lists are not always complete, and a missing dependency is one of the commonest failures of a first build. darwinbuild then builds without the missing project's files, or with Apple's older copy left in the BuildRoot by an earlier build, and the compiler stops on a header that is not there, such as `IOKit/storage/IOBDMedia.h` ([D18](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d18)). The fix is either to put the files into the BuildRoot or to add the dependency to `9A581.plist`; [The rules](rules.md) says which comes first.

### One build, step by step

`darwinbuild <project>` then takes these steps:

1. It asks darwinxref for the project's version, the project whose source an alias uses, and the target and configuration the build list names, if any.
2. It looks in `Sources/` for the source, an unpacked folder `<project>-<version>` or a tarball, and copies it into the BuildRoot, under `SourceCache/<project>/`. It would download the tarball from the build list's source sites if it were not there.
3. It picks the build tool: `xcodebuild` if the source has an Xcode or Project Builder project, `make` otherwise.
4. It installs Roots into the BuildRoot: the `files` project, which lays out the folders of a Mac OS X volume, `bash` and what bash needs, and then every dependency, followed by the headers of the `header` dependencies. For an Xcode project it also installs its copy of the host's Xcode (below).
5. It writes a build script, `build-<project>-<version>~N.sh`, which clears every environment variable, sets `PATH` and the build list's settings, and runs `make install` or `xcodebuild install` with four folders: `SRCROOT`, the source; `OBJROOT`, the object files; `SYMROOT`, the unstripped products; and `DSTROOT`, an empty folder the project installs into.
6. It mounts `devfs` inside the BuildRoot, so that `/dev/null` and the terminals exist there, and runs the script with `chroot`, so that the build sees the BuildRoot as `/`. Everything the build prints goes to `Logs/<project>/<project>-<version>.log~N`.
7. If the build succeeds and `DSTROOT` is not empty, it registers the installed files with darwinxref and copies `DSTROOT` to `Roots/<project>/<project>-<version>.root~N` and `SYMROOT` to `Symbols/`.

```mermaid
flowchart TD
    A["darwinbuild project"] --> B["darwinxref: version,<br/>dependencies, settings"]
    B --> C["copy the source from Sources/<br/>into the BuildRoot"]
    C --> D["install the dependencies' Roots:<br/>ours if built, else Apple's"]
    D --> E["install the header<br/>dependencies' headers"]
    E --> F{"Xcode project?"}
    F -->|yes| G["install darwinbuild's<br/>copy of Xcode"]
    F -->|no| H
    G --> H["chroot into the BuildRoot:<br/>make or xcodebuild install"]
    H --> I{"succeeded, and<br/>DSTROOT not empty?"}
    I -->|yes| J["Roots/project/project-version.root~N"]
    I -->|no| K["only the log"]
```

### Ours before Apple's

When darwinbuild installs a dependency, it first looks in `Roots/<project>/` for a Root it has built itself of the version the list names. It takes the one with the highest number, unless the same one is already installed. Only when there is none does it unpack Apple's Root from `Roots/.DownloadCache/`, where [scripts/stage-roots](https://github.com/nyg/PowerDarwin/blob/master/scripts/stage-roots) put it, or download it from the build list's `binary_sites`, the dead `src.macosforge.org`. If no build in the inheritance chain has the Root, it stops with `ERROR: could not find root`, which [D3](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d3) avoids by staging an older Root.

So the build environment turns from Apple's into ours project by project. Once our libSystem is built, every later build links against it and runs on it. This is also how our files are tested before any image exists: a bug in our `/bin/sh` first showed as ncurses failing to build ([D20](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d20)).

### The `~N` numbers

`N` counts the attempts at one project and version: it is one more than the highest number already in `Logs/`, `Symbols/`, `Headers/` or `Roots/` for that project and version. A failed build leaves a log and no Root, so the Roots of one project may be numbered `root~1`, `root~3`, `root~6`. The book names Roots this way when it matters which build was tested, such as CF `root~5` and `root~6` in the Step 3 tests or OpenSMU `root~7` in Step 8. The project's scripts always take the latest, with the same shell function:

```sh
latest() { ls -d $roots/$1/$1-*~* | sort -t~ -k2 -n | tail -1; }
```

### Options

- `-headers` runs the project's `installhdrs` step instead of `install`, and keeps the result in `Headers/<project>/` as `<project>-<version>.hdrs~N`. Projects whose headers are needed before they can be built, or before their dependants can, are built this way first: launchd's libraries ([D5](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d5)), IOHIDFamily and IOUSBFamily, Security's 32 parts ([D48](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d48)).
- `-target=<name>` builds one target of the Makefile or Xcode project instead of the default one. It builds a part of a project whose other parts need something closed: launchd without SystemStarter ([D21](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d21)), mDNSResponder's library without its daemon ([D11](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d11)), Tcl without Tk ([D71](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d71)).
- A version after the project's name builds that version instead of the list's. `darwinbuild IOATABlockStorage 130.3.1` builds Panther's driver in place of Leopard's unpublished one ([D40](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d40)).
- A build list entry may also name a `configuration`, which darwinbuild passes to `xcodebuild -configuration`; [D51](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d51) builds Security in Apple's own `Darwin` configuration this way.

## Fat binaries

Apple's `9A581.plist` sets `RC_ARCHS = "ppc i386"` for every build, so most projects build each program twice, for PowerPC and for Intel, and join the two into one fat binary: a Mach-O file with one slice per processor, which `lipo -info` lists. Some entries name four: libSystem, CoreFoundation and Security come out of Apple's build for `ppc7400 ppc64 i386 x86_64`, 32- and 64-bit on each processor (`ppc7400` is the G4's 32-bit PowerPC). This project builds for PowerPC only ([D115](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d115)): its copy of `9A581.plist` sets `ppc` for every build, and `ppc ppc64` where Apple's names four, so `ppc64` stays wherever Leopard has it. The same deviation sets the builds that name their architectures themselves instead of reading `RC_ARCHS`: three scripts of this project, CF's `BuildCFLite` and ruby's Makefile. Where a project links one that has `ppc64`, its entry can add `ppc64` too ([D74](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d74)), as for SystemStubs, whose `libSystemStubs.a` gcc links into every program ([D110](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d110)). gcc and gcc_42 are built for `ppc` alone, without the Intel cross compilers that `ppc i386` adds ([D100](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d100)). A PowerPC Mac uses only its own slice of each file, and the comparisons with Apple's binaries read the `ppc` slice of both.

## What a build records

A few builds write down where and when they ran. Perl's `Configure` keeps the host name and the user who ran it, from `hostname` and `logname`, in `Config_heavy.pl` and `CORE/config.h`; Vim's `Makefile` writes "Compiled by" from `logname` and `hostname`; Samba's build writes "Built on" from `uname -a` into `smbd` and `vfstest`; and lsof's `Makefile` puts `uname -n` and `uname -a` into what `lsof -v` prints. These and others, the kernel, OpenSSL and libSystem among them, also record the date of their build. Leopard's Perl says `_www@b04.apple.com`: Apple's build machine and the account that ran its builds.

darwinbuild clears every environment variable and sets `LOGNAME` and `USER` to `root` (above), but neither reaches what these builds read. `logname` asks the kernel for the session's login name, which `sudo` leaves as that of the user logged in to the build machine; the host name is the build machine's own, often with its network's domain; and dates are in the zone of the BuildRoot's `/etc/localtime`, which system_cmds' Root links to `US/Pacific`.

So every build of an image runs in UTC, under neutral names ([D125](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d125)). The build list sets `TZ = UTC` for every project, as it sets `RC_ARCHS`, and [scripts/run-darwinbuild](https://github.com/nyg/PowerDarwin/blob/master/scripts/run-darwinbuild) `<build folder> <darwinbuild argument>...` runs darwinbuild with the build machine's host name set to `powerdarwin.invalid`, and in a session of its own whose login name is `root`. No host can be named in `.invalid`, a top-level domain set aside for names that must not resolve (RFC 2606). The script sets the host name back when darwinbuild ends, and fails the build if something else changed the host name meanwhile, as configd may when the network changes; the name it sets lasts only until the next start in any case. It forwards an interrupt to darwinbuild's session, which no terminal reaches. `build-image` runs every build through it.

On 26 November 2026, lsof and Vim were built again so, in a build folder whose other Roots were built before D125. Before, `lsof -v` said it was constructed by `root` on the build machine, at a time in `PST`, and Vim that it was compiled by the build machine's user on that machine; after, `lsof -v` said `constructed: Thu Nov 26 21:12:01 UTC 2026` and `constructed by and on: root@powerdarwin.invalid`, with `Darwin powerdarwin.invalid` in its system information, and `vim --version` said `Compiled by root@powerdarwin.invalid`, at a time that was UTC. The build machine had its own host name again after each build. `make-release` then still found the build machine's host name in Perl's two files, `smbd` and `vfstest`, whose Roots came from before; Perl, Samba, the kernel, OpenSSL and libSystem are to show the same in a clean build.

## darwinbuild's copy of Xcode

The BuildRoot's `/System/Library/Frameworks` holds the frameworks being built: our CoreFoundation, our IOKit. Xcode itself needs Apple's closed frameworks, at the versions installed on the host. Both cannot be at the same path, so darwinbuild's `installXcode31` copies Xcode to a place of its own.

It finds every file that `xcodebuild` loads, by following `otool -L` from library to library, packs them with `cpio`, and passes the stream through `sed` before it unpacks it into the BuildRoot. The `sed` rewrites, among others, three paths wherever they appear, in file names and inside the binaries: `/System` becomes `/XCD/SY`, `/Developer` becomes `/XCD/loper`, and `/usr/lib` becomes `/XCD/lib`. Each new path has exactly as many letters as the old one, so that a path inside a compiled program keeps its length and the program still runs. That is the origin of the odd names: Xcode's private frameworks are in `/XCD/SY/Library/Frameworks`, and its tools in `/XCD/loper/usr/bin`. darwinbuild then links the compilers and tools there, `gcc`, `mig`, `strip` and the others, to `/usr/bin` of the BuildRoot, where the Roots of the release's toolchain installed them, and sets `DEVELOPER_DIR=/XCD/loper`.

The copy leads to four deviations: two from settings Xcode derives from the rewritten paths, two from what the copy leaves out:

- Xcode's default for `SYSTEM_LIBRARY_DIR` became `/XCD/SY/Library`, so the first build of IOKitUser installed its framework there, under the install name `/XCD/SY/Library/Frameworks/IOKit.framework/Versions/A/IOKit`. `9A581.plist` now sets `SYSTEM_LIBRARY_DIR = /System/Library` for every build ([D25](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d25)).
- `APPLE_INTERNAL_DEVELOPER_DIR` became `/AppleInternal/XCD/loper`, where nothing is installed; `9A581.plist` sets it back to `/AppleInternal/Developer` ([D78](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d78)).
- `nmedit` is not among the tools darwinbuild links, and securityd's build runs `/XCD/loper/usr/bin/nmedit` ([D82](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d82)).
- Xcode's `copyplist` is a Ruby script, and the BuildRoot has no Ruby ([D24](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d24)).

darwinbuild puts two more tools into the BuildRoot. When there is no `ditto`, it copies in a shell script of its own that stands in for Apple's closed one, and lacks one of its behaviours ([D99](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d99), below). It also copies the host's `dsymutil`, which writes the separate debug-symbol files, into the BuildRoot, where it crashes on every binary ([D7](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d7)). [env/dsymutil](https://github.com/nyg/PowerDarwin/blob/master/env/dsymutil) takes its place: it runs the real one and, when that fails, leaves a copy of the binary where the debug file would be, so that the build goes on.

## Staging

### Sources

[scripts/stage-sources](https://github.com/nyg/PowerDarwin/blob/master/scripts/stage-sources) `<host> <project>[=<version>]...` puts sources on the build machine, and `--here=<build folder>` in place of the host does the same on the build machine itself:

1. It finds the project's version in `manifest/9L30.tsv`, or takes the one given as `<project>=<version>`.
2. It unpacks the tarball from `sources/` into a temporary folder, renaming its one top folder, `<project>-<project>-<version>` in a tarball from GitHub, to `<project>-<version>` ([D1](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d1)). It renames with `mv`, since the `tar` of Leopard, GNU tar 1.15.1, cannot rename as it unpacks. A project of [replacements/](https://github.com/nyg/PowerDarwin/blob/master/replacements/), such as OpenSMU, has no tarball and is copied from there. Such a project is not in `manifest/9L30.tsv`, so its version is always given, as `OpenSMU=1.0` ([D81](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d81)).
3. It applies each patch in `patches/<project>-<version>/`, in order, with `patch -p1`.
4. It touches the files named in that folder's `touch` file, so that `make` rebuilds what is generated from them ([D20](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d20)).
5. It sends the result over SSH as a `tar` stream into `~/9L30/Sources/`, without the extended attributes a current Mac adds to files, or, with `--here`, moves it into that build folder's `Sources/`, replacing an older copy.

darwinbuild then finds an unpacked folder in `Sources/` and never downloads the source, and the patches are applied once, by the repo, rather than by darwinbuild's own patch mechanism.

### Roots

[scripts/stage-roots](https://github.com/nyg/PowerDarwin/blob/master/scripts/stage-roots) `<host> [build folder on host]` copies the Roots and headers Roots chosen in `manifest/9L30-roots-used.tsv`, at most one of each per project, into `~/9L30/Roots/.DownloadCache/` on the build machine, or into the build folder given, and reports any that is missing from `roots/`. `stage-roots --here=<build folder>` copies them on the build machine itself.

## Preparing the BuildRoot

Some fixes are files that darwinbuild does not put into the BuildRoot: a header no Root carries, a stub library, a tool a build calls without listing it. At first such files were copied in by hand, and a Root installed later could undo that: once our CF Root was built, darwinbuild installed it over the `CFBase.h` copied in from the SDK, and kext_tools stopped building again, so that fix became a patch instead ([D16](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d16)). [scripts/prepare-buildroot](https://github.com/nyg/PowerDarwin/blob/master/scripts/prepare-buildroot), added in commit `3dbd3fd`, makes each of the others a step that can be checked and repeated:

```text
prepare-buildroot <ssh host> [--check] [--folder=<build folder on host>] [step...]
prepare-buildroot --here=<build folder> [--check] [step...]
```

Each step is named after its deviation, such as `D6` or `D82`, and `plist` copies `manifest/plists/9A581.plist` into `.build/` and reloads it with `darwinxref loadIndex`; that one step carries every row that changes the build list, such as [D100](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d100)'s and [D110](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d110)'s architectures. With no step named, it runs them all, in this order:

```text
plist D4 D5 D6 D7 D8 D9 D10 D13 D14 D15 D18 D19 D24 D30 D36 D37 D40 D49 D59 D60 D64 D67 D70 D73 D82 D89 D94 D97 D98 D99
```

The script needs the BuildRoot mounted. `-init` only creates the disk image, and darwinbuild attaches it at its first build, so `prepare-buildroot` can run only after one darwinbuild run, and, once a restart of the build machine has detached the image, only after the next one; otherwise it stops with `no build environment at …/BuildRoot: run darwinbuild once to make and mount it`.

On the current Mac it gathers what the steps need: `env/`, `replacements/`, the build list, and three inputs it makes there from files only the current Mac has: [D37](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d37)'s header, taken from Panther's SCSI family tarball; [D49](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d49)'s parser, generated with ANTLR, which needs Java 6 or later; and [D73](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d73)'s four Carbon headers. It copies all of it into a temporary folder on the build machine and runs itself there with `sudo`. With `--here`, on the build machine, it gathers the same into a temporary folder there: D37's and D73's files from the tarballs in `sources/`, and D49's parser from `inputs/antlr2`, which `download-kit` made with Java on the current computer, since Leopard's Java on PowerPC is version 5 and ANTLR's `antlr.jar` needs version 6.

There, each step compares what it would install with what is in place, file by file with `cmp`, and prints one line per file or folder, after the step's name:

- `in place` when the file or folder is already right, and the step does nothing;
- `installed`, `copied`, `unpacked`, `linked`, `made empty` (D70's order file) or `built` when it has just put it there, and `copied and loaded` for the `plist` step;
- with `--check`, without changing anything, `missing` or `differs` for a file, and `<n> of <m> files differ or are missing` for a folder or a Root;
- `waiting` when its input does not exist yet, because a project must first be built or staged, such as launchd's headers for D5; the script is simply run again after that build.

Three steps of Step 10 put in stand-ins for tools that the BuildRoot lacks or that differ from Apple's: [env/dtrace](https://github.com/nyg/PowerDarwin/blob/master/env/dtrace), which only writes headers whose DTrace probes do nothing ([D97](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d97)); [env/sw_vers](https://github.com/nyg/PowerDarwin/blob/master/env/sw_vers), which prints 10.5.8 and `9L30` for Perl's build ([D98](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d98)); and [env/ditto](https://github.com/nyg/PowerDarwin/blob/master/env/ditto), darwinbuild's `ditto` script with the one change that Python's build needs ([D99](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d99)). Three steps build their files inside the BuildRoot, with its own compiler: the sandbox and quarantine stub libraries of [D13](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d13) and [D15](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d15), and D49's `libantlr2c++.a`. A step that is in place does nothing, so the script can be run again at any time: after the build that unblocks a waiting step, or after a build whose Roots may have overwritten a step's file.

```mermaid
flowchart LR
    BR[BuildRoot] --> UB["usr/bin<br/>arch D6, dsymutil D7,<br/>ruby D24, java D49, python D64,<br/>sw_vers D98, ditto D99"]
    BR --> US["usr/sbin<br/>dtrace D97"]
    BR --> UI["usr/include<br/>Carbon headers D73"]
    BR --> ULI["usr/local/include<br/>D8, D59, D94"]
    BR --> ULL["usr/local/lib<br/>stubs D13, D15, ANTLR D49"]
    BR --> KF["Kernel.framework headers<br/>D30, D36, D37, D40, D60"]
    BR --> FW["other frameworks<br/>D24, D60, D64, D67, D89"]
    BR --> XCD["XCD/loper/usr/bin/nmedit<br/>D82"]
    BR --> AI["AppleInternal/OrderFiles<br/>D70"]
    BR --> R["Roots and headers Roots<br/>D4, D9, D10, D14, D18, D19"]
    BF["build folder"] --> H["Headers/launchd<br/>D5"]
    BF --> P[".build/9A581.plist<br/>plist"]
```

## The build order

[manifest/build-order](https://github.com/nyg/PowerDarwin/blob/master/manifest/build-order) lists every build of an image, in order, 283 steps, one per line:

- `build <arguments>`: one darwinbuild run, with darwinbuild's own arguments, such as `build -target=embedded launchd` ([D21](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d21)) or `build IOATABlockStorage 130.3.1` ([D40](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d40)).
- `build -loadonly <project>`: a darwinbuild run that only installs the project's build dependencies into the BuildRoot, and builds nothing (below).
- `prepare <step>...`: steps of `prepare-buildroot`, at the place where the builds first need them, such as `prepare D82` just before securityd.
- `stage <project> <version>`: a source to stage that no build names, CarbonHeaders 18.1, from which `make-test-image` installs headers.

The order is the one the builds of 19 to 22 November 2026 found. darwinbuild keeps a log of every run, with its date, its arguments, the Roots installed in the BuildRoot and its exit status, 387 in all; the order keeps the runs that succeeded, in the order they ran, and puts each `prepare` step where it was first applied. Most projects failed at first and were built again after a fix; with every fix now in the repo from the start, each needs only its successful build. A few changes were then needed:

- A project built again only because a fix came later is built once. CF, built six times, is built once, before IOKitUser, launchd and kext_tools, which need its `MacTypes.h` ([D16](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d16)), and before autozone and ICU, which list it among their build dependencies. It links Apple's archived ICU, objc4 and autozone Roots, whose libraries have the same install names, versions and exported symbols as ours on `ppc` and `ppc64`. Libsystem, built seven times, is built once, after all its parts, with D46's Libinfo and D65's `libdns_sd.a`; SystemStubs once, with [D110](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d110)'s architectures from the start; launchd twice, before its first users and after securityd, as the image's was.
- Kerberos is built after LDAPFramework. [D74](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d74) builds both for four architectures, and a four-way Kerberos needs a four-way LDAP library; the first builds of both were for two architectures, before D74. OpenSSH, which links Kerberos, follows it.
- OpenSSL is built once, after SystemStubs, as the image's was; the builds before it link Apple's OpenSSL Root of the same version, which [D4](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d4) puts into the BuildRoot.
- `darwinbuild -headers securityd` is left out. It ran before [D48](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d48) gave securityd its build dependencies, which darwinbuild installs even for `-headers`, and one of them, libsecurity_agent, has no archived Root and is built later. No project includes securityd's headers.
- darwinbuild installs a project's build dependencies before every run, before it knows whether the build will succeed, and leaves them in the BuildRoot. The first builds relied on Roots that such runs had installed, failed builds and `-headers` runs among them: Libnotify includes `servers/bootstrap.h`, which only the full Root of launchd_libs holds, and found it in Apple's launchd_libs Root, installed by the dependencies of `darwinbuild -headers launchd`; and D6's `arch` survived because Apple's system_cmds Root, which would replace it, came in the same way before D6. Each of darwinbuild's logs lists the Roots installed in the BuildRoot when the run began, so the runs that first installed Roots and are not in the order are known. The order replays each of them, at its place, as `darwinbuild -loadonly <project>`, which installs the same dependencies and stops: 22 such lines.
- Two things the first builds put into the BuildRoot by hand had no row: AppleDisplays' Root, for a header IOKitUser copies, and a `CFBase.h` that includes `MacTypes.h`, first the SDK's, then our CF's, for IOKitUser's `kNilOptions`. darwinbuild installs neither for IOKitUser, whose entry in the build list names neither project; [D114](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d114) adds both to it.
- The everyday tools built after Step 10, gnuzip, awk, gnutar, file, lsof, top and curl, come last, after DarwinTools: no earlier build needs them, so the steps before them run as before. gnuzip comes first, as gnutar, file and lsof list it among their build dependencies. top's entry does not name CF, IOKitUser or ncurses, whose libraries it links, and curl's names no dependency at all, though it links OpenSSL, zlib, Kerberos and libresolv: both build against what the earlier builds left in the BuildRoot, which at the end of the order holds all of them.

[scripts/build-image](https://github.com/nyg/PowerDarwin/blob/master/scripts/build-image), run as root on the build machine from an unpacked kit, then does everything:

1. It checks the machine: a PowerPC Mac, Leopard 10.5, Xcode 3.1.4 and 40 GB free.
2. It checks the SHA-256 of every source and Root in the kit against `manifest/`, with Python 2.5's `hashlib`, since Leopard has no `shasum`.
3. It installs darwinbuild from its source in the kit, unless `/usr/local/bin/darwinbuild` is there, and makes the build folder with `darwinbuild -init 9L30`.
4. It stages every source the order names, with `stage-sources --here`, under the name of the project whose source it is (Libc for Libc_debug, configd for configd_plugins), and every Root, with `stage-roots --here`.
5. It runs each step of the order, each build through `run-darwinbuild` (above), writing its progress into `.build/build-image.progress`. A failed build stops it, with the first errors of darwinbuild's log; so does a `prepare` step that says `waiting`, which here means a step placed before what it needs. Run again, it starts at the step that stopped it.
6. It runs `make-test-image`, then `check-image`, which must list only the files `make-test-image` writes ([Making the image](making-the-image.md)).

On 23 November 2026 `build-image` ran on the iMac G5 from a kit made by `download-kit`, in a build folder it made empty. It stopped twice: at IOKitUser, which needed AppleDisplays and CF ([D114](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d114)), and at `darwinbuild -headers securityd`, which could not find a Root for libsecurity_agent and is now left out of the order (above). Each time, once the fix was in the kit, it was run again and went on from the step that had stopped it. The builds took 842 minutes, and the whole run about 14 and a half hours: from 03:04 to 19:57, less the two stops. The build folder then held 20 GB, and the image 1.6 GB. `check-image` found 28,542 files on it, all from our builds except 23 files of the table in [Making the image](making-the-image.md): every file but `/etc/master.passwd` and DirectoryService's `root.plist`, which `make-test-image` writes only if the build folder holds `root-password.hash`. The order has not yet been run from an empty folder to the end without a stop.

## Andrew-R's patch

In July 2026, Andrew-R described on MacRumors his own darwinbuild builds of Darwin 9 for PowerPC, 100 projects, with a patch to darwinbuild's `Darwin9` branch ([reference/andrew-r/](https://github.com/nyg/PowerDarwin/blob/master/reference/andrew-r/README.md)). His patch points the build lists, the sources and the Roots at a new host, macos-powerpc.org, in place of the dead macosforge sites, and changes the link darwinbuild makes for an alias project's source.

This project does without the patch, and runs darwinbuild unchanged, in the way his thread first described: the build lists are copied into `.build/` before `-init`, the sources are staged into `Sources/`, and the Roots, downloaded from the Wayback Machine, into `Roots/.DownloadCache/`, so darwinbuild never needs a site at all. On 18 November 2026, macos-powerpc.org's `Roots/` folder was empty. His thread also reported first three problems this project met: the `arch` script of the build environment ([D6](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d6)), which he fixed the same way, the crash of `dsymutil` ([D7](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d7)), and the header `IOBDMedia.h` that diskdev_cmds lacks ([D18](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d18)), which he copied from the SDK. The patch file carries no licence, so the repo keeps only its description and its SHA-256.

## Where to look

- [README.md](https://github.com/nyg/PowerDarwin/blob/master/README.md), "Build it" and "Layout".
- Scripts: [stage-sources](https://github.com/nyg/PowerDarwin/blob/master/scripts/stage-sources), [stage-roots](https://github.com/nyg/PowerDarwin/blob/master/scripts/stage-roots), [prepare-buildroot](https://github.com/nyg/PowerDarwin/blob/master/scripts/prepare-buildroot), [download-kit](https://github.com/nyg/PowerDarwin/blob/master/scripts/download-kit), [build-image](https://github.com/nyg/PowerDarwin/blob/master/scripts/build-image), [run-darwinbuild](https://github.com/nyg/PowerDarwin/blob/master/scripts/run-darwinbuild).
- [manifest/build-order](https://github.com/nyg/PowerDarwin/blob/master/manifest/build-order).
- [env/](https://github.com/nyg/PowerDarwin/blob/master/env/), the files the build-environment deviations install.
- [reference/andrew-r/](https://github.com/nyg/PowerDarwin/blob/master/reference/andrew-r/README.md).
- Deviations: [D1](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d1) to [D7](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d7), [D16](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d16), [D23](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d23), [D24](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d24), [D25](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d25), [D78](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d78), [D82](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d82), and every row that `prepare-buildroot` names, [D97](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d97) to [D99](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d99) the latest; [D100](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d100), [D105](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d105), [D110](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d110) and [D115](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d115) for `RC_ARCHS` and `RC_CFLAGS`; [D125](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d125) for the names and the time zone a build records.
- Commit `3dbd3fd`, which added `prepare-buildroot`.
- In darwinbuild's `Darwin9` branch: `darwinbuild/darwinbuild.in` (one build), `darwinbuild/darwinbuild.common` (`InstallRoot`, `Download`), `darwinbuild/installXcode31` (the copy of Xcode), `darwinxref/plugins/dependencies.c` (the kinds of dependency).
- [RESOURCES.md](https://github.com/nyg/PowerDarwin/blob/master/RESOURCES.md), "Tools".
