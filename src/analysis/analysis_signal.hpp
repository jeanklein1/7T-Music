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
 * LAYOUT (288 bytes)
 * ------------------
 * 
 *   Offset  Field                Size
 *   0       t_seconds            4
 *   4       t_beats              4
 *   8       dt                   4
 *   12      _pad0                4
 *   16      stats[64]            256
 *   272     _pad1[4]             16
 *   288     END
 * 
 * STAT ARRAY DESIGN
 * -----------------
 * 
 * The stats array holds 64 floats for multi-channel musical analysis.
 * 
 * Indexing: stats[channel * STATS_PER_CHANNEL + stat]
 * 
 * With 4 channels and 16 stats per channel:
 *   Channel 0: stats[0..15]
 *   Channel 1: stats[16..31]
 *   Channel 2: stats[32..47]
 *   Channel 3: stats[48..63]
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
constexpr int STATS_PER_CHANNEL = 16;
constexpr int TOTAL_STATS = MAX_CHANNELS * STATS_PER_CHANNEL;  // 64

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
    
    // ═══ MUSICAL STATS (256 bytes) ═════════════════════════════════════════
    
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

static_assert(sizeof(AnalysisSignal) == 288, "AnalysisSignal must be 288 bytes");
static_assert(alignof(AnalysisSignal) == 16, "AnalysisSignal must be 16-byte aligned");

} // namespace t7
