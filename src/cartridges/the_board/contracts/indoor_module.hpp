#pragma once
#include <cstdint>
#include <cstddef>                                        // size_t (cap_to_ceiling's array-reference template)
#include "cartridges/the_board/contracts/roster.hpp"        // PopFamily — the table's axis (F-1 pinned)
#include "cartridges/the_board/contracts/entity_types.hpp"  // EntityInstance — cap_to_ceiling scales its params

// ─── indoor_module.hpp ───────────────────────────────────────────
//
// ═══ THE INDOOR MODULE — mood's insert on the spawn chain ═══════
// One table + three dials govern the whole indoor treatment.
// Families keep only their param lists; the module modulates.
//
// HOME: this is mood policy, but its consumers (grounded.hpp,
// spheres.hpp, cube_behaviors.hpp, ribbon.hpp) precede mood.hpp in
// the cohort — so the TYPE + TABLE + dials ride the contracts tier
// (the second-consumer law) and the mood panel keeps banner +
// pointer (direction/mood.hpp, INDOOR ENTITY PLACEMENT).
//
// SIZES (Jean's law): indoors, everything exists at outdoor
// distribution. NATURAL keeps outdoor size; EXACT snaps to the
// ceiling (column — architectural); CAP scales down to the fraction
// of the ceiling only when taller — cap-only, nothing ever inflates.
// Ribbon additionally pre-scales by RIBBON_INDOOR_SCALE before its
// cap. BOUNDS go live at the margin site (spawn_engine's indoor
// clamp): MARGIN = the standing wall-margin clamp; FULL = wholly
// inside (margin + the family's containment extent); FREE = may
// straddle walls (no clamp).

namespace t7 {
namespace the_board {

enum class IndoorSize   : uint32_t { NATURAL, CAP, EXACT };
enum class IndoorBounds : uint32_t { MARGIN, FULL, FREE };
struct IndoorTreatment { IndoorSize size; IndoorBounds bounds; };

inline constexpr float INDOOR_HEIGHT_CAP_FRACTION = 0.75f; // Jean's law
inline constexpr float RIBBON_INDOOR_SCALE        = 0.15f; // Jean's dial — tune on sight

// Clamp (not reject) is intentional: rejection-based logic
// would silently drop entities anchored to corner patches,
// because their seed-determined position keeps landing in the
// wall margin and never recovers. Clamping shifts the candidate
// to the legal-box edge and lets the existing footprint-overlap
// check handle any pile-ups that result.
// Margin doubled (was 10) so that what was mounted on indoor
// walls read as visibly separated from spawning entities. The
// wall art left at PRUNE_1; the margin is KEPT at its measured
// value — the separation it buys between a body and a bare wall
// is a composition Jean has seen, and re-tuning it is a taste
// gate, not a prune's side effect.
inline constexpr float INDOOR_ENTITY_WALL_MARGIN  = 20.0f; // existing, re-homed here

// The ceiling-fit floor (COLUMN CEILING FIT): indoors the cmg kernel
// derives effective column height = ceiling_height − ground_y(center)
// — the capital sits flush with the ceiling plane across topography —
// but never below this floor. PINNED PAIR: world.wgsl declares the
// same value as const COLUMN_MIN_INDOOR_HEIGHT beside the cmg kernel
// bindings (authored in both rooms, named the same, never
// dial-derived — the TILE_GRID_CAPACITY pattern).
inline constexpr float COLUMN_MIN_INDOOR_HEIGHT   = 1.0f;  // extreme-terrain floor: a column never collapses below this

// AXES: PopFamily order, PINNED by F-1. gol size is NATURAL here —
// its height cap is GPU-side (zone_derive_params, once per zone birth),
// noted on the row.
inline constexpr IndoorTreatment INDOOR_TREATMENT[PopFamily::COUNT] = {
    /* pyramid */ { IndoorSize::CAP,     IndoorBounds::MARGIN },
    /* arch    */ { IndoorSize::CAP,     IndoorBounds::MARGIN },
    /* column  */ { IndoorSize::EXACT,   IndoorBounds::MARGIN },  // architectural: touches the ceiling
    /* antenna */ { IndoorSize::CAP,     IndoorBounds::MARGIN },
    /* sphere  */ { IndoorSize::CAP,     IndoorBounds::MARGIN },
    /* ribbon  */ { IndoorSize::CAP,     IndoorBounds::FULL   },  // pre-scaled by RIBBON_INDOOR_SCALE; stays inside
    /* cube    */ { IndoorSize::CAP,     IndoorBounds::MARGIN },
    /* gol     */ { IndoorSize::NATURAL, IndoorBounds::FREE   },  // may straddle; lift capped at derive
};

static_assert(PopFamily::PYRAMID == 0 && PopFamily::ARCH    == 1
           && PopFamily::COLUMN  == 2 && PopFamily::ANTENNA == 3
           && PopFamily::SPHERE  == 4 && PopFamily::RIBBON  == 5
           && PopFamily::CUBE    == 6 && PopFamily::GOL     == 7
           && PopFamily::COUNT   == 8,
    "INDOOR_TREATMENT rows ride F-1's PopFamily order — pyramid, arch, "
    "column, antenna, sphere, ribbon, cube, "
    "gol: re-row this table in lockstep before renumbering any family");

// ─── THE CAP LAW (shared) ────────────────────────────────────────
// If the family's current vertical extent already fits under
// cap_fraction × ceiling, nothing happens — nothing ever inflates.
// Otherwise every listed param scales by the one ratio: miniatures,
// not squashes. The per-family param-index lists (which indices are
// LENGTHS — never ratios, counts, angles, or rates) stay with the
// families; the module holds only the law.
template<size_t N>
inline void cap_to_ceiling(EntityInstance& inst, float ceiling_h,
    float cap_fraction, float current_h,
    const uint32_t (&params_to_scale)[N]) {
    if (current_h <= 1e-3f) return;
    const float cap_h = cap_fraction * ceiling_h;
    if (current_h <= cap_h) return;
    const float scale = cap_h / current_h;
    for (size_t i = 0; i < N; i++) inst.params[params_to_scale[i]] *= scale;
}


// ═══ THE LIVE SURFACE (ORGAN_3 w3, C3 — DESTRUCTIVE) ═════════════
//
// THE TEMPERAMENT LAW, and this bank is its exemplar. An IDEMPOTENT
// author re-speaks at the frame boundary when its bank changes (the
// mood apply, the registry upload — ORGAN_2b's pattern). A DESTRUCTIVE
// author does NOT: re-speaking it would tear down live state, so its
// definitions take effect at the author's next natural event, exactly
// as a non-live mood's definition already waits.
//
// INDOOR_HEIGHT_CAP_FRACTION has BOTH temperaments — apply_mood_lighting
// reads it (idempotent) and every entity spawn's cap_to_ceiling reads it
// (destructive: entity_pipeline ×3, grounded ×3, spheres, cube_behaviors,
// ribbon). The STRICTER temperament governs, so there is no boundary
// wiring here at all: the mood half picks the edit up at the next mood
// change, the spawn half at the next spawn. Neither needs a flag.
//
// A DIAL THAT EDITS THE FUTURE MUST SAY IT EDITS THE FUTURE — so the
// enrollment group is labelled "edits the next spawn", and that label is
// the whole interface contract. Turning it and watching nothing happen
// is the dial working.

struct IndoorSurface {
    float height_cap_fraction;   // × wall_height — the indoor lift ceiling
    float ribbon_scale;          // the ribbon's indoor diminishment
};

inline constexpr IndoorSurface INDOOR_TABLE = {
    INDOOR_HEIGHT_CAP_FRACTION,
    RIBBON_INDOOR_SCALE,
};

inline IndoorSurface INDOOR_LIVE = INDOOR_TABLE;
static_assert(INDOOR_TABLE.height_cap_fraction == INDOOR_HEIGHT_CAP_FRACTION
           && INDOOR_TABLE.ribbon_scale == RIBBON_INDOOR_SCALE,
    "INDOOR_TABLE is seeded FROM the authored dials above — one fact, one "
    "home; if they can disagree the seeding is wrong");

} // namespace the_board
} // namespace t7
