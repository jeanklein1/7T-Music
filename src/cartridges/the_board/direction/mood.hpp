#pragma once
#include <cstdint>
#include "cartridges/the_board/realization/state.hpp"                    // wgpu, GPUSpotLightArray, MAX_SPOT_LIGHTS
#include "cartridges/the_board/contracts/mood_constants.hpp"   // MOOD_COUNT, the Mood IDs, WORLD_DRAW_LIVE
#include "cartridges/the_board/contracts/agent_tiers.hpp"      // TIER_LIVE — the doorway witness reads a walker's contact_radius (ATRIUM_7)
#include "cartridges/the_board/contracts/spine_state.hpp"      // MoodState + the atmosphere vocabulary (CeilingType / MoodProfile / MOOD_TABLE)
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)
#include <algorithm>   // std::max, std::min, std::clamp   // (impl, merged)
#include <cmath>       // std::sqrt, std::sin, std::cos, std::cosh, std::floor, std::abs   // (impl, merged)
#include <iostream>    // mood / lighting / shell logs   // (impl, merged)
#include <iomanip>     // std::fixed, std::setprecision — the arc's facing witness (ATRIUM_5)   // (impl, merged)
#include <vector>      // shell mesh staging   // (impl, merged)

// ─── mood.hpp (MERGED: deps + atmosphere vocabulary + impl) ───────
//
// Atmosphere, indoor lighting, shell geometry.
//
// Mood is VOCABULARY + APPLIERS + THREE DOORS. This header owns the
// vocabulary — CeilingType, MoodProfile, MOOD_TABLE,
// the indoor wall palettes (the indoor treatment — sizes,
// bounds, dials — graduated to contracts/indoor_module.hpp) — and
// the DECLARATIONS of the three doors
// (apply_mood, upload_lights, mood_name) plus the appliers and
// derivers. The Mood IDs are file-scope vocabulary
// (mood_constants.hpp), consumed here. MOOD OWNS NO INSTANCE: struct
// MoodState's TYPE lives in contracts/spine_state.hpp; the instance
// mood_state is SPINE-OWNED orchestration (L38 — assembly only, K4 as
// amended). The force-spawn channel mood once computed values for left
// with the doors (ONE_WORLD-I U2).
// MERGED at the cohort tail:
// MoodState / CeilingType / MoodProfile / MOOD_TABLE + the request
// door decl live in contracts/spine_state.hpp (the spine's organ
// contract — the demo sentence includes mood_constants, so the
// DEMO-reading MoodState rides the spine tier; tile_world/ribbon/the
// config tables read them early);
// this file keeps MoodDeps, the
// decls, and every definition. COHORT: after ribbon/input
// (the fan's door owners + ORB_MOOD_TABLE), before the
// machine natives (they call derive_finite_radius).
//
// The impl additionally reaches the spine-resident state
// (mood_state / cpuSpotLights_ / sun + clear colors / world_state_ and
// the feature-gate flags), the converted modules' surfaces (orbs'
// configure, render_passes' compute_spot_light_vp), ARCH_TIERS / ArchIdx
// (entity_pipeline.hpp), solve_catenary_a (seed_utils.hpp), and
// Dim::PATCH_EXTENT (patch_system.hpp).
//
// SEAM[mood:K1] apply_mood is the single canonical mood entry point.
//   Every mood activation — boot and rebirth alike — funnels through
//   here. The orchestrator owns only ordering and the activate-mood
//   bookkeeping; the substantive work splits across three named
//   sub-functions (apply_mood_lighting, _spot_lights, _indoor_shell).
// apply_mood orchestrates the named applier helpers; the appliers
//   match the natural seams in the flow, and their call order is
//   load-bearing.
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

struct WorldState;   // patch_system.hpp — the doors read seeds/bounds (reference members/params; fwd suffices)
// fwd — the deps face's true reaches and the fan's TARGET organs
// (reference members/params; complete types arrive with their owners
// in the cohort). GPUState + the CPU light array come complete
// from state.hpp (included above).
class Renderer;
struct GoLState;   struct EntitiesState; struct MachineCtx;
struct OrbsState;   struct OrbsDeps;
struct PawnState;

// ═══ MOOD STATE + PROFILE VOCABULARY — GRADUATED ═════════════════
// MoodState / CeilingType / MoodProfile / MOOD_TABLE live in
// contracts/spine_state.hpp: the early consumers read the
// contract; the instance stays at the root.

// ═══ THE WORLD-DRAW BANK'S NOTE ══════════════════════════════════

// SCHEME_WEIGHTS graduated to contracts/mood_constants.hpp (ORGAN_4 P3d)
// as WORLD_DRAW_LIVE: the organ may not include a direction file, so a
// dial on it was impossible until it had a contracts home. It is C3
// DESTRUCTIVE and enrolls with the GEN chip and no wiring. Three of the
// bank's four axes — the portal density, the destination law and the
// portal palette — left with the doors at ONE_WORLD-I U2; the scheme
// roll is what a fresh world still draws.

// ═══ INDOOR WALL PALETTE ═════════════════════════════════════════
//
// SEAM[mood:tuning-data] the palettes are consumed only by
//   apply_mood_indoor_shell; the indoor lighting SCHEMES (the other
//   half of this seam) are consumed only by derive_indoor_lights and
//   stay impl-side (the MODULE IMPLEMENTATION below) — module-internal authoring tables.
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
// ATRIUM_5's string witness asserted that the atrium's pinned index named
// "warm charcoal", and ATRIUM_12 retired it when the entrance stopped
// pointing at a row. Both are gone with the mood. Row 7 stays in the roll
// for ordinary rooms, and INDOOR_PALETTES is untouched.

// ═══ INDOOR ENTITY PLACEMENT → THE INDOOR MODULE ═════════════════
//
// The whole indoor treatment — the size/bounds policy table and
// the three dials (INDOOR_LIVE.height_cap_fraction, INDOOR_LIVE.ribbon_scale,
// INDOOR_ENTITY_WALL_MARGIN) — lives in contracts/indoor_module.hpp:
// mood's insert on the spawn chain, graduated to the contracts tier
// because its consumers (grounded/spheres/cube_behaviors/ribbon)
// precede this header in the cohort. One table, three dials — the
// tuning session edits rows and dials, never wiring.

// ═══ THE DEPS FACE ═══════════════════════════════════════════════
//
// Mood's own organs plus its true reaches — the atmosphere author's
// face: the mood organ, the sun/clear channel, the CPU light staging
// array, the realization pokes
// (GPUState uploads, the frustum-cull flag), the gol mood gate (the
// THE FLAG CHANNEL [mood -> gol]), and a const view of entities (the
// indoor derivations read standing arches). The fan's TARGET organs
// are deliberately NOT members (the B ruling, input's precedent):
// orbs/pawn pairs + the machine face ride apply_mood's
// parameters — the spine addresses the fan's bodies at the call site,
// through the owner command doors.
struct MoodDeps {
    MoodState&           mood_state_;
    const WorldState&    world_state_;
    GPUState&            gpuState_;
    Renderer&            renderer_;          // set_frustum_cull_active (per-mood realization poke)
    GoLState&            gol_state_;         // mood_allowed — the flag channel [mood -> gol]
    const EntitiesState& entities_state_;    // the indoor derivations read standing arches
    float (&sunDirection_)[3];
    float (&sunColor_)[3];
    float (&clearColor_)[3];
    GPUSpotLightArray&   cpuSpotLights_;
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════

// Mood lifecycle (doors). The fan's targets ride apply_mood's tail
// parameters — organ-named, addressed by the spine at the call site.
void apply_mood(MoodDeps* c, uint32_t mood, wgpu::Queue& queue,
    MachineCtx& machine_ctx,
    OrbsState& orbs_state, OrbsDeps& orbs_deps,
    PawnState& pawn_state);
// Appliers (apply_mood's four named sub-functions; each takes only
// the targets its own fan drives)
void apply_mood_arrival(MoodDeps* c, const MoodProfile& m, wgpu::Queue& queue);  // ATRIUM_9 — the orbit, first
void apply_mood_regime(MoodDeps* c, const MoodProfile& m);     // REGIME_1 — the roll, first
void apply_mood_lighting(MoodDeps* c, const MoodProfile& m, wgpu::Queue& queue);
void apply_mood_spot_lights(MoodDeps* c, const MoodProfile& m, wgpu::Queue& queue);
void apply_mood_indoor_shell(MoodDeps* c, const MoodProfile& m, wgpu::Queue& queue);
// Indoor support
// ATRIUM_5 — forced_scheme takes the shape's light_scheme column: SCHEME_ROLL
// lets the seed draw, anything else is the pinned scheme id. The shape's
// columns arrive unpacked here, as wall_height and ceiling_type already do.
void derive_indoor_lights(MoodDeps* c, uint32_t seed, float bmin, float bmax,
    float wall_height, CeilingType ceiling_type = CeilingType::FLAT,
    uint32_t forced_scheme = SCHEME_ROLL);
void generate_indoor_shell(MoodDeps* c, wgpu::Queue& queue, const MoodProfile& m,
    const IndoorPalette& pal);
void clear_indoor_shell(MoodDeps* c, wgpu::Queue& queue);
// Per-frame uploads (door)
void upload_lights(MoodDeps* c, wgpu::Queue& queue);
// Derivers (door)
const char* mood_name(uint32_t mood);
uint32_t derive_finite_radius(uint32_t seed, const MoodProfile& mood);


// ═══ MODULE IMPLEMENTATION ════════════════════════════════════════
//
// The doors + appliers + derivers. The bodies reach the deps face
// (c->mood_state_ / c->world_state_ / c->gpuState_ / c->renderer_ /
// c->gol_state_ / c->entities_state_ / the sun + clear channel / the
// CPU light array), the fan's
// TARGET organs (parameters — orbs/pawn + the machine
// face), ARCH_TIERS /
// ArchIdx (contracts/spawn_services.hpp), solve_catenary_a
// (seed_utils.hpp), and Dim::PATCH_EXTENT (patch_system.hpp).
//
// THE CHANNEL: the force-spawn mutation of the arch belongs to the
// arch's owner. The indoor lighting-scheme tables live impl-side:
// single consumer (derive_indoor_lights); the wall PALETTES — the
// other half of SEAM[mood:tuning-data] — sit above with the deps.


// ═══ TUNING DATA (impl-side — internal authoring tables) ═════════
//
// Per-mood indoor lighting schemes. Seed-driven (a per-mood roll picks
// one of the schemes from the table), then per-light parameters are
// sampled from the chosen entry. The indoor wall PALETTES — this
// seam's other half — live declaration-side in mood.hpp
// (SEAM[mood:tuning-data] names the split).

// ── Indoor Lighting Schemes ───────────────────────────────

enum class LightAnchor : uint32_t {
    CEILING, WALL_NORTH, WALL_SOUTH, WALL_EAST, WALL_WEST
};

struct IndoorLightProp {
    static constexpr uint32_t SCHEME = 1100u;
    static constexpr uint32_t WALL_PAIR = 1101u;
    static constexpr uint32_t ANCHOR_PICK = 1102u;
    static constexpr uint32_t SLOT_BASE = 1110u;  // + slot*10 + field
    // Per-slot field offsets
    static constexpr uint32_t LATERAL = 0u;
    static constexpr uint32_t HEIGHT = 1u;
    static constexpr uint32_t INTENSITY = 2u;
    static constexpr uint32_t INNER_CONE = 3u;
    static constexpr uint32_t OUTER_CONE = 4u;
    static constexpr uint32_t WARMTH = 5u;
    static constexpr uint32_t AIM_PITCH = 6u;
    static constexpr uint32_t AIM_YAW = 7u;
};

struct LightSlotDef {
    LightAnchor anchor;
    float intensity_mean, intensity_sigma;
    float inner_mean, inner_sigma;    // inner half-angle (radians)
    float outer_mean, outer_sigma;    // outer half-angle (radians)
    float warmth_mean, warmth_sigma;  // 0 = warm amber, 1 = cool blue
    float aim_pitch_mean, aim_pitch_sigma;  // radians
    float aim_yaw_mean, aim_yaw_sigma;      // radians
    // Anchor-surface position (carried through from LightSchemeSlot).
    float lat_mean, lat_sigma;        // along anchor surface
    float hfrac_mean, hfrac_sigma;    // ceiling: Z; walls: height
};

// Vault adds an uplight when count < 4, so MOOD_INDOOR_VAULT reads one row up.
// SCHEME_WEIGHTS graduated to contracts/mood_constants.hpp (ORGAN_4 P3d)
// as WORLD_DRAW_LIVE.scheme_weights — the roll below reads the bank.
// SCHEME_COUNT went with it, because it SIZES the bank's row; the two
// tables it also sizes are here and read it unqualified, unchanged.
inline constexpr const char* SCHEME_NAMES[] = { "Cathedral", "Quartet", "Gallery", "Sanctum" };
inline constexpr const char* ANCHOR_NAMES[] = { "ceiling", "wall_N", "wall_S", "wall_E", "wall_W" };

// ── Lighting Scheme Table ──

enum class AnchorRole : uint32_t {
    CEILING,    // always ceiling
    WALL_A,     // seed-selected wall pair, side A
    WALL_B,     // seed-selected wall pair, side B
    SEED_PICK   // anchor chosen from seed (ceiling or wall)
};

struct LightSchemeSlot {
    AnchorRole role;
    float intensity_mean, intensity_sigma;
    float inner_mean, inner_sigma;
    float outer_mean, outer_sigma;
    float warmth_mean, warmth_sigma;
    float aim_pitch_mean, aim_pitch_sigma;
    float aim_yaw_mean, aim_yaw_sigma;
    float lat_mean, lat_sigma;
    float hfrac_mean, hfrac_sigma;
};

struct LightScheme {
    uint32_t slot_count;
    LightSchemeSlot slots[4];  // MAX_SPOT_LIGHTS
};

// ── Scheme Definitions ──
//                                role            int_μ  int_σ  inn_μ inn_σ out_μ out_σ wrm_μ wrm_σ  pit_μ  pit_σ  yaw_μ  yaw_σ   lat_μ lat_σ  hfrac_μ hfrac_σ
inline constexpr LightScheme LIGHT_SCHEMES[SCHEME_COUNT] = {
    /* 0: Cathedral — ceiling primary + 2 wall sconces */
    { 3, {
        { AnchorRole::CEILING,   8.0f, 2.5f, 0.6f, 0.2f,  1.2f, 0.15f, 0.35f, 0.20f,  0.0f,  0.12f, 0.0f, 0.12f,   0.50f, 0.15f, 0.65f, 0.10f },
        { AnchorRole::WALL_A,    5.0f, 1.5f, 0.4f, 0.15f, 1.0f, 0.15f, 0.20f, 0.15f,  0.60f, 0.40f, 0.0f, 0.30f,   0.50f, 0.15f, 0.65f, 0.10f },
        { AnchorRole::WALL_B,    5.0f, 1.5f, 0.4f, 0.15f, 1.0f, 0.15f, 0.75f, 0.15f,  0.60f, 0.40f, 0.0f, 0.30f,   0.50f, 0.15f, 0.65f, 0.10f },
    }},
    /* 1: Quartet — 4 ceiling lights at quadrant corners.
     *    Tight sigma (0.07) keeps each light pinned in its quadrant;
     *    means at 0.30/0.70 produce a 2x2 layout that fills the
     *    room evenly. Per-light intensity dropped to 4.0±1.0 to
     *    keep total scene brightness comparable to Cathedral
     *    (which has ~8.0 from a single ceiling light + sconces). */
    { 4, {
        { AnchorRole::CEILING,   4.0f, 1.0f, 0.6f, 0.2f,  1.2f, 0.15f, 0.35f, 0.20f,  0.0f,  0.10f, 0.0f, 0.10f,   0.30f, 0.07f, 0.30f, 0.07f },
        { AnchorRole::CEILING,   4.0f, 1.0f, 0.6f, 0.2f,  1.2f, 0.15f, 0.35f, 0.20f,  0.0f,  0.10f, 0.0f, 0.10f,   0.70f, 0.07f, 0.30f, 0.07f },
        { AnchorRole::CEILING,   4.0f, 1.0f, 0.6f, 0.2f,  1.2f, 0.15f, 0.35f, 0.20f,  0.0f,  0.10f, 0.0f, 0.10f,   0.30f, 0.07f, 0.70f, 0.07f },
        { AnchorRole::CEILING,   4.0f, 1.0f, 0.6f, 0.2f,  1.2f, 0.15f, 0.35f, 0.20f,  0.0f,  0.10f, 0.0f, 0.10f,   0.70f, 0.07f, 0.70f, 0.07f },
    }},
    /* 2: Gallery — 2 opposing wall lights, no ceiling */
    { 2, {
        { AnchorRole::WALL_A,    7.0f, 2.0f, 0.4f, 0.15f, 1.1f, 0.15f, 0.25f, 0.20f,  0.55f, 0.45f, 0.0f, 0.35f,   0.50f, 0.15f, 0.65f, 0.10f },
        { AnchorRole::WALL_B,    7.0f, 2.0f, 0.4f, 0.15f, 1.1f, 0.15f, 0.65f, 0.20f,  0.55f, 0.45f, 0.0f, 0.35f,   0.50f, 0.15f, 0.65f, 0.10f },
    }},
    /* 3: Sanctum — single dramatic source */
    { 1, {
        { AnchorRole::SEED_PICK, 10.0f, 2.5f, 0.5f, 0.2f, 1.2f, 0.15f, 0.45f, 0.25f,  0.50f, 0.40f, 0.0f, 0.30f,   0.50f, 0.15f, 0.65f, 0.10f },
    }},
};

// ═══ INDOOR LIGHT DERIVATION ═════════════════════════════════════

inline void derive_indoor_lights(MoodDeps* c, uint32_t seed, float bmin, float bmax,
    float wall_height, CeilingType ceiling_type, uint32_t forced_scheme) {
    c->cpuSpotLights_ = GPUSpotLightArray{};

    float room_size = bmax - bmin;
    float room_range = room_size * 0.8f;

    // Scheme selection
    uint32_t scheme = select_tier(seed, IndoorLightProp::SCHEME,
        WORLD_DRAW_LIVE.scheme_weights, SCHEME_COUNT);
    // ATRIUM_13 — THE WEIGHT-0 GUARD IS RETIRED WITH THE ROW IT PROTECTED, and
    // ITS FINDING IS KEPT HERE SO IT IS NOT LOST WITH THE CODE. The guard read
    //
    //     if (WORLD_DRAW_LIVE.scheme_weights[scheme] <= 0.0f) scheme = 0u;
    //
    // and it was RIGHT: select_weighted returns count-1 when the roll lands
    // past the cumulative sum, which a hash of exactly 1.0 does, so one seed in
    // four billion would have handed an ordinary room the atrium's pinned
    // scheme — a weight of 0 that does not quite retire a row. Its subject is
    // gone: every one of the four rows now carries a nonzero weight, and the
    // epsilon tail can only land on the last real scheme. IF ANY SCHEME IS EVER
    // WEIGHTED 0 AGAIN, THE GUARD COMES BACK WITH IT. The mood walk states the
    // same hazard at pick_mood_weighted_ and answers it by skipping zero rows,
    // which is where a reader can find the shape of the cure.
    // ATRIUM_5 — and the SHAPE may say which, after the roll so the roll's
    // props stay consumed and every other seed-derived value below is
    // unmoved.
    if (forced_scheme != SCHEME_ROLL) scheme = std::min(forced_scheme, SCHEME_COUNT - 1u);

    // Wall pair: N/S or E/W — gives axis variety across seeds
    bool use_ew = cpu_hash_f(seed, IndoorLightProp::WALL_PAIR) > 0.5f;
    LightAnchor wall_a = use_ew ? LightAnchor::WALL_EAST : LightAnchor::WALL_NORTH;
    LightAnchor wall_b = use_ew ? LightAnchor::WALL_WEST : LightAnchor::WALL_SOUTH;

    // Read slot definitions from scheme table
    const auto& sch = LIGHT_SCHEMES[scheme];
    LightSlotDef slots[MAX_SPOT_LIGHTS];
    uint32_t count = sch.slot_count;

    for (uint32_t i = 0; i < count; i++) {
        const auto& src = sch.slots[i];
        // Resolve anchor role to concrete LightAnchor
        LightAnchor anchor;
        switch (src.role) {
        case AnchorRole::CEILING:   anchor = LightAnchor::CEILING; break;
        case AnchorRole::WALL_A:    anchor = wall_a; break;
        case AnchorRole::WALL_B:    anchor = wall_b; break;
        case AnchorRole::SEED_PICK: {
            float anchor_roll = cpu_hash_f(seed, IndoorLightProp::ANCHOR_PICK);
            anchor = (anchor_roll < 0.55f) ? LightAnchor::CEILING
                : (anchor_roll < 0.775f) ? wall_a : wall_b;
            break;
        }
        }
        slots[i] = { anchor,
            src.intensity_mean, src.intensity_sigma,
            src.inner_mean, src.inner_sigma,
            src.outer_mean, src.outer_sigma,
            src.warmth_mean, src.warmth_sigma,
            src.aim_pitch_mean, src.aim_pitch_sigma,
            src.aim_yaw_mean, src.aim_yaw_sigma,
            src.lat_mean, src.lat_sigma,
            src.hfrac_mean, src.hfrac_sigma };
    }

    // ATRIUM_13 — THE DRAWN AIM, KEPT. pitch, yaw and warmth are sampled per
    // slot below and then consumed on the spot: the aim becomes a unit
    // direction and the warmth becomes a colour, so GPUSpotLight carries
    // neither. The [Lights] witness needs exactly these three — the aim is the
    // thing that must be SEEN to be drawn rather than authored — so they are
    // kept here, where they are true, instead of being re-derived downstream
    // from a direction that has already been normalized and clamped.
    //
    // ATRIUM_14 — AND THEY RETIRE WHEN [Lights] RETIRES. These three exist for
    // ONE reader, the witness at the tail of this function, and for no other:
    // GPUSpotLight carries a direction and a colour, and every downstream
    // consumer reads those. An instrument that outlives its warrant is dead
    // weight, and one that leaves fields behind is worse — so the warrant is
    // written where the fields are, not only where the line is. NOTHING
    // GPU-SHAPED GREW TO HOLD THEM: they are three function-local arrays,
    // MAX_SPOT_LIGHTS floats each, alive for the length of one derivation.
    // The witness observes its subject; it did not change it.
    float drawn_pitch[MAX_SPOT_LIGHTS]{}, drawn_yaw[MAX_SPOT_LIGHTS]{}, drawn_warm[MAX_SPOT_LIGHTS]{};

    // Derive each light from its slot spec + seed
    for (uint32_t i = 0; i < count; i++) {
        const auto& s = slots[i];
        uint32_t base = IndoorLightProp::SLOT_BASE + i * 10;
        auto& L = c->cpuSpotLights_.lights[i];

        float lat = std::clamp(
            cpu_sample_gaussian(seed, base + IndoorLightProp::LATERAL,
                s.lat_mean, s.lat_sigma),
            0.1f, 0.9f);
        float hfrac_raw = cpu_sample_gaussian(seed,
            base + IndoorLightProp::HEIGHT, s.hfrac_mean, s.hfrac_sigma);
        float hfrac = (s.anchor == LightAnchor::CEILING)
            ? std::clamp(hfrac_raw, 0.1f, 0.9f)    // ceiling: Z-axis position
            : std::clamp(hfrac_raw, 0.4f, 0.85f);  // walls: sconce height

        float wall_off = 1.0f;
        float px, py, pz;

        // Position: anchor-dependent placement on surface
        switch (s.anchor) {
        case LightAnchor::CEILING:
            px = bmin + lat * room_size;
            py = wall_height - 0.5f;
            pz = bmin + hfrac * room_size;
            break;
        case LightAnchor::WALL_NORTH:
            px = bmin + lat * room_size;
            py = wall_height * hfrac;
            pz = bmax - wall_off;
            break;
        case LightAnchor::WALL_SOUTH:
            px = bmin + lat * room_size;
            py = wall_height * hfrac;
            pz = bmin + wall_off;
            break;
        case LightAnchor::WALL_EAST:
            px = bmax - wall_off;
            py = wall_height * hfrac;
            pz = bmin + lat * room_size;
            break;
        case LightAnchor::WALL_WEST:
            px = bmin + wall_off;
            py = wall_height * hfrac;
            pz = bmin + lat * room_size;
            break;
        }

        float pitch = cpu_sample_gaussian(seed, base + IndoorLightProp::AIM_PITCH,
            s.aim_pitch_mean, s.aim_pitch_sigma);
        float yaw = cpu_sample_gaussian(seed, base + IndoorLightProp::AIM_YAW,
            s.aim_yaw_mean, s.aim_yaw_sigma);

        float dx, dy, dz;
        if (s.anchor == LightAnchor::CEILING) {
            // pitch=0 → straight down; positive tilts off-vertical
            pitch = std::clamp(pitch, 0.0f, 0.50f);
            yaw = std::clamp(yaw, -0.60f, 0.60f);
            dx = std::sin(pitch) * std::sin(yaw);
            dy = -std::cos(pitch);
            dz = std::sin(pitch) * std::cos(yaw);
        }
        else {
            // pitch=0 → horizontal into room; π/2 → straight down
            // Floor at 0.08 ensures light never aims upward.
            // Ceiling at 1.45 (~83°) allows steep floor pools.
            pitch = std::clamp(pitch, 0.08f, 1.45f);
            yaw = std::clamp(yaw, -0.80f, 0.80f);
            float cp = std::cos(pitch), sp = std::sin(pitch);
            float sy = std::sin(yaw);
            switch (s.anchor) {
            case LightAnchor::WALL_NORTH: dx = sy; dy = -sp; dz = -cp; break;
            case LightAnchor::WALL_SOUTH: dx = sy; dy = -sp; dz = cp; break;
            case LightAnchor::WALL_EAST:  dx = -cp; dy = -sp; dz = sy; break;
            case LightAnchor::WALL_WEST:  dx = cp; dy = -sp; dz = sy; break;
            default: dx = 0; dy = -1; dz = 0; break;  // unreachable
            }
        }

        // Normalize direction
        float dlen = std::sqrt(dx * dx + dy * dy + dz * dz);
        dx /= dlen; dy /= dlen; dz /= dlen;
        drawn_pitch[i] = pitch; drawn_yaw[i] = yaw;   // ATRIUM_13 — post-clamp, as flown

        // Intensity and cone angles
        float intensity = std::max(1.0f,
            cpu_sample_gaussian(seed, base + IndoorLightProp::INTENSITY,
                s.intensity_mean, s.intensity_sigma));
        float inner_half = std::max(0.2f,
            cpu_sample_gaussian(seed, base + IndoorLightProp::INNER_CONE,
                s.inner_mean, s.inner_sigma));
        float outer_half = std::max(inner_half + 0.3f,
            cpu_sample_gaussian(seed, base + IndoorLightProp::OUTER_CONE,
                s.outer_mean, s.outer_sigma));
        // Hard cap: outer cone must not exceed shadow map FOV capability.
        // At 1.3 rad (75°), shadow FOV = 2×1.3+0.2 = 2.8 rad → coverable.
        static constexpr float MAX_OUTER_HALF = 1.3f;
        outer_half = std::min(outer_half, MAX_OUTER_HALF);
        inner_half = std::min(inner_half, outer_half - 0.1f);

        // Color: warm amber (1.0,0.85,0.65) ↔ cool blue (0.80,0.88,1.0)
        float w = std::clamp(
            cpu_sample_gaussian(seed, base + IndoorLightProp::WARMTH,
                s.warmth_mean, s.warmth_sigma),
            0.0f, 1.0f);
        drawn_warm[i] = w;   // ATRIUM_13 — the drawn warmth, before it becomes a colour
        float cr = 1.00f + w * (0.80f - 1.00f);
        float cg = 0.85f + w * (0.88f - 0.85f);
        float cb = 0.65f + w * (1.00f - 0.65f);

        L.position[0] = px; L.position[1] = py; L.position[2] = pz;
        L.direction[0] = dx; L.direction[1] = dy; L.direction[2] = dz;
        L.color[0] = cr; L.color[1] = cg; L.color[2] = cb;
        L.intensity = intensity;
        L.inner_cone = std::cos(inner_half);
        L.outer_cone = std::cos(outer_half);
        L.range = (s.anchor == LightAnchor::CEILING)
            ? wall_height + 30.0f : room_range;

    }

    c->cpuSpotLights_.count = count;

    // ─── Vault Uplight ───────────────────────────────────────
    //
    if (ceiling_type == CeilingType::VAULT && count < MAX_SPOT_LIGHTS) {
        auto& L = c->cpuSpotLights_.lights[count];
        float center = (bmin + bmax) * 0.5f;
        L.position[0] = center;
        L.position[1] = 2.0f;           // near floor level
        L.position[2] = center;
        L.direction[0] = 0.0f;
        L.direction[1] = 1.0f;           // straight up
        L.direction[2] = 0.0f;
        L.color[0] = 0.95f;
        L.color[1] = 0.90f;
        L.color[2] = 0.80f;              // warm ambient
        L.intensity = 2.5f;               // subtle — structural reveal, not flood
        L.inner_cone = std::cos(0.9f);    // ~52° half-angle
        L.outer_cone = std::cos(1.25f);   // ~72° half-angle — within shadow FOV cap
        // Reaches the crown by headroom rather than by computing it: the
        // light sits at y = 2.0, so the reach needed is crown - 2.0, at most
        // 90.5 against this 105.0. Holds at every legal radius with 14.5 wu
        // spare at the worst. vault_crown_height() is the exact quantity if
        // VAULT_RISE_FRACTION ever moves.
        L.range = wall_height + 80.0f;  // reach the crown with headroom
        // ATRIUM_13 — the uplight is AUTHORED WHOLE: it writes a direction and a
        // colour directly and never draws a pitch, a yaw or a warmth. Its three
        // witness slots stay at their zero-init, and the line reads aim(0,0)
        // warm=0.00 for it — which is the truth about this lamp, not a hole.
        count++;
        c->cpuSpotLights_.count = count;
        std::cout << "[Lighting] Added vault uplight (slot " << (count - 1) << ")\n";
    }

    std::cout << "[Lighting] " << SCHEME_NAMES[scheme]
        << " (" << count << " lights, "
        << (use_ew ? "E/W" : "N/S") << " walls)\n";

    // ═══ DIAGNOSTIC: the indoor light stage (ATRIUM_13) ══════════
    // ABSENT AND DIM LOOK IDENTICAL FROM A CHAIR. The line above names
    // the scheme and counts the slots; this one says what the lamps ARE,
    // after the scheme is drawn and the room is scaled, so a dark room
    // can be told from an unlit one without reading source. One line per
    // world, at the derivation — which is where `scheme` and `count` are
    // both in hand; apply_mood_spot_lights has neither.
    //
    // THE CONES ARE STORED AS COSINES (GPUSpotLight.inner_cone /
    // outer_cone) and printed back as HALF-ANGLES in radians, because the
    // half-angle is what LIGHT_SCHEMES authors and therefore the number a
    // reader can compare against a row. There is no warmth on the GPU
    // struct — warmth is baked into the colour at derivation — so the
    // colour is what prints.
    //
    // Not gated by INSTRUMENTS: one line per world entry, and it is the
    // instrument this round is read against. It retires on Jean's word,
    // beside the passer census and the camera witness.
    std::cout << "[Lights] scheme=" << SCHEME_NAMES[scheme] << " slots=" << count;
    for (uint32_t i = 0; i < count; i++) {
        const auto& L = c->cpuSpotLights_.lights[i];
        std::cout << " | s" << i << " (" << std::fixed << std::setprecision(0)
                  << L.position[0] << "," << L.position[1] << "," << L.position[2] << ")"
                  << " aim(" << std::setprecision(2) << drawn_pitch[i] << "," << drawn_yaw[i] << ")"
                  << " int=" << L.intensity
                  << " cone=" << std::acos(std::clamp(L.inner_cone, -1.0f, 1.0f))
                  << "/"      << std::acos(std::clamp(L.outer_cone, -1.0f, 1.0f))
                  << " warm=" << drawn_warm[i];
    }
    std::cout << std::setprecision(6) << "\n";
}

// ═══ THE ATMOSPHERE DRAW (ATMOS_1) ═══════════════════════════════
// (seed, definition) → instance. PURE: reads the seed and the
// definition, returns a value, touches nothing else — the seeded-sampler
// law (theory §12): the seed is the whole biography, so the same seed
// draws the same sky. The props
// below are FROZEN; they draw off active_seed directly, beside 999u /
// 77u / 5800u / 7950u, and the ATMOS_1 census found the block free.
struct AtmosphereInstance {
    float    sun_direction[3];   // the direction light travels (the table's convention); readers normalize
    float    sun_color[3];
    float    sun_intensity;
    float    sun_ambient;
    float    fog_density;        // the REST the U4 seam composes over
    float    fog_color[3];
    float    clear_color[3];
};

struct AtmosProp {
    static constexpr uint32_t SUN_AZ      = 8100u;
    static constexpr uint32_t SUN_EL      = 8101u;
    static constexpr uint32_t REGIME      = 8102u;   // the WORLD's roll (apply_mood_regime) — kept here because
                                                     // the atmosphere was its first subscriber; the value is frozen
    static constexpr uint32_t INTENSITY   = 8103u;
    static constexpr uint32_t AMBIENT     = 8104u;
    static constexpr uint32_t FOG_DENSITY = 8105u;
    static constexpr uint32_t SUN_COLOR   = 8106u;   // ATMOS_2 — brightness factors
    static constexpr uint32_t FOG_COLOR   = 8107u;
    static constexpr uint32_t CLEAR_COLOR = 8108u;
};

// ± spread, uniform. Spread 0 returns 0 without taking a hash, so a
// zero-spread row costs nothing and moves nothing — the carry witness
// in spine_state.hpp leans on this.
inline float atmos_jitter(uint32_t seed, uint32_t prop, float spread) {
    if (spread <= 0.0f) return 0.0f;
    return spread * (2.0f * cpu_hash_f(seed, prop) - 1.0f);
}

// A COLOUR'S SPREAD is a ± on BRIGHTNESS over the whole triple, hue
// kept: one factor for all three lanes, clamped to [0,1]. Spread 0
// multiplies by exactly 1.0f, the IEEE identity — a carried colour is
// bit-identical to the point value it replaced.
inline void atmos_draw_color(uint32_t seed, uint32_t prop,
                             const float in[3], float spread, float out[3]) {
    const float f = 1.0f + atmos_jitter(seed, prop, spread);
    for (int i = 0; i < 3; ++i) out[i] = std::clamp(in[i] * f, 0.0f, 1.0f);
}

inline AtmosphereInstance draw_atmosphere(uint32_t seed, const Atmosphere& a, uint32_t regime) {
    AtmosphereInstance out{};

    // ── the sun's bearing ──
    // Spread 0 on both axes copies the centre EXACTLY — no trig round
    // trip — which is what keeps the carried rows bit-identical.
    if (a.sun_az_spread_deg <= 0.0f && a.sun_el_spread_deg <= 0.0f) {
        out.sun_direction[0] = a.sun_direction[0];
        out.sun_direction[1] = a.sun_direction[1];
        out.sun_direction[2] = a.sun_direction[2];
    } else {
        // The table's vector is the direction light TRAVELS (y < 0 by
        // day); the light's own bearing is its negation. Decompose that
        // bearing into elevation above the horizon and azimuth about +Y,
        // jitter both, clamp elevation so the light never sits on or
        // under the horizon (the shadow VP degenerates there), recompose.
        const float len = std::sqrt(a.sun_direction[0] * a.sun_direction[0]
                                  + a.sun_direction[1] * a.sun_direction[1]
                                  + a.sun_direction[2] * a.sun_direction[2]);
        const float lx = -a.sun_direction[0] / len;
        const float ly = -a.sun_direction[1] / len;
        const float lz = -a.sun_direction[2] / len;
        constexpr float DEG = 3.14159265359f / 180.0f;
        float el = std::asin(std::clamp(ly, -1.0f, 1.0f))
                 + DEG * atmos_jitter(seed, AtmosProp::SUN_EL, a.sun_el_spread_deg);
        const float az = std::atan2(lz, lx)
                 + DEG * atmos_jitter(seed, AtmosProp::SUN_AZ, a.sun_az_spread_deg);
        el = std::clamp(el, 5.0f * DEG, 88.0f * DEG);
        const float ce = std::cos(el);
        out.sun_direction[0] = -(ce * std::cos(az));
        out.sun_direction[1] = -std::sin(el);
        out.sun_direction[2] = -(ce * std::sin(az));
    }
    // ── the regime — the WORLD's, handed in (REGIME_1) ──
    // apply_mood_regime rolled it and wrote MoodState.regime before this
    // ran; the draw obeys. Indexed, never aliased: every enrolled leaf is
    // reached through `a`, so the reader census (tools/organ_readers.py)
    // can see that each dial has a reader.
    {
        const uint32_t t = regime;
        atmos_draw_color(seed, AtmosProp::SUN_COLOR, a.regime[t].sun_color, a.regime[t].sun_color_spread, out.sun_color);
        out.sun_intensity = std::max(0.0f, a.regime[t].intensity
                          + atmos_jitter(seed, AtmosProp::INTENSITY, a.regime[t].intensity_spread));
        out.sun_ambient   = std::max(0.0f, a.regime[t].ambient
                          + atmos_jitter(seed, AtmosProp::AMBIENT, a.regime[t].ambient_spread));
        // The fog's REST — the U4 seam composes the canvas's deviation over it.
        out.fog_density   = std::max(0.0f, a.regime[t].fog_density
                          + atmos_jitter(seed, AtmosProp::FOG_DENSITY, a.regime[t].fog_density_spread));
        atmos_draw_color(seed, AtmosProp::FOG_COLOR,   a.regime[t].fog_color,   a.regime[t].fog_color_spread,   out.fog_color);
        atmos_draw_color(seed, AtmosProp::CLEAR_COLOR, a.regime[t].clear_color, a.regime[t].clear_color_spread, out.clear_color);
    }
    return out;
}

// ═══ APPLY MOOD ══════════════════════════════════════════════════

// 0) THE ROLL (REGIME_1). (seed, the mood's regime law) → which regime
//    this world is drawn into — a WORLD fact, written to MoodState.regime
//    before any applier reads it. Runs first in apply_mood and first in
//    the boundary's mood re-speak (organ_flush), so a weight dial can
//    still move this world into another regime under the same seed: the
//    roll is the seed's, the thresholds are the weights. A family
//    SUBSCRIBES to the regime by carrying its own [REGIME_COUNT] columns
//    and indexing them with this field at its own apply — no second roll,
//    ever. The atmosphere is the first subscriber.
inline void apply_mood_regime(MoodDeps* c, const MoodProfile& m) {
    // One roll, scaled by the weights' sum rather than dividing the
    // weights (the panel's lanes stay independent; a lane of 0 is an
    // absent regime). The walk skips absent regimes, and the
    // float-epsilon miss lands on the last PRESENT regime rather than on
    // an absent one. All weights 0 → regime 0. ATMOS_1's arithmetic,
    // verbatim, and the same prop: the same seed draws the regime it drew.
    float sum = 0.0f;
    for (uint32_t i = 0; i < REGIME_COUNT; ++i)
        sum += std::max(0.0f, m.regime_weight[i]);
    uint32_t t = 0;
    if (sum > 0.0f) {
        const float roll = cpu_hash_f(c->world_state_.active_seed, AtmosProp::REGIME) * sum;
        float cumul = 0.0f;
        for (uint32_t i = 0; i < REGIME_COUNT; ++i) {
            const float w = std::max(0.0f, m.regime_weight[i]);
            if (w <= 0.0f) continue;
            cumul += w;
            t = i;
            if (roll < cumul) break;
        }
    }
    c->mood_state_.regime = t;
}

// 1) Atmospheric: THE DRAW, then the fan. (seed, definition) → instance,
//    re-run on every apply — a mood entry and a definition edit alike.
//    The seed is the world's, so a panel edit moves the instance WITH the
//    dial (same seed, shifted centre, same offset) rather than re-rolling
//    it. Touches GPU directly + a few member fields.
inline void apply_mood_lighting(MoodDeps* c, const MoodProfile& m, wgpu::Queue& /*queue*/) {
    const AtmosphereInstance ai = draw_atmosphere(c->world_state_.active_seed, m.atmos,
                                                  c->mood_state_.regime);   // rolled by apply_mood_regime
    const float len = std::sqrt(ai.sun_direction[0] * ai.sun_direction[0] +
                                ai.sun_direction[1] * ai.sun_direction[1] +
                                ai.sun_direction[2] * ai.sun_direction[2]);

    c->sunDirection_[0] = ai.sun_direction[0];
    c->sunDirection_[1] = ai.sun_direction[1];
    c->sunDirection_[2] = ai.sun_direction[2];

    // Push to GPU config so update_camera_vp builds the shadow VP from the correct direction.
    c->gpuState_.set_sun_direction(ai.sun_direction[0] / len,
                                   ai.sun_direction[1] / len,
                                   ai.sun_direction[2] / len);

    c->sunColor_[0] = ai.sun_color[0];
    c->sunColor_[1] = ai.sun_color[1];
    c->sunColor_[2] = ai.sun_color[2];
    c->mood_state_.sun_intensity = ai.sun_intensity;
    c->mood_state_.sun_ambient   = ai.sun_ambient;

    // The fog's REST — the mood's since ATMOS_1. The U4 seam
    // (phase_motion_drivers) composes the canvas's deviation over it
    // every frame; this is the rung-3 instance that seam reads.
    c->mood_state_.fog_rest_density  = ai.fog_density;
    c->mood_state_.fog_rest_color[0] = ai.fog_color[0];
    c->mood_state_.fog_rest_color[1] = ai.fog_color[1];
    c->mood_state_.fog_rest_color[2] = ai.fog_color[2];

    c->clearColor_[0] = ai.clear_color[0];
    c->clearColor_[1] = ai.clear_color[1];
    c->clearColor_[2] = ai.clear_color[2];

    c->gpuState_.set_terrain_amp_ceiling(m.shape.terrain_amp_ceiling);
    c->mood_state_.terrain_amp_ceiling = m.shape.terrain_amp_ceiling;
    // The GoL cap rides beside the amp column: indoors the cell lift caps
    // at the module's fraction of the ceiling; 0 disables (the derive
    // kernel's sentinel — outdoor byte-identical). Zones are torn down at
    // every rebirth, so every live zone was derived inside the mood it
    // lives in and the capping is complete, not approximate.
    c->gpuState_.set_indoor_height_cap(
        m.shape.indoor ? INDOOR_LIVE.height_cap_fraction * m.shape.wall_height : 0.0f);
    c->mood_state_.lights_dirty = true;

    // THE WITNESS. One line per (mood, seed, regime), not per draw
    // (ATMOS_1b): a line prints when that triple changes — every entry,
    // and a weight dial that moves this world into another regime, which
    // is the one event a drag should announce. A centre, colour or spread
    // drag re-draws every frame and says nothing here; the panel is its
    // readout. The same seed still prints the same line, and a boot must
    // print regime=1 int=0.9 amb=0.2 fog=0.003 for the sunset until
    // someone changes ATMOS_SUNSET on purpose. `regime=` is the LABEL's
    // number (1-based, the panel's "Regime N"): the operator never sees
    // the index. Function-local statics: the checker's [FLUSH] one-shot
    // in cartridge.hpp is the precedent.
    {
        static uint32_t last_mood = MOOD_COUNT, last_seed = 0u, last_regime = 0u;
        const bool regime_changed = c->mood_state_.active != last_mood
                                 || c->world_state_.active_seed != last_seed
                                 || c->mood_state_.regime != last_regime;
        if (regime_changed) {
            last_mood   = c->mood_state_.active;
            last_seed   = c->world_state_.active_seed;
            last_regime = c->mood_state_.regime;
            constexpr float RAD2DEG = 180.0f / 3.14159265359f;
            const float el = std::asin(std::clamp(-ai.sun_direction[1] / len, -1.0f, 1.0f)) * RAD2DEG;
            const float az = std::atan2(-ai.sun_direction[2], -ai.sun_direction[0]) * RAD2DEG;
            std::cout << "[Atmos] " << mood_name(c->mood_state_.active)
                      << " seed=" << c->world_state_.active_seed
                      << " regime=" << (c->mood_state_.regime + 1u)
                      << " int=" << ai.sun_intensity << " amb=" << ai.sun_ambient
                      << " sun el=" << el << " az=" << az
                      << " fog=" << ai.fog_density << "\n";
        }
    }
}

inline void apply_mood_spot_lights(MoodDeps* c, const MoodProfile& m, wgpu::Queue& queue) {
    c->cpuSpotLights_ = GPUSpotLightArray{};
    if (m.shape.indoor) {
        c->gpuState_.set_mute_coupling(Coupling::PAWN_TO_SUN_VP, true);

        const float bmin = -(float)c->world_state_.finite_radius * Dim::PATCH_EXTENT;
        const float bmax = ((float)c->world_state_.finite_radius + 1.0f) * Dim::PATCH_EXTENT;
        derive_indoor_lights(c, c->world_state_.active_seed, bmin, bmax, m.shape.wall_height, m.shape.ceiling_type, m.shape.light_scheme);

        for (uint32_t i = 0; i < c->cpuSpotLights_.count; i++) {
            compute_spot_light_vp(c->cpuSpotLights_.lights[i],
                                  c->cpuSpotLights_.lights[i].view_proj);
        }
        // ATLAS_1revB U3" — the staging write is gone. upload_lights sends
        // this same cpuSpotLights_ array whole into the frame-R block, view_proj
        // members included, and the shadow VS reads it there.
        c->mood_state_.spot_light_active = true;
    } else {
        c->gpuState_.set_mute_coupling(Coupling::PAWN_TO_SUN_VP, false);
        c->mood_state_.spot_light_active = false;
    }
}

// ═══ THE CROWN LAW — one spelling of the vault arithmetic ═════════
// Spring (wall top), rise, crown for a VAULT room over the finite
// bounds [bmin, bmax]. The generator and the camera ceiling clamp
// both consume THIS — no bare-literal twin survives at either site.
//
// THE ROOM DEFINES THE SPRING; THE HANG FITS UNDER IT. The spring was
// once derived from where the art hangs — wall_height x paint_y_frac,
// plus a top margin, plus a spring margin — so the room's shape depended
// on the hang. That was inert while the fraction was spelled twice, and
// became a live cycle the moment one home was given to it. The
// dependency now runs the other way: the wall stops where the room says
// it stops, and the hang is bounded under that. The clearance the old
// chain approximated (a margin above a NOMINAL paint centre) is
// strengthened by the reversal, because a dense hang's real top has no
// relation to a fraction of the ceiling.
struct VaultCrown { float spring_h; float rise; float crown_h; };
inline VaultCrown vault_crown(const MoodProfile& m, float bmin, float bmax) {
    static constexpr float VAULT_RISE_FRACTION  = 0.30f;
    // A floor beneath the dial above, not a dead term: it does not fire at
    // any legal radius only because VAULT_RISE_FRACTION is currently high.
    // Lower the dial and it binds.
    static constexpr float MIN_RISE_FLOOR       = 5.0f;
    const float half_span = (bmax - bmin) * 0.5f;
    const float spring_h = m.shape.wall_height;
    // The crown clears the spring by construction now — rise is >= 5 — so
    // the old min_rise term (wall_height - spring_h, guarding exactly
    // that) has nothing left to guard and is gone with the derivation.
    const float rise = std::max(half_span * VAULT_RISE_FRACTION, MIN_RISE_FLOOR);
    return VaultCrown{ spring_h, rise, spring_h + rise };
}
inline float vault_crown_height(const MoodProfile& m, float bmin, float bmax) {
    return vault_crown(m, bmin, bmax).crown_h;
}

inline void apply_mood_indoor_shell(MoodDeps* c, const MoodProfile& m, wgpu::Queue& queue) {
    if (m.shape.indoor && m.shape.ceiling_type != CeilingType::NONE) {
        // ATRIUM_5 — THE SHAPE MAY SAY WHICH. PALETTE_ROLL is "let the seed
        // draw", which is every room; a pinned index is clamped rather than
        // trusted, so a table that shrinks cannot read past its end.
        const uint32_t pal_idx = (m.shape.palette != PALETTE_ROLL)
            ? std::min(m.shape.palette, INDOOR_PALETTE_COUNT - 1u)
            : cpu_hash(c->world_state_.active_seed, 5800u) % INDOOR_PALETTE_COUNT;
        const IndoorPalette& pal = INDOOR_PALETTES[pal_idx];
        std::cout << "[Mood] Indoor palette: " << pal.name
                  << " (idx=" << pal_idx << ")\n";
        generate_indoor_shell(c, queue, m, pal);
    } else {
        clear_indoor_shell(c, queue);
    }

    // Camera ceiling clamp — consumes THE CROWN LAW (vault_crown_height).
    if (m.shape.indoor) {
        float effective_ceiling = m.shape.wall_height;
        if (m.shape.ceiling_type == CeilingType::VAULT) {
            const float bmin = -(float)c->world_state_.finite_radius * Dim::PATCH_EXTENT;
            const float bmax = ((float)c->world_state_.finite_radius + 1.0f) * Dim::PATCH_EXTENT;
            effective_ceiling = vault_crown_height(m, bmin, bmax);
        }
        c->gpuState_.set_ceiling_height(effective_ceiling);
    } else {
        c->gpuState_.set_ceiling_height(0.0f);
    }
}
// ── apply_mood (orchestrator) ──
//
inline void apply_mood(MoodDeps* c, uint32_t mood, wgpu::Queue& queue,
    MachineCtx& machine_ctx,
    OrbsState& orbs_state, OrbsDeps& orbs_deps,
    PawnState& pawn_state) {
    mood = std::min(mood, MOOD_COUNT - 1);
    c->mood_state_.active = mood;
    const auto& m = mood_def(mood);   // O1b — the definition IN FORCE

    // Frustum cull is mood-driven (not tied to indoor/outdoor).
    c->renderer_.set_frustum_cull_active(m.shape.allow_frustum_cull);

    // Per-mood feature gates: GoL zones, aura.
    // Aura policy: respect player preference when permitted, force off when forbidden.
    c->gol_state_.mood_allowed     = m.shape.allow_gol_zones;
    apply_aura_mood_policy(pawn_state, m.shape.allow_pawn_aura);  // the pawn door; byte-identical semantics

    apply_mood_regime(c, m);                   // REGIME_1 — the world's roll, before anything reads it
    apply_mood_lighting(c, m, queue);          // sun + fog + amp ceiling (foundational — sun is not a piece)
    if constexpr (ROSTER.spot_lights)          // ROSTER-GATE spot_lights (b) — indoor spot array never configured
        apply_mood_spot_lights(c, m, queue);   // indoor only
    if constexpr (ROSTER.indoor_shell)         // ROSTER-GATE indoor_shell (b) — walls/ceiling never generated
        apply_mood_indoor_shell(c, m, queue);  // shell + camera ceiling clamp
    if constexpr (ROSTER.orbs)                 // ROSTER-GATE orbs (b) — sky dome never configured
        // ORGAN_3b P3 — the world's definition, not the design table.
        // ORGAN_5 P1b — reseed TRUE: a mood change is a new world's sky,
        // so the init kernel re-runs and every orb is re-drawn. This is
        // the heavy path, and it is the one path that should be.
        configure_orbs(orbs_state, &orbs_deps, ORB_MOOD_LIVE[mood], queue,
            /*reseed=*/true);

    std::cout << "[Mood] Applied: " << mood_name(mood)
        << " (mood=" << mood
        << (m.shape.indoor ? " INDOOR" : " outdoor")
        << ")\n";
}

// ═══ INDOOR SHELL GENERATION ═════════════════════════════════════

inline void clear_indoor_shell(MoodDeps* c, wgpu::Queue& queue) {
    c->gpuState_.set_shell_index_count(0);
}

// Helper: push a quad (2 triangles) into vertex/index vectors
inline void push_quad(
    std::vector<ShellVertex>& verts,
    std::vector<uint32_t>& indices,
    float ax, float ay, float az,
    float bx, float by, float bz,
    float cx, float cy, float cz,
    float dx, float dy, float dz,
    float nx, float ny, float nz,
    const float* color
) {
    uint32_t base = static_cast<uint32_t>(verts.size());
    verts.push_back({ { ax, ay, az }, { nx, ny, nz }, { color[0], color[1], color[2] } });
    verts.push_back({ { bx, by, bz }, { nx, ny, nz }, { color[0], color[1], color[2] } });
    verts.push_back({ { cx, cy, cz }, { nx, ny, nz }, { color[0], color[1], color[2] } });
    verts.push_back({ { dx, dy, dz }, { nx, ny, nz }, { color[0], color[1], color[2] } });
    indices.push_back(base); indices.push_back(base + 1); indices.push_back(base + 2);
    indices.push_back(base); indices.push_back(base + 2); indices.push_back(base + 3);
}

inline void generate_indoor_shell(MoodDeps* c, wgpu::Queue& queue, const MoodProfile& m,
    const IndoorPalette& pal) {
    float bmin = -(float)c->world_state_.finite_radius * Dim::PATCH_EXTENT;
    float bmax = ((float)c->world_state_.finite_radius + 1.0f) * Dim::PATCH_EXTENT;
    float ch = m.shape.wall_height;

    std::vector<ShellVertex> verts;
    std::vector<uint32_t> indices;
    verts.reserve(2400);
    indices.reserve(12000);

    static constexpr float JOINT_OVERLAP = 3.0f;
    static constexpr float WALL_FLOOR = -50.0f;

    // ─── Compute wall height (depends on ceiling type) ───────
    //
    float wall_h = ch;  // flat ceiling: walls go to ceiling
    float crown_h = ch; // effective crown height for log
    float rise = 0.0f;

    if (m.shape.ceiling_type == CeilingType::VAULT) {
        const VaultCrown vc = vault_crown(m, bmin, bmax);   // THE CROWN LAW
        wall_h  = vc.spring_h;
        rise    = vc.rise;
        crown_h = vc.crown_h;
    }

    float wall_top = wall_h + JOINT_OVERLAP;

    // ─── 4 Walls (deep floor to wall_top) ────────────────────
    // South wall (z = bmin, facing +Z into room)
    push_quad(verts, indices,
        bmin, WALL_FLOOR, bmin, bmax, WALL_FLOOR, bmin,
        bmax, wall_top, bmin, bmin, wall_top, bmin,
        0.0f, 0.0f, 1.0f, pal.wall_color);
    // North wall (z = bmax, facing -Z into room)
    push_quad(verts, indices,
        bmax, WALL_FLOOR, bmax, bmin, WALL_FLOOR, bmax,
        bmin, wall_top, bmax, bmax, wall_top, bmax,
        0.0f, 0.0f, -1.0f, pal.wall_color);
    // West wall (x = bmin, facing +X into room)
    push_quad(verts, indices,
        bmin, WALL_FLOOR, bmax, bmin, WALL_FLOOR, bmin,
        bmin, wall_top, bmin, bmin, wall_top, bmax,
        1.0f, 0.0f, 0.0f, pal.wall_color);
    // East wall (x = bmax, facing -X into room)
    push_quad(verts, indices,
        bmax, WALL_FLOOR, bmin, bmax, WALL_FLOOR, bmax,
        bmax, wall_top, bmax, bmax, wall_top, bmin,
        -1.0f, 0.0f, 0.0f, pal.wall_color);

    // ─── Ceiling ─────────────────────────────────────────────
    if (m.shape.ceiling_type == CeilingType::FLAT) {
        push_quad(verts, indices,
            bmin, ch, bmin, bmax, ch, bmin,
            bmax, ch, bmax, bmin, ch, bmax,
            0.0f, -1.0f, 0.0f, pal.ceiling_color);
    }
    else if (m.shape.ceiling_type == CeilingType::VAULT) {
        // ─── Groin vault (cross vault) ───────────────────────

        float half_x = (bmax - bmin) * 0.5f;
        float half_z = (bmax - bmin) * 0.5f;
        float center_x = (bmin + bmax) * 0.5f;
        float center_z = (bmin + bmax) * 0.5f;
        float cat_a_x = solve_catenary_a(half_x, rise);
        float cat_a_z = solve_catenary_a(half_z, rise);

        static constexpr uint32_t VAULT_N = 32;

        auto catenary_y = [&](float dist, float cat_a) -> float {
            return crown_h - cat_a * (std::cosh(dist / cat_a) - 1.0f);
            };

        uint32_t v_base = static_cast<uint32_t>(verts.size());
        for (uint32_t iz = 0; iz <= VAULT_N; iz++) {
            float tz = (float)iz / (float)VAULT_N;
            float z = bmin + tz * (bmax - bmin);
            float dz = z - center_z;

            for (uint32_t ix = 0; ix <= VAULT_N; ix++) {
                float tx = (float)ix / (float)VAULT_N;
                float x = bmin + tx * (bmax - bmin);
                float dx = x - center_x;

                float y_barrel_x = catenary_y(dx, cat_a_x);
                float y_barrel_z = catenary_y(dz, cat_a_z);
                float y = std::min(y_barrel_x, y_barrel_z);

                bool on_edge = (ix == 0 || ix == VAULT_N || iz == 0 || iz == VAULT_N);
                if (on_edge) {
                    y = wall_h - JOINT_OVERLAP;
                }

                // Normal via finite differences
                float eps = (bmax - bmin) / (float)VAULT_N * 0.5f;
                float y_px = std::min(catenary_y(dx + eps, cat_a_x), catenary_y(dz, cat_a_z));
                float y_mx = std::min(catenary_y(dx - eps, cat_a_x), catenary_y(dz, cat_a_z));
                float y_pz = std::min(catenary_y(dx, cat_a_x), catenary_y(dz + eps, cat_a_z));
                float y_mz = std::min(catenary_y(dx, cat_a_x), catenary_y(dz - eps, cat_a_z));

                float ddx = (y_px - y_mx) / (2.0f * eps);
                float ddz = (y_pz - y_mz) / (2.0f * eps);
                float nmag = std::sqrt(ddx * ddx + 1.0f + ddz * ddz);
                float fnx = -ddx / nmag;
                float fny = -1.0f / nmag;
                float fnz = -ddz / nmag;

                if (on_edge) { fnx = 0.0f; fny = -1.0f; fnz = 0.0f; }

                verts.push_back({ { x, y, z }, { fnx, fny, fnz },
                    { pal.ceiling_color[0], pal.ceiling_color[1], pal.ceiling_color[2] } });
            }
        }

        for (uint32_t iz = 0; iz < VAULT_N; iz++) {
            for (uint32_t ix = 0; ix < VAULT_N; ix++) {
                uint32_t a = v_base + iz * (VAULT_N + 1) + ix;
                uint32_t b = a + 1;
                uint32_t c = a + (VAULT_N + 1);
                uint32_t d = c + 1;
                indices.push_back(a); indices.push_back(b); indices.push_back(c);
                indices.push_back(b); indices.push_back(d); indices.push_back(c);
            }
        }
    }

    uint32_t vc = std::min(static_cast<uint32_t>(verts.size()), Dim::SHELL_MAX_VERTICES);
    uint32_t ic = std::min(static_cast<uint32_t>(indices.size()), Dim::SHELL_MAX_INDICES);

    c->gpuState_.upload_shell_mesh(queue, verts.data(), vc, indices.data(), ic);

    std::cout << "[Shell] Generated "
        << (m.shape.ceiling_type == CeilingType::FLAT ? "FLAT" : "GROIN VAULT")
        << ": " << vc << " verts, " << ic << " indices"
        << " bounds=[" << bmin << "," << bmax << "]"
        << " wall_h=" << wall_h << " crown=" << crown_h
        << " rise=" << rise << "\n";
}

// ═══ PER-FRAME UPLOAD ════════════════════════════════════════════

// ── upload_lights ──
// (Must precede compute for shadow VP.)
inline void upload_lights(MoodDeps* c, wgpu::Queue& queue) {
    if (!c->mood_state_.lights_dirty) return;
    c->mood_state_.lights_dirty = false;

    // WALLET_1revA: the sun, the point array and the spot array are one
    // uniform block now (GPULighting / world.wgsl `Lighting`). They were
    // already composed in this one function and written back to back, so
    // the merge costs nothing here — three WriteBuffer calls become one.
    GPULighting lighting{};

    GPUDirectionalLight& sun = lighting.sun;
    float len = std::sqrt(c->sunDirection_[0] * c->sunDirection_[0] + c->sunDirection_[1] * c->sunDirection_[1] + c->sunDirection_[2] * c->sunDirection_[2]);
    sun.direction[0] = c->sunDirection_[0] / len;
    sun.direction[1] = c->sunDirection_[1] / len;
    sun.direction[2] = c->sunDirection_[2] / len;

    sun.color[0] = c->sunColor_[0];
    sun.color[1] = c->sunColor_[1];
    sun.color[2] = c->sunColor_[2];
    sun.intensity = c->mood_state_.sun_intensity;
    sun.ambient = c->mood_state_.sun_ambient;

    lighting.points.count = 0;
    lighting.spots = c->cpuSpotLights_;

    c->gpuState_.upload_lighting(queue, lighting);
}

// ═══ DERIVERS ════════════════════════════════════════════════════

inline const char* mood_name(uint32_t mood) {
    return (mood < MOOD_COUNT) ? MOOD_NAMES[mood] : "unknown";
}

// Derive finite world radius from seed within mood-defined bounds.
inline uint32_t derive_finite_radius(uint32_t seed, const MoodProfile& mood) {
    const WorldShape& s = mood.shape;
    if (s.finite_radius_min >= s.finite_radius_max) return s.finite_radius_min;
    uint32_t range = s.finite_radius_max - s.finite_radius_min + 1;
    return s.finite_radius_min + cpu_hash(seed, 77u) % range;
}

} // namespace the_board
} // namespace t7
