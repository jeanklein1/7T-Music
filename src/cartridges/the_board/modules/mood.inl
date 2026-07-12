// ─── mood.inl (IMPL: post-class definitions) ─────────────────────
// Impl of mood.hpp (LADDER-4): history in audit/LADDER.md.
//
// Definitions for mood.hpp's declared doors + appliers + derivers.
// The bodies reach the SPINE-OWNED state (c->mood_state_ /
// c->transitionPhase_ / c->pendingDestination_ / c->backPortalPosition_ /
// c->cpuSpotLights_ / c->cpuPortalArray_ / c->sunDirection_ /
// c->sunColor_ / c->clearColor_ / c->world_state_) and the feature-gate
// flags (c->gol_state_.mood_allowed / c->pawn_state_.aura_enabled /
// c->entities_state_.lights_dirty — request-flags, channel-shaped),
// plus the in-class statics (Cartridge::ARCH_TIERS / Cartridge::ArchIdx /
// Cartridge::solve_catenary_a / Cartridge::TransitionPhase) and
// PATCH_EXTENT (patch_system.hpp).
//
// THE CHANNEL: the force-spawn mutation of the arch belongs to the arch's owner.
//
// The indoor lighting-scheme tables live impl-side: single consumer
// (derive_indoor_lights); module-internal authoring tables. The wall
// PALETTES — the other half of SEAM[mood:tuning-data] — live in mood.hpp.
//
// WRAPPING FORM (fix-2): SELF-WRAPPING — the zone includes impls at FILE SCOPE; law in audit/LADDER.md.
// ─────────────────────────────────────────────────────────────────

#include <algorithm>   // std::max, std::min, std::clamp
#include <cmath>       // std::sqrt, std::sin, std::cos, std::cosh, std::floor, std::abs
#include <cstdint>
#include <cstring>     // std::memcpy (anchor-ribbon placement copy)
#include <iostream>    // mood / lighting / shell / portal logs
#include <vector>      // shell mesh staging

namespace t7 {
namespace the_board {

// ── Impl-internal forward declarations ───────────────────────────
// Used before their definitions (which keep their original section
// homes below). Impl-only — not part of the header surface.
inline void force_spawn_finite_portals(Cartridge* c, wgpu::Queue& queue);

// ═══ TUNING DATA (impl-side — internal authoring tables) ═════════
//
// Per-mood indoor lighting schemes. Seed-driven (a per-mood roll picks
// one of the schemes from the table), then per-light parameters are
// sampled from the chosen entry. The indoor wall PALETTES — this
// seam's other half — live declaration-side in mood.hpp
// (SEAM[mood:tuning-data] names the split).

// ── Indoor Lighting Schemes ───────────────────────────────

enum class LightAnchor : uint32_t {
    CEILING, WALL_NORTH, WALL_SOUTH, WALL_EAST, WALL_WEST
};

struct IndoorLightProp {
    static constexpr uint32_t SCHEME = 1100u;
    static constexpr uint32_t WALL_PAIR = 1101u;
    static constexpr uint32_t ANCHOR_PICK = 1102u;
    static constexpr uint32_t SLOT_BASE = 1110u;  // + slot*10 + field
    // Per-slot field offsets
    static constexpr uint32_t LATERAL = 0u;
    static constexpr uint32_t HEIGHT = 1u;
    static constexpr uint32_t INTENSITY = 2u;
    static constexpr uint32_t INNER_CONE = 3u;
    static constexpr uint32_t OUTER_CONE = 4u;
    static constexpr uint32_t WARMTH = 5u;
    static constexpr uint32_t AIM_PITCH = 6u;
    static constexpr uint32_t AIM_YAW = 7u;
};

struct LightSlotDef {
    LightAnchor anchor;
    float intensity_mean, intensity_sigma;
    float inner_mean, inner_sigma;    // inner half-angle (radians)
    float outer_mean, outer_sigma;    // outer half-angle (radians)
    float warmth_mean, warmth_sigma;  // 0 = warm amber, 1 = cool blue
    float aim_pitch_mean, aim_pitch_sigma;  // radians
    float aim_yaw_mean, aim_yaw_sigma;      // radians
    // Anchor-surface position (carried through from LightSchemeSlot).
    float lat_mean, lat_sigma;        // along anchor surface
    float hfrac_mean, hfrac_sigma;    // ceiling: Z; walls: height
};

inline constexpr float SCHEME_WEIGHTS[] = { 0.35f, 0.35f, 0.15f, 0.15f };
inline constexpr uint32_t SCHEME_COUNT = 4;
inline constexpr const char* SCHEME_NAMES[] = { "Cathedral", "Quartet", "Gallery", "Sanctum" };
inline constexpr const char* ANCHOR_NAMES[] = { "ceiling", "wall_N", "wall_S", "wall_E", "wall_W" };

// ── Lighting Scheme Table ──

enum class AnchorRole : uint32_t {
    CEILING,    // always ceiling
    WALL_A,     // seed-selected wall pair, side A
    WALL_B,     // seed-selected wall pair, side B
    SEED_PICK   // anchor chosen from seed (ceiling or wall)
};

struct LightSchemeSlot {
    AnchorRole role;
    float intensity_mean, intensity_sigma;
    float inner_mean, inner_sigma;
    float outer_mean, outer_sigma;
    float warmth_mean, warmth_sigma;
    float aim_pitch_mean, aim_pitch_sigma;
    float aim_yaw_mean, aim_yaw_sigma;
    float lat_mean, lat_sigma;
    float hfrac_mean, hfrac_sigma;
};

struct LightScheme {
    uint32_t slot_count;
    LightSchemeSlot slots[4];  // MAX_SPOT_LIGHTS
};

// ── Scheme Definitions ──
//                                role            int_μ  int_σ  inn_μ inn_σ out_μ out_σ wrm_μ wrm_σ  pit_μ  pit_σ  yaw_μ  yaw_σ   lat_μ lat_σ  hfrac_μ hfrac_σ
inline constexpr LightScheme LIGHT_SCHEMES[SCHEME_COUNT] = {
    /* 0: Cathedral — ceiling primary + 2 wall sconces */
    { 3, {
        { AnchorRole::CEILING,   8.0f, 2.5f, 0.6f, 0.2f,  1.2f, 0.15f, 0.35f, 0.20f,  0.0f,  0.12f, 0.0f, 0.12f,   0.50f, 0.15f, 0.65f, 0.10f },
        { AnchorRole::WALL_A,    5.0f, 1.5f, 0.4f, 0.15f, 1.0f, 0.15f, 0.20f, 0.15f,  0.60f, 0.40f, 0.0f, 0.30f,   0.50f, 0.15f, 0.65f, 0.10f },
        { AnchorRole::WALL_B,    5.0f, 1.5f, 0.4f, 0.15f, 1.0f, 0.15f, 0.75f, 0.15f,  0.60f, 0.40f, 0.0f, 0.30f,   0.50f, 0.15f, 0.65f, 0.10f },
    }},
    /* 1: Quartet — 4 ceiling lights at quadrant corners.
     *    Tight sigma (0.07) keeps each light pinned in its quadrant;
     *    means at 0.30/0.70 produce a 2x2 layout that fills the
     *    room evenly. Per-light intensity dropped to 4.0±1.0 to
     *    keep total scene brightness comparable to Cathedral
     *    (which has ~8.0 from a single ceiling light + sconces). */
    { 4, {
        { AnchorRole::CEILING,   4.0f, 1.0f, 0.6f, 0.2f,  1.2f, 0.15f, 0.35f, 0.20f,  0.0f,  0.10f, 0.0f, 0.10f,   0.30f, 0.07f, 0.30f, 0.07f },
        { AnchorRole::CEILING,   4.0f, 1.0f, 0.6f, 0.2f,  1.2f, 0.15f, 0.35f, 0.20f,  0.0f,  0.10f, 0.0f, 0.10f,   0.70f, 0.07f, 0.30f, 0.07f },
        { AnchorRole::CEILING,   4.0f, 1.0f, 0.6f, 0.2f,  1.2f, 0.15f, 0.35f, 0.20f,  0.0f,  0.10f, 0.0f, 0.10f,   0.30f, 0.07f, 0.70f, 0.07f },
        { AnchorRole::CEILING,   4.0f, 1.0f, 0.6f, 0.2f,  1.2f, 0.15f, 0.35f, 0.20f,  0.0f,  0.10f, 0.0f, 0.10f,   0.70f, 0.07f, 0.70f, 0.07f },
    }},
    /* 2: Gallery — 2 opposing wall lights, no ceiling */
    { 2, {
        { AnchorRole::WALL_A,    7.0f, 2.0f, 0.4f, 0.15f, 1.1f, 0.15f, 0.25f, 0.20f,  0.55f, 0.45f, 0.0f, 0.35f,   0.50f, 0.15f, 0.65f, 0.10f },
        { AnchorRole::WALL_B,    7.0f, 2.0f, 0.4f, 0.15f, 1.1f, 0.15f, 0.65f, 0.20f,  0.55f, 0.45f, 0.0f, 0.35f,   0.50f, 0.15f, 0.65f, 0.10f },
    }},
    /* 3: Sanctum — single dramatic source */
    { 1, {
        { AnchorRole::SEED_PICK, 10.0f, 2.5f, 0.5f, 0.2f, 1.2f, 0.15f, 0.45f, 0.25f,  0.50f, 0.40f, 0.0f, 0.30f,   0.50f, 0.15f, 0.65f, 0.10f },
    }},
};

// ═══ INDOOR LIGHT DERIVATION ═════════════════════════════════════

inline void derive_indoor_lights(Cartridge* c, uint32_t seed, float bmin, float bmax,
    float ceiling_height, CeilingType ceiling_type) {
    c->cpuSpotLights_ = GPUSpotLightArray{};

    float room_size = bmax - bmin;
    float room_range = room_size * 0.8f;

    // Scheme selection
    uint32_t scheme = select_tier(seed, IndoorLightProp::SCHEME,
        SCHEME_WEIGHTS, SCHEME_COUNT);

    // Wall pair: N/S or E/W — gives axis variety across seeds
    bool use_ew = cpu_hash_f(seed, IndoorLightProp::WALL_PAIR) > 0.5f;
    LightAnchor wall_a = use_ew ? LightAnchor::WALL_EAST : LightAnchor::WALL_NORTH;
    LightAnchor wall_b = use_ew ? LightAnchor::WALL_WEST : LightAnchor::WALL_SOUTH;

    // Read slot definitions from scheme table
    const auto& sch = LIGHT_SCHEMES[scheme];
    LightSlotDef slots[MAX_SPOT_LIGHTS];
    uint32_t count = sch.slot_count;

    for (uint32_t i = 0; i < count; i++) {
        const auto& src = sch.slots[i];
        // Resolve anchor role to concrete LightAnchor
        LightAnchor anchor;
        switch (src.role) {
        case AnchorRole::CEILING:   anchor = LightAnchor::CEILING; break;
        case AnchorRole::WALL_A:    anchor = wall_a; break;
        case AnchorRole::WALL_B:    anchor = wall_b; break;
        case AnchorRole::SEED_PICK: {
            float anchor_roll = cpu_hash_f(seed, IndoorLightProp::ANCHOR_PICK);
            anchor = (anchor_roll < 0.55f) ? LightAnchor::CEILING
                : (anchor_roll < 0.775f) ? wall_a : wall_b;
            break;
        }
        }
        slots[i] = { anchor,
            src.intensity_mean, src.intensity_sigma,
            src.inner_mean, src.inner_sigma,
            src.outer_mean, src.outer_sigma,
            src.warmth_mean, src.warmth_sigma,
            src.aim_pitch_mean, src.aim_pitch_sigma,
            src.aim_yaw_mean, src.aim_yaw_sigma,
            src.lat_mean, src.lat_sigma,
            src.hfrac_mean, src.hfrac_sigma };
    }

    // Derive each light from its slot spec + seed
    for (uint32_t i = 0; i < count; i++) {
        const auto& s = slots[i];
        uint32_t base = IndoorLightProp::SLOT_BASE + i * 10;
        auto& L = c->cpuSpotLights_.lights[i];

        float lat = std::clamp(
            cpu_sample_gaussian(seed, base + IndoorLightProp::LATERAL,
                s.lat_mean, s.lat_sigma),
            0.1f, 0.9f);
        float hfrac_raw = cpu_sample_gaussian(seed,
            base + IndoorLightProp::HEIGHT, s.hfrac_mean, s.hfrac_sigma);
        float hfrac = (s.anchor == LightAnchor::CEILING)
            ? std::clamp(hfrac_raw, 0.1f, 0.9f)    // ceiling: Z-axis position
            : std::clamp(hfrac_raw, 0.4f, 0.85f);  // walls: sconce height

        float wall_off = 1.0f;
        float px, py, pz;

        // Position: anchor-dependent placement on surface
        switch (s.anchor) {
        case LightAnchor::CEILING:
            px = bmin + lat * room_size;
            py = ceiling_height - 0.5f;
            pz = bmin + hfrac * room_size;
            break;
        case LightAnchor::WALL_NORTH:
            px = bmin + lat * room_size;
            py = ceiling_height * hfrac;
            pz = bmax - wall_off;
            break;
        case LightAnchor::WALL_SOUTH:
            px = bmin + lat * room_size;
            py = ceiling_height * hfrac;
            pz = bmin + wall_off;
            break;
        case LightAnchor::WALL_EAST:
            px = bmax - wall_off;
            py = ceiling_height * hfrac;
            pz = bmin + lat * room_size;
            break;
        case LightAnchor::WALL_WEST:
            px = bmin + wall_off;
            py = ceiling_height * hfrac;
            pz = bmin + lat * room_size;
            break;
        }

        float pitch = cpu_sample_gaussian(seed, base + IndoorLightProp::AIM_PITCH,
            s.aim_pitch_mean, s.aim_pitch_sigma);
        float yaw = cpu_sample_gaussian(seed, base + IndoorLightProp::AIM_YAW,
            s.aim_yaw_mean, s.aim_yaw_sigma);

        float dx, dy, dz;
        if (s.anchor == LightAnchor::CEILING) {
            // pitch=0 → straight down; positive tilts off-vertical
            pitch = std::clamp(pitch, 0.0f, 0.50f);
            yaw = std::clamp(yaw, -0.60f, 0.60f);
            dx = std::sin(pitch) * std::sin(yaw);
            dy = -std::cos(pitch);
            dz = std::sin(pitch) * std::cos(yaw);
        }
        else {
            // pitch=0 → horizontal into room; π/2 → straight down
            // Floor at 0.08 ensures light never aims upward.
            // Ceiling at 1.45 (~83°) allows steep floor pools.
            pitch = std::clamp(pitch, 0.08f, 1.45f);
            yaw = std::clamp(yaw, -0.80f, 0.80f);
            float cp = std::cos(pitch), sp = std::sin(pitch);
            float sy = std::sin(yaw);
            switch (s.anchor) {
            case LightAnchor::WALL_NORTH: dx = sy; dy = -sp; dz = -cp; break;
            case LightAnchor::WALL_SOUTH: dx = sy; dy = -sp; dz = cp; break;
            case LightAnchor::WALL_EAST:  dx = -cp; dy = -sp; dz = sy; break;
            case LightAnchor::WALL_WEST:  dx = cp; dy = -sp; dz = sy; break;
            default: dx = 0; dy = -1; dz = 0; break;  // unreachable
            }
        }

        // Normalize direction
        float dlen = std::sqrt(dx * dx + dy * dy + dz * dz);
        dx /= dlen; dy /= dlen; dz /= dlen;

        // Intensity and cone angles
        float intensity = std::max(1.0f,
            cpu_sample_gaussian(seed, base + IndoorLightProp::INTENSITY,
                s.intensity_mean, s.intensity_sigma));
        float inner_half = std::max(0.2f,
            cpu_sample_gaussian(seed, base + IndoorLightProp::INNER_CONE,
                s.inner_mean, s.inner_sigma));
        float outer_half = std::max(inner_half + 0.3f,
            cpu_sample_gaussian(seed, base + IndoorLightProp::OUTER_CONE,
                s.outer_mean, s.outer_sigma));
        // Hard cap: outer cone must not exceed shadow map FOV capability.
        // At 1.3 rad (75°), shadow FOV = 2×1.3+0.2 = 2.8 rad → coverable.
        static constexpr float MAX_OUTER_HALF = 1.3f;
        outer_half = std::min(outer_half, MAX_OUTER_HALF);
        inner_half = std::min(inner_half, outer_half - 0.1f);

        // Color: warm amber (1.0,0.85,0.65) ↔ cool blue (0.80,0.88,1.0)
        float w = std::clamp(
            cpu_sample_gaussian(seed, base + IndoorLightProp::WARMTH,
                s.warmth_mean, s.warmth_sigma),
            0.0f, 1.0f);
        float cr = 1.00f + w * (0.80f - 1.00f);
        float cg = 0.85f + w * (0.88f - 0.85f);
        float cb = 0.65f + w * (1.00f - 0.65f);

        L.position[0] = px; L.position[1] = py; L.position[2] = pz;
        L.direction[0] = dx; L.direction[1] = dy; L.direction[2] = dz;
        L.color[0] = cr; L.color[1] = cg; L.color[2] = cb;
        L.intensity = intensity;
        L.inner_cone = std::cos(inner_half);
        L.outer_cone = std::cos(outer_half);
        L.range = (s.anchor == LightAnchor::CEILING)
            ? ceiling_height + 30.0f : room_range;

    }

    c->cpuSpotLights_.count = count;

    // ─── Vault Uplight ───────────────────────────────────────
    //
    if (ceiling_type == CeilingType::VAULT && count < MAX_SPOT_LIGHTS) {
        auto& L = c->cpuSpotLights_.lights[count];
        float center = (bmin + bmax) * 0.5f;
        L.position[0] = center;
        L.position[1] = 2.0f;           // near floor level
        L.position[2] = center;
        L.direction[0] = 0.0f;
        L.direction[1] = 1.0f;           // straight up
        L.direction[2] = 0.0f;
        L.color[0] = 0.95f;
        L.color[1] = 0.90f;
        L.color[2] = 0.80f;              // warm ambient
        L.intensity = 2.5f;               // subtle — structural reveal, not flood
        L.inner_cone = std::cos(0.9f);    // ~52° half-angle
        L.outer_cone = std::cos(1.25f);   // ~72° half-angle — within shadow FOV cap
        L.range = ceiling_height + 80.0f; // reach the crown with headroom
        count++;
        c->cpuSpotLights_.count = count;
        std::cout << "[Lighting] Added vault uplight (slot " << (count - 1) << ")\n";
    }

    std::cout << "[Lighting] " << SCHEME_NAMES[scheme]
        << " (" << count << " lights, "
        << (use_ew ? "E/W" : "N/S") << " walls)\n";
}

// ═══ APPLY MOOD ══════════════════════════════════════════════════

// 1) Atmospheric: sun direction/color/intensity, fog, ambient,
//    terrain amp ceiling. Touches GPU directly + a few member fields.
inline void apply_mood_lighting(Cartridge* c, const MoodProfile& m, wgpu::Queue& /*queue*/) {
    c->sunDirection_[0] = m.sun_direction[0];
    c->sunDirection_[1] = m.sun_direction[1];
    c->sunDirection_[2] = m.sun_direction[2];

    // Push to GPU config so compute_vp builds the shadow VP from the correct direction.
    {
        const float len = std::sqrt(m.sun_direction[0] * m.sun_direction[0] +
                                    m.sun_direction[1] * m.sun_direction[1] +
                                    m.sun_direction[2] * m.sun_direction[2]);
        c->gpuState_.set_sun_direction(m.sun_direction[0] / len,
                                    m.sun_direction[1] / len,
                                    m.sun_direction[2] / len);
    }

    c->sunColor_[0] = m.sun_color[0];
    c->sunColor_[1] = m.sun_color[1];
    c->sunColor_[2] = m.sun_color[2];
    c->mood_state_.sun_intensity = m.sun_intensity;
    c->mood_state_.sun_ambient   = m.sun_ambient;

    c->clearColor_[0] = m.clear_color[0];
    c->clearColor_[1] = m.clear_color[1];
    c->clearColor_[2] = m.clear_color[2];

    c->gpuState_.set_terrain_amp_ceiling(m.indoor ? 0.5f : 0.0f);
    c->mood_state_.terrain_amp_ceiling = m.indoor ? 0.5f : 0.0f;
    c->entities_state_.lights_dirty = true;
}

inline void apply_mood_spot_lights(Cartridge* c, const MoodProfile& m, wgpu::Queue& queue) {
    c->cpuSpotLights_ = GPUSpotLightArray{};
    if (m.indoor) {
        c->gpuState_.set_mute_coupling(Coupling::PAWN_TO_SUN_VP, true);

        const float bmin = -(float)c->world_state_.finite_radius * PATCH_EXTENT;
        const float bmax = ((float)c->world_state_.finite_radius + 1.0f) * PATCH_EXTENT;
        derive_indoor_lights(c, c->world_state_.active_seed, bmin, bmax, m.ceiling_height, m.ceiling_type);

        for (uint32_t i = 0; i < c->cpuSpotLights_.count; i++) {
            compute_spot_light_vp(c->cpuSpotLights_.lights[i],
                                  c->cpuSpotLights_.lights[i].view_proj);
        }
        c->gpuState_.stage_spot_vps(queue, c->cpuSpotLights_);
        c->mood_state_.spot_light_active = true;
    } else {
        c->gpuState_.set_mute_coupling(Coupling::PAWN_TO_SUN_VP, false);
        c->mood_state_.spot_light_active = false;
    }
}

inline void apply_mood_indoor_shell(Cartridge* c, const MoodProfile& m, wgpu::Queue& queue) {
    if (m.indoor && m.ceiling_type != CeilingType::NONE) {
        const uint32_t pal_idx = cpu_hash(c->world_state_.active_seed, 5800u) % INDOOR_PALETTE_COUNT;
        const auto& pal = INDOOR_PALETTES[pal_idx];
        MoodProfile localMood = m;
        for (int c = 0; c < 3; c++) {
            localMood.wall_color[c]    = pal.wall_color[c];
            localMood.ceiling_color[c] = pal.ceiling_color[c];
        }
        std::cout << "[Mood] Indoor palette: " << pal.name
                  << " (idx=" << pal_idx << ")\n";
        generate_indoor_shell(c, queue, localMood);
    } else {
        clear_indoor_shell(c, queue);
    }

    // Camera ceiling clamp (matches crown computation in generate_indoor_shell).
    if (m.indoor) {
        float effective_ceiling = m.ceiling_height;
        if (m.ceiling_type == CeilingType::VAULT) {
            const float bmin = -(float)c->world_state_.finite_radius * PATCH_EXTENT;
            const float bmax = ((float)c->world_state_.finite_radius + 1.0f) * PATCH_EXTENT;
            const float half_span = (bmax - bmin) * 0.5f;
            const float paint_top = m.ceiling_height * 0.45f + 5.5f;
            const float spring_h  = paint_top + 8.0f;
            const float min_rise  = m.ceiling_height - spring_h;
            const float rise = std::max(half_span * 0.30f, std::max(min_rise, 5.0f));
            effective_ceiling = spring_h + rise;
        }
        c->gpuState_.set_ceiling_height(effective_ceiling);
    } else {
        c->gpuState_.set_ceiling_height(0.0f);
    }
}

// 5) Anchor ribbon spawn — only fires when MoodProfile.has_anchor_ribbon.
//    Seed-derived position centered on the finite world; goes through
//    fill_ribbon_selection_geometry + commit_ribbon (the dual-entry
//    site — SEAM[ribbon:dual-entry]).
inline void apply_mood_anchor_ribbon(Cartridge* c, uint32_t mood, wgpu::Queue& queue) {
    if (!MOOD_TABLE[mood].has_anchor_ribbon) return;

    const uint32_t rseed = tile_seed(c->world_state_.active_seed, 0, 0);

    // Anchor: seed-derived position spread across the finite world + margin
    const float spread   = ((float)c->world_state_.finite_radius + 1.5f) * PATCH_EXTENT;
    const float world_cx = 0.5f * PATCH_EXTENT;
    const float world_cz = 0.5f * PATCH_EXTENT;
    const float ax = world_cx + (cpu_hash_f(rseed, RibbonProp::ANCHOR_X) - 0.5f) * spread + c->ribbon_state_.mood_offset[0];
    const float az = world_cz + (cpu_hash_f(rseed, RibbonProp::ANCHOR_Z) - 0.5f) * spread + c->ribbon_state_.mood_offset[1];

    // Tier selection (neutral weights — no theme bias in mood)
    const uint32_t tier_idx = select_tier(rseed, RibbonProp::TIER,
        RIBBON_BASE_TIER_WEIGHTS, RIBBON_TIER_COUNT);

    // Sample geometry through the shared helper (pure from seed; the ground
    // joins once, at head init in state.hpp)
    RibbonSelection sel{};
    sel.seed       = rseed;
    sel.trigger_gx = 0;
    sel.trigger_gz = 0;
    sel.slot       = 0;
    sel.tier_idx   = tier_idx;
    fill_ribbon_selection_geometry(rseed, tier_idx, sel);

    // Build placement (forced position — no negotiation)
    RibbonPlacement plan{};
    plan.slot           = 0;
    plan.seed           = sel.seed;   // wander channels sample this — without it every mood-forced ribbon draws from seed 0
    plan.trigger_gx     = 0;
    plan.trigger_gz     = 0;
    plan.host_gx        = (int32_t)std::floor(ax / PATCH_EXTENT);
    plan.host_gz        = (int32_t)std::floor(az / PATCH_EXTENT);
    plan.tier_idx       = tier_idx;
    plan.cx             = ax;
    plan.cz             = az;
    plan.cube_count     = sel.cube_count;
    plan.cube_size      = sel.cube_size;
    plan.height         = sel.height;
    plan.orientation    = sel.orientation;
    plan.lateral_amp    = sel.lateral_amp;
    plan.lateral_cycles = sel.lateral_cycles;
    plan.vertical_amp   = sel.vertical_amp;
    plan.color_mode     = sel.color_mode;
    std::memcpy(plan.color, sel.color, sizeof(plan.color));
    std::memcpy(plan.color_b, sel.color_b, sizeof(plan.color_b));
    plan.checker_scatter = sel.checker_scatter;
    plan.checker_hue_spread = sel.checker_hue_spread;

    // Commit through the standard path
    commit_ribbon(c->ribbon_state_, c, plan, 0, 0, queue);

    // Immediate GPU upload (per-frame loop may not run before first render)
    c->gpuState_.upload_ribbon(queue, c->ribbon_state_.gpu[0]);
    c->ribbon_state_.rendered_slot = 0;
}

// ── apply_mood (orchestrator) ──
//
inline void apply_mood(Cartridge* c, uint32_t mood, wgpu::Queue& queue) {
    mood = std::min(mood, MOOD_COUNT - 1);
    c->mood_state_.active = mood;
    const auto& m = MOOD_TABLE[mood];

    // Frustum cull is mood-driven (not tied to indoor/outdoor).
    c->renderer_.set_frustum_cull_active(m.allow_frustum_cull);

    // Per-mood feature gates: GoL zones, aura.
    // Aura policy: respect player preference when permitted, force off when forbidden.
    c->gol_state_.mood_allowed     = m.allow_gol_zones;
    if (!m.allow_pawn_aura) c->pawn_state_.aura_enabled = false;

    apply_mood_lighting(c, m, queue);          // sun + fog + amp ceiling (foundational — sun is not a piece)
    if constexpr (ROSTER.spot_lights)          // ROSTER-GATE spot_lights (b) — indoor spot array never configured
        apply_mood_spot_lights(c, m, queue);   // indoor only
    if constexpr (ROSTER.indoor_shell)         // ROSTER-GATE indoor_shell (b) — walls/ceiling never generated
        apply_mood_indoor_shell(c, m, queue);  // shell + camera ceiling clamp
    apply_mood_anchor_ribbon(c, mood, queue);  // SEAM[mood:K4]/[mood:L1] anchor — has_anchor_ribbon only (ribbon-gated inside)
    if constexpr (ROSTER.orbs)                 // ROSTER-GATE orbs (b) — sky dome never configured
        configure_orbs(c->orbs_state_, c, ORB_MOOD_TABLE[mood], queue);

    std::cout << "[Mood] Applied: " << mood_name(mood)
        << " (mood=" << mood
        << (m.indoor ? " INDOOR" : " outdoor")
        << ")\n";
}

// ═══ INDOOR SHELL GENERATION ═════════════════════════════════════

inline void clear_indoor_shell(Cartridge* c, wgpu::Queue& queue) {
    c->gpuState_.set_shell_index_count(0);
    clear_wall_paintings(c->gallery_state_, c, queue);
}

// Helper: push a quad (2 triangles) into vertex/index vectors
inline void push_quad(
    std::vector<ShellVertex>& verts,
    std::vector<uint32_t>& indices,
    float ax, float ay, float az,
    float bx, float by, float bz,
    float cx, float cy, float cz,
    float dx, float dy, float dz,
    float nx, float ny, float nz,
    const float* color
) {
    uint32_t base = static_cast<uint32_t>(verts.size());
    verts.push_back({ { ax, ay, az }, { nx, ny, nz }, { color[0], color[1], color[2] } });
    verts.push_back({ { bx, by, bz }, { nx, ny, nz }, { color[0], color[1], color[2] } });
    verts.push_back({ { cx, cy, cz }, { nx, ny, nz }, { color[0], color[1], color[2] } });
    verts.push_back({ { dx, dy, dz }, { nx, ny, nz }, { color[0], color[1], color[2] } });
    indices.push_back(base); indices.push_back(base + 1); indices.push_back(base + 2);
    indices.push_back(base); indices.push_back(base + 2); indices.push_back(base + 3);
}

inline void generate_indoor_shell(Cartridge* c, wgpu::Queue& queue, const MoodProfile& m) {
    float bmin = -(float)c->world_state_.finite_radius * PATCH_EXTENT;
    float bmax = ((float)c->world_state_.finite_radius + 1.0f) * PATCH_EXTENT;
    float ch = m.ceiling_height;

    std::vector<ShellVertex> verts;
    std::vector<uint32_t> indices;
    verts.reserve(2400);
    indices.reserve(12000);

    static constexpr float JOINT_OVERLAP = 3.0f;
    static constexpr float WALL_FLOOR = -50.0f;

    // ─── Compute wall height (depends on ceiling type) ───────
    //
    float wall_h = ch;  // flat ceiling: walls go to ceiling
    float crown_h = ch; // effective crown height for log
    float rise = 0.0f;

    if (m.ceiling_type == CeilingType::VAULT) {
        static constexpr float VAULT_RISE_FRACTION = 0.30f;
        static constexpr float SPRING_MARGIN = 8.0f;

        float half_span = (bmax - bmin) * 0.5f;
        float paint_center = ch * 0.45f;
        float paint_top = paint_center + 5.5f;
        wall_h = paint_top + SPRING_MARGIN;

        float min_rise = ch - wall_h;
        rise = std::max(half_span * VAULT_RISE_FRACTION, std::max(min_rise, 5.0f));
        crown_h = wall_h + rise;
    }

    float wall_top = wall_h + JOINT_OVERLAP;

    // ─── 4 Walls (deep floor to wall_top) ────────────────────
    // South wall (z = bmin, facing +Z into room)
    push_quad(verts, indices,
        bmin, WALL_FLOOR, bmin, bmax, WALL_FLOOR, bmin,
        bmax, wall_top, bmin, bmin, wall_top, bmin,
        0.0f, 0.0f, 1.0f, m.wall_color);
    // North wall (z = bmax, facing -Z into room)
    push_quad(verts, indices,
        bmax, WALL_FLOOR, bmax, bmin, WALL_FLOOR, bmax,
        bmin, wall_top, bmax, bmax, wall_top, bmax,
        0.0f, 0.0f, -1.0f, m.wall_color);
    // West wall (x = bmin, facing +X into room)
    push_quad(verts, indices,
        bmin, WALL_FLOOR, bmax, bmin, WALL_FLOOR, bmin,
        bmin, wall_top, bmin, bmin, wall_top, bmax,
        1.0f, 0.0f, 0.0f, m.wall_color);
    // East wall (x = bmax, facing -X into room)
    push_quad(verts, indices,
        bmax, WALL_FLOOR, bmin, bmax, WALL_FLOOR, bmax,
        bmax, wall_top, bmax, bmax, wall_top, bmin,
        -1.0f, 0.0f, 0.0f, m.wall_color);

    // ─── Ceiling ─────────────────────────────────────────────
    if (m.ceiling_type == CeilingType::FLAT) {
        push_quad(verts, indices,
            bmin, ch, bmin, bmax, ch, bmin,
            bmax, ch, bmax, bmin, ch, bmax,
            0.0f, -1.0f, 0.0f, m.ceiling_color);
    }
    else if (m.ceiling_type == CeilingType::VAULT) {
        // ─── Groin vault (cross vault) ───────────────────────

        float half_x = (bmax - bmin) * 0.5f;
        float half_z = (bmax - bmin) * 0.5f;
        float center_x = (bmin + bmax) * 0.5f;
        float center_z = (bmin + bmax) * 0.5f;
        float cat_a_x = Cartridge::solve_catenary_a(half_x, rise);
        float cat_a_z = Cartridge::solve_catenary_a(half_z, rise);

        static constexpr uint32_t VAULT_N = 32;

        auto catenary_y = [&](float dist, float cat_a) -> float {
            return crown_h - cat_a * (std::cosh(dist / cat_a) - 1.0f);
            };

        uint32_t v_base = static_cast<uint32_t>(verts.size());
        for (uint32_t iz = 0; iz <= VAULT_N; iz++) {
            float tz = (float)iz / (float)VAULT_N;
            float z = bmin + tz * (bmax - bmin);
            float dz = z - center_z;

            for (uint32_t ix = 0; ix <= VAULT_N; ix++) {
                float tx = (float)ix / (float)VAULT_N;
                float x = bmin + tx * (bmax - bmin);
                float dx = x - center_x;

                float y_barrel_x = catenary_y(dx, cat_a_x);
                float y_barrel_z = catenary_y(dz, cat_a_z);
                float y = std::min(y_barrel_x, y_barrel_z);

                bool on_edge = (ix == 0 || ix == VAULT_N || iz == 0 || iz == VAULT_N);
                if (on_edge) {
                    y = wall_h - JOINT_OVERLAP;
                }

                // Normal via finite differences
                float eps = (bmax - bmin) / (float)VAULT_N * 0.5f;
                float y_px = std::min(catenary_y(dx + eps, cat_a_x), catenary_y(dz, cat_a_z));
                float y_mx = std::min(catenary_y(dx - eps, cat_a_x), catenary_y(dz, cat_a_z));
                float y_pz = std::min(catenary_y(dx, cat_a_x), catenary_y(dz + eps, cat_a_z));
                float y_mz = std::min(catenary_y(dx, cat_a_x), catenary_y(dz - eps, cat_a_z));

                float ddx = (y_px - y_mx) / (2.0f * eps);
                float ddz = (y_pz - y_mz) / (2.0f * eps);
                float nmag = std::sqrt(ddx * ddx + 1.0f + ddz * ddz);
                float fnx = -ddx / nmag;
                float fny = -1.0f / nmag;
                float fnz = -ddz / nmag;

                if (on_edge) { fnx = 0.0f; fny = -1.0f; fnz = 0.0f; }

                verts.push_back({ { x, y, z }, { fnx, fny, fnz },
                    { m.ceiling_color[0], m.ceiling_color[1], m.ceiling_color[2] } });
            }
        }

        for (uint32_t iz = 0; iz < VAULT_N; iz++) {
            for (uint32_t ix = 0; ix < VAULT_N; ix++) {
                uint32_t a = v_base + iz * (VAULT_N + 1) + ix;
                uint32_t b = a + 1;
                uint32_t c = a + (VAULT_N + 1);
                uint32_t d = c + 1;
                indices.push_back(a); indices.push_back(b); indices.push_back(c);
                indices.push_back(b); indices.push_back(d); indices.push_back(c);
            }
        }
    }

    uint32_t vc = std::min(static_cast<uint32_t>(verts.size()), Dim::SHELL_MAX_VERTICES);
    uint32_t ic = std::min(static_cast<uint32_t>(indices.size()), Dim::SHELL_MAX_INDICES);

    c->gpuState_.upload_shell_mesh(queue, verts.data(), vc, indices.data(), ic);

    place_wall_paintings(c->gallery_state_, c, queue, bmin, bmax, ch);

    std::cout << "[Shell] Generated "
        << (m.ceiling_type == CeilingType::FLAT ? "FLAT" : "GROIN VAULT")
        << ": " << vc << " verts, " << ic << " indices"
        << " bounds=[" << bmin << "," << bmax << "]"
        << " wall_h=" << wall_h << " crown=" << crown_h
        << " rise=" << rise << "\n";
}

// ═══ PORTAL SPAWNING ═════════════════════════════════════════════

// ── force_spawn_portal_at ──
//
inline uint32_t force_spawn_portal_at(Cartridge* c, wgpu::Queue& queue,
    float cx, float cz, float rotation,
    const PortalDestination& dest, bool is_back_portal) {
    const float* pc = is_back_portal
        ? PORTAL_COLOR_BACK
        : PORTAL_COLORS[dest.mood % MOOD_COUNT];

    uint32_t slot = force_spawn_portal_arch(c->entities_state_, c, queue,
        cx, cz, rotation, dest, is_back_portal, pc);

    if (slot != UINT32_MAX) c->mood_state_.portals_dirty = true;
    return slot;
}

// ── force_spawn_back_portal ──
//
inline void force_spawn_back_portal(Cartridge* c, wgpu::Queue& queue) {
    c->mood_state_.back_portal_pending = false;

    // ─── Seed-driven placement ───────────────────────────────────────
    //
    if (c->world_state_.finite_mode) {
        float bmin = -(float)c->world_state_.finite_radius * PATCH_EXTENT;
        float bmax = ((float)c->world_state_.finite_radius + 1.0f) * PATCH_EXTENT;
        float room_center = (bmin + bmax) * 0.5f;
        float room_half = (bmax - bmin) * 0.5f;

        float WALL_MARGIN;
        if (MOOD_TABLE[c->mood_state_.active].indoor) {
            const auto& doorway = Cartridge::ARCH_TIERS[static_cast<uint32_t>(ArchTier::DOORWAY)].profile;
            const float doorway_half_span = doorway.params[Cartridge::ArchIdx::SPAN].mean * 0.5f;
            const float doorway_pier_half = doorway.params[Cartridge::ArchIdx::THICKNESS].mean * 0.5f
                + doorway.params[Cartridge::ArchIdx::PIER_PADDING].mean
                + doorway.params[Cartridge::ArchIdx::EDGE_BLEND].mean;
            WALL_MARGIN = INDOOR_ENTITY_WALL_MARGIN
                + doorway_half_span + doorway_pier_half;
        }
        else {
            WALL_MARGIN = 8.0f;
        }

        constexpr float MIN_FROM_ORIGIN = 30.0f;
        constexpr float MIN_FROM_ORIGIN_SQ = MIN_FROM_ORIGIN * MIN_FROM_ORIGIN;

        struct Spot { float x, z, rotation; };
        Spot candidates[4] = {
            { room_center,        bmin + WALL_MARGIN,  1.5708f  },  // south, faces +Z
            { bmax - WALL_MARGIN, room_center,         3.14159f },  // east,  faces -X
            { room_center,        bmax - WALL_MARGIN, -1.5708f  },  // north, faces -Z
            { bmin + WALL_MARGIN, room_center,         0.0f     },  // west,  faces +X
        };

        // Fisher–Yates on the side order, seeded from the world seed.
        uint32_t order[4] = { 0, 1, 2, 3 };
        for (uint32_t i = 3; i > 0; i--) {
            uint32_t j = cpu_hash(c->world_state_.active_seed, 6600u + i) % (i + 1);
            uint32_t tmp = order[i]; order[i] = order[j]; order[j] = tmp;
        }

        bool placed = false;
        float chosen_rotation = 0.0f;
        for (uint32_t k = 0; k < 4 && !placed; k++) {
            uint32_t side = order[k];
            const auto& cand = candidates[side];
            // Jitter along the wall (perpendicular to its inward normal).
            float jitter = (cpu_hash_f(c->world_state_.active_seed, 6610u + side) - 0.5f) * room_half * 0.4f;
            float x = cand.x, z = cand.z;
            if (side == 0 || side == 2) x += jitter;  // S/N walls run along X
            else                          z += jitter; // E/W walls run along Z

            if (x * x + z * z >= MIN_FROM_ORIGIN_SQ) {
                c->backPortalPosition_[0] = x;
                c->backPortalPosition_[1] = z;
                chosen_rotation = cand.rotation;
                placed = true;
            }
        }
        if (!placed) {
            uint32_t side = order[0];
            c->backPortalPosition_[0] = candidates[side].x;
            c->backPortalPosition_[1] = candidates[side].z;
            chosen_rotation = candidates[side].rotation;
        }

        const auto& retMood = MOOD_TABLE[c->mood_state_.back_portal_return_mood % MOOD_COUNT];
        PortalDestination dest{};
        dest.seed = c->mood_state_.back_portal_return_seed;
        dest.finite = retMood.finite;
        dest.finite_radius = c->mood_state_.back_portal_return_radius;
        dest.mood = c->mood_state_.back_portal_return_mood;

        float cx = c->backPortalPosition_[0];
        float cz = c->backPortalPosition_[1];
        uint32_t slot = force_spawn_portal_at(c, queue, cx, cz, chosen_rotation, dest, true);

        if (slot != UINT32_MAX) {
            std::cout << "[Portal] Back-portal spawned at (" << cx << "," << cz
                << ") rot=" << chosen_rotation << " slot=" << slot
                << " -> return seed=" << c->mood_state_.back_portal_return_seed
                << " mood=" << mood_name(c->mood_state_.back_portal_return_mood) << "\n";
        }
        else {
            std::cout << "[Portal] WARNING: no free arch slot for back-portal\n";
        }

        // Spawn additional forward portals around the room perimeter
        force_spawn_finite_portals(c, queue);
        return;
    }

    // ─── Non-finite fallback (open-world back-portal) ───────────────
    // Open worlds don't normally request back-portals, but if they do
    // we keep the legacy fixed-position behavior at backPortalPosition_.
    const auto& retMood = MOOD_TABLE[c->mood_state_.back_portal_return_mood % MOOD_COUNT];
    PortalDestination dest{};
    dest.seed = c->mood_state_.back_portal_return_seed;
    dest.finite = retMood.finite;
    dest.finite_radius = c->mood_state_.back_portal_return_radius;
    dest.mood = c->mood_state_.back_portal_return_mood;

    float cx = c->backPortalPosition_[0];
    float cz = c->backPortalPosition_[1];
    uint32_t slot = force_spawn_portal_at(c, queue, cx, cz, 0.0f, dest, true);

    if (slot != UINT32_MAX) {
        std::cout << "[Portal] Back-portal spawned at (" << cx << "," << cz
            << ") slot=" << slot
            << " -> return seed=" << c->mood_state_.back_portal_return_seed
            << " mood=" << mood_name(c->mood_state_.back_portal_return_mood) << "\n";
    }
    else {
        std::cout << "[Portal] WARNING: no free arch slot for back-portal\n";
    }

    // Spawn additional forward portals around the room perimeter
    force_spawn_finite_portals(c, queue);
}

// ── force_spawn_finite_portals ──
//
inline void force_spawn_finite_portals(Cartridge* c, wgpu::Queue& queue) {
    float bmin = -(float)c->world_state_.finite_radius * PATCH_EXTENT;
    float bmax = ((float)c->world_state_.finite_radius + 1.0f) * PATCH_EXTENT;
    float room_center = (bmin + bmax) * 0.5f;
    float room_half = (bmax - bmin) * 0.5f;

    float margin;
    if (MOOD_TABLE[c->mood_state_.active].indoor) {
        const auto& doorway = Cartridge::ARCH_TIERS[static_cast<uint32_t>(ArchTier::DOORWAY)].profile;
        const float doorway_half_span = doorway.params[Cartridge::ArchIdx::SPAN].mean * 0.5f;
        const float doorway_pier_half = doorway.params[Cartridge::ArchIdx::THICKNESS].mean * 0.5f
            + doorway.params[Cartridge::ArchIdx::PIER_PADDING].mean
            + doorway.params[Cartridge::ArchIdx::EDGE_BLEND].mean;
        margin = INDOOR_ENTITY_WALL_MARGIN
            + doorway_half_span + doorway_pier_half;
    }
    else {
        margin = 8.0f;
    }

    uint32_t count = 1;
    if (c->world_state_.finite_radius >= 2) count = 2;
    if (c->world_state_.finite_radius >= 3) count = 3;

    // Perimeter positions: distribute along the 4 walls
    struct PortalSpot {
        float x, z, rotation;
    };

    // Fixed candidate positions: one per wall, offset from center
    PortalSpot candidates[] = {
        // South wall, facing +Z (into room)
        { room_center, bmin + margin, 1.5708f },
        // East wall, facing -X (into room)
        { bmax - margin, room_center, 3.14159f },
        // North wall, facing -Z (into room)
        { room_center, bmax - margin, -1.5708f },
        // West wall, facing +X (into room)
        { bmin + margin, room_center, 0.0f },
    };
    uint32_t num_candidates = 4;

    // Shuffle candidates with seed so different rooms use different walls
    for (uint32_t i = num_candidates - 1; i > 0; i--) {
        uint32_t j = cpu_hash(c->world_state_.active_seed, 7700u + i) % (i + 1);
        PortalSpot tmp = candidates[i];
        candidates[i] = candidates[j];
        candidates[j] = tmp;
    }

    uint32_t spawned = 0;
    for (uint32_t i = 0; i < num_candidates && spawned < count; i++) {
        auto& spot = candidates[i];

        // Jitter position along the wall
        float jitter = (cpu_hash_f(c->world_state_.active_seed, 7710u + i) - 0.5f) * room_half * 0.4f;
        // Apply jitter perpendicular to the wall normal
        float jx = spot.x, jz = spot.z;
        if (std::abs(spot.rotation - 1.5708f) < 0.1f || std::abs(spot.rotation + 1.5708f) < 0.1f) {
            jx += jitter;  // south/north wall: jitter along X
        }
        else {
            jz += jitter;  // east/west wall: jitter along Z
        }

        // Don't collide with back-portal (at backPortalPosition_)
        float dbx = jx - c->backPortalPosition_[0];
        float dbz = jz - c->backPortalPosition_[1];
        if (dbx * dbx + dbz * dbz < 10.0f * 10.0f) continue;

        // Generate destination
        uint32_t dest_seed = cpu_hash(c->world_state_.active_seed, 7800u + i);
        uint32_t mood = pick_portal_mood(c, c->world_state_.active_seed, 7900u + i);
        const auto& mp = MOOD_TABLE[mood];
        PortalDestination dest{};
        dest.seed = dest_seed;
        dest.mood = mood;
        dest.finite = mp.finite;
        dest.finite_radius = derive_finite_radius(dest_seed, mp);

        uint32_t slot = force_spawn_portal_at(c, queue, jx, jz, spot.rotation, dest, false);
        if (slot != UINT32_MAX) {
            std::cout << "[Portal] Forward portal " << (spawned + 1)
                << " at (" << jx << "," << jz
                << ") -> seed=" << dest_seed
                << " mood=" << mood_name(mood)
                << (dest.finite ? " FINITE" : " open") << "\n";
            spawned++;
        }
    }

    std::cout << "[Portal] Finite world: " << spawned << " forward portals + 1 back-portal\n";
}

// ═══ PER-FRAME UPLOAD ════════════════════════════════════════════

// ── upload_portal_array ──
inline void upload_portal_array(Cartridge* c, wgpu::Queue& queue) {
    if (!c->mood_state_.portals_dirty) return;
    c->mood_state_.portals_dirty = false;

    c->cpuPortalArray_ = GPUPortalArray{};
    uint32_t count = 0;
    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES && count < MAX_GPU_PORTALS; i++) {
        if (!c->entities_state_.arches[i].active || !c->entities_state_.arches[i].is_portal) continue;
        const auto& aa = c->entities_state_.arches[i];
        auto& entry = c->cpuPortalArray_.portals[count];
        entry.x = aa.world_x;
        entry.z = aa.world_z;
        entry.facing_cos = std::cos(aa.rotation);
        entry.facing_sin = std::sin(aa.rotation);
        float half_depth = aa.depth * 0.5f;
        entry.inv_span_sq = 1.0f / (aa.half_span * aa.half_span);
        entry.inv_depth_sq = 1.0f / (half_depth * half_depth);
        entry.arch_index = i;
        entry._pad = 0;
        count++;
    }
    c->cpuPortalArray_.count = count;
    c->gpuState_.upload_portal_array(queue, c->cpuPortalArray_);
}

// ── upload_lights ──
// (Must precede compute for shadow VP.)
inline void upload_lights(Cartridge* c, wgpu::Queue& queue) {
    if (!c->entities_state_.lights_dirty) return;
    c->entities_state_.lights_dirty = false;

    GPUDirectionalLight sun{};
    float len = std::sqrt(c->sunDirection_[0] * c->sunDirection_[0] + c->sunDirection_[1] * c->sunDirection_[1] + c->sunDirection_[2] * c->sunDirection_[2]);
    sun.direction[0] = c->sunDirection_[0] / len;
    sun.direction[1] = c->sunDirection_[1] / len;
    sun.direction[2] = c->sunDirection_[2] / len;

    sun.color[0] = c->sunColor_[0];
    sun.color[1] = c->sunColor_[1];
    sun.color[2] = c->sunColor_[2];
    sun.intensity = c->mood_state_.sun_intensity;
    sun.ambient = c->mood_state_.sun_ambient;

    c->gpuState_.upload_directional_light(queue, sun);

    GPUPointLightArray pointLights{};
    pointLights.count = 0;
    c->gpuState_.upload_point_lights(queue, pointLights);

    c->gpuState_.upload_spot_lights(queue, c->cpuSpotLights_);
}

// ═══ MOOD TRANSITION REQUEST ═════════════════════════════════════
//
inline void request_mood_transition(Cartridge* c, uint32_t mood) {
    // ROSTER-GATE transitions (b) — ENTRY door #1 (keyboard mood requests).
    // Disabled: the machine stays at IDLE forever; the bit gates requests,
    // nothing structural. Maturity-proof form of the transitions=>portal
    // edge (the v0 form is the manifest static_assert).
    if constexpr (!ROSTER.transitions) { (void)c; (void)mood; return; }
    if (c->transitionPhase_ != Cartridge::TransitionPhase::IDLE) return;
    if (mood >= MOOD_COUNT) return;

    const auto& mp = MOOD_TABLE[mood];
    uint32_t dest_seed = cpu_hash(c->world_state_.active_seed, 999u);
    uint32_t radius = derive_finite_radius(dest_seed, mp);
    c->pendingDestination_ = { dest_seed, mp.finite, radius, mood };
    c->transitionPhase_ = Cartridge::TransitionPhase::FADE_OUT;
    c->mood_state_.transition_timer = 0.0f;

    if (mp.finite) {
        uint32_t side = 2 * radius + 1;
        std::cout << "[World] Transition (" << mood_name(mood) << " "
            << side << "x" << side << "): seed " << c->world_state_.active_seed
            << " -> " << c->pendingDestination_.seed << "\n";
    } else {
        std::cout << "[World] Transition (" << mood_name(mood) << "): seed "
            << c->world_state_.active_seed << " -> " << c->pendingDestination_.seed << "\n";
    }
}

// ═══ DERIVERS ════════════════════════════════════════════════════

inline const char* mood_name(uint32_t mood) {
    // Sized array: the compiler catches an EXTRA entry past
    // MOOD_COUNT, but not a missing one — it zero-fills to nullptr.
    static const char* NAMES[MOOD_COUNT] = {
        "open_default", "open_sunset", "indoor_flat",
        "indoor_vault", "finite_outdoor", "finite_outdoor_ref"
    };
    return (mood < MOOD_COUNT) ? NAMES[mood] : "unknown";
}

// Derive finite world radius from seed within mood-defined bounds.
inline uint32_t derive_finite_radius(uint32_t seed, const MoodProfile& mood) {
    if (mood.finite_radius_min >= mood.finite_radius_max) return mood.finite_radius_min;
    uint32_t range = mood.finite_radius_max - mood.finite_radius_min + 1;
    return mood.finite_radius_min + cpu_hash(seed, 77u) % range;
}

inline uint32_t pick_portal_mood(Cartridge* c, uint32_t seed, uint32_t prop) {
    float roll = cpu_hash_f(seed, prop);
    if (c->world_state_.finite_mode) {
        if (roll < 0.20f) return 0;
        if (roll < 0.40f) return 1;
        if (roll < 0.55f) return 2;
        if (roll < 0.70f) return 3;
        if (roll < 0.85f) return 4;
        return 5;
    }
    return cpu_hash(seed, prop) % MOOD_COUNT;
}

} // namespace the_board
} // namespace t7
