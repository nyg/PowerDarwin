# Step 4: everyday commands

## What you will learn

- Which of Apple's projects hold the commands a Unix user reaches for first, and the two daemons every Unix system runs, a logger and a clock.
- What a static archive is, and why two of them, `SystemStubs` and `libmd`, had still come from Apple when every library on the image was ours.
- How the project checked whether Apple's code from those archives had reached the image.
- What dyld's shared cache is, and why the tool that builds it ran again every few seconds until the image listed its own programs.

## The goal

README.md sets it as: "Over SSH, everyday commands work: `grep`, `ps`, `less` and `vi` run, syslogd writes `/var/log/system.log`, and ntpd sets the clock from the network."

By Step 3 an iMac G5 (`PowerMac12,1`) took SSH logins from another Mac, but the image held little beyond a shell, the core file and text tools, and the daemons that bring up the network. Step 4 adds what a user expects to find after logging in, and the first log file, which turned out to show what the earlier steps could not see.

## Background: static archives

A program uses a library in one of two ways. A dynamic library, such as `libSystem`, stays a file of its own: [dyld](step-1-kernel-and-userland.md), the dynamic linker, loads it each time the program starts. Replace the file, and every program that uses it runs the new code at its next start.

A static library is an archive, a `.a` file that bundles object files. At link time the linker copies the members a program needs into the program itself, and the archive is not needed again. Its code then lives on inside every program linked with it, whatever happens to the archive later.

This matters for rule 5 ([The rules](../method/rules.md)), which keeps every Apple binary off the image. Replacing Apple's libraries in the build environment with ours changes what programs load. It does not change code that an archive had already copied into a program built earlier.

### SystemStubs

Every project in the build list names `SystemStubs` among its static libraries, and until Step 4 darwinbuild had taken it from Apple's 9A581 Root. Its archive, `libSystemStubs.a`, holds small functions for `ppc`, named like `printf$LDBLStub`. Leopard's `libSystem` has two versions of functions such as `printf` and `scanf` on PowerPC, one for an older 64-bit `long double` and one for the 128-bit type, named with `$LDBL128`. Each stub, with the helper `___stub_getrealaddr` in SystemStubs 6's `getrealaddr.c`, looks up the right version in `libSystem` when it is first called.

### Did Apple's code reach the image?

A stub only ends up in a program built to run on Mac OS X versions older than 10.4 as well, for which the system headers turn a call of `printf` into one of `printf$LDBLStub` instead of `printf$LDBL128`; so the question had an answer that could be checked. The project scanned the image's 399 executables and libraries for the functions that `libSystemStubs.a` defines (`___stub_getrealaddr` and the `$LDBLStub` functions). It found one: `___stub_getrealaddr`, in the `ppc` slices of OpenSSL's `libcrypto.0.9.7.dylib` and `libssl.0.9.7.dylib`. A slice is one processor's copy of the code in a [fat binary](../method/darwinbuild.md).

So Apple's code had reached the image, in two files. The project then built SystemStubs from source, in 25 seconds. Its 46 `ppc` members have the same code and data as those of Apple's archive, so the copy already inside OpenSSL was exactly what our build makes. OpenSSL was rebuilt anyway, in 46 minutes 7 seconds, with our SystemStubs in the build environment, and its `___stub_getrealaddr` now comes from our archive.

### libmd

ntp links `libmd`, a small library of message digests such as MD5, statically. `libmd` 2 was built for the same reason, in 5 seconds, as a four-way archive (`ppc`, `ppc64`, `i386`, `x86_64`), before ntp was linked with it. No binary on the image held its code before.

## The commands and their projects

Apple does not publish Darwin one command at a time. Each command belongs to a project, a source tarball with its own version, and a project often holds many commands: `adv_cmds` alone installs 510 files, `ps` among them. [Leopard 9L30](../method/sources-and-roots.md) explains how the build list fixes each project's version.

| Command | Project | What it installs |
|---|---|---|
| `grep` | `grep` 24 | GNU grep |
| `ps` | `adv_cmds` 119 | `ps` and other BSD tools |
| `less` | `less` 20 | less 394 |
| `vi` | `vim` 39 | Vim 7.2 with patches 1 to 22, also installed as `vi` |
| `syslogd` | `syslog` 69.0.4 | `syslogd` and `aslmanager`, with their launchd jobs |
| `ntpd`, `ntpdate` | `ntp` 37.3 | `ntpd`, `ntpdate`, `ntpq` and `ntpd-wrapper` |

Each built at its first attempt, by darwinbuild, from Apple's source as published, in between 26 seconds (syslog) and 5 minutes 35 seconds (vim). Step 4 added no deviation.

Two of them are daemons, which launchd starts at boot from the job files their projects install. syslogd collects the messages that programs and the kernel log, and writes them to the files `/etc/syslog.conf` names, `/var/log/system.log` first among them. ntpd sets the clock from a time server and keeps it right; its launchd job runs `ntpd-wrapper`, a short shell script that first runs `ntpdate` once to set the clock in one step, then starts `ntpd`. Leopard's Date & Time preferences, a closed program, write the server's name into `/etc/ntp.conf`, so [scripts/make-test-image](https://github.com/nyg/PowerDarwin/blob/master/scripts/make-test-image) writes that file instead, with `server time.euro.apple.com`.

One oddity of Apple's own: ntp's launchd job file has a `DOCTYPE` line whose quotes are missing. Apple's 9A581 Root has the same line, so the file was left as built.

## What the first log showed

With syslogd running, `/var/log/system.log` showed, for the first time, messages that had gone to no file before. Most of them became later steps. One was a loop.

### dyld's shared cache

Starting a program means loading its libraries and binding each call to its address, and many programs load the same libraries: `libSystem`, CoreFoundation, IOKit. dyld's shared cache does that work once: one file holds those libraries, already bound to each other, and dyld maps it into every process at the same address, through a region of memory the kernel shares between processes. Programs run without it, only more slowly.

The cache is built by `update_dyld_shared_cache`, from dyld 97.1. When a program starts and finds no cache for its processor, `libSystem` sends a message to the Mach service `com.apple.dyld`. launchd holds that name for dyld's job, `com.apple.dyld`, and starts the tool when the message comes. The tool reads lists of executables from `/var/db/dyld/shared_region_roots/` and puts the libraries they use in the cache.

On the image, the tool failed:

```text
/var/db/dyld/shared_region_roots/ does not exist
```

With no cache made, the next program to start asked again, and launchd started the tool again, every few seconds, for as long as the machine ran. On Leopard, that folder holds lists of executables from closed packages, such as Finder, Safari, Cocoa, QuickTime and Java, none of which the image has. Creating the folder was not enough: with it empty, the tool logs `no entries found in shared_region_roots` and makes no cache.

So `make-test-image` writes a list of its own, `Darwin.paths`, naming `/bin/sh` and `/bin/bash`, as Leopard's `Applications.paths` does, and `configd`, `DirectoryService` and `sshd`. At the next boot dyld's job built a 14 MB cache once (`regenerated cache for arch=ppc`), and launchd did not start it again.

### The next steps, in the log

Three other lines of that first log are told in later chapters:

- ntpd-wrapper's `ntpdate -bvs` failed with `can't find host time.euro.apple.com`, as it ran before configd had named `en0`, the Ethernet port. ntpd itself reached the server minutes later. Why `en0` came so late is the story of [Step 8](step-8-clean-boot.md).
- On the iMac, ntpd's correction was `ntpd[23]: time reset +1795261981.720796 s`: the clock had started in 1970, as no driver read the hardware clock. That is [Step 5](step-5-clock-and-power.md).
- Each SSH connection logged `launchproxy[86]: /usr/libexec/sshd-keygen-wrapper: SessionCreate() failed: 226`, as securityd was not built yet. That is [Step 6](step-6-securityd.md).

## How it was proved

In QEMU's `mac99`, in the fourth multi-user boot test, syslogd started, printed messages on the console and wrote them to `/var/log/system.log`. Over SSH, `grep`, `ps`, `less` and `vi` ran. ntpd set the clock from 1969 to the right date some minutes after boot. With `Darwin.paths` in place, dyld's job built its cache once.

On the iMac, in the sixth boot test on the iMac G5, the same image was restored onto the `Darwin9` partition and chosen in the Startup Manager by holding Option at power-on, with the kernel still started with `-v` for a verbose boot. Over SSH from another Mac:

1. `grep`, `ps`, `less` 394 and Vim 7.2 ran.
2. `/var/log/system.log` filled, and dyld built its cache once.
3. ntpd set the clock about four and a half minutes after boot, and `date` then gave the right time.
4. Logins worked, each with the `SessionCreate() failed` line above.
5. `halt` synced the disks and stopped at `CPU halted`, with the machine still on.

The image held 12,163 files, all from the project's builds except eight that `make-test-image` writes or edits: the six of Step 3, `ntp.conf` and `Darwin.paths`. Step 4 was reached in QEMU and on the iMac.

## What is still missing

- The clock started in 1970 at every boot, and the iMac could neither restart nor switch itself off: [Step 5](step-5-clock-and-power.md).
- Logins had no security session of their own: [Step 6](step-6-securityd.md).
- `ntpdate` failed at boot, and the network came up a minute late: [Step 8](step-8-clean-boot.md).
- `passwd` is still not built, as it needs OpenDirectory, which needs Apple's closed CoreFoundation ([D44](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d44)); `make-test-image` sets root's password instead.

## Where to look

- [scripts/make-test-image](https://github.com/nyg/PowerDarwin/blob/master/scripts/make-test-image): `ntp.conf` and `Darwin.paths`.
- [manifest/9L30.tsv](https://github.com/nyg/PowerDarwin/blob/master/manifest/9L30.tsv), for the projects' versions.
- In dyld-97.1: `launch-cache/update_dyld_shared_cache.cpp` (the roots folder and its messages), `launch-cache/com.apple.dyld.plist` (the job and its Mach service), `src/dyldAPIsInLibSystem.cpp` (the message to `com.apple.dyld`).
- In SystemStubs-6: `getrealaddr.c` and the `s_*.s` stubs.
- In ntp-37.3: `ntpd-wrapper`.
- Commit `05ee463`, which reached the step.
