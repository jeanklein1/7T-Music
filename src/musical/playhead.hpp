#pragma once

// ─── playhead.hpp ────────────────────────────────────────────────
//
// Present-only context. The Playhead reads the active set at the anchor
// and reports the present: which notes sound now, whether there is signal
// or silence, what changed this frame, and how long the present state has
// held. It holds NO memory of the past — the previous note lives in its own
// object (previous_event.hpp), on a different clock.
//
// What it reads: the stream snapshot (active notes) only. No history. The
// anchor is the snapshot beat — the live present.
//
// Reworked against target spec §2. Removed from the prior version: the
// PREVIOUS set, gap_duration, the offset / chord-tolerance machinery, and
// the history dependency. Those concerns moved out (previous_event.hpp) or
// retired with the offset-playhead generalization.
//
// Depends on: musical/stream_data.hpp (StreamSnapshot, ActiveNote,
//             PitchBitmask), <array>, <cstdint>.

#include "musical/stream_data.hpp"
#include <array>
#include <cstdint>

namespace t7 {

constexpr int PLAYHEAD_MAX_POLYPHONY = 16;

// ═══ CURRENT NOTE ════════════════════════════════════════════════
//
// A note sounding at the anchor. onset_beat is kept so an operation can
// take the note's provisional in-window length (anchor − onset) — the
// active contribution the combine sums against the Wagon's completed part.

struct CurrentNote {
    uint8_t pitch = 0;
    uint8_t _pad[3] = {0, 0, 0};
    float   velocity = 0.0f;
    float   onset_beat = 0.0f;
};

static_assert(sizeof(CurrentNote) == 12, "CurrentNote should be 12 bytes");

// ═══ PLAYHEAD READOUT ════════════════════════════════════════════
//
// Raw present data. No held memory, no derived folds.

struct PlayheadReadout {
    float anchor_beat = 0.0f;          // the present moment

    // --- the present ---
    std::array<CurrentNote, PLAYHEAD_MAX_POLYPHONY> current{};
    int          current_count = 0;
    PitchBitmask current_mask;
    bool         current_overflow = false;   // more than MAX_POLYPHONY notes

    // --- transitions this frame (the synchronicity edges) ---
    PitchBitmask onset_mask;
    PitchBitmask release_mask;
    int          onset_count = 0;
    int          release_count = 0;

    // --- gate state ---
    bool  is_onset = false;            // became non-silent this frame
    bool  is_release = false;          // became silent this frame
    float state_duration = 0.0f;       // how long the present state has held

    bool gate()        const { return current_count > 0; }
    bool silent()      const { return current_count == 0; }
    bool has_overflow() const { return current_overflow; }

    // Provisional length of an active note at the anchor.
    float current_duration(int i) const {
        return (i >= 0 && i < current_count)
                   ? anchor_beat - current[i].onset_beat
                   : 0.0f;
    }

    void clear() {
        anchor_beat = 0.0f;
        current_count = 0;
        current_mask.clear_all();
        current_overflow = false;
        onset_mask.clear_all();
        release_mask.clear_all();
        onset_count = release_count = 0;
        is_onset = is_release = false;
        state_duration = 0.0f;
    }
};

// ═══ PLAYHEAD ════════════════════════════════════════════════════

class Playhead {
public:
    Playhead() {
        readout_.clear();
        prev_frame_mask_.clear_all();
    }

    // Update from the present snapshot. Reads active notes only.
    void update(const StreamSnapshot& snap) {
        const PitchBitmask prev_mask = prev_frame_mask_;
        const bool was_gate = readout_.gate();

        rebuild_current(snap);
        detect_transitions(prev_mask);
        update_temporal(was_gate);

        prev_frame_mask_ = readout_.current_mask;
    }

    const PlayheadReadout& readout() const { return readout_; }

    // Reset on a stream discontinuity (seek / state clear).
    void clear() {
        readout_.clear();
        prev_frame_mask_.clear_all();
        state_onset_beat_ = 0.0f;
    }

private:
    PlayheadReadout readout_;
    PitchBitmask    prev_frame_mask_;
    float           state_onset_beat_ = 0.0f;

    void rebuild_current(const StreamSnapshot& snap) {
        readout_.anchor_beat = snap.beat;
        readout_.current_count = 0;
        readout_.current_mask.clear_all();
        readout_.current_overflow = false;

        snap.for_each_active([&](int pitch, const ActiveNote& note) {
            if (readout_.current_count >= PLAYHEAD_MAX_POLYPHONY) {
                readout_.current_overflow = true;
                return;
            }
            CurrentNote& cn = readout_.current[readout_.current_count];
            cn.pitch      = static_cast<uint8_t>(pitch);
            cn.velocity   = note.velocity;
            cn.onset_beat = note.onset_beat;
            readout_.current_mask.set(pitch);
            ++readout_.current_count;
        });
    }

    void detect_transitions(const PitchBitmask& prev_mask) {
        readout_.onset_mask.clear_all();
        readout_.release_mask.clear_all();
        readout_.onset_count = 0;
        readout_.release_count = 0;

        readout_.current_mask.for_each([&](int pitch) {
            if (!prev_mask.test(pitch)) {
                readout_.onset_mask.set(pitch);
                ++readout_.onset_count;
            }
        });
        prev_mask.for_each([&](int pitch) {
            if (!readout_.current_mask.test(pitch)) {
                readout_.release_mask.set(pitch);
                ++readout_.release_count;
            }
        });
    }

    void update_temporal(bool was_gate) {
        const bool now_gate = readout_.gate();
        readout_.is_onset   = !was_gate &&  now_gate;
        readout_.is_release =  was_gate && !now_gate;
        if (was_gate != now_gate) {
            state_onset_beat_ = readout_.anchor_beat;
        }
        readout_.state_duration = readout_.anchor_beat - state_onset_beat_;
    }
};

} // namespace t7
