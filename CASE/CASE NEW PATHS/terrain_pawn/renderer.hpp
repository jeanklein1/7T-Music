#pragma once

/**
 * TERRAIN PAWN - Renderer
 * =======================
 *
 * Pipeline management for terrain + pawn visualization.
 *
 * GPU-NATIVE DESIGN
 * -----------------
 *
 * No mesh buffers. Geometry is generated via vertex pulling:
 * - Terrain: vertex shader computes position from vertex_index
 * - Pawn: vertex shader computes cone geometry from vertex_index
 *
 * CPU role: create pipelines, issue draw calls with vertex counts.
 * GPU role: everything else.
 */

#include "cartridges/terrain_pawn/state.hpp"
#include <webgpu/webgpu_cpp.h>
#include <string>
#include <fstream>
#include <sstream>
#include <array>
#include <iostream>

namespace t7 {
    namespace terrain_pawn {

        // Vertex counts for GPU-generated geometry (must match WGSL constants)
        constexpr uint32_t TERRAIN_RESOLUTION = 200;
        constexpr uint32_t TERRAIN_VERTEX_COUNT = TERRAIN_RESOLUTION * TERRAIN_RESOLUTION * 6;  // 6 verts per cell
        constexpr uint32_t PAWN_SEGMENTS = 16;
        constexpr uint32_t PAWN_VERTEX_COUNT = PAWN_SEGMENTS * 3 * 2;  // body + cap

        class Renderer {
        public:
            Renderer() = default;

            /**
             * Initialize the renderer.
             */
            bool init(
                wgpu::Device device,
                wgpu::BindGroupLayout computeBindGroupLayout,
                wgpu::BindGroupLayout renderBindGroupLayout,

                wgpu::TextureFormat colorFormat,
                wgpu::TextureFormat depthFormat
            ) {
                device_ = device;
                computeBindGroupLayout_ = computeBindGroupLayout;
                renderBindGroupLayout_ = renderBindGroupLayout;
                colorFormat_ = colorFormat;
                depthFormat_ = depthFormat;

                if (!loadShader()) return false;
                if (!createComputePipeline()) return false;
                if (!createTerrainPipeline()) return false;
                if (!createPawnPipeline()) return false;

                std::cout << "[terrain_pawn] Renderer initialized\n";
                return true;
            }

            /**
             * Dispatch the compute shader.
             */
            void dispatch_update(wgpu::ComputePassEncoder& pass, wgpu::BindGroup bindGroup) {
                pass.SetPipeline(computePipeline_);
                pass.SetBindGroup(0, bindGroup);
                pass.DispatchWorkgroups(1);
            }

            /**
             * Draw the terrain (vertex pulling, no buffers).
             */
            void draw_terrain(wgpu::RenderPassEncoder& pass, wgpu::BindGroup bindGroup) {
                pass.SetPipeline(terrainPipeline_);
                pass.SetBindGroup(0, bindGroup);
                pass.Draw(TERRAIN_VERTEX_COUNT);
            }

            /**
             * Draw the pawn (vertex pulling, no buffers).
             */
            void draw_pawn(wgpu::RenderPassEncoder& pass, wgpu::BindGroup bindGroup) {
                pass.SetPipeline(pawnPipeline_);
                pass.SetBindGroup(0, bindGroup);
                pass.Draw(PAWN_VERTEX_COUNT);
            }

        private:
            wgpu::Device device_;
            wgpu::BindGroupLayout computeBindGroupLayout_;
            wgpu::BindGroupLayout renderBindGroupLayout_;
            wgpu::TextureFormat colorFormat_;
            wgpu::TextureFormat depthFormat_;

            wgpu::ShaderModule shaderModule_;
            std::string shaderPath_;

            // Pipelines
            wgpu::ComputePipeline computePipeline_;
            wgpu::RenderPipeline terrainPipeline_;
            wgpu::RenderPipeline pawnPipeline_;

            // ─── Shader Loading ─────────────────────────────────────────────────────

            bool loadShader() {
                // Try multiple paths for different working directories
                std::array<const char*, 4> paths = {
                    "src/cartridges/terrain_pawn/world.wgsl",
                    "cartridges/terrain_pawn/world.wgsl",
                    "../src/cartridges/terrain_pawn/world.wgsl",
                    "world.wgsl"
                };

                std::string shaderSource;
                for (const char* path : paths) {
                    std::ifstream file(path);
                    if (file.is_open()) {
                        std::stringstream buffer;
                        buffer << file.rdbuf();
                        shaderSource = buffer.str();
                        shaderPath_ = path;
                        std::cout << "[terrain_pawn] Loaded shader from: " << path << "\n";
                        break;
                    }
                }

                if (shaderSource.empty()) {
                    std::cerr << "[terrain_pawn] Could not find world.wgsl\n";
                    return false;
                }

                wgpu::ShaderSourceWGSL wgslSource{};
                wgslSource.code = shaderSource.c_str();

                wgpu::ShaderModuleDescriptor desc{};
                desc.nextInChain = &wgslSource;
                desc.label = "terrain_pawn world.wgsl";

                shaderModule_ = device_.CreateShaderModule(&desc);
                return shaderModule_ != nullptr;
            }

            // ─── Pipeline Creation ──────────────────────────────────────────────────

            bool createComputePipeline() {
                wgpu::PipelineLayoutDescriptor layoutDesc{};
                layoutDesc.bindGroupLayoutCount = 1;
                layoutDesc.bindGroupLayouts = &computeBindGroupLayout_;
                wgpu::PipelineLayout layout = device_.CreatePipelineLayout(&layoutDesc);

                wgpu::ComputePipelineDescriptor desc{};
                desc.label = "World Update";
                desc.layout = layout;
                desc.compute.module = shaderModule_;
                desc.compute.entryPoint = "update";

                computePipeline_ = device_.CreateComputePipeline(&desc);
                return computePipeline_ != nullptr;
            }

            bool createTerrainPipeline() {
                wgpu::PipelineLayoutDescriptor layoutDesc{};
                layoutDesc.bindGroupLayoutCount = 1;
                layoutDesc.bindGroupLayouts = &renderBindGroupLayout_;
                wgpu::PipelineLayout layout = device_.CreatePipelineLayout(&layoutDesc);

                // No vertex buffers - vertex pulling from vertex_index

                // Fragment state
                wgpu::ColorTargetState colorTarget{};
                colorTarget.format = colorFormat_;
                colorTarget.writeMask = wgpu::ColorWriteMask::All;

                wgpu::FragmentState fragment{};
                fragment.module = shaderModule_;
                fragment.entryPoint = "terrain_fs";
                fragment.targetCount = 1;
                fragment.targets = &colorTarget;

                // Depth state
                wgpu::DepthStencilState depthStencil{};
                depthStencil.format = depthFormat_;
                depthStencil.depthWriteEnabled = true;
                depthStencil.depthCompare = wgpu::CompareFunction::Less;

                // Pipeline
                wgpu::RenderPipelineDescriptor desc{};
                desc.label = "Terrain";
                desc.layout = layout;
                desc.vertex.module = shaderModule_;
                desc.vertex.entryPoint = "terrain_vs";
                desc.vertex.bufferCount = 0;  // No vertex buffers
                desc.vertex.buffers = nullptr;
                desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                desc.primitive.cullMode = wgpu::CullMode::Back;
                desc.depthStencil = &depthStencil;
                desc.fragment = &fragment;

                terrainPipeline_ = device_.CreateRenderPipeline(&desc);
                return terrainPipeline_ != nullptr;
            }

            bool createPawnPipeline() {
                wgpu::PipelineLayoutDescriptor layoutDesc{};
                layoutDesc.bindGroupLayoutCount = 1;
                layoutDesc.bindGroupLayouts = &renderBindGroupLayout_;
                wgpu::PipelineLayout layout = device_.CreatePipelineLayout(&layoutDesc);

                // No vertex buffers - vertex pulling from vertex_index

                // Fragment state
                wgpu::ColorTargetState colorTarget{};
                colorTarget.format = colorFormat_;
                colorTarget.writeMask = wgpu::ColorWriteMask::All;

                wgpu::FragmentState fragment{};
                fragment.module = shaderModule_;
                fragment.entryPoint = "pawn_fs";
                fragment.targetCount = 1;
                fragment.targets = &colorTarget;

                // Depth state
                wgpu::DepthStencilState depthStencil{};
                depthStencil.format = depthFormat_;
                depthStencil.depthWriteEnabled = true;
                depthStencil.depthCompare = wgpu::CompareFunction::Less;

                // Pipeline
                wgpu::RenderPipelineDescriptor desc{};
                desc.label = "Pawn";
                desc.layout = layout;
                desc.vertex.module = shaderModule_;
                desc.vertex.entryPoint = "pawn_vs";
                desc.vertex.bufferCount = 0;  // No vertex buffers
                desc.vertex.buffers = nullptr;
                desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                desc.primitive.cullMode = wgpu::CullMode::Back;
                desc.depthStencil = &depthStencil;
                desc.fragment = &fragment;

                pawnPipeline_ = device_.CreateRenderPipeline(&desc);
                return pawnPipeline_ != nullptr;
            }
        };

    } // namespace terrain_pawn
} // namespace t7