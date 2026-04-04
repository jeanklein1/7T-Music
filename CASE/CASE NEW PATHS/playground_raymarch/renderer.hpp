#pragma once

/**
 * PLAYGROUND CARTRIDGE — Renderer
 * ================================
 *
 * Minimal pipeline management for sculpture viewing.
 * Single compute pass (update_camera), single render pass (fullscreen raymarch).
 *
 * PIPELINES:
 * ┌────────────────────┬─────────────┬──────────────────────────────┐
 * │ Pipeline           │ Dimension   │ Writes                       │
 * ├────────────────────┼─────────────┼──────────────────────────────┤
 * │ updateCamera       │ 0D (1,1,1)  │ camera state                 │
 * │ render             │ fullscreen  │ backbuffer                   │
 * └────────────────────┴─────────────┴──────────────────────────────┘
 *
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include "cartridges/playground_raymarch/state.hpp"
#include <webgpu/webgpu_cpp.h>
#include <string>
#include <fstream>
#include <sstream>
#include <array>
#include <iostream>

namespace t7 {
    namespace playground_raymarch {


        // ═══════════════════════════════════════════════════════════════════════════════
        // §1 ENTRY POINTS — Must match world.wgsl
        // ═══════════════════════════════════════════════════════════════════════════════

        namespace Entry {
            // Compute
            constexpr const char* UPDATE_CAMERA = "update_camera";

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
            //
            // Single 0D pass: update camera from input.

            void dispatch_update_camera(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup bindGroup
            ) {
                pass.SetPipeline(updateCameraPipeline_);
                pass.SetBindGroup(0, bindGroup);
                pass.DispatchWorkgroups(1, 1, 1);  // 0D: single invocation
            }

            // ─── Render ─────────────────────────────────────────────────────────────
            //
            // Fullscreen triangle. Fragment shader raymarches the subject.

            void draw_world(
                wgpu::RenderPassEncoder& pass,
                wgpu::BindGroup bindGroup
            ) {
                pass.SetPipeline(renderPipeline_);
                pass.SetBindGroup(0, bindGroup);
                pass.Draw(3);  // Fullscreen triangle
            }

        private:
            wgpu::Device device_;
            wgpu::BindGroupLayout computeLayout_;
            wgpu::BindGroupLayout renderLayout_;
            wgpu::TextureFormat colorFormat_;
            wgpu::TextureFormat depthFormat_;

            wgpu::ShaderModule shaderModule_;
            std::string shaderSource_;

            // ─── Pipelines ──────────────────────────────────────────────────────────
            wgpu::ComputePipeline updateCameraPipeline_;
            wgpu::RenderPipeline renderPipeline_;

            // ─────────────────────────────────────────────────────────────────────────
            // CREATION METHODS
            // ─────────────────────────────────────────────────────────────────────────

            bool loadShader() {
                std::array<const char*, 6> paths = {
                    "src/cartridges/playground_raymarch/world.wgsl",
                    "cartridges/playground_raymarch/world.wgsl",
                    "../src/cartridges/playground_raymarch/world.wgsl",
                    "playground_raymarch/world.wgsl",
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
                    std::cerr << "ERROR: Could not find playground_raymarch shader. Tried:\n";
                    for (const char* path : paths) {
                        std::cerr << "  - " << path << "\n";
                    }
                    return false;
                }

                std::cout << "Loaded playground_raymarch shader from: " << loadedPath << "\n";

                wgpu::ShaderSourceWGSL wgslSource{};
                wgslSource.code = shaderSource_.c_str();

                wgpu::ShaderModuleDescriptor desc{};
                desc.nextInChain = &wgslSource;
                desc.label = "world.wgsl (Playground Raymarch Cartridge)";

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
                desc.label = "Playground Raymarch Update Camera (0D)";
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

                wgpu::ColorTargetState colorTarget{};
                colorTarget.format = colorFormat_;
                colorTarget.writeMask = wgpu::ColorWriteMask::All;

                wgpu::FragmentState fragment{};
                fragment.module = shaderModule_;
                fragment.entryPoint = Entry::WORLD_FS;
                fragment.targetCount = 1;
                fragment.targets = &colorTarget;

                wgpu::RenderPipelineDescriptor desc{};
                desc.label = "Playground Raymarch";
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


    } // namespace playground_raymarch
} // namespace t7