// probe_lowest.cpp ───────────────────────────────────────────────────
//
// DAW-synced SELECTION probe — Playhead source. It elects the lowest note
// sounding now, the bass, and holds it forward as a reign: the harmonic
// ground for the bars ahead, not a measurement of the past. A live note
// thrones and resets the clock; silence holds the ground for
// PROBE_REIGN_DECAY beats; once the reign is over the frame falls back to the
// home — a reference must always have a value for what it grounds.
//
// Scope is the present (the Playhead) only. The window (Wagon) is a different
// frame and is left out here; frames are chosen, not combined.
//
//   REIGN      the present's lowest, held forward by the decaying latch, with
//              the composition's home (a D) as fallback. A live or held
//              election prints plain (C2); the fallback prints in parens (D2).
//              Printed when the held note changes (election, abdication, or
//              the return to home).
//   TRANSPORT  the DAW play state + tempo (on change)
//
// Channels side by side under a one-time header, label and beat at the head.
//
// Needs RtMidi. In Ableton, enable this port's Clock/Sync output, then play.
//   ./probe_lowest            (opens first "loopMIDI" port)
//   ./probe_lowest 0          (opens port index 0)

#include "sources/midi_port.hpp"
#include "sources/midi_event.hpp"
#include "musical/context.hpp"
#include "musical/note_select.hpp"
#include "musical/reign.hpp"

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

constexpr int   N_CHANNELS         = 2;
constexpr int   CHANNEL_W          = 7;      // field per channel (one note, room for parens)
constexpr float PROBE_RETENTION    = 16.0f;  // completed-history kept, beats
constexpr float PROBE_REIGN_DECAY  = 2.0f;   // beats of silence the reign survives
constexpr int   PROBE_FALLBACK_PITCH = 38;   // D2 — the composition's home (another criterion)
static const char* SEP = " | ";

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
        std::cout << "\nFailed to open a port. Pass an index, e.g.  ./probe_lowest 0\n";
        return 1;
    }
    std::cout << "\nListening on: " << port.port_name() << "   (Ctrl-C to stop)\n";
    std::cout << "  REIGN  present's lowest, held " << PROBE_REIGN_DECAY
              << " beats then home " << note_name(PROBE_FALLBACK_PITCH)
              << " in parens   (Playhead source only)\n";
    std::cout << "  Waiting for MIDI clock — enable this port's Sync output in Ableton and press play.\n\n";

    std::array<Context, N_CHANNELS> ctx;
    std::array<Reign<CurrentNote>, N_CHANNELS> reign;
    CurrentNote home{};
    home.pitch = static_cast<uint8_t>(PROBE_FALLBACK_PITCH);
    for (int c = 0; c < N_CHANNELS; ++c) {
        ctx[c].set_channel(c);
        ctx[c].set_retention_beats(PROBE_RETENTION);
        reign[c].set_decay(PROBE_REIGN_DECAY);
        reign[c].set_fallback(home);
    }

    print_header();

    struct Shown { bool reigning = false; bool has = false; uint8_t pitch = 0; };
    std::array<Shown, N_CHANNELS> last_shown{};
    bool announced = false;
    bool last_playing = false;
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

        // REIGN — feed each channel's present-lowest into its decaying latch
        // every frame (abdication and the return to home are time-based, not
        // event-based), and reprint when any channel's frame changes — whether
        // a new election, the reign expiring, or the home reasserting.
        bool changed = false;
        for (int c = 0; c < N_CHANNELS; ++c) {
            const PlayheadReadout& ph = ctx[c].playhead();
            reign[c].update(select_lowest(ph.current.data(), ph.current_count), beat);
            const auto nt = reign[c].note();
            Shown cur{ reign[c].reigning(), nt.has_value(), nt ? nt->pitch : (uint8_t)0 };
            if (cur.reigning != last_shown[c].reigning ||
                cur.has      != last_shown[c].has      ||
                cur.pitch    != last_shown[c].pitch) {
                changed = true;
                last_shown[c] = cur;
            }
        }
        if (changed) {
            std::string cells[N_CHANNELS];
            for (int c = 0; c < N_CHANNELS; ++c) {
                const auto nt = reign[c].note();
                if (!nt)                      cells[c] = "-";
                else if (reign[c].reigning()) cells[c] = note_name(nt->pitch);
                else                          cells[c] = "(" + note_name(nt->pitch) + ")";
            }
            print_row("REIGN", beat, cells);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}
