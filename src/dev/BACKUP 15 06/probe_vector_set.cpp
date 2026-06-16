// probe_vector_set.cpp ───────────────────────────────────────────────
//
// DAW-synced VECTOR probe (pitch-class set). Identical wiring to
// probe_vector_count; it prints the SET rather than the count — the
// support of the count, a one in a cell iff that class occurred at all.
// Multiplicity collapses: where the count read 2 or 5, the set reads 1.
//
// The set derives from the count (pc_set(pc_count(view))), so the
// dominance order is visible at the call site: count first, then its
// support.
//
//   PLAYHEAD — set of classes sounding now; on present-change.
//   WAGON    — set of classes that occurred in the window; once per beat,
//              every channel shown (empty windows print as dots).
//
// Needs RtMidi. In Ableton, enable this port's Clock/Sync output, then play.
//   ./probe_vector_set            (opens first "loopMIDI" port)
//   ./probe_vector_set 0          (opens port index 0)

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

constexpr int   N_CHANNELS       = 2;      // channels read across, side by side
constexpr float PROBE_WAGON_SPAN = 4.0f;   // window span, beats
constexpr float PROBE_RETENTION  = 16.0f;  // completed-history kept, beats

// ── Rendering ────────────────────────────────────────────────────────

// 12-column set group for one channel: a one where the class is in the set.
static void print_set_group(const PitchClassBits& s) {
    for (int i = 0; i < 12; ++i) std::printf(s.test(i) ? "  1 " : "  . ");
}

// One row: label + beat once at the head, then the channels by position.
static void print_set_row(const char* label, float beat, const PitchClassBits* sets) {
    std::printf("%-8s@%-7.2f ", label, beat);
    for (int c = 0; c < N_CHANNELS; ++c) { if (c) std::printf("%s", SEP); print_set_group(sets[c]); }
    std::printf("\n");
}

// One-time headers: channel ids over pitch-class names.
static void print_headers() {
    std::printf("%-16s ", "");
    for (int c = 0; c < N_CHANNELS; ++c) {
        if (c) std::printf("%s", SEP);
        char t[8]; std::snprintf(t, sizeof t, "ch%d", c);
        int len = (int)std::strlen(t), pad = (48 - len) / 2;
        for (int i = 0; i < pad; ++i)            { std::printf(" "); }
        std::printf("%s", t);
        for (int i = 0; i < 48 - len - pad; ++i) { std::printf(" "); }
    }
    std::printf("\n");
    std::printf("%-16s ", "");
    for (int c = 0; c < N_CHANNELS; ++c) {
        if (c) std::printf("%s", SEP);
        for (int i = 0; i < 12; ++i) { std::printf("%3s ", PC_NAME[i]); }
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
        std::cout << "\nFailed to open a port. Pass an index, e.g.  ./probe_vector_set 0\n";
        return 1;
    }
    std::cout << "\nListening on: " << port.port_name() << "   (Ctrl-C to stop)\n";
    std::cout << "  pitch-class SET (support of the count): a one iff the class occurred.\n";
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

        // PLAYHEAD — set of the present, reprinted when any channel changes.
        bool present_changed = false;
        for (int c = 0; c < N_CHANNELS; ++c) {
            const PitchBitmask& m = ctx[c].playhead().current_mask;
            if (m.lo != last_present[c].lo || m.hi != last_present[c].hi) {
                present_changed = true;
                last_present[c] = m;
            }
        }
        if (present_changed) {
            PitchClassBits sets[N_CHANNELS];
            for (int c = 0; c < N_CHANNELS; ++c) sets[c] = pc_set(pc_count(ctx[c].playhead()));
            print_set_row("PLAYHEAD", beat, sets);
        }

        // WAGON — set of the window, once per beat, every channel.
        const int ibeat = static_cast<int>(beat);
        if (ibeat != last_beat) {
            last_beat = ibeat;
            PitchClassBits sets[N_CHANNELS];
            for (int c = 0; c < N_CHANNELS; ++c) sets[c] = pc_set(pc_count(ctx[c].wagon(0)));
            print_set_row("WAGON", beat, sets);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}
