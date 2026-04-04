#pragma once

/**
 * PAWN RASTERIZE — Renderer
 * ==========================
 * 
 * Pipeline management for rasterized pawn viewing.
 * Compute pass (camera), Render pass (mesh).
 * 
 * HOT RELOAD: Call reload() when shader file changes.
 *
 * See world.wgsl for the GPU scroll.
 */

#include <webgpu/webgpu_cpp.h>
#include <string>
#include <fstream>
#include <sstream>
#include <array>
#include <iostream>

namespace t7 {
namespace pawn_rasterize {


namespace Entry {
    constexpr const char* UPDATE_CAMERA = "update_camera";
    constexpr const char* MESH_VS = "mesh_vs";
    constexpr const char* MESH_FS = "mesh_fs";
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
        if (!createRenderPipeline()) return false;
        
        std::cout << "[Pawn Rasterize] Renderer initialized\n";
        return true;
    }
    
    // ─── Dispatch ─────────────────────────────────────────────────────────────
    
    void dispatch_update_camera(
        wgpu::ComputePassEncoder& pass,
        wgpu::BindGroup bindGroup
    ) {
        pass.SetPipeline(updateCameraPipeline_);
        pass.SetBindGroup(0, bindGroup);
        pass.DispatchWorkgroups(1, 1, 1);
    }
    
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
    
    // ─── Hot Reload ───────────────────────────────────────────────────────────
    
    bool reload() {
        std::cout << "[Pawn Rasterize] Reloading shader...\n";
        
        if (!loadShader()) {
            std::cerr << "[Pawn Rasterize] Reload failed: shader load error\n";
            return false;
        }
        if (!createComputePipeline()) {
            std::cerr << "[Pawn Rasterize] Reload failed: compute pipeline error\n";
            return false;
        }
        if (!createRenderPipeline()) {
            std::cerr << "[Pawn Rasterize] Reload failed: render pipeline error\n";
            return false;
        }
        
        std::cout << "[Pawn Rasterize] Reload successful\n";
        return true;
    }
    
    const std::string& shader_path() const { return shaderPath_; }
    
private:
    wgpu::Device device_;
    wgpu::BindGroupLayout computeLayout_;
    wgpu::BindGroupLayout renderLayout_;
    wgpu::TextureFormat colorFormat_;
    wgpu::TextureFormat depthFormat_;
    
    wgpu::ShaderModule shaderModule_;
    std::string shaderSource_;
    std::string shaderPath_;
    
    wgpu::ComputePipeline updateCameraPipeline_;
    wgpu::RenderPipeline renderPipeline_;
    
    bool loadShader() {
        // Reload case: use known path
        if (!shaderPath_.empty()) {
            std::ifstream file(shaderPath_);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                shaderSource_ = buffer.str();
            } else {
                std::cerr << "[Pawn Rasterize] Cannot reopen: " << shaderPath_ << "\n";
                return false;
            }
        } else {
            // First load: search for file
            std::array<const char*, 4> paths = {
                "cartridges/pawn_rasterize/world.wgsl",
                "src/cartridges/pawn_rasterize/world.wgsl",
                "../src/cartridges/pawn_rasterize/world.wgsl",
                "pawn_rasterize/world.wgsl"
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
                std::cerr << "[Pawn Rasterize] Could not find shader\n";
                return false;
            }
            
            std::cout << "[Pawn Rasterize] Loaded shader from: " << shaderPath_ << "\n";
        }
        
        wgpu::ShaderSourceWGSL wgslSource{};
        wgslSource.code = shaderSource_.c_str();
        
        wgpu::ShaderModuleDescriptor desc{};
        desc.nextInChain = &wgslSource;
        desc.label = "Pawn Rasterize Shader";
        
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
        desc.label = "Pawn Update Camera";
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
        
        // Vertex layout: position + normal
        wgpu::VertexAttribute attributes[2];
        attributes[0].format = wgpu::VertexFormat::Float32x3;
        attributes[0].offset = 0;
        attributes[0].shaderLocation = 0;
        
        attributes[1].format = wgpu::VertexFormat::Float32x3;
        attributes[1].offset = 12;
        attributes[1].shaderLocation = 1;
        
        wgpu::VertexBufferLayout vertexLayout{};
        vertexLayout.arrayStride = 24;
        vertexLayout.stepMode = wgpu::VertexStepMode::Vertex;
        vertexLayout.attributeCount = 2;
        vertexLayout.attributes = attributes;
        
        // Color target
        wgpu::ColorTargetState colorTarget{};
        colorTarget.format = colorFormat_;
        colorTarget.writeMask = wgpu::ColorWriteMask::All;
        
        wgpu::FragmentState fragment{};
        fragment.module = shaderModule_;
        fragment.entryPoint = Entry::MESH_FS;
        fragment.targetCount = 1;
        fragment.targets = &colorTarget;
        
        // Depth
        wgpu::DepthStencilState depthStencil{};
        depthStencil.format = depthFormat_;
        depthStencil.depthWriteEnabled = true;
        depthStencil.depthCompare = wgpu::CompareFunction::Less;
        
        // Pipeline
        wgpu::RenderPipelineDescriptor desc{};
        desc.label = "Pawn Mesh Render";
        desc.layout = layout;
        desc.vertex.module = shaderModule_;
        desc.vertex.entryPoint = Entry::MESH_VS;
        desc.vertex.bufferCount = 1;
        desc.vertex.buffers = &vertexLayout;
        desc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        desc.primitive.cullMode = wgpu::CullMode::Back;
        desc.primitive.frontFace = wgpu::FrontFace::CCW;
        desc.depthStencil = &depthStencil;
        desc.fragment = &fragment;
        
        renderPipeline_ = device_.CreateRenderPipeline(&desc);
        return renderPipeline_ != nullptr;
    }
};


} // namespace pawn_rasterize
} // namespace t7
