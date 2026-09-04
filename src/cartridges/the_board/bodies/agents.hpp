#pragma once
#include <cstdint>
#include <array>
#include "cartridges/the_board/realization/state.hpp"                    // Dim::MAX_AGENTS, GPUAgentState, GPU_AGENT_*_COUNT, wgpu
#include "cartridges/the_board/bodies/pawn_figures.hpp"        // PAWN_FIGURES, FIGURE_SHARES, family spans (H1) — this TU names them directly
#include "cartridges/the_board/contracts/agent_tiers.hpp"      // Tier vocabulary graduated to contracts/agent_tiers.hpp (ORGAN_2b) — the bank TIER_LIVE is the world's definition; the translator below reads it.
#include "cartridges/the_board/contracts/agent_surface.hpp"   // AGENTS_LIVE — the agents' bank (ONE_WORLD-II U1c)
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)
#include "cartridges/the_board/contracts/control_panel.hpp"   // ORGAN_4 P3b — PANEL_LIVE.possession.radius: the reach, graduated out of this file's console

// ─── agents.hpp (HEADER: registries + console + state + decls) ───
//
// Unified entity registry: the control panel for the agent system.
//
// The walkers' color palette lives here (AGENT_PALETTE) — it graduated
// from the column family's vocabulary when PRUNE_2 excised that family
// and left the agents its only reader.
// ─────────────────────────────────────────────────────────────────

#include <cmath>      // std::sqrt, std::cos, std::sin   // (impl, merged)
#include <algorithm>  // std::min   // (impl, merged)
#include <iostream>   // census + event logs   // (impl, merged)
#include <iomanip>    // std::fixed, std::setprecision   // (impl, merged)
#include "core/instruments.hpp"   // RIBBON_4 — INSTRUMENTS.stream_witness gates the steady path's witness lines

namespace t7 {
namespace the_board {

// ═══ MODULE DEPS ════════════════════════════════════════════════════
// The agent population's requirements face. player_ is NON-const —
// possession re-anchors (agents door, v3 §9 Act III); the rest is
// read-only. (fwds: spine_state / patch_system types follow in the
// cohort.)
struct PlayerState; struct WorldState; struct TimeState;
class GPUState;
struct AgentsDeps {
    GPUState&              gpuState_;
    PlayerState&           player_;         // non-const: possession door
    const PointState&      point_;          // the point's house (position mirror — respawn ring, possession search)
    const WorldState&      world_state_;
    const TimeState&       time_state_;
};

// ═══ BEHAVIOR IDS ════════════════════════════════════════════════
//
// Stable indices into AGENT_BEHAVIORS. The compute kernel's behavior
// switch dispatches on these values. Names below are also exported as
// AGENT_BEHAVIOR_NAMES[] for diagnostics — keep the two in lockstep.

// AgentBehaviorId graduated to contracts/agent_tiers.hpp with the
// table its rows name (ORGAN_3 w3).

// ═══ TIER IDS ════════════════════════════════════════════════════
// AgentTierId (AGENT_TIER_COUNT included) graduated to
// contracts/agent_tiers.hpp with the table whose rows name it
// (ORGAN_2b). The cross-check below stays: it stands on a
// realization constant, and a contract may not include one.

// Cross-check: GPU-side count constants in state.hpp must match the
// authoritative enums above. If you add a behavior or a tier, bump
// the GPU constant in state.hpp at the same time. The compiler will
// catch any drift here.
static_assert(AGENT_BEHAVIOR_COUNT == GPU_AGENT_BEHAVIOR_COUNT,
    "AGENT_BEHAVIOR_COUNT must match GPU_AGENT_BEHAVIOR_COUNT in state.hpp");
static_assert(AGENT_TIER_COUNT == GPU_AGENT_TIER_COUNT,
    "AGENT_TIER_COUNT must match GPU_AGENT_TIER_COUNT in state.hpp");

// Display names for diagnostics (census output, error messages).
// Order MUST match the enums above — index by the enum value.
// Mirrors the ORB_PAL_NAMES / ORB_TIERSET_NAMES pattern from orbs.hpp.
inline constexpr const char* AGENT_BEHAVIOR_NAMES[AGENT_BEHAVIOR_COUNT] = {
    "player",        //  0  PLAYER_CONTROLLED
    "random_walk",   //  1  RANDOM_WALK
    "biased_walk",   //  2  BIASED_WALK
    "wanderer",      //  3  WANDERER
    "home_seeker",   //  4  HOME_SEEKER
    "slow_patrol",   //  5  SLOW_PATROL
    "pursuit",       //  6  PURSUIT
    "flee",          //  7  FLEE
    "flock2d",       //  8  FLOCK2D
    "levy_flight",   //  9  LEVY_FLIGHT
};

inline constexpr const char* AGENT_TIER_NAMES[AGENT_TIER_COUNT] = {
    "worker",        //  0
    "scout",         //  1
    "sentinel",      //  2
    "leader",        //  3
};

// ═══ TUNING CONSOLE ══════════════════════════════════════════════

inline constexpr uint32_t PLAYER_SLOT = 0;

// POSSESSION_RADIUS graduated to contracts/control_panel.hpp (ORGAN_4
// P3b) — PANEL_LIVE.possession.radius is what this module reads, and
// POSSESSION_RADIUS_SQ is retired outright. It was a second constant
// derived at declaration, so a dialled radius and a frozen square would
// have disagreed silently; the one read site squares the LIVE value now,
// which is why the pair could stop being a pair.

//
// AGENT_EVICTION_RADIUS / _SQ and the VEIL CHAIN assert stood here
// (STAGE_0 U2). The pair was a HARDWARE MIRROR — it had to agree with an
// identically-named WGSL const that no compiler could check, and the
// static_assert bound it to Dim::EXIST_RADIUS to give the unassertable
// half at least one fence. All three are gone with the eviction they
// described: WHAT SPAWNS, STAYS, so there is no radius left to mirror and
// no chain left to sit on.

// AGENT_CENSUS_INTERVAL — wall-clock period (seconds). The periodic
// agent census died (BATCH C); the surviving consumer is the ROSTER
// gol-residue proof cadence (phase_census_dumps). The on-demand agent
// census remains at "boot" and "rebirth".
inline constexpr float AGENT_CENSUS_INTERVAL = 30.0f;

// ═══ REGISTRY: BEHAVIORS ═════════════════════════════════════════

// AgentBehaviorDef, AGENT_BEHAVIORS and its row-count assert
// graduated to contracts/agent_tiers.hpp (ORGAN_3 w3), where
// BEHAVIOR_LIVE stands beside them as the live surface. They ride
// with the tier bank because they ride the same author: the
// translator below reads both, and one boundary re-speaks both.

// ═══ REGISTRY: TIER GAINS ════════════════════════════════════════
// AgentTierDef, AGENT_TIER_GAINS and its row-count assert graduated
// to contracts/agent_tiers.hpp (ORGAN_2b), where TIER_LIVE stands
// beside them as the world's definition. The table keeps its two
// jobs — seeding the bank and standing under the asserts — and the
// translator below reads the bank, not the table.

// ═══ REGISTRY: POPULATIONS ═══════════════════════════════════════


// ─── Why no constexpr helper builders ───────────────────────────

// AGENT_POPULATIONS stood here — seven rows, one per mood, and the
// AGENTS_TABLE seeding witness that proved the bank was the sunset row
// transcribed. Both left at ONE_WORLD-II U2: the witness had done its
// whole job in U1c, which is what a transcription witness is for.
// AgentPopulationDef went with them; the bank's own type is
// AgentPopulationBank (contracts/agent_surface.hpp).

// ═══ AGENT MODULE STATE ══════════════════════════════════════════

// THE AGENT SPAWN SALT. `0xA6E00000u + mood_id` in both spawners until
// ONE_WORLD-II U1c; the boot mood was 0, so this constant is the value
// every world has actually drawn with and placement is unchanged. One
// home, because two copies of a hash salt is how a respawn stops landing
// where a spawn put things.
inline constexpr uint32_t AGENT_SPAWN_SALT = 0xA6E00000u;

struct AgentState {
    GPUAgentState slots[Dim::MAX_AGENTS]            = {};
    uint32_t      respawn_counters[Dim::MAX_AGENTS] = {};
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════

// Lifecycle
void upload_agent_registries_to_gpu(AgentsDeps* c, wgpu::Queue& queue);
void spawn_population(AgentState& as, AgentsDeps* c,
                               uint32_t seed,
                               float center_x, float center_z,
                               wgpu::Queue& queue);
// `respawn_evicted_agents` was declared here (STAGE_0 U2) — and the
// declaration was WRONG, in a way nothing could catch. It named four
// parameters (as, c, world_seed, queue); the definition took SIX, adding
// box_min and box_max. Those are two different overloads, so this line
// declared a function that was never defined and never called: legal C++,
// invisible to the TU gate (which type-checks and does not link), invisible
// to the shell gate (which never links this path) and invisible to the
// probe (which never reaches it). Anyone who had called the four-argument
// form would have got a link error at the very end of a build. It goes with
// the definition, and the trap goes with it.
// Player commands
void try_possess_nearest(AgentState& as, AgentsDeps* c, wgpu::Queue& queue);
void seed_player_body(AgentState& as, AgentsDeps* c);
void reseed_player_body(AgentState& as, AgentsDeps* c, uint32_t preserved_tier,
                        float preserved_color_r, float preserved_color_g, float preserved_color_b,
                        uint32_t preserved_skin);
// Logging
void dump_agent_census(const AgentState& as, const AgentsDeps* c, const char* trigger);

// ═══ IMPL:
// bodies deref agent_state_(own) + gpu/player/point/world/time
// via AgentsDeps. COHORT: after patch_system (WorldState) + spine/state.
// No machine.

// ── THE WALKERS' PALETTE (graduated, PRUNE_2 U4) ─────────────────
// Ten authored colors, rolled per agent at authoring time. It arrived
// here as COLUMN_PALETTE, in the column family's vocabulary block, and
// outlived that family: when PRUNE_2 excised COLUMN and ANTENNA the
// agents were its only surviving reader, so it moved to the reader and
// took the reader's name. The VALUES are untouched — same ten swatches,
// same order, same modulus — so every agent color is byte-identical to
// the one it rolled before the move.
inline constexpr float AGENT_PALETTE[][3] = {
    { 0.937f, 0.902f, 0.831f },   // 0: sand      #EFE6D4
    { 0.882f, 0.827f, 0.714f },   // 1: bone      #E1D3B6
    { 0.129f, 0.118f, 0.110f },   // 2: ink       #211E1C
    { 0.353f, 0.333f, 0.298f },   // 3: ink-soft  #5A554C
    { 0.431f, 0.608f, 0.753f },   // 4: sky       #6E9BC0
    { 0.863f, 0.596f, 0.482f },   // 5: coral     #DC987B
    { 0.878f, 0.635f, 0.306f },   // 6: gold      #E0A24E
    { 0.616f, 0.631f, 0.467f },   // 7: olive     #9DA177
    { 0.635f, 0.620f, 0.686f },   // 8: lavender  #A29EAF
    { 0.753f, 0.325f, 0.184f },   // 9: orb       #C0532F
};
inline constexpr uint32_t AGENT_PALETTE_COUNT = 10;

// ═══ REGISTRY UPLOAD (CPU table → GPU buffer, once at world-init) ═
//
// Both registries' truth is now a LIVE BANK — BEHAVIOR_LIVE and
// TIER_LIVE, the world's
// definition bank (contracts/agent_tiers.hpp), seeded at load from
// the authored AGENT_TIER_GAINS — so a panel edit to the bank
// outlives this author, because this author reads it every time it
// speaks. The compute kernels read both from GPU uniform buffers
// (bindings 110 / 111), uploaded at world-init by this helper and
// again at the frame boundary whenever the bank changes (ORGAN_2b).

// upload_agent_registries_to_gpu: takes Cartridge* for gpuState_
// access. No agent state needed — uploads constexpr registries only.
inline void upload_agent_registries_to_gpu(AgentsDeps* c, wgpu::Queue& queue) {
    GPUAgentBehaviorDef gpu_behaviors[AGENT_BEHAVIOR_COUNT] = {};
    for (uint32_t i = 0; i < AGENT_BEHAVIOR_COUNT; i++) {
        const auto& src = BEHAVIOR_LIVE.b[i];   // ORGAN_3 w3 — the world's definition, not the design table
        gpu_behaviors[i].step_rate       = src.step_rate;
        gpu_behaviors[i].step_size       = src.step_size;
        gpu_behaviors[i].persistence     = src.persistence;
        gpu_behaviors[i].drag            = src.drag;
        gpu_behaviors[i].home_pull       = src.home_pull;
        gpu_behaviors[i].neighbor_radius = src.neighbor_radius;
        gpu_behaviors[i].speed_cap       = src.speed_cap;
        gpu_behaviors[i].aux             = src.aux;   // ATRIUM_4 — the column travels
    }

    GPUAgentTierDef gpu_tiers[AGENT_TIER_COUNT] = {};
    for (uint32_t i = 0; i < AGENT_TIER_COUNT; i++) {
        const auto& src = TIER_LIVE.t[i];   // ORGAN_2b — the world's definition, not the design table
        gpu_tiers[i].step_gain     = src.step_gain;
        gpu_tiers[i].persist_gain  = src.persist_gain;
        gpu_tiers[i].speed_gain    = src.speed_gain;
        gpu_tiers[i].color_r       = src.color_r;
        gpu_tiers[i].color_g       = src.color_g;
        gpu_tiers[i].color_b       = src.color_b;
        gpu_tiers[i].contact_radius = src.contact_radius;   // TRUEBAND_CONTACT_1
        gpu_tiers[i].contact_mass   = src.contact_mass;
        gpu_tiers[i].personal_radius  = src.personal_radius;  // CONTACT_2
        gpu_tiers[i].flee_gain_player = src.flee_gain_player;
        gpu_tiers[i]._pad0 = 0.0f;
        gpu_tiers[i]._pad1 = 0.0f;
    }

    c->gpuState_.upload_agent_registries(queue,
        gpu_behaviors, AGENT_BEHAVIOR_COUNT,
        gpu_tiers,     AGENT_TIER_COUNT);

    c->gpuState_.upload_pawn_figures(queue);   // one-shot; packs PAWN_FIGURES -> GPU (H2)
}

// ═══ SHARED POPULATION HELPER ════════════════════════════════════

inline void populate_agent_slot_(const AgentState& as,
                          GPUAgentState& out,
                          const AgentPopulationBank& pop,
                          uint32_t agent_seed,
                          float beh_sum, float tier_sum,
                          float center_x, float center_z,
                          float box_min, float box_max) {
    // ── Roll behavior ─────────────────────────────────────────────
    float w_beh[AGENT_BEHAVIOR_COUNT];
    for (uint32_t b = 0; b < AGENT_BEHAVIOR_COUNT; b++)
        w_beh[b] = pop.behavior_weights[b] / beh_sum;
    uint32_t behavior_id = select_tier(agent_seed, 1u, w_beh, AGENT_BEHAVIOR_COUNT);

    // ── Roll tier ─────────────────────────────────────────────────
    float w_tier[AGENT_TIER_COUNT];
    for (uint32_t t = 0; t < AGENT_TIER_COUNT; t++)
        w_tier[t] = pop.tier_weights[t] / tier_sum;
    uint32_t tier_idx = select_tier(agent_seed, 2u, w_tier, AGENT_TIER_COUNT);

    // ── Sample annulus position (uniform area distribution) ───────
    // ATRIUM_9 — THE ANNULUS RIDES THE ARRIVAL GAZE. The centre passed in
    // is where the VISITOR is; spawn_center_forward walks it along the
    // gaze before the ring is drawn, so a room can put its figures in
    // front of the viewer instead of all around them. The direction is
    // Idle::PAWN_HEADING's — the ARRIVAL gaze, a constant, not the live
    // heading: the composition is the room's, and it must not swing when
    // the visitor turns. Every caller passes 0 here, and at
    // 0 both cosines fall out and the centre is the caller's, unmoved.
    const float two_pi = 6.28318530718f;
    const float gaze = heading_to_bearing(Idle::PAWN_HEADING);
    const float cx = center_x + std::cos(gaze) * pop.spawn_center_forward;
    const float cz = center_z + std::sin(gaze) * pop.spawn_center_forward;
    // ── THE BOX (HEM_1) ───────────────────────────────────────────
    // The annulus is 200-340 wu from the point and the world is WALLED:
    // its half-width is 75 at radius 1. Drawn unchanged, every figure in
    // this population is born outside the wall, and HEM_1's clamp spends
    // its first frame stacking thirty-one of them against it.
    //
    // SCALE, THEN CLAMP. Scaling alone leaves a point near a corner
    // drawing outside; clamping alone maps the whole annulus onto the
    // wall. Rejection sampling was refused: the salt is frozen and a
    // variable number of draws is not a frozen salt.
    //
    // The margin is THIS TIER's contact_radius — the same number
    // agent_settle's clamp insets the box by, read from the same bank.
    // The two rooms agree by construction, not by coincidence.
    //
    // PLACEMENT MOVES IN A FINITE WORLD, and that is this unit. The
    // bit-for-bit claim ONE_WORLD-II U1c recorded holds only on the
    // infinite arm, where `walled` is false, `fit` is 1.0, and not one
    // line below runs.
    const bool  walled = (box_min != 0.0f || box_max != 0.0f);
    const float margin = TIER_LIVE.t[tier_idx].contact_radius;

    float r_in  = pop.spawn_inner_radius;
    float r_out = pop.spawn_radius;
    if (walled) {
        const float usable = std::max(0.0f,
            0.5f * (box_max - box_min) - margin);   // the box's inradius
        const float fit = (r_out > 0.0f)
            ? std::min(1.0f, usable / r_out) : 1.0f;
        r_in  *= fit;
        r_out *= fit;
    }

    float theta = cpu_hash_f(agent_seed, 3u) * two_pi;
    float u = cpu_hash_f(agent_seed, 4u);
    float r = std::sqrt(r_in * r_in + u * (r_out * r_out - r_in * r_in));
    float sx = cx + std::cos(theta) * r;
    float sz = cz + std::sin(theta) * r;

    // ── Sample home offset (uniform on disk around spawn point) ───
    float h_theta = cpu_hash_f(agent_seed, 5u) * two_pi;
    float h_r     = std::sqrt(cpu_hash_f(agent_seed, 6u)) * pop.home_seeding_radius;
    float hx = sx + std::cos(h_theta) * h_r;
    float hz = sz + std::sin(h_theta) * h_r;

    // THE HOME IS CLAMPED TOO, and it is not decoration: home_x/home_z
    // are HOME_SEEKER's spring anchor and WANDERER's tether. A home
    // outside the wall is a constant force holding an agent against it
    // for the life of the world.
    if (walled) {
        const float lo = box_min + margin;
        const float hi = box_max - margin;
        sx = std::clamp(sx, lo, hi);  sz = std::clamp(sz, lo, hi);
        hx = std::clamp(hx, lo, hi);  hz = std::clamp(hz, lo, hi);
    }

    // ── Write the slot ────────────────────────────────────────────
    out.pos_x   = sx;   out.pos_y   = 0.0f; out.pos_z   = sz;
    out.home_x  = hx;   out.home_z  = hz;
    out.heading = 0.0f;
    out.vel_x   = 0.0f; out.vel_y   = 0.0f; out.vel_z   = 0.0f;
    out.orient_x = 0.0f; out.orient_y = 0.0f; out.orient_z = 0.0f; out.orient_w = 1.0f;
    out.seed           = agent_seed;
    out.behavior_id    = behavior_id;
    out.tier_idx       = tier_idx;
    out.is_active      = 1u;

    uint32_t ci = cpu_hash(agent_seed, 7u) % AGENT_PALETTE_COUNT;
    out.color_r = AGENT_PALETTE[ci][0];
    out.color_g = AGENT_PALETTE[ci][1];
    out.color_b = AGENT_PALETTE[ci][2];
    // ── Roll figure (skin_id) — global distribution, deterministic from seed ──
    // Family weighted by FIGURE_SHARES (salt 8u); member uniform within family
    // (salt 9u). Independent of the behavior/tier rolls (distinct salts).
    //
    // No branch — the entrance's figures roll like everyone's (ATRIUM_8).
    {
        float fw[FAM_COUNT];
        float fsum = 0.0f;
        for (uint32_t i = 0; i < FAM_COUNT; ++i) fsum += FIGURE_SHARES[i].share_pct;
        for (uint32_t i = 0; i < FAM_COUNT; ++i) fw[i] = FIGURE_SHARES[i].share_pct / fsum;

        uint32_t fam_i = select_tier(agent_seed, 8u, fw, FAM_COUNT);
        PawnFamilyId fam = FIGURE_SHARES[fam_i].family;   // FIGURE_SHARES is ordered REGULAR,SMOOTH,HERALDIC

        uint32_t base = figure_family_base(fam);
        uint32_t n    = figure_family_member_count(fam);
        if (n <= 1u) {
            out.skin_id = base;                            // single-member family (regular)
        } else {
            float mw[16];                                  // max 7 members; 16 is slack
            for (uint32_t i = 0; i < n; ++i) mw[i] = 1.0f / static_cast<float>(n);
            uint32_t m = select_tier(agent_seed, 9u, mw, n);
            out.skin_id = base + m;
        }
    }
}

// ═══ SPAWN ════════════════════════════════════════════════════════

inline void spawn_population(AgentState& as, AgentsDeps* c,
                               uint32_t seed,
                               float center_x, float center_z,
                               float box_min, float box_max,
                               wgpu::Queue& queue) {
    const auto& pop = AGENTS_LIVE;

    // Zero every non-player slot before refilling. The player's body
    // (slot PLAYER_SLOT) is preserved across a rebirth.
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
            // THE SALT IS FROZEN, NOT DROPPED (ONE_WORLD-II U1c). It read
            // `0xA6E00000u + mood_id`, so the mood was arithmetic in the
            // seed and not merely a table index: removing the term outright
            // would move every agent's tier, behaviour, position and home.
            // The boot mood was 0, so the frozen salt IS the salt this
            // world has always drawn with, and placement is bit-for-bit.
            uint32_t agent_seed = cpu_hash(cpu_hash(seed, AGENT_SPAWN_SALT), i + 1u);

            populate_agent_slot_(as, as.slots[slot], pop, agent_seed,
                                 beh_sum, tier_sum,
                                 center_x, center_z, box_min, box_max);
            spawned++;
        }
    }

    c->gpuState_.upload_agent_state_all(queue, as.slots);
    std::cout << "[Agents] Spawned " << spawned
              << " around (" << center_x << "," << center_z << ")\n";
}

// ═══ RESPAWN (per-frame, evicted slots → fresh agents) ════════════
//
// Writes only the changed slots — never the player slot — so the
// GPU's per-frame player update never sees a stale CPU snapshot.
// (respawn_counters lives in the CPU MIRROR section of agents.hpp.)

// THE DEFINITION STOOD HERE (STAGE_0 U2). It walked the non-player slots,
// found the ones the GPU had deactivated, drew a fresh agent for each from
// `cpu_hash(cpu_hash(world_seed, AGENT_SPAWN_SALT), slot·0x10001 + count·0x100)`
// and uploaded it. It NEVER TESTED A DISTANCE — the eviction was entirely
// the GPU's, and this was only the refill. With eviction retired there is
// nothing left to refill: the population is resident, and a world's agents
// are the agents it was born with.
//
// `AgentState::respawn_counters` survives for now — it is the salt that
// made a respawned agent differ from its predecessor, and cutting it is a
// struct change on a mirrored type. Flagged for the sweep, not taken here.

// ═══ POSSESSION TRANSFER (Caps Lock) ══════════════════════════════

inline void try_possess_nearest(AgentState& as, AgentsDeps* c, wgpu::Queue& queue) {
    const uint32_t cur = c->player_.possessed_slot;
    // THE POINT: possession reaches from the point — the
    // nearest agent to where you ARE. Pawn-host value-identical (same
    // harvest snapshot as the slot mirror); in free-fly Caps Lock
    // grabs a body wherever you flew (xz-plane reach, per the spawn
    // ruling — the population lives there now, so there is one).
    const float px = c->point_.x;
    const float pz = c->point_.z;

    int best_slot = -1;
    // ORGAN_4 P3b — THE SQUARE IS DERIVED HERE, from the live reach. The
    // retired POSSESSION_RADIUS_SQ was a constexpr twin, and a dialled
    // radius against a frozen square is the disagreement its DEFER row
    // named. One authority, squared where it is used.
    const float reach = PANEL_LIVE.possession.radius;
    float best_d2 = reach * reach;
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
        std::cout << "[Possess] No agent within " << reach
                  << " units of the point at (" << px << "," << pz << ")\n";
        return;
    }

    const uint32_t new_slot = (uint32_t)best_slot;

    as.slots[cur].behavior_id = AGENT_BEHAVIOR_RANDOM_WALK;
    if (as.slots[cur].seed == 0u) {
        as.slots[cur].seed = cpu_hash(c->world_state_.active_seed, cur ^ 0xC11Cu);
    }

    // New slot → player control. Reset velocity so the player's first
    // frame on the new body is clean.
    as.slots[new_slot].behavior_id    = AGENT_BEHAVIOR_PLAYER_CONTROLLED;
    as.slots[new_slot].vel_x          = 0.0f;
    as.slots[new_slot].vel_z          = 0.0f;

    c->gpuState_.upload_agent_slot(queue, cur, &as.slots[cur]);
    c->gpuState_.upload_agent_slot(queue, new_slot, &as.slots[new_slot]);

    c->player_.possessed_slot = new_slot;
    c->gpuState_.set_possessed_slot(new_slot);

    std::cout << "[Possess] " << cur << " -> " << new_slot
              << " (tier " << as.slots[new_slot].tier_idx
              << ", dist " << std::sqrt(best_d2) << ")\n";
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

    std::cout << "\n";
}


// ─── Player-body seeding (owner verbs) ─
// boot twin: slot 0 at the Idle pose, WORKER tier (the
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
    as.slots[0].skin_id = 0u;   // player is always the regular pawn
}

// Rebirth twin: keep the CPU mirror in sync with the GPU reset so
// patch streaming + ribbon + Caps Lock see current state; possession
// re-anchors to slot 0 (the possessed_slot write stays with the
// declared possession door's owner). Tier + colors + figure (skin_id)
// preserved by the caller across the reset — the possessed body's
// appearance set. The twins stay twins — byte-exactness
// outranks unification (PRIME INVARIANT); merging them is later
// material if ever pulled.
inline void reseed_player_body(AgentState& as, AgentsDeps* c, uint32_t preserved_tier,
                               float preserved_color_r, float preserved_color_g, float preserved_color_b,
                               uint32_t preserved_skin) {
    std::memset(as.slots, 0, sizeof(as.slots));
    as.slots[0].pos_x   = Idle::PAWN_POS_X;
    as.slots[0].pos_y   = Idle::PAWN_POS_Y;
    as.slots[0].pos_z   = Idle::PAWN_POS_Z;
    as.slots[0].heading = Idle::PAWN_HEADING;
    as.slots[0].orient_w = 1.0f;
    as.slots[0].is_active = 1u;
    as.slots[0].behavior_id = AGENT_BEHAVIOR_PLAYER_CONTROLLED;
    as.slots[0].tier_idx = preserved_tier;
    as.slots[0].color_r = preserved_color_r;
    as.slots[0].color_g = preserved_color_g;
    as.slots[0].color_b = preserved_color_b;
    as.slots[0].skin_id = preserved_skin;   // the figure rides with tier + color
    c->player_.possessed_slot = 0;
}

} // namespace the_board
} // namespace t7
