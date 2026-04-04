#pragma once

/**
 * PLAYGROUND HYBRID — Renderer
 * ============================
 * 
 * Two render pipelines in one cartridge:
 *   1. Terrain: fullscreen raymarch (writes depth)
 *   2. Entity:  mesh rasterization (reads depth)
 * 
 * PIPELINES:
 * ┌────────────────────┬─────────────┬──────────────────────────────┐
 * │ Pipeline           │ Type        │ Writes                       │
 * ├────────────────────┼─────────────┼──────────────────────────────┤
 * │ updateCamera       │ Compute     │ camera state                 │
 * │ terrain            │ Fullscreen  │ color + depth                │
 * │ entity             │ Mesh        │ color (depth test)           │
 * └────────────────────┴─────────────┴──────────────────────────────┘
 * 
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include "cartridges/playground_hybrid/state.hpp"
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
    constexpr const char* TERRAIN_VS = "terrain_vs";
    constexpr const char* TERRAIN_FS = "terrain_fs";
    constexpr const char* ENTITY_VS = "entity_vs";
    constexpr const char* ENTITY_FS = "entity_fs";
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
        depthFormat_ = depthFormat;
        
        if (!loadShader()) return false;
        if (!createComputePipeline()) return false;
        if (!createTerrainPipeline()) return false;
        if (!createEntityPipeline()) return false;
        
        return true;
    }
    
    // ─── Compute ────────────────────────────────────────────────────────────
    
    void dispatch_update_camera(
        wgpu::ComputePassEncoder& pass,
        wgpu::BindGroup bindGroup
    ) {
        pass.SetPipeline(updateCameraPipeline_);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1, 1, 1);
    }
    
    // ─── Terrain (raymarch) ─────────────────────────────────────────────────
    
    void draw_terrain(
        wgpu::RenderPassEncoder& pass,
        wgpu::BindGroup bindGroup
    ) {
        pass.SetPipeline(terrainPipeline_);
        pass.SetBindGroup(0, bindGroup);
        pass.Draw(3);  // Fullscreen triangle
    }
    
    // ─── Entity (mesh) ──────────────────────────────────────────────────────
    
    void draw_entity(
        wgpu::RenderPassEncoder& pass,
        wgpu::BindGroup bindGroup,
        wgpu::Buffer vertexBuffer,
        wgpu::Buffer indexBuffer,
        uint32_t indexCount
    ) {
        pass.SetPipeline(entityPipeline_);
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
    wgpu::RenderPipeline terrainPipeline_;
    wgpu::RenderPipeline entityPipeline_;
    
    bool loadShader() {
        std::array<const char*, 6> paths = {
            "src/cartridges/playground_hybrid/world.wgsl",
            "cartridges/playground_hybrid/world.wgsl",
            "../src/cartridges/playground_hybrid/world.wgsl",
            "playground_hybrid/world.wgsl",
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
            std::cerr << "ERROR: Could not find playground_hybrid shader\n";
            return false;
        }
        
        std::cout << "Loaded playground_hybrid shader from: " << loadedPath << "\n";
        
        wgpu::ShaderSourceWGSL wgslSource{};
        wgslSource.code = shaderSource_.c_str();
        
        wgpu::ShaderModuleDescriptor desc{};
        desc.nextInChain = &wgslSource;
        desc.label = "world.wgsl (Playground Hybrid)";
        
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
        desc.label = "Playground Hybrid Update Camera";
        desc.layout = layout;
        desc.compute.module = shaderModule_;
        desc.compute.entryPoint = Entry::UPDATE_CAMERA;
        
        updateCameraPipeline_ = device_.CreateComputePipeline(&desc);
        return updateCameraPipeline_ != nullptr;
    }
    
    bool createTerrainPipeline() {
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
        fragment.entryPoint = Entry::TERRAIN_FS;
        fragment.targetCount = 1;
        fragment.targets = &colorTarget;
        
        // Terrain writes depth via frag_depth
        wgpu::DepthStencilState depthStencil{};
        depthStencil.format = depthFormat_;
        depthStencil.depthWriteEnabled = true;
        depthStencil.depthCompare = wgpu::CompareFunction::Always;  // Always write (raymarch computes depth)
        
        wgpu::RenderPipelineDescriptor desc{};
        desc.label = "Playground Hybrid Terrain";
        desc.layout = layout;
        desc.vertex.module = shaderModule_;
        desc.vertex.entryPoint = Entry::TERRAIN_VS;
        desc.vertex.bufferCount = 0;
        desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        desc.primitive.cullMode = wgpu::CullMode::None;
        desc.depthStencil = &depthStencil;
        desc.fragment = &fragment;
        
        terrainPipeline_ = device_.CreateRenderPipeline(&desc);
        return terrainPipeline_ != nullptr;
    }
    
    bool createEntityPipeline() {
        wgpu::PipelineLayoutDescriptor layoutDesc{};
        layoutDesc.bindGroupLayoutCount = 1;
        layoutDesc.bindGroupLayouts = &renderLayout_;
        wgpu::PipelineLayout layout = device_.CreatePipelineLayout(&layoutDesc);
        if (!layout) return false;
        
        // Vertex buffer layout
        wgpu::VertexAttribute attributes[2];
        
        attributes[0].format = wgpu::VertexFormat::Float32x3;
        attributes[0].offset = 0;
        attributes[0].shaderLocation = 0;
        
        attributes[1].format = wgpu::VertexFormat::Float32x3;
        attributes[1].offset = 12;
        attributes[1].shaderLocation = 1;
        
        wgpu::VertexBufferLayout vertexBufferLayout{};
        vertexBufferLayout.arrayStride = 24;
        vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;
        vertexBufferLayout.attributeCount = 2;
        vertexBufferLayout.attributes = attributes;
        
        wgpu::ColorTargetState colorTarget{};
        colorTarget.format = colorFormat_;
        colorTarget.writeMask = wgpu::ColorWriteMask::All;
        
        wgpu::FragmentState fragment{};
        fragment.module = shaderModule_;
        fragment.entryPoint = Entry::ENTITY_FS;
        fragment.targetCount = 1;
        fragment.targets = &colorTarget;
        
        // Entity uses depth test against terrain depth
        wgpu::DepthStencilState depthStencil{};
        depthStencil.format = depthFormat_;
        depthStencil.depthWriteEnabled = true;
        depthStencil.depthCompare = wgpu::CompareFunction::Less;
        
        wgpu::RenderPipelineDescriptor desc{};
        desc.label = "Playground Hybrid Entity";
        desc.layout = layout;
        desc.vertex.module = shaderModule_;
        desc.vertex.entryPoint = Entry::ENTITY_VS;
        desc.vertex.bufferCount = 1;
        desc.vertex.buffers = &vertexBufferLayout;
        desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        desc.primitive.cullMode = wgpu::CullMode::Back;
        desc.primitive.frontFace = wgpu::FrontFace::CCW;
        desc.depthStencil = &depthStencil;
        desc.fragment = &fragment;
        
        entityPipeline_ = device_.CreateRenderPipeline(&desc);
        return entityPipeline_ != nullptr;
    }
};


} // namespace playground_hybrid
} // namespace t7
