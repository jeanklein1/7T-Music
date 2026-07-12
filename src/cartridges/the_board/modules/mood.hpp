#pragma once
#include <cstdint>
#include "cartridges/the_board/state.hpp"                    // wgpu, GPUSpotLightArray, MAX_SPOT_LIGHTS
#include "cartridges/the_board/modules/mood_constants.hpp"   // MOOD_COUNT, the Mood IDs, PortalDestination
#include "cartridges/the_board/modules/keyhole.hpp"          // Cartridge + wgpu::Queue fwds (the keyhole)

// ─── mood.hpp (HEADER: vocabulary + palettes + decls) ─────────────
// Converted (LADDER-4, per K4): history in audit/LADDER.md.
//
// Atmosphere, indoor lighting, shell geometry, portals.
//
// Mood is VOCABULARY + APPLIERS + SIX DOORS. This header owns the
// vocabulary — CeilingType, MoodProfile, MOOD_TABLE, the portal color
// table, the indoor wall palettes, INDOOR_ENTITY_WALL_MARGIN (also
// read by negotiate_position) — and the DECLARATIONS of the six doors
// (apply_mood, request_mood_transition, force_spawn_back_portal,
// upload_lights, upload_portal_array, mood_name) plus the appliers and
// derivers. The Mood IDs are file-scope vocabulary
// (mood_constants.hpp), consumed here. MOOD OWNS NO STATE: no
// MoodState exists here or at the composition root — mood_state_ and
// the transition machine (transitionPhase_ / pendingDestination_ and
// kin) are SPINE-OWNED orchestration (SEAM[spine:transitions] at the
// machine's banner; constitution §2). The force-spawn mutation of the
// arch belongs to the arch's owner — mood's force_spawn_* internals
// COMPUTE VALUES and call entities' force_spawn_portal_arch.
// Definitions live in mood.inl, included at FILE SCOPE in the
// post-class MODULE IMPLEMENTATIONS zone. Namespace t7::the_board.
//
// The impl additionally reaches the keyhole's spine-resident state
// (mood_state_ / transitionPhase_ / pendingDestination_ /
// backPortalPosition_ / cpuSpotLights_ / cpuPortalArray_ / sun + clear
// colors / world_state_ and the feature-gate flags), the converted
// modules' surfaces (entities' force_spawn_portal_arch, ribbon's
// fill/commit, gallery's wall paintings, orbs' configure,
// render_passes' compute_spot_light_vp), and in-class statics
// (Cartridge::ARCH_TIERS / Cartridge::ArchIdx /
// Cartridge::solve_catenary_a / Cartridge::PATCH_EXTENT /
// Cartridge::TransitionPhase) through the complete type.
//
// SEAM[mood:K1] apply_mood is the single canonical mood entry point.
//   All mood transitions — keyboard, portal crossings, world
//   teardown — funnel through here. Other code paths set
//   pendingDestination_ / transitionPhase_; the actual mood activation
//   happens here. The orchestrator owns only ordering and the
//   activate-mood bookkeeping; the substantive work splits across
//   four named sub-functions (apply_mood_lighting, _spot_lights,
//   _indoor_shell, _anchor_ribbon).
// SEAM[mood:K4] apply_mood_anchor_ribbon is the second entry
//   point to commit_ribbon (the dual-entry pattern noted at
//   ribbon.hpp::SEAM[ribbon:dual-entry]). Mood-5 forces a ribbon
//   spawn through fill_ribbon_selection_geometry + commit_ribbon
//   without going through the patch-streaming dispatch.
// SEAM[mood:L1] MoodProfile.has_anchor_ribbon is the per-mood flag
//   that gates apply_mood_anchor_ribbon's execution. Default false
//   for moods 0-4; true for mood 5 (FINITE_OUTDOOR_REF). Read there
//   and referenced from orbs as mood:L1 in the anchor-ribbon
//   gating logic.
// apply_mood orchestrates the named applier helpers; the appliers
//   match the natural seams in the flow, and their per-mood-transition
//   call order is load-bearing.
// request_mood_transition is the single canonical transition entry
//   point. Bails if a transition is already in flight. Lives in the
//   mood module rather than input because portal crossings and other
//   code paths can also drive mood transitions. One door, many keys.
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

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

// ═══ PORTAL VOCABULARY ═══════════════════════════════════════════

// ── Portal detection ──
inline constexpr float PORTAL_DENSITY = 1.00f;  // fraction of Doorway arches that become portals

// Portal color by mood (indexed by destination.mood)
inline constexpr float PORTAL_COLORS[6][3] = {
    { 0.90f, 0.45f, 0.70f },  // mood 0  open_default    — pink
    { 0.72f, 0.45f, 0.85f },  // mood 1  open_sunset     — lilac
    { 0.95f, 0.55f, 0.15f },  // mood 2  indoor_flat     — orange
    { 0.95f, 0.80f, 0.20f },  // mood 3  indoor_vault    — yellow
    { 0.85f, 0.20f, 0.15f },  // mood 4  finite_outdoor  — red
    { 0.70f, 0.15f, 0.12f },  // mood 5  finite_outdoor_ref — dark red
};
inline constexpr float PORTAL_COLOR_BACK[3] = { 0.35f, 0.55f, 0.90f };  // back-portal — blue

// ═══ INDOOR WALL PALETTE ═════════════════════════════════════════
//
// SEAM[mood:tuning-data] the palettes are consumed only by
//   apply_mood_indoor_shell; the indoor lighting SCHEMES (the other
//   half of this seam) are consumed only by derive_indoor_lights and
//   stay impl-side (mood.inl) — module-internal authoring tables.
struct IndoorPalette {
    const char* name;
    float wall_color[3];
    float ceiling_color[3];
};
inline constexpr IndoorPalette INDOOR_PALETTES[] = {
    { "warm taupe",     { 0.72f, 0.65f, 0.55f }, { 0.65f, 0.58f, 0.50f } },
    { "slate blue",     { 0.58f, 0.62f, 0.68f }, { 0.50f, 0.55f, 0.62f } },
    { "terracotta",     { 0.65f, 0.50f, 0.42f }, { 0.55f, 0.42f, 0.36f } },
    { "sage",           { 0.62f, 0.68f, 0.58f }, { 0.55f, 0.60f, 0.52f } },
    { "pale linen",     { 0.85f, 0.80f, 0.72f }, { 0.78f, 0.73f, 0.65f } },
    { "deep mocha",     { 0.45f, 0.40f, 0.35f }, { 0.38f, 0.34f, 0.30f } },
    { "dusty rose",     { 0.70f, 0.58f, 0.58f }, { 0.62f, 0.50f, 0.50f } },
    { "warm charcoal",  { 0.40f, 0.38f, 0.36f }, { 0.32f, 0.30f, 0.28f } },
};
inline constexpr uint32_t INDOOR_PALETTE_COUNT =
    sizeof(INDOOR_PALETTES) / sizeof(INDOOR_PALETTES[0]);

// ═══ INDOOR ENTITY PLACEMENT ═════════════════════════════════════
//
// Clamp (not reject) is intentional: rejection-based logic
// would silently drop entities anchored to corner patches,
// because their seed-determined position keeps landing in the
// wall margin and never recovers. Clamping shifts the candidate
// to the legal-box edge and lets the existing footprint-overlap
// check handle any pile-ups that result.
// Margin doubled (was 10) so that paintings and snapshots
// mounted on indoor walls are visibly separated from spawning
// entities — the entity's footprint now reads as distinct from
// the wall surface and any artwork on it.
inline constexpr float INDOOR_ENTITY_WALL_MARGIN = 20.0f;

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════

// Mood lifecycle (doors)
void apply_mood(Cartridge* c, uint32_t mood, wgpu::Queue& queue);
void request_mood_transition(Cartridge* c, uint32_t mood);
// Appliers (apply_mood's four named sub-functions)
void apply_mood_lighting(Cartridge* c, const MoodProfile& m, wgpu::Queue& queue);
void apply_mood_spot_lights(Cartridge* c, const MoodProfile& m, wgpu::Queue& queue);
void apply_mood_indoor_shell(Cartridge* c, const MoodProfile& m, wgpu::Queue& queue);
void apply_mood_anchor_ribbon(Cartridge* c, uint32_t mood, wgpu::Queue& queue);
// Indoor support
void derive_indoor_lights(Cartridge* c, uint32_t seed, float bmin, float bmax,
    float ceiling_height, CeilingType ceiling_type = CeilingType::FLAT);
void generate_indoor_shell(Cartridge* c, wgpu::Queue& queue, const MoodProfile& m);
void clear_indoor_shell(Cartridge* c, wgpu::Queue& queue);
// Portals (door; the internals route through entities' force_spawn_portal_arch)
void force_spawn_back_portal(Cartridge* c, wgpu::Queue& queue);
// Per-frame uploads (doors)
void upload_lights(Cartridge* c, wgpu::Queue& queue);
void upload_portal_array(Cartridge* c, wgpu::Queue& queue);
// Derivers (door + the portal-detection pipeline's shared helpers)
const char* mood_name(uint32_t mood);
uint32_t derive_finite_radius(uint32_t seed, const MoodProfile& mood);
uint32_t pick_portal_mood(Cartridge* c, uint32_t seed, uint32_t prop);

} // namespace the_board
} // namespace t7
