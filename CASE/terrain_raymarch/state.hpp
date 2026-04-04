#pragma once

/**
 * TERRAIN RAYMARCH - GPU State Management
 * =======================================
 * 
 * GPU buffer management for the raymarching visualization.
 * 
 * IDENTICAL BUFFERS
 * -----------------
 * 
 * The raymarching version uses exactly the same GPU state as rasterization:
 * - Same signal layout (time, stats, input)
 * - Same terrain/pawn/camera state
 * - Same trajectories
 * 
 * Only the rendering differs: fragment shader raymarches instead of rasterizing.
 */

#include "analysis/analysis_signal.hpp"
#include <webgpu/webgpu_cpp.h>
#include <cstring>
#include <array>

namespace t7 {
namespace terrain_raymarch {

// =============================================================================
// GPU FRAME SIGNAL - 304 bytes, matches WGSL
// =============================================================================

struct alignas(16) GPUFrameSignal {
    float t_seconds;
    float t_beats;
    float dt;
    float aspect_ratio;
    std::array<float, 64> stats;
    float move_x;
    float move_z;
    float look_az_delta;
    float look_el_delta;
    float zoom_delta;
    float pan_x_delta;
    float pan_y_delta;
    float _pad1;
};

static_assert(sizeof(GPUFrameSignal) == 304, "GPUFrameSignal must be 304 bytes");

// =============================================================================
// GPU STATE STRUCTS
// =============================================================================

struct alignas(16) GPUTrajectory {
    float value;
    float velocity;
    float _pad0;
    float _pad1;
};

struct alignas(16) GPUTerrainState {
    float amplitude_scale;
    float max_amplitude;
    float size;
    float _pad;
};

struct alignas(16) GPUPawnState {
    float pos_x;
    float pos_y;
    float pos_z;
    float heading;
    float orientation[4];
};

struct alignas(16) GPUCameraState {
    float pos_x;
    float pos_y;
    float pos_z;
    float azimuth;
    float elevation;
    float distance;
    float pan_x;
    float pan_y;
    float view_proj[16];
};

static_assert(sizeof(GPUTrajectory) == 16, "GPUTrajectory must be 16 bytes");
static_assert(sizeof(GPUTerrainState) == 16, "GPUTerrainState must be 16 bytes");
static_assert(sizeof(GPUPawnState) == 32, "GPUPawnState must be 32 bytes");
static_assert(sizeof(GPUCameraState) == 96, "GPUCameraState must be 96 bytes");

// =============================================================================
// CONSTANTS
// =============================================================================

constexpr int MAX_TRAJECTORIES = 16;

// Fog color for clear (matches WGSL FOG_COLOR)
constexpr float FOG_COLOR_R = 0.85f;
constexpr float FOG_COLOR_G = 0.78f;
constexpr float FOG_COLOR_B = 0.72f;

// =============================================================================
// GPU STATE CLASS
// =============================================================================

class GPUState {
public:
    GPUState() = default;
    
    bool init(wgpu::Device device) {
        device_ = device;
        
        if (!createBuffers()) return false;
        if (!createBindGroup()) return false;
        if (!initializeState()) return false;
        
        return true;
    }
    
    void upload_signal(wgpu::Queue& queue, const GPUFrameSignal& signal) {
        queue.WriteBuffer(signalBuffer_, 0, &signal, sizeof(GPUFrameSignal));
    }
    
    wgpu::BindGroup compute_bind_group() const { return computeBindGroup_; }
    wgpu::BindGroup render_bind_group() const { return renderBindGroup_; }
    wgpu::BindGroupLayout compute_bind_group_layout() const { return computeBindGroupLayout_; }
    wgpu::BindGroupLayout render_bind_group_layout() const { return renderBindGroupLayout_; }
    
private:
    wgpu::Device device_;
    
    wgpu::Buffer signalBuffer_;
    wgpu::Buffer terrainBuffer_;
    wgpu::Buffer pawnBuffer_;
    wgpu::Buffer cameraBuffer_;
    wgpu::Buffer trajectoriesBuffer_;
    
    wgpu::BindGroupLayout computeBindGroupLayout_;
    wgpu::BindGroupLayout renderBindGroupLayout_;
    wgpu::BindGroup computeBindGroup_;
    wgpu::BindGroup renderBindGroup_;
    
    bool createBuffers() {
        auto createBuffer = [&](const char* label, size_t size) -> wgpu::Buffer {
            wgpu::BufferDescriptor desc{};
            desc.label = label;
            desc.size = size;
            desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            return device_.CreateBuffer(&desc);
        };
        
        signalBuffer_ = createBuffer("FrameSignal", sizeof(GPUFrameSignal));
        terrainBuffer_ = createBuffer("TerrainState", sizeof(GPUTerrainState));
        pawnBuffer_ = createBuffer("PawnState", sizeof(GPUPawnState));
        cameraBuffer_ = createBuffer("CameraState", sizeof(GPUCameraState));
        trajectoriesBuffer_ = createBuffer("Trajectories", sizeof(GPUTrajectory) * MAX_TRAJECTORIES);
        
        return signalBuffer_ && terrainBuffer_ && pawnBuffer_ && cameraBuffer_ && trajectoriesBuffer_;
    }
    
    bool createBindGroup() {
        // Compute bind group layout
        std::array<wgpu::BindGroupLayoutEntry, 5> computeLayoutEntries{};
        for (int i = 0; i < 5; ++i) {
            computeLayoutEntries[i].binding = i;
            computeLayoutEntries[i].visibility = wgpu::ShaderStage::Compute;
            computeLayoutEntries[i].buffer.type = (i == 0) 
                ? wgpu::BufferBindingType::ReadOnlyStorage 
                : wgpu::BufferBindingType::Storage;
        }
        
        wgpu::BindGroupLayoutDescriptor computeLayoutDesc{};
        computeLayoutDesc.label = "Compute BindGroupLayout";
        computeLayoutDesc.entryCount = computeLayoutEntries.size();
        computeLayoutDesc.entries = computeLayoutEntries.data();
        computeBindGroupLayout_ = device_.CreateBindGroupLayout(&computeLayoutDesc);
        if (!computeBindGroupLayout_) return false;
        
        // Render bind group layout
        std::array<wgpu::BindGroupLayoutEntry, 4> renderLayoutEntries{};
        for (int i = 0; i < 4; ++i) {
            renderLayoutEntries[i].binding = 10 + i;
            renderLayoutEntries[i].visibility = wgpu::ShaderStage::Fragment;
            renderLayoutEntries[i].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
        }
        
        wgpu::BindGroupLayoutDescriptor renderLayoutDesc{};
        renderLayoutDesc.label = "Render BindGroupLayout";
        renderLayoutDesc.entryCount = renderLayoutEntries.size();
        renderLayoutDesc.entries = renderLayoutEntries.data();
        renderBindGroupLayout_ = device_.CreateBindGroupLayout(&renderLayoutDesc);
        if (!renderBindGroupLayout_) return false;
        
        // Compute bind group
        std::array<wgpu::BindGroupEntry, 5> computeEntries{};
        wgpu::Buffer buffers[] = { signalBuffer_, terrainBuffer_, pawnBuffer_, cameraBuffer_, trajectoriesBuffer_ };
        size_t sizes[] = { sizeof(GPUFrameSignal), sizeof(GPUTerrainState), sizeof(GPUPawnState), 
                          sizeof(GPUCameraState), sizeof(GPUTrajectory) * MAX_TRAJECTORIES };
        
        for (int i = 0; i < 5; ++i) {
            computeEntries[i].binding = i;
            computeEntries[i].buffer = buffers[i];
            computeEntries[i].size = sizes[i];
        }
        
        wgpu::BindGroupDescriptor computeBindGroupDesc{};
        computeBindGroupDesc.label = "Compute BindGroup";
        computeBindGroupDesc.layout = computeBindGroupLayout_;
        computeBindGroupDesc.entryCount = computeEntries.size();
        computeBindGroupDesc.entries = computeEntries.data();
        computeBindGroup_ = device_.CreateBindGroup(&computeBindGroupDesc);
        if (!computeBindGroup_) return false;
        
        // Render bind group
        std::array<wgpu::BindGroupEntry, 4> renderEntries{};
        for (int i = 0; i < 4; ++i) {
            renderEntries[i].binding = 10 + i;
            renderEntries[i].buffer = buffers[i];
            renderEntries[i].size = sizes[i];
        }
        
        wgpu::BindGroupDescriptor renderBindGroupDesc{};
        renderBindGroupDesc.label = "Render BindGroup";
        renderBindGroupDesc.layout = renderBindGroupLayout_;
        renderBindGroupDesc.entryCount = renderEntries.size();
        renderBindGroupDesc.entries = renderEntries.data();
        renderBindGroup_ = device_.CreateBindGroup(&renderBindGroupDesc);
        if (!renderBindGroup_) return false;
        
        return true;
    }
    
    bool initializeState() {
        wgpu::Queue queue = device_.GetQueue();
        
        GPUTerrainState terrain{};
        terrain.amplitude_scale = 1.0f;
        terrain.max_amplitude = 2.0f;
        terrain.size = 100.0f;
        terrain._pad = 0.0f;
        queue.WriteBuffer(terrainBuffer_, 0, &terrain, sizeof(terrain));
        
        GPUPawnState pawn{};
        pawn.pos_x = 0.0f;
        pawn.pos_y = 0.0f;
        pawn.pos_z = 0.0f;
        pawn.heading = 0.0f;
        pawn.orientation[0] = 0.0f;
        pawn.orientation[1] = 0.0f;
        pawn.orientation[2] = 0.0f;
        pawn.orientation[3] = 1.0f;
        queue.WriteBuffer(pawnBuffer_, 0, &pawn, sizeof(pawn));
        
        GPUCameraState camera{};
        camera.pos_x = 0.0f;
        camera.pos_y = 15.0f;
        camera.pos_z = 30.0f;
        camera.azimuth = 0.0f;
        camera.elevation = 0.4f;
        camera.distance = 30.0f;
        camera.pan_x = 0.0f;
        camera.pan_y = 0.0f;
        std::memset(camera.view_proj, 0, sizeof(camera.view_proj));
        camera.view_proj[0] = 1.0f;
        camera.view_proj[5] = 1.0f;
        camera.view_proj[10] = 1.0f;
        camera.view_proj[15] = 1.0f;
        queue.WriteBuffer(cameraBuffer_, 0, &camera, sizeof(camera));
        
        GPUTrajectory trajectories[MAX_TRAJECTORIES]{};
        for (int i = 0; i < MAX_TRAJECTORIES; ++i) {
            trajectories[i].value = 1.0f;
            trajectories[i].velocity = 0.0f;
        }
        queue.WriteBuffer(trajectoriesBuffer_, 0, trajectories, sizeof(trajectories));
        
        return true;
    }
};

} // namespace terrain_raymarch
} // namespace t7
