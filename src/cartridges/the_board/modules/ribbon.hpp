#pragma once
#include <cstdint>
#include <array>      // RibbonHead propagation history
#include "cartridges/the_board/state.hpp"                    // Dim::*, GPURibbonState, wgpu
#include "cartridges/the_board/modules/mood_constants.hpp"   // MOOD_COUNT (sizes the mood gate)
#include "cartridges/the_board/modules/keyhole.hpp"          // Cartridge + wgpu::Queue fwds (the keyhole)
#include "cartridges/the_board/modules/entity_types.hpp"     // RibbonSelection/RibbonPlacement (the boundary DTOs) + queue types

// ─── ribbon.hpp (HEADER: console + vocabulary + state + decls) ───
// Converted (LADDER-3 c5): history in audit/LADDER.md.
//
// Sky Ribbon: complete subsystem (vocabulary + machinery in one
// module). Single-instance, bespoke pipeline — runs through the
// 3-phase select/place/commit shape but doesn't share entity_pipeline's
// generic machinery.
//
// THE HEAD IS THE INSTRUMENT; THE BODY IS THE LAW'S CONSEQUENCE.
// Every author — player, wanderer, and (soon) the musical canvas —
// plays the head through one steering integrator and one altitude
// pen; the propagation law replays the head's past down the body at
// P. Couple the head, and the rest follows.
//
// Flying ribbons: compound wave functions (lateral + vertical sway)
// forming square-tube geometry in the sky. Each ribbon is a tier
// instance with Gaussian-sampled parameters.
//
// PAIRING (the mirror law, AMENDED at LADDER-3 c5): the constitution's
// practiced convention — the_board/the_chord mirrored-module deltas are
// byte-identical (ribbon.inl named first among them) — is SUSPENDED for
// this module by the header ladder's declared structural divergence:
// the_board's ribbon is header/impl split (this file + the repurposed
// ribbon.inl); the_chord's ribbon.inl remains a class-body include under
// the prior law. Same suspension class as pawn.inl (LADDER-2 c2). The
// BOM stays with ribbon.inl on both sides; content deltas beyond the
// split remain subject to the mirror law's spirit until the pairing is
// re-ratified.
//
// ┌─── Public surface (called from outside this module) ────────────┐
// │                                                                  │
// │  Module functions take RibbonState& explicitly. This makes       │
// │  ribbon's state ownership language-visible and cross-cutting     │
// │  dependencies explicit in function signatures.                   │
// │                                                                  │
// │  Lifecycle (three-phase):                                        │
// │    select_ribbon_for_patch(rs, c, gx, gz, sel)  — Phase 1: roll  │
// │    place_ribbon_from_selection(c, sel, plan)    — Phase 2: place │
// │      (note: takes no RibbonState — only mediates between sel     │
// │       and spawn-engine helpers; not part of ribbon's data)       │
// │    commit_ribbon(rs, c, plan, gx, gz, queue)    — Phase 3: state │
// │                                                                  │
// │  Frame conductor (ONE call per frame from render()):             │
// │    ribbon_frame_tick(rs, c, queue)                               │
// │      — author selection, slot hold/adopt, uploads, advance       │
// │                                                                  │
// │  Head mover (called by the conductor):                           │
// │    ribbon_advance_head(rs, gpuState, queue, ribbon, slot, …)     │
// │    ribbon_head_pose(rs, x, y, z, h)   — the SADDLE (mount)       │
// │    ribbon_head_frame(rs, yaw_off, pitch, roll) — render (saddle) │
// │    ribbon_head_pen(rs, x, z, h)       — the PEN (steering reads) │
// │    ribbon_invalidate_head(rs), ribbon_head_is(rs, slot)          │
// │                                                                  │
// │  Shared geometry helper (also called by mood.inl::apply_mood     │
// │  for the mood-5 forced spawn path):                              │
// │    fill_ribbon_selection_geometry(seed, tier, sel)               │
// │      — pure; no ribbon state needed                              │
// │                                                                  │
// │  Cross-module reads (consumed by spine, mood.inl, render):       │
// │    ribbon_state_.active[], ribbon_state_.gpu[]   — read by spine │
// │    ribbon_state_.active_count                    — read by spine │
// │    ribbon_state_.rendered_slot                   — read by spine │
// │    ribbon_state_.mood_offset                     — read by mood  │
// │    MAX_RIBBON_INSTANCES                                          │
// │                                                                  │
// └──────────────────────────────────────────────────────────────────┘
//
// Depends on: state.hpp (Dim::*, GPURibbonState, GPUState wires, wgpu),
// mood_constants.hpp (MOOD_COUNT). The impl additionally reaches
// spawn-engine services (run_spawn_preamble, negotiate_position,
// record_placement_bookkeeping), seed_utils.hpp, and cartridge core
// (time_state_.seconds/dt/beat_rate, Cartridge::THEMES,
// Cartridge::PATCH_EXTENT, the four ribbon canvas bindings, player_ sky
// fields, estimate_terrain_height / terrain_tile_warm) through the
// complete type (Cartridge:: / keyhole), and the GPU wires
// (upload_ribbon_time / _color / _wave_amps / _head_poses — the flush +
// head laws write through).
//
// SEAM[ribbon:complete-subsystem] complete bespoke pipeline in one
//   module — vocabulary + state + machinery + lifecycle + head laws.
//   Same family as gol_zones (Ch. 12.B) and gallery (Ch. 12.E), and
//   the reference instance of the entity-module pattern: tuning
//   console → registry → tiers → runtime state → author seats →
//   head laws → lifecycle. (surface = the bank rows; the in-module
//   section retired with its last legacy consumer).
// SEAM[ribbon:dual-entry] commit_ribbon has TWO callers:
//   FAMILY_DISPATCH[RIBBON].try_commit during patch streaming, AND
//   mood.inl::apply_mood for mood-5 forced spawn. The dual entry
//   point is owned by mood:K4 (mood-5 reference clone), not by
//   ribbon machinery. Tag-only awareness.
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ TUNING CONSOLE ══════════════════════════════════════════════
//
// System-level dials for the ribbon subsystem — the DESIGN-TIME
// control panel (change and rebuild). Per-tier values (the Gaussian
// means/sigmas that shape each tier's feel) live in RIBBON_TIERS
// below. Everything here applies across all tiers.

// ── Spawn ────────────────────────────────────────────────────────
struct RibbonConfig {
    static constexpr float SPAWN_CHANCE = 0.900f;   // TESTING: was 0.400f -- bumped for ribbon-dev visibility; revert before ship (control-panel constant)
    // SEAM[ribbon:P4] hygiene rows pattern — { open, sunset,
    //   [indoor_flat=0], [indoor_vault=0], [finite_outdoor=1],
    //   [finite_outdoor_ref=0] }. Same family as gol_zones:P4
    //   and floaters:P4.
    static constexpr float MOOD_MULTIPLIER[MOOD_COUNT] = { 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f };
    static constexpr float POSITION_JITTER = 0.3f;
};

// ── Length cap ───────────────────────────────────────────────────
// Total ribbon length (cube_count × cube_size) is capped here to
// keep anchor coverage viable (700 u = 14 patches). RIBBON_MAX_LENGTH
// itself is defined below in this file (Capacity section); consumed
// in fill_ribbon_selection_geometry.

// ── Geometry / placement ─────────────────────────────────────────
// Floors on the Gaussian-sampled shape draws plus the fixed spawn
// footprint and orientation spread. Consumed in
// fill_ribbon_selection_geometry / select_ribbon_for_patch.
inline constexpr float MIN_CUBE_COUNT     = 20.0f;    // floor on Gaussian-sampled cube_count
inline constexpr float MIN_CUBE_SIZE      = 1.0f;     // floor on cube_size
inline constexpr float MIN_ADDED_HEIGHT   = 20.0f;    // floor on the clearance draw
inline constexpr float FOOTPRINT_RADIUS   = 5.0f;     // ribbon spawn footprint radius
inline constexpr float ORIENTATION_SPREAD = 1.0472f;  // ±60° (π/3) around away-from-pawn

// ── Head control law ─────────────────────────────────────────────
// The steering integrator and altitude pen constants — one law, many
// authors. Yaw is STEERING, not free aim: available yaw rate is
// min(RIBBON_YAW_RATE, speed / RIBBON_R_MIN), so the heading can only
// change while moving and the flown path can never be tighter than
// the minimum turn radius. Consumed in ribbon_advance_head. All
// control-panel material.
inline constexpr float RIBBON_YAW_RATE       = 1.0f;    // rad/s cap at full deflection
inline constexpr float RIBBON_MAX_SPEED      = 40.0f;   // world units/s at full throttle (halved; full-throttle turns bottom out at R_MIN)
inline constexpr float RIBBON_R_MIN          = 40.0f;   // minimum turn radius (units)
inline constexpr float RIBBON_CLIMB_RATE     = 15.0f;   // u/s cap on the pen's vertical velocity
inline constexpr float RIBBON_FLOOR_MARGIN   = 25.0f;   // guaranteed gap over tall ground
inline constexpr float RIBBON_ALT_SMOOTH_DIST = 180.0f; // units of travel over which the altitude target relaxes — the head reads the LANDSCAPE, not the terrain texture
inline constexpr float RIBBON_ALT_STIFF      = 0.36f;   // (rad/s)^2 — the pen's stiffness; damping = 2*sqrt(stiffness), critically damped
inline constexpr float RIBBON_MOUNT_SETBACK  = 1.5f;    // pawn seat setback toward the tail (+heading) so the body sits over the tube, not the leading cap
inline constexpr float RIBBON_SKY_YAW_TAU    = 0.6f;    // s; first-order ease on the PLAYER's yaw hand — the body replays the heading history, so bang-bang arrows must become curves; short tau keeps it immediate
inline constexpr float RIBBON_REFERENCE_BPM  = 100.0f;  // the tempo at which the tiers' authored sway is DEFINED; phase advances at live-tempo/this (control-panel)

// ── Frame-law mirrors (BNK-2) ── LOCKSTEP MIRRORS of world.wgsl's
// RIBBON_TANGENT_ALIGN / RIBBON_BANK_GAIN / RIBBON_BANK_MAX. The GPU set
// is the tuning authority (hot-reload); when a value settles there, copy
// it here. Drift is SELF-ANNOUNCING: the rider visibly leans differently
// than the face beneath it. (values below = the settled BNK-1 values)
inline constexpr float MOUNT_TANGENT_ALIGN = 1.0f;
inline constexpr float MOUNT_BANK_GAIN     = 0.9f;
inline constexpr float MOUNT_BANK_MAX      = 0.6f;

// ── Wander policy ─────────────────────────────────────────────────
// The steering channel's IDLE SCRIPT — the shape of autonomous drift.
// Constants: control-panel material.
inline constexpr float WANDER_CHANCE      = 0.30f;   // per-spawn roll
inline constexpr float WANDER_CRUISE_BASE  = 0.35f;   // gaussian mean (fraction of RIBBON_MAX_SPEED)
inline constexpr float WANDER_CRUISE_SIGMA = 0.15f;   // gaussian sigma
inline constexpr float WANDER_CRUISE_MIN  = 0.15f;
inline constexpr float WANDER_CRUISE_MAX  = 0.80f;
inline constexpr float WANDER_LEG_MIN     = 200.0f;  // waypoint leg length (units)
inline constexpr float WANDER_LEG_MAX     = 500.0f;
inline constexpr float WANDER_SPREAD      = 1.0f;    // rad of bearing spread around current motion
inline constexpr float WANDER_RETARGET_MIN = 10.0f;  // seconds between waypoints
inline constexpr float WANDER_RETARGET_VAR = 15.0f;
inline constexpr float WANDER_HATCH_LEG   = 300.0f;  // hatchling's first-waypoint leg (units) — the newborn's opening stride, sized between LEG_MIN/MAX's band; consumed in commit_ribbon's hatchling rule
inline constexpr float WANDER_STEER_SOFT  = 0.5f;    // rad of heading error for full deflection
inline constexpr float WANDER_YAW_MAX     = 0.15f;   // yaw cap: radius >= RIBBON_R_MIN/0.15 (~270 u) — body-scale arcs
inline constexpr float WANDER_YAW_TAU     = 2.0f;    // s; first-order ease on the steering — curvature stays continuous (control-panel)
inline constexpr float WANDER_ARRIVE_RADIUS = 120.0f; // u; arrival = retarget — inside this the bearing chase degenerates (control-panel)

// ═══ COLOR VOCABULARY ════════════════════════════════════════════

struct RibbonColorMode {
    static constexpr uint32_t SMOOTH = 0;  // terrain-derived monochrome
    static constexpr uint32_t TINTED = 1;  // warm/cool hue shift
    static constexpr uint32_t CONTRAST = 2;  // cell skin: per-cell coloring — pair-contrast species (two medians, parity) or median-field species (one median, terrain-patch texture); see fill
    static constexpr uint32_t COUNT = 3;
    static constexpr float WEIGHTS[COUNT] = { 0.40f, 0.35f, 0.25f };
};

// Smooth color palettes: base colors for SMOOTH mode ribbons
inline constexpr float RIBBON_SMOOTH_PALETTE[][3] = {
    { 0.82f, 0.75f, 0.62f },   // warm sandstone
    { 0.55f, 0.65f, 0.78f },   // sky blue
    { 0.85f, 0.78f, 0.58f },   // golden
    { 0.50f, 0.68f, 0.55f },   // sage green
};
inline constexpr uint32_t RIBBON_SMOOTH_PALETTE_COUNT = 4;

// ── Color character ──────────────────────────────────────────────
// Per-mode dials for the color draws in fill_ribbon_selection_geometry.
// Structure (which hash prop feeds which channel, the (1 - hue) on
// CONTRAST green) stays in code; only the magnitudes live here.

// SMOOTH: per-channel variance around the palette base.
//   var = hash * RANGE + BIAS; applied as { +var, +var*G, +var*B }.
inline constexpr float SMOOTH_VAR_RANGE = 0.10f;
inline constexpr float SMOOTH_VAR_BIAS  = -0.05f;
inline constexpr float SMOOTH_VAR_G_SCALE = 0.8f; // green channel gets var * G_SCALE
inline constexpr float SMOOTH_VAR_B_SCALE = 0.6f; // blue  channel gets var * B_SCALE

// TINTED: per-channel hash*range + base (R & B range 0.45, G range 0.40).
inline constexpr float TINTED_RANGE[3] = { 0.45f, 0.40f, 0.45f };
inline constexpr float TINTED_BASE[3]  = { 0.40f, 0.35f, 0.35f };

// CONTRAST — the checker pair raffle, shaped like the terrain's §2.2
// palette system. Each pair authors its colors AND its character:
//   value_var — per-cell lightness texture: fraction of headroom each
//               cell travels toward black/white (clip-free). Legibility
//               guidance: beyond ~0.15 the dark/light parity starts to
//               blur; author past it only on purpose.
//   hue_var   — 0..1, the colorful axis: scales the per-cell hue
//               rotation and chroma injection (world.wgsl skin block).
//               0 = strict chessboard; 1 = full-wheel mosaic on BOTH
//               squares (light cells go pastel at their own lightness).
//   weight    — cumulative-roll probability; rare pairs are events.
// Weights sum to 1. All control-panel; author freely.
struct CheckerPair {
    float dark[3];
    float light[3];
    float value_var;
    float hue_var;
    float weight;
};
inline constexpr CheckerPair CHECKER_PAIRS[] = {
    { {0.16f,0.15f,0.17f}, {0.88f,0.86f,0.82f}, 0.05f, 0.05f, 0.30f },  // obsidian / bone   — strict
    { {0.30f,0.12f,0.18f}, {0.92f,0.78f,0.80f}, 0.06f, 0.20f, 0.20f },  // wine / rose       — calm
    { {0.14f,0.16f,0.34f}, {0.87f,0.76f,0.58f}, 0.06f, 0.30f, 0.20f },  // indigo / sand     — lively
    { {0.10f,0.24f,0.16f}, {0.78f,0.90f,0.80f}, 0.07f, 0.55f, 0.15f },  // forest / mint     — wild
    { {0.38f,0.18f,0.10f}, {0.90f,0.85f,0.74f}, 0.06f, 0.35f, 0.10f },  // rust / cream      — lively
    { {0.13f,0.13f,0.13f}, {0.92f,0.80f,0.45f}, 0.05f, 0.75f, 0.05f },  // charcoal / gold   — rare riot
};
inline constexpr uint32_t CHECKER_PAIR_COUNT =
    sizeof(CHECKER_PAIRS) / sizeof(CHECKER_PAIRS[0]);
inline constexpr float CHECKER_PAIR_JITTER = 0.03f;  // shared per-ribbon median offset
inline constexpr float CHECKER_HUE_SIBLING_JITTER = 0.10f;  // per-ribbon ± around the pair's hue_var

// FREE RAFFLE — the terrain's discrete-region grammar for the skin: both
// medians raffled as points in (luma, chroma, hue) space, both variances
// raffled. The bounds below ARE the lattice; narrow them to tame, widen to
// liberate. The one kept law: disjoint luma bands preserve the dark/light
// parity under any hue — the chessboard survives its own liberation.
// All control-panel.
inline constexpr float FREE_PAIR_CHANCE   = 0.50f;  // vs the authored pair table
inline constexpr float FREE_DARK_LUMA[2]  = { 0.10f, 0.35f };
inline constexpr float FREE_DARK_CHROMA[2]= { 0.05f, 0.30f };
inline constexpr float FREE_LIGHT_LUMA[2] = { 0.70f, 0.95f };
inline constexpr float FREE_LIGHT_CHROMA[2]={ 0.02f, 0.22f };
inline constexpr float FREE_VALUE_VAR[2]  = { 0.02f, 0.30f };  // raffled, generous ceiling
inline constexpr float FREE_HUE_VAR[2]    = { 0.00f, 1.00f };  // raffled, UNCAPPED (full axis)
// Rodrigues basis about the gray axis (unit chroma + its quadrature) —
// the CPU twin of the shader's hue machinery.
inline constexpr float CHROMA_D1[3] = { 0.8165f, -0.4082f, -0.4082f };
inline constexpr float CHROMA_D2[3] = { 0.0f,     0.7071f, -0.7071f };

// MEDIAN-FIELD species — some cell-skinned ribbons are not defined by
// contrast at all: ONE median, cells as variations around it — the
// terrain-patch grammar on a tube (color_b == color; the parity term
// vanishes by algebra). Luma band is broad (no parity to protect);
// VALUE_VAR floor sits higher so the cells read through texture, as
// terrain cells do. All control-panel.
inline constexpr float CELLS_MEDIAN_CHANCE   = 0.35f;  // species roll, above the pair fork
inline constexpr float MEDIAN_LUMA[2]        = { 0.25f, 0.85f };
inline constexpr float MEDIAN_CHROMA[2]      = { 0.04f, 0.30f };
inline constexpr float MEDIAN_VALUE_VAR[2]   = { 0.06f, 0.35f };
inline constexpr float MEDIAN_HUE_VAR[2]     = { 0.00f, 1.00f };

// ═══ PROPERTY INDEX REGISTRY ═════════════════════════════════════
//
// Stride convention (intentional, do not compact):
//   400      SPAWN_ROLL
//   401-409  per-instance scalar rolls (ANCHOR_X..PALETTE_IDX)
//   410-419  cube-count / size / height       (10-row reserve)
//   420-429  lateral wave  (amp, cycles, speed; rest reserved)
//   430-439  vertical wave (amp; rest reserved)
//   440-449  checker skin  (pair roll, median jitter, hue sibling-jitter; rest reserved)
//   450-459  wander        (roll, cruise, rng seed; rest reserved)
//   460-469  checker free raffle (mode, dark l/c/h, light l/c/h, vars; rest reserved)
//   470-479  checker median-field (species roll, luma, chroma, hue, vars; rest reserved)
//   The per-axis stride of 10 leaves room for future per-axis
//   params without renumbering downstream. Same self-documentation
//   discipline used by the WGSL side.

struct RibbonProp {
    static constexpr uint32_t SPAWN_ROLL = 400u;
    static constexpr uint32_t ANCHOR_X = 401u;
    static constexpr uint32_t ANCHOR_Z = 402u;
    static constexpr uint32_t TIER = 403u;
    static constexpr uint32_t COLOR_ROLL = 404u;
    static constexpr uint32_t ORIENTATION = 405u;
    static constexpr uint32_t COLOR_R = 406u;
    static constexpr uint32_t COLOR_G = 407u;
    static constexpr uint32_t COLOR_B = 408u;
    static constexpr uint32_t PALETTE_IDX = 409u;
    // Gaussian draw indices
    static constexpr uint32_t CUBE_COUNT = 410u;
    static constexpr uint32_t CUBE_SIZE = 411u;
    static constexpr uint32_t HEIGHT = 412u;
    static constexpr uint32_t LATERAL_AMP = 420u;
    static constexpr uint32_t LATERAL_CYCLES = 421u;
    static constexpr uint32_t VERTICAL_AMP = 430u;
    static constexpr uint32_t CHECKER_PAIR_ROLL = 440u;   // pair raffle
    static constexpr uint32_t CHECKER_JIT_R     = 441u;   // shared median jitter
    static constexpr uint32_t CHECKER_JIT_G     = 442u;
    static constexpr uint32_t CHECKER_JIT_B     = 443u;
    static constexpr uint32_t CHECKER_HUE_JITTER_ROLL = 444u;  // sibling ± around the pair's hue_var
    static constexpr uint32_t FREE_MODE_ROLL   = 460u;  // free vs authored table
    static constexpr uint32_t FREE_DARK_L      = 461u;
    static constexpr uint32_t FREE_DARK_C      = 462u;
    static constexpr uint32_t FREE_DARK_H      = 463u;
    static constexpr uint32_t FREE_LIGHT_L     = 464u;
    static constexpr uint32_t FREE_LIGHT_C     = 465u;
    static constexpr uint32_t FREE_LIGHT_H     = 466u;
    static constexpr uint32_t FREE_VALUE_ROLL  = 467u;
    static constexpr uint32_t FREE_HUE_ROLL    = 468u;
    static constexpr uint32_t MEDIAN_SPECIES_ROLL = 470u;
    static constexpr uint32_t MEDIAN_L            = 471u;
    static constexpr uint32_t MEDIAN_C            = 472u;
    static constexpr uint32_t MEDIAN_H            = 473u;
    static constexpr uint32_t MEDIAN_VALUE_ROLL   = 474u;
    static constexpr uint32_t MEDIAN_HUE_ROLL     = 475u;
    static constexpr uint32_t WANDER_ROLL = 450u;       // wander yes/no
    static constexpr uint32_t WANDER_CRUISE = 451u;     // gaussian draw: cruise fraction of RIBBON_MAX_SPEED
    static constexpr uint32_t WANDER_RNG = 452u;        // seeds the runtime waypoint stream
};

// ═══ TIER PROFILE + MATRIX ═══════════════════════════════════════
//
// Three tiers — Serpentine, Helix, Streamer — each with mean+sigma
// for every wave/geometry parameter. Pattern matches GoLTierProfile.

inline constexpr uint32_t RIBBON_TIER_COUNT = 3;
inline constexpr float RIBBON_BASE_TIER_WEIGHTS[RIBBON_TIER_COUNT] = {
    0.45f, 0.30f, 0.25f
};

struct RibbonTierProfile {
    // ─── Geometry ────────────────────────────────────────────
    float cube_count_mean, cube_count_sigma;
    float cube_size_mean, cube_size_sigma;

    // Cube count ~50–115 across tiers; cube size ~3.5–10 (Serpentine chunky).

    // ─── Altitude ────────────────────────────────────────────
    float height_mean, height_sigma;

    // Flying height: 50–80 units. All wave params independently seeded.

    // Trail-frame: tiers author lateral_cycles (aesthetic visible cycles)
    // + amps. Temporal rate is derived at commit: freq = cycles * 2pi *
    // propagation_speed / total_length; vertical shares the lateral cycles
    // (one visible wavelength, two amplitudes).

    // ─── Lateral wave ─────────────────────────────────────
    float lateral_amp_mean, lateral_amp_sigma;
    float lateral_cycles_mean, lateral_cycles_sigma;

    // ─── Vertical wave ────────────────────────────────────
    float vertical_amp_mean, vertical_amp_sigma;

    // ─── Propagation (trail-frame head→tail rate, world units/s) ──
    float propagation_speed;

    // ─── Selection ───────────────────────────────────────────
    float weight;
};

//                          ┌── Serpentine ──┬──── Helix ─────┬─── Streamer ───┐
//                          │   μ       σ    │   μ       σ    │   μ       σ    │
// ─── Geometry ────────────┤                │                │                │
//   cube_count             │  70       7    │  85      12    │  90       9    │
//   cube_size              │   8.0     1.0  │   5.0     0.8  │   6.0     0.8  │
// ─── Altitude ────────────┤                │                │                │
//   height                 │  60      15    │  55      12    │  70      20    │
// ─── Lateral wave ────────┤                │                │                │
//   lateral_amp            │  10.0     0.6  │   3.5     0.6  │   5.5     0.8  │
//   lateral_cycles         │   1.2     0.3  │   1.8     0.5  │   1.5     0.4  │
// ─── Vertical wave ───────┤  cycles = lateral (one wavelength, two amps)     │
//   vertical_amp           │   5.0     0.8  │   2.5     0.5  │   8.0     1.2  │
// ─── Trail-frame ─────────┤                │                │                │
//   propagation_speed      │  40.0          │  24.0          │  48.0          │
// ─── Selection ───────────┤                │                │                │
//   weight                 │   0.45         │   0.30         │   0.25         │
//                          └────────────────┴────────────────┴────────────────┘
//
// LENGTH LAW — the tier means live UNDER the cap; they do not fight it.
//   μ_len = count·size ≈ 0.8 × RIBBON_MAX_LENGTH (560 / 425 / 540 u), with
//   +2σ_len ≈ the cap, so the Gaussian breathes and capping is a rare tail
//   event rather than the norm. lateral_cycles were rescaled by μ_len/700
//   from the capped-era values, preserving each tier's temporal sway rate
//   (freq = cycles·2π·P/L): 0.54 / 0.64 / 0.84 rad/s at the means.
inline constexpr RibbonTierProfile RIBBON_TIERS[RIBBON_TIER_COUNT] = {
    // Tier 0: Serpentine — long, massive, slow motion
    {    70.0f,  7.0f,      // cube_count
          8.0f,  1.0f,      // cube_size
         60.0f, 15.0f,      // height
         10.0f,  0.6f,      // lateral_amp
          1.2f,  0.3f,      // lateral_cycles
          5.0f,  0.8f,      // vertical_amp
         40.0f,             // propagation_speed (u/s)
          0.45f },          // weight
    // Tier 1: Helix — tight sway, small cubes, slowest propagation
    {    85.0f, 12.0f,      // cube_count
          5.0f,  0.8f,      // cube_size
         55.0f, 12.0f,      // height
          3.5f,  0.6f,      // lateral_amp
          1.8f,  0.5f,      // lateral_cycles
          2.5f,  0.5f,      // vertical_amp
         24.0f,             // propagation_speed (u/s)
          0.30f },          // weight
    // Tier 2: Streamer — tall vertical form, deep breathing
    {    90.0f,  9.0f,      // cube_count
          6.0f,  0.8f,      // cube_size
         70.0f, 20.0f,      // height
          5.5f,  0.8f,      // lateral_amp
          1.5f,  0.4f,      // lateral_cycles
          8.0f,  1.2f,      // vertical_amp
         48.0f,             // propagation_speed (u/s)
          0.25f },          // weight
};

inline constexpr const char* RIBBON_TIER_NAMES[] = {
    "Serpentine", "Helix", "Streamer"
};
inline constexpr const char* RIBBON_COLOR_NAMES[] = {
    "smooth", "tinted", "contrast"
};

// ═══ SPAWN PAYLOADS — AT THE CONTRACT HOME ═══════════════════════
//
// The ribbon Selection/Placement DTOs live in entity_types.hpp,
// beside the EntityQueueEntry / PlacementEntry unions that are their
// reason to exist: a DTO that exists to cross a boundary belongs to
// the boundary's contract, not to either side.

// ═══ RUNTIME STATE ═══════════════════════════════════════════════
//
// Per-instance ribbon state, the GPU-state mirrors, and the head —
// the one live instrument. Single-render today
// (MAX_RIBBON_INSTANCES = 1), structured to scale when the GPU
// supports multi-ribbon.

// ── Capacity ─────────────────────────────────────────────────────
inline constexpr uint32_t MAX_RIBBON_INSTANCES = 1;  // single-render; raise when GPU supports multi-ribbon
inline constexpr float    RIBBON_MAX_LENGTH = 700.0f;

// ── Per-instance tracking ────────────────────────────────────────
// Two-tip anchoring: ribbon survives until BOTH its tip patches are
// out of view (cross-patch eviction tracking).
struct ActiveRibbon {
    int32_t patch_gx = 0, patch_gz = 0;   // trigger patch
    int32_t host_gx = 0, host_gz = 0;     // host patch (anchor position)
    float anchor_x = 0.0f, anchor_z = 0.0f;
    int32_t near_tip_gx = 0, near_tip_gz = 0;
    int32_t far_tip_gx = 0, far_tip_gz = 0;
    bool near_tip_registered = false;
    bool far_tip_registered = false;
    uint32_t ref_count = 0;     // patches referencing this ribbon via record_entity
    bool active = false;
    float spawn_color[3] = { 0.0f, 0.0f, 0.0f };   // idle base for the line-tint coupling (gen-2): gpu.color = lerp(spawn, stim, mix)
    float spawn_lateral_amp = 0.0f;   // seed-drawn wave amps — the amp pipes'
    float spawn_vertical_amp = 0.0f;  //   idle bases (gpu = base × pipe mult)
    float phase = 0.0f;   // sway phase clock — integrates at the tempo follower's rate (100 BPM ⇒ wall seconds exactly)

    // ── Wander (autonomous drift) ── rolled at commit; a wanderer authors the
    // same yaw/throttle inputs the player does, through the same steering
    // integrator. SEAM[ribbon:sky-mode].
    bool     wander = false;
    float    wander_cruise = 0.0f;      // throttle fraction of RIBBON_MAX_SPEED
    float    wander_tx = 0.0f;          // current waypoint (world XZ)
    float    wander_tz = 0.0f;
    float    wander_retarget = 0.0f;    // seconds until a new waypoint
    uint32_t wander_rng = 1u;           // self-contained xorshift state
    float    wander_yaw_state = 0.0f;   // eased steering output (curvature continuity)
};

// ── The head ──────────────────────────────────────────────────────
// The live instrument: integrated position, heading, the altitude
// pen, and the propagation history the body is rebuilt from. ONE head
// exists (the rendered ribbon wears it); identity is (seeded, slot),
// and since slots are reused, REBIRTH is signalled by
// ribbon_invalidate_head — commit sends it (see the succession note
// there).
struct RibbonHead {
    bool     seeded = false;
    uint32_t slot = UINT32_MAX;
    float    origin[3] = { 0.0f, 0.0f, 0.0f };
    float    alt_target = 0.0f;   // low-passed altitude target (landscape swells, not texture)
    float    y_vel = 0.0f;        // the pen's vertical velocity (critically damped follower)
    bool     alt_baked = false;   // birthright latched? (re-bakes until the ground sample is warm)
    float    heading = 0.0f;               // sky-flight heading (yawed by input)
    float    pos[3] = { 0.0f, 0.0f, 0.0f };  // live integrated head position
    float    mount[3] = { 0.0f, 0.0f, 0.0f }; // visible head-ring center + half-tube (pawn mount point)
    // The saddle's FRAME (BNK-2): the three angles the mounted pawn
    // leans with — same math as the GPU ring frames (MOUNT_* mirrors),
    // sampled at the saddle's arc position. Shipped to the pawn kernel
    // through the frame signal's sky block. Zeros = level (identity).
    float    mount_yaw_off = 0.0f;  // tangent-align yaw deflection (rad)
    float    mount_pitch   = 0.0f;  // tangent-align pitch (rad)
    float    mount_roll    = 0.0f;  // bank into the lateral swing (rad, clamped)

    // ── Propagation history ── the body is the head's past, replayed at
    // propagation speed: ring k wears the head's state from
    // age = k·spacing/P seconds ago. Two channels suffice (heading, y);
    // XZ is reconstructed by integrating the delayed heading tailward.
    static constexpr float    HIST_DT  = 0.05f;  // sample cadence (s)
    static constexpr uint32_t HIST_CAP = 1024u;  // ~51 s of past > max body age (~29 s = RIBBON_MAX_LENGTH / slowest tier P — grow this if the cap grows)
    std::array<float, HIST_CAP> hist_heading{};
    std::array<float, HIST_CAP> hist_y{};
    uint32_t hist_head = 0;    // ring buffer: newest sample index
    float    hist_time = 0.0f; // time of the newest sample
};

// ── Ribbon module state ──────────────────────────────────────────
// Most ribbon-owned state lives in this struct, accessed via
// ribbon_state_ on the Cartridge (declared at the composition root) —
// the exceptions are the four ribbon canvas bindings and
// player_.sky_yaw_eased, which live on the Cartridge (the conductor
// writes them). Module functions take `RibbonState& rs` explicitly
// rather than reaching via Cartridge*, making ownership
// language-visible and dependencies explicit in signatures.
struct RibbonState {
    ActiveRibbon   active[MAX_RIBBON_INSTANCES]{};
    uint32_t       active_count = 0;

    // GPU-state CPU mirror. Per-frame, the spine picks the nearest
    // ribbon to the pawn and uploads its slot to the GPU.
    // rendered_slot is the currently rendered slot (UINT32_MAX = none).
    GPURibbonState gpu[MAX_RIBBON_INSTANCES]{};
    uint32_t       rendered_slot = UINT32_MAX;

    // The head mover — see RibbonHead above.
    RibbonHead     head{};

    // Mood-5 ribbon anchor offset. Seed-derived position centered on
    // the finite world. Adjust to manually shift the mood-5 anchor XZ
    // (read by mood.inl::apply_mood).
    float          mood_offset[2] = { 0.0f, 0.0f };
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════
//
// DEFINED in ribbon.inl (post-class, self-wrapping) — the bodies reach
// the keyhole (gpuState_/time_state_/player_/inputState_/
// visual_canvas_ + the ribbon canvas bindings and the spine services)
// and in-class statics (Cartridge::THEMES / Cartridge::PATCH_EXTENT)
// via the complete type; PopFamily is roster.hpp vocabulary. wander_rand01,
// ribbon_wander_inputs, ribbon_history_sample and
// ribbon_rebuild_body_upload are module-internal (impl-only, not
// declared here).

// Lifecycle (three-phase)
bool select_ribbon_for_patch(RibbonState& rs, Cartridge* c,
    int32_t gx, int32_t gz, RibbonSelection& sel);
bool place_ribbon_from_selection(Cartridge* c,
    const RibbonSelection& sel, RibbonPlacement& plan);
void commit_ribbon(RibbonState& rs, Cartridge* c,
    const RibbonPlacement& plan,
    int32_t trigger_gx, int32_t trigger_gz, wgpu::Queue& queue);
// The evictor — lifecycle, absorbed per §5 EVICTION THUNKS; keyhole-shaped
// to match the FAMILY_DISPATCH evict slot (table in family_dispatch.inl);
// carries the sky-mode pin (SEAM[ribbon:sky-mode]) and ref-count law
void evict_ribbon(Cartridge* self, uint32_t slot, wgpu::Queue& queue);
// Dispatch funnels (table-shaped; the FAMILY_DISPATCH rows point here)
bool dispatch_select_ribbon(Cartridge* self, int32_t gx, int32_t gz, EntityQueueEntry& e);
bool dispatch_place_ribbon(Cartridge* self, EntityQueueEntry& e, PlacementEntry& pe);
void dispatch_commit_ribbon(Cartridge* self, PlacementEntry& pe, wgpu::Queue& queue);
// Frame conductor
void ribbon_frame_tick(RibbonState& rs, Cartridge* c, wgpu::Queue& queue);
// Head mover
void ribbon_advance_head(RibbonState& rs, GPUState& gpuState,
    wgpu::Queue& queue, const GPURibbonState& ribbon,
    uint32_t slot, float t,
    bool flown, float yaw_in, float throttle_in, float dt,
    float ground_y, bool ground_valid);
void ribbon_head_pose(const RibbonState& rs, float& x, float& y, float& z, float& heading);
void ribbon_head_frame(const RibbonState& rs, float& yaw_off, float& pitch, float& roll);
void ribbon_head_pen(const RibbonState& rs, float& x, float& z, float& heading);
void ribbon_invalidate_head(RibbonState& rs);
bool ribbon_head_is(const RibbonState& rs, uint32_t slot);
// Shared geometry helper (dual-entry: dispatch + mood-5 forced spawn)
void fill_ribbon_selection_geometry(uint32_t seed, uint32_t tier_idx,
    RibbonSelection& sel);

} // namespace the_board
} // namespace t7
