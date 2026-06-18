// check_canvas_compound.cpp — verify the per-voice readings and their compounds.
// Three voices, spine on. Each voice publishes current_pc / window_length; the
// union publishes compound current_pc and window_length as VECTOR SUMS (compound
// current_pc = a per-pc voice count; compound window_length = summed length), and
// the compound field. The test drives notes on the three voices and asserts the
// compound equals the element-wise sum of the per-voice vectors, and that the
// compounds land in the group band at their canonical slots. Frame driven
// directly (configure / publish_reading / route / advance), the port dormant.

#include "canvas.hpp"
#include "sources/midi_event.hpp"

#include <cstdio>
#include <cassert>
#include <cmath>
#include <string_view>

using namespace t7;
using namespace t7::canvas_1;

static const StatGroup* find(const Canvas& cv, const char* name) {
    StatLayoutView lay = cv.stat_layout();
    for (uint32_t g = 0; g < lay.count; ++g)
        if (std::string_view(lay.groups[g].name) == name) return &lay.groups[g];
    return nullptr;
}
static float bin(const Canvas& cv, const char* name, int i) {
    const StatGroup* g = find(cv, name);
    return g ? cv.output().stat(g->channel, g->slot_base + i) : -1.0f;
}
static bool near(float a, float b) { return std::fabs(a - b) < 1e-4f; }

// Three voices (0,1,2), spine on; per-voice current_pc + window_length, and the
// compound current_pc / window_length / field over their union.
static void setup(Canvas& cv) {
    for (int v = 0; v < 3; ++v) {
        ContextSpec s = default_spec(/*midi*/ v, /*window*/ 4.0f);
        s.crossings.active = true;          // spine on — current_pc needs it
        cv.configure(v, s);
    }
    cv.publish_reading(Canvas::Reading::CurrentPC,    Canvas::Source::channel(0), "ch0.current_pc");
    cv.publish_reading(Canvas::Reading::WindowLength, Canvas::Source::channel(0), "ch0.window_length");
    cv.publish_reading(Canvas::Reading::CurrentPC,    Canvas::Source::channel(2), "ch2.current_pc");

    const Canvas::Source all = Canvas::Source::group({0, 1, 2});
    cv.publish_reading(Canvas::Reading::Field,        all, "all.field");
    cv.publish_reading(Canvas::Reading::CurrentPC,    all, "all.current_pc");
    cv.publish_reading(Canvas::Reading::WindowLength, all, "all.window_length");
}

int main() {
    Canvas cv; setup(cv);

    // Voices 0 and 1 both sound D; voice 2 sounds F#. Held one beat (onset @1,
    // read @2). Degrees from D: D -> 0, F# -> 4.
    cv.route(MidiEvent::note_on(0, 62, 0.8f, 1.0f));   // ch0 D
    cv.route(MidiEvent::note_on(1, 62, 0.8f, 1.0f));   // ch1 D
    cv.route(MidiEvent::note_on(2, 66, 0.8f, 1.0f));   // ch2 F#
    cv.advance(2.0f);

    // ── Per-voice current_pc: a one-hot at each voice's line note ───────────
    std::printf("per-voice current_pc: ch0[D]=%g  ch2[F#]=%g\n",
                bin(cv, "ch0.current_pc", 0), bin(cv, "ch2.current_pc", 4));
    assert(bin(cv, "ch0.current_pc", 0) == 1.0f);   // ch0 on D
    assert(bin(cv, "ch2.current_pc", 4) == 1.0f);   // ch2 on F#

    // ── Compound current_pc = vector sum (a per-pc voice count) ─────────────
    std::printf("compound current_pc: [D]=%g (expect 2)  [F#]=%g (expect 1)\n",
                bin(cv, "all.current_pc", 0), bin(cv, "all.current_pc", 4));
    assert(bin(cv, "all.current_pc", 0) == 2.0f);   // D held by voices 0 and 1
    assert(bin(cv, "all.current_pc", 4) == 1.0f);   // F# by voice 2

    // ── window_length: per-voice ~1 beat; compound = summed length ──────────
    assert(near(bin(cv, "ch0.window_length", 0), 1.0f));        // ch0 D: one beat
    std::printf("compound window_length: [D]=%g (expect 2)  [F#]=%g (expect 1)\n",
                bin(cv, "all.window_length", 0), bin(cv, "all.window_length", 4));
    assert(near(bin(cv, "all.window_length", 0), 2.0f));        // D: 1 beat x 2 voices
    assert(near(bin(cv, "all.window_length", 4), 1.0f));        // F#: 1 beat

    // ── Placement: compounds in the group band; voices in their own ─────────
    const StatGroup* gc = find(cv, "all.current_pc");
    assert(gc && gc->channel == MAX_CHANNELS - 1 && gc->slot_base == 48);
    const StatGroup* gf = find(cv, "all.field");
    assert(gf && gf->channel == MAX_CHANNELS - 1 && gf->slot_base == 61);
    const StatGroup* g0 = find(cv, "ch0.current_pc");
    assert(g0 && g0->channel == 0 && g0->slot_base == 48);

    // ── Availability still bites: a line reading needs the spine ────────────
    Canvas bare;
    bare.configure(0, default_spec(/*midi*/ 0, /*window*/ 4.0f));   // spine OFF
    assert(!bare.publish_reading(Canvas::Reading::CurrentPC, Canvas::Source::channel(0), "ch0.current_pc"));

    std::printf("\nOK -- per-voice readings publish; the additive compounds are the element-wise\n");
    std::printf("      sum of the voices; compounds sit in the group band; spine gates the line.\n");
    return 0;
}
