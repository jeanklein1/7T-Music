#pragma once
#include <cstdint>
#include "cartridges/the_board/contracts/roster.hpp"                // PopFamily (spawn_weight indexing)
#include "cartridges/the_board/primitives/seed_utils.hpp"    // cpu_hash_f (theme rolls)
#include "cartridges/the_board/contracts/keyhole.hpp"       // Cartridge fwd (the keyhole)

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
// per-family get_theme_tier_weights adapters (the interface trio's
// vocabulary member — theory v2 §4; formalized at the A-era).
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
    uint32_t active_theme_idx_ = 0;   // set per-patch by evaluate_theme_envelope
};

// ═══ MODULE FUNCTIONS ══════════════════════════════════════════════

// Select a theme at a lattice node from cumulative weights
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
uint32_t evaluate_theme_envelope(ThemesState& ts, Cartridge* c, uint32_t tile_seed_value);

} // namespace the_board
} // namespace t7
