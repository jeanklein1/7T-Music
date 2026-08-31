#pragma once
// ─── seed_utils.hpp ──────────────────────────────────────────────
//
// Pure math.
//
// SEAM[seed_utils:P9] textbook "library without state" module — pure
//   functions, no class members referenced, no domain assumptions (zero
//   compilation-order constraints). Same family as
//   coupling/trajectory.hpp and entity_types (P9 instances).
// SEAM[seed_utils:contract] cpu_lattice_node_seed and cpu_sample_gaussian
//   are GPU mirrors — must produce identical bit-for-bit results to the
//   WGSL counterparts (lattice_node_seed, sample_gaussian). The
//   per-function comments name this. Same family as agents:L2 and
//   state.hpp's GPU struct contract.
// ─────────────────────────────────────────────────────────────────

#include <cstdint>
#include <algorithm>  // std::max, std::min
#include <cmath>      // std::sqrt, std::log, std::cos

namespace t7 {
namespace the_board {

// Hashing utilities (mirror GPU hash functions for determinism)
inline uint32_t cpu_hash(uint32_t seed, uint32_t property) {
    uint32_t h = seed * 747796405u + property * 2891336453u + 1u;
    h = ((h >> 16) ^ h) * 2654435769u;
    h = ((h >> 16) ^ h) * 2654435769u;
    h = (h >> 16) ^ h;
    return h;
}

inline float cpu_hash_f(uint32_t seed, uint32_t property) {
    return (float)cpu_hash(seed, property) / (float)0xFFFFFFFFu;
}

inline uint32_t tile_seed(uint32_t master_seed, int32_t gx, int32_t gz) {
    uint32_t h = master_seed;
    h ^= (uint32_t)gx * 73856093u;
    h ^= (uint32_t)gz * 19349663u;
    h = (h ^ (h >> 16)) * 2654435769u;
    h = (h ^ (h >> 16)) * 2654435769u;
    return h;
}

// CPU mirror of WGSL lattice_node_seed (must produce identical results)
inline uint32_t cpu_lattice_node_seed(uint32_t master_seed, int32_t nx, int32_t nz, uint32_t band) {
    uint32_t h = master_seed;
    h ^= (uint32_t)nx * 73856093u;
    h ^= (uint32_t)nz * 19349663u;
    h ^= band * 83492791u;
    h = (h ^ (h >> 16)) * 2654435769u;
    h = (h ^ (h >> 16)) * 2654435769u;
    return h;
}

inline float cpu_smoothstep(float e0, float e1, float x) {
    float t = std::max(0.0f, std::min(1.0f, (x - e0) / (e1 - e0)));
    return t * t * (3.0f - 2.0f * t);
}

// CPU-side Gaussian sampling that mirrors the WGSL sample_gaussian exactly.
// (seed, property) → Box-Muller → truncated at ±3σ.
inline float cpu_sample_gaussian(uint32_t seed, uint32_t property, float mean, float sigma) {
    float u1 = std::max(cpu_hash_f(seed, property), 1e-6f);
    float u2 = cpu_hash_f(seed, property + 1000u);  // matches GAUSSIAN_PAIR_OFFSET
    float z = std::sqrt(-2.0f * std::log(u1)) * std::cos(2.0f * 3.14159265359f * u2);
    z = std::max(-3.0f, std::min(3.0f, z));
    return mean + z * sigma;
}

//
// SEAM[seed_utils:Q10-target] the ONE cumulative-weight bucket walk,
//   shared across every domain. The Q10 consolidation has LANDED:
//   the hand-rolled copies in agents.hpp, gol_zones.hpp and ribbon.hpp
//   now call select_weighted / select_tier. The
//   generic entity pipeline was never a separate copy — it now
//   calls select_tier directly (its thin biased forwarder was
//   retired).

// The bucket walk. Takes a pre-rolled uniform in [0,1); returns the
// first index whose cumulative weight exceeds it; count-1 on the
// float-epsilon miss. Weights are the caller's contract (normalized
// or authored-to-sum-1); the walk does not normalize.
//
// Q4 (fork DOCUMENTED, not migrated): the two THEME selectors
// (select_theme_at_node, evaluate_theme_envelope) deliberately do NOT
// route through here. select_theme_at_node NORMALIZES inline (divides by
// the live weight-sum per step — this walk takes pre-normalized weights),
// and evaluate_theme_envelope is a STATEFUL SEQUENCED sampler (its per-
// patch call sequence mutating cooldowns/elapsed IS the biography). A
// different shape, not one abstraction twice — merging would touch a draw
// path for a 2-site payoff with only rig-level bit-catch. See the
// reciprocal note at select_theme_at_node (population_themes.hpp).
inline uint32_t select_weighted(float roll, const float* weights,
                                uint32_t count) {
    float cumul = 0.0f;
    for (uint32_t t = 0; t < count; t++) {
        cumul += weights[t];
        if (roll < cumul) return t;
    }
    return count - 1;
}

inline uint32_t select_tier(uint32_t seed, uint32_t tier_prop,
                            const float* weights, uint32_t count) {
    return select_weighted(cpu_hash_f(seed, tier_prop), weights, count);
}


// Precompute catenary parameter 'a' from (half_span, rise).
// 50-iteration bisection. It was the arch's; the arch left at
// ONE_WORLD-I U3 and the vault ceiling (mood.hpp) is the caller now.
// Census home: pure math with four cross-module
// consumers (spawn_engine, entity_pipeline, entities, mood) — the
// pure-math leaf is the shared header.
inline float solve_catenary_a(float half_span, float target_h) {
    float a_lo = 0.1f, a_hi = std::max(half_span * 10.0f, 5.0f);
    float a = half_span;
    for (int iter = 0; iter < 50; iter++) {
        a = 0.5f * (a_lo + a_hi);
        float val = a * (std::cosh(half_span / a) - 1.0f);
        if (val > target_h) a_lo = a; else a_hi = a;
    }
    return a;
}

} // namespace the_board
} // namespace t7
