#pragma once
#include <cstdint>
#include "cartridges/the_board/realization/state.hpp"                    // wgpu, GPUState
#include "cartridges/the_board/contracts/mood_constants.hpp"   // MOOD_COUNT, the Mood IDs, WORLD_DRAW_LIVE
#include "cartridges/the_board/contracts/agent_tiers.hpp"      // TIER_LIVE — the doorway witness reads a walker's contact_radius (ATRIUM_7)
#include "cartridges/the_board/contracts/spine_state.hpp"      // MoodState + the atmosphere vocabulary (CeilingType / MoodProfile / MOOD_TABLE)
#include "cartridges/the_board/contracts/atmosphere_surface.hpp"   // ATMOS_LIVE — the atmosphere panel; draw_atmosphere's bank (ONE_WORLD-II U1)
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)
#include <algorithm>   // std::max, std::min, std::clamp   // (impl, merged)
#include <cmath>       // std::sqrt, std::sin, std::cos, std::cosh, std::floor, std::abs   // (impl, merged)
#include <iostream>    // mood / lighting logs   // (impl, merged)
#include <iomanip>     // std::fixed, std::setprecision — the arc's facing witness (ATRIUM_5)   // (impl, merged)

// ─── mood.hpp (MERGED: deps + atmosphere vocabulary + impl) ───────
//
// Atmosphere: the world's sky, drawn from the bank.
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
// configure, render_passes' compute_spot_light_vp),
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

// ═══ THE DEPS FACE ═══════════════════════════════════════════════
//
// Mood's own organs plus its true reaches — the atmosphere author's
// face: the mood organ, the sun/clear channel, the realization pokes
// (GPUState uploads, the frustum-cull flag) and the gol mood gate (the
// THE FLAG CHANNEL [mood -> gol]). The CPU spot-light staging array and
// the const view of entities left with the indoor derivations at
// ONE_WORLD-II U4 — both were that deriver's alone. The fan's TARGET organs
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
    float (&sunDirection_)[3];
    float (&sunColor_)[3];
    float (&clearColor_)[3];
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════

// Mood lifecycle (doors). The fan's targets ride apply_mood's tail
// parameters — organ-named, addressed by the spine at the call site.
void apply_mood(MoodDeps* c, uint32_t mood, wgpu::Queue& queue,
    MachineCtx& machine_ctx,
    OrbsState& orbs_state, OrbsDeps& orbs_deps,
    PawnState& pawn_state);
// The applier. ONE_WORLD-II U4 took the other three: the spot-light
// deriver and the shell generator with the indoor organs, and
// apply_mood_arrival — declared with no body and no caller since before
// this campaign — with them.
void apply_mood_lighting(MoodDeps* c, const MoodProfile& m, wgpu::Queue& queue);
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
// face), and Dim::PATCH_EXTENT (patch_system.hpp).


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

inline AtmosphereInstance draw_atmosphere(uint32_t seed, const AtmosphereBank& a) {
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
    // ── the sky — the bank's, flat (ONE_WORLD-II U1) ──
    // There was an index here: the world's regime, rolled from the mood's
    // weights before this ran. Every mood weighted regime 0 alone, so the
    // roll had one destination and the array had one live slot; both left
    // with the moods. Named, never aliased: every enrolled leaf is reached
    // through `a`, so the reader census (tools/organ_readers.py) can see
    // that each dial has a reader.
    atmos_draw_color(seed, AtmosProp::SUN_COLOR, a.sun_color, a.sun_color_spread, out.sun_color);
    out.sun_intensity = std::max(0.0f, a.intensity
                      + atmos_jitter(seed, AtmosProp::INTENSITY, a.intensity_spread));
    out.sun_ambient   = std::max(0.0f, a.ambient
                      + atmos_jitter(seed, AtmosProp::AMBIENT, a.ambient_spread));
    // The fog's REST — the U4 seam composes the canvas's deviation over it.
    out.fog_density   = std::max(0.0f, a.fog_density
                      + atmos_jitter(seed, AtmosProp::FOG_DENSITY, a.fog_density_spread));
    atmos_draw_color(seed, AtmosProp::FOG_COLOR,   a.fog_color,   a.fog_color_spread,   out.fog_color);
    atmos_draw_color(seed, AtmosProp::CLEAR_COLOR, a.clear_color, a.clear_color_spread, out.clear_color);
    return out;
}

// ═══ APPLY MOOD ══════════════════════════════════════════════════

// 1) Atmospheric: THE DRAW, then the fan. (seed, definition) → instance,
//    re-run on every apply — a mood entry and a definition edit alike.
//    The seed is the world's, so a panel edit moves the instance WITH the
//    dial (same seed, shifted centre, same offset) rather than re-rolling
//    it. Touches GPU directly + a few member fields.
inline void apply_mood_lighting(MoodDeps* c, const MoodProfile& m, wgpu::Queue& /*queue*/) {
    const AtmosphereInstance ai = draw_atmosphere(c->world_state_.active_seed, ATMOS_LIVE);
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

    // THE THREE INDOOR STRUCTURALS LEFT HERE (ONE_WORLD-II U4). This
    // applier authored all three — the terrain amplitude ceiling, its
    // MoodState mirror, and the GoL lift cap composed from the module's
    // fraction of the wall — and each reached a GPUDesignConfig field
    // that is a named pad now. The three metered organ rows that watched
    // them died in the same commit, because a witness whose author is
    // gone meters a permanent zero.
    c->mood_state_.lights_dirty = true;

    // THE WITNESS. One line per (mood, seed), not per draw (ATMOS_1b): a
    // line prints when that PAIR changes — which is every entry, and
    // nothing else. A centre, colour or spread drag re-draws every frame
    // and says nothing here; the panel is its readout. The same seed
    // still prints the same line, and a boot must print
    // int=0.9 amb=0.2 fog=0.003 for the sunset until someone changes
    // ATMOS_TABLE on purpose.
    //
    // THE TRIPLE LOST ITS THIRD TERM (ONE_WORLD-II U1). It was
    // (mood, seed, regime), and `regime=` printed the LABEL's 1-based
    // number so the operator never saw the index. The roll is gone, so
    // the term that could change under a fixed (mood, seed) is gone with
    // it — and so is the one event it existed to announce. The boot log
    // is one field shorter: the rider's §B target moves with it (§C.1,
    // narration changes with its subject), and U8's transcript witness
    // reads the new line.
    // Function-local statics: the checker's [FLUSH] one-shot in
    // cartridge.hpp is the precedent.
    {
        static uint32_t last_mood = MOOD_COUNT, last_seed = 0u;
        const bool world_changed = c->mood_state_.active != last_mood
                                || c->world_state_.active_seed != last_seed;
        if (world_changed) {
            last_mood   = c->mood_state_.active;
            last_seed   = c->world_state_.active_seed;
            constexpr float RAD2DEG = 180.0f / 3.14159265359f;
            const float el = std::asin(std::clamp(-ai.sun_direction[1] / len, -1.0f, 1.0f)) * RAD2DEG;
            const float az = std::atan2(-ai.sun_direction[2], -ai.sun_direction[0]) * RAD2DEG;
            std::cout << "[Atmos] " << mood_name(c->mood_state_.active)
                      << " seed=" << c->world_state_.active_seed
                      << " int=" << ai.sun_intensity << " amb=" << ai.sun_ambient
                      << " sun el=" << el << " az=" << az
                      << " fog=" << ai.fog_density << "\n";
        }
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

    apply_mood_lighting(c, m, queue);          // sun + ambient — the whole of the light now
    if constexpr (ROSTER.orbs)                 // ROSTER-GATE orbs (b) — sky dome never configured
        // ORGAN_3b P3 — the world's definition, not the design table.
        // ORGAN_5 P1b — reseed TRUE: a mood change is a new world's sky,
        // so the init kernel re-runs and every orb is re-drawn. This is
        // the heavy path, and it is the one path that should be.
        configure_orbs(orbs_state, &orbs_deps, ORB_LIVE, queue,
            /*reseed=*/true);

    std::cout << "[Mood] Applied: " << mood_name(mood)
        << " (mood=" << mood << ")\n";
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
