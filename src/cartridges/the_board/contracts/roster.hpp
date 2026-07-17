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
// HOME (ROSTER-1b 3a): this lived as a Cartridge static member until the
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
//   requirements-face resolver (theory v2 §5).
//
// LATENT[roster-split:photographer]: the photographer (capture cadence +
//   snapshot pass) rides gallery's bit for v0. Split into its own bit the
//   day authored-only exhibits with a dead camera are wanted.
//
// ─── GATE-(a) STATUS (the M-j cost column) ──────────────────────────
// ROSTER-1a/1b: creation + graduation history in audit/LADDER.md.
//   The creation-side classification (see audit/ROSTER_GATE_A.md for the
//   full cost table with buffers/groups/pipelines and retirement per piece).
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
    static constexpr uint32_t ARCH = 1;
    static constexpr uint32_t COLUMN = 2;
    static constexpr uint32_t ANTENNA = 3;
    static constexpr uint32_t PALM = 4;
    static constexpr uint32_t CACTUS = 5;
    static constexpr uint32_t BLADE = 6;
    static constexpr uint32_t SPHERE = 7;    // orbital spheres
    static constexpr uint32_t RIBBON = 8;
    static constexpr uint32_t CUBE = 9;      // hover-bob monoliths (split from legacy FLOATING)
    static constexpr uint32_t GOL = 10;       // Game of Life / Pulse automaton zones
    static constexpr uint32_t GALLERY = 11;   // outdoor art exhibitions (composite: 1 center → N paintings)
    static constexpr uint32_t COUNT = 12;
};

// F-1 (the annotation-pass pin): the family ORDER is load-bearing — eight
// spawn tables are POSITIONAL in it (MIN_SEPARATION, the four PROXIMITY_*
// vectors, PROXIMITY_AFFINITY, THEMES[].spawn_weight,
// TilePopulation::spatial_density), as is FAMILY_DISPATCH (whose rows are
// additionally name-checked at boot by validate_spine, F-2). AND (charter
// extended, composition recon R5/§4.2): the enum order IS PLACEMENT
// PRIORITY — select_entities_for_patch loops f=0..COUNT and the queue
// places in push order, so within a patch PYRAMID's footprint registers
// before ARCH's separation check, ARCH's before COLUMN's… the order
// allocates GROUND, not just table columns. Renumbering ANY family
// re-columns the tables AND reorders who wins contested ground — this
// assert turns both into a compile error instead of a silent world-change.
static_assert(PopFamily::PYRAMID == 0 && PopFamily::ARCH    == 1
           && PopFamily::COLUMN  == 2 && PopFamily::ANTENNA == 3
           && PopFamily::PALM    == 4 && PopFamily::CACTUS  == 5
           && PopFamily::BLADE   == 6 && PopFamily::SPHERE  == 7
           && PopFamily::RIBBON  == 8 && PopFamily::CUBE    == 9
           && PopFamily::GOL     == 10 && PopFamily::GALLERY == 11
           && PopFamily::COUNT   == 12,
    "PopFamily ORDER is the spawn tables' row/column contract (F-1): "
    "re-column all eight PopFamily-ordered tables + FAMILY_DISPATCH "
    "before renumbering any family");

struct Roster {
    bool pyramid, arch, column, antenna, palm, cactus, blade,
         sphere, ribbon, cube, gol, gallery;
    // FEATURES (7)
    bool pawn_aura;     // presence ramp + aura terrain compute
    bool orbs;          // sky dome (distinct from the sphere family)
    bool spot_lights;   // indoor spot array + shadow atlas
    bool indoor_shell;  // walls + ceiling mesh
    bool portal;        // force-spawn portal arches (the second door — lives in entities' force_spawn_portal_arch, the arch owner's authoring channel)
    bool transitions;   // mood-transition ENTRY (request + portal trigger)
    bool wanderers;     // mood-authored NPC population (agent slots 1+)

    constexpr bool family_enabled(uint32_t f) const {
        switch (f) {
            case PopFamily::PYRAMID: return pyramid;
            case PopFamily::ARCH:    return arch;
            case PopFamily::COLUMN:  return column;
            case PopFamily::ANTENNA: return antenna;
            case PopFamily::PALM:    return palm;
            case PopFamily::CACTUS:  return cactus;
            case PopFamily::BLADE:   return blade;
            case PopFamily::SPHERE:  return sphere;
            case PopFamily::RIBBON:  return ribbon;
            case PopFamily::CUBE:    return cube;
            case PopFamily::GOL:     return gol;
            case PopFamily::GALLERY: return gallery;
            default: return true;
        }
    }

    constexpr bool all_enabled() const {
        return pyramid && arch && column && antenna && palm && cactus &&
               blade && sphere && ribbon && cube && gol && gallery &&
               pawn_aura && orbs && spot_lights && indoor_shell && portal &&
               transitions && wanderers;
    }

    // Mirror of all_enabled — the degenerate sentence (every tickable
    // bit off). Its one consumer is the matrix's minimal-column golden
    // (demos/matrix.hpp), the compile-time proof that demo=minimal
    // still equals the retired minimal.hpp.
    constexpr bool none_enabled() const {
        return !pyramid && !arch && !column && !antenna && !palm && !cactus &&
               !blade && !sphere && !ribbon && !cube && !gol && !gallery &&
               !pawn_aura && !orbs && !spot_lights && !indoor_shell && !portal &&
               !transitions && !wanderers;
    }
};

// ROSTER (the selected constant) is defined in demos/demo.hpp as
// DEMO.roster — the demo sentence selected at compile time (DEMO-1,
// the config spine; the sentence's bits authored in the MATRIX,
// demos/matrix.hpp, since PANEL-0 p2). The TYPE and the gate law live
// here; the FIRST EDGE static_assert rides the selector, where the
// sentence lands. All-enabled (the matrix's full column) remains
// byte-identical to the pre-spine build — the constexpr chain is
// unchanged end to end.

} // namespace the_board
} // namespace t7
