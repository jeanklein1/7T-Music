#pragma once

/**
 * ANALYSIS SIGNAL - The Musical Analysis Output Contract
 * ======================================================
 * 
 * This struct is the output of an analysis cartridge.
 * It carries time and musical statistics — nothing else.
 * 
 * CATEGORICAL BOUNDARY
 * --------------------
 * 
 * The analysis domain knows about music: beats, notes, polyphony, velocity.
 * The visualization domain knows about rendering: vertices, shaders, cameras.
 * 
 * This signal crosses that boundary. The visualization receives these numbers
 * without knowing what "polyphony" means — just that slot N has value X.
 * 
 * LAYOUT (2080 bytes)
 * -------------------
 *
 *   Offset  Field                Size
 *   0       t_seconds            4
 *   4       t_beats              4
 *   8       dt                   4
 *   12      _pad0                4
 *   16      stats[512]           2048
 *   2064    _pad1[4]             16
 *   2080    END
 *
 * STAT ARRAY DESIGN
 * -----------------
 *
 * The stats array holds 512 floats for multi-channel musical analysis.
 *
 * Indexing: stats[channel * STATS_PER_CHANNEL + stat]
 *
 * With 4 channels and 128 stats per channel:
 *   Channel 0: stats[0..127]
 *   Channel 1: stats[128..255]
 *   Channel 2: stats[256..383]
 *   Channel 3: stats[384..511]
 * 
 * Which slots carry which meaning is defined by the analysis cartridge.
 * The visualization cartridge must know the mapping to interpret the data.
 */

#include <array>
#include <cstdint>

namespace t7 {

// =============================================================================
// STAT ARRAY LAYOUT (infrastructure)
// =============================================================================

constexpr int MAX_CHANNELS = 4;
constexpr int STATS_PER_CHANNEL = 128;
constexpr int TOTAL_STATS = MAX_CHANNELS * STATS_PER_CHANNEL;  // 512

/**
 * Compute array index for a (channel, stat) pair.
 */
constexpr int stat_index(int channel, int stat) {
    return channel * STATS_PER_CHANNEL + stat;
}

// =============================================================================
// ANALYSIS SIGNAL STRUCT
// =============================================================================

struct alignas(16) AnalysisSignal {
    // ═══ TIME (16 bytes) ═══════════════════════════════════════════════════
    
    float t_seconds;        // Wall clock time when computed
    float t_beats;          // Musical time when computed
    float dt;               // Frame delta (seconds)
    float _pad0;            // Alignment padding
    
    // ═══ MUSICAL STATS (2048 bytes) ════════════════════════════════════════

    std::array<float, TOTAL_STATS> stats;
    
    // ═══ PADDING (16 bytes) ════════════════════════════════════════════════
    // Reserve space for future expansion, maintain 16-byte alignment
    
    float _pad1[4];
    
    // ═══ ACCESSORS ═════════════════════════════════════════════════════════
    
    float stat(int channel, int stat_type) const {
        return stats[stat_index(channel, stat_type)];
    }
    
    void set_stat(int channel, int stat_type, float value) {
        stats[stat_index(channel, stat_type)] = value;
    }
    
    /**
     * Clear all stats to zero.
     */
    void clear_stats() {
        stats.fill(0.0f);
    }
};

static_assert(sizeof(AnalysisSignal) == 2080, "AnalysisSignal must be 2080 bytes");
static_assert(alignof(AnalysisSignal) == 16, "AnalysisSignal must be 16-byte aligned");

// ─── STAT LAYOUT DESCRIPTOR ───────────────────────────────────────────
// Lets a consumer (e.g. the_lab) render each stat group in its
// representation shape without hardcoding the cartridge's slot map.
// A cartridge declares its layout; the consumer iterates and dispatches.

enum class StatShape {
    Scalar,   // single value  → scrolling line
    Vector,   // run of `count` values → bar chart
};

struct StatGroup {
    const char* name;       // label, e.g. "abbott.pc_histogram"
    int         channel;    // AnalysisSignal channel
    int         slot_base;  // first slot
    int         count;      // number of slots (1 for scalar)
    StatShape   shape;
};

// Non-owning view over a cartridge's STAT_LAYOUT array. STAT_LAYOUT is
// static constexpr storage, so a pointer into it is valid for the whole
// program — this view is safe to copy and hold. It is the "key" the
// analysis cartridge publishes so the render side can resolve stat
// groups by name at runtime (see musical/signal_layout.hpp).
struct StatLayoutView {
    const StatGroup* groups;
    uint32_t         count;
};

} // namespace t7
