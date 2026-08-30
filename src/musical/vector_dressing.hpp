#pragma once

// ─── vector_dressing.hpp ─────────────────────────────────────────
//
// The dressing a published pitch-class vector carries: an optional
// re-origin and an optional scale, applied as the last step before the
// vector reaches a sink. It is terminal — it shapes a vector for output,
// it never produces one. A measure makes the vector; this dresses it.
//
// Both parts are per-vector and opt-in, because the choice belongs to a
// representation, not to the music. The re-origin rotates the vector so a
// chosen root lands on bin zero, the degrees rising above it; off, or at
// root C, it leaves the vector absolute — the two are the same, since C is
// the identity rotation. The scale either ships the raw weights, rescales
// them to a distribution summing to one, or rescales them to unit length.
//
// One re-origin. The rotation here is `to_degrees`, the vector form of
// `pc_relative_to`; both now live in musical_ops, so dressing a vector
// depends on the floor alone and no longer reaches into the field header.
//
// Depends on: musical/musical_ops.hpp (PitchClassVector, to_degrees).

#include "musical/musical_ops.hpp"

namespace t7 {

// The dressing carried by one published vector.
struct VectorDressing {
    // Re-origin. Off (or root C) leaves the vector absolute; on rotates so
    // `root` lands on bin 0. Root is a pitch class, C = 0 the default.
    bool reorigin = false;
    int  root     = 0;

    // Scale. None ships the raw weights; Normalized rescales to unit sum
    // (a distribution); Unit rescales to unit L2 length.
    enum class Scale { None, Normalized, Unit };
    Scale scale = Scale::None;
};

// Apply a vector's dressing: re-origin, then scale. The scalers are
// rotation-invariant — sum and length are unchanged by a permutation of
// bins — so the order does not change the result; re-origin then scale just
// reads as representation before magnitude.
inline PitchClassVector dress(const PitchClassVector& v, const VectorDressing& d) {
    PitchClassVector out = d.reorigin ? to_degrees(v, d.root) : v;
    switch (d.scale) {
        case VectorDressing::Scale::Normalized: return out.normalized();
        case VectorDressing::Scale::Unit:       return out.unit();
        case VectorDressing::Scale::None:       return out;
    }
    return out;  // unreachable; quiets the compiler
}

} // namespace t7
