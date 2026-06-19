#pragma once

// ─── coupling/trajectory.hpp ─────────────────────────────────────────
//
// The coupling layer's linear motion system (CPU side). A coupling turns a
// musical reading into a moving visual parameter; this is the one mechanism
// that moves it, the same way for every coupling. Two layers:
//
//   MOVE   (Segment, plan_segment, sample_segment) — a straight line from a
//          value to a target over a span of beats, then a hold. Sampled on
//          the beat clock, it arrives exactly at start_beat + duration_beats.
//
//   FOLLOW (trajectory_release) — what a coupling calls each frame: hold one
//          Segment, re-aim it at the current goal, read the value. While the
//          goal keeps moving the value chases it; once the goal settles — at
//          idle when the stimulus stops — the last move runs to completion,
//          so a value returns to idle in exactly DEFAULT_RELEASE_BEATS.
//
// Linear, by choice. The move is velocity-blind — it ignores how fast the
// value was already going, so it kicks as it leaves and stops dead as it
// lands, and a re-aim snaps to a new rate. Accepted for now: idle is just
// another target, leaving and returning are one move pointed at different
// ends, and exact arrival means no endpoint snapping is needed. Easing
// (curved arrivals) and feedback (asymptotic follow) are deferred; their
// math is kept in design notes.
//
// The GPU keeps its own release primitive in world.wgsl §1.2 for amplitude
// envelopes; this header does not mirror it — a move is CPU-side intent the
// GPU realizes as geometry, carrying no shader obligation.
//
// SEAM[trajectory:foundations] the coupling foundation — pure math, no deps.
//   Same family as seed_utils.
//
// Depends on: nothing.

namespace t7 {

    // ═══ MOVE — a linear segment ═══════════════════════════════════════════════
    //
    // A straight run from `from` to `to` over `duration_beats`, anchored at
    // `start_beat`. Arrival is exact at the end; outside the span it reads the
    // endpoints.

    struct Segment {
        float from = 0.0f;
        float to = 0.0f;
        float duration_beats = 0.0f;   // span; arrival is exact at start_beat + this
        float start_beat = 0.0f;
    };

    // Plan a move from a value to a target over `duration_beats`, anchored at
    // `start_beat`. A non-positive span is an immediate hold at the target.
    inline Segment plan_segment(float from, float to, float duration_beats, float start_beat) {
        return Segment{ from, to, (duration_beats > 0.0f) ? duration_beats : 0.0f, start_beat };
    }

    // Sample on the beat clock. Before the span → `from`; within it → the
    // straight lerp; once beat ≥ start_beat + duration_beats → `to` (the hold).
    inline float sample_segment(const Segment& seg, float beat) {
        if (seg.duration_beats <= 0.0f) return seg.to;
        const float u = (beat - seg.start_beat) / seg.duration_beats;
        if (u <= 0.0f) return seg.from;
        if (u >= 1.0f) return seg.to;
        return seg.from + (seg.to - seg.from) * u;
    }

    // ═══ FOLLOW — track a goal ═════════════════════════════════════════════════
    //
    // The per-frame entry point every coupling shares. Hold one Segment; call this
    // each frame with the current goal and beat. When the goal moves it re-aims
    // over DEFAULT_RELEASE_BEATS from wherever the value is; when the goal holds,
    // the move completes and the value rests exactly on it — so a return to idle
    // takes exactly DEFAULT_RELEASE_BEATS, no endpoint snapping. Seed the Segment
    // to the coupling's idle value (e.g. Segment{idle,idle,0,0}) so the first
    // move departs from idle.

    inline constexpr float DEFAULT_RELEASE_BEATS = 8.0f;

    inline float trajectory_release(Segment& seg, float goal, float beat) {
        if (seg.to != goal) {          // goal moved → re-aim from the current value
            seg = plan_segment(sample_segment(seg, beat), goal, DEFAULT_RELEASE_BEATS, beat);
        }
        return sample_segment(seg, beat);
    }

} // namespace t7