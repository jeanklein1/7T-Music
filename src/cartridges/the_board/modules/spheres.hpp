#pragma once
#include "cartridges/the_board/state.hpp"                    // Dim::*, GPUState, GPUFloatingEntityState, wgpu
#include "cartridges/the_board/modules/floater_vocabulary.hpp"  // ActiveFloater
#include "cartridges/the_board/modules/keyhole.hpp"          // Cartridge + wgpu::Queue fwds (the evictor decl)

// ─── spheres.hpp ─────────────────────────────────────────────────
// Born converted (LADDER-2 c0): history in audit/LADDER.md.
//
// The Sphere family's runtime STATE — the active-slot mirror for the
// orbital-sphere floaters. The cartridge declares the instance
// (sphere_state_) in its COMPOSITION ROOT chapter.
//
// The lifecycle registry is entity-owned; "floater" is realization
// vocabulary (the co-owned GPU buffer + the spine-owned P5 readback)
// and gets NO CPU module — each species owns its active-slot state.
//
// The SphereState CONTENT (the ActiveFloater array) was the class-body
// `activeFloaters_[]` / `activeFloaterCount_` in floater_vocabulary.inl;
// it moves here whole. The ActiveFloater TYPE stays shared vocabulary
// (floater_vocabulary.hpp). ActiveFloater -> ActiveSphere rename is
// flagged there (STATUS: LATENT[naming]), not performed.
//
// Sight by declared services: clear_spheres takes GPUState& (the
// keyhole's GPU service), not the whole Cartridge — a before-class
// header cannot dereference the still-incomplete Cartridge, and the
// per-slot GPU clear needs only the GPU wire. Every dependency is
// visible in the signature.
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

// The evictor — lifecycle, absorbed per §5 EVICTION THUNKS.
// Keyhole-shaped to match the FAMILY_DISPATCH evict slot (table in
// family_dispatch.inl). DEFINED in spheres.inl (post-class).
void evict_sphere(Cartridge* self, uint32_t slot, wgpu::Queue& queue);

} // namespace the_board
} // namespace t7
