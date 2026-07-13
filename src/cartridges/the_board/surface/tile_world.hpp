#pragma once
#include <cstdint>
#include <cstddef>        // size_t (GridKeyHash)
#include <unordered_map>  // the tile cache
#include "cartridges/the_board/contracts/roster.hpp"                // PopFamily (TileState theme columns)
#include "cartridges/the_board/contracts/keyhole.hpp"       // Cartridge + wgpu::Queue fwds (the keyhole)

// ─── tile_world.hpp (S2 · HEADER: vocabulary + state + decls) ──────
// Born at LADDER-6 (S2 extraction; tokens merged by the stamped map):
// history in audit/LADDER.md.
//
// What the terrain remembers: per-tile archetype character (amp/bias/
// activation), the entity-density and theme fields sampled at
// generation, and the terrain tokens that carry landform momentum
// from tile to tile.

namespace t7 {
namespace the_board {

// ── Archetypes ─────────────────────────────────────────────────────

inline constexpr uint32_t ARCHETYPE_COUNT = 4;

struct ArchetypeProfile {
    // ─── Terrain modifiers ───────────────────────────────
    float amp_scale;           // height field amplitude multiplier
    float height_bias;         // vertical offset (positive = elevated)
    float activation_scale;    // activity field sensitivity

    // ─── Selection ───────────────────────────────────────
    float base_weight;         // prior probability (before neighbor influence)

    // ─── Per-tile jitter ─────────────────────────────────
    float amp_jitter_range;    // amp_scale *= 1 ± jitter/2
    float bias_jitter_range;   // height_bias += uniform(-jitter/2, +jitter/2)
};

//                                     amp   bias   act   weight  amp_jit  bias_jit
inline constexpr ArchetypeProfile ARCHETYPES[ARCHETYPE_COUNT] = {
    /* 0: mountainous */  {  2.0f,   4.0f,  0.7f,  1.8f,   0.3f,    1.0f  },
    /* 1: varied      */  {  1.0f,   0.0f,  1.0f,  1.3f,   0.3f,    1.0f  },
    /* 2: basin       */  {  0.5f,  -2.0f,  1.3f,  1.0f,   0.3f,    1.0f  },
    /* 3: pool        */  {  0.04f, -0.5f,  0.2f,  0.0f,   0.02f,   0.2f  },
};

struct ArchetypeSelectionRules {
    // Neighbor count thresholds and corresponding weight multipliers.
    // Applied in order: first matching threshold wins.
    static constexpr uint32_t DOMINANT_THRESHOLD = 4;    // >= this many → suppress
    static constexpr float    DOMINANT_MULTIPLIER = 0.2f; // strongly reduced
    static constexpr uint32_t COMMON_THRESHOLD = 2;    // >= this many → mild boost
    static constexpr float    COMMON_MULTIPLIER = 1.5f;
    static constexpr uint32_t PRESENT_THRESHOLD = 1;    // == this many → strong coherence
    static constexpr float    PRESENT_MULTIPLIER = 2.0f;
    // 0 neighbors: weight stays at base_weight (no modification)
};

// ── Entity Density Field ───────────────────────────────────────────

inline constexpr float DENSITY_LATTICE_SPACING = 250.0f;
inline constexpr uint32_t DENSITY_SEED_BAND = 160u;
inline constexpr float DENSITY_EXPONENT = 0.6f;
inline constexpr float DENSITY_MIN = 1.0f;
inline constexpr float DENSITY_MAX = 1.0f;

// ── Terrain Tokens ─────────────────────────────────────────────────

inline constexpr uint32_t MAX_TERRAIN_TOKENS = 8;

struct TerrainToken {
    float archetype_bias[ARCHETYPE_COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f };
    uint32_t budget = 0;
    bool active = false;
};

// Token flow: emit_chance gates emission; pivot_chance picks
// continuation vs pivot; budget (uniform in [min,max]) = tile
// generations survived; bias multiplies archetype weights.

struct TerrainEmissionProfile {
    float emit_chance;                            // [0,1] probability of emitting any token
    uint32_t budget_min, budget_max;              // generation lifespan range
    float continuation_bias[ARCHETYPE_COUNT];     // archetype weight multipliers when continuing
    float pivot_chance;                           // [0,1] probability of pivoting vs continuing
    float pivot_bias[ARCHETYPE_COUNT];            // archetype weight multipliers when pivoting
};

inline constexpr TerrainEmissionProfile TERRAIN_EMISSION[ARCHETYPE_COUNT] = {
    /* 0: mountainous */ { 0.45f,  2, 5,  { 2.0f, 1.5f, 0.3f, 0.0f },  0.25f, { 0.3f, 2.0f, 1.5f, 0.0f } },
    /* 1: varied      */ { 0.25f,  1, 3,  { 0.8f, 1.5f, 0.8f, 0.2f },  0.30f, { 1.5f, 0.5f, 1.5f, 0.1f } },
    /* 2: basin       */ { 0.40f,  2, 4,  { 0.2f, 0.8f, 2.0f, 1.0f },  0.20f, { 0.5f, 1.5f, 0.5f, 0.3f } },
    /* 3: pool        */ { 0.20f,  1, 2,  { 0.0f, 0.5f, 1.5f, 1.5f },  0.35f, { 0.3f, 1.0f, 2.0f, 0.2f } },
};

inline constexpr float AMP_MOMENTUM_THRESHOLD = 0.15f;  // |jitter - 1.0| above this → emit amp momentum
inline constexpr float AMP_MOMENTUM_CARRY = 0.6f;       // fraction of excess carried forward

// ── Tile State ─────────────────────────────────────────────────────

struct TileState {
    uint32_t archetype = 1;      // default: varied
    float height_bias = 0.0f;
    float amp_scale = 1.0f;
    // STATUS: LATENT[tile-activation] — authored here (per-archetype
    // act column) and uploaded + interpolated GPU-side, but consumed
    // by neither height caller; the intended tile-character axis —
    // wiring is one multiply into band activation when wanted.
    float activation_scale = 1.0f;
    float amp_momentum = 0.0f;   // signed amplitude excess, carried by terrain tokens
    float entity_density = 1.0f; // spatial density multiplier for entity spawning
    // Theme: evaluated from theme lattice at tile generation time
    float theme_spawn[PopFamily::COUNT] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f }; // blended per-family spawn multiplier
    uint32_t theme_idx = 0;      // dominant theme index (for tier bias)
};

// Spatial cache: keyed by (grid_x, grid_z)
struct GridKey {
    int32_t x, z;
    bool operator==(const GridKey& o) const { return x == o.x && z == o.z; }
};
struct GridKeyHash {
    size_t operator()(const GridKey& k) const {
        return (size_t)k.x * 73856093u ^ (size_t)k.z * 19349663u;
    }
};

// ═══ MODULE DEPS (DISSOLVE-1 Batch A d2) ═══════════════════════════
// The requirements face made literal: what tile_world's authoring
// verbs consume beyond their own state. Organ-named members, bound
// once at the root. const trio law: world + mood read-only, the GPU
// wire writable. (WorldState fwd — patch_system.hpp follows this
// header in the cohort; the reference member tolerates the
// incomplete type.)
struct WorldState;
struct TileWorldDeps {
    const WorldState& world_state_;
    const MoodState&  mood_state_;
    GPUState&         gpuState_;
};

// ═══ MODULE STATE ══════════════════════════════════════════════════

// Instance (tile_world_state_) lives at the composition root.
struct TileWorldState {
    std::unordered_map<GridKey, TileState, GridKeyHash> tileCache_;
    TerrainToken terrainTokens_[MAX_TERRAIN_TOKENS]{};
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═══════════════════════════════
//
// DEFINED in tile_world.inl (post-class): generate reaches the
// keyhole's mood/world state; upload reaches the GPU wire.
// THE S2/S3 BOUNDARY FACE: the tile cache is read across the boundary
// by the spawn preamble and the surface samplers (estimate_terrain_
// height / terrain_tile_warm) — the interface trio's memory member.

void evict_distant_tiles(TileWorldState& tw, int32_t centerX, int32_t centerZ);
void upload_tile_grid_now(TileWorldState& tw, TileWorldDeps* c, wgpu::Queue& queue, int32_t cx, int32_t cz);
TileState generate_tile_state(TileWorldState& tw, TileWorldDeps* c, int32_t gx, int32_t gz);
// The cache-authoring doors (m4): the streaming conductor demands,
// the owner fills. ensure_tile ticks the terrain tokens (primary);
// ensure_tile_padding does NOT (padding must not advance the
// terrain's memory). reset_* are the teardown/init counterparts.
void ensure_tile(TileWorldState& tw, TileWorldDeps* c, int32_t gx, int32_t gz);
void ensure_tile_padding(TileWorldState& tw, TileWorldDeps* c, int32_t gx, int32_t gz);
void reset_tile_cache(TileWorldState& tw);
void reset_terrain_memory(TileWorldState& tw);
void tick_terrain_tokens(TileWorldState& tw, const TileState& outcome, uint32_t seed);
// THE S2 BOUNDARY FACE, four surfaces (REBUILD-0 m3b — v3 §7's first
// toolbox pull, justified by D1). INVARIANT AT EVERY FACE: callable
// WITHOUT the complete Cartridge — a generated-once surface cast
// could implement these four and the occupiers would never know.
//   F1 HEIGHT · F2 WARMTH · F3 SPAWN-MULT · F4 ARCHETYPE
float estimate_terrain_height(const TileWorldState& tw, float wx, float wz);
bool terrain_tile_warm(const TileWorldState& tw, float wx, float wz);
// F3: applies the tile's spawn modifiers ONTO the accumulator —
// density then per-family theme multiplier, in that order (the two
// multiplies stay separate: bit-identity under FP non-associativity);
// no memory of the tile = no change.
void tile_apply_spawn_mult(const TileWorldState& tw, int32_t gx, int32_t gz,
                           uint32_t family, float& adj_mod);
// F4: the tile's terrain archetype; returns false (out untouched)
// where the tile is cold — callers keep their own miss defaults
// (they differ: pace keeps 1.0, placement keeps archetype 1).
bool tile_archetype(const TileWorldState& tw, int32_t gx, int32_t gz, uint32_t& out);

} // namespace the_board
} // namespace t7
