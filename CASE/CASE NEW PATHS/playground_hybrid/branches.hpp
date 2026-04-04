#pragma once

/**
 * PLAYGROUND HYBRID — Branch Generation
 * =====================================
 *
 * CPU-side procedural generation of branching wall skeleton.
 * Seeded randomness ensures reproducibility.
 *
 * SCROLL INDEX:
 *   §1    DATA STRUCTURES .............. Branch segments and config
 *   §2    RANDOM ...................... Seeded PRNG
 *   §3    VECTOR MATH ................. 2D utilities
 *   §4    GENERATION .................. Recursive branch growth
 *   §5    INTERFACE ................... Public API
 */

#include <vector>
#include <cmath>
#include <cstdint>

namespace t7 {
namespace playground_hybrid {


// ═══════════════════════════════════════════════════════════════════════════════
// §1 DATA STRUCTURES
// ═══════════════════════════════════════════════════════════════════════════════

struct BranchSegment {
    float start_x, start_z;     // Floor position A
    float end_x, end_z;         // Floor position B
    float height_start;         // Wall height at A
    float height_end;           // Wall height at B
    float _pad0, _pad1;         // Align to 32 bytes for GPU
};

static_assert(sizeof(BranchSegment) == 32, "BranchSegment must be 32 bytes");


struct WaveSource {
    float x, z;                 // Position on floor plane
    float phase;                // Phase offset for variety
    float _pad0;                // Align to 16 bytes
};

static_assert(sizeof(WaveSource) == 16, "WaveSource must be 16 bytes");


struct BranchConfig {
    // Seed
    uint32_t seed = 42;
    
    // Spatial bounds
    float perimeter_radius = 20.0f;
    float center_x = 0.0f;
    float center_z = 0.0f;
    
    // Branch topology
    int branch_count_min = 4;
    int branch_count_max = 7;
    int bifurcation_depth = 4;
    float branch_length_min = 3.0f;
    float branch_length_max = 6.0f;
    float branch_angle_variance = 0.4f;   // Radians
    
    // Height distribution
    float height_at_perimeter = 2.0f;
    float height_at_center = 12.0f;
    
    // Interference waves
    float wave_frequency = 0.8f;
    float height_amplitude = 2.0f;
    float thickness_base = 0.8f;
    float thickness_amplitude = 0.4f;
    float min_thickness = 0.4f;
    float max_thickness = 2.0f;
    float min_height = 1.0f;
    float max_height = 15.0f;
    
    // Perimeter porosity
    float entry_gap_width = 2.5f;        // Width of entry gaps
};


struct BranchNetwork {
    std::vector<BranchSegment> segments;
    std::vector<WaveSource> wave_sources;
    BranchConfig config;
};


// ═══════════════════════════════════════════════════════════════════════════════
// §2 RANDOM — Seeded PRNG (xorshift32)
// ═══════════════════════════════════════════════════════════════════════════════

class SeededRandom {
public:
    explicit SeededRandom(uint32_t seed) : state_(seed ? seed : 1) {}
    
    // Returns [0, 1)
    float unit() {
        return static_cast<float>(next()) / static_cast<float>(UINT32_MAX);
    }
    
    // Returns [min, max]
    float range(float min, float max) {
        return min + unit() * (max - min);
    }
    
    // Returns integer [min, max]
    int range_int(int min, int max) {
        return min + static_cast<int>(unit() * (max - min + 1));
    }
    
    // Returns true with probability p
    bool chance(float p) {
        return unit() < p;
    }
    
private:
    uint32_t state_;
    
    uint32_t next() {
        state_ ^= state_ << 13;
        state_ ^= state_ >> 17;
        state_ ^= state_ << 5;
        return state_;
    }
};


// ═══════════════════════════════════════════════════════════════════════════════
// §3 VECTOR MATH — 2D utilities
// ═══════════════════════════════════════════════════════════════════════════════

struct Vec2 {
    float x, z;
    
    Vec2() : x(0), z(0) {}
    Vec2(float x_, float z_) : x(x_), z(z_) {}
    
    Vec2 operator+(const Vec2& o) const { return {x + o.x, z + o.z}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, z - o.z}; }
    Vec2 operator*(float s) const { return {x * s, z * s}; }
    
    float length() const { return std::sqrt(x * x + z * z); }
    float length_sq() const { return x * x + z * z; }
    
    Vec2 normalized() const {
        float len = length();
        return len > 0.0001f ? Vec2{x / len, z / len} : Vec2{0, 0};
    }
    
    static Vec2 from_angle(float angle) {
        return {std::cos(angle), std::sin(angle)};
    }
};

inline float dot(const Vec2& a, const Vec2& b) {
    return a.x * b.x + a.z * b.z;
}

inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline float clamp(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}


// ═══════════════════════════════════════════════════════════════════════════════
// §4 GENERATION — Recursive branch growth
// ═══════════════════════════════════════════════════════════════════════════════

namespace detail {

constexpr float PI = 3.14159265359f;
constexpr float TWO_PI = 6.28318530718f;

// Height based on distance from center
inline float height_for_position(const Vec2& pos, const BranchConfig& cfg) {
    Vec2 center{cfg.center_x, cfg.center_z};
    float dist = (pos - center).length();
    float t = clamp(dist / cfg.perimeter_radius, 0.0f, 1.0f);
    return lerp(cfg.height_at_center, cfg.height_at_perimeter, t);
}


// Recursive branch growth
inline void grow_branch(
    const Vec2& start,
    float direction,
    int depth,
    const BranchConfig& cfg,
    SeededRandom& rng,
    std::vector<BranchSegment>& segments,
    std::vector<WaveSource>& wave_sources
) {
    if (depth > cfg.bifurcation_depth) return;
    
    // Randomize branch length and angle
    float length = rng.range(cfg.branch_length_min, cfg.branch_length_max);
    float angle = direction + rng.range(-cfg.branch_angle_variance, cfg.branch_angle_variance);
    
    Vec2 end = start + Vec2::from_angle(angle) * length;
    
    // Check if end is too close to center or outside perimeter
    Vec2 center{cfg.center_x, cfg.center_z};
    float end_dist = (end - center).length();
    
    // Clamp to stay within reasonable bounds
    if (end_dist > cfg.perimeter_radius * 0.95f) {
        // Redirect inward
        Vec2 to_center = (center - start).normalized();
        angle = std::atan2(to_center.z, to_center.x);
        angle += rng.range(-0.3f, 0.3f);
        end = start + Vec2::from_angle(angle) * length;
    }
    
    // Compute heights
    float h_start = height_for_position(start, cfg);
    float h_end = height_for_position(end, cfg);
    
    // Create segment
    BranchSegment seg{};
    seg.start_x = start.x;
    seg.start_z = start.z;
    seg.end_x = end.x;
    seg.end_z = end.z;
    seg.height_start = h_start;
    seg.height_end = h_end;
    segments.push_back(seg);
    
    // Add wave source at branch end (tips create interference)
    if (depth >= cfg.bifurcation_depth - 1 || rng.chance(0.3f)) {
        WaveSource wave{};
        wave.x = end.x;
        wave.z = end.z;
        wave.phase = rng.range(0.0f, TWO_PI);
        wave_sources.push_back(wave);
    }
    
    // Bifurcation decision
    int n_children = 1;
    
    // Higher chance of branching at lower depths
    float bifurcation_chance = 0.7f - (depth * 0.1f);
    if (rng.chance(bifurcation_chance) && depth < cfg.bifurcation_depth) {
        n_children = rng.range_int(1, 2);
    }
    
    // Stop growing if we're deep enough and close to center
    float end_to_center = (end - center).length();
    if (end_to_center < cfg.perimeter_radius * 0.15f) {
        // Near center — may terminate
        if (rng.chance(0.6f)) return;
    }
    
    // Grow children
    for (int i = 0; i < n_children; i++) {
        float spread = (n_children > 1) 
            ? ((i == 0) ? -0.4f : 0.4f) + rng.range(-0.2f, 0.2f)
            : rng.range(-0.2f, 0.2f);
        
        float child_direction = angle + spread;
        
        grow_branch(end, child_direction, depth + 1, cfg, rng, segments, wave_sources);
    }
}


inline BranchNetwork generate_network(const BranchConfig& cfg) {
    BranchNetwork network;
    network.config = cfg;
    
    SeededRandom rng(cfg.seed);
    
    // Number of primary branches entering from perimeter
    int n_entries = rng.range_int(cfg.branch_count_min, cfg.branch_count_max);
    
    // Distribute entry points around perimeter
    float angle_step = TWO_PI / static_cast<float>(n_entries);
    float angle_offset = rng.range(0.0f, angle_step);
    
    Vec2 center{cfg.center_x, cfg.center_z};
    
    for (int i = 0; i < n_entries; i++) {
        float angle = angle_offset + angle_step * i;
        angle += rng.range(-angle_step * 0.2f, angle_step * 0.2f);
        
        // Start at perimeter
        Vec2 start = center + Vec2::from_angle(angle) * cfg.perimeter_radius;
        
        // Direction points inward (toward center) with some variance
        float inward_angle = angle + PI;
        inward_angle += rng.range(-cfg.branch_angle_variance, cfg.branch_angle_variance);
        
        grow_branch(start, inward_angle, 0, cfg, rng, network.segments, network.wave_sources);
    }
    
    // Add wave source at center
    WaveSource center_wave{};
    center_wave.x = cfg.center_x;
    center_wave.z = cfg.center_z;
    center_wave.phase = 0.0f;
    network.wave_sources.push_back(center_wave);
    
    return network;
}

} // namespace detail


// ═══════════════════════════════════════════════════════════════════════════════
// §5 INTERFACE — Public API
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * Generate a complete branch network from configuration.
 * 
 * Same seed + same config = same network (reproducible).
 */
inline BranchNetwork generate_branches(const BranchConfig& config) {
    return detail::generate_network(config);
}


/**
 * Default configuration for quick testing.
 */
inline BranchConfig default_branch_config() {
    return BranchConfig{};
}


/**
 * Print network statistics (for debugging).
 */
inline void print_network_stats(const BranchNetwork& network) {
    std::printf("[Branches] Segments: %zu, Wave sources: %zu\n",
        network.segments.size(),
        network.wave_sources.size());
    
    // Find bounds
    float min_x = 1e9f, max_x = -1e9f;
    float min_z = 1e9f, max_z = -1e9f;
    float min_h = 1e9f, max_h = -1e9f;
    
    for (const auto& seg : network.segments) {
        min_x = std::min(min_x, std::min(seg.start_x, seg.end_x));
        max_x = std::max(max_x, std::max(seg.start_x, seg.end_x));
        min_z = std::min(min_z, std::min(seg.start_z, seg.end_z));
        max_z = std::max(max_z, std::max(seg.start_z, seg.end_z));
        min_h = std::min(min_h, std::min(seg.height_start, seg.height_end));
        max_h = std::max(max_h, std::max(seg.height_start, seg.height_end));
    }
    
    std::printf("[Branches] X range: [%.1f, %.1f]\n", min_x, max_x);
    std::printf("[Branches] Z range: [%.1f, %.1f]\n", min_z, max_z);
    std::printf("[Branches] Height range: [%.1f, %.1f]\n", min_h, max_h);
}


} // namespace playground_hybrid
} // namespace t7
