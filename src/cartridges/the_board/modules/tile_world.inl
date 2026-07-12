// ─── tile_world.inl (S2 · IMPL: post-class definitions) ───────────
// Born at LADDER-6 (S2 extraction): history in audit/LADDER.md.
//
// The four verbs over what the terrain remembers. generate reaches
// c->mood_state_ / MOOD_TABLE / c->world_state_ and the theme module;
// upload reaches c->gpuState_; evict derives its radius from
// PREGEN_RADIUS (patch_system.hpp).
//
// WRAPPING FORM (fix-2): SELF-WRAPPING — the zone includes impls at FILE SCOPE; law in audit/LADDER.md.

#include <cmath>      // std::floor, std::pow, std::abs
#include <algorithm>  // std::min

namespace t7 {
namespace the_board {

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
inline void upload_tile_grid_now(TileWorldState& tw, Cartridge* c, wgpu::Queue& queue, int32_t cx, int32_t cz) {
    static constexpr int32_t TILE_PAD = 1;
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
                grid.entries[idx].amp_scale = it->second.amp_scale;
                grid.entries[idx].height_bias = it->second.height_bias;
                grid.entries[idx].activation_scale = it->second.activation_scale;
                grid.entries[idx].archetype = it->second.archetype;
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

inline TileState generate_tile_state(TileWorldState& tw, Cartridge* c, int32_t gx, int32_t gz) {
    // Count neighbor archetypes
    uint32_t neighbor_counts[ARCHETYPE_COUNT] = {};
    uint32_t total_neighbors = 0;

    for (int32_t dz = -1; dz <= 1; dz++) {
        for (int32_t dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dz == 0) continue;
            auto it = tw.tileCache_.find({ gx + dx, gz + dz });
            if (it != tw.tileCache_.end()) {
                neighbor_counts[it->second.archetype]++;
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
    ts.archetype = archetype;
    ts.amp_scale = profile.amp_scale * amp_jitter;
    ts.height_bias = profile.height_bias + bias_jitter;
    ts.activation_scale = profile.activation_scale;
    ts.amp_momentum = amp_jitter - 1.0f;  // signed: positive = amplified, negative = dampened

    // ── Entity density field (coarse spatial noise) ──────────
    {
        float patch_cx = (gx + 0.5f) * PATCH_EXTENT;
        float patch_cz = (gz + 0.5f) * PATCH_EXTENT;
        float dlx = patch_cx / DENSITY_LATTICE_SPACING;
        float dlz = patch_cz / DENSITY_LATTICE_SPACING;
        int32_t dbx = (int32_t)std::floor(dlx);
        int32_t dbz = (int32_t)std::floor(dlz);
        float dfx = dlx - dbx, dfz = dlz - dbz;
        float dwx = dfx * dfx * (3.0f - 2.0f * dfx);
        float dwz = dfz * dfz * (3.0f - 2.0f * dfz);
        float density = 0.0f;
        for (int dz = 0; dz <= 1; dz++) for (int dx = 0; dx <= 1; dx++) {
            uint32_t ns = cpu_lattice_node_seed(c->world_state_.active_seed, dbx + dx, dbz + dz, DENSITY_SEED_BAND);
            float raw = cpu_hash_f(ns, 350u);
            float shaped = std::pow(raw, DENSITY_EXPONENT);
            float w = ((dx == 1) ? dwx : (1.0f - dwx)) * ((dz == 1) ? dwz : (1.0f - dwz));
            density += shaped * w;
        }
        ts.entity_density = DENSITY_MIN + density * (DENSITY_MAX - DENSITY_MIN);
    }

    // ── Theme field (coarse compositional character) ─────────
    {
        float patch_cx = (gx + 0.5f) * PATCH_EXTENT;
        float patch_cz = (gz + 0.5f) * PATCH_EXTENT;
        float tlx = patch_cx / THEME_LATTICE_SPACING;
        float tlz = patch_cz / THEME_LATTICE_SPACING;
        int32_t tbx = (int32_t)std::floor(tlx);
        int32_t tbz = (int32_t)std::floor(tlz);
        float tfx = tlx - tbx, tfz = tlz - tbz;
        float twx = tfx * tfx * (3.0f - 2.0f * tfx);
        float twz = tfz * tfz * (3.0f - 2.0f * tfz);

        // Blend spawn weights across 4 lattice nodes.
        // Track dominant node for discrete tier bias lookup.
        float blended_spawn[PopFamily::COUNT] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        float blended_density = 0.0f;
        float best_w = -1.0f;
        uint32_t dominant_theme = 0;

        for (int dz = 0; dz <= 1; dz++) for (int dx = 0; dx <= 1; dx++) {
            uint32_t ns = cpu_lattice_node_seed(c->world_state_.active_seed, tbx + dx, tbz + dz, THEME_SEED_BAND);
            uint32_t tidx = select_theme_at_node(ns);
            const auto& theme = THEMES[tidx];
            float w = ((dx == 1) ? twx : (1.0f - twx)) * ((dz == 1) ? twz : (1.0f - twz));
            for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
                blended_spawn[f] += theme.spawn_weight[f] * w;
            }
            blended_density += theme.density_mult * w;
            if (w > best_w) { best_w = w; dominant_theme = tidx; }
        }

        for (uint32_t f = 0; f < PopFamily::COUNT; f++)
            ts.theme_spawn[f] = blended_spawn[f];
        ts.theme_idx = dominant_theme;
        ts.entity_density *= blended_density;  // theme density stacks with spatial density
    }

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
    const auto& ep = TERRAIN_EMISSION[outcome.archetype];

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
    if (std::abs(outcome.amp_momentum) > AMP_MOMENTUM_THRESHOLD) {
        float carry = outcome.amp_momentum * AMP_MOMENTUM_CARRY;
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
// the terrain's memory for height and warmth (rode in from
// spawn_engine at its conversion, per the Phase R stamp).
inline float estimate_terrain_height(const TileWorldState& tw, float wx, float wz) {
    int32_t tx = (int32_t)std::floor(wx / PATCH_EXTENT);
    int32_t tz = (int32_t)std::floor(wz / PATCH_EXTENT);
    auto it = tw.tileCache_.find({ tx, tz });
    if (it != tw.tileCache_.end())
        return it->second.height_bias + it->second.amp_scale * 5.0f;
    return 0.0f;
}

inline bool terrain_tile_warm(const TileWorldState& tw, float wx, float wz) {
    int32_t tx = (int32_t)std::floor(wx / PATCH_EXTENT);
    int32_t tz = (int32_t)std::floor(wz / PATCH_EXTENT);
    return tw.tileCache_.find({ tx, tz }) != tw.tileCache_.end();
}

} // namespace the_board
} // namespace t7
