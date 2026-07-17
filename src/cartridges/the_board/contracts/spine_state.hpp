#pragma once
#include <cstdint>
#include "cartridges/the_board/contracts/mood_constants.hpp"   // MOOD_COUNT (sizes MOOD_TABLE) + the Mood IDs + PortalDestination (the request door)

// ─── spine_state.hpp (CONTRACT: the spine's organ types) ─────────
// History: audit/LADDER.md
//
// The in-class trio graduates to file scope so module deps structs
// (m3) can name the types without the complete Cartridge. The
// INSTANCES (time_state_, player_, transitionPhase_) stay at the
// composition root; the residency rulings (SEAM[spine:P8],
// SEAM[spine:transitions]) are unchanged — this is a type move, not
// an ownership move. MoodState — the spine-resident organ TYPE —
// lives here with its transition machine, beside InputState,
// along with the atmosphere vocabulary its early readers
// need (CeilingType / MoodProfile / MOOD_TABLE) and the transition
// request door's DECLARATION (the def rides merged mood.hpp).
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
//   · readback_x/z — THE POINT's position (the point contract,
//     contracts/point.hpp): HOST-AUTHORED by the spine's
//     P5 HARVEST — the possessed body's pos when the pawn hosts, the
//     camera's pos.xz when the camera hosts (the point readback,
//     option A, Jean's stamp). SOLE AUTHOR is still the P5 HARVEST;
//     the spine's only other touches are the TEARDOWN reset and the
//     portal door's consume. No module writes them, ever.
//   · readback_portal_trigger — THE POINT'S BUBBLE SENSOR (p1b-d):
//     the probe is host-sourced (the body's own crossing in
//     pawn-host, byte-identical; the camera + the bubble's vertical
//     gate in free-fly). The trigger RIDES the possessed slot's wire
//     and the same P5 harvest — the wire is the realization, the
//     bubble (contracts/point.hpp) is the semantics. Same sole-author
//     law.
//   · possessed_slot — possession is RE-ANCHORING (v3 §9 Act III:
//     the anchor is a role; the camera is what we control). The
//     writes live behind the agents door (try_possess_nearest,
//     reseed_player_body), paired with the GPU selector.
//   · aura_presence — P8, the pawn is the semantic owner (writes in
//     pawn.inl only).
//   · THE CAMERA HAS NO CPU MIRROR — it lives GPU-resident, keyed on
//     config.possessed_slot. The ONE sanctioned window (p1b-a): in
//     CAMERA-HOST the P5 harvest reads camera pos.xz back as the
//     point's position — a two-float harvest into the trio above,
//     not a mirror. There is still NO readback_y (the witness
//     altitude is GPU-only); it is not to be invented.
//   · the sky trio (mode / mode_prev / yaw_eased) LEFT this record
//     at m6 per Option A — it lives in RibbonState.sky, with its
//     single CPU owner (SEAM[ribbon:sky-mode], closed player-side).
//
// SEAM[spine:P8] PlayerState commented "Future (deferred)" fields
//   are explicit latent infrastructure: aura_presence is live here;
//   the other deferred fields await the unified entity layer.
//   Pattern P8 visible in source.
struct PlayerState {
    uint32_t possessed_slot = 0;   // slot in agent_state[] that the player inhabits

    // ── Camera + readback ──
    bool    fpv_mode = false;                // first-person view toggle
    float   readback_x = 0.0f;               // THE POINT's world X (host-authored — p1b-a)
    float   readback_z = 0.0f;               // THE POINT's world Z (host-authored — p1b-a)
    int32_t readback_portal_trigger = -1;    // the point's bubble sensor (p1b-d), on the possessed slot's wire

    // ── Aura presence (closes SEAM[spine:P8]) ──
    float aura_presence = 0.0f;                  // pawn aura ramp (was pawn_state_.aura_presence)

    // Future (deferred):
    //   uint32_t active_couplings;         // COUPLING_* bitmask owned by player
};

// ═══ TRANSITION PHASE ════════════════════════════════════════════
// The transition machine's phase enum. The MACHINE (transitionPhase_,
// pendingDestination_ and kin) stays spine-owned orchestration
// (SEAM[spine:transitions], K4); the enum TYPE lives here so its two
// module readers (mood's request door, agents' possession guard) name
// it unqualified instead of paying the Cartridge:: tax.
enum class TransitionPhase { IDLE, FADE_OUT, TEARDOWN, FADE_IN };

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


struct WorldState;   // contracts/surface_services.hpp — the request door reads active_seed (fwd: reference param)

// ═══ MOOD STATE (the spine's mood organ; instance at the root) ════
// Type at the contract tier; the instance was ALWAYS spine-resident with the
// transition machine (SEAM[spine:transitions], K4). The boot default
// reads the demo sentence; demos/demo.hpp precedes this header in the
// cartridge include cohort (the patch_system.hpp DEMO.seed precedent).
struct MoodState {
    // ── Currently active mood ──
    uint32_t active = DEMO.boot_mood;  // boots from the demo sentence (DEMO-1)

    // ── Mood-applied values (re-set on each apply_mood) ──
    float sun_intensity = 0.8f;
    float sun_ambient   = 0.25f;
    float terrain_amp_ceiling = 0.0f;       // mirrors GPU config.terrain_amp_ceiling
    bool  spot_light_active = false;

    // ── Transition machinery ──
    float transition_timer         = 0.0f;
    float transition_fade_duration = 0.5f;  // seconds per fade direction
    float transition_fade_alpha    = 0.0f;

    // ── Portal upload flag ──
    bool portals_dirty = true;              // true at boot → first upload guaranteed

    // ── Back-portal return state ──
    bool     back_portal_pending       = false;
    uint32_t back_portal_return_seed   = 0;
    uint32_t back_portal_return_mood   = 0;
    uint32_t back_portal_return_radius = 2;

    // ── Sun orbit (musical coupling) ──
    float sun_orbit_phase = 0.0f;

    // ── Light re-upload flag (re-homed from entities_state_ at m4:
    //    mood was both producer and consumer — the organ was wrong,
    //    not the channel). Set true at init/teardown/apply, cleared
    //    after upload. ──
    bool lights_dirty = true;
};

// ═══ MOOD SYSTEM (vocabulary) ════════════════════════════════════

enum class CeilingType : uint32_t {
    NONE = 0,   // outdoor — no shell geometry
    FLAT = 1,   // flat slab ceiling
    VAULT = 2,   // catenary vault ceiling
};

struct MoodProfile {
    // ─── World bounds ───────────────────────────────────────
    bool   finite;                 // true = walled world with finite radius
    uint32_t finite_radius_min;    // min patch radius (when finite)
    uint32_t finite_radius_max;    // max patch radius (when finite)

    // ─── Lighting ───────────────────────────────────────────
    float  sun_direction[3];       // directional light vector (normalized)
    float  sun_color[3];           // sun RGB
    float  sun_intensity;          // diffuse strength
    float  sun_ambient;            // ambient fill strength

    // ─── Atmosphere ─────────────────────────────────────────
    // INTENT[mood-fog-baseline] fog_density/fog_color have ZERO readers —
    //   fog retired from apply_mood when it went field-driven (the
    //   visual-canvas fog flush owns it per-frame). The authored per-mood
    //   baselines are kept as intent: revive-or-delete at the panel era.
    float  fog_density;            // exponential fog coefficient
    float  fog_color[3];           // fog/horizon RGB

    // ─── Indoor shell ───────────────────────────────────────
    bool   indoor;                 // true = enclosed space with ceiling
    CeilingType ceiling_type;      // NONE / FLAT / VAULT
    float  ceiling_height;         // ceiling Y (world units)

    // ─── Background ─────────────────────────────────────────
    float  clear_color[3];         // sky or dark ceiling RGB
    float  wall_color[3];          // indoor wall surface RGB
    float  ceiling_color[3];       // indoor ceiling surface RGB

    // ─── Feature selection (per-mood) ───────────────────────
    bool   allow_gol_zones;        // GoL zone spawning + visualization
    bool   allow_pawn_aura;        // toroidal spring grid tinting + height boost
    bool   allow_frustum_cull;     // GPU frustum cull for LOD0 terrain (Tier 4)
    bool   has_anchor_ribbon;      // mood spawns a fixed reference ribbon at the world center

};

// ═══ MOOD DEFINITIONS ════════════════════════════════════════════
//
// SEAM[mood:K1] indoor/outdoor binary lives here as bool `finite` +
//   bool `indoor` flags. With finite_outdoor and finite_outdoor_ref,
//   the binary doesn't survive contact — the encoding is correct
//   for today but worth re-examining when finite_outdoor design lands.
// has_anchor_ribbon flag (last column): mood 5 (FINITE_OUTDOOR_REF)
//   is the only row that sets it true. The mood ID is an identifier,
//   not a discriminator — atmospheric data is profile-driven. See
//   SEAM[mood:L1] above for the gating call site.
//                                  fin  r_min r_max  sun_dir                sun_color              int   amb   fog_d   fog_color               indoor  ceil       ceil_h  clear_color            wall_color             ceil_color               zones  aura   cull   ribbon
inline constexpr MoodProfile MOOD_TABLE[MOOD_COUNT] = {
    /* MOOD_OPEN_DEFAULT       */  { false, 2, 2, { 0.69f,-0.71f,-0.14f}, {1.0f, 0.95f, 0.90f}, 0.80f, 0.25f, 0.0030f, {0.85f, 0.78f, 0.72f},  false, CeilingType::NONE,  0.0f,  {0.85f, 0.78f, 0.72f}, {0.75f,0.68f,0.60f}, {0.75f,0.68f,0.60f},   true,  true,  true,  false },
    /* MOOD_OPEN_SUNSET        */  { false, 2, 2, { 0.96f,-0.26f,-0.13f}, {1.0f, 0.75f, 0.45f}, 0.90f, 0.20f, 0.0050f, {0.95f, 0.70f, 0.45f},  false, CeilingType::NONE,  0.0f,  {0.95f, 0.70f, 0.45f}, {0.75f,0.68f,0.60f}, {0.75f,0.68f,0.60f},   true,  true,  true,  false },
    /* MOOD_INDOOR_FLAT        */  { true,  1, 4, { 0.20f,-0.90f, 0.00f}, {1.0f, 0.90f, 0.80f}, 0.35f, 0.35f, 0.0003f, {0.15f, 0.12f, 0.10f},  true,  CeilingType::FLAT,  20.0f, {0.15f, 0.12f, 0.10f}, {0.65f,0.58f,0.50f}, {0.60f,0.55f,0.48f},   true,  true,  false, false },
    /* MOOD_INDOOR_VAULT       */  { true,  1, 4, { 0.20f,-0.90f, 0.00f}, {1.0f, 0.90f, 0.80f}, 0.35f, 0.35f, 0.0003f, {0.15f, 0.12f, 0.10f},  true,  CeilingType::VAULT, 25.0f, {0.15f, 0.12f, 0.10f}, {0.70f,0.62f,0.52f}, {0.65f,0.58f,0.50f},   true,  true,  false, false },
    /* MOOD_FINITE_OUTDOOR     */  { true,  1, 4, { 0.69f,-0.71f,-0.14f}, {1.0f, 0.95f, 0.90f}, 0.80f, 0.25f, 0.0030f, {0.85f, 0.78f, 0.72f},  false, CeilingType::NONE,  0.0f,  {0.85f, 0.78f, 0.72f}, {0.75f,0.68f,0.60f}, {0.75f,0.68f,0.60f},   true,  true,  true,  false },
    /* MOOD_FINITE_OUTDOOR_REF */  { true,  1, 4, { 0.69f,-0.71f,-0.14f}, {1.0f, 0.95f, 0.90f}, 0.80f, 0.25f, 0.0030f, {0.85f, 0.78f, 0.72f},  false, CeilingType::NONE,  0.0f,  {0.85f, 0.78f, 0.72f}, {0.75f,0.68f,0.60f}, {0.75f,0.68f,0.60f},   true,  true,  true,  true  },
};

// F-3: MOOD_TABLE rows are POSITIONAL in
// mood-id order and carry no id field (the CUBE_POPULATIONS-style
// per-row assert is impossible here) — so pin the ids AT the table:
// drift in mood_constants.hpp fails HERE, where the rows live.
static_assert(MOOD_OPEN_DEFAULT       == 0 && MOOD_OPEN_SUNSET        == 1
           && MOOD_INDOOR_FLAT        == 2 && MOOD_INDOOR_VAULT       == 3
           && MOOD_FINITE_OUTDOOR     == 4 && MOOD_FINITE_OUTDOOR_REF == 5
           && MOOD_COUNT == 6,
    "MOOD_TABLE rows are positional in mood-id order (F-3): "
    "reorder the table together with the ids");

// ═══ THE TRANSITION REQUEST DOOR (decl; def rides merged mood.hpp) ═
// The single canonical transition entry point — one door, many keys.
// DEPS-FORM: the driver world holds no MachineCtx; the door
// takes the transition channel explicitly (the m3 precedent class).
void request_mood_transition(TransitionPhase& phase, PortalDestination& pending,
    MoodState& ms, const WorldState& ws, uint32_t mood);

} // namespace the_board
} // namespace t7
