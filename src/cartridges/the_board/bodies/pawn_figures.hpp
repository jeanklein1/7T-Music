#pragma once
#include <cstdint>

// ═══════════════════════════════════════════════════════════════════════════
// THE PAWN FIGURES REGISTRY — the design surface for pawn skins ("figures").
// One row per figure; flat matrices; Pawn (1.5/0.5) is the reference scale.
//
// TWO PROFILE IDIOMS, kept in each family's natural form (ratified in-chat):
//   FAM_REGULAR  — the current pawn. Uses the hardcoded world.wgsl
//                  pawn_profile_radius + the legacy per-agent/tier color path.
//                  Its table row carries scale only; profile/palette are null.
//   FAM_SMOOTH   — parametric profile-of-revolution (SmoothProfile, 16 fields),
//                  the designer's pawnRadius(t,P) form.
//   FAM_HERALDIC — segment list (HeraldicProfile, ≤7 segs), the stacked/heraldic
//                  radiusAt() form, each segment shaped by a PawnShape.
//
// Color: figures 1..13 carry a height-gradient palette + a bounded HSV drift
//        envelope (seeded per agent from AgentState.seed). Figure 0 (regular)
//        keeps the existing color logic and takes no palette (drift = 0).
//
// Data transcribed verbatim from 7t_pawn_designer.jsx (Smooth *Profile()/
// *Stops(); Heraldic *Geom()/*Stops()). Source of truth for H2 (GPU pack) and
// H3 (WGSL mirror). Edit numbers HERE; the pipeline flows down.
// ═══════════════════════════════════════════════════════════════════════════

namespace t7 {
namespace the_board {

// ── Axes ────────────────────────────────────────────────────────────────────
enum PawnFamilyId : uint32_t {
    FAM_REGULAR  = 0,
    FAM_SMOOTH   = 1,
    FAM_HERALDIC = 2,
    FAM_COUNT    = 3,
};

// Segment shape vocabulary — MUST match the WGSL profile_shape() switch (H3)
// and the designer's STACKED_SHAPES keys.
enum PawnShape : uint32_t {
    SHAPE_LINEAR  = 0,
    SHAPE_CONCAVE = 1,
    SHAPE_CONVEX  = 2,
    SHAPE_STEP    = 3,
    SHAPE_BELL    = 4,
    SHAPE_FLARE   = 5,
    SHAPE_OGEE    = 6,
    SHAPE_SPHERE  = 7,
    SHAPE_COUNT   = 8,
};

// ═══ DISTRIBUTION — the tweak surface ═══════════════════════════════════════
// Percent of the population per family; equal within a family. A rarity slope
// later = replace this with per-figure weights (one column), no code change.
struct PawnFamilyShare { PawnFamilyId family; const char* name; float share_pct; };

inline constexpr PawnFamilyShare FIGURE_SHARES[FAM_COUNT] = {
    //  family        name        share   (members → each)
    { FAM_REGULAR,  "regular",  70.0f },  // 1 → 70.00%
    { FAM_SMOOTH,   "smooth",   15.0f },  // 6 →  2.50%
    { FAM_HERALDIC, "heraldic", 15.0f },  // 7 →  2.14%
};

// ═══ SMOOTH PROFILES — parametric (16 fields), mirrors pawnRadius(t,P) ═══════
struct SmoothProfile {
    float start_r, flare_r, flare_t;         // base → flare
    float peak_r, peak_t;                     // peak
    float base_t, body_start_r;               // body onset
    float body_t, waist_r;                    // waist
    float neck_t, neck_r;                     // neck
    float collar_t, collar_bulge;             // collar
    float head_t, head_base_r, head_sphere_r; // head (hemisphere) + tip to 0
};

//                                         start flare flr_t  peak pk_t  base body_r  body waist  neck nk_r  coll bulge  head hd_r  sph_r
inline constexpr SmoothProfile PROF_PAWN     = { 0.75f,0.88f,0.04f, 0.92f,0.10f, 0.14f,0.70f, 0.50f,0.22f, 0.62f,0.18f, 0.70f,0.08f, 0.95f,0.15f,0.35f }; // reference (regular pawn uses hardcoded path)
inline constexpr SmoothProfile PROF_SQUAT    = { 0.85f,0.90f,0.05f, 1.05f,0.12f, 0.18f,0.85f, 0.58f,0.25f, 0.60f,0.40f, 0.67f,0.24f, 0.93f,0.30f,0.20f };
inline constexpr SmoothProfile PROF_COLOSSAL = { 0.78f,0.92f,0.05f, 0.98f,0.12f, 0.16f,0.72f, 0.48f,0.16f, 0.60f,0.14f, 0.68f,0.13f, 0.96f,0.18f,0.42f };
inline constexpr SmoothProfile PROF_ACORN    = { 0.50f,0.90f,0.05f, 0.80f,0.18f, 0.28f,0.50f, 0.50f,0.35f, 0.65f,0.30f, 0.70f,0.25f, 0.93f,0.25f,0.20f };
inline constexpr SmoothProfile PROF_SPIRE    = { 0.50f,0.55f,0.05f, 0.60f,0.15f, 0.20f,0.50f, 0.65f,0.30f, 0.75f,0.20f, 0.82f,0.05f, 0.92f,0.18f,0.15f };
inline constexpr SmoothProfile PROF_IDOL     = { 0.50f,0.62f,0.05f, 0.68f,0.10f, 0.16f,0.55f, 0.45f,0.20f, 0.55f,0.20f, 0.66f,0.30f, 0.92f,0.10f,0.20f };
inline constexpr SmoothProfile PROF_STELE    = { 0.78f,0.82f,0.04f, 0.85f,0.10f, 0.14f,0.78f, 0.60f,0.55f, 0.72f,0.38f, 0.80f,0.03f, 0.95f,0.35f,0.20f };

// ═══ HERALDIC PROFILES — segment lists (≤7), mirrors heraldicRadiusAt() ══════
struct HeraldicSeg     { float height, r_bot, r_top; PawnShape shape; };
struct HeraldicProfile { uint32_t seg_count; HeraldicSeg seg[7]; };  // unused segs zeroed

inline constexpr HeraldicProfile PROF_H_BRONZE = { 5, {
    { 0.04f, 1.00f, 0.92f, SHAPE_CONVEX  },   // foot moulding
    { 0.10f, 0.92f, 0.55f, SHAPE_CONVEX  },   // base
    { 0.34f, 0.50f, 0.30f, SHAPE_CONCAVE },   // shank (pinched)
    { 0.04f, 0.30f, 0.42f, SHAPE_CONVEX  },   // collar disc
    { 0.40f, 0.00f, 0.40f, SHAPE_SPHERE  },   // head sphere
    {}, {},
}};
inline constexpr HeraldicProfile PROF_H_SILVER = { 5, {
    { 0.04f, 1.00f, 0.92f, SHAPE_CONVEX },
    { 0.10f, 0.92f, 0.55f, SHAPE_CONVEX },
    { 0.38f, 0.50f, 0.28f, SHAPE_OGEE   },    // refined shank
    { 0.04f, 0.28f, 0.42f, SHAPE_CONVEX },
    { 0.40f, 0.00f, 0.40f, SHAPE_SPHERE },
    {}, {},
}};
inline constexpr HeraldicProfile PROF_H_GOLD = { 6, {
    { 0.04f, 1.00f, 0.92f, SHAPE_CONVEX },
    { 0.10f, 0.92f, 0.55f, SHAPE_CONVEX },
    { 0.36f, 0.50f, 0.26f, SHAPE_OGEE   },
    { 0.04f, 0.26f, 0.26f, SHAPE_BELL   },    // bead
    { 0.04f, 0.26f, 0.42f, SHAPE_CONVEX },
    { 0.42f, 0.00f, 0.42f, SHAPE_SPHERE },
    {},
}};
inline constexpr HeraldicProfile PROF_H_STEEL = { 6, {
    { 0.04f, 1.05f, 0.95f, SHAPE_CONVEX },    // wider foot
    { 0.10f, 0.95f, 0.58f, SHAPE_CONVEX },
    { 0.42f, 0.52f, 0.24f, SHAPE_OGEE   },    // longer shank
    { 0.05f, 0.24f, 0.24f, SHAPE_BELL   },    // thicker bead
    { 0.04f, 0.24f, 0.42f, SHAPE_CONVEX },
    { 0.42f, 0.00f, 0.42f, SHAPE_SPHERE },
    {},
}};
inline constexpr HeraldicProfile PROF_H_CRYSTAL = { 7, {
    { 0.04f, 1.05f, 0.95f, SHAPE_CONVEX },
    { 0.10f, 0.95f, 0.58f, SHAPE_CONVEX },
    { 0.42f, 0.52f, 0.24f, SHAPE_OGEE   },
    { 0.05f, 0.24f, 0.24f, SHAPE_BELL   },
    { 0.04f, 0.24f, 0.42f, SHAPE_CONVEX },
    { 0.40f, 0.00f, 0.40f, SHAPE_SPHERE },
    { 0.06f, 0.00f, 0.10f, SHAPE_SPHERE },    // finial bud
}};
inline constexpr HeraldicProfile PROF_H_STAR = { 7, {
    { 0.04f, 1.00f, 0.90f, SHAPE_CONVEX },
    { 0.09f, 0.90f, 0.52f, SHAPE_CONVEX },
    { 0.48f, 0.48f, 0.20f, SHAPE_OGEE   },    // even longer shank
    { 0.04f, 0.20f, 0.20f, SHAPE_BELL   },    // small refined bead
    { 0.03f, 0.20f, 0.36f, SHAPE_CONVEX },
    { 0.36f, 0.00f, 0.36f, SHAPE_SPHERE },    // smaller head
    { 0.06f, 0.00f, 0.08f, SHAPE_SPHERE },
}};
inline constexpr HeraldicProfile PROF_H_DIVINE = { 7, {
    { 0.04f, 1.00f, 0.88f, SHAPE_CONVEX },
    { 0.10f, 0.88f, 0.50f, SHAPE_CONVEX },
    { 0.50f, 0.46f, 0.18f, SHAPE_OGEE   },
    { 0.07f, 0.20f, 0.20f, SHAPE_BELL   },    // doubled-band bead
    { 0.03f, 0.20f, 0.38f, SHAPE_CONVEX },
    { 0.38f, 0.00f, 0.38f, SHAPE_SPHERE },
    { 0.07f, 0.00f, 0.10f, SHAPE_SPHERE },    // tapered bud
}};

// ═══ PALETTES — height gradients (foot→head). t < 0 ends the list. ══════════
struct PawnStop { float t, r, g, b; };

inline constexpr PawnStop PAL_LAVENDER[] = {   // regular pawn reference (unused — legacy color path)
    {0.00f,0.80f,0.50f,0.80f},{0.14f,0.80f,0.50f,0.80f},{0.50f,0.80f,0.50f,0.80f},
    {0.62f,0.80f,0.50f,0.80f},{0.70f,0.80f,0.50f,0.80f},{0.95f,0.80f,0.50f,0.80f},{-1,0,0,0},
};
inline constexpr PawnStop PAL_BRONZE[] = {     // Squat
    {0.00f,0.55f,0.42f,0.30f},{0.18f,0.68f,0.55f,0.40f},{0.55f,0.78f,0.66f,0.50f},
    {0.65f,0.82f,0.71f,0.55f},{0.72f,0.85f,0.74f,0.58f},{0.92f,0.92f,0.84f,0.68f},{-1,0,0,0},
};
inline constexpr PawnStop PAL_BLACKGOLD[] = {  // Colossal
    {0.00f,0.10f,0.09f,0.09f},{0.16f,0.13f,0.11f,0.11f},{0.48f,0.18f,0.15f,0.13f},
    {0.60f,0.55f,0.42f,0.16f},{0.68f,0.78f,0.62f,0.22f},{0.96f,0.92f,0.80f,0.38f},{-1,0,0,0},
};
inline constexpr PawnStop PAL_ACORN[] = {      // Acorn (non-monotonic: brightest at collar, darkens to cap)
    {0.00f,0.30f,0.18f,0.08f},{0.28f,0.48f,0.30f,0.16f},{0.55f,0.55f,0.36f,0.20f},
    {0.65f,0.62f,0.42f,0.24f},{0.72f,0.42f,0.28f,0.14f},{0.88f,0.22f,0.13f,0.06f},{-1,0,0,0},
};
inline constexpr PawnStop PAL_STEELBLUE[] = {  // Spire
    {0.00f,0.18f,0.20f,0.25f},{0.20f,0.30f,0.35f,0.42f},{0.65f,0.45f,0.52f,0.60f},
    {0.75f,0.55f,0.62f,0.70f},{0.82f,0.62f,0.70f,0.78f},{0.92f,0.78f,0.85f,0.92f},{-1,0,0,0},
};
inline constexpr PawnStop PAL_JADE[] = {       // Idol
    {0.00f,0.10f,0.22f,0.20f},{0.16f,0.15f,0.32f,0.28f},{0.45f,0.20f,0.45f,0.40f},
    {0.55f,0.30f,0.55f,0.50f},{0.66f,0.45f,0.70f,0.62f},{0.92f,0.65f,0.85f,0.75f},{-1,0,0,0},
};
inline constexpr PawnStop PAL_LIMESTONE[] = {  // Stele
    {0.00f,0.55f,0.52f,0.48f},{0.14f,0.62f,0.60f,0.55f},{0.60f,0.72f,0.70f,0.66f},
    {0.72f,0.78f,0.76f,0.72f},{0.80f,0.82f,0.80f,0.76f},{0.95f,0.88f,0.86f,0.82f},{-1,0,0,0},
};
inline constexpr PawnStop PAL_H_BRONZE[] = {
    {0.00f,0.32f,0.20f,0.10f},{0.06f,0.45f,0.28f,0.14f},{0.20f,0.55f,0.36f,0.18f},
    {0.55f,0.65f,0.42f,0.22f},{1.00f,0.78f,0.55f,0.28f},{-1,0,0,0},
};
inline constexpr PawnStop PAL_H_SILVER[] = {
    {0.00f,0.55f,0.55f,0.58f},{0.06f,0.68f,0.68f,0.70f},{0.20f,0.78f,0.78f,0.80f},
    {0.55f,0.85f,0.85f,0.86f},{1.00f,0.92f,0.92f,0.94f},{-1,0,0,0},
};
inline constexpr PawnStop PAL_H_GOLD[] = {
    {0.00f,0.42f,0.30f,0.10f},{0.06f,0.62f,0.45f,0.16f},{0.20f,0.78f,0.58f,0.20f},
    {0.50f,0.92f,0.72f,0.28f},{0.58f,0.85f,0.65f,0.24f},{1.00f,0.98f,0.82f,0.38f},{-1,0,0,0},
};
inline constexpr PawnStop PAL_H_STEEL[] = {
    {0.00f,0.22f,0.26f,0.32f},{0.06f,0.35f,0.42f,0.50f},{0.20f,0.50f,0.58f,0.65f},
    {0.55f,0.62f,0.70f,0.75f},{0.62f,0.58f,0.66f,0.72f},{1.00f,0.78f,0.85f,0.90f},{-1,0,0,0},
};
inline constexpr PawnStop PAL_H_CRYSTAL[] = {
    {0.00f,0.30f,0.50f,0.55f},{0.06f,0.45f,0.65f,0.70f},{0.20f,0.60f,0.78f,0.82f},
    {0.55f,0.72f,0.86f,0.88f},{0.62f,0.78f,0.90f,0.92f},{0.92f,0.88f,0.96f,0.98f},{1.00f,0.98f,1.00f,1.00f},{-1,0,0,0},
};
inline constexpr PawnStop PAL_H_STAR[] = {
    {0.00f,0.62f,0.60f,0.72f},{0.06f,0.78f,0.75f,0.85f},{0.20f,0.88f,0.85f,0.92f},
    {0.62f,0.92f,0.88f,0.95f},{0.68f,0.95f,0.92f,0.96f},{0.92f,0.98f,0.96f,1.00f},{1.00f,1.00f,1.00f,1.00f},{-1,0,0,0},
};
inline constexpr PawnStop PAL_H_DIVINE[] = {
    {0.00f,0.85f,0.78f,0.55f},{0.06f,0.92f,0.85f,0.60f},{0.20f,0.96f,0.90f,0.65f},
    {0.62f,1.00f,0.95f,0.70f},{0.70f,1.00f,0.96f,0.75f},{0.92f,1.00f,0.98f,0.85f},{1.00f,1.00f,1.00f,0.95f},{-1,0,0,0},
};

// ═══ COLOR DRIFT — bounded per-agent HSV travel (seed-driven) ════════════════
// Each figure moves only where its design tolerates it. Regular pawn = 0 (legacy).
struct PawnDrift { float hue_deg, sat, val; };

// ═══ THE MASTER MATRIX ══════════════════════════════════════════════════════
// smooth / heraldic / palette are null where not applicable to the family.
struct PawnFigureDef {
    PawnFamilyId            family;
    const char*             name;
    float                   height, radius;   // world units; Pawn = 1.5/0.5 reference
    const SmoothProfile*    smooth;           // set iff FAM_SMOOTH
    const HeraldicProfile*  heraldic;         // set iff FAM_HERALDIC
    const PawnStop*         palette;          // null for FAM_REGULAR (legacy color)
    PawnDrift               drift;
};

inline constexpr PawnFigureDef PAWN_FIGURES[] = {
  //  family        name        H      R      smooth          heraldic         palette         drift{hue,sat,val}
  { FAM_REGULAR,  "Pawn",     1.50f, 0.50f, nullptr,        nullptr,         nullptr,        { 0.0f, 0.00f, 0.00f} }, // 0  legacy profile + legacy color
  { FAM_SMOOTH,   "Squat",    1.90f, 0.55f, &PROF_SQUAT,    nullptr,         PAL_BRONZE,     {10.0f, 0.15f, 0.12f} }, // 1
  { FAM_SMOOTH,   "Colossal", 3.50f, 0.75f, &PROF_COLOSSAL, nullptr,         PAL_BLACKGOLD,  { 8.0f, 0.10f, 0.20f} }, // 2  vary contrast, not hue
  { FAM_SMOOTH,   "Acorn",    1.70f, 0.50f, &PROF_ACORN,    nullptr,         PAL_ACORN,      {12.0f, 0.15f, 0.10f} }, // 3  scale arc, keep the cap dip
  { FAM_SMOOTH,   "Spire",    2.80f, 0.35f, &PROF_SPIRE,    nullptr,         PAL_STEELBLUE,  {10.0f, 0.12f, 0.12f} }, // 4
  { FAM_SMOOTH,   "Idol",     1.80f, 0.55f, &PROF_IDOL,     nullptr,         PAL_JADE,       {10.0f, 0.15f, 0.10f} }, // 5
  { FAM_SMOOTH,   "Stele",    2.00f, 0.40f, &PROF_STELE,    nullptr,         PAL_LIMESTONE,  { 6.0f, 0.08f, 0.15f} }, // 6  value is the lever
  { FAM_HERALDIC, "Bronze",   1.00f, 0.50f, nullptr,        &PROF_H_BRONZE,  PAL_H_BRONZE,   {10.0f, 0.15f, 0.12f} }, // 7  patina↔polished
  { FAM_HERALDIC, "Silver",   1.10f, 0.50f, nullptr,        &PROF_H_SILVER,  PAL_H_SILVER,   { 4.0f, 0.06f, 0.15f} }, // 8  stays neutral
  { FAM_HERALDIC, "Gold",     1.15f, 0.50f, nullptr,        &PROF_H_GOLD,    PAL_H_GOLD,     { 6.0f, 0.10f, 0.12f} }, // 9
  { FAM_HERALDIC, "Steel",    1.30f, 0.55f, nullptr,        &PROF_H_STEEL,   PAL_H_STEEL,    { 8.0f, 0.10f, 0.12f} }, // 10
  { FAM_HERALDIC, "Crystal",  1.35f, 0.55f, nullptr,        &PROF_H_CRYSTAL, PAL_H_CRYSTAL,  { 8.0f, 0.08f, 0.10f} }, // 11 hue at base, value near white tip
  { FAM_HERALDIC, "Star",     1.45f, 0.55f, nullptr,        &PROF_H_STAR,    PAL_H_STAR,     {10.0f, 0.06f, 0.08f} }, // 12
  { FAM_HERALDIC, "Divine",   1.55f, 0.55f, nullptr,        &PROF_H_DIVINE,  PAL_H_DIVINE,   { 4.0f, 0.05f, 0.06f} }, // 13 radiant by design — least drift
};

inline constexpr uint32_t PAWN_FIGURE_COUNT = sizeof(PAWN_FIGURES) / sizeof(PAWN_FIGURES[0]);

// ── Family spans — derived from PAWN_FIGURES (for the spawn roll) ────────────
// skin_id layout is contiguous per family: [regular | smooth | heraldic].
inline constexpr uint32_t figure_family_member_count(PawnFamilyId fam) {
    uint32_t n = 0;
    for (uint32_t i = 0; i < PAWN_FIGURE_COUNT; ++i)
        if (PAWN_FIGURES[i].family == fam) ++n;
    return n;
}
inline constexpr uint32_t figure_family_base(PawnFamilyId fam) {
    for (uint32_t i = 0; i < PAWN_FIGURE_COUNT; ++i)
        if (PAWN_FIGURES[i].family == fam) return i;
    return 0u;
}

// ── Witnesses ────────────────────────────────────────────────────────────────
static_assert(PAWN_FIGURE_COUNT == 14, "PAWN_FIGURES must declare 14 figures");
static_assert(FIGURE_SHARES[FAM_REGULAR].share_pct
            + FIGURE_SHARES[FAM_SMOOTH].share_pct
            + FIGURE_SHARES[FAM_HERALDIC].share_pct == 100.0f,
            "family shares must sum to 100%");
static_assert(PAWN_FIGURES[0].family == FAM_REGULAR && PAWN_FIGURES[0].palette == nullptr,
            "figure 0 is the regular pawn: legacy path, no palette");
static_assert(figure_family_member_count(FAM_REGULAR)  == 1
           && figure_family_member_count(FAM_SMOOTH)   == 6
           && figure_family_member_count(FAM_HERALDIC) == 7,
           "figure roll assumes 1/6/7 members per family");
static_assert(figure_family_base(FAM_REGULAR) == 0u
           && figure_family_base(FAM_SMOOTH)  == 1u
           && figure_family_base(FAM_HERALDIC) == 7u,
           "figure families must be contiguous in PAWN_FIGURES order");

} // namespace the_board
} // namespace t7
