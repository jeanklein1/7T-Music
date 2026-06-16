// probe.cpp ─────────────────────────────────────────────────────────
//
// DAW-synced reading probe. Incoming events feed the per-channel Context;
// musical time comes from the DAW (the MidiPort counts MIDI timing-clock
// pulses into a beat position), so the Wagon's windows fall on Ableton's
// beats and a stop freezes them.
//
// Channels are read across at one instant — side by side, separated by the
// pipe, label and beat stated once at the head, a one-time header naming
// the columns. `-` is an empty collection.
//
//   PLAYHEAD   the present set, on present-change
//      WAGON   completed notes in the window, name(in-window length), per beat
//   TRANSPORT  the DAW play state + tempo (on change)
//
// Needs RtMidi. In Ableton, enable this port's Clock/Sync output, then play.
//   ./probe            (opens first "loopMIDI" port)
//   ./probe 0          (opens port index 0)

#include "sources/midi_port.hpp"
#include "sources/midi_event.hpp"
#include "musical/context.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

using namespace t7;

static std::string note_name(int midi) {
    static const char* n[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    return std::string(n[((midi % 12) + 12) % 12]) + std::to_string(midi / 12 - 1);
}

constexpr int   N_CHANNELS       = 2;
constexpr int   CHANNEL_W        = 30;     // field per channel (note lists)
constexpr float PROBE_WAGON_SPAN = 4.0f;   // window span, beats
constexpr float PROBE_RETENTION  = 16.0f;  // completed-history kept, beats
static const char* SEP = " | ";

// ── Cell builders ────────────────────────────────────────────────────

static std::string present_list(const PlayheadReadout& ph) {
    if (ph.current_count == 0) return "-";
    std::string s;
    for (int i = 0; i < ph.current_count; ++i) { if (i) s += ' '; s += note_name(ph.current[i].pitch); }
    if (ph.has_overflow()) s += " (+more)";
    return s;
}
static std::string window_list(const WagonReadout& wg) {
    if (wg.note_count == 0) return "-";
    std::string s; char d[16];
    for (int i = 0; i < wg.note_count; ++i) {
        if (i) s += ' ';
        std::snprintf(d, sizeof d, "%.2f", wg.notes[i].window_duration());
        s += note_name(wg.notes[i].pitch); s += '('; s += d; s += ')';
    }
    if (wg.has_overflow()) s += " (+more)";
    return s;
}

// ── Rendering ────────────────────────────────────────────────────────

static void print_header() {
    std::printf("%-16s ", "");
    for (int c = 0; c < N_CHANNELS; ++c) {
        if (c) std::printf("%s", SEP);
        char t[8]; std::snprintf(t, sizeof t, "ch%d", c);
        int len = (int)std::strlen(t), pad = (CHANNEL_W - len) / 2;
        for (int i = 0; i < pad; ++i)                   { std::printf(" "); }
        std::printf("%s", t);
        for (int i = 0; i < CHANNEL_W - len - pad; ++i) { std::printf(" "); }
    }
    std::printf("\n");
}
static void print_row(const char* label, float beat, const std::string* cells) {
    std::printf("%-8s@%-7.2f ", label, beat);
    for (int c = 0; c < N_CHANNELS; ++c) { if (c) std::printf("%s", SEP); std::printf("%-*s", CHANNEL_W, cells[c].c_str()); }
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
        std::cout << "\nFailed to open a port. Pass an index, e.g.  ./probe 0\n";
        return 1;
    }
    std::cout << "\nListening on: " << port.port_name() << "   (Ctrl-C to stop)\n";
    std::cout << "  PLAYHEAD present, on change   WAGON window, name(length), per beat   `-` = empty\n";
    std::cout << "  Waiting for MIDI clock — enable this port's Sync output in Ableton and press play.\n\n";

    std::array<Context, N_CHANNELS> ctx;
    for (int c = 0; c < N_CHANNELS; ++c) {
        ctx[c].set_channel(c);
        ctx[c].set_retention_beats(PROBE_RETENTION);
        ctx[c].add_wagon(PROBE_WAGON_SPAN);
    }

    print_header();

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
            char line[64];
            std::snprintf(line, sizeof line, "TRANSPORT  %-4s  @%-8.2f bpm %.1f",
                          last_playing ? "play" : "stop", beat, port.bpm());
            std::cout << line << "\n";
        }

        // PLAYHEAD — present sets, reprinted when any channel's present changes.
        bool present_changed = false;
        for (int c = 0; c < N_CHANNELS; ++c) {
            const PitchBitmask& m = ctx[c].playhead().current_mask;
            if (m.lo != last_present[c].lo || m.hi != last_present[c].hi) {
                present_changed = true;
                last_present[c] = m;
            }
        }
        if (present_changed) {
            std::string cells[N_CHANNELS];
            for (int c = 0; c < N_CHANNELS; ++c) cells[c] = present_list(ctx[c].playhead());
            print_row("PLAYHEAD", beat, cells);
        }

        // WAGON — window note-lists, once per beat, every channel.
        const int ibeat = static_cast<int>(beat);
        if (ibeat != last_beat) {
            last_beat = ibeat;
            std::string cells[N_CHANNELS];
            for (int c = 0; c < N_CHANNELS; ++c) cells[c] = window_list(ctx[c].wagon(0));
            print_row("WAGON", beat, cells);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}
