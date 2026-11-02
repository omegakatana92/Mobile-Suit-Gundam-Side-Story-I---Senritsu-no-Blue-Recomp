// SPDX-License-Identifier: GPL-3.0-only
// Copyright (C) 2026 Tommy McLeroy. Part of satrecomp: https://github.com/tmcleroy/satrecomp
// The CD drive's host interface, as far as the games still use it. Files, movie data and CD music are mostly
// read by the port itself (games/<id>/src: the game's library routines replaced); what is left are the
// commands a CD library gives when it starts: status, table of contents, set-up of the buffer's filters,
// seeks; and data sectors read into the buffer and taken through the data port, for library code that has
// not been replaced (Fighters Megamix's second program reads the disc's directory that way). The buffer is
// one partition; the filters are not applied.
#include "machine.h"
#include "lift.h"
#include "music.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iterator>

namespace satrecomp::machine {

namespace {
    // Interrupt request bits (HIRQ)
    enum : uint16 {
        kCMOK = 0x0001, // command accepted, response ready
        kDRDY = 0x0002, // data ready at the data port
        kCSCT = 0x0004, // a sector was read
        kBFUL = 0x0008, // the buffer is full
        kPEND = 0x0010, // the end of what was to be played
        kDCHG = 0x0020, // the disc was changed
        kESEL = 0x0040, // selector set-up done
        kEHST = 0x0080, // data transfer ended
        kEFLS = 0x0200, // file system command done
        kSCDQ = 0x0400, // subcode read
    };
    constexpr uint32 kBufferSectors = 200;
} // namespace

void Machine::CdReset() {
    satrecomp::music::Stop();
    m_cd = {};
    m_cd.periodicDue = m_now + m_clock / 75;
}

uint16 Machine::CdRead(uint32 address) {
    auto &cd = m_cd;
    const uint32 auxiliary = address & 0xFFFFF;
    if (auxiliary >= 0x95000 && auxiliary < 0x96000) { // (the auxiliary window: read back what was written)
        if (getenv("SATRECOMP_NO_MODEM")) {
            return 0; // (diagnostic: a Saturn with no NetLink/XBand device)
        }
        const uint16 value =
            static_cast<uint16>((cd.aux[(auxiliary - 0x95000) & 0xFFE] << 8) | cd.aux[(auxiliary - 0x95000) & 0xFFF]);
        static const bool log = getenv("SATRECOMP_CD_LOG") != nullptr;
        static unsigned reads = 0;
        if (log && reads++ < 32) {
            fprintf(stderr, "cd aux read %05X -> %04X (PC %08X PR %08X)\n", auxiliary, value, masterSH2.PC, masterSH2.PR);
        }
        return value;
    }
    if (address < 0x90000 || address >= 0x98000 || (address & 0xFFFF) < 0x08) { // the data port (0x25818000, 0x25898000)
        if (cd.transferOffset >= cd.transferSize) {
            return 0xFFFF;
        }
        const uint32 offset = cd.transferOffset++;
        if (cd.transferSectors == 0) {
            return cd.transfer[offset];
        }
        // (a sector's data, read from the disc when the transfer comes to it)
        static uint8 sector[2048];
        static uint32 sectorFAD = 0xFFFFFFFF;
        const uint32 fad = cd.transferFAD[offset / 1024];
        if (fad != sectorFAD) {
            sectorFAD = fad;
            if (!satrecomp::ReadDiscSector(fad, sector)) {
                std::fill(std::begin(sector), std::end(sector), 0);
            }
        }
        const uint32 at = (offset % 1024) * 2;
        return static_cast<uint16>((sector[at] << 8) | sector[at + 1]);
    }
    switch (address & 0x3C) {
    case 0x08: return cd.HIRQ;
    case 0x0C: return cd.HIRQMask;
    case 0x18: return cd.CR[0];
    case 0x1C: return cd.CR[1];
    case 0x20: return cd.CR[2];
    case 0x24:
        cd.responseTaken = true; // (from now on the drive reports its status by itself again)
        return cd.CR[3];
    default: return 0;
    }
}

void Machine::CdWrite(uint32 address, uint16 value) {
    auto &cd = m_cd;
    const uint32 auxiliary = address & 0xFFFFF;
    if (auxiliary >= 0x95000 && auxiliary < 0x96000) {
        static const bool log = getenv("SATRECOMP_CD_LOG") != nullptr;
        static unsigned writes = 0;
        if (log && writes++ < 32) {
            fprintf(stderr, "cd aux write %05X = %04X (PC %08X)\n", auxiliary, value, masterSH2.PC);
        }
        cd.aux[(auxiliary - 0x95000) & 0xFFE] = static_cast<uint8>(value >> 8);
        cd.aux[(auxiliary - 0x95000) & 0xFFF] = static_cast<uint8>(value);
        return;
    }
    switch (address & 0x3C) {
    case 0x08:
        cd.HIRQ &= value; // (writing 0 to a bit clears it)
        CdUpdateInterrupt();
        break;
    case 0x0C: {
        static const bool log = getenv("SATRECOMP_CD_LOG") != nullptr;
        if (log) {
            fprintf(stderr, "cd HIRQMASK = %04X (HIRQ %04X), caller PC %08X\n", value, cd.HIRQ, masterSH2.PC);
        }
        cd.HIRQMask = value;
        CdUpdateInterrupt();
        break;
    }
    case 0x18: cd.command[0] = value; break;
    case 0x1C: cd.command[1] = value; break;
    case 0x20: cd.command[2] = value; break;
    case 0x24:
        cd.command[3] = value;
        CdCommand();
        break;
    default: break;
    }
}

// The CD block's interrupt line to the SCU: A-Bus external interrupt 0 while any enabled HIRQ bit is set.
void Machine::CdUpdateInterrupt() {
    if (m_cd.HIRQ & m_cd.HIRQMask) {
        static const bool log = getenv("SATRECOMP_CD_LOG") != nullptr;
        static unsigned raised = 0;
        if (log && raised < 8) {
            fprintf(stderr, "cd interrupt: HIRQ %04X & mask %04X -> A-Bus external 0\n", m_cd.HIRQ, m_cd.HIRQMask);
            ++raised;
        }
        ScuRaise(16);
    }
}

// The usual response: status, what is under the pickup (track 1's control bits, track and index), where
void Machine::CdReport(uint8 status) {
    auto &cd = m_cd;
    if (cd.status == 0x02) { // (stopped: no position)
        cd.CR[0] = static_cast<uint16>((status << 8) | 0xFF);
        cd.CR[1] = cd.CR[2] = cd.CR[3] = 0xFFFF;
        return;
    }
    uint8 track = 1, control = 0x41;
    for (const Disc::Track &t : m_disc.tracks) {
        if (cd.fad >= t.startFAD) {
            track = t.number;
            control = static_cast<uint8>((t.control << 4) | 1);
        }
    }
    cd.CR[0] = static_cast<uint16>((status << 8) | 0x80);
    cd.CR[1] = static_cast<uint16>((control << 8) | track);
    cd.CR[2] = static_cast<uint16>(0x0100 | ((cd.fad >> 16) & 0xFF));
    cd.CR[3] = static_cast<uint16>(cd.fad);
}

void Machine::CdCommand() {
    auto &cd = m_cd;
    const uint8 command = cd.command[0] >> 8;
    cdCommands[command]++;
    ++g_activity;
    uint16 flags = kCMOK;
    switch (command) {
    case 0x00: CdReport(cd.status); break; // status
    case 0x02: { // table of contents: 102 long words at the data port
        std::fill(std::begin(cd.transfer), std::end(cd.transfer), 0xFFFF);
        cd.transferSize = 204;
        cd.transferSectors = 0;
        auto entry = [&](size_t index, uint32 value) {
            cd.transfer[index * 2] = static_cast<uint16>(value >> 16);
            cd.transfer[index * 2 + 1] = static_cast<uint16>(value);
        };
        uint8 first = 1, last = 1, firstControl = 0x41, lastControl = 0x41;
        for (size_t i = 0; i < m_disc.tracks.size(); i++) {
            const Disc::Track &t = m_disc.tracks[i];
            const uint8 control = static_cast<uint8>((t.control << 4) | 1);
            if (t.number >= 1 && t.number <= 99) {
                entry(t.number - 1u, (static_cast<uint32>(control) << 24) | t.startFAD);
            }
            if (i == 0) {
                first = t.number;
                firstControl = control;
            }
            last = t.number;
            lastControl = control;
        }
        entry(99, (static_cast<uint32>(firstControl) << 24) | (static_cast<uint32>(first) << 16));
        entry(100, (static_cast<uint32>(lastControl) << 24) | (static_cast<uint32>(last) << 16));
        entry(101, (static_cast<uint32>(lastControl) << 24) | m_disc.leadOutFAD);
        cd.transferOffset = 0;
        cd.CR[0] = static_cast<uint16>(cd.status << 8);
        cd.CR[1] = 204;
        cd.CR[2] = cd.CR[3] = 0;
        flags |= kDRDY;
        break;
    }
    case 0x03: // session information
        cd.CR[0] = static_cast<uint16>(cd.status << 8);
        cd.CR[1] = 0;
        cd.CR[2] = 0x0100;
        cd.CR[3] = 0;
        break;
    case 0x04: // initialise
        CdReport(cd.status);
        flags |= kESEL;
        break;
    case 0x06: // end of a data transfer: how much was taken
        if (cd.transferSize == 0) {
            cd.CR[0] = static_cast<uint16>((cd.status << 8) | 0xFF);
            cd.CR[1] = 0xFFFF;
        } else {
            cd.CR[0] = static_cast<uint16>((cd.status << 8) | ((cd.transferOffset >> 16) & 0xFF));
            cd.CR[1] = static_cast<uint16>(cd.transferOffset);
        }
        cd.CR[2] = cd.CR[3] = 0;
        cd.transferSize = 0;
        cd.transferOffset = 0;
        cd.transferSectors = 0;
        flags |= kEHST;
        break;
    case 0x10: { // play: a frame address range, a track:index range, or resume
        const uint32 start = ((cd.command[0] & 0xFF) << 16) | cd.command[1];
        const uint32 end = ((cd.command[2] & 0xFF) << 16) | cd.command[3];
        const uint8 mode = static_cast<uint8>(cd.command[2] >> 8);
        static const bool log = getenv("SATRECOMP_CD_LOG") != nullptr;
        if (satrecomp::music::SectorPlayback()) {
            cd.fad = satrecomp::music::CurrentFAD();
        }
        // All-ones parameters resume the saved range without discarding a partial sector.
        if (start == 0xFFFFFF && end == 0xFFFFFF && mode == 0xFF) {
            if (log) {
                fprintf(stderr, "cd 10 resume at FAD %u\n", cd.fad);
            }
            satrecomp::music::Resume();
            cd.status = 0x03;
            cd.reading = true;
            CdReport(cd.status);
            break;
        }
        auto trackStart = [&](uint32 parameter) {
            if (m_disc.tracks.empty()) {
                return uint32{150};
            }
            const uint32 number =
                std::clamp<uint32>((parameter >> 8) & 0xFF, m_disc.tracks.front().number, m_disc.tracks.back().number);
            for (const Disc::Track &t : m_disc.tracks) {
                if (t.number == number) {
                    // (index 0 is the track's pregap, 150 sectors before its index 1)
                    const uint32 index = parameter & 0xFF;
                    return index == 0 && t.startFAD >= 150 ? t.startFAD - 150 : t.startFAD;
                }
            }
            return m_disc.leadOutFAD;
        };
        auto trackEnd = [&](uint32 parameter) {
            if (m_disc.tracks.empty()) {
                return m_disc.leadOutFAD;
            }
            const uint32 number =
                std::clamp<uint32>((parameter >> 8) & 0xFF, m_disc.tracks.front().number, m_disc.tracks.back().number);
            for (const Disc::Track &t : m_disc.tracks) {
                if (t.number != number) {
                    continue;
                }
                // The next track's INDEX 01 leaves a 2-second pregap in the image; the track's own last sector
                // is the end of its audio (CUE files put INDEX 00 at the file start).
                if (t.endFAD >= t.startFAD) {
                    return t.endFAD + 1;
                }
                for (const Disc::Track &next : m_disc.tracks) {
                    if (next.number > t.number) {
                        return next.startFAD;
                    }
                }
                return m_disc.leadOutFAD;
            }
            return m_disc.leadOutFAD;
        };
        uint32 first = start == 0xFFFFFF ? cd.playStart
                       : (start & 0x800000) ? (start & 0x7FFFFF)
                                            : trackStart(start);
        uint32 limit = cd.playEnd;
        if (end != 0xFFFFFF) {
            limit = m_disc.leadOutFAD;
            if (end & 0x800000) {
                limit = first + (end & 0x7FFFFF);
            } else if (end != 0) {
                limit = trackEnd(end);
            }
        }
        limit = std::min(limit, m_disc.leadOutFAD);
        if (mode != 0xFF) {
            cd.playMode = mode;
        }
        const bool continuePosition = mode == 0xFF || (mode & 0x80);
        if (!continuePosition || cd.fad < first || cd.fad >= limit) {
            cd.fad = first;
        }
        cd.playStart = first;
        cd.playEnd = limit;
        satrecomp::music::Stop();
        bool audio = false;
        for (const Disc::Track &t : m_disc.tracks) {
            if (t.startFAD <= cd.fad) {
                audio = !(t.control & 4);
            }
        }
        if (log) {
            fprintf(stderr, "cd 10 play start=%06X end=%06X mode=%02X -> FAD %u..%u (%s), caller PC %08X PR %08X, cycle %llu\n", start,
                    end, mode, first, limit, audio ? "CDDA" : "data", masterSH2.PC, masterSH2.PR,
                    static_cast<unsigned long long>(m_now));
        }
        if (audio) {
            satrecomp::music::PlaySectors(cd.fad, limit, cd.playMode & 0x0F);
            cd.status = 0x03;
            cd.reading = false; // (audio never fills the data FIFO)
        } else {
            cd.status = 0x03;
            cd.reading = true;
        }
        CdReport(0x04); // (it says "seeking" first)
        break;
    }
    case 0x11: { // seek: to a frame address and pause, or stop, or pause where it is
        const uint32 target = ((cd.command[0] & 0xFF) << 16) | cd.command[1];
        static const bool log = getenv("SATRECOMP_CD_LOG") != nullptr;
        if (satrecomp::music::SectorPlayback()) {
            cd.fad = satrecomp::music::CurrentFAD();
        }
        if (log) {
            fprintf(stderr, "cd 11 seek target=%06X (%s) at FAD %u\n", target,
                    target == 0xFFFFFF ? "pause" : target == 0 ? "stop" : "seek", cd.fad);
        }
        cd.reading = false;
        if (target == 0xFFFFFF) {
            satrecomp::music::Pause();
            cd.status = 0x01;
        } else {
            satrecomp::music::Stop();
            cd.status = target == 0 ? 0x02 : 0x01;
            if (target & 0x800000) {
                cd.fad = target & 0x7FFFFF;
            } else if (target != 0) {
                for (const Disc::Track &t : m_disc.tracks) {
                    if (t.number == ((target >> 8) & 0xFF)) {
                        cd.fad = t.startFAD;
                        break;
                    }
                }
            }
        }
        CdReport(cd.status);
        break;
    }
    case 0x30: // which filter the drive's sectors go to
    case 0x42: // a filter's subheader conditions, mode and outputs
    case 0x46:
    case 0x60: // the sector length
        CdReport(cd.status);
        flags |= kESEL;
        break;
    case 0x44: { // a filter's mode
        const uint8 filter = static_cast<uint8>(cd.command[2] >> 8);
        if (filter < 24) {
            cd.filterMode[filter] = static_cast<uint8>(cd.command[0] & 0xFF);
        }
        CdReport(cd.status);
        flags |= kESEL;
        break;
    }
    case 0x40: { // a filter's frame address range
        const uint8 filter = static_cast<uint8>(cd.command[2] >> 8);
        if (filter < 24) {
            cd.filterStart[filter] = ((cd.command[0] & 0xFF) << 16) | cd.command[1];
            cd.filterCount[filter] = ((cd.command[2] & 0xFF) << 16) | cd.command[3];
        }
        CdReport(cd.status);
        flags |= kESEL;
        break;
    }
    case 0x41: { // the range a filter was given (standard status, then the range)
        const uint8 filter = static_cast<uint8>(cd.command[2] >> 8);
        const uint32 start = filter < 24 ? cd.filterStart[filter] : 0;
        const uint32 count = filter < 24 ? cd.filterCount[filter] : 0;
        cd.CR[0] = static_cast<uint16>((cd.status << 8) | ((start >> 16) & 0xFF));
        cd.CR[1] = static_cast<uint16>(start);
        cd.CR[2] = static_cast<uint16>((filter << 8) | ((count >> 16) & 0xFF));
        cd.CR[3] = static_cast<uint16>(count);
        break;
    }
    case 0x43: // a filter's subheader conditions (not applied; read back as none)
    case 0x45: { // a filter's mode
        const uint8 filter = static_cast<uint8>(cd.command[2] >> 8);
        cd.CR[0] = static_cast<uint16>(cd.status << 8);
        cd.CR[1] = filter < 24 ? cd.filterMode[filter] : 0;
        cd.CR[2] = static_cast<uint16>(filter << 8);
        cd.CR[3] = 0;
        break;
    }
    case 0x47: // a filter's connections (not modelled)
        cd.CR[0] = static_cast<uint16>(cd.status << 8);
        cd.CR[1] = cd.CR[2] = cd.CR[3] = 0;
        break;
    case 0x48: // reset the selector: the buffer is emptied
        cd.buffered = 0;
        cd.HIRQ &= ~kBFUL;
        CdReport(cd.status);
        flags |= kESEL;
        break;
    case 0x50: // buffer size: free sectors, filters, sectors in all
        cd.CR[0] = static_cast<uint16>(cd.status << 8);
        cd.CR[1] = static_cast<uint16>(kBufferSectors - cd.buffered);
        cd.CR[2] = 0x1800;
        cd.CR[3] = kBufferSectors;
        break;
    case 0x51: // sectors in a buffer partition
        cd.CR[0] = static_cast<uint16>(cd.status << 8);
        cd.CR[1] = cd.CR[2] = 0;
        cd.CR[3] = static_cast<uint16>(cd.buffered);
        break;
    case 0x52: { // calculate the actual size of sectors: offset, partition, count
        uint32 first = 0, count = 0;
        CdSectorRange(first, count);
        cd.actualSize = count * 1024;
        CdReport(cd.status);
        flags |= kESEL;
        break;
    }
    case 0x53: // the actual size (words)
        cd.CR[0] = static_cast<uint16>((cd.status << 8) | ((cd.actualSize >> 16) & 0xFF));
        cd.CR[1] = static_cast<uint16>(cd.actualSize);
        cd.CR[2] = cd.CR[3] = 0;
        break;
    case 0x54: { // a sector's information: its frame address, file and channel, submode and coding
        uint32 first = 0, count = 0;
        CdSectorRange(first, count);
        const uint32 fad = count ? cd.bufferFAD[first] : 0;
        cd.CR[0] = static_cast<uint16>((cd.status << 8) | ((fad >> 16) & 0xFF));
        cd.CR[1] = static_cast<uint16>(fad);
        cd.CR[2] = cd.CR[3] = 0;
        flags |= kESEL;
        break;
    }
    case 0x61: // sectors' data to the data port
    case 0x62: // sectors deleted from the buffer
    case 0x63: { // both
        uint32 first = 0, count = 0;
        CdSectorRange(first, count);
        if (command != 0x62) {
            std::copy(cd.bufferFAD + first, cd.bufferFAD + first + count, cd.transferFAD);
            cd.transferSectors = count;
            cd.transferSize = count * 1024;
            cd.transferOffset = 0;
            flags |= kDRDY;
        }
        if (command != 0x61) {
            std::copy(cd.bufferFAD + first + count, cd.bufferFAD + cd.buffered, cd.bufferFAD + first);
            cd.buffered -= count;
            cd.HIRQ &= ~kBFUL;
            if (command == 0x62) {
                flags |= kEHST;
            }
        }
        CdReport(cd.status);
        break;
    }
    case 0x75: // abort file
        CdReport(cd.status);
        flags |= kEFLS;
        break;
    case 0xE0: // authenticate the disc
        cd.CR[0] = static_cast<uint16>((cd.status << 8) | 0xFF);
        cd.CR[1] = cd.CR[2] = cd.CR[3] = 0xFFFF;
        flags |= kEFLS;
        break;
    default: {
        static bool reported[256];
        if (!reported[command]) {
            reported[command] = true;
            fprintf(stderr, "CD drive: command %02X is not one this machine knows (%04X %04X %04X %04X)\n", command, cd.command[0],
                    cd.command[1], cd.command[2], cd.command[3]);
        }
        CdReport(cd.status);
        break;
    }
    }
    cd.HIRQ |= flags;
    cd.responseTaken = false;
    CdUpdateInterrupt();
    // Debugging aid (SATRECOMP_CD_TRACE=1): every command and the response
    static const bool trace = getenv("SATRECOMP_CD_TRACE") != nullptr;
    if (trace) {
        fprintf(stderr, "cd cmd %04X %04X %04X %04X -> %04X %04X %04X %04X (HIRQ %04X)\n", cd.command[0], cd.command[1], cd.command[2],
                cd.command[3], cd.CR[0], cd.CR[1], cd.CR[2], cd.CR[3], cd.HIRQ);
    }
}

// 150 times a second while the drive reads (data is read at double speed): a sector goes into the buffer,
// until that is full or the end of what was asked for is reached
void Machine::CdPeriodic() {
    auto &cd = m_cd;
    cd.periodicDue += m_clock / 150;
    cd.HIRQ |= kSCDQ; // (with every periodic report, the drive stopped too: Fighters Megamix's file library waits for it)
    // CD-DA advances at the sound generator's 44.1 kHz sample clock, not the data drive's 150-sector/second
    // buffer clock. It never fills the data FIFO.
    if (satrecomp::music::SectorPlayback()) {
        cd.fad = satrecomp::music::CurrentFAD();
        if (satrecomp::music::GetState() != satrecomp::music::State::Playing && cd.status == 0x03) {
            // (the end of what was asked for, or a pause: the drive stops there and says so)
            cd.status = 0x01;
            cd.HIRQ |= kPEND;
            ++g_activity;
        }
        if (cd.responseTaken) {
            CdReport(cd.status | 0x20); // the periodic report (marked as one)
        }
        CdUpdateInterrupt();
        return;
    }
    if (cd.responseTaken) {
        CdReport(cd.status | 0x20); // the periodic report (marked as one)
    }
    if (!cd.reading) {
        CdUpdateInterrupt();
        return;
    }
    if (cd.fad >= cd.playEnd) {
        cd.reading = false;
        cd.status = 0x01;
        cd.HIRQ |= kPEND;
        ++g_activity;
        CdUpdateInterrupt();
        return;
    }
    cd.bufferFAD[cd.buffered++] = cd.fad;
    cd.fad++;
    cd.HIRQ |= kCSCT;
    if (cd.fad >= cd.playEnd) {
        // (the last sector asked for: the drive pauses there at once, as Ymir's does; Fighters Megamix's movie
        // player looks at the status straight after taking that sector)
        cd.reading = false;
        cd.status = 0x01;
        cd.HIRQ |= kPEND;
    } else if (cd.buffered >= kBufferSectors) {
        cd.reading = false; // (it waits there, still "playing", until the buffer is emptied)
        cd.HIRQ |= kBFUL;
    }
    ++g_activity;
    CdUpdateInterrupt();
}

// The sectors a command names: CR2 the first (0xFFFF: the last one), CR4 how many (0xFFFF: to the end), in
// the buffer as it is (one partition)
void Machine::CdSectorRange(uint32 &first, uint32 &count) {
    auto &cd = m_cd;
    const uint32 offset = cd.command[1], wanted = cd.command[3];
    first = offset == 0xFFFF ? (cd.buffered ? cd.buffered - 1 : 0) : std::min<uint32>(offset, cd.buffered);
    count = wanted == 0xFFFF ? cd.buffered - first : std::min<uint32>(wanted, cd.buffered - first);
}

} // namespace satrecomp::machine
