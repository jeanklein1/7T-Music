#pragma once

// ─── pc_count.hpp ────────────────────────────────────────────────
//
// Canvas operations over a view's notes, in the 12-slot pitch-class space
// (C = 0, octaves folded by pc_of). The count is the base; the others
// derive from it — the same loop with a different weight, a fold of the
// count, or the disjoint sum of two views.
//
//   pc_count  — weight 1 per occurrence (the base). Over the present, the
//               window, or their union.
//   pc_length — weight = the note's length. The present carries a
//               provisional length (anchor − onset, still growing while
//               held); the window a completed one; the union sums the two,
//               which are disjoint because a note is sounding or completed,
//               never both.
//   pc_onset  — weight = the note-on's velocity, filtered to onsets since a
//               caller-held beat (the per-frame aperture). Present + window,
//               disjoint as the length is.
//   pc_set    — support of a count: the pitch-class SET, a class in iff it
//               occurred at all. Derives FROM the count (count dominates;
//               the set is recoverable from the count, not the reverse).
//
// The union operations take both readouts and sum the single-view results.
// The disjointness of present and window is what lets the sum stand without
// double-counting: the same note instance never appears in both.
//
// Depends on: musical/musical_ops.hpp (PitchClassVector, PitchClassBits,
//             pc_of), musical/playhead.hpp, musical/wagon.hpp.

#include "musical/musical_ops.hpp"
#include "musical/playhead.hpp"
#include "musical/wagon.hpp"

namespace t7 {

// ═══ COUNT ═══════════════════════════════════════════════════════

// Present notes → pitch-class counts. One sounding note adds 1 at its pc;
// two sounding notes of the same class (different octaves) add 2.
inline PitchClassVector pc_count(const PlayheadReadout& ph) {
    PitchClassVector v;
    for (int i = 0; i < ph.current_count; ++i)
        v[pc_of(ph.current[i].pitch)] += 1.0f;
    return v;
}

// Windowed completed occurrences → pitch-class counts. Each WindowNote is
// one occurrence; repeats of a class within the window accumulate.
inline PitchClassVector pc_count(const WagonReadout& wg) {
    PitchClassVector v;
    for (int i = 0; i < wg.note_count; ++i)
        v[pc_of(wg.notes[i].pitch)] += 1.0f;
    return v;
}

// Present + window counts, the disjoint union. A note is sounding now or
// completed in the window, never both, so the sum does not double-count.
inline PitchClassVector pc_count(const PlayheadReadout& ph, const WagonReadout& wg) {
    PitchClassVector v = pc_count(ph);
    const PitchClassVector w = pc_count(wg);
    for (int i = 0; i < 12; ++i) v.v[i] += w.v[i];
    return v;
}

// ═══ LENGTH ══════════════════════════════════════════════════════

// Present notes → cumulative provisional length per pitch class. The same
// reduction as the present count, with the weight switched from 1 to the
// note's provisional span at the anchor (anchor − onset, still growing).
inline PitchClassVector pc_length(const PlayheadReadout& ph) {
    PitchClassVector v;
    for (int i = 0; i < ph.current_count; ++i)
        v[pc_of(ph.current[i].pitch)] += ph.current_duration(i);
    return v;
}

// Windowed cumulative in-window length per pitch class. The same reduction
// as the window count, with the weight switched from 1 to the note's clipped
// span (window_duration). Summed across occurrences of the class.
inline PitchClassVector pc_length(const WagonReadout& wg) {
    PitchClassVector v;
    for (int i = 0; i < wg.note_count; ++i)
        v[pc_of(wg.notes[i].pitch)] += wg.notes[i].window_duration();
    return v;
}

// Present + window length, the disjoint union: the present's provisional
// span summed with the window's completed span. Disjoint as the counts are,
// so the two halves add without overlap.
inline PitchClassVector pc_length(const PlayheadReadout& ph, const WagonReadout& wg) {
    PitchClassVector v = pc_length(ph);
    const PitchClassVector w = pc_length(wg);
    for (int i = 0; i < 12; ++i) v.v[i] += w.v[i];
    return v;
}

// ═══ ONSET ═══════════════════════════════════════════════════════

// Note-on impulses since `since_beat` (exclusive) → velocity-weighted
// pitch-class sums. The same reduction as the counts, with the weight
// switched from 1 to the note-on's velocity (already [0,1] at the stream
// layer) and the population filtered to onsets inside (since_beat, anchor].
// The present carries the still-sounding onsets; the window carries the
// ones already completed (an on-and-off within a single frame) — disjoint
// as ever, so the sum stands without double-counting. Each onset lands in
// exactly one aperture: the caller advances since_beat to its anchor after
// every read. Same-class retriggers inside one aperture sum.
inline PitchClassVector pc_onset(const PlayheadReadout& ph, const WagonReadout& wg, float since_beat) {
    PitchClassVector v;
    for (int i = 0; i < ph.current_count; ++i)
        if (ph.current[i].onset_beat > since_beat)
            v[pc_of(ph.current[i].pitch)] += ph.current[i].velocity;
    for (int i = 0; i < wg.note_count; ++i)
        if (wg.notes[i].onset_beat > since_beat)
            v[pc_of(wg.notes[i].pitch)] += wg.notes[i].velocity;
    return v;
}

// ═══ SET ═════════════════════════════════════════════════════════

// Support of a count vector: the pitch-class set. A class is in the set iff
// its count is greater than zero. The set type carries set algebra (union,
// intersection, complement) for the compound layer; here it is simply the
// thresholded count.
inline PitchClassBits pc_set(const PitchClassVector& count) {
    PitchClassBits s;
    for (int i = 0; i < 12; ++i) if (count.v[i] > 0.0f) s.set(i);
    return s;
}

} // namespace t7
