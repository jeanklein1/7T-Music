// ─── spheres.inl (IMPL: post-class definitions) ──────────────────
// Born at LADDER-5 e3: history in audit/LADDER.md.
//
// The Sphere family's lifecycle verbs — today the evictor alone
// (absorbed per §5 EVICTION THUNKS), named by the FAMILY_DISPATCH
// table (family_dispatch.inl). SphereState itself is spheres.hpp.
//
// WRAPPING FORM (the proven fix-2 rule): SELF-WRAPPING — opens
// t7::the_board itself, carries its own standard includes; the MODULE
// IMPLEMENTATIONS zone includes it at FILE SCOPE. Definitions are
// `inline` free functions.

#include <iostream>   // [DIAG:EVICT] logging (flag-gated)

namespace t7 {
namespace the_board {

inline void evict_sphere(Cartridge* self,
    uint32_t slot, wgpu::Queue& queue) {
    self->sphere_state_.activeFloaters_[slot].active = false;  // sphere state owned by SphereState
    self->sphere_state_.activeFloaterCount_--;
    GPUFloatingEntityState empty{};
    self->gpuState_.upload_sphere_entity_slot(queue, slot, empty);
#ifdef DIAG_ENTITY_LIFECYCLE
    std::cout << "[DIAG:EVICT]   sph slot=" << slot << "\n";
#endif
}

} // namespace the_board
} // namespace t7
