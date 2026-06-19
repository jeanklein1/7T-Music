#pragma once

// ─── coupling/trajectory.hpp ─────────────────────────────────────────
//
// The coupling layer's linear motion system (CPU side). A coupling turns a
// musical reading into a moving visual parameter; this is the mechanism that
// moves it. Two layers:
//
//   MOVE   (Segment, plan_segment, sample_segment) — a straight line from a
//          value to a target over a span of beats, then a hold. Sampled on
//          the beat clock, arrival is exact at start_beat + duration_beats.
//
//   FOLLOW (trajectory_release(Segment&, …)) — the per-frame call every
//          coupling shares: hold one Segment, re-aim at the goal, read the
//          value. A value returns to idle in span_beats once its goal settles
//          (default DEFAULT_RELEASE_BEATS); the move lands exactly, so no
//          endpoint snapping is needed.
//
// COMPAT (TEMPORARY, at the foot). The previous first-order release survives
// as the Trajectory overload — exponential, the prior behavior — purely so the
// not-yet-migrated modules (musical.inl, pawn.inl) keep building and running
// unchanged. Convert those call sites to the Segment FOLLOW one at a time;
// when none remain, delete the COMPAT section and the <cmath> include.
//
// Linear, by choice. The move is velocity-blind — it kicks as it leaves and
// stops dead as it lands, and a re-aim snaps to a new rate. Accepted: idle is
// just another target, leaving and returning are one move, exact arrival
// means no snapping. Easing and asymptotic feedback are deferred; their math
// is kept in design notes.
//
// The GPU keeps its own release primitive in world.wgsl §1.2; this header
// does not mirror it — a move is CPU-side intent the GPU realizes as geometry.
//
// SEAM[trajectory:foundations] the coupling foundation. The linear system has
//   no dependencies; <cmath> is pulled only for the temporary COMPAT shim.

#include <cmath>

namespace t7 {

    // ═══ MOVE — a linear segment ═══════════════════════════════════════════════
    //
    // A straight run from `from` to `to` over `duration_beats`, anchored at
    // `start_beat`. Arrival is exact at the end; outside the span, the endpoints.

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
    // over `span_beats` from wherever the value is; when the goal holds, the move
    // completes and the value rests exactly on it — so a return to idle takes
    // exactly `span_beats`. Seed the Segment to the coupling's idle value (e.g.
    // Segment{idle,idle,0,0}) so the first move departs from idle.

    inline constexpr float DEFAULT_RELEASE_BEATS = 8.0f;

    inline float trajectory_release(Segment& seg, float goal, float beat,
        float span_beats = DEFAULT_RELEASE_BEATS) {
        if (seg.to != goal) {          // goal moved → re-aim from the current value
            seg = plan_segment(sample_segment(seg, beat), goal, span_beats, beat);
        }
        return sample_segment(seg, beat);
    }

    // ═══ COMPAT — first-order release (TEMPORARY) ══════════════════════════════
    //
    // The previous release primitive, verbatim, so musical.inl and pawn.inl build
    // and run unchanged until each migrates to the Segment FOLLOW above.
    // Exponential (the prior behavior); matches world.wgsl §1.2. It overloads on
    // Trajectory, so it never collides with the Segment FOLLOW. DELETE this whole
    // section, and the <cmath> include, once no module calls the Trajectory form.

    struct Trajectory {
        float value = 0.0f;
        float velocity = 0.0f;
        float _pad0 = 0.0f;
        float _pad1 = 0.0f;
    };

    inline Trajectory trajectory_release(Trajectory t, float goal, float dt, float rate) {
        const float new_val = t.value + (goal - t.value) * (1.0f - std::exp(-rate * dt));
        return Trajectory{ new_val, 0.0f, 0.0f, 0.0f };
    }

} // namespace t7