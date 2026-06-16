#pragma once

// ─── note_select.hpp ─────────────────────────────────────────────
//
// Selection over a collection of notes: pick one note under a criterion,
// nothing if the collection is empty. A SELECTION in note space, orthogonal
// to the pitch-class reductions — those tally what occurred, a measure, and
// measures add; this elects one note, a candidate frame, and frames are
// chosen, not summed.
//
// The criterion is a `beats` predicate: beats(a, b) is true when a should win
// over b. Lowest is beats = (a.pitch < b.pitch); highest, longest, loudest
// swap the predicate. The elected note is the raw material of a reign — the
// harmonic ground for the bars ahead — once a hold gives it duration (see
// reign.hpp). The bare select is the instant; the reign is its tenure.
//
// What this deliberately leaves to later layers:
//   · scope    — selecting over a window of time vs a bare collection
//   · silence  — what the empty result becomes (the reign's decay)
//
// Depends on: <optional>. Generic over any note type carrying the fields the
// predicate reads (e.g. .pitch).

#include <optional>

namespace t7 {

// Select one note from a collection under a criterion. Returns the note for
// which `beats` holds against every other; nothing if the collection is empty.
template <class Note, class Beats>
std::optional<Note> select(const Note* notes, int count, Beats beats) {
    if (count <= 0) return std::nullopt;
    int best = 0;
    for (int i = 1; i < count; ++i)
        if (beats(notes[i], notes[best])) best = i;
    return notes[best];
}

// The first criterion: least pitch. (Highest, longest, loudest are the same
// select with a different predicate.)
template <class Note>
std::optional<Note> select_lowest(const Note* notes, int count) {
    return select(notes, count, [](const Note& a, const Note& b) { return a.pitch < b.pitch; });
}

} // namespace t7
