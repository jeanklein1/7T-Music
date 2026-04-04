#pragma once

/**
 * PLAYHEAD - Point-in-Time Musical Context
 * =========================================
 * 
 * Captures the context at a point in time: what's active at ANCHOR and what
 * just completed (PREVIOUS). Operations are defined separately.
 * 
 * DOMAIN
 * ------
 * 
 * The Playhead sees at most two sets of polyphonic events:
 * 
 *     PREVIOUS              ANCHOR (observation point)
 *         |                     |
 *     ---[###]---------------[###]-------> time
 *     (completed)            (sounding)
 * 
 * If ANCHOR is silent:
 * 
 *     PREVIOUS              ANCHOR
 *         |                     |
 *     ---[###]------------------o---------> time
 *                            (gap)
 * 
 * SNAPSHOT-BASED
 * --------------
 * 
 * The Playhead receives snapshots rather than holding stream references.
 * This enables parallel analysis and clean data flow.
 * 
 * DATA ONLY
 * ---------
 * 
 * PlayheadReadout contains raw data. Operations are pure functions
 * defined elsewhere that take the readout as input.
 * 
 * USAGE
 * -----
 * 
 *     MidiStream stream(router, channel);
 *     Playhead playhead;
 *     
 *     stream.update(current_beat);
 *     auto snap = stream.snapshot();
 *     playhead.update(snap, stream.history());
 *     
 *     const auto& r = playhead.readout();
 *     // Pass r to operations defined elsewhere
 */

#include "musical/stream_data.hpp"
#include <array>

namespace t7 {

// =============================================================================
// CONSTANTS
// =============================================================================

constexpr int PLAYHEAD_MAX_POLYPHONY = 16;

// =============================================================================
// CURRENT NOTE - Snapshot of a note sounding at anchor
// =============================================================================

/**
 * Raw data for a note that was sounding at anchor_beat.
 * 
 * For live playheads (offset=0), these are active notes.
 * For offset playheads, these may include notes that have since completed.
 * 
 * Duration at anchor computed as: anchor_beat - onset_beat
 */
struct CurrentNote {
    uint8_t pitch;
    uint8_t flags;
    uint8_t _pad[2];
    float velocity;
    float onset_beat;
    
    // Flags
    static constexpr uint8_t FLAG_STILL_ACTIVE = 0x01;  // Note is still sounding (not just at anchor)
    static constexpr uint8_t FLAG_FROM_HISTORY = 0x02;  // Reconstructed from completed ring
    
    bool still_active() const { return flags & FLAG_STILL_ACTIVE; }
    bool from_history() const { return flags & FLAG_FROM_HISTORY; }
};

static_assert(sizeof(CurrentNote) == 12, "CurrentNote should be 12 bytes");

// =============================================================================
// PLAYHEAD READOUT - Raw data, no derived computations
// =============================================================================

struct PlayheadReadout {
    // --- Temporal Context ---
    
    float current_beat = 0.0f;    // Real current time
    float anchor_beat = 0.0f;     // Observation point (current_beat - offset)
    float offset = 0.0f;          // How far behind real time
    
    // --- CURRENT: Notes sounding at anchor_beat ---
    
    std::array<CurrentNote, PLAYHEAD_MAX_POLYPHONY> current{};
    int current_count = 0;
    PitchBitmask current_mask;
    bool current_overflow = false;  // True if more notes than MAX_POLYPHONY
    
    // --- PREVIOUS: Most recently completed set before anchor ---
    
    std::array<CompletedNote, PLAYHEAD_MAX_POLYPHONY> previous{};
    int previous_count = 0;
    float previous_offset_beat = 0.0f;  // When PREVIOUS ended
    bool previous_overflow = false;
    
    // --- Transitions This Frame ---
    
    PitchBitmask onset_mask;      // Notes that appeared this frame
    PitchBitmask release_mask;    // Notes that disappeared this frame
    int onset_count = 0;
    int release_count = 0;
    
    // --- Temporal Measurements ---
    
    float gap_duration = 0.0f;    // If silent: time since PREVIOUS ended
    float state_duration = 0.0f;  // How long in current state (gate/silent)
    
    // --- State Flags ---
    
    bool is_onset = false;        // Became non-silent this frame
    bool is_release = false;      // Became silent this frame
    
    // =========================================================================
    // SIMPLE STATE QUERIES - No computation, just reading fields
    // =========================================================================
    
    bool gate() const { return current_count > 0; }
    bool silent() const { return current_count == 0; }
    bool has_previous() const { return previous_count > 0; }
    bool has_overflow() const { return current_overflow || previous_overflow; }
    
    // --- Duration helper (computed on demand) ---
    
    float current_duration(int index) const {
        if (index < 0 || index >= current_count) return 0.0f;
        return anchor_beat - current[index].onset_beat;
    }
    
    // --- Clear ---
    
    void clear() {
        current_beat = anchor_beat = offset = 0.0f;
        current_count = previous_count = 0;
        current_mask.clear_all();
        onset_mask.clear_all();
        release_mask.clear_all();
        onset_count = release_count = 0;
        gap_duration = state_duration = 0.0f;
        previous_offset_beat = 0.0f;
        is_onset = is_release = false;
        current_overflow = previous_overflow = false;
    }
};

// =============================================================================
// PLAYHEAD CLASS
// =============================================================================

class Playhead {
public:
    /**
     * Construct a Playhead.
     * 
     * @param offset_beats    How far behind real time (0 = live)
     * @param chord_tolerance Simultaneity tolerance for PREVIOUS detection (beats)
     */
    Playhead(float offset_beats = 0.0f, float chord_tolerance = 0.05f)
        : offset_beats_(offset_beats)
        , chord_tolerance_(chord_tolerance)
    {
        readout_.clear();
        prev_frame_mask_.clear_all();
    }
    
    // --- Update ---
    
    /**
     * Update the Playhead from a snapshot.
     * 
     * @param snap    Current ephemeral state (value, copied)
     * @param history Completed notes (read-only reference to append-only data)
     */
    void update(const StreamSnapshot& snap, const CompletedRing& history) {
        float anchor = snap.beat - offset_beats_;
        
        // Store previous frame state for transition detection
        PitchBitmask prev_mask = prev_frame_mask_;
        bool was_gate = readout_.gate();
        
        // Rebuild readout
        rebuild_current(snap, history, anchor);
        rebuild_previous(history, anchor);
        
        // Detect transitions
        detect_transitions(prev_mask, was_gate);
        
        // Update temporal measurements
        update_temporal(anchor, was_gate);
        
        // Store current mask for next frame
        prev_frame_mask_ = readout_.current_mask;
    }
    
    // --- Access ---
    
    const PlayheadReadout& readout() const { return readout_; }
    
    float offset() const { return offset_beats_; }
    void set_offset(float beats) { offset_beats_ = beats; }
    
    float chord_tolerance() const { return chord_tolerance_; }
    void set_chord_tolerance(float beats) { chord_tolerance_ = beats; }
    
private:
    float offset_beats_;
    float chord_tolerance_;
    
    PlayheadReadout readout_;
    PitchBitmask prev_frame_mask_;
    float state_onset_beat_ = 0.0f;
    
    void rebuild_current(const StreamSnapshot& snap, const CompletedRing& history, float anchor_beat) {
        readout_.current_beat = snap.beat;
        readout_.anchor_beat = anchor_beat;
        readout_.offset = offset_beats_;
        
        readout_.current_count = 0;
        readout_.current_mask.clear_all();
        readout_.current_overflow = false;
        
        if (offset_beats_ <= 0.0f) {
            // Live playhead: just use snapshot's active notes (fast path)
            snap.for_each_active([&](int pitch, const ActiveNote& note) {
                add_current_note(pitch, note.velocity, note.onset_beat, 
                                CurrentNote::FLAG_STILL_ACTIVE);
            });
        } else {
            // Offset playhead: reconstruct "what was sounding at anchor"
            
            // 1. Active notes that started at or before anchor
            //    (If they're still active now, they were definitely sounding at anchor)
            snap.for_each_active([&](int pitch, const ActiveNote& note) {
                if (note.onset_beat <= anchor_beat) {
                    add_current_note(pitch, note.velocity, note.onset_beat,
                                    CurrentNote::FLAG_STILL_ACTIVE);
                }
            });
            
            // 2. Completed notes that were sounding at anchor
            //    (onset <= anchor < offset)
            history.for_each([&](const CompletedNote& note) {
                if (note.onset_beat <= anchor_beat && note.offset_beat > anchor_beat) {
                    // Skip if we already added this pitch from active set
                    if (!readout_.current_mask.test(note.pitch)) {
                        add_current_note(note.pitch, note.velocity, note.onset_beat,
                                        CurrentNote::FLAG_FROM_HISTORY);
                    }
                }
            });
        }
    }
    
    void add_current_note(int pitch, float velocity, float onset_beat, uint8_t flags) {
        if (readout_.current_count >= PLAYHEAD_MAX_POLYPHONY) {
            readout_.current_overflow = true;
            return;
        }
        
        CurrentNote& cn = readout_.current[readout_.current_count];
        cn.pitch = static_cast<uint8_t>(pitch);
        cn.flags = flags;
        cn.velocity = velocity;
        cn.onset_beat = onset_beat;
        
        readout_.current_mask.set(pitch);
        ++readout_.current_count;
    }
    
    void rebuild_previous(const CompletedRing& history, float anchor_beat) {
        readout_.previous_overflow = false;
        readout_.previous_count = 0;
        
        int count;
        if (offset_beats_ <= 0.0f) {
            // Live playhead: most recent completed set
            count = history.most_recent_set(
                chord_tolerance_,
                readout_.previous.data(),
                PLAYHEAD_MAX_POLYPHONY
            );
        } else {
            // Offset playhead: completed set that ended before anchor
            count = most_recent_set_before(
                history,
                anchor_beat,
                chord_tolerance_,
                readout_.previous.data(),
                PLAYHEAD_MAX_POLYPHONY
            );
        }
        
        // Check if there might be overflow
        if (count == PLAYHEAD_MAX_POLYPHONY) {
            readout_.previous_overflow = true;
        }
        
        readout_.previous_count = count;
        
        if (count > 0) {
            readout_.previous_offset_beat = readout_.previous[0].offset_beat;
        } else {
            readout_.previous_offset_beat = 0.0f;
        }
    }
    
    // Helper: find most recent completed set before a given beat
    static int most_recent_set_before(const CompletedRing& history,
                                       float anchor_beat,
                                       float tolerance_beats,
                                       CompletedNote* out,
                                       int max_out) {
        if (history.size() == 0 || max_out <= 0) return 0;
        
        // Find the most recent note that completed before anchor
        float reference_beat = -1.0f;
        history.for_each_reverse([&](const CompletedNote& note) {
            if (reference_beat < 0.0f && note.offset_beat < anchor_beat) {
                reference_beat = note.offset_beat;
            }
        });
        
        if (reference_beat < 0.0f) return 0;
        
        float cutoff = reference_beat - tolerance_beats;
        int written = 0;
        
        history.for_each_reverse([&](const CompletedNote& note) {
            if (written >= max_out) return;
            if (note.offset_beat < anchor_beat && note.offset_beat >= cutoff) {
                out[written++] = note;
            }
        });
        
        return written;
    }
    
    void detect_transitions(const PitchBitmask& prev_mask, bool was_gate) {
        readout_.onset_mask.clear_all();
        readout_.release_mask.clear_all();
        readout_.onset_count = 0;
        readout_.release_count = 0;
        
        // Notes that appeared (in current but not in prev frame)
        readout_.current_mask.for_each([&](int pitch) {
            if (!prev_mask.test(pitch)) {
                readout_.onset_mask.set(pitch);
                ++readout_.onset_count;
            }
        });
        
        // Notes that disappeared (in prev frame but not in current)
        prev_mask.for_each([&](int pitch) {
            if (!readout_.current_mask.test(pitch)) {
                readout_.release_mask.set(pitch);
                ++readout_.release_count;
            }
        });
        
        // State transitions
        bool now_gate = readout_.gate();
        readout_.is_onset = !was_gate && now_gate;
        readout_.is_release = was_gate && !now_gate;
    }
    
    void update_temporal(float anchor_beat, bool was_gate) {
        bool now_gate = readout_.gate();
        
        if (was_gate != now_gate) {
            state_onset_beat_ = anchor_beat;
        }
        
        readout_.state_duration = anchor_beat - state_onset_beat_;
        
        if (!now_gate && readout_.previous_count > 0) {
            readout_.gap_duration = anchor_beat - readout_.previous_offset_beat;
        } else {
            readout_.gap_duration = 0.0f;
        }
    }
};

// =============================================================================
// SIZE VERIFICATION
// =============================================================================

// CurrentNote: 12 bytes
// CompletedNote: 16 bytes (from stream_data.hpp)

// PlayheadReadout memory:
//   current[16]        = 16 * 12 = 192 bytes
//   previous[16]       = 16 * 16 = 256 bytes  
//   masks (3)          = 3 * 16  = 48 bytes
//   scalars + flags    ~ 40 bytes
//   Total              ~ 536 bytes

// Playhead memory:
//   readout_           ~ 536 bytes
//   prev_frame_mask_   = 16 bytes
//   scalars            ~ 12 bytes
//   Total              ~ 564 bytes

} // namespace t7
