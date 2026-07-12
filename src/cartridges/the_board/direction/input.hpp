#pragma once
#include <cstdint>
#include "cartridges/the_board/contracts/keyhole.hpp"  // Cartridge + wgpu::Queue fwds (the keyhole)

// ─── input.hpp (HEADER: state + decls) ────────────────────────────
// Converted (LADDER-3 c6, G2): history in audit/LADDER.md.
//
// The home of input dispatch.
// ── A note on bindings (READ ME if changing keys) ─────────────────
// ──────────────────────────────────────────────────────────────────
//
// The impl reaches GLFW (<GLFW/glfw3.h>, its own include), the
// keyhole's organs (inputState_ / keys_ / mouse_ / player_ /
// world_state_ / device_ / gpuState_ / pawn_state_ / agent_state_ /
// orbs_state_ / cube_behaviors_state_), the converted modules' commands
// (agents.hpp / orbs.hpp / cube_behaviors.hpp / floater_vocabulary.hpp
// / mood.hpp's request_mood_transition) + mood_constants.hpp's MOOD_*
// IDs, and the patch radii (GRID_RADIUS / PREGEN_RADIUS —
// patch_system.hpp vocabulary).
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

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

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════

// GLFW callbacks (routed by the spine's on_input override)
void on_key_down(Cartridge* c, int key);
void on_key_up(Cartridge* c, int key);
void on_mouse_move(Cartridge* c, float dx, float dy);
void on_mouse_button(Cartridge* c, int button, bool pressed);
void on_scroll(Cartridge* c, float delta);
// Per-frame
void update_movement_intent(Cartridge* c);
void clear_input_deltas(Cartridge* c);
// Camera / view commands
void toggle_fpv_mode(Cartridge* c);
void toggle_sky_mode(Cartridge* c);
void set_render_radius(Cartridge* c, uint32_t r);

} // namespace the_board
} // namespace t7
