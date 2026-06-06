#pragma once

// ─── previous_event.hpp ──────────────────────────────────────────
//
// Held memory of the prior onset-group. A latch, not a window: it survives
// any silence. Fed onset events; groups notes struck within a simultaneity
// tolerance; when a new group opens, the open group becomes "previous" and
// is held until the next group opens.
//
// "Previous" is defined by ONSET, never release — melodic motion is
// attack-to-attack. The melodic interval is the shortest distance from a
// current note to a member of the previous group (minimal motion; no voice
// tracking). All of the group's onset-time data is kept (pitch, velocity,
// onset beat). The temporal distance to a given anchor is anchor − onset.
//
// This is the fast instance of the held-value-surviving-silence primitive;
// the reign is the slow instance.
//
// Target spec §3. Pure event-driven state machine: no span, no pruning, no
// release logic, no voice tracking. Beats are assumed monotonic (the stream
// invariant); on a discontinuity the caller calls clear().
//
// Depends on: <array>, <cstdint>.

#include <array>
#include <cstdint>

namespace t7 {

constexpr int PREVIOUS_GROUP_MAX = 16;

// ═══ PREVIOUS NOTE ═══════════════════════════════════════════════
//
// A note as latched at its onset. Offset is not tracked: the group is an
// onset construct, and no relation here needs the release.

struct PrevNote {
    uint8_t pitch = 0;
    uint8_t _pad[3] = {0, 0, 0};
    float   velocity = 0.0f;
    float   onset_beat = 0.0f;
};

static_assert(sizeof(PrevNote) == 12, "PrevNote should be 12 bytes");

// ═══ PREVIOUS EVENT ══════════════════════════════════════════════

class PreviousEvent {
public:
    explicit PreviousEvent(float tolerance = 0.05f) : tolerance_(tolerance) {}

    void  set_tolerance(float beats) { tolerance_ = beats; }
    float tolerance() const { return tolerance_; }

    // Feed a note onset. Runs the grouping state machine; may latch the open
    // group as "previous".
    void on_onset(int pitch, float velocity, float beat) {
        if (open_count_ == 0) {
            open_onset_ = beat;
            push_open(pitch, velocity, beat);
        } else if (beat - open_onset_ <= tolerance_) {
            push_open(pitch, velocity, beat);          // same group
        } else {
            latch();                                   // open group -> previous
            open_onset_ = beat;
            push_open(pitch, velocity, beat);          // start a new group
        }
    }

    // --- the previous group (held) ---

    bool            has_previous() const { return previous_count_ > 0; }
    int             previous_count() const { return previous_count_; }
    bool            previous_overflow() const { return previous_overflow_; }
    const PrevNote& previous_note(int i) const { return previous_[i]; }
    float           previous_onset() const { return previous_onset_; }

    // Time since the previous group was struck. Caller checks has_previous().
    float temporal_distance(float anchor) const {
        return has_previous() ? anchor - previous_onset_ : 0.0f;
    }

    // The previous member nearest `target` (minimal motion). Tie -> higher.
    // Returns -1 if there is no previous. The signed melodic interval
    // (target − closest_pitch) is a musical_op built on this.
    int closest_pitch(int target) const {
        if (previous_count_ == 0) return -1;
        int best      = previous_[0].pitch;
        int best_dist = iabs(int(previous_[0].pitch) - target);
        for (int i = 1; i < previous_count_; ++i) {
            const int p = previous_[i].pitch;
            const int d = iabs(p - target);
            if (d < best_dist || (d == best_dist && p > best)) {
                best      = p;
                best_dist = d;
            }
        }
        return best;
    }

    // Reset on a stream discontinuity (seek / state clear).
    void clear() {
        open_count_        = 0;
        open_overflow_     = false;
        previous_count_    = 0;
        previous_overflow_ = false;
        previous_onset_    = 0.0f;
    }

private:
    static int iabs(int x) { return x < 0 ? -x : x; }

    void push_open(int pitch, float velocity, float beat) {
        if (open_count_ >= PREVIOUS_GROUP_MAX) { open_overflow_ = true; return; }
        PrevNote& n  = open_[open_count_];
        n.pitch      = static_cast<uint8_t>(pitch);
        n.velocity   = velocity;
        n.onset_beat = beat;
        ++open_count_;
    }

    void latch() {
        previous_          = open_;
        previous_count_    = open_count_;
        previous_overflow_ = open_overflow_;
        previous_onset_    = open_onset_;
        open_count_        = 0;
        open_overflow_     = false;
    }

    float tolerance_;

    // open (in-progress) onset group
    std::array<PrevNote, PREVIOUS_GROUP_MAX> open_{};
    int   open_count_    = 0;
    float open_onset_    = 0.0f;
    bool  open_overflow_ = false;

    // previous (held) onset group
    std::array<PrevNote, PREVIOUS_GROUP_MAX> previous_{};
    int   previous_count_    = 0;
    float previous_onset_    = 0.0f;
    bool  previous_overflow_ = false;
};

} // namespace t7
