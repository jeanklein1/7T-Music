// check_canvas_field.cpp — verify the field reading: which of six declared
// fields the music sits in, published as a one-based index, with the top of
// the hierarchy as the default before any field scores. The field is the one
// step — the held incumbent moves on a strict win and holds otherwise.

#include "canvas.hpp"
#include "sources/midi_event.hpp"

#include <cstdio>
#include <cassert>
#include <string_view>

using namespace t7;

static int field_of(const Canvas& cv) {   // published ch0.field, as an int
    StatLayoutView lay = cv.stat_layout();
    for (uint32_t g = 0; g < lay.count; ++g)
        if (std::string_view(lay.groups[g].name) == "ch0.field")
            return static_cast<int>(cv.output().stat(lay.groups[g].channel, lay.groups[g].slot_base));
    return -999;
}

static void setup(Canvas& cv) {     // configure in place — the owned port makes Canvas non-movable
    ContextSpec s = default_spec(/*midi*/ 0, /*window*/ 4.0f);
    s.crossings.active = true;
    cv.configure(0, s);
}

int main() {
    // 1. Silence: no field has scored, so the index is the top of the hierarchy.
    {
        Canvas cv; setup(cv);
        cv.advance(0.5f);
        std::printf("silence              -> field %d   (expect 1, top of hierarchy)\n", field_of(cv));
        assert(field_of(cv) == 1);
    }

    // 2. A D-major colouring (D E F# C#) -> degrees {0,2,4,11}: Major (rank 3)
    //    strictly out-scores every field above and below it.
    {
        Canvas cv; setup(cv);
        cv.route(MidiEvent::note_on(0, 62, 0.8f, 1.0f));  // D
        cv.route(MidiEvent::note_on(0, 64, 0.8f, 1.0f));  // E
        cv.route(MidiEvent::note_on(0, 66, 0.8f, 1.0f));  // F#
        cv.route(MidiEvent::note_on(0, 61, 0.8f, 1.0f));  // C#
        cv.advance(1.0f);
        std::printf("D major (D E F# C#)  -> field %d   (expect 3, Major)\n", field_of(cv));
        assert(field_of(cv) == 3);
    }

    // 3. A harmonic-minor colouring (D F A Bb C#) -> degrees {0,3,7,8,11}:
    //    Harmonic Minor (rank 5) strictly out-scores the rest.
    {
        Canvas cv; setup(cv);
        cv.route(MidiEvent::note_on(0, 62, 0.8f, 1.0f));  // D
        cv.route(MidiEvent::note_on(0, 65, 0.8f, 1.0f));  // F
        cv.route(MidiEvent::note_on(0, 69, 0.8f, 1.0f));  // A
        cv.route(MidiEvent::note_on(0, 70, 0.8f, 1.0f));  // Bb
        cv.route(MidiEvent::note_on(0, 61, 0.8f, 1.0f));  // C#
        cv.advance(1.0f);
        std::printf("D harmonic minor     -> field %d   (expect 5, Harmonic Minor)\n", field_of(cv));
        assert(field_of(cv) == 5);
    }

    // 4. Persistence: once Major holds, silence does not release it. Release the
    //    chord and let it age past the four-beat window; the held field holds.
    {
        Canvas cv; setup(cv);
        cv.route(MidiEvent::note_on(0, 62, 0.8f, 1.0f));  // D
        cv.route(MidiEvent::note_on(0, 64, 0.8f, 1.0f));  // E
        cv.route(MidiEvent::note_on(0, 66, 0.8f, 1.0f));  // F#
        cv.route(MidiEvent::note_on(0, 61, 0.8f, 1.0f));  // C#
        cv.advance(1.0f);
        assert(field_of(cv) == 3);                        // Major established
        cv.route(MidiEvent::note_off(0, 62, 1.5f));
        cv.route(MidiEvent::note_off(0, 64, 1.5f));
        cv.route(MidiEvent::note_off(0, 66, 1.5f));
        cv.route(MidiEvent::note_off(0, 61, 1.5f));
        cv.advance(8.0f);                                  // silence, chord aged out
        std::printf("then silence (aged)  -> field %d   (expect 3, held)\n", field_of(cv));
        assert(field_of(cv) == 3);
    }

    std::printf("\nOK -- the field publishes: one-based index, top-of-hierarchy default, strict-win move, hold otherwise.\n");
    return 0;
}
