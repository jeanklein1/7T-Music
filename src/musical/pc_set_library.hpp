#pragma once

// ─── pc_set_library.hpp ──────────────────────────────────────────
//
// Convenience constructors for commonly-named pitch-class sets. They
// live OUTSIDE the analyzer module because they encode coupling-side
// conventions (named scales with roots) rather than analyzer-side
// measurements.
//
// Depends on: musical/musical_ops.hpp (PitchClassSet).

#include "musical/musical_ops.hpp"

namespace t7 {

inline PitchClassSet major_scale(int root = 0) {
    PitchClassSet s;
    // Major: W W H W W W H (0, 2, 4, 5, 7, 9, 11)
    constexpr int intervals[] = {0, 2, 4, 5, 7, 9, 11};
    for (int i : intervals) s.pitch_classes.set((root + i) % 12);
    return s;
}

inline PitchClassSet minor_scale(int root = 0) {
    PitchClassSet s;
    // Natural minor: W H W W H W W (0, 2, 3, 5, 7, 8, 10)
    constexpr int intervals[] = {0, 2, 3, 5, 7, 8, 10};
    for (int i : intervals) s.pitch_classes.set((root + i) % 12);
    return s;
}

inline PitchClassSet pentatonic_major(int root = 0) {
    PitchClassSet s;
    constexpr int intervals[] = {0, 2, 4, 7, 9};
    for (int i : intervals) s.pitch_classes.set((root + i) % 12);
    return s;
}

inline PitchClassSet chromatic_scale(int root = 0) {
    PitchClassSet s;
    s.pitch_classes.bits = 0x0FFF;  // All 12 bits
    return s;
}

} // namespace t7
