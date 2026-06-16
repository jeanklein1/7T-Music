// check_canvas_publish.cpp — sandbox verification of the canvas's speaking
// half. RtMidi-free: drives the canvas with synthetic events the way the
// live probe will with Ableton's, then reads the published signal back the
// way the_lab does — iterating the layout, pulling each group's slots.

#include "canvas.hpp"
#include "sources/midi_event.hpp"

#include <cstdio>
#include <cassert>
#include <string_view>

using namespace t7;

// Read one named scalar group out of the published signal, the_lab-style:
// find the group in the layout, pull its slot. -1 if the name is absent.
static float published_scalar(const Canvas& cv, const char* name) {
    const AnalysisSignal& sig = cv.output();
    StatLayoutView lay = cv.stat_layout();
    for (uint32_t g = 0; g < lay.count; ++g) {
        const StatGroup& grp = lay.groups[g];
        if (std::string_view(grp.name) == name)
            return sig.stat(grp.channel, grp.slot_base);
    }
    return -1.0f;
}

int main() {
    Canvas cv;

    // Two channels, present + window + spine, on MIDI ch 0 and ch 1.
    for (int slot = 0; slot < 2; ++slot) {
        ContextSpec s = default_spec(/*midi*/ slot, /*window*/ 4.0f);
        s.crossings.active = true;
        cv.configure(slot, s);
    }

    // The published layout: one scalar group per channel.
    StatLayoutView lay = cv.stat_layout();
    std::printf("layout: %u group(s)\n", lay.count);
    for (uint32_t g = 0; g < lay.count; ++g) {
        const StatGroup& grp = lay.groups[g];
        std::printf("  %-16s ch %d  slot %d  count %d  %s\n",
                    grp.name, grp.channel, grp.slot_base, grp.count,
                    grp.shape == StatShape::Scalar ? "scalar" : "vector");
    }
    assert(lay.count == 2);

    auto show = [&](const char* tag) {
        std::printf("%-22s ch0.polyphony=%g  ch1.polyphony=%g\n", tag,
                    published_scalar(cv, "ch0.polyphony"),
                    published_scalar(cv, "ch1.polyphony"));
    };

    cv.update(0.0f);                                   // silence
    show("silent");
    assert(published_scalar(cv, "ch0.polyphony") == 0.0f);

    cv.route(MidiEvent::note_on(0, 60, 0.8f, 1.0f));   // ch0: C
    cv.update(1.0f);
    show("ch0: C on");
    assert(published_scalar(cv, "ch0.polyphony") == 1.0f);
    assert(published_scalar(cv, "ch1.polyphony") == 0.0f);

    cv.route(MidiEvent::note_on(0, 64, 0.8f, 1.5f));   // ch0: + E -> dyad
    cv.update(1.5f);
    show("ch0: C E on");
    assert(published_scalar(cv, "ch0.polyphony") == 2.0f);

    cv.route(MidiEvent::note_on(1, 48, 0.8f, 1.6f));   // ch1: C, independent
    cv.update(1.6f);
    show("ch1: C on");
    assert(published_scalar(cv, "ch0.polyphony") == 2.0f);
    assert(published_scalar(cv, "ch1.polyphony") == 1.0f);

    cv.route(MidiEvent::note_off(0, 60, 2.0f));        // ch0: release C -> 1
    cv.update(2.0f);
    show("ch0: C off");
    assert(published_scalar(cv, "ch0.polyphony") == 1.0f);
    assert(published_scalar(cv, "ch1.polyphony") == 1.0f);

    std::printf("\nOK -- the canvas speaks: polyphony published per channel, read back through the layout.\n");
    return 0;
}
