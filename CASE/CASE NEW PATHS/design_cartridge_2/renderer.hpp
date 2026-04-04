#pragma once

/**
 * DESIGN CARTRIDGE 2 — Renderer
 * ==============================
 * 
 * Pipeline management for compute-first architecture.
 * 
 * PIPELINES:
 * ┌─────────────────────────┬─────────────┬──────────────────────────────┐
 * │ Pipeline                │ Dimension   │ Writes                       │
 * ├─────────────────────────┼─────────────┼──────────────────────────────┤
 * │ updateWorld             │ 0D (1,1,1)  │ ground, pawn, camera, traj   │
 * │ updateHeightField       │ 2D (32²)    │ height_field texture         │
 * │ evolveTrajectoryField   │ 2D (32²)    │ trajectory_field texture NEW │
 * │ updateTiles             │ 2D (8²)     │ tile_state texture           │
 * │ render                  │ fullscreen  │ backbuffer                   │
 * └─────────────────────────┴─────────────┴──────────────────────────────┘
 * 
 * EXECUTION ORDER (see cartridge.hpp §2):
 *   1. update_world           — 0D state
 *   2. update_height_field    — 2D height texture
 *   3. evolve_trajectory_field — 2D activation texture (NEW)
 *   4. update_tiles           — 2D tile colors (reads trajectory field)
 *   5. render                 — fullscreen raymarch
 * 
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include "cartridges/design_cartridge_2/state.hpp"
#include <webgpu/webgpu_cpp.h>
#include <string>
#include <fstream>
#include <sstream>
#include <array>
#include <iostream>

namespace t7 {
namespace design_cartridge_2 {


// ═══════════════════════════════════════════════════════════════════════════════
// §1 ENTRY POINTS — Must match world.wgsl
// ═══════════════════════════════════════════════════════════════════════════════

namespace Entry {
    // Compute (ordered by dependency)
    constexpr const char* UPDATE_WORLD              = "update_world";              // 0D
    constexpr const char* UPDATE_HEIGHT_FIELD       = "update_height_field";       // 2D
    constexpr const char* EVOLVE_TRAJECTORY_FIELD   = "evolve_trajectory_field";   // 2D NEW
    constexpr const char* UPDATE_TILES              = "update_tiles";              // 2D
    
    // Render
    constexpr const char* FULLSCREEN_VS = "fullscreen_vs";
    constexpr const char* WORLD_FS      = "world_fs";
}


// ═══════════════════════════════════════════════════════════════════════════════
// §2 RENDERER CLASS
// ═══════════════════════════════════════════════════════════════════════════════

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
    
    // ─── Compute Dispatch ───────────────────────────────────────────────────
    //
    // ORDER MATTERS: Each pass may read what previous pass wrote.
    //   1. update_world             — 0D state (reads signal, writes entities)
    //   2. update_height_field      — 2D texture (reads ground_state)
    //   3. evolve_trajectory_field  — 2D texture (reads signal, pawn, prev field)
    //   4. update_tiles             — 2D texture (reads trajectory field)
    
    void dispatch_update_world(
        wgpu::ComputePassEncoder& pass,
        wgpu::BindGroup entityBindGroup,
        wgpu::BindGroup textureBindGroup
    ) {
        pass.SetPipeline(updateWorldPipeline_);
        pass.SetBindGroup(0, entityBindGroup);
        pass.SetBindGroup(1, textureBindGroup);
        pass.DispatchWorkgroups(1, 1, 1);  // 0D: single invocation
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
        pass.DispatchWorkgroups(workgroups, workgroups, 1);  // 2D: 32×32 workgroups of 8×8
    }
    
    // NEW: Evolve trajectory field
    //
    // Per-texel activation evolves based on:
    //   - Global stimulus: polyphony from signal
    //   - Local stimulus: pawn proximity
    //   - Previous activation: spring attack / exponential decay
    //
    void dispatch_evolve_trajectory_field(
        wgpu::ComputePassEncoder& pass,
        wgpu::BindGroup entityBindGroup,
        wgpu::BindGroup textureBindGroup,
        uint32_t workgroups
    ) {
        pass.SetPipeline(evolveTrajectoryFieldPipeline_);
        pass.SetBindGroup(0, entityBindGroup);
        pass.SetBindGroup(1, textureBindGroup);
        pass.DispatchWorkgroups(workgroups, workgroups, 1);  // 2D: 32×32 workgroups of 8×8
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
        pass.DispatchWorkgroups(workgroups, workgroups, 1);  // 2D: 8×8 workgroups of 8×8
    }
    
    // ─── Render ─────────────────────────────────────────────────────────────
    //
    // Fullscreen triangle. Fragment shader raymarches, samples precomputed textures.
    
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
    
    // ─── Pipelines ──────────────────────────────────────────────────────────
    wgpu::ComputePipeline updateWorldPipeline_;             // 0D
    wgpu::ComputePipeline updateHeightFieldPipeline_;       // 2D
    wgpu::ComputePipeline evolveTrajectoryFieldPipeline_;   // 2D NEW
    wgpu::ComputePipeline updateTilesPipeline_;             // 2D
    wgpu::RenderPipeline renderPipeline_;                   // fullscreen
    
    // ─────────────────────────────────────────────────────────────────────────
    // CREATION METHODS
    // ─────────────────────────────────────────────────────────────────────────
    
    bool loadShader() {
        // Shader lives inside the cartridge folder
        std::array<const char*, 6> paths = {
            "src/cartridges/design_cartridge_2/world.wgsl",
            "cartridges/design_cartridge_2/world.wgsl",
            "../src/cartridges/design_cartridge_2/world.wgsl",
            "design_cartridge_2/world.wgsl",
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
        desc.label = "world.wgsl (Design Cartridge 2)";
        
        shaderModule_ = device_.CreateShaderModule(&desc);
        return shaderModule_ != nullptr;
    }
    
    bool createComputePipelines() {
        // ─── Shared pipeline layout for all compute passes ──────────────────
        std::array<wgpu::BindGroupLayout, 2> computeLayouts = {
            computeEntityLayout_,
            computeTextureLayout_
        };
        
        wgpu::PipelineLayoutDescriptor layoutDesc{};
        layoutDesc.bindGroupLayoutCount = computeLayouts.size();
        layoutDesc.bindGroupLayouts = computeLayouts.data();
        wgpu::PipelineLayout computeLayout = device_.CreatePipelineLayout(&layoutDesc);
        if (!computeLayout) return false;
        
        // ─── Pipeline 1: update_world (0D) ──────────────────────────────────
        //
        // Workgroups: (1, 1, 1)
        // Reads: signal, config
        // Writes: ground_state, pawn_state, camera_state, trajectories
        {
            wgpu::ComputePipelineDescriptor desc{};
            desc.label = "Update World (0D)";
            desc.layout = computeLayout;
            desc.compute.module = shaderModule_;
            desc.compute.entryPoint = Entry::UPDATE_WORLD;
            
            updateWorldPipeline_ = device_.CreateComputePipeline(&desc);
            if (!updateWorldPipeline_) return false;
        }
        
        // ─── Pipeline 2: update_height_field (2D) ───────────────────────────
        //
        // Workgroups: (32, 32, 1) with workgroup_size(8, 8)
        // Reads: signal, ground_state
        // Writes: height_field texture (RGBA16Float)
        {
            wgpu::ComputePipelineDescriptor desc{};
            desc.label = "Update Height Field (2D)";
            desc.layout = computeLayout;
            desc.compute.module = shaderModule_;
            desc.compute.entryPoint = Entry::UPDATE_HEIGHT_FIELD;
            
            updateHeightFieldPipeline_ = device_.CreateComputePipeline(&desc);
            if (!updateHeightFieldPipeline_) return false;
        }
        
        // ─── Pipeline 3: evolve_trajectory_field (2D) — NEW ─────────────────
        //
        // Workgroups: (32, 32, 1) with workgroup_size(8, 8)
        // Reads: signal, pawn_state, trajectory_field_read
        // Writes: trajectory_field_write (RG32Float)
        //
        // Per-texel spring/decay dynamics for activation visualization
        {
            wgpu::ComputePipelineDescriptor desc{};
            desc.label = "Evolve Trajectory Field (2D)";
            desc.layout = computeLayout;
            desc.compute.module = shaderModule_;
            desc.compute.entryPoint = Entry::EVOLVE_TRAJECTORY_FIELD;
            
            evolveTrajectoryFieldPipeline_ = device_.CreateComputePipeline(&desc);
            if (!evolveTrajectoryFieldPipeline_) return false;
        }
        
        // ─── Pipeline 4: update_tiles (2D) ──────────────────────────────────
        //
        // Workgroups: (8, 8, 1) with workgroup_size(8, 8)
        // Reads: signal, ground_state, trajectory_field
        // Writes: tile_state texture (RGBA8Unorm)
        {
            wgpu::ComputePipelineDescriptor desc{};
            desc.label = "Update Tiles (2D)";
            desc.layout = computeLayout;
            desc.compute.module = shaderModule_;
            desc.compute.entryPoint = Entry::UPDATE_TILES;
            
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
        fragment.entryPoint = Entry::WORLD_FS;
        fragment.targetCount = 1;
        fragment.targets = &colorTarget;
        
        wgpu::RenderPipelineDescriptor desc{};
        desc.label = "World Raymarch (Design Cartridge 2)";
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

} // namespace design_cartridge_2
} // namespace t7
