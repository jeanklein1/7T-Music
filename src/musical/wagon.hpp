#pragma once

// ─── wagon.hpp ───────────────────────────────────────────────────
//
// Time-window musical context: captures what notes exist within a span
// of time. Operations are defined separately (musical_ops.hpp).
//
// Domain — the Wagon sees completed notes within a time window:
//
//     window_start              window_end (anchor)
//          |                        |
//     -----+---[##]--[###]--[#]-----+---------> time
//          |________ span __________|
//
// By default only notes entirely inside the window are included;
// straddling notes can be included via configuration.
//
// Snapshot-based: the Wagon receives snapshots rather than holding
// stream references, which enables parallel analysis and clean data
// flow. Data only — WagonReadout holds raw data; operations are pure
// functions elsewhere that take the readout as input.
//
// Usage:
//   MidiStream stream(router, channel);
//   Wagon wagon(4.0f);  // 4-beat window
//   stream.update(current_beat);
//   auto snap = stream.snapshot();
//   wagon.update(snap, stream.history());
//   const auto& r = wagon.readout();
//
// Depends on: musical/stream_data.hpp (StreamSnapshot, CompletedRing),
//             <array>.

#include "musical/stream_data.hpp"
#include <array>

namespace t7 {

// ═══ CONSTANTS ═══════════════════════════════════════════════════

constexpr int WAGON_MAX_NOTES = 128;

// ═══ WINDOW NOTE - A note within the wagon's window ══════════════

/**
 * A note as seen through the wagon's window.
 * 
 * Contains the original note data plus window-relative information.
 * If the note straddles a window boundary (when include_straddling is true),
 * the clipped times indicate how much of the note is visible.
 */
struct WindowNote {
    // Original note data
    uint8_t pitch;
    uint8_t flags;
    uint8_t _pad[2];
    float velocity;
    float onset_beat;
    float offset_beat;
    
    // Window-relative (clipped to window bounds if straddling)
    float window_onset;   // max(onset_beat, window_start)
    float window_offset;  // min(offset_beat, window_end)
    
    // Flags
    static constexpr uint8_t FLAG_STRADDLING_START = 0x01;  // Note started before window
    static constexpr uint8_t FLAG_STRADDLING_END   = 0x02;  // Note ended after window (active)
    static constexpr uint8_t FLAG_ACTIVE           = 0x04;  // Note is still sounding
    
    bool straddling_start() const { return flags & FLAG_STRADDLING_START; }
    bool straddling_end() const { return flags & FLAG_STRADDLING_END; }
    bool is_active() const { return flags & FLAG_ACTIVE; }
    bool entirely_inside() const { return flags == 0; }
    
    // Derived (computed on demand)
    float duration() const { return offset_beat - onset_beat; }
    float window_duration() const { return window_offset - window_onset; }
    int pitch_class() const { return pitch % 12; }
    int octave() const { return pitch / 12; }
};

static_assert(sizeof(WindowNote) == 24, "WindowNote should be 24 bytes");

// ═══ WAGON READOUT - Raw data, no derived computations ═══════════

struct WagonReadout {
    // --- Window Definition ---
    
    float current_beat = 0.0f;    // Real current time
    float anchor_beat = 0.0f;     // Window end (current_beat - offset)
    float window_start = 0.0f;    // anchor_beat - span
    float window_end = 0.0f;      // Same as anchor_beat
    float span = 0.0f;            // Window size in beats
    float offset = 0.0f;          // How far behind real time
    
    // --- Notes in Window ---
    
    std::array<WindowNote, WAGON_MAX_NOTES> notes{};
    int note_count = 0;
    bool overflow = false;        // True if more notes than MAX_NOTES
    
    // --- Counts ---
    
    int entirely_inside_count = 0;   // Notes fully within window
    int straddling_count = 0;        // Notes crossing boundaries
    int active_count = 0;            // Notes still sounding
    
    // ── Simple State Queries ─────────────────────────────────────
    // No computation, just reading fields.
    
    bool empty() const { return note_count == 0; }
    bool has_notes() const { return note_count > 0; }
    bool has_overflow() const { return overflow; }
    bool has_active() const { return active_count > 0; }
    
    // --- Clear ---
    
    void clear() {
        current_beat = anchor_beat = window_start = window_end = 0.0f;
        span = offset = 0.0f;
        note_count = 0;
        overflow = false;
        entirely_inside_count = straddling_count = active_count = 0;
    }
};

// ═══ WAGON CLASS ═════════════════════════════════════════════════

class Wagon {
public:
    /**
     * Default constructor: span 0, no straddling, no active inclusion.
     * Used when Wagons are owned in a fixed-size container and configured
     * later via set_span() / set_offset().
     */
    Wagon()
        : span_beats_(0.0f)
        , offset_beats_(0.0f)
        , include_straddling_(false)
        , include_active_(false)
    {
        readout_.clear();
    }

    /**
     * Construct a Wagon.
     *
     * @param span_beats         Window size in beats
     * @param offset_beats       How far behind real time (0 = live)
     * @param include_straddling Include notes that cross window boundaries
     * @param include_active     Include currently sounding notes
     */
    Wagon(float span_beats,
          float offset_beats = 0.0f,
          bool include_straddling = false,
          bool include_active = false)
        : span_beats_(span_beats)
        , offset_beats_(offset_beats)
        , include_straddling_(include_straddling)
        , include_active_(include_active)
    {
        readout_.clear();
    }
    
    // --- Update ---
    
    /**
     * Update the Wagon from a snapshot.
     * 
     * @param snap    Current ephemeral state (value, copied)
     * @param history Completed notes (read-only reference to append-only data)
     */
    void update(const StreamSnapshot& snap, const CompletedRing& history) {
        if (update_period_ > 0.0f) {
            float elapsed = snap.beat - last_update_beat_;
            if (elapsed >= 0.0f && elapsed < update_period_) {
                return;
            }
        }
        last_update_beat_ = snap.beat;

        float anchor = snap.beat - offset_beats_;
        float window_start = anchor - span_beats_;
        float window_end = anchor;
        
        rebuild_readout(snap, history, anchor, window_start, window_end);
    }
    
    // --- Access ---
    
    const WagonReadout& readout() const { return readout_; }
    
    float span() const { return span_beats_; }
    void set_span(float beats) { span_beats_ = beats; }
    
    float offset() const { return offset_beats_; }
    void set_offset(float beats) { offset_beats_ = beats; }
    
    bool include_straddling() const { return include_straddling_; }
    void set_include_straddling(bool v) { include_straddling_ = v; }
    
    bool include_active() const { return include_active_; }
    void set_include_active(bool v) { include_active_ = v; }

    void  set_update_period(float beats) { update_period_ = beats; }
    float update_period() const { return update_period_; }
    
private:
    float span_beats_;
    float offset_beats_;
    bool include_straddling_;
    bool include_active_;
    float update_period_    = 0.0f;
    float last_update_beat_ = -1.0e30f;
    
    WagonReadout readout_;
    
    void rebuild_readout(const StreamSnapshot& snap, const CompletedRing& history,
                         float anchor, float ws, float we) {
        readout_.current_beat = snap.beat;
        readout_.anchor_beat = anchor;
        readout_.window_start = ws;
        readout_.window_end = we;
        readout_.span = span_beats_;
        readout_.offset = offset_beats_;
        
        readout_.note_count = 0;
        readout_.overflow = false;
        readout_.entirely_inside_count = 0;
        readout_.straddling_count = 0;
        readout_.active_count = 0;
        
        // Query completed notes from history
        if (include_straddling_) {
            history.for_each_overlapping_window(ws, we, [&](const CompletedNote& cn) {
                add_completed_note(cn, ws, we);
            });
        } else {
            history.for_each_inside_window(ws, we, [&](const CompletedNote& cn) {
                add_completed_note(cn, ws, we);
            });
        }
        
        // Query active notes from snapshot if requested
        if (include_active_) {
            snap.for_each_active([&](int pitch, const ActiveNote& an) {
                // Only include if onset is before window_end
                if (an.onset_beat < we) {
                    add_active_note(pitch, an, ws, we, anchor);
                }
            });
        }
    }
    
    void add_completed_note(const CompletedNote& cn, float ws, float we) {
        if (readout_.note_count >= WAGON_MAX_NOTES) {
            readout_.overflow = true;
            return;
        }
        
        WindowNote& wn = readout_.notes[readout_.note_count];
        wn.pitch = cn.pitch;
        wn.velocity = cn.velocity;
        wn.onset_beat = cn.onset_beat;
        wn.offset_beat = cn.offset_beat;
        
        // Determine flags and clipped bounds
        wn.flags = 0;
        wn.window_onset = cn.onset_beat;
        wn.window_offset = cn.offset_beat;
        
        if (cn.onset_beat < ws) {
            wn.flags |= WindowNote::FLAG_STRADDLING_START;
            wn.window_onset = ws;
            ++readout_.straddling_count;
        }
        if (cn.offset_beat > we) {
            wn.flags |= WindowNote::FLAG_STRADDLING_END;
            wn.window_offset = we;
            if (!(wn.flags & WindowNote::FLAG_STRADDLING_START)) {
                ++readout_.straddling_count;
            }
        }
        
        if (wn.flags == 0) {
            ++readout_.entirely_inside_count;
        }
        
        ++readout_.note_count;
    }
    
    void add_active_note(int pitch, const ActiveNote& an, float ws, float we, float anchor) {
        if (readout_.note_count >= WAGON_MAX_NOTES) {
            readout_.overflow = true;
            return;
        }
        
        WindowNote& wn = readout_.notes[readout_.note_count];
        wn.pitch = static_cast<uint8_t>(pitch);
        wn.velocity = an.velocity;
        wn.onset_beat = an.onset_beat;
        wn.offset_beat = anchor;  // Provisional: extends to now
        
        // Active notes always straddle the end (they're ongoing)
        wn.flags = WindowNote::FLAG_ACTIVE | WindowNote::FLAG_STRADDLING_END;
        wn.window_onset = an.onset_beat;
        wn.window_offset = we;
        
        if (an.onset_beat < ws) {
            wn.flags |= WindowNote::FLAG_STRADDLING_START;
            wn.window_onset = ws;
        }
        
        ++readout_.straddling_count;
        ++readout_.active_count;
        ++readout_.note_count;
    }
};

// ═══ SIZE VERIFICATION ═══════════════════════════════════════════

// WindowNote: 24 bytes

// WagonReadout memory:
//   notes[128]         = 128 * 24 = 3072 bytes
//   scalars + flags    ~ 40 bytes
//   Total              ~ 3.1 KB

// Wagon memory:
//   readout_           ~ 3.1 KB
//   scalars            ~ 16 bytes
//   Total              ~ 3.1 KB

} // namespace t7
