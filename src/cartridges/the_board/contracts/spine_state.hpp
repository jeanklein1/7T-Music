#pragma once
#include <cstdint>

// ─── spine_state.hpp (CONTRACT: the spine's organ types) ─────────
//
// The in-class trio graduates to file scope so module deps structs
// can name the types without the complete Cartridge. The
// INSTANCES (time_state_, player_) stay at the composition root;
// the residency rulings (SEAM[spine:P8]) are unchanged — this is a
// type move, not an ownership move. SkyState — the spine-resident
// organ TYPE — lives here beside InputState. The sky VOCABULARY that
// stood beside it left with the moods (ONE_WORLD-II U2); what the
// world wears is a bank now, at contracts/atmosphere_surface.hpp.
//
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ TIME STATE ══════════════════════════════════════════════════
// Per-frame clock state used everywhere. beats/seconds advance
// monotonically; dt is the most recent frame delta.
struct TimeState {
    float beats   = 0.0f;
    float seconds = 0.0f;
    float dt      = 0.016f;
    // Musical tempo follower: beats/sec, HELD-LAST through silence
    // and stopped transport; defaults to 100 BPM (the calibration
    // anchor for the authored idle motion).
    float beat_rate   = 100.0f / 60.0f;
    float prev_beats  = 0.0f;
};

// ═══ PLAYER STATE — THE WITNESS RECORD (v3 §11) ═══════════════════
//
// THE WITNESS CONTRACT, declared and census-checked (the score
// census, Direction W):
//   · THE POINT'S RECORD LEFT THIS STRUCT at POINT_1 — the position
//     mirror (x/z) lives in its semantic home, PointState
//     (contracts/point.hpp), which carries the full authoring law
//     (P5 HARVEST sole author; the rebirth reset is the spine's only
//     other touch).
//   · possessed_slot — possession is RE-ANCHORING (v3 §9 Act III:
//     the anchor is a role; the camera is what we control). The
//     writes live behind the agents door (try_possess_nearest,
//     reseed_player_body), paired with the GPU selector.
//   · aura_presence — P8, the pawn is the semantic owner (writes in
//     bodies/pawn.hpp only).
//   · THE CAMERA HAS NO CPU MIRROR — it lives GPU-resident, keyed on
//     config.possessed_slot. The ONE sanctioned window: in
//     CAMERA-HOST the P5 harvest reads camera pos.xz back as the
//     point's position (PointState.x/z) — a two-float harvest, not a
//     mirror. RIBBON_1 added PointState.y/heading, and they are the
//     BODY hosts' alone: the possessed slot's readback authors them, so
//     possess() can capture the pose the body left. The camera still has
//     no y — the witness altitude is GPU-only and is not to be invented.
//   · the rider state LEFT this record per Option A — it lives in
//     RibbonState.sky, which RIBBON_1 reduced to the possess()-staged
//     release request (the eased hand went to the head kernel, where the
//     hand it eases is read); riding ROUTES on the host machine
//     (point_.host == RIBBON — RESIDUE_3, closed player-side).
//
// SEAM[spine:P8] PlayerState commented "Future (deferred)" fields
//   are explicit latent infrastructure: aura_presence is live here;
//   the other deferred fields await the unified entity layer.
//   Pattern P8 visible in source.
struct PlayerState {
    uint32_t possessed_slot = 0;   // slot in agent_state[] that the player inhabits

    // ── Camera ──
    bool    fpv_mode = false;                // first-person view toggle
    // (The point's position mirror moved HOME at POINT_1:
    //  PointState.x/z — contracts/point.hpp.)

    // ── Aura presence (closes SEAM[spine:P8]) ──
    float aura_presence = 0.0f;                  // pawn aura ramp (was pawn_state_.aura_presence)

    // Future (deferred):
    //   uint32_t active_couplings;         // COUPLING_* bitmask owned by player
};

// ═══ INPUT STATE — THE DRIVER'S INTENT ORGAN ══════════════════════
// Type at the contract tier, instance at the root. The
// driver WRITES it (the callbacks + update_movement_intent); the
// spine's signal fill and the ribbon's sky flight READ it (v3 §9
// Act I: drivers write intents; bodies translate them). KeyState /
// MouseState stay with input — they are the driver's private organs;
// this record is the intent CHANNEL the bodies consume.
struct InputState {
    float move_x = 0.0f;
    float move_z = 0.0f;
    float look_az_delta = 0.0f;
    float look_el_delta = 0.0f;
    float zoom_delta = 0.0f;
    float pan_x_delta = 0.0f;
    float pan_y_delta = 0.0f;
};


// ═══ SKY STATE (the world's drawn sky; instance at the root) ═════
// Type at the contract tier; the instance is spine-resident (K4).
//
// IT WAS MoodState (ONE_WORLD-II U2), and its first field was the live
// mood id — the fact the whole system turned on. The id is gone and a
// struct named for it would be a lying name, which is the class of
// defect this campaign exists to remove. What the struct actually holds,
// and always held, is the world's DRAWN SKY: rung 3 of the persistence
// ladder, the environment's instance that rung 5 composes over every
// frame.
struct SkyState {
    // ── Drawn values (authored by stage_world_birth, boot included) ──
    // 0 is deliberate: if the world's birth ever failed to run, the sun goes
    // out and the failure is visible on frame 1 rather than hiding behind a
    // value some other author happened to leave. Fails loud.
    float sun_intensity = 0.0f;
    float sun_ambient   = 0.0f;
    // ATMOS_1 — the fog's REST, drawn per world from ATMOS_LIVE. The U4
    // seam (phase_motion_drivers) composes the canvas's deviation over it
    // every frame. 0 is the same fails-loud choice as the sun's: if the
    // draw never ran, the world is fogless and black-fogged on frame 1
    // rather than quietly wearing the bank's.
    float fog_rest_density  = 0.0f;
    float fog_rest_color[3] = { 0.0f, 0.0f, 0.0f };

    // ── Sun orbit (musical coupling) ──
    float sun_orbit_phase = 0.0f;

    // ── Light re-upload flag (re-homed from entities_state_:
    //    the sky was both producer and consumer, so the organ was
    //    wrong and not the channel). Set true at init, at teardown and
    //    at the world's birth; cleared after upload. ──
    bool lights_dirty = true;
};

// ═══ THE MOOD SYSTEM STOOD HERE ═════════════════════════════════
//
// Six declared symbols, taken by ENUMERATION and not as a span
// (Amendment B clause 3): WorldShape, shape_is_open, Regime,
// REGIME_COUNT, Atmosphere, MoodProfile — each death-verified against
// the whole tree before the cut, and the region re-read afterwards to
// prove nothing else lived between the banners. With them: MOOD_TABLE,
// MOOD_LIVE, mood_def, the five SHAPE_* rows, the seven ATMOS_* rows,
// mood_carries_point, and seventeen column-drift and carry witnesses
// whose subject was one of those.
//
// The clause exists because this region had already taken something
// once: the PERSISTENCE LADDER sat between these banners and had to be
// relocated out (U0) before the cut could be lawful. Two units later a
// banner-anchored span in world.wgsl took veil_dither_noise and veil_t,
// and one in orb_surface.hpp took ORB_TABLE. Three strikes, one law.
//
// What survives the moods lives where it belongs now: the atmosphere is
// ATMOS_LIVE (contracts/atmosphere_surface.hpp), the sky's per-world
// instance is below, and the two facts WorldShape carried that outlive
// it — the finite pin and its radius dials — are beside WorldState
// (contracts/surface_services.hpp).

// AND THE PROSE WENT WITH THE CODE (ONE_WORLD-II U7). Below this banner
// stood ~190 lines of comment and nothing else: THE SHAPES, THE
// ATMOSPHERES, THE TWO NEW SKIES, THE ATRIUM'S SKY, MOOD DEFINITIONS,
// F-3's row pin, the column witnesses, THE MOOD DEFINITION IN FORCE
// (O1b) and mood_def's refusal note. Every one of them described a
// symbol taken above, and prose that outlives its subject is a lie the
// compiler cannot catch (L30). What each still-true sentence became:
//   · the atmosphere's draw law, the centre/spread contract and the
//     transcription rule → contracts/atmosphere_surface.hpp
//   · the finite pin and its radius dials → WorldState
//     (contracts/surface_services.hpp)
//   · the stale-key refusal ATTIC_ATRIUM wrote → src/console/
//     organ_registry.hpp, where the boundary now refuses a definition
//     target above 0 by the same argument
//   · the persistence ladder → docs/LAWS.md (relocated U0, amended U2)
// Nothing was carried forward that described only the dead.

} // namespace the_board
} // namespace t7
