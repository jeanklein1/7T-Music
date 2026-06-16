#pragma once

// ─── musical_ops.hpp ─────────────────────────────────────────────
//
// Vocabulary only: the typed representations that are the currency of
// the analysis, plus the two pitch-class primitives every operation is
// built from. No extractors, no reductions, no relations — those are
// built when a concrete statistic makes them self-evident, against the
// readout they actually read (Playhead / Wagon / PreviousEvent).
//
// Representations : PitchClassBits (12-bit membership mask, a set with
//                   union/intersection/complement), PitchClassVector
//                   (12-float weighted distribution, the linear-algebra
//                   currency).
// Primitives      : pc_of (MIDI → pitch class), pc_relative_to (re-origin a
//                   pitch class — the reign's transpose), distance (signed
//                   registral interval between two pitches), to_degrees
//                   (re-origin a whole vector — pc_relative_to's rotation
//                   carried to the distribution).
//
// Depends on: <array>, <cmath>, <cstdint>. Nothing upstream.

#include <array>
#include <cmath>
#include <cstdint>

namespace t7 {

// ═══ PITCH-CLASS PRIMITIVES ══════════════════════════════════════

/**
 * Pitch class of a MIDI note in the default C-origin convention (C = 0).
 * Input: MIDI pitch (0-127). Output: pitch class 0-11.
 */
inline int pc_of(int midi_pitch) {
    return ((midi_pitch % 12) + 12) % 12;
}

/**
 * Re-express a pitch class with `origin` as zero (origin transpose).
 * `origin` is the C-origin index of the desired zero (e.g. 2 = D).
 * Input: pc 0-11, origin 0-11. Output: transposed pc 0-11.
 */
inline int pc_relative_to(int pc, int origin) {
    return ((pc - origin) % 12 + 12) % 12;
}

/**
 * Signed registral interval between two pitches, in semitones: the distance
 * travelled from `from` up to `to`, positive upward. The registral cousin of
 * pc_relative_to — that folds onto one class, this keeps the octave.
 */
inline int distance(int from, int to) {
    return to - from;
}

// ═══ REPRESENTATIONS ═════════════════════════════════════════════

// ── Pitch Class Bits ─────────────────────────────────────────────

/**
 * 12-bit mask representing pitch classes present.
 * Bit N = pitch class N is present (C=0, C#=1, ... B=11).
 *
 * The set representation: union (|), intersection (&), symmetric
 * difference (^), complement (~, masked to 12 bits).
 */
struct PitchClassBits {
    uint16_t bits = 0;

    void set(int pc) { bits = uint16_t(bits | (1u << (pc % 12))); }
    void clear(int pc) { bits = uint16_t(bits & ~(1u << (pc % 12))); }
    bool test(int pc) const { return (bits & (1u << (pc % 12))) != 0; }
    void clear_all() { bits = 0; }

    int count() const {
        uint16_t b = bits;
        b = uint16_t(b - ((b >> 1) & 0x5555));
        b = uint16_t((b & 0x3333) + ((b >> 2) & 0x3333));
        b = uint16_t((b + (b >> 4)) & 0x0F0F);
        return (b + (b >> 8)) & 0x1F;
    }

    bool empty() const { return bits == 0; }

    PitchClassBits operator&(PitchClassBits o) const { return {uint16_t(bits & o.bits)}; }
    PitchClassBits operator|(PitchClassBits o) const { return {uint16_t(bits | o.bits)}; }
    PitchClassBits operator^(PitchClassBits o) const { return {uint16_t(bits ^ o.bits)}; }
    PitchClassBits operator~() const { return {uint16_t(~bits & 0x0FFF)}; }

    bool operator==(PitchClassBits o) const { return bits == o.bits; }
    bool operator!=(PitchClassBits o) const { return bits != o.bits; }
};

// ── Pitch Class Vector ───────────────────────────────────────────

/**
 * 12-float vector: weight of each pitch class. The currency that edges
 * in the operation DAG carry. Weights are whatever the extractor lifts
 * in — velocity, length, length*velocity, or binary 0/1.
 */
struct PitchClassVector {
    std::array<float, 12> v = {};

    float& operator[](int pc) { return v[pc % 12]; }
    float operator[](int pc) const { return v[pc % 12]; }

    void clear() { v.fill(0.0f); }

    float sum() const {
        float s = 0.0f;
        for (float x : v) s += x;
        return s;
    }

    float max() const {
        float m = 0.0f;
        for (float x : v) if (x > m) m = x;
        return m;
    }

    /** Normalize to unit sum (a distribution). Identity if empty. */
    PitchClassVector normalized() const {
        float s = sum();
        if (s <= 0.0f) return *this;
        PitchClassVector r;
        for (int i = 0; i < 12; ++i) r.v[i] = v[i] / s;
        return r;
    }

    /** Normalize to unit L2 length. Identity if empty. */
    PitchClassVector unit() const {
        float sq = 0.0f;
        for (float x : v) sq += x * x;
        if (sq <= 0.0f) return *this;
        float norm = std::sqrt(sq);
        PitchClassVector r;
        for (int i = 0; i < 12; ++i) r.v[i] = v[i] / norm;
        return r;
    }
};

// ═══ RE-ORIGIN (VECTOR) ══════════════════════════════════════════

/**
 * Rotate a vector so `root_pc` lands on bin 0; bin i is then the degree i
 * semitones above the root. The vector form of pc_relative_to — one class
 * re-expressed becomes the whole distribution re-expressed. Root 0 (C) is
 * the identity.
 */
inline PitchClassVector to_degrees(const PitchClassVector& abs, int root_pc) {
    PitchClassVector p;
    int r = ((root_pc % 12) + 12) % 12;
    for (int i = 0; i < 12; ++i) p.v[i] = abs.v[(r + i) % 12];
    return p;
}

} // namespace t7
