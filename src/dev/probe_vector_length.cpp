// probe_vector_length.cpp ────────────────────────────────────────────
//
// DAW-synced VECTOR probe (count + cumulative length). Identical wiring to
// probe_vector_count, with one change: the WAGON publishes, per pitch class,
// a tuple — the count and the cumulative in-window length of the occurrences.
// A C heard twice, 1.00 and 0.50 beats inside the window, reads (2, 1.50).
//
// Length is wagon-only by nature: a completed note has a final length, a
// sounding one only a provisional anchor−onset. So the PLAYHEAD row stays
// count, the WAGON row carries the (count, length) pair. The two reductions
// are one operation, two weights — pc_count (weight 1) and pc_length
// (weight = clipped span).
//
//   PLAYHEAD — count of notes sounding now, per pc; on present-change.
//   WAGON    — (count, cumulative length) per pc; once per beat, every
//              channel shown (empty windows print as dots).
//
// Needs RtMidi. In Ableton, enable this port's Clock/Sync output, then play.
//   ./probe_vector_length            (opens first "loopMIDI" port)
//   ./probe_vector_length 0          (opens port index 0)

#include "sources/midi_port.hpp"
#include "sources/midi_event.hpp"
#include "musical/context.hpp"
#include "musical/pc_count.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

using namespace t7;

static const char* PC_NAME[] =
    {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
static const char* SEP = " | ";   // channel boundary

constexpr int   N_CHANNELS       = 3;      // channels read across, side by side
constexpr int   CELL             = 10;     // column width (fits "(c, l.ll)")
constexpr float PROBE_WAGON_SPAN = 4.0f;   // window span, beats
constexpr float PROBE_RETENTION  = 16.0f;  // completed-history kept, beats

// ── Cell formatting ──────────────────────────────────────────────────

static void cell_count(char* b, size_t n, float c) {
    if (c == 0.0f) std::snprintf(b, n, ".");
    else           std::snprintf(b, n, "%d", static_cast<int>(c));
}
static void cell_tuple(char* b, size_t n, float c, float l) {
    if (c == 0.0f) std::snprintf(b, n, ".");
    else           std::snprintf(b, n, "(%d, %.2f)", static_cast<int>(c), l);
}

// ── Rendering ────────────────────────────────────────────────────────

// One-time headers: channel ids over pitch-class names.
static void print_headers() {
    std::printf("%-16s ", "");
    const int gw = 12 * CELL;
    for (int c = 0; c < N_CHANNELS; ++c) {
        if (c) std::printf("%s", SEP);
        char t[8]; std::snprintf(t, sizeof t, "ch%d", c);
        int len = (int)std::strlen(t), pad = (gw - len) / 2;
        for (int i = 0; i < pad; ++i)            { std::printf(" "); }
        std::printf("%s", t);
        for (int i = 0; i < gw - len - pad; ++i) { std::printf(" "); }
    }
    std::printf("\n");
    std::printf("%-16s ", "");
    for (int c = 0; c < N_CHANNELS; ++c) {
        if (c) std::printf("%s", SEP);
        for (int i = 0; i < 12; ++i) { std::printf("%-*s", CELL, PC_NAME[i]); }
    }
    std::printf("\n");
}

// PLAYHEAD row: count per pc, present count for each channel.
static void print_playhead(const std::array<Context, N_CHANNELS>& ctx, float beat) {
    std::printf("%-8s@%-7.2f ", "PLAYHEAD", beat);
    for (int c = 0; c < N_CHANNELS; ++c) {
        if (c) std::printf("%s", SEP);
        PitchClassVector v = pc_count(ctx[c].playhead());
        for (int i = 0; i < 12; ++i) { char b[16]; cell_count(b, sizeof b, v.v[i]); std::printf("%-*s", CELL, b); }
    }
    std::printf("\n");
}

// WAGON row: (count, cumulative length) per pc, windowed for each channel.
static void print_wagon(const std::array<Context, N_CHANNELS>& ctx, float beat) {
    std::printf("%-8s@%-7.2f ", "WAGON", beat);
    for (int c = 0; c < N_CHANNELS; ++c) {
        if (c) std::printf("%s", SEP);
        const WagonReadout& wg = ctx[c].wagon(0);
        PitchClassVector cnt = pc_count(wg);
        PitchClassVector len = pc_length(wg);
        for (int i = 0; i < 12; ++i) { char b[16]; cell_tuple(b, sizeof b, cnt.v[i], len.v[i]); std::printf("%-*s", CELL, b); }
    }
    std::printf("\n");
}

int main(int argc, char** argv) {
    std::cout << std::unitbuf;

    MidiPort port;
    auto ports = port.enumerate();
    std::cout << "MIDI inputs:\n";
    for (size_t i = 0; i < ports.size(); ++i)
        std::cout << "  [" << i << "] " << ports[i] << "\n";
    if (ports.empty()) {
        std::cout << "  (none - is loopMIDI running and Ableton routed to it?)\n";
        return 1;
    }

    const bool opened = (argc > 1)
        ? port.open(static_cast<unsigned>(std::stoi(argv[1])))
        : port.open_by_name("loopMIDI");
    if (!opened) {
        std::cout << "\nFailed to open a port. Pass an index, e.g.  ./probe_vector_length 0\n";
        return 1;
    }
    std::cout << "\nListening on: " << port.port_name() << "   (Ctrl-C to stop)\n";
    std::cout << "  PLAYHEAD = count of notes sounding now, per pc, on present-change.\n";
    std::cout << "  WAGON = (count, cumulative in-window length) per pc, once per beat.\n";
    std::cout << "  Waiting for MIDI clock — enable this port's Sync output in Ableton and press play.\n\n";

    std::array<Context, N_CHANNELS> ctx;
    for (int c = 0; c < N_CHANNELS; ++c) {
        ctx[c].set_channel(c);
        ctx[c].set_retention_beats(PROBE_RETENTION);
        ctx[c].add_wagon(PROBE_WAGON_SPAN);
    }

    print_headers();

    std::array<PitchBitmask, N_CHANNELS> last_present{};
    bool announced = false;
    bool last_playing = false;
    int  last_beat = -1;
    MidiEvent ev[256];

    while (true) {
        if (!announced && port.ever_synced()) {
            announced = true;
            std::cout << "  -- MIDI clock detected --\n";
        }

        const float beat = static_cast<float>(port.beats());

        const int n = port.poll(beat, ev, 256);
        for (int i = 0; i < n; ++i) {
            const MidiEvent& e = ev[i];
            if (e.channel < N_CHANNELS) ctx[e.channel].receive(e);
        }
        for (int c = 0; c < N_CHANNELS; ++c) ctx[c].update(beat);

        // TRANSPORT — on play/stop change.
        if (port.playing() != last_playing) {
            last_playing = port.playing();
            std::printf("TRANSPORT  %-4s  @%.2f  bpm %.1f\n",
                        last_playing ? "play" : "stop", beat, port.bpm());
        }

        // PLAYHEAD — reprint the full row when any channel's present changes.
        bool present_changed = false;
        for (int c = 0; c < N_CHANNELS; ++c) {
            const PitchBitmask& m = ctx[c].playhead().current_mask;
            if (m.lo != last_present[c].lo || m.hi != last_present[c].hi) {
                present_changed = true;
                last_present[c] = m;
            }
        }
        if (present_changed) print_playhead(ctx, beat);

        // WAGON — once per beat, every channel (empty windows as dots).
        const int ibeat = static_cast<int>(beat);
        if (ibeat != last_beat) {
            last_beat = ibeat;
            print_wagon(ctx, beat);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}
