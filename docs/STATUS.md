# Mobile Suit Gundam Side Story I: Senritsu no Blue — status

Updated: 2026-11-01 (local build)

## Disc

| Field | Value |
| --- | --- |
| Image | `Mobile Suit Gundam Side Story I - Senritsu no Blue (Japan).cue` |
| Header | `SEGA SEGASATURN`, maker `SEGA TP`, product `T-13306G`, version `V1.000`, date `19960531`, area `J` |
| Header title | `MOBILE SUIT GUNDAM SIDE STORY1 A SHUDDER OF THE BLUE` |
| Tracks | 12: track 1 `MODE1/2352` (52,772 sectors), tracks 2–12 `AUDIO` |
| CUE files | all 12 `FILE` entries resolve; quoted names with spaces and brackets are read as they are |
| Track layout | per-file `INDEX 00 00:00:00`, `INDEX 01 00:02:00` (track 2) or `00:01:74` (tracks 3–12); the reader keeps each track's own pregap and index-1 start |
| ISO 9660 | `GUNDAM_SIDE_STORY_1`; 47 files including `1.BIN`, `2.BIN`, `3.BIN`, the `S*/SE/SC` stage files, `SS_SND.BIN`, the `XX_CDA.Xnn` files and the movie files |

## What works

- **Boot:** the native BIOS path (`satrecomp/runtime/hle_bios.cpp`) authenticates and seeks
  the disc, reads the ISO 9660 file system, loads `1.BIN` at `0x06003000` and starts the game.
- **Title and intro:** Bandai logo, the U.C.0079 text pages, the opening animation, the title
  screen (`NEW GAME` / `OPTION`) and the cockpit gameplay have been reached and captured.
- **Gameplay:** the cockpit HUD, the central tactical display, the map and the cockpit view
  render; Start, the directions and A/B/C are exercised by the input movies in
  `work/movies/`.
- **Translation:** 7,494 blocks (90,810 instructions, 1,038 multi-block functions) are
  translated for the recorded path. A 15,000-frame action run is 100.00% recompiled,
  0 interpreted; a 9,000-frame OPTION/sound-test run is also 100.00% recompiled.
- **Sound:** the SCSP and its 68000 run; the output is 44.1 kHz, 16-bit stereo. Intro, title
  and gameplay audio is non-silent, with no clipping in the captured 200 s. CD-DA track 3 is
  played by the game's CD library and appears in the output (see below).
- **Launcher:** chooses and verifies the disc (`T-13306G`, `V1.000`), shows the Display and
  Controllers pages, and starts the port with the player's save and settings paths.

## Checks actually run

| Check | Command | Result |
| --- | --- | --- |
| Disc/CUE | script parse of the CUE and the 12 `.bin` files | 12/12 files present, track types and indexes as above |
| Header | read sector 0 user data | `T-13306G`, `V1.000`, `19960531`, area `J` |
| Build | `make -j8` in MSYS2 MINGW64 | succeeds; 103 targets, `build/gundam1/gundam1.exe` |
| Boot | `gundam1 --headless --frames 600 --hash-log ... --stats` | 600 frames; no invalid bus access, no missing code |
| Boot to title | `--shot-every 60` over 900–6000 frames | title at about frame 2400 |
| Gameplay | `--movie work/movies/action.txt --frames 15000` | cockpit gameplay, 100.00% recompiled, 0 interpreted |
| Block database | `--record-blocks work/gundam1/blocks.txt` | 7,494 blocks, 7,494 with native code |
| Equivalence | `make check FRAMES=3000 MOVIE=games/gundam1/movies/action.txt` | `OK`: interpreter == translated; two runs equal |
| OPTION / sound test | `--frames 9000 --movie work/movies/soundtest.txt` | OPTION menu, sound-test selection; 100.00% recompiled, 0 interpreted |
| OPTION equivalence | `make check FRAMES=6000 MOVIE=work/movies/soundtest.txt` | `OK`: interpreter == translated; two runs equal |
| Toolkit copy | `make check-toolkit` | `satrecomp/: commit f95e2341…` |
| Shared menu copy | `make check-common` | `recomp-common/: commit 28503a2e…` |
| Launcher screenshot | `GUNDAM1_HOME=… gundam1-launcher --screenshot work/launcher/shot --size 980x660` | 3 pages; disc shown as `verified.` |
| Launcher → port | `--play -- --headless --frames 300 --hash-log …` | game.log: `video: 300 frames`; `saves/backup.bin` created (32,768 bytes) |
| Audio dump | `SATRECOMP_AUDIO_DUMP=work/audio/game.pcm`, 12,000 frames | 8,854,622 stereo samples (200.8 s), non-silent, no clipped samples |
| CD-DA log | `SATRECOMP_CD_LOG=1 SATRECOMP_MUSIC_LOG=1` | `cd 10 play … (CDDA) FAD 60892..66951`, `music: PlaySectors FAD 60892..66951`; OPTION sound test repeats track 2 (`FAD 53072..60743`) |
| Windows package | `tools/package_windows.sh`; run from `C:\Windows\Temp` with the package in a path with spaces | 300 frames, 300 hashes, 100% recompiled; launcher screenshot `verified.`; `--play` created `saves/backup.bin` |
| Linux build | Ubuntu 22.04 WSL, GCC 11.4, SDL3 3.4.18 | `gundam1` and `gundam1-launcher` build |
| Linux check | `satrecomp/tools/check.sh 3000 games/gundam1/movies/action.txt` | `OK`: interpreter == translated; two runs equal |
| Linux coverage | 15,000-frame action run | 100.00% recompiled, 0 interpreted |
| AppImage | `tools/package_appimage.sh` | 10,863,808 bytes; max ELF symbol `GLIBC_2.34`; extracted port runs 300 frames 100%; AppRun launcher screenshot and `--play` under Xvfb |

## Packages

| Package | SHA-256 |
| --- | --- |
| `dist/Mobile-Suit-Gundam-Side-Story-I-Windows-x64.zip` (12,190,092 bytes) | `DA4DA99E138F940F0CBBDA9A9674305582DB9C83411472A733BC501CF8C8013A` |
| `dist/Mobile-Suit-Gundam-Side-Story-I-v0.1.0-SteamDeck-x86_64.AppImage` (10,863,808 bytes) | `0622336E0FF1A3B5FAFD03F544B9E76170FAD6738A581976358D3BD569860B44` |

The AppImage bundles SDL3, zlib, libstdc++ and libgcc_s; the host's glibc (2.35+), Mesa and
X11/Wayland/audio libraries are used. It was tested by extracting it and by running its
launcher under Xvfb in WSL; **it has not been run on Steam Deck hardware**, so it is a
Steam Deck candidate, not "verified".

## Audio diagnosis

Two separate paths were checked, as asked:

1. **SCSP / 68000.** The sound driver runs, the intro, title and gameplay output is
   non-silent (5-second RMS is typically 700–5,200 counts, peaks up to 32,767), and the
   captured audio has no clipped samples. This is the path that carries gameplay music and
   effects.
2. **CD-DA.** The game's CD library issues a `PLAY` for track 3
   (`start=000301 end=000363`), which the machine resolves to raw audio sectors
   FAD 60892..66951. `runtime/music.cpp` plays those sectors at 44.1 kHz into the SCSP's
   CD-audio input. The output around the start of that play correlates with the track's PCM
   (best normalized correlation 0.39 at the expected offset; the SCSP mix is on top, so this
   is not a clean channel). The OPTION screen and its sound test play track 2
   (`PlaySectors FAD 53072..60743`, repeated), and the captured audio is non-silent across
   those screens. This is the game's background music; it is not silent. Before the fix
   below the game did not get past the intro at all.

The earlier Hyper Duel fix in this toolkit (raw CD-DA sectors on the 44.1 kHz clock) is
present in the synchronized copy and was reused; the Gundam disc's CUE is multi-file with a
2-second/149-sector pregap, and the reader keeps those per-file offsets.

## What was changed

Toolkit (`satrecomp-omega-edtion`, committed `c94636b` and `f95e234`; copied here by
`sync-toolkit.sh`):

- `machine/cd.cpp`, `machine/machine.h`: command `0x40` (**set filter range**) and the
  getter commands `0x41`/`0x43`/`0x45`/`0x47`. The game's disc library sets a filter range,
  then waits for the selector-done interrupt (`ESEL`). The machine did not know `0x40`, so
  it reported the command without `ESEL` and the library polled status forever. This was the
  boot blocker: the screen stayed black at 900 and 3,000 frames; after the fix the Bandai
  logo and intro appear. Filter ranges are stored for the getters; the machine still uses one
  buffer partition and does not route sectors through filters (documented limitation).
- `build.mk`: export `DISC`, so `make check` gives `check.sh` the image named in `local.mk`.
- `machine/sh2.cpp`, `machine/sh2.h`, `runtime/recomp.cpp`: optional
  `SATRECOMP_PC_RING=1` ring of recent instructions/blocks, printed with an illegal
  instruction; `runtime/music.cpp`: `SATRECOMP_MUSIC_LOG` diagnostics; `machine/cd.cpp`:
  cycle number in the CD play log. These are development aids, off by default.

Game project:

- `Makefile`, `games/gundam1/game.cfg`, `games/gundam1/check_rewrites.sh`, `local.mk`
  (git-ignored), `VERSION`, `.gitignore`, `README.md`, `docs/`, `tools/`, `LICENSES/`.
- No game data, generated C++, block database, save or build output is committed.

## Known limitations / not yet validated

- **Slave CPU exception after the title.** After the title/opening, the slave stops with
  `illegal instruction FFFF at 0C133774`. The instruction ring shows it entered
  `0x06033872`, which is inside a literal pool (`0x060330A8`, `0x060335E8`); the `0x0603`
  word decodes as `bsrf r6` and `r6` is `0x060FFEFC`, giving `0x0C133774`. The same happens
  in the interpreter and in the translated build, so it is not a translation mismatch. The
  game continues and the master keeps loading stages; `make check` still passes. This needs
  a comparison with the reference/hardware to tell whether it is the game's deliberate slave
  stop or a race in the machine's command handshake. It is recorded, not papered over.
- **VDP2 colour calculation.** 540,906 lines in the 15,000-frame run were composed without
  the requested colour calculation (transparency between layers). The renderer reports it;
  the picture is drawn without the effect. RBG0 also reports 2,599,072 lines with mosaic or
  line-colour-screen enabled that are drawn without it. The title and cockpit are readable,
  but translucency/fades are approximate.
- **VDP2 RBG0 detail.** The central tactical display and cockpit are drawn; the exact
  rotation-screen modes beyond what the toolkit implements are not verified against a
  reference.
- **Saves.** The launcher creates `saves/backup.bin` (32,768 bytes) and the port uses it.
  The in-game save/load screens have not been exercised; a save after a mission is the next
  interactive test.
- **Long play.** 15,000 frames (about 4 minutes) of the opening and cockpit were run. A full
  mission, stage transitions, bosses, endings and the second loop are not tested.
- **Controllers.** Keyboard and gamepad mappings are the toolkit defaults; the six face
  buttons and shoulders are bound (A/B/C, X/Y/Z, L/R). A real controller was not available
  for this run.
- **Linux / Steam Deck.** The source is portable and the toolkit is GPL-3.0; the Linux
  binary and AppImage are built in the companion Linux environment (see
  `docs/BUILDING.md`). No Steam Deck hardware test has been made, so the AppImage must not
  be called Steam Deck verified.
- **Reference build.** The unpublished Ymir reference patch was not available; no
  `make verify` was claimed.
