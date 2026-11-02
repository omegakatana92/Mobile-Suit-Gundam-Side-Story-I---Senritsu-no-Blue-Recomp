# Mobile Suit Gundam Side Story I: Senritsu no Blue — v0.1.0

Native Windows x64 and Linux x86_64 recompilation of **Mobile Suit Gundam Side Story I:
Senritsu no Blue** (Bandai, Sega Saturn, Japan, 1996).

**You must supply your own disc image.** No ROM, BIOS, disc image, extracted game asset,
save, or game data is included in this repository or these downloads. The launcher reads
your `.cue` and its `.bin` tracks in place.

## Downloads

| File | Size | SHA-256 |
| --- | --- | --- |
| `Mobile-Suit-Gundam-Side-Story-I-Windows-x64.zip` | 12,183,353 | `8D6983F20E782D6040169C0E17EF9FD61AA9334BE8D7903CF3BE72E2A29C69E8` |
| `Mobile-Suit-Gundam-Side-Story-I-v0.1.0-SteamDeck-x86_64.AppImage` | 10,867,904 | `DBC1E2B4802FC4ED145ED51C9154E56D8665182AC4862226E15FAC0A94F490B2` |

The Windows ZIP is a portable folder: `gundam1.exe`, `gundam1-launcher.exe`, SDL3, zlib and
the MinGW runtime DLLs, licenses and docs. The AppImage is a Linux x86_64 candidate for
Steam Deck: SDL3, zlib, libstdc++ and libgcc_s are bundled; glibc/Mesa/X11/Wayland/audio
come from the host. Maximum ELF symbol is `GLIBC_2.34`.

## What works in this build

- Boots the disc, reads the ISO 9660 file system, and reaches the Bandai logo, the U.C.0079
  intro, the title screen (`NEW GAME` / `OPTION`) and cockpit gameplay.
- 7,494 SH-2 blocks (90,810 instructions) translated for the recorded paths; **100.00%
  recompiled, 0 interpreted** on the 15,000-frame action run and the 9,000-frame
  OPTION/sound-test run.
- `make check` passes on both recorded paths: interpreter and translated code produce the
  same state, and two runs are identical.
- SCSP/68000 sound and CD-DA are audible. The intro plays CD-DA track 3; the OPTION screen
  and sound test play CD-DA track 2. 44.1 kHz stereo, no clipped samples in the captured
  200 s.
- Windows and Linux builds, portable Windows package, and AppImage; launcher verifies
  product `T-13306G` and version `V1.000`.

## Known limitations

- A slave-CPU exception (`illegal instruction FFFF at 0C133774`) occurs after the title.
  The game continues and the master loads stages; it is present in interpreter and
  translated builds and needs reference/hardware comparison.
- VDP2 colour calculation and some RBG0 effects are approximate. The renderer reports the
  lines it draws without them.
- A full mission, stage BGM transitions, saves/reloads, a physical six-button pad and the
  second loop were not tested. The AppImage has not been run on Steam Deck hardware.

## Requirements

- Windows 11 x64, or Linux x86_64 with glibc 2.35+ (Ubuntu 22.04 baseline).
- Your own Japanese Saturn disc image: `.cue` plus its `.bin` tracks, product `T-13306G`,
  version `V1.000`.
- No BIOS image is needed.

## Run

Windows: unzip anywhere, run `Play Mobile Suit Gundam Side Story I.bat` (or
`gundam1-launcher.exe`), choose the `.cue`, press Play.

Linux: `chmod +x` the AppImage and run it. If FUSE is unavailable:
`./Mobile-Suit-Gundam-Side-Story-I-v0.1.0-SteamDeck-x86_64.AppImage --appimage-extract-and-run`.

Saves and settings go to the player's application folder (`%APPDATA%\Mobile Suit Gundam
Side Story I - Senritsu no Blue\` on Windows; the XDG data folder on Linux). Set
`GUNDAM1_HOME` to choose another folder.

## Source and licensing

The source is in this repository. The original project files are **PolyForm Noncommercial
1.0.0** (`LICENSE`, `LICENSING.md`). The synchronized `satrecomp/` toolkit is
**GPL-3.0-only** and is not relicensed; because the binaries contain it, the combined
binaries are distributed under GPL-3.0. The corresponding source is this repository,
including `satrecomp/`. Third-party notices are in `THIRD_PARTY_NOTICES.md` and `LICENSES/`.

The source tree contains the CMake build definitions required to build. No CMake cache,
build directory, generated translation, disc image, BIOS, save or game data is included.

The game and its assets are the property of their owners. This project is not affiliated
with or endorsed by Sega, Bandai or Sunrise.
