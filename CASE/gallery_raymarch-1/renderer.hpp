#pragma once

/**
 * GALLERY CARTRIDGE — Renderer
 * ============================
 *
 * Fullscreen raymarching renderer.
 * 
 * PIPELINES:
 *   - Compute: update_world (pawn/camera state)
 *   - Render: fullscreen quad → world_fs (raymarch)
 *
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include <webgpu/webgpu_cpp.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace t7 {
namespace gallery {


class Renderer {
public:
    Renderer() = default;
    
    bool init(
        wgpu::Device device,
        wgpu::BindGroupLayout computeBindGroupLayout,
        wgpu::BindGroupLayout renderBindGroupLayout,
        wgpu::TextureFormat colorFormat,
        wgpu::TextureFormat depthFormat
    ) {
        device_ = device;
        
        // ─── Load shader ─────────────────────────────────────────────────────
        std::string shaderPath = "cartridges/gallery/world.wgsl";
        std::ifstream file(shaderPath);
        if (!file) {
            std::cerr << "[Gallery] Failed to open shader: " << shaderPath << std::endl;
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string shaderSource = buffer.str();
        
        wgpu::ShaderSourceWGSL wgslSource{};
        wgslSource.code = shaderSource.c_str();
        
        wgpu::ShaderModuleDescriptor shaderDesc{};
        shaderDesc.label = "Gallery Shader";
        shaderDesc.nextInChain = &wgslSource;
        
        shaderModule_ = device.CreateShaderModule(&shaderDesc);
        if (!shaderModule_) {
            std::cerr << "[Gallery] Failed to create shader module" << std::endl;
            return false;
        }
        
        // ─── Compute pipeline ────────────────────────────────────────────────
        {
            wgpu::PipelineLayoutDescriptor layoutDesc{};
            layoutDesc.label = "Gallery Compute Layout";
            layoutDesc.bindGroupLayoutCount = 1;
            layoutDesc.bindGroupLayouts = &computeBindGroupLayout;
            
            wgpu::PipelineLayout layout = device.CreatePipelineLayout(&layoutDesc);
            
            wgpu::ComputePipelineDescriptor desc{};
            desc.label = "Gallery Update World";
            desc.layout = layout;
            desc.compute.module = shaderModule_;
            desc.compute.entryPoint = "update_world";
            
            computePipeline_ = device.CreateComputePipeline(&desc);
            if (!computePipeline_) {
                std::cerr << "[Gallery] Failed to create compute pipeline" << std::endl;
                return false;
            }
        }
        
        // ─── Render pipeline ─────────────────────────────────────────────────
        {
            wgpu::PipelineLayoutDescriptor layoutDesc{};
            layoutDesc.label = "Gallery Render Layout";
            layoutDesc.bindGroupLayoutCount = 1;
            layoutDesc.bindGroupLayouts = &renderBindGroupLayout;
            
            wgpu::PipelineLayout layout = device.CreatePipelineLayout(&layoutDesc);
            
            wgpu::ColorTargetState colorTarget{};
            colorTarget.format = colorFormat;
            colorTarget.writeMask = wgpu::ColorWriteMask::All;
            
            wgpu::FragmentState fragment{};
            fragment.module = shaderModule_;
            fragment.entryPoint = "world_fs";
            fragment.targetCount = 1;
            fragment.targets = &colorTarget;
            
            wgpu::RenderPipelineDescriptor desc{};
            desc.label = "Gallery Raymarch";
            desc.layout = layout;
            desc.vertex.module = shaderModule_;
            desc.vertex.entryPoint = "fullscreen_vs";
            desc.vertex.bufferCount = 0;
            desc.fragment = &fragment;
            desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
            desc.primitive.cullMode = wgpu::CullMode::None;
            
            renderPipeline_ = device.CreateRenderPipeline(&desc);
            if (!renderPipeline_) {
                std::cerr << "[Gallery] Failed to create render pipeline" << std::endl;
                return false;
            }
        }
        
        std::cout << "[Gallery] Renderer initialized" << std::endl;
        return true;
    }
    
    // ─── Dispatch ────────────────────────────────────────────────────────────
    
    void dispatch_update_world(wgpu::ComputePassEncoder& pass, wgpu::BindGroup bindGroup) {
        pass.SetPipeline(computePipeline_);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1, 1, 1);
    }
    
    void draw_world(wgpu::RenderPassEncoder& pass, wgpu::BindGroup bindGroup) {
        pass.SetPipeline(renderPipeline_);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(3, 1, 0, 0);  // Fullscreen triangle
    }
    
private:
    wgpu::Device device_;
    wgpu::ShaderModule shaderModule_;
    wgpu::ComputePipeline computePipeline_;
    wgpu::RenderPipeline renderPipeline_;
};


} // namespace gallery
} // namespace t7
