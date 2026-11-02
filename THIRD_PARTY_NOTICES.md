# Third-party notices

This project is distributed under the **GNU General Public License v3.0 only**
(`LICENSE`, `LICENSES/GPL-3.0-only.txt`). The components below are compatible with it.

The repository originally carried a GPL-2.0 text; it is kept as
`LICENSES/GPL-2.0.txt` for provenance.

## satrecomp

`satrecomp/` is a copy of [satrecomp](https://github.com/tmcleroy/satrecomp) commit
`c94636b` (then `f95e234`), Copyright (C) 2026 Tommy McLeroy, **GPL-3.0-only**. It is
copied by its own `tools/sync-toolkit.sh` and is never edited here. Its own
`THIRD_PARTY_NOTICES.md` is in the copy and applies in full; the important points are:

- The machine's models (`machine/`, `video/`, `sound/`, `runtime/backup.*`) are the toolkit's
  own, written with Ymir's source open beside them as the hardware description. Where that is
  derived from Ymir, it is used under GPL-3.0 and this is its notice. **Ymir is not part of
  this build or package.**
- The VDP1 counters are the toolkit's own code; the counting rules were learnt from Ymir.
- `port/librashader.h` is librashader 0.12.1's C header, MIT, Copyright (c) chyyran and
  contributors.

## recomp-common

`recomp-common/` (the in-game menu, settings rules and launcher) is a copy of
[recomp-common](https://github.com/tmcleroy/recomp-common), GPL-3.0, made by
`tools/sync-common.sh`. Its launcher descends from
[wiiparty-recomp](https://github.com/arelkair/wiiparty-recomp) by arelkair
(GPL-3.0-or-later); files carrying that notice keep it.

## Downloaded at build time (not stored in this repository)

| Component | License | Use |
| --- | --- | --- |
| [SDL 3](https://libsdl.org) 3.4.16 | zlib; Copyright (C) 1997-2026 Sam Lantinga | Window, input, audio, GPU in the port and launcher |
| [Dear ImGui](https://github.com/ocornut/imgui) 1.92.9b | MIT; Copyright (c) 2014-2026 Omar Cornut | Launcher interface, compiled into the launcher |
| [Inter](https://rsms.me/inter/) 4.1 | SIL Open Font License 1.1; Copyright (c) 2016 The Inter Project Authors | Launcher font, compiled into the launcher |
| [zlib](https://zlib.net) | zlib; Copyright (C) 1995-2024 Jean-loup Gailly and Mark Adler | PNG screenshots and compression |

The Windows package also carries the MinGW runtime DLLs (`libgcc_s_seh-1`, `libstdc++-6`,
`libwinpthread-1`) under the GCC Runtime Library Exception; see
`LICENSES/GCC-Runtime.txt`, `LICENSES/GCC-libstdc++-COPYING3.txt` and
`LICENSES/libwinpthread-COPYING.txt`. The Linux AppImage carries `libstdc++` and `libgcc_s`
under the same exception.

A binary distribution must carry these license texts, the GPL-3.0 text, and the offer of
corresponding source that goes with the GPL.

## Reference/development only

Ymir (GPL-3.0) is the emulator the toolkit uses as a development reference when its
unpublished reference patch is available. Neither Ymir nor that patch is part of this
project's builds or distribution. No reference verification was performed for this port.

## What is not included

No disc image, no part of one, no extracted game files, no BIOS image, no save data, no
input recording containing game code, no block database and no generated translated C++.
The player supplies their own disc. Screenshots containing game art must not be published
as release assets.
