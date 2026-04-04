#pragma once

/**
 * TERRAIN RAYMARCH - Renderer
 * ===========================
 * 
 * Minimal pipeline management for raymarching visualization.
 * 
 * ONE PIPELINE, ONE DRAW CALL
 * ---------------------------
 * 
 * - Compute pipeline: update dynamics (same as rasterized)
 * - Render pipeline: fullscreen triangle, fragment shader raymarches
 * 
 * No vertex buffers, no meshes. The fragment shader does everything.
 */

#include "cartridges/terrain_raymarch/state.hpp"
#include <webgpu/webgpu_cpp.h>
#include <string>
#include <fstream>
#include <sstream>

namespace t7 {
namespace terrain_raymarch {

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
        computeBindGroupLayout_ = computeBindGroupLayout;
        renderBindGroupLayout_ = renderBindGroupLayout;
        colorFormat_ = colorFormat;
        depthFormat_ = depthFormat;
        
        if (!loadShader()) return false;
        if (!createComputePipeline()) return false;
        if (!createRenderPipeline()) return false;
        
        return true;
    }
    
    void dispatch_update(wgpu::ComputePassEncoder& pass, wgpu::BindGroup bindGroup) {
        pass.SetPipeline(computePipeline_);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1);
    }
    
    void draw_world(wgpu::RenderPassEncoder& pass, wgpu::BindGroup bindGroup) {
        pass.SetPipeline(renderPipeline_);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(3);  // Fullscreen triangle
    }
    
private:
    wgpu::Device device_;
    wgpu::BindGroupLayout computeBindGroupLayout_;
    wgpu::BindGroupLayout renderBindGroupLayout_;
    wgpu::TextureFormat colorFormat_;
    wgpu::TextureFormat depthFormat_;
    
    wgpu::ShaderModule shaderModule_;
    std::string shaderSource_;
    
    wgpu::ComputePipeline computePipeline_;
    wgpu::RenderPipeline renderPipeline_;
    
    bool loadShader() {
        // Shader path is an implementation detail of this cartridge
        std::ifstream file("shaders/world_raymarch.wgsl");
        if (!file.is_open()) {
            // Fallback for running from different directories
            file.open("world_raymarch.wgsl");
            if (!file.is_open()) {
                return false;
            }
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        shaderSource_ = buffer.str();
        
        wgpu::ShaderSourceWGSL wgslSource{};
        wgslSource.code = shaderSource_.c_str();
        
        wgpu::ShaderModuleDescriptor desc{};
        desc.nextInChain = &wgslSource;
        desc.label = "world.wgsl (raymarch)";
        
        shaderModule_ = device_.CreateShaderModule(&desc);
        return shaderModule_ != nullptr;
    }
    
    bool createComputePipeline() {
        wgpu::PipelineLayoutDescriptor layoutDesc{};
        layoutDesc.bindGroupLayoutCount = 1;
        layoutDesc.bindGroupLayouts = &computeBindGroupLayout_;
        wgpu::PipelineLayout layout = device_.CreatePipelineLayout(&layoutDesc);
        
        wgpu::ComputePipelineDescriptor desc{};
        desc.label = "World Update (raymarch)";
        desc.layout = layout;
        desc.compute.module = shaderModule_;
        desc.compute.entryPoint = "update";
        
        computePipeline_ = device_.CreateComputePipeline(&desc);
        return computePipeline_ != nullptr;
    }
    
    bool createRenderPipeline() {
        wgpu::PipelineLayoutDescriptor layoutDesc{};
        layoutDesc.bindGroupLayoutCount = 1;
        layoutDesc.bindGroupLayouts = &renderBindGroupLayout_;
        wgpu::PipelineLayout layout = device_.CreatePipelineLayout(&layoutDesc);
        
        // No vertex buffers - fullscreen triangle from vertex_index
        
        wgpu::ColorTargetState colorTarget{};
        colorTarget.format = colorFormat_;
        colorTarget.writeMask = wgpu::ColorWriteMask::All;
        
        wgpu::FragmentState fragment{};
        fragment.module = shaderModule_;
        fragment.entryPoint = "world_fs";
        fragment.targetCount = 1;
        fragment.targets = &colorTarget;
        
        // No depth test needed for raymarching (fragment shader handles it)
        
        wgpu::RenderPipelineDescriptor desc{};
        desc.label = "World Raymarch";
        desc.layout = layout;
        desc.vertex.module = shaderModule_;
        desc.vertex.entryPoint = "fullscreen_vs";
        desc.vertex.bufferCount = 0;
        desc.vertex.buffers = nullptr;
        desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        desc.primitive.cullMode = wgpu::CullMode::None;
        desc.fragment = &fragment;
        // No depth stencil
        
        renderPipeline_ = device_.CreateRenderPipeline(&desc);
        return renderPipeline_ != nullptr;
    }
};

} // namespace terrain_raymarch
} // namespace t7
