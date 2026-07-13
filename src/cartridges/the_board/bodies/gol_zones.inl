// ─── gol_zones.inl (IMPL: post-class definitions) ────────────────
// Impl of gol_zones.hpp (LADDER-3 c1): history in audit/LADDER.md.
//
// Definitions for gol_zones.hpp's declared lifecycle + per-frame
// functions. The bodies reach c->gpuState_ / c->renderer_ / c->device_ /
// the S2 boundary faces (tile_world.hpp, m3b) / c->mood_state_ / c->world_state_ / c->time_state_ and
// the spine services (check_position / register_footprint /
// record_placement_bookkeeping — spawn_engine.hpp), plus
// GLOBAL_ENTITY_DENSITY (spawn_engine.hpp) and PATCH_EXTENT
// (patch_system.hpp); PopFamily is roster.hpp vocabulary.
//
// WRAPPING FORM (fix-2): SELF-WRAPPING — the zone includes impls at FILE SCOPE; law in audit/LADDER.md.
// ─────────────────────────────────────────────────────────────────

#include <cmath>      // std::floor
#include <algorithm>  // std::max, std::min
#include <iostream>   // the spawn log
#include <vector>     // life / height-factor staging

namespace t7 {
namespace the_board {

// ═══ LIFECYCLE — three-phase + helper ════════════════════════════

// ─── select_gol_for_patch ─────────────────────────────────────

inline bool select_gol_for_patch(GoLState& gs, Cartridge* c,
    int32_t gx, int32_t gz, GoLSelection& sel) {
    // Mood gate
    float adj_mod = GoLZoneSpawnConfig::MOOD_MULTIPLIER[c->mood_state_.active];
    if (adj_mod <= 0.0f) return false;

    // Density + theme modifiers
    adj_mod *= GLOBAL_ENTITY_DENSITY;
    tile_apply_spawn_mult(c->tile_world_state_, gx, gz, PopFamily::GOL, adj_mod);  // F3 (m3b)

    // Scan lattice nodes overlapping this patch
    float wx0 = gx * PATCH_EXTENT;
    float wx1 = (gx + 1) * PATCH_EXTENT;
    float wz0 = gz * PATCH_EXTENT;
    float wz1 = (gz + 1) * PATCH_EXTENT;

    int32_t nx0 = (int32_t)std::floor(wx0 / MODE_LATTICE_SPACING);
    int32_t nx1 = (int32_t)std::floor(wx1 / MODE_LATTICE_SPACING);
    int32_t nz0 = (int32_t)std::floor(wz0 / MODE_LATTICE_SPACING);
    int32_t nz1 = (int32_t)std::floor(wz1 / MODE_LATTICE_SPACING);

    for (int32_t nz = nz0; nz <= nz1; nz++) {
        for (int32_t nx = nx0; nx <= nx1; nx++) {
            // Zone center from lattice node
            float raw_cx = (nx + 0.5f) * MODE_LATTICE_SPACING;
            float raw_cz = (nz + 0.5f) * MODE_LATTICE_SPACING;

            // Authoritative patch: only the patch containing the center owns this node
            int32_t auth_gx = (int32_t)std::floor(raw_cx / PATCH_EXTENT);
            int32_t auth_gz = (int32_t)std::floor(raw_cz / PATCH_EXTENT);
            if (auth_gx != gx || auth_gz != gz) continue;

            // Idempotency: already active at this node?
            bool exists = false;
            for (uint32_t i = 0; i < Dim::MAX_GOL_ZONES; i++) {
                if (gs.zones[i].active &&
                    gs.zones[i].zone_nx == nx && gs.zones[i].zone_nz == nz) {
                    exists = true; break;
                }
            }
            if (exists) continue;

            // Spawn roll with modifiers
            uint32_t seed = cpu_lattice_node_seed(c->world_state_.active_seed, nx, nz, GoLZoneProp::SEED_BAND);
            float roll = cpu_hash_f(seed, GoLZoneProp::SPAWN_ROLL);
            float chance = GoLZoneSpawnConfig::SPAWN_CHANCE * adj_mod;
            chance = std::max(0.0f, std::min(1.0f, chance));
            if (roll >= chance) continue;

            // Find free slot
            uint32_t slot = UINT32_MAX;
            for (uint32_t i = 0; i < Dim::MAX_GOL_ZONES; i++) {
                if (!gs.zones[i].active) { slot = i; break; }
            }
            if (slot == UINT32_MAX) continue;

            // Reserve slot
            gs.zones[slot].active = true;

            // Zone corner (cell-grid-snapped)
            float corner_x = std::floor(
                (raw_cx - GoLZoneSpawnConfig::ZONE_EXTENT * 0.5f) / PATCH_CELL_SIZE) * PATCH_CELL_SIZE;
            float corner_z = std::floor(
                (raw_cz - GoLZoneSpawnConfig::ZONE_EXTENT * 0.5f) / PATCH_CELL_SIZE) * PATCH_CELL_SIZE;

            // Algorithm selection
            float algo_roll = cpu_hash_f(seed, PulseZoneProp::ALGORITHM_ROLL);
            uint32_t algorithm = (algo_roll < PULSE_ALGORITHM_CHANCE)
                ? AlgorithmType::PULSE : AlgorithmType::CONWAY;

            // Height enabled
            float height_roll = cpu_hash_f(seed, GoLZoneProp::HEIGHT_ROLL);
            bool height_enabled = (height_roll < GoLZoneSpawnConfig::HEIGHT_CHANCE);

            // Tier + CPU-side params
            float tick_period = 1.0f;
            float initial_density = 0.0f;
            uint32_t tier_idx = 0;

            if (algorithm == AlgorithmType::CONWAY) {
                float w[GOL_TIER_COUNT];
                for (uint32_t t = 0; t < GOL_TIER_COUNT; t++) w[t] = GOL_TIERS[t].weight;
                uint32_t tier = select_tier(seed, GoLZoneProp::TIER, w, GOL_TIER_COUNT);
                const auto& tp = GOL_TIERS[tier];
                if (tp.force_no_height) height_enabled = false;
                tick_period = std::max(0.1f,
                    cpu_sample_gaussian(seed, GoLZoneProp::TICK_PERIOD,
                        tp.tick_period_mean, tp.tick_period_sigma));
                initial_density = std::max(0.05f, std::min(0.9f,
                    cpu_sample_gaussian(seed, GoLZoneProp::DENSITY,
                        tp.density_mean, tp.density_sigma)));
                tier_idx = tier;  // Conway: 0–6
            }
            else {
                float w[PULSE_TIER_COUNT];
                for (uint32_t t = 0; t < PULSE_TIER_COUNT; t++) w[t] = PULSE_TIERS[t].weight;
                uint32_t tier = select_tier(seed, PulseZoneProp::PULSE_TIER, w, PULSE_TIER_COUNT);
                const auto& pp = PULSE_TIERS[tier];
                if (pp.force_no_height) height_enabled = false;
                tick_period = std::max(0.1f,
                    cpu_sample_gaussian(seed, GoLZoneProp::TICK_PERIOD,
                        pp.tick_period_mean, pp.tick_period_sigma));
                initial_density = 0.0f;
                tier_idx = GOL_TIER_COUNT + tier;  // Pulse: 7–9 (compound index)
            }

            // Fill selection
            sel.seed = seed;
            sel.trigger_gx = gx;
            sel.trigger_gz = gz;
            sel.slot = slot;
            sel.zone_nx = nx;
            sel.zone_nz = nz;
            sel.corner_x = corner_x;
            sel.corner_z = corner_z;
            sel.algorithm = algorithm;
            sel.tier_idx = tier_idx;
            sel.tick_period = tick_period;
            sel.initial_density = initial_density;
            sel.height_enabled = height_enabled;
            sel.footprint_r = GoLZoneSpawnConfig::FOOTPRINT_RADIUS;

            return true;  // at most one zone per patch
        }
    }
    return false;
}

// ─── place_gol_from_selection ─────────────────────────────────

inline bool place_gol_from_selection(Cartridge* c,
    const GoLSelection& sel, GoLPlacement& plan) {
    float cx = sel.corner_x + GoLZoneSpawnConfig::ZONE_EXTENT * 0.5f;
    float cz = sel.corner_z + GoLZoneSpawnConfig::ZONE_EXTENT * 0.5f;

    if (!check_position(c, cx, cz, sel.footprint_r, PopFamily::GOL))
        return false;

    int32_t host_gx = (int32_t)std::floor(cx / PATCH_EXTENT);
    int32_t host_gz = (int32_t)std::floor(cz / PATCH_EXTENT);

    if (register_footprint(c, cx, cz, sel.footprint_r,
        host_gx, host_gz, PopFamily::GOL, sel.tier_idx) == UINT32_MAX)
        return false;

    plan = GoLPlacement{};
    plan.slot = sel.slot;
    plan.trigger_gx = sel.trigger_gx;
    plan.trigger_gz = sel.trigger_gz;
    plan.host_gx = host_gx;
    plan.host_gz = host_gz;
    plan.tier_idx = sel.tier_idx;
    plan.cx = cx;
    plan.cz = cz;
    plan.zone_nx = sel.zone_nx;
    plan.zone_nz = sel.zone_nz;
    plan.corner_x = sel.corner_x;
    plan.corner_z = sel.corner_z;
    plan.algorithm = sel.algorithm;
    plan.tick_period = sel.tick_period;
    plan.initial_density = sel.initial_density;
    plan.height_enabled = sel.height_enabled;

    record_placement_bookkeeping(PopFamily::GOL, plan.tier_idx);
    return true;
}

// ─── commit_gol ──────────────────────────────────────────────

inline void commit_gol(GoLState& gs, Cartridge* c,
    const GoLPlacement& plan,
    int32_t trigger_gx, int32_t trigger_gz, wgpu::Queue& queue)
{
    (void)trigger_gx; (void)trigger_gz;
    auto& zone = gs.zones[plan.slot];
    zone.zone_nx = plan.zone_nx;
    zone.zone_nz = plan.zone_nz;
    zone.host_gx = plan.host_gx;
    zone.host_gz = plan.host_gz;
    zone.active = true;
    zone.algorithm = plan.algorithm;
    zone.tick_period = plan.tick_period;
    zone.initial_density = plan.initial_density;
    zone.last_tick_index = -1;
    gs.zone_count++;

    seed_gol_zone(gs, c, plan.slot, queue);

    if (gs.pending_derive_requests.count < Dim::MAX_GOL_ZONES) {
        auto& req = gs.pending_derive_requests.requests[gs.pending_derive_requests.count++];
        req.slot = plan.slot;
        req.nx = plan.zone_nx;
        req.nz = plan.zone_nz;
        req.algorithm = plan.algorithm;
        req.height_enabled = plan.height_enabled ? 1u : 0u;
        req.world_seed = c->world_state_.active_seed;
    }

    std::cout << "[GoL] "
        << (plan.algorithm == AlgorithmType::PULSE ? "Pulse" : "Conway")
        << " slot=" << plan.slot
        << " node=(" << plan.zone_nx << "," << plan.zone_nz << ")"
        << " corner=(" << plan.corner_x << "," << plan.corner_z << ")"
        << " host=(" << plan.host_gx << "," << plan.host_gz << ")"
        << (plan.height_enabled ? " HEIGHT" : "")
        << " period=" << plan.tick_period
        << "\n";
}

// ─── seed_gol_zone ───────────────────────────────────────────

inline void seed_gol_zone(GoLState& gs, Cartridge* c,
    uint32_t slot, wgpu::Queue& queue) {
    auto& zone = gs.zones[slot];
    uint32_t seed = cpu_lattice_node_seed(c->world_state_.active_seed, zone.zone_nx, zone.zone_nz, GoLZoneProp::SEED_BAND);

    // Generate initial pattern
    std::vector<float> life(Dim::GOL_ZONE_CELLS, 0.0f);
    if (zone.algorithm == AlgorithmType::CONWAY) {
        // Conway: random alive/dead from density
        for (uint32_t i = 0; i < Dim::GOL_ZONE_CELLS; i++) {
            float roll = cpu_hash_f(seed + i, GoLZoneProp::DENSITY);
            life[i] = (roll < zone.initial_density) ? 1.0f : 0.0f;
        }
    }

    // Generate per-cell height factors: Gaussian draw, clamped
    std::vector<float> height_factors(Dim::GOL_ZONE_CELLS);
    for (uint32_t i = 0; i < Dim::GOL_ZONE_CELLS; i++) {
        float hf = cpu_sample_gaussian(seed + i, GoLZoneProp::HEIGHT_FACTOR,
            GoLZoneSpawnConfig::HEIGHT_FACTOR_MEAN, GoLZoneSpawnConfig::HEIGHT_FACTOR_SIGMA);
        height_factors[i] = std::max(GoLZoneSpawnConfig::HEIGHT_FACTOR_CLAMP_LO,
            std::min(GoLZoneSpawnConfig::HEIGHT_FACTOR_CLAMP_HI, hf));
    }

    // Upload all 7 slots
    c->gpuState_.upload_zone_life(queue, slot, life.data(), height_factors.data(), Dim::GOL_ZONE_CELLS);
}

// ═══ PER-FRAME UPLOAD ════════════════════════════════════════════

inline void upload_gol_zone_config(GoLState& gs, Cartridge* c, wgpu::Queue& queue) {
    uint32_t count = 0;
    uint32_t tick_mask = 0;

    for (uint32_t i = 0; i < Dim::MAX_GOL_ZONES; i++) {
        if (!gs.zones[i].active) continue;

        // Conway tick gating: exactly one tick per period
        float effective_period = std::max(gs.zones[i].tick_period, 0.01f);
        int32_t current_tick = (int32_t)std::floor(c->time_state_.beats / effective_period);
        if (current_tick != gs.zones[i].last_tick_index) {
            tick_mask |= (1u << i);
            gs.zones[i].last_tick_index = current_tick;
        }

        count = i + 1;
    }
    c->gpuState_.upload_zone_config_header(queue, count, c->time_state_.beats, c->time_state_.dt, tick_mask);
    gs.active_slot_count = count;
}

// Flush pending zone derive requests as a GPU compute dispatch.
// Called once per frame after all patch generation is complete.
inline void flush_zone_derive_requests(GoLState& gs, Cartridge* c, wgpu::Queue& queue) {
    if (gs.pending_derive_requests.count == 0) return;

    c->gpuState_.upload_zone_derive_requests(queue, gs.pending_derive_requests);

    wgpu::CommandEncoder encoder = c->device_.CreateCommandEncoder();
    wgpu::ComputePassDescriptor desc{};
    desc.label = "Zone Derive Params";
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&desc);
    c->renderer_.dispatch_zone_derive_params(
        pass,
        c->gpuState_.zone_gol_compute_group(),
        gs.pending_derive_requests.count);
    pass.End();
    wgpu::CommandBuffer cmd = encoder.Finish();
    queue.Submit(1, &cmd);

    gs.pending_derive_requests.count = 0;
}

// ═══ DISPATCH FUNNELS (table-shaped; declared in entity_types.hpp) ═

inline bool dispatch_select_gol(Cartridge* self,
    int32_t gx, int32_t gz, EntityQueueEntry& e) {
    if (!self->gol_state_.mood_allowed) { return false; }   // mood gate — no new zones
    return select_gol_for_patch(self->gol_state_, self, gx, gz, e.gol);
}

inline bool dispatch_place_gol(Cartridge* self,
    EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (place_gol_from_selection(self, e.gol, pe.gol)) {
        return true;
    }
    else {
        self->gol_state_.zones[e.gol.slot].active = false;
        return false;
    }
}

inline void dispatch_commit_gol(Cartridge* self,
    PlacementEntry& pe, wgpu::Queue& queue) {
    auto* host = find_patch(self, pe.gol.host_gx, pe.gol.host_gz);
    if (host) {
        commit_gol(self->gol_state_, self, pe.gol, pe.gx, pe.gz, queue);
        host->record_entity(PopFamily::GOL, pe.gol.slot);
    }
    else {
        self->gol_state_.zones[pe.gol.slot].active = false;
#ifdef DIAG_ENTITY_LIFECYCLE
        std::cout << "[DIAG:REJECT] gol slot=" << pe.gol.slot
            << " host=(" << pe.gol.host_gx << "," << pe.gol.host_gz
            << ") -- no host patch\n";
#endif
    }
}

// ═══ THE EVICTOR ══════════════════════════════════════════════════

inline void evict_gol(Cartridge* self,
    uint32_t slot, wgpu::Queue& queue) {
    self->gpuState_.deactivate_zone_slot(queue, slot);
    self->gol_state_.zones[slot].active = false;
    self->gol_state_.zone_count--;
#ifdef DIAG_ENTITY_LIFECYCLE
    std::cout << "[DIAG:EVICT]   gol slot=" << slot << "\n";
#endif
}


// ─── Teardown (owner verb; REBUILD-0 m2, stamp D4) ────────────────
inline void teardown_gol(Cartridge* c, wgpu::Queue& queue) {
    // GoL zones
    for (uint32_t i = 0; i < Dim::MAX_GOL_ZONES; i++) {
        c->gol_state_.zones[i] = GoLZoneState{};
    }
    c->gol_state_.zone_count = 0;
    c->gol_state_.active_slot_count = 0;
    c->gol_state_.pending_derive_requests.count = 0;
    GPUGoLZoneArray emptyZones{};
    c->gpuState_.upload_zone_config(queue, emptyZones);
}

// ─── Zone compute passes (owner verbs; REBUILD-0 m2 — stray (6)
// comes home) ─ derive params + sync + evolve + mesh, SEPARATE passes
// for the GPU barrier (O-6a). Callers order them sync -> evolve ->
// mesh after flush_zone_derive_requests + upload_gol_zone_config.
inline void dispatch_zone_sync(GoLState& gs, Cartridge* c, wgpu::CommandEncoder& encoder) {
    wgpu::ComputePassDescriptor cpd{};
    cpd.label = "GoL Zone Sync";
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
    c->renderer_.dispatch_zone_gol_sync(pass,
        c->gpuState_.zone_gol_compute_group(), gs.active_slot_count);
    pass.End();
}

inline void dispatch_zone_evolve(GoLState& gs, Cartridge* c, wgpu::CommandEncoder& encoder) {
    wgpu::ComputePassDescriptor cpd{};
    cpd.label = "GoL Zone Evolve";
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
    c->renderer_.dispatch_zone_gol_evolve(pass,
        c->gpuState_.zone_gol_compute_group(), gs.active_slot_count);
    pass.End();
}

inline void dispatch_zone_mesh(GoLState& gs, Cartridge* c, wgpu::CommandEncoder& encoder) {
    // Mesh gen pass (Group 0 = compute entity, Group 1 = zone mesh gen)
    wgpu::ComputePassDescriptor cpd{};
    cpd.label = "GoL Zone Mesh Gen";
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
    c->renderer_.dispatch_zone_mesh_reset(pass,
        c->gpuState_.zone_mesh_gen_group());
    c->renderer_.dispatch_zone_mesh_gen(pass,
        c->gpuState_.zone_mesh_gen_group(),
        gs.active_slot_count);
    pass.End();
}

} // namespace the_board
} // namespace t7
