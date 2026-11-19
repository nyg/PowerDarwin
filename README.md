# PowerDarwin

Darwin 9.8 (Mac OS X 10.5.8, build `9L30`) built for PowerPC from Apple's published sources, with the smallest documented fixes.

## Step 1

A disk image whose files all come from this repo's builds boots to a single-user root shell (`-v -s`) in QEMU `mac99` and on an iMac G5.

## Rules

1. Project versions are exactly those in [manifest/9L30.tsv](manifest/9L30.tsv), taken from darwinbuild's 9L30 build list. A project moves to another version only to backport Apple's own fix.
2. Sources stay as Apple published them: fetched by tag from [apple-oss-distributions](https://github.com/apple-oss-distributions), their SHA-256 recorded in [manifest/9L30.sha256](manifest/9L30.sha256), never edited in place. The 14 projects marked `archive` have no matching tag there and come from the archived opensource.apple.com tarballs.
3. A fix changes the build environment first. Failing that, it backports Apple's fix from a later Apple release. Only then is it a new patch, as small as possible, with no reformatting or unrelated change.
4. Every patch in [patches/](patches/), every workaround in [env/](env/) and every replacement in [replacements/](replacements/) is listed in [DEVIATIONS.md](DEVIATIONS.md).
5. Builds use the toolchain of the release: `gcc` 5465, `gcc_42` 5531, `cctools` 667.3, `ld64` 77.1.
6. The final image contains no Apple binary: darwinbuild's Roots and the host's SDK only bootstrap the first build.

## Layout

- `manifest/`: project versions, their source, and the hashes of the fetched tarballs.
- `patches/<project>-<version>/`: one patch file per fix.
- `env/`: build-environment workarounds, each with the error it fixes.
- `replacements/`: new code standing in for a closed Apple library, each listed in DEVIATIONS.md.
- `scripts/fetch-sources`: downloads the `github` sources into `sources/` (not committed), records their hashes on the first run and checks them on later runs.
- `scripts/resolve-buildlist`, `scripts/fetch-roots`, `scripts/stage-sources`, `scripts/stage-roots`: work out each project's version and build, fetch darwinbuild's Roots from the Wayback Machine, and copy sources and Roots to the build machine.
- `reference/`: what the reference system runs, and a description of Andrew-R's darwinbuild patch.
- [RESOURCES.md](RESOURCES.md): other people's work, tools, and where Apple's sources and binaries live.

## Building

A current Mac downloads the sources and Roots, since Leopard cannot reach GitHub or archive.org over TLS. A PowerPC Mac running Leopard with Xcode 3.1.4 and darwinbuild's `Darwin9` branch builds, in the build folder `~/9L30`.

1. Copy `manifest/plists/` into `~/9L30/.build/`, then run `darwinbuild -init 9L30` in `~/9L30` ([D2](DEVIATIONS.md)).
2. `scripts/stage-sources <host> <project>...` unpacks the GitHub tarballs as `<project>-<version>` into `~/9L30/Sources` ([D1](DEVIATIONS.md)).
3. `scripts/stage-roots <host>` copies the chosen Roots into `~/9L30/Roots/.DownloadCache`; `manifest/9L30-roots-used.tsv` says which build each came from.
4. On the build machine, `cd ~/9L30 && sudo /usr/local/bin/darwinbuild <project>` (Leopard's sudo has no `/usr/local/bin` in its path). Self-built Roots land in `~/9L30/Roots/<project>/` and take precedence over downloaded ones.
5. A failure is fixed by rule 3 and listed in DEVIATIONS.md.

The build environment (`~/9L30/BuildRoot`, a sparse disk image) holds workarounds put there by hand: [D4](DEVIATIONS.md), [D6](DEVIATIONS.md) to [D10](DEVIATIONS.md), [D13](DEVIATIONS.md), [D14](DEVIATIONS.md), and, for the unfinished IOKitUser attempt, the 9L30 `IOKitUser.hdrs` and `AppleDisplays` Roots. Re-apply them if it is recreated.

## Status

Built from source so far: `bootstrap_cmds`, `file_cmds`, `shell_cmds`, launchd's headers, the kernel (`xnu`, booted to a single-user shell on the iMac G5) and `libSystem` with all its parts. `kext_tools` waits on IOKitUser.

The reference system is an iMac G5 running Sorbet Leopard, a community-modified Leopard that reports 10.5.9 build `9L31b` on Apple's 10.5.8 kernel. Results gathered on it stay provisional until they are checked against a Leopard installed from a retail disc.
