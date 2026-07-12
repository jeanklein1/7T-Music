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
};

inline constexpr Roster ROSTER = {
    // families, PopFamily order: pyr arch col ant palm cact blade sph rib cube gol gall
    true, true, true, true, true, true, true, true, true, true, true, true,
    // features: pawn_aura orbs spot_lights indoor_shell portal transitions wanderers
    true,      true, true,       true,        true,   true,       true,
};

// THE FIRST EDGE — transitions REQUIRE portal: portals are both the
// trigger IN and the guaranteed return OUT; transitions on + portal
// off soft-locks. CONDITIONAL so transitions+portal both-off (the
// lean build) stays legal.
static_assert(!ROSTER.transitions || ROSTER.portal,
    "ROSTER: portal disabled while transitions enabled — "
    "transitions REQUIRE portal (the trigger in, the return path out)");

} // namespace the_board
} // namespace t7
