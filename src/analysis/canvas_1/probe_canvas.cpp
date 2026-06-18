// probe_canvas.cpp ──────────────────────────────────────────────────
//
// DAW-synced lab for canvas_1 — run as the cartridge it now is. It constructs
// the canvas, calls initialize() (which composes its two voices and opens
// loopMIDI), drives it the way the_lab does (update() each frame), and reads the
// published signal back through stat_layout().
//
// canvas_1's composition: two voices — slot 0 <- MIDI 0, slot 1 <- MIDI 1 —
// each a present and a four-beat window, the spine off. It publishes ONE
// reading: the field, taken across the UNION of both voices, in the group band.
//
// At startup it prints the binding of every active slot and the published
// layout. Then, on each note transition (seen from any voice's playhead) it
// prints three things in step: what the canvas speaks (the published field);
// the present notes of BOTH voices as ground truth; and the combined present-
// and-window pitch-class set the field elects from — so the printed field rank
// is explained by the notes that produced it. Whatever the layout advertises is
// printed, so a reading added later appears here without changing this probe.
//
// The canvas owns its port and reads the beat from it, so this harness passes
// wall-clock dt to update() (the signal's t_seconds telemetry) and reads the
// musical beat back from the published signal; it routes no events of its own.
//
// Needs RtMidi and the transport-aware MidiPort. In Ableton, enable loopMIDI's
// Clock/Sync output, then play.  (Ctrl-C to stop)
//   ./probe_canvas

#include "canvas.hpp"
#include "sources/midi_event.hpp"

#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>
#include <thread>

using namespace t7;
using namespace t7::canvas_1;   // the Canvas now lives in the cartridge's namespace

static std::string note_name(int midi) {
    static const char* n[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    return std::string(n[((midi % 12) + 12) % 12]) + std::to_string(midi / 12 - 1);
}

static const char* yn(bool b)    { return b ? "yes" : "no"; }
static const char* onoff(bool b) { return b ? "on"  : "off"; }
static const char* oracle_name(OracleBinding o) {
    switch (o) { case OracleBinding::Lowest: return "lowest"; }
    return "lowest";
}

// The static binding for one slot, printed once before the flow starts.
static void print_binding(const Canvas& canvas, int slot) {
    if (!canvas.active(slot)) return;
    const ContextSpec& s = canvas.spec(slot);
    const Context&     c = canvas.context(slot);
    std::printf("  slot %d  <-  midi ch %d   present %s   windows %d   event %s   spine %s",
                slot, s.channel, yn(s.has_present()), s.window_count(),
                onoff(s.has_event()), onoff(s.has_crossings()));
    if (s.has_crossings())
        std::printf(" (tol %.3f oracle %s)", s.crossings.tolerance_beats, oracle_name(s.crossings.oracle));
    std::printf("   [ctx ch %d, wagons %d]\n", c.channel(), c.wagon_count());
}

// The published layout: the named slot map the canvas advertises. A render
// side receives exactly this and resolves its sources against it.
static void print_layout(const Canvas& canvas) {
    StatLayoutView lay = canvas.stat_layout();
    std::printf("canvas output -- the published layout (%u group%s)\n",
                lay.count, lay.count == 1 ? "" : "s");
    for (uint32_t g = 0; g < lay.count; ++g) {
        const StatGroup& grp = lay.groups[g];
        std::printf("  %-18s  ch %d   slot %d..%d   %s\n",
                    grp.name, grp.channel,
                    grp.slot_base, grp.slot_base + grp.count - 1,
                    grp.shape == StatShape::Scalar ? "scalar" : "vector");
    }
}

// Read the published signal back the way the_lab does: walk the layout, pull
// each group's slots out of the signal. Scalars print as one value, vectors as
// a bracketed row of their nonzero bins, each as degree:value above D.
static void print_published(const Canvas& canvas) {
    const AnalysisSignal& sig = canvas.output();
    StatLayoutView lay = canvas.stat_layout();
    for (uint32_t g = 0; g < lay.count; ++g) {
        const StatGroup& grp = lay.groups[g];
        if (g) std::printf("   ");
        if (grp.shape == StatShape::Scalar) {
            std::printf("%s=%g", grp.name, sig.stat(grp.channel, grp.slot_base));
        } else {
            std::printf("%s=[", grp.name);
            bool first = true;
            for (int i = 0; i < grp.count; ++i) {
                const float x = sig.stat(grp.channel, grp.slot_base + i);
                if (x == 0.0f) continue;
                if (!first) std::printf(" ");
                std::printf("%d:%g", i, x);
                first = false;
            }
            std::printf("]");
        }
    }
}

// A note transition this frame, read from the playhead rather than from events:
// any voice that started or ended on any configured channel.
static bool note_changed(const Canvas& canvas) {
    for (int slot = 0; slot < MAX_CHANNELS; ++slot) {
        if (!canvas.active(slot)) continue;
        const PlayheadReadout& ph = canvas.context(slot).playhead();
        if (ph.onset_count > 0 || ph.release_count > 0) return true;
    }
    return false;
}

// The present notes of one voice — ground truth for what that channel holds now.
static std::string present_notes(const Canvas& canvas, int slot) {
    const PlayheadReadout& ph = canvas.context(slot).playhead();
    std::string s;
    for (int k = 0; k < ph.current_count; ++k) {
        if (k) s += " ";
        s += note_name(ph.current[k].pitch);
    }
    return s;
}

// The field's actual input: the cross-voice union of each active channel's
// present-and-window pitch-class set, built from the same present_set /
// present_union the canvas feeds the field. Printed as pitch-class letters so
// the published field rank is explained by the classes that produced it. (For
// canvas_1 the active voices are exactly the field's source, {0,1}.)
static std::string field_input(const Canvas& canvas) {
    PitchClassVector sets[MAX_CHANNELS];
    int n = 0;
    for (int slot = 0; slot < MAX_CHANNELS; ++slot) {
        if (!canvas.active(slot)) continue;
        const Context& c = canvas.context(slot);
        sets[n++] = present_set(c.playhead(), c.wagon(0));
    }
    const PitchClassVector u = present_union(sets, n);
    static const char* PC[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    std::string s;
    for (int i = 0; i < 12; ++i)
        if (u.v[i] > 0.0f) { if (!s.empty()) s += " "; s += PC[i]; }
    return s;
}

int main() {
    std::cout << std::unitbuf;

    // Construct and bring up the cartridge: initialize() composes two voices
    // (slot 0 <- MIDI 0, slot 1 <- MIDI 1), each present + a four-beat window,
    // the spine off; publishes the union field; and opens loopMIDI.
    Canvas canvas;
    canvas.initialize(/*asset_path*/ nullptr);

    std::cout << "canvas wiring -- the bindings\n";
    for (int slot = 0; slot < MAX_CHANNELS; ++slot) print_binding(canvas, slot);
    std::cout << "\n";
    print_layout(canvas);
    std::cout << "\n";

    if (!canvas.is_open()) {
        std::cout << "No port open -- is loopMIDI running and Ableton routed to it?\n";
        return 1;
    }
    std::cout << "Reading " << canvas.port_name()
              << " as a cartridge. Enable its Sync in Ableton and play.\n\n";

    // Drive it: pass the real wall-clock dt (the signal's t_seconds telemetry),
    // let update() drain the port and advance on the DAW's beat, then read the
    // beat back from the published signal.
    auto prev = std::chrono::steady_clock::now();
    while (true) {
        const auto now = std::chrono::steady_clock::now();
        const float dt = std::chrono::duration<float>(now - prev).count();
        prev = now;

        canvas.update(dt);   // drains the owned port, advances, publishes

        if (note_changed(canvas)) {
            const float beat = canvas.output().t_beats;
            std::printf("@%-7.2f ", beat);
            print_published(canvas);                 // what the canvas speaks (the field)
            std::printf("   ");
            for (int slot = 0; slot < MAX_CHANNELS; ++slot) {
                if (!canvas.active(slot)) continue;
                std::printf("ch%d(%s) ", slot, present_notes(canvas, slot).c_str());
            }
            std::printf("  field-input{ %s }\n", field_input(canvas).c_str());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}
