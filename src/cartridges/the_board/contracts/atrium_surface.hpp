#pragma once
#include <cstdint>
#include <cstddef>   // offsetof — the touched mask names its bits by field (ATRIUM_13)
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
    // ATRIUM_12 — THE ENTRANCE'S OWN WALL AND ROOF. Every other room draws
    // INDOOR_PALETTES by index — pinned or rolled — and an index is a row you
    // PICK. The entrance authors its pair here, beside its arc and its poster,
    // because this file is where the room's authored facts live and because a
    // contract is what the ORGAN may include.
    //
    // IndoorPalette carries exactly two colour fields (its third member is the
    // NAME, which is the console line's and never reaches the GPU), so these
    // two are the whole palette and not a partial one. generate_indoor_shell
    // reads wall_color on the four walls and ceiling_color on the flat lid and
    // the vault crown, and they reach the GPU as ShellVertex colour.
    float wall_color[3];
    float ceiling_color[3];
};
inline constexpr AtriumSurface ATRIUM_TABLE = {
    38.0f, 180.0f, 0.0f, 40.0f,
    // THE POSTER'S TWO DIALS MOVE TOGETHER. Apparent size is height/distance;
    // Jean gated 1.2 at (10, 12) and asked for distance without losing it, so
    // (15, 18) holds 1.2 exactly. THE CEILING IS THE STOP: the quad stands on
    // the sand, so height must clear SHAPE_ATRIUM's wall_height (20) with
    // room to spare — 18 leaves 2. Past distance ~15 the poster either
    // shrinks in the frame or the room's wall has to grow; there is no third
    // dial (ATRIUM_8). The headroom witness prints all four numbers at the
    // hang.
    { { 0.0f, 15.0f, 18.0f } },   // ATRIUM_0 — the controls scheme, back at fifteen
    // THE RESTS ARE INDOOR_PALETTES[7] "warm charcoal" WRITTEN OUT, so this
    // commit changes nothing on screen: the seam opens and Jean composes.
    { 0.40f, 0.38f, 0.36f },   // wall
    { 0.32f, 0.30f, 0.28f },   // ceiling
};
inline AtriumSurface ATRIUM_LIVE = ATRIUM_TABLE;

// ═══ THE ENTRANCE'S TOUCHED MASK (ATRIUM_13) ═════════════════════
// The rows here are GEN — read when a world is BUILT — so a dial turned
// mid-world wrote a value nothing would look at again until the next entry,
// and the desk read that as "the dial does not work". organ_set raises
// organ_mark_dirty(12), which sets a bit in organTouched_ that GPUState's own
// flush has no arm for; nothing else happened at all.
//
// The orb bank's idiom answers it exactly: A BIT IS AN OFFSET / 4 into
// AtriumSurface, the flag says the bank moved, the mask says where, and the
// boundary redoes only what that field feeds. Nothing debounced, nothing
// polled — the mask coalesces a drag into one raise, the way a slider drag is
// one WriteBuffer.
//
// THREE CONSEQUENCES, and every word belongs to exactly one:
//   COLOUR  -> the shell is regenerated (walls + ceiling), the hang untouched
//   SAND    -> the poster is reseated in place
//   ARC     -> refused, and the witness says so (ATRIUM_13 R6: no release verb
//              exists for the arc's arches, and one is not CC's to write)
//
// THE LAMPS ARE NOT A FOURTH. The entrance points at a scheme and draws its
// lamps as any indoor room does (A13.2, Jean's amendment), so it authors
// nothing about them and has nothing here to route. A word in two sets would
// be a word with two homes; the asserts below prove the three are pairwise
// disjoint AND total.
inline uint32_t g_atrium_touched = 0;
inline uint32_t take_atrium_touched() {
    const uint32_t m = g_atrium_touched;
    g_atrium_touched = 0;
    return m;
}

// THE BITS NAME THEIR FIELDS, never a literal: a column inserted above moves
// every offset below it, and offsetof follows while a number would not.
inline constexpr uint32_t ATRIUM_BIT(uint32_t off)  { return 1u << (off / 4u); }
inline constexpr uint32_t ATRIUM_BIT3(uint32_t off) {
    return ATRIUM_BIT(off) | ATRIUM_BIT(off + 4u) | ATRIUM_BIT(off + 8u);
}
inline constexpr uint32_t ATRIUM_ARC_BITS =
      ATRIUM_BIT(offsetof(AtriumSurface, arc_radius))
    | ATRIUM_BIT(offsetof(AtriumSurface, arc_span_deg))
    | ATRIUM_BIT(offsetof(AtriumSurface, arc_bearing_deg))
    | ATRIUM_BIT(offsetof(AtriumSurface, arc_center_offset));
// sand[i] is three words each — bearing, distance, height — and
// ATRIUM_SAND_SPOTS is 1 today; the fold is written so a second spot joins the
// set by itself.
inline constexpr uint32_t atrium_sand_bits() {
    uint32_t m = 0;
    for (uint32_t i = 0; i < ATRIUM_SAND_SPOTS; i++)
        m |= ATRIUM_BIT3(offsetof(AtriumSurface, sand) + i * sizeof(AtriumSandSpot));
    return m;
}
inline constexpr uint32_t ATRIUM_SAND_BITS   = atrium_sand_bits();
inline constexpr uint32_t ATRIUM_COLOUR_BITS =
      ATRIUM_BIT3(offsetof(AtriumSurface, wall_color))
    | ATRIUM_BIT3(offsetof(AtriumSurface, ceiling_color));

static_assert(sizeof(AtriumSurface) / 4u <= 32u,
    "the touched mask is a uint32: every field's offset/4 must be a bit it can hold");
static_assert((ATRIUM_COLOUR_BITS & ATRIUM_SAND_BITS) == 0
           && (ATRIUM_COLOUR_BITS & ATRIUM_ARC_BITS)  == 0
           && (ATRIUM_SAND_BITS   & ATRIUM_ARC_BITS)  == 0
           && (ATRIUM_COLOUR_BITS | ATRIUM_SAND_BITS | ATRIUM_ARC_BITS)
              == ((1u << (sizeof(AtriumSurface) / 4u)) - 1u),
    "every word of AtriumSurface belongs to exactly one consequence (ATRIUM_13)");

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
static_assert(sizeof(AtriumSurface) == (4 + 3 * ATRIUM_SAND_SPOTS + 6) * sizeof(float),
    "ATRIUM_LIVE is a whole-struct copy of the design row");
}}
