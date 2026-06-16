#pragma once

// ─── spine_ops.hpp ───────────────────────────────────────────────
//
// Canvas operations over the spine's line — the two readings the packet
// takes from it. The spine elects the line (a current note and the previous
// it descended from); these turn that line into the forms the canvas ships.
//
//   current_note  — the holder as a one-hot pitch-class vector. Which note
//                   currently holds the line, octave-agnostic. Raw, like the
//                   counts: the per-vector dressing (re-origin, scale) is
//                   applied at the sink, the same for every vector.
//   line_distance — the signed registral semitones from the previous note to
//                   the current one (holder − previous): how far the line
//                   just moved, with direction.
//
// Both fall quiet when the line is illegible — an empty vector, a zero
// distance — so a vertical with no single line, or silence, reads as nothing
// rather than as a stale value.
//
// distance(from, to), the general two-note interval line_distance is built
// from, now lives in musical_ops beside pc_relative_to — the spine is one
// supplier of the pair, not the only one, so the interval belongs on the
// floor rather than here.
//
// Depends on: musical/musical_ops.hpp (PitchClassVector, pc_of, distance),
//             musical/spine.hpp (the line).

#include "musical/musical_ops.hpp"
#include "musical/spine.hpp"

namespace t7 {

// The current note as a one-hot at the holder's pitch class. Empty when the
// line is illegible. Raw — dressing is applied at the sink, with the rest.
inline PitchClassVector current_note(const Spine& spine) {
    PitchClassVector v;
    if (spine.line_legible())
        v[pc_of(spine.line_holder())] = 1.0f;
    return v;
}

// How far the line just moved: holder minus previous, signed and registral.
// Zero when the line is illegible or has no previous (a unison is zero of its
// own accord, since the two pitches are equal).
inline int line_distance(const Spine& spine) {
    if (spine.line_legible() && spine.has_line_previous())
        return distance(spine.line_previous(), spine.line_holder());
    return 0;
}

} // namespace t7
