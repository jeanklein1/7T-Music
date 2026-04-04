#pragma once

/**
 * GALLERY HYBRID — Renderer
 * =========================
 *
 * Rasterize pipelines for room and pawn.
 * Simple vertex transform + material-based fragment shading.
 */

#include <webgpu/webgpu_cpp.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <array>

#include "mesh.hpp"

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
        
        // Load shader
        std::string shaderPath = "cartridges/gallery/world.wgsl";
        std::ifstream file(shaderPath);
        if (!file) {
            std::cerr << "Failed to open shader: " << shaderPath << std::endl;
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string shaderSource = buffer.str();
        
        wgpu::ShaderSourceWGSL wgslSource{};
        wgslSource.code = shaderSource.c_str();
        
        wgpu::ShaderModuleDescriptor shaderDesc{};
        shaderDesc.nextInChain = &wgslSource;
        shaderDesc.label = "world.wgsl (Gallery Hybrid)";
        
        shaderModule_ = device.CreateShaderModule(&shaderDesc);
        if (!shaderModule_) {
            std::cerr << "Failed to create shader module" << std::endl;
            return false;
        }
        
        std::cout << "Loaded gallery hybrid shader from: " << shaderPath << std::endl;
        
        // Create pipelines
        if (!createComputePipeline(computeBindGroupLayout)) return false;
        if (!createRoomPipeline(renderBindGroupLayout, colorFormat, depthFormat)) return false;
        if (!createPawnPipeline(renderBindGroupLayout, colorFormat, depthFormat)) return false;
        
        return true;
    }
    
    // ─── Compute Dispatch ─────────────────────────────────────────────────────
    
    void dispatch_update_world(wgpu::ComputePassEncoder& pass, wgpu::BindGroup bindGroup) {
        pass.SetPipeline(computePipeline_);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1, 1, 1);
    }
    
    // ─── Room Draw ────────────────────────────────────────────────────────────
    
    void draw_room(
        wgpu::RenderPassEncoder& pass,
        wgpu::BindGroup bindGroup,
        wgpu::Buffer vertexBuffer,
        wgpu::Buffer indexBuffer,
        uint32_t indexCount
    ) {
        pass.SetPipeline(roomPipeline_);
        pass.SetBindGroup(0, bindGroup);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.DrawIndexed(indexCount, 1, 0, 0, 0);
    }
    
    // ─── Pawn Draw ────────────────────────────────────────────────────────────
    
    void draw_pawn(
        wgpu::RenderPassEncoder& pass,
        wgpu::BindGroup bindGroup,
        wgpu::Buffer vertexBuffer,
        wgpu::Buffer indexBuffer,
        uint32_t indexCount
    ) {
        pass.SetPipeline(pawnPipeline_);
        pass.SetBindGroup(0, bindGroup);
        pass.SetVertexBuffer(0, vertexBuffer);
        pass.SetIndexBuffer(indexBuffer, wgpu::IndexFormat::Uint32);
        pass.DrawIndexed(indexCount, 1, 0, 0, 0);
    }
    
private:
    wgpu::Device device_;
    wgpu::ShaderModule shaderModule_;
    wgpu::ComputePipeline computePipeline_;
    wgpu::RenderPipeline roomPipeline_;
    wgpu::RenderPipeline pawnPipeline_;
    
    
    bool createComputePipeline(wgpu::BindGroupLayout bindGroupLayout) {
        wgpu::PipelineLayoutDescriptor layoutDesc{};
        layoutDesc.label = "Gallery Compute Layout";
        layoutDesc.bindGroupLayoutCount = 1;
        layoutDesc.bindGroupLayouts = &bindGroupLayout;
        
        wgpu::PipelineLayout layout = device_.CreatePipelineLayout(&layoutDesc);
        
        wgpu::ComputePipelineDescriptor desc{};
        desc.label = "Gallery Update World";
        desc.layout = layout;
        desc.compute.module = shaderModule_;
        desc.compute.entryPoint = "update_world";
        
        computePipeline_ = device_.CreateComputePipeline(&desc);
        return computePipeline_ != nullptr;
    }
    
    wgpu::VertexBufferLayout createVertexBufferLayout() {
        static std::array<wgpu::VertexAttribute, 4> attributes{};
        
        // position: vec3<f32>
        attributes[0].format = wgpu::VertexFormat::Float32x3;
        attributes[0].offset = 0;
        attributes[0].shaderLocation = 0;
        
        // normal: vec3<f32>
        attributes[1].format = wgpu::VertexFormat::Float32x3;
        attributes[1].offset = 12;
        attributes[1].shaderLocation = 1;
        
        // uv: vec2<f32>
        attributes[2].format = wgpu::VertexFormat::Float32x2;
        attributes[2].offset = 24;
        attributes[2].shaderLocation = 2;
        
        // material: u32
        attributes[3].format = wgpu::VertexFormat::Uint32;
        attributes[3].offset = 32;
        attributes[3].shaderLocation = 3;
        
        wgpu::VertexBufferLayout layout{};
        layout.arrayStride = sizeof(Vertex);  // 48 bytes
        layout.stepMode = wgpu::VertexStepMode::Vertex;
        layout.attributeCount = attributes.size();
        layout.attributes = attributes.data();
        
        return layout;
    }
    
    bool createRoomPipeline(
        wgpu::BindGroupLayout bindGroupLayout,
        wgpu::TextureFormat colorFormat,
        wgpu::TextureFormat depthFormat
    ) {
        wgpu::PipelineLayoutDescriptor layoutDesc{};
        layoutDesc.label = "Gallery Room Layout";
        layoutDesc.bindGroupLayoutCount = 1;
        layoutDesc.bindGroupLayouts = &bindGroupLayout;
        
        wgpu::PipelineLayout layout = device_.CreatePipelineLayout(&layoutDesc);
        
        wgpu::VertexBufferLayout vertexLayout = createVertexBufferLayout();
        
        wgpu::ColorTargetState colorTarget{};
        colorTarget.format = colorFormat;
        colorTarget.writeMask = wgpu::ColorWriteMask::All;
        
        wgpu::FragmentState fragment{};
        fragment.module = shaderModule_;
        fragment.entryPoint = "main_fs";
        fragment.targetCount = 1;
        fragment.targets = &colorTarget;
        
        wgpu::DepthStencilState depthStencil{};
        depthStencil.format = depthFormat;
        depthStencil.depthWriteEnabled = true;
        depthStencil.depthCompare = wgpu::CompareFunction::Less;
        
        wgpu::RenderPipelineDescriptor desc{};
        desc.label = "Gallery Room Pipeline";
        desc.layout = layout;
        desc.vertex.module = shaderModule_;
        desc.vertex.entryPoint = "room_vs";
        desc.vertex.bufferCount = 1;
        desc.vertex.buffers = &vertexLayout;
        desc.fragment = &fragment;
        desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        desc.primitive.cullMode = wgpu::CullMode::Back;
        desc.primitive.frontFace = wgpu::FrontFace::CW;
        desc.depthStencil = &depthStencil;
        
        roomPipeline_ = device_.CreateRenderPipeline(&desc);
        return roomPipeline_ != nullptr;
    }
    
    bool createPawnPipeline(
        wgpu::BindGroupLayout bindGroupLayout,
        wgpu::TextureFormat colorFormat,
        wgpu::TextureFormat depthFormat
    ) {
        wgpu::PipelineLayoutDescriptor layoutDesc{};
        layoutDesc.label = "Gallery Pawn Layout";
        layoutDesc.bindGroupLayoutCount = 1;
        layoutDesc.bindGroupLayouts = &bindGroupLayout;
        
        wgpu::PipelineLayout layout = device_.CreatePipelineLayout(&layoutDesc);
        
        wgpu::VertexBufferLayout vertexLayout = createVertexBufferLayout();
        
        wgpu::ColorTargetState colorTarget{};
        colorTarget.format = colorFormat;
        colorTarget.writeMask = wgpu::ColorWriteMask::All;
        
        wgpu::FragmentState fragment{};
        fragment.module = shaderModule_;
        fragment.entryPoint = "main_fs";
        fragment.targetCount = 1;
        fragment.targets = &colorTarget;
        
        wgpu::DepthStencilState depthStencil{};
        depthStencil.format = depthFormat;
        depthStencil.depthWriteEnabled = true;
        depthStencil.depthCompare = wgpu::CompareFunction::Less;
        
        wgpu::RenderPipelineDescriptor desc{};
        desc.label = "Gallery Pawn Pipeline";
        desc.layout = layout;
        desc.vertex.module = shaderModule_;
        desc.vertex.entryPoint = "pawn_vs";  // Different vertex shader!
        desc.vertex.bufferCount = 1;
        desc.vertex.buffers = &vertexLayout;
        desc.fragment = &fragment;
        desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        desc.primitive.cullMode = wgpu::CullMode::Back;
        desc.primitive.frontFace = wgpu::FrontFace::CW;
        desc.depthStencil = &depthStencil;
        
        pawnPipeline_ = device_.CreateRenderPipeline(&desc);
        return pawnPipeline_ != nullptr;
    }
};


} // namespace gallery
} // namespace t7
