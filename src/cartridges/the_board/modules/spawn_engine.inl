// ─── spawn_engine.inl ────────────────────────────────────────────
// Payload relocations (LADDER-3): history in audit/LADDER.md.
//
// How and when things appear: shared spawn helpers, footprint registry,
// proximity affinity, mesh gen prep, distance culling, census, plus the
// dispatch loops that drive both generic and bespoke families through
// select → place → commit.
//
// SEAM[spawn_engine:P11] home of pattern P11 (templated active-array
//   helper) — run_spawn_preamble<ActiveT> is the canonical instance.
//   One implementation, ten callers. Same family as P10's per-family
//   vocabulary block at the algorithm level.
// SEAM[spawn_engine:structural] RETIRED. The EntityQueueEntry /
//   PlacementEntry unions and every type they embed live together in
//   entity_types.hpp (the contract home) — the union-member
//   completeness constraint is satisfied inside one header, and this
//   file holds only the queues and loops. spawn_engine stays ONE
//   file, never split into pre/post files.
// SEAM[spawn_engine:L1] latent diagnostic — DIAG_ENTITY_LIFECYCLE is
//   compile-time guarded (#define commented out below). Same family
//   as the [DIAG:*] stdout pattern noted across the codebase.
//   Document alongside any other diagnostic switches when the
//   exhibition-guard discussion happens.
// ─────────────────────────────────────────────────────────────────

// ═══ SHARED SPAWN HELPERS ════════════════════════════════════════

// ── Helper 1: SpawnGatePreamble ──────────────────────────────

struct SpawnGatePreambleResult {
    uint32_t seed;          // from evaluate_spawn_gate
    uint32_t slot;          // reserved slot index
    uint32_t theme_idx;     // active_theme_idx_ at evaluation time
    bool ok;                // false = early exit (idempotency, gate, no slot)
};

template<typename ActiveT>
SpawnGatePreambleResult run_spawn_preamble(
    int32_t gx, int32_t gz,
    ActiveT* active_arr, uint32_t max_instances,
    uint32_t spawn_roll_prop, float spawn_chance,
    const float* mood_mult,
    uint32_t family, const char* diag_name)
{
    SpawnGatePreambleResult r{};
    r.ok = false;

    // 1. Idempotency
    for (uint32_t i = 0; i < max_instances; i++) {
        if (active_arr[i].active &&
            active_arr[i].patch_gx == gx &&
            active_arr[i].patch_gz == gz) {
            return r;
        }
    }

    // 2-6. Spawn modifier chain
    float adj_mod = mood_mult[mood_state_.active];
    adj_mod *= GLOBAL_ENTITY_DENSITY;
    r.theme_idx = active_theme_idx_;
    {
        auto dit = tileCache_.find({ gx, gz });
        if (dit != tileCache_.end()) {
            adj_mod *= dit->second.entity_density;
            adj_mod *= dit->second.theme_spawn[family];
        }
    }

    // 6b. Proximity affinity boost (nearby entities attract)
    {
        float pcx = (gx + 0.5f) * PATCH_EXTENT;
        float pcz = (gz + 0.5f) * PATCH_EXTENT;
        adj_mod *= proximity_affinity_boost(pcx, pcz, family);
    }

    // 7. Spawn gate
    auto ctx = evaluate_spawn_gate(gx, gz, spawn_roll_prop,
        spawn_chance, adj_mod);
    if (!ctx.passed) return r;

    // 8-9. Find and reserve slot
    uint32_t slot = UINT32_MAX;
    for (uint32_t i = 0; i < max_instances; i++) {
        if (!active_arr[i].active) { slot = i; break; }
    }
    if (slot == UINT32_MAX) return r;
    active_arr[slot].active = true;

#ifdef DIAG_ENTITY_LIFECYCLE
    std::cout << "[DIAG:SEL] " << diag_name << " slot=" << slot
        << " patch=(" << gx << "," << gz << ")\n";
#endif

    r.seed = ctx.seed;
    r.slot = slot;
    r.ok = true;
    return r;
}

// ── Helper 2: NegotiatePosition ─────────────────────────────

struct PositionResult {
    float cx, cz, rotation;
    int32_t host_gx, host_gz;
    bool ok;
};

PositionResult negotiate_position(
    uint32_t seed, int32_t trigger_gx, int32_t trigger_gz,
    uint32_t pos_x_prop, uint32_t pos_z_prop, float jitter,
    uint32_t rotation_seed_prop,
    float footprint_r, uint32_t family, uint32_t tier = 0)
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
    if (world_state_.finite_mode && MOOD_TABLE[mood_state_.active].indoor) {
        float bmin = -(float)world_state_.finite_radius * PATCH_EXTENT;
        float bmax = ((float)world_state_.finite_radius + 1.0f) * PATCH_EXTENT;
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
    if (!check_position(r.cx, r.cz, footprint_r, family))
        return r;

    // 3. Host patch + footprint registration
    r.host_gx = (int32_t)std::floor(r.cx / PATCH_EXTENT);
    r.host_gz = (int32_t)std::floor(r.cz / PATCH_EXTENT);
    if (register_footprint(r.cx, r.cz, footprint_r,
        r.host_gx, r.host_gz, family, tier) == UINT32_MAX) return r;

    r.ok = true;
    return r;
}

// ── Helper 3: record_placement_bookkeeping ──────────────────

void record_placement_bookkeeping(uint32_t /*family*/, uint32_t /*tier_idx*/)
{
}

// ═══ MESH GEN PREPARERS + CULLING ════════════════════════════════

// ─── Column / Arch / Pyramid mesh-gen preparers ───────────────

// ─── Pier Write Helper ───────────────────────────────────────────

void write_pier(wgpu::Queue& queue, uint32_t slot, const GPUPierInstance& pier) {
    cpuPiers_[slot] = pier;
    gpuState_.upload_pier_slot(queue, slot, pier);
    world_state_.pier_count_dirty = true;
    world_state_.ground_entries_dirty = true;
}

void clear_pier(wgpu::Queue& queue, uint32_t slot) {
    GPUPierInstance empty{};
    cpuPiers_[slot] = empty;
    gpuState_.upload_pier_slot(queue, slot, empty);
    world_state_.pier_count_dirty = true;
    world_state_.ground_entries_dirty = true;
}

void recompute_and_upload_pier_count(wgpu::Queue& queue) {
    uint32_t highest = 0;
    for (uint32_t i = 0; i < Dim::PIER_TOTAL; i++) {
        if (cpuPiers_[i].is_active) highest = i + 1;
    }
    gpuState_.config().pier_count = highest;
    gpuState_.upload_pier_count(queue);
}

void flush_pier_count(wgpu::Queue& queue) {
    if (!world_state_.pier_count_dirty) return;
    world_state_.pier_count_dirty = false;
    recompute_and_upload_pier_count(queue);
}

// ─── Entity Distance Culling ─────────────────────────────────────
//
// Size-awareness is re-signed: a taller entity culls slightly EARLIER (its
// base stays safely inside the edge), never later — the old outward lead is
// gone. The inset is small and capped. Hysteresis prevents oscillation.

static constexpr float ENTITY_CULL_EDGE_MARGIN    = 0.5f * Dim::PATCH_EXTENT;  // 25 wu inside the visible edge
static constexpr float ENTITY_CULL_SIZE_INSET     = 0.5f;    // wu of inward inset per unit of entity size
static constexpr float ENTITY_CULL_SIZE_INSET_MAX = 60.0f;   // cap: never cull nearer than base − this
static constexpr float ENTITY_CULL_HYSTERESIS     = 40.0f;   // band: hide at the (inset) edge, show 40 wu inside

// Rebuild GPUArchMeshParams from cached ActiveArch data.
GPUArchMeshParams build_arch_mesh_params(uint32_t slot) const {
    const auto& a = entities_state_.arches[slot];
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
static GPUColumnMeshParams build_column_mesh_params_from(const ActiveColumn& c) {
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

GPUColumnMeshParams build_column_mesh_params(uint32_t slot) const {
    return build_column_mesh_params_from(entities_state_.columns[slot]);
}

// Scan all active entities, toggle draw_visible with hysteresis,
// and upload mesh param changes. Returns count of currently hidden entities.
uint32_t update_entity_draw_visibility(wgpu::Queue& queue) {
    uint32_t culled = 0;

    const float cull_base = VISIBILITY_CYLINDER_RADIUS - ENTITY_CULL_EDGE_MARGIN;  // 275 − 25 = 250

    // Arches
    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
        if (!entities_state_.arches[i].active) continue;
        const auto& a = entities_state_.arches[i];
        float dx = a.world_x - player_.readback_x;
        float dz = a.world_z - player_.readback_z;
        float dist = std::sqrt(dx * dx + dz * dz);

        float entity_size = std::max(a.half_span * 2.0f, a.total_height);
        float inset = std::min(entity_size * ENTITY_CULL_SIZE_INSET, ENTITY_CULL_SIZE_INSET_MAX);
        float cull_far  = cull_base - inset;                 // taller ⇒ earlier, never past the edge
        float cull_near = cull_far - ENTITY_CULL_HYSTERESIS; // show only when this far inside

        bool should_show = a.draw_visible
            ? (dist <= cull_far)          // visible: hide at the (inset) edge
            : (dist <= cull_near);        // hidden:  show when comfortably inside

        if (should_show != a.draw_visible) {
            entities_state_.arches[i].draw_visible = should_show;
            if (should_show) {
                gpuState_.upload_arch_mesh_params_slot(queue, i, build_arch_mesh_params(i));
            }
            else {
                GPUArchMeshParams empty{};
                gpuState_.upload_arch_mesh_params_slot(queue, i, empty);
            }
            entities_state_.arch_mesh_gen_pending = true;
        }

        if (!entities_state_.arches[i].draw_visible) culled++;
    }

    // Columns
    for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++) {
        if (!entities_state_.columns[i].active) continue;
        const auto& c = entities_state_.columns[i];
        float dx = c.world_x - player_.readback_x;
        float dz = c.world_z - player_.readback_z;
        float dist = std::sqrt(dx * dx + dz * dz);

        float inset = std::min(c.height * ENTITY_CULL_SIZE_INSET, ENTITY_CULL_SIZE_INSET_MAX);
        float cull_far  = cull_base - inset;
        float cull_near = cull_far - ENTITY_CULL_HYSTERESIS;

        bool should_show = c.draw_visible
            ? (dist <= cull_far)
            : (dist <= cull_near);

        if (should_show != c.draw_visible) {
            entities_state_.columns[i].draw_visible = should_show;
            if (should_show) {
                gpuState_.upload_column_mesh_params_slot(queue, i, build_column_mesh_params(i));
            }
            else {
                GPUColumnMeshParams empty{};
                gpuState_.upload_column_mesh_params_slot(queue, i, empty);
            }
            entities_state_.column_mesh_gen_pending = true;
        }

        if (!entities_state_.columns[i].draw_visible) culled++;
    }

    // Antennas
    for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++) {
        if (!entities_state_.antennas[i].active) continue;
        const auto& c = entities_state_.antennas[i];
        float dx = c.world_x - player_.readback_x;
        float dz = c.world_z - player_.readback_z;
        float dist = std::sqrt(dx * dx + dz * dz);
        uint32_t gpu_slot = i + Dim::ANTENNA_SLOT_OFFSET;

        float inset = std::min(c.height * ENTITY_CULL_SIZE_INSET, ENTITY_CULL_SIZE_INSET_MAX);
        float cull_far  = cull_base - inset;
        float cull_near = cull_far - ENTITY_CULL_HYSTERESIS;

        bool should_show = c.draw_visible
            ? (dist <= cull_far)
            : (dist <= cull_near);

        if (should_show != c.draw_visible) {
            entities_state_.antennas[i].draw_visible = should_show;
            if (should_show) {
                gpuState_.upload_column_mesh_params_slot(queue, gpu_slot, build_column_mesh_params_from(c));
            }
            else {
                GPUColumnMeshParams empty{};
                gpuState_.upload_column_mesh_params_slot(queue, gpu_slot, empty);
            }
            entities_state_.column_mesh_gen_pending = true;
        }

        if (!entities_state_.antennas[i].draw_visible) culled++;
    }

    return culled;
}

// ═══ FOOTPRINT REGISTRY ══════════════════════════════════════════

struct GroundFootprint {
    float x = 0.0f, z = 0.0f;
    float radius = 0.0f;
    int32_t patch_gx = 0, patch_gz = 0;
    uint32_t family = UINT32_MAX;  // PopFamily index
    uint32_t tier = 0;             // tier index within family
    float spawn_time = 0.0f;       // time_state_.seconds at registration
    bool active = false;
};

static constexpr uint32_t MAX_FOOTPRINTS = 128;
GroundFootprint footprints_[MAX_FOOTPRINTS]{};

bool check_position(float px, float pz, float placing_radius,
    uint32_t placing_family) const {
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (!footprints_[i].active) continue;
        float dx = px - footprints_[i].x;
        float dz = pz - footprints_[i].z;
        float effective_min = placing_radius + footprints_[i].radius;
        if (footprints_[i].family < PopFamily::COUNT) {
            float min_gap = MIN_SEPARATION[placing_family][footprints_[i].family];
            if (min_gap > 0.0f) {
                float aff = PROXIMITY_AFFINITY[placing_family][footprints_[i].family];
                if (aff > 0.0f) min_gap *= (1.0f - aff * PROXIMITY_GAP_REDUCTION[placing_family]);
                effective_min += min_gap;
            }
        }
        if (dx * dx + dz * dz < effective_min * effective_min) return false;
    }
    return true;
}

uint32_t register_footprint(float x, float z, float radius,
    int32_t gx, int32_t gz, uint32_t family = UINT32_MAX,
    uint32_t tier = 0) {
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (!footprints_[i].active) {
            footprints_[i] = { x, z, radius, gx, gz, family, tier, time_state_.seconds, true };
            return i;
        }
    }
    return UINT32_MAX;  // full — entity should not spawn
}

void unregister_footprints_for_patch(int32_t gx, int32_t gz) {
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (footprints_[i].active &&
            footprints_[i].patch_gx == gx && footprints_[i].patch_gz == gz) {
            footprints_[i].active = false;
        }
    }
}

// ═══ ENTITY CENSUS ═══════════════════════════════════════════════

float lastCensusDump_ = -999.0f;
static constexpr float CENSUS_DUMP_INTERVAL = 30.0f;

static const char* family_short_name(uint32_t family) {
    static const char* NAMES[] = { "pyr", "arch", "col", "ant", "palm", "cact", "blad", "sph", "ribn", "cube", "gol", "gall" };
    return (family < PopFamily::COUNT) ? NAMES[family] : "???";
}

static const char* theme_short_name(uint32_t theme) {
    static const char* NAMES[] = { "transition", "monumental", "colonnade", "antenna", "barren" };
    return (theme < THEME_COUNT) ? NAMES[theme] : "???";
}

void dump_entity_census(const char* trigger) const {
    uint32_t count = 0;
    uint32_t by_family[PopFamily::COUNT] = {};
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (!footprints_[i].active) continue;
        if (footprints_[i].family >= PopFamily::COUNT) continue;
        count++;
        by_family[footprints_[i].family]++;
    }

    std::cout << "[CENSUS t=" << std::fixed << std::setprecision(1) << time_state_.seconds
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
        if (!footprints_[i].active || footprints_[i].family >= PopFamily::COUNT) continue;
        entries[n++] = { i, footprints_[i].family, footprints_[i].tier, footprints_[i].spawn_time };
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
        const auto& fp = footprints_[entries[i].fp_idx];
        std::cout << "  " << family_short_name(fp.family)
            << " t" << fp.tier
            << " (" << std::setw(8) << std::setprecision(1) << fp.x
            << "," << std::setw(8) << fp.z << ")"
            << " p(" << std::setw(3) << fp.patch_gx << "," << std::setw(3) << fp.patch_gz << ")"
            << " age=" << std::setprecision(1) << (time_state_.seconds - fp.spawn_time)
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

struct SpawnPreamble {
    uint32_t seed;          // tile_seed(world_state_.active_seed, gx, gz)
    uint32_t archetype;     // 0=mountainous, 1=varied, 2=basin, 3=pool
    bool passed;            // false if spawn gate failed
};

// Evaluate the spawn gate: seed + flat probability check.
// adjacency_mod is a multiplier from the full spawn cascade.
SpawnPreamble evaluate_spawn_gate(int32_t gx, int32_t gz,
    uint32_t spawn_roll_prop,
    float spawn_chance,
    float adjacency_mod = 1.0f) const {
    SpawnPreamble result{};
    result.archetype = 1;
    auto tile_it = tileCache_.find({ gx, gz });
    if (tile_it != tileCache_.end()) result.archetype = tile_it->second.archetype;

    result.seed = tile_seed(world_state_.active_seed, gx, gz);
    float chance = std::min(spawn_chance * adjacency_mod, 1.0f);
    result.passed = cpu_hash_f(result.seed, spawn_roll_prop) < chance;
    return result;
}

// Jittered world position within a patch.
static void jittered_position(uint32_t seed, int32_t gx, int32_t gz,
    uint32_t prop_x, uint32_t prop_z, float jitter,
    float& out_x, float& out_z) {
    out_x = (gx + 0.5f) * PATCH_EXTENT + (cpu_hash_f(seed, prop_x) - 0.5f) * PATCH_EXTENT * jitter;
    out_z = (gz + 0.5f) * PATCH_EXTENT + (cpu_hash_f(seed, prop_z) - 0.5f) * PATCH_EXTENT * jitter;
}

// ─── Spawn Configuration Summary ────────────────────────────────
//
// * Ribbon CHANCE 0.900 is a TESTING bump for ribbon-dev visibility;
//   ship value 0.400 (ribbon.inl SPAWN_CHANCE, reverted at ship).

// ─── Global Entity Density ──────────────────────────────────────
static constexpr float GLOBAL_ENTITY_DENSITY = 1.0f;

// ─── Property Index Registry ────────────────────────────────────

// ── Minimum Separation Matrix ─────────────────────────────────────
//
// Read as: row = entity being placed, column = existing entity.
// The check is asymmetric: placing an arch near a pyramid may have a
// different minimum than placing a pyramid near an arch.

static constexpr float MIN_SEPARATION[PopFamily::COUNT][PopFamily::COUNT] = {
    //                near:  Pyr    Arch   Col    Ant    Palm   Cact   Blad   Sph    Ribn   Cube   GoL    Gall
    /* placing Pyramid  */ { 65.0f, 60.0f,  5.0f, 55.0f,  5.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
    /* placing Arch     */ { 60.0f, 20.0f, 10.0f, 60.0f,  8.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
    /* placing Column   */ {  5.0f, 10.0f,  8.0f,  6.0f,  5.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
    /* placing Antenna  */ { 55.0f, 60.0f,  6.0f, 12.0f,  5.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
    /* placing Palm     */ {  5.0f,  8.0f,  5.0f,  5.0f,  8.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
    /* placing Cactus   */ {  5.0f,  5.0f,  5.0f,  5.0f,  5.0f,  8.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
    /* placing Blade    */ {  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f },
    /* placing Sphere   */ {  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 20.0f,  0.0f,  0.0f,  0.0f,  0.0f },
    /* placing Ribbon   */ {  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 40.0f,  0.0f,  0.0f,  0.0f },
    /* placing Cube     */ {  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 15.0f,  0.0f,  0.0f },
    /* placing GoL      */ { 10.0f, 10.0f,  5.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 60.0f,  0.0f },
    /* placing Gallery  */ { 10.0f, 10.0f,  5.0f,  5.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, 10.0f, 30.0f },
};

// ═══ PROXIMITY AFFINITY ══════════════════════════════════════════

//                              Pyr    Arch   Col    Ant    Palm   Cact   Blad   Sph    Ribn   Cube   GoL    Gall
static constexpr float    PROXIMITY_RADIUS[PopFamily::COUNT] = { 0.0f,  0.0f, 60.0f,  0.0f,150.0f,120.0f,120.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f };
static constexpr float    PROXIMITY_MAX_BOOST[PopFamily::COUNT] = { 1.0f,  1.0f,  2.0f,  1.0f,  3.0f,  3.0f,  3.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f };
static constexpr uint32_t PROXIMITY_THRESHOLD[PopFamily::COUNT] = { 0,     0,     2,     0,     1,     1,     1,     0,     0,     0,     0,     0 };
static constexpr float    PROXIMITY_GAP_REDUCTION[PopFamily::COUNT] = { 0.0f, 0.0f, 0.3f, 0.0f, 0.6f, 0.6f, 0.6f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

static constexpr float PROXIMITY_AFFINITY[PopFamily::COUNT][PopFamily::COUNT] = {
    //           near: Pyr   Arch  Col   Ant   Palm  Cact  Blad  Sph   Ribn  Cube  GoL   Gall
    /* Pyr   */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Arch  */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Col   */ { 0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Ant   */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Palm  */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.65f, 0.3f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Cact  */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.5f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Blad  */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.3f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Sph   */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Ribn  */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Cube  */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* GoL   */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    /* Gall  */ { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
};

// Precomputed: does this family have any non-zero affinity?
static constexpr bool proximity_row_active(uint32_t family) {
    for (uint32_t f = 0; f < PopFamily::COUNT; f++)
        if (PROXIMITY_AFFINITY[family][f] > 0.0f) return true;
    return false;
}

float proximity_affinity_boost(float cx, float cz, uint32_t family) const {
    if (!proximity_row_active(family)) return 1.0f;
    float radius = PROXIMITY_RADIUS[family];
    if (radius <= 0.0f) return 1.0f;
    float r2 = radius * radius;
    float weighted = 0.0f;
    uint32_t count = 0;
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (!footprints_[i].active) continue;
        if (footprints_[i].family >= PopFamily::COUNT) continue;
        float aff = PROXIMITY_AFFINITY[family][footprints_[i].family];
        if (aff <= 0.0f) continue;
        float dx = cx - footprints_[i].x;
        float dz = cz - footprints_[i].z;
        if (dx * dx + dz * dz < r2) {
            weighted += aff;
            count++;
        }
    }
    if (count < PROXIMITY_THRESHOLD[family]) return 1.0f;
    return std::min(1.0f + weighted, PROXIMITY_MAX_BOOST[family]);
}

void mark_patches_for_regen(float min_wx, float min_wz,
    float max_wx, float max_wz,
    int32_t home_gx, int32_t home_gz) {
    int32_t pg_x0 = (int32_t)std::floor(min_wx / PATCH_EXTENT);
    int32_t pg_x1 = (int32_t)std::floor(max_wx / PATCH_EXTENT);
    int32_t pg_z0 = (int32_t)std::floor(min_wz / PATCH_EXTENT);
    int32_t pg_z1 = (int32_t)std::floor(max_wz / PATCH_EXTENT);

    for (uint32_t p = 0; p < world_state_.active_patch_count; p++) {
        if (patches_[p].phase != PatchPhase::GENERATED) continue;
        if (patches_[p].grid_x == home_gx && patches_[p].grid_z == home_gz) continue;
        if (patches_[p].grid_x >= pg_x0 && patches_[p].grid_x <= pg_x1 &&
            patches_[p].grid_z >= pg_z0 && patches_[p].grid_z <= pg_z1) {
            patches_[p].phase = PatchPhase::NEEDS_REGEN;
        }
    }
}

// Precompute catenary parameter 'a' from (half_span, rise).
// 50-iteration bisection, passed to GPU in ArchMeshParams.
static float solve_catenary_a(float half_span, float target_h) {
    float a_lo = 0.1f, a_hi = std::max(half_span * 10.0f, 5.0f);
    float a = half_span;
    for (int iter = 0; iter < 50; iter++) {
        a = 0.5f * (a_lo + a_hi);
        float val = a * (std::cosh(half_span / a) - 1.0f);
        if (val > target_h) a_lo = a; else a_hi = a;
    }
    return a;
}

// ═══ BESPOKE-FAMILY SELECTION/PLACEMENT PAYLOADS ═════════════════
//
// Three bespoke families (GoL, Gallery, Ribbon) don't fit the
// generic pipeline's EntityInstance shape — their selection
// records carry family-specific fields (lattice node, painting
// count, wave parameters). The payload DTOs AND the tagged unions
// that carry them (EntityQueueEntry / PlacementEntry) live together
// in entity_types.hpp — the contract home; a DTO that exists to
// cross a boundary belongs to the boundary's contract. See
// SEAM[spawn_engine:structural] in the file header.

// ═══ ENTITY DISPATCH PIPELINE ════════════════════════════════════

// ─── The queues (machine state) ──────────────────────────────────
//
// EntityQueueEntry / PlacementEntry are contract vocabulary
// (entity_types.hpp); the QUEUES they fill are spine state and live
// here. entityQueue_ decouples WHAT exists from WHERE it goes;
// placementResults_ holds entities past spatial negotiation, ready
// for GPU commit.

std::vector<EntityQueueEntry> entityQueue_;
std::vector<PlacementEntry> placementResults_;

// ─── Select / Place / Commit dispatch loops ─────────────────────

void select_entities_for_patch(int32_t gx, int32_t gz) {
    for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
        if (!ROSTER.family_enabled(f)) continue;  // ROSTER-GATE family (b) — disabled family never selected -> never placed/committed/meshed/drawn. Budgeted stream path, not the per-frame hot path.
        EntityQueueEntry e{};
        e.family = f;
        e.gx = gx; e.gz = gz;
        if (FAMILY_DISPATCH[f].try_select(this, gx, gz, e))
            entityQueue_.push_back(e);
    }
}

// ─── Place: spatial negotiation (no GPU writes) ──────────────

void place_entity_queue() {
    for (auto& e : entityQueue_) {
        PlacementEntry pe{};
        if (FAMILY_DISPATCH[e.family].try_place(this, e, pe))
            placementResults_.push_back(pe);
    }
    entityQueue_.clear();
}

// ─── Commit: GPU writes from placement results ──────────────

void commit_entity_queue(wgpu::Queue& queue) {
    for (auto& pe : placementResults_)
        FAMILY_DISPATCH[pe.family].try_commit(this, pe, queue);
    placementResults_.clear();
}

//
float estimate_terrain_height(float wx, float wz) const {
    int32_t tx = (int32_t)std::floor(wx / PATCH_EXTENT);
    int32_t tz = (int32_t)std::floor(wz / PATCH_EXTENT);
    auto it = tileCache_.find({ tx, tz });
    if (it != tileCache_.end())
        return it->second.height_bias + it->second.amp_scale * 5.0f;
    return 0.0f;
}

bool terrain_tile_warm(float wx, float wz) const {
    int32_t tx = (int32_t)std::floor(wx / PATCH_EXTENT);
    int32_t tz = (int32_t)std::floor(wz / PATCH_EXTENT);
    return tileCache_.find({ tx, tz }) != tileCache_.end();
}
