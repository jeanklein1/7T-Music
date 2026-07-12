// ─── spheres.inl (IMPL: post-class definitions) ──────────────────
// Born at LADDER-5 e3: history in audit/LADDER.md.
//
// The Sphere family's lifecycle: the evictor (absorbed per §5
// EVICTION THUNKS) and the full recipe (tier tables, traits, adapter,
// dispatch funnels), all named by the FAMILY_DISPATCH table
// (family_dispatch.inl). SphereState itself is spheres.hpp.
//
// WRAPPING FORM (the proven fix-2 rule): SELF-WRAPPING — opens
// t7::the_board itself, carries its own standard includes; the MODULE
// IMPLEMENTATIONS zone includes it at FILE SCOPE. Definitions are
// `inline` free functions.

#include <iostream>   // [DIAG:EVICT] logging (flag-gated)

namespace t7 {
namespace the_board {

inline void evict_sphere(Cartridge* self,
    uint32_t slot, wgpu::Queue& queue) {
    self->sphere_state_.activeFloaters_[slot].active = false;  // sphere state owned by SphereState
    self->sphere_state_.activeFloaterCount_--;
    GPUFloatingEntityState empty{};
    self->gpuState_.upload_sphere_entity_slot(queue, slot, empty);
#ifdef DIAG_ENTITY_LIFECYCLE
    std::cout << "[DIAG:EVICT]   sph slot=" << slot << "\n";
#endif
}


// ═══ THE SPHERE RECIPE (relocated from entity_pipeline.inl) ═══════
//
// Tier tables, traits, adapter, and dispatch funnels — beside the
// evictor. Funnels declared in spheres.hpp; table rows point here
// (family_dispatch.inl). THEMES is reached as Cartridge::THEMES
// (INTENT[services:themes] at its definition).

// ═══ FAMILY: SPHERE ═══════════════════════════════════════════════
//
// Orbital floating entity. No ground contact.
//

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
    true, 200.0f, 0.0f,
    FloatingEntityProp::SPAWN_ROLL, SphereConfig::SPAWN_CHANCE,
    SphereConfig::MOOD_MULTIPLIER, SphereConfig::POSITION_JITTER,
    SPHERE_TIER_COUNT, FloatingEntityProp::TIER,
    SPHERE_PARAM_DEFS, SPHERE_PARAM_COUNT,
    FloatingEntityProp::ANCHOR_X, FloatingEntityProp::ANCHOR_Z, FloatingEntityProp::ROTATION, false,
    0, nullptr,
};

inline SpawnGateOutput sphere_run_gate(Cartridge* c, int32_t gx, int32_t gz) {
    auto gate = c->run_spawn_preamble(gx, gz, c->sphere_state_.activeFloaters_, Dim::MAX_SPHERE_INSTANCES,
        FloatingEntityProp::SPAWN_ROLL, SphereConfig::SPAWN_CHANCE,
        SphereConfig::MOOD_MULTIPLIER, PopFamily::SPHERE, "sph");
    return { gate.ok, gate.seed, gate.slot, gate.theme_idx };
}
inline const float* sphere_get_theme_tier_weights(uint32_t ti) { return Cartridge::THEMES[ti].tier_wt_sphere; }

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

inline void sphere_write_active(Cartridge* c, const EntityInstance& inst) {
    auto& af = c->sphere_state_.activeFloaters_[inst.slot];
    af.patch_gx = inst.trigger_gx; af.patch_gz = inst.trigger_gz;
    af.host_gx = inst.host_gx; af.host_gz = inst.host_gz;
    af.last_alloc_time = c->time_state_.seconds;
    af.active = true;
    c->sphere_state_.activeFloaterCount_++;
}

inline void sphere_write_gpu(Cartridge* c, const EntityInstance& inst, wgpu::Queue& queue) {
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
    sphere_run_gate, sphere_get_theme_tier_weights,
    nullptr,                              // apply_indoor_rescale → not eligible (floaters, not grounded)
    sphere_compute_solid_half, sphere_compute_colors,
    sphere_write_active, sphere_write_gpu, nullptr,
    sphere_get_tier_profile,
};

inline bool dispatch_select_sphere_generic(Cartridge* self, int32_t gx, int32_t gz, EntityQueueEntry& e) {
    EntityInstance inst{};
    if (!self->generic_select(SPHERE_TRAITS, SPHERE_ADAPTER, gx, gz, inst)) return false;
    e.family = PopFamily::SPHERE; e.gx = gx; e.gz = gz; e.generic = inst; return true;
}
inline bool dispatch_place_sphere_generic(Cartridge* self, EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (self->generic_place(SPHERE_TRAITS, e.generic)) { pe.generic = e.generic; return true; }
    self->sphere_state_.activeFloaters_[e.generic.slot].active = false; return false;
}
inline void dispatch_commit_sphere_generic(Cartridge* self, PlacementEntry& pe, wgpu::Queue& queue) {
    auto* host = self->find_patch(pe.generic.host_gx, pe.generic.host_gz);
    if (host) {
        self->generic_commit(SPHERE_TRAITS, SPHERE_ADAPTER, pe.generic, queue);
        // Lifecycle Phase 2: sphere lifetime is no longer tied to its
        // host patch. We don't call host->record_entity() here, so
        // evict_patch_entities will never evict_sphere on this
        // slot — the GPU-side pawn-distance test in update_sphere is
        // the sole eviction path. The find_patch() lookup is retained
        // because a missing host still means "spawn was invalid"; we
        // just don't link the sphere into the patch's eviction list.
        //
        // CPU's activeFloaters_[slot].active stays true until the next
        // mood transition zeroes the buffer. With 8 slots and a 1.5%
        // spawn chance, allocator pressure from stale CPU bools is
        // unlikely in practice; if it surfaces, add a continuous readback
        // mirroring agent_state_readback_staging (cartridge.hpp ~7990).
    }
    else { self->sphere_state_.activeFloaters_[pe.generic.slot].active = false; }
}

} // namespace the_board
} // namespace t7
