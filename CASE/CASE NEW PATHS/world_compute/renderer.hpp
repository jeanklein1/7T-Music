#pragma once

/**
 * WORLD COMPUTE - Renderer
 * ========================
 * 
 * Compute-first pipeline management.
 * 
 * THREE COMPUTE PASSES:
 *   1. update_world        — 0D state (1 thread)
 *   2. update_height_field — 2D height texture (256×256 threads)
 *   3. update_tiles        — 2D tile texture (64×64 threads)
 * 
 * ONE RENDER PASS:
 *   - Fullscreen triangle, fragment shader raymarches
 *   - Samples precomputed textures
 */

#include "cartridges/world_compute/state.hpp"
#include <webgpu/webgpu_cpp.h>
#include <string>
#include <fstream>
#include <sstream>
#include <array>
#include <iostream>

namespace t7 {
namespace world_compute {

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
    
    // ─── Compute Dispatch ────────────────────────────────────────────────────
    
    void dispatch_update_world(
        wgpu::ComputePassEncoder& pass, 
        wgpu::BindGroup entityBindGroup,
        wgpu::BindGroup textureBindGroup
    ) {
        pass.SetPipeline(updateWorldPipeline_);
        pass.SetBindGroup(0, entityBindGroup);
        pass.SetBindGroup(1, textureBindGroup);
        pass.DispatchWorkgroups(1, 1, 1);
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
        pass.DispatchWorkgroups(workgroups, workgroups, 1);
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
        pass.DispatchWorkgroups(workgroups, workgroups, 1);
    }
    
    // ─── Render ──────────────────────────────────────────────────────────────
    
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
    
    // Three compute pipelines
    wgpu::ComputePipeline updateWorldPipeline_;
    wgpu::ComputePipeline updateHeightFieldPipeline_;
    wgpu::ComputePipeline updateTilesPipeline_;
    
    // One render pipeline
    wgpu::RenderPipeline renderPipeline_;
    
    bool loadShader() {
        // Shader lives inside the cartridge folder
        std::array<const char*, 6> paths = {
            "src/cartridges/world_compute/world.wgsl",
            "cartridges/world_compute/world.wgsl",
            "../src/cartridges/world_compute/world.wgsl",
            "world_compute/world.wgsl",
            "world.wgsl",
            "shaders/world.wgsl"  // fallback
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
            std::cerr << "ERROR: Could not find shader. Tried:\n";
            for (const char* path : paths) {
                std::cerr << "  - " << path << "\n";
            }
            return false;
        }
        
        std::cout << "Loaded shader from: " << loadedPath << "\n";
        
        wgpu::ShaderSourceWGSL wgslSource{};
        wgslSource.code = shaderSource_.c_str();
        
        wgpu::ShaderModuleDescriptor desc{};
        desc.nextInChain = &wgslSource;
        desc.label = "world.wgsl (compute-first)";
        
        shaderModule_ = device_.CreateShaderModule(&desc);
        return shaderModule_ != nullptr;
    }
    
    bool createComputePipelines() {
        // Shared pipeline layout for compute passes
        std::array<wgpu::BindGroupLayout, 2> computeLayouts = {
            computeEntityLayout_,
            computeTextureLayout_
        };
        
        wgpu::PipelineLayoutDescriptor layoutDesc{};
        layoutDesc.bindGroupLayoutCount = computeLayouts.size();
        layoutDesc.bindGroupLayouts = computeLayouts.data();
        wgpu::PipelineLayout computeLayout = device_.CreatePipelineLayout(&layoutDesc);
        if (!computeLayout) return false;
        
        // Pipeline 1: update_world (0D)
        {
            wgpu::ComputePipelineDescriptor desc{};
            desc.label = "Update World (0D)";
            desc.layout = computeLayout;
            desc.compute.module = shaderModule_;
            desc.compute.entryPoint = "update_world";
            
            updateWorldPipeline_ = device_.CreateComputePipeline(&desc);
            if (!updateWorldPipeline_) return false;
        }
        
        // Pipeline 2: update_height_field (2D)
        {
            wgpu::ComputePipelineDescriptor desc{};
            desc.label = "Update Height Field (2D)";
            desc.layout = computeLayout;
            desc.compute.module = shaderModule_;
            desc.compute.entryPoint = "update_height_field";
            
            updateHeightFieldPipeline_ = device_.CreateComputePipeline(&desc);
            if (!updateHeightFieldPipeline_) return false;
        }
        
        // Pipeline 3: update_tiles (2D)
        {
            wgpu::ComputePipelineDescriptor desc{};
            desc.label = "Update Tiles (2D)";
            desc.layout = computeLayout;
            desc.compute.module = shaderModule_;
            desc.compute.entryPoint = "update_tiles";
            
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
        fragment.entryPoint = "world_fs";
        fragment.targetCount = 1;
        fragment.targets = &colorTarget;
        
        wgpu::RenderPipelineDescriptor desc{};
        desc.label = "World Raymarch (Compute-First)";
        desc.layout = layout;
        desc.vertex.module = shaderModule_;
        desc.vertex.entryPoint = "fullscreen_vs";
        desc.vertex.bufferCount = 0;
        desc.vertex.buffers = nullptr;
        desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        desc.primitive.cullMode = wgpu::CullMode::None;
        desc.fragment = &fragment;
        
        renderPipeline_ = device_.CreateRenderPipeline(&desc);
        return renderPipeline_ != nullptr;
    }
};

} // namespace world_compute
} // namespace t7
