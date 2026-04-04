#pragma once

/**
 * PLAYGROUND HYBRID — Renderer
 * ============================
 * 
 * Pipeline management with precomputed branch field.
 * 
 * PIPELINES:
 * ┌────────────────────┬─────────────┬──────────────────────────────┐
 * │ Pipeline           │ Dimension   │ Writes                       │
 * ├────────────────────┼─────────────┼──────────────────────────────┤
 * │ updateCamera       │ 0D (1,1,1)  │ camera_state                 │
 * │ updateBranchField  │ 2D (32²)    │ branch_field texture         │
 * │ render             │ fullscreen  │ backbuffer                   │
 * └────────────────────┴─────────────┴──────────────────────────────┘
 * 
 * Hot reload enabled.
 * See world.wgsl for the GPU scroll.
 */

#include <webgpu/webgpu_cpp.h>
#include <string>
#include <fstream>
#include <sstream>
#include <array>
#include <iostream>

namespace t7 {
namespace playground_hybrid {


namespace Entry {
    constexpr const char* UPDATE_CAMERA = "update_camera";
    constexpr const char* UPDATE_BRANCH_FIELD = "update_branch_field";
    constexpr const char* TERRAIN_VS = "terrain_vs";
    constexpr const char* TERRAIN_FS = "terrain_fs";
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
        
        return true;
    }
    
    // ─── Compute Dispatches ─────────────────────────────────────────────────
    
    void dispatch_update_camera(
        wgpu::ComputePassEncoder& pass,
        wgpu::BindGroup bufferBindGroup,
        wgpu::BindGroup textureBindGroup
    ) {
        pass.SetPipeline(updateCameraPipeline_);
        pass.SetBindGroup(0, bufferBindGroup);
        pass.SetBindGroup(1, textureBindGroup);
        pass.DispatchWorkgroups(1, 1, 1);
    }
    
    void dispatch_update_branch_field(
        wgpu::ComputePassEncoder& pass,
        wgpu::BindGroup bufferBindGroup,
        wgpu::BindGroup textureBindGroup,
        uint32_t workgroups
    ) {
        pass.SetPipeline(updateBranchFieldPipeline_);
        pass.SetBindGroup(0, bufferBindGroup);
        pass.SetBindGroup(1, textureBindGroup);
        pass.DispatchWorkgroups(workgroups, workgroups, 1);
    }
    
    // ─── Render ─────────────────────────────────────────────────────────────
    
    void draw_terrain(
        wgpu::RenderPassEncoder& pass,
        wgpu::BindGroup bufferBindGroup,
        wgpu::BindGroup textureBindGroup
    ) {
        pass.SetPipeline(terrainPipeline_);
        pass.SetBindGroup(0, bufferBindGroup);
        pass.SetBindGroup(1, textureBindGroup);
        pass.Draw(3);
    }
    
    // ─── Hot Reload ─────────────────────────────────────────────────────────
    
    bool reload() {
        std::cout << "[Playground Hybrid] Reloading shader...\n";
        
        if (!loadShader()) {
            std::cerr << "[Playground Hybrid] Reload failed: shader load error\n";
            return false;
        }
        if (!createComputePipelines()) {
            std::cerr << "[Playground Hybrid] Reload failed: compute pipeline error\n";
            return false;
        }
        if (!createRenderPipeline()) {
            std::cerr << "[Playground Hybrid] Reload failed: render pipeline error\n";
            return false;
        }
        
        std::cout << "[Playground Hybrid] Reload successful\n";
        return true;
    }
    
    const std::string& shader_path() const { return shaderPath_; }
    
private:
    wgpu::Device device_;
    wgpu::BindGroupLayout computeBufferLayout_;
    wgpu::BindGroupLayout computeTextureLayout_;
    wgpu::BindGroupLayout renderBufferLayout_;
    wgpu::BindGroupLayout renderTextureLayout_;
    wgpu::TextureFormat colorFormat_;
    
    wgpu::ShaderModule shaderModule_;
    std::string shaderSource_;
    std::string shaderPath_;
    
    wgpu::ComputePipeline updateCameraPipeline_;
    wgpu::ComputePipeline updateBranchFieldPipeline_;
    wgpu::RenderPipeline terrainPipeline_;
    
    bool loadShader() {
        if (!shaderPath_.empty()) {
            std::ifstream file(shaderPath_);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                shaderSource_ = buffer.str();
            } else {
                std::cerr << "[Playground Hybrid] Cannot reopen: " << shaderPath_ << "\n";
                return false;
            }
        } else {
            std::array<const char*, 6> paths = {
                "src/cartridges/playground_hybrid/world.wgsl",
                "cartridges/playground_hybrid/world.wgsl",
                "../src/cartridges/playground_hybrid/world.wgsl",
                "playground_hybrid/world.wgsl",
                "world.wgsl",
                "shaders/world.wgsl"
            };
            
            for (const char* path : paths) {
                std::ifstream file(path);
                if (file.is_open()) {
                    std::stringstream buffer;
                    buffer << file.rdbuf();
                    shaderSource_ = buffer.str();
                    shaderPath_ = path;
                    break;
                }
            }
            
            if (shaderSource_.empty()) {
                std::cerr << "[Playground Hybrid] Could not find shader\n";
                return false;
            }
            
            std::cout << "[Playground Hybrid] Loaded shader from: " << shaderPath_ << "\n";
        }
        
        wgpu::ShaderSourceWGSL wgslSource{};
        wgslSource.code = shaderSource_.c_str();
        
        wgpu::ShaderModuleDescriptor desc{};
        desc.nextInChain = &wgslSource;
        desc.label = "Playground Hybrid Shader";
        
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
        
        // Update Camera (0D)
        {
            wgpu::ComputePipelineDescriptor desc{};
            desc.label = "Update Camera";
            desc.layout = computeLayout;
            desc.compute.module = shaderModule_;
            desc.compute.entryPoint = Entry::UPDATE_CAMERA;
            
            updateCameraPipeline_ = device_.CreateComputePipeline(&desc);
            if (!updateCameraPipeline_) return false;
        }
        
        // Update Branch Field (2D)
        {
            wgpu::ComputePipelineDescriptor desc{};
            desc.label = "Update Branch Field";
            desc.layout = computeLayout;
            desc.compute.module = shaderModule_;
            desc.compute.entryPoint = Entry::UPDATE_BRANCH_FIELD;
            
            updateBranchFieldPipeline_ = device_.CreateComputePipeline(&desc);
            if (!updateBranchFieldPipeline_) return false;
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
        fragment.entryPoint = Entry::TERRAIN_FS;
        fragment.targetCount = 1;
        fragment.targets = &colorTarget;
        
        wgpu::RenderPipelineDescriptor desc{};
        desc.label = "Terrain Raymarch";
        desc.layout = layout;
        desc.vertex.module = shaderModule_;
        desc.vertex.entryPoint = Entry::TERRAIN_VS;
        desc.vertex.bufferCount = 0;
        desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        desc.primitive.cullMode = wgpu::CullMode::None;
        desc.fragment = &fragment;
        
        terrainPipeline_ = device_.CreateRenderPipeline(&desc);
        return terrainPipeline_ != nullptr;
    }
};


} // namespace playground_hybrid
} // namespace t7
