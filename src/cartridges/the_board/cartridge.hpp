#pragma once

// ─── cartridge.hpp ───────────────────────────────────────────────
// MOD campaign (ROSTER-1a/1b + LADDER-1..4): conversion history in audit/LADDER.md.
//
// THE_BOARD — Generative world engine.
//
// ONE REGIME (constitution §1, completion executed 2026-07-12,
// LADDER-6): every module is a file-scope pair around the class;
// the cartridge is the composition root alone.
//
// SEAM[spine:owns] FAMILY_DISPATCH is genuinely spine work — the
//   integration hub that ties the 12 families together. Each row's
//   body lives in the family's owning module. Per Ch. 15 of the seam
//   map. Adding a new family means: write select/place/commit/
//   evict/prepare_mesh in the owning module, add wrappers below,
//   add 1 row to FAMILY_DISPATCH.
// SEAM[spine:K2-related] the dispatch_prepare_mesh_*,
//   dispatch_mesh_gen_* wrappers (~400 lines below FamilyDispatch)
//   are integration glue, not module work. They live here correctly.
//   NOTE[seam-map] keep wrappers here; they're the integration layer
//   between FAMILY_DISPATCH and per-family modules.
// SEAM[spine:P5] readback state machines + world_state_.world_gen counter are
//   pattern P5 (release-pending sentinel) at the spine level. Pawn +
//   floater readbacks each protect against stale callbacks from
//   previous worlds via world_state_.world_gen capture in the closure. Genuinely
//   spine-owned, not a leak.
// SEAM[spine:P8] PlayerState's commented "Future (deferred)" fields
//   are explicit latent infrastructure: aura_presence is live here;
//   the other deferred fields await the unified entity layer.
//   Pattern P8 visible in source.
// SEAM[spine:portal-system] portal/transition state machine. Owns
//   transitionPhase_ (enum type in contracts/spine_state.hpp, REBUILD-0
//   m1), mood_state_.transition_timer, pendingDestination_, the
//   PORTAL_COLORS table, the back-portal pending state, and the
//   trigger-detection hooks called by readback. Mood.inl drives portal
//   spawning (force_spawn_portal_at, force_spawn_back_portal,
//   force_spawn_finite_portals); spine owns the request → activation
//   flow.
// SEAM[spine:family-dispatch] all evict_<family> (owner-side),
//   dispatch_prepare_mesh_<family>, dispatch_mesh_gen_<family>
//   wrapper functions land here — referenced by FAMILY_DISPATCH and
//   by spawn_engine.inl's commit/evict pipelines.
// ─────────────────────────────────────────────────────────────────

#include "render/render_cartridge.hpp"
#include "core/input_event.hpp"
#include "cartridges/the_board/contracts/roster.hpp"
#include "cartridges/the_board/demos/demo.hpp"             // THE SELECTED SENTENCE: DEMO + ROSTER (compile-time, INCUBATE_DEMO; default full)
#include "cartridges/the_board/primitives/seed_utils.hpp"           // hash/gaussian/tier-select helpers (pure-math leaf)
#include "cartridges/the_board/contracts/ground_architecture.hpp"  // ground contributor/policy tables + compile-time DAG checks
#include "cartridges/the_board/contracts/entity_types.hpp"         // THE CONTRACT HOME: pipeline contracts + boundary DTOs + queue unions + dispatch row/table decl
#include "cartridges/the_board/contracts/mood_constants.hpp"       // MOOD_COUNT + the Mood IDs + PortalDestination
#include "cartridges/the_board/contracts/spine_state.hpp"          // TimeState + PlayerState + TransitionPhase (spine organ TYPES; instances stay at the root — REBUILD-0 m1, stamp D3)
#include "cartridges/the_board/contracts/floater_vocabulary.hpp"   // floater TYPES (ActiveFloater/ActiveCube), file scope
#include "cartridges/the_board/bodies/pawn.hpp"                 // PawnState + configs + decls (impl is pawn.inl, post-class)
#include "cartridges/the_board/realization/state.hpp"
#include "cartridges/the_board/bodies/spheres.hpp"              // SphereState + evictor/funnel decls (impl is spheres.inl, post-class)
#include "cartridges/the_board/bodies/entities.hpp"             // grounded-family vocabulary + EntitiesState + preparer decls (impl is entities.inl, post-class)
#include "cartridges/the_board/bodies/orbs.hpp"                 // orb console/registries + OrbsState + ORB_MOOD_TABLE + decls (impl is orbs.inl, post-class)
#include "cartridges/the_board/bodies/gol_zones.hpp"            // GoL vocabulary + GoLState + decls (impl is gol_zones.inl, post-class)
#include "cartridges/the_board/bodies/agents.hpp"               // agent registries + console + AgentState + decls (impl is agents.inl, post-class)
#include "cartridges/the_board/bodies/cube_behaviors.hpp"       // cube behavior registry + CubeBehaviorsState + decls (impl is cube_behaviors.inl, post-class)
#include "cartridges/the_board/bodies/gallery.hpp"              // shot vocabulary + console + GalleryState + decls (impl is gallery.inl, post-class)
#include "cartridges/the_board/bodies/ribbon.hpp"               // ribbon console + color vocabulary + tiers + RibbonState + decls (impl is ribbon.inl, post-class; pairing suspension named in its banner)
#include "cartridges/the_board/direction/input.hpp"                // InputState/KeyState/MouseState + decls (impl is input.inl, post-class; carries its own GLFW include)
#include "cartridges/the_board/realization/render_passes.hpp"        // the nine pass/dispatch + light-VP decls (impl is render_passes.inl, post-class; module owns no state)
#include "cartridges/the_board/direction/mood.hpp"                 // MoodProfile + MOOD_TABLE + portal colors + palettes + door/applier/deriver decls (impl is mood.inl, post-class; mood owns no state)
#include "cartridges/the_board/surface/population_themes.hpp"  // S2: THEMES + ThemeEnvelope + ThemesState + decls (impl is population_themes.inl, post-class)
#include "cartridges/the_board/surface/tile_world.hpp"          // S2: archetypes + tokens + TileState/cache + TileWorldState + decls (impl is tile_world.inl, post-class)
#include "cartridges/the_board/surface/patch_system.hpp"     // S2: WorldState + ActivePatch + budgets + visibility + PatchSystemState + decls (impl is patch_system.inl, post-class)
#include "cartridges/the_board/machine/spawn_engine.hpp"     // S3: spawn vocabulary + separation/proximity tables + SpawnEngineState + the preamble template + decls (impl is spawn_engine.inl, post-class)
#include "cartridges/the_board/machine/entity_pipeline.hpp"   // S3: the rescale template + arch vocabulary (ArchIdx/ARCH_TIERS) + the three-phase verb decls (impl is entity_pipeline.inl, post-class)
#include "cartridges/the_board/realization/renderer.hpp"
#include "coupling/visual_canvas.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <filesystem>
#include <algorithm>
#include <string>
#include <vector>
#include "external/stb_image.h"

namespace t7 {
    namespace the_board {

        class Cartridge : public RenderCartridge {

        // COMPOSITION ROOT — ORGANS ARE PUBLIC: sight is free; writes pass
        // through declared seams; the census enforces the seam law, not
        // access control.
        public:

            wgpu::Device device_;
            wgpu::TextureFormat colorFormat_;
            wgpu::TextureFormat depthFormat_;

            GPUState gpuState_;
            Renderer renderer_;

            // ═══ COMPOSITION ROOT — MODULE STATE ════════════════════════
            //
            SphereState sphere_state_;

            //   cube_behaviors_state_ — CubeBehaviorsState (cube_behaviors.hpp),
            //     the cube diagnostics + the cube active-slot mirror.
            CubeBehaviorsState cube_behaviors_state_;

            //   pawn_state_ — PawnState (pawn.hpp), the pawn aura + presence state.
            PawnState pawn_state_;

            EntitiesState entities_state_;

            //   orbs_state_ — OrbsState (orbs.hpp), the sky-dome lifecycle +
            //     player-owned anchor/rule/gesture state.
            OrbsState orbs_state_;

            //   gol_state_ — GoLState (gol_zones.hpp), the zone slots + counts +
            //     mood gate + derive-request queue.
            GoLState gol_state_;

            //   agent_state_ — AgentState (agents.hpp), the 32-slot CPU mirror +
            //     respawn counters + diagnostic overrides.
            AgentState agent_state_;

            GalleryState gallery_state_;

            RibbonState ribbon_state_;

            //   themes_state_ — ThemesState (population_themes.hpp), the theme
            //     envelope machine + the per-patch selection.
            ThemesState themes_state_;

            //   tile_world_state_ — TileWorldState (tile_world.hpp), the tile
            //     cache + the terrain tokens (what the terrain remembers).
            TileWorldState tile_world_state_;

            //   patch_system_state_ — PatchSystemState (patch_system.hpp), the
            //     active-patch registry + the free-layer pool.
            PatchSystemState patch_system_state_;

            //   world_state_ — WorldState (patch_system.hpp), the world seed +
            //     radii + patch counts + dirty flags. ROOT ORGAN (Phase R
            //     stamp, R-a): the struct lives with patch_system; the
            //     instance stays here at the root.
            WorldState world_state_;

            //   spawn_engine_state_ — SpawnEngineState (spawn_engine.hpp), the
            //     two dispatch queues + the footprint registry + the census clock.
            SpawnEngineState spawn_engine_state_;

            InputState inputState_;
            KeyState keys_;
            MouseState mouse_;

            // ═══ TIME STATE ═════════════════════════════════════════════
            // Struct TimeState graduated to contracts/spine_state.hpp
            // (REBUILD-0 m1, stamp D3); the instance stays here.
            TimeState time_state_;

            VisualCanvas  visual_canvas_;
            TargetBinding fog_density_dst_{};   // resolved "fog.density" pipe
            // Ribbon amp pipes (pitch compass) — resolved once at bind.
            TargetBinding ribbon_amp_lat_dst_{};
            TargetBinding ribbon_amp_vert_dst_{};
            TargetBinding ribbon_tint_stim_dst_{};
            TargetBinding ribbon_tint_mix_dst_{};
            TargetBinding fog_color_dst_{};      // resolved "fog.color" pipe (3 wide)

            // Sun + atmosphere (driven by active mood — see apply_mood)
            float sunDirection_[3] = { 0.69f, -0.71f, -0.14f };
            float sunColor_[3] = { 1.0f, 0.95f, 0.9f };
            float clearColor_[3] = { 0.85f, 0.78f, 0.72f };

            // ═══ MOOD STATE ═════════════════════════════════════════════
            //
            // Struct MoodState lives with its semantic owner
            // (direction/mood.hpp — the WorldState pattern, R-a; REBUILD-0
            // m1, stamp D3). The INSTANCE stays spine-resident because
            // mood-applied values feed every other subsystem
            // (SEAM[spine:transitions], K4).
            MoodState mood_state_;

            // ═══ PLAYER STATE ════════════════════════════════════════════
            //
            // Struct PlayerState graduated to contracts/spine_state.hpp
            // (REBUILD-0 m1, stamp D3) — SEAM[spine:P8] rides with the
            // struct; the instance stays here.
            PlayerState player_{};

            GPUSpotLightArray cpuSpotLights_{};  // count=0 disables (outdoor)

            // ═══ PORTAL & TRANSITION STATE MACHINE ═══════════════════════
            //
            // SEAM[spine:transitions] (K4, Jean, 2026-07-11): the transition
            //   machine and its working members — transitionPhase_,
            //   pendingDestination_, backPortalPosition_, cpuPortalArray_,
            //   mood_state_ and kin — are DECLARED SPINE-OWNED
            //   ORCHESTRATION per the §2 residency law, the same legitimacy
            //   class as the P5 readbacks. Mood (mood.hpp/.inl) supplies
            //   vocabulary + appliers + six doors and owns NO instance;
            //   struct MoodState's TYPE lives with its semantic owner
            //   (direction/mood.hpp — the WorldState pattern, R-a) per the
            //   REBUILD-0 stamp (D3). Constitution §2 carries the K4 line.
            // SEAM[spine:portal-system] consumed by the mood module
            //   (force_spawn_* functions read pendingDestination_), input.inl
            //   (keypress mood transitions request via mood.hpp's
            //   request_mood_transition), render() (readback callback drives
            //   portal trigger detection). PORTAL_COLORS lives in mood.hpp —
            //   portal color is mood vocabulary; the machine keeps the
            //   pending state and the trigger hooks.

            // enum TransitionPhase graduated to contracts/spine_state.hpp
            // (REBUILD-0 m1, stamp D3); the machine member stays here.
            TransitionPhase transitionPhase_ = TransitionPhase::IDLE;

            PortalDestination pendingDestination_{};

            GPUPortalArray cpuPortalArray_{};

            // ── Back-portal (guaranteed exit from finite worlds) ──
            // Position is configurable so special-case layouts can relocate it.
            float backPortalPosition_[2] = { 10.0f, 0.0f };   // world XZ

            // ═══ GPU READBACK + WORLDGEN ═════════════════════════════════
            //
            // SEAM[spine:P5] readback state machines + world_state_.world_gen counter are
            //   pattern P5 (release-pending sentinel) at the spine level.
            //   Pawn + floater readbacks each protect against stale callbacks
            //   from previous worlds via world_state_.world_gen capture in the closure.
            //   Genuinely spine-owned, not a leak.

            enum class PawnReadbackState { IDLE, COPIED, MAPPING };
            PawnReadbackState pawnReadbackState_ = PawnReadbackState::IDLE;
            enum class FloaterReadbackState { IDLE, COPIED, MAPPING };
            FloaterReadbackState floaterReadbackState_ = FloaterReadbackState::IDLE;

            // ROSTER-RESIDUE gol (2e) instrumentation: count of frames the GoL
            // zone-compute block ran (the sole writer of the zone GPU buffers),
            // and the residue-report cadence timer. Read only by the residue
            // check when ROSTER.gol is disabled (proves the buffers pristine).
            uint64_t rosterGolZoneRuns_ = 0;
            float    rosterGolResidueDump_ = 0.0f;

            // ── Terrain CPU mirror deleted ────────────────────────────────


            // ═══ FAMILY DISPATCH TABLE ═══════════════════════════════════
            //
            // SEAM[spine:owns] FAMILY_DISPATCH is the integration hub that
            //   ties the 12 families together. Each row's body lives in
            //   the family's owning module.
            // SEAM[spine:K2-related] the six real dispatch_prepare_mesh_* /
            //   dispatch_mesh_gen_* adapter pairs below are integration glue
            //   between FAMILY_DISPATCH and the per-family modules (their
            //   signatures adapt module preparers and renderer dispatches to
            //   the row slots). The bespoke select/place/commit funnels AND
            //   the twelve evictors live with their owners (§5 EVICTION
            //   THUNKS: retirement fulfilled); the no-op mesh adapters are
            //   shared (family_dispatch.inl).
            // SEAM[spine:family-dispatch] anchor for cross-file references —
            //   eviction routes through FAMILY_DISPATCH[f].evict_slot to the
            //   owner-side evict_<family> functions.
            //
            // The row type (struct FamilyDispatch) and the queue-entry
            // unions it walks (EntityQueueEntry / PlacementEntry) live in
            // entity_types.hpp — the contract home.

            // ═══ DISPATCH WRAPPERS ═══════════════════════════════════════

            // ── Mesh gen wrappers ──

            static bool dispatch_prepare_mesh_pyramid(Cartridge* self, wgpu::Queue& queue) {
                return prepare_pyramid_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_pyramid(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_pyramid_mesh_gen(pass, self->gpuState_.pyramid_mesh_gen_group());
            }

            static bool dispatch_prepare_mesh_arch(Cartridge* self, wgpu::Queue& queue) {
                return prepare_arch_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_arch(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_arch_mesh_gen(pass, self->gpuState_.arch_mesh_gen_group());
            }

            static bool dispatch_prepare_mesh_column(Cartridge* self, wgpu::Queue& queue) {
                return prepare_column_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_column(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_column_mesh_gen(pass, self->gpuState_.column_mesh_gen_group());
            }

            // ── Mesh gen dispatch wrappers (palm) ──

            static bool dispatch_prepare_mesh_palm(Cartridge* self, wgpu::Queue& queue) {
                return prepare_palm_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_palm(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_palm_mesh_gen(pass, self->gpuState_.palm_mesh_gen_group());
            }

            // ── Mesh gen dispatch wrappers (cactus) ──

            static bool dispatch_prepare_mesh_cactus(Cartridge* self, wgpu::Queue& queue) {
                return prepare_cactus_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_cactus(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_cactus_mesh_gen(pass, self->gpuState_.cactus_mesh_gen_group());
            }

            static bool dispatch_prepare_mesh_blade(Cartridge* self, wgpu::Queue& queue) {
                return prepare_blade_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_blade(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_blade_mesh_gen(pass, self->gpuState_.blade_mesh_gen_group());
            }

            // ── The dispatch table (FAMILY_DISPATCH) is defined at file


            //
            // The spine-owned piece-enable manifest (struct Roster, the
            // ROSTER constant, the transitions=>portal edge, and the full doc
            // block — RIDER A / MATURITY DIAL / FOUNDATIONAL / LATENT /
            // gate-(a) status column) now lives in
            // cartridges/the_board/roster.hpp. It met its SECOND CONSUMER —
            // GPUState::init (state.hpp) gates creation on the feature bits —
            // so the reading publishes at the shared header (the standing
            // law). ROSTER is visible here by namespace lookup
            // (t7::the_board::ROSTER); every ROSTER-GATE / ROSTER-RESIDUE
            // consult below is unchanged.

        public:

            // ═══ PUBLIC: CARTRIDGE LIFECYCLE ═════════════════════════════

            Cartridge() = default;

            Cartridge(const Cartridge&) = delete;
            Cartridge& operator=(const Cartridge&) = delete;

            void initialize(wgpu::Device device) override {
                device_ = device;
                auto tGpu0 = std::chrono::high_resolution_clock::now();
                gpuState_.init(device);
                auto tGpu1 = std::chrono::high_resolution_clock::now();
                std::cout << "[Cartridge] GPUState init:    "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(tGpu1 - tGpu0).count()
                    << " ms\n";

                {
                    float inactive[6] = { -1.f, -1.f, -1.f, -1.f, -1.f, -1.f };
                    float zeros6[6] = {};
                    gpuState_.set_band_motion(inactive, zeros6);
                    gpuState_.set_terrain_time(0.0f);
                    gpuState_.set_mode_color_shift(0.0f);
                    gpuState_.set_mode_checker_scatter(0.0f);
                    gpuState_.set_mode_palette_drift(0.0f, 0.0f, 0.0f);
                    gpuState_.set_mode_gol_scales(1.0f, 1.0f);
                    float zero_pulses[32] = {};
                    gpuState_.set_pulse_data(0, zero_pulses);
                }
            }

            bool init_renderer(
                wgpu::TextureFormat colorFormat,
                wgpu::TextureFormat depthFormat
            ) {
                colorFormat_ = colorFormat;
                depthFormat_ = depthFormat;

                auto t0 = std::chrono::high_resolution_clock::now();
                if (!renderer_.init(
                    device_,
                    gpuState_,
                    colorFormat,
                    depthFormat
                )) return false;

                // Create offscreen textures with the actual swapchain format
                if (!gpuState_.initOffscreenResources(colorFormat)) {
                    std::cerr << "[Cartridge] Failed to init offscreen resources\n";
                    return false;
                }

                auto t1 = std::chrono::high_resolution_clock::now();

                // --- One-shot: generate terrain index buffer on GPU -----------------
                {
                    wgpu::CommandEncoder encoder = device_.CreateCommandEncoder();
                    wgpu::ComputePassDescriptor desc{};
                    desc.label = "Terrain Index Gen (one-shot)";
                    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&desc);
                    renderer_.dispatch_generate_terrain_indices(
                        pass,
                        gpuState_.terrain_index_gen_group(),
                        GPUState::terrain_mesh_workgroups()
                    );
                    pass.End();
                    wgpu::CommandBuffer cmd = encoder.Finish();
                    device_.GetQueue().Submit(1, &cmd);
                }
                auto t2 = std::chrono::high_resolution_clock::now();

                init_patch_system(this);
                setup_test_rig_piers(this, device_.GetQueue());

                // Sky orbs for the initial mood (apply_mood runs only on transitions).
                if constexpr (ROSTER.orbs) {  // ROSTER-GATE orbs (c) — boot one-shot skipped when disabled
                    wgpu::Queue q = device_.GetQueue();
                    configure_orbs(orbs_state_, this, ORB_MOOD_TABLE[mood_state_.active], q);
                }

                // Agent registries — single source of truth in bodies/agents.inl
                // (AGENT_BEHAVIORS / AGENT_TIER_GAINS), uploaded once to GPU
                // storage buffers at bindings 110 + 111. Values are
                // constexpr-equivalent and never change during a session,
                // so this is a one-shot write at boot.
                {
                    wgpu::Queue q = device_.GetQueue();
                    upload_agent_registries_to_gpu(this, q);
                }

                {
                    agent_state_.slots[0].pos_x = Idle::PAWN_POS_X;
                    agent_state_.slots[0].pos_y = Idle::PAWN_POS_Y;
                    agent_state_.slots[0].pos_z = Idle::PAWN_POS_Z;
                    agent_state_.slots[0].heading = Idle::PAWN_HEADING;
                    agent_state_.slots[0].orient_w = 1.0f;
                    agent_state_.slots[0].is_active = 1u;
                    agent_state_.slots[0].behavior_id = AGENT_BEHAVIOR_PLAYER_CONTROLLED;
                    agent_state_.slots[0].tier_idx = AGENT_TIER_WORKER;
                    agent_state_.slots[0].portal_trigger = -1;

                    wgpu::Queue q = device_.GetQueue();
                    // ROSTER-GATE wanderers (c) — boot population (agent slots
                    // 1+). Slot 0 (the pawn, seeded just above) is untouched.
                    if constexpr (ROSTER.wanderers)
                        spawn_population_for_mood(agent_state_, this, mood_state_.active, world_state_.active_seed,
                            Idle::PAWN_POS_X, Idle::PAWN_POS_Z, q);
                    dump_agent_census(agent_state_, this, "boot");
                }

                // Eager-load authored paintings at boot (avoids mid-frame stall on first gallery)
                {
                    wgpu::Queue q = device_.GetQueue();
                    load_authored_textures(gallery_state_, this, q);
                }

                auto t3 = std::chrono::high_resolution_clock::now();

                std::cout << "[Cartridge] Renderer init:    "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << " ms\n";
                std::cout << "[Cartridge] Terrain gen:      "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms\n";
                std::cout << "[Cartridge] Patch system:     "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count() << " ms\n";
                std::cout << "[Cartridge] Total init:       "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t0).count() << " ms\n";

                if constexpr (!ROSTER.all_enabled()) {
                    std::string off;
                    auto mark = [&](bool enabled, const char* name) {
                        if (!enabled) { if (!off.empty()) off += ", "; off += name; }
                    };
                    mark(ROSTER.pyramid, "pyramid");     mark(ROSTER.arch, "arch");
                    mark(ROSTER.column, "column");       mark(ROSTER.antenna, "antenna");
                    mark(ROSTER.palm, "palm");           mark(ROSTER.cactus, "cactus");
                    mark(ROSTER.blade, "blade");         mark(ROSTER.sphere, "sphere");
                    mark(ROSTER.ribbon, "ribbon");       mark(ROSTER.cube, "cube");
                    mark(ROSTER.gol, "gol");             mark(ROSTER.gallery, "gallery");
                    mark(ROSTER.pawn_aura, "pawn_aura"); mark(ROSTER.orbs, "orbs");
                    mark(ROSTER.spot_lights, "spot_lights");
                    mark(ROSTER.indoor_shell, "indoor_shell");
                    mark(ROSTER.portal, "portal");       mark(ROSTER.transitions, "transitions");
                    mark(ROSTER.wanderers, "wanderers");
                    // Buffer creation: only indoor_shell (SEP) skips in v0;
                    // pipelines gate per piece (gate a', DEMO-1c).
                    const char* skipped = ROSTER.indoor_shell
                        ? "(none — every disabled piece is SH-shared, created-pristine)"
                        : "indoor_shell (shell VB/IB)";
                    std::cout << "[ROSTER] pieces disabled: " << off
                        << " | buffer creations skipped: " << skipped
                        << " | pipelines skipped: " << Renderer::pipelines_skipped() << "\n";
                }

                return true;
            }

            void bind_signal_layout(StatLayoutView v) {
                visual_canvas_.bind(v);
                fog_density_dst_ = visual_canvas_.layout().resolve("fog.density");
                fog_color_dst_   = visual_canvas_.layout().resolve("fog.color");
                ribbon_amp_lat_dst_  = visual_canvas_.layout().resolve("ribbon.amp_lateral_mult");
                ribbon_amp_vert_dst_ = visual_canvas_.layout().resolve("ribbon.amp_vertical_mult");
                ribbon_tint_stim_dst_ = visual_canvas_.layout().resolve("ribbon.color_stim");
                ribbon_tint_mix_dst_  = visual_canvas_.layout().resolve("ribbon.color_mix");
                std::fprintf(stderr,
                    "[the_board] fog.density base=%d valid=%d | fog.color base=%d count=%d valid=%d\n",
                    fog_density_dst_.base, (int)fog_density_dst_.valid,
                    fog_color_dst_.base, fog_color_dst_.count, (int)fog_color_dst_.valid);
            }

            void update(const AnalysisSignal& signal,
                float aspect_ratio,
                wgpu::Queue& queue) override {
                // --- Build GPU signal from analysis + input -------------------------
                GPUFrameSignal gpuSignal;

                gpuSignal.t_seconds = signal.t_seconds;
                gpuSignal.t_beats = signal.t_beats;
                gpuSignal.dt = signal.dt;
                gpuSignal.aspect_ratio = aspect_ratio;

                for (size_t i = 0; i < gpuSignal.stats.size(); ++i) {
                    gpuSignal.stats[i] = signal.stats[i];
                }

                gpuSignal.move_x = inputState_.move_x;
                gpuSignal.move_z = inputState_.move_z;
                gpuSignal.look_az_delta = inputState_.look_az_delta;
                gpuSignal.look_el_delta = inputState_.look_el_delta;
                gpuSignal.zoom_delta = inputState_.zoom_delta;
                gpuSignal.pan_x_delta = inputState_.pan_x_delta;
                gpuSignal.pan_y_delta = inputState_.pan_y_delta;
                gpuSignal.dt_beats = signal.t_beats - time_state_.prev_beats;  // beats since last frame -> step_trigger

                // Sky mode (SNAP-1 / M5 same-frame coherence): the pose/frame
                // words here are NEUTRAL placeholders. The authoritative
                // author is resync_sky_head, ordered AFTER ribbon_frame_tick
                // and BEFORE dispatch_compute — queue writes apply in
                // submission order, so the kernel always reads THIS frame's
                // advance, the same advance the ring transforms render.
                // Filling from ribbon_head_pose here would carry the PREVIOUS
                // frame's pose (the M5 skew); zeros instead make any future
                // loss of the resync fail LOUD (pawn to origin) rather than
                // silently one frame late. SEAM[ribbon:sky-mode].
                {
                    gpuSignal.sky_mode    = player_.sky_mode ? 1u : 0u;
                    gpuSignal.sky_head_x  = 0.0f;
                    gpuSignal.sky_head_y  = 0.0f;
                    gpuSignal.sky_head_z  = 0.0f;
                    gpuSignal.sky_heading = 0.0f;
                    gpuSignal.sky_yaw_off = 0.0f;
                    gpuSignal.sky_pitch   = 0.0f;
                    gpuSignal.sky_roll    = 0.0f;
                }

                time_state_.beats = signal.t_beats;
                time_state_.seconds = signal.t_seconds;
                time_state_.dt = signal.dt;
                {
                    const float db = signal.t_beats - time_state_.prev_beats;
                    if (db > 1e-6f && time_state_.dt > 1e-6f)
                        time_state_.beat_rate = db / time_state_.dt;
                    time_state_.prev_beats = signal.t_beats;
                }

                visual_canvas_.tick(signal);
                if (fog_density_dst_.valid && fog_color_dst_.valid) {
                    const VisualParams& fp = visual_canvas_.params();
                    gpuState_.set_fog(fp.get(fog_density_dst_.base),
                                      fp.get(fog_color_dst_.base + 0),
                                      fp.get(fog_color_dst_.base + 1),
                                      fp.get(fog_color_dst_.base + 2));
                }

                // Pawn presence ramp + aura height computation.
                // Lives in pawn.inl as a real-time exponential tick (closes pawn:K1).
                if constexpr (ROSTER.pawn_aura)  // ROSTER-GATE pawn_aura (b) — disabled: presence never raised, no aura height
                    tick_pawn_couplings(pawn_state_, this, queue);
                gpuState_.set_world_seed(world_state_.active_seed);
                if (world_state_.finite_mode) {
                    float bmin = -(float)world_state_.finite_radius * PATCH_EXTENT;
                    float bmax = ((float)world_state_.finite_radius + 1.0f) * PATCH_EXTENT;
                    gpuState_.set_world_bounds(bmin, bmin, bmax, bmax);
                }
                else {
                    gpuState_.set_world_bounds(0.0f, 0.0f, 0.0f, 0.0f);
                }

                // --- Transition state machine ---
                if (transitionPhase_ != TransitionPhase::IDLE) {
                    mood_state_.transition_timer += signal.dt;
                    switch (transitionPhase_) {
                    case TransitionPhase::FADE_OUT:
                        mood_state_.transition_fade_alpha = std::min(1.0f, mood_state_.transition_timer / mood_state_.transition_fade_duration);
                        if (mood_state_.transition_fade_alpha >= 1.0f) {
                            transitionPhase_ = TransitionPhase::TEARDOWN;
                        }
                        break;
                    case TransitionPhase::TEARDOWN:
                    {
                        // This TEARDOWN block stays focused on the
                        //   integration concerns it correctly owns: worldGen
                        //   bump (P5 stale-callback guard), return-state
                        //   capture, agent reset, ribbon cleanup, patch
                        //   teardown.
                        // SEAM[spine:P5] world_state_.world_gen++ at top of TEARDOWN is the
                        //   stale-callback guard (P5 family). Genuinely
                        //   spine-owned.

                        world_state_.world_gen++;

                        // Capture return seed + mood + radius before overwrite
                        mood_state_.back_portal_return_seed = world_state_.active_seed;
                        mood_state_.back_portal_return_mood = mood_state_.active;
                        mood_state_.back_portal_return_radius = world_state_.finite_radius;

                        world_state_.active_seed = pendingDestination_.seed;
                        world_state_.finite_mode = pendingDestination_.finite;
                        world_state_.finite_radius = pendingDestination_.finite_radius;
                        teardown_world(this, queue);
                        player_.readback_portal_trigger = -1;
                        player_.readback_x = 0.0f;
                        player_.readback_z = 0.0f;
                        uint32_t preserved_tier = agent_state_.slots[player_.possessed_slot].tier_idx;
                        float preserved_color_r = agent_state_.slots[player_.possessed_slot].color_r;
                        float preserved_color_g = agent_state_.slots[player_.possessed_slot].color_g;
                        float preserved_color_b = agent_state_.slots[player_.possessed_slot].color_b;

                        gpuState_.reset_player_agent(queue, preserved_tier,
                            preserved_color_r, preserved_color_g, preserved_color_b);
                        gpuState_.set_possessed_slot(0);
                        // Keep agent_state_.slots in sync with the GPU reset so
                        // patch streaming + ribbon + Caps Lock see current state.
                        std::memset(agent_state_.slots, 0, sizeof(agent_state_.slots));
                        agent_state_.slots[0].pos_x = 0.0f;  // Idle::PAWN_POS_X
                        agent_state_.slots[0].pos_y = 0.0f;
                        agent_state_.slots[0].pos_z = 0.0f;
                        agent_state_.slots[0].orient_w = 1.0f;
                        agent_state_.slots[0].is_active = 1u;
                        agent_state_.slots[0].behavior_id = AGENT_BEHAVIOR_PLAYER_CONTROLLED;
                        agent_state_.slots[0].tier_idx = preserved_tier;
                        agent_state_.slots[0].color_r = preserved_color_r;
                        agent_state_.slots[0].color_g = preserved_color_g;
                        agent_state_.slots[0].color_b = preserved_color_b;
                        agent_state_.slots[0].portal_trigger = -1;
                        player_.possessed_slot = 0;
                        gpuState_.set_world_seed(world_state_.active_seed);
                        apply_mood(this, pendingDestination_.mood, queue);
                        // ROSTER-GATE wanderers (c) — transition population (slots 1+); slot 0 preserved above.
                        if constexpr (ROSTER.wanderers)
                            spawn_population_for_mood(agent_state_, this, pendingDestination_.mood, world_state_.active_seed,
                                Idle::PAWN_POS_X, Idle::PAWN_POS_Z, queue);
                        dump_agent_census(agent_state_, this, "mood-transition");
                        // Deactivate ribbons in finite mode unless the mood
                        // spawns its own anchor ribbon in apply_mood.
                        if (world_state_.finite_mode && ribbon_state_.active_count > 0 && !MOOD_TABLE[mood_state_.active].has_anchor_ribbon) {
                            for (uint32_t i = 0; i < MAX_RIBBON_INSTANCES; i++) {
                                ribbon_state_.active[i] = ActiveRibbon{};
                                ribbon_state_.gpu[i] = GPURibbonState{};
                            }
                            ribbon_state_.active_count = 0;
                            ribbon_state_.rendered_slot = UINT32_MAX;
                            GPURibbonState empty{};
                            gpuState_.upload_ribbon(queue, empty);
                        }
                        // Schedule guaranteed back-portal in finite worlds
                        mood_state_.back_portal_pending = world_state_.finite_mode;

                        transitionPhase_ = TransitionPhase::FADE_IN;
                        mood_state_.transition_timer = 0.0f;
                        uint32_t side = world_state_.finite_mode ? 2 * world_state_.finite_radius + 1 : 0;
                        std::cout << "[World] Teardown complete, seed=" << world_state_.active_seed
                            << " mode=" << (world_state_.finite_mode ? "finite" : "open")
                            << (world_state_.finite_mode ? " " + std::to_string(side) + "x" + std::to_string(side) : "")
                            << "\n";
                    }
                    break;
                    case TransitionPhase::FADE_IN:
                        mood_state_.transition_fade_alpha = std::max(0.0f, 1.0f - mood_state_.transition_timer / mood_state_.transition_fade_duration);
                        if (mood_state_.transition_fade_alpha <= 0.0f) {
                            transitionPhase_ = TransitionPhase::IDLE;
                            mood_state_.transition_fade_alpha = 0.0f;
                        }
                        break;
                    default: break;
                    }
                }
                gpuState_.set_fade(mood_state_.transition_fade_alpha, 0.0f, 0.0f, 0.0f);

                gpuState_.upload_signal(queue, gpuSignal);

                gpuState_.upload_config(queue);

                // Orb dome anchor: follow pawn when toggled on. Uses
                // last-frame pawn readback — one-frame lag is imperceptible.
                if constexpr (ROSTER.orbs)  // ROSTER-GATE orbs (b)
                    update_orb_anchor(orbs_state_, this, player_.readback_x, player_.readback_z, queue);

                // --- Clear deltas for next frame ------------------------------------
                update_photographer(gallery_state_, this, queue);
                clear_input_deltas(this);
            }

            // SEAM[spine:owns] render() is genuinely spine work: readback state
            //   machines, stale-callback guards, portal trigger handling,
            //   patch streaming, photographer cadence. The K1 observation
            //   doesn't apply to render() the same way it applies to update();
            //   render() mixes orchestration (correct) with smaller per-module
            //   GPU upload calls (each lives in its module already).
            void render(wgpu::CommandEncoder& encoder,
                wgpu::TextureView backbuffer,
                wgpu::TextureView depth) override {

                wgpu::Queue queue = device_.GetQueue();

                //
                if (pawnReadbackState_ == PawnReadbackState::COPIED) {
                    pawnReadbackState_ = PawnReadbackState::MAPPING;
                    gpuState_.agent_state_readback_staging().MapAsync(
                        wgpu::MapMode::Read, 0, GPUState::agent_state_buffer_size(),
                        wgpu::CallbackMode::AllowSpontaneous,
                        [this, gen = world_state_.world_gen](wgpu::MapAsyncStatus status, wgpu::StringView) {
                            if (status == wgpu::MapAsyncStatus::Success) {
                                // Drop stale callbacks from a previous world: gen
                                // captured at issue time differs from current
                                // world_state_.world_gen if a teardown happened in between.
                                // Buffer is still successfully mapped though, so
                                // we Unmap unconditionally (mapping contract is
                                // independent of whether we read the data).
                                if (gen == world_state_.world_gen) {
                                    const auto* data = static_cast<const GPUAgentState*>(
                                        gpuState_.agent_state_readback_staging().GetConstMappedRange(
                                            0, GPUState::agent_state_buffer_size()));
                                    if (data) {
                                        std::memcpy(agent_state_.slots, data,
                                            GPUState::agent_state_buffer_size());
                                        const auto& p = agent_state_.slots[player_.possessed_slot];
                                        player_.readback_x = p.pos_x;
                                        player_.readback_z = p.pos_z;
                                        player_.readback_portal_trigger = p.portal_trigger;
                                    }
                                }
                                gpuState_.agent_state_readback_staging().Unmap();
                            }
                            pawnReadbackState_ = PawnReadbackState::IDLE;
                        });
                }

                //
                if (floaterReadbackState_ == FloaterReadbackState::COPIED) {
                    floaterReadbackState_ = FloaterReadbackState::MAPPING;
                    gpuState_.floating_entity_readback_staging().MapAsync(
                        wgpu::MapMode::Read, 0, GPUState::floating_entity_buffer_size(),
                        wgpu::CallbackMode::AllowSpontaneous,
                        [this, gen = world_state_.world_gen](wgpu::MapAsyncStatus status, wgpu::StringView) {
                            if (status == wgpu::MapAsyncStatus::Success) {
                                // Drop stale callbacks from a previous world.
                                // Buffer is still mapped, so Unmap unconditionally.
                                if (gen == world_state_.world_gen) {
                                    const auto* data = static_cast<const GPUFloatingEntityState*>(
                                        gpuState_.floating_entity_readback_staging().GetConstMappedRange(
                                            0, GPUState::floating_entity_buffer_size()));
                                    if (data) {
                                        static constexpr float SPAWN_PROTECTION_S = 0.10f;
                                        float now = time_state_.seconds;
                                        // Spheres: slots [0, MAX_SPHERE_INSTANCES)
                                        for (uint32_t i = 0; i < Dim::MAX_SPHERE_INSTANCES; i++) {
                                            bool gpu_active = (data[i].is_active != 0u);
                                            // sphere active-slot mirror owned by SphereState (spheres.hpp)
                                            if (sphere_state_.activeFloaters_[i].active && !gpu_active &&
                                                (now - sphere_state_.activeFloaters_[i].last_alloc_time) > SPAWN_PROTECTION_S) {
                                                sphere_state_.activeFloaters_[i].active = false;
                                                if (sphere_state_.activeFloaterCount_ > 0) sphere_state_.activeFloaterCount_--;
                                            }
                                        }
                                        // Cubes: slots [CUBE_SLOT_OFFSET, TOTAL_FLOATING_SLOTS)
                                        for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
                                            bool gpu_active = (data[Dim::CUBE_SLOT_OFFSET + i].is_active != 0u);
                                            // cube active-slot mirror owned by CubeBehaviorsState (cube_behaviors.hpp)
                                            if (cube_behaviors_state_.activeCubes_[i].active && !gpu_active &&
                                                (now - cube_behaviors_state_.activeCubes_[i].last_alloc_time) > SPAWN_PROTECTION_S) {
                                                cube_behaviors_state_.activeCubes_[i].active = false;
                                                if (cube_behaviors_state_.activeCubeCount_ > 0) cube_behaviors_state_.activeCubeCount_--;
                                            }
                                        }
                                    }
                                }
                                gpuState_.floating_entity_readback_staging().Unmap();
                            }
                            floaterReadbackState_ = FloaterReadbackState::IDLE;
                        });
                }

                // Check if GPU reported a portal trigger
                // ROSTER-GATE transitions (b) — ENTRY door #2 (portal trigger).
                // With transitions off + portal on (a legal config), portal
                // arches exist but stepping through must NOT start a transition.
                if constexpr (ROSTER.transitions)
                if (player_.readback_portal_trigger >= 0 && transitionPhase_ == TransitionPhase::IDLE) {
                    uint32_t arch_idx = static_cast<uint32_t>(player_.readback_portal_trigger);
                    player_.readback_portal_trigger = -1;
                    if (arch_idx < Dim::MAX_ARCH_INSTANCES &&
                        entities_state_.arches[arch_idx].active &&
                        entities_state_.arches[arch_idx].is_portal) {
                        pendingDestination_ = entities_state_.arches[arch_idx].destination;
                        transitionPhase_ = TransitionPhase::FADE_OUT;
                        mood_state_.transition_timer = 0.0f;
                        std::cout << "[Portal] GPU trigger: arch " << arch_idx
                            << " -> seed=" << pendingDestination_.seed
                            << " finite=" << pendingDestination_.finite << "\n";
                    }
                }

                // Refill any agent slots the GPU evicted last frame.
                // No-op when no slots were evicted — just a 32-slot scan.
                // ROSTER-GATE wanderers (b) — per-frame refill of evicted NPC slots (1+); slot 0 never evicted.
                if constexpr (ROSTER.wanderers)
                    respawn_evicted_agents(agent_state_, this, mood_state_.active, world_state_.active_seed, queue);

                tick_cube_corral_animations(cube_behaviors_state_, this, queue);

                stream_patches(this, encoder, queue);

                // DIAG-unwrapped (census: constitution §5): autonomous
                // stdout — wrap in #ifdef DIAG_AGENT_CENSUS at ship.
                // Periodic agent census dump — followed by the player's
                // last-known position from the GPU readback. The pos line
                // tells us at-a-glance whether the readback is current
                // (a stuck readback would freeze the position; an idle
                // player would do the same — pair the two by visiting
                // the world manually if you need to disambiguate).
                // ROSTER-RESIDUE gol (2e) — residue recipe. When gol is
                // disabled it is never selected (b), so zone_count stays 0 and
                // the sole writer of the zone GPU buffers (the compute block
                // above, guarded by zone_count>0) never runs. Prove it across
                // frames: report pristine, and fail LOUD if either invariant
                // is ever violated. Used at gate G3. Zero effect when enabled.
                if constexpr (!ROSTER.gol) {
                    if (time_state_.seconds - rosterGolResidueDump_ >= AGENT_CENSUS_INTERVAL) {
                        if (gol_state_.zone_count != 0 || rosterGolZoneRuns_ != 0) {
                            std::cerr << "[ROSTER residue] VIOLATION: gol disabled but zone_count="
                                << gol_state_.zone_count << " runs=" << rosterGolZoneRuns_ << "\n";
                        } else {
                            std::cout << "[ROSTER residue] gol disabled: zone buffers pristine"
                                << " (zone_count=0, zone-compute runs this session=0)\n";
                        }
                        rosterGolResidueDump_ = time_state_.seconds;
                    }
                }

                if (time_state_.seconds - agent_state_.last_census_dump >= AGENT_CENSUS_INTERVAL) {
                    dump_agent_census(agent_state_, this, "periodic");
                    const auto& player = agent_state_.slots[0];
                    std::cout << "[Player] pos=(" << std::fixed << std::setprecision(1)
                        << player.pos_x << "," << player.pos_z
                        << ") slot=" << player_.possessed_slot
                        << " behavior=" << player.behavior_id
                        << "\n";
                    agent_state_.last_census_dump = time_state_.seconds;
                }

                // Periodic entity census dump
#ifdef DIAG_ENTITY_CENSUS
                if (time_state_.seconds - spawn_engine_state_.lastCensusDump_ >= CENSUS_DUMP_INTERVAL) {
                    dump_entity_census(this, "periodic");
                    spawn_engine_state_.lastCensusDump_ = time_state_.seconds;
                }
#endif

                // ─── Ribbon per-frame ── one call; the conductor lives in
                // bodies/ribbon.inl (FRAME ORCHESTRATION). SEAM[ribbon:sky-mode].
                ribbon_frame_tick(ribbon_state_, this, queue);

                // ─── Entity mesh gen: single compute pass for all dirty families ──
                {
                    bool dirty[PopFamily::COUNT] = {};
                    bool anyDirty = false;
                    // ROSTER-GATE family (b) — the per-frame mesh-prepare
                    // loop folds over the roster at COMPILE TIME (R1 hot-path
                    // caveat): a disabled family's prepare_mesh is eliminated
                    // (no call, no runtime branch on a disabled piece).
                    // All-enabled unrolls to the original 12 calls in order.
                    #define ROSTER_PREP_FAMILY(F) \
                        if constexpr (ROSTER.family_enabled(F)) { \
                            dirty[F] = FAMILY_DISPATCH[F].prepare_mesh(this, queue); \
                            anyDirty = anyDirty || dirty[F]; \
                        }
                    ROSTER_PREP_FAMILY(PopFamily::PYRAMID);
                    ROSTER_PREP_FAMILY(PopFamily::ARCH);
                    ROSTER_PREP_FAMILY(PopFamily::COLUMN);
                    ROSTER_PREP_FAMILY(PopFamily::ANTENNA);
                    ROSTER_PREP_FAMILY(PopFamily::PALM);
                    ROSTER_PREP_FAMILY(PopFamily::CACTUS);
                    ROSTER_PREP_FAMILY(PopFamily::BLADE);
                    ROSTER_PREP_FAMILY(PopFamily::SPHERE);
                    ROSTER_PREP_FAMILY(PopFamily::RIBBON);
                    ROSTER_PREP_FAMILY(PopFamily::CUBE);
                    ROSTER_PREP_FAMILY(PopFamily::GOL);
                    ROSTER_PREP_FAMILY(PopFamily::GALLERY);
                    #undef ROSTER_PREP_FAMILY
                    if (anyDirty) {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "Entity Mesh Gen";
                        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                        // dispatch skips disabled families structurally:
                        // dirty[f] stays false for a disabled family (never
                        // set above), so this branches on dirty-ness, not on
                        // the enable bit.
                        for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
                            if (dirty[f]) FAMILY_DISPATCH[f].dispatch_mesh(this, pass);
                        }
                        pass.End();
                    }
                }
                upload_portal_array(this, queue);
                upload_lights(this, queue);

                // Re-sync the pawn mount to THIS frame. ribbon_advance_head (in the
                // ribbon block above) just recomputed the head mount, but the signal
                // uploaded earlier — and therefore the pawn — still carries the previous
                // frame's mount. Re-write the sky_* block so the pawn and the ribbon are
                // sampled at the same frame: the one-frame lag (a ~throttle·MAX_SPEED·dt
                // slide along the tube) disappears, leaving MOUNT_SETBACK as the sole
                // seat offset. Ordered before dispatch_compute, which runs the player
                // agent kernel that reads sky_*. SEAM[ribbon:sky-mode].
                {
                    float hx, hy, hz, hh;
                    ribbon_head_pose(ribbon_state_, hx, hy, hz, hh);
                    float fyaw, fpitch, froll;
                    ribbon_head_frame(ribbon_state_, fyaw, fpitch, froll);
                    gpuState_.resync_sky_head(queue, player_.sky_mode ? 1u : 0u,
                                              hx, hy, hz, hh, fyaw, fpitch, froll);
                }

                dispatch_compute(this, encoder);

                // Copy full agent buffer from GPU to staging (for readback next frame)
                if (pawnReadbackState_ == PawnReadbackState::IDLE) {
                    encoder.CopyBufferToBuffer(
                        gpuState_.agent_state_buffer(), 0,
                        gpuState_.agent_state_readback_staging(), 0,
                        GPUState::agent_state_buffer_size());
                    pawnReadbackState_ = PawnReadbackState::COPIED;
                }

                if (floaterReadbackState_ == FloaterReadbackState::IDLE) {
                    encoder.CopyBufferToBuffer(
                        gpuState_.floating_entity_buffer(), 0,
                        gpuState_.floating_entity_readback_staging(), 0,
                        GPUState::floating_entity_buffer_size());
                    floaterReadbackState_ = FloaterReadbackState::COPIED;
                }

                // GoL zone compute — derive params + sync + evolve (separate passes for barrier)
                if (gol_state_.zone_count > 0) {
                    rosterGolZoneRuns_++;  // ROSTER-RESIDUE gol (2e) — the only writer of the zone GPU buffers; counted so the disabled-piece residue check can prove pristine
                    flush_zone_derive_requests(gol_state_, this, queue);
                    upload_gol_zone_config(gol_state_, this, queue);

                    {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "GoL Zone Sync";
                        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                        renderer_.dispatch_zone_gol_sync(pass,
                            gpuState_.zone_gol_compute_group(), gol_state_.active_slot_count);
                        pass.End();
                    }
                    {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "GoL Zone Evolve";
                        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                        renderer_.dispatch_zone_gol_evolve(pass,
                            gpuState_.zone_gol_compute_group(), gol_state_.active_slot_count);
                        pass.End();
                    }

                    // Mesh gen pass (Group 0 = compute entity, Group 1 = zone mesh gen)
                    {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "GoL Zone Mesh Gen";
                        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                        renderer_.dispatch_zone_mesh_reset(pass,
                            gpuState_.zone_mesh_gen_group());
                        renderer_.dispatch_zone_mesh_gen(pass,
                            gpuState_.zone_mesh_gen_group(),
                            gol_state_.active_slot_count);
                        pass.End();
                    }
                }

                // Pawn aura compute — persistent terrain influence
                // Run while presence > 0 (ramping down after toggle-off) or clearing
                // ROSTER-GATE pawn_aura (b) — disabled: the whole aura compute
                // (config upload + dispatch) is eliminated; zero GPU writes.
                if constexpr (ROSTER.pawn_aura)
                if (player_.aura_presence > 0.0f || pawn_state_.aura_needs_clear) {
                    if (pawn_state_.aura_cfg_dirty) {
                        // Full config upload — profile changed or first frame
                        pawn_state_.aura_cfg_dirty = false;
                        const auto& ap = pawn_state_.active_aura_profile;

                        // Presence scales all aura params for smooth raise/lower
                        float p = player_.aura_presence;

                        GPUPawnAuraConfig auraCfg{};
                        auraCfg.cell_size = PATCH_CELL_SIZE;
                        auraCfg.influence_radius = ap.influence_radius * p;
                        auraCfg.attack_stiffness = ap.attack_stiffness;
                        auraCfg.attack_damping = ap.attack_damping;
                        auraCfg.release_rate = (p > 0.01f) ? ap.release_rate : 999.0f;
                        auraCfg.dt = time_state_.dt;
                        auraCfg.effect_mask = ap.effect_mask;
                        auraCfg.aura_n = 64;
                        auraCfg.tint_strength = std::min(ap.tint_strength * p, 1.0f);
                        auraCfg.tint_r = ap.tint_r;
                        auraCfg.tint_g = ap.tint_g;
                        auraCfg.tint_b = ap.tint_b;
                        auraCfg.delta_mode = ap.delta_mode;
                        auraCfg.delta_magnitude = ap.delta_magnitude;
                        auraCfg.t_beats = time_state_.beats;
                        // height_scale gates the compute shader's R channel write (> 0.01 = enabled).
                        // Actual terrain extrusion magnitude comes from config.pawn_aura_height in the VS.
                        auraCfg.height_scale = (pawn_state_.aura_height_enabled && p > 0.01f) ? ap.height_scale : 0.0f;
                        gpuState_.upload_pawn_aura_config(queue, auraCfg);
                    }
                    else {
                        // Steady state — only dt and t_beats change per frame
                        gpuState_.upload_pawn_aura_frame(queue, time_state_.dt, time_state_.beats);
                    }

                    wgpu::ComputePassDescriptor cpd{};
                    cpd.label = "Pawn Aura";
                    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                    renderer_.dispatch_compute_pawn_aura(pass,
                        gpuState_.pawn_aura_compute_group(),
                        GPUState::pawn_aura_workgroups());
                    pass.End();

                    // After one cleanup frame with release_rate=999, all cells are zero
                    if (pawn_state_.aura_needs_clear) { pawn_state_.aura_needs_clear = false; }
                }

                // Orb sky layer: one-shot init, optional color-only refresh,
                // snapshot previous state for flocking neighbor reads, then
                // advance dynamics.
                // ROSTER-GATE orbs (b) — disabled: no orb compute dispatched.
                if constexpr (ROSTER.orbs) {
                    dispatch_orb_init(orbs_state_, this, encoder);
                    dispatch_orb_recolor(orbs_state_, this, encoder);
                    dispatch_orb_copy_prev(orbs_state_, this, encoder);
                    dispatch_orb_dynamics(orbs_state_, this, encoder, queue);
                }

                if (world_state_.ground_entries_dirty) {
                    world_state_.ground_entries_dirty = false;
                    world_state_.placement_dirty = true;
                    upload_ground_entries(this, queue);
                }
                if (world_state_.placement_dirty) {
                    world_state_.placement_dirty = false;
                    dispatch_placement_correction(this, encoder);
                }

                // DIAG: frustum cull re-enabled — indirect draw active
                dispatch_frustum_cull(this, encoder, queue);

                render_shadow_pass(this, encoder);
                render_main_pass(this, encoder, backbuffer, depth);
                render_snapshot_pass(gallery_state_, this, encoder);

                for (uint32_t i = 0; i < gallery_state_.pending_promotion_count; i++) {
                    auto& p = gallery_state_.pending_promotions[i];
                    wgpu::Texture src = p.is_snapshot
                        ? gpuState_.snapshot_staging_texture()
                        : gpuState_.authored_staging_texture();
                    gpuState_.promote_to_exhibition(encoder, src, p.staging_layer, p.exhibition_layer);
                }
                gallery_state_.pending_promotion_count = 0;
            }

            // Mood is VOCABULARY + APPLIERS + SIX DOORS: CeilingType /
            // MoodProfile / MOOD_TABLE / portal colors / indoor palettes +
            // the door, applier, and deriver declarations are in mood.hpp
            // (file scope, above the class); the definitions (which reach
            // the spine-owned state + in-class statics via the complete
            // type) are in mood.inl, included at FILE SCOPE in the
            // post-class MODULE IMPLEMENTATIONS zone. MOOD OWNS NO STATE —
            // nothing at the COMPOSITION ROOT; mood_state_ and the
            // transition machine are spine-resident
            // (SEAM[spine:transitions], constitution §2). The force-spawn
            // mutation belongs to the arch's owner: entities'
            // force_spawn_portal_arch (the ROSTER portal door lives
            // there). The lighting-scheme tables stay impl-side. See §1.

        public:

            void on_input(const InputEvent& event) override {
                switch (event.type) {
                case InputEvent::Type::KeyDown:
                    on_key_down(this, event.key);
                    break;
                case InputEvent::Type::KeyUp:
                    on_key_up(this, event.key);
                    break;
                case InputEvent::Type::MouseMove:
                    on_mouse_move(this, event.x, event.y);
                    break;
                case InputEvent::Type::MouseButton:
                    on_mouse_button(this, event.button, event.pressed);
                    break;
                case InputEvent::Type::Scroll:
                    on_scroll(this, event.y);
                    break;
                }
            }

        private:

        public:

            void get_clear_color(float& r, float& g, float& b) const override {
                r = clearColor_[0];
                g = clearColor_[1];
                b = clearColor_[2];
            }

            wgpu::TextureFormat depth_format() const override {
                return wgpu::TextureFormat::Depth24Plus;
            }

            bool supports_backspace() const override {
                return true;
            }

            bool reload_shaders() override { return renderer_.reload(); }
            const std::string& shader_path() const { return renderer_.shader_path(); }

        };

    } // namespace the_board
} // namespace t7

// ═══ MODULE IMPLEMENTATIONS (post-class, FILE SCOPE) ══════════════════
//
// WIRING FORM (fix-2): SELF-WRAPPING — the zone includes impls at FILE SCOPE; law in audit/LADDER.md.
#include "bodies/pawn.inl"       // tick_pawn_couplings
#include "bodies/entities.inl"   // the six preparers + the seven grounded-family evictors + the blade/palm/cactus recipes
#include "bodies/spheres.inl"    // the sphere evictor + recipe
#include "bodies/orbs.inl"       // orb lifecycle/commands/dispatches/render
#include "bodies/gol_zones.inl"  // GoL three-phase lifecycle + per-frame uploads/dispatch
#include "bodies/agents.inl"     // agent registry upload + spawn/respawn/possession/diagnostics
#include "bodies/cube_behaviors.inl"  // cube registry upload + corral/kite/coordination + clear + evictor + recipe
#include "bodies/gallery.inl"    // photographer + gallery sites + authored loading + wall paintings
#include "bodies/ribbon.inl"     // author seats + head laws + frame conductor + three-phase lifecycle
#include "direction/input.inl"      // key/mouse dispatch + movement intent + camera commands (own GLFW include)
#include "realization/render_passes.inl"  // ground-entry prep + compute dispatch + shadow/main passes + light VPs
#include "direction/mood.inl"       // indoor light derivation + appliers + apply_mood + shell + portals + uploads + transition request + derivers
#include "surface/population_themes.inl"  // the envelope machine per-patch step
#include "surface/tile_world.inl"  // the four verbs over what the terrain remembers
#include "machine/spawn_engine.inl"  // the spawn engine — negotiation + footprints + culling + census + the select/place/commit loops
#include "surface/patch_system.inl"  // the active-patch machine — registry lifecycle + budgets + teardown + allocator + the streaming conductor
#include "machine/entity_pipeline.inl"  // the generic three-phase verbs + the welded four family blocks (column/antenna/pyramid/arch)
#include "machine/family_dispatch.inl"  // THE TABLE — FAMILY_DISPATCH definition + shared no-op mesh adapters
