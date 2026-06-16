// ─── probe_interval.cpp ──────────────────────────────────────────
//
// READING PROBE — the step. Each arriving NOTE_ON is the reference:
// the difference is computed from the elected previous to it, at its
// arrival, stamped with its own beat. The tuple is a fact about the
// event, not about frame state.
//
// The composition this probe makes visible:
//
//   MidiPort → Spine → step(event, spine)
//
// Nothing else. No Context, no Playhead, no PreviousEvent — the
// spine is self-contained: it witnesses the crossings, performs the
// election (survived / minted / chosen), and supplies the previous
// the step measures from. That this probe needs so little is the
// point.
//
// Rows:
//   INTV  the step, one row per arriving NOTE_ON, in its channel:
//           (E4, +2) minted    the event, signed semitones from the
//                              previous, and how that previous was
//                              elected
//           (E4, -)            first event — previous undefined
//           -                  this event took no line (a vertical
//                              member); no scalar exists for it
//   SPINE the previous-note line, printed when it moves:
//           F4 < C4 (minted)   holder and its previous
//           = C4 (survived)    the standing election, between events
//           ~3                 vertical of 3, no line
//
// A chord's first member prints a provisional step which its partner
// voids — the SPINE row going ~N shows the retraction. Honest, not
// a bug: the machine narrates its own corrections.
//
// Tolerance is in seconds of real arrival time (the grouping clock),
// the same contract as the spine itself.
//
// Build (CMake): copy of the probe recipe —
//   add_executable(probe_interval src/dev/probe_interval.cpp
//                  src/external/RtMidi.cpp)
//   include dirs src/dev, src; MSVC: __WINDOWS_MM__ + winmm.
//
// Usage:
//   ./probe_interval            (opens first "loopMIDI" port)
//   ./probe_interval 0          (opens port index 0)

#include "sources/midi_port.hpp"
#include "sources/midi_event.hpp"
#include "musical/spine.hpp"

#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

using namespace t7;

static std::string note_name(int midi) {
    static const char* n[] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
    return std::string(n[((midi % 12) + 12) % 12]) + std::to_string(midi / 12 - 1);
}

constexpr int   N_CHANNELS = 2;
constexpr int   CHANNEL_W = 24;     // field per channel
constexpr float PROBE_TOLERANCE = 0.05f;  // simultaneity window, seconds (real arrival time)
static const char* SEP = " | ";

// ── Cell readers ─────────────────────────────────────────────────────

// The step, anchored on the arriving event: the event is the reference;
// the difference is computed from the elected previous to it, at its
// arrival. '-' when the event took no line (a vertical member).
static std::string step_for(const Spine& sp, const MidiEvent& e) {
    if (!(sp.line_legible() && sp.line_holder() == e.pitch)) return "-";
    std::string s = "(" + note_name(e.pitch) + ", ";
    if (sp.has_line_previous()) {
        const int d = e.pitch - sp.line_previous();
        char buf[16];
        std::snprintf(buf, sizeof buf, "%+d", d);
        s += buf;
        s += ") ";
        s += provenance_name(sp.line_previous_provenance());
    }
    else {
        s += "-)";
    }
    return s;
}

// The spine cell, three states of the line.
static std::string spine_cell(const Spine& sp) {
    if (sp.line_legible()) {
        std::string s = note_name(sp.line_holder()) + " < ";
        if (sp.has_line_previous()) {
            s += note_name(sp.line_previous());
            s += " ("; s += provenance_name(sp.line_previous_provenance()); s += ")";
        }
        else {
            s += "-";
        }
        return s;
    }
    if (sp.sounding_count() > 0) return "~" + std::to_string(sp.sounding_count());
    if (sp.has_resolved()) {
        return "= " + note_name(sp.resolved_pitch())
            + " (" + provenance_name(sp.resolved_provenance()) + ")";
    }
    return "-";
}

// ── Rendering ────────────────────────────────────────────────────────

static void print_header() {
    std::printf("%-16s ", "");
    for (int c = 0; c < N_CHANNELS; ++c) {
        if (c) std::printf("%s", SEP);
        char t[8]; std::snprintf(t, sizeof t, "ch%d", c);
        int len = (int)std::strlen(t), pad = (CHANNEL_W - len) / 2;
        for (int i = 0; i < pad; ++i) { std::printf(" "); }
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

// ── Main ─────────────────────────────────────────────────────────────

int main(int argc, char** argv) {
    MidiPort port;

    std::cout << "MIDI inputs:\n";
    auto names = port.enumerate();
    for (size_t i = 0; i < names.size(); ++i)
        std::cout << "  [" << i << "] " << names[i] << "\n";

    bool ok = false;
    if (argc > 1) ok = port.open(static_cast<unsigned>(std::atoi(argv[1])));
    else          ok = port.open_by_name("loopMIDI");
    if (!ok) { std::cerr << "Could not open a MIDI input.\n"; return 1; }

    std::cout << "Listening on: " << port.port_name() << "   (Ctrl-C to stop)\n";
    std::cout << "  INTV = the step: '(E4, +2) minted' = current note, signed semitones"
        " travelled from the previous, and how it was elected. '(E4, -)' = no previous yet.\n";
    std::cout << "  SPINE = the previous-note line. 'F4 < C4 (minted)' = holder and its"
        " previous; '= C4 (survived)' = the standing election; '~3' = vertical, no line.\n";
    std::cout << "  INTV prints one row per arriving note, in its channel, stamped with"
        " the event's beat; SPINE prints when the line moves.\n"
        "  Waiting for MIDI clock — enable this port's Sync output in Ableton and press play.\n\n";

    print_header();

    std::array<Spine, N_CHANNELS> spine;
    for (int c = 0; c < N_CHANNELS; ++c) spine[c].set_tolerance(PROBE_TOLERANCE);

    bool announced = false;
    bool last_playing = false;
    MidiEvent ev[256];
    const auto t_start = std::chrono::steady_clock::now();
    std::array<std::string, N_CHANNELS> last_spine;   // spine cell last shown

    while (true) {
        if (!announced && port.ever_synced()) {
            announced = true;
            std::cout << "  -- MIDI clock detected --\n";
        }

        const float beat = static_cast<float>(port.beats());
        const float now_s = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - t_start).count();

        const int n = port.poll(beat, ev, 256);
        for (int i = 0; i < n; ++i) {
            const MidiEvent& e = ev[i];
            if (e.channel >= N_CHANNELS) continue;
            if (e.type == MidiEvent::NOTE_ON) {
                spine[e.channel].on_onset(e.pitch, now_s);
                // The arriving event is the reference. Compute the
                // difference from the elected previous to it, here, at
                // its arrival, stamped with its own beat.
                std::string cells[N_CHANNELS];
                cells[e.channel] = step_for(spine[e.channel], e);
                print_row("INTV", e.beat, cells);
            }
            else if (e.type == MidiEvent::NOTE_OFF) {
                spine[e.channel].on_offset(e.pitch, now_s);
            }
        }

        // TRANSPORT — on play/stop change.
        if (port.playing() != last_playing) {
            last_playing = port.playing();
            char line[64];
            std::snprintf(line, sizeof line, "TRANSPORT  %-4s  @%-8.2f bpm %.1f",
                last_playing ? "play" : "stop", beat, port.bpm());
            std::cout << line << "\n";
        }

        // SPINE when the line moves: mint, void, handover, election.
        std::array<std::string, N_CHANNELS> spn;
        bool spine_changed = false;
        for (int c = 0; c < N_CHANNELS; ++c) {
            spn[c] = spine_cell(spine[c]);
            if (spn[c] != last_spine[c]) spine_changed = true;
        }
        if (spine_changed) {
            last_spine = spn;
            print_row("SPINE", beat, spn.data());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}