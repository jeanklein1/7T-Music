// ─── spawn_engine.inl (S3 · IMPL: post-class definitions) ─────────
// Converted at LADDER-6; payload relocations (LADDER-3): history in
// audit/LADDER.md.
//
// The engine's verbs: position negotiation, the footprint registry,
// mesh-param rebuilds + distance culling, the census, gate
// evaluation, proximity affinity, and the select → place → commit
// dispatch loops. Reaches the keyhole for the root organs
// (c->world_state_ / c->time_state_ / c->mood_state_ /
// c->themes_state_ / c->tile_world_state_ / c->entities_state_ /
// c->player_) and the GPU wire (c->gpuState_); the loops route
// through FAMILY_DISPATCH with the keyhole as the row argument.
//
// WRAPPING FORM (fix-2): SELF-WRAPPING — the zone includes impls at FILE SCOPE; law in audit/LADDER.md.

#include <cmath>      // std::floor, std::sqrt, std::min/max companions
#include <algorithm>  // std::min, std::max
#include <iostream>   // census + DIAG stdout
#include <iomanip>    // census column formatting

namespace t7 {
namespace the_board {

// ── Helper 2: NegotiatePosition ─────────────────────────────

inline PositionResult negotiate_position(Cartridge* c,
    uint32_t seed, int32_t trigger_gx, int32_t trigger_gz,
    uint32_t pos_x_prop, uint32_t pos_z_prop, float jitter,
    uint32_t rotation_seed_prop,
    float footprint_r, uint32_t family, uint32_t tier)
{
    PositionResult r{};
    r.ok = false;

    // 1. Jittered position
    jittered_position(seed, trigger_gx, trigger_gz,
        pos_x_prop, pos_z_prop, jitter, r.cx, r.cz);
    r.rotation = cpu_hash_f(seed, rotation_seed_prop) * 6.283185f;

    //
    // In finite indoor worlds, push the candidate inward so the
    // entity's footprint stays at least INDOOR_ENTITY_WALL_MARGIN
    // from every wall. We clamp instead of rejecting because
    // rejection would silently drop entities anchored to corner
    // patches (their seed-determined position keeps landing in
    // the wall margin and never recovers). Clamping shifts the
    // candidate to the boundary of the legal box, then the
    // existing footprint-overlap check handles any pile-ups.
    //
    // If the room is too small for the entity plus margins on
    // both sides (lo > hi), we clamp to the room center —
    // shouldn't happen for typical indoor entities (max
    // footprint at radius=1 is 65; rescaled entities are well
    // under that).
    if (c->world_state_.finite_mode && MOOD_TABLE[c->mood_state_.active].indoor) {
        float bmin = -(float)c->world_state_.finite_radius * PATCH_EXTENT;
        float bmax = ((float)c->world_state_.finite_radius + 1.0f) * PATCH_EXTENT;
        float clearance = INDOOR_ENTITY_WALL_MARGIN + footprint_r;
        float lo = bmin + clearance;
        float hi = bmax - clearance;
        if (lo > hi) {
            float center = (bmin + bmax) * 0.5f;
            r.cx = center;
            r.cz = center;
        }
        else {
            if (r.cx < lo) r.cx = lo;
            else if (r.cx > hi) r.cx = hi;
            if (r.cz < lo) r.cz = lo;
            else if (r.cz > hi) r.cz = hi;
        }
    }

    // 2. Separation + footprint check (single pass)
    if (!check_position(c, r.cx, r.cz, footprint_r, family))
        return r;

    // 3. Host patch + footprint registration
    r.host_gx = (int32_t)std::floor(r.cx / PATCH_EXTENT);
    r.host_gz = (int32_t)std::floor(r.cz / PATCH_EXTENT);
    if (register_footprint(c, r.cx, r.cz, footprint_r,
        r.host_gx, r.host_gz, family, tier) == UINT32_MAX) return r;

    r.ok = true;
    return r;
}

// ── Helper 3: record_placement_bookkeeping ──────────────────

inline void record_placement_bookkeeping(uint32_t /*family*/, uint32_t /*tier_idx*/)
{
}

// ═══ MESH GEN PREPARERS + CULLING ════════════════════════════════

// ─── Column / Arch / Pyramid mesh-gen preparers ───────────────

// Rebuild GPUArchMeshParams from cached ActiveArch data.
inline GPUArchMeshParams build_arch_mesh_params(Cartridge* c, uint32_t slot) {
    const auto& a = c->entities_state_.arches[slot];
    GPUArchMeshParams p{};
    p.center_x = a.world_x;
    p.center_z = a.world_z;
    p.rotation = a.rotation;
    p.half_span = a.half_span;
    p.rise = a.rise;
    p.depth = a.depth;
    p.thickness = a.thickness;
    p.pier_height = a.pier_height;
    p.burial = a.burial;
    p.catenary_a = solve_catenary_a(a.half_span, a.rise);
    p.segs_u = a.segs_u;
    p.segs_v = a.segs_v;
    // Portal color override (mirrors spawn logic)
    if (a.is_portal) {
        const float* pc = a.is_back_portal
            ? PORTAL_COLOR_BACK
            : PORTAL_COLORS[a.destination.mood % MOOD_COUNT];
        p.color_r = pc[0]; p.color_g = pc[1]; p.color_b = pc[2];
    }
    else {
        p.color_r = a.col_r; p.color_g = a.col_g; p.color_b = a.col_b;
    }
    p.is_active = 1;
    return p;
}

// Rebuild GPUColumnMeshParams from cached ActiveColumn data.
inline GPUColumnMeshParams build_column_mesh_params_from(const ActiveColumn& c) {
    GPUColumnMeshParams p{};
    p.center_x = c.world_x;
    p.center_z = c.world_z;
    p.height = c.height;
    p.shaft_radius = c.shaft_radius;
    p.taper = c.taper;
    p.entasis = c.entasis;
    p.base_height = c.base_height;
    p.base_overhang = c.base_overhang;
    p.capital_height = c.cap_height;
    p.capital_overhang = c.cap_overhang;
    p.burial = c.burial;
    p.color_r = c.col_r;
    p.color_g = c.col_g;
    p.color_b = c.col_b;
    p.base_layers = c.base_layers;
    p.capital_layers = c.cap_layers;
    p.segs_around = c.segs_around;
    p.shaft_rings = c.shaft_rings;
    p.is_active = 1;
    p.tier = c.tier_idx;
    p.drum_color_r1 = c.drum_colors[0];
    p.drum_color_g1 = c.drum_colors[1];
    p.drum_color_b1 = c.drum_colors[2];
    p.drum_color_r2 = c.drum_colors[3];
    p.drum_color_g2 = c.drum_colors[4];
    p.drum_color_b2 = c.drum_colors[5];
    p.drum_color_r3 = c.drum_colors[6];
    p.drum_color_g3 = c.drum_colors[7];
    p.drum_color_b3 = c.drum_colors[8];
    return p;
}

inline GPUColumnMeshParams build_column_mesh_params(Cartridge* c, uint32_t slot) {
    return build_column_mesh_params_from(c->entities_state_.columns[slot]);
}

// Scan all active entities, toggle draw_visible with hysteresis,
// and upload mesh param changes. Returns count of currently hidden entities.
inline uint32_t update_entity_draw_visibility(Cartridge* c, wgpu::Queue& queue) {
    uint32_t culled = 0;

    const float cull_base = VISIBILITY_CYLINDER_RADIUS - ENTITY_CULL_EDGE_MARGIN;  // 275 − 25 = 250

    // Arches
    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
        if (!c->entities_state_.arches[i].active) continue;
        const auto& a = c->entities_state_.arches[i];
        float dx = a.world_x - c->player_.readback_x;
        float dz = a.world_z - c->player_.readback_z;
        float dist = std::sqrt(dx * dx + dz * dz);

        float entity_size = std::max(a.half_span * 2.0f, a.total_height);
        float inset = std::min(entity_size * ENTITY_CULL_SIZE_INSET, ENTITY_CULL_SIZE_INSET_MAX);
        float cull_far  = cull_base - inset;                 // taller ⇒ earlier, never past the edge
        float cull_near = cull_far - ENTITY_CULL_HYSTERESIS; // show only when this far inside

        bool should_show = a.draw_visible
            ? (dist <= cull_far)          // visible: hide at the (inset) edge
            : (dist <= cull_near);        // hidden:  show when comfortably inside

        if (should_show != a.draw_visible) {
            c->entities_state_.arches[i].draw_visible = should_show;
            if (should_show) {
                c->gpuState_.upload_arch_mesh_params_slot(queue, i, build_arch_mesh_params(c, i));
            }
            else {
                GPUArchMeshParams empty{};
                c->gpuState_.upload_arch_mesh_params_slot(queue, i, empty);
            }
            c->entities_state_.arch_mesh_gen_pending = true;
        }

        if (!c->entities_state_.arches[i].draw_visible) culled++;
    }

    // Columns
    for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++) {
        if (!c->entities_state_.columns[i].active) continue;
        const auto& c = c->entities_state_.columns[i];
        float dx = c.world_x - c->player_.readback_x;
        float dz = c.world_z - c->player_.readback_z;
        float dist = std::sqrt(dx * dx + dz * dz);

        float inset = std::min(c.height * ENTITY_CULL_SIZE_INSET, ENTITY_CULL_SIZE_INSET_MAX);
        float cull_far  = cull_base - inset;
        float cull_near = cull_far - ENTITY_CULL_HYSTERESIS;

        bool should_show = c.draw_visible
            ? (dist <= cull_far)
            : (dist <= cull_near);

        if (should_show != c.draw_visible) {
            c->entities_state_.columns[i].draw_visible = should_show;
            if (should_show) {
                c->gpuState_.upload_column_mesh_params_slot(queue, i, build_column_mesh_params(c, i));
            }
            else {
                GPUColumnMeshParams empty{};
                c->gpuState_.upload_column_mesh_params_slot(queue, i, empty);
            }
            c->entities_state_.column_mesh_gen_pending = true;
        }

        if (!c->entities_state_.columns[i].draw_visible) culled++;
    }

    // Antennas
    for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++) {
        if (!c->entities_state_.antennas[i].active) continue;
        const auto& c = c->entities_state_.antennas[i];
        float dx = c.world_x - c->player_.readback_x;
        float dz = c.world_z - c->player_.readback_z;
        float dist = std::sqrt(dx * dx + dz * dz);
        uint32_t gpu_slot = i + Dim::ANTENNA_SLOT_OFFSET;

        float inset = std::min(c.height * ENTITY_CULL_SIZE_INSET, ENTITY_CULL_SIZE_INSET_MAX);
        float cull_far  = cull_base - inset;
        float cull_near = cull_far - ENTITY_CULL_HYSTERESIS;

        bool should_show = c.draw_visible
            ? (dist <= cull_far)
            : (dist <= cull_near);

        if (should_show != c.draw_visible) {
            c->entities_state_.antennas[i].draw_visible = should_show;
            if (should_show) {
                c->gpuState_.upload_column_mesh_params_slot(queue, gpu_slot, build_column_mesh_params_from(c));
            }
            else {
                GPUColumnMeshParams empty{};
                c->gpuState_.upload_column_mesh_params_slot(queue, gpu_slot, empty);
            }
            c->entities_state_.column_mesh_gen_pending = true;
        }

        if (!c->entities_state_.antennas[i].draw_visible) culled++;
    }

    return culled;
}

// ═══ FOOTPRINT REGISTRY ══════════════════════════════════════════

inline bool check_position(Cartridge* c, float px, float pz, float placing_radius,
    uint32_t placing_family) {
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (!c->spawn_engine_state_.footprints_[i].active) continue;
        float dx = px - c->spawn_engine_state_.footprints_[i].x;
        float dz = pz - c->spawn_engine_state_.footprints_[i].z;
        float effective_min = placing_radius + c->spawn_engine_state_.footprints_[i].radius;
        if (c->spawn_engine_state_.footprints_[i].family < PopFamily::COUNT) {
            float min_gap = MIN_SEPARATION[placing_family][c->spawn_engine_state_.footprints_[i].family];
            if (min_gap > 0.0f) {
                float aff = PROXIMITY_AFFINITY[placing_family][c->spawn_engine_state_.footprints_[i].family];
                if (aff > 0.0f) min_gap *= (1.0f - aff * PROXIMITY_GAP_REDUCTION[placing_family]);
                effective_min += min_gap;
            }
        }
        if (dx * dx + dz * dz < effective_min * effective_min) return false;
    }
    return true;
}

inline uint32_t register_footprint(Cartridge* c, float x, float z, float radius,
    int32_t gx, int32_t gz, uint32_t family,
    uint32_t tier) {
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (!c->spawn_engine_state_.footprints_[i].active) {
            c->spawn_engine_state_.footprints_[i] = { x, z, radius, gx, gz, family, tier, c->time_state_.seconds, true };
            return i;
        }
    }
    return UINT32_MAX;  // full — entity should not spawn
}

inline void unregister_footprints_for_patch(Cartridge* c, int32_t gx, int32_t gz) {
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (c->spawn_engine_state_.footprints_[i].active &&
            c->spawn_engine_state_.footprints_[i].patch_gx == gx && c->spawn_engine_state_.footprints_[i].patch_gz == gz) {
            c->spawn_engine_state_.footprints_[i].active = false;
        }
    }
}

// ═══ ENTITY CENSUS ═══════════════════════════════════════════════

inline const char* family_short_name(uint32_t family) {
    static const char* NAMES[] = { "pyr", "arch", "col", "ant", "palm", "cact", "blad", "sph", "ribn", "cube", "gol", "gall" };
    return (family < PopFamily::COUNT) ? NAMES[family] : "???";
}

inline void dump_entity_census(Cartridge* c, const char* trigger) {
    uint32_t count = 0;
    uint32_t by_family[PopFamily::COUNT] = {};
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (!c->spawn_engine_state_.footprints_[i].active) continue;
        if (c->spawn_engine_state_.footprints_[i].family >= PopFamily::COUNT) continue;
        count++;
        by_family[c->spawn_engine_state_.footprints_[i].family]++;
    }

    std::cout << "[CENSUS t=" << std::fixed << std::setprecision(1) << c->time_state_.seconds
        << " trigger=" << trigger << "] " << count << " entities (";
    for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
        if (f > 0) std::cout << " ";
        std::cout << family_short_name(f) << ":" << by_family[f];
    }
    std::cout << ")\n";

    // Per-entity detail, sorted by family then spawn_time
    struct CensusEntry { uint32_t fp_idx; uint32_t family; uint32_t tier; float spawn_time; };
    CensusEntry entries[MAX_FOOTPRINTS];
    uint32_t n = 0;
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (!c->spawn_engine_state_.footprints_[i].active || c->spawn_engine_state_.footprints_[i].family >= PopFamily::COUNT) continue;
        entries[n++] = { i, c->spawn_engine_state_.footprints_[i].family, c->spawn_engine_state_.footprints_[i].tier, c->spawn_engine_state_.footprints_[i].spawn_time };
    }
    // Insertion sort by (family, spawn_time)
    for (uint32_t i = 1; i < n; i++) {
        CensusEntry key = entries[i]; uint32_t j = i;
        while (j > 0 && (entries[j - 1].family > key.family ||
            (entries[j - 1].family == key.family && entries[j - 1].spawn_time > key.spawn_time))) {
            entries[j] = entries[j - 1]; j--;
        }
        entries[j] = key;
    }
    for (uint32_t i = 0; i < n; i++) {
        const auto& fp = c->spawn_engine_state_.footprints_[entries[i].fp_idx];
        std::cout << "  " << family_short_name(fp.family)
            << " t" << fp.tier
            << " (" << std::setw(8) << std::setprecision(1) << fp.x
            << "," << std::setw(8) << fp.z << ")"
            << " p(" << std::setw(3) << fp.patch_gx << "," << std::setw(3) << fp.patch_gz << ")"
            << " age=" << std::setprecision(1) << (c->time_state_.seconds - fp.spawn_time)
            << "\n";
    }
    std::cout << std::flush;
}

// ═══ SPAWN UTILITIES ═════════════════════════════════════════════
//
// The spawn lifecycle's smallest building blocks: gate evaluation,
// jittered position, the family enum, the global density dial,
// and two load-bearing tag tables (Spawn Configuration Summary,
// Property Index Registry) that document the contracts every family
// participates in.

// ─── Spawn gate ──────────────────────────────────────────────────

// Evaluate the spawn gate: seed + flat probability check.
// adjacency_mod is a multiplier from the full spawn cascade.
inline SpawnPreamble evaluate_spawn_gate(Cartridge* c, int32_t gx, int32_t gz,
    uint32_t spawn_roll_prop,
    float spawn_chance,
    float adjacency_mod) {
    SpawnPreamble result{};
    result.archetype = 1;
    auto tile_it = c->tile_world_state_.tileCache_.find({ gx, gz });
    if (tile_it != c->tile_world_state_.tileCache_.end()) result.archetype = tile_it->second.archetype;

    result.seed = tile_seed(c->world_state_.active_seed, gx, gz);
    float chance = std::min(spawn_chance * adjacency_mod, 1.0f);
    result.passed = cpu_hash_f(result.seed, spawn_roll_prop) < chance;
    return result;
}

// Jittered world position within a patch.
inline void jittered_position(uint32_t seed, int32_t gx, int32_t gz,
    uint32_t prop_x, uint32_t prop_z, float jitter,
    float& out_x, float& out_z) {
    out_x = (gx + 0.5f) * PATCH_EXTENT + (cpu_hash_f(seed, prop_x) - 0.5f) * PATCH_EXTENT * jitter;
    out_z = (gz + 0.5f) * PATCH_EXTENT + (cpu_hash_f(seed, prop_z) - 0.5f) * PATCH_EXTENT * jitter;
}

inline float proximity_affinity_boost(Cartridge* c, float cx, float cz, uint32_t family) {
    if (!proximity_row_active(family)) return 1.0f;
    float radius = PROXIMITY_RADIUS[family];
    if (radius <= 0.0f) return 1.0f;
    float r2 = radius * radius;
    float weighted = 0.0f;
    uint32_t count = 0;
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (!c->spawn_engine_state_.footprints_[i].active) continue;
        if (c->spawn_engine_state_.footprints_[i].family >= PopFamily::COUNT) continue;
        float aff = PROXIMITY_AFFINITY[family][c->spawn_engine_state_.footprints_[i].family];
        if (aff <= 0.0f) continue;
        float dx = cx - c->spawn_engine_state_.footprints_[i].x;
        float dz = cz - c->spawn_engine_state_.footprints_[i].z;
        if (dx * dx + dz * dz < r2) {
            weighted += aff;
            count++;
        }
    }
    if (count < PROXIMITY_THRESHOLD[family]) return 1.0f;
    return std::min(1.0f + weighted, PROXIMITY_MAX_BOOST[family]);
}

// ─── Select / Place / Commit dispatch loops ─────────────────────

inline void select_entities_for_patch(Cartridge* c, int32_t gx, int32_t gz) {
    for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
        if (!ROSTER.family_enabled(f)) continue;  // ROSTER-GATE family (b) — disabled family never selected -> never placed/committed/meshed/drawn. Budgeted stream path, not the per-frame hot path.
        EntityQueueEntry e{};
        e.family = f;
        e.gx = gx; e.gz = gz;
        if (FAMILY_DISPATCH[f].try_select(c, gx, gz, e))
            c->spawn_engine_state_.entityQueue_.push_back(e);
    }
}

// ─── Place: spatial negotiation (no GPU writes) ──────────────

inline void place_entity_queue(Cartridge* c) {
    for (auto& e : c->spawn_engine_state_.entityQueue_) {
        PlacementEntry pe{};
        if (FAMILY_DISPATCH[e.family].try_place(c, e, pe))
            c->spawn_engine_state_.placementResults_.push_back(pe);
    }
    c->spawn_engine_state_.entityQueue_.clear();
}

// ─── Commit: GPU writes from placement results ──────────────

inline void commit_entity_queue(Cartridge* c, wgpu::Queue& queue) {
    for (auto& pe : c->spawn_engine_state_.placementResults_)
        FAMILY_DISPATCH[pe.family].try_commit(c, pe, queue);
    c->spawn_engine_state_.placementResults_.clear();
}

} // namespace the_board
} // namespace t7
