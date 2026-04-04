#pragma once

/**
 * GALLERY CARTRIDGE — Renderer
 * ============================
 *
 * Precomputed room field for fast raymarching.
 *
 * PIPELINES:
 * ┌────────────────────┬─────────────┬──────────────────────────────┐
 * │ Pipeline           │ Dimension   │ Writes                       │
 * ├────────────────────┼─────────────┼──────────────────────────────┤
 * │ updateWorld        │ 0D (1,1,1)  │ pawn_state, camera_state     │
 * │ updateRoomField    │ 2D (16²)    │ room_field texture           │
 * │ render             │ fullscreen  │ backbuffer                   │
 * └────────────────────┴─────────────┴──────────────────────────────┘
 *
 * See world.wgsl for the GPU scroll.
 */

#include <webgpu/webgpu_cpp.h>
#include <fstream>
#include <sstream>
#include <array>
#include <iostream>

namespace t7 {
    namespace gallery_raymarch {


        namespace Entry {
            constexpr const char* UPDATE_WORLD = "update_world";
            constexpr const char* UPDATE_ROOM_FIELD = "update_room_field";
            constexpr const char* FULLSCREEN_VS = "fullscreen_vs";
            constexpr const char* WORLD_FS = "world_fs";
        }


        class Renderer {
        public:
            Renderer() = default;

            bool init(
                wgpu::Device device,
                wgpu::BindGroupLayout computeBufferLayout,
                wgpu::BindGroupLayout computeTextureLayout,
                wgpu::BindGroupLayout renderBufferLayout,
                wgpu::BindGroupLayout renderTextureLayout,
                wgpu::TextureFormat colorFormat
            ) {
                device_ = device;
                computeBufferLayout_ = computeBufferLayout;
                computeTextureLayout_ = computeTextureLayout;
                renderBufferLayout_ = renderBufferLayout;
                renderTextureLayout_ = renderTextureLayout;
                colorFormat_ = colorFormat;

                if (!loadShader()) return false;
                if (!createComputePipelines()) return false;
                if (!createRenderPipeline()) return false;

                std::cout << "[Gallery] Renderer initialized\n";
                return true;
            }

            // ─── Compute Dispatches ─────────────────────────────────────────────────

            void dispatch_update_world(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup bufferBindGroup,
                wgpu::BindGroup textureBindGroup
            ) {
                pass.SetPipeline(updateWorldPipeline_);
                pass.SetBindGroup(0, bufferBindGroup);
                pass.SetBindGroup(1, textureBindGroup);
                pass.DispatchWorkgroups(1, 1, 1);
            }

            void dispatch_update_room_field(
                wgpu::ComputePassEncoder& pass,
                wgpu::BindGroup bufferBindGroup,
                wgpu::BindGroup textureBindGroup,
                uint32_t workgroups
            ) {
                pass.SetPipeline(updateRoomFieldPipeline_);
                pass.SetBindGroup(0, bufferBindGroup);
                pass.SetBindGroup(1, textureBindGroup);
                pass.DispatchWorkgroups(workgroups, workgroups, 1);
            }

            // ─── Render ─────────────────────────────────────────────────────────────

            void draw_world(
                wgpu::RenderPassEncoder& pass,
                wgpu::BindGroup bufferBindGroup,
                wgpu::BindGroup textureBindGroup
            ) {
                pass.SetPipeline(renderPipeline_);
                pass.SetBindGroup(0, bufferBindGroup);
                pass.SetBindGroup(1, textureBindGroup);
                pass.Draw(3);
            }

            // ─── Hot Reload ─────────────────────────────────────────────────────────

            bool reload() {
                std::cout << "[Gallery] Reloading shader...\n";

                if (!loadShader()) return false;
                if (!createComputePipelines()) return false;
                if (!createRenderPipeline()) return false;

                std::cout << "[Gallery] Reload successful\n";
                return true;
            }

        private:
            wgpu::Device device_;
            wgpu::BindGroupLayout computeBufferLayout_;
            wgpu::BindGroupLayout computeTextureLayout_;
            wgpu::BindGroupLayout renderBufferLayout_;
            wgpu::BindGroupLayout renderTextureLayout_;
            wgpu::TextureFormat colorFormat_;

            wgpu::ShaderModule shaderModule_;
            std::string shaderPath_;

            wgpu::ComputePipeline updateWorldPipeline_;
            wgpu::ComputePipeline updateRoomFieldPipeline_;
            wgpu::RenderPipeline renderPipeline_;

            bool loadShader() {
                std::array<const char*, 4> paths = {
                    "src/cartridges/gallery_raymarch/world.wgsl",
                    "cartridges/gallery_raymarch/world.wgsl",
                    "../src/cartridges/gallery_raymarch/world.wgsl",
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
                        break;
                    }
                }

                if (shaderSource.empty()) {
                    std::cerr << "[Gallery] Could not find shader\n";
                    return false;
                }

                wgpu::ShaderSourceWGSL wgslSource{};
                wgslSource.code = shaderSource.c_str();

                wgpu::ShaderModuleDescriptor desc{};
                desc.nextInChain = &wgslSource;
                desc.label = "Gallery Shader";

                shaderModule_ = device_.CreateShaderModule(&desc);
                return shaderModule_ != nullptr;
            }

            bool createComputePipelines() {
                std::array<wgpu::BindGroupLayout, 2> layouts = {
                    computeBufferLayout_,
                    computeTextureLayout_
                };

                wgpu::PipelineLayoutDescriptor layoutDesc{};
                layoutDesc.bindGroupLayoutCount = layouts.size();
                layoutDesc.bindGroupLayouts = layouts.data();
                wgpu::PipelineLayout computeLayout = device_.CreatePipelineLayout(&layoutDesc);
                if (!computeLayout) return false;

                // Update World (0D)
                {
                    wgpu::ComputePipelineDescriptor desc{};
                    desc.label = "Update World";
                    desc.layout = computeLayout;
                    desc.compute.module = shaderModule_;
                    desc.compute.entryPoint = Entry::UPDATE_WORLD;

                    updateWorldPipeline_ = device_.CreateComputePipeline(&desc);
                    if (!updateWorldPipeline_) return false;
                }

                // Update Room Field (2D)
                {
                    wgpu::ComputePipelineDescriptor desc{};
                    desc.label = "Update Room Field";
                    desc.layout = computeLayout;
                    desc.compute.module = shaderModule_;
                    desc.compute.entryPoint = Entry::UPDATE_ROOM_FIELD;

                    updateRoomFieldPipeline_ = device_.CreateComputePipeline(&desc);
                    if (!updateRoomFieldPipeline_) return false;
                }

                return true;
            }

            bool createRenderPipeline() {
                std::array<wgpu::BindGroupLayout, 2> layouts = {
                    renderBufferLayout_,
                    renderTextureLayout_
                };

                wgpu::PipelineLayoutDescriptor layoutDesc{};
                layoutDesc.bindGroupLayoutCount = layouts.size();
                layoutDesc.bindGroupLayouts = layouts.data();
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
                desc.label = "Gallery Raymarch";
                desc.layout = layout;
                desc.vertex.module = shaderModule_;
                desc.vertex.entryPoint = Entry::FULLSCREEN_VS;
                desc.vertex.bufferCount = 0;
                desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
                desc.primitive.cullMode = wgpu::CullMode::None;
                desc.fragment = &fragment;

                renderPipeline_ = device_.CreateRenderPipeline(&desc);
                return renderPipeline_ != nullptr;
            }
        };


    } // namespace gallery_raymarch
} // namespace t7