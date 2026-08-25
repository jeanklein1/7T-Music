#pragma once
#include <cstdint>
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
inline constexpr uint32_t ATRIUM_SAND_SPOTS = 3;

struct AtriumSurface {
    float arc_radius;        // wu from the arrival point to every door
    float arc_span_deg;      // the doors spread over this, centred on arc_bearing
    float arc_bearing_deg;   // 0 = dead ahead; the arc sits in the forward hemisphere
                             // so the first frame holds controls + doors + passers
    AtriumSandSpot sand[ATRIUM_SAND_SPOTS];   // [0] is the controls image, dead ahead
};
inline constexpr AtriumSurface ATRIUM_TABLE = {
    45.0f, 180.0f, 0.0f,
    { {  0.0f, 16.0f, 9.0f },     // ATRIUM_0 — the controls scheme, facing the arrival point
      { -35.0f, 22.0f, 7.0f },    // a hand photo, left
      {  35.0f, 22.0f, 7.0f } },  // a hand photo, right
};
inline AtriumSurface ATRIUM_LIVE = ATRIUM_TABLE;
static_assert(sizeof(AtriumSurface) == (3 + 3 * ATRIUM_SAND_SPOTS) * sizeof(float),
    "ATRIUM_LIVE is a whole-struct copy of the design row");
}}
