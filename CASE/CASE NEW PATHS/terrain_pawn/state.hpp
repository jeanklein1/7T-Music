#pragma once

/**
 * TERRAIN PAWN - GPU State Management
 * ====================================
 * 
 * GPU buffer management for the terrain_pawn visualization.
 * 
 * GPU-NATIVE DESIGN
 * -----------------
 * 
 * The GPU owns as much as possible:
 * - Geometry generation (vertex pulling from vertex_index)
 * - Colors (shader constants)
 * - All dynamics and animation
 * 
 * CPU provides only:
 * - Initial configuration (terrain size, max amplitude)
 * - Per-frame signal (time, stats, input)
 * - Fog color for render pass clear (must match shader constant)
 * 
 * STRUCT LAYOUT
 * -------------
 * 
 * GPUFrameSignal must EXACTLY match the WGSL FrameSignal struct (304 bytes).
 * Other structs are minimal - just what GPU needs from CPU.
 */

#include "analysis/analysis_signal.hpp"
#include <webgpu/webgpu_cpp.h>
#include <cstring>
#include <array>

namespace t7 {
namespace terrain_pawn {

// =============================================================================
// GPU FRAME SIGNAL - The combined struct uploaded to GPU
// =============================================================================
// 
// CRITICAL: This struct MUST match the WGSL FrameSignal struct exactly.
// Layout: 304 bytes total
//
//   Offset  Field                Size
//   0       t_seconds            4
//   4       t_beats              4
//   8       dt                   4
//   12      aspect_ratio         4
//   16      stats[64]            256
//   272     move_x               4
//   276     move_z               4
//   280     look_az_delta        4
//   284     look_el_delta        4
//   288     zoom_delta           4
//   292     pan_x_delta          4
//   296     pan_y_delta          4
//   300     _pad1                4
//   304     END

struct alignas(16) GPUFrameSignal {
    // ═══ TIME + VIEWPORT (16 bytes) ════════════════════════════════════════
    float t_seconds;        // 0  - from AnalysisSignal
    float t_beats;          // 4  - from AnalysisSignal
    float dt;               // 8  - from AnalysisSignal
    float aspect_ratio;     // 12 - from console
    
    // ═══ MUSICAL STATS (256 bytes) ═════════════════════════════════════════
    std::array<float, 64> stats;  // 16 - from AnalysisSignal
    
    // ═══ INPUT INTENT (32 bytes) ═══════════════════════════════════════════
    float move_x;           // 272 - from input state
    float move_z;           // 276 - from input state
    float look_az_delta;    // 280 - from input state
    float look_el_delta;    // 284 - from input state
    float zoom_delta;       // 288 - from input state
    float pan_x_delta;      // 292 - from input state
    float pan_y_delta;      // 296 - from input state
    float _pad1;            // 300 - padding
};

static_assert(sizeof(GPUFrameSignal) == 304, "GPUFrameSignal must be 304 bytes to match WGSL");

// =============================================================================
// GPU STRUCT MIRRORS - Must match WGSL exactly
// =============================================================================
// 
// CRITICAL: All structs use explicit f32 scalars to guarantee alignment match.
// Never use float[3] where WGSL has vec3 - the alignment rules differ!

// --- Trajectory (16 bytes) ---
struct alignas(16) GPUTrajectory {
    float value;
    float velocity;
    float _pad0;
    float _pad1;
};

// --- TerrainState (16 bytes) ---
// Colors are now shader constants - only dynamic/config values remain
struct alignas(16) GPUTerrainState {
    float amplitude_scale;       // 0  - trajectory-driven multiplier
    float max_amplitude;         // 4  - config: max height displacement
    float size;                  // 8  - world size
    float _pad;                  // 12
};                               // 16 bytes

// --- PawnState (32 bytes) ---
struct alignas(16) GPUPawnState {
    float pos_x;
    float pos_y;
    float pos_z;
    float heading;
    float orientation[4];  // quaternion (x, y, z, w)
};

// --- CameraState (96 bytes) ---
struct alignas(16) GPUCameraState {
    float pos_x;
    float pos_y;
    float pos_z;
    float azimuth;
    float elevation;
    float distance;
    float pan_x;
    float pan_y;
    float view_proj[16];  // mat4x4
};

static_assert(sizeof(GPUTrajectory) == 16, "GPUTrajectory must be 16 bytes");
static_assert(sizeof(GPUTerrainState) == 16, "GPUTerrainState must be 16 bytes");
static_assert(sizeof(GPUPawnState) == 32, "GPUPawnState must be 32 bytes");
static_assert(sizeof(GPUCameraState) == 96, "GPUCameraState must be 96 bytes");

// =============================================================================
// CONSTANTS
// =============================================================================

constexpr int MAX_TRAJECTORIES = 16;

// Fog color for clear (matches WGSL TERRAIN_FOG_COLOR)
constexpr float FOG_COLOR_R = 0.85f;
constexpr float FOG_COLOR_G = 0.78f;
constexpr float FOG_COLOR_B = 0.72f;

// =============================================================================
// GPU STATE CLASS
// =============================================================================

class GPUState {
public:
    GPUState() = default;
    
    /**
     * Initialize all buffers and bind groups.
     */
    bool init(wgpu::Device device) {
        device_ = device;
        
        if (!createBuffers()) return false;
        if (!createBindGroup()) return false;
        if (!initializeState()) return false;
        
        return true;
    }
    
    /**
     * Upload GPUFrameSignal to GPU.
     * Call once per frame before compute dispatch.
     */
    void upload_signal(wgpu::Queue& queue, const GPUFrameSignal& signal) {
        queue.WriteBuffer(signalBuffer_, 0, &signal, sizeof(GPUFrameSignal));
    }
    
    /**
     * Get the bind group for compute pipeline.
     */
    wgpu::BindGroup compute_bind_group() const { return computeBindGroup_; }
    
    /**
     * Get the bind group for render pipelines.
     */
    wgpu::BindGroup render_bind_group() const { return renderBindGroup_; }
    
    /**
     * Get the bind group layout for compute pipeline creation.
     */
    wgpu::BindGroupLayout compute_bind_group_layout() const { return computeBindGroupLayout_; }
    
    /**
     * Get the bind group layout for render pipeline creation.
     */
    wgpu::BindGroupLayout render_bind_group_layout() const { return renderBindGroupLayout_; }
    
private:
    wgpu::Device device_;
    
    // Buffers
    wgpu::Buffer signalBuffer_;
    wgpu::Buffer terrainBuffer_;
    wgpu::Buffer pawnBuffer_;
    wgpu::Buffer cameraBuffer_;
    wgpu::Buffer trajectoriesBuffer_;
    
    // Bind groups
    wgpu::BindGroupLayout computeBindGroupLayout_;
    wgpu::BindGroupLayout renderBindGroupLayout_;
    wgpu::BindGroup computeBindGroup_;
    wgpu::BindGroup renderBindGroup_;
    
    bool createBuffers() {
        // Signal buffer
        {
            wgpu::BufferDescriptor desc{};
            desc.label = "FrameSignal";
            desc.size = sizeof(GPUFrameSignal);
            desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            signalBuffer_ = device_.CreateBuffer(&desc);
            if (!signalBuffer_) return false;
        }
        
        // Terrain state buffer
        {
            wgpu::BufferDescriptor desc{};
            desc.label = "TerrainState";
            desc.size = sizeof(GPUTerrainState);
            desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            terrainBuffer_ = device_.CreateBuffer(&desc);
            if (!terrainBuffer_) return false;
        }
        
        // Pawn state buffer
        {
            wgpu::BufferDescriptor desc{};
            desc.label = "PawnState";
            desc.size = sizeof(GPUPawnState);
            desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            pawnBuffer_ = device_.CreateBuffer(&desc);
            if (!pawnBuffer_) return false;
        }
        
        // Camera state buffer
        {
            wgpu::BufferDescriptor desc{};
            desc.label = "CameraState";
            desc.size = sizeof(GPUCameraState);
            desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            cameraBuffer_ = device_.CreateBuffer(&desc);
            if (!cameraBuffer_) return false;
        }
        
        // Trajectories buffer
        {
            wgpu::BufferDescriptor desc{};
            desc.label = "Trajectories";
            desc.size = sizeof(GPUTrajectory) * MAX_TRAJECTORIES;
            desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            trajectoriesBuffer_ = device_.CreateBuffer(&desc);
            if (!trajectoriesBuffer_) return false;
        }
        
        return true;
    }
    
    bool createBindGroup() {
        // ─── Compute Bind Group Layout (bindings 0-4) ───────────────────────────
        std::array<wgpu::BindGroupLayoutEntry, 5> computeLayoutEntries{};
        
        // Binding 0: FrameSignal (storage, read-only for compute)
        computeLayoutEntries[0].binding = 0;
        computeLayoutEntries[0].visibility = wgpu::ShaderStage::Compute;
        computeLayoutEntries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
        computeLayoutEntries[0].buffer.minBindingSize = sizeof(GPUFrameSignal);
        
        // Binding 1: TerrainState (storage, read-write for compute)
        computeLayoutEntries[1].binding = 1;
        computeLayoutEntries[1].visibility = wgpu::ShaderStage::Compute;
        computeLayoutEntries[1].buffer.type = wgpu::BufferBindingType::Storage;
        computeLayoutEntries[1].buffer.minBindingSize = sizeof(GPUTerrainState);
        
        // Binding 2: PawnState (storage, read-write for compute)
        computeLayoutEntries[2].binding = 2;
        computeLayoutEntries[2].visibility = wgpu::ShaderStage::Compute;
        computeLayoutEntries[2].buffer.type = wgpu::BufferBindingType::Storage;
        computeLayoutEntries[2].buffer.minBindingSize = sizeof(GPUPawnState);
        
        // Binding 3: CameraState (storage, read-write for compute)
        computeLayoutEntries[3].binding = 3;
        computeLayoutEntries[3].visibility = wgpu::ShaderStage::Compute;
        computeLayoutEntries[3].buffer.type = wgpu::BufferBindingType::Storage;
        computeLayoutEntries[3].buffer.minBindingSize = sizeof(GPUCameraState);
        
        // Binding 4: Trajectories (storage, read-write for compute)
        computeLayoutEntries[4].binding = 4;
        computeLayoutEntries[4].visibility = wgpu::ShaderStage::Compute;
        computeLayoutEntries[4].buffer.type = wgpu::BufferBindingType::Storage;
        computeLayoutEntries[4].buffer.minBindingSize = sizeof(GPUTrajectory) * MAX_TRAJECTORIES;
        
        wgpu::BindGroupLayoutDescriptor computeLayoutDesc{};
        computeLayoutDesc.label = "GPUState Compute BindGroupLayout";
        computeLayoutDesc.entryCount = computeLayoutEntries.size();
        computeLayoutDesc.entries = computeLayoutEntries.data();
        computeBindGroupLayout_ = device_.CreateBindGroupLayout(&computeLayoutDesc);
        if (!computeBindGroupLayout_) return false;
        
        // ─── Render Bind Group Layout (bindings 10-13) ──────────────────────────
        std::array<wgpu::BindGroupLayoutEntry, 4> renderLayoutEntries{};
        
        // Binding 10: FrameSignal (storage, read-only)
        renderLayoutEntries[0].binding = 10;
        renderLayoutEntries[0].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        renderLayoutEntries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
        renderLayoutEntries[0].buffer.minBindingSize = sizeof(GPUFrameSignal);
        
        // Binding 11: TerrainState (read-only for render)
        renderLayoutEntries[1].binding = 11;
        renderLayoutEntries[1].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        renderLayoutEntries[1].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
        renderLayoutEntries[1].buffer.minBindingSize = sizeof(GPUTerrainState);
        
        // Binding 12: PawnState (read-only for render)
        renderLayoutEntries[2].binding = 12;
        renderLayoutEntries[2].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        renderLayoutEntries[2].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
        renderLayoutEntries[2].buffer.minBindingSize = sizeof(GPUPawnState);
        
        // Binding 13: CameraState (read-only for render)
        renderLayoutEntries[3].binding = 13;
        renderLayoutEntries[3].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
        renderLayoutEntries[3].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
        renderLayoutEntries[3].buffer.minBindingSize = sizeof(GPUCameraState);
        
        wgpu::BindGroupLayoutDescriptor renderLayoutDesc{};
        renderLayoutDesc.label = "GPUState Render BindGroupLayout";
        renderLayoutDesc.entryCount = renderLayoutEntries.size();
        renderLayoutDesc.entries = renderLayoutEntries.data();
        renderBindGroupLayout_ = device_.CreateBindGroupLayout(&renderLayoutDesc);
        if (!renderBindGroupLayout_) return false;
        
        // ─── Create Compute Bind Group ──────────────────────────────────────────
        std::array<wgpu::BindGroupEntry, 5> computeEntries{};
        
        computeEntries[0].binding = 0;
        computeEntries[0].buffer = signalBuffer_;
        computeEntries[0].size = sizeof(GPUFrameSignal);
        
        computeEntries[1].binding = 1;
        computeEntries[1].buffer = terrainBuffer_;
        computeEntries[1].size = sizeof(GPUTerrainState);
        
        computeEntries[2].binding = 2;
        computeEntries[2].buffer = pawnBuffer_;
        computeEntries[2].size = sizeof(GPUPawnState);
        
        computeEntries[3].binding = 3;
        computeEntries[3].buffer = cameraBuffer_;
        computeEntries[3].size = sizeof(GPUCameraState);
        
        computeEntries[4].binding = 4;
        computeEntries[4].buffer = trajectoriesBuffer_;
        computeEntries[4].size = sizeof(GPUTrajectory) * MAX_TRAJECTORIES;
        
        wgpu::BindGroupDescriptor computeBindGroupDesc{};
        computeBindGroupDesc.label = "GPUState Compute BindGroup";
        computeBindGroupDesc.layout = computeBindGroupLayout_;
        computeBindGroupDesc.entryCount = computeEntries.size();
        computeBindGroupDesc.entries = computeEntries.data();
        computeBindGroup_ = device_.CreateBindGroup(&computeBindGroupDesc);
        if (!computeBindGroup_) return false;
        
        // ─── Create Render Bind Group ───────────────────────────────────────────
        std::array<wgpu::BindGroupEntry, 4> renderEntries{};
        
        renderEntries[0].binding = 10;
        renderEntries[0].buffer = signalBuffer_;
        renderEntries[0].size = sizeof(GPUFrameSignal);
        
        renderEntries[1].binding = 11;
        renderEntries[1].buffer = terrainBuffer_;
        renderEntries[1].size = sizeof(GPUTerrainState);
        
        renderEntries[2].binding = 12;
        renderEntries[2].buffer = pawnBuffer_;
        renderEntries[2].size = sizeof(GPUPawnState);
        
        renderEntries[3].binding = 13;
        renderEntries[3].buffer = cameraBuffer_;
        renderEntries[3].size = sizeof(GPUCameraState);
        
        wgpu::BindGroupDescriptor renderBindGroupDesc{};
        renderBindGroupDesc.label = "GPUState Render BindGroup";
        renderBindGroupDesc.layout = renderBindGroupLayout_;
        renderBindGroupDesc.entryCount = renderEntries.size();
        renderBindGroupDesc.entries = renderEntries.data();
        renderBindGroup_ = device_.CreateBindGroup(&renderBindGroupDesc);
        if (!renderBindGroup_) return false;
        
        return true;
    }
    
    /**
     * Initialize GPU state buffers with default values.
     * 
     * CPU provides initial configuration (size, max_amplitude).
     * WGSL owns visual constants (colors, fog density).
     */
    bool initializeState() {
        wgpu::Queue queue = device_.GetQueue();
        
        // Initialize terrain state (colors are now shader constants)
        GPUTerrainState terrain{};
        terrain.amplitude_scale = 1.0f;
        terrain.max_amplitude = 2.0f;
        terrain.size = 100.0f;
        terrain._pad = 0.0f;
        queue.WriteBuffer(terrainBuffer_, 0, &terrain, sizeof(terrain));
        
        // Initialize pawn state
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
        
        // Initialize camera state
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
        
        // Initialize trajectories
        GPUTrajectory trajectories[MAX_TRAJECTORIES]{};
        for (int i = 0; i < MAX_TRAJECTORIES; ++i) {
            trajectories[i].value = 1.0f;
            trajectories[i].velocity = 0.0f;
            trajectories[i]._pad0 = trajectories[i]._pad1 = 0.0f;
        }
        queue.WriteBuffer(trajectoriesBuffer_, 0, trajectories, sizeof(trajectories));
        
        return true;
    }
};

} // namespace terrain_pawn
} // namespace t7
