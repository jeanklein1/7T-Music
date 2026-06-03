#pragma once

// ─── musical_ops.hpp ─────────────────────────────────────────────
//
// Musical operations: typed representations of musical data, extractors
// that build them from Playhead/Wagon readouts, and operations that
// reduce them to scalars and comparisons.
//
//   Readout → Extractor → Representation → Operation → Result
//
// Representations : PitchClassBits (12-bit mask), PitchClassVector
//                   (12-float weighted distribution), PitchClassSet,
//                   RhythmBin (quantized duration category).
// Extractors      : extract_*(PlayheadReadout) / extract_*(WagonReadout).
// Operations      : relational (between two playhead events), aggregate
//                   (over the notes in a wagon window), and pure ops on
//                   the representations themselves.
//
// Depends on: musical/playhead.hpp (PlayheadReadout), musical/wagon.hpp
//             (WagonReadout, NoteView), <array>, <cmath>, <cstdint>.

#include "musical/playhead.hpp"
#include "musical/wagon.hpp"
#include <array>
#include <cmath>
#include <cstdint>

namespace t7 {

// ═══ REPRESENTATIONS ═════════════════════════════════════════════

// ── Pitch Class Helpers ──────────────────────────────────────────

/**
 * Pitch class of a MIDI note in the default C-origin convention (C = 0).
 * Input: MIDI pitch (0-127). Output: pitch class 0-11.
 */
inline int pc_of(int midi_pitch) {
    return ((midi_pitch % 12) + 12) % 12;
}

/**
 * Re-express a pitch class with `origin` as zero (origin transpose).
 * `origin` is the C-origin index of the desired zero (e.g. 2 = D).
 * Input: pc 0-11, origin 0-11. Output: transposed pc 0-11.
 */
inline int pc_relative_to(int pc, int origin) {
    return ((pc - origin) % 12 + 12) % 12;
}

// ── Pitch Class Bits ─────────────────────────────────────────────

/**
 * 12-bit mask representing pitch classes present.
 * Bit N = pitch class N is present (C=0, C#=1, ... B=11)
 *
 * Use for: set operations, scale adherence, chord identification
 */
struct PitchClassBits {
    uint16_t bits = 0;

    void set(int pc) { bits |= (1 << (pc % 12)); }
    void clear(int pc) { bits &= ~(1 << (pc % 12)); }
    bool test(int pc) const { return (bits & (1 << (pc % 12))) != 0; }
    void clear_all() { bits = 0; }

    int count() const {
        uint16_t b = bits;
        b = b - ((b >> 1) & 0x5555);
        b = (b & 0x3333) + ((b >> 2) & 0x3333);
        b = (b + (b >> 4)) & 0x0F0F;
        return (b + (b >> 8)) & 0x1F;
    }

    bool empty() const { return bits == 0; }

    // Set operations
    PitchClassBits operator&(PitchClassBits other) const { return {uint16_t(bits & other.bits)}; }
    PitchClassBits operator|(PitchClassBits other) const { return {uint16_t(bits | other.bits)}; }
    PitchClassBits operator^(PitchClassBits other) const { return {uint16_t(bits ^ other.bits)}; }
    PitchClassBits operator~() const { return {uint16_t(~bits & 0x0FFF)}; }

    bool operator==(PitchClassBits other) const { return bits == other.bits; }
    bool operator!=(PitchClassBits other) const { return bits != other.bits; }
};

// ── Pitch Class Vector ───────────────────────────────────────────

/**
 * 12-float vector representing weighted pitch class distribution.
 * Index N = weight of pitch class N.
 *
 * Weights can be: velocity, duration, duration*velocity, or binary (0/1).
 *
 * Use for: similarity measures, centroid, spread, visualization
 */
struct PitchClassVector {
    std::array<float, 12> v = {};

    float& operator[](int pc) { return v[pc % 12]; }
    float operator[](int pc) const { return v[pc % 12]; }

    void clear() { v.fill(0.0f); }

    float sum() const {
        float s = 0.0f;
        for (float x : v) s += x;
        return s;
    }

    float max() const {
        float m = 0.0f;
        for (float x : v) if (x > m) m = x;
        return m;
    }

    /**
     * Normalize to unit sum.
     */
    PitchClassVector normalized() const {
        float s = sum();
        if (s <= 0.0f) return *this;
        PitchClassVector result;
        for (int i = 0; i < 12; ++i) result.v[i] = v[i] / s;
        return result;
    }

    /**
     * Normalize to unit length (L2 norm).
     */
    PitchClassVector unit() const {
        float sq = 0.0f;
        for (float x : v) sq += x * x;
        if (sq <= 0.0f) return *this;
        float norm = std::sqrt(sq);
        PitchClassVector result;
        for (int i = 0; i < 12; ++i) result.v[i] = v[i] / norm;
        return result;
    }
};

// ── Pitch Class Set ──────────────────────────────────────────────

/**
 * An unordered set of pitch classes (no root, no degree structure).
 *
 * Use for: set overlap, membership tests, chord/key membership
 */
struct PitchClassSet {
    PitchClassBits pitch_classes;

    bool contains(int pc) const { return pitch_classes.test(pc); }
    int note_count() const { return pitch_classes.count(); }
};

// ── Rhythm Bin ───────────────────────────────────────────────────

/**
 * Quantized duration category.
 *
 * Use for: rhythm analysis, duration histogram
 */
enum class RhythmBin : uint8_t {
    VeryLong,       // >= 4 beats
    Whole,          // 3-4 beats
    Half,           // 1.5-3 beats
    Quarter,        // 0.75-1.5 beats
    Eighth,         // 0.375-0.75 beats
    Sixteenth,      // 0.1875-0.375 beats
    VeryShort       // < 0.1875 beats
};

inline RhythmBin quantize_rhythm(float duration_beats) {
    if (duration_beats >= 4.0f) return RhythmBin::VeryLong;
    if (duration_beats >= 3.0f) return RhythmBin::Whole;
    if (duration_beats >= 1.5f) return RhythmBin::Half;
    if (duration_beats >= 0.75f) return RhythmBin::Quarter;
    if (duration_beats >= 0.375f) return RhythmBin::Eighth;
    if (duration_beats >= 0.1875f) return RhythmBin::Sixteenth;
    return RhythmBin::VeryShort;
}

// ═══ EXTRACTORS ══════════════════════════════════════════════════

// ── From PlayheadReadout ─────────────────────────────────────────

/**
 * Extract pitch class bits from CURRENT notes.
 */
inline PitchClassBits extract_pitch_class_bits_current(const PlayheadReadout& r) {
    PitchClassBits pcb;
    for (int i = 0; i < r.current_count; ++i) {
        pcb.set(r.current[i].pitch % 12);
    }
    return pcb;
}

/**
 * Extract pitch class bits from PREVIOUS notes.
 */
inline PitchClassBits extract_pitch_class_bits_previous(const PlayheadReadout& r) {
    PitchClassBits pcb;
    for (int i = 0; i < r.previous_count; ++i) {
        pcb.set(r.previous[i].pitch % 12);
    }
    return pcb;
}

/**
 * Extract pitch class vector from CURRENT (velocity-weighted).
 */
inline PitchClassVector extract_pitch_class_vector_current(const PlayheadReadout& r) {
    PitchClassVector pcv;
    for (int i = 0; i < r.current_count; ++i) {
        pcv[r.current[i].pitch % 12] += r.current[i].velocity;
    }
    return pcv;
}

/**
 * Extract pitch class vector from PREVIOUS (velocity-weighted).
 */
inline PitchClassVector extract_pitch_class_vector_previous(const PlayheadReadout& r) {
    PitchClassVector pcv;
    for (int i = 0; i < r.previous_count; ++i) {
        pcv[r.previous[i].pitch % 12] += r.previous[i].velocity;
    }
    return pcv;
}

// ── From WagonReadout ────────────────────────────────────────────

/**
 * Extract pitch class bits from wagon notes.
 */
inline PitchClassBits extract_pitch_class_bits(const WagonReadout& r) {
    PitchClassBits pcb;
    for (int i = 0; i < r.note_count; ++i) {
        pcb.set(r.notes[i].pitch % 12);
    }
    return pcb;
}

/**
 * Extract pitch class vector (duration-weighted).
 */
inline PitchClassVector extract_pitch_class_vector_duration(const WagonReadout& r) {
    PitchClassVector pcv;
    for (int i = 0; i < r.note_count; ++i) {
        pcv[r.notes[i].pitch % 12] += r.notes[i].window_duration();
    }
    return pcv;
}

/**
 * Extract pitch class vector (velocity-weighted).
 */
inline PitchClassVector extract_pitch_class_vector_velocity(const WagonReadout& r) {
    PitchClassVector pcv;
    for (int i = 0; i < r.note_count; ++i) {
        pcv[r.notes[i].pitch % 12] += r.notes[i].velocity;
    }
    return pcv;
}

/**
 * Extract pitch class vector (duration * velocity weighted).
 */
inline PitchClassVector extract_pitch_class_vector_weighted(const WagonReadout& r) {
    PitchClassVector pcv;
    for (int i = 0; i < r.note_count; ++i) {
        pcv[r.notes[i].pitch % 12] += r.notes[i].window_duration() * r.notes[i].velocity;
    }
    return pcv;
}

// ═══ OPERATIONS ON REPRESENTATIONS ═══════════════════════════════

// ── PitchClassBits Operations ────────────────────────────────────

/**
 * Fraction of pitch classes in set a that are also in set b.
 * Input: two PitchClassBits
 * Output: 0.0 to 1.0 (1.0 when a is empty — vacuously true)
 */
inline float pc_set_overlap_fraction(PitchClassBits a, PitchClassBits b) {
    if (a.empty()) return 1.0f;
    return static_cast<float>((a & b).count()) / a.count();
}

/**
 * Number of common pitch classes.
 * Input: two PitchClassBits
 * Output: 0 to 12
 */
inline int pitch_class_intersection(PitchClassBits a, PitchClassBits b) {
    return (a & b).count();
}

/**
 * Jaccard similarity between two pitch class sets.
 * Input: two PitchClassBits
 * Output: 0.0 to 1.0
 */
inline float pitch_class_jaccard(PitchClassBits a, PitchClassBits b) {
    int intersection = (a & b).count();
    int union_count = (a | b).count();
    if (union_count == 0) return 1.0f;
    return static_cast<float>(intersection) / union_count;
}

// ── PitchClassVector Operations ──────────────────────────────────

/**
 * Dot product of two pitch class vectors.
 * Input: two PitchClassVector
 */
inline float pcv_dot(const PitchClassVector& a, const PitchClassVector& b) {
    float sum = 0.0f;
    for (int i = 0; i < 12; ++i) sum += a.v[i] * b.v[i];
    return sum;
}

/**
 * Cosine similarity between two pitch class vectors.
 * Input: two PitchClassVector
 * Output: -1.0 to 1.0 (usually 0.0 to 1.0 for non-negative vectors)
 */
inline float pcv_cosine_similarity(const PitchClassVector& a, const PitchClassVector& b) {
    float dot = pcv_dot(a, b);
    float norm_a = std::sqrt(pcv_dot(a, a));
    float norm_b = std::sqrt(pcv_dot(b, b));
    if (norm_a <= 0.0f || norm_b <= 0.0f) return 0.0f;
    return dot / (norm_a * norm_b);
}

/**
 * Euclidean distance between two pitch class vectors.
 * Input: two PitchClassVector
 */
inline float pcv_distance(const PitchClassVector& a, const PitchClassVector& b) {
    float sum = 0.0f;
    for (int i = 0; i < 12; ++i) {
        float diff = a.v[i] - b.v[i];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

/**
 * Centroid of pitch class vector (circular mean on pitch class circle).
 * Input: PitchClassVector
 * Output: 0.0 to 12.0 (fractional pitch class)
 */
inline float pcv_centroid(const PitchClassVector& pcv) {
    // Use circular mean to handle wraparound (B close to C)
    float sin_sum = 0.0f;
    float cos_sum = 0.0f;
    float total_weight = 0.0f;

    constexpr float TWO_PI_12 = 2.0f * 3.14159265f / 12.0f;

    for (int i = 0; i < 12; ++i) {
        float angle = i * TWO_PI_12;
        sin_sum += pcv.v[i] * std::sin(angle);
        cos_sum += pcv.v[i] * std::cos(angle);
        total_weight += pcv.v[i];
    }

    if (total_weight <= 0.0f) return 0.0f;

    float mean_angle = std::atan2(sin_sum, cos_sum);
    if (mean_angle < 0.0f) mean_angle += 2.0f * 3.14159265f;

    return mean_angle / TWO_PI_12;
}

// ═══ PLAYHEAD OPERATIONS (RELATIONAL) ════════════════════════════

/**
 * Velocity-weighted centroid of CURRENT pitches.
 * Input: PlayheadReadout
 * Output: 0.0 to 127.0 (MIDI pitch)
 */
inline float playhead_centroid_current(const PlayheadReadout& r) {
    if (r.current_count == 0) return 0.0f;
    float weighted_sum = 0.0f;
    float total_weight = 0.0f;
    for (int i = 0; i < r.current_count; ++i) {
        weighted_sum += r.current[i].pitch * r.current[i].velocity;
        total_weight += r.current[i].velocity;
    }
    return total_weight > 0.0f ? weighted_sum / total_weight : 0.0f;
}

/**
 * Velocity-weighted centroid of PREVIOUS pitches.
 * Input: PlayheadReadout
 * Output: 0.0 to 127.0 (MIDI pitch)
 */
inline float playhead_centroid_previous(const PlayheadReadout& r) {
    if (r.previous_count == 0) return 0.0f;
    float weighted_sum = 0.0f;
    float total_weight = 0.0f;
    for (int i = 0; i < r.previous_count; ++i) {
        weighted_sum += r.previous[i].pitch * r.previous[i].velocity;
        total_weight += r.previous[i].velocity;
    }
    return total_weight > 0.0f ? weighted_sum / total_weight : 0.0f;
}

/**
 * Interval between PREVIOUS and CURRENT centroids.
 * Input: PlayheadReadout
 * Output: semitones (can be negative)
 */
inline float playhead_interval(const PlayheadReadout& r) {
    if (r.current_count == 0 || r.previous_count == 0) return 0.0f;
    return playhead_centroid_current(r) - playhead_centroid_previous(r);
}

/**
 * Absolute interval.
 * Input: PlayheadReadout
 * Output: semitones (always positive)
 */
inline float playhead_interval_abs(const PlayheadReadout& r) {
    return std::abs(playhead_interval(r));
}

/**
 * Signed centroid change since last frame.
 * Input: PlayheadReadout
 * Output: semitones (positive = ascending, negative = descending, 0 = no notes)
 */
inline float playhead_centroid_change(const PlayheadReadout& r) {
    if (r.current_count == 0 || r.previous_count == 0) return 0.0f;
    return playhead_centroid_current(r) - playhead_centroid_previous(r);
}

/**
 * Lowest pitch in CURRENT.
 * Input: PlayheadReadout
 * Output: MIDI pitch (0-127) or -1 if empty
 */
inline int playhead_lowest_pitch_current(const PlayheadReadout& r) {
    return r.current_mask.lowest();
}

/**
 * Lowest pitch in PREVIOUS.
 * Input: PlayheadReadout
 * Output: MIDI pitch (0-127) or -1 if empty
 */
inline int playhead_lowest_pitch_previous(const PlayheadReadout& r) {
    if (r.previous_count == 0) return -1;
    int lowest = 127;
    for (int i = 0; i < r.previous_count; ++i) {
        if (r.previous[i].pitch < lowest) lowest = r.previous[i].pitch;
    }
    return lowest;
}

/**
 * Interval between lowest pitches (current minus previous).
 * Input: PlayheadReadout
 * Output: semitones (can be negative)
 */
inline int playhead_lowest_pitch_interval(const PlayheadReadout& r) {
    int curr = playhead_lowest_pitch_current(r);
    int prev = playhead_lowest_pitch_previous(r);
    if (curr < 0 || prev < 0) return 0;
    return curr - prev;
}

/**
 * Highest pitch in CURRENT.
 * Input: PlayheadReadout
 * Output: MIDI pitch (0-127) or -1 if empty
 */
inline int playhead_highest_pitch_current(const PlayheadReadout& r) {
    return r.current_mask.highest();
}

/**
 * Highest pitch in PREVIOUS.
 * Input: PlayheadReadout
 * Output: MIDI pitch (0-127) or -1 if empty
 */
inline int playhead_highest_pitch_previous(const PlayheadReadout& r) {
    if (r.previous_count == 0) return -1;
    int highest = 0;
    for (int i = 0; i < r.previous_count; ++i) {
        if (r.previous[i].pitch > highest) highest = r.previous[i].pitch;
    }
    return highest;
}

/**
 * Interval between highest pitches (current minus previous).
 * Input: PlayheadReadout
 * Output: semitones (can be negative)
 */
inline int playhead_highest_pitch_interval(const PlayheadReadout& r) {
    int curr = playhead_highest_pitch_current(r);
    int prev = playhead_highest_pitch_previous(r);
    if (curr < 0 || prev < 0) return 0;
    return curr - prev;
}

/**
 * Mean velocity of CURRENT.
 * Input: PlayheadReadout
 * Output: 0.0 to 1.0
 */
inline float playhead_velocity_current(const PlayheadReadout& r) {
    if (r.current_count == 0) return 0.0f;
    float sum = 0.0f;
    for (int i = 0; i < r.current_count; ++i) {
        sum += r.current[i].velocity;
    }
    return sum / r.current_count;
}

/**
 * Mean velocity of PREVIOUS.
 * Input: PlayheadReadout
 * Output: 0.0 to 1.0
 */
inline float playhead_velocity_previous(const PlayheadReadout& r) {
    if (r.previous_count == 0) return 0.0f;
    float sum = 0.0f;
    for (int i = 0; i < r.previous_count; ++i) {
        sum += r.previous[i].velocity;
    }
    return sum / r.previous_count;
}

/**
 * Velocity change between PREVIOUS and CURRENT.
 * Input: PlayheadReadout
 * Output: can be negative (quieter) or positive (louder)
 */
inline float playhead_velocity_delta(const PlayheadReadout& r) {
    return playhead_velocity_current(r) - playhead_velocity_previous(r);
}

/**
 * Pitch span of CURRENT (highest - lowest).
 * Input: PlayheadReadout
 * Output: semitones
 */
inline int playhead_span_current(const PlayheadReadout& r) {
    if (r.current_count < 2) return 0;
    return playhead_highest_pitch_current(r) - playhead_lowest_pitch_current(r);
}

/**
 * Voice-leading minimum: pitch in PREVIOUS closest to target_pitch.
 * On equal distance, the higher pitch wins (soprano-voice tiebreak).
 * Input: PlayheadReadout, target MIDI pitch
 * Output: MIDI pitch (0-127) or -1 if empty
 */
inline int playhead_closest_to_previous(const PlayheadReadout& r, int target_pitch) {
    if (r.previous_count == 0) return -1;
    int best = r.previous[0].pitch;
    int best_dist = std::abs(int(r.previous[0].pitch) - target_pitch);
    for (int i = 1; i < r.previous_count; ++i) {
        int p = r.previous[i].pitch;
        int d = std::abs(p - target_pitch);
        if (d < best_dist || (d == best_dist && p > best)) {
            best = p;
            best_dist = d;
        }
    }
    return best;
}

/**
 * Last-to-release: pitch with the maximum offset_beat in PREVIOUS.
 * On equal offset_beat, the higher pitch wins.
 * Input: PlayheadReadout
 * Output: MIDI pitch (0-127) or -1 if empty
 */
inline int playhead_latest_previous(const PlayheadReadout& r) {
    if (r.previous_count == 0) return -1;
    int best = r.previous[0].pitch;
    float best_off = r.previous[0].offset_beat;
    for (int i = 1; i < r.previous_count; ++i) {
        int p = r.previous[i].pitch;
        float off = r.previous[i].offset_beat;
        if (off > best_off || (off == best_off && p > best)) {
            best = p;
            best_off = off;
        }
    }
    return best;
}

/**
 * First-to-start: pitch with the minimum onset_beat in PREVIOUS.
 * On equal onset_beat, the higher pitch wins.
 * Input: PlayheadReadout
 * Output: MIDI pitch (0-127) or -1 if empty
 */
inline int playhead_earliest_previous(const PlayheadReadout& r) {
    if (r.previous_count == 0) return -1;
    int best = r.previous[0].pitch;
    float best_on = r.previous[0].onset_beat;
    for (int i = 1; i < r.previous_count; ++i) {
        int p = r.previous[i].pitch;
        float on = r.previous[i].onset_beat;
        if (on < best_on || (on == best_on && p > best)) {
            best = p;
            best_on = on;
        }
    }
    return best;
}

/**
 * Loudest: pitch with the maximum velocity in PREVIOUS.
 * On equal velocity, the higher pitch wins.
 * Input: PlayheadReadout
 * Output: MIDI pitch (0-127) or -1 if empty
 */
inline int playhead_loudest_previous(const PlayheadReadout& r) {
    if (r.previous_count == 0) return -1;
    int best = r.previous[0].pitch;
    float best_vel = r.previous[0].velocity;
    for (int i = 1; i < r.previous_count; ++i) {
        int p = r.previous[i].pitch;
        float vel = r.previous[i].velocity;
        if (vel > best_vel || (vel == best_vel && p > best)) {
            best = p;
            best_vel = vel;
        }
    }
    return best;
}

/**
 * Longest: pitch with the maximum duration (offset_beat - onset_beat) in PREVIOUS.
 * On equal duration, the higher pitch wins.
 * Input: PlayheadReadout
 * Output: MIDI pitch (0-127) or -1 if empty
 */
inline int playhead_longest_previous(const PlayheadReadout& r) {
    if (r.previous_count == 0) return -1;
    int best = r.previous[0].pitch;
    float best_dur = r.previous[0].duration();
    for (int i = 1; i < r.previous_count; ++i) {
        int p = r.previous[i].pitch;
        float dur = r.previous[i].duration();
        if (dur > best_dur || (dur == best_dur && p > best)) {
            best = p;
            best_dur = dur;
        }
    }
    return best;
}

/**
 * Count of currently sounding notes.
 * Input: PlayheadReadout
 * Output: note count (0 if silent)
 */
inline int playhead_polyphony(const PlayheadReadout& r) {
    return r.current_count;
}

/**
 * Pitch class (0-11) of the lowest currently sounding note.
 * Input: PlayheadReadout
 * Output: pitch class 0-11, or -1 if silent
 */
inline int playhead_lowest_pc_current(const PlayheadReadout& r, int origin = 0) {
    int pitch = playhead_lowest_pitch_current(r);
    return (pitch >= 0) ? pc_relative_to(pc_of(pitch), origin) : -1;
}

/**
 * One-hot predicate: 1.0 if the lowest current note's pitch class equals pc.
 * Input: PlayheadReadout, target pitch class (0-11)
 * Output: 1.0f if match, 0.0f otherwise (also 0.0f if silent)
 */
inline float playhead_lowest_pc_one_hot_current(const PlayheadReadout& r, int pc, int origin = 0) {
    return (playhead_lowest_pc_current(r, origin) == pc) ? 1.0f : 0.0f;
}

// ═══ WAGON OPERATIONS (AGGREGATE) ════════════════════════════════

/**
 * Velocity-weighted centroid of all notes.
 * Input: WagonReadout
 * Output: 0.0 to 127.0 (MIDI pitch)
 */
inline float wagon_centroid(const WagonReadout& r) {
    if (r.note_count == 0) return 0.0f;
    float weighted_sum = 0.0f;
    float total_weight = 0.0f;
    for (int i = 0; i < r.note_count; ++i) {
        float weight = r.notes[i].velocity * r.notes[i].window_duration();
        weighted_sum += r.notes[i].pitch * weight;
        total_weight += weight;
    }
    return total_weight > 0.0f ? weighted_sum / total_weight : 0.0f;
}

/**
 * Normalized centroid.
 * Input: WagonReadout
 * Output: 0.0 to 1.0
 */
inline float wagon_centroid_normalized(const WagonReadout& r) {
    return wagon_centroid(r) / 127.0f;
}

/**
 * Pitch spread (standard deviation from centroid).
 * Input: WagonReadout
 * Output: semitones
 */
inline float wagon_spread(const WagonReadout& r) {
    if (r.note_count < 2) return 0.0f;

    float centroid = wagon_centroid(r);
    float variance_sum = 0.0f;
    float total_weight = 0.0f;

    for (int i = 0; i < r.note_count; ++i) {
        float weight = r.notes[i].velocity * r.notes[i].window_duration();
        float diff = r.notes[i].pitch - centroid;
        variance_sum += weight * diff * diff;
        total_weight += weight;
    }

    return total_weight > 0.0f ? std::sqrt(variance_sum / total_weight) : 0.0f;
}

/**
 * Normalized spread.
 * Input: WagonReadout
 * Output: 0.0 to 1.0 (clamped, 24 semitones = 1.0)
 */
inline float wagon_spread_normalized(const WagonReadout& r) {
    return std::min(1.0f, wagon_spread(r) / 24.0f);
}

/**
 * Pitch range (highest - lowest).
 * Input: WagonReadout
 * Output: semitones
 */
inline int wagon_range(const WagonReadout& r) {
    if (r.note_count == 0) return 0;
    int lowest = 127, highest = 0;
    for (int i = 0; i < r.note_count; ++i) {
        if (r.notes[i].pitch < lowest) lowest = r.notes[i].pitch;
        if (r.notes[i].pitch > highest) highest = r.notes[i].pitch;
    }
    return highest - lowest;
}

/**
 * Total sounding time (sum of all note durations within window).
 * Input: WagonReadout
 * Output: beats
 */
inline float wagon_total_sounding_time(const WagonReadout& r) {
    float sum = 0.0f;
    for (int i = 0; i < r.note_count; ++i) {
        sum += r.notes[i].window_duration();
    }
    return sum;
}

/**
 * Coverage: fraction of window with at least one note sounding.
 * Input: WagonReadout
 * Output: 0.0 to 1.0
 *
 * Note: This is an approximation. Exact coverage requires interval merging.
 */
inline float wagon_coverage(const WagonReadout& r) {
    if (r.note_count == 0 || r.span <= 0.0f) return 0.0f;

    // Simple approximation: mean polyphony * total_time / span
    // More accurate would be interval merging, but this is fast
    float total = wagon_total_sounding_time(r);

    // Clamp to 1.0 (multiple overlapping notes can exceed window)
    return std::min(1.0f, total / r.span);
}

/**
 * Onset density: number of note onsets per beat.
 * Input: WagonReadout
 * Output: onsets per beat
 */
inline float wagon_onset_density(const WagonReadout& r) {
    if (r.span <= 0.0f) return 0.0f;

    // Count notes that started inside the window
    int onset_count = 0;
    for (int i = 0; i < r.note_count; ++i) {
        if (!r.notes[i].straddling_start()) {
            ++onset_count;
        }
    }
    return static_cast<float>(onset_count) / r.span;
}

/**
 * Mean velocity of all notes.
 * Input: WagonReadout
 * Output: 0.0 to 1.0
 */
inline float wagon_velocity_mean(const WagonReadout& r) {
    if (r.note_count == 0) return 0.0f;
    float sum = 0.0f;
    for (int i = 0; i < r.note_count; ++i) {
        sum += r.notes[i].velocity;
    }
    return sum / r.note_count;
}

/**
 * Max velocity in window.
 * Input: WagonReadout
 * Output: 0.0 to 1.0
 */
inline float wagon_velocity_max(const WagonReadout& r) {
    float max_vel = 0.0f;
    for (int i = 0; i < r.note_count; ++i) {
        if (r.notes[i].velocity > max_vel) max_vel = r.notes[i].velocity;
    }
    return max_vel;
}

/**
 * Mean note duration within window.
 * Input: WagonReadout
 * Output: beats
 */
inline float wagon_duration_mean(const WagonReadout& r) {
    if (r.note_count == 0) return 0.0f;
    float sum = 0.0f;
    for (int i = 0; i < r.note_count; ++i) {
        sum += r.notes[i].window_duration();
    }
    return sum / r.note_count;
}

/**
 * Count of notes in the window whose pitch class equals `pc`.
 * Pitch classes are taken relative to `origin` (default 0 = C-origin).
 * Input: WagonReadout, pc 0-11, origin 0-11. Output: integer count.
 */
inline int wagon_pc_count(const WagonReadout& r, int pc, int origin = 0) {
    int count = 0;
    for (int i = 0; i < r.note_count; ++i)
        if (pc_relative_to(pc_of(r.notes[i].pitch), origin) == pc) ++count;
    return count;
}

/**
 * Length-weighted presence of pitch class `pc`: sum of the in-window
 * durations of notes with that pitch class. Pitch classes are taken
 * relative to `origin` (default 0 = C-origin).
 * Input: WagonReadout, pc 0-11, origin 0-11. Output: total beats.
 */
inline float wagon_pc_length(const WagonReadout& r, int pc, int origin = 0) {
    float total = 0.0f;
    for (int i = 0; i < r.note_count; ++i)
        if (pc_relative_to(pc_of(r.notes[i].pitch), origin) == pc)
            total += r.notes[i].window_duration();
    return total;
}

/**
 * Total in-window sounding time across all notes.
 * Input: WagonReadout. Output: total beats.
 */
inline float wagon_total_length(const WagonReadout& r) {
    float total = 0.0f;
    for (int i = 0; i < r.note_count; ++i)
        total += r.notes[i].window_duration();
    return total;
}

// ═══ CROSS-READOUT OPERATIONS ════════════════════════════════════

/**
 * Pitch class similarity between Playhead CURRENT and Wagon.
 * Uses cosine similarity of velocity-weighted vectors.
 */
inline float similarity_playhead_wagon(const PlayheadReadout& pr, const WagonReadout& wr) {
    PitchClassVector a = extract_pitch_class_vector_current(pr);
    PitchClassVector b = extract_pitch_class_vector_weighted(wr);
    return pcv_cosine_similarity(a, b);
}

/**
 * Pitch class similarity between two Wagons.
 */
inline float similarity_wagon_wagon(const WagonReadout& a, const WagonReadout& b) {
    PitchClassVector va = extract_pitch_class_vector_weighted(a);
    PitchClassVector vb = extract_pitch_class_vector_weighted(b);
    return pcv_cosine_similarity(va, vb);
}

/**
 * Pitch class similarity between two Playhead CURRENTs.
 */
inline float similarity_playhead_playhead(const PlayheadReadout& a, const PlayheadReadout& b) {
    PitchClassVector va = extract_pitch_class_vector_current(a);
    PitchClassVector vb = extract_pitch_class_vector_current(b);
    return pcv_cosine_similarity(va, vb);
}

} // namespace t7
