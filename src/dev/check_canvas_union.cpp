// check_canvas_union.cpp — verify the union field: the one published reading of
// the first instance. Two voices are subscribed (slot 0 ← MIDI 0, slot 1 ←
// MIDI 1); the field is published once, source the union {0, 1}, in the
// reserved group band under the bare name `field`. The test drives notes on
// both channels and asserts the field is elected from the COMBINED present-set
// — a note on one voice moving the field the other established — that it lands
// in the group band at the field's canonical slot, and that no per-channel
// reading is published. The frame is driven directly (configure /
// publish_reading / route / advance), the port dormant.

#include "canvas.hpp"
#include "sources/midi_event.hpp"

#include <cstdio>
#include <cassert>
#include <string_view>

using namespace t7;

// Subscribe two voices and publish the union field — the first instance,
// composed without opening the port (the test's standing-in for initialize()).
static void setup(Canvas& cv) {
    cv.configure(0, default_spec(/*midi*/ 0, /*window*/ 4.0f));
    cv.configure(1, default_spec(/*midi*/ 1, /*window*/ 4.0f));
    const bool ok = cv.publish_reading(Canvas::Reading::Field,
                                       Canvas::Source::group({0, 1}), "field");
    assert(ok);
    (void)ok;
}

static const StatGroup* find(const Canvas& cv, const char* name) {
    StatLayoutView lay = cv.stat_layout();
    for (uint32_t g = 0; g < lay.count; ++g)
        if (std::string_view(lay.groups[g].name) == name) return &lay.groups[g];
    return nullptr;
}

static int field_of(const Canvas& cv) {   // the published group `field`, as an int
    const StatGroup* g = find(cv, "field");
    return g ? static_cast<int>(cv.output().stat(g->channel, g->slot_base)) : -999;
}

int main() {
    // 1. Silence: no field has scored, so the index is the top of the hierarchy.
    {
        Canvas cv; setup(cv);
        cv.advance(0.5f);
        std::printf("silence                       -> field %d   (expect 1, top of hierarchy)\n", field_of(cv));
        assert(field_of(cv) == 1);
    }

    // 2. The union elects across both voices, and a note on one voice moves the
    //    field the other established.
    //
    //    Voice 0 sounds D E F#  (degrees {0,2,4} from D): Mixolydian and Major
    //    tie at the top, the hierarchy keeps Mixolydian (rank 2). Then voice 1
    //    adds C# (degree 11): the COMBINED set {0,2,4,11} makes Major (rank 3)
    //    strictly out-score Mixolydian, so the held field moves to Major. The
    //    move is driven by a note on a channel the field did not start on —
    //    proof the field reads the union, not either voice alone.
    {
        Canvas cv; setup(cv);

        cv.route(MidiEvent::note_on(0, 62, 0.8f, 1.0f));   // D  on voice 0
        cv.route(MidiEvent::note_on(0, 64, 0.8f, 1.0f));   // E  on voice 0
        cv.route(MidiEvent::note_on(0, 66, 0.8f, 1.0f));   // F# on voice 0
        cv.advance(1.5f);
        std::printf("voice0: D E F#                -> field %d   (expect 2, Mixolydian)\n", field_of(cv));
        assert(field_of(cv) == 2);

        cv.route(MidiEvent::note_on(1, 61, 0.8f, 2.0f));   // C# on voice 1
        cv.advance(2.5f);
        std::printf("voice1 adds C#  (union)       -> field %d   (expect 3, Major — moved by voice 1)\n", field_of(cv));
        assert(field_of(cv) == 3);

        // Persistence: release both voices and age past the window; the held
        // field holds through the silence that follows.
        cv.route(MidiEvent::note_off(0, 62, 3.0f));
        cv.route(MidiEvent::note_off(0, 64, 3.0f));
        cv.route(MidiEvent::note_off(0, 66, 3.0f));
        cv.route(MidiEvent::note_off(1, 61, 3.0f));
        cv.advance(12.0f);
        std::printf("then silence (aged)           -> field %d   (expect 3, held)\n", field_of(cv));
        assert(field_of(cv) == 3);
    }

    // 3. Placement and opt-in: the field is published once, in the reserved
    //    group band (the last index) at the field's canonical slot, under the
    //    bare name `field` — and nothing per-channel is published.
    {
        Canvas cv; setup(cv);
        cv.advance(0.5f);

        const StatGroup* g = find(cv, "field");
        assert(g != nullptr);
        assert(g->channel   == MAX_CHANNELS - 1);   // the reserved group band
        assert(g->slot_base == 61);                 // the field's canonical slot
        assert(g->count     == 1);
        assert(g->shape     == StatShape::Scalar);

        assert(find(cv, "ch0.field")         == nullptr);
        assert(find(cv, "ch1.field")         == nullptr);
        assert(find(cv, "ch0.present_count") == nullptr);
        assert(cv.stat_layout().count == 1);        // the union field is the sole tenant
        std::printf("placement: `field` at band %d slot %d, sole published reading\n",
                    g->channel, g->slot_base);
    }

    // 4. Availability-binding: a line reading needs the spine, which the spec
    //    leaves off, so the declaration is refused and the contract is unchanged.
    {
        Canvas cv; setup(cv);
        const bool refused = !cv.publish_reading(Canvas::Reading::CurrentPC,
                                                 Canvas::Source::channel(0), "ch0.current_pc");
        assert(refused);
        (void)refused;
        assert(cv.stat_layout().count == 1);        // nothing was added
        std::printf("availability: line reading refused (spine off), contract unchanged\n");
    }

    // 5. Write-gating: a reading whose value-writer is not yet wired is refused
    //    even when the analysis could feed it — so the layout never advertises a
    //    slot that would stay zero. present_count is available (the present is
    //    always there) but unwired this round, so its declaration is refused.
    {
        Canvas cv; setup(cv);
        const bool refused = !cv.publish_reading(Canvas::Reading::PresentCount,
                                                 Canvas::Source::channel(0), "ch0.present_count");
        assert(refused);
        (void)refused;
        assert(cv.stat_layout().count == 1);        // nothing was added
        std::printf("write-gating: available-but-unwired reading refused, contract unchanged\n");
    }

    std::printf("\nOK -- the union field publishes: one reading across both voices, in the group band, opt-in and availability-bound.\n");
    return 0;
}
