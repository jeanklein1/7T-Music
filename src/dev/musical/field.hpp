#pragma once

// field.hpp ───────────────────────────────────────────────────────────────
//
// Locate a pitch-class set among declared harmonic fields. The barest harmonic
// reading there is.
//
// Take which pitch classes are sounding -- present (Playhead) OR completed in
// the recent window (Wagon) -- as a BINARY set: one per present class, no
// weight, no time. Re-origin it onto the home D, so each entry is a degree above
// the root. Dot it against each declared field, whose degrees are themselves a
// binary vector. The dot of two binary vectors is the count of shared degrees --
// how many of the sounding notes belong to that field. One number per field is
// the set's coordinates in field-space.
//
// What is deliberately ABSENT, to be added later one at a time, each leaving
// this map untouched: weighting (length / count / recency enrich the input
// vector's values), the in-minus-out penalty (refines the field masks), and the
// moment-to-moment held frame (acts on the stream of these vectors, downstream).
//
// Depends on: musical/playhead.hpp, musical/wagon.hpp, musical/musical_ops.hpp.

#include "musical/playhead.hpp"
#include "musical/wagon.hpp"
#include "musical/musical_ops.hpp"

namespace t7 {

// Binary presence over absolute pitch classes (C = 0): 1 if any note of that
// class is sounding now or completed in the window, else 0. Present and window
// are disjoint; OR-ing them cannot double-mark.
inline PitchClassVector present_set(const PlayheadReadout& ph,
                                    const WagonReadout& wg) {
    PitchClassVector s;
    for (int i = 0; i < wg.note_count; ++i)   s.v[wg.notes[i].pitch % 12] = 1.0f;
    for (int i = 0; i < ph.current_count; ++i) s.v[ph.current[i].pitch % 12] = 1.0f;
    return s;
}

// Presence union across channels: a class is in the union iff it is present in
// ANY of the given per-channel sets -- "sounding somewhere on the field". Each
// input is a binary present_set, so the union is their element-wise maximum,
// which cannot pass 1; the result is again a clean binary set. The field reads
// it exactly as it reads one voice's set -- the rise from a voice to a group is
// invisible past this point, and that invisibility is the whole mechanism: a
// reading rises by widening its source, with nothing downstream changed.
inline PitchClassVector present_union(const PitchClassVector* sets, int n) {
    PitchClassVector u;
    for (int c = 0; c < n; ++c)
        for (int i = 0; i < 12; ++i)
            if (sets[c].v[i] > u.v[i]) u.v[i] = sets[c].v[i];
    return u;
}

// to_degrees — the re-origin onto the root — now lives in musical_ops, beside
// pc_relative_to, since the dressing and the spine reach for it too. field.hpp
// includes musical_ops, so the rotation is in scope here unchanged.

// A declared field: a name and its degrees as a root-position binary vector
// (bin 0 = root). The bank of fields is the axes of the musical space; it is
// composition policy, declared by the caller.
struct Field {
    const char*      name;
    PitchClassVector mask;
};

// Build a field mask from a root-position degree list (semitones above root).
inline PitchClassVector field_mask(std::initializer_list<int> degrees) {
    PitchClassVector m;
    for (int d : degrees) m.v[((d % 12) + 12) % 12] = 1.0f;
    return m;
}

// Overlap of a degree-set with a field: shared degrees. For binary inputs this
// is the count of the set's degrees that lie in the field.
inline float field_overlap(const PitchClassVector& degrees,
                           const PitchClassVector& mask) {
    float s = 0.0f;
    for (int i = 0; i < 12; ++i) s += degrees.v[i] * mask.v[i];
    return s;
}

// The result of electing one field from the bank by overlap and hierarchy.
struct FieldChoice {
    int   index = 0;     // chosen field's position in the bank
    float overlap = 0.0f; // its overlap with the set
    int   tie = 0;        // how many fields share the top overlap (1 = clear)

    bool ambiguous() const { return tie > 1; }
};

// Elect a field: the MAXIMUM overlap, ties broken by HIERARCHY -- and the bank's
// order is the hierarchy, earlier = higher. The first pass keeps the earliest
// field at the top overlap (a later equal never displaces it), so a lower-ranked
// field wins only by strictly exceeding every field above it -- which it can do
// only by holding a degree they lack, its signature. `tie` counts how many share
// the top, so ambiguity stays visible rather than hidden behind the pick.
inline FieldChoice elect_field(const PitchClassVector& degrees,
                               const Field* bank, int n) {
    FieldChoice c;
    c.overlap = -1.0f;
    for (int i = 0; i < n; ++i) {
        const float ov = field_overlap(degrees, bank[i].mask);
        if (ov > c.overlap) { c.overlap = ov; c.index = i; }   // strict: earlier wins ties
    }
    for (int i = 0; i < n; ++i)
        if (field_overlap(degrees, bank[i].mask) == c.overlap) ++c.tie;
    return c;
}

// The held field: the standing harmonic reading. This is the one stateful piece
// of the field card. It holds the incumbent through ambiguity and moves only
// when another field strictly out-scores it -- the cascade as persistence:
//
//   - no incumbent yet            -> take the elected leader        (bootstrap)
//   - some field strictly beats it -> move to the elected leader     (the music)
//   - otherwise                    -> hold                           (persistence)
//
// "Otherwise" covers two cases at once. A draw that still includes the incumbent
// (its overlap equals the maximum) holds, because the elected leader's overlap
// does not exceed it. And silence holds for the same reason without a special
// case: an empty set scores zero on every field, so the incumbent is tied for
// the top at zero and is not beaten. The threshold that would one day let a
// challenger need MORE than a bare strict win, or let silence eventually release
// the field, attaches to these two edges later; here a single strict beat moves
// it and silence holds it forever.
struct HeldField {
    int incumbent = -1;   // -1 = none yet

    int step(const PitchClassVector& degrees, const Field* bank, int n) {
        const FieldChoice pick = elect_field(degrees, bank, n);
        if (pick.overlap <= 0.0f) return incumbent;             // empty / silence: hold
        if (incumbent < 0) { incumbent = pick.index; return incumbent; }   // bootstrap
        const float held = field_overlap(degrees, bank[incumbent].mask);
        if (pick.overlap > held) incumbent = pick.index;        // strictly beaten: move
        return incumbent;                                        // else hold
    }

    bool settled() const { return incumbent >= 0; }
};

} // namespace t7
