# Mobile Suit Gundam Side Story I: Senritsu no Blue — native recompilation

A native Windows and Linux port of **Mobile Suit Gundam Side Story I: Senritsu no Blue**
(Bandai, Sega Saturn, Japan, 1996), built with the
[satrecomp](https://github.com/tmcleroy/satrecomp) toolkit.

The SH-2 program is translated ahead of time to C++ and compiled into the port. The port
implements the Saturn board itself: the two SH-2s, the bus, the SCU, the VDP1/VDP2, the
SCSP and its 68000, CD-DA, the SMPC and the backup memory. There is no emulator in the
program and no BIOS image is needed.

**No game data is included.** The player supplies their own disc image. The launcher reads
it in place and never copies or changes it.

## What is here

```
Makefile                 GAME = gundam1, SR = satrecomp
games/gundam1/           game.cfg (disc T-13306G, V1.000), checks
satrecomp/               the toolkit, copied by tools/sync-toolkit.sh; never edited here
docs/                    STATUS.md, TESTING.md, BUILDING.md
tools/                   packaging and icon scripts
work/                    generated translations, saves and captures (git-ignored)
build/                   build output (git-ignored)
```

## Build

The toolkit needs a C++20 compiler, CMake 3.28+, Ninja, Python 3.11+, SDL3 and zlib.
On Windows use an MSYS2 **MINGW64** shell with `mingw-w64-x86_64-SDL3`, `cmake`,
`ninja`, `gcc` and `python`; on Linux use the distribution's SDL3 development package.

```sh
# from this repository's root
make                  # translate work/gundam1/blocks.txt and build build/gundam1/gundam1
make launcher         # build build/gundam1/gundam1-launcher
```

Put your disc image somewhere readable. `local.mk` (git-ignored) can name it so `make`
and `make check` find it:

```make
DISC := /path/to/Mobile Suit Gundam Side Story I - Senritsu no Blue (Japan).cue
```

or pass it to the port directly:

```sh
build/gundam1/gundam1 --disc "/path/to/the.cue"
```

The toolkit's copy is synchronized, not edited:

```sh
SATRECOMP=/path/to/satrecomp-omega-edtion satrecomp/tools/sync-toolkit.sh
make check-toolkit
```

## Run

```sh
build/gundam1/gundam1-launcher        # choose the .cue once, then Play
build/gundam1/gundam1 --disc "....cue"  # directly
```

The launcher checks the disc's product number (`T-13306G`) and version (`V1.000`), and
starts the game with the player's save file and settings.

Saves and settings live in the player's application folder (`%APPDATA%\Mobile Suit Gundam
Side Story I - Senritsu no Blue\` on Windows, the XDG data folder on Linux), not beside the
program. Set `GUNDAM1_HOME` to choose another folder.

Controls (defaults, changeable on the launcher's Controllers page): arrows move; Return is
Start; Z/X/C are A/B/C; A/S/D are X/Y/Z; Q/E are L/R. A gamepad's lower face buttons are
A/B/C and Start is Start.

## Checks

```sh
python3 satrecomp/tools/gamecfg.py name gundam1
make check FRAMES=3000 MOVIE=work/movies/action.txt
```

`make check` runs the interpreter and the translated code on the same input and compares a
state hash of work RAM, registers, picture and sound after every frame; it also runs the game
twice and compares the runs. See `docs/TESTING.md` for the exact commands and results.

## Status and limitations

The port boots to the title screen and into the cockpit gameplay. See `docs/STATUS.md` for
what was actually tested: translation coverage, audio analysis, the disc header, saves and
the remaining graphics and slave-CPU issues.

## License

The port and the toolkit are distributed under **GPL-3.0-only**. See `LICENSE`,
`THIRD_PARTY_NOTICES.md` and `LICENSES/`. The game, its disc image, its graphics, music and
samples are the property of their owners and are not part of this repository or its packages.
