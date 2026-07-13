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
#include "cartridges/the_board/realization/state.hpp"
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
#include "cartridges/the_board/surface/population_themes.hpp"  // S2: THEMES + ThemeEnvelope + ThemesState — MERGED single file (DISSOLVE-1 d3 #1)
#include "cartridges/the_board/surface/patch_system.hpp"     // S2: WorldState + ActivePatch + budgets + visibility + PatchSystemState + decls (impl is patch_system.inl, post-class)
#include "cartridges/the_board/surface/tile_world.hpp"          // S2: archetypes + tokens + TileState/cache + TileWorldDeps + impl — MERGED single file (DISSOLVE-1 Batch A d3); after patch_system for WorldState/PATCH_EXTENT
#include "cartridges/the_board/machine/spawn_engine.hpp"     // S3: spawn vocabulary + separation/proximity tables + SpawnEngineState + the preamble template + decls (impl is spawn_engine.inl, post-class)
#include "cartridges/the_board/machine/entity_pipeline.hpp"   // S3: the rescale template + arch vocabulary (ArchIdx/ARCH_TIERS) + the three-phase verb decls (impl is entity_pipeline.inl, post-class)
#include "cartridges/the_board/bodies/spheres.hpp"              // SphereState + SphereDeps + impl — MERGED single file (DISSOLVE-1 Batch A d3); after entity_pipeline for the generic funnels
#include "cartridges/the_board/realization/renderer.hpp"
#include "cartridges/the_board/bodies/pawn.hpp"                 // PawnState + PawnDeps + impl — MERGED single file (DISSOLVE-1 Batch A d3); after renderer for Renderer/GPUState complete
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

            // ═══ THE MACHINE FACE (DISSOLVE-1 d1) ═══════════════════════
            // The one declared context the dispatch contract hands the
            // rows — references bound once, in the constructor, to the
            // organs above (contracts/entity_types.hpp owns the type).
            MachineCtx machine_ctx_;

            // Per-module deps faces (DISSOLVE-1 Batch A d2) — bound once
            // in the ctor, each the module's requirements made literal.
            TileWorldDeps tile_world_deps_;
            SphereDeps    sphere_deps_;
            PawnDeps      pawn_deps_;

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

            static bool dispatch_prepare_mesh_pyramid(MachineCtx* self, wgpu::Queue& queue) {
                return prepare_pyramid_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_pyramid(MachineCtx* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_pyramid_mesh_gen(pass, self->gpuState_.pyramid_mesh_gen_group());
            }

            static bool dispatch_prepare_mesh_arch(MachineCtx* self, wgpu::Queue& queue) {
                return prepare_arch_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_arch(MachineCtx* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_arch_mesh_gen(pass, self->gpuState_.arch_mesh_gen_group());
            }

            static bool dispatch_prepare_mesh_column(MachineCtx* self, wgpu::Queue& queue) {
                return prepare_column_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_column(MachineCtx* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_column_mesh_gen(pass, self->gpuState_.column_mesh_gen_group());
            }

            // ── Mesh gen dispatch wrappers (palm) ──

            static bool dispatch_prepare_mesh_palm(MachineCtx* self, wgpu::Queue& queue) {
                return prepare_palm_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_palm(MachineCtx* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_palm_mesh_gen(pass, self->gpuState_.palm_mesh_gen_group());
            }

            // ── Mesh gen dispatch wrappers (cactus) ──

            static bool dispatch_prepare_mesh_cactus(MachineCtx* self, wgpu::Queue& queue) {
                return prepare_cactus_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_cactus(MachineCtx* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_cactus_mesh_gen(pass, self->gpuState_.cactus_mesh_gen_group());
            }

            static bool dispatch_prepare_mesh_blade(MachineCtx* self, wgpu::Queue& queue) {
                return prepare_blade_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_blade(MachineCtx* self, wgpu::ComputePassEncoder& pass) {
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

            Cartridge()
                : machine_ctx_{ world_state_, tile_world_state_, themes_state_,
                                mood_state_, patch_system_state_, spawn_engine_state_,
                                entities_state_, sphere_state_, cube_behaviors_state_,
                                ribbon_state_, gol_state_, gallery_state_,
                                time_state_, player_, gpuState_, renderer_ }
                , tile_world_deps_{ world_state_, mood_state_, gpuState_ }
                , sphere_deps_{ time_state_ }
                , pawn_deps_{ player_, time_state_, gpuState_, renderer_ } {}

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

                // ═══ MOVEMENT: BOOT — REALIZATION (the stage exists first) ══
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

                // ═══ MOVEMENT: BOOT — S2 THE SURFACE ════════════════════════
                init_patch_system(this);
                setup_test_rig_piers(this, device_.GetQueue());

                // ═══ MOVEMENT: BOOT — PER-PIECE BOOT VERBS (part one) ═══════
                // Order is today's, preserved byte-for-byte (PRIME INVARIANT);
                // one conductor call per piece, presence constexpr-gated.
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

                // ═══ MOVEMENT: BOOT — S3 PLACEMENT ══════════════════════════
                {
                    // Slot 0, the pawn — ungated: the player body is
                    // unconditional (owner verb; REBUILD-0 m2, stray (3) home).
                    seed_player_body(agent_state_, this);

                    wgpu::Queue q = device_.GetQueue();
                    // ROSTER-GATE wanderers (c) — boot population (agent slots
                    // 1+). Slot 0 (the pawn, seeded just above) is untouched.
                    if constexpr (ROSTER.wanderers)
                        spawn_population_for_mood(agent_state_, this, mood_state_.active, world_state_.active_seed,
                            Idle::PAWN_POS_X, Idle::PAWN_POS_Z, q);
                    dump_agent_census(agent_state_, this, "boot");
                }

                // ═══ MOVEMENT: BOOT — PER-PIECE BOOT VERBS (part two) ═══════
                // Eager-load authored paintings at boot (avoids mid-frame stall on first gallery).
                // ROSTER-GATE gallery (c) — P2 DIES STRUCTURALLY (REBUILD-0):
                // disabled, the authored-staging textures stay pristine.
                if constexpr (ROSTER.gallery) {
                    wgpu::Queue q = device_.GetQueue();
                    load_authored_textures(gallery_state_, &machine_ctx_, q);
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
                // ═══ MOVEMENT: THE CLOCK AND THE SIGNAL (root) ══════════════
                // Frame-signal fill (O-5a: dt_beats reads prev_beats BEFORE the
                // clock block advances it), the SNAP-1 neutral sky words, then
                // the clock + tempo follower.
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
                    gpuSignal.sky_mode    = 0u;  // m6: the whole block is neutral now — the ribbon tick's tail resync is the sole author
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

                // ═══ MOVEMENT: S4 MOTION — DRIVERS ══════════════════════════
                // The music driver authors params through the canvas; fog is
                // its first staged consumer. (Input was harvested by the
                // on_input callbacks; its deltas rode the signal fill above.)
                visual_canvas_.tick(signal);
                if (fog_density_dst_.valid && fog_color_dst_.valid) {
                    const VisualParams& fp = visual_canvas_.params();
                    gpuState_.set_fog(fp.get(fog_density_dst_.base),
                                      fp.get(fog_color_dst_.base + 0),
                                      fp.get(fog_color_dst_.base + 1),
                                      fp.get(fog_color_dst_.base + 2));
                }

                // ═══ MOVEMENT: S4 MOTION — BODIES ═══════════════════════════
                // Pawn presence ramp + aura height computation.
                // Lives in pawn.inl as a real-time exponential tick (closes pawn:K1).
                if constexpr (ROSTER.pawn_aura)  // ROSTER-GATE pawn_aura (b) — disabled: presence never raised, no aura height
                    tick_pawn_couplings(pawn_state_, &pawn_deps_, queue);

                // ═══ MOVEMENT: REALIZATION STAGING (part one — world seed +
                // finite bounds) ═════════════════════════════════════════════
                // Stays PRE-machine (RC policy: today's order kept, constraint
                // recorded): the TEARDOWN case re-stages the seed itself, and
                // moving the bounds after the machine would ship the NEW
                // world's bounds one frame early on the teardown frame.
                gpuState_.set_world_seed(world_state_.active_seed);
                if (world_state_.finite_mode) {
                    float bmin = -(float)world_state_.finite_radius * PATCH_EXTENT;
                    float bmax = ((float)world_state_.finite_radius + 1.0f) * PATCH_EXTENT;
                    gpuState_.set_world_bounds(bmin, bmin, bmax, bmax);
                }
                else {
                    gpuState_.set_world_bounds(0.0f, 0.0f, 0.0f, 0.0f);
                }

                // ═══ MOVEMENT: THE TRANSITION MACHINE (spine-owned;
                // SEAM[spine:transitions]) ═══════════════════════════════════
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
                        // ═══ MOVEMENT: TEARDOWN (fixed sequence O-3) ════════
                        // This TEARDOWN block owns the integration concerns:
                        //   worldGen bump (P5 stale-callback guard),
                        //   return-state capture, per-owner teardown verbs
                        //   (REBUILD-0 m2, stamp D4 — the owner-verb pattern
                        //   that already lived inside teardown_world,
                        //   completed), agent reset, repopulation.
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

                        // The surface core first, then one teardown verb per
                        // owner. The per-organ clears are independent (each
                        // touches only its organ + its own GPU slots), so the
                        // owner-verb order is free; the new gates eliminate
                        // only zeros-over-pristine GPU writes (disclosed at
                        // the ladder).
                        teardown_surface(this, queue);
                        teardown_entities(this, queue);
                        if constexpr (ROSTER.gol)      // ROSTER-GATE gol (c) — teardown clear skipped when disabled (organ pristine)
                            teardown_gol(this, queue);
                        if constexpr (ROSTER.ribbon)   // ROSTER-GATE ribbon (c) — same zero-write elimination
                            teardown_ribbon(this, queue);
                        if constexpr (ROSTER.sphere)   // ROSTER-GATE sphere (c)
                            clear_spheres(sphere_state_, gpuState_, queue);
                        if constexpr (ROSTER.cube)     // ROSTER-GATE cube (c)
                            clear_cubes(cube_behaviors_state_, gpuState_, queue);
                        // The gallery organ is SHARED with indoor_shell (wall
                        // frames live in the same painting slots — form_type).
                        if constexpr (ROSTER.gallery || ROSTER.indoor_shell)  // ROSTER-GATE gallery+indoor_shell (c)
                            teardown_gallery(this, queue);
                        if constexpr (ROSTER.pawn_aura)  // ROSTER-GATE pawn_aura (c) — teardown clear skipped when disabled (no aura to clear)
                            teardown_pawn_aura(pawn_state_);
                        // Sky orbs: apply_mood re-enables + re-seeds as needed
                        if constexpr (ROSTER.orbs)  // ROSTER-GATE orbs (c) — teardown one-shot skipped when disabled
                            teardown_orbs(orbs_state_, this);

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
                        // CPU mirror reseed rides with its owner (agents;
                        // REBUILD-0 m2, stray (3)'s transition twin).
                        reseed_player_body(agent_state_, this, preserved_tier,
                            preserved_color_r, preserved_color_g, preserved_color_b);
                        gpuState_.set_world_seed(world_state_.active_seed);
                        apply_mood(this, pendingDestination_.mood, queue);
                        // ROSTER-GATE wanderers (c) — transition population (slots 1+); slot 0 preserved above.
                        if constexpr (ROSTER.wanderers)
                            spawn_population_for_mood(agent_state_, this, pendingDestination_.mood, world_state_.active_seed,
                                Idle::PAWN_POS_X, Idle::PAWN_POS_Z, queue);
                        dump_agent_census(agent_state_, this, "mood-transition");
                        // ROSTER-GATE ribbon (c) — finite-mode release, owner
                        // verb (REBUILD-0 m2, stray (4) home). Zero effect
                        // when ribbon is off (active_count stays 0). ORDER
                        // (O-3): after apply_mood set mood_state_.active.
                        if constexpr (ROSTER.ribbon)
                            release_finite_ribbons(this, queue);
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
                // ═══ MOVEMENT: REALIZATION STAGING (part two — fade + the two
                // uploads) ═══════════════════════════════════════════════════
                // O-5b/c: fade after the machine (alpha is current-frame);
                // upload_signal then upload_config after ALL staging setters.
                gpuState_.set_fade(mood_state_.transition_fade_alpha, 0.0f, 0.0f, 0.0f);

                gpuState_.upload_signal(queue, gpuSignal);

                gpuState_.upload_config(queue);

                // ═══ MOVEMENT: WITNESS ══════════════════════════════════════
                // Orb dome anchor: follow pawn when toggled on. Uses
                // last-frame pawn readback — one-frame lag is imperceptible (O-5d).
                if constexpr (ROSTER.orbs)  // ROSTER-GATE orbs (b)
                    update_orb_anchor(orbs_state_, this, player_.readback_x, player_.readback_z, queue);
                // ROSTER-GATE gallery (b) — P1 DIES STRUCTURALLY (REBUILD-0):
                // the photographer never walks in a gallery-less demo.
                if constexpr (ROSTER.gallery)
                    update_photographer(gallery_state_, this, queue);

                // ═══ MOVEMENT: DRIVER BOOKKEEPING ═══════════════════════════
                // O-5e: dead-last — the signal fill above consumed the deltas.
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

                // ═══ MOVEMENT: WITNESS — HARVEST (P5 maps; consumes LAST
                // frame's capture) ═══════════════════════════════════════════
                // Leads the score: every downstream consumer (stream center,
                // portal door, corral, sorts) eats its output. The charter's
                // after-motion seat is vetoed here by the P5 protocol (O-2);
                // the CAPTURE half sits after dispatch_compute below.
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
                                        // Owner mirror reconciliation (REBUILD-0
                                        // m2 — stray (1) home; funnels live with
                                        // spheres / cube_behaviors).
                                        if constexpr (ROSTER.sphere)  // ROSTER-GATE sphere (b) — no spheres, no mirror to release
                                            reconcile_sphere_mirror(sphere_state_, &sphere_deps_, data);
                                        if constexpr (ROSTER.cube)    // ROSTER-GATE cube (b)
                                            reconcile_cube_mirror(cube_behaviors_state_, this, data);
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

                // ═══ MOVEMENT: S2 SURFACE LIFECYCLE ═════════════════════════
                // The streaming conductor; carries the declared S3-trigger
                // seam inside (SEAM[patch:spawn-trigger] — select/place/commit
                // fire from the stream's own cadence).
                stream_patches(this, encoder, queue);

                // ═══ MOVEMENT: S3 PLACEMENT ═════════════════════════════════
                // REORDER RC-1 (stamped policy): respawn moved AFTER stream —
                // S3 after S2. Safety: respawn touches slots 1+ only (slot 0
                // is never evicted), and the stream's bubble center reads
                // player_.readback_x/z refreshed at HARVEST — no data edge.
                // Refill any agent slots the GPU evicted last frame.
                // No-op when no slots were evicted — just a 32-slot scan.
                // ROSTER-GATE wanderers (b) — per-frame refill of evicted NPC slots (1+); slot 0 never evicted.
                if constexpr (ROSTER.wanderers)
                    respawn_evicted_agents(agent_state_, this, mood_state_.active, world_state_.active_seed, queue);

                // ═══ MOVEMENT: S4 MOTION — BODIES ═══════════════════════════
                // REORDER RC-2 (stamped policy): corral moved after stream —
                // S4 after S2; the corral tick and patch eviction touch
                // disjoint cube fields per frame, and the rig's pixel gate
                // arbitrates. ROSTER-GATE cube (b) — ungated-site closed
                // (REBUILD-0): no cubes, no corral animation to advance.
                if constexpr (ROSTER.cube)
                    tick_cube_corral_animations(cube_behaviors_state_, this, queue);

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
                    dump_entity_census(&machine_ctx_, "periodic");
                    spawn_engine_state_.lastCensusDump_ = time_state_.seconds;
                }
#endif

                // ─── Ribbon per-frame ── one call; the conductor lives in
                // bodies/ribbon.inl (FRAME ORCHESTRATION). SEAM[ribbon:sky-mode].
                // ROSTER-GATE ribbon (b) — ungated-site closed (REBUILD-0):
                // disabled, the per-frame walk over empty slots is eliminated,
                // and with it the SNAP-1 resync at its tail (m6) — the sky
                // words then hold update()'s neutral zeros forever, which is
                // exactly the ribbon-less contract (F8 is D9-gated too).
                if constexpr (ROSTER.ribbon)
                    ribbon_frame_tick(ribbon_state_, this, queue);

                // ═══ MOVEMENT: REALIZATION ══════════════════════════════════
                // ─── Entity mesh gen: single compute pass for all dirty families ──
                {
                    bool dirty[PopFamily::COUNT] = {};
                    bool anyDirty = false;
                    // Twelve explicit prepare lines, one per family, each
                    // presence constexpr-gated — THE SCORE RULING (REBUILD-0
                    // m2): the typelist fold dissolved into prose. A disabled
                    // family's prepare is eliminated at COMPILE TIME (no call,
                    // no runtime branch); all-enabled compiles to the same 12
                    // calls in the same order.
                    if constexpr (ROSTER.pyramid) {   // ROSTER-GATE pyramid (b)
                        dirty[PopFamily::PYRAMID] = FAMILY_DISPATCH[PopFamily::PYRAMID].prepare_mesh(&machine_ctx_, queue);
                        anyDirty = anyDirty || dirty[PopFamily::PYRAMID];
                    }
                    if constexpr (ROSTER.arch) {      // ROSTER-GATE arch (b)
                        dirty[PopFamily::ARCH] = FAMILY_DISPATCH[PopFamily::ARCH].prepare_mesh(&machine_ctx_, queue);
                        anyDirty = anyDirty || dirty[PopFamily::ARCH];
                    }
                    if constexpr (ROSTER.column) {    // ROSTER-GATE column (b)
                        dirty[PopFamily::COLUMN] = FAMILY_DISPATCH[PopFamily::COLUMN].prepare_mesh(&machine_ctx_, queue);
                        anyDirty = anyDirty || dirty[PopFamily::COLUMN];
                    }
                    if constexpr (ROSTER.antenna) {   // ROSTER-GATE antenna (b)
                        dirty[PopFamily::ANTENNA] = FAMILY_DISPATCH[PopFamily::ANTENNA].prepare_mesh(&machine_ctx_, queue);
                        anyDirty = anyDirty || dirty[PopFamily::ANTENNA];
                    }
                    if constexpr (ROSTER.palm) {      // ROSTER-GATE palm (b)
                        dirty[PopFamily::PALM] = FAMILY_DISPATCH[PopFamily::PALM].prepare_mesh(&machine_ctx_, queue);
                        anyDirty = anyDirty || dirty[PopFamily::PALM];
                    }
                    if constexpr (ROSTER.cactus) {    // ROSTER-GATE cactus (b)
                        dirty[PopFamily::CACTUS] = FAMILY_DISPATCH[PopFamily::CACTUS].prepare_mesh(&machine_ctx_, queue);
                        anyDirty = anyDirty || dirty[PopFamily::CACTUS];
                    }
                    if constexpr (ROSTER.blade) {     // ROSTER-GATE blade (b)
                        dirty[PopFamily::BLADE] = FAMILY_DISPATCH[PopFamily::BLADE].prepare_mesh(&machine_ctx_, queue);
                        anyDirty = anyDirty || dirty[PopFamily::BLADE];
                    }
                    if constexpr (ROSTER.sphere) {    // ROSTER-GATE sphere (b)
                        dirty[PopFamily::SPHERE] = FAMILY_DISPATCH[PopFamily::SPHERE].prepare_mesh(&machine_ctx_, queue);
                        anyDirty = anyDirty || dirty[PopFamily::SPHERE];
                    }
                    if constexpr (ROSTER.ribbon) {    // ROSTER-GATE ribbon (b)
                        dirty[PopFamily::RIBBON] = FAMILY_DISPATCH[PopFamily::RIBBON].prepare_mesh(&machine_ctx_, queue);
                        anyDirty = anyDirty || dirty[PopFamily::RIBBON];
                    }
                    if constexpr (ROSTER.cube) {      // ROSTER-GATE cube (b)
                        dirty[PopFamily::CUBE] = FAMILY_DISPATCH[PopFamily::CUBE].prepare_mesh(&machine_ctx_, queue);
                        anyDirty = anyDirty || dirty[PopFamily::CUBE];
                    }
                    if constexpr (ROSTER.gol) {       // ROSTER-GATE gol (b)
                        dirty[PopFamily::GOL] = FAMILY_DISPATCH[PopFamily::GOL].prepare_mesh(&machine_ctx_, queue);
                        anyDirty = anyDirty || dirty[PopFamily::GOL];
                    }
                    if constexpr (ROSTER.gallery) {   // ROSTER-GATE gallery (b)
                        dirty[PopFamily::GALLERY] = FAMILY_DISPATCH[PopFamily::GALLERY].prepare_mesh(&machine_ctx_, queue);
                        anyDirty = anyDirty || dirty[PopFamily::GALLERY];
                    }
                    if (anyDirty) {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "Entity Mesh Gen";
                        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                        // dispatch skips disabled families structurally:
                        // dirty[f] stays false for a disabled family (never
                        // set above), so this branches on dirty-ness, not on
                        // the enable bit.
                        for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
                            if (dirty[f]) FAMILY_DISPATCH[f].dispatch_mesh(&machine_ctx_, pass);
                        }
                        pass.End();
                    }
                }
                upload_portal_array(this, queue);
                upload_lights(this, queue);

                // The SNAP-1 sky resync lives at the ribbon tick's tail now
                // (m6, Option A) — O-1 by construction: tick above,
                // dispatch below, queue writes in submission order.
                dispatch_compute(this, encoder);

                // ═══ MOVEMENT: WITNESS — CAPTURE (O-2: staging copies AFTER
                // compute; feeds next frame's HARVEST) ══════════════════════
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

                // ═══ MOVEMENT: REALIZATION, CONTINUED ═══════════════════════
                // GoL zone compute — derive params + sync + evolve, owner
                // verbs, SEPARATE passes for the barrier (O-6a; stray (6)
                // home). ROSTER-GATE gol (b) — D7 (REBUILD-0 stamp): the
                // structural gate above the runtime zone_count gate;
                // behavior-identical per the residue proof (zone_count stays
                // 0 when disabled), and the census tool stays exception-free.
                if constexpr (ROSTER.gol)
                if (gol_state_.zone_count > 0) {
                    rosterGolZoneRuns_++;  // ROSTER-RESIDUE gol (2e) — the only writer of the zone GPU buffers; counted so the disabled-piece residue check can prove pristine
                    flush_zone_derive_requests(gol_state_, this, queue);
                    upload_gol_zone_config(gol_state_, this, queue);
                    dispatch_zone_sync(gol_state_, this, encoder);
                    dispatch_zone_evolve(gol_state_, this, encoder);
                    dispatch_zone_mesh(gol_state_, this, encoder);
                }

                // Pawn aura compute — persistent terrain influence, one owner
                // verb (REBUILD-0 m2 — stray (2) home; the runtime while-
                // presence/clearing condition lives inside).
                // ROSTER-GATE pawn_aura (b) — disabled: the whole aura compute
                // (config upload + dispatch) is eliminated; zero GPU writes.
                if constexpr (ROSTER.pawn_aura)
                    dispatch_pawn_aura(pawn_state_, &pawn_deps_, encoder, queue);

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

                // O-7 tail: cull before the draw passes (indirect draws
                // consume the cull output); snapshot before promotions.
                // DIAG: frustum cull re-enabled — indirect draw active
                dispatch_frustum_cull(this, encoder, queue);

                render_shadow_pass(this, encoder);
                render_main_pass(this, encoder, backbuffer, depth);
                render_snapshot_pass(gallery_state_, this, encoder);

                // Promotion drain, one owner verb (REBUILD-0 m2 — stray (5)
                // home). ROSTER-GATE gallery+indoor_shell (b) — with both off,
                // no writer of pending_promotions exists.
                if constexpr (ROSTER.gallery || ROSTER.indoor_shell)
                    drain_gallery_promotions(gallery_state_, this, encoder);
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
#include "bodies/entities.inl"   // the six preparers + the seven grounded-family evictors + the blade/palm/cactus recipes
#include "bodies/orbs.inl"       // orb lifecycle/commands/dispatches/render
#include "bodies/gol_zones.inl"  // GoL three-phase lifecycle + per-frame uploads/dispatch
#include "bodies/agents.inl"     // agent registry upload + spawn/respawn/possession/diagnostics
#include "bodies/cube_behaviors.inl"  // cube registry upload + corral/kite/coordination + clear + evictor + recipe
#include "bodies/gallery.inl"    // photographer + gallery sites + authored loading + wall paintings
#include "bodies/ribbon.inl"     // author seats + head laws + frame conductor + three-phase lifecycle
#include "direction/input.inl"      // key/mouse dispatch + movement intent + camera commands (own GLFW include)
#include "realization/render_passes.inl"  // ground-entry prep + compute dispatch + shadow/main passes + light VPs
#include "direction/mood.inl"       // indoor light derivation + appliers + apply_mood + shell + portals + uploads + transition request + derivers
#include "machine/spawn_engine.inl"  // the spawn engine — negotiation + footprints + culling + census + the select/place/commit loops
#include "surface/patch_system.inl"  // the active-patch machine — registry lifecycle + budgets + teardown + allocator + the streaming conductor
#include "machine/entity_pipeline.inl"  // the generic three-phase verbs + the welded four family blocks (column/antenna/pyramid/arch)
// ═══ THE TABLE — FAMILY_DISPATCH (DISSOLVE-1 Batch A, A1) ══════════
// Inlined from machine/family_dispatch.inl (retired): the definition
// is SEAM[spine:owns] spine work — it takes the Cartridge mesh-wrapper
// static ADDRESSES and the family row addresses, so it lives with its
// owner, the composition root, at the post-class point where its
// include sat. No struct, no pair — the file simply comes home.
namespace t7 {
namespace the_board {
// ─── Shared no-op adapters ────────────────────────────────────────

inline bool dispatch_prepare_mesh_none(MachineCtx* self, wgpu::Queue& queue) {
    (void)self; (void)queue;
    return false;
}
inline void dispatch_mesh_gen_none(MachineCtx* self, wgpu::ComputePassEncoder& pass) {
    (void)self; (void)pass;
}

// ─── The table ─────────────────────────────────────────────────────

inline const FamilyDispatch FAMILY_DISPATCH[PopFamily::COUNT] = {
    { dispatch_select_pyramid_generic, dispatch_place_pyramid_generic, dispatch_commit_pyramid_generic,
      evict_pyramid, Cartridge::dispatch_prepare_mesh_pyramid, Cartridge::dispatch_mesh_gen_pyramid,
      "pyr" },
    { dispatch_select_arch_generic, dispatch_place_arch_generic, dispatch_commit_arch_generic,
      evict_arch,    Cartridge::dispatch_prepare_mesh_arch,    Cartridge::dispatch_mesh_gen_arch,
      "arch" },
    { dispatch_select_column_generic, dispatch_place_column_generic, dispatch_commit_column_generic,
      evict_column,  Cartridge::dispatch_prepare_mesh_column,  Cartridge::dispatch_mesh_gen_column,
      "col" },
    { dispatch_select_antenna_generic, dispatch_place_antenna_generic, dispatch_commit_antenna_generic,
      evict_antenna, Cartridge::dispatch_prepare_mesh_column,  Cartridge::dispatch_mesh_gen_column,
      "ant" },
    { dispatch_select_palm_generic, dispatch_place_palm_generic, dispatch_commit_palm_generic,
      evict_palm,   Cartridge::dispatch_prepare_mesh_palm,   Cartridge::dispatch_mesh_gen_palm,
      "palm" },
    { dispatch_select_cactus_generic, dispatch_place_cactus_generic, dispatch_commit_cactus_generic,
      evict_cactus, Cartridge::dispatch_prepare_mesh_cactus, Cartridge::dispatch_mesh_gen_cactus,
      "cact" },
    { dispatch_select_blade_generic, dispatch_place_blade_generic, dispatch_commit_blade_generic,
      evict_blade, Cartridge::dispatch_prepare_mesh_blade, Cartridge::dispatch_mesh_gen_blade,
      "blad" },
    { dispatch_select_sphere_generic, dispatch_place_sphere_generic, dispatch_commit_sphere_generic,
      evict_sphere, dispatch_prepare_mesh_none, dispatch_mesh_gen_none,
      "sph" },   // no CPU mesh gen — GPU compute handles update_sphere
    { dispatch_select_ribbon, dispatch_place_ribbon, dispatch_commit_ribbon,
      evict_ribbon, dispatch_prepare_mesh_none, dispatch_mesh_gen_none,
      "ribn" },  // no CPU mesh gen — GPU compute handles ribbon rendering
    { dispatch_select_cube_generic, dispatch_place_cube_generic, dispatch_commit_cube_generic,
      evict_cube, dispatch_prepare_mesh_none, dispatch_mesh_gen_none,
      "cube" },  // no CPU mesh gen — GPU compute handles update_cube
    { dispatch_select_gol, dispatch_place_gol, dispatch_commit_gol,
      evict_gol, dispatch_prepare_mesh_none, dispatch_mesh_gen_none,
      "gol" },   // zone mesh gen is a separate compute pass
    { dispatch_select_gallery, dispatch_place_gallery, dispatch_commit_gallery,
      evict_gallery, dispatch_prepare_mesh_none, dispatch_mesh_gen_none,
      "gall" },
};
} // namespace the_board
} // namespace t7
