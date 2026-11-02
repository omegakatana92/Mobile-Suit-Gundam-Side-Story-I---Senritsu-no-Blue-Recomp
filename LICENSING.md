# Licensing

This repository contains two kinds of material, under two licenses.

## The game project (PolyForm Noncommercial 1.0.0)

The original files written for this port — `Makefile`, `games/`, `docs/`, `tools/`,
`assets/`, the input movies under `games/gundam1/movies/`, `README.md`, this file, and the
project’s own scripts — are Copyright (c) 2026 omegakatana92 and are licensed under the
**PolyForm Noncommercial License 1.0.0** (`LICENSE`). Use, change and share them for any
noncommercial purpose, with the notices below.

Required Notice: Copyright (c) 2026 omegakatana92

## The synchronized toolkit (GPL-3.0-only)

`satrecomp/` is a copy of the [satrecomp](https://github.com/tmcleroy/satrecomp) toolkit,
Copyright (C) 2026 Tommy McLeroy, made by `satrecomp/tools/sync-toolkit.sh`. It is
**GPL-3.0-only** (`satrecomp/LICENSE`, `LICENSES/GPL-3.0-only.txt`) and is **not** covered
by the PolyForm Noncommercial license above. Its `THIRD_PARTY_NOTICES.md` applies in full.
`satrecomp/recomp-common/` is also GPL-3.0.

The toolkit is not relicensed here. The person who publishes a modified toolkit must do so
under GPL-3.0, and the toolkit’s copyright holder is the only one who can offer other terms
for it.

## What this means for the binaries

The Windows and Linux programs are built by compiling the game’s translated code together
with the GPL-3.0-only toolkit. The resulting binaries contain GPL-3.0 code, so **GPL-3.0
governs those binaries and the combined work**. They are distributed with the GPL-3.0 text
and the corresponding source (`satrecomp/` plus this repository), as GPL-3.0 requires. The
PolyForm Noncommercial terms apply only to the original project code where it can be
separated from the GPL-covered toolkit.

If you need a binary that is exclusively noncommercial, it cannot include this toolkit
without permission from the toolkit’s copyright holder; the alternative is to replace the
toolkit with one whose license permits that.

## Third-party components

SDL3 (zlib), Dear ImGui (MIT), Inter (SIL OFL 1.1), zlib (zlib), librashader’s C header
(MIT), and the MinGW/GCC runtime (GCC Runtime Library Exception) are used as described in
`THIRD_PARTY_NOTICES.md` and `LICENSES/`. MAME and Ymir are development references only;
neither is linked into the released game.

## Game data

No disc image, ROM, BIOS, extracted asset, save, generated translation, or game data is part
of this repository or its releases. The player supplies their own disc image. The game and
its assets remain the property of their owners.
