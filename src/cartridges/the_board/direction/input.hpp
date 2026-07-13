#pragma once
#include <cstdint>
#include "cartridges/the_board/contracts/spine_state.hpp"      // PlayerState (the anchor's organ) + TransitionPhase (the transition channel)
#include "cartridges/the_board/contracts/mood_constants.hpp"   // MOOD_* IDs (the mood keys) + PortalDestination

// ─── input.hpp (HEADER: state + deps + decls) ─────────────────────
// Converted (LADDER-3 c6, G2); keyhole dissolved (DISSOLVE-1 Batch C):
// history in audit/LADDER.md.
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

// ═══ INPUT STATE ═════════════════════════════════════════════════

struct InputState {
    float move_x = 0.0f;
    float move_z = 0.0f;
    float look_az_delta = 0.0f;
    float look_el_delta = 0.0f;
    float zoom_delta = 0.0f;
    float pan_x_delta = 0.0f;
    float pan_y_delta = 0.0f;
};

// Held movement keys (arrow keys) — update_movement_intent folds
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
void set_render_radius(InputDeps* c, uint32_t r);

} // namespace the_board
} // namespace t7
