#pragma once
#include <cstdint>

// ═══ ROSTER MANIFEST (v0) ═══════════════════════════════════════════
//
// Spine-owned single source of truth for which PIECES are enabled. The
// roster gates DOORS — (a) buffer creation, (b) dispatch/registration,
// (c) boot/teardown; it never changes an algorithm (scope guard).
// Consulted ONLY at gate sites, each tagged with the literal sentinel
// ROSTER-GATE (piece + gate kind a/b/c); the residue check (gol) is
// tagged ROSTER-RESIDUE; shared creation sites are tagged
// LATENT[gate-a-shared].
//
// HOME: this lived as a Cartridge static member until the
//   roster met its SECOND CONSUMER — GPUState::init (state.hpp) needs the
//   feature bits to gate creation. The standing law (readings publish at
//   the second consumer) graduates it here, included by state.hpp and
//   cartridge.hpp. Namespace-scope `inline constexpr` = one definition,
//   compile-time folded at every use (all-enabled is byte-identical to the
//   pre-graduation build).
//
// RIDER A: DISABLED = ZERO GPU WRITES, not merely zero draws. A disabled
//   piece is never collected / registered / dispatched / drawn and writes
//   no instance buffer, indirect arg, or compute output. A SHARED piece's
//   buffers still EXIST (co-resident in a megabind) but stay pristine.
//
// MATURITY DIAL: the GATES are the stable contract; the table's
//   CONSTNESS is the dial — v0 constexpr -> boot-time table ->
//   requirements-face resolver (the program theory).
//
// ─── GATE-(a) STATUS (the cost column) ──────────────────────────
//   The creation-side classification (full cost table with buffers/
//   groups/pipelines and retirement per piece).
//   SEP     = separable now: creation skipped atomically, boot+draws safe.
//   SH·mb   = shared, an EXCLUSIVE buffer/texture is bound into a megabind
//             (created-pristine; retirement = re-section the group).
//   SH·dc   = dedicated resources but draw-coupled: the co-owned instance
//             store stays, the forward draw isn't self-gated (created-
//             pristine; retirement = a behavior-identical draw self-gate,
//             then skip — no re-section).
//   NO-RES  = no gateable creation of its own.
//
//   Only SEP pieces skip creation in v0; SH·* sites carry
//   LATENT[gate-a-shared] with the retirement condition; NO-RES pieces own
//   nothing to skip. This arc gates one SEP piece (indoor_shell).

namespace t7 {
namespace the_board {

// ═══ FAMILY IDENTITY ═════════════════════════════════════════════

struct PopFamily {
    static constexpr uint32_t PYRAMID = 0;
    static constexpr uint32_t SPHERE = 1;    // orbital spheres
    static constexpr uint32_t RIBBON = 2;
    static constexpr uint32_t CUBE = 3;      // hover-bob monoliths (split from legacy FLOATING)
    static constexpr uint32_t GOL = 4;       // Game of Life / Pulse automaton zones
    static constexpr uint32_t COUNT = 5;
};

// F-1: the family ORDER is load-bearing — FIVE
// tables are POSITIONAL in it (the five proximity tables left at
// ONE_WORLD-I U5: MIN_SEPARATION, THEMES[].spawn_weight, MOOD_SPAWN_MULT,
// TilePopulation::spatial_density, INDOOR_TREATMENT, and
// family_short_name's NAMES[] — the eleventh, which PRUNE_2 found outside
// this roll call and pinned to COUNT where it lives), as is
// FAMILY_DISPATCH (whose rows are
// additionally name-checked at boot by validate_spine, F-2, THROUGH that
// same NAMES[]) and PLACEMENT_ORDER (pinned by F-6 below). AND (charter
// extended): the enum order IS PLACEMENT
// PRIORITY — select_entities_for_patch loops f=0..COUNT and the queue
// places in push order, so within a patch PYRAMID's footprint registers
// before ARCH's separation check, ARCH's before SPHERE's… the order
// allocates GROUND, not just table columns. Renumbering ANY family
// re-columns the tables AND reorders who wins contested ground — this
// assert turns both into a compile error instead of a silent world-change.
//
// PRUNE_1 U6 removed GALLERY, and could, because GALLERY was the LAST
// family: a tail cut is pure truncation. No surviving family renumbered,
// no surviving table column moved relative to another, and placement
// priority among the survivors is unchanged. The F-1 fear does not bite a
// tail cut — it bites a renumbering, and there was none.
//
// PRUNE_2 IS THE OTHER KIND, and ONE_WORLD-I U3 is the same kind again:
// both excise MID-TABLE families, so every family above the cut
// renumbers and every one of the positional tables loses that column.
// The fear bites, and the answer is not to dodge it: each excision
// commit re-columns all eleven tables AND FAMILY_DISPATCH in the same
// commit, and rewrites this assert to the surviving pins. Relative order
// among survivors is preserved — the cut closes ranks, it never
// reshuffles — so placement priority among the survivors is unchanged,
// exactly as in a tail cut. U3 took ARCH at index 1, so SPHERE, RIBBON,
// CUBE and GOL each dropped one.
static_assert(PopFamily::PYRAMID == 0
           && PopFamily::SPHERE  == 1 && PopFamily::RIBBON  == 2
           && PopFamily::CUBE    == 3 && PopFamily::GOL     == 4
           && PopFamily::COUNT   == 5,
    "PopFamily ORDER is the spawn tables' row/column contract (F-1): "
    "re-column all eleven PopFamily-ordered tables + FAMILY_DISPATCH "
    "before renumbering any family");

// ═══ PLACEMENT ORDER ═════════════════════════════════════════════
//
// WHO WINS CONTESTED GROUND, as data. select_entities_for_patch walks this
// array and pushes in that order, and the queue places in push order — so
// within one patch the family listed first registers its footprint before the
// next family's check_position runs. The order allocates GROUND.
//
// It was previously the loop counter itself (`for f = 0..COUNT`), which
// welded placement priority to the enum. But the enum is ALSO the column
// order of eleven positional tables and the row order of FAMILY_DISPATCH
// (F-1), so re-ranking priority meant re-columning everything. Splitting the
// two lets priority be re-ranked here, alone, without touching a single
// table — PopFamily stays pinned.
//
// IDENTITY DEFAULT: today this is exactly PopFamily order, so behaviour is
// unchanged. Reordering is a deliberate, isolated edit.
inline constexpr uint32_t PLACEMENT_ORDER[PopFamily::COUNT] = {
    PopFamily::PYRAMID, PopFamily::SPHERE,
    PopFamily::RIBBON,  PopFamily::CUBE,   PopFamily::GOL,
};

// F-6: PLACEMENT_ORDER must be a PERMUTATION of 0..COUNT-1 — every family
// exactly once. A duplicated index would silently drop the family it displaced
// from every spawn, forever, with no other symptom: no crash, no log, just a
// world missing a family. This is the cheapest possible guard against that,
// and it runs at compile time.
constexpr bool placement_order_is_permutation() {
    bool seen[PopFamily::COUNT] = {};
    for (uint32_t i = 0; i < PopFamily::COUNT; i++) {
        const uint32_t f = PLACEMENT_ORDER[i];
        if (f >= PopFamily::COUNT) return false;   // out of range
        if (seen[f]) return false;                 // duplicate
        seen[f] = true;
    }
    for (uint32_t f = 0; f < PopFamily::COUNT; f++)
        if (!seen[f]) return false;                // omitted
    return true;
}
static_assert(placement_order_is_permutation(),
    "F-6: PLACEMENT_ORDER must list every PopFamily exactly once — a duplicate "
    "or omission silently removes a family from every spawn");

struct Roster {
    bool pyramid, sphere, ribbon, cube, gol;
    // FEATURES (5)
    bool pawn_aura;     // presence ramp + aura terrain compute
    bool orbs;          // sky dome (distinct from the sphere family)
    bool spot_lights;   // indoor spot array + shadow atlas
    bool indoor_shell;  // walls + ceiling mesh
    bool wanderers;     // mood-authored NPC population (agent slots 1+)

    constexpr bool family_enabled(uint32_t f) const {
        switch (f) {
            case PopFamily::PYRAMID: return pyramid;
            case PopFamily::SPHERE:  return sphere;
            case PopFamily::RIBBON:  return ribbon;
            case PopFamily::CUBE:    return cube;
            case PopFamily::GOL:     return gol;
            default: return true;
        }
    }

    constexpr bool all_enabled() const {
        return pyramid && sphere && ribbon && cube && gol &&
               pawn_aura && orbs && spot_lights && indoor_shell &&
               wanderers;
    }

    // Mirror of all_enabled — the degenerate sentence (every tickable
    // bit off). Its one consumer is the matrix's minimal-column golden
    // (demos/matrix.hpp), the compile-time proof that demo=minimal
    // still equals the retired minimal.hpp.
    constexpr bool none_enabled() const {
        return !pyramid && !sphere && !ribbon && !cube && !gol &&
               !pawn_aura && !orbs && !spot_lights && !indoor_shell &&
               !wanderers;
    }
};

// ROSTER (the selected constant) is defined in demos/demo.hpp as
// DEMO.roster — the demo sentence selected at compile time (the
// config spine; the sentence's bits authored in the MATRIX,
// demos/matrix.hpp). The TYPE and the gate law live
// here; the FIRST EDGE static_assert rides the selector, where the
// sentence lands. All-enabled (the matrix's full column) remains
// byte-identical to the pre-spine build — the constexpr chain is
// unchanged end to end.

} // namespace the_board
} // namespace t7
