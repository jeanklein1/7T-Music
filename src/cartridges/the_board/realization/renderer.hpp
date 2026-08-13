#pragma once

// THE_BOARD CARTRIDGE -- Renderer (Rasterized)
// ==================================================

#include "cartridges/the_board/realization/state.hpp"
#include <webgpu/webgpu_cpp.h>
#include <string>
#include <fstream>
#include <sstream>
#include <array>
#include <chrono>
#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>
#include <iomanip>

namespace t7 {
    namespace the_board {

        namespace Entry {
            // Compute — split world update (ordered by dependency)
            constexpr const char* UPDATE_PLAYER_AGENT = "update_player_agent";        // 0D (1 thread, possessed slot)
            constexpr const char* UPDATE_OTHER_AGENTS = "update_other_agents";        // 1D (32 threads, non-possessed slots)
            constexpr const char* UPDATE_CAMERA = "update_camera";                  // 0D
            constexpr const char* UPDATE_SPHERE = "update_sphere";                  // 0D
            constexpr const char* UPDATE_CUBE = "update_cube";                      // 0D
            constexpr const char* COMPUTE_VP = "compute_vp";                    // 0D

            // On-demand compute
            constexpr const char* GENERATE_PATCH_HEIGHTS = "generate_patch_heights";          // 2D -- per-patch, pass 1
            constexpr const char* GENERATE_PATCH_GRADIENTS = "generate_patch_gradients";      // 2D -- per-patch, pass 2
            constexpr const char* GENERATE_PATCH_CELLS = "generate_patch_cells";              // 2D -- per-patch
            constexpr const char* COMPUTE_RIBBON_RINGS = "compute_ribbon_rings";              // 1D -- per ring
            constexpr const char* COMPUTE_PAWN_AURA = "compute_pawn_aura";                  // 2D -- toroidal grid
            constexpr const char* WRITE_LIVE_CARD_HEIGHTS = "write_live_card_heights";      // 2D -- card pass 1 (TRUEBAND_CONTACT_1)
            constexpr const char* WRITE_LIVE_CARD_RESOLVE = "write_live_card_resolve";      // 2D -- card pass 2 (gradients + store)
            constexpr const char* ZONE_SEED_MASK = "zone_seed_mask";                        // 2D -- the vocabulary mask (UNIFIED_GROUND_1)

            // Render
            constexpr const char* PATCH_TERRAIN_VS = "patch_terrain_vs";
            constexpr const char* PATCH_TERRAIN_FS = "patch_terrain_fs";
            constexpr const char* SHADOW_PATCH_TERRAIN_VS = "shadow_patch_terrain_vs";
            constexpr const char* PAWN_VS = "pawn_vs";
            constexpr const char* SPHERE_VS = "sphere_vs";
            constexpr const char* MONOLITH_VS = "monolith_vs";
            constexpr const char* ENTITY_FS = "entity_fs";
            constexpr const char* RIBBON_FS = "ribbon_fs";   // the veil's ruled exemption: entity shading, veil_scale 0
            constexpr const char* SHADOW_PAWN_VS = "shadow_pawn_vs";
            constexpr const char* SHADOW_SPHERE_VS = "shadow_sphere_vs";
            constexpr const char* SHADOW_MONOLITH_VS = "shadow_monolith_vs";
            constexpr const char* RIBBON_VS = "ribbon_vs";
            constexpr const char* SHADOW_RIBBON_VS = "shadow_ribbon_vs";
            constexpr const char* ARCH_VS = "arch_vs";
            constexpr const char* SHADOW_ARCH_VS = "shadow_arch_vs";
            constexpr const char* COLUMN_VS = "column_vs";
            constexpr const char* SHADOW_COLUMN_VS = "shadow_column_vs";
            // PYRAMID_VS / SHADOW_PYRAMID_VS CUT — pyramid mesh never drawn
            constexpr const char* SHELL_VS = "shell_vs";
            constexpr const char* SHADOW_SHELL_VS = "shadow_shell_vs";

            // Gallery (self-portrait painting frames)
            constexpr const char* GALLERY_FRAME_VS = "gallery_frame_vs";
            constexpr const char* GALLERY_FRAME_FS = "gallery_frame_fs";
            constexpr const char* SHADOW_GALLERY_FRAME_VS = "shadow_gallery_frame_vs";

            // Wall-mounted framed paintings (indoor)
            constexpr const char* WALL_PAINTING_VS        = "wall_painting_vs";
            constexpr const char* WALL_PAINTING_CANVAS_FS = "wall_painting_canvas_fs";
            constexpr const char* WALL_PAINTING_FRAME_FS  = "wall_painting_frame_fs";
            constexpr const char* SHADOW_WALL_PAINTING_VS = "shadow_wall_painting_vs";

            // Photographer compute (GPU-coupled snapshot camera)
            constexpr const char* COMPUTE_PHOTOGRAPHER_VP = "compute_photographer_vp";

            // Entity placement Y-correction (decoupled from photographer)
            constexpr const char* COMPUTE_ENTITY_PLACEMENT = "compute_entity_placement";

            // GPU frustum culling (every frame, after compute_vp)
            constexpr const char* FRUSTUM_CULL_PATCHES = "frustum_cull_patches";

            // GoL zone compute (zone-local automaton)
            constexpr const char* ZONE_GOL_SYNC = "zone_gol_sync";
            constexpr const char* ZONE_GOL_EVOLVE = "zone_gol_evolve";
            constexpr const char* ZONE_DERIVE_PARAMS = "zone_derive_params";

            // GPU Entity Mesh Gen (Phase 2: Arches, Phase 3: Columns — pyramid mesh-gen CUT)
            constexpr const char* ARCH_MESH_GEN = "arch_mesh_gen";
            constexpr const char* COLUMN_MESH_GEN = "column_mesh_gen";
            constexpr const char* PALM_MESH_GEN = "palm_mesh_gen";
            constexpr const char* PALM_VS = "palm_vs";
            constexpr const char* SHADOW_PALM_VS = "shadow_palm_vs";
            constexpr const char* CACTUS_MESH_GEN = "cactus_mesh_gen";
            constexpr const char* CACTUS_VS = "cactus_vs";
            constexpr const char* SHADOW_CACTUS_VS = "shadow_cactus_vs";
            constexpr const char* BLADE_MESH_GEN = "blade_cluster_mesh_gen";
            constexpr const char* BLADE_VS = "blade_cluster_vs";
            constexpr const char* SHADOW_BLADE_VS = "shadow_blade_cluster_vs";

            // Fade overlay (fullscreen transition)
            constexpr const char* FADE_OVERLAY_VS = "fade_overlay_vs";
            constexpr const char* FADE_OVERLAY_FS = "fade_overlay_fs";

            // Orb sky layer (luminous points on a dome)
            constexpr const char* ORB_INIT           = "orb_init";             // 1D compute
            constexpr const char* ORB_DYNAMICS       = "orb_dynamics";         // 1D compute
            constexpr const char* ORB_RECOLOR        = "orb_recolor";          // 1D compute
            constexpr const char* ORB_STATE_PREV_COPY = "orb_state_prev_copy"; // 1D compute (Pass 9)
            constexpr const char* ORB_VS             = "orb_vs";
            constexpr const char* ORB_FS             = "orb_fs";
        }

        class Renderer {

            wgpu::Device device_;
            wgpu::BindGroupLayout computeEntityLayout_;
            wgpu::BindGroupLayout computeTextureLayout_;   // Group 1 for live-contributor compute (sphere/cube)
            wgpu::BindGroupLayout roomLayout_;    // Group 2, agent + floater kernels — THE ROOM
            wgpu::BindGroupLayout patchGenLayout_;
            wgpu::BindGroupLayout renderEntityLayout_;
            // ATLAS_1revB D3" — the offset every non-shadow set of the
            // render-entity group passes. A dynamic-offset layout requires an
            // offset at EVERY set; only the shadow tile loop varies it.
            static constexpr uint32_t kShadowSlotZero = 0;
            wgpu::BindGroupLayout renderTextureLayout_;
            wgpu::BindGroupLayout shadowTextureLayout_;
            wgpu::BindGroupLayout ribbonComputeLayout_;
            wgpu::BindGroupLayout galleryEntityLayout_;
            wgpu::BindGroupLayout galleryTextureLayout_;
            wgpu::BindGroupLayout meshGenEntityLayout_;  // binding 1 only (config) — reused by fade overlay
            wgpu::BindGroupLayout photographerComputeLayout_;
            wgpu::BindGroupLayout pawnAuraComputeLayout_;
            wgpu::BindGroupLayout liveCardWriterLayout_;   // GROUND_CARD_1
            wgpu::BindGroupLayout zoneMaskLayout_;         // UNIFIED_GROUND_1 U5
            wgpu::BindGroupLayout zoneGolComputeLayout_;
            wgpu::BindGroupLayout archMeshGenLayout_;
            wgpu::BindGroupLayout columnMeshGenLayout_;
            wgpu::BindGroupLayout palmMeshGenLayout_;
            wgpu::BindGroupLayout cactusMeshGenLayout_;
            wgpu::BindGroupLayout bladeMeshGenLayout_;
            wgpu::TextureFormat colorFormat_;
            wgpu::TextureFormat depthFormat_;

            wgpu::ShaderModule shaderModule_;
            std::string shaderSource_;
            std::string shaderPath_;

            struct PipelineTiming { std::string label; long long ms; };
            std::vector<PipelineTiming> pipelineTimings_;

            template <typename F>
            bool tPipe(const char* label, F&& fn) {
                auto t0 = std::chrono::high_resolution_clock::now();
                bool ok = fn();
                auto t1 = std::chrono::high_resolution_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
                std::cout << "  [Pipeline] " << label << ": " << ms << " ms\n";
                pipelineTimings_.push_back({label, ms});
                return ok;
            }

            // THE SHARED BUILDERS (cable management): the two collapses every compute pipeline shared.
            // computeLayoutFor wraps a single bind-group layout into a pipeline layout
            // (the ~24 dedicated compute pipelines each repeated this 4-line boilerplate).
            // makeComputePipeline is the uniform creation ALL 30 compute pipelines shared —
            // a pure (entry-point, pipeline-layout) pair over the one shaderModule_; the
            // descriptor carried no other per-pipeline state. The FORKS stay at the call
            // site: which layout, which entry string (passed VERBATIM — the sole C++->shader
            // link), which member, and the ROSTER gate. (The layout-build moves just outside
            // the tPipe timing block; the boot-leaderboard ms now excludes the trivial layout
            // creation — no behavior/pixel effect.)
            wgpu::PipelineLayout computeLayoutFor(wgpu::BindGroupLayout bgl) {
                std::array<wgpu::BindGroupLayout, 1> a = { bgl };
                wgpu::PipelineLayoutDescriptor d{};
                d.bindGroupLayoutCount = a.size();
                d.bindGroupLayouts = a.data();
                return device_.CreatePipelineLayout(&d);
            }
            bool makeComputePipeline(const char* label, const char* dbgLabel,
                                     wgpu::PipelineLayout layout, const char* entry,
                                     wgpu::ComputePipeline& out) {
                return tPipe(label, [&]() {
                    wgpu::ComputePipelineDescriptor desc{};
                    desc.label = dbgLabel;
                    desc.layout = layout;
                    desc.compute.module = shaderModule_;
                    desc.compute.entryPoint = entry;
                    out = device_.CreateComputePipeline(&desc);
                    return out != nullptr;
                });
            }

            // Compute pipelines -- per-frame (split world update)
            wgpu::ComputePipeline updatePlayerAgentPipeline_;    // 0D (1 thread, possessed slot)
            wgpu::ComputePipeline updateOtherAgentsPipeline_;    // 1D (32 threads, non-possessed)
            wgpu::ComputePipeline updateCameraPipeline_;         // 0D
            wgpu::ComputePipeline updateSpherePipeline_;         // 0D
            wgpu::ComputePipeline updateCubePipeline_;           // 0D
            wgpu::ComputePipeline computeVPPipeline_;         // 0D

            // Compute pipelines -- patch heightfield generation (per-patch, two-pass)
            wgpu::ComputePipeline generatePatchHeightsPipeline_;     // 2D -- pass 1: heights only
            wgpu::ComputePipeline generatePatchGradientsPipeline_;   // 2D -- pass 2: gradients + complexity
            wgpu::ComputePipeline generatePatchCellsPipeline_;        // 2D
            wgpu::ComputePipeline ribbonRingPipeline_;                    // 1D -- ribbon ring transforms

            // Render pipelines
            wgpu::RenderPipeline pawnPipeline_;          // Chess pawn entity
            wgpu::RenderPipeline spherePipeline_;        // Sphere entity
            wgpu::RenderPipeline monolithPipeline_;      // Monolith entity
            wgpu::RenderPipeline ribbonPipeline_;        // Sky ribbon entity
            wgpu::RenderPipeline archPipeline_;          // Catenary arch entity
            wgpu::RenderPipeline columnPipeline_;        // Generative column entity
            wgpu::RenderPipeline palmPipeline_;          // Palm tree entity
            wgpu::RenderPipeline cactusPipeline_;         // Cactus entity
            wgpu::RenderPipeline bladePipeline_;          // Blade cluster entity
            // pyramidPipeline_ CUT — pyramid mesh never drawn
            wgpu::RenderPipeline shellPipeline_;         // Indoor shell (ceiling + walls)

            // Shadow pass pipelines (depth-only, no fragment shader)
            wgpu::RenderPipeline shadowPawnPipeline_;
            wgpu::RenderPipeline shadowSpherePipeline_;
            wgpu::RenderPipeline shadowMonolithPipeline_;
            wgpu::RenderPipeline shadowRibbonPipeline_;
            wgpu::RenderPipeline shadowArchPipeline_;
            wgpu::RenderPipeline shadowColumnPipeline_;
            wgpu::RenderPipeline shadowPalmPipeline_;
            wgpu::RenderPipeline shadowCactusPipeline_;
            wgpu::RenderPipeline shadowBladePipeline_;
            // shadowPyramidPipeline_ CUT
            wgpu::RenderPipeline shadowShellPipeline_;

            // Patch terrain pipelines (instanced rendering)
            wgpu::RenderPipeline patchTerrainPipeline_;          // direct draw; all FS features compiled in
            wgpu::RenderPipeline patchTerrainIndirectPipeline_;  // same + USE_PATCH_INDIRECTION=true (for frustum cull)
            bool useIndirectTerrainPipeline_ = false;
            wgpu::RenderPipeline shadowPatchTerrainPipeline_;

            // Gallery frame pipeline (painting quads in the world)
            wgpu::RenderPipeline galleryFramePipeline_;
            wgpu::RenderPipeline shadowGalleryFramePipeline_;

            // Wall-mounted framed paintings (indoor) — uses galleryEntity + galleryTexture layouts
            wgpu::RenderPipeline wallPaintingCanvasPipeline_;
            wgpu::RenderPipeline wallPaintingFramePipeline_;
            wgpu::RenderPipeline shadowWallPaintingPipeline_;

            // Photographer VP compute pipeline (0D, GPU-coupled camera)
            wgpu::ComputePipeline photographerVPPipeline_;
            // Entity placement Y-correction pipeline (0D, decoupled from photographer)
            wgpu::ComputePipeline entityPlacementPipeline_;
            wgpu::ComputePipeline frustumCullPipeline_;
            wgpu::BindGroupLayout frustumCullLayout_;
            wgpu::BindGroupLayout entityPlacementComputeLayout_;
            wgpu::ComputePipeline pawnAuraPipeline_;
            wgpu::ComputePipeline liveCardHeightsPipeline_;  // TRUEBAND_CONTACT_1 (two-pass writer)
            wgpu::ComputePipeline liveCardResolvePipeline_;
            wgpu::ComputePipeline zoneSeedMaskPipeline_;     // UNIFIED_GROUND_1 U5

            // Orb sky layer pipelines
            wgpu::BindGroupLayout orbComputeLayout_;
            wgpu::BindGroupLayout orbCopyLayout_;
            wgpu::ComputePipeline orbInitPipeline_;
            wgpu::ComputePipeline orbDynamicsPipeline_;
            wgpu::ComputePipeline orbRecolorPipeline_;
            wgpu::ComputePipeline orbCopyPrevPipeline_;
            wgpu::RenderPipeline  orbRenderPipeline_;

            // GoL zone compute pipelines (dedicated layout, z-dispatched per zone)
            // ZONE_GRID_WG: workgroups per axis for the three 8×8 zone kernels
            // (zone_gol_sync / zone_gol_evolve / zone_seed_mask). DERIVED from
            // the capacity constant — the one-spelling law. Kernel-side the
            // bound is the zone's own grid_size, so this over-dispatches a
            // sub-32 tier and the guard retires the excess threads.
            static constexpr uint32_t ZONE_GRID_WG = (Dim::GOL_ZONE_GRID + 7u) / 8u;
            wgpu::ComputePipeline zoneGolSyncPipeline_;
            wgpu::ComputePipeline zoneGolEvolvePipeline_;

            // Zone parameter derivation (shares the GoL compute layout; one
            // workgroup per pending derive request)
            wgpu::ComputePipeline zoneDeriveParamsPipeline_;

            // Fade overlay (fullscreen alpha-blended triangle)
            wgpu::RenderPipeline fadeOverlayPipeline_;

            // GPU entity mesh gen (Phase 2: arches, Phase 3: columns — pyramid mesh-gen CUT)
            wgpu::ComputePipeline archMeshGenPipeline_;
            wgpu::ComputePipeline columnMeshGenPipeline_;
            wgpu::ComputePipeline palmMeshGenPipeline_;
            wgpu::ComputePipeline cactusMeshGenPipeline_;
            wgpu::ComputePipeline bladeMeshGenPipeline_;

        public:

            Renderer() = default;
            Renderer(const Renderer&) = delete;
            Renderer& operator=(const Renderer&) = delete;

            bool init(
                wgpu::Device device,
                const GPUState& gpuState,
                wgpu::TextureFormat colorFormat,
                wgpu::TextureFormat depthFormat
            ) {
                device_ = device;
                computeEntityLayout_ = gpuState.compute_entity_layout();
                computeTextureLayout_ = gpuState.compute_texture_layout();
                roomLayout_ = gpuState.room_layout();
                patchGenLayout_ = gpuState.patch_gen_layout();
                renderEntityLayout_ = gpuState.render_entity_layout();
                renderTextureLayout_ = gpuState.render_texture_layout();
                shadowTextureLayout_ = gpuState.shadow_texture_layout();
                ribbonComputeLayout_ = gpuState.ribbon_compute_layout();
                galleryEntityLayout_ = gpuState.gallery_entity_layout();
                galleryTextureLayout_ = gpuState.gallery_texture_layout();
                meshGenEntityLayout_ = gpuState.mesh_gen_entity_layout();
                photographerComputeLayout_ = gpuState.photographer_compute_layout();
                entityPlacementComputeLayout_ = gpuState.entity_placement_compute_layout();
                frustumCullLayout_ = gpuState.frustum_cull_layout();
                pawnAuraComputeLayout_ = gpuState.pawn_aura_compute_layout();
                liveCardWriterLayout_ = gpuState.live_card_writer_layout();
                zoneMaskLayout_ = gpuState.zone_mask_layout();
                orbComputeLayout_ = gpuState.orb_compute_layout();
                orbCopyLayout_    = gpuState.orb_copy_layout();
                zoneGolComputeLayout_ = gpuState.zone_gol_compute_layout();
                archMeshGenLayout_ = gpuState.arch_mesh_gen_layout();
                columnMeshGenLayout_ = gpuState.column_mesh_gen_layout();
                palmMeshGenLayout_ = gpuState.palm_mesh_gen_layout();
                cactusMeshGenLayout_ = gpuState.cactus_mesh_gen_layout();
                bladeMeshGenLayout_ = gpuState.blade_mesh_gen_layout();
                colorFormat_ = colorFormat;
                depthFormat_ = depthFormat;

                if (!loadShader()) return false;

                auto t0 = std::chrono::high_resolution_clock::now();
                if (!createComputePipelines()) return false;
                auto t1 = std::chrono::high_resolution_clock::now();
                if (!createRenderPipelines()) return false;
                auto t2 = std::chrono::high_resolution_clock::now();

                // Sorted bottleneck leaderboard
                {
                    auto sorted = pipelineTimings_;
                    std::sort(sorted.begin(), sorted.end(),
                        [](const PipelineTiming& a, const PipelineTiming& b) { return a.ms > b.ms; });
                    std::cout << "\n[Renderer] Pipelines by compile time (descending):\n";
                    for (const auto& t : sorted) {
                        std::cout << "  " << std::setw(8) << t.ms << " ms  " << t.label << "\n";
                    }
                    std::cout << "\n";
                }

                std::cout << "[Renderer] Compute pipelines: "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << " ms\n";
                std::cout << "[Renderer] Render pipelines:  "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms\n";
                std::cout << "[Renderer] Total pipelines:   "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t0).count() << " ms\n";

                return true;
            }

            // ═══ OIL_1 U11 (ledger: R10, C7) — THE PASS-HEAD CONTRACT ═══
            // The six kernel helpers below ride binds set ONCE by
            // dispatch_compute at the pass head: group0 = the compute
            // entity group, group1 = the compute texture group, group2 =
            // the room — identical for every kernel (verified: each had
            // exactly one caller, and all passed these same three
            // objects). Bind-group state is sticky within a pass, and
            // WebGPU permits bound groups a pipeline's layout does not
            // use (camera reads 0/1, VP reads 0 — the extra binds are
            // ignored, not faulted). Each helper keeps SetPipeline +
            // dispatch. The ribbon-ring dispatch is NOT under this
            // contract — it binds its own group0 and runs BEFORE the
            // pass-head binds.
            void dispatch_update_player_agent(wgpu::ComputePassEncoder& pass) {
                pass.SetPipeline(updatePlayerAgentPipeline_);
                pass.DispatchWorkgroups(1, 1, 1);         // 1 workgroup × 1 thread = the possessed slot
            }

            void dispatch_update_other_agents(wgpu::ComputePassEncoder& pass) {
                if constexpr (!(ROSTER.wanderers)) return;  // ROSTER-GATE wanderers (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(updateOtherAgentsPipeline_);
                pass.DispatchWorkgroups(1, 1, 1);         // 1 workgroup × 32 threads = all non-player slots
            }

            void dispatch_update_camera(wgpu::ComputePassEncoder& pass) {
                pass.SetPipeline(updateCameraPipeline_);
                pass.DispatchWorkgroups(1, 1, 1);
            }

            void dispatch_update_sphere(wgpu::ComputePassEncoder& pass) {
                if constexpr (!(ROSTER.sphere)) return;  // ROSTER-GATE sphere (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(updateSpherePipeline_);
                pass.DispatchWorkgroups(1, 1, 1);
            }

            void dispatch_update_cube(wgpu::ComputePassEncoder& pass) {
                if constexpr (!(ROSTER.cube)) return;  // ROSTER-GATE cube (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(updateCubePipeline_);
                pass.DispatchWorkgroups(1, 1, 1);
            }

            void dispatch_compute_vp(wgpu::ComputePassEncoder& pass) {
                pass.SetPipeline(computeVPPipeline_);
                pass.DispatchWorkgroups(1, 1, 1);  // 0D: single invocation
            }

            // Pass 1: evaluate ground_formed() per texel, store height only.
            void dispatch_generate_patch_heights(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup patchGenBindGroup,
                uint32_t workgroups
            ) {
                pass.SetPipeline(generatePatchHeightsPipeline_);
                pass.SetBindGroup(0, patchGenBindGroup);
                pass.DispatchWorkgroups(workgroups, workgroups, 1);
            }

            // Pass 2: read stored heights from neighbors, compute gradients + complexity.
            void dispatch_generate_patch_gradients(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup patchGenBindGroup,
                uint32_t workgroups
            ) {
                pass.SetPipeline(generatePatchGradientsPipeline_);
                pass.SetBindGroup(0, patchGenBindGroup);
                pass.DispatchWorkgroups(workgroups, workgroups, 1);
            }

            void dispatch_generate_patch_cells(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup patchGenBindGroup,
                uint32_t workgroups
            ) {
                pass.SetPipeline(generatePatchCellsPipeline_);
                pass.SetBindGroup(0, patchGenBindGroup);
                pass.DispatchWorkgroups(workgroups, workgroups, 1);
            }

            void dispatch_compute_ribbon_rings(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup ribbonComputeBindGroup,
                uint32_t workgroups
            ) {
                if constexpr (!(ROSTER.ribbon)) return;  // ROSTER-GATE ribbon (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(ribbonRingPipeline_);
                pass.SetBindGroup(0, ribbonComputeBindGroup);
                pass.DispatchWorkgroups(workgroups, 1, 1);
            }

            void dispatch_compute_photographer_vp(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup photographerComputeBindGroup
            ) {
                if constexpr (!(ROSTER.gallery)) return;  // ROSTER-GATE gallery (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(photographerVPPipeline_);
                pass.SetBindGroup(0, photographerComputeBindGroup);
                pass.DispatchWorkgroups(1, 1, 1);
            }

            void dispatch_entity_placement(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup entityPlacementBindGroup
            ) {
                pass.SetPipeline(entityPlacementPipeline_);
                pass.SetBindGroup(0, entityPlacementBindGroup);
                pass.DispatchWorkgroups(1, 1, 1);
            }

            void dispatch_frustum_cull(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup frustumCullBindGroup
            ) {
                pass.SetPipeline(frustumCullPipeline_);
                pass.SetBindGroup(0, frustumCullBindGroup);
                // ceil(MAX_ACTIVE_PATCHES / 64) — derived, never hardcoded again
                // (was hardcoded 4 = 256 threads vs 289 slots: slots 256–288 were
                //  never culled at full window — audit CC-8a).
                pass.DispatchWorkgroups((Dim::MAX_ACTIVE_PATCHES + 63u) / 64u, 1, 1);
            }

            void dispatch_compute_pawn_aura(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup auraComputeBindGroup,
                uint32_t workgroups
            ) {
                if constexpr (!(ROSTER.pawn_aura)) return;  // ROSTER-GATE pawn_aura (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(pawnAuraPipeline_);
                pass.SetBindGroup(0, auraComputeBindGroup);
                pass.DispatchWorkgroups(workgroups, workgroups, 1);
            }

            void dispatch_live_card_write(wgpu::ComputePassEncoder& pass,
                                          wgpu::BindGroup group) {
                // Two-pass writer (TRUEBAND_CONTACT_1): heights → scratch,
                // then resolve (gradients + store). Sequential dispatches in
                // ONE pass — storage-buffer visibility between dispatches is
                // guaranteed (the U5a same-pass law).
                pass.SetPipeline(liveCardHeightsPipeline_);
                pass.SetBindGroup(0, group);
                pass.DispatchWorkgroups(Dim::LIVE_CARD_SIZE / 8u,
                                        Dim::LIVE_CARD_SIZE / 8u, 1);
                pass.SetPipeline(liveCardResolvePipeline_);
                pass.DispatchWorkgroups(Dim::LIVE_CARD_SIZE / 16u,
                                        Dim::LIVE_CARD_SIZE / 16u, 1);
            }

            void dispatch_orb_init(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup orbComputeGroup,
                uint32_t workgroups
            ) {
                if constexpr (!(ROSTER.orbs)) return;  // ROSTER-GATE orbs (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(orbInitPipeline_);
                pass.SetBindGroup(0, orbComputeGroup);
                pass.DispatchWorkgroups(workgroups, 1, 1);
            }

            void dispatch_orb_dynamics(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup orbComputeGroup,
                uint32_t workgroups
            ) {
                if constexpr (!(ROSTER.orbs)) return;  // ROSTER-GATE orbs (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(orbDynamicsPipeline_);
                pass.SetBindGroup(0, orbComputeGroup);
                pass.DispatchWorkgroups(workgroups, 1, 1);
            }

            void dispatch_orb_recolor(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup orbComputeGroup,
                uint32_t workgroups
            ) {
                if constexpr (!(ROSTER.orbs)) return;  // ROSTER-GATE orbs (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(orbRecolorPipeline_);
                pass.SetBindGroup(0, orbComputeGroup);
                pass.DispatchWorkgroups(workgroups, 1, 1);
            }

            void dispatch_orb_copy_prev(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup orbCopyGroup,
                uint32_t workgroups
            ) {
                if constexpr (!(ROSTER.orbs)) return;  // ROSTER-GATE orbs (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(orbCopyPrevPipeline_);
                pass.SetBindGroup(0, orbCopyGroup);
                pass.DispatchWorkgroups(workgroups, 1, 1);
            }

            void draw_orbs(
                wgpu::RenderPassEncoder& pass,
                wgpu::BindGroup entityBindGroup,
                wgpu::BindGroup textureBindGroup,
                wgpu::Buffer quadVB,
                wgpu::Buffer quadIB,
                wgpu::Buffer orbStateVB,
                uint32_t orbCount
            ) {
                if constexpr (!(ROSTER.orbs)) return;  // ROSTER-GATE orbs (a') — pipeline never created; the holder tolerates
                if (orbCount == 0) return;
                pass.SetPipeline(orbRenderPipeline_);
                // ATLAS_1revB D3" — group 0 is dynamic-offset now; every set
                // of it supplies one offset. The orbs never vary it.
                pass.SetBindGroup(0, entityBindGroup, 1, &kShadowSlotZero);
                pass.SetBindGroup(1, textureBindGroup);
                pass.SetVertexBuffer(0, quadVB);
                // ORB_V: slot 1 is the instance-step OrbState stream that
                // replaced the binding-400 storage read in orb_vs.
                pass.SetVertexBuffer(1, orbStateVB);
                pass.SetIndexBuffer(quadIB, wgpu::IndexFormat::Uint16);
                pass.DrawIndexed(6, orbCount, 0, 0, 0);
            }

            void dispatch_zone_gol_sync(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup zoneComputeBindGroup,
                uint32_t zone_count
            ) {
                if constexpr (!(ROSTER.gol)) return;  // ROSTER-GATE gol (a') — pipeline never created; the holder tolerates
                if (zone_count == 0) return;
                pass.SetPipeline(zoneGolSyncPipeline_);
                pass.SetBindGroup(0, zoneComputeBindGroup);
                // CAPACITY-shaped dispatch, SIZE-bounded kernel: the grid is
                // derived from Dim::GOL_ZONE_GRID over the 8×8 workgroup (was
                // a hard 4 with a "32/8=4" comment — the one-spelling law).
                // The kernel early-outs on cell >= z.grid_size, so dispatching
                // to capacity is correct for every tier; only the guard costs.
                pass.DispatchWorkgroups(ZONE_GRID_WG, ZONE_GRID_WG, zone_count);
            }

            void dispatch_zone_gol_evolve(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup zoneComputeBindGroup,
                uint32_t zone_count
            ) {
                if constexpr (!(ROSTER.gol)) return;  // ROSTER-GATE gol (a') — pipeline never created; the holder tolerates
                if (zone_count == 0) return;
                pass.SetPipeline(zoneGolEvolvePipeline_);
                pass.SetBindGroup(0, zoneComputeBindGroup);
                pass.DispatchWorkgroups(ZONE_GRID_WG, ZONE_GRID_WG, zone_count);
            }

            // Zone parameter derivation (GPU-authoritative tier selection + Gaussian sampling)
            void dispatch_zone_derive_params(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup zoneGroup,
                uint32_t request_count
            ) {
                if constexpr (!(ROSTER.gol)) return;  // ROSTER-GATE gol (a') — pipeline never created; the holder tolerates
                if (request_count == 0) return;
                pass.SetPipeline(zoneDeriveParamsPipeline_);
                pass.SetBindGroup(0, zoneGroup);
                pass.DispatchWorkgroups(request_count, 1, 1);
            }

            void dispatch_zone_seed_mask(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup maskGroup,
                uint32_t request_count
            ) {
                if constexpr (!(ROSTER.gol)) return;  // ROSTER-GATE gol (a') — pipeline never created; the holder tolerates
                if (request_count == 0) return;
                pass.SetPipeline(zoneSeedMaskPipeline_);
                pass.SetBindGroup(0, maskGroup);
                pass.DispatchWorkgroups(ZONE_GRID_WG, ZONE_GRID_WG, request_count);
            }

            // dispatch_pyramid_mesh_gen CUT — mesh never drawn;
            // the FAMILY_DISPATCH pyramid mesh hook now routes to the none-fork.

            // GPU arch mesh gen — generates all 16 slots × 4 sub-meshes.
            // gid.x = slot (0..15), gid.y = sub-mesh (outer shell, inner shell, front cap, back cap).
            void dispatch_arch_mesh_gen(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup meshGenGroup
            ) {
                if constexpr (!(ROSTER.arch)) return;  // ROSTER-GATE arch (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(archMeshGenPipeline_);
                pass.SetBindGroup(0, meshGenGroup);
                pass.DispatchWorkgroups(Dim::MAX_ARCH_INSTANCES, 4, 1);
            }

            // GPU column mesh gen — generates all 32 slots in one dispatch.
            void dispatch_column_mesh_gen(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup meshGenGroup
            ) {
                if constexpr (!(ROSTER.column || ROSTER.antenna)) return;  // ROSTER-GATE column+antenna (shared pipelines) (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(columnMeshGenPipeline_);
                pass.SetBindGroup(0, meshGenGroup);
                pass.DispatchWorkgroups(Dim::MAX_COLUMN_INSTANCES, 1, 1);
            }

            void dispatch_palm_mesh_gen(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup meshGenGroup
            ) {
                if constexpr (!(ROSTER.palm)) return;  // ROSTER-GATE palm (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(palmMeshGenPipeline_);
                pass.SetBindGroup(0, meshGenGroup);
                pass.DispatchWorkgroups(Dim::MAX_PALM_INSTANCES, 1, 1);
            }

            void dispatch_cactus_mesh_gen(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup meshGenGroup
            ) {
                if constexpr (!(ROSTER.cactus)) return;  // ROSTER-GATE cactus (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(cactusMeshGenPipeline_);
                pass.SetBindGroup(0, meshGenGroup);
                pass.DispatchWorkgroups(Dim::MAX_CACTUS_INSTANCES, 1, 1);
            }

            void dispatch_blade_mesh_gen(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup group
            ) {
                if constexpr (!(ROSTER.blade)) return;  // ROSTER-GATE blade (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(bladeMeshGenPipeline_);
                pass.SetBindGroup(0, group);
                pass.DispatchWorkgroups(Dim::MAX_BLADE_INSTANCES, 1, 1);
            }

            // THE DRAW PLAN: one helper, three invocations — the entity
            // group carries the list window, the args slot rides the
            // offset (0 / 20 / 40 bytes into the 3 x 5-u32 args buffer).
            // OIL_1 U13 (ledger: R19, C7): the three plan slots shared one
            // pipeline and one texture group and re-set both — the pipeline
            // is now set ONCE by begin_patch_terrain_plan and group1 rides
            // the pass head. Group 0 stays per-slot: the plan A/B/C windows
            // ARE three different bind groups, the one genuinely varying
            // piece of state here.
            void begin_patch_terrain_plan(wgpu::RenderPassEncoder& pass) {
                pass.SetPipeline(patchTerrainIndirectPipeline_);
            }
            void draw_patch_terrain_plan_slot(
                wgpu::RenderPassEncoder& pass,
                wgpu::BindGroup entityBindGroup,
                wgpu::Buffer indexBuffer,
                wgpu::Buffer indirectArgs,
                uint64_t indirectOffset
            ) {
                pass.SetBindGroup(0, entityBindGroup, 1, &kShadowSlotZero);
                pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
                pass.DrawIndexedIndirect(indirectArgs, indirectOffset);
            }

            // Direct terrain draw — uses non-indirect pipeline (outdoor or indoor variant).
            // For LOD1 outdoor, LOD0+LOD1 indoor, snapshot pass, etc.
            void draw_patch_terrain_direct(
                wgpu::RenderPassEncoder& pass,
                wgpu::BindGroup entityBindGroup,
                wgpu::BindGroup textureBindGroup,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount,
                uint32_t instanceCount,
                uint32_t firstInstance = 0
            ) {
                pass.SetPipeline(patchTerrainPipeline_);
                pass.SetBindGroup(0, entityBindGroup, 1, &kShadowSlotZero);
                pass.SetBindGroup(1, textureBindGroup);
                pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
                pass.DrawIndexed(indexCount, instanceCount, 0, 0, firstInstance);
            }

            // STATUS: LATENT[mood_cull_opt_out] — the flag is WRITTEN every
            // mood change (mood.hpp apply_mood, from MoodProfile::
            // allow_frustum_cull) and READ BY NOBODY. Its one reader was
            // render_passes.hpp's `if (!use_indirect_terrain()) return;`
            // early-out, retired with the direct indoor path when the draw
            // plan took every mood ("the kernel runs in EVERY mood now",
            // dispatch_frustum_cull). So the two indoor MOOD_TABLE rows say
            // allow_frustum_cull = false and their terrain is culled anyway
            // — the table reads as a knob and is not one. Found by OPT_1's
            // O0-f recensus; the cut (this pair, the MoodProfile column, its
            // two drift asserts, and the apply_mood poke) is a positional-
            // table edit and wants a build, so it is Jean's ruling, not a
            // residue sweep's. Revive-or-rewire when this region is worked.
            void set_frustum_cull_active(bool active) { useIndirectTerrainPipeline_ = active; }
            bool use_indirect_terrain() const { return useIndirectTerrainPipeline_; }

            // ═══ OIL_1 U13 (ledger: R19, C7) — THE COLOR PASS-HEAD CONTRACT
            // The entity draw helpers below ride group0 (the pass's entity
            // window — render_entity_group in the main pass, the
            // photographer's in the snapshot pass) and group1 (the render
            // texture group) bound ONCE by the caller before draw_table.
            // They were identical at every call within a pass and every
            // helper re-set them. All these pipelines share ONE layout
            // (renderLayout), so every draw sees the groups it saw before.
            // The gallery draws keep their own layout and bind their own
            // pair, once, at their call site.

            // Shared helper for all "indexed mesh" COLOR draws — the twin
            // of draw_shadow_indexed_mesh below, same signature and same
            // body. Per-family wrappers differ only in pipeline and
            // (rarely) instance count / first instance.
            //
            // TIDY_0d: `indexCount == 0` returns before SetPipeline. A
            // species whose mesh is empty draws nothing either way — the
            // guard changes no pixel — but Dawn logs "Draw with an index
            // count of 0 is unusual" for the submitted zero-count draw,
            // and on the web twin that warning reaches the audience's
            // browser console. Every species submits its high-water prefix
            // rather than a live count, so a family with nothing live still
            // arrives here with 0. Replacing the prefix with a live count
            // is the ARENA-era fix; this only stops the warning.
            void draw_indexed_mesh(
                wgpu::RenderPassEncoder& pass,
                wgpu::RenderPipeline pipeline,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount,
                uint32_t instanceCount = 1,
                uint32_t firstInstance = 0
            ) {
                if (indexCount == 0) return;
                pass.SetPipeline(pipeline);
                pass.SetVertexBuffer(0, vertexBuffer);
                pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
                pass.DrawIndexed(indexCount, instanceCount, 0, 0, firstInstance);
            }

            void draw_pawn(
                wgpu::RenderPassEncoder& pass,
                uint32_t vertexCount
            ) {
                pass.SetPipeline(pawnPipeline_);
                // One instance per agent slot. Inactive slots collapse via a
                // zero-scale local mesh in pawn_vs (see is_active branch).
                pass.Draw(vertexCount, /*instanceCount=*/ Dim::MAX_AGENTS);
            }

            void draw_sphere(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                if constexpr (!(ROSTER.sphere)) return;  // ROSTER-GATE sphere (a') — pipeline never created; the holder tolerates
                draw_indexed_mesh(pass, spherePipeline_,
                    vertexBuffer, indexBuffer, indexCount,
                    Dim::MAX_SPHERE_INSTANCES);
            }

            void draw_monolith(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                if constexpr (!(ROSTER.cube)) return;  // ROSTER-GATE cube (a') — pipeline never created; the holder tolerates
                draw_indexed_mesh(pass, monolithPipeline_,
                    vertexBuffer, indexBuffer, indexCount,
                    Dim::MAX_CUBE_INSTANCES, Dim::CUBE_SLOT_OFFSET);
            }

            void draw_ribbon(
                wgpu::RenderPassEncoder& pass,
                uint32_t vertexCount
            ) {
                if constexpr (!(ROSTER.ribbon)) return;  // ROSTER-GATE ribbon (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(ribbonPipeline_);
                pass.Draw(vertexCount);
            }

            void draw_arch(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                if constexpr (!(ROSTER.arch)) return;  // ROSTER-GATE arch (a') — pipeline never created; the holder tolerates
                draw_indexed_mesh(pass, archPipeline_,
                    vertexBuffer, indexBuffer, indexCount);
            }

            void draw_column(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                if constexpr (!(ROSTER.column || ROSTER.antenna)) return;  // ROSTER-GATE column+antenna (shared pipelines) (a') — pipeline never created; the holder tolerates
                draw_indexed_mesh(pass, columnPipeline_,
                    vertexBuffer, indexBuffer, indexCount);
            }

            void draw_palm(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                if constexpr (!(ROSTER.palm)) return;  // ROSTER-GATE palm (a') — pipeline never created; the holder tolerates
                draw_indexed_mesh(pass, palmPipeline_,
                    vertexBuffer, indexBuffer, indexCount);
            }

            void draw_cactus(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                if constexpr (!(ROSTER.cactus)) return;  // ROSTER-GATE cactus (a') — pipeline never created; the holder tolerates
                draw_indexed_mesh(pass, cactusPipeline_,
                    vertexBuffer, indexBuffer, indexCount);
            }

            void draw_blade(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                if constexpr (!(ROSTER.blade)) return;  // ROSTER-GATE blade (a') — pipeline never created; the holder tolerates
                draw_indexed_mesh(pass, bladePipeline_,
                    vertexBuffer, indexBuffer, indexCount);
            }

            // draw_pyramid CUT — caller-free; pyramid mesh never drawn

            void draw_shell(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                if constexpr (!(ROSTER.indoor_shell)) return;  // ROSTER-GATE indoor_shell (a') — pipeline never created; the holder tolerates
                // Helper's `if (indexCount == 0) return;` covers the early-out
                // that this wrapper used to carry explicitly (the shadow twin
                // says the same at draw_shadow_shell).
                draw_indexed_mesh(pass, shellPipeline_,
                    vertexBuffer, indexBuffer, indexCount);
            }

            // OIL_1 U13: the gallery pair is bound ONCE by the caller
            // before the wall/frame draws (they share the gallery layout).
            void draw_gallery_frames(
                wgpu::RenderPassEncoder& pass,
                uint32_t activePaintingCount,
                uint32_t slotHighWater          // one past the highest ACTIVE slot
            ) {
                if constexpr (!(ROSTER.gallery)) return;  // ROSTER-GATE gallery (a') — pipeline never created; the holder tolerates
                if (activePaintingCount == 0 || slotHighWater == 0) return;
                pass.SetPipeline(galleryFramePipeline_);
                // Instance count is the live mark, not Dim::PAINTING_MAX_SLOTS.
                // The shader culls per slot either way, so the constant meant
                // paying for the ceiling every frame; gallery_frame_vs's bounds
                // guard (B1) makes drawing fewer safe by construction.
                pass.Draw(Dim::PAINTING_QUAD_VERTS, slotHighWater);
            }

            void draw_wall_paintings(
                wgpu::RenderPassEncoder& pass,
                uint32_t wallFrameCount,
                uint32_t slotHighWater          // one past the highest ACTIVE slot
            ) {
                if constexpr (!(ROSTER.gallery)) return;  // ROSTER-GATE gallery (a') — pipeline never created; the holder tolerates
                if (wallFrameCount == 0 || slotHighWater == 0) return;

                // Both passes walk vid/PAINTING_FRAME_VERTS_PER as a slot index,
                // so the vertex count is the live mark x the per-frame stride
                // rather than Dim::PAINTING_FRAME_VERTEX_COUNT's ceiling.
                // wall_painting_vs already guards the decoded index (world.wgsl).
                const uint32_t verts = slotHighWater * Dim::PAINTING_FRAME_VERTS_PER;

                // Canvas pass (textured surface)
                pass.SetPipeline(wallPaintingCanvasPipeline_);
                pass.Draw(verts);

                // Frame pass (solid color) — same pair, already bound.
                pass.SetPipeline(wallPaintingFramePipeline_);
                pass.Draw(verts);
            }

            void draw_fade_overlay(
                wgpu::RenderPassEncoder& pass,
                wgpu::BindGroup configBindGroup,
                float fadeAlpha
            ) {
                if constexpr (!(ROSTER.transitions)) return;  // ROSTER-GATE transitions (a') — pipeline never created; the holder tolerates
                if (fadeAlpha < 0.001f) return;
                pass.SetPipeline(fadeOverlayPipeline_);
                pass.SetBindGroup(0, configBindGroup);
                pass.Draw(3);  // fullscreen triangle from vertex ID
            }


            // Shared helper for all "indexed mesh" shadow draws. Per-family
            // wrappers below differ only in pipeline + (rarely) instance count.
            // ═══ OIL_1 U12 (ledger: R18, C7) — THE SHADOW PASS-HEAD CONTRACT
            // Every draw_shadow_* below rides binds set ONCE at the pass
            // head by render_shadow_pass: group0 = the render entity
            // group, group1 = the shadow texture group. They were
            // identical at every call (the DrawBind pair), so each
            // helper's own pair was a redundant re-set of unchanged
            // state — ~18-22 per pass, and per atlas tile indoors.
            // Bind-group state is sticky within a pass and all these
            // pipelines share ONE layout (shadowRenderLayout), so every
            // draw sees exactly the groups it saw before. The two
            // gallery draws are NOT under this contract: they carry
            // their own layout and bind their own pair, once, at the
            // tail of draw_shadow_all.
            void draw_shadow_indexed_mesh(
                wgpu::RenderPassEncoder& pass,
                wgpu::RenderPipeline pipeline,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount,
                uint32_t instanceCount = 1,
                uint32_t firstInstance = 0
            ) {
                if (indexCount == 0) return;
                pass.SetPipeline(pipeline);
                pass.SetVertexBuffer(0, vertexBuffer);
                pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
                pass.DrawIndexed(indexCount, instanceCount, 0, 0, firstInstance);
            }

            void draw_shadow_patch_terrain(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount,
                uint32_t instanceCount
            ) {
                pass.SetPipeline(shadowPatchTerrainPipeline_);
                pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
                pass.DrawIndexed(indexCount, instanceCount);
            }

            void draw_shadow_pawn(
                wgpu::RenderPassEncoder& pass,
                uint32_t vertexCount
            ) {
                pass.SetPipeline(shadowPawnPipeline_);
                pass.Draw(vertexCount, /*instanceCount=*/ Dim::MAX_AGENTS);
            }

            void draw_shadow_sphere(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                if constexpr (!(ROSTER.sphere)) return;  // ROSTER-GATE sphere (a') — pipeline never created; the holder tolerates
                draw_shadow_indexed_mesh(pass, shadowSpherePipeline_,
                    vertexBuffer, indexBuffer, indexCount,
                    Dim::MAX_SPHERE_INSTANCES);
            }

            void draw_shadow_monolith(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                if constexpr (!(ROSTER.cube)) return;  // ROSTER-GATE cube (a') — pipeline never created; the holder tolerates
                draw_shadow_indexed_mesh(pass, shadowMonolithPipeline_,
                    vertexBuffer, indexBuffer, indexCount,
                    Dim::MAX_CUBE_INSTANCES, Dim::CUBE_SLOT_OFFSET);
            }

            void draw_shadow_ribbon(
                wgpu::RenderPassEncoder& pass,
                uint32_t vertexCount
            ) {
                if constexpr (!(ROSTER.ribbon)) return;  // ROSTER-GATE ribbon (a') — pipeline never created; the holder tolerates
                pass.SetPipeline(shadowRibbonPipeline_);
                pass.Draw(vertexCount);
            }

            void draw_shadow_arch(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                if constexpr (!(ROSTER.arch)) return;  // ROSTER-GATE arch (a') — pipeline never created; the holder tolerates
                draw_shadow_indexed_mesh(pass, shadowArchPipeline_,
                    vertexBuffer, indexBuffer, indexCount);
            }

            void draw_shadow_column(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                if constexpr (!(ROSTER.column || ROSTER.antenna)) return;  // ROSTER-GATE column+antenna (shared pipelines) (a') — pipeline never created; the holder tolerates
                draw_shadow_indexed_mesh(pass, shadowColumnPipeline_,
                    vertexBuffer, indexBuffer, indexCount);
            }

            void draw_shadow_palm(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                if constexpr (!(ROSTER.palm)) return;  // ROSTER-GATE palm (a') — pipeline never created; the holder tolerates
                draw_shadow_indexed_mesh(pass, shadowPalmPipeline_,
                    vertexBuffer, indexBuffer, indexCount);
            }

            void draw_shadow_cactus(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                if constexpr (!(ROSTER.cactus)) return;  // ROSTER-GATE cactus (a') — pipeline never created; the holder tolerates
                draw_shadow_indexed_mesh(pass, shadowCactusPipeline_,
                    vertexBuffer, indexBuffer, indexCount);
            }

            void draw_shadow_blade(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                if constexpr (!(ROSTER.blade)) return;  // ROSTER-GATE blade (a') — pipeline never created; the holder tolerates
                draw_shadow_indexed_mesh(pass, shadowBladePipeline_,
                    vertexBuffer, indexBuffer, indexCount);
            }

            // draw_shadow_pyramid CUT — caller-free

            void draw_shadow_shell(
                wgpu::RenderPassEncoder& pass,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                if constexpr (!(ROSTER.indoor_shell)) return;  // ROSTER-GATE indoor_shell (a') — pipeline never created; the holder tolerates
                // Helper's `if (indexCount == 0) return;` covers the early-out
                // that the original draw_shadow_shell had explicitly.
                draw_shadow_indexed_mesh(pass, shadowShellPipeline_,
                    vertexBuffer, indexBuffer, indexCount);
            }

            // OIL_1 U12: the gallery pair is bound ONCE by the caller
            // before these two draws (they share galleryShadowLayout and
            // sit at the tail of draw_shadow_all).
            void draw_shadow_gallery_frames(
                wgpu::RenderPassEncoder& pass,
                uint32_t activePaintingCount,
                uint32_t slotHighWater          // one past the highest ACTIVE slot
            ) {
                if constexpr (!(ROSTER.gallery)) return;
                if (activePaintingCount == 0 || slotHighWater == 0) return;
                pass.SetPipeline(shadowGalleryFramePipeline_);
                pass.Draw(Dim::PAINTING_QUAD_VERTS, slotHighWater);
            }

            void draw_shadow_wall_paintings(
                wgpu::RenderPassEncoder& pass,
                uint32_t wallFrameCount,
                uint32_t slotHighWater          // one past the highest ACTIVE slot
            ) {
                if constexpr (!(ROSTER.gallery)) return;
                if (wallFrameCount == 0 || slotHighWater == 0) return;
                // ONE draw where the color pass needs two: with no fragment
                // stage the canvas/frame split has nothing to distinguish.
                pass.SetPipeline(shadowWallPaintingPipeline_);
                pass.Draw(slotHighWater * Dim::PAINTING_FRAME_VERTS_PER);
            }

            // Gate (a'): compile-time count of pipelines the
            // selected demo skips — the boot summary's number.
            static constexpr uint32_t pipelines_skipped() {
                uint32_t n = 0;
                if (!(ROSTER.sphere)) n += 3;
                if (!(ROSTER.cube)) n += 3;
                if (!(ROSTER.ribbon)) n += 3;
                if (!(ROSTER.arch)) n += 3;
                if (!(ROSTER.column || ROSTER.antenna)) n += 3;
                if (!(ROSTER.palm)) n += 3;
                if (!(ROSTER.cactus)) n += 3;
                if (!(ROSTER.blade)) n += 3;
                // pyramid: 0 pipelines (mesh-gen + render + shadow all cut)
                if (!(ROSTER.gol)) n += 7;
                if (!(ROSTER.gallery)) n += 6;
                if (!(ROSTER.orbs)) n += 5;
                if (!(ROSTER.pawn_aura)) n += 1;
                if (!(ROSTER.indoor_shell)) n += 2;
                if (!(ROSTER.wanderers)) n += 1;
                if (!(ROSTER.transitions)) n += 1;
                return n;
            }

            bool reload() {
                if (!loadShader()) return false;
                if (!createComputePipelines()) return false;
                if (!createRenderPipelines()) return false;
                std::cout << "[Hot Reload] Shader reloaded successfully\n";
                return true;
            }

            const std::string& shader_path() const { return shaderPath_; }

        private:

            //
            // Reads world.wgsl from disk by searching a small list of known
            // relative paths (see the search loop below) — next to the executable
            // or one directory up. If the path moves, update the search list
            // rather than relying on cwd. The lookup logic is the contract.

            bool loadShader() {
                // On reload, use the already-known path instead of searching
                if (!shaderPath_.empty()) {
                    std::ifstream file(shaderPath_);
                    if (file.is_open()) {
                        std::stringstream buffer;
                        buffer << file.rdbuf();
                        shaderSource_ = buffer.str();
                        std::cout << "Reloaded shader from: " << shaderPath_ << "\n";
                    }
                    else {
                        std::cerr << "ERROR: Could not reload shader from: " << shaderPath_ << "\n";
                        return false;
                    }
                }
                else {
                    // First load: search for the shader. Array is sized to its
                    // contents — trailing nullptr slots would reach
                    // ifstream(nullptr) (a CRT assert dialog, not a readable
                    // error) exactly when the file is missing.
                    std::array<const char*, 2> paths = {
                        "../../../src/cartridges/the_board/realization/world.wgsl",
                        "src/cartridges/the_board/realization/world.wgsl",
                    };

                    const char* loadedPath = nullptr;
                    for (const char* path : paths) {
                        std::ifstream file(path);
                        if (file.is_open()) {
                            std::stringstream buffer;
                            buffer << file.rdbuf();
                            shaderSource_ = buffer.str();
                            shaderPath_ = path;
                            loadedPath = path;
                            break;
                        }
                    }

                    if (shaderSource_.empty()) {
                        std::cerr << "ERROR: Could not find shader. Tried:\n";
                        for (const char* path : paths) {
                            std::cerr << "  - " << path << "\n";
                        }
                        return false;
                    }

                    std::cout << "Loaded shader from: " << loadedPath << "\n";
                }

                wgpu::ShaderSourceWGSL wgslSource{};
                wgslSource.code = shaderSource_.c_str();

                wgpu::ShaderModuleDescriptor desc{};
                desc.nextInChain = &wgslSource;
                desc.label = "world.wgsl (The_Board Cartridge)";

                auto tShader0 = std::chrono::high_resolution_clock::now();
                shaderModule_ = device_.CreateShaderModule(&desc);
                auto tShader1 = std::chrono::high_resolution_clock::now();
                std::cout << "[Renderer] Shader compile:    "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(tShader1 - tShader0).count()
                    << " ms\n";
                return shaderModule_ != nullptr;
            }

            bool createComputePipelines() {
                // Shared pipeline layout for all standard compute passes (Group 0 only)
                std::array<wgpu::BindGroupLayout, 1> computeLayouts = {
                    computeEntityLayout_
                };

                wgpu::PipelineLayoutDescriptor layoutDesc{};
                layoutDesc.bindGroupLayoutCount = computeLayouts.size();
                layoutDesc.bindGroupLayouts = computeLayouts.data();
                wgpu::PipelineLayout computeLayout = device_.CreatePipelineLayout(&layoutDesc);
                if (!computeLayout) return false;

                // Shared pipeline layout for compute pipelines that evaluate
                // query_ground_flyer / query_ground_walker. Group 0 is the
                // same compute-entity layout; Group 1 adds the aura texture +
                // sampler needed by sample_pawn_aura on the compute path.
                // Used by update_sphere, update_cube (POLICY_FLYER),
                // update_player_agent (POLICY_WALKER), and update_other_agents
                // (POLICY_WALKER_AGENT — same texture binding for sample_pawn_aura).
                // Created here (before any pipeline that needs it) so the kernel
                // can reach it during behavior dispatch.
                std::array<wgpu::BindGroupLayout, 2> liveContribLayouts = {
                    computeEntityLayout_,
                    computeTextureLayout_
                };
                wgpu::PipelineLayoutDescriptor liveContribLayoutDesc{};
                liveContribLayoutDesc.bindGroupLayoutCount = liveContribLayouts.size();
                liveContribLayoutDesc.bindGroupLayouts = liveContribLayouts.data();
                wgpu::PipelineLayout liveContribComputeLayout =
                    device_.CreatePipelineLayout(&liveContribLayoutDesc);
                if (!liveContribComputeLayout) return false;

                // THE ROOM (Option B, Batch F; FIELD_2 amendment): the two
                // agent kernels AND the two floater kernels carry group 2
                // on top of the shared live-contributor pair. The camera
                // keeps the two-group layout untouched, so tenant-side
                // binding growth (the occupier windows, the field pair)
                // never widens its compile surface.
                std::array<wgpu::BindGroupLayout, 3> roomLayouts = {
                    computeEntityLayout_,
                    computeTextureLayout_,
                    roomLayout_
                };
                wgpu::PipelineLayoutDescriptor roomLayoutDesc{};
                roomLayoutDesc.bindGroupLayoutCount = roomLayouts.size();
                roomLayoutDesc.bindGroupLayouts = roomLayouts.data();
                wgpu::PipelineLayout roomComputeLayout =
                    device_.CreatePipelineLayout(&roomLayoutDesc);
                if (!roomComputeLayout) return false;

                // Pipeline: update_player_agent (0D, 1 thread — possessed slot only)
                // Room layout (live-contributor pair + the room) —
                // pawn_ground_resolve, terrain_normal_at
                // call query_ground_walker → contrib_pawn_aura_at → sample_pawn_aura.
                // The walker-policy heavy path inlines once, for one slot.
                if (!makeComputePipeline("update_player_agent", "Update Player Agent (0D, 1 thread)",
                    roomComputeLayout, Entry::UPDATE_PLAYER_AGENT, updatePlayerAgentPipeline_)) return false;

                // Pipeline: update_other_agents (1D, 32 threads — non-possessed slots)
                // Room layout (live-contributor pair + the room) —
                // query_ground_walker_agent reads aura
                // grid via contrib_pawn_aura_at_external → sample_pawn_aura.
                // The walker-policy heavy path is NOT inlined here; algorithmic
                // behaviors only.
                if constexpr (ROSTER.wanderers) {  // ROSTER-GATE wanderers (a') — shader compile skipped when disabled
                if (!makeComputePipeline("update_other_agents", "Update Other Agents (1D, 32 threads)",
                    roomComputeLayout, Entry::UPDATE_OTHER_AGENTS, updateOtherAgentsPipeline_)) return false;
                }

                // Pipeline: update_camera (0D)
                // Live-contributor layout — the camera clamp uses a walker-style
                // policy that reads the aura texture (sample_pawn_aura).
                if (!makeComputePipeline("update_camera", "Update Camera (0D)",
                    liveContribComputeLayout, Entry::UPDATE_CAMERA, updateCameraPipeline_)) return false;

                // Pipeline: update_sphere (0D)
                // Room layout (FIELD_2 tenancy) — coupling_terrain_to_sphere_orbit_height
                // still calls query_ground_flyer (→ contrib_pawn_aura_at → sample_pawn_aura);
                // group 2 adds the field bindings. Unused group members are legal.
                if constexpr (ROSTER.sphere) {  // ROSTER-GATE sphere (a') — shader compile skipped when disabled
                if (!makeComputePipeline("update_sphere", "Update Sphere (0D)",
                    roomComputeLayout, Entry::UPDATE_SPHERE, updateSpherePipeline_)) return false;
                }

                // Pipeline: update_cube (0D)
                // Room layout (FIELD_2 tenancy) — update_cube calls
                // query_ground_flyer directly for hover-base clearance;
                // group 2 adds the field bindings.
                if constexpr (ROSTER.cube) {  // ROSTER-GATE cube (a') — shader compile skipped when disabled
                if (!makeComputePipeline("update_cube", "Update Cube (0D)",
                    roomComputeLayout, Entry::UPDATE_CUBE, updateCubePipeline_)) return false;
                }

                // Pipeline: compute_vp (0D)
                if (!makeComputePipeline("compute_vp", "Compute VP Matrix (0D)",
                    computeLayout, Entry::COMPUTE_VP, computeVPPipeline_)) return false;

                // Pipeline: generate_patch_heights (2D, pass 1 — heights only)
                {
                    wgpu::PipelineLayout pl = computeLayoutFor(patchGenLayout_);
                    if (!pl) return false;
                    if (!makeComputePipeline("gen_patch_heights", "Generate Patch Heights (2D, pass 1)",
                        pl, Entry::GENERATE_PATCH_HEIGHTS, generatePatchHeightsPipeline_)) return false;
                }

                // Pipeline: generate_patch_gradients (2D, pass 2 — gradients + complexity)
                {
                    wgpu::PipelineLayout pl = computeLayoutFor(patchGenLayout_);
                    if (!pl) return false;
                    if (!makeComputePipeline("gen_patch_gradients", "Generate Patch Gradients (2D, pass 2)",
                        pl, Entry::GENERATE_PATCH_GRADIENTS, generatePatchGradientsPipeline_)) return false;
                }

                // Pipeline: generate_patch_cells (2D, on demand)
                {
                    wgpu::PipelineLayout pl = computeLayoutFor(patchGenLayout_);
                    if (!pl) return false;
                    if (!makeComputePipeline("gen_patch_cells", "Generate Patch Cells (2D, on demand)",
                        pl, Entry::GENERATE_PATCH_CELLS, generatePatchCellsPipeline_)) return false;
                }

                // Pipeline: compute_ribbon_rings (1D, per frame when ribbon active)
                if constexpr (ROSTER.ribbon) {  // ROSTER-GATE ribbon (a') — shader compile skipped when disabled
                    wgpu::PipelineLayout pl = computeLayoutFor(ribbonComputeLayout_);
                    if (!pl) return false;
                    if (!makeComputePipeline("compute_ribbon_rings", "Compute Ribbon Rings (1D, per frame)",
                        pl, Entry::COMPUTE_RIBBON_RINGS, ribbonRingPipeline_)) return false;
                }

                // Photographer VP compute pipeline (0D, reads pawn → writes VP)
                if constexpr (ROSTER.gallery) {  // ROSTER-GATE gallery (a') — shader compile skipped when disabled
                    wgpu::PipelineLayout pl = computeLayoutFor(photographerComputeLayout_);
                    if (!pl) return false;
                    if (!makeComputePipeline("compute_photographer_vp", "Compute Photographer VP (0D)",
                        pl, Entry::COMPUTE_PHOTOGRAPHER_VP, photographerVPPipeline_)) return false;
                }

                // Entity placement Y-correction pipeline (0D, decoupled from photographer)
                {
                    // Placement gains Group 1 (compute textures): the card's
                    // cell-exact GoL fetch (GROUND_CARD_1 H5). Shared @group(1)
                    // declarations serve; unused group members are legal — the
                    // layout must cover the shader, not vice versa.
                    std::array<wgpu::BindGroupLayout, 2> placementLayouts = {
                        entityPlacementComputeLayout_,
                        computeTextureLayout_
                    };
                    wgpu::PipelineLayoutDescriptor pld{};
                    pld.bindGroupLayoutCount = placementLayouts.size();
                    pld.bindGroupLayouts = placementLayouts.data();
                    wgpu::PipelineLayout pl = device_.CreatePipelineLayout(&pld);
                    if (!pl) return false;
                    if (!makeComputePipeline("compute_entity_placement", "Compute Entity Placement (0D)",
                        pl, Entry::COMPUTE_ENTITY_PLACEMENT, entityPlacementPipeline_)) return false;
                }

                // GPU frustum cull pipeline (dedicated layout)
                {
                    wgpu::PipelineLayout pl = computeLayoutFor(frustumCullLayout_);
                    if (!pl) return false;
                    if (!makeComputePipeline("frustum_cull_patches", "Frustum Cull Patches",
                        pl, Entry::FRUSTUM_CULL_PATCHES, frustumCullPipeline_)) return false;
                }

                // Pawn aura compute pipeline (dedicated layout)
                if constexpr (ROSTER.pawn_aura) {  // ROSTER-GATE pawn_aura (a') — shader compile skipped when disabled
                    wgpu::PipelineLayout pl = computeLayoutFor(pawnAuraComputeLayout_);
                    if (!pl) return false;
                    if (!makeComputePipeline("compute_pawn_aura", "Compute Pawn Aura (2D)",
                        pl, Entry::COMPUTE_PAWN_AURA, pawnAuraPipeline_)) return false;
                }

                // Live card writer pipelines (two-pass — TRUEBAND_CONTACT_1;
                // the patch-gen dispatch-pair shape at card size)
                {
                    wgpu::PipelineLayout pl = computeLayoutFor(liveCardWriterLayout_);
                    if (!pl) return false;
                    if (!makeComputePipeline("write_live_card_heights", "Live Card Heights (2D)",
                        pl, Entry::WRITE_LIVE_CARD_HEIGHTS, liveCardHeightsPipeline_)) return false;
                    if (!makeComputePipeline("write_live_card_resolve", "Live Card Resolve (2D)",
                        pl, Entry::WRITE_LIVE_CARD_RESOLVE, liveCardResolvePipeline_)) return false;
                }

                // Orb compute pipelines (init + dynamics + recolor share the dedicated orb layout)
                if constexpr (ROSTER.orbs) {  // ROSTER-GATE orbs (a') — shader compile skipped when disabled
                    wgpu::PipelineLayout pl = computeLayoutFor(orbComputeLayout_);
                    if (!pl) return false;
                    if (!makeComputePipeline("orb_init", "Orb Init", pl, Entry::ORB_INIT, orbInitPipeline_)) return false;
                    if (!makeComputePipeline("orb_dynamics", "Orb Dynamics", pl, Entry::ORB_DYNAMICS, orbDynamicsPipeline_)) return false;
                    if (!makeComputePipeline("orb_recolor", "Orb Recolor", pl, Entry::ORB_RECOLOR, orbRecolorPipeline_)) return false;
                }

                // Orb copy-prev pipeline (Pass 9) — dedicated layout because
                // it flips the access modes on orb_state / orb_state_prev.
                if constexpr (ROSTER.orbs) {  // ROSTER-GATE orbs (a') — shader compile skipped when disabled
                    wgpu::PipelineLayout pl = computeLayoutFor(orbCopyLayout_);
                    if (!pl) return false;
                    if (!makeComputePipeline("orb_state_prev_copy", "Orb State Prev Copy",
                        pl, Entry::ORB_STATE_PREV_COPY, orbCopyPrevPipeline_)) return false;
                }

                // GoL zone compute pipelines (dedicated layout, z-dispatched)
                if constexpr (ROSTER.gol) {  // ROSTER-GATE gol (a') — shader compile skipped when disabled
                    wgpu::PipelineLayout pl = computeLayoutFor(zoneGolComputeLayout_);
                    if (!pl) return false;
                    if (!makeComputePipeline("zone_gol_sync", "GoL Zone Sync", pl, Entry::ZONE_GOL_SYNC, zoneGolSyncPipeline_)) return false;
                    if (!makeComputePipeline("zone_gol_evolve", "GoL Zone Evolve", pl, Entry::ZONE_GOL_EVOLVE, zoneGolEvolvePipeline_)) return false;
                }

                // Zone derive pipeline (shared GoL layout)
                if constexpr (ROSTER.gol) {  // ROSTER-GATE gol (a') — shader compile skipped when disabled
                    wgpu::PipelineLayout pl = computeLayoutFor(zoneGolComputeLayout_);
                    if (!pl) return false;
                    if (!makeComputePipeline("zone_derive_params", "Zone Derive Params", pl, Entry::ZONE_DERIVE_PARAMS, zoneDeriveParamsPipeline_)) return false;
                }

                // Zone mask pipeline (dedicated layout — UNIFIED_GROUND_1 U5)
                if constexpr (ROSTER.gol) {  // ROSTER-GATE gol (a') — shader compile skipped when disabled
                    wgpu::PipelineLayout pl = computeLayoutFor(zoneMaskLayout_);
                    if (!pl) return false;
                    if (!makeComputePipeline("zone_seed_mask", "Zone Seed Mask (2D)",
                        pl, Entry::ZONE_SEED_MASK, zoneSeedMaskPipeline_)) return false;
                }

                // Mesh-gen compute pipelines — one dedicated single-group layout each,
                // per-family ROSTER gate; identical creation modulo (layout, entry, member).
                // pyramid mesh-gen pipeline CUT — mesh never drawn.

                if constexpr (ROSTER.arch) {  // ROSTER-GATE arch (a') — shader compile skipped when disabled
                    wgpu::PipelineLayout pl = computeLayoutFor(archMeshGenLayout_);   // bindings 193-195
                    if (!pl) return false;
                    if (!makeComputePipeline("arch_mesh_gen", "Arch Mesh Gen",
                        pl, Entry::ARCH_MESH_GEN, archMeshGenPipeline_)) return false;
                }

                if constexpr (ROSTER.column || ROSTER.antenna) {  // ROSTER-GATE column+antenna (shared pipelines) (a') — shader compile skipped when disabled
                    wgpu::PipelineLayout pl = computeLayoutFor(columnMeshGenLayout_);  // bindings 196-198
                    if (!pl) return false;
                    if (!makeComputePipeline("column_mesh_gen", "Column Mesh Gen",
                        pl, Entry::COLUMN_MESH_GEN, columnMeshGenPipeline_)) return false;
                }

                if constexpr (ROSTER.palm) {  // ROSTER-GATE palm (a') — shader compile skipped when disabled
                    wgpu::PipelineLayout pl = computeLayoutFor(palmMeshGenLayout_);
                    if (!pl) return false;
                    if (!makeComputePipeline("palm_mesh_gen", "Palm Mesh Gen",
                        pl, Entry::PALM_MESH_GEN, palmMeshGenPipeline_)) return false;
                }

                if constexpr (ROSTER.cactus) {  // ROSTER-GATE cactus (a') — shader compile skipped when disabled
                    wgpu::PipelineLayout pl = computeLayoutFor(cactusMeshGenLayout_);
                    if (!pl) return false;
                    if (!makeComputePipeline("cactus_mesh_gen", "Cactus Mesh Gen",
                        pl, Entry::CACTUS_MESH_GEN, cactusMeshGenPipeline_)) return false;
                }

                if constexpr (ROSTER.blade) {  // ROSTER-GATE blade (a') — shader compile skipped when disabled
                    wgpu::PipelineLayout pl = computeLayoutFor(bladeMeshGenLayout_);
                    if (!pl) return false;
                    if (!makeComputePipeline("blade_cluster_mesh_gen", "Blade Mesh Gen",
                        pl, Entry::BLADE_MESH_GEN, bladeMeshGenPipeline_)) return false;
                }

                return true;
            }

            bool createRenderPipelines() {
                // Shadow pipeline layout (entity + textures WITHOUT shadow map)
                std::array<wgpu::BindGroupLayout, 2> shadowLayouts = {
                    renderEntityLayout_,
                    shadowTextureLayout_
                };

                wgpu::PipelineLayoutDescriptor shadowLayoutDesc{};
                shadowLayoutDesc.bindGroupLayoutCount = shadowLayouts.size();
                shadowLayoutDesc.bindGroupLayouts = shadowLayouts.data();
                wgpu::PipelineLayout shadowRenderLayout = device_.CreatePipelineLayout(&shadowLayoutDesc);
                if (!shadowRenderLayout) return false;

                // Main render pipeline layout (entity + textures WITH shadow map)
                std::array<wgpu::BindGroupLayout, 2> renderLayouts = {
                    renderEntityLayout_,
                    renderTextureLayout_
                };

                wgpu::PipelineLayoutDescriptor layoutDesc{};
                layoutDesc.bindGroupLayoutCount = renderLayouts.size();
                layoutDesc.bindGroupLayouts = renderLayouts.data();
                wgpu::PipelineLayout renderLayout = device_.CreatePipelineLayout(&layoutDesc);
                if (!renderLayout) return false;

                // Shared depth stencil state
                wgpu::DepthStencilState depthStencil{};
                depthStencil.format = depthFormat_;
                depthStencil.depthWriteEnabled = true;
                depthStencil.depthCompare = wgpu::CompareFunction::Less;

                // Shared color target
                wgpu::ColorTargetState colorTarget{};
                colorTarget.format = colorFormat_;
                colorTarget.writeMask = wgpu::ColorWriteMask::All;

                // THE SHARED BUILDER (entity category): the builder every ENTITY_FS pipeline shares —
                // renderLayout + depthStencil + colorTarget + ENTITY_FS + TriangleList + CCW.
                // The genuine forks are parameters: the VS entry (passed VERBATIM), the
                // vertex-buffer layout (nullptr = bufferless, GPU-generated from vertex_index),
                // and cullMode — a REAL per-pipeline field, NOT noise: single-sided frond/
                // blade/column quads disable backface cull (None), solids keep Back. Same
                // shared desc the originals mutated in place, rebuilt fresh per call
                // (byte-identical result). Captures renderLayout/depthStencil/colorTarget.
                auto makeEntity = [&](const char* label, const char* dbgLabel, const char* vsEntry,
                                      const wgpu::VertexBufferLayout* vbl, wgpu::CullMode cull,
                                      wgpu::RenderPipeline& out) -> bool {
                    wgpu::FragmentState fragment{};
                    fragment.module = shaderModule_;
                    fragment.entryPoint = Entry::ENTITY_FS;
                    fragment.targetCount = 1;
                    fragment.targets = &colorTarget;

                    wgpu::RenderPipelineDescriptor desc{};
                    desc.label = dbgLabel;
                    desc.layout = renderLayout;
                    desc.vertex.module = shaderModule_;
                    desc.vertex.entryPoint = vsEntry;
                    desc.vertex.bufferCount = vbl ? 1u : 0u;
                    desc.vertex.buffers = vbl;
                    desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                    desc.primitive.cullMode = cull;
                    desc.primitive.frontFace = wgpu::FrontFace::CCW;
                    desc.depthStencil = &depthStencil;
                    desc.fragment = &fragment;
                    return tPipe(label, [&]() {
                        out = device_.CreateRenderPipeline(&desc);
                        return out != nullptr;
                    });
                };

                // Patch terrain pipeline -- instanced, no vertex buffer
                {
                    wgpu::FragmentState fragment{};
                    fragment.module = shaderModule_;
                    fragment.entryPoint = Entry::PATCH_TERRAIN_FS;
                    fragment.targetCount = 1;
                    fragment.targets = &colorTarget;

                    wgpu::RenderPipelineDescriptor desc{};
                    desc.label = "Patch Terrain (instanced)";
                    desc.layout = renderLayout;
                    desc.vertex.module = shaderModule_;
                    desc.vertex.entryPoint = Entry::PATCH_TERRAIN_VS;
                    desc.vertex.bufferCount = 0;
                    desc.vertex.buffers = nullptr;
                    desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                    desc.primitive.cullMode = wgpu::CullMode::Back;
                    desc.primitive.frontFace = wgpu::FrontFace::CCW;
                    desc.depthStencil = &depthStencil;
                    desc.fragment = &fragment;

                    if (!tPipe("patch_terrain", [&]() {
                        patchTerrainPipeline_ = device_.CreateRenderPipeline(&desc);
                        return patchTerrainPipeline_ != nullptr;
                    })) return false;
                }

                // Indirect terrain variant — USE_PATCH_INDIRECTION=true.
                // VS reads patch_instances[visible_patch_indices[instance_index]].
                {
                    wgpu::ConstantEntry overrides[1]{};
                    overrides[0].key = "USE_PATCH_INDIRECTION"; overrides[0].value = 1.0;

                    wgpu::FragmentState fragment{};
                    fragment.module = shaderModule_;
                    fragment.entryPoint = Entry::PATCH_TERRAIN_FS;
                    fragment.targetCount = 1;
                    fragment.targets = &colorTarget;

                    wgpu::RenderPipelineDescriptor desc{};
                    desc.label = "Patch Terrain Indirect (VS indirection)";
                    desc.layout = renderLayout;
                    desc.vertex.module = shaderModule_;
                    desc.vertex.entryPoint = Entry::PATCH_TERRAIN_VS;
                    desc.vertex.constantCount = 1;
                    desc.vertex.constants = overrides;
                    desc.vertex.bufferCount = 0;
                    desc.vertex.buffers = nullptr;
                    desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                    desc.primitive.cullMode = wgpu::CullMode::Back;
                    desc.primitive.frontFace = wgpu::FrontFace::CCW;
                    desc.depthStencil = &depthStencil;
                    desc.fragment = &fragment;

                    if (!tPipe("patch_terrain_indirect", [&]() {
                        patchTerrainIndirectPipeline_ = device_.CreateRenderPipeline(&desc);
                        return patchTerrainIndirectPipeline_ != nullptr;
                    })) return false;
                }

                // Pawn pipeline -- chess pawn, GPU-generated from vertex_index (bufferless, cull None)
                if (!makeEntity("pawn", "Pawn Entity (Chess Pawn)", Entry::PAWN_VS,
                    nullptr, wgpu::CullMode::None, pawnPipeline_)) return false;

                // Sphere pipeline -- sphere entity, MeshVertex (pos+normal)
                {
                    std::array<wgpu::VertexAttribute, 2> meshAttrs{};
                    meshAttrs[0].format = wgpu::VertexFormat::Float32x3;
                    meshAttrs[0].offset = 0;
                    meshAttrs[0].shaderLocation = 0;
                    meshAttrs[1].format = wgpu::VertexFormat::Float32x3;
                    meshAttrs[1].offset = 12;
                    meshAttrs[1].shaderLocation = 1;

                    wgpu::VertexBufferLayout meshVBL{};
                    meshVBL.arrayStride = 24;
                    meshVBL.stepMode = wgpu::VertexStepMode::Vertex;
                    meshVBL.attributeCount = meshAttrs.size();
                    meshVBL.attributes = meshAttrs.data();

                    // Sphere + Monolith — same MeshVertex format, Back cull, differ only by VS.
                    if constexpr (ROSTER.sphere) {  // ROSTER-GATE sphere (a') — shader compile skipped when disabled
                    if (!makeEntity("sphere", "Sphere Entity (Rasterized)", Entry::SPHERE_VS,
                        &meshVBL, wgpu::CullMode::Back, spherePipeline_)) return false;
                    }
                    if constexpr (ROSTER.cube) {  // ROSTER-GATE cube (a') — shader compile skipped when disabled
                    if (!makeEntity("monolith", "Monolith Entity (Rasterized)", Entry::MONOLITH_VS,
                        &meshVBL, wgpu::CullMode::Back, monolithPipeline_)) return false;
                    }
                }

                // Arch pipeline -- catenary arch, ArchVertex (pos+normal+color+arch_index), static world-space
                {
                    std::array<wgpu::VertexAttribute, 4> archAttrs{};
                    archAttrs[0].format = wgpu::VertexFormat::Float32x3;
                    archAttrs[0].offset = 0;
                    archAttrs[0].shaderLocation = 0;
                    archAttrs[1].format = wgpu::VertexFormat::Float32x3;
                    archAttrs[1].offset = 12;
                    archAttrs[1].shaderLocation = 1;
                    archAttrs[2].format = wgpu::VertexFormat::Float32x3;
                    archAttrs[2].offset = 24;
                    archAttrs[2].shaderLocation = 2;
                    archAttrs[3].format = wgpu::VertexFormat::Float32;
                    archAttrs[3].offset = 36;
                    archAttrs[3].shaderLocation = 3;

                    wgpu::VertexBufferLayout archVBL{};
                    archVBL.arrayStride = 40;
                    archVBL.stepMode = wgpu::VertexStepMode::Vertex;
                    archVBL.attributeCount = archAttrs.size();
                    archVBL.attributes = archAttrs.data();

                    // Arch/column/palm/cactus/blade/pyramid — same ArchVertex format; differ by
                    // VS + cull. Single-sided column/palm/cactus/blade quads disable backface
                    // cull (None); arch + pyramid are solids (Back). (cullMode is a real fork.)
                    if constexpr (ROSTER.arch) {  // ROSTER-GATE arch (a') — shader compile skipped when disabled
                    if (!makeEntity("arch", "Catenary Arch (Rasterized)", Entry::ARCH_VS,
                        &archVBL, wgpu::CullMode::Back, archPipeline_)) return false;
                    }
                    if constexpr (ROSTER.column || ROSTER.antenna) {  // ROSTER-GATE column+antenna (shared pipelines) (a') — shader compile skipped when disabled
                    if (!makeEntity("column", "Generative Column (Rasterized)", Entry::COLUMN_VS,
                        &archVBL, wgpu::CullMode::None, columnPipeline_)) return false;
                    }
                    if constexpr (ROSTER.palm) {  // ROSTER-GATE palm (a') — shader compile skipped when disabled
                    if (!makeEntity("palm", "Palm Tree (Rasterized)", Entry::PALM_VS,
                        &archVBL, wgpu::CullMode::None, palmPipeline_)) return false;
                    }
                    if constexpr (ROSTER.cactus) {  // ROSTER-GATE cactus (a') — shader compile skipped when disabled
                    if (!makeEntity("cactus", "Cactus (Rasterized)", Entry::CACTUS_VS,
                        &archVBL, wgpu::CullMode::None, cactusPipeline_)) return false;
                    }
                    if constexpr (ROSTER.blade) {  // ROSTER-GATE blade (a') — shader compile skipped when disabled
                    if (!makeEntity("blade", "Blade Cluster (Rasterized)", Entry::BLADE_VS,
                        &archVBL, wgpu::CullMode::None, bladePipeline_)) return false;
                    }
                    // pyramid render pipeline CUT — mesh never drawn
                }

                // Shell pipeline -- indoor ceiling + walls, ShellVertex (pos+normal+color), static world-space
                {
                    std::array<wgpu::VertexAttribute, 3> shellAttrs{};
                    shellAttrs[0].format = wgpu::VertexFormat::Float32x3;
                    shellAttrs[0].offset = 0;
                    shellAttrs[0].shaderLocation = 0;  // pos
                    shellAttrs[1].format = wgpu::VertexFormat::Float32x3;
                    shellAttrs[1].offset = 12;
                    shellAttrs[1].shaderLocation = 1;  // normal
                    shellAttrs[2].format = wgpu::VertexFormat::Float32x3;
                    shellAttrs[2].offset = 24;
                    shellAttrs[2].shaderLocation = 2;  // color

                    wgpu::VertexBufferLayout shellVBL{};
                    shellVBL.arrayStride = 36;
                    shellVBL.stepMode = wgpu::VertexStepMode::Vertex;
                    shellVBL.attributeCount = shellAttrs.size();
                    shellVBL.attributes = shellAttrs.data();

                    // ROSTER-GATE indoor_shell (a) — SEPARABLE: skip the shell
                    // pipeline creation when disabled. draw_shell self-gates on
                    // shell_index_count==0 (stays 0 — apply_mood_indoor_shell is
                    // (b)-gated), so the null pipeline is never bound.
                    // cull None: shell normals face inward (ceiling) + outward (walls).
                    if constexpr (ROSTER.indoor_shell) {  // ROSTER-GATE indoor_shell (a') — shader compile skipped when disabled
                    if (!makeEntity("shell", "Indoor Shell (Ceiling + Walls)", Entry::SHELL_VS,
                        &shellVBL, wgpu::CullMode::None, shellPipeline_)) return false;
                    }
                }

                // Ribbon pipeline -- sky ribbon, GPU-generated cubes from vertex_index.
                // RIBBON_FS = entity shading with veil_scale 0 (the ruled fork: a
                // flown structure stays whole beyond the band).
                {
                    wgpu::FragmentState fragment{};
                    fragment.module = shaderModule_;
                    fragment.entryPoint = Entry::RIBBON_FS;
                    fragment.targetCount = 1;
                    fragment.targets = &colorTarget;

                    wgpu::RenderPipelineDescriptor desc{};
                    desc.label = "Sky Ribbon Entity";
                    desc.layout = renderLayout;
                    desc.vertex.module = shaderModule_;
                    desc.vertex.entryPoint = Entry::RIBBON_VS;
                    desc.vertex.bufferCount = 0;
                    desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                    desc.primitive.cullMode = wgpu::CullMode::None;
                    desc.primitive.frontFace = wgpu::FrontFace::CCW;
                    desc.depthStencil = &depthStencil;
                    desc.fragment = &fragment;

                    if constexpr (ROSTER.ribbon) {  // ROSTER-GATE ribbon (a') — shader compile skipped when disabled
                    if (!tPipe("ribbon", [&]() {
                        ribbonPipeline_ = device_.CreateRenderPipeline(&desc);
                        return ribbonPipeline_ != nullptr;
                    })) return false;
                    }
                }

                // Orb pipeline -- billboarded glowing sprites, additive blended,
                // depth-tested but not depth-writing (transparent, stack safely).
                {
                    wgpu::VertexAttribute orbAttr{};
                    orbAttr.format = wgpu::VertexFormat::Float32x2;
                    orbAttr.offset = 0;
                    orbAttr.shaderLocation = 0;

                    wgpu::VertexBufferLayout orbVBL{};
                    orbVBL.arrayStride = 8;  // 2 × f32
                    orbVBL.stepMode = wgpu::VertexStepMode::Vertex;
                    orbVBL.attributeCount = 1;
                    orbVBL.attributes = &orbAttr;

                    // ORB_V: OrbState arrives as an INSTANCE-STEP vertex buffer
                    // rather than a VS storage binding — one attribute per
                    // OrbState field, offsets mirroring the struct (world.wgsl
                    // `struct OrbState` / `GPUOrbState`, state.hpp, 80 B), so
                    // orb_vs reconstructs the value it used to fetch. Frees the
                    // vertex stage's binding 400 seat family-wide; see
                    // BINDING_LEDGER Table F.
                    std::array<wgpu::VertexAttribute, 12> orbStateAttrs{};
                    orbStateAttrs[0].format = wgpu::VertexFormat::Float32x3;  // pos
                    orbStateAttrs[0].offset = 0;
                    orbStateAttrs[0].shaderLocation = 1;
                    orbStateAttrs[1].format = wgpu::VertexFormat::Float32;    // _pad0
                    orbStateAttrs[1].offset = 12;
                    orbStateAttrs[1].shaderLocation = 2;
                    orbStateAttrs[2].format = wgpu::VertexFormat::Float32x3;  // vel
                    orbStateAttrs[2].offset = 16;
                    orbStateAttrs[2].shaderLocation = 3;
                    orbStateAttrs[3].format = wgpu::VertexFormat::Float32;    // _pad1
                    orbStateAttrs[3].offset = 28;
                    orbStateAttrs[3].shaderLocation = 4;
                    orbStateAttrs[4].format = wgpu::VertexFormat::Float32x3;  // base_color
                    orbStateAttrs[4].offset = 32;
                    orbStateAttrs[4].shaderLocation = 5;
                    orbStateAttrs[5].format = wgpu::VertexFormat::Float32;    // brightness
                    orbStateAttrs[5].offset = 44;
                    orbStateAttrs[5].shaderLocation = 6;
                    orbStateAttrs[6].format = wgpu::VertexFormat::Float32x3;  // current_color
                    orbStateAttrs[6].offset = 48;
                    orbStateAttrs[6].shaderLocation = 7;
                    orbStateAttrs[7].format = wgpu::VertexFormat::Float32;    // twinkle_phase
                    orbStateAttrs[7].offset = 60;
                    orbStateAttrs[7].shaderLocation = 8;
                    orbStateAttrs[8].format = wgpu::VertexFormat::Float32;    // size
                    orbStateAttrs[8].offset = 64;
                    orbStateAttrs[8].shaderLocation = 9;
                    orbStateAttrs[9].format = wgpu::VertexFormat::Float32;    // mass
                    orbStateAttrs[9].offset = 68;
                    orbStateAttrs[9].shaderLocation = 10;
                    orbStateAttrs[10].format = wgpu::VertexFormat::Float32;   // drag
                    orbStateAttrs[10].offset = 72;
                    orbStateAttrs[10].shaderLocation = 11;
                    orbStateAttrs[11].format = wgpu::VertexFormat::Uint32;    // tier_idx
                    orbStateAttrs[11].offset = 76;
                    orbStateAttrs[11].shaderLocation = 12;

                    wgpu::VertexBufferLayout orbStateVBL{};
                    orbStateVBL.arrayStride = 80;  // sizeof(GPUOrbState)
                    orbStateVBL.stepMode = wgpu::VertexStepMode::Instance;
                    orbStateVBL.attributeCount = orbStateAttrs.size();
                    orbStateVBL.attributes = orbStateAttrs.data();

                    std::array<wgpu::VertexBufferLayout, 2> orbVBLs{{ orbVBL, orbStateVBL }};

                    // Additive blend (premultiplied alpha in FS: out.rgb = color*intensity).
                    wgpu::BlendState orbBlend{};
                    orbBlend.color.srcFactor = wgpu::BlendFactor::One;
                    orbBlend.color.dstFactor = wgpu::BlendFactor::One;
                    orbBlend.color.operation = wgpu::BlendOperation::Add;
                    orbBlend.alpha.srcFactor = wgpu::BlendFactor::One;
                    orbBlend.alpha.dstFactor = wgpu::BlendFactor::One;
                    orbBlend.alpha.operation = wgpu::BlendOperation::Add;

                    wgpu::ColorTargetState orbColorTarget{};
                    orbColorTarget.format = colorFormat_;
                    orbColorTarget.blend = &orbBlend;
                    orbColorTarget.writeMask = wgpu::ColorWriteMask::All;

                    wgpu::FragmentState fragment{};
                    fragment.module = shaderModule_;
                    fragment.entryPoint = Entry::ORB_FS;
                    fragment.targetCount = 1;
                    fragment.targets = &orbColorTarget;

                    // Depth: test yes, write no — orbs don't occlude each other
                    // or geometry behind them.
                    wgpu::DepthStencilState orbDepth{};
                    orbDepth.format = depthFormat_;
                    orbDepth.depthCompare = wgpu::CompareFunction::Less;
                    orbDepth.depthWriteEnabled = false;

                    wgpu::RenderPipelineDescriptor desc{};
                    desc.label = "Orb Sky Layer";
                    desc.layout = renderLayout;
                    desc.vertex.module = shaderModule_;
                    desc.vertex.entryPoint = Entry::ORB_VS;
                    desc.vertex.bufferCount = orbVBLs.size();
                    desc.vertex.buffers = orbVBLs.data();
                    desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                    desc.primitive.cullMode = wgpu::CullMode::None;  // billboards face camera
                    desc.primitive.frontFace = wgpu::FrontFace::CCW;
                    desc.depthStencil = &orbDepth;
                    desc.fragment = &fragment;

                    if constexpr (ROSTER.orbs) {  // ROSTER-GATE orbs (a') — shader compile skipped when disabled
                    if (!tPipe("orb", [&]() {
                        orbRenderPipeline_ = device_.CreateRenderPipeline(&desc);
                        return orbRenderPipeline_ != nullptr;
                    })) return false;
                    }
                }

                // ─── Gallery Frame Pipeline ──────────────────────────────────────
                // Instanced subdivided quads textured with painting snapshots.
                // Dedicated pipeline layout (galleryEntity + galleryTexture).
                {
                    wgpu::PipelineLayoutDescriptor pld{};
                    std::array<wgpu::BindGroupLayout, 2> galleryGroups = {
                        galleryEntityLayout_, galleryTextureLayout_
                    };
                    pld.bindGroupLayoutCount = galleryGroups.size();
                    pld.bindGroupLayouts = galleryGroups.data();
                    wgpu::PipelineLayout galleryLayout = device_.CreatePipelineLayout(&pld);

                    wgpu::ColorTargetState colorTarget{};
                    colorTarget.format = colorFormat_;
                    colorTarget.writeMask = wgpu::ColorWriteMask::All;

                    wgpu::BlendState blend{};
                    blend.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
                    blend.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
                    blend.alpha.srcFactor = wgpu::BlendFactor::One;
                    blend.alpha.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
                    colorTarget.blend = &blend;

                    wgpu::FragmentState frag{};
                    frag.module = shaderModule_;
                    frag.entryPoint = Entry::GALLERY_FRAME_FS;
                    frag.targetCount = 1;
                    frag.targets = &colorTarget;

                    wgpu::DepthStencilState galleryDepth{};
                    galleryDepth.format = depthFormat_;
                    galleryDepth.depthWriteEnabled = true;
                    galleryDepth.depthCompare = wgpu::CompareFunction::Less;

                    wgpu::RenderPipelineDescriptor desc{};
                    desc.label = "Gallery Frame";
                    desc.layout = galleryLayout;
                    desc.vertex.module = shaderModule_;
                    desc.vertex.entryPoint = Entry::GALLERY_FRAME_VS;
                    desc.vertex.bufferCount = 0;
                    desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                    desc.primitive.cullMode = wgpu::CullMode::None;
                    desc.primitive.frontFace = wgpu::FrontFace::CCW;
                    desc.depthStencil = &galleryDepth;
                    desc.fragment = &frag;

                    if constexpr (ROSTER.gallery) {  // ROSTER-GATE gallery (a') — shader compile skipped when disabled
                    if (!tPipe("gallery_frame", [&]() {
                        galleryFramePipeline_ = device_.CreateRenderPipeline(&desc);
                        return galleryFramePipeline_ != nullptr;
                    })) return false;
                    }

                    // Shadow Gallery Frame lives in the shadow block below, on a
                    // third build of this same layout pair (UMBRA_9).
                }

                // ─── Wall Painting Pipelines (framed paintings on indoor walls) ──
                // Uses same bind group layouts as gallery frames (galleryEntity + galleryTexture)
                {
                    wgpu::PipelineLayoutDescriptor pld{};
                    std::array<wgpu::BindGroupLayout, 2> wpGroups = {
                        galleryEntityLayout_, galleryTextureLayout_
                    };
                    pld.bindGroupLayoutCount = wpGroups.size();
                    pld.bindGroupLayouts = wpGroups.data();
                    wgpu::PipelineLayout wpLayout = device_.CreatePipelineLayout(&pld);

                    wgpu::ColorTargetState colorTarget{};
                    colorTarget.format = colorFormat_;
                    colorTarget.writeMask = wgpu::ColorWriteMask::All;

                    wgpu::DepthStencilState wpDepth{};
                    wpDepth.format = depthFormat_;
                    wpDepth.depthWriteEnabled = true;
                    wpDepth.depthCompare = wgpu::CompareFunction::Less;

                    // Canvas pipeline (textured)
                    {
                        wgpu::FragmentState frag{};
                        frag.module = shaderModule_;
                        frag.entryPoint = Entry::WALL_PAINTING_CANVAS_FS;
                        frag.targetCount = 1;
                        frag.targets = &colorTarget;

                        wgpu::RenderPipelineDescriptor desc{};
                        desc.label = "Wall Painting Canvas";
                        desc.layout = wpLayout;
                        desc.vertex.module = shaderModule_;
                        desc.vertex.entryPoint = Entry::WALL_PAINTING_VS;
                        desc.vertex.bufferCount = 0;
                        desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                        desc.primitive.cullMode = wgpu::CullMode::None;  // visible from both sides (outdoor monuments)
                        desc.primitive.frontFace = wgpu::FrontFace::CCW;
                        desc.depthStencil = &wpDepth;
                        desc.fragment = &frag;

                        if constexpr (ROSTER.gallery) {  // ROSTER-GATE gallery (a') — shader compile skipped when disabled
                        if (!tPipe("wall_painting_canvas", [&]() {
                            wallPaintingCanvasPipeline_ = device_.CreateRenderPipeline(&desc);
                            return wallPaintingCanvasPipeline_ != nullptr;
                        })) return false;
                        }
                    }

                    // Frame pipeline (solid color)
                    {
                        wgpu::FragmentState frag{};
                        frag.module = shaderModule_;
                        frag.entryPoint = Entry::WALL_PAINTING_FRAME_FS;
                        frag.targetCount = 1;
                        frag.targets = &colorTarget;

                        wgpu::RenderPipelineDescriptor desc{};
                        desc.label = "Wall Painting Frame";
                        desc.layout = wpLayout;
                        desc.vertex.module = shaderModule_;
                        desc.vertex.entryPoint = Entry::WALL_PAINTING_VS;
                        desc.vertex.bufferCount = 0;
                        desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                        desc.primitive.cullMode = wgpu::CullMode::None;  // visible from both sides (outdoor monuments)
                        desc.primitive.frontFace = wgpu::FrontFace::CCW;
                        desc.depthStencil = &wpDepth;
                        desc.fragment = &frag;

                        if constexpr (ROSTER.gallery) {  // ROSTER-GATE gallery (a') — shader compile skipped when disabled
                        if (!tPipe("wall_painting_frame", [&]() {
                            wallPaintingFramePipeline_ = device_.CreateRenderPipeline(&desc);
                            return wallPaintingFramePipeline_ != nullptr;
                        })) return false;
                        }
                    }

                    // Shadow Wall Painting lives in the shadow block below — ONE
                    // pipeline where the color side needs two (UMBRA_9).
                }

                // ─── Shadow Pipelines (depth-only, Depth32Float) ─────────────────
                // Same bind group layouts as main render, but no fragment shader,
                // no color target, and shadow map depth format.
                {
                    wgpu::DepthStencilState shadowDepth{};
                    shadowDepth.format = kShadowDepthFormat;   // FORMAT_1 D1 — the one authority (state.hpp)
                    shadowDepth.depthWriteEnabled = true;
                    shadowDepth.depthCompare = wgpu::CompareFunction::Less;

                    // BIAS HAS ONE HOME, AND IT IS HERE (UMBRA_6). Every
                    // shadow pipeline below shares this state, so these lines are
                    // the whole of the program's shadow DEPTH bias. (Not the whole
                    // of its bias: PENUMBRA_1 P3 and P5 put a normal offset back
                    // in both samplers. Depth bias and normal offset are
                    // different instruments — see the closing paragraph.)
                    //
                    // Slope-scale does most of the work. It multiplies the
                    // ACTUAL window-space depth gradient of the primitive —
                    // applied after the perspective divide, so it is exact for
                    // the spot path's projection too, not an approximation of
                    // it — and it costs no fragment work at all.
                    //
                    // WHERE THE TWO CURVES AGREE AND WHERE THEY DO NOT. Against
                    // the mix(MAX, MIN, cos) this replaces, the slope term
                    // tracks well through the mid-range and diverges at both
                    // ends (incidence: new/old):
                    //     0deg 0.00 · 30deg 1.11 · 45deg 1.04 · 60deg 1.13
                    //          80deg 2.32 · 85deg 4.26 · 88deg 10.13 · ->inf
                    // At NORMAL incidence the new bias is exactly zero where
                    // the old had a 1.0e-4 floor. At GRAZING it RAN unbounded,
                    // where the old was capped at SHADOW_BIAS_MAX by
                    // construction. Both ends were named because "45 degrees
                    // matches" is true and is not the same claim as "this is
                    // equivalent".
                    //
                    // BOTH ENDS ARE NOW CLOSED. The grazing end by the
                    // depthBiasClamp set below (PENUMBRA_1 P2 — 0.0 had meant
                    // NO clamp, not clamp-at-zero); the normal-incidence end by
                    // the normal offset's FLOOR in sample_shadow_pcf
                    // (PENUMBRA_1 P3). The table above is history that
                    // motivated both, not a description of today.
                    //
                    // THE CONSTANT — HISTORY, kept because the arithmetic is the
                    // reason it is gone and a future editor will otherwise re-add
                    // it. depthBias was DELETED by PENUMBRA_1 P2. Depth32Float
                    // makes depthBias a ULP multiple of the primitive's max
                    // depth, so the exponent matters: the sun frustum is 599.9
                    // deep and the light sits SUN_ALTITUDE = 250 above the
                    // ground, so stored depths cluster near z = 0.417, not near
                    // 1. One ULP there is 2^-25 = 2.98e-8, so depthBias = 2
                    // buys 6.0e-8 NDC = 3.6e-5 world units. (Reading the ULP at
                    // z = 1 would overstate it 4x — the easy error here.)
                    //
                    // The scale of that: restoring the old floor (1.0e-4 NDC)
                    // would have meant depthBias ~= 3355, and the literal
                    // value-at-deletion of SHADOW_BIAS_MAX (2.0e-3, the
                    // resolution-only carry) ~67100. It was never a dial in
                    // steps of one, which is why P2 removed it rather than
                    // tuning it.
                    //
                    // DIRECTION, because it is easy to get backwards: bias
                    // pushes the STORED caster depth away from the light, so
                    // more bias = more lit = weaker shadow. ACNE wants it UP.
                    // DETACHMENT (peter-panning, shadow pulling off contact)
                    // wants it DOWN.
                    //
                    // THE CEILING (PENUMBRA_1 P2). The old shader term did TWO
                    // jobs — a floor AND a ceiling — and UMBRA_6 replaced only
                    // the floor. mix(MAX, MIN, cos) could never exceed
                    // SHADOW_BIAS_MAX by construction; slope-scale with no
                    // clamp WAS unbounded, running 4.3x the old term at 85 deg
                    // and 10.1x at 88 deg. Seven of the eleven shadow pipelines
                    // are CullMode::None, and an unbounded bias on a body whose
                    // faces reach grazing could push a caster clean out of the
                    // depth range so it stopped occluding at all.
                    //
                    // (This sentence used to call those seven "zero-thickness
                    // sheets". It is false and it was mine: PENUMBRA_3 C1 read
                    // the builders and found the RIBBON is a CLOSED capped tube
                    // — 4 faces x 6 verts plus 2 caps x 6 — and the PAWN a closed
                    // solid of revolution with a bottom cap fan. CullMode::None
                    // is a draw decision, not a thickness claim. The correct
                    // discriminator is at the BiasProfile enum below.)
                    //
                    // 2.8e-3 is the old SHADOW_BIAS_MAX carried across
                    // UMBRA_5 CORRECTLY: 8.192/2048 = 4.0e-3 NDC at the old
                    // 0.29297 wu texel is 0.013653 NDC per wu of texel, and
                    // 0.013653 x 0.20508 = 2.8e-3 at today's texel. Note this
                    // is NOT what the retired per-texel form would have given:
                    // 8.192/4096 = 2.0e-3, which is 1.40x short, because that
                    // form tracked RESOLUTION and UMBRA_5 also changed RADIUS.
                    // ECONOMY_1 E6's "carries its bias for free" held for a
                    // resolution ruling alone; UMBRA_5 was not one.
                    //
                    // WHERE SLOPE-SCALE CANNOT REACH, named because the
                    // campaign's own caster diet created it: the shadow pass
                    // draws terrain with the LOD1 index buffer while the main
                    // pass draws near terrain with a LOD0 one, so caster and
                    // receiver are DIFFERENT tessellations of the same
                    // heightfield — the LOD1 mesh is a chord over a 1.5625 wu
                    // span of a surface sampled at 0.78125. In a concave dip
                    // the chord rides above the true surface and the receiver
                    // reads as self-shadowed. Slope-scale cannot compensate
                    // it: it corrects a primitive's own depth gradient, not a
                    // difference between two meshes — and the error is largest
                    // exactly where the slope, and therefore the slope term,
                    // goes to zero. That gap is now covered by the normal
                    // offset's FLOOR (sample_shadow_pcf, PENUMBRA_1 P3), in
                    // texel units, which is the correct unit for it.
                    //
                    // depthBias is DELETED, not set to zero. Under
                    // Depth32Float it is a ULP multiple of the primitive's max
                    // depth: at this scene's z = 0.417 one ULP is 2^-25, so
                    // the old value of 2 bought 6.0e-8 NDC — about 3,355x
                    // short of the floor it was nominally replacing. It is not
                    // a dial in steps of one, and a line that reads like a dial
                    // and is not one costs more than it buys. THE LIVE
                    // INSTRUMENTS ARE slopeScale, depthBiasClamp, AND the normal
                    // offset — whose floor and ceiling are separate rungs.
                    shadowDepth.depthBiasSlopeScale = 2.0f;
                    // THE CEILING IS A WORLD QUANTITY WEARING NDC CLOTHES
                    // (PENUMBRA_2 N2). depthBiasClamp is in depth-buffer
                    // units, so its meaning scales with the sun's depth
                    // range — and N2 widened that range from 599.9 wu to
                    // 1100.0 wu so the caster set fits along the light axis
                    // in every mood. Carrying 2.8e-3 across unchanged would
                    // have silently widened the ceiling from 1.680 wu to
                    // 3.080 wu, 1.83x, with nothing in the diff to show it.
                    //
                    // THE WORLD NUMBER IS THE FACT. Re-derived, not carried:
                    //
                    //     1.680 wu / 1100.0 wu = 1.527e-3 NDC
                    //
                    // where 1.680 wu is the ceiling PENUMBRA_1 P2 arrived at
                    // (SHADOW_BIAS_MAX carried across UMBRA_5 by texel ratio)
                    // and 1100.0 is SUN_FAR - SUN_NEAR in world.wgsl.
                    //
                    // CROSS-ROOM: the divisor lives in WGSL and the quotient
                    // in C++, with nothing but this comment holding them
                    // together — the same shape as the SHADOW_MAP_SIZE twin,
                    // and filed with it in the report's HORIZON. If SUN_NEAR
                    // or SUN_FAR moves, this number is wrong and silent.
                    shadowDepth.depthBiasClamp      = 1.527e-3f;   // = 1.680 wu / 1100.0 wu

                    // THE SHARED BUILDER (shadow/depth category): the builder every shadow pipeline
                    // shares — shadowRenderLayout + shadowDepth (Depth32Float shadow map) +
                    // NO fragment (depth-only) + TriangleList + CCW. It is a SEPARATE builder
                    // from makeEntity (not one with an isShadow flag): color-vs-depth is a
                    // real category boundary (different layout, different depth state, no FS).
                    // Forks are parameters: shadow-VS (verbatim), VBL, cullMode — same
                    // Back/None split as the entity family (single-sided column/palm/cactus/
                    // blade + pawn/ribbon/shell → None; solids → Back).
                    // TWO BIAS PROFILES (PENUMBRA_3 C2). The profile rides the
                    // existing cullMode fork as a DEFAULTED 7th parameter, so the
                    // ten call sites that keep SOLID stay byte-identical. It must
                    // sit after `out` and not beside `cull`: default arguments are
                    // trailing, and `out` has none.
                    //
                    // WHY TWO. depthBiasClamp bounds how far slope-scale may push a
                    // caster. On terrain that bound is never reached — cell caps are
                    // gently sloped, so the slope term stays far below it. On a body
                    // whose faces reach GRAZING it is reached constantly, and it
                    // RELEASES as the face turns away, so the shadow edge steps in
                    // and out along the body. That is the serration the gate reports
                    // on cubes and the ribbon, and it is why terrain improved while
                    // they did not: they are the bodies that reach the ceiling.
                    //
                    // A tighter ceiling is the right direction for them and not a
                    // compromise. A body that reaches grazing gains nothing from a
                    // large bias — the receiver-side normal offset in
                    // sample_shadow_pcf already covers its self-shadowing — while it
                    // pays the full displacement.
                    //
                    // THE CLASSIFICATION IS NOT cullMode, AND NOT sheet-vs-solid.
                    // Both proxies were tried and both are wrong here (C1):
                    //   · cullMode == None misses the cube entirely — shadow_monolith
                    //     is Back, because a cube IS closed.
                    //   · "thin sheet" is false for the two loudest cases: the ribbon
                    //     is a CLOSED capped tube and the pawn a closed solid of
                    //     revolution, both drawn None.
                    // The real discriminator C1 found is that the serrating bodies are
                    // built from NON-PLANAR QUADS SPLIT INTO TWO TRIANGLES — the cube
                    // by independent corner jitter (J = 0.06, giving up to 5.04
                    // degrees between the two triangles of one face), the ribbon by
                    // twist. Slope-scale is computed PER TRIANGLE, so the two halves
                    // of one visual face disagree, and the ceiling bounds how far that
                    // disagreement can travel. Hence: terrain keeps the loose ceiling
                    // because it is the one body with a large gently-sloped area that
                    // never reaches it; everything else takes the tight one.
                    enum class BiasProfile { SOLID, GRAZING };

                    // THE LAYOUT FORK (UMBRA_9). Trailing and defaulted, so the ten
                    // call sites that take shadowRenderLayout stay byte-identical;
                    // the gallery's two pass their own. It rides BEHIND `profile`
                    // for the same reason `profile` rides behind `out`: default
                    // arguments are trailing.
                    //
                    // NULL IS THE SENTINEL FOR shadowRenderLayout, and it must be —
                    // the layout cannot be spelled as the default argument itself.
                    // shadowRenderLayout is a local of the enclosing function, and a
                    // lambda's default argument sits in the lambda's PARAMETER scope
                    // rather than its block, where an enclosing local is not
                    // odr-usable ([basic.def.odr]/10). Both GCC and Clang reject
                    // `= shadowRenderLayout` outright; MSVC accepts it only outside
                    // /permissive-. The resolution is one line, at desc.layout below.
                    auto makeShadow = [&](const char* label, const char* dbgLabel, const char* vsEntry,
                                          const wgpu::VertexBufferLayout* vbl, wgpu::CullMode cull,
                                          wgpu::RenderPipeline& out,
                                          BiasProfile profile = BiasProfile::GRAZING,
                                          wgpu::PipelineLayout layout = nullptr) -> bool {
                        // Body-local, so its address is valid for exactly as long as
                        // `desc`'s is — and CreateRenderPipeline is SYNCHRONOUS here
                        // (tPipe invokes its closure immediately;
                        // CreateRenderPipelineAsync is absent repo-wide), so nothing
                        // outlives this call. Starts from the shared state, so the
                        // format/write/compare triple has one home still.
                        wgpu::DepthStencilState ds = shadowDepth;
                        if (profile == BiasProfile::GRAZING) {
                            // 0.300 wu / 1100.0 wu. World units are the fact; the NDC
                            // number is derived against SUN_FAR - SUN_NEAR, exactly as
                            // the SOLID ceiling is. 0.300 wu is 37% of the 0.820 wu
                            // visible penumbra, so what shift survives stays inside
                            // the blur meant to hide it.
                            ds.depthBiasSlopeScale = 0.5f;
                            ds.depthBiasClamp      = 2.727e-4f;
                        }

                        wgpu::RenderPipelineDescriptor desc{};
                        desc.label = dbgLabel;
                        desc.layout = layout ? layout : shadowRenderLayout;
                        desc.vertex.module = shaderModule_;
                        desc.vertex.entryPoint = vsEntry;
                        desc.vertex.bufferCount = vbl ? 1u : 0u;
                        desc.vertex.buffers = vbl;
                        desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                        desc.primitive.cullMode = cull;
                        desc.primitive.frontFace = wgpu::FrontFace::CCW;
                        desc.depthStencil = &ds;
                        desc.fragment = nullptr;
                        return tPipe(label, [&]() {
                            out = device_.CreateRenderPipeline(&desc);
                            return out != nullptr;
                        });
                    };

                    // MeshVertex layout (pos+normal) for sphere shadow
                    std::array<wgpu::VertexAttribute, 2> shadowMeshAttrs{};
                    shadowMeshAttrs[0].format = wgpu::VertexFormat::Float32x3;
                    shadowMeshAttrs[0].offset = 0;
                    shadowMeshAttrs[0].shaderLocation = 0;
                    shadowMeshAttrs[1].format = wgpu::VertexFormat::Float32x3;
                    shadowMeshAttrs[1].offset = 12;
                    shadowMeshAttrs[1].shaderLocation = 1;

                    wgpu::VertexBufferLayout shadowMeshVBL{};
                    shadowMeshVBL.arrayStride = 24;
                    shadowMeshVBL.stepMode = wgpu::VertexStepMode::Vertex;
                    shadowMeshVBL.attributeCount = shadowMeshAttrs.size();
                    shadowMeshVBL.attributes = shadowMeshAttrs.data();

                    // Shadow patch-terrain (bufferless, Back) + pawn (bufferless, None).
                    // The one SOLID: terrain's cell caps are gently sloped and never
                    // reach the ceiling, and it is the body whose gate already passed.
                    // GoL slabs, pyramids and the 8 wu patch skirt ride this pipeline
                    // too and DO present vertical faces (C1) — flagged, not split,
                    // because no measurement has asked and one pipeline cannot carry
                    // two profiles.
                    if (!makeShadow("shadow_patch_terrain", "Shadow Patch Terrain", Entry::SHADOW_PATCH_TERRAIN_VS,
                        nullptr, wgpu::CullMode::Back, shadowPatchTerrainPipeline_,
                        BiasProfile::SOLID)) return false;
                    if (!makeShadow("shadow_pawn", "Shadow Pawn", Entry::SHADOW_PAWN_VS,
                        nullptr, wgpu::CullMode::None, shadowPawnPipeline_)) return false;

                    // Shadow sphere + monolith (MeshVertex, Back).
                    if constexpr (ROSTER.sphere) {  // ROSTER-GATE sphere (a') — shader compile skipped when disabled
                    if (!makeShadow("shadow_sphere", "Shadow Sphere", Entry::SHADOW_SPHERE_VS,
                        &shadowMeshVBL, wgpu::CullMode::Back, shadowSpherePipeline_)) return false;
                    }
                    if constexpr (ROSTER.cube) {  // ROSTER-GATE cube (a') — shader compile skipped when disabled
                    if (!makeShadow("shadow_monolith", "Shadow Monolith", Entry::SHADOW_MONOLITH_VS,
                        &shadowMeshVBL, wgpu::CullMode::Back, shadowMonolithPipeline_)) return false;
                    }

                    // Shadow Arch (ArchVertex buffer: pos+normal+color+arch_index, stride 40)
                    {
                        std::array<wgpu::VertexAttribute, 4> shadowArchAttrs{};
                        shadowArchAttrs[0].format = wgpu::VertexFormat::Float32x3;
                        shadowArchAttrs[0].offset = 0;
                        shadowArchAttrs[0].shaderLocation = 0;
                        shadowArchAttrs[1].format = wgpu::VertexFormat::Float32x3;
                        shadowArchAttrs[1].offset = 12;
                        shadowArchAttrs[1].shaderLocation = 1;
                        shadowArchAttrs[2].format = wgpu::VertexFormat::Float32x3;
                        shadowArchAttrs[2].offset = 24;
                        shadowArchAttrs[2].shaderLocation = 2;
                        shadowArchAttrs[3].format = wgpu::VertexFormat::Float32;
                        shadowArchAttrs[3].offset = 36;
                        shadowArchAttrs[3].shaderLocation = 3;

                        wgpu::VertexBufferLayout shadowArchVBL{};
                        shadowArchVBL.arrayStride = 40;
                        shadowArchVBL.stepMode = wgpu::VertexStepMode::Vertex;
                        shadowArchVBL.attributeCount = shadowArchAttrs.size();
                        shadowArchVBL.attributes = shadowArchAttrs.data();

                        // arch/column/palm/cactus/blade shadows — same ArchVertex
                        // format; cull matches the color pass (arch Back, the
                        // single-sided column/palm/cactus/blade None). pyramid shadow cut.
                        if constexpr (ROSTER.arch) {  // ROSTER-GATE arch (a') — shader compile skipped when disabled
                        if (!makeShadow("shadow_arch", "Shadow Catenary Arch", Entry::SHADOW_ARCH_VS,
                            &shadowArchVBL, wgpu::CullMode::Back, shadowArchPipeline_)) return false;
                        }
                        if constexpr (ROSTER.column || ROSTER.antenna) {  // ROSTER-GATE column+antenna (shared pipelines) (a') — shader compile skipped when disabled
                        if (!makeShadow("shadow_column", "Shadow Generative Column", Entry::SHADOW_COLUMN_VS,
                            &shadowArchVBL, wgpu::CullMode::None, shadowColumnPipeline_)) return false;
                        }
                        if constexpr (ROSTER.palm) {  // ROSTER-GATE palm (a') — shader compile skipped when disabled
                        if (!makeShadow("shadow_palm", "Shadow Palm Tree", Entry::SHADOW_PALM_VS,
                            &shadowArchVBL, wgpu::CullMode::None, shadowPalmPipeline_)) return false;
                        }
                        if constexpr (ROSTER.cactus) {  // ROSTER-GATE cactus (a') — shader compile skipped when disabled
                        if (!makeShadow("shadow_cactus", "Shadow Cactus", Entry::SHADOW_CACTUS_VS,
                            &shadowArchVBL, wgpu::CullMode::None, shadowCactusPipeline_)) return false;
                        }
                        if constexpr (ROSTER.blade) {  // ROSTER-GATE blade (a') — shader compile skipped when disabled
                        if (!makeShadow("shadow_blade", "Shadow Blade Cluster", Entry::SHADOW_BLADE_VS,
                            &shadowArchVBL, wgpu::CullMode::None, shadowBladePipeline_)) return false;
                        }
                        // shadow_pyramid pipeline CUT — mesh never drawn
                    }

                    // Shadow Shell (ShellVertex: pos+normal+color, stride 36)
                    {
                        std::array<wgpu::VertexAttribute, 3> shadowShellAttrs{};
                        shadowShellAttrs[0].format = wgpu::VertexFormat::Float32x3;
                        shadowShellAttrs[0].offset = 0;
                        shadowShellAttrs[0].shaderLocation = 0;
                        shadowShellAttrs[1].format = wgpu::VertexFormat::Float32x3;
                        shadowShellAttrs[1].offset = 12;
                        shadowShellAttrs[1].shaderLocation = 1;
                        shadowShellAttrs[2].format = wgpu::VertexFormat::Float32x3;
                        shadowShellAttrs[2].offset = 24;
                        shadowShellAttrs[2].shaderLocation = 2;

                        wgpu::VertexBufferLayout shadowShellVBL{};
                        shadowShellVBL.arrayStride = 36;
                        shadowShellVBL.stepMode = wgpu::VertexStepMode::Vertex;
                        shadowShellVBL.attributeCount = shadowShellAttrs.size();
                        shadowShellVBL.attributes = shadowShellAttrs.data();

                        // ROSTER-GATE indoor_shell (a) — SEPARABLE: skip the
                        // shadow-shell pipeline too. draw_shadow_shell self-gates
                        // on count==0 (shared helper's early-out).
                        if constexpr (ROSTER.indoor_shell) {  // ROSTER-GATE indoor_shell (a') — shader compile skipped when disabled
                        if (!makeShadow("shadow_shell", "Shadow Indoor Shell", Entry::SHADOW_SHELL_VS,
                            &shadowShellVBL, wgpu::CullMode::None, shadowShellPipeline_)) return false;
                        }
                    }

                    // Shadow ribbon (bufferless, GPU-generated from vertex_index; None).
                    if constexpr (ROSTER.ribbon) {  // ROSTER-GATE ribbon (a') — shader compile skipped when disabled
                    if (!makeShadow("shadow_ribbon", "Shadow Sky Ribbon", Entry::SHADOW_RIBBON_VS,
                        nullptr, wgpu::CullMode::None, shadowRibbonPipeline_)) return false;
                    }

                    // Shadow gallery frame + wall painting (both bufferless, None —
                    // the color side's mode, so the caster silhouette is the drawn
                    // body's). A body that is DRAWN is a body that CASTS; there was
                    // never an artwork exception, only an artwork omission.
                    //
                    // THE GALLERY PIPELINE LAYOUT, THIRD INSTANCE. galleryLayout and
                    // wpLayout are locals of their own pipeline blocks above and do
                    // not reach here, so this rebuilds the SAME pair of bind group
                    // layouts — layout-compatible with the two color families, and
                    // the whole reason this campaign is small: the gallery entity
                    // group already binds vpBuffer_ at sizeof(GPUVPMatrix), so
                    // render_vp.light_vp is already reachable, and the gallery
                    // texture group binds no shadow map, so it is already legal
                    // inside a depth-only pass. ZERO new bindings, ZERO new
                    // bind-group layouts. Do not grow either one.
                    if constexpr (ROSTER.gallery) {  // ROSTER-GATE gallery (a') — shader compile skipped when disabled
                    // ATLAS_1revB G2 — group 0 is the RENDER-ENTITY layout here,
                    // not the gallery entity layout. Under D2'/D3" these two
                    // shadow VSes call shadow_light_vp(), which reads
                    // render_lighting and shadow_slot; the gallery entity layout
                    // carries neither, so Dawn would reject both pipelines at
                    // creation. It is a strict subset for everything they DO
                    // use — config (Uniform), render_vp and render_camera (both
                    // ReadOnlyStorage) are present on the render-entity layout
                    // with the same types — so nothing is lost by the swap, and
                    // draw_shadow_all sheds a bind per light because group 0 no
                    // longer changes mid-tile. Group 1 is untouched: the
                    // painting slots and array still come from the gallery
                    // texture layout. The COLOUR gallery pipelines keep the
                    // gallery entity layout; only the shadow pair moves.
                    std::array<wgpu::BindGroupLayout, 2> galleryShadowGroups = {
                        renderEntityLayout_, galleryTextureLayout_
                    };
                    wgpu::PipelineLayoutDescriptor galleryShadowPld{};
                    galleryShadowPld.bindGroupLayoutCount = galleryShadowGroups.size();
                    galleryShadowPld.bindGroupLayouts = galleryShadowGroups.data();
                    wgpu::PipelineLayout galleryShadowLayout = device_.CreatePipelineLayout(&galleryShadowPld);
                    if (!galleryShadowLayout) return false;

                    if (!makeShadow("shadow_gallery_frame", "Shadow Gallery Frame",
                        Entry::SHADOW_GALLERY_FRAME_VS, nullptr,
                        wgpu::CullMode::None, shadowGalleryFramePipeline_,
                        BiasProfile::GRAZING, galleryShadowLayout)) return false;
                    if (!makeShadow("shadow_wall_painting", "Shadow Wall Painting",
                        Entry::SHADOW_WALL_PAINTING_VS, nullptr,
                        wgpu::CullMode::None, shadowWallPaintingPipeline_,
                        BiasProfile::GRAZING, galleryShadowLayout)) return false;
                    }

                }

                // ─── Fade Overlay Pipeline ───────────────────────────────────────
                // Fullscreen triangle, alpha blending, no depth write.
                // Uses meshGenEntityLayout_ (binding 1 = config only).
                {
                    wgpu::PipelineLayoutDescriptor pld{};
                    std::array<wgpu::BindGroupLayout, 1> groups = { meshGenEntityLayout_ };
                    pld.bindGroupLayoutCount = groups.size();
                    pld.bindGroupLayouts = groups.data();
                    wgpu::PipelineLayout layout = device_.CreatePipelineLayout(&pld);

                    wgpu::ColorTargetState colorTarget{};
                    colorTarget.format = colorFormat_;
                    colorTarget.writeMask = wgpu::ColorWriteMask::All;

                    wgpu::BlendState blend{};
                    blend.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
                    blend.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
                    blend.alpha.srcFactor = wgpu::BlendFactor::One;
                    blend.alpha.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
                    colorTarget.blend = &blend;

                    wgpu::FragmentState frag{};
                    frag.module = shaderModule_;
                    frag.entryPoint = Entry::FADE_OVERLAY_FS;
                    frag.targetCount = 1;
                    frag.targets = &colorTarget;

                    wgpu::RenderPipelineDescriptor desc{};
                    desc.label = "Fade Overlay";
                    desc.layout = layout;
                    desc.vertex.module = shaderModule_;
                    desc.vertex.entryPoint = Entry::FADE_OVERLAY_VS;
                    desc.vertex.bufferCount = 0;
                    desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                    desc.fragment = &frag;

                    wgpu::DepthStencilState fadeDepth{};
                    fadeDepth.format = depthFormat_;
                    fadeDepth.depthWriteEnabled = false;
                    fadeDepth.depthCompare = wgpu::CompareFunction::Always;
                    desc.depthStencil = &fadeDepth;

                    if constexpr (ROSTER.transitions) {  // ROSTER-GATE transitions (a') — shader compile skipped when disabled
                    if (!tPipe("fade_overlay", [&]() {
                        fadeOverlayPipeline_ = device_.CreateRenderPipeline(&desc);
                        return fadeOverlayPipeline_ != nullptr;
                    })) return false;
                    }
                }

                return true;
            }
        };

    } // namespace the_board
} // namespace t7
