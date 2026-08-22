#pragma once
#include <cstdint>

// ─── contracts/ribbon_surface.hpp ──────────────────────────────────
//
// THE RIBBON'S HEAD LAW, GRADUATED (ORGAN_3 w2, C2). bodies/ribbon.hpp
// says of these constants, in its own banner: "All control-panel
// material." This is that panel.
//
// RIBBON_TABLE is the DESIGN; RIBBON_LIVE is what ribbon_advance_head
// and the wander steering read every frame. Values carried verbatim
// from the module's tuning console, which keeps its banner and loses
// its numbers — one fact, one home.
//
// THE STEERING LAW IS A `min`, NOT AN ASSERT, and that is why both of
// its terms may go live. ribbon.hpp states it:
//
//     available yaw rate is min(RIBBON_LIVE.yaw_rate, speed / RIBBON_LIVE.r_min)
//
// so the heading can only change while moving and the flown path can
// never be tighter than the minimum turn radius. A `min` stays honest
// at every value either dial can reach — unlike the beacon's
// static_assert, which is why THAT pair stayed authored. Same shape of
// question, opposite answer, because the mechanism differs.
//
// R_MIN'S FLOOR IS 1, NOT 0. It is a divisor; zero would divide the
// turn radius to nothing and hand the head an unbounded yaw rate. The
// enrollment line carries that floor.
//
// HERE SINCE ORGAN_4 P3d, IN A SECOND BANK: the spawn-rolled wander
// policy (chance, cruise, legs, retarget, hatch leg) and the colour
// vocabulary. They are DESTRUCTIVE-temperament facts and they edit the
// NEXT spawn rather than this one, which is why they are a second bank
// with a second temperament and not four more fields on the first.
//
// THE DIALS REACH THE GPU THROUGH `config` (RIBBON_1). The head and the
// body are kernels now; they read config.ribbon_* and nothing else. The
// boot pins this table into GPUDesignConfig's tail (GROWTH LAW,
// state.hpp) and the organ edits it there — one home authors, one
// transport carries, and the rooms cannot drift.
//
// THE FRAME-LAW MIRRORS ARE GONE, not withheld. RIBBON_1 deleted the CPU
// head that carried this room's half of them: the frame law is the body
// kernel's alone, so there is no second half to keep in lockstep and no
// hazard to keep a dial out of. RIBBON_BANK_GAIN / RIBBON_BANK_MAX stay
// WGSL consts — hot-reloadable by save, tuned on screen — until a round
// asks for them on the panel.
// ────────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

struct RibbonSurface {
    // ── Head control law ─────────────────────────────────────────
    float yaw_rate;         // rad/s cap at full deflection
    float max_speed;        // world units/s at full throttle
    float r_min;            // minimum turn radius (units) — a DIVISOR
    float climb_rate;       // u/s cap on the pen's vertical velocity
    float floor_margin;     // guaranteed gap over tall ground
    float alt_smooth_dist;  // units of travel over which the altitude target relaxes
    float alt_stiff;        // (rad/s)^2 — the pen's stiffness
    float mount_setback;    // pawn seat setback toward the tail
    float sky_yaw_tau;      // s — first-order ease on the player's yaw hand
    float reference_bpm;    // the tempo at which the tiers' sway is DEFINED
    // ── The Sky Rule (RIBBON_1) — self-preservation, two readers ──
    float lookahead;        // wu ahead of the nose where the head reads the rule
    float clear_head;       // wu of clearance the HEAD keeps from a shell
    float clear_body;       // wu of clearance the BODY keeps from a shell
    // ── Wander steering (the per-frame half; the rolls are w3) ────
    float wander_steer_soft;    // rad of heading error for full deflection
    float wander_yaw_max;       // yaw cap — radius >= r_min / this
    float wander_yaw_tau;       // s — first-order ease on the steering
    float wander_arrive_radius; // u — arrival = retarget
};

inline constexpr RibbonSurface RIBBON_TABLE = {
    1.0f,     // yaw_rate
    40.0f,    // max_speed — halved; full-throttle turns bottom out at r_min
    40.0f,    // r_min
    15.0f,    // climb_rate
    25.0f,    // floor_margin
    180.0f,   // alt_smooth_dist — the head reads the LANDSCAPE, not the texture
    0.36f,    // alt_stiff — damping = 2*sqrt(stiffness), critically damped
    1.5f,     // mount_setback
    0.6f,     // sky_yaw_tau — short tau keeps the yaw hand immediate
    100.0f,   // reference_bpm
    120.0f,   // lookahead — three seconds of flight at full throttle
    25.0f,    // clear_head — the nose gives a shaft the floor_margin's berth
    8.0f,     // clear_body — the body flows closer than the head steers
    0.5f,     // wander_steer_soft
    0.15f,    // wander_yaw_max — radius >= r_min/0.15 (~270 u), body-scale arcs
    2.0f,     // wander_yaw_tau — curvature stays continuous
    120.0f,   // wander_arrive_radius — inside this the bearing chase degenerates
};

inline RibbonSurface RIBBON_LIVE = RIBBON_TABLE;
static_assert(RIBBON_TABLE.r_min > 0.0f,
    "r_min is a divisor in the steering law's min(); the design value must "
    "be positive and the enrollment line must floor the dial above zero");


// ═══ THE SPAWN BANK, GRADUATED (ORGAN_4 P3d) ═════════════════════
//
// C3 DESTRUCTIVE, every row, and that is why it is a SECOND bank rather
// than more fields on the first. RIBBON_LIVE is read every frame by the
// head mover; everything below is read ONCE, as a ribbon is drawn:
// `select_ribbon_for_patch` rolls the spawn chance and the colour mode,
// `fill_ribbon_selection_geometry` draws the colour, `commit_ribbon`
// rolls the wander policy, and none of them looks again. Re-speaking a
// destructive author means tearing down the ribbons that already exist
// to apply a slider, so this bank has NO BOUNDARY WIRING (D5) and its
// enrollment rows wear the GEN chip: the edit lands on the next ribbon.
//
// SIZE COUNTS THAT ARE NOT DIALS STAY IN THE MODULE.
// RIBBON_SMOOTH_PALETTE_COUNT sizes the palette row here and the draw
// there; RibbonColorMode's three ids are a vocabulary, not a range.
inline constexpr uint32_t RIBBON_SMOOTH_PALETTE_COUNT = 4;
inline constexpr uint32_t RIBBON_COLOR_MODE_COUNT = 3;

struct RibbonSpawnSurface {
    // ── The spawn rolls ──────────────────────────────────────────
    float spawn_chance;          // per-patch: does a ribbon happen at all
    float position_jitter;       // anchor scatter inside the trigger patch
    float wander_chance;         // per-spawn: is this one an idle wanderer
    float wander_cruise_base;    // gaussian mean, as a fraction of max_speed
    float wander_cruise_sigma;   // gaussian sigma
    float wander_cruise_min;     // the draw's clamp, low
    float wander_cruise_max;     // the draw's clamp, high
    float wander_leg_min;        // waypoint leg length (units)
    float wander_leg_max;
    float wander_spread;         // rad of bearing spread around current motion
    float wander_retarget_min;   // seconds between waypoints
    float wander_retarget_var;
    float wander_hatch_leg;      // the newborn's opening stride (units)
    // ── The colour vocabulary ────────────────────────────────────
    float color_weights[RIBBON_COLOR_MODE_COUNT];   // SMOOTH / TINTED / CONTRAST
    float smooth_palette[RIBBON_SMOOTH_PALETTE_COUNT][3];
    float smooth_var_range;      // var = hash * RANGE + BIAS
    float smooth_var_bias;
    float smooth_var_g_scale;    // green gets var * G_SCALE
    float smooth_var_b_scale;    // blue  gets var * B_SCALE
    float tinted_range[3];       // per-channel hash * range + base
    float tinted_base[3];
};

inline constexpr RibbonSpawnSurface RIBBON_SPAWN_TABLE = {
    0.900f,   // spawn_chance    — the module's authored value
    0.3f,     // position_jitter
    0.30f,    // wander_chance
    0.35f,    // wander_cruise_base
    0.15f,    // wander_cruise_sigma
    0.15f,    // wander_cruise_min
    0.80f,    // wander_cruise_max
    200.0f,   // wander_leg_min
    500.0f,   // wander_leg_max
    1.0f,     // wander_spread
    10.0f,    // wander_retarget_min
    15.0f,    // wander_retarget_var
    300.0f,   // wander_hatch_leg — sized inside the LEG_MIN/MAX band
    { 0.40f, 0.35f, 0.25f },   // color_weights — SMOOTH / TINTED / CONTRAST
    {
        { 0.82f, 0.75f, 0.62f },   // warm sandstone
        { 0.55f, 0.65f, 0.78f },   // sky blue
        { 0.85f, 0.78f, 0.58f },   // golden
        { 0.50f, 0.68f, 0.55f },   // sage green
    },
    0.10f,    // smooth_var_range
    -0.05f,   // smooth_var_bias
    0.8f,     // smooth_var_g_scale
    0.6f,     // smooth_var_b_scale
    { 0.45f, 0.40f, 0.45f },   // tinted_range
    { 0.40f, 0.35f, 0.35f },   // tinted_base
};

inline RibbonSpawnSurface RIBBON_SPAWN_LIVE = RIBBON_SPAWN_TABLE;
static_assert(sizeof(RibbonSpawnSurface)
              == (13 + RIBBON_COLOR_MODE_COUNT
                     + RIBBON_SMOOTH_PALETTE_COUNT * 3 + 4 + 3 + 3) * sizeof(float),
    "RIBBON_SPAWN_LIVE is a whole-struct copy of the design row: a field "
    "added to one is added to the other by construction");
static_assert(RIBBON_SPAWN_TABLE.wander_cruise_min
              <= RIBBON_SPAWN_TABLE.wander_cruise_max,
    "the cruise draw clamps low-then-high; an inverted pair would pin every "
    "wanderer to the low bound and read as a dead gaussian");
static_assert(RIBBON_SPAWN_TABLE.wander_leg_min <= RIBBON_SPAWN_TABLE.wander_leg_max,
    "the leg draw is min + (max - min) * u; an inverted pair would walk the "
    "waypoint backwards");

} // namespace the_board
} // namespace t7
