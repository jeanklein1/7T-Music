#pragma once
#include <cstdint>
#include <cstddef>        // size_t (GridKeyHash)
#include <unordered_map>  // the tile cache
#include <cmath>           // std::floor, std::pow, std::abs (impl)
#include <algorithm>      // std::min (impl)
#include <iostream>       // std::cerr (the R5 loud tile-cache assert)
#include <cstdlib>        // std::abort (ditto)
#include "cartridges/the_board/contracts/roster.hpp"                // PopFamily (TileState theme columns)
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)

// ─── tile_world.hpp (S2 · MERGED single file) ─────────────────────────
// History: audit/LADDER.md
// hpp+inl collapsed: vocabulary + state + deps + inline impl.
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
// The DENSITY_* lattice constants + TilePopulation + its authoring moved
// to population_themes.hpp (Q6b) — the density/theme half of the tile is
// a population concern. Only TileState (below) still names TilePopulation.

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

// b4 (the tile_world cross-cut split): one tile's
// memory carried TWO concerns welded under one name (TERRAIN-0 Law 2) —
// landform SHAPE and entity POPULATION. Split by concern here so the
// SHAPE half is the heightfield CAST's (the sphere replaces it at Stage
// 3) and the POPULATION half is the population/themes concern. They stay
// rolled together at one generation moment under one (gx,gz) key
// (generate_tile_state fills both); only the TYPE separates the concern,
// and the readers touch the half they own. The GPU mirror (GPUTileEntry)
// was already shape-only — the split does not touch the C++/WGSL wire.

// THE SHAPE HALF — the cast's per-tile base-shape modulation. Read by
// the GPU tile upload (amp/bias/activation/archetype), estimate_terrain_
// height (F1), tile_archetype (F4), the neighbor archetype roll, and the
// terrain-token emission.
struct TileShape {
    uint32_t archetype = 1;      // default: varied
    float height_bias = 0.0f;
    float amp_scale = 1.0f;
    // STATUS: LATENT[tile-activation] — authored here (per-archetype
    // act column) and uploaded + interpolated GPU-side, but consumed
    // by neither height caller; the intended tile-character axis —
    // wiring is one multiply into band activation when wanted.
    float activation_scale = 1.0f;
    float amp_momentum = 0.0f;   // signed amplitude excess, carried by terrain tokens
};

// THE POPULATION HALF — TilePopulation (spawn density + per-family theme
// weights) lives in population_themes.hpp (Q6b), beside the authoring that
// fills it and the theme vocabulary it samples. It is complete here: that
// header precedes this one in the cohort. Read by tile_apply_spawn_mult (F3).

struct TileState {
    TileShape      shape;   // the cast's (landform character)
    TilePopulation pop;     // themes' (spawn density + weights)
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

// ═══ MODULE DEPS ════════════════════════════════════════════════════
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
// DEFINED below (the merged impl): generate reaches the
// machine's mood/world state; upload reaches the GPU wire.
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
// THE S2 BOUNDARY FACE, four surfaces (v3 §7's first
// toolbox pull). INVARIANT AT EVERY FACE: callable
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

// ═══ IMPL: the
// bodies deref WorldState/MoodState/GPUState via TileWorldDeps (no
// Cartridge). COHORT PROOF: the merged file sits AFTER patch_system.hpp
// (WorldState complete + PATCH_EXTENT/PREGEN_RADIUS) and after
// population_themes.hpp (THEMES) and mood.hpp (MOOD_TABLE); the S2
// faces stay declared before the machine templates instantiate. ══════

// Forgetting radius: tiles beyond this many grid cells get evicted
inline constexpr int32_t FORGET_RADIUS = (int32_t)PREGEN_RADIUS + 2;  // eviction radius (beyond pre-gen)

inline void evict_distant_tiles(TileWorldState& tw, int32_t centerX, int32_t centerZ) {
    auto it = tw.tileCache_.begin();
    while (it != tw.tileCache_.end()) {
        int32_t dx = it->first.x - centerX;
        int32_t dz = it->first.z - centerZ;
        if (dx < -FORGET_RADIUS || dx > FORGET_RADIUS ||
            dz < -FORGET_RADIUS || dz > FORGET_RADIUS) {
            it = tw.tileCache_.erase(it);
        }
        else {
            ++it;
        }
    }
}

// Build and upload GPUTileGrid from tile cache, centered on (cx, cz).
inline void upload_tile_grid_now(TileWorldState& tw, TileWorldDeps* c, wgpu::Queue& queue, int32_t cx, int32_t cz) {
    static constexpr int32_t TILE_PAD = 1;
    // Q8: TILE_GRID capacity guard. The GPUTileGrid DTO (state.hpp) sizes
    // entries[] for radius PATCH_PREGEN_RADIUS + 1 (TILE_GRID_SIDE = 17), and
    // active_radius is runtime-clamped to <= PATCH_PREGEN_RADIUS
    // (set_render_radius; the finite cap goes lower). The built window radius
    // is active_radius + TILE_PAD, so it fits the DTO iff TILE_PAD <= 1 (the
    // DTO's own pad). Both the DTO side and the active_radius clamp track
    // PATCH_PREGEN_RADIUS, so the ONLY free variable that could overflow is
    // TILE_PAD — this closes the compile-time half of the surface; the
    // runtime half is the existing active_radius clamp.
    static_assert(TILE_PAD <= 1,
        "tile-grid window (active_radius <= PATCH_PREGEN_RADIUS, plus TILE_PAD) "
        "must fit the GPUTileGrid DTO sized for PATCH_PREGEN_RADIUS + 1");
    int32_t rp = (int32_t)c->world_state_.active_radius + TILE_PAD;
    uint32_t tileGridSide = 2 * (c->world_state_.active_radius + TILE_PAD) + 1;
    GPUTileGrid grid{};
    grid.origin_x = cx - rp;
    grid.origin_z = cz - rp;
    grid.side = tileGridSide;
    grid.cell_extent = PATCH_EXTENT;

    for (int32_t gz = cz - rp; gz <= cz + rp; gz++) {
        for (int32_t gx = cx - rp; gx <= cx + rp; gx++) {
            int32_t lx = gx - grid.origin_x;
            int32_t lz = gz - grid.origin_z;
            uint32_t idx = lz * tileGridSide + lx;
            auto it = tw.tileCache_.find({ gx, gz });
            if (it != tw.tileCache_.end()) {
                grid.entries[idx].amp_scale = it->second.shape.amp_scale;
                grid.entries[idx].height_bias = it->second.shape.height_bias;
                grid.entries[idx].activation_scale = it->second.shape.activation_scale;
                grid.entries[idx].archetype = it->second.shape.archetype;
            }
            else {
                grid.entries[idx].amp_scale = 1.0f;
                grid.entries[idx].height_bias = 0.0f;
                grid.entries[idx].activation_scale = 1.0f;
                grid.entries[idx].archetype = 1;
            }
        }
    }
    c->gpuState_.upload_tile_grid(queue, grid);
}

inline TileState generate_tile_state(TileWorldState& tw, TileWorldDeps* c, int32_t gx, int32_t gz) {
    // Count neighbor archetypes
    uint32_t neighbor_counts[ARCHETYPE_COUNT] = {};
    uint32_t total_neighbors = 0;

    for (int32_t dz = -1; dz <= 1; dz++) {
        for (int32_t dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dz == 0) continue;
            auto it = tw.tileCache_.find({ gx + dx, gz + dz });
            if (it != tw.tileCache_.end()) {
                neighbor_counts[it->second.shape.archetype]++;
                total_neighbors++;
            }
        }
    }

    // Build selection weights from archetype base weights + neighbor influence
    float weights[ARCHETYPE_COUNT];
    for (uint32_t a = 0; a < ARCHETYPE_COUNT; a++) {
        weights[a] = ARCHETYPES[a].base_weight;
    }

    static constexpr uint32_t POOL_IDX = 3;
    if (MOOD_TABLE[c->mood_state_.active].indoor) {
        weights[POOL_IDX] = 1.5f;   // ~30% of indoor tiles become pools
    }
    else {
        weights[POOL_IDX] = 0.05f;  // ~1.5% of outdoor tiles
    }

    // ── Terrain token priors: multiply active tokens into weights ──
    for (uint32_t t = 0; t < MAX_TERRAIN_TOKENS; t++) {
        if (!tw.terrainTokens_[t].active) continue;
        for (uint32_t a = 0; a < ARCHETYPE_COUNT; a++) {
            weights[a] *= tw.terrainTokens_[t].archetype_bias[a];
        }
    }

    if (total_neighbors > 0) {
        using R = ArchetypeSelectionRules;
        for (uint32_t a = 0; a < ARCHETYPE_COUNT; a++) {
            if (neighbor_counts[a] >= R::DOMINANT_THRESHOLD) {
                weights[a] *= R::DOMINANT_MULTIPLIER;
            }
            else if (neighbor_counts[a] >= R::COMMON_THRESHOLD) {
                weights[a] *= R::COMMON_MULTIPLIER;
            }
            else if (neighbor_counts[a] >= R::PRESENT_THRESHOLD) {
                weights[a] *= R::PRESENT_MULTIPLIER;
            }
        }
    }

    // Normalize and roll
    float total_weight = 0.0f;
    for (uint32_t a = 0; a < ARCHETYPE_COUNT; a++) total_weight += weights[a];
    for (uint32_t a = 0; a < ARCHETYPE_COUNT; a++) weights[a] /= total_weight;

    uint32_t seed = tile_seed(c->world_state_.active_seed, gx, gz);
    float roll = cpu_hash_f(seed, 300u);

    uint32_t archetype = ARCHETYPE_COUNT - 1;
    float cumulative = 0.0f;
    for (uint32_t a = 0; a < ARCHETYPE_COUNT; a++) {
        cumulative += weights[a];
        if (roll < cumulative) { archetype = a; break; }
    }

    // Per-tile jitter from archetype profile
    const auto& profile = ARCHETYPES[archetype];
    float amp_jitter = 1.0f + (cpu_hash_f(seed, 301u) - 0.5f) * profile.amp_jitter_range;
    float bias_jitter = (cpu_hash_f(seed, 302u) - 0.5f) * profile.bias_jitter_range;

    TileState ts;
    ts.shape.archetype = archetype;
    ts.shape.amp_scale = profile.amp_scale * amp_jitter;
    ts.shape.height_bias = profile.height_bias + bias_jitter;
    ts.shape.activation_scale = profile.activation_scale;
    ts.shape.amp_momentum = amp_jitter - 1.0f;  // signed: positive = amplified, negative = dampened

    // ── The population half (Q6b) ────────────────────────────
    // Authored by population_themes.hpp off the density/theme seed bands
    // (active_seed passed in) — disjoint from the shape draws above, which
    // consume tile_seed props. Same one (gx,gz) generation moment; the
    // TYPE-line split (b4) still holds, the authoring now lives with its
    // vocabulary.
    ts.pop = generate_tile_population(c->world_state_.active_seed, gx, gz);

    return ts;
}

// Called ONCE per primary tile generation, NEVER for neighbor padding.
// Decrements all active token budgets, clears expired tokens,
// then evaluates the tile outcome for emission of a new token.
inline void tick_terrain_tokens(TileWorldState& tw, const TileState& outcome, uint32_t seed) {
    // ── Tick existing tokens ─────────────────────────────────
    for (uint32_t t = 0; t < MAX_TERRAIN_TOKENS; t++) {
        if (!tw.terrainTokens_[t].active) continue;
        if (tw.terrainTokens_[t].budget <= 1) {
            tw.terrainTokens_[t].active = false;
        }
        else {
            tw.terrainTokens_[t].budget--;
        }
    }

    // ── Emission from outcome ────────────────────────────────
    const auto& ep = TERRAIN_EMISSION[outcome.shape.archetype];

    // Roll: does this outcome emit a token?
    // Property index 310: decorrelated from archetype roll (300-302)
    float emit_roll = cpu_hash_f(seed, 310u);
    if (emit_roll >= ep.emit_chance) return;

    // Roll: continuation or pivot?
    float pivot_roll = cpu_hash_f(seed, 311u);
    bool pivot = (pivot_roll < ep.pivot_chance);

    // Budget draw (uniform in [budget_min, budget_max])
    float budget_t = cpu_hash_f(seed, 312u);
    uint32_t budget = ep.budget_min +
        (uint32_t)(budget_t * (float)(ep.budget_max - ep.budget_min + 1));
    budget = std::min(budget, ep.budget_max);  // clamp rounding

    // Build the token
    TerrainToken token{};
    const float* bias = pivot ? ep.pivot_bias : ep.continuation_bias;
    for (uint32_t a = 0; a < ARCHETYPE_COUNT; a++) {
        token.archetype_bias[a] = bias[a];
    }

    // Amplitude momentum: if this patch rolled extreme, carry it
    if (std::abs(outcome.shape.amp_momentum) > AMP_MOMENTUM_THRESHOLD) {
        float carry = outcome.shape.amp_momentum * AMP_MOMENTUM_CARRY;
        if (carry > 0.0f) {
            token.archetype_bias[0] *= (1.0f + carry);  // mountainous
        }
        else {
            token.archetype_bias[2] *= (1.0f - carry);  // basin (carry is negative)
        }
    }

    token.budget = budget;
    token.active = true;

    // ── Insert into stack ────────────────────────────────────
    // Find a free slot. If none, evict the token with lowest budget.
    uint32_t slot = MAX_TERRAIN_TOKENS;
    uint32_t min_budget = UINT32_MAX;
    uint32_t min_slot = 0;

    for (uint32_t t = 0; t < MAX_TERRAIN_TOKENS; t++) {
        if (!tw.terrainTokens_[t].active) { slot = t; break; }
        if (tw.terrainTokens_[t].budget < min_budget) {
            min_budget = tw.terrainTokens_[t].budget;
            min_slot = t;
        }
    }
    if (slot == MAX_TERRAIN_TOKENS) slot = min_slot;  // evict oldest

    tw.terrainTokens_[slot] = token;
}

// THE S2/S3 BOUNDARY FACE: the surface samplers — the occupiers ask
// the terrain's memory for height and warmth.
// Q6a: the world-xz → tile grid key. The ONE place a boundary-face reader
// turns world coords into (x,z), so every reader reproduces the SAME
// (int)floor(w/PATCH_EXTENT) derivation (it was recomputed at each site).
// NOTE: this is a container LOOKUP key — GridKeyHash's primes are a bucket
// hash, NOT a (seed,prop)→value biography path.
inline GridKey tile_key(float wx, float wz) {
    return { (int32_t)std::floor(wx / PATCH_EXTENT), (int32_t)std::floor(wz / PATCH_EXTENT) };
}

inline float estimate_terrain_height(const TileWorldState& tw, float wx, float wz) {
    auto it = tw.tileCache_.find(tile_key(wx, wz));
    if (it != tw.tileCache_.end())
        return it->second.shape.height_bias + it->second.shape.amp_scale * 5.0f;
    return 0.0f;
}

inline bool terrain_tile_warm(const TileWorldState& tw, float wx, float wz) {
    return tw.tileCache_.find(tile_key(wx, wz)) != tw.tileCache_.end();
}

// F3 (m3b): the spawn-modifier face. The two multiplies preserve the
// consumers' original operation order exactly (density, then theme).
inline void tile_apply_spawn_mult(const TileWorldState& tw, int32_t gx, int32_t gz,
                                  uint32_t family, float& adj_mod) {
    auto it = tw.tileCache_.find({ gx, gz });
    if (it == tw.tileCache_.end()) {
        // LOUD (R5): the gate's theme layer silently
        // depended on ensure_tile having run at patch allocation. A miss here
        // means the allocation→spawn ordering broke — fail at the seam, not
        // by quietly dropping the theme layer from the spawn stack.
        std::cerr << "[SPAWN] tile_apply_spawn_mult MISS at (" << gx << "," << gz
                  << ") family " << family << " — ensure_tile did not precede the gate\n";
        std::abort();
    }
    adj_mod *= it->second.pop.entity_density;
    adj_mod *= it->second.pop.spatial_density[family];
}

// F4 (m3b): the archetype face, bool-out — the miss default stays
// with the caller (pace keeps 1.0, placement keeps archetype 1).
inline bool tile_archetype(const TileWorldState& tw, int32_t gx, int32_t gz, uint32_t& out) {
    auto it = tw.tileCache_.find({ gx, gz });
    if (it != tw.tileCache_.end()) { out = it->second.shape.archetype; return true; }
    return false;
}


// ─── The cache-authoring doors (owner verbs) ───────────────────────
inline void ensure_tile(TileWorldState& tw, TileWorldDeps* c, int32_t gx, int32_t gz) {
    GridKey key{ gx, gz };
    if (tw.tileCache_.find(key) == tw.tileCache_.end()) {
        TileState ts = generate_tile_state(tw, c, gx, gz);
        tick_terrain_tokens(tw, ts, tile_seed(c->world_state_.active_seed, gx, gz));
        tw.tileCache_[key] = ts;
    }
}

inline void ensure_tile_padding(TileWorldState& tw, TileWorldDeps* c, int32_t gx, int32_t gz) {
    GridKey nk{ gx, gz };
    if (tw.tileCache_.find(nk) == tw.tileCache_.end()) {
        tw.tileCache_[nk] = generate_tile_state(tw, c, gx, gz);
    }
}

inline void reset_tile_cache(TileWorldState& tw) {
    tw.tileCache_.clear();
}

inline void reset_terrain_memory(TileWorldState& tw) {
    for (uint32_t t = 0; t < MAX_TERRAIN_TOKENS; t++) {
        tw.terrainTokens_[t] = TerrainToken{};
    }
}

} // namespace the_board
} // namespace t7