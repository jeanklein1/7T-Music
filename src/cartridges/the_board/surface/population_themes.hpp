#pragma once
#include <cstdint>
#include "cartridges/the_board/contracts/roster.hpp"                // PopFamily (spawn_weight indexing)
#include "cartridges/the_board/primitives/seed_utils.hpp"    // cpu_hash_f (theme rolls)
// keyhole include RETIRED at the merge (d3 #1) — nothing here names Cartridge;
// MachineCtx arrives from contracts/entity_types.hpp earlier in the cohort.

// ─── population_themes.hpp (S2 · HEADER: vocabulary + state + decls) ─
// Born at LADDER-6 (S2 extraction): history in audit/LADDER.md.
//
// Compositional intent per region: what spawns, how densely, in which
// tier mix — THEMES rows selected per patch by the envelope machine.
//
// The impl reaches the keyhole only for the flag-gated census dump.

namespace t7 {
namespace the_board {

inline constexpr float THEME_LATTICE_SPACING = 500.0f;
inline constexpr uint32_t THEME_SEED_BAND = 170u;
inline constexpr uint32_t THEME_COUNT = 5;
inline constexpr float THEME_BASE_WEIGHT = 10.0f;

struct ThemeEnvelope {
    int32_t  active = -1;              // theme index, or -1 (no bias)
    uint32_t elapsed = 0;               // patches since this theme fired
    uint32_t cooldowns[THEME_COUNT]{};  // per-theme remaining cooldown
};

struct PopulationTheme {
    float spawn_weight[PopFamily::COUNT];          // multiplier on base spawn chance per family
    float tier_wt_pyramid[3];                      // multiplier on pyramid tier base weights
    float tier_wt_arch[3];                         // multiplier on arch tier base weights
    float tier_wt_column[3];                       // multiplier on column tier base weights (Pillar, Doric, Ornate)
    float tier_wt_antenna[3];                      // multiplier on antenna tier base weights (Antenna, Squat, Colossal)
    float tier_wt_palm[3];                         // multiplier on palm tier base weights (Sapling, Coastal, Royal)
    float tier_wt_cactus[3];                       // multiplier on cactus tier base weights (Finger, Saguaro, Candelabra)
    float tier_wt_blade[3];                        // multiplier on blade tier base weights (Sprout, Clump, Thicket)
    float tier_wt_sphere[2];                       // multiplier on sphere tier base weights (Sentinel, Anomaly)
    float tier_wt_ribbon[3];                       // multiplier on ribbon tier base weights (Serpentine, Helix, Streamer)
    float tier_wt_cube[4];                         // multiplier on cube tier base weights (SmCube..Monolith)
    float density_mult;                            // multiplier on entity_density

    // Envelope parameters (replace lattice weight for theme selection)
    float    spike;
    uint32_t sustain;       // patches at full spike
    uint32_t decay;         // patches for linear decay to base
    uint32_t cooldown;      // patches before re-eligible after expiry

    // Lattice node weight (spatial distribution of themes)
    float weight;
};

// THE S2/S3 BOUNDARY FACE: THEMES is read across the boundary by the
// theme_tier_weights accessor (the interface trio's vocabulary member —
// theory v2 §4; formalized at the A-era; Q5 unified the per-family plugs).
inline constexpr PopulationTheme THEMES[THEME_COUNT] = {
    // ── 0: TRANSITION — sparse connective tissue ─────────────────
    {   { 0.4f, 0.3f, 0.7f, 0.3f, 0.3f, 0.3f, 0.5f, 0.3f, 1.0f, 0.5f, 0.5f, 0.5f },   // spawn_weight [pyr..sph, ribn, cube, gol, gall]
        { 1.0f, 1.0f, 1.0f },                                       // tier_pyr
        { 1.0f, 0.3f, 1.0f },                                       // tier_arch
        { 0.1f, 0.2f, 0.3f },                                       // tier_col
        { 0.1f, 2.0f, 0.7f },                                       // tier_ant
        { 1.0f, 1.0f, 1.0f },                                       // tier_palm
        { 1.0f, 1.0f, 1.0f },                                       // tier_cactus
        { 1.0f, 1.0f, 1.0f },                                       // tier_blade
        { 1.0f, 1.0f },                                              // tier_sphere (neutral)
        { 1.0f, 1.0f, 1.0f },                                       // tier_ribbon (neutral)
        { 1.0f, 1.0f, 1.0f, 1.0f },                                 // tier_cube (neutral)
        1.0f,                                                         // density
        150.0f, 20u, 3u, 0u,                                          // spike, sustain, decay, cooldown
        0.21f                                                         // weight
    },
    // ── 1: MONUMENTAL — big pyramids, varied arches, heavy columns
    {   { 1.5f, 1.0f, 1.0f, 0.5f, 0.2f, 0.2f, 0.5f, 0.3f, 1.0f, 0.3f, 0.3f, 0.3f },
        { 0.2f, 0.5f, 3.0f },
        { 2.0f, 0.1f, 3.0f },
        { 0.01f, 0.01f, 1.0f },
        { 0.5f, 1.5f, 0.5f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f },
        1.0f,
        150.0f, 10u, 10u, 8u,
        0.30f
    },
    // ── 2: COLONNADE — dense columns, moderate arches ────────────
    {   { 0.3f, 1.0f, 4.0f, 0.5f, 0.3f, 0.3f, 0.5f, 0.3f, 1.0f, 0.3f, 0.5f, 0.4f },
        { 1.0f, 1.0f, 1.0f },
        { 3.0f, 0.5f, 1.0f },
        { 0.3f, 3.0f, 5.0f },
        { 0.2f, 0.1f, 0.1f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f },
        1.0f,
        150.0f, 15u, 6u, 6u,
        0.31f
    },
    // ── 3: ANTENNA — antenna-dominant corridor ───────────────────
    {   { 0.5f, 0.5f, 1.0f, 4.0f, 0.5f, 0.5f, 0.5f, 0.3f, 1.0f, 0.3f, 0.3f, 0.3f },
        { 1.0f, 0.05f, 2.0f },
        { 1.0f, 0.2f, 0.8f },
        { 0.1f, 0.3f, 0.3f },
        { 0.5f, 3.5f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f },
        1.0f,
        180.0f, 10u, 5u, 5u,
        0.18f
    },
    // ── 4: BARREN — near-empty ───────────────────────────────────
    {   { 0.4f, 0.3f, 0.5f, 0.3f, 0.2f, 0.2f, 0.1f, 0.3f, 1.0f, 0.1f, 0.2f, 0.6f },
        { 2.0f, 0.5f, 0.2f },
        { 1.0f, 1.0f, 1.0f },
        { 0.2f, 0.5f, 0.5f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f },
        1.0f,
        100.0f, 12u, 3u, 4u,
        0.04f
    },
};

// ═══ MODULE STATE ══════════════════════════════════════════════════

// The envelope machine's working state + the per-patch selection.
// Instance (themes_state_) lives at the composition root.
struct ThemesState {
    ThemeEnvelope envelope_{};
    uint32_t temporal_flavor = 0;   // Q7 TEMPORAL axis: the drifting theme index (drives tier weights), set per-patch by evaluate_theme_envelope. Independent of spatial_density — a different axis, not a duplicate.
};

// ═══ MODULE FUNCTIONS ══════════════════════════════════════════════

// Select a theme at a lattice node from cumulative weights
// Q5: the ONE tier-weight accessor over the family→member map. Replaces the
// per-family *_get_theme_tier_weights plugs — the generic pipeline calls it
// with traits.family_id (a PopFamily). Tables untouched; bit-safe (returns
// the same const float* into the PopulationTheme row the old plug did).
inline const float* theme_tier_weights(uint32_t theme_idx, uint32_t family_id) {
    const PopulationTheme& th = THEMES[theme_idx];
    switch (family_id) {
        case PopFamily::PYRAMID: return th.tier_wt_pyramid;
        case PopFamily::ARCH:    return th.tier_wt_arch;
        case PopFamily::COLUMN:  return th.tier_wt_column;
        case PopFamily::ANTENNA: return th.tier_wt_antenna;
        case PopFamily::PALM:    return th.tier_wt_palm;
        case PopFamily::CACTUS:  return th.tier_wt_cactus;
        case PopFamily::BLADE:   return th.tier_wt_blade;
        case PopFamily::SPHERE:  return th.tier_wt_sphere;
        case PopFamily::RIBBON:  return th.tier_wt_ribbon;
        case PopFamily::CUBE:    return th.tier_wt_cube;
        default:                 return th.tier_wt_column;
    }
}

// Q4: the SPATIAL theme selector. Its bucket walk stays SEPARATE from
// select_weighted (seed_utils) BY DESIGN — it normalizes inline against the
// live theme weight-sum (select_weighted takes pre-normalized weights), and
// together with evaluate_theme_envelope (the temporal, stateful-sequenced
// selector) it forms the theme-sampler family — a different shape from the
// stateless entity bucket walk. Documented-not-migrated (Q4 ruling).
inline uint32_t select_theme_at_node(uint32_t node_seed) {
    float roll = cpu_hash_f(node_seed, 370u);
    float cumul = 0.0f;
    float total = 0.0f;
    for (uint32_t t = 0; t < THEME_COUNT; t++) total += THEMES[t].weight;
    for (uint32_t t = 0; t < THEME_COUNT; t++) {
        cumul += THEMES[t].weight / total;
        if (roll < cumul) return t;
    }
    return THEME_COUNT - 1;
}

inline float theme_envelope_weight(const PopulationTheme& theme, uint32_t elapsed) {
    if (elapsed < theme.sustain) return theme.spike;
    if (elapsed < theme.sustain + theme.decay) {
        float t = (float)(elapsed - theme.sustain) / (float)theme.decay;
        return theme.spike + (THEME_BASE_WEIGHT - theme.spike) * t;
    }
    return THEME_BASE_WEIGHT;
}

// Called ONCE per patch, inside the spawn loop, BEFORE per-family gates.
// Returns the theme index to use for this patch. DEFINED in
// population_themes.inl (post-class — the flag-gated census dump
// reaches the keyhole).
uint32_t evaluate_theme_envelope(ThemesState& ts, MachineCtx* c, uint32_t tile_seed_value);  // stores its own result (m4); takes the face (d2)
void reset_theme_envelope(ThemesState& ts);


// ═══ IMPL (merged from population_themes.inl — DISSOLVE-1 d3, merge
// #1; the parked definition met its condition: ZERO keyhole residue
// in any signature, no dispatch rows). COHORT PROOF: every callee is
// declared earlier in the cohort — cpu_hash_f (seed_utils, contracts
// side), THEMES/theme_envelope_weight (above), EXCEPT the DIAG-gated
// census dump, fwd-declared here under its own flag (spawn_engine.hpp
// follows this header in the cohort; disclosed per the cohort law).
// The keyhole include is RETIRED with the merge — nothing here names
// Cartridge. MachineCtx's type precedes via contracts/entity_types.hpp
// (the patch_system.hpp cohort precedent). ═══════════════════════════

#ifdef DIAG_ENTITY_CENSUS
void dump_entity_census(MachineCtx* c, const char* trigger);  // fwd (cohort law disclosure)
#endif

// Census home (LADDER-6 3b): sole consumer is the census dump below;
// the names are this module's vocabulary.
inline const char* theme_short_name(uint32_t theme) {
    static const char* NAMES[] = { "transition", "monumental", "colonnade", "antenna", "barren" };
    return (theme < THEME_COUNT) ? NAMES[theme] : "???";
}

inline uint32_t evaluate_theme_envelope(ThemesState& ts, MachineCtx* c, uint32_t tile_seed_value) {
    (void)c;
    auto& env = ts.envelope_;

    // Build effective weights
    float weights[THEME_COUNT];
    float total = 0.0f;
    for (uint32_t i = 0; i < THEME_COUNT; i++) {
        if (env.cooldowns[i] > 0) {
            weights[i] = 0.0f;
        }
        else if ((int32_t)i == env.active) {
            weights[i] = theme_envelope_weight(THEMES[i], env.elapsed);
        }
        else {
            weights[i] = THEME_BASE_WEIGHT;
        }
        total += weights[i];
    }
    if (total < 0.001f) total = 1.0f;

    // Roll from weights
    float roll = cpu_hash_f(tile_seed_value, 370u);
    uint32_t selected = THEME_COUNT - 1;
    float cumul = 0.0f;
    for (uint32_t i = 0; i < THEME_COUNT; i++) {
        cumul += weights[i] / total;
        if (roll < cumul) { selected = i; break; }
    }

    // State transitions
    if ((int32_t)selected != env.active) {
        if (env.active >= 0) {
            env.cooldowns[env.active] = THEMES[env.active].cooldown;
        }
        env.active = (int32_t)selected;
        env.elapsed = 0;

        // Census dump on theme transition
#ifdef DIAG_ENTITY_CENSUS
        dump_entity_census(c, theme_short_name(selected));
#endif
    }
    else {
        env.elapsed++;
    }

    // Check expiry
    if (env.active >= 0) {
        const auto& th = THEMES[env.active];
        if (env.elapsed >= th.sustain + th.decay) {
            env.cooldowns[env.active] = th.cooldown;
            env.active = -1;
            env.elapsed = 0;
        }
    }

    // Tick cooldowns
    for (uint32_t i = 0; i < THEME_COUNT; i++) {
        if (env.cooldowns[i] > 0) env.cooldowns[i]--;
    }

    ts.temporal_flavor = selected;  // stores its own result (m4) — the caller no longer writes the organ
    return selected;
}

// ─── Teardown reset (owner verb; REBUILD-0 m4) ────────────────────
inline void reset_theme_envelope(ThemesState& ts) {
    ts = ThemesState{};
}


} // namespace the_board
} // namespace t7
