// probe_vector_count.cpp ──────────────────────────────────────────────────
//
// DAW-synced VECTOR probe. Same wiring as probe.cpp (MIDI clock → beat →
// per-channel Context), but instead of listing notes it prints the
// layer-B pitch-class COUNT vector for each view. The raw-reading probe
// stays untouched; this is the second diagnostic, for the vector
// representation.
//
// Layout: the N channels are read ACROSS at one instant — one row, the
// channels side by side as fixed positions (a one-time header names them).
// Label and beat state once at the head; channel identity is the column,
// not a repeated tag.
//
//   PLAYHEAD — notes sounding now, per pc; reprints when any channel's
//              present changes (its own cadence).
//   WAGON    — completed occurrences in the window, per pc; once per beat,
//              every channel shown (empty windows print as dots).
//
// Reading channels side by side under one beat is honest: they all come
// from the same poll at the same instant. (What is NOT co-sampled is the
// present vs the window — those keep separate cadences, as in probe.cpp.)
//
// Needs RtMidi. In Ableton, enable this port's Clock/Sync output, then play.
//   ./probe_vector_count            (opens first "loopMIDI" port)
//   ./probe_vector_count 0          (opens port index 0)

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
constexpr float PROBE_WAGON_SPAN = 4.0f;   // window span, beats
constexpr float PROBE_RETENTION  = 16.0f;  // completed-history kept, beats

// ── Rendering ────────────────────────────────────────────────────────

// 12-column pitch-class group for one channel.
static void print_group(const PitchClassVector& v) {
    for (int i = 0; i < 12; ++i) {
        if (v.v[i] == 0.0f) std::printf("  . ");
        else                std::printf(" %2.0f ", v.v[i]);
    }
}

// One row: label + beat once at the head, then the channels by position.
static void print_row(const char* label, float beat, const PitchClassVector* vs) {
    std::printf("%-8s@%-7.2f ", label, beat);
    for (int c = 0; c < N_CHANNELS; ++c) { if (c) std::printf("%s", SEP); print_group(vs[c]); }
    std::printf("\n");
}

// One-time headers: channel ids over pitch-class names.
static void print_headers() {
    std::printf("%-16s ", "");
    for (int c = 0; c < N_CHANNELS; ++c) {
        if (c) std::printf("%s", SEP);
        char t[8]; std::snprintf(t, sizeof t, "ch%d", c);
        int len = (int)std::strlen(t), pad = (48 - len) / 2;
        for (int i = 0; i < pad; ++i) std::printf(" ");
        std::printf("%s", t);
        for (int i = 0; i < 48 - len - pad; ++i) std::printf(" ");
    }
    std::printf("\n");
    std::printf("%-16s ", "");
    for (int c = 0; c < N_CHANNELS; ++c) {
        if (c) std::printf("%s", SEP);
        for (int i = 0; i < 12; ++i) std::printf("%3s ", PC_NAME[i]);
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
        std::cout << "\nFailed to open a port. Pass an index, e.g.  ./probe_vector_count 0\n";
        return 1;
    }
    std::cout << "\nListening on: " << port.port_name() << "   (Ctrl-C to stop)\n";
    std::cout << "  pitch-class COUNT vectors, C=0, octaves folded; " << N_CHANNELS
              << " channels read across at one instant.\n";
    std::cout << "  PLAYHEAD = sounding now, on present-change   WAGON = window, once per beat\n";
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
        if (present_changed) {
            PitchClassVector vs[N_CHANNELS];
            for (int c = 0; c < N_CHANNELS; ++c) vs[c] = pc_count(ctx[c].playhead());
            print_row("PLAYHEAD", beat, vs);
        }

        // WAGON — once per beat, every channel (empty windows as dots).
        const int ibeat = static_cast<int>(beat);
        if (ibeat != last_beat) {
            last_beat = ibeat;
            PitchClassVector vs[N_CHANNELS];
            for (int c = 0; c < N_CHANNELS; ++c) vs[c] = pc_count(ctx[c].wagon(0));
            print_row("WAGON", beat, vs);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}
