# PowerDarwin

Darwin 9.8 (Mac OS X 10.5.8, build `9L30`) built for PowerPC from Apple's published sources, with the smallest documented fixes.

## Step 1

A disk image whose files all come from this repo's builds boots to a single-user root shell (`-v -s`) in QEMU `mac99` and on an iMac G5.

## Rules

1. Project versions are exactly those in [manifest/9L30.tsv](manifest/9L30.tsv), taken from darwinbuild's 9L30 build list. A project moves to another version only to backport Apple's own fix.
2. Sources stay as Apple published them: fetched by tag from [apple-oss-distributions](https://github.com/apple-oss-distributions), their SHA-256 recorded in [manifest/9L30.sha256](manifest/9L30.sha256), never edited in place. The 14 projects marked `archive` have no matching tag there and come from the archived opensource.apple.com tarballs.
3. A fix changes the build environment first. Failing that, it backports Apple's fix from a later Apple release. Only then is it a new patch, as small as possible, with no reformatting or unrelated change.
4. Every patch in [patches/](patches/) and every workaround in [env/](env/) is listed in [DEVIATIONS.md](DEVIATIONS.md).
5. Builds use the toolchain of the release: `gcc` 5465, `gcc_42` 5531, `cctools` 667.3, `ld64` 77.1.
6. The final image contains no Apple binary: darwinbuild's Roots and the host's SDK only bootstrap the first build.

## Layout

- `manifest/`: project versions, their source, and the hashes of the fetched tarballs.
- `patches/<project>-<version>/`: one patch file per fix.
- `env/`: build-environment workarounds, each with the error it fixes.
- `scripts/fetch-sources`: downloads the `github` sources into `sources/` (not committed), records their hashes on the first run and checks them on later runs.
- `scripts/resolve-buildlist`, `scripts/fetch-roots`: work out each project's version and build, and fetch darwinbuild's Roots from the Wayback Machine.
- `reference/`: what the reference system runs.

## Status

The reference system is an iMac G5 running Sorbet Leopard, a community-modified Leopard that reports 10.5.9 build `9L31b` on Apple's 10.5.8 kernel. Results gathered on it stay provisional until they are checked against a Leopard installed from a retail disc.
