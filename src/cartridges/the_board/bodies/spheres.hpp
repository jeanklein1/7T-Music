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

// Teardown owner-clear: the sphere half of the score's TEARDOWN
// movement (REBUILD-0 m2) — CPU clear + per-slot GPU clear, paired.
// Keyhole form since m3a (the GPUState& bypass retired; the defn
// moved to spheres.inl — the pre-class header cannot deref the
// incomplete Cartridge, which is why the bypass existed).
void clear_spheres(SphereState& ss, Cartridge* c, wgpu::Queue& queue);

void evict_sphere(Cartridge* self, uint32_t slot, wgpu::Queue& queue);
void reconcile_sphere_mirror(SphereState& ss, Cartridge* c, const GPUFloatingEntityState* data);
// Dispatch funnels (table-shaped; defined in spheres.inl beside the recipe)
bool dispatch_select_sphere_generic(Cartridge* self, int32_t gx, int32_t gz, EntityQueueEntry& e);
bool dispatch_place_sphere_generic(Cartridge* self, EntityQueueEntry& e, PlacementEntry& pe);
void dispatch_commit_sphere_generic(Cartridge* self, PlacementEntry& pe, wgpu::Queue& queue);

} // namespace the_board
} // namespace t7
