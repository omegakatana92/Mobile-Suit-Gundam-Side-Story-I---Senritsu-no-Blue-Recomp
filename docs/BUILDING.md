# Building

The same `runtime/`, `machine/`, `video/`, `sound/` and `port/` sources build on Windows x64
and Linux x86_64. Platform differences are the toolkit's, not this game's.

## Windows 11 x64

Use an **MSYS2 MINGW64** shell and install:

```sh
pacman -S --needed base-devel mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-ninja mingw-w64-x86_64-python mingw-w64-x86_64-SDL3 \
  mingw-w64-x86_64-zlib
```

Make sure `/mingw64/bin` comes before the MSYS `/usr/bin` when CMake looks for the compiler:

```sh
export PATH="/mingw64/bin:/c/Program Files/CMake/bin:$PATH"
make -j8
make -j8 launcher
```

The port is `build/gundam1/gundam1.exe`; the launcher is
`build/gundam1/gundam1-launcher.exe`. Copy the MinGW runtime DLLs beside them for a portable
folder:

```sh
for d in SDL3.dll zlib1.dll libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll; do
    cp "/mingw64/bin/$d" build/gundam1/
done
```

`tools/package_windows.sh` does this and writes `dist/Mobile-Suit-Gundam-Side-Story-I-Windows-x64/`.
It resolves the launcher's own files beside the executable.

## Linux x86_64 / Steam Deck

Build on the oldest glibc you intend to support. SteamOS 3 uses glibc 2.35 or newer; Ubuntu
22.04 (glibc 2.35) is a good baseline for the AppImage. Install a C++20 compiler, CMake 3.28+,
Ninja, Python 3.11+, SDL3 and zlib. On Ubuntu 22.04 SDL3 is not in the archive, so build and
install SDL3 from source (static is preferred for the AppImage):

```sh
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build python3 zlib1g-dev \
  libx11-dev libxext-dev libxrandr-dev libxi-dev libxcursor-dev libxfixes-dev \
  libwayland-dev libdecor-0-dev libasound2-dev libpulse-dev libudev-dev \
  libdrm-dev libgbm-dev libegl1-mesa-dev libgles2-mesa-dev
# SDL3 3.4.x from https://github.com/libsdl-org/SDL/releases, then:
cmake -S SDL3-3.4.16 -B SDL3-build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DSDL_STATIC=ON -DSDL_SHARED=OFF \
  -DCMAKE_INSTALL_PREFIX="$HOME/sdl3"
cmake --build SDL3-build -j
cmake --install SDL3-build
export CMAKE_PREFIX_PATH="$HOME/sdl3"
```

Then, from this repository:

```sh
make -j"$(nproc)"
make -j"$(nproc)" launcher
```

Run the Linux checks the same way as Windows:

```sh
make check FRAMES=3000 MOVIE=games/gundam1/movies/action.txt
```

### AppImage

`tools/package_appimage.sh` builds the port and launcher, stages an AppDir with `AppRun`,
desktop metadata and an original icon, and runs `appimagetool`:

```sh
APPIMAGETOOL=/path/to/appimagetool tools/package_appimage.sh
```

It bundles `libstdc++`, `libgcc_s`, `libatomic` and `libz`; SDL3 is linked statically. The
host's glibc, Mesa/graphics drivers and X11/Wayland/audio libraries are used, as they must be
on SteamOS. Saves and settings go to the XDG data folder (`$XDG_DATA_HOME` or
`~/.local/share`), which is writable on Steam Deck; set `GUNDAM1_HOME` to choose another
folder.

The AppImage is not Steam Deck **verified**: that requires a hardware test in Desktop Mode
and Gaming Mode, and no device was available here.

## Cross-checking the toolkit copy

`satrecomp/` is a copy of the toolkit made by `tools/sync-toolkit.sh`; do not edit it here.
Change the toolkit in its own repository, then run:

```sh
SATRECOMP=/path/to/satrecomp-omega-edtion satrecomp/tools/sync-toolkit.sh
make check-toolkit
```
