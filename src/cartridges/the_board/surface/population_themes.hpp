#pragma once

// ─── population_themes.hpp — THE POPULATION PANEL ────────────────
//
// ONE_WORLD-II U3. This file was the THEME ENGINE: a five-row THEMES
// vocabulary, a spatial lattice selector, a stateful temporal envelope
// with per-theme cooldowns, per-family tier-weight biases, and a
// mood x family multiplier matrix. All of it is gone. What survives is
// the file's ROLE, which its WGSL twin has named all along — world.wgsl
// §2.2 calls population_themes "the population panel", a foundational
// panel beside CameraControls — while this room's own banner said only
// "S2 · HEADER: vocabulary + state + decls". The two rooms agree now.
//
// WHY THE ENGINE WENT. It biased spawns two ways and both were bias on
// top of facts the panel should hold directly: a SPATIAL density per tile
// (authored off a 500-unit lattice) and a TEMPORAL flavour (a sequenced
// envelope with cooldowns). Neither was a dial anyone could turn; both
// were rolls under the world seed, layered over the per-family base
// chances that actually decide a population. The campaign's whole
// argument is that a distribution the panel owns beats a roll the seed
// hides, and the theme engine was the largest hidden roll in the tree.
//
// WHAT IS TO LIVE HERE. The five families' spawn dials and the global
// density, enrolled as the panel's own rows. GLOBAL_ENTITY_DENSITY moves
// in with this commit, its first resident; the per-family base chances
// still sit on their families' config structs and are NEW enrollment,
// which is U6's by ruling. The panel is an INSTITUTION, not a struct: it
// is a home with an address, and it fills up as the campaign hands it
// facts.
//
// NOT THE AGENTS' BANK. Two facts wore the name "POP_LIVE" in the
// handoff and the ruling split them: the AGENT population bank (count,
// behaviour and tier weights) is AGENTS_LIVE at
// contracts/agent_surface.hpp. This is the ENTITY surface's panel. They
// were never the same struct.

#include <cstdint>

namespace t7 {
namespace the_board {

// THE GLOBAL DENSITY — the one term of the composition law that outlived
// the mood and the tile terms. `compose_spawn_chance` reads it as the whole of
// the world's say over a family's base chance:
//     chance = base_chance x GLOBAL_ENTITY_DENSITY, clamped.
// It lived at contracts/spawn_services.hpp beside the law that read it;
// it lives here now, beside the dials it will be turned with. 1.0 is
// neutral, and neutral is what it has always been.
inline constexpr float GLOBAL_ENTITY_DENSITY = 1.0f;

} // namespace the_board
} // namespace t7
