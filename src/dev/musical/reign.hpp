#pragma once

// ─── reign.hpp ───────────────────────────────────────────────────
//
// The held harmonic ground — a frame, not a measurement. An elected note,
// the bass, takes the throne and governs the bars ahead. The reign is
// forward-looking: it does not summarize the past the way count, length, and
// set do; it sets the context the coming material will be heard against. That
// is why it persists — a frame governs the future until replaced, so it
// outlives the note that struck it.
//
// Fed each frame with the current selection and the current beat:
//   fresh, non-empty → a live note takes the throne, resetting the clock
//   fresh, empty     → the ground holds through `decay` beats of silence,
//                      then the reign is over
//
// FALLBACK. A reference frame must always have a value — you cannot compare
// against nothing. So when the reign is over (and before any note has reigned)
// the ground is the fallback: a home chosen by ANOTHER criterion, mirroring
// the start of the chain. There a criterion picked the lowest from the live
// collection; here a different criterion picks the home for when there is no
// live collection. In one composition the home is a D; the reign holds only
// the resulting note, not the criterion that chose it. With no fallback set,
// the reign is over means null.
//
// `decay` is the reign-length when the source is the bare present, which has
// no window of its own. The reign is a frame: it does not combine by addition
// — origins do not sum — so two sources are composed by precedence, never
// blended. Decay 0 is no hold.
//
// Stateful, unlike the pure select it wraps. Depends on: <optional>.

#include <optional>

namespace t7 {

template <class Note>
class Reign {
public:
    void  set_decay(float beats)       { decay_ = beats; }
    float decay() const                { return decay_; }

    // The home reference — the fallback criterion's result for this
    // composition. Held only as a value; the criterion lives at the caller.
    void  set_fallback(const Note& fb) { fallback_ = fb; }
    void  clear_fallback()             { fallback_.reset(); }

    void update(const std::optional<Note>& fresh, float beat) {
        if (beat < last_seen_) last_seen_ = beat;             // backward jump (loop/seek)
        if (fresh) { held_ = fresh; last_seen_ = beat; }      // a live note takes the throne
        else if (held_ && (beat - last_seen_) >= decay_)      // silence outlasted the decay
            held_.reset();
    }

    // The reference frame: the reigning election while one holds, otherwise
    // the fallback. Defined whenever a fallback is set — a frame must always
    // have a value for the comparisons it grounds.
    std::optional<Note> note() const { return held_ ? held_ : fallback_; }

    // True only for a live or held election — not when standing on the fallback.
    bool reigning() const { return held_.has_value(); }
    void clear()          { held_.reset(); }

private:
    float               decay_     = 0.0f;   // reign-length in beats; set via set_decay
    std::optional<Note> held_;
    std::optional<Note> fallback_;           // home reference; empty = no fallback (null)
    float               last_seen_ = 0.0f;
};

} // namespace t7
