// ─── agents.inl (IMPL: post-class definitions) ───────────────────
// Impl of agents.hpp (LADDER-3 c2): history in audit/LADDER.md.
//
// Definitions for agents.hpp's declared laws, plus the module-internal
// helpers (populate_agent_slot_, apply_agent_overrides_). The bodies
// reach c->gpuState_ / c->player_ / c->world_state_ / c->time_state_ /
// c->transitionPhase_ (the phase enum, contracts/spine_state.hpp)
// and read COLUMN_PALETTE from entities.hpp.
//
// WRAPPING FORM (fix-2): SELF-WRAPPING — the zone includes impls at
// FILE SCOPE; law in audit/LADDER.md.
// ─────────────────────────────────────────────────────────────────

#include <cmath>      // std::sqrt, std::cos, std::sin
#include <algorithm>  // std::min
#include <iostream>   // census + event logs
#include <iomanip>    // std::fixed, std::setprecision

namespace t7 {
namespace the_board {

// ═══ REGISTRY UPLOAD (CPU table → GPU buffer, once at world-init) ═
//
// AGENT_BEHAVIORS and AGENT_TIER_GAINS are the single source of
// truth for behavior/tier parameters. The compute kernels read them
// from GPU uniform buffers (bindings 110 / 111), uploaded once at
// world-init by this helper. Values are constexpr-equivalent —
// they never change during a session, so a one-shot upload at boot
// is sufficient.

// upload_agent_registries_to_gpu: takes Cartridge* for gpuState_
// access. No agent state needed — uploads constexpr registries only.
inline void upload_agent_registries_to_gpu(AgentsDeps* c, wgpu::Queue& queue) {
    GPUAgentBehaviorDef gpu_behaviors[AGENT_BEHAVIOR_COUNT] = {};
    for (uint32_t i = 0; i < AGENT_BEHAVIOR_COUNT; i++) {
        const auto& src = AGENT_BEHAVIORS[i];
        gpu_behaviors[i].step_rate       = src.step_rate;
        gpu_behaviors[i].step_size       = src.step_size;
        gpu_behaviors[i].persistence     = src.persistence;
        gpu_behaviors[i].drag            = src.drag;
        gpu_behaviors[i].home_pull       = src.home_pull;
        gpu_behaviors[i].neighbor_radius = src.neighbor_radius;
        gpu_behaviors[i].speed_cap       = src.speed_cap;
        gpu_behaviors[i]._pad            = 0.0f;
    }

    GPUAgentTierDef gpu_tiers[AGENT_TIER_COUNT] = {};
    for (uint32_t i = 0; i < AGENT_TIER_COUNT; i++) {
        const auto& src = AGENT_TIER_GAINS[i];
        gpu_tiers[i].step_gain     = src.step_gain;
        gpu_tiers[i].persist_gain  = src.persist_gain;
        gpu_tiers[i].speed_gain    = src.speed_gain;
        gpu_tiers[i].color_r       = src.color_r;
        gpu_tiers[i].color_g       = src.color_g;
        gpu_tiers[i].color_b       = src.color_b;
        gpu_tiers[i]._pad[0] = 0.0f;
        gpu_tiers[i]._pad[1] = 0.0f;
    }

    c->gpuState_.upload_agent_registries(queue,
        gpu_behaviors, AGENT_BEHAVIOR_COUNT,
        gpu_tiers,     AGENT_TIER_COUNT);
}

// ═══ SHARED POPULATION HELPER ════════════════════════════════════

inline void populate_agent_slot_(const AgentState& as,
                          GPUAgentState& out,
                          const AgentPopulationDef& pop,
                          uint32_t agent_seed,
                          float beh_sum, float tier_sum,
                          float center_x, float center_z) {
    // ── Roll behavior (or honor override) ─────────────────────────
    uint32_t behavior_id = AGENT_BEHAVIOR_RANDOM_WALK;
    if (as.behavior_override != AGENT_OVERRIDE_NONE) {
        behavior_id = as.behavior_override;
    } else {
        float w[AGENT_BEHAVIOR_COUNT];
        for (uint32_t b = 0; b < AGENT_BEHAVIOR_COUNT; b++)
            w[b] = pop.behavior_weights[b] / beh_sum;
        behavior_id = select_tier(agent_seed, 1u, w, AGENT_BEHAVIOR_COUNT);
    }

    // ── Roll tier (or honor override) ─────────────────────────────
    uint32_t tier_idx = AGENT_TIER_WORKER;
    if (as.tier_override != AGENT_OVERRIDE_NONE) {
        tier_idx = as.tier_override;
    } else {
        float w[AGENT_TIER_COUNT];
        for (uint32_t t = 0; t < AGENT_TIER_COUNT; t++)
            w[t] = pop.tier_weights[t] / tier_sum;
        tier_idx = select_tier(agent_seed, 2u, w, AGENT_TIER_COUNT);
    }

    // ── Sample annulus position (uniform area distribution) ───────
    const float two_pi = 6.28318530718f;
    float theta = cpu_hash_f(agent_seed, 3u) * two_pi;
    const float inner_sq = pop.spawn_inner_radius * pop.spawn_inner_radius;
    const float outer_sq = pop.spawn_radius       * pop.spawn_radius;
    float u = cpu_hash_f(agent_seed, 4u);
    float r = std::sqrt(inner_sq + u * (outer_sq - inner_sq));
    float sx = center_x + std::cos(theta) * r;
    float sz = center_z + std::sin(theta) * r;

    // ── Sample home offset (uniform on disk around spawn point) ───
    float h_theta = cpu_hash_f(agent_seed, 5u) * two_pi;
    float h_r     = std::sqrt(cpu_hash_f(agent_seed, 6u)) * pop.home_seeding_radius;
    float hx = sx + std::cos(h_theta) * h_r;
    float hz = sz + std::sin(h_theta) * h_r;

    // ── Write the slot ────────────────────────────────────────────
    out.pos_x   = sx;   out.pos_y   = 0.0f; out.pos_z   = sz;
    out.home_x  = hx;   out.home_y  = 0.0f; out.home_z  = hz;
    out.heading = 0.0f;
    out.vel_x   = 0.0f; out.vel_y   = 0.0f; out.vel_z   = 0.0f;
    out.orient_x = 0.0f; out.orient_y = 0.0f; out.orient_z = 0.0f; out.orient_w = 1.0f;
    out.seed           = agent_seed;
    out.behavior_id    = behavior_id;
    out.tier_idx       = tier_idx;
    out.is_active      = 1u;
    out.portal_trigger = -1;

    uint32_t ci = cpu_hash(agent_seed, 7u) % COLUMN_PALETTE_COUNT;
    out.color_r = COLUMN_PALETTE[ci][0];
    out.color_g = COLUMN_PALETTE[ci][1];
    out.color_b = COLUMN_PALETTE[ci][2];
    out._pad0   = 0.0f;
}

// ═══ SPAWN ════════════════════════════════════════════════════════

inline void spawn_population_for_mood(AgentState& as, AgentsDeps* c,
                               uint32_t mood_id,
                               uint32_t seed,
                               float center_x, float center_z,
                               wgpu::Queue& queue) {
    if (mood_id >= MOOD_COUNT) return;
    const auto& pop = AGENT_POPULATIONS[mood_id];

    // Zero every non-player slot before refilling. The player's body
    // (slot PLAYER_SLOT) is preserved across mood transitions.
    for (uint32_t s = PLAYER_SLOT + 1; s < Dim::MAX_AGENTS; s++) {
        as.slots[s] = GPUAgentState{};
    }

    float beh_sum = 0.0f;
    for (uint32_t b = 0; b < AGENT_BEHAVIOR_COUNT; b++) beh_sum += pop.behavior_weights[b];
    float tier_sum = 0.0f;
    for (uint32_t t = 0; t < AGENT_TIER_COUNT; t++) tier_sum += pop.tier_weights[t];

    uint32_t spawned = 0;
    const uint32_t n = std::min(pop.count, Dim::MAX_AGENTS - 1u);
    if (n > 0 && beh_sum > 0.0f && tier_sum > 0.0f) {
        for (uint32_t i = 0; i < n; i++) {
            // Slot 0 is reserved for PLAYER_SLOT; non-player slots
            // pack densely from slot 1 upward.
            uint32_t slot = i + 1u;
            uint32_t agent_seed = cpu_hash(cpu_hash(seed, 0xA6E00000u + mood_id), i + 1u);

            populate_agent_slot_(as, as.slots[slot], pop, agent_seed,
                                 beh_sum, tier_sum,
                                 center_x, center_z);
            spawned++;
        }
    }

    c->gpuState_.upload_agent_state_all(queue, as.slots);
    std::cout << "[Agents] Spawned " << spawned << " for mood " << mood_id
              << " around (" << center_x << "," << center_z << ")\n";
}

// ═══ RESPAWN (per-frame, evicted slots → fresh agents) ════════════
//
// Writes only the changed slots — never the player slot — so the
// GPU's per-frame player update never sees a stale CPU snapshot.
// (respawn_counters lives in the CPU MIRROR section of agents.hpp.)

inline void respawn_evicted_agents(AgentState& as, AgentsDeps* c,
                            uint32_t mood_id,
                            uint32_t world_seed,
                            wgpu::Queue& queue) {
    if (mood_id >= MOOD_COUNT) return;
    const auto& pop = AGENT_POPULATIONS[mood_id];
    if (pop.count == 0) return;

    float beh_sum = 0.0f;
    for (uint32_t b = 0; b < AGENT_BEHAVIOR_COUNT; b++) beh_sum += pop.behavior_weights[b];
    float tier_sum = 0.0f;
    for (uint32_t t = 0; t < AGENT_TIER_COUNT; t++) tier_sum += pop.tier_weights[t];
    if (beh_sum <= 0.0f || tier_sum <= 0.0f) return;

    const uint32_t possessed = c->player_.possessed_slot;
    const float px = as.slots[possessed].pos_x;
    const float pz = as.slots[possessed].pos_z;

    const uint32_t n = std::min(pop.count, Dim::MAX_AGENTS - 1u);
    uint32_t respawned = 0;

    for (uint32_t i = 0; i < n; i++) {
        // Non-player slots pack densely from slot 1 upward.
        uint32_t slot = i + 1u;
        if (slot == possessed) continue;
        if (as.slots[slot].is_active != 0u) continue;

        as.respawn_counters[slot]++;
        uint32_t agent_seed = cpu_hash(
            cpu_hash(world_seed, 0xA6E00000u + mood_id),
            slot * 0x10001u + as.respawn_counters[slot] * 0x100u);

        populate_agent_slot_(as, as.slots[slot], pop, agent_seed,
                             beh_sum, tier_sum,
                             px, pz);

        c->gpuState_.upload_agent_slot(queue, slot, &as.slots[slot]);
        respawned++;
    }

    if (respawned > 0) {
        std::cout << "[Agents] Respawn " << respawned
                  << " around (" << px << "," << pz << ")\n";
    }
}

// ═══ POSSESSION TRANSFER (Caps Lock) ══════════════════════════════

inline void try_possess_nearest(AgentState& as, AgentsDeps* c, wgpu::Queue& queue) {
    if (c->transitionPhase_ != TransitionPhase::IDLE) {
        std::cout << "[Possess] Blocked (mid-transition)\n";
        return;
    }

    const uint32_t cur = c->player_.possessed_slot;
    const float px = as.slots[cur].pos_x;
    const float pz = as.slots[cur].pos_z;

    int best_slot = -1;
    float best_d2 = POSSESSION_RADIUS_SQ;
    for (uint32_t s = 0; s < Dim::MAX_AGENTS; s++) {
        if (s == cur) continue;
        const auto& a = as.slots[s];
        if (a.is_active == 0u) continue;
        if (a.behavior_id == AGENT_BEHAVIOR_PLAYER_CONTROLLED) continue;

        float dx = a.pos_x - px;
        float dz = a.pos_z - pz;
        float d2 = dx * dx + dz * dz;
        if (d2 < best_d2) {
            best_d2 = d2;
            best_slot = (int)s;
        }
    }

    if (best_slot < 0) {
        std::cout << "[Possess] No agent within " << POSSESSION_RADIUS
                  << " units of player at (" << px << "," << pz << ")\n";
        return;
    }

    const uint32_t new_slot = (uint32_t)best_slot;

    as.slots[cur].behavior_id = AGENT_BEHAVIOR_RANDOM_WALK;
    if (as.slots[cur].seed == 0u) {
        as.slots[cur].seed = cpu_hash(c->world_state_.active_seed, cur ^ 0xC11Cu);
    }

    // New slot → player control. Reset velocity + portal trigger so the
    // player's first frame on the new body is clean.
    as.slots[new_slot].behavior_id    = AGENT_BEHAVIOR_PLAYER_CONTROLLED;
    as.slots[new_slot].vel_x          = 0.0f;
    as.slots[new_slot].vel_z          = 0.0f;
    as.slots[new_slot].portal_trigger = -1;

    c->gpuState_.upload_agent_slot(queue, cur, &as.slots[cur]);
    c->gpuState_.upload_agent_slot(queue, new_slot, &as.slots[new_slot]);

    c->player_.possessed_slot = new_slot;
    c->gpuState_.set_possessed_slot(new_slot);

    std::cout << "[Possess] " << cur << " -> " << new_slot
              << " (tier " << as.slots[new_slot].tier_idx
              << ", dist " << std::sqrt(best_d2) << ")\n";
}

// ═══ DIAGNOSTIC CYCLING ══════════════════════════════════════════

inline void apply_agent_overrides_(AgentState& as, AgentsDeps* c, wgpu::Queue& queue) {
    for (uint32_t s = PLAYER_SLOT + 1; s < Dim::MAX_AGENTS; s++) {
        auto& a = as.slots[s];
        if (a.is_active == 0u) continue;
        bool changed = false;
        if (as.behavior_override != AGENT_OVERRIDE_NONE
            && a.behavior_id != as.behavior_override) {
            a.behavior_id = as.behavior_override;
            changed = true;
        }
        if (as.tier_override != AGENT_OVERRIDE_NONE
            && a.tier_idx != as.tier_override) {
            a.tier_idx = as.tier_override;
            changed = true;
        }
        if (changed) {
            c->gpuState_.upload_agent_slot(queue, s, &as.slots[s]);
        }
    }
}

inline void cycle_agent_behavior_override(AgentState& as, AgentsDeps* c, wgpu::Queue& queue) {
    if (as.behavior_override == AGENT_OVERRIDE_NONE) {
        as.behavior_override = AGENT_BEHAVIOR_RANDOM_WALK;
    } else {
        uint32_t next = as.behavior_override + 1u;
        as.behavior_override = (next >= AGENT_BEHAVIOR_COUNT)
            ? AGENT_OVERRIDE_NONE : next;
    }

    apply_agent_overrides_(as, c, queue);

    if (as.behavior_override == AGENT_OVERRIDE_NONE) {
        std::cout << "[Agents] behavior override: none\n";
    } else {
        std::cout << "[Agents] behavior override: "
                  << AGENT_BEHAVIOR_NAMES[as.behavior_override] << "\n";
    }
}

// Cycle: NONE → WORKER → SCOUT → SENTINEL → LEADER → NONE.
inline void cycle_agent_tier_override(AgentState& as, AgentsDeps* c, wgpu::Queue& queue) {
    if (as.tier_override == AGENT_OVERRIDE_NONE) {
        as.tier_override = AGENT_TIER_WORKER;
    } else {
        uint32_t next = as.tier_override + 1u;
        as.tier_override = (next >= AGENT_TIER_COUNT)
            ? AGENT_OVERRIDE_NONE : next;
    }

    apply_agent_overrides_(as, c, queue);

    if (as.tier_override == AGENT_OVERRIDE_NONE) {
        std::cout << "[Agents] tier override: none\n";
    } else {
        std::cout << "[Agents] tier override: "
                  << AGENT_TIER_NAMES[as.tier_override] << "\n";
    }
}

inline void force_respawn_population(AgentState& as, AgentsDeps* c, wgpu::Queue& queue) {
    uint32_t cleared = 0;
    for (uint32_t s = PLAYER_SLOT + 1; s < Dim::MAX_AGENTS; s++) {
        if (as.slots[s].is_active == 0u) continue;
        as.slots[s].is_active = 0u;
        c->gpuState_.upload_agent_slot(queue, s, &as.slots[s]);
        cleared++;
    }
    std::cout << "[Agents] force-respawn cleared " << cleared
              << " slot(s); refill on next frame\n";
}

// ═══ DIAGNOSTIC: agent census ═════════════════════════════════════

inline void dump_agent_census(const AgentState& as, const AgentsDeps* c, const char* trigger) {
    uint32_t active = 0;
    uint32_t by_behavior[AGENT_BEHAVIOR_COUNT] = {};
    uint32_t by_tier[AGENT_TIER_COUNT] = {};

    for (uint32_t i = 0; i < Dim::MAX_AGENTS; i++) {
        const auto& a = as.slots[i];
        if (a.is_active == 0u) continue;
        active++;
        if (a.behavior_id < AGENT_BEHAVIOR_COUNT) by_behavior[a.behavior_id]++;
        if (a.tier_idx     < AGENT_TIER_COUNT)     by_tier[a.tier_idx]++;
    }

    std::cout << "[AGENTS t=" << std::fixed << std::setprecision(1) << c->time_state_.seconds
              << " trigger=" << trigger << "] " << active << "/" << Dim::MAX_AGENTS
              << " active, possessed=" << c->player_.possessed_slot;

    std::cout << " tier:{";
    bool first = true;
    for (uint32_t t = 0; t < AGENT_TIER_COUNT; t++) {
        if (by_tier[t] == 0) continue;
        if (!first) std::cout << " ";
        std::cout << AGENT_TIER_NAMES[t] << "=" << by_tier[t];
        first = false;
    }
    std::cout << "}";

    std::cout << " drv:{";
    first = true;
    for (uint32_t b = 0; b < AGENT_BEHAVIOR_COUNT; b++) {
        if (by_behavior[b] == 0) continue;
        if (!first) std::cout << " ";
        std::cout << AGENT_BEHAVIOR_NAMES[b] << "=" << by_behavior[b];
        first = false;
    }
    std::cout << "}";

    if (as.behavior_override != AGENT_OVERRIDE_NONE
        || as.tier_override != AGENT_OVERRIDE_NONE) {
        std::cout << " override:{";
        bool first_o = true;
        if (as.behavior_override != AGENT_OVERRIDE_NONE) {
            std::cout << "beh=" << AGENT_BEHAVIOR_NAMES[as.behavior_override];
            first_o = false;
        }
        if (as.tier_override != AGENT_OVERRIDE_NONE) {
            if (!first_o) std::cout << " ";
            std::cout << "tier=" << AGENT_TIER_NAMES[as.tier_override];
        }
        std::cout << "}";
    }

    std::cout << "\n";
}


// ─── Player-body seeding (owner verbs; REBUILD-0 m2 — stray (3)
// comes home) ─ boot twin: slot 0 at the Idle pose, WORKER tier (the
// GPU-side twin is seeded by GPUState::initializeState).
inline void seed_player_body(AgentState& as, AgentsDeps* c) {
    (void)c;
    as.slots[0].pos_x = Idle::PAWN_POS_X;
    as.slots[0].pos_y = Idle::PAWN_POS_Y;
    as.slots[0].pos_z = Idle::PAWN_POS_Z;
    as.slots[0].heading = Idle::PAWN_HEADING;
    as.slots[0].orient_w = 1.0f;
    as.slots[0].is_active = 1u;
    as.slots[0].behavior_id = AGENT_BEHAVIOR_PLAYER_CONTROLLED;
    as.slots[0].tier_idx = AGENT_TIER_WORKER;
    as.slots[0].portal_trigger = -1;
}

// Transition twin: keep the CPU mirror in sync with the GPU reset so
// patch streaming + ribbon + Caps Lock see current state; possession
// re-anchors to slot 0 (the possessed_slot write stays with the
// declared possession door's owner). Tier + colors preserved by the
// caller across the reset. The twins stay twins — byte-exactness
// outranks unification (PRIME INVARIANT); merging them is later
// material if ever pulled.
inline void reseed_player_body(AgentState& as, AgentsDeps* c, uint32_t preserved_tier,
                               float preserved_color_r, float preserved_color_g, float preserved_color_b) {
    std::memset(as.slots, 0, sizeof(as.slots));
    as.slots[0].pos_x = 0.0f;  // Idle::PAWN_POS_X
    as.slots[0].pos_y = 0.0f;
    as.slots[0].pos_z = 0.0f;
    as.slots[0].orient_w = 1.0f;
    as.slots[0].is_active = 1u;
    as.slots[0].behavior_id = AGENT_BEHAVIOR_PLAYER_CONTROLLED;
    as.slots[0].tier_idx = preserved_tier;
    as.slots[0].color_r = preserved_color_r;
    as.slots[0].color_g = preserved_color_g;
    as.slots[0].color_b = preserved_color_b;
    as.slots[0].portal_trigger = -1;
    c->player_.possessed_slot = 0;
}

} // namespace the_board
} // namespace t7
