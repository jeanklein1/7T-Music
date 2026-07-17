#pragma once
#include <cstdint>
#include <cstddef>                                        // size_t (cap_to_ceiling's array-reference template)
#include "cartridges/the_board/contracts/roster.hpp"        // PopFamily — the table's axis (F-1 pinned)
#include "cartridges/the_board/contracts/entity_types.hpp"  // EntityInstance — cap_to_ceiling scales its params

// ─── indoor_module.hpp ───────────────────────────────────────────
// History: audit/LADDER.md
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
// Margin doubled (was 10) so that paintings and snapshots
// mounted on indoor walls are visibly separated from spawning
// entities — the entity's footprint now reads as distinct from
// the wall surface and any artwork on it.
inline constexpr float INDOOR_ENTITY_WALL_MARGIN  = 20.0f; // existing, re-homed here

// AXES: PopFamily order, PINNED by F-1. gol size is NATURAL here —
// its height cap is GPU-side (the zone mesh kernel), noted on the row.
inline constexpr IndoorTreatment INDOOR_TREATMENT[PopFamily::COUNT] = {
    /* pyramid */ { IndoorSize::CAP,     IndoorBounds::MARGIN },
    /* arch    */ { IndoorSize::CAP,     IndoorBounds::MARGIN },
    /* column  */ { IndoorSize::EXACT,   IndoorBounds::MARGIN },  // architectural: touches the ceiling
    /* antenna */ { IndoorSize::CAP,     IndoorBounds::MARGIN },
    /* palm    */ { IndoorSize::CAP,     IndoorBounds::MARGIN },
    /* cactus  */ { IndoorSize::NATURAL, IndoorBounds::MARGIN },  // Jean: keeps size
    /* blade   */ { IndoorSize::NATURAL, IndoorBounds::MARGIN },  // Jean: keeps size
    /* sphere  */ { IndoorSize::CAP,     IndoorBounds::MARGIN },
    /* ribbon  */ { IndoorSize::CAP,     IndoorBounds::FULL   },  // pre-scaled by RIBBON_INDOOR_SCALE; stays inside
    /* cube    */ { IndoorSize::CAP,     IndoorBounds::MARGIN },
    /* gol     */ { IndoorSize::NATURAL, IndoorBounds::FREE   },  // may straddle; height capped GPU-side
    /* gallery */ { IndoorSize::NATURAL, IndoorBounds::FULL   },  // sand-standing exhibits wholly inside
};

static_assert(PopFamily::PYRAMID == 0 && PopFamily::ARCH    == 1
           && PopFamily::COLUMN  == 2 && PopFamily::ANTENNA == 3
           && PopFamily::PALM    == 4 && PopFamily::CACTUS  == 5
           && PopFamily::BLADE   == 6 && PopFamily::SPHERE  == 7
           && PopFamily::RIBBON  == 8 && PopFamily::CUBE    == 9
           && PopFamily::GOL     == 10 && PopFamily::GALLERY == 11
           && PopFamily::COUNT   == 12,
    "INDOOR_TREATMENT rows ride F-1's PopFamily order — pyramid, arch, "
    "column, antenna, palm, cactus, blade, sphere, ribbon, cube, gol, "
    "gallery: re-row this table in lockstep before renumbering any family");

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

} // namespace the_board
} // namespace t7
