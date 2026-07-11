// ─── entities.inl (IMPL: post-class definitions) ─────────────────
//
// Definitions for entities.hpp's declared preparers. Included AFTER the
// Cartridge class (LADDER-2 c1 header/impl split) so the keyhole is a
// complete type — each preparer dereferences c->gpuState_'s index-count
// setters. Vocabulary + EntitiesState + declarations live in entities.hpp
// (file scope, above the class).
//
// WRAPPING FORM (the proven fix-2 rule): this file is SELF-WRAPPING — it
// opens t7::the_board itself — so the MODULE IMPLEMENTATIONS zone includes
// it at FILE SCOPE, after the namespace closes. Definitions are `inline`
// free functions.
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ MESH-GEN PREPARERS ═══════════════════════════════════════════
//
// Per-family CPU-side mesh-gen prep. Each preparer:
//   • Reads the family's *_mesh_gen_pending flag and clears it
//   • Scans the family's active array to find the highest active slot
//   • Uploads index count = (max_slot + 1) * indices_per_slot to GPU
//
// Counterparts for arch / column / antenna / pyramid live here too:
// migration #10 hoisted them out of spawn_engine.inl alongside the
// remaining helpers.

inline bool prepare_palm_mesh_gen(EntitiesState& es, Cartridge* c, wgpu::Queue& queue) {
    (void)queue;
    if (!es.palm_mesh_gen_pending) return false;
    es.palm_mesh_gen_pending = false;
    uint32_t maxSlot = 0;
    bool anyActive = false;
    for (uint32_t i = 0; i < Dim::MAX_PALM_INSTANCES; i++) {
        if (es.palms[i].active) { maxSlot = i; anyActive = true; }
    }
    c->gpuState_.set_palm_index_count(anyActive
        ? (maxSlot + 1) * Dim::PALMG_MAX_INDICES_PER_SLOT : 0);
    return true;
}

inline bool prepare_cactus_mesh_gen(EntitiesState& es, Cartridge* c, wgpu::Queue& queue) {
    (void)queue;
    if (!es.cactus_mesh_gen_pending) return false;
    es.cactus_mesh_gen_pending = false;
    uint32_t maxSlot = 0;
    bool anyActive = false;
    for (uint32_t i = 0; i < Dim::MAX_CACTUS_INSTANCES; i++) {
        if (es.cacti[i].active) { maxSlot = i; anyActive = true; }
    }
    c->gpuState_.set_cactus_index_count(anyActive
        ? (maxSlot + 1) * Dim::CACTUSG_MAX_INDICES_PER_SLOT : 0);
    return true;
}

inline bool prepare_blade_mesh_gen(EntitiesState& es, Cartridge* c, wgpu::Queue& queue) {
    (void)queue;
    if (!es.blade_mesh_gen_pending) return false;
    es.blade_mesh_gen_pending = false;
    uint32_t maxSlot = 0;
    bool anyActive = false;
    for (uint32_t i = 0; i < Dim::MAX_BLADE_INSTANCES; i++) {
        if (es.blades[i].active) { maxSlot = i; anyActive = true; }
    }
    c->gpuState_.set_blade_index_count(anyActive
        ? (maxSlot + 1) * Dim::BLADEG_MAX_INDICES_PER_SLOT : 0);
    return true;
}

inline bool prepare_column_mesh_gen(EntitiesState& es, Cartridge* c, wgpu::Queue& queue) {
    (void)queue;
    if (!es.column_mesh_gen_pending) return false;
    es.column_mesh_gen_pending = false;

    uint32_t maxSlot = 0;
    bool anyActive = false;
    for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++) {
        if (es.columns[i].active) { maxSlot = i; anyActive = true; }
    }
    for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++) {
        if (es.antennas[i].active) {
            maxSlot = i + Dim::ANTENNA_SLOT_OFFSET;
            anyActive = true;
        }
    }
    c->gpuState_.set_column_index_count(anyActive
        ? (maxSlot + 1) * Dim::CMG_MAX_INDICES_PER_SLOT : 0);
    return true;
}

inline bool prepare_arch_mesh_gen(EntitiesState& es, Cartridge* c, wgpu::Queue& queue) {
    (void)queue;
    if (!es.arch_mesh_gen_pending) return false;
    es.arch_mesh_gen_pending = false;

    uint32_t maxSlot = 0;
    bool anyActive = false;
    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
        if (es.arches[i].active) { maxSlot = i; anyActive = true; }
    }
    c->gpuState_.set_arch_index_count(anyActive
        ? (maxSlot + 1) * Dim::AMG_MAX_INDICES_PER_SLOT : 0);
    return true;
}

inline bool prepare_pyramid_mesh_gen(EntitiesState& es, Cartridge* c, wgpu::Queue& queue) {
    (void)queue;
    if (!es.pyramid_mesh_gen_pending) return false;
    es.pyramid_mesh_gen_pending = false;

    uint32_t maxSlot = 0;
    bool anyActive = false;
    for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) {
        if (es.pyramids[i].active) { maxSlot = i; anyActive = true; }
    }
    c->gpuState_.set_pyramid_index_count(anyActive
        ? (maxSlot + 1) * Dim::PMG_MAX_INDICES_PER_SLOT : 0);
    return true;
}

} // namespace the_board
} // namespace t7
