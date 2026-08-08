#pragma once
#include <cstdint>
#include "cartridges/the_board/contracts/mood_constants.hpp"   // MOOD_COUNT (sizes MOOD_TABLE) + the Mood IDs + PortalDestination (the request door)

// ─── spine_state.hpp (CONTRACT: the spine's organ types) ─────────
// History: audit/LADDER.md
//
// The in-class trio graduates to file scope so module deps structs
// can name the types without the complete Cartridge. The
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
//   · THE POINT'S RECORD LEFT THIS STRUCT at POINT_1 — the position
//     mirror (x/z) and the bubble sensor (portal_trigger) live in
//     their semantic home, PointState (contracts/point.hpp), which
//     carries the full authoring law (P5 HARVEST sole author; the
//     TEARDOWN reset and the portal door's consume are the spine's
//     only other touches).
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
//     mirror. There is still NO point y (the witness altitude is
//     GPU-only); it is not to be invented.
//   · the rider state LEFT this record per Option A — it lives in
//     RibbonState.sky (yaw_eased + the possess()-staged release
//     request); riding ROUTES on the host machine (point_.host ==
//     RIBBON — RESIDUE_3, closed player-side).
//
// SEAM[spine:P8] PlayerState commented "Future (deferred)" fields
//   are explicit latent infrastructure: aura_presence is live here;
//   the other deferred fields await the unified entity layer.
//   Pattern P8 visible in source.
struct PlayerState {
    uint32_t possessed_slot = 0;   // slot in agent_state[] that the player inhabits

    // ── Camera ──
    bool    fpv_mode = false;                // first-person view toggle
    // (The point's position mirror + bubble sensor moved HOME at
    //  POINT_1: PointState.x/z/portal_trigger — contracts/point.hpp.)

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
// transition machine (SEAM[spine:transitions], K4). The boot value is
// authored at the composition root — no include-order cable.
struct MoodState {
    // ── Currently active mood ──
    uint32_t active = 0;  // authored at the composition root (Cartridge ctor) from DEMO.boot_mood

    // ── Mood-applied values (authored by apply_mood, boot included) ──
    // 0 is deliberate: if apply_mood ever failed to run, the sun goes out and
    // the failure is visible on frame 1 rather than hiding behind mood 0's
    // values. Fails loud.
    float sun_intensity = 0.0f;
    float sun_ambient   = 0.0f;
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
    // One-shot arm for the door-guarantee fallback (U2). TRUE at boot
    // (the boot world runs no teardown) and re-armed at every teardown;
    // consumed by the world's FIRST fullRegen. NOT tied to fullRegen
    // itself: request_recenter re-arms fullRegen mid-world (the
    // render-radius keys), and the guarantee is AT POPULATION only.
    bool     door_fallback_pending     = true;
    uint32_t back_portal_return_seed   = 0;
    uint32_t back_portal_return_mood   = 0;
    uint32_t back_portal_return_radius = 2;

    // ── Sun orbit (musical coupling) ──
    float sun_orbit_phase = 0.0f;

    // ── Light re-upload flag (re-homed from entities_state_:
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

    // ─── Indoor shell ───────────────────────────────────────
    bool   indoor;                 // true = enclosed space with ceiling
    CeilingType ceiling_type;      // NONE / FLAT / VAULT
    float  wall_height;            // where the VERTICAL wall stops (world units).
                                   // NOT the ceiling on VAULT — there the ceiling is
                                   // the crown, 47-92 against this 25. See vault_crown.
    float  terrain_amp_ceiling;    // indoor terrain-amp cap (0 = uncapped, outdoor)

    // ─── Background ─────────────────────────────────────────
    float  clear_color[3];         // sky or dark ceiling RGB
    // wall/ceiling colors: INDOOR_PALETTES (mood.hpp) is the authority —
    // seed-picked per world; the profile never authored them in effect.

    // ─── Feature selection (per-mood) ───────────────────────
    bool   allow_gol_zones;        // GoL zone spawning + visualization
    bool   allow_pawn_aura;        // toroidal spring grid tinting + height boost
    bool   allow_frustum_cull;     // STATUS: LATENT[mood_cull_opt_out] — INERT.
                                   // Reaches renderer_.set_frustum_cull_active
                                   // and stops: the flag's reader was retired
                                   // when the draw plan took every mood, so
                                   // indoor terrain IS culled despite the two
                                   // `false` rows below. See renderer.hpp for
                                   // the full note and the cut (OPT_1 O0-f).

};

// ═══ MOOD DEFINITIONS ════════════════════════════════════════════
//
// SEAM[mood:K1] indoor/outdoor binary lives here as bool `finite` +
//   bool `indoor` flags. finite_outdoor is walled AND outdoor, so it
//   sits astride the binary and the encoding doesn't survive contact —
//   correct for today but worth re-examining when finite_outdoor
//   design lands.
//                                  fin  r_min r_max  sun_dir                sun_color              int   amb   indoor  ceil       wall_h  amp_c  clear_color            zones  aura   cull
inline constexpr MoodProfile MOOD_TABLE[MOOD_COUNT] = {
    /* MOOD_OPEN_SUNSET        */  { false, 2, 2, { 0.94f,-0.29f,-0.13f}, {1.0f, 0.75f, 0.45f}, 0.90f, 0.20f,  false, CeilingType::NONE,  0.0f,  0.0f,  {0.95f, 0.70f, 0.45f}, true,  true,  true  },
    /* MOOD_INDOOR_FLAT        */  { true,  1, 4, { 0.20f,-0.90f, 0.00f}, {1.0f, 0.90f, 0.80f}, 0.35f, 0.35f,  true,  CeilingType::FLAT,  20.0f, 0.5f,  {0.15f, 0.12f, 0.10f}, true,  true,  false },
    /* MOOD_INDOOR_VAULT       */  { true,  1, 4, { 0.20f,-0.90f, 0.00f}, {1.0f, 0.90f, 0.80f}, 0.35f, 0.35f,  true,  CeilingType::VAULT, 25.0f, 0.5f,  {0.15f, 0.12f, 0.10f}, true,  true,  false },
    /* MOOD_FINITE_OUTDOOR     */  { true,  1, 4, { 0.56f,-0.82f,-0.11f}, {1.0f, 0.95f, 0.90f}, 0.80f, 0.25f,  false, CeilingType::NONE,  0.0f,  0.0f,  {0.85f, 0.78f, 0.72f}, true,  true,  true  },
};

// F-3: MOOD_TABLE rows are POSITIONAL in
// mood-id order and carry no id field (the CUBE_POPULATIONS-style
// per-row assert is impossible here) — so pin the ids AT the table:
// drift in mood_constants.hpp fails HERE, where the rows live.
static_assert(MOOD_OPEN_SUNSET  == 0 && MOOD_INDOOR_FLAT    == 1
           && MOOD_INDOOR_VAULT == 2 && MOOD_FINITE_OUTDOOR == 3
           && MOOD_COUNT == 4,
    "MOOD_TABLE rows are positional in mood-id order (F-3): "
    "reorder the table together with the ids");

// COLUMN WITNESSES. F-3 pins ROW order; these pin COLUMN offsets. The
// rows are positionally brace-initialised, so a column added or cut
// mid-row shifts every field after it with no diagnostic. One probe per
// region of the row — head, middle, tail — so a shift anywhere trips.
static_assert(MOOD_TABLE[MOOD_OPEN_SUNSET].finite         == false, "MOOD_TABLE column drift: finite (head)");
static_assert(MOOD_TABLE[MOOD_FINITE_OUTDOOR].finite      == true,  "MOOD_TABLE column drift: finite (head)");
static_assert(MOOD_TABLE[MOOD_OPEN_SUNSET].indoor         == false, "MOOD_TABLE column drift: indoor (middle)");
static_assert(MOOD_TABLE[MOOD_INDOOR_VAULT].indoor        == true,  "MOOD_TABLE column drift: indoor (middle)");
static_assert(MOOD_TABLE[MOOD_INDOOR_FLAT].wall_height  == 20.0f, "MOOD_TABLE column drift: wall_height");
static_assert(MOOD_TABLE[MOOD_INDOOR_VAULT].wall_height == 25.0f, "MOOD_TABLE column drift: wall_height");
// The tail probe followed has_anchor_ribbon out; allow_frustum_cull is the
// last field now and takes it. Both values differ from `indoor` at the same
// rows, so the tail probe still names something the middle probe does not.
static_assert(MOOD_TABLE[MOOD_OPEN_SUNSET].allow_frustum_cull == true,  "MOOD_TABLE column drift: allow_frustum_cull (tail)");
static_assert(MOOD_TABLE[MOOD_INDOOR_FLAT].allow_frustum_cull == false, "MOOD_TABLE column drift: allow_frustum_cull (tail)");

// ═══ THE TRANSITION REQUEST DOOR (decl; def rides merged mood.hpp) ═
// The single canonical transition entry point — one door, many keys.
// DEPS-FORM: the driver world holds no MachineCtx; the door
// takes the transition channel explicitly (the deps-form precedent).
void request_mood_transition(TransitionPhase& phase, PortalDestination& pending,
    MoodState& ms, const WorldState& ws, uint32_t mood);

} // namespace the_board
} // namespace t7
