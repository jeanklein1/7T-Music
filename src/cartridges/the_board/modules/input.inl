// ─── input.inl (IMPL: post-class definitions) ────────────────────
//
// Definitions for input.hpp's declared dispatch + per-frame + command
// functions. Included AFTER the Cartridge class (LADDER-3 c6 header/impl
// split) so the keyhole is a complete type — the bodies reach
// c->inputState_ / c->keys_ / c->mouse_ / c->player_ / c->world_state_ /
// c->device_ / c->gpuState_ / c->pawn_state_ / c->agent_state_ /
// c->orbs_state_ / c->cube_behaviors_state_, the mood door
// request_mood_transition (mood.hpp — K4 ruled, LADDER-4), and the in-class
// statics (Cartridge::GRID_RADIUS / Cartridge::PREGEN_RADIUS) via the
// complete type — the keyhole's static form (the c6 retrofit: the
// module's ambient reads become keyhole reads; no other body changes).
//
// THE UNPAPERED DEPENDENCY (c6): this impl includes <GLFW/glfw3.h>
// itself. Under the class-body law the GLFW key codes leaked in from
// whichever host TU included the cartridge after GLFW — a papered
// dependency. The include below names it; the #ifndef fallbacks stay
// for GLFW variants that omit the numpad/function-key constants.
//
// WRAPPING FORM (the proven fix-2 rule): SELF-WRAPPING — opens
// t7::the_board itself, carries its own standard includes; the MODULE
// IMPLEMENTATIONS zone includes it at FILE SCOPE. Definitions are
// `inline` free functions.
// ─────────────────────────────────────────────────────────────────

#include <algorithm>       // std::max, std::min
#include <cmath>           // std::sqrt
#include <cstdint>
#include <iostream>        // toggle / radius logs
#include <GLFW/glfw3.h>    // the key codes (unpapered — c6)

// ═══ KEY BINDING REGISTRY ════════════════════════════════════════
//
// What every key does, organized by purpose. Bindings are temporary
// (see the note in input.hpp's banner); functions are durable. When
// changing what a key does, change both this table and the dispatch
// below.
//
// ── Movement (held; arrow keys) ──────────────────────────────────
//   ↑              keys_.forward                — move pawn forward
//   ↓              keys_.backward               — move pawn backward
//   ←              keys_.left                   — strafe pawn left
//   →              keys_.right                  — strafe pawn right
//
// ── World / aura toggles ─────────────────────────────────────────
//   1              gpuState_.toggle_freeze_sphere() — freeze chase sphere
//   2              pawn_state_.aura_height_enabled flip      — terrain extrusion on/off
//   3              pawn_state_.aura_enabled flip            — aura field on/off
//   5              request_mood_transition(MOOD_OPEN_SUNSET)
//   6              request_mood_transition(MOOD_INDOOR_FLAT)
//   7              request_mood_transition(MOOD_INDOOR_VAULT)
//   8              request_mood_transition(MOOD_FINITE_OUTDOOR)
//   9              request_mood_transition(MOOD_FINITE_OUTDOOR_REF)
//   0              cycle_orb_palette()          — next orb palette
//   [              set_render_radius(-1)        — smaller patch ring
//   ]              set_render_radius(+1)        — larger patch ring
//
// ── Orb utilities (numpad) ───────────────────────────────────────
//   KP_8           cycle_orb_motion_rule()      — next orb motion rule
//   KP_9           toggle_orb_anchor()          — orb anchor on/off
//   KP_DECIMAL     cycle_orb_gesture()          — next gesture per rule
//
// ── Camera / possession ──────────────────────────────────────────
//   L_CTRL/R_CTRL  toggle_fpv_mode()            — orbit ↔ first-person
//   CAPS_LOCK      try_possess_nearest()        — possess nearest agent
//
// ── Diagnostics (function keys) ──────────────────────────────────
//   F1             cycle_agent_behavior_override   none → random_walk → ...
//   F2             cycle_agent_tier_override       none → worker → ...
//   F3             force_respawn_population        repopulate evicted agents
//   F4             cycle_cube_behavior_override    stationary → curlfield → phasewave
//   F5             cycle_floater_coordination      0.0 → 0.5 → 1.0
//   F6             corral_cubes                    glide cubes around pawn (4s)
//   F7             toggle_cube_kite_mode           cubes follow pawn on/off
//   F8             toggle_sky_mode                 sky-flight ribbon steering on/off
//
// ── Mouse / scroll ───────────────────────────────────────────────
//   LMB drag       look_az_delta, look_el_delta
//   RMB drag       pan_x_delta, pan_y_delta
//   scroll         zoom_delta
// ─────────────────────────────────────────────────────────────────


// ═══ GLFW KEY CODE FALLBACKS ═════════════════════════════════════
//
// Some GLFW header configurations don't expose all of these key
// constants by default. Defining them here as fallbacks lets the
// switch below compile regardless of which GLFW variant the host
// project ships. Values match the GLFW 3.x canonical numbering.

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
//
// Routes each GLFW key event to the module-owned function listed in
// the registry above. Sub-grouped to mirror the registry's order so
// the table and the dispatch read in parallel.

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
    // DONE[input:L1] five copy-paste cases collapsed into one helper
    //   call. request_mood_transition() lives in the mood module
    //   (mood.hpp declares it — LADDER-4).
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
    r = std::max(r, Cartridge::GRID_RADIUS);
    r = std::min(r, Cartridge::PREGEN_RADIUS);
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
