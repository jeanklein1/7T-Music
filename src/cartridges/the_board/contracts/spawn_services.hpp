#pragma once
#include <cstdint>
#include <cstddef>                                            // size_t (the rescale template's array extent)
#include "cartridges/the_board/contracts/roster.hpp"          // PopFamily (sizes MIN_SEPARATION)
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)
#include "cartridges/the_board/contracts/entity_types.hpp"    // MachineCtx + EntityInstance + traits/adapter + TierProfile

// ─── spawn_services.hpp (CONTRACT: the machine's decl tier) ───────
// The machine natives (spawn_engine, entity_pipeline) each held a
// two-tier shape — a DECL tier consumed BEFORE entities (the preamble
// + generic + rescale templates, the service decls
// vocabulary, MIN_SEPARATION) and a BODY tier that needs entities
// COMPLETE. The decl tier graduates HERE so the merged machine
// headers ride the cohort tail whole. The bodies bind by the TU's own
// law: an inline function or template declared before its callers may
// be DEFINED later in the same TU (templates instantiate at
// end-of-TU) — named at the contract.
//
// OWNERS: every DEFINITION lives in machine/spawn_engine.hpp or
// machine/entity_pipeline.hpp (merged, cohort tail). This file owns
// only the boundary — entity_types.hpp's sibling.
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// fwd — state.hpp's GPU mesh-param records (return values below; a
// non-defining declaration tolerates the incomplete type).

// ── Shared spawn helper vocabulary ─────────────────────────────────

struct SpawnGatePreambleResult {
    uint32_t seed;          // from evaluate_spawn_gate
    uint32_t slot;          // reserved slot index
    bool ok;                // false = early exit (idempotency, gate, no slot)
};

struct PositionResult {
    float cx, cz, rotation;
    int32_t host_gx, host_gz;
    bool ok;
};

// ── Spawn gate vocabulary ──────────────────────────────────────────

struct SpawnPreamble {
    uint32_t seed;          // tile_seed(world_state_.active_seed, gx, gz)
    bool passed;            // false if spawn gate failed
};

// ─── Global Entity Density ──────────────────────────────────────
// (Graduated with the decl tier: gol's bespoke spawn funnel reads it
// before the machine tail.)
// GLOBAL_ENTITY_DENSITY moved to surface/population_themes.hpp — the
// population panel, where the dials it is turned with will live
// (ONE_WORLD-II U3).

// ── Minimum Separation Matrix ─────────────────────────────────────
//
// WHAT: the extra edge-to-edge gap a candidate placement must keep from
//   every registered footprint, per ordered family pair.
// AXES: MIN_SEPARATION[placing][existing] — row = family being PLACED,
//   column = the EXISTING footprint's family. ASYMMETRIC by design: the
//   gap one family keeps from another need not be the gap the other
//   keeps from it. (The pair that taught this — antenna vs arch, 60 one
//   way and 8 the other — left with PRUNE_2 and ONE_WORLD-I; the
//   asymmetry stays a property of the table, not of that pair.)
// UNITS: world-units, ADDITIVE — the consumer sums the two footprint
//   radii first, then adds this gap on top (and may shrink it by
//   the pair's gap; the clustering families that could shrink it left
//   at PRUNE_2 and the mechanism with them at ONE_WORLD-I U5).
// ORDER: rows and columns both follow PopFamily order (PYRAMID=0 …
//   GOL=5), PINNED by the F-1 static_assert at roster.hpp —
//   renumbering a family is a compile error, not a silent re-column.
// CONSUMER: check_position(), machine/spawn_engine.hpp (sole reader).
// SENTINEL: 0.0 = no gap constraint for that pair (only the radii sum
//   applies; the consumer skips the gap term entirely).
// Placement determinant — frozen biography (§12): changing a number
// changes which candidate positions survive, i.e. changes worlds.
//
// NON-PARTICIPANTS — SPHERE (2) and CUBE (4), ruling 21/23. Both are
//   unreachable in BOTH directions and no number in either line can change
//   anything:
//     · their ROWS never execute — negotiate_position skips check_position
//       entirely for a non-`grounded` family, so those families never read
//       the table at all;
//     · their COLUMNS never match — they register no footprint, so no
//       sphere or cube entry will ever be found by the scan.
//   The rows and columns REMAIN because F-1 pins this table in PopFamily
//   order and it is one of ELEVEN positional tables — deleting a line for a
//   family that still EXISTS would re-column all of them behind F-1's back.
//   They are held as structural zeros.
//   (PRUNE_1 U6 shrank the arity from 12: GALLERY was the TAIL family, so
//   its row and column were truncated off the end and no survivor moved.
//   PRUNE_2 then cut MID-TABLE lines — but only for families it EXCISED
//   whole, re-columning all eleven tables and FAMILY_DISPATCH in the same
//   commit and rewriting F-1 to the surviving pins. That is the licensed
//   form. What this note still forbids is dropping a LIVE family's line.)
//   Their diagonals were 20 (sph) and 15 (cube); ruling 22 retired them with
//   the footprint. If floaters ever claim ground again, the values are in git
//   and this note is what tells you they were deliberate.
// THE GOL ROW AND COLUMN LEFT AT ONE_SURFACE-II U2, and they carried the
// table's only two live off-diagonal values: `placing GoL near Pyr = 10`
// (a zone kept 10 wu off a pyramid's footprint) and `placing GoL near
// GoL = 60` (zones did not overlap each other). Both were about ISLANDS
// CLAIMING GROUND. The automaton claims none — it is the ground — so
// there is nothing to separate it from and nothing it can crowd.
//
// What is left is three self-separations and a table of zeros, which is
// an honest picture of a world whose only ground-claiming families are
// the pyramid and the ribbon.
inline constexpr float MIN_SEPARATION[PopFamily::COUNT][PopFamily::COUNT] = {
    //                near:  Pyr    Sph    Ribn   Cube
    /* placing Pyramid  */ { 65.0f,  0.0f,  0.0f,  0.0f },
    /* placing Sphere   */ {  0.0f,  0.0f,  0.0f,  0.0f },   // ruling 22: self-sep retired with the footprint (was 20)
    /* placing Ribbon   */ {  0.0f,  0.0f, 40.0f,  0.0f },
    /* placing Cube     */ {  0.0f,  0.0f,  0.0f,  0.0f },   // ruling 22: self-sep retired with the footprint (was 15)
};

// ═══ SPAWN SERVICES — DECLARATIONS (spawn_engine) ═════════════════
//
// DEFINED in machine/spawn_engine.hpp (merged, cohort tail): the
// engine reaches the machine face for the root organs (world/time/
// world/tile state, entities_state_, the GPU wire) and routes
// the six families through FAMILY_DISPATCH.

// ═══ THE COMPOSITION LAW — the collapse ═════════════════════
// ONE stack, authored once, called by both spawn authors (the
// generic preamble, GoL). Per-consumer FACTS travel as DATA:
// base-chance authority (scalar, or archetype-indexed — resolved by
// the caller before the call), clamp policy, and
// The float multiplication ORDER inside the definition is the
// bit-identity contract, and ONE_WORLD-II U3 shortened it to
// GLOBAL_ENTITY_DENSITY → base × adj → clamp: the mood term (identity at
// the kept row) and the tile term (authored off the theme lattice) both
// left, and the mood-zero veto style left with them. Seed domains
// and the rolls themselves stay with the consumers.

enum class SpawnClamp : uint32_t {
    MIN1,     // min(chance, 1.0)        — the generic preamble
    RANGE01,  // max(0, min(1, chance))  — GoL
};

// ONE RETURN CHANNEL (ONE_WORLD-II U3). SpawnChanceResult carried a second
// field, `vetoed`, true only under veto_on_zero_mood with a mood
// multiplier of 0 — GoL's arm, and the only caller that took it. The mood
// multipliers left with the moods, nothing can be 0 in the stack any more,
// and a struct for one float is a struct for nothing.
float compose_spawn_chance(MachineCtx* c, int32_t gx, int32_t gz,
    uint32_t family, float base_chance, SpawnClamp clamp);

SpawnPreamble evaluate_spawn_gate(MachineCtx* c, int32_t gx, int32_t gz,
    uint32_t spawn_roll_prop,
    float chance);
void jittered_position(uint32_t seed, int32_t gx, int32_t gz,
    uint32_t prop_x, uint32_t prop_z, float jitter,
    float& out_x, float& out_z);
bool check_position(MachineCtx* c, float px, float pz, float placing_radius,
    uint32_t placing_family);
uint32_t register_footprint(MachineCtx* c, float x, float z, float radius,
    int32_t gx, int32_t gz, uint32_t family, uint32_t slot,
    uint32_t tier = 0);
// Release by owner identity (family, slot). No index is stored anywhere — the
// registry is scanned. See the definition for why that is the design.
void unregister_footprint_for(MachineCtx* c, uint32_t family, uint32_t slot);
// `grounded` (ruling 21) decides whether footprint_r means anything: a family
// registers iff its own extent touches the ground plane. FALSE skips BOTH the
// check and the registration — the floater is neither blocked by ground nor a
// claimant of it. It does NOT skip the containment clamp (containment is a
// different concept) nor the host-patch derivation (eviction bookkeeping,
// which every family needs). Deliberately NOT defaulted: with two call sites,
// an explicit value at each beats a default that would silently register a
// future floater family whose author forgot the flag.
PositionResult negotiate_position(MachineCtx* c,
    uint32_t seed, int32_t trigger_gx, int32_t trigger_gz,
    uint32_t pos_x_prop, uint32_t pos_z_prop, float jitter,
    uint32_t rotation_seed_prop,
    bool grounded,
    float footprint_r, uint32_t family, uint32_t slot,
    uint32_t tier = 0);
uint32_t update_entity_draw_visibility(MachineCtx* c, wgpu::Queue& queue);
const char* family_short_name(uint32_t family);
void dump_entity_census(MachineCtx* c, const char* trigger);
void select_entities_for_patch(MachineCtx* c, int32_t gx, int32_t gz);
void place_entity_queue(MachineCtx* c);
void commit_entity_queue(MachineCtx* c, wgpu::Queue& queue);

// The preamble template (SEAM[spawn_engine:P11]) — DECLARATION only;
// the definition rides machine/spawn_engine.hpp (end-of-TU
// instantiation binds every pre-tail caller).
template<typename C, typename ActiveT>
SpawnGatePreambleResult run_spawn_preamble(C* c,
    int32_t gx, int32_t gz,
    ActiveT* active_arr, uint32_t max_instances,
    uint32_t spawn_roll_prop, float spawn_chance,
    uint32_t family);

// The generic gate — DECLARATION only; defined beside run_spawn_preamble at
// the cohort tail. Same binding law: the family run_gates that call it
// (grounded.hpp, spheres.hpp, cube_behaviors.hpp all precede spawn_engine.hpp
// in the cohort) bind by end-of-TU instantiation.
//
// Every per-family constant travels on the traits row; only the ACTIVE ARRAY
// is a parameter, because its type varies and ActiveT deduces from it.
template<typename ActiveT>
SpawnGateOutput gate_from_traits(MachineCtx* c, int32_t gx, int32_t gz,
    const EntityFamilyTraits& t, ActiveT* active_arr);

// ═══ PIPELINE VERBS — DECLARATIONS (entity_pipeline) ══════════════
//
// DEFINED in machine/entity_pipeline.hpp (merged, cohort tail): the
// verbs reach the machine face for c->sky_state_ / c->world_state_
// and route through the spawn services; the family adapters write
// c->entities_state_ and the GPU wire.

bool generic_select(MachineCtx* c,
    const EntityFamilyTraits& traits,
    const EntityFamilyAdapter& adapter,
    int32_t gx, int32_t gz,
    EntityInstance& inst);
bool generic_place(MachineCtx* c,
    const EntityFamilyTraits& traits,
    EntityInstance& inst);
void generic_commit(MachineCtx* c,
    const EntityFamilyTraits& traits,
    const EntityFamilyAdapter& adapter,
    const EntityInstance& inst,
    wgpu::Queue& queue);

} // namespace the_board
} // namespace t7
