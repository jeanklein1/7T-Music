#pragma once

// ─── coupling/trajectory.hpp ─────────────────────────────────────
//
// The coupling layer's scalar motion primitive (CPU side). A coupling turns
// a musical reading into a moving visual parameter; this header is the
// mechanism that moves it. It carries the kinematic state a parameter
// occupies — value, velocity, acceleration — and offers two ways to drive
// that state over time, one for each thing a coupling ever asks of motion.
//
//   PLANNED (Segment, plan_segment, sample_segment) — "arrive there, in
//   sync." A boundary-condition polynomial from the current state to a
//   target landing state over an exact span of beats. The arrival time is an
//   input, not an outcome, so a gesture meant to land on a beat lands on it;
//   the landing velocity and acceleration are inputs too, so it can stop dead
//   or glide through at a chosen speed. Fitting from the CURRENT
//   value/velocity/acceleration is what makes a mid-flight re-target
//   continuous: a cubic is C1 across the seam, a quintic C2. This is the
//   primary tool — everything that must synchronize uses it.
//
//   FEEDBACK (trajectory_release) — "follow it, or ring." First-order
//   exponential release toward a goal: asymptotic, never formally arriving,
//   but re-targeting for free by reading only the current value and the goal.
//   Its home is the two couplings that never have to arrive — a goal that
//   drifts continuously (tracking) and a struck parameter that decays (ring).
//   Unchanged from its prior life; the planned half is the new arrival.
//
// On the clock. The planned primitive runs on the beat clock: sample_segment
// takes a beat, and arrival is exact at start_beat + duration_beats, so a
// gesture meant to land on a beat lands on it even as the tempo breathes. The
// release is unit-agnostic — rate and dt share whatever unit the caller uses
// (the legacy stubs pass seconds; new couplings pass beats, matching planned).
//
// SEAM[trajectory:contract] trajectory_release MUST match world.wgsl §1.2
//   Trajectory. The struct is never uploaded, so its layout need not be
//   byte-identical, but the release semantics MUST agree:
//     new_val = old + (goal - old) * (1 - exp(-rate * dt))
//   The PLANNED primitive has no GPU mirror — it is CPU-side intent the GPU
//   realizes as geometry — so it adds no shader obligation.
//
// SEAM[trajectory:foundations] the coupling foundation — pure math, no deps.
//   Same family as seed_utils.
//
// Depends on: <cmath>, <cstdint>.
//
// HISTORY
//   Moved from musical/ to coupling/ (it is the middleman's tool, not the
//   analysis's) and gained the planned-polynomial primitives. Previously the
//   release alone, factored from a per-cartridge trajectory.inl.

#include <cmath>
#include <cstdint>

namespace t7 {

    // ═══ KINEMATIC STATE ═════════════════════════════════════════════
    //
    // The state a driven parameter occupies: its value and the first two
    // derivatives. The planned sampler fills all three; the feedback release
    // fills value and leaves the higher derivatives zero (first-order carries no
    // momentum). Sixteen bytes, never uploaded.

    struct Trajectory {
        float value = 0.0f;
        float velocity = 0.0f;
        float accel = 0.0f;
        float _pad1 = 0.0f;
    };
    static_assert(sizeof(Trajectory) == 16, "Trajectory stays 16 bytes");

    // ═══ FEEDBACK — first-order release ══════════════════════════════
    //
    // Exponential approach toward a goal, one step of the per-frame ramp. For
    // tracking a moving goal and for strike-and-ring; NOT for synchronized
    // arrival (it is asymptotic). rate and dt share a time unit (rate 1/u, dt u);
    // the formula is unit-agnostic. Writes value; leaves velocity and accel zero.
    //
    //   new_value = old + (goal - old) * (1 - exp(-rate * dt))

    inline Trajectory trajectory_release(Trajectory t, float goal, float dt, float rate) {
        const float new_val = t.value + (goal - t.value) * (1.0f - std::exp(-rate * dt));
        return Trajectory{ new_val, 0.0f, 0.0f, 0.0f };
    }

    // ═══ PLANNED — boundary-condition polynomial ═════════════════════
    //
    // The continuity class is the polynomial's degree: a cubic matches position
    // and velocity at both ends (C1); a quintic also matches acceleration (C2).
    // The enum value IS the degree.

    enum class Continuity : uint8_t {
        C1 = 3,   // cubic   — match position and velocity at both ends
        C2 = 5,   // quintic — match position, velocity, and acceleration
    };

    // A fitted segment: monomial coefficients in elapsed beats τ = beat -
    // start_beat, plus the span. Arrival is exact at start_beat + duration_beats.
    struct Segment {
        float   a[6] = { 0, 0, 0, 0, 0, 0 };   // a0 + a1·τ + … + a5·τ⁵  (cubic uses a0..a3)
        float   start_beat = 0.0f;
        float   duration_beats = 0.0f;                 // T — the exact arrival span
        uint8_t degree = 5;                    // 3 = cubic, 5 = quintic
        uint8_t _pad[3] = { 0, 0, 0 };
    };

    // Fit from the CURRENT kinematic state to a target landing state over
    // `duration_beats`, anchored at `start_beat`. Seeding from the current
    // value/velocity/acceleration is what makes a mid-flight re-target continuous.
    // A non-positive span degenerates to an immediate hold at the target.
    inline Segment plan_segment(const Trajectory& from, const Trajectory& to,
        float duration_beats, float start_beat,
        Continuity cont = Continuity::C2) {
        Segment s{};
        s.start_beat = start_beat;
        s.duration_beats = duration_beats;
        s.degree = static_cast<uint8_t>(cont);

        const float h = duration_beats;
        if (h <= 0.0f) {                 // degenerate span: arrive now, hold there
            s.a[0] = to.value;
            s.duration_beats = 0.0f;
            return s;
        }

        const float x0 = from.value, v0 = from.velocity, ac0 = from.accel;
        const float x1 = to.value, v1 = to.velocity, ac1 = to.accel;

        s.a[0] = x0;
        s.a[1] = v0;

        if (cont == Continuity::C1) {                  // cubic Hermite
            const float P = x1 - x0 - v0 * h;
            const float Q = v1 - v0;
            s.a[2] = 3.0f * P / (h * h) - Q / h;
            s.a[3] = -2.0f * P / (h * h * h) + Q / (h * h);
        }
        else {                                       // quintic Hermite
            const float h2 = h * h, h3 = h2 * h, h4 = h3 * h, h5 = h4 * h;
            s.a[2] = 0.5f * ac0;
            const float R0 = x1 - x0 - v0 * h - 0.5f * ac0 * h2;
            const float R1 = v1 - v0 - ac0 * h;
            const float R2 = ac1 - ac0;
            s.a[3] = (10.0f * R0 - 4.0f * R1 * h + 0.5f * R2 * h2) / h3;
            s.a[4] = (-15.0f * R0 + 7.0f * R1 * h - R2 * h2) / h4;
            s.a[5] = (6.0f * R0 - 3.0f * R1 * h + 0.5f * R2 * h2) / h5;
        }
        return s;
    }

    // Sample on the beat clock. Before the span it reads the seeded start state;
    // within it, the polynomial and its first two derivatives; once
    // beat ≥ start_beat + duration_beats the value goes constant — the hold —
    // exact for a rest landing (a glide-through re-plans before then).
    inline Trajectory sample_segment(const Segment& seg, float beat) {
        const float* a = seg.a;
        const float tau = beat - seg.start_beat;

        if (tau >= seg.duration_beats) {
            const float T = seg.duration_beats;
            const float pos =
                ((((a[5] * T + a[4]) * T + a[3]) * T + a[2]) * T + a[1]) * T + a[0];
            return Trajectory{ pos, 0.0f, 0.0f, 0.0f };
        }

        const float t = (tau > 0.0f) ? tau : 0.0f;     // defensive clamp at the start
        const float value =
            ((((a[5] * t + a[4]) * t + a[3]) * t + a[2]) * t + a[1]) * t + a[0];
        const float vel =
            (((5.0f * a[5] * t + 4.0f * a[4]) * t + 3.0f * a[3]) * t + 2.0f * a[2]) * t + a[1];
        const float acc =
            ((20.0f * a[5] * t + 12.0f * a[4]) * t + 6.0f * a[3]) * t + 2.0f * a[2];
        return Trajectory{ value, vel, acc, 0.0f };
    }

} // namespace t7