#pragma once
#include <cstdint>
#include "cartridges/the_board/contracts/spine_state.hpp"      // PlayerState (the anchor's organ) + TransitionPhase (the transition channel) + InputState (graduated)
#include "cartridges/the_board/contracts/mood_constants.hpp"   // MOOD_* IDs (the mood keys) + PortalDestination
#include "cartridges/the_board/contracts/point.hpp"             // PointState/PointHost (the point — the driver toggles its host; PANEL-0 p1a)
#include <algorithm>       // std::max, std::min   // (impl, merged)
#include <cmath>           // std::sqrt   // (impl, merged)
#include <iostream>        // toggle / radius logs   // (impl, merged)
#include <GLFW/glfw3.h>    // the key codes (unpapered — c6)   // (impl, merged)

// ─── input.hpp (MERGED: state + deps + decls + impl) ──────────────
// Converted (LADDER-3 c6, G2); keyhole dissolved + hpp/inl MERGED
// (DISSOLVE-1 Batch C — input.inl retired): history in audit/LADDER.md.
// COHORT: after ribbon.hpp (toggle_sky_mode derefs RibbonState.sky) +
// the door owners (pawn/orbs/agents/cube 66-71); InputState graduated
// to contracts/spine_state.hpp (it precedes ribbon in the cohort).
//
// The home of input dispatch.
// ── A note on bindings (READ ME if changing keys) ─────────────────
// ──────────────────────────────────────────────────────────────────
//
// The impl reaches GLFW (<GLFW/glfw3.h>, its own include), the deps
// face below (inputState_ / keys_ / mouse_ / player_ / world_state_ /
// ribbon_state_ / gpuState_ / device_), the command fan's TARGET
// organs (on_key_down's parameters — pawn / orbs / agents / cubes +
// the transition channel), the m4 command doors (pawn.hpp / orbs.hpp
// / agents.hpp / cube_behaviors.hpp / mood.hpp's
// request_mood_transition / patch_system.hpp's request_recenter) +
// mood_constants.hpp's MOOD_* IDs, and the patch radii (GRID_RADIUS /
// PREGEN_RADIUS — patch_system.hpp vocabulary).
// ─────────────────────────────────────────────────────────────────

// LOCKSTEP INSURANCE (keyhole.hpp's form): the Device declaration
// mirrors webgpu_cpp.h's (`class Device`, in namespace wgpu). The
// deps face holds the queue-fetch handle (the S5 pattern, gol).
namespace wgpu { class Device; }

namespace t7 {
namespace the_board {

// fwd — the driver's true reaches (InputDeps reference members) and
// the command fan's target organs (reference params in on_key_down's
// declaration); complete types arrive with their owners in the cohort.
struct WorldState; struct RibbonState; class GPUState;
struct PawnState;  struct PawnDeps;
struct OrbsState;  struct OrbsDeps;
struct AgentState; struct AgentsDeps;
struct CubeBehaviorsState; struct CubeDeps;
struct MoodState;

// ═══ CameraControls — PARAMETER PANEL (PANEL-0 p1a; the campaign's
// first, deliberately MINIMAL — the FORM TEST for p3's per-module
// panels: one organized block, clear names, editable without hunting.
// Terrain inherits this convention.) ═══════════════════════════════
//
// Deferred growth (named, not carried): smoothing/damping, invert-Y,
// sprint multiplier, split H/V sensitivity, a vertical-lift axis.
struct CameraControls {
    // Mouse → rotation rate (radians per pixel of drag). Feeds every
    // mouse-authored look/pan delta (on_mouse_move).
    static constexpr float LOOK_SENSITIVITY = 0.005f;
    // W/S/A/D velocity in free-fly (world units per second) — the
    // camera host's MOVE_SPEED, wired to config.point_fly_speed at
    // boot. The pawn host's walk speed is Idle::PAWN_SPEED (state.hpp),
    // untouched by this dial.
    static constexpr float MOVE_SPEED = 15.0f;
};

// ═══ INPUT STATE ═════════════════════════════════════════════════
// struct InputState GRADUATED to contracts/spine_state.hpp (the
// driver's intent organ — read by the spine's signal fill and the
// ribbon's sky flight, so it precedes them in the cohort). KeyState /
// MouseState stay here: the driver's private organs.

// Held movement keys (W/S/A/D — THE UNIVERSAL MOVE CHANNEL, p1a-fix):
// one key set authors the driver's move-intent; WHOEVER HOSTS THE
// POINT consumes it, under that host's own constraint AND mapping —
// the constraint-and-mapping IS the host's behavioral identity (pawn:
// camera-relative full-directional, snap; camera: camera-relative
// full-directional, rule none; ribbon in sky mode: its own
// forward-biased grammar — a ribbon that could reverse and strafe
// wouldn't be a ribbon). Arrows RETIRED completely (census: no
// non-movement arrow use existed). update_movement_intent folds
// these into inputState_.move_x/move_z.
struct KeyState {
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
};

// Mouse drag state — on_mouse_move reads these to decide which
// deltas a drag writes.
struct MouseState {
    bool left_dragging = false;
    bool right_dragging = false;
};

// ═══ THE DEPS FACE (DISSOLVE-1 Batch C d2) ═══════════════════════
//
// Input's own organs plus its true reaches — the driver's face (v3
// §9 Act I: a driver writes intents through bodies it does not own).
// The command fan's TARGET organs are deliberately NOT members: they
// ride on_key_down's parameters — the root addresses the fan's
// targets at the call site, through the m4 command doors. THE F6
// SOCKET stays RESERVED (m4 registry): when a driver must address a
// body it does not own by synchronous command beyond this fan, the
// addressed-intent socket (v3 §9 Act II, §13) is where it routes.
struct InputDeps {
    InputState&   inputState_;
    KeyState&     keys_;
    MouseState&   mouse_;
    PlayerState&  player_;        // fpv — the anchor toggle (v3 §9 Act III)
    WorldState&   world_state_;   // active_radius — the radius command
    RibbonState&  ribbon_state_;  // sky.mode — the ribbon's fixture (m6, Option A)
    GPUState&     gpuState_;      // the freeze toggle + the fpv wire
    wgpu::Device& device_;        // the queue fetch (the S5-style declared handle)
    PointState&   point_;         // the point (PANEL-0 p1a) — the host toggle (key 4)
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════

// GLFW callbacks (routed by the spine's on_input override). The
// command fan's targets are parameters — organ-named, so the fan's
// door calls read as organ addressing (m4 registry order).
void on_key_down(InputDeps* c, int key,
    PawnState& pawn_state_, PawnDeps& pawn_deps_,
    OrbsState& orbs_state_, OrbsDeps& orbs_deps_,
    AgentState& agent_state_, AgentsDeps& agents_deps_,
    CubeBehaviorsState& cube_behaviors_state_, CubeDeps& cube_deps_,
    TransitionPhase& transitionPhase_, PortalDestination& pendingDestination_,
    MoodState& mood_state_);
void on_key_up(InputDeps* c, int key);
void on_mouse_move(InputDeps* c, float dx, float dy);
void on_mouse_button(InputDeps* c, int button, bool pressed);
void on_scroll(InputDeps* c, float delta);
// Per-frame
void update_movement_intent(InputDeps* c);
void clear_input_deltas(InputDeps* c);
// Camera / view commands
void toggle_fpv_mode(InputDeps* c);
void toggle_sky_mode(InputDeps* c);
void toggle_point_host(InputDeps* c);
void set_render_radius(InputDeps* c, uint32_t r);


// ═══ MODULE IMPLEMENTATION (merged; was input.inl) ═══════════════
//
// WRAPPING FORM history (fix-2) in audit/LADDER.md; the impl now
// rides its own header (DISSOLVE-1 Batch C). GLFW is named here —
// the dependency is the module's, not inherited from the host TU.
// The fallback #defines below are preprocessor — namespace-blind,
// byte-carried from the .inl.

// ═══ KEY BINDING REGISTRY ════════════════════════════════════════
//
// ── Movement (held; W/S/A/D — the universal move channel) ────────
//
// ── World / aura toggles ─────────────────────────────────────────
//
// ── Orb utilities (numpad) ───────────────────────────────────────
//
// ── Camera / possession ──────────────────────────────────────────
//
// ── Diagnostics (function keys) ──────────────────────────────────
//
// ── Mouse / scroll ───────────────────────────────────────────────
// ─────────────────────────────────────────────────────────────────

// ═══ GLFW KEY CODE FALLBACKS ═════════════════════════════════════

#ifndef GLFW_KEY_KP_0
#define GLFW_KEY_KP_0  320
#endif
#ifndef GLFW_KEY_KP_1
#define GLFW_KEY_KP_1  321
#endif
#ifndef GLFW_KEY_KP_2
#define GLFW_KEY_KP_2  322
#endif
#ifndef GLFW_KEY_KP_3
#define GLFW_KEY_KP_3  323
#endif
#ifndef GLFW_KEY_KP_4
#define GLFW_KEY_KP_4  324
#endif
#ifndef GLFW_KEY_KP_5
#define GLFW_KEY_KP_5  325
#endif
#ifndef GLFW_KEY_KP_6
#define GLFW_KEY_KP_6  326
#endif
#ifndef GLFW_KEY_KP_7
#define GLFW_KEY_KP_7  327
#endif
#ifndef GLFW_KEY_KP_8
#define GLFW_KEY_KP_8  328
#endif
#ifndef GLFW_KEY_KP_9
#define GLFW_KEY_KP_9  329
#endif
#ifndef GLFW_KEY_KP_DECIMAL
#define GLFW_KEY_KP_DECIMAL  330
#endif
#ifndef GLFW_KEY_LEFT_CONTROL
#define GLFW_KEY_LEFT_CONTROL   341
#endif
#ifndef GLFW_KEY_RIGHT_CONTROL
#define GLFW_KEY_RIGHT_CONTROL  345
#endif
#ifndef GLFW_KEY_CAPS_LOCK
#define GLFW_KEY_CAPS_LOCK   280
#endif
#ifndef GLFW_KEY_4
#define GLFW_KEY_4  52
#endif
#ifndef GLFW_KEY_W
#define GLFW_KEY_W  87
#endif
#ifndef GLFW_KEY_A
#define GLFW_KEY_A  65
#endif
#ifndef GLFW_KEY_S
#define GLFW_KEY_S  83
#endif
#ifndef GLFW_KEY_D
#define GLFW_KEY_D  68
#endif
#ifndef GLFW_KEY_F1
#define GLFW_KEY_F1  290
#endif
#ifndef GLFW_KEY_F2
#define GLFW_KEY_F2  291
#endif
#ifndef GLFW_KEY_F3
#define GLFW_KEY_F3  292
#endif
#ifndef GLFW_KEY_F4
#define GLFW_KEY_F4  293
#endif
#ifndef GLFW_KEY_F5
#define GLFW_KEY_F5  294
#endif
#ifndef GLFW_KEY_F6
#define GLFW_KEY_F6  295
#endif
#ifndef GLFW_KEY_F7
#define GLFW_KEY_F7  296
#endif
#ifndef GLFW_KEY_F8
#define GLFW_KEY_F8  297
#endif


// ═══ KEY DISPATCH ════════════════════════════════════════════════

inline void on_key_down(InputDeps* c, int key,
    PawnState& pawn_state_, PawnDeps& pawn_deps_,
    OrbsState& orbs_state_, OrbsDeps& orbs_deps_,
    AgentState& agent_state_, AgentsDeps& agents_deps_,
    CubeBehaviorsState& cube_behaviors_state_, CubeDeps& cube_deps_,
    TransitionPhase& transitionPhase_, PortalDestination& pendingDestination_,
    MoodState& mood_state_)
{
    // Single queue fetch: every queue-using case below reuses this.
    wgpu::Queue q = c->device_.GetQueue();

    switch (key) {

    // ── Movement ─────────────────────────────────────────────────
    case GLFW_KEY_W: c->keys_.forward = true;  break;
    case GLFW_KEY_S: c->keys_.backward = true; break;
    case GLFW_KEY_A: c->keys_.left = true;     break;
    case GLFW_KEY_D: c->keys_.right = true;    break;

    // ── World / aura toggles ─────────────────────────────────────
    case GLFW_KEY_1:
        c->gpuState_.toggle_freeze_sphere();
        break;
    case GLFW_KEY_2: toggle_aura_height(pawn_state_, &pawn_deps_);  break;  // pawn command door (m4)
    case GLFW_KEY_3: toggle_aura(pawn_state_, &pawn_deps_);          break;  // pawn command door (m4)
    case GLFW_KEY_4: toggle_point_host(c);                                break;  // the point's host: pawn (kite) <-> camera (free-fly) — PANEL-0 p1a
    case GLFW_KEY_5: request_mood_transition(transitionPhase_, pendingDestination_, mood_state_, c->world_state_, MOOD_OPEN_SUNSET);        break;
    case GLFW_KEY_6: request_mood_transition(transitionPhase_, pendingDestination_, mood_state_, c->world_state_, MOOD_INDOOR_FLAT);        break;
    case GLFW_KEY_7: request_mood_transition(transitionPhase_, pendingDestination_, mood_state_, c->world_state_, MOOD_INDOOR_VAULT);       break;
    case GLFW_KEY_8: request_mood_transition(transitionPhase_, pendingDestination_, mood_state_, c->world_state_, MOOD_FINITE_OUTDOOR);     break;
    case GLFW_KEY_9: request_mood_transition(transitionPhase_, pendingDestination_, mood_state_, c->world_state_, MOOD_FINITE_OUTDOOR_REF); break;
    case GLFW_KEY_0:              cycle_orb_palette(orbs_state_, &orbs_deps_, q);          break;
    case GLFW_KEY_LEFT_BRACKET:   set_render_radius(c, c->world_state_.active_radius - 1); break;
    case GLFW_KEY_RIGHT_BRACKET:  set_render_radius(c, c->world_state_.active_radius + 1); break;

    // ── Orb utilities (numpad) ───────────────────────────────────
    case GLFW_KEY_KP_8:       cycle_orb_motion_rule(orbs_state_, &orbs_deps_, q);            break;
    // KP_9 freed (p1b-e): the dome anchor toggle retired — the dome is
    // a skybox, eye-centered always.
    case GLFW_KEY_KP_DECIMAL: cycle_orb_gesture(orbs_state_, &orbs_deps_, q);                break;

    // ── Camera / possession ──────────────────────────────────────
    case GLFW_KEY_LEFT_CONTROL:
    case GLFW_KEY_RIGHT_CONTROL:
        toggle_fpv_mode(c);
        break;
    case GLFW_KEY_CAPS_LOCK:  try_possess_nearest(agent_state_, &agents_deps_, q);  break;

    // ── Diagnostics (function keys) ──────────────────────────────
    case GLFW_KEY_F1: cycle_agent_behavior_override(agent_state_, &agents_deps_, q);  break;
    case GLFW_KEY_F2: cycle_agent_tier_override(agent_state_, &agents_deps_, q);      break;
    case GLFW_KEY_F3: force_respawn_population(agent_state_, &agents_deps_, q);       break;
    case GLFW_KEY_F4: cycle_cube_behavior_override(cube_behaviors_state_, &cube_deps_, q);   break;
    case GLFW_KEY_F5: cycle_floater_coordination(cube_behaviors_state_, &cube_deps_);        break;
    case GLFW_KEY_F6: corral_cubes(cube_behaviors_state_, &cube_deps_, q);                   break;
    case GLFW_KEY_F7: toggle_cube_kite_mode(cube_behaviors_state_, &cube_deps_, q);          break;
    case GLFW_KEY_F8:
        // ROSTER-GATE ribbon (b) — D9 (REBUILD-0 stamp): sky-flight's entry
        // door rides the ribbon bit. Ungated, F8 in a ribbon-less demo snaps
        // the pawn to origin (the fail-LOUD zeros turned player-facing —
        // recon §5). m6 re-homes the machinery (Option A); this closes the
        // trap early.
        if constexpr (ROSTER.ribbon) toggle_sky_mode(c);
        break;
    }
    update_movement_intent(c);
}

inline void on_key_up(InputDeps* c, int key) {
    switch (key) {
    case GLFW_KEY_W: c->keys_.forward = false;  break;
    case GLFW_KEY_S: c->keys_.backward = false; break;
    case GLFW_KEY_A: c->keys_.left = false;     break;
    case GLFW_KEY_D: c->keys_.right = false;    break;
    }
    update_movement_intent(c);
}

// ═══ MOUSE / SCROLL ══════════════════════════════════════════════

inline void on_mouse_move(InputDeps* c, float dx, float dy) {
    constexpr float sensitivity = CameraControls::LOOK_SENSITIVITY;  // the panel dial (PANEL-0 p1a)
    if (c->mouse_.left_dragging) {
        c->inputState_.look_az_delta += dx * sensitivity;
        c->inputState_.look_el_delta += dy * sensitivity;
    }
    if (c->mouse_.right_dragging) {
        c->inputState_.pan_x_delta += dx * sensitivity;
        c->inputState_.pan_y_delta -= dy * sensitivity;
    }
}

inline void on_mouse_button(InputDeps* c, int button, bool pressed) {
    if (button == 0) c->mouse_.left_dragging = pressed;
    if (button == 1) c->mouse_.right_dragging = pressed;
}

inline void on_scroll(InputDeps* c, float delta) {
    c->inputState_.zoom_delta -= delta * 2.0f;
}

// ═══ MOVEMENT INTENT + DELTA CLEAR ═══════════════════════════════

inline void update_movement_intent(InputDeps* c) {
    c->inputState_.move_x = 0.0f;
    c->inputState_.move_z = 0.0f;

    // THE UNIVERSAL MOVE CHANNEL (p1a-fix): W/S/A/D author this fold;
    // consumption is host-routed downstream (the pawn kernel's
    // point_camera_hosted guard, the camera's fly branch, the ribbon's
    // sky steering) — no CPU-side routing needed with one key set.
    if (c->keys_.forward)  c->inputState_.move_z -= 1.0f;
    if (c->keys_.backward) c->inputState_.move_z += 1.0f;
    if (c->keys_.left)     c->inputState_.move_x -= 1.0f;
    if (c->keys_.right)    c->inputState_.move_x += 1.0f;

    float len = std::sqrt(c->inputState_.move_x * c->inputState_.move_x +
        c->inputState_.move_z * c->inputState_.move_z);
    if (len > 1.0f) {
        c->inputState_.move_x /= len;
        c->inputState_.move_z /= len;
    }
}

inline void clear_input_deltas(InputDeps* c) {
    c->inputState_.look_az_delta = 0.0f;
    c->inputState_.look_el_delta = 0.0f;
    c->inputState_.zoom_delta = 0.0f;
    c->inputState_.pan_x_delta = 0.0f;
    c->inputState_.pan_y_delta = 0.0f;
}

// ═══ CAMERA / VIEW COMMANDS ══════════════════════════════════════

inline void toggle_fpv_mode(InputDeps* c) {
    c->player_.fpv_mode = !c->player_.fpv_mode;
    c->gpuState_.set_fpv_mode(c->player_.fpv_mode ? 1 : 0);
    std::cout << "[the_board] Camera mode: "
        << (c->player_.fpv_mode ? "First-Person View" : "Orbit") << std::endl;
}

// Sky-flight toggle. While ON, the move channel (W/S = throttle,
// A/D = yaw) steers the rendered ribbon's head — the ribbon consumes
// the ONE channel under its own forward-biased grammar (p1a-fix: the
// host owns its mapping; sky mode's arrow special case retired into
// the general rule); the sky altitude
// is held by a critically damped pen, not fixed. While OFF, the ribbon
// holds its stationary arc. The pawn snap and camera follow have landed;
// only the fade transition remains unbuilt. SEAM[ribbon:sky-mode].
inline void toggle_sky_mode(InputDeps* c) {
    c->ribbon_state_.sky.mode = !c->ribbon_state_.sky.mode;  // the ribbon's fixture (m6, Option A)
    std::cout << "[the_board] Sky mode: "
        << (c->ribbon_state_.sky.mode ? "ON (fly the ribbon with W/S/A/D)" : "OFF") << std::endl;
}

// The point's host toggle (PANEL-0 p1a) — the driver's iteration
// tool, key 4: PAWN (the kite — today's follow, byte-untouched) <->
// CAMERA (free-fly: W/A/S/D + mouse, terrain rule NONE, bubble
// sensors dormant). Not roster-gated: the camera always exists.
inline void toggle_point_host(InputDeps* c) {
    c->point_.host = (c->point_.host == PointHost::PAWN)
        ? PointHost::CAMERA : PointHost::PAWN;
    c->gpuState_.set_point_host(static_cast<uint32_t>(c->point_.host));
    std::cout << "[Point] Host: "
        << (c->point_.host == PointHost::CAMERA ? "CAMERA (free-fly)" : "PAWN (the kite)") << "\n";
}

inline void set_render_radius(InputDeps* c, uint32_t r) {
    r = std::max(r, GRID_RADIUS);
    r = std::min(r, PREGEN_RADIUS);
    if (r == c->world_state_.active_radius) return;
    c->world_state_.active_radius = r;
    uint32_t side = 2 * r + 1;
    std::cout << "[the_board] Render radius: " << r
        << " (" << side << "x" << side << " = " << side * side << " patches)" << std::endl;
    // Force full re-evaluation on next frame — through the owner's door (m4)
    request_recenter(c->world_state_);
}

} // namespace the_board
} // namespace t7
