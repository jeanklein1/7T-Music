#pragma once
#include "cartridges/the_board/realization/state.hpp"                    // Dim::*, GPUState, GPUFloatingEntityState, wgpu
#include "cartridges/the_board/contracts/floater_vocabulary.hpp"  // ActiveFloater
#include "cartridges/the_board/contracts/keyhole.hpp"          // Cartridge + wgpu::Queue fwds (the evictor decl)
#include "cartridges/the_board/contracts/entity_types.hpp"  // queue types (the funnel signatures)

// ─── spheres.hpp ─────────────────────────────────────────────────
// Born converted (LADDER-2 c0): history in audit/LADDER.md.
//
// The Sphere family's runtime STATE — the active-slot mirror for the
// orbital-sphere floaters.
//
// The SphereState CONTENT (the ActiveFloater array) was the class-body
// `activeFloaters_[]` / `activeFloaterCount_` in floater_vocabulary.inl;
// it moves here whole. The ActiveFloater TYPE stays shared vocabulary
// (floater_vocabulary.hpp). ActiveFloater -> ActiveSphere rename is
// flagged there (STATUS: LATENT[naming]), not performed.
//
// SEAM[sphere:P5] the ActiveFloater last_alloc_time race protection lives
//   with the type (floater_vocabulary.hpp); this owner holds the array the
//   render() floater-sync block and the evict path read.
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

struct SphereState {
    ActiveFloater activeFloaters_[Dim::MAX_SPHERE_INSTANCES]{};
    uint32_t      activeFloaterCount_ = 0;
};

// Teardown owner-clear: the sphere half of teardown_world's bulk
// sweep — CPU clear + per-slot GPU clear, paired.
inline void clear_spheres(SphereState& ss, GPUState& gpu, wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::MAX_SPHERE_INSTANCES; i++) {
        ss.activeFloaters_[i] = ActiveFloater{};
        GPUFloatingEntityState empty{};
        gpu.upload_sphere_entity_slot(queue, i, empty);
    }
    ss.activeFloaterCount_ = 0;
}

void evict_sphere(Cartridge* self, uint32_t slot, wgpu::Queue& queue);
// Dispatch funnels (table-shaped; defined in spheres.inl beside the recipe)
bool dispatch_select_sphere_generic(Cartridge* self, int32_t gx, int32_t gz, EntityQueueEntry& e);
bool dispatch_place_sphere_generic(Cartridge* self, EntityQueueEntry& e, PlacementEntry& pe);
void dispatch_commit_sphere_generic(Cartridge* self, PlacementEntry& pe, wgpu::Queue& queue);

} // namespace the_board
} // namespace t7
