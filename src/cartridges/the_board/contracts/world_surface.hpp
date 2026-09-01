#pragma once
#include <cstdint>

// ─── contracts/world_surface.hpp ──────────────────────────────────
//
// THE WORLD'S OWN BANK (THE_PANEL I U1). One fact: the seed the NEXT
// world is drawn from. It is the hand's half of the seed door — the
// door presses `rebirth_world`, and this is the number it presses it
// with.
//
// C3 DESTRUCTIVE, the whole bank, and there is only the one row.
// `rebirth_world` tears the standing world down before it draws
// another: every teardown verb, the agent reset, the repopulation,
// the surface rebuild. Re-speaking that author on a slider drag would
// destroy the world once per event, so the bank has NO BOUNDARY
// WIRING (D5) and its row wears the GEN chip — L44's temperament law,
// the same answer RIBBON_SPAWN_LIVE and AGENTS_LIVE give.
//
// NO DESIGN TABLE, ON PURPOSE, and this is the one enrolled bank
// without one. Every other bank graduated from a constexpr the module
// had already authored; a world's seed was never authored. DRAW_0 is
// explicit that it is DRAWN at boot — wall clock folded through
// cpu_hash, or the T7_WORLD_SEED pin, or `--seed=` — so a
// `WORLD_TABLE` would have to invent a number no one chose and call
// it the design. The composition root seats this bank from the seed
// it actually drew (cartridge.hpp, the one authoring site), which is
// also what the manifest's own law wants: a surface opens showing the
// PROGRAM, not its own defaults.
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

struct WorldSurface {
    // The seed the seed door hands `rebirth_world`. Seated at boot from
    // the world's own drawn seed; read at exactly one site — the door
    // handler at the frame boundary (organ_boundary.inc).
    uint32_t next_seed;
};

// SEATED BY THE ROOT, NOT HERE. The in-struct default would be a second
// authoring site for a fact the composition root already settles, and
// the root's line runs after boot_seed() and after the `--seed=`
// override, so it is the only line that can know the answer.
inline WorldSurface WORLD_LIVE = { 0u };

}  // namespace the_board
}  // namespace t7
