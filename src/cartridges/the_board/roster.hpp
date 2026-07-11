#pragma once
#include <cstdint>

// ═══ ROSTER MANIFEST (v0) — ROSTER-1a / 1b ═══════════════════════════════
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
// MATURITY DIAL: v0 is this constexpr table. The GATES are the stable
//   contract; the table's CONSTNESS is the dial — v0 constexpr -> boot-time
//   table -> requirements-face resolver (theory v1 M-j). The dependency
//   edges below (FOUNDATIONAL + the transitions=>portal edge) are the
//   resolver embryo: one structure of bits, edges, and root-anchored
//   edges — not a table plus a footnote.
//
// FOUNDATIONAL (non-gateable — a reader might expect a bit; there is none,
//   by dependency, anchored at the build root):
//   * agents machinery — agentStateBuffer_, the agent compute kernel, and
//     slot 0. The reference body (the pawn) IS agent slot 0, and the buffer
//     co-resides 7 shared bind groups (ROSTER_RECON R2). Disabling it would
//     unmake the pawn. The GATEABLE half is `wanderers` (the mood-authored
//     population, agent slots 1+); slot 0 is untouched by it.
//   * the sun — directional light + shadow map + its shadow pipelines. The
//     sun rides upload_lights (rider comment at the call site); it is not a
//     piece. Terrain substrate, camera, frustum cull, patch stores, the
//     frame signal / config / VP are foundational likewise.
//
// LATENT[roster-split:photographer]: the photographer (capture cadence +
//   snapshot pass) rides gallery's bit for v0. Split into its own bit the
//   day authored-only exhibits with a dead camera are wanted.
//
// spot_lights x indoor_shell: INDEPENDENT (ROSTER-1a §5 check) —
//   apply_mood_spot_lights keys off derive_indoor_lights,
//   apply_mood_indoor_shell off the ceiling mesh; both off m.indoor
//   separately, no shared state. Either may be disabled without the other.
//   No edge.
//
// ─── GATE-(a) STATUS (ROSTER-1b Phase R' — the M-j cost column) ──────────
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
//     SEP     indoor_shell
//     SH·mb   pyramid arch column ribbon gol gallery pawn_aura orbs spot_lights
//     SH·dc   sphere cube palm cactus blade
//     NO-RES  antenna (rides column mesh)   portal (rides arch storage)
//             transitions (owns only fadeOverlayPipeline_, draw-coupled)
//             wanderers (rides foundational agentStateBuffer_ slots 1+)
//
//   Only SEP pieces skip creation in v0; SH·* sites carry
//   LATENT[gate-a-shared] with the retirement condition; NO-RES pieces own
//   nothing to skip. This arc gates one SEP piece (indoor_shell).

namespace t7 {
namespace the_board {

struct Roster {
    // FAMILIES (12) — order MUST match PopFamily. (b) gate: the select loop
    // (spawn_engine.inl); the per-frame mesh-prepare loop folds these by
    // constexpr (R1 hot-path caveat — no runtime branch on a disabled piece
    // in the hot path).
    bool pyramid, arch, column, antenna, palm, cactus, blade,
         sphere, ribbon, cube, gol, gallery;
    // FEATURES (7)
    bool pawn_aura;     // presence ramp + aura terrain compute
    bool orbs;          // sky dome (distinct from the sphere family)
    bool spot_lights;   // indoor spot array + shadow atlas
    bool indoor_shell;  // walls + ceiling mesh
    bool portal;        // force-spawn portal arches (the second door)
    bool transitions;   // mood-transition ENTRY (request + portal trigger)
    bool wanderers;     // mood-authored NPC population (agent slots 1+)

    // Family bit by PopFamily id — constexpr, usable at runtime (select
    // loop) and compile time (mesh fold: `if constexpr
    // (ROSTER.family_enabled(F))`). PopFamily (spawn_engine.inl) is
    // class-nested and NOT visible here (this header is included before the
    // class); the literal order below mirrors it, and cartridge.hpp
    // static-asserts the binding (PopFamily::PYRAMID==0 … GALLERY==11,
    // COUNT==12) so a family reorder fails loud at that seam.
    constexpr bool family_enabled(uint32_t f) const {
        switch (f) {
            case 0:  return pyramid;   // PopFamily::PYRAMID
            case 1:  return arch;      // PopFamily::ARCH
            case 2:  return column;    // PopFamily::COLUMN
            case 3:  return antenna;   // PopFamily::ANTENNA
            case 4:  return palm;      // PopFamily::PALM
            case 5:  return cactus;    // PopFamily::CACTUS
            case 6:  return blade;     // PopFamily::BLADE
            case 7:  return sphere;    // PopFamily::SPHERE
            case 8:  return ribbon;    // PopFamily::RIBBON
            case 9:  return cube;      // PopFamily::CUBE
            case 10: return gol;       // PopFamily::GOL
            case 11: return gallery;   // PopFamily::GALLERY
            default: return true;
        }
    }
};

// v0: ALL PIECES ENABLED — the golden anchor (G0: all-enabled is
// byte-identical to the pre-roster build). Flip a bit to gate a piece.
// Lean music-viz build (the founding customer): set
// transitions/portal/indoor_shell/spot_lights/wanderers false.
inline constexpr Roster ROSTER = {
    // families, PopFamily order: pyr arch col ant palm cact blade sph rib cube gol gall
    true, true, true, true, true, true, true, true, true, true, true, true,
    // features: pawn_aura orbs spot_lights indoor_shell portal transitions wanderers
    true,      true, true,       true,        true,   true,       true,
};

// THE FIRST EDGE (2c) — transitions REQUIRE portal (M-j embryo). Portals
// are BOTH the transition trigger IN and the guaranteed return path OUT;
// transitions on + portal off soft-locks. CONDITIONAL (not unconditional)
// so the lean music-viz build — the founding customer, transitions+portal
// off together — stays legal; an unconditional assert would make the portal
// bit never legally false. DUAL MATURITY: this static_assert is v0's form
// of the edge; the early-return door in request_mood_transition (and
// force_spawn_portal_at) is the maturity-proof form that goes live when the
// table turns boot-time. Both doors, both correct, different maturities.
static_assert(!ROSTER.transitions || ROSTER.portal,
    "ROSTER: portal disabled while transitions enabled — "
    "transitions REQUIRE portal (the trigger in, the return path out)");

} // namespace the_board
} // namespace t7
