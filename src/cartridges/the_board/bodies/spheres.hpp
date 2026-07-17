#pragma once
#include "cartridges/the_board/realization/state.hpp"                    // Dim::*, GPUState, GPUFloatingEntityState, wgpu
#include "cartridges/the_board/contracts/floater_vocabulary.hpp"  // ActiveFloater
#include "cartridges/the_board/contracts/keyhole.hpp"          // Cartridge + wgpu::Queue fwds (the evictor decl)
#include "cartridges/the_board/contracts/entity_types.hpp"  // queue types (the funnel signatures) + MachineCtx + organ fwds
#include <iostream>   // [DIAG:EVICT] logging (flag-gated)   // (impl)

// ─── spheres.hpp (MERGED single file — DISSOLVE-1 Batch A d3) ─────
// Born converted LADDER-2 c0; deps-converted + merged Batch A.
// hpp+inl collapsed: SphereState + SphereDeps + inline impl.
//
// The Sphere family's runtime STATE — the active-slot mirror for the
// orbital-sphere floaters.
//
// The SphereState CONTENT (the ActiveFloater array) was the class-body
// `activeFloaters_[]` / `activeFloaterCount_` in floater_vocabulary.inl;
// it moves here whole. The ActiveFloater TYPE stays shared vocabulary
// (floater_vocabulary.hpp). ActiveFloater -> ActiveSphere rename is
// flagged there (STATUS: LATENT[naming]), not performed.
//
// SEAM[sphere:P5] the ActiveFloater last_alloc_time race protection lives
//   with the type (floater_vocabulary.hpp); this owner holds the array the
//   render() floater-sync block and the evict path read.
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

struct SphereState {
    ActiveFloater activeFloaters_[Dim::MAX_SPHERE_INSTANCES]{};
    uint32_t      activeFloaterCount_ = 0;
};

// ═══ MODULE DEPS (DISSOLVE-1 Batch A d2) ═══════════════════════════
// The sphere half's non-machine requirements face: the readback
// mirror reconciler reads the clock. (TimeState fwd arrives via
// entity_types.hpp / spine_state.hpp earlier in the cohort.)
struct SphereDeps {
    const TimeState& time_state_;
};

// Teardown owner-clear: the sphere half of the score's TEARDOWN
// movement (REBUILD-0 m2) — CPU clear + per-slot GPU clear, paired.
// DEPS-FORM PRECEDENT (m3 ruling): the explicit GPUState& parameter
// is the deps form's first citizen, born-converted — boundary-honest,
// callable WITHOUT the complete Cartridge. Reclassified from
// "keyhole bypass" by the stamp; not to be re-shaped keyhole-ward.
inline void clear_spheres(SphereState& ss, GPUState& gpu, wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::MAX_SPHERE_INSTANCES; i++) {
        ss.activeFloaters_[i] = ActiveFloater{};
        GPUFloatingEntityState empty{};
        gpu.upload_sphere_entity_slot(queue, i, empty);
    }
    ss.activeFloaterCount_ = 0;
}

void evict_sphere(MachineCtx* self, uint32_t slot, wgpu::Queue& queue);
void reconcile_sphere_mirror(SphereState& ss, SphereDeps* c, const GPUFloatingEntityState* data);
// Dispatch funnels (table-shaped; defined below beside the recipe)
bool dispatch_select_sphere_generic(MachineCtx* self, int32_t gx, int32_t gz, EntityQueueEntry& e);
bool dispatch_place_sphere_generic(MachineCtx* self, EntityQueueEntry& e, PlacementEntry& pe);
void dispatch_commit_sphere_generic(MachineCtx* self, PlacementEntry& pe, wgpu::Queue& queue);

// ═══ IMPL: the row
// bodies deref sphere_state_ + world/mood/gpu/time via MachineCtx;
// reconcile via SphereDeps. COHORT PROOF: sits AFTER
// contracts/spawn_services.hpp (generic_select/place/commit +
// run_spawn_preamble + negotiate_position DECLS — the machine bodies
// ride the cohort tail, Batch C); WorldState/MoodState complete upstream.
// clear_spheres keeps its deps-form GPUState& (m3 precedent). ═════════

inline void evict_sphere(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue) {
    self->sphere_state_.activeFloaters_[slot].active = false;  // sphere state owned by SphereState
    self->sphere_state_.activeFloaterCount_--;
    GPUFloatingEntityState empty{};
    self->gpuState_.upload_sphere_entity_slot(queue, slot, empty);
#ifdef DIAG_ENTITY_LIFECYCLE
    std::cout << "[DIAG:EVICT]   sph slot=" << slot << "\n";
#endif
}

// ═══ THE SPHERE RECIPE ════════════════════════════════════════════
//
// Tier tables, traits, adapter, and dispatch funnels — beside the
// evictor. Funnels declared above; the FAMILY_DISPATCH rows
// (cartridge.hpp, post-class) point here. THEMES is reached as THEMES
// (INTENT[services:themes] at its definition).

// ═══ FAMILY: SPHERE ═══════════════════════════════════════════════

struct SphIdx {
    static constexpr uint32_t BODY_RADIUS      = 0;
    static constexpr uint32_t ORBIT_RADIUS     = 1;
    static constexpr uint32_t ORBIT_HEIGHT     = 2;
    static constexpr uint32_t ORBIT_SPEED      = 3;
    static constexpr uint32_t INFLUENCE_RADIUS = 4;
    static constexpr uint32_t COUNT            = 5;
};

inline constexpr TierParamDef SPHERE_PARAM_DEFS[] = {
    { FloatingEntityProp::BODY_RADIUS,      0.5f,  1e30f, false, ParamDist::GAUSSIAN },
    { FloatingEntityProp::ORBIT_RADIUS,     0.0f,  1e30f, false, ParamDist::GAUSSIAN },
    { FloatingEntityProp::ORBIT_HEIGHT,     3.0f,  1e30f, false, ParamDist::GAUSSIAN },
    { FloatingEntityProp::ORBIT_SPEED,      0.05f, 1e30f, false, ParamDist::GAUSSIAN },
    { FloatingEntityProp::INFLUENCE_RADIUS, 3.0f,  1e30f, false, ParamDist::GAUSSIAN },
};
inline constexpr uint32_t SPHERE_PARAM_COUNT = sizeof(SPHERE_PARAM_DEFS) / sizeof(TierParamDef);
static_assert(SPHERE_PARAM_COUNT == SphIdx::COUNT,
    "F-4: SPHERE_PARAM_DEFS must cover SphIdx exactly (row order IS the index)");

// params[] order MUST match SPHERE_PARAM_DEFS:
//   [0]BODY_RADIUS [1]ORBIT_RADIUS [2]ORBIT_HEIGHT [3]ORBIT_SPEED
//   [4]INFLUENCE_RADIUS
//
// Note: the original SphereTierProfile carried a number of fields
// (spin_speed, bob_amplitude/period, spin_tilt, aspect_y/z, face_variance,
// geometry_type, motion_type) that the sphere adapter never reads —
// sphere_write_gpu hardcodes those slots in the GPU upload. We preserve
// them here verbatim per the migration spec ("preserve all numeric
// values"); dropping the dead fields would be a separate cleanup.
struct SphereTierRow {
    TierProfile profile;
    // Dead-but-preserved extras (not consumed by sphere adapter).
    float       spin_speed_mean,    spin_speed_sigma;
    float       bob_amplitude_mean, bob_amplitude_sigma;
    float       bob_period_mean,    bob_period_sigma;
    float       spin_tilt_sigma;
    float       aspect_y_mean,      aspect_y_sigma;
    float       aspect_z_mean,      aspect_z_sigma;
    float       face_variance_mean, face_variance_sigma;
    uint32_t    geometry_type;
    uint32_t    motion_type;
};

inline constexpr SphereTierRow SPHERE_TIERS[SPHERE_TIER_COUNT] = {
    /* 0: Sentinel */ {
        { 0.65f, 0.0f, { {1.5f, 0.3f}, {12.0f, 3.0f}, {6.0f, 2.0f}, {1.4f, 0.3f}, {8.0f, 2.0f} }},
        0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f,
        1.0f, 0.0f,  1.0f, 0.0f,  0.0f, 0.0f,  0, 0
    },
    /* 1: Anomaly  */ {
        { 0.35f, 0.0f, { {1.2f, 0.2f}, {8.0f, 2.0f},  {4.0f, 1.5f}, {2.0f, 0.5f}, {6.0f, 1.5f} }},
        0.0f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f,  0.0f,
        1.0f, 0.0f,  1.0f, 0.0f,  0.0f, 0.0f,  0, 0
    },
};

inline const TierProfile& sphere_get_tier_profile(uint32_t tier_idx) {
    return SPHERE_TIERS[tier_idx].profile;
}

inline constexpr EntityFamilyTraits SPHERE_TRAITS = {
    PopFamily::SPHERE, "sph", Dim::MAX_SPHERE_INSTANCES,
    false, false, 0,      // not grounded
    true,
    FloatingEntityProp::SPAWN_ROLL, SphereConfig::SPAWN_CHANCE,
    SphereConfig::MOOD_MULTIPLIER, SphereConfig::POSITION_JITTER,
    SPHERE_TIER_COUNT, FloatingEntityProp::TIER,
    SPHERE_PARAM_DEFS, SPHERE_PARAM_COUNT,
    FloatingEntityProp::ANCHOR_X, FloatingEntityProp::ANCHOR_Z, FloatingEntityProp::ROTATION, false,
    0, nullptr,
};

inline SpawnGateOutput sphere_run_gate(MachineCtx* c, int32_t gx, int32_t gz) {
    auto gate = run_spawn_preamble(c, gx, gz, c->sphere_state_.activeFloaters_, Dim::MAX_SPHERE_INSTANCES,
        FloatingEntityProp::SPAWN_ROLL, SphereConfig::SPAWN_CHANCE,
        SphereConfig::MOOD_MULTIPLIER, PopFamily::SPHERE, "sph");
    return { gate.ok, gate.seed, gate.slot, gate.theme_idx };
}

inline void sphere_compute_solid_half(EntityInstance& inst, const TierProfile&) {
    inst.solid_half = inst.params[SphIdx::BODY_RADIUS] + inst.params[SphIdx::ORBIT_RADIUS];
    inst.ground_y_offset = 0.0f;
    inst.burial = 0.0f;
}

inline void sphere_compute_colors(EntityInstance& inst, const EntityFamilyTraits&, const TierProfile& /*tier*/) {
    inst.colors[0] = cpu_hash_f(inst.seed, FloatingEntityProp::COLOR_R) * 0.55f + 0.35f;
    inst.colors[1] = cpu_hash_f(inst.seed, FloatingEntityProp::COLOR_G) * 0.50f + 0.30f;
    inst.colors[2] = cpu_hash_f(inst.seed, FloatingEntityProp::COLOR_B) * 0.60f + 0.20f;
}

inline void sphere_write_active(MachineCtx* c, const EntityInstance& inst) {
    auto& af = c->sphere_state_.activeFloaters_[inst.slot];
    af.patch_gx = inst.trigger_gx; af.patch_gz = inst.trigger_gz;
    af.host_gx = inst.host_gx; af.host_gz = inst.host_gz;
    af.last_alloc_time = c->time_state_.seconds;
    af.active = true;
    c->sphere_state_.activeFloaterCount_++;
}

inline void sphere_write_gpu(MachineCtx* c, const EntityInstance& inst, wgpu::Queue& queue) {
    GPUFloatingEntityState fe{};
    fe.anchor[0] = inst.cx; fe.anchor[1] = 0.0f; fe.anchor[2] = inst.cz;
    fe.body_radius = inst.params[SphIdx::BODY_RADIUS];
    fe.orbit_radius = inst.params[SphIdx::ORBIT_RADIUS];
    fe.orbit_height = inst.params[SphIdx::ORBIT_HEIGHT];
    fe.orbit_speed = inst.params[SphIdx::ORBIT_SPEED];
    fe.influence_radius = inst.params[SphIdx::INFLUENCE_RADIUS];
    fe.spin_speed = 0.0f; fe.bob_amplitude = 0.0f; fe.bob_period = 1.0f;
    fe.spin_tilt_x = 0.0f; fe.spin_tilt_z = 0.0f;
    fe.base_color[0] = inst.colors[0]; fe.base_color[1] = inst.colors[1]; fe.base_color[2] = inst.colors[2];
    fe.color[0] = inst.colors[0]; fe.color[1] = inst.colors[1]; fe.color[2] = inst.colors[2];
    fe.aspect_y = 1.0f; fe.aspect_z = 1.0f; fe.face_variance = 0.0f;
    fe.geometry_type = 0; fe.motion_type = 0;
    fe.entity_seed = inst.slot;
    fe.t = 0.0f; fe.orientation[3] = 1.0f;
    fe.pos[0] = inst.cx + fe.orbit_radius; fe.pos[1] = fe.orbit_height; fe.pos[2] = inst.cz;
    fe.is_active = 1;
    c->gpuState_.upload_sphere_entity_slot(queue, inst.slot, fe);
}

inline constexpr EntityFamilyAdapter SPHERE_ADAPTER = {
    sphere_run_gate,
    nullptr,                              // apply_indoor_rescale → not eligible (floaters, not grounded)
    sphere_compute_solid_half, sphere_compute_colors,
    sphere_write_active, sphere_write_gpu, nullptr,
    sphere_get_tier_profile,
};

inline bool dispatch_select_sphere_generic(MachineCtx* self, int32_t gx, int32_t gz, EntityQueueEntry& e) {
    EntityInstance inst{};
    if (!generic_select(self, SPHERE_TRAITS, SPHERE_ADAPTER, gx, gz, inst)) return false;
    e.family = PopFamily::SPHERE; e.gx = gx; e.gz = gz; e.generic = inst; return true;
}
inline bool dispatch_place_sphere_generic(MachineCtx* self, EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (generic_place(self, SPHERE_TRAITS, e.generic)) { pe.generic = e.generic; return true; }
    self->sphere_state_.activeFloaters_[e.generic.slot].active = false; return false;
}
inline void dispatch_commit_sphere_generic(MachineCtx* self, PlacementEntry& pe, wgpu::Queue& queue) {
    auto* host = find_patch(self, pe.generic.host_gx, pe.generic.host_gz);
    if (host) {
        generic_commit(self, SPHERE_TRAITS, SPHERE_ADAPTER, pe.generic, queue);
        // Lifecycle Phase 2: sphere lifetime is no longer tied to its
        // host patch. We don't call host->record_entity() here, so
        // evict_patch_entities will never evict_sphere on this
        // slot — the GPU-side pawn-distance test in update_sphere is
        // the sole eviction path. The find_patch() lookup is retained
        // because a missing host still means "spawn was invalid"; we
        // just don't link the sphere into the patch's eviction list.
        //
    }
    else { self->sphere_state_.activeFloaters_[pe.generic.slot].active = false; }
}


// ─── Readback mirror reconciliation (owner verb; REBUILD-0 m2 —
// stray (1) comes home) ─ the sphere half of the floater-readback
// funnel: release CPU mirror slots the GPU deactivated, honoring the
// spawn-protection window (SPAWN_PROTECTION_S, floater_vocabulary.hpp).
inline void reconcile_sphere_mirror(SphereState& ss, SphereDeps* c, const GPUFloatingEntityState* data) {
    float now = c->time_state_.seconds;
    // Spheres: slots [0, MAX_SPHERE_INSTANCES)
    for (uint32_t i = 0; i < Dim::MAX_SPHERE_INSTANCES; i++) {
        bool gpu_active = (data[i].is_active != 0u);
        // sphere active-slot mirror owned by SphereState (spheres.hpp)
        if (ss.activeFloaters_[i].active && !gpu_active &&
            (now - ss.activeFloaters_[i].last_alloc_time) > SPAWN_PROTECTION_S) {
            ss.activeFloaters_[i].active = false;
            if (ss.activeFloaterCount_ > 0) ss.activeFloaterCount_--;
        }
    }
}

} // namespace the_board
} // namespace t7