#pragma once

/**
 * TRAJECTORY - Shared scalar release primitive (CPU side)
 * =======================================================
 *
 * A Trajectory is a scalar value plus its velocity, smoothed over time
 * toward a goal via exponential release. The shape exists on the GPU
 * for analytical-amplitude releases (world.wgsl §1.2 TRAJECTORY PRIMITIVES);
 * this header is the CPU mirror so per-frame ramps in the spine collapse
 * from copy-pasted exp(-rate*dt) expressions into a single named primitive.
 *
 * Public surface:
 *   Trajectory                            — { value, velocity, _pad0, _pad1 }
 *   trajectory_release(t, goal, dt, rate) → Trajectory
 *
 * Consumers (verified by grep for the #include and trajectory_release call):
 *   cartridges/the_chord/modules/musical.inl   — calls trajectory_release
 *   cartridges/the_chord/modules/pawn.inl      — calls trajectory_release
 *   cartridges/the_board/modules/musical.inl   — calls trajectory_release
 *   cartridges/the_board/modules/pawn.inl      — calls trajectory_release
 *   the_lab.cpp                                — calls trajectory_release (iterating couplings)
 *   cartridges/the_chord/cartridge.hpp         — #includes this header
 *   cartridges/the_board/cartridge.hpp         — #includes this header
 *
 * SEAM[trajectory:contract] MUST match world.wgsl §1.2 Trajectory.
 *   Field shape doesn't need to be byte-identical (this struct is
 *   never uploaded), but the release semantics MUST agree:
 *     new_val = old + (goal - old) * (1 - exp(-rate * dt))
 *   Drift between the two formulas would mean CPU-side ramps and
 *   GPU-side ramps move at different speeds for the same rate.
 *
 * SEAM[trajectory:foundations] foundations layer — pure math, no deps.
 *   Same family as seed_utils as a P9 instance.
 *
 * HISTORY
 * -------
 * Previously lived as trajectory.inl spliced into Cartridge class body.
 * Factored to a header so both the_board's musical.inl and the_lab's
 * main loop share the same primitive without textual duplication.
 */

#include <cmath>

namespace t7 {

struct Trajectory {
    float value    = 0.0f;
    float velocity = 0.0f;   // reserved for spring-style ramps; today release-only writes 0
    float _pad0    = 0.0f;
    float _pad1    = 0.0f;
};

// Exponential release toward a goal — one step of the per-frame ramp.
// Returns the updated Trajectory (caller assigns; matches WGSL shape).
//
//   new_value = old + (goal - old) * (1 - exp(-rate * dt))
//
// rate units: 1/seconds. Higher rate → faster catch-up.
// For symmetric attack/release, pick one rate. For different
// attack vs release rates, pick at the call site:
//   t = trajectory_release(t, goal, dt, (goal > t.value) ? attack : release);
inline Trajectory trajectory_release(Trajectory t, float goal, float dt, float rate) {
    const float new_val = t.value + (goal - t.value) * (1.0f - std::exp(-rate * dt));
    return Trajectory{ new_val, 0.0f, 0.0f, 0.0f };
}

} // namespace t7
