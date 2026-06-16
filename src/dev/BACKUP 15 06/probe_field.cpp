// probe_field.cpp ───────────────────────────────────────────────────────────
//
// DAW-synced FIELD probe. Same wiring as the rest of the family (MIDI clock ->
// beat -> per-channel Context). The reading is the barest harmonic location:
// take which pitch classes are sounding -- present or in the window -- as a
// BINARY set (no weight, no time), re-origin onto D so they read as degrees, and
// dot against each declared field. Each number is the count of sounding degrees
// that belong to that field: the set's coordinates in field-space.
//
// The bank below is the composer's declared slot. Sampled once per beat; a
// channel's line reprints only when its sounding set changes. ASCII only.
//
// Needs RtMidi. In Ableton, enable this port's Clock/Sync output, then play.
//   ./probe_field            (opens first "loopMIDI" port)
//   ./probe_field 0          (opens port index 0)

#include "sources/midi_port.hpp"
#include "sources/midi_event.hpp"
#include "musical/context.hpp"
#include "musical/field.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <thread>

using namespace t7;

static const char* PC_NAME[12] =
    {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
static const char* IVL[12] =
    {"R","b2","2","b3","3","4","#4","5","b6","6","b7","7"};

constexpr int   PROBE_CHANNELS   = 16;
constexpr float PROBE_WAGON_SPAN = 4.0f;   // window span, beats
constexpr float PROBE_RETENTION  = 16.0f;  // completed-history kept, beats
constexpr int   FIELD_ROOT_PC    = 2;      // D -- the composition's home

// The present degrees as a compact bitmask, for change detection.
static uint16_t degree_mask(const PitchClassVector& degs) {
    uint16_t m = 0;
    for (int i = 0; i < 12; ++i) if (degs.v[i] > 0.0f) m |= uint16_t(1 << i);
    return m;
}

static std::string degree_str(const PitchClassVector& degs) {
    std::string s;
    for (int i = 0; i < 12; ++i)
        if (degs.v[i] > 0.0f) { if (!s.empty()) s += ' '; s += IVL[i]; }
    return s.empty() ? "-" : s;
}

int main(int argc, char** argv) {
    std::cout << std::unitbuf;

    // ── The bank, in HIERARCHY order: rank 1 at the top. The order is the
    //    ranking that breaks a draw among challengers (composition policy). ──
    const std::array<Field, 6> bank = {{
        {"PhrDom",  field_mask({0,1,4,5,7,8,10})},   // 1  D Phrygian dominant
        {"Mixo",    field_mask({0,2,4,5,7,9,10})},   // 2  D Mixolydian
        {"Dorian",  field_mask({0,2,3,5,7,9,10})},   // 3  D Dorian
        {"Major",   field_mask({0,2,4,5,7,9,11})},   // 4  D Ionian
        {"HarmMin", field_mask({0,2,3,5,7,8,11})},   // 5  D harmonic minor
        {"Lyd#2",   field_mask({0,3,4,6,7,9,11})},   // 6  D Lydian #2
    }};

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
        std::cout << "\nFailed to open a port. Pass an index, e.g.  ./probe_field 0\n";
        return 1;
    }
    std::cout << "\nListening on: " << port.port_name() << "   (Ctrl-C to stop)\n";
    std::cout << "  FIELD overlap: sounding degrees (present + window, binary) over root "
              << PC_NAME[FIELD_ROOT_PC] << ", dotted against each field.\n";
    std::cout << "  each number = how many sounding degrees lie in that field. Sampled once per beat.\n";
    std::cout << "  pick  = this beat's election (max overlap, ties to the earlier field).\n";
    std::cout << "  field = the standing reading: holds while tied for the top, moves only when\n";
    std::cout << "          a field strictly out-scores it; silence holds it.\n";
    std::cout << "  fields, in hierarchy order (a tie resolves to the earlier):";
    for (const auto& f : bank) std::cout << " " << f.name;
    std::cout << "\n  Waiting for MIDI clock -- enable this port's Sync output in Ableton and press play.\n\n";

    std::array<Context, PROBE_CHANNELS> ctx;
    for (int c = 0; c < PROBE_CHANNELS; ++c) {
        ctx[c].set_channel(c);
        ctx[c].set_retention_beats(PROBE_RETENTION);
        ctx[c].add_wagon(PROBE_WAGON_SPAN);
    }

    std::array<uint16_t, PROBE_CHANNELS> last_mask{};  // 0 = silent; quiet until content
    std::array<HeldField, PROBE_CHANNELS> held;        // the standing field, per channel
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

        // TRANSPORT -- on play/stop change.
        if (port.playing() != last_playing) {
            last_playing = port.playing();
            std::printf("TRANSPORT  %-4s  @%.2f  bpm %.1f\n",
                        last_playing ? "play" : "stop", beat, port.bpm());
        }

        // FIELD -- once per beat. Capture (poll + receive) ran every frame
        // above; the context snapshot and the reading are beat-paced. Because
        // events are received every frame, the stream already holds everything
        // up to this beat -- so the snapshot here is complete (receive, then
        // snapshot). Reprint a channel only when its sounding set changes.
        const int ibeat = static_cast<int>(beat);
        if (ibeat != last_beat) {
            last_beat = ibeat;
            for (int c = 0; c < PROBE_CHANNELS; ++c) ctx[c].update(beat);
            for (int c = 0; c < PROBE_CHANNELS; ++c) {
                const PitchClassVector degs =
                    to_degrees(present_set(ctx[c].playhead(), ctx[c].wagon(0)),
                               FIELD_ROOT_PC);
                const uint16_t m = degree_mask(degs);
                if (m == last_mask[c]) continue;
                last_mask[c] = m;

                // Advance the standing field. It holds the incumbent through
                // ambiguity and moves only when a field strictly out-scores it;
                // silence holds it (the empty set ties every field at zero).
                const int n_fields = static_cast<int>(bank.size());
                const int fld = held[c].step(degs, bank.data(), n_fields);

                if (m == 0) {
                    if (fld >= 0)
                        std::printf("ch%-2d @%-6.2f  (silent)   field %s [held]\n",
                                    c, beat, bank[fld].name);
                    else
                        std::printf("ch%-2d @%-6.2f  (silent)\n", c, beat);
                    continue;
                }
                std::printf("ch%-2d @%-6.2f  set[%-16s]", c, beat, degree_str(degs).c_str());
                for (const auto& f : bank)
                    std::printf("  %-7s %.0f", f.name, field_overlap(degs, f.mask));

                // pick = this beat's raw election (max overlap, hierarchy tiebreak).
                // field = the standing reading. They diverge when persistence holds
                // the incumbent against an election that would have jumped.
                const FieldChoice pick = elect_field(degs, bank.data(), n_fields);
                if (pick.ambiguous())
                    std::printf("   pick %-7s (tie %d)", bank[pick.index].name, pick.tie);
                else
                    std::printf("   pick %-7s        ", bank[pick.index].name);
                std::printf("   field %s\n", bank[fld].name);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}
