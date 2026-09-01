#pragma once
#include <cstdint>

// ─── contracts/world_surface.hpp ──────────────────────────────────
//
// THE WORLD'S OWN BANK (THE_PANEL I U1, widened at U3). What a world is
// CHOSEN by, before anything in it is drawn: the seed the next world
// comes from, and the range its radius is drawn within. It is the hand's
// half of the seed door — the door presses `rebirth_world`, and these
// are the numbers it presses it with.
//
// C3 DESTRUCTIVE, the whole bank, and there is only the one row.
// `rebirth_world` tears the standing world down before it draws
// another: every teardown verb, the agent reset, the repopulation,
// the surface rebuild. Re-speaking that author on a slider drag would
// destroy the world once per event, so the bank has NO BOUNDARY
// WIRING (D5) and its row wears the GEN chip — L44's temperament law,
// the same answer RIBBON_SPAWN_LIVE and AGENTS_LIVE give.
//
// NO DESIGN TABLE, AND THE REASON IS PER-FIELD RATHER THAN PER-BANK.
// Every other enrolled bank graduated whole from a constexpr its module
// had already authored. Here the RADIUS RANGE has such a constexpr —
// FINITE_RADIUS_MIN/MAX, in contracts/surface_services.hpp — and the
// SEED does not: DRAW_0 is explicit that a world's seed is DRAWN at
// boot (wall clock folded through cpu_hash, or the T7_WORLD_SEED pin,
// or `--seed=`), so a `WORLD_TABLE` row for it would have to invent a
// number nobody chose and call it the design.
//
// So the composition root seats ALL THREE at the one line that can know
// every answer — after boot_seed(), after the `--seed=` override, and
// before become_world draws the first radius (cartridge.hpp, the one
// authoring site). One seat for the bank, not a table for two thirds of
// it and a seat for the rest. That is also what the manifest's own law
// wants: a surface opens showing the PROGRAM, not its own defaults.
//
// SO THE UNTOUCHED DOOR IS A REDRAW. Press it without turning the
// dial and the world is torn down and drawn again from the same
// number — the same world, rebuilt. That is the honest acceptance
// test for the rebirth path, and turning the dial first is the other
// half of it.
//
// THE FLOAT WIRE'S CEILING IS A FACT ABOUT THE ABI, NOT ABOUT THIS
// FACT. `organ_set` takes a float and `organ_get` returns one, so a
// u32 above 2^24 (16,777,216) does not round-trip exactly: the hand
// types a seed and the wire hands the program the nearest float's
// integer. The RANGE on the enrollment line is still the full u32,
// because that is `next_seed`'s true domain and the door passes it
// whole; the coarseness belongs to the wire and is stated at the row
// rather than disguised as a narrower dial. Every U32 row in the tree
// shares it — this is simply the first one whose domain reaches that
// far. Below 2^24 every seed is exact, and `--seed=` and the pin
// carry the full u32 at boot either way.
// ────────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ THE PIN'S DIALS — the DESIGN, and the CAPACITY ══════════════════
//
// The radius range every world draws from. WorldShape's
// finite_radius_min/max, rehomed to surface_services.hpp beside
// `finite_mode` at ONE_WORLD-II U2, and rehomed here at THE_PANEL I U3
// with the bank that made them live: one home for what a world is
// CHOSEN by. The atrium's pinned radius (min == max, no roll) died with
// the atrium; these are SHAPE_FINITE's, the row the campaign kept.
//
// THEY ARE STILL CONSTEXPR AND THEY MUST BE. FINITE_RADIUS_MAX is not
// only a design value — three static_asserts in
// contracts/surface_services.hpp bind it to frustum-cull segments A and
// B and to the layer pool, and a fourth binds Dim::AUTO_GRID_MAX to
// (2*MAX+1)*PATCH_CELL_N, the automaton's life buffer. Those asserts
// live there because they need Dim:: and FC_SEG_*, which this header
// stands upstream of. A world wider than this pin does not look wrong;
// it indexes past the end of that buffer on its last row, every frame.
//
// SO THE DIAL BELOW IS A DIAL OVER A CAPACITY, and it is fenced three
// times: the enrollment line spells this constant as its ceiling by
// name, organ_set clamps a consumer to it, and derive_finite_radius
// clamps again at the last line before the number becomes an index.
inline constexpr uint32_t FINITE_RADIUS_MIN = 1;
inline constexpr uint32_t FINITE_RADIUS_MAX = 4;

struct WorldSurface {
    // The seed the seed door hands `rebirth_world`. Seated at boot from
    // the world's own drawn seed; read at exactly one site — the door
    // handler at the frame boundary (organ_boundary.inc).
    uint32_t next_seed;

    // ── THE RADIUS RANGE (THE_PANEL I U3) ────────────────────────
    //
    // The dials of the finiteness pin, graduated at last. ONE_WORLD-II
    // §1.7 promised the enrollment and did not land it; ONE_SURFACE §1.4
    // then presumed these were already dials "whose gen-cadence edits
    // reach the world through rebirth". Neither was true, for one
    // reason: nothing could call `rebirth_world`, so an enrolled radius
    // would have been a belief no reader proved (L45). U1 built the
    // caller and this is the enrollment it unblocked.
    //
    // A RANGE, NOT A RADIUS, because the radius is DRAWN.
    // derive_finite_radius takes the world seed through cpu_hash with
    // salt 77 and lands inside [min, max]; setting min == max pins it,
    // which is how a hand asks for one size without a second mechanism.
    // The draw and the salt are unchanged — a given seed under the
    // boot range draws exactly the radius it always drew.
    //
    // AND `max` IS A DIAL OVER A CAPACITY, WHICH IS THE WHOLE CARE HERE.
    // FINITE_RADIUS_MAX is not just a design value: three static_asserts
    // in contracts/surface_services.hpp bind it to frustum-cull segment
    // A and B, to the layer pool, and — through Dim::AUTO_GRID_MAX — to
    // the automaton's life buffer, which is sized (2*MAX+1)*PATCH_CELL_N
    // cells square. A world wider than the pin does not look wrong, it
    // indexes past the end of that buffer on its last row, every frame.
    // So the constant stays compile-time and stays the ASSERTED
    // capacity; the dial's own ceiling IS that constant, the enrollment
    // line spells it by name rather than by number, and
    // derive_finite_radius clamps besides. Three fences, because the
    // thing behind them is memory.
    uint32_t radius_min;
    uint32_t radius_max;
};

// SEATED BY THE ROOT, NOT HERE. These initializers are a POD's floor and
// nothing more — a legible, in-range nothing (the smallest world, seed
// 0) so the bank is never uninitialised if a consumer reads it before
// boot finishes. The values the program runs on are written at the
// composition root, which is the only line that can know them: it runs
// after boot_seed(), after the `--seed=` override, and before
// become_world draws the first radius from this range.
inline WorldSurface WORLD_LIVE = { 0u, 1u, 1u };

}  // namespace the_board
}  // namespace t7
