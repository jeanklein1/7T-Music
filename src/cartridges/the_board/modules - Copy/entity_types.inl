// ─── entity_types.inl ────────────────────────────────────────────
//
// Core type definitions for the generic entity pipeline.
// Included BEFORE EntityQueueEntry/PlacementEntry so EntityInstance
// can be a union member. The implementation (generic functions,
// family data, adapters, dispatch wrappers) lives in entity_pipeline.inl,
// included AFTER the unions.
//
// Included inside the Cartridge class body.
// ─────────────────────────────────────────────────────────────────

static constexpr uint32_t MAX_ENTITY_PARAMS = 32;
static constexpr uint32_t MAX_COLOR_CHANNELS = 12;

enum class ParamDist : uint32_t {
    GAUSSIAN,
    UNIFORM_01,
    UNIFORM_TAU,
};

struct TierParamDef {
    uint32_t   prop;
    float      floor;
    float      ceiling;    // upper clamp (1e30 = no ceiling)
    bool       do_round;
    ParamDist  dist;
};

struct TierMuSigma {
    float mean, sigma;
};

struct TierProfile {
    float          weight;
    TierMuSigma    params[MAX_ENTITY_PARAMS];
};

struct ColorPartDef {
    float    base[3];
    float    variance;
    uint32_t prop_base;
    uint32_t prop_offset;
};

struct EntityFamilyTraits {
    uint32_t    family_id;
    const char* short_name;
    uint32_t    max_instances;
    bool        grounded;
    bool        creates_ground;
    uint32_t    piers_per_entity;
    bool        has_footprint;
    float       cull_base;
    float       cull_height_scale;
    uint32_t    spawn_roll_prop;
    float       spawn_chance;
    const float* mood_multiplier;
    float       position_jitter;
    uint32_t    tier_count;
    uint32_t    tier_prop;
    const TierParamDef* param_defs;
    uint32_t    param_count;
    const TierProfile*  tier_profiles;
    uint32_t    pos_x_prop;
    uint32_t    pos_z_prop;
    uint32_t    rotation_prop;
    bool        gpu_ground_y;       // true = GPU compute corrects ground_y (CPU uploads offset only)
    uint32_t    color_part_count;
    const ColorPartDef* color_parts;
};

struct SpawnGateOutput {
    bool     ok;
    uint32_t seed;
    uint32_t slot;
    uint32_t theme_idx;
};

struct EntityInstance {
    uint32_t family_id   = 0;
    uint32_t seed        = 0;
    int32_t  trigger_gx  = 0, trigger_gz = 0;
    int32_t  host_gx     = 0, host_gz    = 0;
    uint32_t slot        = 0;
    uint32_t tier_idx    = 0;
    uint32_t theme_idx   = 0;
    float    cx       = 0.0f, cz = 0.0f;
    float    rotation = 0.0f;
    float    params[MAX_ENTITY_PARAMS]{};
    float    solid_half       = 0.0f;
    float    cached_ground_y  = 0.0f;
    float    ground_y_offset  = 0.0f;  // added to terrain Y (e.g. solid_height for pier entities)
    float    burial           = 0.0f;
    float    colors[MAX_COLOR_CHANNELS]{};
};

struct EntityFamilyAdapter {
    SpawnGateOutput (*run_gate)(Cartridge* c, int32_t gx, int32_t gz);
    const float* (*get_theme_tier_weights)(uint32_t theme_idx);
    void (*compute_solid_half)(EntityInstance& inst, const TierProfile& tier);
    void (*compute_colors)(EntityInstance& inst, const EntityFamilyTraits& traits);
    void (*write_active)(Cartridge* c, const EntityInstance& inst);
    void (*write_gpu)(Cartridge* c, const EntityInstance& inst, wgpu::Queue& queue);
    void (*post_commit)(Cartridge* c, const EntityInstance& inst, wgpu::Queue& queue);
};

// ═══ END entity_types.inl ════════════════════════════════════════
