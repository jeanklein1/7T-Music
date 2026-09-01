#pragma once
#include <cstdint>
#include "cartridges/the_board/contracts/mood_constants.hpp"   // MOOD_COUNT — sizes ORB_MOOD_TABLE / ORB_MOOD_LIVE

// ─── contracts/orb_surface.hpp ─────────────────────────────────────
//
// THE ORB CONSOLE, GRADUATED (ORGAN_3 w2, C2). The three dials of
// bodies/orbs.hpp's TUNING CONSOLE that are read live rather than
// rolled at spawn: the dome the sky is painted on, the base size
// every orb scales from, and the noise floor the motion rests at.
//
// ORB_CONSOLE is the DESIGN — the authored row, two jobs only:
// seeding ORB_CONSOLE_LIVE and standing under its assert.
// ORB_CONSOLE_LIVE is what configure_orbs reads.
//
// The dome radius carries its own provenance: orbs.hpp's comment says
// "skybox radius — 700 fell into the fog; 500 is the visible dial
// (Jean's dial)". A constant that names itself Jean's dial and then
// cannot be turned is the exact condition this campaign exists to end.
//
// NOT HERE, deliberately: the nine ORB_DEFAULT_* fallbacks (every
// live path overrides them from the mood config — a dial on a default
// nothing reads would never be seen to move), the palettes, the tier
// sets and the three gesture registries (D5 tables, priced in the
// ledger), and ORB_MOOD_TABLE, whose disposition is a DEFINITION and
// not a bank (ORGAN_3 w3).
// ────────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

struct OrbConsole {
    float dome_radius;   // skybox radius the orb field is painted on
    float base_size;     // world units before per-tier scaling
    float noise_floor;   // motion noise amplitude at rest
    // ORGAN_5 P3a — MASTER MOTION STRENGTH, all rules; 1.0 = the authored
    // dance. The kernel scales every rule's energy source by it: brownian's
    // noise injection, orbital's angular speed, flocking's speed ceiling.
    // FROZEN has none to scale, which is that rule's defining property and
    // not an omission.
    //
    // A gen-2 coupling CLAIMS this field through a rest+gain seam when it
    // arrives; until then the dial IS the rest. The field existed in
    // GPUOrbConfig with an uploader and no author since the gen-1 coupling
    // retired — an authored landing pad with nothing landing on it.
    float speed_mult;
};

// The authored design — values carried verbatim from bodies/orbs.hpp's
// TUNING CONSOLE, where they lived before ORGAN_3 w2.
inline constexpr OrbConsole ORB_CONSOLE = {
    500.0f,   // dome_radius — 700 fell into the fog; 500 is the visible dial (Jean's dial)
    2.35f,    // base_size — tuned on the desk
    0.3f,     // noise_floor — rests at the floor (driverless since the gen-1 retirement)
    3.33f,    // speed_mult — no longer identity: the desk turned ORGAN_5
              // P3a's master up, so every rule's energy source is scaled
};

// The live surface — the panel's block and configure_orbs' read.
inline OrbConsole ORB_CONSOLE_LIVE = ORB_CONSOLE;
static_assert(sizeof(OrbConsole) == 4 * sizeof(float),
    "ORB_CONSOLE_LIVE is a whole-struct copy of the design row: a field "
    "added to one is added to the other by construction");


// ═══ THE ORB MOOD VOCABULARY, GRADUATED (ORGAN_3b P3) ═════════════
// The agent_tiers precedent, for the same reason stated the same way:
// the organ needs to name the definition, and THE ORGAN MAY NOT INCLUDE
// A BODY. So OrbMoodConfig, its table, and the id constants its default
// initialisers name move here; bodies/orbs.hpp keeps its registries, its
// gestures and its impl, and includes this header as it already did.
//
// ORB_MOOD_TABLE is the DESIGN — the authored rows, two jobs only:
// seeding ORB_MOOD_LIVE and standing under its assert. ORB_MOOD_LIVE is
// the world's per-mood definition, and it is MOOD-SELECTED: unlike TIER
// and BEHAVIOR, which are one bank the target ignores, this one has a row
// per mood exactly as MoodProfile does, and the write's target picks it.
//
// The rows are POSITIONAL in mood-id order and carry no id field, so they
// move with the ids or not at all — the MOOD_TABLE pattern, kept.

// ── Rule-critical parameter floors ───────────────────────────────
inline constexpr float ORB_DEFAULT_DRAG = 0.5f;
inline constexpr float ORB_DEFAULT_ORBITAL_SPEED = 0.15f;
inline constexpr float ORB_DEFAULT_FLOCK_SEP_R = 50.0f;
inline constexpr float ORB_DEFAULT_FLOCK_ALIGN_R = 120.0f;
inline constexpr float ORB_DEFAULT_FLOCK_COH_R = 200.0f;
inline constexpr float ORB_DEFAULT_FLOCK_SEP_W = 30.0f;
inline constexpr float ORB_DEFAULT_FLOCK_ALIGN_W = 8.0f;
inline constexpr float ORB_DEFAULT_FLOCK_COH_W = 15.0f;
inline constexpr float ORB_DEFAULT_FLOCK_MAX_SPEED = 60.0f;

inline constexpr uint32_t ORB_PAL_JWST_DEEP = 0;
inline constexpr uint32_t ORB_PAL_PILLARS = 1;
inline constexpr uint32_t ORB_PAL_CARINA = 2;
inline constexpr uint32_t ORB_PAL_WARM_MONO = 3;
inline constexpr uint32_t ORB_PAL_COUNT = 4;

inline constexpr uint32_t ORB_TIERSET_JWST = 0;
inline constexpr uint32_t ORB_TIERSET_RES = 1;
inline constexpr uint32_t ORB_TIERSET_COUNT = 2;
inline constexpr uint32_t ORB_TIERSET_NONE = 0xFFFFFFFFu;  // → uniform population

struct OrbMoodConfig {
    // Population
    uint32_t enabled = 0u;     // ORGAN_3b P3 (D2): widened from C++ bool.
                               // A bool is one byte and the ABI's BOOL write
                               // is four; this struct has NO GPU mirror, so
                               // the widen is lawful and the table's `true`
                               // / `false` rows initialise a u32 unchanged.
    uint32_t count = 0;        // clamped to Dim::MAX_ORBS
    // Color (legacy — superseded by palette_id when a palette is set)
    float    base_hue = 0.08f;
    float    hue_variance = 0.05f;
    float    brightness = 0.8f;     // value center; palette spreads around it
    // Motion
    float    drag = ORB_DEFAULT_DRAG;
    uint32_t motion_rule = 0;                       // 0=Brownian 1=Orbital 2=Frozen 3=Flocking
    float    rotation_speed = 0.0f;                   // rad/s
    float    rotation_axis[3] = { 0.0f, 1.0f, 0.0f };   // normalized in configure_orbs
    float    orbital_base_speed = 0.0f;               // rad/s, rule 1 only
    // Color palette
    uint32_t palette_id = ORB_PAL_JWST_DEEP;
    float    hue_converge_target = 0.12f;
    // Population variety (ORB_TIERSET_NONE = uniform population)
    uint32_t tierset_id = ORB_TIERSET_NONE;
    // Flocking (used when motion_rule == 3)
    float    flock_sep_radius = 50.0f;
    float    flock_align_radius = 120.0f;
    float    flock_coh_radius = 200.0f;
    float    flock_sep_weight = 30.0f;
    float    flock_align_weight = 8.0f;
    float    flock_coh_weight = 15.0f;
    float    flock_max_speed = 60.0f;
    // Flock gesture seed (player cycle persists across transitions)
    uint32_t flock_gesture_default = 0u;
    // Per-rule drag multipliers (0 = pass-through, 1.0×)
    float    rule_drag_brownian = 0.0f;
    float    rule_drag_orbital = 0.0f;
    float    rule_drag_frozen = 0.0f;
    float    rule_drag_flocking = 0.0f;
};

// ─── Orb Mood Table ─────────────────────────────────────────────
//
// Rows are POSITIONAL in mood-id order (the MOOD_TABLE pattern) and
//   carry no id field, so they move with the ids or not at all.
//
//                                              en     n    hueB   hueV   bri    drg   rul  rotS    rotAxis                  orbS  pal  hct    anc    trs           sepR   alnR    cohR    sepW   alnW   cohW   maxS   gst  drgB  drgO  drgF  drgK
inline constexpr OrbMoodConfig ORB_MOOD_TABLE[MOOD_COUNT] = {
    /* 0 open_sunset         */ {  true,  128, 0.08f, 0.06f, 0.85f, 0.4f,  3u,  0.012f, {0.15f, 0.97f, 0.10f},  0.0f, 0u,  0.08f, 0u,           50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
    /* 1 indoor_flat         */ {  false, 0,   0.08f, 0.05f, 0.80f, 0.5f,  0u,  0.000f, {0.00f, 1.00f, 0.00f},  0.0f, 0u,  0.12f, 0xFFFFFFFFu,  50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
    /* 2 indoor_vault        */ {  false, 0,   0.08f, 0.05f, 0.80f, 0.5f,  0u,  0.000f, {0.00f, 1.00f, 0.00f},  0.0f, 0u,  0.12f, 0xFFFFFFFFu,  50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
    /* 3 finite_outdoor      */ {  true,  128, 0.08f, 0.06f, 0.85f, 0.4f,  0u,  0.012f, {0.15f, 0.97f, 0.10f},  0.0f, 0u,  0.12f, 0u,           50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
    // ATMOS_1 — the night is the sunset's field, fuller, brighter and slower
    // (256 stars, bri 0.95, a slow near-axial turn); the noon is the
    // indoor disabled row verbatim — no stars by day.
    // THE NIGHT'S MOTION ROW, TUNED. Two of the four per-rule drags carry
    // real multipliers now — BROWNIAN 1.22 and ORBITAL 1.64, both ABOVE 1,
    // so those two rules damp HARDER than the row's own drag asks. FROZEN
    // and FLOCKING keep the SENTINEL 0, which configure_orbs reads as
    // pass-through (×1.0), and orbital_base_speed keeps its own sentinel,
    // which falls back to ORB_DEFAULT_ORBITAL_SPEED (0.15 rad/s). A desk
    // cannot dial a sentinel back — organ_params.inc floors these five one
    // step off it on purpose — so a sentinel surviving in this row is a
    // value the TABLE holds and the panel can only leave alone.
    /* 4 open_night          */ {  true,  256, 0.08f, 0.06f, 0.605f, 0.4f, 3u,  0.007010115f, {0.15f, 0.59f, 0.05f},  0.0f, 0u,  0.08f, 0u,     50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  1.22f, 1.64f, 0.0f, 0.0f },
    /* 5 open_noon           */ {  false, 0,   0.08f, 0.05f, 0.80f, 0.5f,  0u,  0.000f, {0.00f, 1.00f, 0.00f},  0.0f, 0u,  0.12f, 0xFFFFFFFFu,  50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
    /* MOOD_ATRIUM — the flat room's sky (ATRIUM_1) */
    /* 6 atrium              */ {  false, 0,   0.08f, 0.05f, 0.80f, 0.5f,  0u,  0.000f, {0.00f, 1.00f, 0.00f},  0.0f, 0u,  0.12f, 0xFFFFFFFFu,  50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
};

// ─── THE ORB BANK (ONE_WORLD-II U1b) ────────────────────────────
// ORB_MOOD_LIVE was seven rows, one per mood, and the sky wore the live
// one. There is one sky now, so there is one row: ORB_TABLE the design,
// ORB_LIVE the bank, the tree's ORGAN_3 shape.
//
// TRANSCRIBED, NOT DERIVED (the canonized pattern). The literal below is
// ORB_MOOD_TABLE's sunset row copied verbatim — positional, in field
// order — because a `= ORB_MOOD_TABLE[MOOD_OPEN_SUNSET]` initializer
// would die with the table at U2 and leave twenty-five values to be typed
// at the one moment nothing could check them. The witness pins every
// field while the source still stands; it does its whole job in this
// commit and leaves with ORB_MOOD_TABLE.
//                          en     n    hueB   hueV   bri    drg   rul  rotS    rotAxis                  orbS  pal  hct    trs   sepR   alnR    cohR    sepW   alnW   cohW   maxS   gst  drgB  drgO  drgF  drgK
inline constexpr OrbMoodConfig ORB_TABLE =
    {  true,  128, 0.08f, 0.06f, 0.85f, 0.4f,  3u,  0.012f, {0.15f, 0.97f, 0.10f},  0.0f, 0u,  0.08f, 0u,   50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f };

// THE SEEDING WITNESS (Amendment A). Every field against the row it was
// transcribed from — a slip in a twenty-five-value positional literal is
// a COMPILE ERROR rather than a sky that is subtly wrong and blamed on
// the seed.
static_assert(ORB_TABLE.enabled             == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].enabled
           && ORB_TABLE.count               == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].count
           && ORB_TABLE.base_hue            == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].base_hue
           && ORB_TABLE.hue_variance        == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].hue_variance
           && ORB_TABLE.brightness          == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].brightness
           && ORB_TABLE.drag                == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].drag
           && ORB_TABLE.motion_rule         == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].motion_rule
           && ORB_TABLE.rotation_speed      == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].rotation_speed,
    "ORB_TABLE's population and motion head is the sunset row's, transcribed (ONE_WORLD-II U1b)");
static_assert(ORB_TABLE.rotation_axis[0]    == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].rotation_axis[0]
           && ORB_TABLE.rotation_axis[1]    == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].rotation_axis[1]
           && ORB_TABLE.rotation_axis[2]    == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].rotation_axis[2]
           && ORB_TABLE.orbital_base_speed  == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].orbital_base_speed
           && ORB_TABLE.palette_id          == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].palette_id
           && ORB_TABLE.hue_converge_target == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].hue_converge_target
           && ORB_TABLE.tierset_id          == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].tierset_id,
    "ORB_TABLE's axis, palette and tierset are the sunset row's, transcribed (ONE_WORLD-II U1b)");
static_assert(ORB_TABLE.flock_sep_radius     == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].flock_sep_radius
           && ORB_TABLE.flock_align_radius   == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].flock_align_radius
           && ORB_TABLE.flock_coh_radius     == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].flock_coh_radius
           && ORB_TABLE.flock_sep_weight     == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].flock_sep_weight
           && ORB_TABLE.flock_align_weight   == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].flock_align_weight
           && ORB_TABLE.flock_coh_weight     == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].flock_coh_weight
           && ORB_TABLE.flock_max_speed      == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].flock_max_speed
           && ORB_TABLE.flock_gesture_default == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].flock_gesture_default,
    "ORB_TABLE's flock is the sunset row's, transcribed (ONE_WORLD-II U1b)");
static_assert(ORB_TABLE.rule_drag_brownian  == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].rule_drag_brownian
           && ORB_TABLE.rule_drag_orbital   == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].rule_drag_orbital
           && ORB_TABLE.rule_drag_frozen    == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].rule_drag_frozen
           && ORB_TABLE.rule_drag_flocking  == ORB_MOOD_TABLE[MOOD_OPEN_SUNSET].rule_drag_flocking,
    "ORB_TABLE's per-rule drags are the sunset row's, transcribed (ONE_WORLD-II U1b)");

inline OrbMoodConfig ORB_LIVE = ORB_TABLE;

} // namespace the_board
} // namespace t7
