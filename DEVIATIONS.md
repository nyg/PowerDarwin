# Deviations

Every difference from Apple's published sources and build environment, one row each.

| ID | Project | Kind | Change | Error it fixes | Upstream |
|---|---|---|---|---|---|
| D1 | all | `build-env` | Each source tarball from Apple's GitHub is unpacked and its top folder renamed from `<project>-<project>-<version>` to `<project>-<version>`, then placed in the build folder's `Sources/`, where darwinbuild takes a plain folder. Contents unchanged. | darwinbuild looks for `Sources/<project>-<version>`; GitHub names the folder after repo and tag. | none |
| D2 | darwinbuild | `bitrot` | The nine Darwin 9 build lists are copied into the build folder's `.build/` before `darwinbuild -init`, which then finds them instead of downloading. darwinbuild itself is unchanged (`Darwin9` branch at `4848b7f`). | The default list site `svn.macosforge.org` no longer exists. | none |

Kinds: `build-env` (build environment, no source change), `bitrot` (dead URL or vanished host tool), `bug`, `closed-replacement` (stands in for a closed Apple binary). Upstream names the later Apple release with the same fix, or "none".
