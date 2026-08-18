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
// NOT HERE: the spawn-rolled wander policy (chance, cruise, legs,
// retarget, hatch leg) and the colour vocabulary — those are
// DESTRUCTIVE-temperament facts, ORGAN_3 w3, and they edit the next
// spawn rather than this one. Nor the MOUNT_* frame-law mirrors:
// those are LOCKSTEP MIRRORS of world.wgsl and a dial on one half of
// a hand-kept mirror is the L3 hazard the panel exists to avoid.
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
    0.5f,     // wander_steer_soft
    0.15f,    // wander_yaw_max — radius >= r_min/0.15 (~270 u), body-scale arcs
    2.0f,     // wander_yaw_tau — curvature stays continuous
    120.0f,   // wander_arrive_radius — inside this the bearing chase degenerates
};

inline RibbonSurface RIBBON_LIVE = RIBBON_TABLE;
static_assert(RIBBON_TABLE.r_min > 0.0f,
    "r_min is a divisor in the steering law's min(); the design value must "
    "be positive and the enrollment line must floor the dial above zero");

} // namespace the_board
} // namespace t7
