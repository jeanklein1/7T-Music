// check_canvas_window.cpp — verify the present-plus-window content vectors:
// the present summed with the trailing window, distinct from present-only. A
// note released into the window appears in the window reading but not the
// present reading; a note still sounding appears in both.

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
    std::printf("]\n");
}
static bool near(float a, float b) { return std::fabs(a - b) < 1e-5f; }

int main() {
    Canvas cv;
    ContextSpec s = default_spec(/*midi*/ 0, /*window*/ 4.0f);
    s.crossings.active = true;
    cv.configure(0, s);

    // C sounds at beat 1, releases at beat 2 (now completed, inside the
    // window). E sounds at beat 2 and is still held. Read at beat 3.
    cv.route(MidiEvent::note_on (0, 60, 0.8f, 1.0f));   // C on
    cv.route(MidiEvent::note_off(0, 60,       2.0f));   // C off -> completed
    cv.route(MidiEvent::note_on (0, 64, 0.8f, 2.0f));   // E on (held)
    cv.update(3.0f);

    std::printf("E sounding now; C released into the window:\n");
    show(cv, "ch0.present_count");
    show(cv, "ch0.window_count");
    show(cv, "ch0.present_length");
    show(cv, "ch0.window_length");

    // present: only E (degree 2). C has left the present.
    assert(bin(cv, "ch0.present_count", 2)  == 1.0f);
    assert(bin(cv, "ch0.present_count", 10) == 0.0f);

    // window = present + completed window: E (present) and C (completed).
    assert(bin(cv, "ch0.window_count", 2)  == 1.0f);    // E
    assert(bin(cv, "ch0.window_count", 10) == 1.0f);    // C, only via the window

    // lengths in beats: E held 3-2 = 1; C completed 2-1 = 1.
    assert(near(bin(cv, "ch0.present_length", 2), 1.0f));
    assert(near(bin(cv, "ch0.window_length",  2), 1.0f));   // E present
    assert(near(bin(cv, "ch0.window_length", 10), 1.0f));   // C completed, in window
    assert(bin(cv, "ch0.present_length", 10) == 0.0f);      // C not in the present

    std::printf("\nOK -- window content publishes: present summed with the trailing window, distinct from present alone.\n");
    return 0;
}
