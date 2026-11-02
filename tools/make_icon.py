#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Draw the project's original 256x256 icon (no game art)."""
import math
import struct
import sys
import zlib

S = 256


def blend(dst, src, a):
    return tuple(int(dst[i] * (1 - a) + src[i] * a) for i in range(4))


def in_round_rect(x, y, x0, y0, x1, y1, r):
    if x < x0 or x > x1 or y < y0 or y > y1:
        return False
    cx = min(max(x, x0 + r), x1 - r)
    cy = min(max(y, y0 + r), y1 - r)
    return (x - cx) ** 2 + (y - cy) ** 2 <= r * r


def main(path):
    px = [[(0, 0, 0, 0) for _ in range(S)] for _ in range(S)]
    # Dark blue rounded panel
    for y in range(S):
        for x in range(S):
            if in_round_rect(x, y, 8, 8, 247, 247, 40):
                px[y][x] = (8, 20, 52, 255)
    # Two concentric rings: a Saturn orbit, not game art
    for y in range(S):
        for x in range(S):
            d = math.hypot(x - 128, y - 124)
            if 74 <= d <= 79:
                px[y][x] = (208, 164, 54, 255)
            if 58 <= d <= 60:
                px[y][x] = (70, 120, 200, 255)
    # White/blue chevron rising from the ring
    for y in range(64, 182):
        for x in range(96, 160):
            dx = abs(x - 128)
            if dx <= 4 + (180 - y) * 0.42 and y >= 92:
                px[y][x] = (235, 242, 255, 255)
            if 112 <= x <= 144 and 132 <= y <= 174:
                px[y][x] = (46, 96, 176, 255)
    # Gold star points (original decoration)
    for sx, sy, r in ((52, 58, 3), (205, 70, 2), (70, 190, 2), (198, 198, 3), (44, 128, 2)):
        for y in range(sy - r, sy + r + 1):
            for x in range(sx - r, sx + r + 1):
                if 0 <= x < S and 0 <= y < S and (x - sx) ** 2 + (y - sy) ** 2 <= r * r:
                    px[y][x] = (220, 235, 255, 255)

    raw = bytearray()
    for y in range(S):
        raw.append(0)
        for x in range(S):
            raw.extend(px[y][x])

    def chunk(t, data):
        return struct.pack(">I", len(data)) + t + data + struct.pack(">I", zlib.crc32(t + data) & 0xFFFFFFFF)

    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", S, S, 8, 6, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(bytes(raw), 9))
    png += chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(png)


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "ms-gundam-1.png")
