#pragma once
#include <cstdint>

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
// live path overrides them from the bank — a dial on a default nothing
// reads would never be seen to move), the palettes, the tier sets and
// the three gesture registries (D5 tables, priced in the ledger).
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


// ═══ THE ORB VOCABULARY, GRADUATED (ORGAN_3b P3) ═════════════════
// The agent_tiers precedent, for the same reason stated the same way:
// the organ needs to name the definition, and THE ORGAN MAY NOT INCLUDE
// A BODY. So OrbConfig, its table, and the id constants its default
// initialisers name live here; bodies/orbs.hpp keeps its registries, its
// gestures and its impl, and includes this header as it already did.
//
// IT WAS OrbMoodConfig (ONE_SURFACE-I U0 housekeeping, Jean's word). Its
// rows stopped being seven at ONE_WORLD-II U1b and there is one world now,
// so the moods' name outlived the moods. The rename is WIRE-SAFE by
// construction: an organ row's id is `#BLOCK "." #FIELD` (the enrollment
// macro, src/console/organ_registry.hpp), so the block name ORB_BANK is
// what a stored preset key carries and no struct name ever reaches one.

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

struct OrbConfig {
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

// ─── THE ORB BANK (ONE_WORLD-II U1b) ────────────────────────────
// ORB_MOOD_LIVE was seven rows, one per mood, and the sky wore the live
// one. There is one sky now, so there is one row: ORB_TABLE the design,
// ORB_LIVE the bank, the tree's ORGAN_3 shape.
//
// TRANSCRIBED, NOT DERIVED (the canonized pattern). The literal below is
// the old per-mood table's sunset row copied verbatim — positional, in
// field order — because a `= ORB_MOOD_TABLE[MOOD_OPEN_SUNSET]` initializer
// would die with the table at U2 and leave twenty-five values to be typed
// at the one moment nothing could check them. The witness pins every
// field while the source still stands; it does its whole job in this
// commit and leaves with ORB_MOOD_TABLE.
//                          en     n    hueB   hueV   bri    drg   rul  rotS    rotAxis                  orbS  pal  hct    trs   sepR   alnR    cohR    sepW   alnW   cohW   maxS   gst  drgB  drgO  drgF  drgK
inline constexpr OrbConfig ORB_TABLE =
    {  true,  128, 0.08f, 0.06f, 0.85f, 0.4f,  3u,  0.012f, {0.15f, 0.97f, 0.10f},  0.0f, 0u,  0.08f, 0u,   50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f };

// ORB_MOOD_TABLE stood here — seven rows, one per mood — with the
// four-part witness that proved ORB_TABLE's twenty-five transcribed
// values against its sunset row. Both left at ONE_WORLD-II U2; the
// witness spent itself in U1b, as designed.

inline OrbConfig ORB_LIVE = ORB_TABLE;

} // namespace the_board
} // namespace t7
