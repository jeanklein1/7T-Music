#pragma once

// ─── stream_data.hpp ─────────────────────────────────────────────
//
// Foundational storage for musical analysis — two categorically
// distinct collections:
//   ActiveSet     — notes currently sounding (onset known, offset
//                   undetermined)
//   CompletedRing — notes finished sounding (onset and offset both
//                   known)
// These are the atoms; analyzers query them, the Stream owns them.
//
// Categorical distinction (must be preserved): an ActiveNote is a live
// process whose duration increases each frame; a CompletedNote is a
// historical fact whose duration is immutable. Functions that need both
// must explicitly request both.
//
// Depends on: <array>, <cstdint>, <algorithm>.

#include <array>
#include <cstdint>
#include <algorithm>

namespace t7 {

// ═══ CONSTANTS ═══════════════════════════════════════════════════

constexpr int MIDI_PITCH_COUNT = 128;
constexpr int COMPLETED_RING_CAPACITY = 2048;

// ═══ BIT OPERATIONS (shared utility) ═════════════════════════════

namespace bits {

inline int popcount64(uint64_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(x);
#else
    x = x - ((x >> 1) & 0x5555555555555555ULL);
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    return static_cast<int>((x * 0x0101010101010101ULL) >> 56);
#endif
}

inline int ctz64(uint64_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return x ? __builtin_ctzll(x) : 64;
#else
    if (x == 0) return 64;
    int n = 0;
    if ((x & 0xFFFFFFFF) == 0) { n += 32; x >>= 32; }
    if ((x & 0xFFFF) == 0) { n += 16; x >>= 16; }
    if ((x & 0xFF) == 0) { n += 8; x >>= 8; }
    if ((x & 0xF) == 0) { n += 4; x >>= 4; }
    if ((x & 0x3) == 0) { n += 2; x >>= 2; }
    if ((x & 0x1) == 0) { n += 1; }
    return n;
#endif
}

} // namespace bits

// ═══ PITCH BITMASK - 128-bit mask for MIDI pitches ═══════════════

struct PitchBitmask {
    uint64_t lo = 0;  // Pitches 0-63
    uint64_t hi = 0;  // Pitches 64-127
    
    void set(int pitch) { 
        if (pitch < 64) lo |= (1ULL << pitch); 
        else hi |= (1ULL << (pitch - 64)); 
    }
    
    void clear(int pitch) { 
        if (pitch < 64) lo &= ~(1ULL << pitch); 
        else hi &= ~(1ULL << (pitch - 64)); 
    }
    
    bool test(int pitch) const { 
        return pitch < 64 ? (lo & (1ULL << pitch)) : (hi & (1ULL << (pitch - 64))); 
    }
    
    void clear_all() { lo = 0; hi = 0; }
    
    int count() const { 
        return bits::popcount64(lo) + bits::popcount64(hi); 
    }
    
    bool empty() const { return lo == 0 && hi == 0; }
    
    int lowest() const {
        if (lo) return bits::ctz64(lo);
        if (hi) return bits::ctz64(hi) + 64;
        return -1;
    }
    
    int highest() const {
        if (hi) { 
            for (int n = 63; n >= 0; --n) 
                if (hi & (1ULL << n)) return n + 64; 
        }
        if (lo) { 
            for (int n = 63; n >= 0; --n) 
                if (lo & (1ULL << n)) return n; 
        }
        return -1;
    }
    
    template<typename Fn>
    void for_each(Fn&& fn) const {
        for (uint64_t b = lo; b; b &= b - 1) fn(bits::ctz64(b));
        for (uint64_t b = hi; b; b &= b - 1) fn(bits::ctz64(b) + 64);
    }
};

// ═══ ACTIVE NOTE - A note currently sounding ═════════════════════

/**
 * An active note has known onset but undetermined offset.
 * Duration is provisional: current_beat - onset_beat.
 * 
 * This is stored in ActiveSet indexed by pitch.
 * velocity = 0 means the slot is empty.
 */
struct ActiveNote {
    float velocity = 0.0f;      // 0 = slot empty, >0 = note active
    float onset_beat = 0.0f;
    
    bool is_active() const { return velocity > 0.0f; }
    
    float duration_at(float current_beat) const {
        return is_active() ? current_beat - onset_beat : 0.0f;
    }
};

// ═══ COMPLETED NOTE - A note that has finished sounding ══════════

/**
 * A completed note has known onset and offset.
 * Duration is fixed and immutable.
 * 
 * This is stored in CompletedRing.
 */
struct CompletedNote {
    float onset_beat;
    float offset_beat;
    float velocity;
    uint8_t pitch;
    uint8_t _pad[3];
    
    float duration() const { return offset_beat - onset_beat; }
    int pitch_class() const { return pitch % 12; }
    int octave() const { return pitch / 12; }
};

static_assert(sizeof(CompletedNote) == 16, "CompletedNote should be 16 bytes");

// ═══ ACTIVE SET - Collection of currently sounding notes ═════════

/**
 * Fixed array of 128 slots indexed by MIDI pitch.
 * Bitmask tracks which slots are occupied.
 * 
 * Guarantees:
 * - O(1) lookup by pitch
 * - O(active_count) iteration via bitmask
 * - Zero allocation
 */
struct ActiveSet {
    std::array<ActiveNote, MIDI_PITCH_COUNT> notes{};
    PitchBitmask mask;
    
    // --- Modification ---
    
    void note_on(int pitch, float velocity, float beat) {
        if (pitch < 0 || pitch >= MIDI_PITCH_COUNT) return;
        notes[pitch].velocity = velocity;
        notes[pitch].onset_beat = beat;
        mask.set(pitch);
    }
    
    /**
     * Deactivate a note. Returns the note that was active (for transfer to CompletedRing).
     * Returns an empty ActiveNote if pitch was not active.
     */
    ActiveNote note_off(int pitch) {
        if (pitch < 0 || pitch >= MIDI_PITCH_COUNT) return ActiveNote{};
        ActiveNote result = notes[pitch];
        notes[pitch].velocity = 0.0f;
        mask.clear(pitch);
        return result;
    }
    
    void clear() {
        for (auto& n : notes) n.velocity = 0.0f;
        mask.clear_all();
    }
    
    // --- Queries ---
    
    bool is_active(int pitch) const {
        return pitch >= 0 && pitch < MIDI_PITCH_COUNT && notes[pitch].is_active();
    }
    
    int count() const { return mask.count(); }
    bool empty() const { return mask.empty(); }
    
    float velocity(int pitch) const {
        return (pitch >= 0 && pitch < MIDI_PITCH_COUNT) ? notes[pitch].velocity : 0.0f;
    }
    
    float onset(int pitch) const {
        return (pitch >= 0 && pitch < MIDI_PITCH_COUNT) ? notes[pitch].onset_beat : 0.0f;
    }
    
    int lowest_pitch() const { return mask.lowest(); }
    int highest_pitch() const { return mask.highest(); }
    
    /**
     * Iterate all active notes.
     * Callback signature: void(int pitch, const ActiveNote& note)
     */
    template<typename Fn>
    void for_each(Fn&& fn) const {
        mask.for_each([&](int pitch) {
            fn(pitch, notes[pitch]);
        });
    }
};

// ═══ COMPLETED RING - Collection of finished notes ═══════════════

/**
 * Ring buffer of completed notes, ordered by offset_beat (completion time).
 * Newest notes at head, oldest pruned when beyond retention.
 * 
 * Guarantees:
 * - O(1) push
 * - O(n) window query (could optimize with index if needed)
 * - Fixed memory footprint
 * - Zero allocation after construction
 */
struct CompletedRing {
    std::array<CompletedNote, COMPLETED_RING_CAPACITY> notes{};
    int head = 0;   // Next write position
    int count = 0;  // Number of valid entries
    
    // --- Modification ---
    
    void push(const CompletedNote& note) {
        notes[head] = note;
        head = (head + 1) % COMPLETED_RING_CAPACITY;
        if (count < COMPLETED_RING_CAPACITY) ++count;
    }
    
    void push(int pitch, float velocity, float onset_beat, float offset_beat) {
        CompletedNote note;
        note.pitch = static_cast<uint8_t>(pitch);
        note.velocity = velocity;
        note.onset_beat = onset_beat;
        note.offset_beat = offset_beat;
        push(note);
    }
    
    void clear() {
        head = 0;
        count = 0;
    }
    
    /**
     * Remove notes older than cutoff_beat.
     * Call during update() to maintain retention window.
     */
    void prune_before(float cutoff_beat) {
        // Find how many old notes to remove from tail
        int tail = (head - count + COMPLETED_RING_CAPACITY) % COMPLETED_RING_CAPACITY;
        while (count > 0) {
            if (notes[tail].offset_beat >= cutoff_beat) break;
            tail = (tail + 1) % COMPLETED_RING_CAPACITY;
            --count;
        }
    }
    
    // --- Queries ---
    
    bool empty() const { return count == 0; }
    int size() const { return count; }
    
    /**
     * Get the most recently completed note.
     * Returns false if ring is empty.
     */
    bool most_recent(CompletedNote& out) const {
        if (count == 0) return false;
        int idx = (head - 1 + COMPLETED_RING_CAPACITY) % COMPLETED_RING_CAPACITY;
        out = notes[idx];
        return true;
    }
    
    /**
     * Iterate all notes in the ring (oldest to newest).
     * Callback signature: void(const CompletedNote& note)
     */
    template<typename Fn>
    void for_each(Fn&& fn) const {
        int tail = (head - count + COMPLETED_RING_CAPACITY) % COMPLETED_RING_CAPACITY;
        for (int i = 0; i < count; ++i) {
            int idx = (tail + i) % COMPLETED_RING_CAPACITY;
            fn(notes[idx]);
        }
    }
    
    /**
     * Iterate notes in reverse order (newest to oldest).
     * Callback signature: void(const CompletedNote& note)
     */
    template<typename Fn>
    void for_each_reverse(Fn&& fn) const {
        for (int i = 0; i < count; ++i) {
            int idx = (head - 1 - i + COMPLETED_RING_CAPACITY) % COMPLETED_RING_CAPACITY;
            fn(notes[idx]);
        }
    }
    
    /**
     * Iterate notes entirely inside a time window [start, end].
     * A note is inside if: onset >= start AND offset <= end
     * 
     * This is the default, strict query. Notes straddling boundaries are excluded.
     * Callback signature: void(const CompletedNote& note)
     */
    template<typename Fn>
    void for_each_inside_window(float start, float end, Fn&& fn) const {
        int tail = (head - count + COMPLETED_RING_CAPACITY) % COMPLETED_RING_CAPACITY;
        for (int i = 0; i < count; ++i) {
            int idx = (tail + i) % COMPLETED_RING_CAPACITY;
            const CompletedNote& note = notes[idx];
            if (note.onset_beat >= start && note.offset_beat <= end) {
                fn(note);
            }
        }
    }
    
    /**
     * Iterate notes overlapping a time window [start, end].
     * A note overlaps if: onset < end AND offset > start
     * 
     * Use when you want to include notes that straddle window boundaries.
     * Callback signature: void(const CompletedNote& note)
     */
    template<typename Fn>
    void for_each_overlapping_window(float start, float end, Fn&& fn) const {
        int tail = (head - count + COMPLETED_RING_CAPACITY) % COMPLETED_RING_CAPACITY;
        for (int i = 0; i < count; ++i) {
            int idx = (tail + i) % COMPLETED_RING_CAPACITY;
            const CompletedNote& note = notes[idx];
            if (note.onset_beat < end && note.offset_beat > start) {
                fn(note);
            }
        }
    }
    
    /**
     * Count notes entirely inside a time window.
     */
    int count_inside_window(float start, float end) const {
        int result = 0;
        for_each_inside_window(start, end, [&](const CompletedNote&) { ++result; });
        return result;
    }
    
    /**
     * Count notes overlapping a time window.
     */
    int count_overlapping_window(float start, float end) const {
        int result = 0;
        for_each_overlapping_window(start, end, [&](const CompletedNote&) { ++result; });
        return result;
    }
    
    /**
     * Iterate the N most recently completed notes (newest first).
     * Callback signature: void(const CompletedNote& note)
     */
    template<typename Fn>
    void for_each_recent(int max_count, Fn&& fn) const {
        int n = std::min(max_count, count);
        for (int i = 0; i < n; ++i) {
            int idx = (head - 1 - i + COMPLETED_RING_CAPACITY) % COMPLETED_RING_CAPACITY;
            fn(notes[idx]);
        }
    }
    
    /**
     * Get notes that completed within a tolerance of the most recent.
     * This captures "simultaneous" note-offs (chords releasing together).
     * 
     * @param tolerance_beats  Maximum time difference to consider simultaneous
     * @param out              Output array (caller provides)
     * @param max_out          Maximum notes to return
     * @return                 Number of notes written to out
     */
    int most_recent_set(float tolerance_beats, CompletedNote* out, int max_out) const {
        if (count == 0 || max_out <= 0) return 0;
        
        int most_recent_idx = (head - 1 + COMPLETED_RING_CAPACITY) % COMPLETED_RING_CAPACITY;
        float reference_beat = notes[most_recent_idx].offset_beat;
        float cutoff = reference_beat - tolerance_beats;
        
        int written = 0;
        for (int i = 0; i < count && written < max_out; ++i) {
            int idx = (head - 1 - i + COMPLETED_RING_CAPACITY) % COMPLETED_RING_CAPACITY;
            if (notes[idx].offset_beat >= cutoff) {
                out[written++] = notes[idx];
            } else {
                break;  // Notes are ordered by offset, so we can stop
            }
        }
        return written;
    }
    
    /**
     * Get the completed note with offset_beat closest to (but before) the given beat.
     * For finding "previous note" relative to a point in time.
     * 
     * Since notes are ordered by offset_beat (completion time), we iterate
     * from newest to oldest and return the first note that ended before the given beat.
     */
    bool previous_before(float beat, CompletedNote& out) const {
        for (int i = 0; i < count; ++i) {
            int idx = (head - 1 - i + COMPLETED_RING_CAPACITY) % COMPLETED_RING_CAPACITY;
            const CompletedNote& note = notes[idx];
            if (note.offset_beat < beat) {
                out = note;
                return true;
            }
        }
        return false;
    }
};

// ═══ STREAM SNAPSHOT - Ephemeral state at one instant ════════════

/**
 * A snapshot captures the ephemeral state (ActiveSet) at a specific beat.
 * This is a value type - safe to copy, store, pass across thread boundaries.
 * 
 * The snapshot decouples observation from the live stream, enabling:
 * - Parallel analysis (no shared mutable state)
 * - Historical comparison (store snapshots across frames)
 * - Clean data flow (values in, values out)
 */
struct StreamSnapshot {
    float beat = 0.0f;
    int channel = 0;
    ActiveSet active;  // Full copy (~1KB)
    
    // Convenience accessors
    bool silent() const { return active.empty(); }
    int count() const { return active.count(); }
    const PitchBitmask& mask() const { return active.mask; }
    int lowest_pitch() const { return active.lowest_pitch(); }
    int highest_pitch() const { return active.highest_pitch(); }
    
    template<typename Fn>
    void for_each_active(Fn&& fn) const {
        active.for_each(std::forward<Fn>(fn));
    }
};

// ═══ SIZE VERIFICATION ═══════════════════════════════════════════

// ActiveSet memory:
//   notes[128] = 128 * 8 = 1024 bytes
//   mask       = 16 bytes
//   Total      ~ 1 KB

// CompletedRing memory:
//   notes[2048] = 2048 * 16 = 32768 bytes
//   head, count = 8 bytes
//   Total       ~ 32 KB

// StreamSnapshot memory:
//   beat, channel = 8 bytes
//   active        ~ 1 KB
//   Total         ~ 1 KB

} // namespace t7
