// check_canvas_field_global.cpp — the live counterpart to check_field_union:
// the field over the union of two channels, published through the canvas. The
// canvas now speaks one reading, the group field, at the group band; this drives
// notes on both MIDI channels and reads that one field back.
//
//   - the published surface is the single group "field", on the group band
//   - silence reads the top of the hierarchy, as a channel field does
//   - a D-major triad on one channel is ambiguous (it reads as the hierarchy's
//     top); the second channel's C and E widen the union to a mixolydian
//     skeleton, and the held group field moves to it on the strict win — a
//     reading neither voice carries alone.

#include "canvas.hpp"
#include "sources/midi_event.hpp"

#include <cstdio>
#include <cassert>
#include <string_view>

using namespace t7;

static const StatGroup* find_group(const Canvas& cv, const char* name) {
    StatLayoutView lay = cv.stat_layout();
    for (uint32_t g = 0; g < lay.count; ++g)
        if (std::string_view(lay.groups[g].name) == name)
            return &lay.groups[g];
    return nullptr;
}

static int field_global(const Canvas& cv) {
    const StatGroup* g = find_group(cv, "field");
    return g ? static_cast<int>(cv.output().stat(g->channel, g->slot_base)) : -999;
}

static void setup(Canvas& cv) {   // two channels, full specs; publish the group field
    ContextSpec s0 = default_spec(/*midi*/ 0, /*window*/ 4.0f);
    ContextSpec s1 = default_spec(/*midi*/ 1, /*window*/ 4.0f);
    s0.crossings.active = true;
    s1.crossings.active = true;
    cv.configure(0, s0);
    cv.configure(1, s1);
    cv.publish_field_global();
}

int main() {
    // 0. The published surface: one reading, the group field, on the group band.
    {
        Canvas cv; setup(cv);
        StatLayoutView lay = cv.stat_layout();
        const StatGroup* g = find_group(cv, "field");
        std::printf("published groups     -> %u   (expect 1, the group field alone)\n", lay.count);
        assert(lay.count == 1);
        assert(g != nullptr);
        assert(g->channel == MAX_CHANNELS - 1);   // the group band, no voice prefix
        assert(g->slot_base == 61);               // the field's canonical slot
        std::printf("  \"field\" at band %d, slot %d\n\n", g->channel, g->slot_base);
    }

    // 1. Silence: no field has scored, so the index is the top of the hierarchy.
    {
        Canvas cv; setup(cv);
        cv.advance(0.5f);
        std::printf("silence              -> field %d   (expect 1, top of hierarchy)\n", field_global(cv));
        assert(field_global(cv) == 1);
    }

    // 2. The union forms across the two channels and moves the reading. A bare
    //    D-major triad on channel 0 is ambiguous, reading as the hierarchy's top
    //    (phrygian dominant). The C and E entering on channel 1 widen the union
    //    to {D E F# A C}, a mixolydian skeleton that strictly out-scores the
    //    incumbent, so the held group field moves to mixolydian (rank 2).
    {
        Canvas cv; setup(cv);
        cv.route(MidiEvent::note_on(0, 62, 0.8f, 1.0f));   // D  on channel 0
        cv.route(MidiEvent::note_on(0, 66, 0.8f, 1.0f));   // F#
        cv.route(MidiEvent::note_on(0, 69, 0.8f, 1.0f));   // A
        cv.advance(1.0f);
        std::printf("ch0 triad alone      -> field %d   (expect 1, ambiguous -> hierarchy top)\n", field_global(cv));
        assert(field_global(cv) == 1);

        cv.route(MidiEvent::note_on(1, 60, 0.8f, 1.0f));   // C  on channel 1
        cv.route(MidiEvent::note_on(1, 64, 0.8f, 1.0f));   // E
        cv.advance(1.5f);
        std::printf("+ ch1 C, E (union)   -> field %d   (expect 2, mixolydian, the union's reading)\n", field_global(cv));
        assert(field_global(cv) == 2);
    }

    std::printf("\nOK -- the canvas publishes one reading, the field over the union of its channels;\n");
    std::printf("      the combination reads what neither voice states alone.\n");
    return 0;
}
