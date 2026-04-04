#pragma once

/**
 * PLAYGROUND RASTERIZE — Renderer
 * ================================
 *
 * Minimal pipeline management for mesh viewing.
 * Single compute pass (update_camera), single render pass (mesh rasterization).
 *
 * DIFFERENCE FROM RAYMARCH:
 *   - Render pipeline has vertex buffer layout
 *   - Uses mesh_vs and mesh_fs entry points
 *   - Depth testing enabled (meshes need it)
 *
 * PIPELINES:
 * ┌────────────────────┬─────────────┬──────────────────────────────┐
 * │ Pipeline           │ Dimension   │ Writes                       │
 * ├────────────────────┼─────────────┼──────────────────────────────┤
 * │ updateCamera       │ 0D (1,1,1)  │ camera state                 │
 * │ render             │ per-vertex  │ backbuffer + depth           │
 * └────────────────────┴─────────────┴──────────────────────────────┘
 *
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include "cartridges/playground_rasterize/state.hpp"
#include <webgpu/webgpu_cpp.h>
#include <string>
#include <fstream>
#include <sstream>
#include <array>
#include <iostream>

namespace t7 {
    namespace playground_rasterize {


        // ═══════════════════════════════════════════════════════════════════════════════
        // §1 ENTRY POINTS — Must match world.wgsl
        // ═══════════════════════════════════════════════════════════════════════════════

        namespace Entry {
            constexpr const char* UPDATE_CAMERA = "update_camera";
            constexpr const char* MESH_VS = "mesh_vs";
            constexpr const char* MESH_FS = "mesh_fs";
        }


        // ═══════════════════════════════════════════════════════════════════════════════
        // §2 RENDERER CLASS
        // ═══════════════════════════════════════════════════════════════════════════════

        class Renderer {
        public:
            Renderer() = default;

            bool init(
                wgpu::Device device,
                wgpu::BindGroupLayout computeLayout,
                wgpu::BindGroupLayout renderLayout,
                wgpu::TextureFormat colorFormat,
                wgpu::TextureFormat depthFormat
            ) {
                device_ = device;
                computeLayout_ = computeLayout;
                renderLayout_ = renderLayout;
                colorFormat_ = colorFormat;
                depthFormat_ = depthFormat;

                if (!loadShader()) return false;
                if (!createComputePipeline()) return false;
                if (!createRenderPipeline()) return false;

                return true;
            }

            // ─── Compute Dispatch ───────────────────────────────────────────────────

            void dispatch_update_camera(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup bindGroup
            ) {
                pass.SetPipeline(updateCameraPipeline_);
                pass.SetBindGroup(0, bindGroup);
                pass.DispatchWorkgroups(1, 1, 1);
            }

            // ─── Render ─────────────────────────────────────────────────────────────

            void draw_mesh(
                wgpu::RenderPassEncoder& pass,
                wgpu::BindGroup bindGroup,
                wgpu::Buffer vertexBuffer,
                wgpu::Buffer indexBuffer,
                uint32_t indexCount
            ) {
                pass.SetPipeline(renderPipeline_);
                pass.SetBindGroup(0, bindGroup);
                pass.SetVertexBuffer(0, vertexBuffer);
                pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
                pass.DrawIndexed(indexCount);
            }

        private:
            wgpu::Device device_;
            wgpu::BindGroupLayout computeLayout_;
            wgpu::BindGroupLayout renderLayout_;
            wgpu::TextureFormat colorFormat_;
            wgpu::TextureFormat depthFormat_;

            wgpu::ShaderModule shaderModule_;
            std::string shaderSource_;

            wgpu::ComputePipeline updateCameraPipeline_;
            wgpu::RenderPipeline renderPipeline_;

            // ─────────────────────────────────────────────────────────────────────────
            // CREATION METHODS
            // ─────────────────────────────────────────────────────────────────────────

            bool loadShader() {
                std::array<const char*, 6> paths = {
                    "src/cartridges/playground_rasterize/world.wgsl",
                    "cartridges/playground_rasterize/world.wgsl",
                    "../src/cartridges/playground_rasterize/world.wgsl",
                    "playground_rasterize/world.wgsl",
                    "world.wgsl",
                    "shaders/world.wgsl"
                };

                const char* loadedPath = nullptr;
                for (const char* path : paths) {
                    std::ifstream file(path);
                    if (file.is_open()) {
                        std::stringstream buffer;
                        buffer << file.rdbuf();
                        shaderSource_ = buffer.str();
                        loadedPath = path;
                        break;
                    }
                }

                if (shaderSource_.empty()) {
                    std::cerr << "ERROR: Could not find playground_rasterize shader. Tried:\n";
                    for (const char* path : paths) {
                        std::cerr << "  - " << path << "\n";
                    }
                    return false;
                }

                std::cout << "Loaded playground_rasterize shader from: " << loadedPath << "\n";

                wgpu::ShaderSourceWGSL wgslSource{};
                wgslSource.code = shaderSource_.c_str();

                wgpu::ShaderModuleDescriptor desc{};
                desc.nextInChain = &wgslSource;
                desc.label = "world.wgsl (Playground Rasterize)";

                shaderModule_ = device_.CreateShaderModule(&desc);
                return shaderModule_ != nullptr;
            }

            bool createComputePipeline() {
                wgpu::PipelineLayoutDescriptor layoutDesc{};
                layoutDesc.bindGroupLayoutCount = 1;
                layoutDesc.bindGroupLayouts = &computeLayout_;
                wgpu::PipelineLayout layout = device_.CreatePipelineLayout(&layoutDesc);
                if (!layout) return false;

                wgpu::ComputePipelineDescriptor desc{};
                desc.label = "Playground Rasterize Update Camera";
                desc.layout = layout;
                desc.compute.module = shaderModule_;
                desc.compute.entryPoint = Entry::UPDATE_CAMERA;

                updateCameraPipeline_ = device_.CreateComputePipeline(&desc);
                return updateCameraPipeline_ != nullptr;
            }

            bool createRenderPipeline() {
                wgpu::PipelineLayoutDescriptor layoutDesc{};
                layoutDesc.bindGroupLayoutCount = 1;
                layoutDesc.bindGroupLayouts = &renderLayout_;
                wgpu::PipelineLayout layout = device_.CreatePipelineLayout(&layoutDesc);
                if (!layout) return false;

                // ─── Vertex buffer layout ───────────────────────────────────────────
                // Vertex { position: vec3, normal: vec3 } = 24 bytes

                wgpu::VertexAttribute attributes[2];

                // Position at offset 0
                attributes[0].format = wgpu::VertexFormat::Float32x3;
                attributes[0].offset = 0;
                attributes[0].shaderLocation = 0;

                // Normal at offset 12
                attributes[1].format = wgpu::VertexFormat::Float32x3;
                attributes[1].offset = 12;
                attributes[1].shaderLocation = 1;

                wgpu::VertexBufferLayout vertexBufferLayout{};
                vertexBufferLayout.arrayStride = 24;  // sizeof(Vertex)
                vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;
                vertexBufferLayout.attributeCount = 2;
                vertexBufferLayout.attributes = attributes;

                // ─── Color target ───────────────────────────────────────────────────

                wgpu::ColorTargetState colorTarget{};
                colorTarget.format = colorFormat_;
                colorTarget.writeMask = wgpu::ColorWriteMask::All;

                wgpu::FragmentState fragment{};
                fragment.module = shaderModule_;
                fragment.entryPoint = Entry::MESH_FS;
                fragment.targetCount = 1;
                fragment.targets = &colorTarget;

                // ─── Depth stencil ──────────────────────────────────────────────────

                wgpu::DepthStencilState depthStencil{};
                depthStencil.format = depthFormat_;
                depthStencil.depthWriteEnabled = true;
                depthStencil.depthCompare = wgpu::CompareFunction::Less;

                // ─── Pipeline ───────────────────────────────────────────────────────

                wgpu::RenderPipelineDescriptor desc{};
                desc.label = "Playground Rasterize Mesh";
                desc.layout = layout;
                desc.vertex.module = shaderModule_;
                desc.vertex.entryPoint = Entry::MESH_VS;
                desc.vertex.bufferCount = 1;
                desc.vertex.buffers = &vertexBufferLayout;
                desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                desc.primitive.cullMode = wgpu::CullMode::Back;
                desc.primitive.frontFace = wgpu::FrontFace::CCW;
                desc.depthStencil = &depthStencil;
                desc.fragment = &fragment;

                renderPipeline_ = device_.CreateRenderPipeline(&desc);
                return renderPipeline_ != nullptr;
            }
        };


    } // namespace playground_rasterize
} // namespace t7