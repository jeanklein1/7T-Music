// probe.cpp ─────────────────────────────────────────────────────────
//
// DAW-synced reading probe. Incoming events feed the per-channel Context;
// musical time comes from the DAW, not a local clock — the MidiPort counts
// MIDI timing-clock pulses (24/quarter) into a beat position, so the Wagon's
// windows fall on Ableton's beats and a stop freezes them.
//
// Three self-identifying line types:
//   TRANSPORT  the DAW play state + tempo (prints on change)
//   PLAYHEAD   the present set, @instant
//   WAGON      completed notes in a window [t0..t1], each with (in-window length)
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
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

using namespace t7;

static std::string note_name(int midi) {
    static const char* n[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    return std::string(n[((midi % 12) + 12) % 12]) + std::to_string(midi / 12 - 1);
}

constexpr int   PROBE_CHANNELS   = 16;
constexpr float PROBE_WAGON_SPAN = 4.0f;   // window span, beats
constexpr float PROBE_RETENTION  = 16.0f;  // completed-history kept, beats

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
    std::cout << "  PLAYHEAD  present @instant      WAGON  window [t0..t1], notes carry (in-window length)\n";
    std::cout << "  Waiting for MIDI clock — enable this port's Sync output in Ableton and press play.\n\n";

    std::array<Context, PROBE_CHANNELS> ctx;
    for (int c = 0; c < PROBE_CHANNELS; ++c) {
        ctx[c].set_channel(c);
        ctx[c].set_retention_beats(PROBE_RETENTION);
        ctx[c].add_wagon(PROBE_WAGON_SPAN);
    }

    std::array<PitchBitmask, PROBE_CHANNELS> last_present{};
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
            if (e.channel < PROBE_CHANNELS) ctx[e.channel].receive(e);
        }
        for (int c = 0; c < PROBE_CHANNELS; ++c) ctx[c].update(beat);

        // TRANSPORT — on play/stop change.
        if (port.playing() != last_playing) {
            last_playing = port.playing();
            char line[64];
            std::snprintf(line, sizeof line, "TRANSPORT  %-4s  @%-8.2f bpm %.1f",
                          last_playing ? "play" : "stop", beat, port.bpm());
            std::cout << line << "\n";
        }

        // PLAYHEAD — present set, on change.
        for (int c = 0; c < PROBE_CHANNELS; ++c) {
            const PlayheadReadout& ph = ctx[c].playhead();
            const PitchBitmask& m = ph.current_mask;
            if (m.lo != last_present[c].lo || m.hi != last_present[c].hi) {
                last_present[c] = m;
                char when[24];
                std::snprintf(when, sizeof when, "@%.2f", beat);
                std::cout << "PLAYHEAD  ch" << std::left << std::setw(3) << c
                          << std::setw(14) << when << std::right;
                if (ph.current_count == 0) std::cout << "-";
                else for (int k = 0; k < ph.current_count; ++k) {
                    if (k) std::cout << ' ';
                    std::cout << note_name(ph.current[k].pitch);
                }
                if (ph.has_overflow()) std::cout << "  (+more)";
                std::cout << "\n";
            }
        }

        // WAGON — once per beat, non-empty windows.
        const int ibeat = static_cast<int>(beat);
        if (ibeat != last_beat) {
            last_beat = ibeat;
            for (int c = 0; c < PROBE_CHANNELS; ++c) {
                const WagonReadout& wg = ctx[c].wagon(0);
                if (wg.note_count == 0) continue;
                char span[32];
                std::snprintf(span, sizeof span, "[%.1f..%.1f]", wg.window_start, wg.window_end);
                std::cout << "   WAGON  ch" << std::left << std::setw(3) << c
                          << std::setw(14) << span << std::right;
                for (int k = 0; k < wg.note_count; ++k) {
                    const WindowNote& wn = wg.notes[k];
                    char d[16];
                    std::snprintf(d, sizeof d, "%.2f", wn.window_duration());
                    if (k) std::cout << "  ";
                    std::cout << note_name(wn.pitch) << "(" << d << ")";
                }
                if (wg.has_overflow()) std::cout << "  (+more)";
                std::cout << "\n";
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}
