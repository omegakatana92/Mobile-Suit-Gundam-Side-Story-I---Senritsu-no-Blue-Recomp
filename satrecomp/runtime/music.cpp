// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of satrecomp: https://github.com/tmcleroy/satrecomp
#include "music.h"

#include "state.h"

#include <cstdio>
#include <cstdlib>
#include <utility>

namespace satrecomp::music {

namespace {
    struct Player {
        Disc disc;
        State state = State::Stopped;
        int first = 0, last = 0, repeats = 0;
        int track = 0;
        uint32_t sector = 0, sectors = 0; // within the track
        uint32_t sample = 588;            // within the sector held in `data` (588: none held)
        uint8_t data[2352];
        // A range of the disc's raw audio sectors, as a CD PLAY command with frame addresses names it.
        bool sectorPlayback = false;
        uint32_t firstFAD = 0, endFAD = 0, nextFAD = 0;
    } g;
    std::function<bool(uint32_t, uint8_t *)> g_sectorReader;

    // The sector to play next into `data`; moves on to the next track, or back to the first, at a track's end
    bool Load() {
        while (g.sector >= g.sectors) {
            if (g.track < g.last) {
                g.track++;
            } else if (g.repeats > 0) {
                if (g.repeats < 15) {
                    g.repeats--;
                }
                g.track = g.first;
            } else {
                g.state = State::Paused; // (as the drive is left at the end of what it was told to play)
                return false;
            }
            g.sector = 0;
            g.sectors = g.disc.sectors ? g.disc.sectors(g.track) : 0;
            if (g.sectors == 0) {
                g.state = State::Stopped;
                return false;
            }
        }
        if (!g.disc.read(g.track, g.sector, g.data)) {
            g.state = State::Stopped;
            return false;
        }
        g.sector++;
        g.sample = 0;
        return true;
    }
} // namespace

void SetDisc(Disc disc) {
    g.disc = std::move(disc);
}

void SetSectorReader(std::function<bool(uint32_t, uint8_t *)> reader) {
    g_sectorReader = std::move(reader);
}

void PlaySectors(uint32_t first, uint32_t end, int repeats) {
    static const bool log = getenv("SATRECOMP_MUSIC_LOG") != nullptr;
    Stop();
    if (first >= end || !g_sectorReader) {
        return;
    }
    g.sectorPlayback = true;
    g.firstFAD = g.nextFAD = first;
    g.endFAD = end;
    g.repeats = repeats;
    g.state = State::Playing;
    if (log) {
        fprintf(stderr, "music: PlaySectors FAD %u..%u (exclusive), repeats=%d\n", first, end, repeats);
    }
}

bool SectorPlayback() {
    return g.sectorPlayback;
}

uint32_t CurrentFAD() {
    // (the sector being played is the one before the next, until the next has been loaded)
    return g.nextFAD - ((g.sample < 588 && g.nextFAD > g.firstFAD) ? 1u : 0u);
}

bool IsAudioTrack(int track) {
    return g.disc.sectors && g.disc.sectors(track) > 0;
}

void Play(int first, int last, int repeats) {
    g.sectorPlayback = false;
    if (!IsAudioTrack(first)) {
        Stop();
        return;
    }
    g.first = first;
    g.last = last < first ? first : last;
    g.repeats = repeats;
    g.track = first;
    g.sector = 0;
    g.sectors = g.disc.sectors(first);
    g.sample = 588;
    g.state = State::Playing;
}

void Pause() {
    if (g.state == State::Playing) {
        g.state = State::Paused;
    }
}

void Resume() {
    if (g.state == State::Paused && (g.track != 0 || g.sectorPlayback)) {
        g.state = State::Playing;
    }
}

void Stop() {
    g.sectorPlayback = false;
    g.state = State::Stopped;
    g.track = 0;
    g.sample = 588;
}

State GetState() {
    return g.state;
}
int CurrentTrack() {
    return g.track;
}
uint32_t CurrentSector() {
    return g.sector;
}

bool Next(int16_t &left, int16_t &right) {
    static const bool log = getenv("SATRECOMP_MUSIC_LOG") != nullptr;
    static uint64_t calls = 0;
    if (log && (calls++ % 44100) == 0) {
        fprintf(stderr, "music: Next call %llu state=%d sectorPlayback=%d FAD %u..%u next %u sample %u\n",
                static_cast<unsigned long long>(calls), static_cast<int>(g.state), g.sectorPlayback ? 1 : 0, g.firstFAD,
                g.endFAD, g.nextFAD, g.sample);
    }
    left = right = 0;
    if (g.state != State::Playing) {
        return false;
    }
    if (g.sample >= 588) {
        if (g.sectorPlayback) {
            if (g.nextFAD >= g.endFAD) {
                if (g.repeats == 0) {
                    g.state = State::Paused; // (the drive is left at the end of what it was told to play)
                    return false;
                }
                if (g.repeats < 15) {
                    --g.repeats;
                }
                g.nextFAD = g.firstFAD;
            }
            if (!g_sectorReader || !g_sectorReader(g.nextFAD, g.data)) {
                fprintf(stderr, "CDDA: cannot read the audio sector at FAD %u\n", g.nextFAD);
                g.state = State::Stopped;
                return false;
            }
            ++g.nextFAD;
            g.sample = 0;
        } else if (!Load()) {
            return false;
        }
    }
    const uint8_t *at = &g.data[g.sample * 4];
    left = static_cast<int16_t>(at[0] | (at[1] << 8));
    right = static_cast<int16_t>(at[2] | (at[3] << 8));
    g.sample++;
    return true;
}

void Serialize(Archive &ar) {
    ar.Mark(0x4D555349);
    ar.Pod(g.state);
    ar.Pod(g.first);
    ar.Pod(g.last);
    ar.Pod(g.repeats);
    ar.Pod(g.track);
    ar.Pod(g.sector);
    ar.Pod(g.sectors);
    ar.Pod(g.sample);
    ar.Pod(g.data);
    ar.Pod(g.sectorPlayback);
    ar.Pod(g.firstFAD);
    ar.Pod(g.endFAD);
    ar.Pod(g.nextFAD);
}

} // namespace satrecomp::music
