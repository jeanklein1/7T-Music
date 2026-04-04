#pragma once

/**
 * N-DIMENSIONAL_4 CARTRIDGE — Renderer
 * =====================================
 *
 * Pipeline management for trajectory-driven cell grid with sphere entity.
 *
 * PIPELINES:
 * ┌────────────────────┬─────────────┬──────────────────────────────┐
 * │ Pipeline           │ Dimension   │ Writes                       │
 * ├────────────────────┼─────────────┼──────────────────────────────┤
 * │ updateWorld        │ 0D (1,1,1)  │ ground, pawn, camera, sphere │
 * │ updateHeightField  │ 2D (32²)    │ height_field texture         │
 * │ syncLifeState      │ 2D (8²)     │ cells buffer (life field)    │
 * │ updateCells        │ 2D (8²)     │ cells buffer                 │
 * │ updateCellHeights  │ 2D (8²)     │ cell_height_field texture    │
 * │ updateTiles        │ 2D (8²)     │ tile_state texture           │
 * │ render             │ fullscreen  │ backbuffer                   │
 * └────────────────────┴─────────────┴──────────────────────────────┘
 *
 * EXECUTION ORDER:
 *   1. update_world        — entities, global trajectories
 *   2. update_height_field — terrain elevation (waves)
 *   3. sync_life_state     — Game of Life synchronization
 *   4. update_cells        — evolve color trajectories
 *   5. update_cell_heights — cell-driven terrain geometry
 *   6. update_tiles        — copy cell.value to texture
 *   7. render              — fullscreen raymarch
 *
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include "cartridges/n_dimensional_4/state.hpp"
#include <webgpu/webgpu_cpp.h>
#include <string>
#include <fstream>
#include <sstream>
#include <array>
#include <iostream>

namespace t7 {
    namespace n_dimensional_4 {


        // ═══════════════════════════════════════════════════════════════════════════════
        // §1 ENTRY POINTS — Must match world.wgsl §10
        // ═══════════════════════════════════════════════════════════════════════════════

        namespace Entry {
            // Compute (ordered by dependency)
            constexpr const char* UPDATE_WORLD = "update_world";         // 0D
            constexpr const char* UPDATE_HEIGHT_FIELD = "update_height_field";  // 2D
            constexpr const char* SYNC_LIFE_STATE = "sync_life_state";   // 2D
            constexpr const char* UPDATE_CELLS = "update_cells";         // 2D
            constexpr const char* UPDATE_CELL_HEIGHTS = "update_cell_heights";  // 2D
            constexpr const char* UPDATE_TILES = "update_tiles";         // 2D

            // Render
            constexpr const char* FULLSCREEN_VS = "fullscreen_vs";
            constexpr const char* WORLD_FS = "world_fs";
        }


        // ═══════════════════════════════════════════════════════════════════════════════
        // §2 RENDERER CLASS
        // ═══════════════════════════════════════════════════════════════════════════════

        class Renderer {
        public:
            Renderer() = default;

            bool init(
                wgpu::Device device,
                wgpu::BindGroupLayout computeEntityLayout,
                wgpu::BindGroupLayout computeTextureLayout,
                wgpu::BindGroupLayout renderEntityLayout,
                wgpu::BindGroupLayout renderTextureLayout,
                wgpu::TextureFormat colorFormat,
                wgpu::TextureFormat depthFormat
            ) {
                device_ = device;
                computeEntityLayout_ = computeEntityLayout;
                computeTextureLayout_ = computeTextureLayout;
                renderEntityLayout_ = renderEntityLayout;
                renderTextureLayout_ = renderTextureLayout;
                colorFormat_ = colorFormat;
                depthFormat_ = depthFormat;

                if (!loadShader()) return false;
                if (!createComputePipelines()) return false;
                if (!createRenderPipeline()) return false;

                return true;
            }

            // ─── Compute Dispatch ───────────────────────────────────────────────────
            //
            // ORDER MATTERS: Each pass may read what previous pass wrote.
            //   1. update_world        — 0D state (reads signal, writes entities)
            //   2. update_height_field — 2D texture (reads ground_state)
            //   3. sync_life_state     — 2D sync (reads cells.life_next, writes cells.life)
            //   4. update_cells        — 2D cell trajectories (reads entities)
            //   5. update_tiles        — 2D tile texture (reads cells)

            void dispatch_update_world(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup entityBindGroup,
                wgpu::BindGroup textureBindGroup
            ) {
                pass.SetPipeline(updateWorldPipeline_);
                pass.SetBindGroup(0, entityBindGroup);
                pass.SetBindGroup(1, textureBindGroup);
                pass.DispatchWorkgroups(1, 1, 1);  // 0D: single invocation
            }

            void dispatch_update_height_field(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup entityBindGroup,
                wgpu::BindGroup textureBindGroup,
                uint32_t workgroups
            ) {
                pass.SetPipeline(updateHeightFieldPipeline_);
                pass.SetBindGroup(0, entityBindGroup);
                pass.SetBindGroup(1, textureBindGroup);
                pass.DispatchWorkgroups(workgroups, workgroups, 1);  // 2D: 32×32 workgroups of 8×8
            }

            void dispatch_sync_life_state(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup entityBindGroup,
                wgpu::BindGroup textureBindGroup,
                uint32_t workgroups
            ) {
                pass.SetPipeline(syncLifePipeline_);
                pass.SetBindGroup(0, entityBindGroup);
                pass.SetBindGroup(1, textureBindGroup);
                pass.DispatchWorkgroups(workgroups, workgroups, 1);  // 2D: 8×8 workgroups of 8×8 (cell grid)
            }

            void dispatch_update_cells(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup entityBindGroup,
                wgpu::BindGroup textureBindGroup,
                uint32_t workgroups
            ) {
                pass.SetPipeline(updateCellsPipeline_);
                pass.SetBindGroup(0, entityBindGroup);
                pass.SetBindGroup(1, textureBindGroup);
                pass.DispatchWorkgroups(workgroups, workgroups, 1);  // 2D: 8×8 workgroups of 8×8
            }

            void dispatch_update_cell_heights(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup entityBindGroup,
                wgpu::BindGroup textureBindGroup,
                uint32_t workgroups
            ) {
                pass.SetPipeline(updateCellHeightsPipeline_);
                pass.SetBindGroup(0, entityBindGroup);
                pass.SetBindGroup(1, textureBindGroup);
                pass.DispatchWorkgroups(workgroups, workgroups, 1);  // 2D: 8×8 workgroups of 8×8
            }

            void dispatch_update_tiles(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup entityBindGroup,
                wgpu::BindGroup textureBindGroup,
                uint32_t workgroups
            ) {
                pass.SetPipeline(updateTilesPipeline_);
                pass.SetBindGroup(0, entityBindGroup);
                pass.SetBindGroup(1, textureBindGroup);
                pass.DispatchWorkgroups(workgroups, workgroups, 1);  // 2D: 8×8 workgroups of 8×8
            }

            // ─── Render ─────────────────────────────────────────────────────────────
            //
            // Fullscreen triangle. Fragment shader raymarches, samples precomputed textures.

            void draw_world(
                wgpu::RenderPassEncoder& pass,
                wgpu::BindGroup entityBindGroup,
                wgpu::BindGroup textureBindGroup
            ) {
                pass.SetPipeline(renderPipeline_);
                pass.SetBindGroup(0, entityBindGroup);
                pass.SetBindGroup(1, textureBindGroup);
                pass.Draw(3);  // Fullscreen triangle
            }

            // ─── Hot Reload ─────────────────────────────────────────────────────────
            //
            // Called by incubator when .wgsl file changes on disk.
            // Reloads shader and recreates all pipelines.

            bool reload() {
                if (!loadShader()) return false;
                if (!createComputePipelines()) return false;
                if (!createRenderPipeline()) return false;
                std::cout << "[Hot Reload] Shader reloaded successfully\n";
                return true;
            }

            const std::string& shader_path() const { return shaderPath_; }

        private:
            wgpu::Device device_;
            wgpu::BindGroupLayout computeEntityLayout_;
            wgpu::BindGroupLayout computeTextureLayout_;
            wgpu::BindGroupLayout renderEntityLayout_;
            wgpu::BindGroupLayout renderTextureLayout_;
            wgpu::TextureFormat colorFormat_;
            wgpu::TextureFormat depthFormat_;

            wgpu::ShaderModule shaderModule_;
            std::string shaderSource_;
            std::string shaderPath_;  // For hot reload: path to .wgsl file

            // ─── Pipelines ──────────────────────────────────────────────────────────
            wgpu::ComputePipeline updateWorldPipeline_;       // 0D
            wgpu::ComputePipeline updateHeightFieldPipeline_; // 2D
            wgpu::ComputePipeline syncLifePipeline_;          // 2D (GoL sync)
            wgpu::ComputePipeline updateCellsPipeline_;       // 2D (NEW IN ND3)
            wgpu::ComputePipeline updateCellHeightsPipeline_; // 2D (cell-driven terrain)
            wgpu::ComputePipeline updateTilesPipeline_;       // 2D
            wgpu::RenderPipeline renderPipeline_;             // fullscreen

            // ─────────────────────────────────────────────────────────────────────────
            // CREATION METHODS
            // ─────────────────────────────────────────────────────────────────────────

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
                    // First load: search for the shader
                    std::array<const char*, 6> paths = {
                        "../../../src/cartridges/n_dimensional_4/world.wgsl",  // ← ADD THIS (from build dir to src)
                        "src/cartridges/n_dimensional_4/world.wgsl",
          
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
                desc.label = "world.wgsl (N-Dimensional_4 Cartridge)";

                shaderModule_ = device_.CreateShaderModule(&desc);
                return shaderModule_ != nullptr;
            }

            bool createComputePipelines() {
                // ─── Shared pipeline layout for all compute passes ──────────────────
                std::array<wgpu::BindGroupLayout, 2> computeLayouts = {
                    computeEntityLayout_,
                    computeTextureLayout_
                };

                wgpu::PipelineLayoutDescriptor layoutDesc{};
                layoutDesc.bindGroupLayoutCount = computeLayouts.size();
                layoutDesc.bindGroupLayouts = computeLayouts.data();
                wgpu::PipelineLayout computeLayout = device_.CreatePipelineLayout(&layoutDesc);
                if (!computeLayout) return false;

                // ─── Pipeline 1: update_world (0D) ──────────────────────────────────
                //
                // Workgroups: (1, 1, 1)
                // Reads: signal, config
                // Writes: ground_state, pawn_state, camera_state, sphere_state, trajectories
                {
                    wgpu::ComputePipelineDescriptor desc{};
                    desc.label = "Update World (0D)";
                    desc.layout = computeLayout;
                    desc.compute.module = shaderModule_;
                    desc.compute.entryPoint = Entry::UPDATE_WORLD;

                    updateWorldPipeline_ = device_.CreateComputePipeline(&desc);
                    if (!updateWorldPipeline_) return false;
                }

                // ─── Pipeline 2: update_height_field (2D) ───────────────────────────
                //
                // Workgroups: (32, 32, 1) with workgroup_size(8, 8)
                // Reads: signal, ground_state
                // Writes: height_field texture (RGBA16Float)
                {
                    wgpu::ComputePipelineDescriptor desc{};
                    desc.label = "Update Height Field (2D)";
                    desc.layout = computeLayout;
                    desc.compute.module = shaderModule_;
                    desc.compute.entryPoint = Entry::UPDATE_HEIGHT_FIELD;

                    updateHeightFieldPipeline_ = device_.CreateComputePipeline(&desc);
                    if (!updateHeightFieldPipeline_) return false;
                }

                // ─── Pipeline 3: sync_life_state (2D) ───────────────────────────────
                //
                // Workgroups: (8, 8, 1) with workgroup_size(8, 8)
                // Reads: cells buffer (life_next field)
                // Writes: cells buffer (life field)
                // Purpose: Synchronize Game of Life state for deterministic evolution
                {
                    wgpu::ComputePipelineDescriptor desc{};
                    desc.label = "Sync Life State (2D)";
                    desc.layout = computeLayout;
                    desc.compute.module = shaderModule_;
                    desc.compute.entryPoint = Entry::SYNC_LIFE_STATE;

                    syncLifePipeline_ = device_.CreateComputePipeline(&desc);
                    if (!syncLifePipeline_) return false;
                }

                // ─── Pipeline 4: update_cells (2D) ──────────────────────────────────
                //
                // Workgroups: (8, 8, 1) with workgroup_size(8, 8)
                // Reads: signal, ground_state, pawn_state, sphere_state
                // Writes: cells buffer (color trajectories)
                {
                    wgpu::ComputePipelineDescriptor desc{};
                    desc.label = "Update Cells (2D)";
                    desc.layout = computeLayout;
                    desc.compute.module = shaderModule_;
                    desc.compute.entryPoint = Entry::UPDATE_CELLS;

                    updateCellsPipeline_ = device_.CreateComputePipeline(&desc);
                    if (!updateCellsPipeline_) return false;
                }

                // ─── Pipeline 5: update_cell_heights (2D) ────────────────────────────
                //
                // Workgroups: (8, 8, 1) with workgroup_size(8, 8)
                // Reads: cells buffer (life, value.a)
                // Writes: cell_height_field texture (R32Float)
                // Purpose: Generate cell-driven terrain geometry (separate from waves)
                {
                    wgpu::ComputePipelineDescriptor desc{};
                    desc.label = "Update Cell Heights (2D)";
                    desc.layout = computeLayout;
                    desc.compute.module = shaderModule_;
                    desc.compute.entryPoint = Entry::UPDATE_CELL_HEIGHTS;

                    updateCellHeightsPipeline_ = device_.CreateComputePipeline(&desc);
                    if (!updateCellHeightsPipeline_) return false;
                }

                // ─── Pipeline 6: update_tiles (2D) ──────────────────────────────────
                //
                // Workgroups: (8, 8, 1) with workgroup_size(8, 8)
                // Reads: signal, ground_state, pawn_state, sphere_state, trajectory_field, cells
                // Writes: tile_state texture (RGBA8Unorm)
                {
                    wgpu::ComputePipelineDescriptor desc{};
                    desc.label = "Update Tiles (2D)";
                    desc.layout = computeLayout;
                    desc.compute.module = shaderModule_;
                    desc.compute.entryPoint = Entry::UPDATE_TILES;

                    updateTilesPipeline_ = device_.CreateComputePipeline(&desc);
                    if (!updateTilesPipeline_) return false;
                }

                return true;
            }

            bool createRenderPipeline() {
                std::array<wgpu::BindGroupLayout, 2> renderLayouts = {
                    renderEntityLayout_,
                    renderTextureLayout_
                };

                wgpu::PipelineLayoutDescriptor layoutDesc{};
                layoutDesc.bindGroupLayoutCount = renderLayouts.size();
                layoutDesc.bindGroupLayouts = renderLayouts.data();
                wgpu::PipelineLayout layout = device_.CreatePipelineLayout(&layoutDesc);
                if (!layout) return false;

                wgpu::ColorTargetState colorTarget{};
                colorTarget.format = colorFormat_;
                colorTarget.writeMask = wgpu::ColorWriteMask::All;

                wgpu::FragmentState fragment{};
                fragment.module = shaderModule_;
                fragment.entryPoint = Entry::WORLD_FS;
                fragment.targetCount = 1;
                fragment.targets = &colorTarget;

                wgpu::RenderPipelineDescriptor desc{};
                desc.label = "World Raymarch (N-Dimensional_4 Cartridge)";
                desc.layout = layout;
                desc.vertex.module = shaderModule_;
                desc.vertex.entryPoint = Entry::FULLSCREEN_VS;
                desc.vertex.bufferCount = 0;
                desc.vertex.buffers = nullptr;
                desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                desc.primitive.cullMode = wgpu::CullMode::None;
                desc.fragment = &fragment;

                renderPipeline_ = device_.CreateRenderPipeline(&desc);
                return renderPipeline_ != nullptr;
            }
        };

    } // namespace n_dimensional_4
} // namespace t7