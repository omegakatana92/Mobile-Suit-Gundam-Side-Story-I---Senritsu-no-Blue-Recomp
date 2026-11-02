#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Build the Windows x64 port and launcher and put a portable package in dist/.
# Run from an MSYS2 MINGW64 shell:  tools/package_windows.sh
# No disc image, BIOS, save, generated code or build output is copied.
set -eu

root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"

B=build/gundam1
OUT=dist/Mobile-Suit-Gundam-Side-Story-I-Windows-x64
MINGW_BIN=${MINGW_BIN:-/mingw64/bin}

make -j"$(nproc)"
make -j"$(nproc)" launcher

rm -rf "$OUT"
mkdir -p "$OUT/LICENSES"

cp "$B/gundam1.exe" "$B/gundam1-launcher.exe" "$OUT/"
for dll in SDL3.dll zlib1.dll libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll libiconv-2.dll; do
    if [ -f "$MINGW_BIN/$dll" ]; then
        cp "$MINGW_BIN/$dll" "$OUT/"
    fi
done

cp LICENSE THIRD_PARTY_NOTICES.md README.md "$OUT/"
cp docs/STATUS.md docs/TESTING.md docs/BUILDING.md "$OUT/"
cp LICENSES/* "$OUT/LICENSES/"

cat > "$OUT/Play Mobile Suit Gundam Side Story I.bat" <<'EOF'
@echo off
setlocal
cd /d "%~dp0"
start "" "%~dp0gundam1-launcher.exe"
EOF

cat > "$OUT/FIRST-RUN.txt" <<'EOF'
Mobile Suit Gundam Side Story I: Senritsu no Blue - Windows x64 portable package
================================================================================

1. Put your own Japanese Saturn disc image (the .cue and its .bin tracks)
   anywhere you like. Do not copy it into this folder unless you want to.
2. Run "Play Mobile Suit Gundam Side Story I.bat" (or gundam1-launcher.exe).
3. On the launcher, choose the .cue file once, then press Play.
4. The launcher remembers the disc path. It only reads the image; it never copies or
   changes it.

The expected disc is Mobile Suit Gundam Side Story I: Senritsu no Blue (Japan),
product T-13306G, version V1.000.

Saves and settings live in %APPDATA%\Mobile Suit Gundam Side Story I - Senritsu no Blue\
(backup.bin and settings.ini), not in this folder. Logs are in its logs\ folder. Set
GUNDAM1_HOME to a folder to keep them somewhere else.

Controls: arrows move; Return is Start; Z/X/C are A/B/C; A/S/D are X/Y/Z; Q/E are L/R.
On a gamepad, the lower face buttons are A/B/C and Start is Start. The launcher's
Controllers page changes them.

This package contains no game data, no BIOS and no saves. See LICENSE,
THIRD_PARTY_NOTICES.md and LICENSES\.
EOF

printf '%s\n' "package: $OUT"
find "$OUT" -maxdepth 2 -type f -printf '%P %s\n' | LC_ALL=C sort
