// ─── input.inl (IMPL: post-class definitions) ────────────────────
// Impl of input.hpp (LADDER-3 c6): history in audit/LADDER.md.
//
// Definitions for input.hpp's declared dispatch + per-frame + command
// functions. The bodies reach c->inputState_ / c->keys_ / c->mouse_ /
// c->player_ / c->world_state_ / c->device_ / c->gpuState_ /
// c->pawn_state_ / c->agent_state_ / c->orbs_state_ /
// c->cube_behaviors_state_, the mood door request_mood_transition
// (mood.hpp), and the patch radii (GRID_RADIUS / PREGEN_RADIUS —
// patch_system.hpp vocabulary).
//
// This impl includes <GLFW/glfw3.h> itself — the dependency is named here, not inherited from the host TU.
//
// WRAPPING FORM (fix-2): SELF-WRAPPING — the zone includes impls at FILE SCOPE; law in audit/LADDER.md.
// ─────────────────────────────────────────────────────────────────

#include <algorithm>       // std::max, std::min
#include <cmath>           // std::sqrt
#include <cstdint>
#include <iostream>        // toggle / radius logs
#include <GLFW/glfw3.h>    // the key codes (unpapered — c6)

// ═══ KEY BINDING REGISTRY ════════════════════════════════════════
//
// ── Movement (held; arrow keys) ──────────────────────────────────
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

namespace t7 {
namespace the_board {

// ═══ KEY DISPATCH ════════════════════════════════════════════════

inline void on_key_down(Cartridge* c, int key) {
    // Single queue fetch: every queue-using case below reuses this.
    wgpu::Queue q = c->device_.GetQueue();

    switch (key) {

    // ── Movement ─────────────────────────────────────────────────
    case GLFW_KEY_UP:    c->keys_.forward = true;  break;
    case GLFW_KEY_DOWN:  c->keys_.backward = true; break;
    case GLFW_KEY_LEFT:  c->keys_.left = true;     break;
    case GLFW_KEY_RIGHT: c->keys_.right = true;    break;

    // ── World / aura toggles ─────────────────────────────────────
    case GLFW_KEY_1:
        c->gpuState_.toggle_freeze_sphere();
        break;
    case GLFW_KEY_2:
        c->pawn_state_.aura_height_enabled = !c->pawn_state_.aura_height_enabled;
        c->pawn_state_.aura_cfg_dirty = true;
        std::cout << "[Aura] Height extrusion: " << (c->pawn_state_.aura_height_enabled ? "ON" : "OFF") << "\n";
        break;
    case GLFW_KEY_3:
        c->pawn_state_.aura_enabled = !c->pawn_state_.aura_enabled;
        c->pawn_state_.aura_cfg_dirty = true;
        std::cout << "[Aura] Field: " << (c->pawn_state_.aura_enabled ? "ON" : "OFF") << "\n";
        break;
    case GLFW_KEY_5: request_mood_transition(c, MOOD_OPEN_SUNSET);        break;
    case GLFW_KEY_6: request_mood_transition(c, MOOD_INDOOR_FLAT);        break;
    case GLFW_KEY_7: request_mood_transition(c, MOOD_INDOOR_VAULT);       break;
    case GLFW_KEY_8: request_mood_transition(c, MOOD_FINITE_OUTDOOR);     break;
    case GLFW_KEY_9: request_mood_transition(c, MOOD_FINITE_OUTDOOR_REF); break;
    case GLFW_KEY_0:              cycle_orb_palette(c->orbs_state_, c, q);          break;
    case GLFW_KEY_LEFT_BRACKET:   set_render_radius(c, c->world_state_.active_radius - 1); break;
    case GLFW_KEY_RIGHT_BRACKET:  set_render_radius(c, c->world_state_.active_radius + 1); break;

    // ── Orb utilities (numpad) ───────────────────────────────────
    case GLFW_KEY_KP_8:       cycle_orb_motion_rule(c->orbs_state_, c, q);            break;
    case GLFW_KEY_KP_9:       toggle_orb_anchor(c->orbs_state_, c);                 break;
    case GLFW_KEY_KP_DECIMAL: cycle_orb_gesture(c->orbs_state_, c, q);                break;

    // ── Camera / possession ──────────────────────────────────────
    case GLFW_KEY_LEFT_CONTROL:
    case GLFW_KEY_RIGHT_CONTROL:
        toggle_fpv_mode(c);
        break;
    case GLFW_KEY_CAPS_LOCK:  try_possess_nearest(c->agent_state_, c, q);  break;

    // ── Diagnostics (function keys) ──────────────────────────────
    case GLFW_KEY_F1: cycle_agent_behavior_override(c->agent_state_, c, q);  break;
    case GLFW_KEY_F2: cycle_agent_tier_override(c->agent_state_, c, q);      break;
    case GLFW_KEY_F3: force_respawn_population(c->agent_state_, c, q);       break;
    case GLFW_KEY_F4: cycle_cube_behavior_override(c->cube_behaviors_state_, c, q);   break;
    case GLFW_KEY_F5: cycle_floater_coordination(c->cube_behaviors_state_, c);        break;
    case GLFW_KEY_F6: corral_cubes(c->cube_behaviors_state_, c, q);                   break;
    case GLFW_KEY_F7: toggle_cube_kite_mode(c->cube_behaviors_state_, c, q);          break;
    case GLFW_KEY_F8: toggle_sky_mode(c);                                             break;
    }
    update_movement_intent(c);
}

inline void on_key_up(Cartridge* c, int key) {
    switch (key) {
    case GLFW_KEY_UP:    c->keys_.forward = false;  break;
    case GLFW_KEY_DOWN:  c->keys_.backward = false; break;
    case GLFW_KEY_LEFT:  c->keys_.left = false;     break;
    case GLFW_KEY_RIGHT: c->keys_.right = false;    break;
    }
    update_movement_intent(c);
}

// ═══ MOUSE / SCROLL ══════════════════════════════════════════════

inline void on_mouse_move(Cartridge* c, float dx, float dy) {
    constexpr float sensitivity = 0.005f;
    if (c->mouse_.left_dragging) {
        c->inputState_.look_az_delta += dx * sensitivity;
        c->inputState_.look_el_delta += dy * sensitivity;
    }
    if (c->mouse_.right_dragging) {
        c->inputState_.pan_x_delta += dx * sensitivity;
        c->inputState_.pan_y_delta -= dy * sensitivity;
    }
}

inline void on_mouse_button(Cartridge* c, int button, bool pressed) {
    if (button == 0) c->mouse_.left_dragging = pressed;
    if (button == 1) c->mouse_.right_dragging = pressed;
}

inline void on_scroll(Cartridge* c, float delta) {
    c->inputState_.zoom_delta -= delta * 2.0f;
}

// ═══ MOVEMENT INTENT + DELTA CLEAR ═══════════════════════════════

inline void update_movement_intent(Cartridge* c) {
    c->inputState_.move_x = 0.0f;
    c->inputState_.move_z = 0.0f;

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

inline void clear_input_deltas(Cartridge* c) {
    c->inputState_.look_az_delta = 0.0f;
    c->inputState_.look_el_delta = 0.0f;
    c->inputState_.zoom_delta = 0.0f;
    c->inputState_.pan_x_delta = 0.0f;
    c->inputState_.pan_y_delta = 0.0f;
}

// ═══ CAMERA / VIEW COMMANDS ══════════════════════════════════════

inline void toggle_fpv_mode(Cartridge* c) {
    c->player_.fpv_mode = !c->player_.fpv_mode;
    c->gpuState_.set_fpv_mode(c->player_.fpv_mode ? 1 : 0);
    std::cout << "[the_board] Camera mode: "
        << (c->player_.fpv_mode ? "First-Person View" : "Orbit") << std::endl;
}

// Sky-flight toggle. While ON, the arrow keys steer the rendered
// ribbon's head (up/down = throttle, left/right = yaw); the sky altitude
// is held by a critically damped pen, not fixed. While OFF, the ribbon
// holds its stationary arc. The pawn snap and camera follow have landed;
// only the fade transition remains unbuilt. SEAM[ribbon:sky-mode].
inline void toggle_sky_mode(Cartridge* c) {
    c->player_.sky_mode = !c->player_.sky_mode;
    std::cout << "[the_board] Sky mode: "
        << (c->player_.sky_mode ? "ON (fly the ribbon with arrows)" : "OFF") << std::endl;
}

inline void set_render_radius(Cartridge* c, uint32_t r) {
    r = std::max(r, GRID_RADIUS);
    r = std::min(r, PREGEN_RADIUS);
    if (r == c->world_state_.active_radius) return;
    c->world_state_.active_radius = r;
    uint32_t side = 2 * r + 1;
    std::cout << "[the_board] Render radius: " << r
        << " (" << side << "x" << side << " = " << side * side << " patches)" << std::endl;
    // Force full re-evaluation on next frame
    c->world_state_.last_center_x = INT32_MAX;
    c->world_state_.last_center_z = INT32_MAX;
}

} // namespace the_board
} // namespace t7
