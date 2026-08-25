#pragma once
#include <cstdint>
#include "cartridges/the_board/contracts/mood_constants.hpp"   // MOOD_COUNT + the Mood IDs — the arc roster is per mood (ATRIUM_7)
// ─── atrium_surface.hpp — THE ATRIUM'S COMPOSITION (ATRIUM_2) ─────
//
// The contract tier, because the ORGAN may not include a direction file
// and this bank is enrolled. C3 DESTRUCTIVE, GEN chip: every row is read
// while the world is DRAWN — the arc as the doors spawn, the sand spots
// as the images hang — and re-read only at the next world change. A
// panel edit lands at the author's own next natural event, which is
// exactly what the GEN chip says.
//
// Everything here is in the ARRIVAL FRAME: origin = Idle::PAWN_POS,
// bearing 0 = the pawn's forward at Idle::PAWN_HEADING
// (heading_to_bearing, realization/state.hpp). Degrees for the hand;
// radians are computed at the read.
namespace t7 { namespace the_board {

struct AtriumSandSpot { float bearing_deg; float distance; float height; };
// ATRIUM_5 — ONE IMAGE ON THE SAND. Three spots put two hand photos beside
// the controls scheme and the entrance read as a gallery; the entrance is
// not a gallery. The folder's first image, dead ahead, facing the pawn —
// and the walls take the rest, as they always did.
inline constexpr uint32_t ATRIUM_SAND_SPOTS = 1;

struct AtriumSurface {
    float arc_radius;        // wu from the arrival point to every door
    float arc_span_deg;      // the doors spread over this, centred on arc_bearing
    float arc_bearing_deg;   // 0 = dead ahead; the arc sits in the forward hemisphere
                             // so the first frame holds controls + doors + passers
    float arc_center_offset; // ATRIUM_5 — wu from the arrival point, along the gaze,
                             // to the arc's CENTRE: the pawn stands BEHIND the chord,
                             // near the wall behind it; the doors face this centre,
                             // not the pawn (Jean)
    AtriumSandSpot sand[ATRIUM_SAND_SPOTS];   // [0] is the controls image, dead ahead
};
inline constexpr AtriumSurface ATRIUM_TABLE = {
    38.0f, 180.0f, 0.0f, 40.0f,
    { { 0.0f, 10.0f, 12.0f } },   // ATRIUM_0 — the controls scheme: twelve units tall at ten
                                  //            units, so the audience reads it (ATRIUM_6, Jean's dial)
};
inline AtriumSurface ATRIUM_LIVE = ATRIUM_TABLE;

// WHO IS ON THE ENTRANCE'S ARC — the one home (ATRIUM_7). A mood off the
// arc stays reachable by weight from the open field; the atrium offers the
// worlds a newcomer can read from a doorway. Finite outdoor is a field with
// a wall — off the arc by Jean's ruling. The atrium itself never offers
// itself. NOT a dial: the roster is a fact about what the entrance says,
// which is a decision, not a taste.
inline constexpr bool ATRIUM_ARC_DOOR[MOOD_COUNT] = {
    /* open_sunset    */ true,
    /* indoor_flat    */ true,
    /* indoor_vault   */ true,
    /* finite_outdoor */ false,
    /* open_night     */ true,
    /* open_noon      */ true,
    /* atrium         */ false,
};
inline constexpr uint32_t atrium_arc_door_count() {
    uint32_t n = 0;
    for (uint32_t m = 0; m < MOOD_COUNT; m++) n += ATRIUM_ARC_DOOR[m] ? 1u : 0u;
    return n;
}
static_assert(!ATRIUM_ARC_DOOR[MOOD_ATRIUM], "the atrium never offers itself");
static_assert(atrium_arc_door_count() >= 2,
    "an arc is at least two doors — the passers need an end to walk to");
static_assert(sizeof(AtriumSurface) == (4 + 3 * ATRIUM_SAND_SPOTS) * sizeof(float),
    "ATRIUM_LIVE is a whole-struct copy of the design row");
}}
