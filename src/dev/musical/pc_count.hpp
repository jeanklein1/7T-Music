#pragma once

// ─── pc_count.hpp ────────────────────────────────────────────────
//
// The first layer-C operation: lift a view's notes into the 12-slot
// pitch-class COUNT vector. Each occurrence adds 1 at its pitch class;
// octaves are folded by pc_of. The twelve slots are the basis (C = 0),
// the vector a point in that space — counting is just summing the unit
// vectors of what occurred.
//
//   Playhead → counts the notes sounding NOW, per pitch class.
//   Wagon    → counts the completed occurrences in the window, per pc.
//
// Count is one weight choice; the loop is weight-agnostic. Count uses
// weight 1. A length-weighted vector is the SAME loop with the weight set
// to the note's in-window length — the seam is marked below, deliberately
// not built until asked.
//
// Kept here, beside probe_vector, until a second consumer (the canvas)
// makes it shared vocabulary and earns a move into musical_ops.
//
// Depends on: musical/musical_ops.hpp (PitchClassVector, pc_of),
//             musical/playhead.hpp (PlayheadReadout),
//             musical/wagon.hpp (WagonReadout).

#include "musical/musical_ops.hpp"
#include "musical/playhead.hpp"
#include "musical/wagon.hpp"

namespace t7 {

// Present notes → pitch-class counts. One sounding note adds 1 at its pc;
// two sounding notes of the same class (different octaves) add 2.
inline PitchClassVector pc_count(const PlayheadReadout& ph) {
    PitchClassVector v;
    for (int i = 0; i < ph.current_count; ++i)
        v[pc_of(ph.current[i].pitch)] += 1.0f;   // count: weight 1
    return v;
}

// Windowed completed occurrences → pitch-class counts. Each WindowNote is
// one occurrence; repeats of a class within the window accumulate.
inline PitchClassVector pc_count(const WagonReadout& wg) {
    PitchClassVector v;
    for (int i = 0; i < wg.note_count; ++i)
        v[pc_of(wg.notes[i].pitch)] += 1.0f;     // count: weight 1
                                                 // length mode (later):
                                                 //   += wg.notes[i].window_duration()
    return v;
}

} // namespace t7
