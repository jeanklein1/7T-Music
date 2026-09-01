#pragma once
#include <cstdint>
#include "cartridges/the_board/realization/state.hpp"                    // wgpu, GPUState
#include "cartridges/the_board/contracts/agent_tiers.hpp"      // TIER_LIVE — the doorway witness reads a walker's contact_radius (ATRIUM_7)
#include "cartridges/the_board/contracts/spine_state.hpp"      // SkyState — the world's drawn sky, the instance this file authors
#include "cartridges/the_board/contracts/atmosphere_surface.hpp"   // ATMOS_LIVE — the atmosphere panel; draw_atmosphere's bank (ONE_WORLD-II U1)
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)
#include <algorithm>   // std::max, std::min, std::clamp   // (impl, merged)
#include <cmath>       // std::sqrt, std::sin, std::cos, std::cosh, std::floor, std::abs   // (impl, merged)
#include <iostream>    // the [Atmos] and [World] lines   // (impl, merged)
#include <iomanip>     // std::fixed, std::setprecision — the arc's facing witness (ATRIUM_5)   // (impl, merged)

// ─── sky.hpp (MERGED: deps + the draw + the world's birth) ───────
//
// THE WORLD'S SKY: drawn from ATMOS_LIVE by the world's seed, applied
// once at every birth, and re-drawn at the boundary when a dial moves.
//
// IT WAS mood.hpp (ONE_WORLD-II U7). The file owned a VOCABULARY —
// CeilingType, MoodProfile, MOOD_TABLE, the indoor wall palettes — and
// four appliers that fanned a mood's definition out to the light, the
// spots, the shell and the sky. The vocabulary died at U2 and U4, three
// of the four appliers with it. What is left is one applier, one draw
// and the sequence that stages a world, and none of it is a mood's —
// which is why the survivor is `stage_sky` and not `apply_mood_lighting`
// (U7; a name that outlives its subject is the defect this campaign is
// for, and the rename is Jean's to veto).
namespace t7 {
namespace the_board {

struct WorldState;   // patch_system.hpp — the doors read seeds/bounds (reference members/params; fwd suffices)
// fwd — the deps face's true reaches and the fan's TARGET organs
// (reference members/params; complete types arrive with their owners
// in the cohort). GPUState + the CPU light array come complete
// from state.hpp (included above).
// `class Renderer;` stood here for SkyDeps::renderer_, which carried one
// write and left with it (ONE_SURFACE-I U0). The sky reaches realization
// through GPUState alone now.
struct EntitiesState; struct MachineCtx;
struct OrbsState;   struct OrbsDeps;
struct PawnState;

// ═══ SKY STATE — GRADUATED ═══════════════════════════════════════
// SkyState lives in contracts/spine_state.hpp: the early consumers read
// the contract; the instance stays at the root. The vocabulary that stood
// beside it there left at ONE_WORLD-II U2 and U4.

// ═══ THE DEPS FACE ═══════════════════════════════════════════════
//
// The sky's own organs plus its true reaches: the sky organ, the
// sun/clear channel, the realization pokes (GPUState uploads, the
// frustum-cull flag) and the gol gate (THE FLAG CHANNEL [sky -> gol]).
// The CPU spot-light staging array and the const view of entities left
// with the rooms' derivations at ONE_WORLD-II U4 — both were that
// deriver's alone. The fan's TARGET organs are deliberately NOT members
// (the B ruling, input's precedent): orbs/pawn pairs + the machine face
// ride stage_world_birth's parameters — the spine addresses the fan's
// bodies at the call site, through the owner command doors.
struct SkyDeps {
    SkyState&           sky_state_;
    const WorldState&    world_state_;
    GPUState&            gpuState_;
    float (&sunDirection_)[3];
    float (&sunColor_)[3];
    float (&clearColor_)[3];
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════

// The world's lifecycle (doors). The fan's targets ride the birth's tail
// parameters — organ-named, addressed by the spine at the call site.
void stage_world_birth(SkyDeps* c, wgpu::Queue& queue,
    MachineCtx& machine_ctx,
    OrbsState& orbs_state, OrbsDeps& orbs_deps,
    PawnState& pawn_state);
// The applier. ONE_WORLD-II U4 took the other three: the spot-light
// deriver and the shell generator with the rooms' organs, and
// apply_mood_arrival — declared with no body and no caller since before
// this campaign — with them.
void stage_sky(SkyDeps* c, wgpu::Queue& queue);
// Per-frame uploads (door)
void upload_lights(SkyDeps* c, wgpu::Queue& queue);
// Derivers (door)
uint32_t derive_finite_radius(uint32_t seed);


// ═══ MODULE IMPLEMENTATION ════════════════════════════════════════
//
// The doors + appliers + derivers. The bodies reach the deps face
// (c->sky_state_ / c->world_state_ / c->gpuState_ /
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
    // There was an index here: the world's regime, rolled from a mood's
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

// ═══ STAGE THE SKY ═══════════════════════════════════════════════

// THE DRAW, then the fan. (seed, bank) → instance, re-run on every apply
//    — a world's birth and a panel edit alike. The seed is the world's,
//    so a panel edit moves the instance WITH the dial (same seed, shifted
//    centre, same offset) rather than re-rolling it. Touches GPU directly
//    + a few member fields.
inline void stage_sky(SkyDeps* c, wgpu::Queue& /*queue*/) {
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
    c->sky_state_.sun_intensity = ai.sun_intensity;
    c->sky_state_.sun_ambient   = ai.sun_ambient;

    // The fog's REST — the sky's own since ATMOS_1. The U4 seam
    // (phase_motion_drivers) composes the canvas's deviation over it
    // every frame; this is the rung-3 instance that seam reads.
    c->sky_state_.fog_rest_density  = ai.fog_density;
    c->sky_state_.fog_rest_color[0] = ai.fog_color[0];
    c->sky_state_.fog_rest_color[1] = ai.fog_color[1];
    c->sky_state_.fog_rest_color[2] = ai.fog_color[2];

    c->clearColor_[0] = ai.clear_color[0];
    c->clearColor_[1] = ai.clear_color[1];
    c->clearColor_[2] = ai.clear_color[2];

    // THE THREE ROOM STRUCTURALS LEFT HERE (ONE_WORLD-II U4). This
    // applier authored all three — the terrain amplitude cap, its
    // SkyState mirror, and the GoL lift cap composed from the module's
    // fraction of the wall — and each reached a GPUDesignConfig field
    // that is a named pad now. The three metered organ rows that watched
    // them died in the same commit, because a witness whose author is
    // gone meters a permanent zero.
    c->sky_state_.lights_dirty = true;

    // THE WITNESS. One line per world, not per draw (ATMOS_1b): a line
    // prints when the SEED changes — which is every birth, and nothing
    // else. A centre, colour or spread drag re-draws every frame
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
        // ONE TERM LEFT (ONE_WORLD-II U2). It was (mood, seed, regime);
        // U1 took the regime with the roll and U2 takes the mood with the
        // moods. The SEED is what a world is now, so the line prints once
        // per world — which is what "every entry, and nothing else" always
        // meant.
        static uint32_t last_seed = 0xFFFFFFFFu;
        const bool world_changed = c->world_state_.active_seed != last_seed;
        if (world_changed) {
            last_seed   = c->world_state_.active_seed;
            constexpr float RAD2DEG = 180.0f / 3.14159265359f;
            const float el = std::asin(std::clamp(-ai.sun_direction[1] / len, -1.0f, 1.0f)) * RAD2DEG;
            const float az = std::atan2(-ai.sun_direction[2], -ai.sun_direction[0]) * RAD2DEG;
            std::cout << "[Atmos] seed=" << c->world_state_.active_seed
                      << " int=" << ai.sun_intensity << " amb=" << ai.sun_ambient
                      << " sun el=" << el << " az=" << az
                      << " fog=" << ai.fog_density << "\n";
        }
    }
}
// ── stage_world_birth (orchestrator) ──
//
// THE WORLD'S BIRTH (ONE_WORLD-II U2). It was apply_mood: it took a mood
// id, clamped it, stored it, read that mood's definition and fanned the
// definition's columns out to the frustum cull, the GoL gate, the aura
// policy, the light and the sky. There is one world now, so there is no
// id to take and no definition to look up — the banks ARE the definition
// and each applier reads its own. What is left is the SEQUENCE, which is
// what the verb was always for.
//
// The three feature gates were per-mood columns and are now the world's,
// stated once here rather than authored in a table nobody can turn.
inline void stage_world_birth(SkyDeps* c, wgpu::Queue& queue,
    MachineCtx& machine_ctx,
    OrbsState& orbs_state, OrbsDeps& orbs_deps,
    PawnState& pawn_state) {
    // `c->gol_state_.zones_allowed = true;` STOOD HERE — the [sky -> gol]
    // flag channel, the world's gate on new Game-of-Life zones. There are
    // no zones to allow: the automaton is born with the world, from the
    // world's own seed, and no gate can refuse it (ONE_SURFACE-II U2).
    apply_aura_policy(pawn_state, true);        // the pawn door; byte-identical semantics

    stage_sky(c, queue);                        // sun + ambient — the whole of the light now
    if constexpr (ROSTER.orbs)                  // ROSTER-GATE orbs (b) — sky dome never configured
        // ORGAN_5 P1b — reseed TRUE: a world's birth is a new sky, so the
        // init kernel re-runs and every orb is re-drawn. This is the heavy
        // path, and it is the one path that should be.
        configure_orbs(orbs_state, &orbs_deps, ORB_LIVE, queue,
            /*reseed=*/true);

    std::cout << "[World] Staged\n";
}

// ═══ PER-FRAME UPLOAD ════════════════════════════════════════════

// ── upload_lights ──
// (Must precede compute for shadow VP.)
inline void upload_lights(SkyDeps* c, wgpu::Queue& queue) {
    if (!c->sky_state_.lights_dirty) return;
    c->sky_state_.lights_dirty = false;

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
    sun.intensity = c->sky_state_.sun_intensity;
    sun.ambient = c->sky_state_.sun_ambient;

    c->gpuState_.upload_lighting(queue, lighting);
}

// ═══ DERIVERS ════════════════════════════════════════════════════


// Derive the finite world's radius from the seed, within the pin's dials.
// The world's radius, drawn from the dials. It read a WorldShape's own
// min/max until ONE_WORLD-II U2 rehomed the pair beside finite_mode; the
// SALT IS UNCHANGED at 77u, so a given seed draws the radius it always
// drew. The min >= max guard stays: a pinned range is still a legal
// range, and it is how a future dial pins one.
inline uint32_t derive_finite_radius(uint32_t seed) {
    if (FINITE_RADIUS_MIN >= FINITE_RADIUS_MAX) return FINITE_RADIUS_MIN;
    constexpr uint32_t range = FINITE_RADIUS_MAX - FINITE_RADIUS_MIN + 1;
    return FINITE_RADIUS_MIN + cpu_hash(seed, 77u) % range;
}

} // namespace the_board
} // namespace t7
