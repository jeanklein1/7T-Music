// ─── trajectory.inl ──────────────────────────────────────────────
//
// CPU mirror of WGSL §1.2 TRAJECTORY PRIMITIVES (world.wgsl line 176).
// A Trajectory is a scalar value plus its velocity, smoothed over time
// toward a goal via exponential release. The shape exists on the GPU
// for analytical-amplitude releases; the CPU side mirrors it so per-
// frame ramps in the spine collapse from copy-pasted exp(-rate*dt)
// expressions into a single named primitive.
//
// Included inside the Cartridge class body.
// Depends on: nothing (foundations module — pure math)
//
// SEAM[trajectory:contract] MUST match world.wgsl §1.2 Trajectory.
//   Field shape doesn't need to be byte-identical (this struct is
//   never uploaded), but the release semantics MUST agree:
//     new_val = old + (goal - old) * (1 - exp(-rate * dt))
//   Drift between the two formulas would mean CPU-side ramps and
//   GPU-side ramps move at different speeds for the same rate.
// SEAM[trajectory:foundations] new module created Phase 4.1
//   (resolves D-trajectory:a). Lives between seed_utils.inl and
//   the consumer modules so Trajectory is visible to musical.inl,
//   pawn.inl, mood.inl when they declare Trajectory fields.
// ─────────────────────────────────────────────────────────────────

struct Trajectory {
    float value    = 0.0f;
    float velocity = 0.0f;   // reserved for spring-style ramps; today release-only writes 0
    float _pad0    = 0.0f;
    float _pad1    = 0.0f;
};

// Exponential release toward a goal — one step of the per-frame ramp.
// Returns the updated Trajectory (caller assigns; matches WGSL shape).
//
// new_value = old + (goal - old) * (1 - exp(-rate * dt))
//
// rate units: 1/seconds. Higher rate → faster catch-up.
// For symmetric attack/release, pick one rate. For different
// attack vs release rates, pick at the call site:
//   t = trajectory_release(t, goal, dt, (goal > t.value) ? attack : release);
static Trajectory trajectory_release(Trajectory t, float goal, float dt, float rate) {
    const float new_val = t.value + (goal - t.value) * (1.0f - std::exp(-rate * dt));
    return Trajectory{ new_val, 0.0f, 0.0f, 0.0f };
}
