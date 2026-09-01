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

// THE WORLD-DRAW BANK DIED WHOLE (ONE_WORLD-II U4). WorldDrawSurface
// held four scheme weights — Cathedral / Quartet / Gallery / Sanctum —
// that the seed walked to pick an indoor light scheme, and its ONLY
// declared reader was derive_indoor_lights. Bank, design table, sizing
// constant and its four organ rows left together: rows whose author is
// gone are the Amendment D failure, and "the mood/scheme facts" of it
// underpriced what was actually a whole bank with one reader.

// PORTAL_1's ONE DERIVATION stood here — portal_color_for, which read a
// door's colour off its DESTINATION so the fact was derived and never
// stored twice. Both its callers and its palette left with the doors
// (ONE_WORLD-I); an arch wears the colour its tier rolls, like every
// other family.

} // namespace the_board
} // namespace t7
