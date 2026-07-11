#pragma once
#include <cstdint>

// ─── input.hpp (HEADER: state + decls) ────────────────────────────
//
// The home of input dispatch. Keyboard, mouse, scroll. Routes raw
// GLFW events to module-owned commands.
//
// CONVERTED (LADDER-3 c6, header/impl split + G2): this header owns
// the three input state structs — InputState (per-frame intent +
// deltas), KeyState (held movement keys), MouseState (drag state) —
// graduated from the Cartridge class body (G2: file-scope headers and
// the composition root need them DECLARATION-side), and the ten
// function DECLARATIONS. The cartridge declares the instances
// (inputState_ / keys_ / mouse_) at its COMPOSITION ROOT. Definitions
// live in input.inl, included at FILE SCOPE in the post-class MODULE
// IMPLEMENTATIONS zone; the impl carries its own <GLFW/glfw3.h>
// include (the unpapered dependency — c6). Namespace t7::the_board.
//
// ┌─── Public surface (called from outside this module) ────────────┐
// │                                                                  │
// │  GLFW callbacks (the spine's on_input override routes raw        │
// │  events here):                                                   │
// │    on_key_down(c, key)                                           │
// │    on_key_up(c, key)                                             │
// │    on_mouse_move(c, dx, dy)                                      │
// │    on_mouse_button(c, button, pressed)                           │
// │    on_scroll(c, delta)                                           │
// │                                                                  │
// │  Per-frame:                                                      │
// │    update_movement_intent(c)   — recomputes (move_x, move_z)     │
// │    clear_input_deltas(c)       — zeroes mouse deltas at frame end│
// │                                                                  │
// │  Camera/view commands (driven by key dispatch):                  │
// │    toggle_fpv_mode(c)                                            │
// │    toggle_sky_mode(c)                                            │
// │    set_render_radius(c, r)                                       │
// │                                                                  │
// │  Cross-module reads (this module's state read by others):        │
// │    inputState_.move_x/move_z       — pawn kernel signal, ribbon  │
// │    inputState_.look/zoom/pan       — camera signal fill          │
// │                                                                  │
// └──────────────────────────────────────────────────────────────────┘
//
// ── A note on bindings (READ ME if changing keys) ─────────────────
//
// All key bindings in this module are temporary scaffolding. The
// program is being developed as a synth for visuals: the goal is for
// the functions invoked here to be drivable by music, automation, or
// remappable input vocabularies — not by these specific keys.
//
// What's durable: the *functions* in the registry (input.inl)
//   (try_possess_nearest, cycle_orb_palette, ...).
//   Other modules' Public surfaces document those functions as their
//   permanent entry points.
//
// What's temporary: every key code in the registry. Bindings will
//   be remapped, replaced with MIDI control changes, or driven by
//   the analysis layer in time.
//
// One inherited constraint worth knowing: A–Z keys are claimed by
// the analysis layer above the cartridge as MIDI piano notes (per
// the boot banner: "A-Z=piano keys"). Letter keys cannot double as
// cartridge diagnostics without playing notes every time you cycle
// a behavior. That's why the diagnostic surface uses function keys.
//
// ──────────────────────────────────────────────────────────────────
//
// Depends on: nothing header-side (plain aggregates + declarations).
// The impl reaches GLFW (<GLFW/glfw3.h>, its own include), the
// keyhole's organs (inputState_ / keys_ / mouse_ / player_ /
// world_state_ / device_ / gpuState_ / pawn_state_ / agent_state_ /
// orbs_state_ / cube_behaviors_state_), the converted modules'
// commands (agents.hpp / orbs.hpp / cube_behaviors.hpp /
// floater_vocabulary.hpp), mood.inl's request_mood_transition (a
// member — K4's territory) + mood_constants.hpp's MOOD_* IDs, and
// in-class statics (Cartridge::GRID_RADIUS / Cartridge::PREGEN_RADIUS)
// through the complete type (Cartridge:: / keyhole).
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

class Cartridge;  // fwd — module fns take the keyhole (defined in input.inl, post-class)

// ═══ INPUT STATE (G2 — graduated from the class body) ════════════

// Per-frame movement intent + look/zoom/pan deltas. Written by the
// dispatch below; read by the pawn kernel signal fill, the camera
// signal fill, and the ribbon conductor (sky mode).
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
//
// DEFINED in input.inl (post-class, self-wrapping) — the bodies reach
// the keyhole's organs and the other modules' command functions via
// the complete type (the c6 ~41-read retrofit to keyhole form).

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
