#pragma once
#include <cstdint>

// ─── floater_vocabulary.hpp (TYPES) ──────────────────────────────
//
// The runtime-tracking TYPES for the two generic-pipeline floater
// families: ActiveFloater (sphere) and ActiveCube (cube). Pure structs,
// shared vocabulary — never state (the state ARRAYS moved to their owners
// in LADDER-2 c0: SphereState in spheres.hpp; CubeBehaviorsState in
// cube_behaviors.inl).
//
// LADDER-2 NOTE (disclosed deviation on ordering): these type definitions
//   lead the vocabulary's headerization. The state owners — spheres.hpp
//   (born converted, file scope) and CubeBehaviorsState — must reference
//   ActiveFloater / ActiveCube at file scope, so the types graduate here at
//   c0. The vocabulary's CONFIGS / tier tables / property registries stay
//   in floater_vocabulary.inl (class body) until c4, when they join this
//   header and the .inl is retired. c0.3 ("types stay in the vocabulary")
//   is honored — the vocabulary is this header now.
//
// STATUS: LATENT[naming] — ActiveFloater is the sphere family's active
//   struct; the ActiveFloater -> ActiveSphere rename is flagged, not
//   performed (seed-stable churn rides a later stage; the "FloatingEntity"
//   property name is likewise preserved for hash stability — see the .inl).
//
// SEAM[sphere:P5] last_alloc_time is pattern P5 (release-pending sentinel /
//   race protection) — CPU-timestamp variant. When GPU readback arrives
//   stale ("kernel evicted this slot"), the timestamp protects freshly-
//   allocated slots from being incorrectly marked inactive. Same intent as
//   cube_behaviors.inl::toggle_cube_kite_mode's GPU sentinel; different
//   mechanism.
// SEAM[cube:cx-cz-mirror] ActiveCube has cx, cz fields — CPU mirror of GPU
//   anchor for cube_behaviors.inl::corral_cubes / toggle_cube_kite_mode to
//   read without GPU readback. Same family as agents:D2 (slot-0 reads);
//   when pawn.inl extracts and provides accessors, corral/kite could
//   analogously have cube_anchor(slot) accessors.
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ── Active Sphere Tracking ───────────────────────────────────────
struct ActiveFloater {
    int32_t patch_gx = 0, patch_gz = 0;
    int32_t host_gx = 0, host_gz = 0;
    // See ActiveCube::last_alloc_time — same race protection for
    // sphere slots. Spheres rarely evict in practice (orbital,
    // anchored at origin), but the readback path covers them
    // uniformly so the protection covers them uniformly too.
    float   last_alloc_time = -1000.0f;
    bool active = false;
};

// ── Active Cube Tracking ─────────────────────────────────────────
struct ActiveCube {
    int32_t patch_gx = 0, patch_gz = 0;
    int32_t host_gx = 0, host_gz = 0;
    // World XZ of the cube's anchor — mirror of fe.anchor[0,2] on GPU.
    // Captured at spawn so cube_behaviors.inl::corral_cubes can read
    // the current anchor without a GPU readback. Updated when corral
    // writes a new anchor.
    float   cx = 0.0f, cz = 0.0f;
    // Time (time_state_.seconds) when this slot was last marked active.
    // Used to suppress race between freshly allocated slots and the
    // floater readback path: readback callbacks process previous-frame
    // data, so a slot allocated this frame would be incorrectly marked
    // inactive by the readback (which sees the *prior tenant* as
    // evicted). Suppression window covers two readback cycles. See
    // render() floater sync block for the consumer.
    float   last_alloc_time = -1000.0f;
    bool active = false;
};

} // namespace the_board
} // namespace t7
