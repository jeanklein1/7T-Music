#pragma once
#include <cstdint>
#include <iostream>       // census + the indoor-skip line
#include <cmath>      // std::floor, std::sqrt, std::min/max companions   // (impl, merged)
#include <algorithm>  // std::min, std::max   // (impl, merged)
#include <iomanip>    // census column formatting   // (impl, merged)
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)

// ─── spawn_engine.hpp (S3 · MERGED: vocabulary + state + impl) ─────
//
// How and when things appear: shared spawn helpers, footprint
// registry, mesh-param rebuilds, distance
// culling, census, plus the dispatch loops that drive both generic
// and bespoke families through select → place → commit.
//
// SEAM[spawn_engine:P11] home of pattern P11 (templated active-array
//   helper) — run_spawn_preamble<C, ActiveT> is the canonical
//   instance. One implementation, four callers.
// The EntityQueueEntry / PlacementEntry unions and every type they
//   embed live in entity_types.hpp (the contract home); this module
//   holds only the queues and loops. spawn_engine stays ONE pair.
//
// Depends on cohort include order: roster.hpp (PopFamily),
// entity_types.hpp (queue unions), state.hpp (GPU mesh params),
// grounded.hpp (EntitiesState — COMPLETE, the merged
// bodies deref them), patch_system.hpp (Dim::PATCH_EXTENT — the preamble
// template reads it at definition), renderer.hpp. MERGED at the
// cohort tail (the B ruling): the decl tier
// lives in contracts/spawn_services.hpp; every pre-tail caller binds
// by same-TU late definition (templates at end-of-TU).

namespace t7 {
namespace the_board {

// ─── Entity Distance Culling — THE RING (re-ruled) ─────────────────
//
// THE RING is the draw authority (chain, state.hpp Dim; live value =
// config veil_ring): an entity is IN the draw set iff any part of it
// reaches inside the ring — center-distance MINUS its horizontal extent
// ≤ ring (the "center±extent" metric; replaces the retired per-size
// inset). Hysteresis sits OUTSIDE the ring, in the fully-iced zone:
//   show when (dist − extent) ≤ ring        (entering fragments are at
//                                            icing = 1 → invisible join)
//   hide when (dist − extent) > ring + HYST (fragments fully iced for
//                                            the whole band → invisible exit)
// Both toggle edges are behind the icing — materialize inside the fade.
inline constexpr float ENTITY_CULL_HYSTERESIS     = 40.0f;   // toggle band, wholly beyond the ring

// ── Footprint registry vocabulary ──────────────────────────────────

struct GroundFootprint {
    float x = 0.0f, z = 0.0f;
    float radius = 0.0f;
    int32_t patch_gx = 0, patch_gz = 0;
    uint32_t family = UINT32_MAX;  // PopFamily index
    uint32_t slot = UINT32_MAX;    // slot within that family — (family, slot) IS the owner
    uint32_t tier = 0;             // tier index within family
    float spawn_time = 0.0f;       // time_state_.seconds at registration
    bool active = false;
};

inline constexpr uint32_t MAX_FOOTPRINTS = 128;
inline constexpr float CENSUS_DUMP_INTERVAL = 30.0f;
// Hard ceiling on the census detail listing. The arrivals filter should
// already bound it well under this; the cap is what guarantees the print
// can never cost a frame regardless of what the world does. An instrument
// must be cheap enough not to become the phenomenon.
inline constexpr uint32_t CENSUS_LISTING_MAX = 12;

// THE PROXIMITY SUBSYSTEM stood here — five PopFamily-indexed tables
// (RADIUS / MAX_BOOST / THRESHOLD / GAP_REDUCTION and the
// AFFINITY matrix) and one mechanism: a family with a non-zero
// affinity row was drawn toward standing neighbours, which both
// multiplied its spawn chance and shrank its separation gap.
//
// EVERY CLUSTERING FAMILY IS GONE. COLUMN, PALM, CACTUS and BLADE
// left at PRUNE_2 and the whole matrix has read zero since; the
// mechanism has been standing unexercised, folding to false at
// compile time, waiting for a family that never came. ONE_WORLD-I
// U5 retires it: a mechanism kept for a hypothetical is exactly
// what the sweep is for, and git holds the numbers (L30).

// ═══ MODULE STATE ══════════════════════════════════════════════════

// ═══ BESPOKE-FAMILY SELECTION/PLACEMENT PAYLOADS ═════════════════
//
// Two bespoke families (GoL, Ribbon) don't fit the
// generic pipeline's EntityInstance shape — their selection
// records carry family-specific fields (lattice node, wave
// parameters). The payload DTOs AND the tagged unions
// that carry them (EntityQueueEntry / PlacementEntry) live together
// in entity_types.hpp — the contract home; a DTO that exists to
// cross a boundary belongs to the boundary's contract. See
// SEAM[spawn_engine:structural] in the file header.

// ─── The queues (machine state) ──────────────────────────────────
//
// EntityQueueEntry / PlacementEntry are contract vocabulary
// (entity_types.hpp); the QUEUES they fill are spine state and live
// here. entityQueue_ decouples WHAT exists from WHERE it goes;
// placementResults_ holds entities past spatial negotiation, ready
// for GPU commit.

// Instance (spawn_engine_state_) lives at the composition root.
// THE BOUND, PROVEN — and the proof is now structural rather than budgetary
// (PANORAMA_0 RIDE_1). spawn_selected_patches (surface/patch_system.hpp) is
// the SOLE caller of all three queue verbs, and it now runs them PER PATCH:
//
//   fill    select_entities_for_patch, for ONE patch; it pushes at most one
//           entry per family, so at most PopFamily::COUNT
//   drain   place_entity_queue  — unconditional full-range loop, then an
//           unconditional reset of the count
//   drain   commit_entity_queue — same shape
//
// There is NO partial drain and no early exit in either drain, so nothing can
// carry to the next patch's fill, let alone the next frame's. That is what
// makes the bound a bound. placementResults_ takes at most one push per
// entityQueue_ entry, so it shares the ceiling and cannot exceed it.
//
// THE OLD FACTOR WAS A FICTION, AND IT FIRED AT EVERY BIRTH. The capacity read
// SPAWN_BUDGET_PER_FRAME x PopFamily::COUNT, which described the STEADY-STATE
// caller — the per-frame budgeted spawn, two patches. The fullRegen arm hands
// the same function all 49 patches of the priority window in one call, and the
// budget is not consulted there: 49 x 12 selections into 24 slots. The
// `[SPAWN] entityQueue_ OVERFLOW` line fired at boot and at every rebirth on
// every device, and the families it dropped were the tail of PLACEMENT_ORDER —
// galleries first. No number would have fixed that; only the drain does.
//
// Derived symbolically rather than written as 12: adding a family moves the
// capacity with it, instead of silently outgrowing a literal.
inline constexpr uint32_t SPAWN_QUEUE_MAX = PopFamily::COUNT;

struct SpawnEngineState {
    EntityQueueEntry entityQueue_[SPAWN_QUEUE_MAX]{};
    uint32_t         entityQueueCount_ = 0;
    PlacementEntry   placementResults_[SPAWN_QUEUE_MAX]{};
    uint32_t         placementCount_ = 0;
    GroundFootprint  footprints_[MAX_FOOTPRINTS]{};
    float lastCensusDump_ = -999.0f;
};

// ═══ MODULE FUNCTIONS ══════════════════════════════════════════════
//
// DECLARATIONS live in contracts/spawn_services.hpp (the
// machine's decl tier) with the boundary DTOs
// (SpawnGatePreambleResult / PositionResult / SpawnPreamble), the
// MIN_SEPARATION and GLOBAL_ENTITY_DENSITY (gol
// reads it pre-tail). Definitions are all below.

// ── Helper 1: SpawnGatePreamble ──────────────────────────────

// SEAM[spawn_engine:P11] the canonical templated active-array helper.
// THE TEMPLATE KEYHOLE, retired to a doorway:
// every instantiation now deduces C = MachineCtx (the machine face);
// the DECLARATION lives in contracts/spawn_services.hpp so the
// pre-tail callers bind here at end-of-TU instantiation. The typename
// C stays — one implementation, four callers, the active-array type
// still varies per family (ActiveT).
template<typename C, typename ActiveT>
SpawnGatePreambleResult run_spawn_preamble(C* c,
    int32_t gx, int32_t gz,
    ActiveT* active_arr, uint32_t max_instances,
    uint32_t spawn_roll_prop, float spawn_chance,
    uint32_t family)
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

    // 2-6. THE COMPOSITION LAW: the stack, authored once —
    // global → base × adj → min(·,1). The mood and tile terms left with
    // ONE_WORLD-II U3; the mood term was identity at the kept row.
    const float composed = compose_spawn_chance(c, gx, gz, family,
        spawn_chance, SpawnClamp::MIN1);

    // 7. Spawn gate (seed + roll; the chance arrives composed)
    auto ctx = evaluate_spawn_gate(c, gx, gz, spawn_roll_prop, composed);
    if (!ctx.passed) return r;

    // 8-9. Find and reserve slot
    uint32_t slot = UINT32_MAX;
    for (uint32_t i = 0; i < max_instances; i++) {
        if (!active_arr[i].active) { slot = i; break; }
    }
    if (slot == UINT32_MAX) return r;
    active_arr[slot].active = true;


    r.seed = ctx.seed;
    r.slot = slot;
    r.ok = true;
    return r;
}

// ── The generic gate: one law, one per-family fact ─────────────────
//
// The generic families ran identical bodies here, each restating five constants that
// its own TRAITS row already declares — max_instances, spawn_roll_prop,
// spawn_chance, family_id — around one call. The restating
// was why four of those fields read as DEAD: the row was the right home and
// nobody read it, so the cut was about to remove the home and keep the nine
// duplicates.
//
// The only genuinely per-family fact is the ACTIVE ARRAY. Its type varies, so
// it stays a parameter and the template deduces ActiveT from it; everything
// else travels as data on the traits row.
//
// BIT-IDENTITY: same callee, same arguments, same order. run_spawn_preamble is
// untouched. The SpawnGatePreambleResult -> SpawnGateOutput conversion moves
// from nine copies to one; note the two structs order their fields
// DIFFERENTLY (preamble: seed, slot, ok — output: ok, seed, slot), so this is
// a real field-by-field reorder and must stay written out rather than
// becoming a cast or a copy. Both lost their theme_idx at ONE_WORLD-II U3.
template<typename ActiveT>
inline SpawnGateOutput gate_from_traits(MachineCtx* c, int32_t gx, int32_t gz,
    const EntityFamilyTraits& t, ActiveT* active_arr)
{
    auto gate = run_spawn_preamble(c, gx, gz,
        active_arr, t.max_instances,
        t.spawn_roll_prop, t.spawn_chance,
        t.family_id);
    return { gate.ok, gate.seed, gate.slot };
}


// ═══ MODULE IMPLEMENTATION ════════════════════════════════════════
//
// The engine's verbs: position negotiation, the footprint registry,
// mesh-param rebuilds + distance culling, the census, gate
// evaluation, and the select → place → commit
// dispatch loops. Reaches the machine face for the root organs
// (c->world_state_ / c->time_state_ / c->mood_state_ /
// c->tile_world_state_ / c->entities_state_ /
// c->player_) and the GPU wire (c->gpuState_); the loops route
// through FAMILY_DISPATCH with the machine face as the row argument.


// ── Helper 1b: the indoor bounds law ────────────────────────
//
// One law for every placement site (negotiate_position). In
// finite indoor worlds, push the
// candidate inward so the clamped radius stays at least
// INDOOR_ENTITY_WALL_MARGIN from every wall. We clamp instead of
// rejecting because rejection would silently drop entities
// anchored to corner patches (their seed-determined position
// keeps landing in the wall margin and never recovers). Clamping
// shifts the candidate to the boundary of the legal box, then
// the existing footprint-overlap check handles any pile-ups.
//
// ── Helper 2: NegotiatePosition ─────────────────────────────

inline PositionResult negotiate_position(MachineCtx* c,
    uint32_t seed, int32_t trigger_gx, int32_t trigger_gz,
    uint32_t pos_x_prop, uint32_t pos_z_prop, float jitter,
    uint32_t rotation_seed_prop,
    bool grounded,
    float footprint_r, uint32_t family, uint32_t slot, uint32_t tier)
{
    PositionResult r{};
    r.ok = false;

    // 1. Jittered position
    jittered_position(seed, trigger_gx, trigger_gz,
        pos_x_prop, pos_z_prop, jitter, r.cx, r.cz);
    r.rotation = cpu_hash_f(seed, rotation_seed_prop) * 6.283185f;

    // (The indoor bounds law ran here — INDOOR_TREATMENT's per-family
    //  MARGIN / FULL / FREE policy, clamping a spawn off the walls or
    //  skipping it when the room was too small for the family's whole
    //  extent. It left with the walls at ONE_WORLD-II U4. The FINITE
    //  containment clamp is a different law and lives in the shader,
    //  where it always did — finite_bounds_resolve.)

    // 2. Separation + footprint check — GROUND CLAIM, and only for bodies
    // that touch the ground. A non-grounded family (sphere, cube) is not
    // blocked by a standing footprint: a pyramid beneath a hovering cube is a
    // composition, not a collision. Ruling 21 — the campaign law is "a family
    // registers iff its own extent touches the ground plane", which is what
    // `grounded` already means and says.
    if (grounded && !check_position(c, r.cx, r.cz, footprint_r, family))
        return r;

    // 3. Host patch. NOT part of the ground claim — this is the entity's
    // address for eviction bookkeeping and EVERY family needs it, floaters
    // included: dispatch_commit_sphere_generic (spheres.hpp) and
    // dispatch_commit_cube_generic (cube_behaviors.hpp) both gate their commit
    // on find_patch(host_gx, host_gz). Step 3 used to fuse this with
    // registration as "one key derivation"; the fusion is why the two look
    // like one concept. They are not, and skipping this alongside the
    // footprint would leave every floater addressed to patch (0,0) — silently
    // dropping any that spawned away from the world origin.
    auto hk = tile_key(r.cx, r.cz);
    r.host_gx = hk.x; r.host_gz = hk.z;

    // 4. Footprint registration — the claim itself. Skipped for the same
    // families, in the same direction: they claim no ground. Both steps
    // conditional, or the ruling is half-applied.
    if (grounded && register_footprint(c, r.cx, r.cz, footprint_r,
        r.host_gx, r.host_gz, family, slot, tier) == UINT32_MAX) return r;

    r.ok = true;
    return r;
}

// ═══ MESH GEN PREPARERS + CULLING ════════════════════════════════

// Scan all active entities, toggle draw_visible with hysteresis,
// and upload mesh param changes. Returns count of currently hidden entities.
// THE RING is the correctness gate (re-ruled): draw membership = any part
// of the entity inside the live ring (center − extent ≤ ring). Anchor: the
// point (readback — the same yardstick as the terrain band). Both toggle
// edges sit at/beyond the ring where the icing is already 1 — invisible.
inline uint32_t update_entity_draw_visibility(MachineCtx* c, wgpu::Queue& queue) {
    uint32_t culled = 0;

    const float ring = c->gpuState_.veil_ring();   // live chain value — the draw authority

    // THE ARCH LOOP stood here — the only family whose mesh could be
    // zeroed at range, because it was the only family with a GPU mesh to
    // zero. It left at ONE_WORLD-I U3, and with it the whole reason this
    // sweep touched the GPU. No surviving family carries generated
    // geometry, so nothing is culled here today.
    (void)queue; (void)ring;

    return culled;
}

// ═══ FOOTPRINT REGISTRY ══════════════════════════════════════════

inline bool check_position(MachineCtx* c, float px, float pz, float placing_radius,
    uint32_t placing_family) {
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (!c->spawn_engine_state_.footprints_[i].active) continue;
        float dx = px - c->spawn_engine_state_.footprints_[i].x;
        float dz = pz - c->spawn_engine_state_.footprints_[i].z;
        float effective_min = placing_radius + c->spawn_engine_state_.footprints_[i].radius;
        if (c->spawn_engine_state_.footprints_[i].family < PopFamily::COUNT) {
            float min_gap = MIN_SEPARATION[placing_family][c->spawn_engine_state_.footprints_[i].family];
            if (min_gap > 0.0f) effective_min += min_gap;
        }
        if (dx * dx + dz * dz < effective_min * effective_min) return false;
    }
    return true;
}

inline uint32_t register_footprint(MachineCtx* c, float x, float z, float radius,
    int32_t gx, int32_t gz, uint32_t family,
    uint32_t slot, uint32_t tier) {
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        if (!c->spawn_engine_state_.footprints_[i].active) {
            c->spawn_engine_state_.footprints_[i] = { x, z, radius, gx, gz, family, slot, tier, c->time_state_.seconds, true };
            return i;
        }
    }
    // SATURATION WAS SILENT, and which family lost was decided by PopFamily
    // order — the tail (cube, gol) simply stopped appearing, with no
    // symptom anywhere. Capacity stays 128 (ruling 8: post-SPAWN_2 occupancy
    // peaked at 65% and settles near 45%), so if this ever prints, the leak it
    // names is the thing to fix, not the number.
    std::cerr << "[SPAWN] footprint registry FULL (" << MAX_FOOTPRINTS
              << ") — dropping " << family_short_name(family)
              << " slot " << slot << "; spawn silently denied\n";
    return UINT32_MAX;
}

// Release by OWNER. The (family, slot) pair is the identity; nothing stores the
// registry index anywhere, deliberately.
//
// A stored index would be a second copy of a fact the registry already holds,
// and it would have to be threaded through PositionResult, the placement DTOs
// and every family's active record to reach the evictor. This campaign has now
// found EIGHT defects whose shape was two copies of one fact. A 128-slot scan
// costs nothing at eviction cadence (EVICT_BUDGET_PER_FRAME is 4) and cannot
// drift, because there is nothing to drift from.
inline void unregister_footprint_for(MachineCtx* c, uint32_t family, uint32_t slot) {
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        auto& fp = c->spawn_engine_state_.footprints_[i];
        if (fp.active && fp.family == family && fp.slot == slot) {
            fp.active = false;
            return;   // one footprint per owner
        }
    }
}

// ═══ ENTITY CENSUS ═══════════════════════════════════════════════

inline const char* family_short_name(uint32_t family) {
    static const char* NAMES[] = { "pyr", "sph", "ribn", "cube", "gol" };
    // F-1's ELEVENTH positional table, and until PRUNE_2 the only one with
    // no compile-time tie to PopFamily::COUNT. The runtime bound below reads
    // COUNT, not this array, so the two could disagree in silence: trim
    // COUNT without trimming NAMES and the tail rows go unnameable; trim
    // NAMES without trimming COUNT and NAMES[family] reads OUT OF BOUNDS —
    // and F-2's boot name-check would then compare against garbage, reaching
    // UB BEFORE the abort that exists to catch exactly that. PRUNE_2 trimmed
    // this array five times by hand with nothing checking it; this is the
    // check.
    static_assert(sizeof(NAMES) / sizeof(NAMES[0]) == PopFamily::COUNT,
        "F-1: family_short_name's NAMES[] is PopFamily-positional — re-column "
        "it with the other ten tables before renumbering any family (F-2's "
        "boot name-check indexes it by family id)");
    return (family < PopFamily::COUNT) ? NAMES[family] : "???";
}

// TWO INDEPENDENT REGISTRIES, ONE LINE. `active` is the family's own
// array, scanned through FAMILY_DISPATCH[f].active_count. `claimed` is
// the footprints keyed to that family. They measure different things
// and MUST agree: delta = claimed − active, and a nonzero delta is a
// leak that names its family — a body with no ground, or ground with
// no body.
//
// The old print walked footprints_[] alone and called the result
// "entities". It reported a proxy: it could not see a family that had
// spawned without registering, and it counted ground that outlived its
// owner as though the owner were alive.
// The delta column: explicit sign, except zero — which has none. Zero is
// the resting state and reads as noise with a sign glued to it.
inline void census_put_delta(int32_t d) {
    if (d == 0) std::cout << std::setw(8) << 0;
    else        std::cout << std::setw(8) << std::showpos << d << std::noshowpos;
}

// NON-PARTICIPANT, not zero. A family that claims no ground (ruling 21) has
// nothing to report in the footprint-derived columns, and `0` would be a
// MEASUREMENT — it would read as "registered nothing", which is a different
// claim from "does not register". The distinction is load-bearing: after
// SPAWN_2 the floaters hold a live `active` against no footprint forever, and
// a row that disagrees permanently and CORRECTLY teaches the reader to
// discount the delta column — the one thing here that catches real leaks.
// Do not "repair" these back to 0.
//
// Padded by hand because the em-dash is three UTF-8 bytes and one display
// column, and setw counts bytes.
inline void census_put_dash(int width) {
    for (int i = 1; i < width; i++) std::cout << ' ';
    std::cout << "—";
}

inline void dump_entity_census(MachineCtx* c, const char* trigger) {
    // The claimed side: footprints, keyed by family. `arrived` (the `new`
    // column) rides this same pass — one scan, not two.
    //
    // NAME IT HONESTLY: `new` is FOOTPRINT-derived, so it counts claimed
    // arrivals, not active ones. It is consistent with the listing beneath
    // it, which is also footprint data. A family that spawned WITHOUT
    // registering shows 0 here while its `active` count still moves — and
    // that disagreement is exactly what the delta column exists to catch.
    uint32_t claimed[PopFamily::COUNT] = {};
    uint32_t arrived[PopFamily::COUNT] = {};
    uint32_t claimed_total = 0;   // sum over the six families
    uint32_t arrived_total = 0;
    uint32_t occupancy = 0;       // every live slot, family or not
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        const auto& fp = c->spawn_engine_state_.footprints_[i];
        if (!fp.active) continue;
        occupancy++;
        if (fp.family >= PopFamily::COUNT) continue;
        claimed[fp.family]++;
        claimed_total++;
        if (c->time_state_.seconds - fp.spawn_time < CENSUS_DUMP_INTERVAL) {
            arrived[fp.family]++;
            arrived_total++;
        }
    }

    std::cout << "[CENSUS t=" << std::fixed << std::setprecision(1) << std::setw(7)
        << c->time_state_.seconds << " trigger=" << trigger << "]\n"
        << "  fam    active  claimed   delta     new\n";

    uint32_t active_total = 0;            // all six — reports what EXISTS
    uint32_t active_grounded_total = 0;   // registrants only — feeds the delta
    for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
        const uint32_t a = FAMILY_DISPATCH[f].active_count(c);
        active_total += a;
        std::cout << "  " << std::left << std::setw(7) << family_short_name(f) << std::right
            << std::setw(6) << a;
        if (FAMILY_DISPATCH[f].grounded) {
            active_grounded_total += a;
            std::cout << std::setw(9) << claimed[f];
            census_put_delta((int32_t)claimed[f] - (int32_t)a);
            // `new` is an unsigned count: no sign, and a plain 0 at rest.
            std::cout << std::setw(8) << arrived[f];
        }
        else {
            // claimed / delta / new are all footprint-derived — see
            // census_put_dash. This family does not participate.
            census_put_dash(9); census_put_dash(8); census_put_dash(8);
        }
        std::cout << "\n";
    }

    // TOTAL's columns answer different questions, deliberately. `active` sums
    // all six, because it reports what exists. `claimed` sums only
    // registrants, because only registrants can have footprints. The delta
    // must therefore be the sum of the PRINTED deltas — measured against
    // active_grounded_total — or TOTAL would report a permanent leak equal to
    // the live floater population, which is not a leak at all.
    std::cout << "  " << std::left << std::setw(7) << "TOTAL" << std::right
        << std::setw(6) << active_total
        << std::setw(9) << claimed_total;
    census_put_delta((int32_t)claimed_total - (int32_t)active_grounded_total);
    std::cout << std::setw(8) << arrived_total
        << "    footprints " << occupancy << "/" << MAX_FOOTPRINTS << "\n";

    // An unfamilied live footprint is unreachable through the three
    // register sites (all pass a real PopFamily), so this never fires
    // today. It is here because occupancy is what saturates, and the
    // per-family sum is what the delta column is built from: if those
    // two ever part company, every delta above is understated.
    if (occupancy != claimed_total) {
        std::cout << "  [CENSUS] WARNING: " << (occupancy - claimed_total)
            << " live footprint(s) carry no family — deltas above are understated\n";
    }

    // ─── SLOT OCCUPANCY (ARCH_2) ──────────────────────────────────────
    //
    // THE TABLE ABOVE HAS NO DENOMINATOR. It answers "how many stand" and
    // "does ground agree with body"; it cannot answer "could there have
    // been more", because nothing in it names a ceiling. This block adds
    // the ceiling and the reach, and it is the number the arch tier
    // weights have to be read against.
    //
    // WHY IT DECIDES: ArchConfig::SPAWN_CHANCE sets how many arches are
    // ATTEMPTED; a family's tier weights only choose WHICH tier each
    // attempt becomes. They are zero-sum in absolute counts. So a tier
    // reweight adds big arches only while the array has room — if live is
    // already sitting at capacity, the same reweight adds nothing and
    // merely reshuffles which bodies win a first-come race for the
    // sixteen slots. Same table, opposite conclusion, and the only thing
    // that tells them apart is printed here.
    //
    // ALL SIX ROWS, not the grounded four. The dash convention above is
    // for FOOTPRINT-derived columns, and a family that claims no ground
    // genuinely has nothing to report there. These columns are
    // ARRAY-derived: every family has an instance array with a bound, so
    // every family has an honest answer, floaters included — and with the
    // portal column gone there is no dash left in this table at all.
    //
    // hi-wtr is one past the highest live slot AT THIS SCAN — the
    // allocator's reach, not a session peak. Nothing is stored between
    // dumps. live == cap is a full array; hi-wtr == cap with live under it
    // is an array that has been full and now carries holes, which is what
    // a population cycling against its ceiling looks like when the census
    // samples it mid-breath. Both readings mean "the ceiling is binding".
    //
    // THE PORTAL COLUMN STOOD HERE, arch-only and dashed on every other
    // row: a subset of the arch row's live, counting the doors among the
    // arches. It left with the doors (ONE_WORLD-I).

    std::cout << "  fam      live   hi-wtr     cap\n";
    for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
        const SlotCensus s = FAMILY_DISPATCH[f].slot_census(c);
        std::cout << "  " << std::left << std::setw(7) << family_short_name(f) << std::right
            << std::setw(6) << s.live
            << std::setw(9) << s.high_water
            << std::setw(8) << s.capacity;
        std::cout << "\n";
    }

    // ARRIVALS ONLY — the detail listing shows what appeared since the last
    // census, not everything that stands.
    //
    // A snapshot re-printed on a timer cannot tell a world at rest from a
    // world in motion: it emits the same ~140 lines either way. That is how a
    // static list came to read as a spawn sequence, how a frozen world looked
    // busy, and how the print's own cost read as a spawn burst. Filtering to
    // arrivals makes stillness print nothing, which is the point.
    //
    // THE WINDOW IS THE INTERVAL CONSTANT, not a delta against
    // lastCensusDump_. Boot and mood-transition never write that field (they
    // mirror the agent census, which does not either), so a delta would
    // measure the wrong span immediately after a transition; a fixed window
    // means the same thing at every trigger. Sub-frame slop is expected and
    // deliberately untreated — the periodic gate fires at 30.0-30.02s, so an
    // arrival may miss its window by a frame. No epsilon.
    struct CensusEntry { uint32_t fp_idx; float spawn_time; };
    CensusEntry entries[MAX_FOOTPRINTS];
    uint32_t n = 0;
    for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
        const auto& fp = c->spawn_engine_state_.footprints_[i];
        if (!fp.active || fp.family >= PopFamily::COUNT) continue;
        if (c->time_state_.seconds - fp.spawn_time >= CENSUS_DUMP_INTERVAL) continue;
        entries[n++] = { i, fp.spawn_time };
    }
    // Insertion sort by spawn_time ASCENDING, and by nothing else. The family
    // term is gone: family-major ordering is precisely what made a static
    // listing read as arrival order. Sorted by age alone, a burst shows up as
    // a run of near-equal ages — which is how the patch-row cadence was
    // diagnosed by hand in the first place.
    for (uint32_t i = 1; i < n; i++) {
        CensusEntry key = entries[i]; uint32_t j = i;
        while (j > 0 && entries[j - 1].spawn_time > key.spawn_time) {
            entries[j] = entries[j - 1]; j--;
        }
        entries[j] = key;
    }
    // CLAIMED GROUND, not entities. XZ, host patch and age live in the
    // registry, so this listing can only ever describe footprints — the
    // owning body may already be gone. Only the CONTENTS narrowed to
    // arrivals; the claim the label makes is unchanged.
    if (n > 0) {
        // Oldest-first, so the tail that gets dropped is the most recent. A
        // later census will not show them: a known, accepted loss for a
        // diagnostic that must not cost a frame.
        const uint32_t shown = (n < CENSUS_LISTING_MAX) ? n : CENSUS_LISTING_MAX;
        std::cout << "  claimed ground — arrivals (" << n << "):\n";
        for (uint32_t i = 0; i < shown; i++) {
            const auto& fp = c->spawn_engine_state_.footprints_[entries[i].fp_idx];
            std::cout << "  " << family_short_name(fp.family)
                << " t" << fp.tier
                << " (" << std::setw(8) << std::setprecision(1) << fp.x
                << "," << std::setw(8) << fp.z << ")"
                << " p(" << std::setw(3) << fp.patch_gx << "," << std::setw(3) << fp.patch_gz << ")"
                << " age=" << std::setprecision(1) << (c->time_state_.seconds - fp.spawn_time)
                << "\n";
        }
        if (n > shown) std::cout << "    ... +" << (n - shown) << " more\n";
    }
    std::cout << std::flush;
}

// ═══ SPAWN UTILITIES ═════════════════════════════════════════════
//
// The spawn lifecycle's smallest building blocks: the composition
// law, gate evaluation, jittered position, and the
// affinity boost.

// ─── Spawn gate ──────────────────────────────────────────────────

// ═══ THE COMPOSITION LAW — definition (decl: spawn_services.hpp) ═══
// The ONE place the spawn-probability stack is authored. The
// float multiplication ORDER below is the bit-identity contract
// — do not reorder a multiply, do not move a
// clamp. Exact argument orders of min/max preserved per policy.
inline float compose_spawn_chance(MachineCtx* c, int32_t gx, int32_t gz,
    uint32_t family, float base_chance, SpawnClamp clamp) {
    // THE MOOD TERM WAS IDENTITY AND IS NOW ABSENT (ONE_WORLD-II U3). The
    // stack read mood -> global -> tile (F3) -> base x adj -> clamp. The
    // mood term indexed MOOD_SPAWN_MULT by the live mood; the sunset row
    // read { 1, 1, 1, 1, 1 } for all five families, so this is
    // behaviour-identical and not merely intended. The TILE term went with
    // the theme lattice that authored it. What is left is the global
    // density and the family's own base chance.
    float adj_mod = GLOBAL_ENTITY_DENSITY;
    float chance = base_chance * adj_mod;
    switch (clamp) {
        case SpawnClamp::MIN1:    chance = std::min(chance, 1.0f); break;
        case SpawnClamp::RANGE01: chance = std::max(0.0f, std::min(1.0f, chance)); break;
    }
    return chance;
}

// Evaluate the spawn gate: seed + flat probability check.
inline SpawnPreamble evaluate_spawn_gate(MachineCtx* c, int32_t gx, int32_t gz,
    uint32_t spawn_roll_prop,
    float chance) {
    SpawnPreamble result{};
    // (per-gate archetype lookup CUT: computed for
    //  every generic gate, read by nobody; its sole consumer called
    //  tile_archetype itself, and left with PRUNE_1.)
    result.seed = tile_seed(c->world_state_.active_seed, gx, gz);
    result.passed = cpu_hash_f(result.seed, spawn_roll_prop) < chance;
    return result;
}

// Jittered world position within a patch.
inline void jittered_position(uint32_t seed, int32_t gx, int32_t gz,
    uint32_t prop_x, uint32_t prop_z, float jitter,
    float& out_x, float& out_z) {
    out_x = (gx + 0.5f) * Dim::PATCH_EXTENT + (cpu_hash_f(seed, prop_x) - 0.5f) * Dim::PATCH_EXTENT * jitter;
    out_z = (gz + 0.5f) * Dim::PATCH_EXTENT + (cpu_hash_f(seed, prop_z) - 0.5f) * Dim::PATCH_EXTENT * jitter;
}


// ─── Select / Place / Commit dispatch loops ─────────────────────

inline void select_entities_for_patch(MachineCtx* c, int32_t gx, int32_t gz) {
    // PLACEMENT_ORDER (roster.hpp, F-6) is the priority, not the loop counter:
    // push order IS placement order, so whoever comes first claims contested
    // ground first. Identity today, so this is bit-identical to the old
    // `f = 0..COUNT`.
    for (uint32_t i = 0; i < PopFamily::COUNT; i++) {
        const uint32_t f = PLACEMENT_ORDER[i];
        if (!ROSTER.family_enabled(f)) continue;  // ROSTER-GATE family (b) — disabled family never selected -> never placed/committed/meshed/drawn. Budgeted stream path, not the per-frame hot path.
        EntityQueueEntry e{};
        e.family = f;
        e.gx = gx; e.gz = gz;
        if (!FAMILY_DISPATCH[f].try_select(c, gx, gz, e)) continue;
        auto& st = c->spawn_engine_state_;
        if (st.entityQueueCount_ >= SPAWN_QUEUE_MAX) {
            // Unreachable if the bound above holds. LOUD rather than silent:
            // a dropped selection is an entity that never existed, with no
            // other symptom anywhere.
            std::cerr << "[SPAWN] entityQueue_ OVERFLOW at " << SPAWN_QUEUE_MAX
                      << " — dropping " << family_short_name(f)
                      << " and the rest of this patch. The proven bound"
                         " (SPAWN_BUDGET_PER_FRAME x PopFamily::COUNT) is wrong.\n";
            break;
        }
        st.entityQueue_[st.entityQueueCount_++] = e;
    }
}

// ─── Place: spatial negotiation (no GPU writes) ──────────────

inline void place_entity_queue(MachineCtx* c) {
    auto& st = c->spawn_engine_state_;
    for (uint32_t i = 0; i < st.entityQueueCount_; i++) {
        PlacementEntry pe{};
        if (!FAMILY_DISPATCH[st.entityQueue_[i].family].try_place(c, st.entityQueue_[i], pe))
            continue;
        if (st.placementCount_ >= SPAWN_QUEUE_MAX) {
            // Structurally unreachable — at most one push per queue entry, and
            // the queue shares this ceiling. Guarded anyway, and loudly.
            std::cerr << "[SPAWN] placementResults_ OVERFLOW at " << SPAWN_QUEUE_MAX
                      << " — dropping a placed entity\n";
            break;
        }
        st.placementResults_[st.placementCount_++] = pe;
    }
    st.entityQueueCount_ = 0;
}

// ─── Commit: GPU writes from placement results ──────────────

inline void commit_entity_queue(MachineCtx* c, wgpu::Queue& queue) {
    auto& st = c->spawn_engine_state_;
    for (uint32_t i = 0; i < st.placementCount_; i++)
        FAMILY_DISPATCH[st.placementResults_[i].family].try_commit(c, st.placementResults_[i], queue);
    st.placementCount_ = 0;
}

} // namespace the_board
} // namespace t7
