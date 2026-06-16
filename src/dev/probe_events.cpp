// probe_events.cpp ───────────────────────────────────────────────────
//
// DAW-synced READING probe for the comparison to come. It announces, per
// channel, the two onset-groups the PreviousEvent holds — nothing is
// compared yet; this is the "what is there" stage.
//
//   CURR  the current event: the onset-cluster being struck now (the open
//         group). The [N] prefix is the polyphony — how many notes are
//         sounding at this instant (the present count), which can exceed the
//         cluster when earlier notes are still held.
//   PREV  the previous event: the prior onset-cluster, latched, surviving
//         silence until the next opens. The [bracketed] voice is the previous
//         note proper — the one that goes offset last, the voice that lingered
//         to the seam; as the chord lets go, the bracket moves to it. `-`
//         before the first event.
//   TRANSPORT  the DAW play state + tempo (on change)
//
// An "event" is onset-defined — notes struck within PROBE_TOLERANCE seconds of
// each other form one cluster; a strike beyond it opens the next. Grouping runs
// off real arrival time (a wall clock), not the DAW beat: simultaneity is a fact
// of the hands, and a wall clock advances whether or not the transport is
// running, so the segmentation never collapses when the beat is paused. The
// @beat in the head is still the DAW's musical position. The block prints on
// each onset; the PREV row also reprints when a release shifts the previous
// group's last-to-offset voice.
//
// Channels side by side under a one-time header, label and beat at the head.
//
// Needs RtMidi. In Ableton, enable this port's Clock/Sync output, then play.
//   ./probe_events            (opens first "loopMIDI" port)
//   ./probe_events 0          (opens port index 0)

#include "sources/midi_port.hpp"
#include "sources/midi_event.hpp"
#include "musical/context.hpp"
#include "musical/previous_event.hpp"

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

constexpr int   N_CHANNELS      = 2;
constexpr int   CHANNEL_W       = 24;     // field per channel (a cluster of notes)
constexpr float PROBE_RETENTION = 16.0f;  // completed-history kept, beats
constexpr float PROBE_TOLERANCE = 0.05f;  // simultaneity window, seconds (real arrival time)
static const char* SEP = " | ";

// ── Cluster readers ──────────────────────────────────────────────────

static std::string open_cluster(const PreviousEvent& pe) {
    if (!pe.has_open()) return "-";
    std::string s;
    for (int i = 0; i < pe.open_count(); ++i) { if (i) s += ' '; s += note_name(pe.open_note(i).pitch); }
    if (pe.open_overflow()) s += " +";
    return s;
}
static std::string prev_cluster(const PreviousEvent& pe) {
    if (!pe.has_previous()) return "-";
    const int rep = pe.previous_last_offset_index();   // the voice that goes offset last
    std::string s;
    for (int i = 0; i < pe.previous_count(); ++i) {
        if (i) s += ' ';
        if (i == rep) { s += '['; s += note_name(pe.previous_note(i).pitch); s += ']'; }
        else          {           s += note_name(pe.previous_note(i).pitch);          }
    }
    if (pe.previous_overflow()) s += " +";
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
        std::cout << "\nFailed to open a port. Pass an index, e.g.  ./probe_events 0\n";
        return 1;
    }
    std::cout << "\nListening on: " << port.port_name() << "   (Ctrl-C to stop)\n";
    std::cout << "  CURR = current onset-cluster, [N] = notes sounding now (polyphony)."
                 "   PREV = previous onset-cluster (latched).\n";
    std::cout << "  +Nms on a CURR row = grouping-clock gap since the last onset"
                 " (<= tolerance joins the cluster, beyond it opens a new event).\n";
    std::cout << "  Rows print on each onset. Waiting for MIDI clock — enable this port's Sync"
                 " output in Ableton and press play.\n\n";

    std::array<Context, N_CHANNELS>       ctx;
    std::array<PreviousEvent, N_CHANNELS> prev;
    for (int c = 0; c < N_CHANNELS; ++c) {
        ctx[c].set_channel(c);
        ctx[c].set_retention_beats(PROBE_RETENTION);
        prev[c].set_tolerance(PROBE_TOLERANCE);
    }

    print_header();

    bool announced = false;
    bool last_playing = false;
    MidiEvent ev[256];
    const auto t_start = std::chrono::steady_clock::now();
    std::array<int, N_CHANNELS> last_rep;
    last_rep.fill(-1);     // previous representative pitch last shown (-1 = none)
    float last_onset_s = -1.0f;   // grouping-clock time of the last onset (any channel)

    while (true) {
        if (!announced && port.ever_synced()) {
            announced = true;
            std::cout << "  -- MIDI clock detected --\n";
        }

        const float beat  = static_cast<float>(port.beats());
        const float now_s = std::chrono::duration<float>(
                                std::chrono::steady_clock::now() - t_start).count();

        const int n = port.poll(beat, ev, 256);
        bool onset = false;
        for (int i = 0; i < n; ++i) {
            const MidiEvent& e = ev[i];
            if (e.channel < N_CHANNELS) {
                ctx[e.channel].receive(e);
                if (e.type == MidiEvent::NOTE_ON) {
                    prev[e.channel].on_onset(e.pitch, e.velocity, now_s);  // group by arrival time
                    onset = true;
                } else if (e.type == MidiEvent::NOTE_OFF) {
                    prev[e.channel].on_offset(e.pitch, now_s);             // record release
                }
            }
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

        // Announce on each onset (the events move), and also when a release
        // shifts the previous group's last-to-offset voice — the elected
        // previous note — so you watch the bracket move as the chord lets go.
        bool rep_changed = false;
        std::array<int, N_CHANNELS> rep;
        for (int c = 0; c < N_CHANNELS; ++c) {
            rep[c] = prev[c].previous_last_offset_pitch();
            if (rep[c] != last_rep[c]) rep_changed = true;
        }

        if (onset || rep_changed) {
            last_rep = rep;
            std::string prv[N_CHANNELS];
            for (int c = 0; c < N_CHANNELS; ++c) prv[c] = prev_cluster(prev[c]);

            if (onset) {
                // gap in the grouping clock since the previous onset — this is the
                // number the tolerance acts on; if it exceeds PROBE_TOLERANCE*1000,
                // the strike opened a new event rather than joining the cluster.
                const int gap_ms = (last_onset_s < 0.0f) ? -1
                    : static_cast<int>((now_s - last_onset_s) * 1000.0f + 0.5f);
                last_onset_s = now_s;

                std::string curr[N_CHANNELS];
                for (int c = 0; c < N_CHANNELS; ++c) {
                    const int poly = ctx[c].playhead().current_count;
                    curr[c] = "[" + std::to_string(poly) + "] " + open_cluster(prev[c]);
                }
                std::printf("%-8s@%-7.2f ", "CURR", beat);
                for (int c = 0; c < N_CHANNELS; ++c) {
                    if (c) std::printf("%s", SEP);
                    std::printf("%-*s", CHANNEL_W, curr[c].c_str());
                }
                if (gap_ms >= 0) std::printf("   +%dms", gap_ms);
                std::printf("\n");
            }
            print_row("PREV", beat, prv);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}
