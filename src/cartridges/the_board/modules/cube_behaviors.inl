// ─── cube_behaviors.inl (IMPL: post-class definitions) ───────────
// Impl of cube_behaviors.hpp (LADDER-3 c3): history in audit/LADDER.md.
//
// Definitions for cube_behaviors.hpp's declared laws, plus the module-
// internal helpers (corral_ease_, apply_cube_behavior_override). The
// bodies reach c->gpuState_ / c->agent_state_ / c->player_ /
// c->time_state_.
//
// WRAPPING FORM (the proven fix-2 rule): SELF-WRAPPING — opens
// t7::the_board itself, carries its own standard includes; the MODULE
// IMPLEMENTATIONS zone includes it at FILE SCOPE. Definitions are
// `inline` free functions.
// ─────────────────────────────────────────────────────────────────

#include <cmath>      // std::cos, std::sin
#include <algorithm>  // std::clamp
#include <iostream>   // diagnostics feedback

namespace t7 {
namespace the_board {

// Apply tier gains to base substrate values. Called by cube_write_gpu
// during spawn — the result is what gets stored on the cube.
inline void apply_cube_tier_gains(float& spring_stiffness, float& drag, uint32_t tier_idx) {
    if (tier_idx >= CUBE_TIER_COUNT) return;  // defensive; tier_idx is bounded at select
    const auto& g = CUBE_TIER_GAINS[tier_idx];
    spring_stiffness *= g.spring_stiffness_mult;
    drag             *= g.drag_mult;
}

// Stochastic behavior pick. seed is the cube's spawn seed (used as
// the entity_seed in the GPU struct), so behavior assignments are
// deterministic across runs with the same world seed. mood_id is
// the active mood at spawn time. Returns one of CUBE_BEHAVIOR_*.
//
// Mix constant 0xBEEF11A0u is unrelated to behavior_phase's mix
// constant (0xF10A7E70u in entity_pipeline.inl) so the two derived
// values are decorrelated.
inline uint32_t pick_cube_behavior_for_spawn(uint32_t mood_id, uint32_t seed) {
    if (mood_id >= MOOD_COUNT) return CUBE_BEHAVIOR_STATIONARY;
    const auto& pop = CUBE_POPULATIONS[mood_id];

    float total = 0.0f;
    for (uint32_t i = 0; i < CUBE_BEHAVIOR_COUNT; i++) total += pop.behavior_weights[i];
    if (total <= 0.0f) return CUBE_BEHAVIOR_STATIONARY;

    uint32_t h = cpu_hash(seed, 0xBEEF11A0u);
    float r = (float(h) / 4294967296.0f) * total;

    float acc = 0.0f;
    for (uint32_t i = 0; i < CUBE_BEHAVIOR_COUNT; i++) {
        acc += pop.behavior_weights[i];
        if (r < acc) return i;
    }
    // Numerical edge case (r == total exactly): return last non-zero.
    for (uint32_t i = CUBE_BEHAVIOR_COUNT; i > 0; i--) {
        if (pop.behavior_weights[i - 1] > 0.0f) return i - 1;
    }
    return CUBE_BEHAVIOR_STATIONARY;  // unreachable; total > 0 was checked
}

// ═══ DIAGNOSTICS ═════════════════════════════════════════════════
//
// Inspection tools, not part of the system's primary expression.
// Keypress-driven; the system runs without them. Anything below
// this point can be commented out and the cubes still spawn,
// populate, and behave correctly per the registries in the header.
//
// ─── Coordination cycle ─────────────────────────────────────────────
//
// F5 steps coordination through 0.0 / 0.5 / 1.0 for quick visual
// inspection. Music-driven smooth values are a future concern.
//
// ─── Behavior override ──────────────────────────────────────────────
//
// F4 walks every active cube slot and rewrites behavior_id. No
// "OVERRIDE_NONE" sentinel because the natural state is already
// "whatever the population assigned at spawn"; the cycle just steps
// 0 → 1 → 2 → 0. Useful for visually verifying each behavior's
// look without waiting for a population to favor it.
//
// ─── Corral animation ──────────────────────────────────────────────
//
// F6 smoothly gathers every active cube into a small ring around
// the pawn. Animates anchor (anchor mode) or pawn_offset (kite mode)
// over CUBE_CORRAL_DURATION using smooth-step easing.
//
// Re-pressing mid-glide captures the current interpolated value as
// the new from-position so the formation doesn't snap.
//
// ─── Kite mode ─────────────────────────────────────────────────────
//
// F7 toggles cubes between anchor-relative (default) and pawn-
// relative (kite) anchoring. In kite mode the kernel computes home
// from pawn.pos + pawn_offset, so cubes follow the pawn around the
// world like kites on invisible strings.
//
// Toggle on  — captures each cube's current xz as a pawn-relative
// offset. Cube xz is preserved exactly; y resets to pawn.y +
// orbit_height (small visible jump only if pawn's altitude differs
// from where the cube was hovering).
//
// Toggle off — anchor freezes at the cube's current world xz.
// pawn_offset tracks the per-cube offset so we reconstruct
// world position correctly: cube_xz = pawn.xz + offset.xz.

// The cube half of teardown_world's bulk sweep — CPU clear + per-slot
// GPU clear, paired. Uses only the GPU service, matching
// spheres.hpp::clear_spheres for symmetry.
inline void clear_cubes(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        cbs.activeCubes_[i] = ActiveCube{};
        GPUFloatingEntityState empty{};
        gpu.upload_cube_entity_slot(queue, i, empty);
    }
    cbs.activeCubeCount_ = 0;
}

inline float corral_ease_(float t) {
    return t * t * (3.0f - 2.0f * t);  // smoothstep
}

inline void cycle_floater_coordination(CubeBehaviorsState& cbs, Cartridge* c) {
    cbs.coordination_step = (cbs.coordination_step + 1) % 3;
    float v = FLOATER_COORDINATION_STEPS[cbs.coordination_step];
    c->gpuState_.config().floater_coordination = v;
    std::cout << "[Floaters] coordination: " << v << "\n";
}

inline void apply_cube_behavior_override(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        if (!cbs.activeCubes_[i].active) continue;
        c->gpuState_.upload_cube_behavior_id(queue, i, cbs.behavior_override);
    }
}

inline void cycle_cube_behavior_override(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue) {
    cbs.behavior_override = (cbs.behavior_override + 1) % CUBE_BEHAVIOR_COUNT;
    apply_cube_behavior_override(cbs, c, queue);
    std::cout << "[Floaters] cube behavior: "
              << CUBE_BEHAVIOR_NAMES[cbs.behavior_override] << "\n";
}

inline void corral_cubes(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue) {
    (void)queue;
    const float px = c->agent_state_.slots[c->player_.possessed_slot].pos_x;
    const float pz = c->agent_state_.slots[c->player_.possessed_slot].pos_z;

    uint32_t active_count = 0;
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        if (cbs.activeCubes_[i].active) active_count++;
    }
    if (active_count == 0) {
        std::cout << "[Floaters] corral: no active cubes\n";
        return;
    }

    // Distribute by stable index so successive presses don't shuffle
    // the layout. In kite mode the ring is in pawn-relative offset
    // space; otherwise world space.
    uint32_t armed = 0;
    uint32_t k = 0;
    const float two_pi = 6.28318530718f;
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        if (!cbs.activeCubes_[i].active) continue;
        float theta = (float(k) / float(active_count)) * two_pi;
        float target_x, target_z;
        if (cbs.kite_mode) {
            target_x = std::cos(theta) * CUBE_CORRAL_RADIUS;
            target_z = std::sin(theta) * CUBE_CORRAL_RADIUS;
        } else {
            target_x = px + std::cos(theta) * CUBE_CORRAL_RADIUS;
            target_z = pz + std::sin(theta) * CUBE_CORRAL_RADIUS;
        }

        // Start position: current interpolated value if a previous
        // animation is in flight, else the CPU mirror.
        float from_x, from_z;
        if (cbs.corral_anim[i].active) {
            float t_norm = (c->time_state_.seconds - cbs.corral_anim[i].t0)
                           / cbs.corral_anim[i].duration;
            t_norm = std::clamp(t_norm, 0.0f, 1.0f);
            float eased = corral_ease_(t_norm);
            from_x = cbs.corral_anim[i].ax_from
                    + (cbs.corral_anim[i].ax_to - cbs.corral_anim[i].ax_from) * eased;
            from_z = cbs.corral_anim[i].az_from
                    + (cbs.corral_anim[i].az_to - cbs.corral_anim[i].az_from) * eased;
        } else if (cbs.kite_mode) {
            from_x = cbs.pawn_offset[i][0];
            from_z = cbs.pawn_offset[i][1];
        } else {
            from_x = cbs.activeCubes_[i].cx;
            from_z = cbs.activeCubes_[i].cz;
        }

        cbs.corral_anim[i] = {
            /*active=*/   true,
            /*t0=*/       c->time_state_.seconds,
            /*duration=*/ CUBE_CORRAL_DURATION,
            from_x, from_z,
            target_x, target_z,
        };
        if (cbs.kite_mode) {
            cbs.pawn_offset[i][0] = target_x;
            cbs.pawn_offset[i][1] = target_z;
        } else {
            cbs.activeCubes_[i].cx = target_x;
            cbs.activeCubes_[i].cz = target_z;
        }
        armed++;
        k++;
    }

    std::cout << "[Floaters] corral: " << armed << " cube(s) gliding "
              << (cbs.kite_mode ? "to ring offset around pawn" : "to ring")
              << " radius " << CUBE_CORRAL_RADIUS
              << " over " << CUBE_CORRAL_DURATION << "s\n";
}

// Per-frame: advance any active animations and push interpolated
// position to GPU. Skips cleanly when nothing is animating. Called
// from the cartridge's render() path.
inline void tick_cube_corral_animations(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        auto& anim = cbs.corral_anim[i];
        if (!anim.active) continue;
        if (!cbs.activeCubes_[i].active) { anim.active = false; continue; }

        float t_norm = (c->time_state_.seconds - anim.t0) / anim.duration;
        bool finishing = false;
        if (t_norm >= 1.0f) { t_norm = 1.0f; finishing = true; }
        if (t_norm < 0.0f) t_norm = 0.0f;
        float eased = corral_ease_(t_norm);
        float ax = anim.ax_from + (anim.ax_to - anim.ax_from) * eased;
        float az = anim.az_from + (anim.az_to - anim.az_from) * eased;
        if (cbs.kite_mode) {
            c->gpuState_.upload_cube_pawn_offset(queue, i, ax, 0.0f, az);
        } else {
            c->gpuState_.upload_cube_anchor(queue, i, ax, 0.0f, az);
        }
        if (finishing) anim.active = false;
    }
}

// ─── Kite mode toggle (F7) ──────────────────────────────────────

inline void toggle_cube_kite_mode(CubeBehaviorsState& cbs, Cartridge* c, wgpu::Queue& queue) {
    cbs.kite_mode = !cbs.kite_mode;
    const float px = c->agent_state_.slots[c->player_.possessed_slot].pos_x;
    const float pz = c->agent_state_.slots[c->player_.possessed_slot].pos_z;

    uint32_t affected = 0;
    if (cbs.kite_mode) {
        for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
            if (!cbs.activeCubes_[i].active) continue;
            float ox = cbs.activeCubes_[i].cx - px;
            float oz = cbs.activeCubes_[i].cz - pz;
            cbs.pawn_offset[i][0] = ox;
            cbs.pawn_offset[i][1] = oz;
            c->gpuState_.upload_cube_pawn_offset(queue, i, ox, 0.0f, oz);
            c->gpuState_.upload_cube_follow_pawn(queue, i, 1u);
            affected++;
        }
    } else {
        // Toggle OFF: rather than write anchor on CPU (which can't see
        // the cube's drift, so anchor would land at home rather than
        // at the cube's visible position), we hand the work to the
        // kernel. follow_pawn = 2u is a "release pending" sentinel:
        // the next update_cube run sees it, sets anchor = current pos,
        // zeroes drift, switches follow_pawn back to 0. Visible
        // position is preserved exactly — no CPU drift estimation
        // needed, no readback latency.
        for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
            if (!cbs.activeCubes_[i].active) continue;
            // Update CPU mirror cx/cz to a best-effort estimate so a
            // subsequent corral or kite-on doesn't start from a stale
            // value. The kernel's actual anchor write next frame may
            // differ by a few units of drift, but for diagnostic
            // anchoring this is close enough.
            float ax = px + cbs.pawn_offset[i][0];
            float az = pz + cbs.pawn_offset[i][1];
            cbs.activeCubes_[i].cx = ax;
            cbs.activeCubes_[i].cz = az;
            c->gpuState_.upload_cube_follow_pawn(queue, i, 2u);
            affected++;
        }
    }

    std::cout << "[Floaters] kite mode: " << (cbs.kite_mode ? "ON" : "OFF")
              << " (" << affected << " cube(s))\n";
}


// ═══ THE EVICTOR (lifecycle, absorbed per §5 EVICTION THUNKS) ═════
//
// Named by the FAMILY_DISPATCH table (family_dispatch.inl).

inline void evict_cube(Cartridge* self,
    uint32_t slot, wgpu::Queue& queue) {
    self->cube_behaviors_state_.activeCubes_[slot].active = false;  // cube state owned by CubeBehaviorsState
    self->cube_behaviors_state_.activeCubeCount_--;
    GPUFloatingEntityState empty{};
    self->gpuState_.upload_cube_entity_slot(queue, slot, empty);
#ifdef DIAG_ENTITY_LIFECYCLE
    std::cout << "[DIAG:EVICT]   cube slot=" << slot << "\n";
#endif
}

} // namespace the_board
} // namespace t7
