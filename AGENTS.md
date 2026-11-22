# PowerDarwin

A guide for coding agents working in this repo. The project's goal, rules and steps are in [README.md](README.md).

## Read first

- [README.md](README.md): the goal, the rules every change follows, and the steps.
- [DEVIATIONS.md](DEVIATIONS.md): every change from Apple's sources and build, one row each.
- [hardware/](hardware/): the machines tested, and the drivers each one needs.
- The book in [docs/src/](docs/src/): the method, and how each step was reached.
- [RESOURCES.md](RESOURCES.md): other people's work, tools, and where Apple's sources and binaries live.

## Machines

- A current Mac fetches the sources and Roots with `scripts/fetch-sources` and `scripts/fetch-roots`, since Leopard cannot reach GitHub or archive.org over TLS. They land in `sources/` and `roots/`, which git ignores, and `manifest/` holds their hashes.
- A PowerPC Mac with Leopard, Xcode 3.1.4 and darwinbuild's `Darwin9` branch builds, in the build folder `~/9L30`. The scripts reach it through the SSH host name given as `<host>`.
- Keep the access details of your own machines in a git-ignored file, such as `CLAUDE.local.md`.

## Making a change

- Follow the README's rules in order: change the build environment first, then take Apple's own fix from another release, and only then write a new patch, as small as possible.
- List every patch, build-environment workaround and replacement in DEVIATIONS.md, with the next free ID, its kind, and the Apple release with the same fix, if any.
- Patches go in `patches/<project>-<version>/`, one file per fix, whose first line gives the deviation, the date of its last change and its licence, as the others do. Build-environment workarounds go in `env/`, each with a step in `scripts/prepare-buildroot`. New code that stands in for a closed Apple part goes in `replacements/`, with its own copy of the licence.
- Check each image with `scripts/check-image`: only the files that `scripts/make-test-image` writes may come from outside the builds.
- Ask the machine's owner before restoring an image onto a partition or changing what a machine boots.

## On the build machine

- Leopard's sudo has no `/usr/local/bin` in its path: run `sudo /usr/local/bin/darwinbuild <project>` from `~/9L30`.
- Start long builds with `nohup … < /dev/null &` as a command of its own, never inside an `&&` list, then grep the log for `error:` and `BUILD FAILED`.
- Build a project's headers first, with `darwinbuild -headers <project>`, when the build environment holds older ones.
- The `prepare-buildroot` steps that write into the build environment need its sparse bundle attached: `sudo hdiutil attach .build/buildroot.sparsebundle -readwrite -owners on` in `~/9L30`.
- In a chroot of the image on the build machine, never run a `launchctl` command that talks to launchd: the chroot shares the host's launchd.
- Leopard has no `shasum` on its path: use `md5 -r`.

## Commits and documents

- Commits follow Conventional Commits (`feat:`, `fix:`, `build:`, `docs:`, `chore:`), subject line only, one step of progress each.
- Keep out of git: serial numbers, the platform UUID, volume UUIDs, MAC addresses, `ioreg -l` and `nvram` dumps (git ignores `reference/*/ioreg-l.txt` and `reference/*/nvram.txt`), test passwords (git ignores `images/`), and photo metadata (strip it with `exiftool -all=`).
- Documents have one line per paragraph or list item. The book follows [docs/STYLE.md](docs/STYLE.md).
