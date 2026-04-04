#pragma once

/**
 * MIDI STREAM - The River of Musical Events
 * ==========================================
 * 
 * One Stream per MIDI channel. The single source of truth for that channel.
 * 
 * INERT CONSTRUCTION
 * ------------------
 * 
 * The stream is constructed without any external coupling. It receives
 * events explicitly via receive(), enabling:
 * - Fixed array storage (no heap allocation)
 * - Testability (inject events directly)
 * - Visible control flow (composition routes events)
 * 
 * RESPONSIBILITIES
 * ----------------
 * 
 * - Receive note events via receive()
 * - Maintain ActiveSet (currently sounding)
 * - Maintain CompletedRing (finished sounding)
 * - Prune history beyond retention window
 * - Produce snapshots for analyzers
 * 
 * TIME INVARIANT
 * --------------
 * 
 * The stream assumes time (beat) is monotonically increasing. If time jumps
 * backward (loop, seek), the stream automatically clears all state to maintain
 * consistency.
 * 
 * USAGE
 * -----
 * 
 *     MidiStream streams[4];
 *     
 *     // Configure
 *     for (int i = 0; i < 4; ++i) {
 *         streams[i].set_channel(i);
 *     }
 *     
 *     // Route events (from MidiFile, Keyboard, etc)
 *     MidiEvent event = MidiEvent::note_on(2, 60, 0.8f, beat);
 *     streams[event.channel].receive(event);
 *     
 *     // Update (prunes old history)
 *     for (auto& s : streams) s.update(beat);
 *     
 *     // Snapshot for analyzers
 *     auto snap = streams[2].snapshot();
 */

#include "musical/stream_data.hpp"
#include "sources/midi_event.hpp"

namespace t7 {

// =============================================================================
// MIDI STREAM
// =============================================================================

class MidiStream {
public:
    /**
     * Default constructor - inert, no external coupling.
     */
    MidiStream() = default;
    
    /**
     * Construct with channel assignment.
     */
    explicit MidiStream(int channel, float retention_beats = 64.0f)
        : channel_(channel)
        , retention_beats_(retention_beats)
    {}
    
    // Non-copyable (owns significant state)
    MidiStream(const MidiStream&) = delete;
    MidiStream& operator=(const MidiStream&) = delete;
    
    // Movable
    MidiStream(MidiStream&&) = default;
    MidiStream& operator=(MidiStream&&) = default;
    
    // =========================================================================
    // CONFIGURATION
    // =========================================================================
    
    void set_channel(int channel) { channel_ = channel; }
    int channel() const { return channel_; }
    
    void set_retention_beats(float beats) { retention_beats_ = beats; }
    float retention_beats() const { return retention_beats_; }
    
    // =========================================================================
    // EVENT RECEPTION
    // =========================================================================
    
    /**
     * Receive a MIDI event.
     * Called by composition to route events to this stream.
     */
    void receive(const MidiEvent& event) {
        if (event.type == MidiEvent::NOTE_ON) {
            active_.note_on(event.pitch, event.velocity, event.beat);
        } else {
            ActiveNote was_active = active_.note_off(event.pitch);
            if (was_active.is_active()) {
                completed_.push(event.pitch, was_active.velocity, 
                               was_active.onset_beat, event.beat);
            }
        }
    }
    
    // =========================================================================
    // FRAME UPDATE
    // =========================================================================
    
    /**
     * Advance stream time and prune old history.
     * Call once per frame after routing all events.
     * 
     * IMPORTANT: If beat jumps backward (loop/seek), the stream automatically
     * clears all state. This maintains the monotonic time invariant.
     */
    void update(float current_beat) {
        // Detect backward time jump (loop/seek)
        constexpr float BACKWARD_THRESHOLD = 0.001f;
        had_discontinuity_ = false;
        
        if (current_beat < current_beat_ - BACKWARD_THRESHOLD) {
            clear();
            had_discontinuity_ = true;
        }
        
        current_beat_ = current_beat;
        
        // Prune completed notes older than retention window
        float cutoff = current_beat - retention_beats_;
        completed_.prune_before(cutoff);
    }
    
    // =========================================================================
    // SNAPSHOT
    // =========================================================================
    
    /**
     * Produce a snapshot of current ephemeral state.
     * 
     * This is a value type - safe to copy, store, pass to parallel analysis.
     */
    StreamSnapshot snapshot() const {
        StreamSnapshot snap;
        snap.beat = current_beat_;
        snap.channel = channel_;
        snap.active = active_;
        return snap;
    }
    
    /**
     * Access to history (read-only).
     * CompletedRing is append-only facts, safe for concurrent reads.
     */
    const CompletedRing& history() const { return completed_; }
    
    // =========================================================================
    // QUERIES
    // =========================================================================
    
    float current_beat() const { return current_beat_; }
    bool had_time_discontinuity() const { return had_discontinuity_; }
    
    int active_count() const { return active_.count(); }
    bool is_silent() const { return active_.empty(); }
    bool has_active() const { return !active_.empty(); }
    
    int completed_count() const { return completed_.size(); }
    bool has_completed() const { return !completed_.empty(); }
    
    // =========================================================================
    // DIRECT ACCESS
    // =========================================================================
    
    const ActiveSet& active_set() const { return active_; }
    const CompletedRing& completed_ring() const { return completed_; }
    
    // =========================================================================
    // STATE MANAGEMENT
    // =========================================================================
    
    /**
     * Clear all state. Use when seeking or resetting playback.
     * Called automatically if update() detects backward time jump.
     */
    void clear() {
        active_.clear();
        completed_.clear();
    }
    
private:
    int channel_ = 0;
    float retention_beats_ = 64.0f;
    float current_beat_ = 0.0f;
    bool had_discontinuity_ = false;
    
    ActiveSet active_;
    CompletedRing completed_;
};

// =============================================================================
// SIZE VERIFICATION
// =============================================================================

// MidiStream memory:
//   ActiveSet     ~  1 KB
//   CompletedRing ~ 32 KB
//   Scalars       ~ 16 bytes
//   Total         ~ 33 KB per channel

} // namespace t7
