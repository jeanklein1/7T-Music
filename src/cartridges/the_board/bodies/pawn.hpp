#pragma once
#include <cstdint>
#include <cmath>      // std::exp (the presence ramp)
#include <algorithm>  // std::min (aura config assembly)
#include <iostream>   // command-door logs
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)

// ─── pawn.hpp (MERGED single file) ────────────────────────────────
// History: audit/LADDER.md
// hpp+inl collapsed: struct + deps + inline impl, one pre-class file.
// The wgpu handle fwds the signatures name ride contracts/wgpu_fwd.hpp.
//
// Player-relative state: aura field (toroidal 64×64 spring grid that
// activates near the pawn and releases when it moves away), presence
// trajectory, per-frame coupling tick.
//
// The aura ramp is a self-contained real-time exponential (std::exp, below).
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ TUNING CONSOLE ══════════════════════════════════════════════

// ── Aura presence ramp ───────────────────────────────────────────
inline constexpr float AURA_PRESENCE_ATTACK  = 1.0f;   // 1/s — ~3s to full (spring converges in ~0.5s)
inline constexpr float AURA_PRESENCE_RELEASE = 1.5f;   // 1/s — ~2s to zero

struct PawnAuraDeltaMode {
    static constexpr uint32_t CONVERGENT = 0;  // all cells shift toward signature tint
    static constexpr uint32_t RANDOM = 1;  // each cell gets unique random delta
};

struct PawnAuraProfile {
    float influence_radius;
    float attack_stiffness;
    float attack_damping;
    float release_rate;
    float tint_strength;
    float tint_r, tint_g, tint_b;
    uint32_t delta_mode;
    float delta_magnitude;     // random mode: max offset per channel
    uint32_t effect_mask;      // STATUS: INTENT — uploaded, never read.
                               // Bit 0=color / bit 1=height was the intent;
                               // no shader tests it (TUNE_1 A4 census). The
                               // live gates are tint_strength (color) and
                               // height_scale (height).
    float height_scale;        // height extrusion in world units
};

inline constexpr PawnAuraProfile PAWN_AURA_DEFAULT = {
    20.0f,             // influence_radius
    12.0f,             // attack_stiffness
    0.7f,              // attack_damping
    1.5f,              // release_rate
    0.0f,              // tint_strength — MUTED (TUNE_1 A4). The tint's
                       // only consumer is color_blend in the aura compute
                       // kernel (world.wgsl), so 0 silences the terrain
                       // tint outright. effect_mask bit 0 does NOT gate
                       // that write — the field is uploaded and never
                       // read — so the mask was not the dial. The height
                       // effect rides height_scale and is untouched.
    0.4f, 0.2f, 0.5f, // tint RGB (purple) — authored, kept
    PawnAuraDeltaMode::CONVERGENT,
    0.3f,              // delta_magnitude (used in random mode)
    0x3u,              // effect_mask: authored; unread (see the field at
                       // the struct, above — no shader tests this)
    3.0f,              // height_scale
};

// ═══ PAWN MODULE STATE ═══════════════════════════════════════════
//
// Field roles:
//   active_aura_profile — Currently active profile (PAWN_AURA_DEFAULT;
//     swappable by landmarks/commands).
//   aura_enabled — On/off intent (numpad 3, direction/input.hpp); the presence ramp
//     smooths the transition. Temporary binding; the function persists.
//   aura_height_enabled — Height-effect gate (key 2, direction/input.hpp);
//     flattens the extrusion. It USED to leave the color tint visible; since
//     TUNE_1 A4 muted tint_strength there is no tint left to leave, so the
//     key is now a height-only toggle.
//   aura_needs_clear — Internal: clear cells next frame after aura disable.
//   aura_cfg_dirty — Internal: full-config upload flag; true at boot; set by
//     external writers on a parameter shift.
//   (aura_presence — lives at player_.aura_presence, SEAM[spine:P8];
//    read as c->player_.aura_presence; written in tick_pawn_couplings.)

struct PawnState {
    PawnAuraProfile active_aura_profile = PAWN_AURA_DEFAULT;
    bool            aura_enabled        = false;
    bool            aura_height_enabled = true;
    bool            aura_needs_clear    = false;
    bool            aura_cfg_dirty      = true;
};

// ═══ MODULE DEPS ════════════════════════════════════════════════════
// The pawn's requirements face. player_ is NON-const — the P8 door
// (aura_presence is pawn-written, SEAM[spine:P8]); census W allows it
// here. (GPUState/Renderer fwd — state.hpp/renderer.hpp follow this
// header in the cohort; reference members tolerate incomplete types.)
struct PlayerState;
struct TimeState;
class GPUState;
class Renderer;
struct PawnDeps {
    PlayerState&     player_;       // non-const: P8 aura_presence write
    const TimeState& time_state_;
    GPUState&        gpuState_;
    Renderer&        renderer_;
};

// ─── IMPL:
// bodies deref only PawnDeps members; no Cartridge. The merged file
// sits after renderer.hpp in the cohort (Renderer/GPUState complete;
// Dim::PATCH_CELL_SIZE from patch_system.hpp). ───────────────────────────

// ─── Per-frame pawn coupling tick ────────────────────────────────
inline void tick_pawn_couplings(PawnState& ps, PawnDeps* c, wgpu::Queue& queue) {
    (void)queue;
    // Aura presence ramp: smooth 0→1 on enable / 1→0 on disable.
    // (aura_presence lives on player_ — SEAM[spine:P8]; see PlayerState, contracts/spine_state.hpp)
    {
        const float target = ps.aura_enabled ? 1.0f : 0.0f;
        const float prev   = c->player_.aura_presence;
        const float rate   = (target > prev) ? AURA_PRESENCE_ATTACK : AURA_PRESENCE_RELEASE;
        c->player_.aura_presence = prev + (target - prev) * (1.0f - std::exp(-rate * c->time_state_.dt));

        // Snap to endpoints to avoid perpetual drift
        if (c->player_.aura_presence < 0.001f && target == 0.0f) c->player_.aura_presence = 0.0f;
        if (c->player_.aura_presence > 0.999f && target == 1.0f) c->player_.aura_presence = 1.0f;
        if (c->player_.aura_presence != prev) ps.aura_cfg_dirty = true;
    }

    // Pawn aura height: presence × base height. Same value is consumed
    // by terrain VS for extrusion, so pawn and terrain always agree.
    const float effective_aura_height = ps.aura_height_enabled
        ? ps.active_aura_profile.height_scale * c->player_.aura_presence
        : 0.0f;
    c->gpuState_.set_pawn_aura_height(effective_aura_height);
    // Keep compute running while ramping down (so the trail decays cleanly).
    c->gpuState_.set_aura_enabled(c->player_.aura_presence > 0.001f);
}


// ─── Teardown (owner verb) ────────────────────────────────────────
// The pawn half of the world-teardown sweep: schedule a one-frame
// aura clear + full config re-upload for the new world.
inline void teardown_pawn_aura(PawnState& ps) {
    ps.aura_needs_clear = true;
    ps.aura_cfg_dirty = true;
}

// ─── Aura compute (owner verb) ──────────────────────────────────────
// Persistent terrain influence. Runs while presence > 0 (ramping down
// after toggle-off) or clearing; no-op otherwise. DEFERRED-UPLOAD FLAG
// aura_cfg_dirty (O-4): full config upload on change, dt/t_beats-only
// in steady state.
inline void dispatch_pawn_aura(PawnState& ps, PawnDeps* c,
                               wgpu::CommandEncoder& encoder, wgpu::Queue& queue) {
    if (!(c->player_.aura_presence > 0.0f || ps.aura_needs_clear)) return;
        if (ps.aura_cfg_dirty) {
            // Full config upload — profile changed or first frame
            ps.aura_cfg_dirty = false;
            const auto& ap = ps.active_aura_profile;

            // Presence scales all aura params for smooth raise/lower
            float p = c->player_.aura_presence;

            GPUPawnAuraConfig auraCfg{};
            auraCfg.cell_size = Dim::PATCH_CELL_SIZE;
            auraCfg.influence_radius = ap.influence_radius * p;
            auraCfg.attack_stiffness = ap.attack_stiffness;
            auraCfg.attack_damping = ap.attack_damping;
            auraCfg.release_rate = (p > 0.01f) ? ap.release_rate : 999.0f;
            auraCfg.dt = c->time_state_.dt;
            auraCfg.effect_mask = ap.effect_mask;
            auraCfg.aura_n = 64;
            auraCfg.tint_strength = std::min(ap.tint_strength * p, 1.0f);
            auraCfg.tint_r = ap.tint_r;
            auraCfg.tint_g = ap.tint_g;
            auraCfg.tint_b = ap.tint_b;
            auraCfg.delta_mode = ap.delta_mode;
            auraCfg.delta_magnitude = ap.delta_magnitude;
            auraCfg.t_beats = c->time_state_.beats;
            // height_scale gates the compute shader's R channel write (> 0.01 = enabled).
            // Actual terrain extrusion magnitude comes from config.pawn_aura_height in the VS.
            auraCfg.height_scale = (ps.aura_height_enabled && p > 0.01f) ? ap.height_scale : 0.0f;
            c->gpuState_.upload_pawn_aura_config(queue, auraCfg);
        }
        else {
            // Steady state — only dt and t_beats change per frame
            c->gpuState_.upload_pawn_aura_frame(queue, c->time_state_.dt, c->time_state_.beats);
        }

        wgpu::ComputePassDescriptor cpd{};
        cpd.label = "Pawn Aura";
        cpd.timestampWrites = c->gpuState_.meter_arm_compute(meter_row::PawnAura);
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
        // LOOM_2 pass head: WORLD + FRAME are every pipeline's strata 0/1.
        // FRAME carries the shadow-slot dynamic window; compute never moves it.
        { pass.SetBindGroup(0, c->gpuState_.world_group());
          pass.SetBindGroup(1, c->gpuState_.frame_c_group()); }
        c->renderer_.dispatch_compute_pawn_aura(pass,
            c->gpuState_.aura_state_group(), c->gpuState_.aura_textures_group(),
            GPUState::pawn_aura_workgroups());
        pass.End();

        // After one cleanup frame with release_rate=999, all cells are zero
        if (ps.aura_needs_clear) { ps.aura_needs_clear = false; }
}


// ─── Player commands (owner verbs — the input fan's
// pawn pair, matching the orbs/agents/cube command pattern) ────────
inline void toggle_aura_height(PawnState& ps, PawnDeps* c) {
    (void)c;
    ps.aura_height_enabled = !ps.aura_height_enabled;
    ps.aura_cfg_dirty = true;
    std::cout << "[Aura] Height extrusion: " << (ps.aura_height_enabled ? "ON" : "OFF") << "\n";
}

inline void toggle_aura(PawnState& ps, PawnDeps* c) {
    (void)c;
    ps.aura_enabled = !ps.aura_enabled;
    ps.aura_cfg_dirty = true;
    std::cout << "[Aura] Field: " << (ps.aura_enabled ? "ON" : "OFF") << "\n";
}

// ─── Mood policy door: respect player preference when
// permitted, force off when forbidden — the mood driver speaks through
// the pawn's own door instead of writing the organ. Semantics
// byte-identical to the direct write it replaces (disclosure rule).
inline void apply_aura_mood_policy(PawnState& ps, bool allow) {
    if (!allow) ps.aura_enabled = false;
}

} // namespace the_board
} // namespace t7
