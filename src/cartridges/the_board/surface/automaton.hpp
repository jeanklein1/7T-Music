#pragma once
// ═══ THE AUTOMATON — THE GROUND'S OWN GAME OF LIFE ═══════════════════
//
// It lives in surface/ and not in bodies/, and the move is the claim
// this whole unit makes: a body is something the world CONTAINS, and
// this is something the world IS. Its neighbours here are patch_system
// (which builds the ground) and terrain_looks (which decides how the
// ground reads); its old neighbours were agents, orbs and the ribbon.
//
// WHAT IT REPLACES. bodies/gol_zones.hpp ran eight islands: a lattice
// node rolled whether to spawn one, a weighted table picked its tier, a
// GPU kernel derived its parameters from that tier's Gaussians, the CPU
// generated its cells and uploaded five planes of them, and a slot
// registry tracked which of eight were alive. All of that machinery
// answered ONE question — "which zone is this?" — and the automaton has
// no answer to give: it is the whole cell grid.
//
// THE THREE ROOMS OF ONE FACT (Amendment D):
//   contracts/automaton_surface.hpp   the BANK   — AUTO_TABLE / AUTO_LIVE
//   surface/automaton.hpp (here)      the APPLIER — draw + upload + dispatch
//   realization/world.wgsl            the TRANSPORT — AutomatonConfig
// A commit that moves any of them moves all three.

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <iostream>

#include "cartridges/the_board/contracts/automaton_surface.hpp"
#include "cartridges/the_board/contracts/surface_services.hpp"
#include "cartridges/the_board/realization/state.hpp"
#include "cartridges/the_board/realization/renderer.hpp"

namespace t7 {
namespace the_board {

    // ═══ MODULE DEPS (S5) ════════════════════════════════════════════
    //
    // GolDeps carried a fourth member, `wgpu::Device&`, marked
    // SEAM[gol:derive-submit] — "immediate mid-render submit; never
    // folds into the frame encoder". It is not here, and the seam is not
    // here, because the CAUSE is not here: the seam existed so a zone
    // spawning mid-frame could derive its parameters before the agent
    // kernels read them. The automaton is seeded at BIRTH, on the birth
    // encoder, and nothing spawns after. A dependency that exists only to
    // serve a mid-frame submit dies with the mid-frame submit.
    struct AutomatonDeps {
        GPUState&        gpuState_;
        Renderer&        renderer_;
        const TimeState& time_state_;   // the header reads beats/dt
    };

    // ═══ RUNTIME STATE ═══════════════════════════════════════════════
    //
    // Eleven words where a slot registry stood. GoLState held
    // GoLZoneState[8] (each with node coords, host patch, a persisted
    // world footprint, an algorithm, a period, a density and a tick
    // cursor), a zone count, an active-slot high-water mark, a
    // zones_allowed gate and a derive-request queue. What survives is the
    // one thing that was never about zones: WHEN DOES CONWAY STEP.
    struct AutomatonState {
        GPUAutomatonConfig cfg{};        // the drawn world, CPU-side mirror
        bool     born = false;           // has the seed dispatch run for this world
        float    tick_period = 1.0f;     // beats — drawn, mirrored here for the gate
        int32_t  last_tick_index = -1;   // the tick cursor (was per zone)
    };

    // ═══ THE DRAW — ONCE PER WORLD ═══════════════════════════════════
    //
    // draw_atmosphere's shape exactly (ONE_WORLD-II U1): read the bank,
    // draw each centre±spread from the world seed at its own property
    // index, and hand back a finished instance. A spread of 0 returns the
    // centre by the IEEE identity — no hash, no round trip — which is
    // what lets a point row stay bit-identical to the value it replaced.
    //
    // THE PROPERTY BAND IS THE AUTOMATON'S OWN (AUTO_SEED_BAND, 260). The
    // zones drew from band 250 at a LATTICE NODE; these draw from the
    // world seed at the WORLD's own key. A band is a namespace, and
    // reusing 250 would have made two different questions share answers.
    inline constexpr uint32_t AUTO_SEED_BAND = 260u;
    struct AutoProp {
        static constexpr uint32_t DENSITY     = 940u;
        static constexpr uint32_t TICK_PERIOD = 941u;
        static constexpr uint32_t TRANSITION  = 942u;
        static constexpr uint32_t HEIGHT      = 943u;
        static constexpr uint32_t TARGET_R    = 944u;
        static constexpr uint32_t TARGET_G    = 945u;
        static constexpr uint32_t TARGET_B    = 946u;
        // 960/961 are the SEED KERNEL's, per cell, on the GPU side
        // (world.wgsl AUTO_PROP_*). Kept out of this list on purpose:
        // they are drawn per cell, not per world, and sharing an index
        // between a world draw and a cell draw is how two facts start
        // moving together by accident.
    };

    // A centre ± spread, drawn once. `spread <= 0` short-circuits to the
    // centre — the point-row identity ATMOS_TABLE's witness protects.
    inline float auto_draw(uint32_t seed, uint32_t prop, float centre, float spread) {
        if (spread <= 0.0f) return centre;
        return cpu_sample_gaussian(seed, prop, centre, spread);
    }

    inline void draw_automaton(AutomatonState& as, const AutomatonBank& bank,
                               uint32_t world_seed, uint32_t finite_radius) {
        const uint32_t seed = cpu_lattice_node_seed(world_seed, 0, 0, AUTO_SEED_BAND);

        // ── The world's grid IS the ground's cell grid ──
        const uint32_t grid = (2u * finite_radius + 1u) * Dim::PATCH_CELL_N;
        as.cfg.grid_size = grid;
        as.cfg.cell_origin_x = -(int32_t)(finite_radius * Dim::PATCH_CELL_N);
        as.cfg.cell_origin_z = -(int32_t)(finite_radius * Dim::PATCH_CELL_N);

        // ── Selections: dials, not rolls ──
        // GOL_PULSE_ALGORITHM_CHANCE (0.35) drew this per zone. The
        // handoff rules it out by name: CONWAY boots, PULSE is
        // dial-selectable. A world whose rule changes with its seed is a
        // world you cannot ask for.
        as.cfg.algorithm     = bank.algorithm;
        as.cfg.rule_mask     = bank.rule_mask;
        as.cfg.field_fn      = bank.field_fn;
        as.cfg.color_mode    = bank.color_mode;
        as.cfg.boundary_mode = bank.boundary_mode;

        // ── Draws: the parametric spirit, once per world ──
        as.cfg.density = std::clamp(
            auto_draw(seed, AutoProp::DENSITY, bank.density, bank.density_spread),
            0.0f, 1.0f);
        as.cfg.tick_period = std::max(0.1f,
            auto_draw(seed, AutoProp::TICK_PERIOD, bank.tick_period, bank.tick_period_spread));
        as.cfg.transition_fraction = std::clamp(
            auto_draw(seed, AutoProp::TRANSITION, bank.transition_fraction, bank.transition_fraction_spread),
            0.01f, 0.5f);
        as.cfg.alive_height = std::max(0.0f,
            auto_draw(seed, AutoProp::HEIGHT, bank.alive_height, bank.alive_height_spread));
        as.cfg.target_r = std::clamp(auto_draw(seed, AutoProp::TARGET_R, bank.target[0], bank.target_spread), 0.0f, 1.0f);
        as.cfg.target_g = std::clamp(auto_draw(seed, AutoProp::TARGET_G, bank.target[1], bank.target_spread), 0.0f, 1.0f);
        as.cfg.target_b = std::clamp(auto_draw(seed, AutoProp::TARGET_B, bank.target[2], bank.target_spread), 0.0f, 1.0f);

        // ── Per-cell scatter amounts: carried, not drawn ──
        as.cfg.spring_variance     = bank.spring_variance;
        as.cfg.phase_randomness    = bank.phase_randomness;
        as.cfg.tempo_randomness    = bank.tempo_randomness;
        as.cfg.height_factor_mean  = bank.height_factor_mean;
        as.cfg.height_factor_sigma = bank.height_factor_sigma;
        as.cfg.height_factor_lo    = bank.height_factor_lo;
        as.cfg.height_factor_hi    = bank.height_factor_hi;
        // bank.mode_threshold is NOT copied: it is a hardware mirror
        // (world.wgsl AUTO_MODE_THRESHOLD), not a transported field.

        // ── The header starts at rest; the frame writes it ──
        as.cfg.t_beats = 0.0f;
        as.cfg.dt = 0.0f;
        as.cfg.should_tick = 0u;

        as.tick_period = as.cfg.tick_period;
        as.last_tick_index = -1;
        as.born = false;
    }

    // ═══ THE BIRTH — ONE DISPATCH, ON THE BIRTH ENCODER ══════════════
    //
    // The config upload MUST precede the dispatch in queue order, and it
    // does by LATTICE_1's law: the queue orders every WriteBuffer ahead of
    // the whole command buffer, so a write issued before Finish() is
    // visible to every dispatch inside it.
    inline void birth_automaton(AutomatonState& as, AutomatonDeps* c,
                                wgpu::Queue& queue, wgpu::CommandEncoder& encoder) {
        c->gpuState_.upload_automaton_config(queue, as.cfg);

        wgpu::ComputePassDescriptor cpd{};
        cpd.label = "Automaton Seed";
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
        // LOOM_2 pass head: WORLD + FRAME are every pipeline's strata 0/1.
        { pass.SetBindGroup(0, c->gpuState_.world_group());
          pass.SetBindGroup(1, c->gpuState_.frame_c_group()); }
        c->renderer_.dispatch_automaton_seed(
            pass,
            c->gpuState_.automaton_state_group(), c->gpuState_.automaton_textures_group());
        pass.End();
        as.born = true;
    }

    // ═══ PER-FRAME: THE HEADER AND ITS TICK GATE ═════════════════════
    //
    // upload_gol_zone_config's whole body, minus the loop over eight
    // slots. The gate is unchanged in kind: floor(beats / period) crosses
    // an integer, so exactly one tick fires per period however many
    // frames it spans.
    inline void upload_automaton_header(AutomatonState& as, AutomatonDeps* c,
                                        wgpu::Queue& queue) {
        const float effective_period = std::max(as.tick_period, 0.01f);
        const int32_t current_tick = (int32_t)std::floor(c->time_state_.beats / effective_period);
        const bool should_tick = (current_tick != as.last_tick_index);
        if (should_tick) as.last_tick_index = current_tick;

        c->gpuState_.upload_automaton_header(
            queue, c->time_state_.beats, c->time_state_.dt, should_tick);
    }

    // ═══ THE FRAME'S TWO PASSES ══════════════════════════════════════
    //
    // Separate passes for the GPU barrier (O-6a): evolve reads the TARGET
    // plane that sync wrote. Callers order them sync -> evolve after the
    // header upload.
    //
    // NO COUNT GUARD. Both zone helpers opened with
    // `if (gs.zone_count == 0) return;` and the phase rows repeated it.
    // The ground is always there.
    inline void dispatch_automaton_sync(AutomatonDeps* c, wgpu::CommandEncoder& encoder) {
        wgpu::ComputePassDescriptor cpd{};
        cpd.label = "Automaton Sync";
        cpd.timestampWrites = c->gpuState_.meter_arm_compute(meter_row::AutomatonStep);
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
        { pass.SetBindGroup(0, c->gpuState_.world_group());
          pass.SetBindGroup(1, c->gpuState_.frame_c_group()); }
        c->renderer_.dispatch_automaton_sync(pass,
            c->gpuState_.automaton_state_group(), c->gpuState_.automaton_textures_group());
        pass.End();
    }

    inline void dispatch_automaton_evolve(AutomatonDeps* c, wgpu::CommandEncoder& encoder) {
        wgpu::ComputePassDescriptor cpd{};
        cpd.label = "Automaton Evolve";
        cpd.timestampWrites = c->gpuState_.meter_arm_compute(meter_row::AutomatonStep);
        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
        { pass.SetBindGroup(0, c->gpuState_.world_group());
          pass.SetBindGroup(1, c->gpuState_.frame_c_group()); }
        c->renderer_.dispatch_automaton_evolve(pass,
            c->gpuState_.automaton_state_group(), c->gpuState_.automaton_textures_group());
        pass.End();
    }

    // ═══ TEARDOWN (owner verb; P8 chain) ═════════════════════════════
    //
    // teardown_gol zeroed eight GoLZoneState structs, three counters and a
    // request queue, then uploaded an empty zone array. This zeroes one
    // struct and uploads it — which also blanks the config the shader
    // reads, so a torn-down world draws no automaton until the next birth
    // rewrites it. Latency is not exemption (P8's law): the verb runs on
    // the teardown path whether or not a caller exists yet.
    inline void teardown_automaton(AutomatonState& as, AutomatonDeps* c, wgpu::Queue& queue) {
        as = AutomatonState{};
        c->gpuState_.upload_automaton_config(queue, as.cfg);
    }

} // namespace the_board
} // namespace t7
