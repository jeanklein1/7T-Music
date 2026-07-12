// ─── entity_pipeline.inl ─────────────────────────────────────────
//
// Generic entity lifecycle for the nine cookie-cutter families (Blade,
// Palm, Cactus, Column+Antenna, Pyramid, Sphere, Cube, Arch).
//
// Each family contributes a block with the same 10-element template
// (see "Family block template" below). Type definitions live in
// entity_types.hpp, a file-scope header included above the class, so
// they precede every union by construction. This file is included
// AFTER the unions.
//
// SEAM[entity_pipeline:K1] tier sampling profile + extras live as a
//   single per-family TierRow struct embedded in this file. Single
//   source of truth — no converters, no derived tables.
// ─────────────────────────────────────────────────────────────────

// ═══ GENERIC HELPERS ═════════════════════════════════════════════

// ─── Generic Color Derivation ────────────────────────────────────

static void generic_compute_colors(EntityInstance& inst,
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

// ─── Indoor Rescale Helper ───────────────────────────────────────
//
//   • Columns: HEIGHT is set to ceiling_height exactly, and every
//     other length param scales by the same ratio so proportions
//     hold. The capital meets the ceiling — the column reads as
//     part of the room's architecture, not a freestanding object.
//
// Param indices below are hand-curated per family — only LENGTH
// dimensions get scaled, never ratios (TAPER, ENTASIS, ASPECT...),
// counts (BASE_LAYERS, RIBS, ARM_COUNT...), or angles (LEAN_DIR,
// FROND_DROOP...). Adding a new eligible family means picking which
// pattern it follows, declaring its own
// <family>_apply_indoor_rescale, and registering it in the adapter.
// Property index 7777u is reserved for the rescale-target hash
// (no other family uses it).
//
// SEAM[entity_pipeline:rescale-per-family] DONE — was a free-function
//   switch on family_id; lifted to per-family adapter slot during
//   Pass 7 of the modularity rollout.

//
// Column does NOT use this helper — its policy is "snap to ceiling
// exactly" rather than "roll a target ratio."
//
// The templated array reference avoids needing <initializer_list>;
// each caller passes a constexpr uint32_t array of param indices.
template<size_t N>
static void rescale_to_rolled_target(EntityInstance& inst, float ceiling_h,
    float target_lo, float target_hi, float current_h,
    const uint32_t (&params_to_scale)[N]) {
    if (current_h <= 1e-3f) return;
    constexpr uint32_t RESCALE_TARGET_PROP = 7777u;
    const float t = cpu_hash_f(inst.seed, RESCALE_TARGET_PROP);
    const float target_h = ceiling_h * (target_lo + (target_hi - target_lo) * t);
    const float scale = target_h / current_h;
    for (size_t i = 0; i < N; i++) inst.params[params_to_scale[i]] *= scale;
}

// ═══ GENERIC THREE-PHASE PIPELINE ════════════════════════════════

// ─── Generic Select ──────────────────────────────────────────────

bool generic_select(
    const EntityFamilyTraits& traits,
    const EntityFamilyAdapter& adapter,
    int32_t gx, int32_t gz,
    EntityInstance& inst)
{
    // ── Spawn gate (delegates to existing run_spawn_preamble) ──
    auto gate = adapter.run_gate(this, gx, gz);
    if (!gate.ok) return false;

    // ── Tier selection with theme bias ──
    // Tier weights and profiles come from the adapter's per-family
    // accessor; there's no generic table on traits to index.
    float weights[8]{};
    for (uint32_t t = 0; t < traits.tier_count && t < 8; t++)
        weights[t] = adapter.get_tier_profile(t).weight;

    // Apply theme tier weights (per-family array from PopulationTheme)
    const float* theme_tw = adapter.get_theme_tier_weights(gate.theme_idx);
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

    // ── Indoor rescale (must run before compute_solid_half so the
    //    solid extents are derived from the scaled params) ──
    if (MOOD_TABLE[mood_state_.active].indoor && adapter.apply_indoor_rescale) {
        adapter.apply_indoor_rescale(inst, MOOD_TABLE[mood_state_.active].ceiling_height);
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

bool generic_place(
    const EntityFamilyTraits& traits,
    EntityInstance& inst)
{
    auto pos = negotiate_position(inst.seed,
        inst.trigger_gx, inst.trigger_gz,
        traits.pos_x_prop, traits.pos_z_prop,
        traits.position_jitter,
        traits.rotation_prop,
        inst.solid_half, traits.family_id, inst.tier_idx);
    if (!pos.ok) return false;

    inst.host_gx  = pos.host_gx;
    inst.host_gz  = pos.host_gz;
    inst.cx       = pos.cx;
    inst.cz       = pos.cz;
    inst.rotation = pos.rotation;

    inst.cached_ground_y = 0.0f;

    record_placement_bookkeeping(traits.family_id, inst.tier_idx);
    return true;
}

// ─── Generic Commit ──────────────────────────────────────────────

void generic_commit(
    const EntityFamilyTraits& traits,
    const EntityFamilyAdapter& adapter,
    const EntityInstance& inst,
    wgpu::Queue& queue)
{
    // ── Active tracking (per-family array) ──
    adapter.write_active(this, inst);

    // ── GPU mesh params (per-family struct mapping) ──
    adapter.write_gpu(this, inst, queue);

    // ── Post-commit: piers, regen, portals, etc. ──
    if (adapter.post_commit)
        adapter.post_commit(this, inst, queue);

    world_state_.ground_entries_dirty = true;
}

// ═══ FAMILIES: BLADE / PALM / CACTUS — RELOCATED ══════════════════

// ═══ FAMILY: COLUMN + ANTENNA ═════════════════════════════════════

// Shared param index layout (both families sample the same 13 params)
struct ColIdx {
    static constexpr uint32_t HEIGHT          = 0;
    static constexpr uint32_t SHAFT_RADIUS    = 1;
    static constexpr uint32_t TAPER           = 2;
    static constexpr uint32_t ENTASIS         = 3;
    static constexpr uint32_t BASE_LAYERS     = 4;
    static constexpr uint32_t BASE_HEIGHT     = 5;
    static constexpr uint32_t BASE_OVERHANG   = 6;
    static constexpr uint32_t CAP_LAYERS      = 7;
    static constexpr uint32_t CAP_HEIGHT      = 8;
    static constexpr uint32_t CAP_OVERHANG    = 9;
    static constexpr uint32_t SOLID_PADDING   = 10;
    static constexpr uint32_t SOLID_HEIGHT    = 11;
    static constexpr uint32_t EDGE_BLEND      = 12;
    static constexpr uint32_t COUNT           = 13;
};

// Column param defs (ColumnProp indices)
//                                                    prop                        floor   ceil   round  dist
static constexpr TierParamDef COLUMN_PARAM_DEFS[] = {
    { ColumnProp::HEIGHT,          1.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { ColumnProp::SHAFT_RADIUS,    0.1f, 1e30f, false, ParamDist::GAUSSIAN },
    { ColumnProp::TAPER,           0.5f, 1.0f,  false, ParamDist::GAUSSIAN },  // ceiling!
    { ColumnProp::ENTASIS,         0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { ColumnProp::BASE_LAYERS,     0.0f, 1e30f, true,  ParamDist::GAUSSIAN },
    { ColumnProp::BASE_HEIGHT,     0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { ColumnProp::BASE_OVERHANG,   0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { ColumnProp::CAPITAL_LAYERS,  0.0f, 1e30f, true,  ParamDist::GAUSSIAN },
    { ColumnProp::CAPITAL_HEIGHT,  0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { ColumnProp::CAPITAL_OVERHANG,0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { ColumnProp::SOLID_PADDING,   0.05f,1e30f, false, ParamDist::GAUSSIAN },
    { ColumnProp::SOLID_HEIGHT,    0.6f, 1e30f, false, ParamDist::GAUSSIAN },
    { ColumnProp::EDGE_BLEND,      0.1f, 1e30f, false, ParamDist::GAUSSIAN },
};
static constexpr uint32_t COLUMN_PARAM_COUNT = sizeof(COLUMN_PARAM_DEFS) / sizeof(TierParamDef);

// Antenna param defs (AntennaProp indices, same layout)
static constexpr TierParamDef ANTENNA_PARAM_DEFS[] = {
    { AntennaProp::HEIGHT,          1.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { AntennaProp::SHAFT_RADIUS,    0.1f, 1e30f, false, ParamDist::GAUSSIAN },
    { AntennaProp::TAPER,           0.5f, 1.0f,  false, ParamDist::GAUSSIAN },
    { AntennaProp::ENTASIS,         0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { AntennaProp::BASE_LAYERS,     0.0f, 1e30f, true,  ParamDist::GAUSSIAN },
    { AntennaProp::BASE_HEIGHT,     0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { AntennaProp::BASE_OVERHANG,   0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { AntennaProp::CAPITAL_LAYERS,  0.0f, 1e30f, true,  ParamDist::GAUSSIAN },
    { AntennaProp::CAPITAL_HEIGHT,  0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { AntennaProp::CAPITAL_OVERHANG,0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { AntennaProp::SOLID_PADDING,   0.05f,1e30f, false, ParamDist::GAUSSIAN },
    { AntennaProp::SOLID_HEIGHT,    0.6f, 1e30f, false, ParamDist::GAUSSIAN },
    { AntennaProp::EDGE_BLEND,      0.1f, 1e30f, false, ParamDist::GAUSSIAN },
};

// params[] order MUST match COLUMN_PARAM_DEFS / ANTENNA_PARAM_DEFS:
//   [0]HEIGHT [1]SHAFT_R [2]TAPER [3]ENTASIS [4]BASE_LAYERS [5]BASE_H
//   [6]BASE_OH [7]CAP_LAYERS [8]CAP_H [9]CAP_OH [10]SOLID_PAD
//   [11]SOLID_H [12]EDGE_BLEND
//
// Note: name is ColumnTierRow, not ColumnTier — entities.inl declares
// `enum class ColumnTier`, occupying that name.
struct ColumnTierRow {
    TierProfile profile;
    float       color_override;
    float       burial;
    uint32_t    segs_around;
    uint32_t    shaft_rings;
};

static constexpr ColumnTierRow COLUMN_TIERS[] = {
    /* PILLAR */ {
        { 0.05f, 0.0f, { {6.5f, 1.2f}, {1.80f, 0.30f}, {1.00f, 0.0f},  {0.00f, 0.0f},
                   {1.0f, 0.3f}, {0.50f, 0.10f}, {0.20f, 0.05f},
                   {1.0f, 0.3f}, {0.40f, 0.08f}, {0.15f, 0.04f},
                   {0.3f, 0.08f}, {1.5f, 0.3f},  {0.4f, 0.08f} }},
        0.85f, 0.25f, 16, 4
    },
    /* DORIC  */ {
        { 0.20f, 0.0f, { {6.4f, 1.2f}, {0.38f, 0.06f}, {0.85f, 0.03f}, {0.02f, 0.01f},
                   {0.0f, 0.0f}, {0.00f, 0.00f}, {0.00f, 0.00f},
                   {2.0f, 0.5f}, {0.50f, 0.10f}, {0.15f, 0.04f},
                   {0.2f, 0.05f}, {1.0f, 0.2f},  {0.3f, 0.05f} }},
        0.85f, 0.20f, 20, 8
    },
    /* ORNATE */ {
        { 0.18f, 0.0f, { {16.8f, 2.8f}, {1.35f, 0.19f}, {0.82f, 0.03f}, {0.04f, 0.01f},
                   {2.0f, 0.5f},  {1.20f, 0.25f}, {0.30f, 0.08f},
                   {3.0f, 0.5f},  {1.50f, 0.30f}, {0.40f, 0.10f},
                   {0.4f, 0.10f}, {1.5f, 0.3f},   {0.5f, 0.10f} }},
        0.85f, 0.20f, 28, 12
    },
};

static constexpr ColumnTierRow ANTENNA_TIERS[] = {
    /* ANTENNA  */ {
        { 0.10f, 0.0f, { {17.5f, 3.5f}, {0.30f, 0.05f}, {0.85f, 0.05f}, {0.00f, 0.0f},
                   {2.0f, 0.5f},  {2.1f, 0.42f},  {1.5f, 0.3f},
                   {0.0f, 0.0f},  {1.5f, 0.3f},   {0.0f, 0.0f},
                   {0.2f, 0.05f}, {1.0f, 0.2f},   {0.3f, 0.05f} }},
        0.40f, 0.20f, 16, 6
    },
    /* SQUAT    */ {
        { 0.22f, 0.0f, { {32.5f, 6.5f}, {0.90f, 0.15f}, {0.85f, 0.05f}, {0.00f, 0.0f},
                   {2.0f, 0.5f},  {2.0f, 0.4f},   {6.0f, 1.2f},
                   {0.0f, 0.0f},  {1.5f, 0.3f},   {0.0f, 0.0f},
                   {0.4f, 0.10f}, {1.5f, 0.3f},   {0.4f, 0.08f} }},
        0.40f, 0.20f, 16, 6
    },
    /* COLOSSAL */ {
        { 0.13f, 0.0f, { {125.0f, 25.0f}, {3.00f, 0.50f}, {0.85f, 0.05f}, {0.00f, 0.0f},
                   {2.0f, 0.5f},    {7.5f, 1.5f},   {17.5f, 3.5f},
                   {0.0f, 0.0f},    {7.5f, 1.5f},   {0.0f, 0.0f},
                   {1.95f, 0.39f},  {12.0f, 2.4f},  {1.0f, 0.20f} }},
        0.40f, 0.20f, 20, 8
    },
};

static const TierProfile& column_get_tier_profile(uint32_t tier_idx) {
    return COLUMN_TIERS[tier_idx].profile;
}
static const TierProfile& antenna_get_tier_profile(uint32_t tier_idx) {
    return ANTENNA_TIERS[tier_idx].profile;
}

// ── Column traits ──

static constexpr EntityFamilyTraits COLUMN_TRAITS = {
    PopFamily::COLUMN, "col", Dim::MAX_COLUMN_ONLY,
    true, true, 1,                                     // grounded, creates ground, 1 pier
    true, 80.0f, 2.5f,
    ColumnProp::SPAWN_ROLL, ColumnConfig::SPAWN_CHANCE,
    ColumnConfig::MOOD_MULTIPLIER, ColumnConfig::POSITION_JITTER,
    COLUMN_TIER_COUNT, ColumnProp::TIER,
    COLUMN_PARAM_DEFS, COLUMN_PARAM_COUNT,
    ColumnProp::POSITION_X, ColumnProp::POSITION_Z, 355u, true,
    0, nullptr,  // color handled entirely by adapter
};

static constexpr EntityFamilyTraits ANTENNA_TRAITS = {
    PopFamily::ANTENNA, "ant", Dim::MAX_ANTENNA_ONLY,
    true, true, 1,
    true, 80.0f, 2.5f,
    AntennaProp::SPAWN_ROLL, AntennaConfig::SPAWN_CHANCE,
    AntennaConfig::MOOD_MULTIPLIER, AntennaConfig::POSITION_JITTER,
    ANTENNA_TIER_COUNT, AntennaProp::TIER,
    ANTENNA_PARAM_DEFS, COLUMN_PARAM_COUNT,
    AntennaProp::POSITION_X, AntennaProp::POSITION_Z, 355u, true,
    0, nullptr,
};

// ── Column adapter functions ──

static SpawnGateOutput column_run_gate(Cartridge* c, int32_t gx, int32_t gz) {
    auto gate = c->run_spawn_preamble(gx, gz,
        c->entities_state_.columns, Dim::MAX_COLUMN_ONLY,
        ColumnProp::SPAWN_ROLL, ColumnConfig::SPAWN_CHANCE,
        ColumnConfig::MOOD_MULTIPLIER, PopFamily::COLUMN, "col");
    return { gate.ok, gate.seed, gate.slot, gate.theme_idx };
}
static const float* column_get_theme_tier_weights(uint32_t ti) { return THEMES[ti].tier_wt_column; }

static constexpr uint32_t COLUMN_INDOOR_RESCALE_PARAMS[] = {
    ColIdx::HEIGHT, ColIdx::SHAFT_RADIUS,
    ColIdx::BASE_HEIGHT, ColIdx::BASE_OVERHANG,
    ColIdx::CAP_HEIGHT,  ColIdx::CAP_OVERHANG,
    ColIdx::SOLID_PADDING, ColIdx::SOLID_HEIGHT, ColIdx::EDGE_BLEND,
    // TAPER (ratio), ENTASIS (ratio), BASE_LAYERS / CAP_LAYERS
    // (counts) intentionally not scaled.
};

// Column policy: snap to ceiling exactly. The capital meets the
// ceiling — column reads as part of the room's architecture, not a
// freestanding object. Does NOT use rescale_to_rolled_target — its
// scale factor is ceiling_h/current_h, not a rolled ratio.
static void column_apply_indoor_rescale(EntityInstance& inst, float ceiling_h) {
    const float current_h = inst.params[ColIdx::HEIGHT];
    if (current_h <= 1e-3f) return;
    const float scale = ceiling_h / current_h;
    for (uint32_t pi : COLUMN_INDOOR_RESCALE_PARAMS) inst.params[pi] *= scale;
}

static void column_compute_solid_half(EntityInstance& inst, const TierProfile&) {
    float shaft_r = inst.params[ColIdx::SHAFT_RADIUS];
    float base_oh = inst.params[ColIdx::BASE_OVERHANG];
    float cap_oh  = inst.params[ColIdx::CAP_OVERHANG];
    float pad     = inst.params[ColIdx::SOLID_PADDING];
    float blend   = inst.params[ColIdx::EDGE_BLEND];
    float solid_h = inst.params[ColIdx::SOLID_HEIGHT];
    inst.solid_half = shaft_r + std::max(base_oh, cap_oh) + pad + blend;
    inst.ground_y_offset = solid_h;
    inst.burial = std::max(0.2f, solid_h * COLUMN_TIERS[inst.tier_idx].burial);
}

static void column_compute_colors(EntityInstance& inst, const EntityFamilyTraits&, const TierProfile& /*tier*/) {
    // Sandstone / palette override
    if (cpu_hash_f(inst.seed, ColumnProp::COLOR_OVER) < COLUMN_TIERS[inst.tier_idx].color_override) {
        uint32_t pal_idx = cpu_hash(inst.seed, ColumnProp::COLOR_OVER + 1u) % COLUMN_PALETTE_COUNT;
        inst.colors[0] = COLUMN_PALETTE[pal_idx][0] + (cpu_hash_f(inst.seed, ColumnProp::COLOR_VAR_R) - 0.5f) * 0.06f;
        inst.colors[1] = COLUMN_PALETTE[pal_idx][1] + (cpu_hash_f(inst.seed, ColumnProp::COLOR_VAR_G) - 0.5f) * 0.06f;
        inst.colors[2] = COLUMN_PALETTE[pal_idx][2] + (cpu_hash_f(inst.seed, ColumnProp::COLOR_VAR_B) - 0.5f) * 0.06f;
    } else {
        inst.colors[0] = COLUMN_SANDSTONE_BASE[0] + (cpu_hash_f(inst.seed, ColumnProp::COLOR_VAR_R) - 0.5f) * COLUMN_SANDSTONE_VARIANCE * 2.0f;
        inst.colors[1] = COLUMN_SANDSTONE_BASE[1] + (cpu_hash_f(inst.seed, ColumnProp::COLOR_VAR_G) - 0.5f) * COLUMN_SANDSTONE_VARIANCE * 2.0f;
        inst.colors[2] = COLUMN_SANDSTONE_BASE[2] + (cpu_hash_f(inst.seed, ColumnProp::COLOR_VAR_B) - 0.5f) * COLUMN_SANDSTONE_VARIANCE * 2.0f;
    }
    // No drum colors for classical columns
    for (int i = 3; i < 12; i++) inst.colors[i] = 0.0f;
}

static void column_write_active(Cartridge* c, const EntityInstance& inst) {
    auto& ac = c->entities_state_.columns[inst.slot];
    ac.patch_gx = inst.trigger_gx; ac.patch_gz = inst.trigger_gz;
    ac.host_gx = inst.host_gx; ac.host_gz = inst.host_gz;
    ac.active = true; ac.draw_visible = true;
    ac.world_x = inst.cx; ac.world_z = inst.cz;
    ac.height = inst.params[ColIdx::HEIGHT];
    ac.shaft_radius = inst.params[ColIdx::SHAFT_RADIUS];
    ac.taper = inst.params[ColIdx::TAPER];
    ac.entasis = inst.params[ColIdx::ENTASIS];
    ac.base_layers = (uint32_t)inst.params[ColIdx::BASE_LAYERS];
    ac.base_height = inst.params[ColIdx::BASE_HEIGHT];
    ac.base_overhang = inst.params[ColIdx::BASE_OVERHANG];
    ac.cap_layers = (uint32_t)inst.params[ColIdx::CAP_LAYERS];
    ac.cap_height = inst.params[ColIdx::CAP_HEIGHT];
    ac.cap_overhang = inst.params[ColIdx::CAP_OVERHANG];
    ac.solid_height = inst.params[ColIdx::SOLID_HEIGHT];
    ac.burial = inst.burial;
    ac.segs_around = COLUMN_TIERS[inst.tier_idx].segs_around;
    ac.shaft_rings = COLUMN_TIERS[inst.tier_idx].shaft_rings;
    ac.tier_idx = inst.tier_idx;
    ac.cached_ground_y = inst.cached_ground_y;
    ac.col_r = inst.colors[0]; ac.col_g = inst.colors[1]; ac.col_b = inst.colors[2];
    std::memcpy(ac.drum_colors, &inst.colors[3], 9 * sizeof(float));
    c->entities_state_.column_count++;
}

static void column_write_gpu(Cartridge* c, const EntityInstance& inst, wgpu::Queue& queue) {
    GPUColumnMeshParams mp{};
    mp.center_x = inst.cx; mp.center_z = inst.cz;
    mp.height = inst.params[ColIdx::HEIGHT];
    mp.shaft_radius = inst.params[ColIdx::SHAFT_RADIUS];
    mp.taper = inst.params[ColIdx::TAPER];
    mp.entasis = inst.params[ColIdx::ENTASIS];
    mp.base_height = inst.params[ColIdx::BASE_HEIGHT];
    mp.base_overhang = inst.params[ColIdx::BASE_OVERHANG];
    mp.capital_height = inst.params[ColIdx::CAP_HEIGHT];
    mp.capital_overhang = inst.params[ColIdx::CAP_OVERHANG];
    mp.burial = inst.burial;
    mp.color_r = inst.colors[0]; mp.color_g = inst.colors[1]; mp.color_b = inst.colors[2];
    mp.base_layers = (uint32_t)inst.params[ColIdx::BASE_LAYERS];
    mp.capital_layers = (uint32_t)inst.params[ColIdx::CAP_LAYERS];
    mp.segs_around = COLUMN_TIERS[inst.tier_idx].segs_around;
    mp.shaft_rings = COLUMN_TIERS[inst.tier_idx].shaft_rings;
    mp.is_active = 1;
    mp.tier = inst.tier_idx;
    // drum colors from inst.colors[3..11]
    mp.drum_color_r1 = inst.colors[3]; mp.drum_color_g1 = inst.colors[4]; mp.drum_color_b1 = inst.colors[5];
    mp.drum_color_r2 = inst.colors[6]; mp.drum_color_g2 = inst.colors[7]; mp.drum_color_b2 = inst.colors[8];
    mp.drum_color_r3 = inst.colors[9]; mp.drum_color_g3 = inst.colors[10]; mp.drum_color_b3 = inst.colors[11];
    c->gpuState_.upload_column_mesh_params_slot(queue, inst.slot, mp);
    c->entities_state_.column_mesh_gen_pending = true;
}

static void column_post_commit(Cartridge* c, const EntityInstance& inst, wgpu::Queue& queue) {
    uint32_t pier_slot = Dim::PIER_COLUMN_BASE + inst.slot;
    GPUPierInstance pier{};
    pier.origin[0] = inst.cx;
    pier.origin[1] = inst.cz;
    pier.half_size[0] = inst.solid_half;
    pier.half_size[1] = inst.solid_half;
    pier.height_near = inst.params[ColIdx::SOLID_HEIGHT];
    pier.height_far = inst.params[ColIdx::SOLID_HEIGHT];
    pier.rotation = 0.0f;
    pier.edge_blend = inst.params[ColIdx::EDGE_BLEND];
    pier.tier = PierTier::COL_PILLAR + inst.tier_idx;
    pier.is_active = 1;
    c->write_pier(queue, pier_slot, pier);
}

static constexpr EntityFamilyAdapter COLUMN_ADAPTER = {
    column_run_gate, column_get_theme_tier_weights,
    column_apply_indoor_rescale,
    column_compute_solid_half, column_compute_colors,
    column_write_active, column_write_gpu, column_post_commit,
    column_get_tier_profile,
};

// ── Column dispatch wrappers ──

static bool dispatch_select_column_generic(Cartridge* self, int32_t gx, int32_t gz, EntityQueueEntry& e) {
    EntityInstance inst{};
    if (!self->generic_select(COLUMN_TRAITS, COLUMN_ADAPTER, gx, gz, inst)) return false;
    e.family = PopFamily::COLUMN; e.gx = gx; e.gz = gz; e.generic = inst; return true;
}
static bool dispatch_place_column_generic(Cartridge* self, EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (self->generic_place(COLUMN_TRAITS, e.generic)) { pe.generic = e.generic; return true; }
    self->entities_state_.columns[e.generic.slot].active = false; return false;
}
static void dispatch_commit_column_generic(Cartridge* self, PlacementEntry& pe, wgpu::Queue& queue) {
    auto* host = find_patch(self, pe.generic.host_gx, pe.generic.host_gz);
    if (host) { self->generic_commit(COLUMN_TRAITS, COLUMN_ADAPTER, pe.generic, queue); host->record_entity(PopFamily::COLUMN, pe.generic.slot); }
    else { self->entities_state_.columns[pe.generic.slot].active = false; }
}

// ── Antenna adapter functions ──

static SpawnGateOutput antenna_run_gate(Cartridge* c, int32_t gx, int32_t gz) {
    auto gate = c->run_spawn_preamble(gx, gz,
        c->entities_state_.antennas, Dim::MAX_ANTENNA_ONLY,
        AntennaProp::SPAWN_ROLL, AntennaConfig::SPAWN_CHANCE,
        AntennaConfig::MOOD_MULTIPLIER, PopFamily::ANTENNA, "ant");
    return { gate.ok, gate.seed, gate.slot, gate.theme_idx };
}
static const float* antenna_get_theme_tier_weights(uint32_t ti) { return THEMES[ti].tier_wt_antenna; }

static void antenna_apply_indoor_rescale(EntityInstance& inst, float ceiling_h) {
    rescale_to_rolled_target(inst, ceiling_h,
        /*target_lo*/ 0.50f, /*target_hi*/ 0.95f,
        /*current_h*/ inst.params[ColIdx::HEIGHT],
        COLUMN_INDOOR_RESCALE_PARAMS);
}

static void antenna_compute_solid_half(EntityInstance& inst, const TierProfile&) {
    float shaft_r = inst.params[ColIdx::SHAFT_RADIUS];
    float pad     = inst.params[ColIdx::SOLID_PADDING];
    float blend   = inst.params[ColIdx::EDGE_BLEND];
    float solid_h = inst.params[ColIdx::SOLID_HEIGHT];
    inst.solid_half = shaft_r * 2.0f + pad + blend;  // antenna formula: wraps the post
    inst.ground_y_offset = solid_h;
    inst.burial = std::max(0.2f, solid_h * ANTENNA_TIERS[inst.tier_idx].burial);
    // GPU tier offset: antenna tiers map to 3,4,5 on the GPU
    inst.tier_idx = inst.tier_idx + COLUMN_TIER_COUNT;
}

static void antenna_compute_colors(EntityInstance& inst, const EntityFamilyTraits&, const TierProfile& /*tier*/) {
    // Sandstone / palette override (same as column)
    if (cpu_hash_f(inst.seed, AntennaProp::COLOR_OVER) < ANTENNA_TIERS[inst.tier_idx - COLUMN_TIER_COUNT].color_override) {
        uint32_t pal_idx = cpu_hash(inst.seed, AntennaProp::COLOR_OVER + 1u) % COLUMN_PALETTE_COUNT;
        inst.colors[0] = COLUMN_PALETTE[pal_idx][0] + (cpu_hash_f(inst.seed, AntennaProp::COLOR_VAR_R) - 0.5f) * 0.06f;
        inst.colors[1] = COLUMN_PALETTE[pal_idx][1] + (cpu_hash_f(inst.seed, AntennaProp::COLOR_VAR_G) - 0.5f) * 0.06f;
        inst.colors[2] = COLUMN_PALETTE[pal_idx][2] + (cpu_hash_f(inst.seed, AntennaProp::COLOR_VAR_B) - 0.5f) * 0.06f;
    } else {
        inst.colors[0] = COLUMN_SANDSTONE_BASE[0] + (cpu_hash_f(inst.seed, AntennaProp::COLOR_VAR_R) - 0.5f) * COLUMN_SANDSTONE_VARIANCE * 2.0f;
        inst.colors[1] = COLUMN_SANDSTONE_BASE[1] + (cpu_hash_f(inst.seed, AntennaProp::COLOR_VAR_G) - 0.5f) * COLUMN_SANDSTONE_VARIANCE * 2.0f;
        inst.colors[2] = COLUMN_SANDSTONE_BASE[2] + (cpu_hash_f(inst.seed, AntennaProp::COLOR_VAR_B) - 0.5f) * COLUMN_SANDSTONE_VARIANCE * 2.0f;
    }
    // Drum colors: 3 palettes, decorrelated picks
    static constexpr float DRUM_PALETTE[][3] = {
        {0.85f,0.55f,0.35f},{0.45f,0.60f,0.70f},{0.70f,0.65f,0.45f},
        {0.55f,0.70f,0.55f},{0.75f,0.50f,0.55f},{0.60f,0.55f,0.68f},
    };
    static constexpr uint32_t DRUM_PAL_COUNT = 6;
    uint32_t d1 = cpu_hash(inst.seed, 850u) % DRUM_PAL_COUNT;
    uint32_t d2 = (d1 + 1 + cpu_hash(inst.seed, 851u) % (DRUM_PAL_COUNT - 1)) % DRUM_PAL_COUNT;
    uint32_t d3 = (d2 + 1 + cpu_hash(inst.seed, 852u) % (DRUM_PAL_COUNT - 2)) % DRUM_PAL_COUNT;
    float v = 0.04f;
    inst.colors[3]  = DRUM_PALETTE[d1][0] + (cpu_hash_f(inst.seed, 860u) - 0.5f) * v;
    inst.colors[4]  = DRUM_PALETTE[d1][1] + (cpu_hash_f(inst.seed, 861u) - 0.5f) * v;
    inst.colors[5]  = DRUM_PALETTE[d1][2] + (cpu_hash_f(inst.seed, 862u) - 0.5f) * v;
    inst.colors[6]  = DRUM_PALETTE[d2][0] + (cpu_hash_f(inst.seed, 863u) - 0.5f) * v;
    inst.colors[7]  = DRUM_PALETTE[d2][1] + (cpu_hash_f(inst.seed, 864u) - 0.5f) * v;
    inst.colors[8]  = DRUM_PALETTE[d2][2] + (cpu_hash_f(inst.seed, 865u) - 0.5f) * v;
    inst.colors[9]  = DRUM_PALETTE[d3][0] + (cpu_hash_f(inst.seed, 866u) - 0.5f) * v;
    inst.colors[10] = DRUM_PALETTE[d3][1] + (cpu_hash_f(inst.seed, 867u) - 0.5f) * v;
    inst.colors[11] = DRUM_PALETTE[d3][2] + (cpu_hash_f(inst.seed, 868u) - 0.5f) * v;
}

static void antenna_write_active(Cartridge* c, const EntityInstance& inst) {
    auto& ac = c->entities_state_.antennas[inst.slot];
    ac.patch_gx = inst.trigger_gx; ac.patch_gz = inst.trigger_gz;
    ac.host_gx = inst.host_gx; ac.host_gz = inst.host_gz;
    ac.active = true; ac.draw_visible = true;
    ac.world_x = inst.cx; ac.world_z = inst.cz;
    ac.height = inst.params[ColIdx::HEIGHT];
    ac.shaft_radius = inst.params[ColIdx::SHAFT_RADIUS];
    ac.taper = inst.params[ColIdx::TAPER];
    ac.entasis = inst.params[ColIdx::ENTASIS];
    ac.base_layers = (uint32_t)inst.params[ColIdx::BASE_LAYERS];
    ac.base_height = inst.params[ColIdx::BASE_HEIGHT];
    ac.base_overhang = inst.params[ColIdx::BASE_OVERHANG];
    ac.cap_layers = (uint32_t)inst.params[ColIdx::CAP_LAYERS];
    ac.cap_height = inst.params[ColIdx::CAP_HEIGHT];
    ac.cap_overhang = inst.params[ColIdx::CAP_OVERHANG];
    ac.solid_height = inst.params[ColIdx::SOLID_HEIGHT];
    ac.burial = inst.burial;
    ac.segs_around = ANTENNA_TIERS[inst.tier_idx - COLUMN_TIER_COUNT].segs_around;
    ac.shaft_rings = ANTENNA_TIERS[inst.tier_idx - COLUMN_TIER_COUNT].shaft_rings;
    ac.tier_idx = inst.tier_idx;
    ac.cached_ground_y = inst.cached_ground_y;
    ac.col_r = inst.colors[0]; ac.col_g = inst.colors[1]; ac.col_b = inst.colors[2];
    std::memcpy(ac.drum_colors, &inst.colors[3], 9 * sizeof(float));
    c->entities_state_.antenna_count++;
}

static void antenna_write_gpu(Cartridge* c, const EntityInstance& inst, wgpu::Queue& queue) {
    uint32_t gpu_slot = inst.slot + Dim::ANTENNA_SLOT_OFFSET;
    uint32_t raw_tier = inst.tier_idx - COLUMN_TIER_COUNT;
    GPUColumnMeshParams mp{};
    mp.center_x = inst.cx; mp.center_z = inst.cz;
    mp.height = inst.params[ColIdx::HEIGHT];
    mp.shaft_radius = inst.params[ColIdx::SHAFT_RADIUS];
    mp.taper = inst.params[ColIdx::TAPER];
    mp.entasis = inst.params[ColIdx::ENTASIS];
    mp.base_height = inst.params[ColIdx::BASE_HEIGHT];
    mp.base_overhang = inst.params[ColIdx::BASE_OVERHANG];
    mp.capital_height = inst.params[ColIdx::CAP_HEIGHT];
    mp.capital_overhang = inst.params[ColIdx::CAP_OVERHANG];
    mp.burial = inst.burial;
    mp.color_r = inst.colors[0]; mp.color_g = inst.colors[1]; mp.color_b = inst.colors[2];
    mp.base_layers = (uint32_t)inst.params[ColIdx::BASE_LAYERS];
    mp.capital_layers = (uint32_t)inst.params[ColIdx::CAP_LAYERS];
    mp.segs_around = ANTENNA_TIERS[raw_tier].segs_around;
    mp.shaft_rings = ANTENNA_TIERS[raw_tier].shaft_rings;
    mp.is_active = 1;
    mp.tier = inst.tier_idx;  // GPU tier: 3,4,5 for antennas
    mp.drum_color_r1 = inst.colors[3]; mp.drum_color_g1 = inst.colors[4]; mp.drum_color_b1 = inst.colors[5];
    mp.drum_color_r2 = inst.colors[6]; mp.drum_color_g2 = inst.colors[7]; mp.drum_color_b2 = inst.colors[8];
    mp.drum_color_r3 = inst.colors[9]; mp.drum_color_g3 = inst.colors[10]; mp.drum_color_b3 = inst.colors[11];
    c->gpuState_.upload_column_mesh_params_slot(queue, gpu_slot, mp);
    c->entities_state_.column_mesh_gen_pending = true;
}

static void antenna_post_commit(Cartridge* c, const EntityInstance& inst, wgpu::Queue& queue) {
    uint32_t gpu_slot = inst.slot + Dim::ANTENNA_SLOT_OFFSET;
    uint32_t pier_slot = Dim::PIER_COLUMN_BASE + gpu_slot;
    GPUPierInstance pier{};
    pier.origin[0] = inst.cx;
    pier.origin[1] = inst.cz;
    pier.half_size[0] = inst.solid_half;
    pier.half_size[1] = inst.solid_half;
    pier.height_near = inst.params[ColIdx::SOLID_HEIGHT];
    pier.height_far = inst.params[ColIdx::SOLID_HEIGHT];
    pier.rotation = 0.0f;
    pier.edge_blend = inst.params[ColIdx::EDGE_BLEND];
    pier.tier = PierTier::COL_PILLAR + inst.tier_idx;
    pier.is_active = 1;
    c->write_pier(queue, pier_slot, pier);
}

static constexpr EntityFamilyAdapter ANTENNA_ADAPTER = {
    antenna_run_gate, antenna_get_theme_tier_weights,
    antenna_apply_indoor_rescale,
    antenna_compute_solid_half, antenna_compute_colors,
    antenna_write_active, antenna_write_gpu, antenna_post_commit,
    antenna_get_tier_profile,
};

// ── Antenna dispatch wrappers ──

static bool dispatch_select_antenna_generic(Cartridge* self, int32_t gx, int32_t gz, EntityQueueEntry& e) {
    EntityInstance inst{};
    if (!self->generic_select(ANTENNA_TRAITS, ANTENNA_ADAPTER, gx, gz, inst)) return false;
    e.family = PopFamily::ANTENNA; e.gx = gx; e.gz = gz; e.generic = inst; return true;
}
static bool dispatch_place_antenna_generic(Cartridge* self, EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (self->generic_place(ANTENNA_TRAITS, e.generic)) { pe.generic = e.generic; return true; }
    self->entities_state_.antennas[e.generic.slot].active = false; return false;
}
static void dispatch_commit_antenna_generic(Cartridge* self, PlacementEntry& pe, wgpu::Queue& queue) {
    auto* host = find_patch(self, pe.generic.host_gx, pe.generic.host_gz);
    if (host) { self->generic_commit(ANTENNA_TRAITS, ANTENNA_ADAPTER, pe.generic, queue); host->record_entity(PopFamily::ANTENNA, pe.generic.slot); }
    else { self->entities_state_.antennas[pe.generic.slot].active = false; }
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

static constexpr TierParamDef PYRAMID_PARAM_DEFS[] = {
    { PyramidProp::HEIGHT,     20.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { PyramidProp::BASE_HALF,  5.0f,  1e30f, false, ParamDist::GAUSSIAN },
    { PyramidProp::ASPECT,     0.5f,  2.0f,  false, ParamDist::GAUSSIAN },  // ceiling 2.0
    { PyramidProp::TRUNCATION, 0.0f,  0.5f,  false, ParamDist::GAUSSIAN },  // ceiling 0.5
    { PyramidProp::EDGE_BLEND, 0.5f,  1e30f, false, ParamDist::GAUSSIAN },
};
static constexpr uint32_t PYRAMID_PARAM_COUNT = sizeof(PYRAMID_PARAM_DEFS) / sizeof(TierParamDef);

// params[] order MUST match PYRAMID_PARAM_DEFS:
//   [0]HEIGHT [1]BASE_HALF [2]ASPECT [3]TRUNCATION [4]EDGE_BLEND
// Note: cannot reuse `PyramidTier` as the struct name — entities.inl
// declares `enum class PyramidTier`, which occupies the same name slot.
// `PyramidTierRow` keeps the new struct distinct without renaming the
// enum (used widely as PyramidTier::OBELISK etc.).
struct PyramidTierRow {
    TierProfile profile;
    float color_override;
    float color_variance;
};

static constexpr PyramidTierRow PYRAMID_TIERS[] = {
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

static const TierProfile& pyramid_get_tier_profile(uint32_t tier_idx) {
    return PYRAMID_TIERS[tier_idx].profile;
}

static constexpr EntityFamilyTraits PYRAMID_TRAITS = {
    PopFamily::PYRAMID, "pyr", Dim::MAX_PYRAMID_INSTANCES,
    true, false, 0,       // grounded, no piers (bakes into heightfield instead)
    true, 200.0f, 1.0f,
    PyramidProp::SPAWN_ROLL, PyramidConfig::SPAWN_CHANCE,
    PyramidConfig::MOOD_MULTIPLIER, PyramidConfig::POSITION_JITTER,
    3, PyramidProp::TIER,
    PYRAMID_PARAM_DEFS, PYRAMID_PARAM_COUNT,
    PyramidProp::POSITION_X, PyramidProp::POSITION_Z, PyramidProp::ROTATION, true,
    0, nullptr,  // color handled by adapter
};

static SpawnGateOutput pyramid_run_gate(Cartridge* c, int32_t gx, int32_t gz) {
    auto gate = c->run_spawn_preamble(gx, gz,
        c->entities_state_.pyramids, Dim::MAX_PYRAMID_INSTANCES,
        PyramidProp::SPAWN_ROLL, PyramidConfig::SPAWN_CHANCE,
        PyramidConfig::MOOD_MULTIPLIER, PopFamily::PYRAMID, "pyr");
    return { gate.ok, gate.seed, gate.slot, gate.theme_idx };
}
static const float* pyramid_get_theme_tier_weights(uint32_t ti) { return THEMES[ti].tier_wt_pyramid; }

static constexpr uint32_t PYRAMID_INDOOR_RESCALE_PARAMS[] = {
    PyrIdx::HEIGHT, PyrIdx::BASE_HALF, PyrIdx::EDGE_BLEND,
    // ASPECT, TRUNCATION are ratios — not scaled.
};

static void pyramid_apply_indoor_rescale(EntityInstance& inst, float ceiling_h) {
    rescale_to_rolled_target(inst, ceiling_h,
        /*target_lo*/ 0.50f, /*target_hi*/ 0.95f,
        /*current_h*/ inst.params[PyrIdx::HEIGHT],
        PYRAMID_INDOOR_RESCALE_PARAMS);
}

static void pyramid_compute_solid_half(EntityInstance& inst, const TierProfile&) {
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

static void pyramid_compute_colors(EntityInstance& inst, const EntityFamilyTraits&, const TierProfile& /*tier*/) {
    // Sandstone only (color_override check is dead code in legacy — both branches identical)
    inst.colors[0] = PYRAMID_SANDSTONE_BASE[0] + (cpu_hash_f(inst.seed, PyramidProp::COLOR_VAR_R) - 0.5f) * PYRAMID_SANDSTONE_VARIANCE * 2.0f;
    inst.colors[1] = PYRAMID_SANDSTONE_BASE[1] + (cpu_hash_f(inst.seed, PyramidProp::COLOR_VAR_G) - 0.5f) * PYRAMID_SANDSTONE_VARIANCE * 2.0f;
    inst.colors[2] = PYRAMID_SANDSTONE_BASE[2] + (cpu_hash_f(inst.seed, PyramidProp::COLOR_VAR_B) - 0.5f) * PYRAMID_SANDSTONE_VARIANCE * 2.0f;
}

static void pyramid_write_active(Cartridge* c, const EntityInstance& inst) {
    auto& ap = c->entities_state_.pyramids[inst.slot];
    ap.patch_gx = inst.trigger_gx; ap.patch_gz = inst.trigger_gz;
    ap.host_gx = inst.host_gx; ap.host_gz = inst.host_gz;
    ap.active = true;
    ap.col_r = inst.colors[0]; ap.col_g = inst.colors[1]; ap.col_b = inst.colors[2];

    // 5-point ground_y: GPU compute shader evaluates the heightfield
    // at center + 4 rotated corners and takes the min. CPU uploads 0.
    ap.cached_ground_y = 0.0f;

    c->entities_state_.pyramid_count++;
}

static void pyramid_write_gpu(Cartridge* c, const EntityInstance& inst, wgpu::Queue& queue) {
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

    // Write mesh gen params
    GPUPyramidMeshParams mp{};
    mp.center_x = inst.cx; mp.center_z = inst.cz;
    mp.rotation = inst.rotation;
    mp.half_x = half_x; mp.half_z = half_z;
    mp.height = inst.params[PyrIdx::HEIGHT];
    mp.truncation = inst.params[PyrIdx::TRUNCATION];
    mp.color_r = inst.colors[0]; mp.color_g = inst.colors[1]; mp.color_b = inst.colors[2];
    mp.is_active = 1;
    c->gpuState_.upload_pyramid_mesh_params_slot(queue, inst.slot, mp);
    c->entities_state_.pyramid_mesh_gen_pending = true;
}

static void pyramid_post_commit(Cartridge* c, const EntityInstance& inst, wgpu::Queue&) {
    // Mark heightfield patches for regen (pyramid gets baked in)
    float half_x = inst.params[PyrIdx::BASE_HALF];
    float half_z = inst.params[PyrIdx::BASE_HALF] * inst.params[PyrIdx::ASPECT];
    float blend  = inst.params[PyrIdx::EDGE_BLEND];
    float cr = std::cos(inst.rotation), sr = std::sin(inst.rotation);
    float abs_cr = std::abs(cr), abs_sr = std::abs(sr);
    float ext_x = (half_x + blend) * abs_cr + (half_z + blend) * abs_sr;
    float ext_z = (half_x + blend) * abs_sr + (half_z + blend) * abs_cr;
    c->mark_patches_for_regen(
        inst.cx - ext_x, inst.cz - ext_z,
        inst.cx + ext_x, inst.cz + ext_z,
        inst.host_gx, inst.host_gz);
}

static constexpr EntityFamilyAdapter PYRAMID_ADAPTER = {
    pyramid_run_gate, pyramid_get_theme_tier_weights,
    pyramid_apply_indoor_rescale,
    pyramid_compute_solid_half, pyramid_compute_colors,
    pyramid_write_active, pyramid_write_gpu, pyramid_post_commit,
    pyramid_get_tier_profile,
};

// ── Pyramid dispatch wrappers ──

static bool dispatch_select_pyramid_generic(Cartridge* self, int32_t gx, int32_t gz, EntityQueueEntry& e) {
    EntityInstance inst{};
    if (!self->generic_select(PYRAMID_TRAITS, PYRAMID_ADAPTER, gx, gz, inst)) return false;
    e.family = PopFamily::PYRAMID; e.gx = gx; e.gz = gz; e.generic = inst; return true;
}
static bool dispatch_place_pyramid_generic(Cartridge* self, EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (self->generic_place(PYRAMID_TRAITS, e.generic)) { pe.generic = e.generic; return true; }
    self->entities_state_.pyramids[e.generic.slot].active = false; return false;
}
static void dispatch_commit_pyramid_generic(Cartridge* self, PlacementEntry& pe, wgpu::Queue& queue) {
    auto* host = find_patch(self, pe.generic.host_gx, pe.generic.host_gz);
    if (host) { self->generic_commit(PYRAMID_TRAITS, PYRAMID_ADAPTER, pe.generic, queue); host->record_entity(PopFamily::PYRAMID, pe.generic.slot); }
    else { self->entities_state_.pyramids[pe.generic.slot].active = false; }
}

// ═══ FAMILIES: SPHERE / CUBE — RELOCATED ═════════════════════════

// ═══ FAMILY: ARCH ═════════════════════════════════════════════════

struct ArchIdx {
    static constexpr uint32_t SPAN         = 0;  // full span (halved in compute_solid_half)
    static constexpr uint32_t RISE         = 1;
    static constexpr uint32_t DEPTH        = 2;
    static constexpr uint32_t THICKNESS    = 3;
    static constexpr uint32_t PIER_HEIGHT  = 4;
    static constexpr uint32_t PIER_PADDING = 5;
    static constexpr uint32_t EDGE_BLEND   = 6;
    static constexpr uint32_t COUNT        = 7;
};

// Floor for SPAN is 1.0 (raw span); compute_solid_half halves it → half_span ≥ 0.5
static constexpr TierParamDef ARCH_PARAM_DEFS[] = {
    { ArchProp::SPAN,         1.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { ArchProp::RISE,         1.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { ArchProp::DEPTH,        0.5f, 1e30f, false, ParamDist::GAUSSIAN },
    { ArchProp::THICKNESS,    0.1f, 1e30f, false, ParamDist::GAUSSIAN },
    { ArchProp::PIER_HEIGHT,  0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { ArchProp::PIER_PADDING, 0.1f, 1e30f, false, ParamDist::GAUSSIAN },
    { ArchProp::EDGE_BLEND,   0.1f, 1e30f, false, ParamDist::GAUSSIAN },
};
static constexpr uint32_t ARCH_PARAM_COUNT = sizeof(ARCH_PARAM_DEFS) / sizeof(TierParamDef);

// params[] order MUST match ARCH_PARAM_DEFS:
//   [0]SPAN [1]RISE [2]DEPTH [3]THICKNESS [4]PIER_HEIGHT [5]PIER_PADDING [6]EDGE_BLEND
//
// Note: name is ArchTierRow, not ArchTier — the enum class ArchTier
// (DOORWAY/STANDARD/MONUMENTAL) already occupies that name in entities.inl.
struct ArchTierRow {
    TierProfile profile;
    float       color_override;
    float       burial;
    uint32_t    segs_u;
    uint32_t    segs_v;
};

static constexpr ArchTierRow ARCH_TIERS[] = {
    /* DOORWAY    */ {
        { 0.50f, 0.0f, { {12.0f, 2.4f}, {12.0f, 2.4f}, {4.5f, 0.9f}, {1.2f, 0.18f}, {1.5f, 0.9f}, {0.9f, 0.3f}, {0.9f, 0.15f} }},
        0.15f, 0.20f, 16, 4
    },
    /* STANDARD   */ {
        { 0.15f, 0.0f, { {50.0f, 15.0f}, {42.0f, 7.0f}, {5.6f, 1.1f}, {1.4f, 0.21f}, {5.6f, 2.1f}, {0.7f, 0.3f}, {0.7f, 0.14f} }},
        0.25f, 0.20f, 32, 8
    },
    /* MONUMENTAL */ {
        { 0.15f, 0.0f, { {60.0f, 10.0f}, {80.0f, 12.0f}, {10.0f, 2.0f}, {2.5f, 0.30f}, {8.0f, 2.5f}, {1.0f, 0.3f}, {0.8f, 0.15f} }},
        0.35f, 0.20f, 48, 12
    },
};

static const TierProfile& arch_get_tier_profile(uint32_t tier_idx) {
    return ARCH_TIERS[tier_idx].profile;
}

static constexpr EntityFamilyTraits ARCH_TRAITS = {
    PopFamily::ARCH, "arch", Dim::MAX_ARCH_INSTANCES,
    true, true, 2,        // grounded, creates ground, 2 piers
    true, 120.0f, 1.5f,
    ArchProp::SPAWN_ROLL, ArchConfig::SPAWN_CHANCE,
    ArchConfig::MOOD_MULTIPLIER, ArchConfig::POSITION_JITTER,
    static_cast<uint32_t>(ArchTier::COUNT), ArchProp::TIER,
    ARCH_PARAM_DEFS, ARCH_PARAM_COUNT,
    ArchProp::POSITION_X, ArchProp::POSITION_Z, ArchProp::ROTATION, true,
    0, nullptr,
};

static SpawnGateOutput arch_run_gate(Cartridge* c, int32_t gx, int32_t gz) {
    auto gate = c->run_spawn_preamble(gx, gz, c->entities_state_.arches, Dim::MAX_ARCH_INSTANCES,
        ArchProp::SPAWN_ROLL, ArchConfig::SPAWN_CHANCE,
        ArchConfig::MOOD_MULTIPLIER, PopFamily::ARCH, "arch");
    return { gate.ok, gate.seed, gate.slot, gate.theme_idx };
}
static const float* arch_get_theme_tier_weights(uint32_t ti) { return THEMES[ti].tier_wt_arch; }

static constexpr uint32_t ARCH_INDOOR_RESCALE_PARAMS[] = {
    ArchIdx::SPAN, ArchIdx::RISE, ArchIdx::DEPTH, ArchIdx::THICKNESS,
    ArchIdx::PIER_HEIGHT, ArchIdx::PIER_PADDING, ArchIdx::EDGE_BLEND,
};

// Arch total height = pier_height + rise (catenary apex). Rolled
// target in [0.50, 0.95] × ceiling_h, scale every length param.
static void arch_apply_indoor_rescale(EntityInstance& inst, float ceiling_h) {
    rescale_to_rolled_target(inst, ceiling_h,
        /*target_lo*/ 0.50f, /*target_hi*/ 0.95f,
        /*current_h*/ inst.params[ArchIdx::PIER_HEIGHT] + inst.params[ArchIdx::RISE],
        ARCH_INDOOR_RESCALE_PARAMS);
}

static void arch_compute_solid_half(EntityInstance& inst, const TierProfile&) {
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

static void arch_compute_colors(EntityInstance& inst, const EntityFamilyTraits&, const TierProfile& /*tier*/) {
    const auto& tp = ARCH_TIERS[inst.tier_idx];
    // Base color: sandstone/palette
    if (cpu_hash_f(inst.seed, ArchProp::COLOR_OVER) < tp.color_override) {
        uint32_t pal_idx = cpu_hash(inst.seed, ArchProp::COLOR_OVER + 1u) % ARCH_PALETTE_COUNT;
        inst.colors[0] = ARCH_PALETTE[pal_idx][0] + (cpu_hash_f(inst.seed, ArchProp::COLOR_VAR_R) - 0.5f) * 0.06f;
        inst.colors[1] = ARCH_PALETTE[pal_idx][1] + (cpu_hash_f(inst.seed, ArchProp::COLOR_VAR_G) - 0.5f) * 0.06f;
        inst.colors[2] = ARCH_PALETTE[pal_idx][2] + (cpu_hash_f(inst.seed, ArchProp::COLOR_VAR_B) - 0.5f) * 0.06f;
    } else {
        inst.colors[0] = ARCH_SANDSTONE_BASE[0] + (cpu_hash_f(inst.seed, ArchProp::COLOR_VAR_R) - 0.5f) * ARCH_SANDSTONE_VARIANCE * 2.0f;
        inst.colors[1] = ARCH_SANDSTONE_BASE[1] + (cpu_hash_f(inst.seed, ArchProp::COLOR_VAR_G) - 0.5f) * ARCH_SANDSTONE_VARIANCE * 2.0f;
        inst.colors[2] = ARCH_SANDSTONE_BASE[2] + (cpu_hash_f(inst.seed, ArchProp::COLOR_VAR_B) - 0.5f) * ARCH_SANDSTONE_VARIANCE * 2.0f;
    }
    // Mesh color defaults to base; portal override applied in write_gpu
    // (needs ActiveArch.is_portal which is set by write_active first)
    inst.colors[3] = inst.colors[0];
    inst.colors[4] = inst.colors[1];
    inst.colors[5] = inst.colors[2];
}

static void arch_write_active(Cartridge* c, const EntityInstance& inst) {
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

    // Ground Y: GPU compute shader samples the heightfield at both pier
    // feet (which already includes pier effect) and takes the min. CPU uploads 0.
    aa.cached_ground_y = 0.0f;

    // Portal state: recompute from seed
    aa.is_portal = false;
    aa.is_back_portal = false;
    aa.position_hash = cpu_hash(inst.seed, ArchProp::ROTATION + 100u);
    aa.destination = PortalDestination{};
    if (inst.tier_idx == static_cast<uint32_t>(ArchTier::DOORWAY)) {
        float portal_roll = cpu_hash_f(inst.seed, ArchProp::ROTATION + 200u);
        if (portal_roll < PORTAL_DENSITY) {
            aa.is_portal = true;
            uint32_t dest_seed = cpu_hash(aa.position_hash, 1u);
            uint32_t mood = pick_portal_mood(c, aa.position_hash, 2u);
            const auto& mp = MOOD_TABLE[mood];
            aa.destination.seed = dest_seed;
            aa.destination.mood = mood;
            aa.destination.finite = mp.finite;
            aa.destination.finite_radius = derive_finite_radius(dest_seed, mp);
        }
    }

    c->entities_state_.arch_count++;
    c->mood_state_.portals_dirty = true;
}

static void arch_write_gpu(Cartridge* c, const EntityInstance& inst, wgpu::Queue& queue) {
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
    mp.color_r    = inst.colors[3]; mp.color_g = inst.colors[4]; mp.color_b = inst.colors[5];
    mp.is_active  = 1;
    c->gpuState_.upload_arch_mesh_params_slot(queue, inst.slot, mp);
    c->entities_state_.arch_mesh_gen_pending = true;
}

static void arch_post_commit(Cartridge* c, const EntityInstance& inst, wgpu::Queue& queue) {
    float half_span    = inst.params[ArchIdx::SPAN];
    float thickness    = inst.params[ArchIdx::THICKNESS];
    float depth        = inst.params[ArchIdx::DEPTH];
    float pier_padding = inst.params[ArchIdx::PIER_PADDING];
    float edge_blend   = inst.params[ArchIdx::EDGE_BLEND];
    float pier_height  = inst.params[ArchIdx::PIER_HEIGHT];

    float pier_half_x = thickness * 0.5f + pier_padding + edge_blend;
    float pier_half_z = depth * 0.5f + pier_padding + edge_blend;

    float cos_r = std::cos(inst.rotation), sin_r = std::sin(inst.rotation);
    float pl_x = inst.cx + (-half_span) * cos_r;
    float pl_z = inst.cz + (-half_span) * sin_r;
    float pr_x = inst.cx + half_span * cos_r;
    float pr_z = inst.cz + half_span * sin_r;

    // Left pier
    uint32_t pier_l_slot = Dim::PIER_ARCH_BASE + inst.slot * 2;
    GPUPierInstance pier_l{};
    pier_l.origin[0] = pl_x; pier_l.origin[1] = pl_z;
    pier_l.half_size[0] = pier_half_x; pier_l.half_size[1] = pier_half_z;
    pier_l.height_near = pier_height; pier_l.height_far = pier_height;
    pier_l.rotation = inst.rotation;
    pier_l.edge_blend = edge_blend;
    pier_l.tier = PierTier::ARCH_DOORWAY + inst.tier_idx;
    pier_l.is_active = 1;
    c->write_pier(queue, pier_l_slot, pier_l);

    // Right pier
    GPUPierInstance pier_r{};
    pier_r.origin[0] = pr_x; pier_r.origin[1] = pr_z;
    pier_r.half_size[0] = pier_half_x; pier_r.half_size[1] = pier_half_z;
    pier_r.height_near = pier_height; pier_r.height_far = pier_height;
    pier_r.rotation = inst.rotation;
    pier_r.edge_blend = edge_blend;
    pier_r.tier = PierTier::ARCH_DOORWAY + inst.tier_idx;
    pier_r.is_active = 1;
    c->write_pier(queue, pier_l_slot + 1, pier_r);

    // Regen AABB
    float reach = std::max(pier_half_x, pier_half_z) + edge_blend;
    c->mark_patches_for_regen(
        std::min(pl_x, pr_x) - reach, std::min(pl_z, pr_z) - reach,
        std::max(pl_x, pr_x) + reach, std::max(pl_z, pr_z) + reach,
        inst.host_gx, inst.host_gz);
}

static constexpr EntityFamilyAdapter ARCH_ADAPTER = {
    arch_run_gate, arch_get_theme_tier_weights,
    arch_apply_indoor_rescale,
    arch_compute_solid_half, arch_compute_colors,
    arch_write_active, arch_write_gpu, arch_post_commit,
    arch_get_tier_profile,
};

static bool dispatch_select_arch_generic(Cartridge* self, int32_t gx, int32_t gz, EntityQueueEntry& e) {
    EntityInstance inst{};
    if (!self->generic_select(ARCH_TRAITS, ARCH_ADAPTER, gx, gz, inst)) return false;
    e.family = PopFamily::ARCH; e.gx = gx; e.gz = gz; e.generic = inst; return true;
}
static bool dispatch_place_arch_generic(Cartridge* self, EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (self->generic_place(ARCH_TRAITS, e.generic)) { pe.generic = e.generic; return true; }
    self->entities_state_.arches[e.generic.slot].active = false; return false;
}
static void dispatch_commit_arch_generic(Cartridge* self, PlacementEntry& pe, wgpu::Queue& queue) {
    auto* host = find_patch(self, pe.generic.host_gx, pe.generic.host_gz);
    if (host) { self->generic_commit(ARCH_TRAITS, ARCH_ADAPTER, pe.generic, queue); host->record_entity(PopFamily::ARCH, pe.generic.slot); }
    else { self->entities_state_.arches[pe.generic.slot].active = false; }
}

// ─── FAMILY_DISPATCH Integration ─────────────────────────────────

// ═══ END entity_pipeline.inl ═════════════════════════════════════
