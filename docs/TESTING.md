# Testing

All commands are run from this repository's root. On Windows use an MSYS2 **MINGW64**
shell; on Linux the same commands with `/` paths.

```sh
export PATH="/mingw64/bin:/c/Program Files/CMake/bin:$PATH"
```

## 1. Disc and CUE

The supplied image is never modified. Check that every `FILE` in the CUE resolves and
that the tracks are as expected:

```sh
python3 - <<'PY'
import os, re
cue = r"path/to/Mobile Suit Gundam Side Story I - Senritsu no Blue (Japan).cue"
base = os.path.dirname(cue)
for line in open(cue, encoding="latin-1"):
    m = re.match(r'FILE "(.+)" BINARY', line)
    if m:
        p = os.path.join(base, m.group(1))
        print(("OK " if os.path.exists(p) else "MISSING ") + m.group(1),
              os.path.getsize(p) if os.path.exists(p) else "")
PY
```

Expected: 12 files, `OK`, track 1 124,119,744 bytes (52,772 sectors of 2352), tracks 2–12
audio. The header is read from sector 0 user data: `T-13306G`, `V1.000`, `19960531`, area
`J`.

## 2. Build

```sh
make -j8
make -j8 launcher
```

Expected: `build/gundam1/gundam1.exe` and `build/gundam1/gundam1-launcher.exe`. The
`make` target runs `tools/recomp.py` on `work/gundam1/blocks.txt`; without a block database
it builds an interpreter-only program.

`local.mk` (git-ignored) may name the image:

```make
DISC := C:/path/to/Mobile Suit Gundam Side Story I - Senritsu no Blue (Japan).cue
```

## 3. Boot and coverage

```sh
build/gundam1/gundam1 --disc "$DISC" --headless --frames 600 --hash-log work/boot.hashes --stats
```

Expected: 600 lines in `work/boot.hashes`; no `Cannot read`, no invalid bus access, no
`unsupported`. Without translated code the stats show interpreted blocks; with the
database they show 100% recompiled for the recorded path.

Record more of the game (the movie is in `games/gundam1/movies/action.txt`):

```sh
build/gundam1/gundam1 --disc "$DISC" --headless --frames 15000 \
  --movie games/gundam1/movies/action.txt \
  --record-blocks work/gundam1/blocks.txt --stats
make gen
make -j8
```

Expected after the block database is complete for this path:

```
satrecomp: block runs: recompiled <N> (100.00%), interpreted 0 (0.00%)
```

The OPTION menu and its sound test are reached with
`games/gundam1/movies/soundtest.txt` (Start, Start, Down + Start, then Down, Down):

```sh
build/gundam1/gundam1 --disc "$DISC" --headless --frames 9000 \
  --movie games/gundam1/movies/soundtest.txt \
  --record-blocks work/gundam1/blocks.txt --stats
```

Expected: the OPTION page and `SOUND TEST` selection, 100% recompiled for the recorded
path, and CD-DA track 2 in the music log (`FAD 53072..60743`).

## 4. Equivalence check

```sh
make check FRAMES=3000 MOVIE=games/gundam1/movies/action.txt
```

Expected:

```
OK: action: the interpreter and the translated code give the same state for 3000 frames
OK: action: two runs are the same for 3000 frames
```

`make check` also runs `check-common` (the `satrecomp/recomp-common` copy is unchanged) and
`check-toolkit` (the `satrecomp/` copy is what `sync-toolkit.sh` recorded).

## 5. Pictures

```sh
build/gundam1/gundam1 --disc "$DISC" --headless --frames 6000 \
  --movie games/gundam1/movies/start.txt --shot-every 300:work/shots/shot_
```

Expected: black frames during the first second, the Bandai logo, the U.C.0079 text, the
title screen and the cockpit. `work/shots/` is git-ignored. Do not publish screenshots that
contain game art.

## 6. Sound

```sh
SATRECOMP_AUDIO_DUMP=work/audio/game.pcm \
SATRECOMP_CD_LOG=1 SATRECOMP_MUSIC_LOG=1 \
build/gundam1/gundam1 --disc "$DISC" --headless --frames 12000 \
  --movie games/gundam1/movies/start.txt 2> work/logs/audio.log
```

The dump is 16-bit stereo, 44.1 kHz. A quick level check:

```sh
python3 - <<'PY'
import array, math, os
p = "work/audio/game.pcm"
a = array.array("h")
with open(p, "rb") as f:
    a.fromfile(f, os.path.getsize(p) // 2)
l = a[0::2]
for sec in range(0, len(l) // 44100, 5):
    seg = l[sec * 44100:(sec + 5) * 44100]
    print(sec, int(math.sqrt(sum(x * x for x in seg) / len(seg))))
PY
```

Expected: non-zero RMS through the intro, title and gameplay; no clipped samples. The CD log
shows the CD-DA play (`FAD 60892..66951`) and `music: PlaySectors`; the music log shows the
same sector range being fed to the SCSP.

## 7. Launcher

```sh
GUNDAM1_HOME="$PWD/work/launcher-home" \
  build/gundam1/gundam1-launcher --screenshot work/launcher/shot --size 980x660
```

Expected: `shot0.png` … with the Game page showing the disc as `verified.` and Play
enabled. Then:

```sh
GUNDAM1_HOME="$PWD/work/launcher-home" \
  build/gundam1/gundam1-launcher --play -- --headless --frames 300
```

The launcher creates `saves/backup.bin` (32,768 bytes) and `logs/game.log`; the log ends
with `video: 300 frames`.

## 8. Package smoke test

Run the package from a directory other than the package, with a path containing spaces.
The port must find its DLLs beside the executable and read the disc in place. See
`tools/package_windows.sh`.

## Manual checks still needed

- Play a full mission, including a save and a reload after closing the port.
- Listen to gameplay BGM and CD-DA transitions with a real audio device (headless dumps
  were used here).
- Test a physical Saturn pad (six face buttons, L/R) and the in-game menu.
- Compare VDP2 colour calculation and RBG0 against the reference or real hardware.
- Confirm the slave-CPU `0C133774` stop is benign or identify the missing handshake.
- Linux/Steam Deck: run the AppImage in Desktop Mode and Gaming Mode on the device.
