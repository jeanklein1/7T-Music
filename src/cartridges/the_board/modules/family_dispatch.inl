// ─── family_dispatch.inl (IMPL: the dispatch table, post-class) ───
// Born at LADDER-5 e2: history in audit/LADDER.md.
//
// THE TABLE. One row per family, PopFamily order — the file-scope
// definition of the FAMILY_DISPATCH declared in entity_types.hpp
// (the contract home). It lives post-class because the rows take
// wrapper ADDRESSES: the generic funnels and the remaining spine-side
// mesh adapters are Cartridge statics (named through the complete
// type); the bespoke funnels and the twelve evictors are file-scope
// functions in their owners' impls, declared in their owners'
// headers (the e5a arrow correction: the contract home carries no
// owner vocabulary, so owner headers include it and declare their
// own queue-shaped functions). Function
// addresses are link-time constants — declarations suffice, so this
// file's position in the zone does not depend on the other impls.
//
// SEAM[spine:owns] consumed by the spine's dispatch loops
//   (select_entities_for_patch / place_entity_queue /
//   commit_entity_queue in spawn_engine.inl), evict_patch_entities,
//   and the mesh prepare/dispatch loops in render(). Adding a family:
//   write the six verbs with its owner, add 1 row here.
//
// WRAPPING FORM (the proven fix-2 rule): SELF-WRAPPING — opens
// t7::the_board itself, carries its own standard includes; the MODULE
// IMPLEMENTATIONS zone includes it at FILE SCOPE. Definitions are
// `inline` free data/functions.

namespace t7 {
namespace the_board {

// ─── Shared no-op adapters ────────────────────────────────────────
//
// Families with no CPU mesh-generation stage share one pair instead
// of carrying per-family stubs. Same observable behavior: prepare
// reports nothing-to-do; dispatch is a no-op.

inline bool dispatch_prepare_mesh_none(Cartridge* self, wgpu::Queue& queue) {
    (void)self; (void)queue;
    return false;
}
inline void dispatch_mesh_gen_none(Cartridge* self, wgpu::ComputePassEncoder& pass) {
    (void)self; (void)pass;
}

// ─── The table ─────────────────────────────────────────────────────
// Rows in PopFamily order; the antenna row reuses column's mesh pair
// (the two families co-own one GPU mesh store — the named weld).

inline const FamilyDispatch FAMILY_DISPATCH[PopFamily::COUNT] = {
    { Cartridge::dispatch_select_pyramid_generic, Cartridge::dispatch_place_pyramid_generic, Cartridge::dispatch_commit_pyramid_generic,
      evict_pyramid, Cartridge::dispatch_prepare_mesh_pyramid, Cartridge::dispatch_mesh_gen_pyramid,
      "pyr" },
    { Cartridge::dispatch_select_arch_generic, Cartridge::dispatch_place_arch_generic, Cartridge::dispatch_commit_arch_generic,
      evict_arch,    Cartridge::dispatch_prepare_mesh_arch,    Cartridge::dispatch_mesh_gen_arch,
      "arch" },
    { Cartridge::dispatch_select_column_generic, Cartridge::dispatch_place_column_generic, Cartridge::dispatch_commit_column_generic,
      evict_column,  Cartridge::dispatch_prepare_mesh_column,  Cartridge::dispatch_mesh_gen_column,
      "col" },
    { Cartridge::dispatch_select_antenna_generic, Cartridge::dispatch_place_antenna_generic, Cartridge::dispatch_commit_antenna_generic,
      evict_antenna, Cartridge::dispatch_prepare_mesh_column,  Cartridge::dispatch_mesh_gen_column,
      "ant" },
    { dispatch_select_palm_generic, dispatch_place_palm_generic, dispatch_commit_palm_generic,
      evict_palm,   Cartridge::dispatch_prepare_mesh_palm,   Cartridge::dispatch_mesh_gen_palm,
      "palm" },
    { dispatch_select_cactus_generic, dispatch_place_cactus_generic, dispatch_commit_cactus_generic,
      evict_cactus, Cartridge::dispatch_prepare_mesh_cactus, Cartridge::dispatch_mesh_gen_cactus,
      "cact" },
    { dispatch_select_blade_generic, dispatch_place_blade_generic, dispatch_commit_blade_generic,
      evict_blade, Cartridge::dispatch_prepare_mesh_blade, Cartridge::dispatch_mesh_gen_blade,
      "blad" },
    { dispatch_select_sphere_generic, dispatch_place_sphere_generic, dispatch_commit_sphere_generic,
      evict_sphere, dispatch_prepare_mesh_none, dispatch_mesh_gen_none,
      "sph" },   // no CPU mesh gen — GPU compute handles update_sphere
    { dispatch_select_ribbon, dispatch_place_ribbon, dispatch_commit_ribbon,
      evict_ribbon, dispatch_prepare_mesh_none, dispatch_mesh_gen_none,
      "ribn" },  // no CPU mesh gen — GPU compute handles ribbon rendering
    { dispatch_select_cube_generic, dispatch_place_cube_generic, dispatch_commit_cube_generic,
      evict_cube, dispatch_prepare_mesh_none, dispatch_mesh_gen_none,
      "cube" },  // no CPU mesh gen — GPU compute handles update_cube
    { dispatch_select_gol, dispatch_place_gol, dispatch_commit_gol,
      evict_gol, dispatch_prepare_mesh_none, dispatch_mesh_gen_none,
      "gol" },   // zone mesh gen is a separate compute pass
    { dispatch_select_gallery, dispatch_place_gallery, dispatch_commit_gallery,
      evict_gallery, dispatch_prepare_mesh_none, dispatch_mesh_gen_none,
      "gall" },
};

} // namespace the_board
} // namespace t7
