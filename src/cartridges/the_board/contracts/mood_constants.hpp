#pragma once
#include <cstdint>

// ─── mood_constants.hpp ──────────────────────────────────────────
//
// The mood-count constant, the Mood IDs and the world-draw bank at file
// scope: the config-bearing module headers size their per-mood tables by
// MOOD_COUNT and name the IDs declaration-side; every reference is
// unqualified and resolves here. The portal palette lived here too, with
// the destination it described, until the doors left (ONE_WORLD-I).
//
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

inline constexpr uint32_t MOOD_COUNT = 7;

// ─── Mood IDs ───────────────────────────────────────────────────
// Three outdoor worlds open (one sky each), one outdoor world walled,
// two rooms, and the atrium.
inline constexpr uint32_t MOOD_OPEN_SUNSET = 0;
inline constexpr uint32_t MOOD_INDOOR_FLAT = 1;
inline constexpr uint32_t MOOD_INDOOR_VAULT = 2;
inline constexpr uint32_t MOOD_FINITE_OUTDOOR = 3;
inline constexpr uint32_t MOOD_OPEN_NIGHT     = 4;  // ATMOS_1 — the open shape under a night sky
inline constexpr uint32_t MOOD_OPEN_NOON      = 5;  // ATMOS_1 — the open shape under a high sun
inline constexpr uint32_t MOOD_ATRIUM         = 6;  // ATRIUM_1 — the entrance: a small dark room,
                                                    // an arc of every door, the visitor's own figure
                                                    // walking through them. Boot mood; rare afterward.

// The names, positional by id (F-3's kin). mood_name() reads them, the
// registry emits them to the panel's mood select (organ_mood_names), and
// a new mood names itself here in the same commit as its id.
// Sized array: the compiler catches an EXTRA entry past MOOD_COUNT, but
// not a missing one — it zero-fills to nullptr.
inline constexpr const char* MOOD_NAMES[MOOD_COUNT] = {
    "open_sunset", "indoor_flat", "indoor_vault", "finite_outdoor",
    "open_night",  "open_noon",  "atrium",
};

// ═══ THE WORLD-DRAW BANK, GRADUATED (ORGAN_4 P3d) ════════════════
//
// C3 DESTRUCTIVE. The one surviving fact is read while a WORLD IS BEING
// DRAWN — the scheme roll as each indoor room derives its lights. Its
// three companions (the portal roll, the destination law, the portal
// palette) left with the doors at ONE_WORLD-I.
// It is not re-read afterwards, which is exactly why the bank has NO
// BOUNDARY WIRING: re-speaking a destructive author means tearing the
// world down to apply a slider (D5, and the 3b D4 law behind it — a
// wrong re-speak tears down a world). The edit lands at the author's own
// next natural event, and the enrollment rows say so with the GEN chip.
//
// WHY IT LIVES HERE. `SCHEME_WEIGHTS` was `inline constexpr` in
// direction/mood.hpp, which the ORGAN may not include. The bank keeps
// its shape and its question — what does a fresh world roll? — on the
// one axis a world still rolls.
//
// WORLD_DRAW_TABLE is the DESIGN, two jobs only: seeding the bank and
// standing under its assert. WORLD_DRAW_LIVE is what the rollers read.

inline constexpr uint32_t SCHEME_COUNT = 4;   // indoor light schemes; sizes
                                              // LIGHT_SCHEMES and SCHEME_NAMES
                                              // in direction/mood.hpp
// ATRIUM_13 — SCHEME_ATRIUM IS GONE, and the count is four again. It was a
// fifth row that said "four downlights, straight down, every sigma 0"; the
// entrance points at QUARTET now (A13.2) and nothing pointed at row 4 any
// more. An authored row with no author left is not annotated, it is deleted.
//
// THE ROW THE ENTRANCE POINTS AT NOW. A pinned index must name its row
// (ATRIUM_5's rule, and the reason the palette pin carried a string witness):
// the other three schemes are positional because nothing pins them, and this
// one is not.
inline constexpr uint32_t SCHEME_QUARTET = 1;  // four ceiling lamps at the quarter
                                               // points, aim drawn — the indoor default

struct WorldDrawSurface {
    float scheme_weights[SCHEME_COUNT];        // Cathedral / Quartet / Gallery / Sanctum
};

inline constexpr WorldDrawSurface WORLD_DRAW_TABLE = {
    { 0.42f, 0.43f, 0.10f, 0.05f },          // cathedral / quartet / gallery / sanctum (ATRIUM_13 — the
                                             // atrium's weight-0 fifth went with its row)
};

inline WorldDrawSurface WORLD_DRAW_LIVE = WORLD_DRAW_TABLE;
static_assert(sizeof(WorldDrawSurface) == SCHEME_COUNT * sizeof(float),
    "WORLD_DRAW_LIVE is a whole-struct copy of the design row: a field "
    "added to one is added to the other by construction");
static_assert(SCHEME_COUNT == 4,
    "WORLD_DRAW_TABLE's scheme row is POSITIONAL — a new scheme needs its "
    "column here, in the same commit");

// PORTAL_1's ONE DERIVATION stood here — portal_color_for, which read a
// door's colour off its DESTINATION so the fact was derived and never
// stored twice. Both its callers and its palette left with the doors
// (ONE_WORLD-I); an arch wears the colour its tier rolls, like every
// other family.

} // namespace the_board
} // namespace t7
