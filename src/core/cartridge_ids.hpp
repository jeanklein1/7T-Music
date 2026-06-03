#pragma once

// ─── cartridge_ids.hpp ───────────────────────────────────────────
//
// Single source of truth for all cartridge identifiers. Used by door
// targets, transition callbacks, and CartridgeManager.
//
// Depends on: <cstdint>.

#include <cstdint>

namespace t7 {
namespace CartridgeId {

    constexpr uint32_t NONE               = 0;   // No transition
    constexpr uint32_t GALLERY_RAYMARCH   = 1;   // Entry point (hub) - raymarched
    constexpr uint32_t GALLERY            = 2;   // Gallery annex - rasterized
    constexpr uint32_t N_DIMENSIONAL_2    = 3;
    constexpr uint32_t WORLD_COMPUTE      = 4;
    constexpr uint32_t TERRAIN_PAWN       = 5;
    constexpr uint32_t PAWN_RASTERIZE     = 6;
    constexpr uint32_t PLAYGROUND_HYBRID  = 7;
    constexpr uint32_t SPECIES_STUDIO     = 8;
    constexpr uint32_t THE_BOARD          = 9;

} // namespace CartridgeId
} // namespace t7
