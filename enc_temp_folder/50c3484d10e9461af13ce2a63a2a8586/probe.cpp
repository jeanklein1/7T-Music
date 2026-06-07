// probe.cpp ─────────────────────────────────────────────────────────
//
// Reading probe. One rung past midi_monitor: incoming events now feed the
// real per-channel Context, and we print what the substrate actually sees —
// the Playhead (present set) when it changes, and once per beat the Wagon's
// completed-in-window notes. No analysis operations yet; this confirms the
// substrate reads correctly from live MIDI before layer B is built.
//
// RtMidi is the only non-header dependency — no Dawn, no ImGui. Adjust the
// include paths to your tree if needed. Run on the machine with Ableton +
// loopMIDI:
//     ./probe            (opens the first "loopMIDI" port)
//     ./probe 0          (opens port index 0)

#include "sources/midi_port.hpp"
#include "sources/midi_event.hpp"
#include "core/clock.hpp"
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
    static const char* n[] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
    return std::string(n[((midi % 12) + 12) % 12]) + std::to_string(midi / 12 - 1);
}

// ── Probe configuration ──────────────────────────────────────────────
constexpr int   PROBE_CHANNELS = 16;     // one Context per MIDI channel
constexpr float PROBE_WAGON_SPAN = 4.0f;   // window span, beats (a 4/4 bar)
constexpr float PROBE_RETENTION = 16.0f;  // completed-history kept, beats
constexpr float PROBE_BPM = 120.0f; // set to your Ableton tempo

int main(int argc, char** argv) {
    std::cout << std::unitbuf;   // live, line-by-line output

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
    std::cout << "  PLAYHEAD  the present, @instant       "
        "WAGON  a window [t0..t1]; each note carries its (in-window length)\n\n";

    Clock clock;
    clock.set_bpm(PROBE_BPM);   // free-running local clock, not Ableton's transport

    // One Context per channel, each with a single bar-length Wagon.
    std::array<Context, PROBE_CHANNELS> ctx;
    for (int c = 0; c < PROBE_CHANNELS; ++c) {
        ctx[c].set_channel(c);
        ctx[c].set_retention_beats(PROBE_RETENTION);
        ctx[c].add_wagon(PROBE_WAGON_SPAN);
    }

    // Change detection: last printed present-mask, and last wagon size, per channel.
    std::array<PitchBitmask, PROBE_CHANNELS> last_present{};
    std::array<int, PROBE_CHANNELS> last_wagon{};
    last_wagon.fill(-1);

    using clk = std::chrono::steady_clock;
    auto prev = clk::now();
    int last_beat = -1;
    MidiEvent ev[256];

    while (true) {
        const auto now = clk::now();
        clock.tick(std::chrono::duration<float>(now - prev).count());
        prev = now;
        const float beat = clock.t_beats();

        // Route each event to its channel's Context.
        const int n = port.poll(beat, ev, 256);
        for (int i = 0; i < n; ++i) {
            const MidiEvent& e = ev[i];
            if (e.channel < PROBE_CHANNELS) ctx[e.channel].receive(e);
        }

        // Advance every Context: snapshot the present, rebuild the windows.
        for (int c = 0; c < PROBE_CHANNELS; ++c) ctx[c].update(beat);

        // Present set — print on change (never per frame).
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

        // Wagon — once per beat, for any channel whose window is non-empty.
        const int ibeat = static_cast<int>(beat);
        if (ibeat != last_beat) {
            last_beat = ibeat;
            for (int c = 0; c < PROBE_CHANNELS; ++c) {
                const WagonReadout& wg = ctx[c].wagon(0);
                if (wg.note_count == 0) { last_wagon[c] = 0; continue; }
                last_wagon[c] = wg.note_count;
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