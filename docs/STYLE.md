# How the book is written

Rules for anyone writing a chapter of the book in `docs/src/`. [Step 5](src/steps/step-5-clock-and-power.md) shows these rules applied.

## Reader

The reader knows Unix: a shell, `make`, a C compiler, the split between a kernel and its userland. They do not know Darwin, Mac OS X's internals, PowerPC Macs, firmware, drivers or hardware. Every such idea is explained where a chapter first needs it, in a sentence or two, with a link to [Hardware and boot basics](src/background/hardware-basics.md) or the chapter that explains it fully, and is in the glossary.

## Shape of a chapter

- The title is the one in `SUMMARY.md`, as the chapter's only `#` heading.
- `## What you will learn` comes first: three to five items.
- Step chapters then have, in order: `## The goal` (README.md's words, quoted), the background the step needs, what was built, what broke and how it was fixed, `## How it was proved`, `## What is still missing`. Other chapters choose their own sections.
- `## Where to look` comes last: the repo files, deviations and commits the chapter drew on.
- A long chapter is better split by `##` headings than by more files.

## Writing

- One line per paragraph or list item, never wrapped by hand.
- Plain English, short sentences, active voice. Present tense for how things work, past tense for what was done.
- British spelling, as in the rest of the repo: licence, colour, behaviour, normalise.
- "Our" before a name means built by this project from Apple's source ("our kernel", "our AppleK2"); "Apple's" means Apple's own binary, as Leopard ships it. "Ours", as a label in tables and diagrams, means code the project wrote itself, such as OpenSMU.
- The iMac is "an iMac G5 (`PowerMac12,1`)" at first mention in a chapter, then "the iMac". It is the reference machine, not the subject: no chapter names or describes the particular machine.
- "QEMU's `mac99`", then "QEMU". "The build machine" for the Mac that runs darwinbuild. "The image" for the test disk image. "Darwin 9" for the system, `Darwin9` only for the partition of that name.
- Numbers come from the project's own builds and tests, never measured again for the book. In prose, "30 seconds", "4 GB", "65 °C"; in tables, `s` and `rpm` are fine.
- PowerPC assembly stays out.
- No speculation about why Apple did something: say what the sources and binaries show.

## Code, logs and names

- Commands, file names, symbols, registry properties and log lines in backticks.
- Errors and log lines are quoted exactly, as the builds and tests printed them and DEVIATIONS.md quotes them.
- Short excerpts of Apple's source may be quoted to explain a mechanism, with the project, version and file they come from; never a whole file.

## Links

- Other chapters: relative links to their `.md` file, such as `[the boot chain](../background/boot-chain.md)`.
- Repo files: absolute GitHub links, `https://github.com/nyg/PowerDarwin/blob/master/<path>`, since the book is served apart from the repo.
- Deviations: `[D81](https://github.com/nyg/PowerDarwin/blob/master/DEVIATIONS.md#d81)`, the anchor of the deviation's row in DEVIATIONS.md.
- Tests: by name in the prose ("the seventh boot test on the iMac G5"), never as a link.

## Diagrams and images

- Mermaid in fenced `mermaid` blocks, which both the book and GitHub draw. Short labels, the default theme, no colours of their own, so that light and dark themes both work.
- SVG only where Mermaid cannot draw the picture, in `src/images/`, with a transparent background and mid-tone strokes.
- Photos of the iMac's screen only, cropped to the screen, with every metadata field removed (`exiftool -all= <file>`), named after their chapter (`step-5-shutdown.jpg`).

## Glossary

A new term gets an entry in `reference/glossary.md`: a one- or two-sentence definition and the chapter that explains it.

## Kept out

Serial numbers, UUIDs, MAC addresses, host names, network addresses, home folders, the test password, and anything from `ioreg-l.txt` or `nvram.txt` beyond property and node names and values that every iMac G5 of the model shares (`u4@0,f8000000`, `pci1002,5b62`, `mac-boot`).
