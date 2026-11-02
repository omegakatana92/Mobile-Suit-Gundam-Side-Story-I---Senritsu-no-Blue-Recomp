#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Build the Linux x86_64 port and launcher and package a Steam Deck AppImage.
#
# Requires a Linux x86_64 environment with the project's dependencies, a working
# SDL3 CMake package (static SDL3 preferred) and appimagetool:
#   APPIMAGETOOL=/path/to/appimagetool tools/package_appimage.sh
#
# Build on the oldest glibc baseline you intend to support. Ubuntu 22.04 (glibc 2.35)
# is older than current SteamOS; that is the recommended baseline.
set -eu

root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"

GAME=gundam1
B=build/$GAME
VER=$(cat VERSION)
OUT=dist/Mobile-Suit-Gundam-Side-Story-I-v${VER}-SteamDeck-x86_64.AppImage
APPDIR=dist/Mobile-Suit-Gundam-Side-Story-I.AppDir
APPIMAGETOOL=${APPIMAGETOOL:-appimagetool}

make -j"$(nproc)"
make -j"$(nproc)" launcher

rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/lib" \
         "$APPDIR/usr/share/applications" \
         "$APPDIR/usr/share/icons/hicolor/256x256/apps" \
         "$APPDIR/usr/share/licenses/mobile-suit-gundam-side-story-i-1"

cp "$B/$GAME" "$B/$GAME-launcher" "$APPDIR/usr/bin/"
cp assets/ms-gundam-1.png "$APPDIR/ms-gundam-1.png"
cp assets/ms-gundam-1.png "$APPDIR/usr/share/icons/hicolor/256x256/apps/ms-gundam-1.png"
cp assets/ms-gundam-1.desktop "$APPDIR/ms-gundam-1.desktop"
cp assets/ms-gundam-1.desktop "$APPDIR/usr/share/applications/ms-gundam-1.desktop"

# The C++ runtime is the only non-base runtime library we add here. SDL3 should be
# linked statically; X11/Wayland/audio/GL are provided by SteamOS (and are not replaced
# by the AppImage, so graphics drivers and libc stay the host's).
needed=$(for exe in "$B/$GAME" "$B/$GAME-launcher"; do
    ldd "$exe" 2>/dev/null
done | awk '{print $1}' | sort -u)
for lib in libstdc++.so.6 libgcc_s.so.1 libatomic.so.1 libz.so.1 libSDL3.so.0; do
    if printf '%s\n' "$needed" | grep -qx "$lib"; then
        p=$(ldconfig -p 2>/dev/null | awk -v n="$lib" '$1 == n {print $NF; exit}')
        [ -n "$p" ] && cp -L "$p" "$APPDIR/usr/lib/"
    fi
done
for exe in "$APPDIR/usr/bin/$GAME" "$APPDIR/usr/bin/$GAME-launcher"; do
    patchelf --set-rpath '$ORIGIN/../lib' "$exe"
done

cp LICENSE LICENSING.md THIRD_PARTY_NOTICES.md "$APPDIR/usr/share/licenses/mobile-suit-gundam-side-story-i-1/"
cp LICENSES/* "$APPDIR/usr/share/licenses/mobile-suit-gundam-side-story-i-1/"
cp satrecomp/LICENSE "$APPDIR/usr/share/licenses/mobile-suit-gundam-side-story-i-1/satrecomp-LICENSE"

cat > "$APPDIR/AppRun" <<'EOF'
#!/bin/sh
# AppImage entry point: everything is relative to APPDIR, never the current directory.
HERE=$(dirname "$(readlink -f "$0")")
APPDIR=${APPDIR:-$HERE}
export APPDIR
export LD_LIBRARY_PATH="$APPDIR/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
exec "$APPDIR/usr/bin/gundam1-launcher" "$@"
EOF
chmod +x "$APPDIR/AppRun"

ARCH=x86_64 VERSION="$VER" "$APPIMAGETOOL" --no-appstream "$APPDIR" "$OUT"
chmod +x "$OUT"
echo "built $OUT"
