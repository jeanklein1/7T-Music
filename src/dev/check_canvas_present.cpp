// check_canvas_present.cpp — sandbox verification of the present content
// vectors: the present's pitch classes by count and by length, re-origined
// to D and published, read back through the layout. With root D, C lands at
// degree 10, E at degree 2, D at degree 0; length is measured in beats.

#include "canvas.hpp"
#include "sources/midi_event.hpp"

#include <cstdio>
#include <cassert>
#include <cmath>
#include <string_view>

using namespace t7;

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
static void show(const Canvas& cv, const char* name) {
    const StatGroup* g = find(cv, name);
    std::printf("  %-20s [", name);
    bool first = true;
    for (int i = 0; i < g->count; ++i) {
        float x = cv.output().stat(g->channel, g->slot_base + i);
        if (x == 0.0f) continue;
        if (!first) std::printf(" ");
        std::printf("%d:%g", i, x);
        first = false;
    }
    std::printf("]   (nonzero degrees from D)\n");
}
static bool near(float a, float b) { return std::fabs(a - b) < 1e-5f; }

int main() {
    Canvas cv;
    ContextSpec s = default_spec(/*midi*/ 0, /*window*/ 4.0f);
    s.crossings.active = true;
    cv.configure(0, s);

    // Sound C4 and E4 at beat 1, read at beat 2 (each held one beat).
    cv.route(MidiEvent::note_on(0, 60, 0.8f, 1.0f));   // C -> degree 10
    cv.route(MidiEvent::note_on(0, 64, 0.8f, 1.0f));   // E -> degree 2
    cv.update(2.0f);

    std::printf("C + E sounding, held one beat:\n");
    show(cv, "ch0.present_count");
    show(cv, "ch0.present_length");

    assert(bin(cv, "ch0.present_count", 10) == 1.0f);   // C
    assert(bin(cv, "ch0.present_count", 2)  == 1.0f);   // E
    assert(bin(cv, "ch0.present_count", 0)  == 0.0f);   // D not sounding
    assert(near(bin(cv, "ch0.present_length", 10), 1.0f));   // C held 2-1 = 1 beat
    assert(near(bin(cv, "ch0.present_length", 2),  1.0f));   // E held 1 beat

    // Add D4 at beat 2, read at beat 3: C and E now two beats, D one.
    cv.route(MidiEvent::note_on(0, 62, 0.8f, 2.0f));   // D -> degree 0 (the root)
    cv.update(3.0f);

    std::printf("after adding D (C,E held two beats; D one):\n");
    show(cv, "ch0.present_count");
    show(cv, "ch0.present_length");

    assert(bin(cv, "ch0.present_count", 0) == 1.0f);          // D now present
    assert(near(bin(cv, "ch0.present_length", 0),  1.0f));    // D: one beat
    assert(near(bin(cv, "ch0.present_length", 10), 2.0f));    // C: two beats
    assert(near(bin(cv, "ch0.present_length", 2),  2.0f));    // E: two beats

    // Polyphony still rides alongside.
    assert(bin(cv, "ch0.polyphony", 0) == 3.0f);

    std::printf("\nOK -- present content publishes: count and length, re-origined to D, length in beats.\n");
    return 0;
}
