#pragma once

/**
 * SPECIES STUDIO — Renderer
 * ==========================
 * 
 * Pipeline management for species design viewer.
 * Compute pass (camera), Render pass (fullscreen raymarch).
 *
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include <webgpu/webgpu_cpp.h>
#include <string>
#include <fstream>
#include <sstream>
#include <array>
#include <iostream>

namespace t7 {
namespace species_studio {


namespace Entry {
    constexpr const char* UPDATE_CAMERA = "update_camera";
    constexpr const char* FULLSCREEN_VS = "fullscreen_vs";
    constexpr const char* WORLD_FS = "world_fs";
}


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
        
        if (!loadShader()) return false;
        if (!createComputePipeline()) return false;
        if (!createRenderPipeline()) return false;
        
        std::cout << "[Species Studio] Renderer initialized\n";
        return true;
    }
    
    void dispatch_update_camera(
        wgpu::ComputePassEncoder& pass,
        wgpu::BindGroup bindGroup
    ) {
        pass.SetPipeline(updateCameraPipeline_);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1, 1, 1);
    }
    
    void draw_world(
        wgpu::RenderPassEncoder& pass,
        wgpu::BindGroup bindGroup
    ) {
        pass.SetPipeline(renderPipeline_);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(3);
    }
    
    // ─── Hot Reload ──────────────────────────────────────────────────────────
    
    /**
     * Reload shader from disk and recreate pipelines.
     * Called by incubator when file changes.
     */
    bool reload() {
        std::cout << "[Species Studio] Reloading shader...\n";
        
        if (!loadShader()) {
            std::cerr << "[Species Studio] Reload failed: shader load error\n";
            return false;
        }
        if (!createComputePipeline()) {
            std::cerr << "[Species Studio] Reload failed: compute pipeline error\n";
            return false;
        }
        if (!createRenderPipeline()) {
            std::cerr << "[Species Studio] Reload failed: render pipeline error\n";
            return false;
        }
        
        std::cout << "[Species Studio] Reload successful\n";
        return true;
    }
    
    /**
     * Get the shader file path (for file watching).
     */
    const std::string& shader_path() const { return shaderPath_; }
    
private:
    wgpu::Device device_;
    wgpu::BindGroupLayout computeLayout_;
    wgpu::BindGroupLayout renderLayout_;
    wgpu::TextureFormat colorFormat_;
    
    wgpu::ShaderModule shaderModule_;
    std::string shaderSource_;
    std::string shaderPath_;  // Path that was successfully loaded
    
    wgpu::ComputePipeline updateCameraPipeline_;
    wgpu::RenderPipeline renderPipeline_;
    
    bool loadShader() {
        // If we already have a path (reload case), use it directly
        if (!shaderPath_.empty()) {
            std::ifstream file(shaderPath_);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                shaderSource_ = buffer.str();
            } else {
                std::cerr << "[Species Studio] Cannot reopen: " << shaderPath_ << "\n";
                return false;
            }
        } else {
            // First load: search for the file
            std::array<const char*, 4> paths = {
                "cartridges/species_studio/world.wgsl",
                "src/cartridges/species_studio/world.wgsl",
                "../src/cartridges/species_studio/world.wgsl",
                "species_studio/world.wgsl"
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
                std::cerr << "[Species Studio] Could not find shader\n";
                return false;
            }
            
            std::cout << "[Species Studio] Loaded shader from: " << shaderPath_ << "\n";
        }
        
        wgpu::ShaderSourceWGSL wgslSource{};
        wgslSource.code = shaderSource_.c_str();
        
        wgpu::ShaderModuleDescriptor desc{};
        desc.nextInChain = &wgslSource;
        desc.label = "Species Studio Shader";
        
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
        desc.label = "Species Update Camera";
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
        desc.label = "Species Raymarch";
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


} // namespace species_studio
} // namespace t7
