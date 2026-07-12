// ─── gol_zones.inl (IMPL: post-class definitions) ────────────────
// Impl of gol_zones.hpp (LADDER-3 c1): history in audit/LADDER.md.
//
// Definitions for gol_zones.hpp's declared lifecycle + per-frame
// functions. The bodies reach c->gpuState_ / c->renderer_ / c->device_ /
// c->tileCache_ / c->mood_state_ / c->world_state_ / c->time_state_ and
// the spine services (check_position / register_footprint /
// record_placement_bookkeeping), plus the in-class statics
// (Cartridge::PATCH_EXTENT / Cartridge::GLOBAL_ENTITY_DENSITY);
// PopFamily is roster.hpp vocabulary.
//
// WRAPPING FORM (the proven fix-2 rule): SELF-WRAPPING — opens
// t7::the_board itself, carries its own standard includes; the MODULE
// IMPLEMENTATIONS zone includes it at FILE SCOPE. Definitions are
// `inline` free functions.
// ─────────────────────────────────────────────────────────────────

#include <cmath>      // std::floor
#include <algorithm>  // std::max, std::min
#include <iostream>   // the spawn log
#include <vector>     // life / height-factor staging

namespace t7 {
namespace the_board {

// ═══ LIFECYCLE — three-phase + helper ════════════════════════════

// ─── select_gol_for_patch ─────────────────────────────────────
//
// Phase 1: lattice-node-gated selection. Scans which MODE_LATTICE
// nodes overlap this patch, runs spawn roll, algorithm + tier
// selection, parameter sampling. At most one zone per patch.

inline bool select_gol_for_patch(GoLState& gs, Cartridge* c,
    int32_t gx, int32_t gz, GoLSelection& sel) {
    // Mood gate
    float adj_mod = GoLZoneSpawnConfig::MOOD_MULTIPLIER[c->mood_state_.active];
    if (adj_mod <= 0.0f) return false;

    // Density + theme modifiers
    adj_mod *= Cartridge::GLOBAL_ENTITY_DENSITY;
    {
        auto dit = c->tileCache_.find({ gx, gz });
        if (dit != c->tileCache_.end()) {
            adj_mod *= dit->second.entity_density;
            adj_mod *= dit->second.theme_spawn[PopFamily::GOL];
        }
    }

    // Scan lattice nodes overlapping this patch
    float wx0 = gx * Cartridge::PATCH_EXTENT;
    float wx1 = (gx + 1) * Cartridge::PATCH_EXTENT;
    float wz0 = gz * Cartridge::PATCH_EXTENT;
    float wz1 = (gz + 1) * Cartridge::PATCH_EXTENT;

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
            int32_t auth_gx = (int32_t)std::floor(raw_cx / Cartridge::PATCH_EXTENT);
            int32_t auth_gz = (int32_t)std::floor(raw_cz / Cartridge::PATCH_EXTENT);
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
//
// Phase 2: footprint check + registration. Position is lattice-
// determined (no jitter), so we bypass negotiate_position and
// call check_position + register_footprint directly.
//
// Note: this function takes no GoLState — like ribbon's place
// function, it only mediates between selection and spawn-engine
// helpers. Doesn't touch GoL's data.

inline bool place_gol_from_selection(Cartridge* c,
    const GoLSelection& sel, GoLPlacement& plan) {
    float cx = sel.corner_x + GoLZoneSpawnConfig::ZONE_EXTENT * 0.5f;
    float cz = sel.corner_z + GoLZoneSpawnConfig::ZONE_EXTENT * 0.5f;

    if (!c->check_position(cx, cz, sel.footprint_r, PopFamily::GOL))
        return false;

    int32_t host_gx = (int32_t)std::floor(cx / Cartridge::PATCH_EXTENT);
    int32_t host_gz = (int32_t)std::floor(cz / Cartridge::PATCH_EXTENT);

    if (c->register_footprint(cx, cz, sel.footprint_r,
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

    c->record_placement_bookkeeping(PopFamily::GOL, plan.tier_idx);
    return true;
}

// ─── commit_gol ──────────────────────────────────────────────
//
// Phase 3: CPU state + life buffer seeding + GPU derive request.

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
//
// Helper called from commit_gol. Generates the initial life pattern
// (random alive/dead for Conway, all zeros for Pulse) and the per-cell
// height factors (Gaussian draws). Uploads both to the GPU zone slot.

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
    // Pulse: all zeros — oscillation builds from silence

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

// upload_gol_zone_config: per-frame header-only upload.
// Per-zone config is GPU-derived via zone_derive_params — we only
// write count, t_beats, dt, tick_mask. Slot deactivation happens
// at eviction time (via dispatch_evict_gol through entity_refs).
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

} // namespace the_board
} // namespace t7
