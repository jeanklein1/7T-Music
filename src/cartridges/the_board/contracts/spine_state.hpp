#pragma once
#include <iostream>   // ATTIC_ATRIUM — mood_def refuses an out-of-range id, loudly
#include <cstdint>

// ─── spine_state.hpp (CONTRACT: the spine's organ types) ─────────
//
// The in-class trio graduates to file scope so module deps structs
// can name the types without the complete Cartridge. The
// INSTANCES (time_state_, player_) stay at the composition root;
// the residency rulings (SEAM[spine:P8]) are unchanged — this is a
// type move, not an ownership move. SkyState — the spine-resident
// organ TYPE — lives here beside InputState, along with the
// atmosphere vocabulary its early readers need (CeilingType /
// MoodProfile / MOOD_TABLE).
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
    // ── Drawn values (authored by apply_mood_lighting, boot included) ──
    // 0 is deliberate: if apply_mood ever failed to run, the sun goes out and
    // the failure is visible on frame 1 rather than hiding behind mood 0's
    // values. Fails loud.
    float sun_intensity = 0.0f;
    float sun_ambient   = 0.0f;
    // ATMOS_1 — the fog's REST, drawn per world from the mood's atmosphere.
    // The U4 seam (phase_motion_drivers) composes the canvas's deviation
    // over it every frame. 0 is the same fails-loud choice as the sun's:
    // if the draw never ran, the world is fogless and black-fogged on
    // frame 1 rather than quietly wearing the sunset's.
    float fog_rest_density  = 0.0f;
    float fog_rest_color[3] = { 0.0f, 0.0f, 0.0f };

    // ── Sun orbit (musical coupling) ──
    float sun_orbit_phase = 0.0f;

    // ── Light re-upload flag (re-homed from entities_state_:
    //    mood was both producer and consumer — the organ was wrong,
    //    not the channel). Set true at init/teardown/apply, cleared
    //    after upload. ──
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

// ═══ THE SHAPES ══════════════════════════════════════════════════
// One authored home per shape. Three moods wear SHAPE_OPEN, and that
// they are one stage is stated by this constant, not by three copies.
//                                              fin    r_min r_max  zones aura  cull
// THE ATRIUM'S SHAPE (ATRIUM_1). Radius pinned (min == max, no roll): every
// visitor's first room is the same room. No GoL — the floor is for the images
// and the passers. Flat ceiling, the flat room's wall. The roster is the ARC
// (ATRIUM_2): one door per other mood, not PORTAL_2's triad.
//
// ATRIUM_13 — THE LAMPS ARE DRAWN, NOT AUTHORED (Jean). The entrance PINS the
// scheme so the count is four and does not roll; everything else about the
// lamps — aim, intensity, placement — is QUARTET's spreads under the world's
// seed, exactly as any indoor room. The entrance had a scheme of its own whose
// whole content was "four, straight down, nothing drawn", and that is the
// sentence being retracted; what is left after the retraction is QUARTET,
// which already says the rest better.
//
// THIS IS THE ONE THING IN THE ENTRANCE THAT NOW VARIES BETWEEN VISITS. The
// walls, the floor, the camera, the arc, the palette and the images are all
// pinned; the light is not. That is the trade "as usual" buys, and SCHEME_ROLL
// is the word that would take the COUNT with it — which is why the pin stays
// and only the index moves. Same column, same clamp, no new mechanism.
//
// ATRIUM_12 — AND THE FLOOR IS PINNED TOO, at 0.01 and NOT at 0. Every other
// room rolls its ground and the poster's fit was a hope: the CPU cannot
// sample an indoor floor (the seat is a GPU pass), so the headroom witness
// could only ever print the flat-floor answer and call it clearance. Near-flat
// makes that answer true to within a bound the arithmetic can state, and it
// keeps the camera's eye out of a rise — which at elevation -18, where the
// composed eye is already under the floor and riding the terrain clamp, is
// not free. The last rolled thing in the entrance, and the entrance is always
// as it is.
//
// ZERO WOULD HAVE DONE THE OPPOSITE. terrain_amp_ceiling is a per-wave
// amplitude CAP applied as `if (config.terrain_amp_ceiling > 0.0) { amp =
// min(amp, ceiling); }` (world.wgsl, evaluate_lattice_wave_pair), so 0 is the
// OUTDOOR sentinel — uncapped — and writing it here would have made the
// entrance the most rolled indoor room in the program. 0.01 is the smallest
// number that still ARMS the cap.
//
// AND "FLAT" IS A BOUNDED CLAIM, NOT AN ABSOLUTE. The cap governs the wave
// term only: six bands, each a bilinear blend over four lattice nodes whose
// weights sum to 1, so |wave sum| <= 6 * 0.01 = 0.06 wu. The other
// contributors to the walker's ground are untouched and named here so the
// claim cannot be mistaken for more than it is — GoL zones (already off for
// this shape, the `false` below), the baked pyramids inside
// sample_terrain_y_at, the live card's radial pulses, and the pawn's own aura
// dome, which is a floor by Jean's ruling and is meant to lift the eye.
// ATRIUM_5 — THE SMALL ROOM. Radius 1: 3x3 patches, 150 wu a side. The
// bounds are asymmetric by their own formula, [-r*PE, (r+1)*PE] = [-50, 100],
// so the pawn at the origin has 50 wu of room behind it and 100 ahead on each
// axis — the wall behind, the arc ahead, and the long side is +X +Z.

// ═══ THE ATMOSPHERES ═════════════════════════════════════════════
// The carried rows are the pre-ATMOS_1 MOOD_TABLE values exactly: one
// regime at weight 1 with every spread 0 — so the boot draw is the old
// table, bit for bit (the witnesses below pin it). Their fog rest is
// the old drivers'-room rest, FOG_DENSITY_NONE / FOG_COLOR_NONE
// (coupling/visual_canvas.hpp), which every mood wore before the rest
// came home to the mood.
//
// Row shape:  sun centre, az spread, el spread ·
//             regimes { sun colour, sun± · int, int± · amb, amb± ·
//                       fog density, density± · fog colour, colour± ·
//                       clear colour, clear± }   — {} is absent
//             weights: MOOD_TABLE, per mood
// THE TWO ROOMS STOPPED BEING ONE SKY. ATMOS_ROOM was one home for both
// because both wore the same numbers; the desk gave them different ones,
// so one home became two. Neither is a point row any more either — the
// flat's bearing wanders ±14° and its fog has a spread, the vault's
// light has one — which is why the carry witness below now names only
// the two rows that still carry.
//
// BOTH SUNS CAME A LITTLE OFF AMBER on Jean's eye, after the export and
// against it. RED DOES NOT MOVE — it is each colour's brightest channel
// and already near the ceiling — and green and blue come about a third
// of the way up to it, so the light DESATURATES rather than brightening.
// The fog colours are untouched: the note was about the light.

// ═══ THE TWO NEW SKIES (ATMOS_1, regimes at ATMOS_2) ═════════════
// BOTH SKIES ARE TUNED NOW — no number below is a sketch any more; each
// came back from the panel and was transcribed here at SHIP TIME
// (docs/ORGAN.md, "Presets"). Both collapsed the same way: every regime
// ROW survives, and regime 0 alone carries weight (MOOD_TABLE, below), so
// each mood draws ONE sky — the night a hard moon over air that is clear
// unless the fog's spread finds it, the noon a high clear day. The rest
// are ABSENT, not deleted; that is what a weight of 0 means, and giving
// one weight back brings its row back unchanged.
//
// THE NIGHT'S FOG CENTRE IS 0 WITH A SPREAD OF 0.0011. The draw is
// max(0, centre + jitter) and the jitter is ±spread, so half the seeds
// get no fog at all and the other half get up to 0.0011 — a rectified
// draw, and the only place in the table where a centre sits on the floor
// of its own distribution. THE SPREAD IS THE CEILING here: with the
// centre at 0 it is the whole of what "how thick can this night get"
// means, which is why halving the thickest night was halving this one
// number (0.0022 -> 0.0011) and nothing else.

// ═══ THE ATRIUM'S SKY (ATRIUM_1) ═════════════════════════════════
// A POINT, ON PURPOSE — the atrium draws the same sky for everyone; Jean
// tunes it on the desk (?organ=1&mood=6) and the export lands here. Every
// spread is 0 and both bearing spreads are 0, so no hash is taken and the
// draw is these numbers exactly. The centres are the flat room's regime 0
// — the atrium wears the flat room's wall, so it wears the flat room's
// light — with the intensity, the ambient and the clear colour each halved:
// a small DARK room, the images and the doors carrying it. The fog centre
// is unchanged, and only its spread goes: fog is the room's depth, not its
// brightness. Regimes 1-3 are absent.
// ATRIUM_14 — NO FOG IN THE ENTRANCE (Jean). The rest goes 0.0024 -> 0. The
// entrance is a small room whose whole job is to be READ: the controls poster
// at 15 wu, the arc's doors at 38, and a haze over either is a haze over the
// lesson. Every other room keeps its own.
//
// AND ZERO HERE IS ZERO ON SCREEN, at rest. The live value is
// max(0, fog_rest_density + gain * deviation) at the U4 seam, and the canvas
// writes the deviation as FOG_BY_FIELD[idx] - FOG_BY_FIELD[0] — a difference
// from field 0, so it is exactly 0 until the music changes field. The mood's
// number is therefore the whole of the fog in a quiet room; the drivers'
// gain is global and stays every mood's, not this one's to silence.
//
// THE FOG COLOUR STAYS AUTHORED. At density 0 nothing reads it, and a rest
// of black would be a second, invisible decision to unpick the day a hand or
// the music lifts the density off the floor.

// ═══ MOOD DEFINITIONS ════════════════════════════════════════════
//
// SEAM[mood:K1] indoor/outdoor binary lives in WorldShape as bool
//   `finite` + bool `indoor`. finite_outdoor is walled AND outdoor, so
//   it sits astride the binary and the encoding doesn't survive contact
//   — correct for today but worth re-examining when finite_outdoor
//   design lands.
//                                      shape             atmosphere

// F-3: MOOD_TABLE rows are POSITIONAL in
// mood-id order and carry no id field (the CUBE_POPULATIONS-style
// per-row assert is impossible here) — so pin the ids AT the table:
// drift in mood_constants.hpp fails HERE, where the rows live.

// COLUMN WITNESSES. F-3 pins ROW order; these pin COLUMN offsets. The
// shapes and the atmospheres are positionally brace-initialised, so a
// column added or cut mid-row shifts every field after it with no
// diagnostic. One probe per region of each row — head, middle, tail —
// so a shift anywhere trips. light_scheme / palette are WorldShape's last
// two fields and take the tail probe (ATRIUM_5); allow_frustum_cull, the
// field they displaced, keeps a probe of its own one step in, and no two
// of the three agree at the same rows, so none names what another does.
// The last two witnesses over MOOD_TABLE stood here — the open-family
// assert and ATMOS_1's carry witness, which proved the untuned rows
// still drew their old point values exactly. Both named a table that no
// longer exists; both did their whole job before it left.

// ═══ THE MOOD DEFINITION IN FORCE (O1b) ══════════════════════════
// MOOD_TABLE above is the DESIGNED definition: constexpr, asserted,
// the value the program ships with. MOOD_LIVE is the definition IN
// FORCE — seeded from the design when the program loads, and the one
// thing every RUNTIME reader reads. The split exists because a panel
// that can only write the INSTANCE writes something the next mood
// apply takes back; to change what a mood MEANS, the panel has to
// reach the definition, and a constexpr table is not reachable.
//
// ONE HOME, NOT TWO. MOOD_TABLE keeps exactly two jobs — seeding this
// array and standing under the asserts above — and no runtime reader
// is left pointing at it. The constexpr readers
// are the deliberate exception: they are compile-time budgets derived
// from the DESIGN, and a wall's geometry allowance is not a live dial.
//
// THE ELIGIBILITY RULE (docs/ORGAN.md, "Instance and definition").
// A field may take a definition target in the ORGAN registry only if
// the mood apply is its ONLY runtime reader. The whole of `atmos`
// passes: apply_mood_lighting is the one place it is read (through
// draw_atmosphere), and re-running it is how a change to it lands. The
// whole of `shape` does not: it is read all over world GENERATION, and
// rewriting it without regenerating the world would mean nothing at
// best and disagree at worst.

// The one runtime door onto a mood's fields. Non-const because the
// panel writes through it; every reader takes it by const reference.
// ATTIC_ATRIUM — AN OUT-OF-RANGE ID FAILS LOUDLY, IT DOES NOT ALIAS. The
// wrap was `% MOOD_COUNT`, which landed a stale id silently on mood 0 —
// filed in docs/OPEN.md as an alias-rather-than-refusal for as long as
// nothing could produce a stale id. Deleting a mood is what makes one
// producible: a preset exported before the deletion keys its definitions
// "<mood>/<param>", and importing it after would have written the retired
// mood's rows onto the sunset's without a word. It still returns a
// reference, so the refusal is a clamp and a line, not a throw — but the
// line names the id, once per id, and the value it lands on.

} // namespace the_board
} // namespace t7
