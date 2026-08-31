#pragma once
#include <cstdint>
#include <cmath>      // std::round, std::min/max, std::cos/sin/abs   // (impl, merged)
#include <cstring>    // std::memcpy (drum colors)   // (impl, merged)
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)

// ─── entity_pipeline.hpp (S3 · MERGED: the pipeline law + impl) ─────
//
// Generic entity lifecycle for the cookie-cutter families: the
// three-phase verbs (select → place → commit) driven per family by
// traits + adapter rows; the welded pair's blocks (pyramid, arch)
// live in the impl. The module owns no state — the
// queues it fills are spawn_engine's.
//
// SEAM[entity_pipeline:K1] tier sampling profile + extras live as a
//   single per-family TierRow struct in the impl. Single source of
//   truth — no converters, no derived tables.
//
// Depends on cohort include order: entity_types.hpp (traits/adapter/
// instance vocabulary), grounded.hpp (props/configs/palettes + the
// tier enums — COMPLETE, the merged bodies deref them), state.hpp
// (GPU mesh params), mood.hpp (MOOD_TABLE / portal
// doors), machine/spawn_engine.hpp (the services, defined just above
// in the cohort). MERGED at the cohort tail (the
// B ruling): the decl tier (the generic_* decls, the arch
// vocabulary) lives in contracts/spawn_services.hpp.

namespace t7 {
namespace the_board {

// ─── Indoor Sizing (THE INDOOR MODULE's per-family hooks) ────────
//
// Policy rides INDOOR_TREATMENT + the dials
// (contracts/indoor_module.hpp); the families below keep only their
// hand-curated param-index lists — only LENGTH dimensions get
// scaled, never ratios (TAPER, ENTASIS, ASPECT...), counts
// (BASE_LAYERS, RIBS, ARM_COUNT...), or angles (LEAN_DIR,
// FROND_DROOP...). CAP families call the shared cap_to_ceiling law.
// (EXACT — snap HEIGHT to wall_height and scale every other length
// param by the same ratio so proportions hold — left with the column,
// its only practitioner; IndoorSize::EXACT stands unclaimed.)
// Adding a new eligible family means declaring its own
// <family>_apply_indoor_rescale (a cap_to_ceiling call + its list),
// registering it in the adapter, and rowing INDOOR_TREATMENT.
//
// SEAM[entity_pipeline:rescale-per-family] DONE — was a free-function
//   switch on family_id; lifted to per-family adapter slot during
//   Pass 7 of the modularity rollout. The rolled-band helper
//   (rescale_to_rolled_target: target in [lo,hi]×ceiling, property
//   index 7777u for the roll) lost its callers to the module's cap
//   law — held by git.

// ═══ MODULE FUNCTIONS ══════════════════════════════════════════════
//
// DECLARATIONS live in contracts/spawn_services.hpp (the
// machine's decl tier) with the arch vocabulary
// (ArchIdx / ArchTierRow / ARCH_TIERS) and the rescale decl. The
// definitions — the rescale template above, the three-phase verbs and
// the welded four below — all live HERE.


// ═══ MODULE IMPLEMENTATION ════════════════════════════════════════
//
// The three-phase verbs and the welded family blocks (pyramid, arch —
// the families that weld to the ground/regen services). Each block
// keeps the same 10-element template. Reaches
// the machine face for c->mood_state_ / c->world_state_ /
// c->entities_state_ and the GPU wire (c->gpuState_); routes through
// the spawn services (run_spawn_preamble / negotiate_position /
// mark_patches_for_regen) as free calls.


// ═══ GENERIC HELPERS ═════════════════════════════════════════════

// THE SPREAD LAW's one arithmetic (MOSAIC_2d) — median + jitter·spread,
// clamped. One home for both paths (palette and sandstone), so "varied"
// has a single definition. It sat above its first consumer, the column's
// color derivation; PRUNE_2 excised that family and the arch is now the
// only consumer, so the law comes up here to the generic helpers where a
// shared arithmetic belongs and where C++ still sees it first.
inline float entity_spread(uint32_t seed, uint32_t prop) {
    return ENTITY_SPREAD_BASE + cpu_hash_f(seed, prop) * ENTITY_SPREAD_SPAN;
}
inline float entity_tint(float median, uint32_t seed, uint32_t prop, float spread) {
    const float v = median + (cpu_hash_f(seed, prop) - 0.5f) * 2.0f * spread;
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

// ─── Generic Color Derivation ────────────────────────────────────

inline void generic_compute_colors(EntityInstance& inst,
    const EntityFamilyTraits& traits,
    const TierProfile& tier) {
    for (uint32_t p = 0; p < traits.color_part_count; p++) {
        const auto& part = traits.color_parts[p];
        float v = (tier.color_var > 0.0f) ? tier.color_var : part.variance;
        uint32_t ci = p * 3;
        inst.colors[ci + 0] = part.base[0]
            + (cpu_hash_f(inst.seed, part.prop_base + part.prop_offset + 0) - 0.5f) * v;
        inst.colors[ci + 1] = part.base[1]
            + (cpu_hash_f(inst.seed, part.prop_base + part.prop_offset + 1) - 0.5f) * v;
        inst.colors[ci + 2] = part.base[2]
            + (cpu_hash_f(inst.seed, part.prop_base + part.prop_offset + 2) - 0.5f) * v;
    }
}

// ═══ GENERIC THREE-PHASE PIPELINE ════════════════════════════════

// ─── Generic Select ──────────────────────────────────────────────

inline bool generic_select(MachineCtx* c,
    const EntityFamilyTraits& traits,
    const EntityFamilyAdapter& adapter,
    int32_t gx, int32_t gz,
    EntityInstance& inst)
{
    // ── Spawn gate (delegates to existing run_spawn_preamble) ──
    auto gate = adapter.run_gate(c, gx, gz);
    if (!gate.ok) return false;

    // ── Tier selection with theme bias ──
    // Tier weights and profiles come from the adapter's per-family
    // accessor; there's no generic table on traits to index.
    float weights[8]{};
    for (uint32_t t = 0; t < traits.tier_count && t < 8; t++)
        weights[t] = adapter.get_tier_profile(t).weight;

    // Apply theme tier weights (per-family array from PopulationTheme)
    const float* theme_tw = theme_tier_weights(gate.theme_idx, traits.family_id);
    for (uint32_t t = 0; t < traits.tier_count && t < 8; t++)
        weights[t] *= theme_tw[t];

    uint32_t tier = select_tier(gate.seed, traits.tier_prop,
        weights, traits.tier_count);
    const auto& profile = adapter.get_tier_profile(tier);

    // ── Sample all parameters from tier profile ──
    for (uint32_t i = 0; i < traits.param_count; i++) {
        const auto& pd = traits.param_defs[i];
        float val = 0.0f;

        switch (pd.dist) {
        case ParamDist::GAUSSIAN:
            val = cpu_sample_gaussian(gate.seed, pd.prop,
                profile.params[i].mean, profile.params[i].sigma);
            break;
        case ParamDist::UNIFORM_01:
            val = cpu_hash_f(gate.seed, pd.prop);
            break;
        case ParamDist::UNIFORM_TAU:
            val = cpu_hash_f(gate.seed, pd.prop) * 6.283185307f;
            break;
        }

        if (pd.do_round) val = std::round(val);
        val = std::max(pd.floor, val);
        if (pd.ceiling < 1e29f) val = std::min(pd.ceiling, val);
        inst.params[i] = val;
    }

    // ── Populate instance header ──
    inst.family_id  = traits.family_id;
    inst.seed       = gate.seed;
    inst.trigger_gx = gx;
    inst.trigger_gz = gz;
    inst.slot       = gate.slot;
    inst.tier_idx   = tier;
    inst.theme_idx  = gate.theme_idx;

    // ── Indoor sizing (must run before compute_solid_half so the
    //    solid extents are derived from the scaled params). THE
    //    INDOOR MODULE dispatches on its policy table: NATURAL
    //    skips; EXACT and CAP run the family's adapter hook. ──
    if (mood_def(c->mood_state_.active).shape.indoor
        && INDOOR_TREATMENT[traits.family_id].size != IndoorSize::NATURAL
        && adapter.apply_indoor_rescale) {
        adapter.apply_indoor_rescale(inst, mood_def(c->mood_state_.active).shape.wall_height);
    }

    // ── Per-family derived values ──
    adapter.compute_solid_half(inst, profile);
    if (adapter.compute_colors)
        adapter.compute_colors(inst, traits, profile);
    else
        generic_compute_colors(inst, traits, profile);

    return true;
}

// ─── Generic Place ───────────────────────────────────────────────

inline bool generic_place(MachineCtx* c,
    const EntityFamilyTraits& traits,
    EntityInstance& inst)
{
    auto pos = negotiate_position(c, inst.seed,
        inst.trigger_gx, inst.trigger_gz,
        traits.pos_x_prop, traits.pos_z_prop,
        traits.position_jitter,
        traits.rotation_prop,
        traits.grounded,   // ruling 21: the ground-claim policy, from the family's own record
        inst.solid_half, /*containment_r*/ inst.solid_half, traits.family_id, inst.slot, inst.tier_idx);
    if (!pos.ok) return false;

    inst.host_gx  = pos.host_gx;
    inst.host_gz  = pos.host_gz;
    inst.cx       = pos.cx;
    inst.cz       = pos.cz;
    inst.rotation = pos.rotation;

    inst.cached_ground_y = 0.0f;

    return true;
}

// ─── Generic Commit ──────────────────────────────────────────────

inline void generic_commit(MachineCtx* c,
    const EntityFamilyTraits& traits,
    const EntityFamilyAdapter& adapter,
    const EntityInstance& inst,
    wgpu::Queue& queue)
{
    // ── Active tracking (per-family array) ──
    adapter.write_active(c, inst);

    // ── GPU mesh params (per-family struct mapping) ──
    adapter.write_gpu(c, inst, queue);

    // ── Post-commit: the pyramid's heightfield regen (sole tenant) ──
    if (adapter.post_commit)
        adapter.post_commit(c, inst, queue);

    c->world_state_.ground_entries_dirty = true;
}


// ═══ FAMILY: PYRAMID ══════════════════════════════════════════════

struct PyrIdx {
    static constexpr uint32_t HEIGHT     = 0;
    static constexpr uint32_t BASE_HALF  = 1;
    static constexpr uint32_t ASPECT     = 2;
    static constexpr uint32_t TRUNCATION = 3;
    static constexpr uint32_t EDGE_BLEND = 4;
    static constexpr uint32_t COUNT      = 5;
};

inline constexpr TierParamDef PYRAMID_PARAM_DEFS[] = {
    { PyramidProp::HEIGHT,     20.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { PyramidProp::BASE_HALF,  5.0f,  1e30f, false, ParamDist::GAUSSIAN },
    { PyramidProp::ASPECT,     0.5f,  2.0f,  false, ParamDist::GAUSSIAN },  // ceiling 2.0
    { PyramidProp::TRUNCATION, 0.0f,  0.5f,  false, ParamDist::GAUSSIAN },  // ceiling 0.5
    { PyramidProp::EDGE_BLEND, 0.5f,  1e30f, false, ParamDist::GAUSSIAN },
};
inline constexpr uint32_t PYRAMID_PARAM_COUNT = sizeof(PYRAMID_PARAM_DEFS) / sizeof(TierParamDef);
static_assert(PYRAMID_PARAM_COUNT == PyrIdx::COUNT,
    "F-4: PYRAMID_PARAM_DEFS must cover PyrIdx exactly (row order IS the index)");

// params[] order MUST match PYRAMID_PARAM_DEFS:
//   [0]HEIGHT [1]BASE_HALF [2]ASPECT [3]TRUNCATION [4]EDGE_BLEND
// Note: cannot reuse `PyramidTier` as the struct name — grounded.hpp
// declares `enum class PyramidTier`, which occupies the same name slot.
// `PyramidTierRow` keeps the new struct distinct without renaming the
// enum (used widely as PyramidTier::OBELISK etc.).
struct PyramidTierRow {
    TierProfile profile;
    float color_override;
    float color_variance;
};

// ── Pyramid tier table ─────────────────────────────────────────────
// WHAT: per-tier pyramid recipe (the pyramid's realization IS the
//   terrain — these numbers shape the heightfield bake).
// AXES: row = PyramidTier order (0 OBELISK / 1 TEMPLE / 2 COLOSSUS);
//   the 5 {μ,σ} pairs are in PyrIdx order:
//     HEIGHT  BASE_HALF  ASPECT  TRUNCATION  EDGE_BLEND
//   then: color_override, color_variance.
// UNITS: HEIGHT/BASE_HALF/EDGE_BLEND = wu; ASPECT = multiplier
//   (ceiling 2.0); TRUNCATION = fraction (ceiling 0.5); weight =
//   tier-selection weight.
// CONSUMERS: pyramid_get_tier_profile (weight + {μ,σ} sampling).
//   color_override/color_variance are retained but unread today —
//   see pyramid_compute_colors' dead-code note.
// Biography determinant — frozen biography (§12).
inline constexpr PyramidTierRow PYRAMID_TIERS[] = {
    /* OBELISK  */ {
        { 0.50f, 0.0f, { {28.0f, 6.0f},  {16.0f, 3.0f},  {1.0f, 0.15f}, {0.00f, 0.00f}, {1.5f, 0.3f}  }},
        0.10f, 0.04f
    },
    /* TEMPLE   */ {
        { 0.25f, 0.0f, { {45.0f, 8.0f},  {40.0f, 6.0f},  {1.0f, 0.20f}, {0.25f, 0.08f}, {3.0f, 0.75f} }},
        0.15f, 0.04f
    },
    /* COLOSSUS */ {
        { 0.25f, 0.0f, { {78.0f, 14.4f}, {60.0f, 9.6f},  {1.0f, 0.10f}, {0.05f, 0.04f}, {3.6f, 1.0f}  }},
        0.20f, 0.04f
    },
};
static_assert(sizeof(PYRAMID_TIERS) / sizeof(PyramidTierRow) == static_cast<uint32_t>(PyramidTier::COUNT),
    "F-5: PYRAMID_TIERS must have exactly one row per PyramidTier");

inline const TierProfile& pyramid_get_tier_profile(uint32_t tier_idx) {
    return PYRAMID_TIERS[tier_idx].profile;
}

inline constexpr EntityFamilyTraits PYRAMID_TRAITS = {
    PopFamily::PYRAMID, Dim::MAX_PYRAMID_INSTANCES,
    true,                 // grounded — bakes into the heightfield
    PyramidProp::SPAWN_ROLL, PyramidConfig::SPAWN_CHANCE,
    mood_mult_for(PopFamily::PYRAMID), PyramidConfig::POSITION_JITTER,
    3, PyramidProp::TIER,
    PYRAMID_PARAM_DEFS, PYRAMID_PARAM_COUNT,
    PyramidProp::POSITION_X, PyramidProp::POSITION_Z, PyramidProp::ROTATION,
    0, nullptr,  // color handled by adapter
};

inline SpawnGateOutput pyramid_run_gate(MachineCtx* c, int32_t gx, int32_t gz) {
    return gate_from_traits(c, gx, gz, PYRAMID_TRAITS, c->entities_state_.pyramids);
}

inline constexpr uint32_t PYRAMID_INDOOR_RESCALE_PARAMS[] = {
    PyrIdx::HEIGHT, PyrIdx::BASE_HALF, PyrIdx::EDGE_BLEND,
    // ASPECT, TRUNCATION are ratios — not scaled.
};

inline void pyramid_apply_indoor_rescale(EntityInstance& inst, float ceiling_h) {
    cap_to_ceiling(inst, ceiling_h, INDOOR_LIVE.height_cap_fraction,
        /*current_h*/ inst.params[PyrIdx::HEIGHT],
        PYRAMID_INDOOR_RESCALE_PARAMS);
}

inline void pyramid_compute_solid_half(EntityInstance& inst, const TierProfile&) {
    float base_half = inst.params[PyrIdx::BASE_HALF];
    float aspect    = inst.params[PyrIdx::ASPECT];
    float half_x = base_half;
    float half_z = base_half * aspect;

    // Proportion constraint: height ≤ 1.5 × longest base side
    float max_h = 1.5f * 2.0f * std::max(half_x, half_z);
    inst.params[PyrIdx::HEIGHT] = std::min(inst.params[PyrIdx::HEIGHT], max_h);

    inst.solid_half = std::max(half_x, half_z) + inst.params[PyrIdx::EDGE_BLEND];
    inst.burial = 0.0f;
    inst.ground_y_offset = 0.0f;
}

inline void pyramid_compute_colors(EntityInstance& inst, const EntityFamilyTraits&, const TierProfile& /*tier*/) {
    // Sandstone only (color_override check is dead code in legacy — both branches identical)
    inst.colors[0] = PYRAMID_SANDSTONE_BASE[0] + (cpu_hash_f(inst.seed, PyramidProp::COLOR_VAR_R) - 0.5f) * PYRAMID_SANDSTONE_VARIANCE * 2.0f;
    inst.colors[1] = PYRAMID_SANDSTONE_BASE[1] + (cpu_hash_f(inst.seed, PyramidProp::COLOR_VAR_G) - 0.5f) * PYRAMID_SANDSTONE_VARIANCE * 2.0f;
    inst.colors[2] = PYRAMID_SANDSTONE_BASE[2] + (cpu_hash_f(inst.seed, PyramidProp::COLOR_VAR_B) - 0.5f) * PYRAMID_SANDSTONE_VARIANCE * 2.0f;
}

inline void pyramid_write_active(MachineCtx* c, const EntityInstance& inst) {
    auto& ap = c->entities_state_.pyramids[inst.slot];
    ap.patch_gx = inst.trigger_gx; ap.patch_gz = inst.trigger_gz;
    ap.host_gx = inst.host_gx; ap.host_gz = inst.host_gz;
    ap.active = true;
    ap.col_r = inst.colors[0]; ap.col_g = inst.colors[1]; ap.col_b = inst.colors[2];

    // 5-point ground_y: GPU compute shader evaluates the heightfield
    // at center + 4 rotated corners and takes the min. CPU uploads 0.
    ap.cached_ground_y = 0.0f;
}

inline void pyramid_write_gpu(MachineCtx* c, const EntityInstance& inst, wgpu::Queue& queue) {
    float half_x = inst.params[PyrIdx::BASE_HALF];
    float half_z = inst.params[PyrIdx::BASE_HALF] * inst.params[PyrIdx::ASPECT];

    // Write GPU solid instance (heightfield baking)
    GPUPyramidInstance gpu_inst{};
    gpu_inst.origin[0] = inst.cx;
    gpu_inst.origin[1] = inst.cz;
    gpu_inst.half_size[0] = half_x;
    gpu_inst.half_size[1] = half_z;
    gpu_inst.height = inst.params[PyrIdx::HEIGHT];
    gpu_inst.rotation = inst.rotation;
    gpu_inst.edge_blend = inst.params[PyrIdx::EDGE_BLEND];
    gpu_inst.truncation = inst.params[PyrIdx::TRUNCATION];
    c->entities_state_.cpu_pyramids.instances[inst.slot] = gpu_inst;

    uint32_t max_idx = 0;
    for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) {
        if (i == inst.slot || c->entities_state_.pyramids[i].active) max_idx = i + 1;
    }
    c->entities_state_.cpu_pyramids.count = max_idx;
    c->gpuState_.upload_pyramids(queue, c->entities_state_.cpu_pyramids);
}

inline void pyramid_post_commit(MachineCtx* c, const EntityInstance& inst, wgpu::Queue&) {
    // Mark heightfield patches for regen (pyramid gets baked in)
    float half_x = inst.params[PyrIdx::BASE_HALF];
    float half_z = inst.params[PyrIdx::BASE_HALF] * inst.params[PyrIdx::ASPECT];
    float blend  = inst.params[PyrIdx::EDGE_BLEND];
    float cr = std::cos(inst.rotation), sr = std::sin(inst.rotation);
    float abs_cr = std::abs(cr), abs_sr = std::abs(sr);
    float ext_x = (half_x + blend) * abs_cr + (half_z + blend) * abs_sr;
    float ext_z = (half_x + blend) * abs_sr + (half_z + blend) * abs_cr;
    mark_patches_for_regen(c, 
        inst.cx - ext_x, inst.cz - ext_z,
        inst.cx + ext_x, inst.cz + ext_z,
        inst.host_gx, inst.host_gz);
}

inline constexpr EntityFamilyAdapter PYRAMID_ADAPTER = {
    pyramid_run_gate,
    pyramid_apply_indoor_rescale,
    pyramid_compute_solid_half, pyramid_compute_colors,
    pyramid_write_active, pyramid_write_gpu, pyramid_post_commit,
    pyramid_get_tier_profile,
};

// ── Pyramid dispatch wrappers ──

inline bool dispatch_select_pyramid_generic(MachineCtx* self, int32_t gx, int32_t gz, EntityQueueEntry& e) {
    EntityInstance inst{};
    if (!generic_select(self, PYRAMID_TRAITS, PYRAMID_ADAPTER, gx, gz, inst)) return false;
    e.family = PopFamily::PYRAMID; e.gx = gx; e.gz = gz; e.generic = inst; return true;
}
inline bool dispatch_place_pyramid_generic(MachineCtx* self, EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (generic_place(self, PYRAMID_TRAITS, e.generic)) { pe.generic = e.generic; return true; }
    self->entities_state_.pyramids[e.generic.slot].active = false; return false;
}
inline void dispatch_commit_pyramid_generic(MachineCtx* self, PlacementEntry& pe, wgpu::Queue& queue) {
    auto* host = find_patch(self, pe.generic.host_gx, pe.generic.host_gz);
    if (host) { generic_commit(self, PYRAMID_TRAITS, PYRAMID_ADAPTER, pe.generic, queue); host->record_entity(PopFamily::PYRAMID, pe.generic.slot); }
    // HOST PATCH GONE. The footprint was registered at place; its host
    // vanished before commit. Release by OWNER — the one release path.
    else { unregister_footprint_for(self, PopFamily::PYRAMID, pe.generic.slot); self->entities_state_.pyramids[pe.generic.slot].active = false; }
}

// ═══ FAMILIES: SPHERE / CUBE — RELOCATED ═════════════════════════

// ═══ FAMILY: ARCH ═════════════════════════════════════════════════


// Floor for SPAN is 1.0 (raw span); compute_solid_half halves it → half_span ≥ 0.5
inline constexpr TierParamDef ARCH_PARAM_DEFS[] = {
    { ArchProp::SPAN,         1.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { ArchProp::RISE,         1.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { ArchProp::DEPTH,        0.5f, 1e30f, false, ParamDist::GAUSSIAN },
    { ArchProp::THICKNESS,    0.1f, 1e30f, false, ParamDist::GAUSSIAN },
    { ArchProp::PIER_HEIGHT,  0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { ArchProp::PIER_PADDING, 0.1f, 1e30f, false, ParamDist::GAUSSIAN },
    { ArchProp::EDGE_BLEND,   0.1f, 1e30f, false, ParamDist::GAUSSIAN },
};
inline constexpr uint32_t ARCH_PARAM_COUNT = sizeof(ARCH_PARAM_DEFS) / sizeof(TierParamDef);
static_assert(ARCH_PARAM_COUNT == ArchIdx::COUNT,
    "F-4: ARCH_PARAM_DEFS must cover ArchIdx exactly (row order IS the index)");
static_assert(sizeof(ARCH_TIERS) / sizeof(ArchTierRow) == static_cast<uint32_t>(ArchTier::COUNT),
    "F-5: ARCH_TIERS must have exactly one row per ArchTier");

inline const TierProfile& arch_get_tier_profile(uint32_t tier_idx) {
    return ARCH_TIERS[tier_idx].profile;
}

inline constexpr EntityFamilyTraits ARCH_TRAITS = {
    PopFamily::ARCH, Dim::MAX_ARCH_INSTANCES,
    true,                 // grounded
    ArchProp::SPAWN_ROLL, ArchConfig::SPAWN_CHANCE,
    mood_mult_for(PopFamily::ARCH), ArchConfig::POSITION_JITTER,
    static_cast<uint32_t>(ArchTier::COUNT), ArchProp::TIER,
    ARCH_PARAM_DEFS, ARCH_PARAM_COUNT,
    ArchProp::POSITION_X, ArchProp::POSITION_Z, ArchProp::ROTATION,
    0, nullptr,
};

inline SpawnGateOutput arch_run_gate(MachineCtx* c, int32_t gx, int32_t gz) {
    return gate_from_traits(c, gx, gz, ARCH_TRAITS, c->entities_state_.arches);
}

inline constexpr uint32_t ARCH_INDOOR_RESCALE_PARAMS[] = {
    ArchIdx::SPAN, ArchIdx::RISE, ArchIdx::DEPTH, ArchIdx::THICKNESS,
    ArchIdx::PIER_HEIGHT, ArchIdx::PIER_PADDING, ArchIdx::EDGE_BLEND,
};

// Arch total height = pier_height + rise (catenary apex). CAP at
// the module's fraction, scale every length param.
inline void arch_apply_indoor_rescale(EntityInstance& inst, float ceiling_h) {
    cap_to_ceiling(inst, ceiling_h, INDOOR_LIVE.height_cap_fraction,
        /*current_h*/ inst.params[ArchIdx::PIER_HEIGHT] + inst.params[ArchIdx::RISE],
        ARCH_INDOOR_RESCALE_PARAMS);
}

inline void arch_compute_solid_half(EntityInstance& inst, const TierProfile&) {
    float half_span    = inst.params[ArchIdx::SPAN] * 0.5f;
    inst.params[ArchIdx::SPAN] = half_span;  // overwrite: SPAN now holds half_span
    float thickness    = inst.params[ArchIdx::THICKNESS];
    float depth        = inst.params[ArchIdx::DEPTH];
    float pier_padding = inst.params[ArchIdx::PIER_PADDING];
    float edge_blend   = inst.params[ArchIdx::EDGE_BLEND];
    float pier_height  = inst.params[ArchIdx::PIER_HEIGHT];

    float pier_half_x = thickness * 0.5f + pier_padding + edge_blend;
    float pier_half_z = depth * 0.5f + pier_padding + edge_blend;
    inst.solid_half = half_span + std::max(pier_half_x, pier_half_z);
    inst.ground_y_offset = pier_height;
    inst.burial = std::max(0.2f, pier_height * ARCH_TIERS[inst.tier_idx].burial);
}

inline void arch_compute_colors(EntityInstance& inst, const EntityFamilyTraits&, const TierProfile& /*tier*/) {
    const auto& tp = ARCH_TIERS[inst.tier_idx];
    // THE MOSAIC ROLL (MOSAIC_2) — a TIER fact, not a sub-roll of the
    // palette. Orthogonal to color_over by ruling: what a body's
    // fallback color is, and whether it is ceramic, are two facts, and
    // nesting them capped the mosaic at 85% for no reason. A painted
    // body never reads its fallback at any range (the far field is the
    // passage median at variance zero), so the fallback below is now
    // written for the PLAIN population alone.
    if (cpu_hash_f(inst.seed, ArchProp::MOSAIC_ROLL) < tp.mosaic_chance) {
        inst.mosaic_seed = 1u + (uint32_t)(cpu_hash_f(inst.seed, ArchProp::MOSAIC_SEED) * 65534.0f);
    }
    // THE SPREAD (MOSAIC_2d): rolled once per body, before the branch —
    // how far THIS arch sits from whichever median it lands on. Both
    // paths read it, so "varied" is one definition.
    const float spread = entity_spread(inst.seed, ArchProp::COLOR_SPREAD);
    // Base color: sandstone/palette — THE PLAIN POPULATION's scheme. A
    // mosaic arch never reads this at any range (MOSAIC_2c).
    if (cpu_hash_f(inst.seed, ArchProp::COLOR_OVER) < tp.color_override) {
        uint32_t pal_idx = cpu_hash(inst.seed, ArchProp::COLOR_OVER + 1u) % ARCH_PALETTE_COUNT;
        inst.colors[0] = entity_tint(ARCH_PALETTE[pal_idx][0], inst.seed, ArchProp::COLOR_VAR_R, spread);
        inst.colors[1] = entity_tint(ARCH_PALETTE[pal_idx][1], inst.seed, ArchProp::COLOR_VAR_G, spread);
        inst.colors[2] = entity_tint(ARCH_PALETTE[pal_idx][2], inst.seed, ArchProp::COLOR_VAR_B, spread);
    } else {
        inst.colors[0] = entity_tint(ARCH_SANDSTONE_BASE[0], inst.seed, ArchProp::COLOR_VAR_R, spread);
        inst.colors[1] = entity_tint(ARCH_SANDSTONE_BASE[1], inst.seed, ArchProp::COLOR_VAR_G, spread);
        inst.colors[2] = entity_tint(ARCH_SANDSTONE_BASE[2], inst.seed, ArchProp::COLOR_VAR_B, spread);
    }
}

inline void arch_write_active(MachineCtx* c, const EntityInstance& inst) {
    float half_span = inst.params[ArchIdx::SPAN];  // already halved
    float rise      = inst.params[ArchIdx::RISE];
    float pier_h    = inst.params[ArchIdx::PIER_HEIGHT];

    auto& aa = c->entities_state_.arches[inst.slot];
    aa.patch_gx = inst.trigger_gx; aa.patch_gz = inst.trigger_gz;
    aa.host_gx = inst.host_gx; aa.host_gz = inst.host_gz;
    aa.active = true; aa.draw_visible = true;
    aa.world_x = inst.cx; aa.world_z = inst.cz;
    aa.rotation = inst.rotation;
    aa.half_span = half_span;
    aa.total_height = pier_h + rise;
    aa.tier = static_cast<ArchTier>(inst.tier_idx);
    aa.depth = inst.params[ArchIdx::DEPTH];
    aa.thickness = inst.params[ArchIdx::THICKNESS];
    aa.rise = rise;
    aa.pier_height = pier_h;
    aa.burial = inst.burial;
    aa.segs_u = ARCH_TIERS[inst.tier_idx].segs_u;
    aa.segs_v = ARCH_TIERS[inst.tier_idx].segs_v;
    aa.col_r = inst.colors[0]; aa.col_g = inst.colors[1]; aa.col_b = inst.colors[2];

    // Ground Y: GPU compute shader samples the heightfield at both leg
    // positions and takes the min (slope straddle). CPU uploads 0.
    aa.cached_ground_y = 0.0f;
    aa.mosaic_seed = inst.mosaic_seed;

    // Portal state: recompute from seed
    aa.is_portal = false;
    aa.is_back_portal = false;
    aa.position_hash = cpu_hash(inst.seed, ArchProp::ROTATION + 100u);
    aa.destination = PortalDestination{};
    if (inst.tier_idx == static_cast<uint32_t>(ArchTier::DOORWAY)
        && !c->world_state_.finite_mode) {
        // PORTAL_2 — the triad is a finite world's whole roster. A
        // dispatch DOORWAY indoors stays an arch; it opens nowhere.
        float portal_roll = cpu_hash_f(inst.seed, ArchProp::ROTATION + 200u);
        if (portal_roll < WORLD_DRAW_LIVE.portal_density) {
            aa.is_portal = true;
            uint32_t dest_seed = cpu_hash(aa.position_hash, 1u);
            uint32_t mood = pick_portal_mood(aa.position_hash, 2u);
            const auto& mp = mood_def(mood);
            aa.destination.seed = dest_seed;
            aa.destination.mood = mood;
            aa.destination.finite = mp.shape.finite;
            aa.destination.finite_radius = derive_finite_radius(dest_seed, mp);
            // A portal wears its destination's color — paint has no home here.
            // Zeroed AT THE DECISION so both mesh-param producers read one
            // correct value (MOSAIC_2b: the guard had two homes and they
            // disagreed across a cull cycle).
            aa.mosaic_seed = 0u;
            // PORTAL_1 — the SAME decision, for the same reason, one line
            // later. This is the only point in the tree that knows this arch
            // is a portal while its appearance is still being set. col_* is
            // now the ONE home of an arch's colour; every mesh-param producer
            // reads it and asks nothing.
            const float* pc = portal_color_for(aa.destination, aa.is_back_portal);
            aa.col_r = pc[0]; aa.col_g = pc[1]; aa.col_b = pc[2];
        }
    }

    c->mood_state_.portals_dirty = true;
}

inline void arch_write_gpu(MachineCtx* c, const EntityInstance& inst, wgpu::Queue& queue) {
    float half_span = inst.params[ArchIdx::SPAN];
    float rise      = inst.params[ArchIdx::RISE];

    GPUArchMeshParams mp{};
    mp.center_x   = inst.cx;
    mp.center_z   = inst.cz;
    mp.rotation   = inst.rotation;
    mp.half_span  = half_span;
    mp.rise       = rise;
    mp.depth      = inst.params[ArchIdx::DEPTH];
    mp.thickness  = inst.params[ArchIdx::THICKNESS];
    mp.pier_height = inst.params[ArchIdx::PIER_HEIGHT];
    mp.burial     = inst.burial;
    mp.catenary_a = solve_catenary_a(half_span, rise);
    mp.segs_u     = ARCH_TIERS[inst.tier_idx].segs_u;
    mp.segs_v     = ARCH_TIERS[inst.tier_idx].segs_v;
    // PORTAL_1: col_* is the one home — written by arch_write_active, which
    // generic_commit calls first, and already carrying the portal override.
    // The instance's own colours would miss that decision, and did.
    {
        const auto& aa = c->entities_state_.arches[inst.slot];
        mp.color_r = aa.col_r; mp.color_g = aa.col_g; mp.color_b = aa.col_b;
    }
    // MOSAIC_2b: the ONE home. arch_write_active (generic_commit calls it
    // first) has already written this slot's seed and zeroed it if the
    // arch is a portal, so both producers read one correct value — the
    // instance's own field would miss that decision.
    mp.mosaic_seed = c->entities_state_.arches[inst.slot].mosaic_seed;
    mp.is_active  = 1;
    c->gpuState_.upload_arch_mesh_params_slot(queue, inst.slot, mp);
    c->entities_state_.arch_mesh_gen_pending = true;
}


inline constexpr EntityFamilyAdapter ARCH_ADAPTER = {
    arch_run_gate,
    arch_apply_indoor_rescale,
    arch_compute_solid_half, arch_compute_colors,
    arch_write_active, arch_write_gpu, nullptr,       // no post_commit (the pier pair + its regen died with the bake)
    arch_get_tier_profile,
};

inline bool dispatch_select_arch_generic(MachineCtx* self, int32_t gx, int32_t gz, EntityQueueEntry& e) {
    EntityInstance inst{};
    if (!generic_select(self, ARCH_TRAITS, ARCH_ADAPTER, gx, gz, inst)) return false;
    e.family = PopFamily::ARCH; e.gx = gx; e.gz = gz; e.generic = inst; return true;
}
inline bool dispatch_place_arch_generic(MachineCtx* self, EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (generic_place(self, ARCH_TRAITS, e.generic)) { pe.generic = e.generic; return true; }
    self->entities_state_.arches[e.generic.slot].active = false; return false;
}
inline void dispatch_commit_arch_generic(MachineCtx* self, PlacementEntry& pe, wgpu::Queue& queue) {
    auto* host = find_patch(self, pe.generic.host_gx, pe.generic.host_gz);
    if (host) { generic_commit(self, ARCH_TRAITS, ARCH_ADAPTER, pe.generic, queue); host->record_entity(PopFamily::ARCH, pe.generic.slot); }
    // HOST PATCH GONE. The footprint was registered at place; its host
    // vanished before commit. Release by OWNER — the one release path.
    else { unregister_footprint_for(self, PopFamily::ARCH, pe.generic.slot); self->entities_state_.arches[pe.generic.slot].active = false; }
}

// ─── FAMILY_DISPATCH Integration ─────────────────────────────────

// ═══ F-5: THE COLLAPSE'S BIT-IDENTITY PIN ═════════════════════════
//
// Before the collapse, each *_run_gate passed five constants by hand. Now
// gate_from_traits reads them off the traits row instead. These asserts are
// the PROOF that the swap was value-for-value — the row must carry exactly
// what the hand-written call passed, or the gate math moved and no compiler
// would otherwise say so.
//
// They also outlive the collapse. max_instances is now the ONLY declaration of
// each family's slot bound reachable from the gate; if it ever drifts from the
// array's real extent, the gate would scan a different count than the array
// holds. This is the line that stops that.
//
// This is the last point in the cohort where all nine TRAITS are visible
// (entity_pipeline.hpp trails grounded/spheres/cube_behaviors), which is why
// the block lives here rather than at the contract.
// The pin doubles as the POSITIONAL-SHIFT PROOF for the A2 field cut. All nine
// TRAITS are positional aggregates, so removing a field silently slides every
// later initialiser up one slot — and glaw1 catches a type mismatch but NOT a
// same-typed value landing in the wrong field. Five fields were removed at
// positions 2, 5, 6, 7 and 19 of twenty-one.
//
// Every removal point has a pinned field immediately after it, so a slide
// cannot pass unnoticed:
//   after short_name(2)                        -> max_instances   PINNED
//   after creates_ground/piers/has_footprint(5,6,7) -> spawn_roll_prop PINNED
//   after gpu_ground_y(19)                     -> color_part_count PINNED
// grounded(4) is pinned too, which brackets the first removal from both sides.
#define T7_GATE_PIN(TR, FAM, MAXN, GND, PROP, CHANCE, NCOL)                       \
    static_assert(TR.family_id       == FAM,     #TR " family_id must match");    \
    static_assert(TR.max_instances   <= MAXN,    #TR " population law ≤ capacity; the lattice owns the living ceiling");\
    static_assert(TR.grounded        == GND,     #TR " grounded must match");     \
    static_assert(TR.spawn_roll_prop == PROP,    #TR " spawn_roll_prop must match"); \
    static_assert(TR.spawn_chance    == CHANCE,  #TR " spawn_chance must match"); \
    static_assert(TR.color_part_count == NCOL,   #TR " color_part_count must match"); \
    static_assert(TR.mood_multiplier == mood_mult_for(FAM), #TR " mood_multiplier must match")

T7_GATE_PIN(PYRAMID_TRAITS, PopFamily::PYRAMID, Dim::MAX_PYRAMID_INSTANCES, true,  PyramidProp::SPAWN_ROLL, PyramidConfig::SPAWN_CHANCE,     0u);
T7_GATE_PIN(ARCH_TRAITS,    PopFamily::ARCH,    Dim::MAX_ARCH_INSTANCES,    true,  ArchProp::SPAWN_ROLL,    ArchConfig::SPAWN_CHANCE,        0u);
T7_GATE_PIN(SPHERE_TRAITS,  PopFamily::SPHERE,  Dim::MAX_SPHERE_INSTANCES,  false, SphereProp::SPAWN_ROLL,  SphereConfig::SPAWN_CHANCE,      0u);
T7_GATE_PIN(CUBE_TRAITS,    PopFamily::CUBE,    Dim::MAX_CUBE_INSTANCES,    false, CubeProp::SPAWN_ROLL,    CubeConfig::SPAWN_CHANCE,        0u);

#undef T7_GATE_PIN

// The tail pair, checked once per family by pointer identity: color_parts is
// the LAST field, so if anything above it slid, this is where it shows.
static_assert(PYRAMID_TRAITS.color_parts == nullptr,            "PYRAMID_TRAITS color_parts");
static_assert(ARCH_TRAITS.color_parts    == nullptr,            "ARCH_TRAITS color_parts");
static_assert(SPHERE_TRAITS.color_parts  == nullptr,            "SPHERE_TRAITS color_parts");
static_assert(CUBE_TRAITS.color_parts    == nullptr,            "CUBE_TRAITS color_parts");

// ═══ END MODULE IMPLEMENTATION ═══════════════════════════════════

} // namespace the_board
} // namespace t7
