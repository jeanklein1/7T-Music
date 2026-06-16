#pragma once

// ─── previous_event.hpp ──────────────────────────────────────────
//
// Holds the two most recent onset-groups: the open one being struck now —
// the CURRENT event — and the prior one — the PREVIOUS event — latched. A
// latch, not a window: the previous survives any silence. Fed onset events;
// notes struck within a simultaneity tolerance join the open group; when a
// new group opens beyond tolerance, the open group becomes previous and is
// held until the next opens.
//
// An event is defined by ONSET: notes struck together are one group, and the
// grouping never consults release. But the previous event's REPRESENTATIVE —
// the single note that speaks for the group in a comparison — is the voice
// that goes offset LAST, the one that lingered to the seam. So onset groups;
// offset elects. Releases are recorded (on_offset) for that election; a
// still-sounding voice counts as the latest possible offset, so a held note
// outranks any that has already let go. The minimal-motion closest_pitch
// remains as an alternate criterion.
//
// This is the fast instance of the held-value-surviving-silence primitive;
// the reign is the slow instance.
//
// Times are assumed monotonic; on a discontinuity the caller calls clear().
//
// Depends on: <array>, <cstdint>, <limits>.

#include <array>
#include <cstdint>
#include <limits>

namespace t7 {

constexpr int   PREVIOUS_GROUP_MAX = 16;

// A still-sounding note ranks as the latest possible offset, so the
// last-to-offset election prefers a voice that is still being held.
constexpr float PREV_SOUNDING = std::numeric_limits<float>::infinity();

// ═══ PREVIOUS NOTE ═══════════════════════════════════════════════
//
// A note as latched at its onset, with its release recorded when it comes:
// offset stays PREV_SOUNDING until the note goes silent. The pair (onset,
// offset) is what the last-to-offset election reads.

struct PrevNote {
    uint8_t pitch = 0;
    uint8_t _pad[3] = {0, 0, 0};
    float   velocity   = 0.0f;
    float   onset_beat = 0.0f;
    float   offset     = PREV_SOUNDING;   // release time; PREV_SOUNDING while still on
};

static_assert(sizeof(PrevNote) == 16, "PrevNote should be 16 bytes");

// ═══ PREVIOUS EVENT ══════════════════════════════════════════════

class PreviousEvent {
public:
    explicit PreviousEvent(float tolerance = 0.1f) : tolerance_(tolerance) {}

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

    // Feed a note release. Records the offset on the matching still-sounding
    // note — current group first, then previous — so the previous event can
    // elect the voice that goes offset last.
    void on_offset(int pitch, float time) {
        for (int i = 0; i < open_count_; ++i)
            if (open_[i].pitch == pitch && open_[i].offset == PREV_SOUNDING) { open_[i].offset = time; return; }
        for (int i = 0; i < previous_count_; ++i)
            if (previous_[i].pitch == pitch && previous_[i].offset == PREV_SOUNDING) { previous_[i].offset = time; return; }
    }

    // --- the previous group (held) ---

    bool            has_previous() const { return previous_count_ > 0; }
    int             previous_count() const { return previous_count_; }
    bool            previous_overflow() const { return previous_overflow_; }
    const PrevNote& previous_note(int i) const { return previous_[i]; }
    float           previous_onset() const { return previous_onset_; }

    // The previous member that goes offset last — the voice that lingered to
    // the seam (a still-sounding member counts as latest). Tie -> higher pitch.
    // Returns -1 if there is no previous. This is the previous event's chosen
    // representative; the signed interval to a current note is a musical_op
    // built on its pitch.
    int previous_last_offset_index() const {
        if (previous_count_ == 0) return -1;
        int best = 0;
        for (int i = 1; i < previous_count_; ++i) {
            const float oi = previous_[i].offset, ob = previous_[best].offset;
            if (oi > ob || (oi == ob && previous_[i].pitch > previous_[best].pitch)) best = i;
        }
        return best;
    }
    int previous_last_offset_pitch() const {
        const int i = previous_last_offset_index();
        return i < 0 ? -1 : previous_[i].pitch;
    }

    // --- the current group (open): the cluster being struck now ---

    bool            has_open() const { return open_count_ > 0; }
    int             open_count() const { return open_count_; }
    bool            open_overflow() const { return open_overflow_; }
    const PrevNote& open_note(int i) const { return open_[i]; }
    float           open_onset() const { return open_onset_; }

    // Time since the previous group was struck. Caller checks has_previous().
    float temporal_distance(float anchor) const {
        return has_previous() ? anchor - previous_onset_ : 0.0f;
    }

    // The previous member nearest `target` (minimal motion). Tie -> higher.
    // Returns -1 if there is no previous. An alternate criterion to the
    // last-to-offset election above.
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
        n.offset     = PREV_SOUNDING;   // still sounding until released
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
