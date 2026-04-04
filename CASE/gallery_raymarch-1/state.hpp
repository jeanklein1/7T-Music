#pragma once

/**
 * GALLERY CARTRIDGE — GPU State Management
 * =========================================
 *
 * Updated to use Portable Door system.
 *
 * CHANGES FROM PREVIOUS VERSION:
 *   - GPUDoor (64 bytes) → GPUPortableDoor (96 bytes)
 *   - DoorBuilder → PortableDoorBuilder  
 *   - Door array buffer size updated
 *
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include <webgpu/webgpu_cpp.h>
#include <cstring>
#include <cmath>
#include <array>
#include <iostream>

namespace t7 {
namespace gallery {


// ═══════════════════════════════════════════════════════════════════════════════
// §1 CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════════

constexpr uint32_t MAX_DOORS = 8;

// Room dimensions
constexpr float ROOM_HALF_WIDTH = 15.0f;
constexpr float ROOM_HEIGHT = 6.0f;
constexpr float ROOM_HALF_DEPTH = 15.0f;

// Pawn dimensions
constexpr float PAWN_RADIUS = 0.5f;
constexpr float PAWN_HEIGHT = 1.5f;

// Colors (for clear color)
constexpr float FOG_COLOR_R = 0.88f;
constexpr float FOG_COLOR_G = 0.90f;
constexpr float FOG_COLOR_B = 0.92f;


// ═══════════════════════════════════════════════════════════════════════════════
// §2 DOOR SHAPE TYPES
// ═══════════════════════════════════════════════════════════════════════════════

namespace DoorShape {
    constexpr uint32_t RECTANGLE    = 0;
    constexpr uint32_t ARCH         = 1;
    constexpr uint32_t CIRCLE       = 2;
    constexpr uint32_t POINTED_ARCH = 3;
    // Custom shapes: 100+
}

namespace DoorTarget {
    constexpr uint32_t NONE = 0xFFFFFFFF;
    constexpr uint32_t GALLERY = 0;
    constexpr uint32_t N_DIMENSIONAL_2 = 1;
    constexpr uint32_t PLAYGROUND_RAYMARCH = 2;
    constexpr uint32_t PLAYGROUND_RASTERIZE = 3;
    constexpr uint32_t PLAYGROUND_HYBRID = 4;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §3 INITIAL STATE
// ═══════════════════════════════════════════════════════════════════════════════

namespace Initial {
    constexpr float PAWN_POS_X = 0.0f;
    constexpr float PAWN_POS_Y = 0.0f;
    constexpr float PAWN_POS_Z = 5.0f;
    constexpr float PAWN_HEADING = 3.14159f;
    
    constexpr float CAMERA_AZIMUTH = 0.0f;
    constexpr float CAMERA_ELEVATION = 0.4f;
    constexpr float CAMERA_DISTANCE = 10.0f;
    constexpr float CAMERA_PAN_X = 0.0f;
    constexpr float CAMERA_PAN_Y = 0.0f;
    
    constexpr float PAWN_SPEED = 8.0f;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §4 COUPLING BITS
// ═══════════════════════════════════════════════════════════════════════════════

namespace Coupling {
    constexpr uint32_t INPUT_MOVES_PAWN       = 1u << 0;
    constexpr uint32_t PAWN_TO_CAMERA_TARGET  = 1u << 1;
    constexpr uint32_t INPUT_ORBITS_CAMERA    = 1u << 2;
    constexpr uint32_t INPUT_ZOOMS_CAMERA     = 1u << 3;
    
    constexpr uint32_t ALL = 0xFu;
    constexpr uint32_t NONE = 0u;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §5 GPU STRUCTURES — Must match world.wgsl exactly
// ═══════════════════════════════════════════════════════════════════════════════

struct alignas(16) GPUFrameSignal {
    float t_seconds;
    float dt;
    float aspect_ratio;
    float _pad0;
    float move_x;
    float move_z;
    float look_az_delta;
    float look_el_delta;
    float zoom_delta;
    float pan_x_delta;
    float pan_y_delta;
    float _pad1;
};

struct alignas(16) GPUDesignConfig {
    uint32_t mute_dynamics;
    uint32_t mute_couplings;
    float pawn_speed;
    float _pad0;
};

struct alignas(16) GPUPawnState {
    float pos[3];
    float heading;
    float orientation[4];
};

struct alignas(16) GPUCameraState {
    float pos[3];
    float azimuth;
    float elevation;
    float distance;
    float pan_x;
    float pan_y;
};

// ─── Portable Door Structure (96 bytes) ──────────────────────────────────────
//
// Key change: forward/up vectors instead of wall normal.
// This allows doors to be placed anywhere, at any orientation.

struct alignas(16) GPUPortableDoor {
    // Placement (48 bytes)
    float position[3];          // Bottom center of door frame
    float _pad0;
    float forward[3];           // Direction "through" door (entry → exit)
    float _pad1;
    float up[3];                // Up direction (typically 0,1,0)
    float _pad2;
    
    // Dimensions (16 bytes)
    float width;
    float height;
    float thickness;            // Passable depth
    float commitment_depth;     // How far past center to trigger
    
    // Identity (16 bytes)
    uint32_t shape_type;
    uint32_t target_id;
    uint32_t _pad3;
    uint32_t _pad4;
    
    // Appearance (16 bytes)
    float color[3];
    float glow;
};

struct alignas(16) GPUDoorHeader {
    uint32_t count;
    uint32_t _pad0;
    uint32_t _pad1;
    uint32_t _pad2;
};

// Size verification
static_assert(sizeof(GPUFrameSignal) == 48, "GPUFrameSignal must be 48 bytes");
static_assert(sizeof(GPUDesignConfig) == 16, "GPUDesignConfig must be 16 bytes");
static_assert(sizeof(GPUPawnState) == 32, "GPUPawnState must be 32 bytes");
static_assert(sizeof(GPUCameraState) == 32, "GPUCameraState must be 32 bytes");
static_assert(sizeof(GPUPortableDoor) == 96, "GPUPortableDoor must be 96 bytes");
static_assert(sizeof(GPUDoorHeader) == 16, "GPUDoorHeader must be 16 bytes");


// ═══════════════════════════════════════════════════════════════════════════════
// §6 PORTABLE DOOR — CPU-side queries and state
// ═══════════════════════════════════════════════════════════════════════════════

class PortableDoor {
public:
    GPUPortableDoor gpu;
    
    // Derived (computed on build)
    float right[3];
    
    // Runtime state
    bool committed = false;
    
    void compute_derived() {
        // right = cross(up, forward)
        right[0] = gpu.up[1] * gpu.forward[2] - gpu.up[2] * gpu.forward[1];
        right[1] = gpu.up[2] * gpu.forward[0] - gpu.up[0] * gpu.forward[2];
        right[2] = gpu.up[0] * gpu.forward[1] - gpu.up[1] * gpu.forward[0];
        
        float len = std::sqrt(right[0]*right[0] + right[1]*right[1] + right[2]*right[2]);
        if (len > 0.0001f) {
            right[0] /= len;
            right[1] /= len;
            right[2] /= len;
        }
    }
    
    // Transform world → door-local
    void world_to_local(float wx, float wy, float wz,
                        float& lx, float& ly, float& lz) const {
        float dx = wx - gpu.position[0];
        float dy = wy - gpu.position[1];
        float dz = wz - gpu.position[2];
        
        lx = dx * right[0]       + dy * right[1]       + dz * right[2];
        ly = dx * gpu.up[0]      + dy * gpu.up[1]      + dz * gpu.up[2];
        lz = dx * gpu.forward[0] + dy * gpu.forward[1] + dz * gpu.forward[2];
    }
    
    // 2D SDF in door's XY plane
    float sdf_2d(float lx, float ly) const {
        float half_w = gpu.width * 0.5f;
        float h = gpu.height;
        
        switch (gpu.shape_type) {
            case DoorShape::RECTANGLE: {
                float cx = 0.0f, cy = h * 0.5f;
                float dx = std::abs(lx - cx) - half_w;
                float dy = std::abs(ly - cy) - h * 0.5f;
                return std::sqrt(std::max(dx,0.f)*std::max(dx,0.f) + 
                                 std::max(dy,0.f)*std::max(dy,0.f)) +
                       std::min(std::max(dx, dy), 0.0f);
            }
            case DoorShape::ARCH: {
                float arch_y = h - half_w;
                if (ly > arch_y) {
                    return std::sqrt(lx*lx + (ly-arch_y)*(ly-arch_y)) - half_w;
                } else {
                    float dx = std::abs(lx) - half_w;
                    float dy = std::abs(ly) - arch_y;
                    return std::sqrt(std::max(dx,0.f)*std::max(dx,0.f) + 
                                     std::max(dy,0.f)*std::max(dy,0.f)) +
                           std::min(std::max(dx, dy), 0.0f);
                }
            }
            case DoorShape::CIRCLE: {
                float radius = std::min(half_w, h * 0.5f);
                return std::sqrt(lx*lx + (ly-radius)*(ly-radius)) - radius;
            }
            case DoorShape::POINTED_ARCH: {
                float arch_y = h * 0.6f;
                if (ly > arch_y) {
                    float radius = half_w * 1.5f;
                    float d_left = std::sqrt((lx + half_w*0.5f)*(lx + half_w*0.5f) + 
                                             (ly - arch_y)*(ly - arch_y)) - radius;
                    float d_right = std::sqrt((lx - half_w*0.5f)*(lx - half_w*0.5f) + 
                                              (ly - arch_y)*(ly - arch_y)) - radius;
                    return std::max(d_left, d_right);
                } else {
                    float dx = std::abs(lx) - half_w;
                    float dy = std::abs(ly) - arch_y;
                    return std::sqrt(std::max(dx,0.f)*std::max(dx,0.f) + 
                                     std::max(dy,0.f)*std::max(dy,0.f)) +
                           std::min(std::max(dx, dy), 0.0f);
                }
            }
            default:
                return 0.0f;
        }
    }
    
    // Is point inside door's 2D silhouette?
    bool point_in_silhouette(float wx, float wy, float wz) const {
        float lx, ly, lz;
        world_to_local(wx, wy, wz, lx, ly, lz);
        return sdf_2d(lx, ly) < 0.0f;
    }
    
    // Is point inside door's passable volume?
    bool point_in_volume(float wx, float wy, float wz) const {
        float lx, ly, lz;
        world_to_local(wx, wy, wz, lx, ly, lz);
        if (sdf_2d(lx, ly) >= 0.0f) return false;
        return std::abs(lz) < gpu.thickness * 0.5f;
    }
    
    // Is point past commitment threshold?
    bool point_committed(float wx, float wy, float wz) const {
        float lx, ly, lz;
        world_to_local(wx, wy, wz, lx, ly, lz);
        if (sdf_2d(lx, ly) >= 0.0f) return false;
        return lz > gpu.commitment_depth;
    }
    
    // Update state, return true if committed this frame
    bool update(float px, float py, float pz) {
        committed = point_committed(px, py, pz);
        return committed;
    }
    
    uint32_t target() const { return gpu.target_id; }
};


// ═══════════════════════════════════════════════════════════════════════════════
// §7 DOOR BUILDER — Fluent API
// ═══════════════════════════════════════════════════════════════════════════════

class PortableDoorBuilder {
public:
    PortableDoorBuilder() {
        door_.gpu = {};
        door_.gpu.forward[2] = 1.0f;  // Default: faces +Z
        door_.gpu.up[1] = 1.0f;       // Default: Y-up
        door_.gpu.width = 2.0f;
        door_.gpu.height = 3.0f;
        door_.gpu.thickness = 4.0f;
        door_.gpu.commitment_depth = 1.5f;
        door_.gpu.shape_type = DoorShape::RECTANGLE;
        door_.gpu.target_id = DoorTarget::NONE;
        door_.gpu.color[0] = 0.8f;
        door_.gpu.color[1] = 0.8f;
        door_.gpu.color[2] = 0.8f;
    }
    
    // ─── Placement ───────────────────────────────────────────────────────────
    
    PortableDoorBuilder& at(float x, float y, float z) {
        door_.gpu.position[0] = x;
        door_.gpu.position[1] = y;
        door_.gpu.position[2] = z;
        return *this;
    }
    
    PortableDoorBuilder& facing(float fx, float fy, float fz) {
        float len = std::sqrt(fx*fx + fy*fy + fz*fz);
        if (len > 0.0001f) {
            door_.gpu.forward[0] = fx / len;
            door_.gpu.forward[1] = fy / len;
            door_.gpu.forward[2] = fz / len;
        }
        return *this;
    }
    
    // Convenience: door in back wall (faces -Z, walk through toward -Z)
    PortableDoorBuilder& in_back_wall(float x = 0.0f) {
        return at(x, 0.0f, -ROOM_HALF_DEPTH).facing(0.0f, 0.0f, -1.0f);
    }
    
    // Convenience: door in front wall (faces +Z)
    PortableDoorBuilder& in_front_wall(float x = 0.0f) {
        return at(x, 0.0f, ROOM_HALF_DEPTH).facing(0.0f, 0.0f, 1.0f);
    }
    
    // Convenience: door in left wall (faces -X)
    PortableDoorBuilder& in_left_wall(float z = 0.0f) {
        return at(-ROOM_HALF_WIDTH, 0.0f, z).facing(-1.0f, 0.0f, 0.0f);
    }
    
    // Convenience: door in right wall (faces +X)
    PortableDoorBuilder& in_right_wall(float z = 0.0f) {
        return at(ROOM_HALF_WIDTH, 0.0f, z).facing(1.0f, 0.0f, 0.0f);
    }
    
    // ─── Dimensions ──────────────────────────────────────────────────────────
    
    PortableDoorBuilder& size(float w, float h) {
        door_.gpu.width = w;
        door_.gpu.height = h;
        return *this;
    }
    
    PortableDoorBuilder& thickness(float t) {
        door_.gpu.thickness = t;
        return *this;
    }
    
    PortableDoorBuilder& commitment(float d) {
        door_.gpu.commitment_depth = d;
        return *this;
    }
    
    // ─── Identity ────────────────────────────────────────────────────────────
    
    PortableDoorBuilder& shape(uint32_t type) {
        door_.gpu.shape_type = type;
        return *this;
    }
    
    PortableDoorBuilder& target(uint32_t id) {
        door_.gpu.target_id = id;
        return *this;
    }
    
    // ─── Appearance ──────────────────────────────────────────────────────────
    
    PortableDoorBuilder& color(float r, float g, float b) {
        door_.gpu.color[0] = r;
        door_.gpu.color[1] = g;
        door_.gpu.color[2] = b;
        return *this;
    }
    
    PortableDoor build() {
        door_.compute_derived();
        return door_;
    }
    
private:
    PortableDoor door_;
};


// ═══════════════════════════════════════════════════════════════════════════════
// §8 GPU STATE CLASS
// ═══════════════════════════════════════════════════════════════════════════════

class GPUState {
public:
    GPUState() = default;
    
    bool init(wgpu::Device device) {
        device_ = device;
        
        if (!createBuffers()) return false;
        if (!createBindGroups()) return false;
        if (!initializeState()) return false;
        
        return true;
    }
    
    // ─── Uploads ─────────────────────────────────────────────────────────────
    
    void upload_signal(wgpu::Queue& queue, const GPUFrameSignal& signal) {
        queue.WriteBuffer(signalBuffer_, 0, &signal, sizeof(GPUFrameSignal));
    }
    
    void upload_config(wgpu::Queue& queue) {
        queue.WriteBuffer(configBuffer_, 0, &config_, sizeof(GPUDesignConfig));
    }
    
    void upload_doors(wgpu::Queue& queue, const PortableDoor* doors, uint32_t count) {
        GPUDoorHeader header{};
        header.count = count;
        queue.WriteBuffer(doorHeaderBuffer_, 0, &header, sizeof(header));
        
        // Upload door GPU structs
        for (uint32_t i = 0; i < count && i < MAX_DOORS; ++i) {
            queue.WriteBuffer(
                doorArrayBuffer_,
                i * sizeof(GPUPortableDoor),
                &doors[i].gpu,
                sizeof(GPUPortableDoor)
            );
        }
    }
    
    void reset_pawn(wgpu::Queue& queue) {
        GPUPawnState pawn{};
        pawn.pos[0] = Initial::PAWN_POS_X;
        pawn.pos[1] = Initial::PAWN_POS_Y;
        pawn.pos[2] = Initial::PAWN_POS_Z;
        pawn.heading = Initial::PAWN_HEADING;
        pawn.orientation[1] = 1.0f;
        queue.WriteBuffer(pawnBuffer_, 0, &pawn, sizeof(pawn));
    }
    
    // ─── Config ──────────────────────────────────────────────────────────────
    
    void set_mute_couplings(uint32_t mask) { config_.mute_couplings = mask; }
    
    // ─── Accessors ───────────────────────────────────────────────────────────
    
    wgpu::BindGroupLayout compute_bind_group_layout() const { return computeBindGroupLayout_; }
    wgpu::BindGroupLayout render_bind_group_layout() const { return renderBindGroupLayout_; }
    wgpu::BindGroup compute_bind_group() const { return computeBindGroup_; }
    wgpu::BindGroup render_bind_group() const { return renderBindGroup_; }
    
private:
    wgpu::Device device_;
    
    wgpu::Buffer signalBuffer_;
    wgpu::Buffer configBuffer_;
    wgpu::Buffer pawnBuffer_;
    wgpu::Buffer cameraBuffer_;
    wgpu::Buffer doorHeaderBuffer_;
    wgpu::Buffer doorArrayBuffer_;
    
    wgpu::BindGroupLayout computeBindGroupLayout_;
    wgpu::BindGroupLayout renderBindGroupLayout_;
    wgpu::BindGroup computeBindGroup_;
    wgpu::BindGroup renderBindGroup_;
    
    GPUDesignConfig config_{};
    
    bool createBuffers() {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        
        desc.label = "Gallery Signal";
        desc.size = sizeof(GPUFrameSignal);
        signalBuffer_ = device_.CreateBuffer(&desc);
        if (!signalBuffer_) return false;
        
        desc.label = "Gallery Config";
        desc.size = sizeof(GPUDesignConfig);
        configBuffer_ = device_.CreateBuffer(&desc);
        if (!configBuffer_) return false;
        
        desc.label = "Gallery Pawn";
        desc.size = sizeof(GPUPawnState);
        pawnBuffer_ = device_.CreateBuffer(&desc);
        if (!pawnBuffer_) return false;
        
        desc.label = "Gallery Camera";
        desc.size = sizeof(GPUCameraState);
        cameraBuffer_ = device_.CreateBuffer(&desc);
        if (!cameraBuffer_) return false;
        
        desc.label = "Gallery Door Header";
        desc.size = sizeof(GPUDoorHeader);
        doorHeaderBuffer_ = device_.CreateBuffer(&desc);
        if (!doorHeaderBuffer_) return false;
        
        // NOTE: 96 bytes per door now (was 64)
        desc.label = "Gallery Doors";
        desc.size = MAX_DOORS * sizeof(GPUPortableDoor);
        doorArrayBuffer_ = device_.CreateBuffer(&desc);
        if (!doorArrayBuffer_) return false;
        
        return true;
    }
    
    bool createBindGroups() {
        // Compute: 0=signal, 1=config, 2=pawn(rw), 3=camera(rw), 4=doorHeader, 5=doors
        {
            std::array<wgpu::BindGroupLayoutEntry, 6> entries{};
            
            entries[0].binding = 0;
            entries[0].visibility = wgpu::ShaderStage::Compute;
            entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[1].binding = 1;
            entries[1].visibility = wgpu::ShaderStage::Compute;
            entries[1].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[2].binding = 2;
            entries[2].visibility = wgpu::ShaderStage::Compute;
            entries[2].buffer.type = wgpu::BufferBindingType::Storage;
            
            entries[3].binding = 3;
            entries[3].visibility = wgpu::ShaderStage::Compute;
            entries[3].buffer.type = wgpu::BufferBindingType::Storage;
            
            entries[4].binding = 4;
            entries[4].visibility = wgpu::ShaderStage::Compute;
            entries[4].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[5].binding = 5;
            entries[5].visibility = wgpu::ShaderStage::Compute;
            entries[5].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            wgpu::BindGroupLayoutDescriptor desc{};
            desc.label = "Gallery Compute Layout";
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            computeBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
            if (!computeBindGroupLayout_) return false;
        }
        
        // Render: 10=signal, 11=pawn, 12=camera, 13=doorHeader, 14=doors
        {
            std::array<wgpu::BindGroupLayoutEntry, 5> entries{};
            
            entries[0].binding = 10;
            entries[0].visibility = wgpu::ShaderStage::Fragment;
            entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[1].binding = 11;
            entries[1].visibility = wgpu::ShaderStage::Fragment;
            entries[1].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[2].binding = 12;
            entries[2].visibility = wgpu::ShaderStage::Fragment;
            entries[2].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[3].binding = 13;
            entries[3].visibility = wgpu::ShaderStage::Fragment;
            entries[3].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[4].binding = 14;
            entries[4].visibility = wgpu::ShaderStage::Fragment;
            entries[4].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            wgpu::BindGroupLayoutDescriptor desc{};
            desc.label = "Gallery Render Layout";
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            renderBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
            if (!renderBindGroupLayout_) return false;
        }
        
        // Create bind groups
        {
            std::array<wgpu::BindGroupEntry, 6> entries{};
            
            entries[0].binding = 0;
            entries[0].buffer = signalBuffer_;
            entries[0].size = sizeof(GPUFrameSignal);
            
            entries[1].binding = 1;
            entries[1].buffer = configBuffer_;
            entries[1].size = sizeof(GPUDesignConfig);
            
            entries[2].binding = 2;
            entries[2].buffer = pawnBuffer_;
            entries[2].size = sizeof(GPUPawnState);
            
            entries[3].binding = 3;
            entries[3].buffer = cameraBuffer_;
            entries[3].size = sizeof(GPUCameraState);
            
            entries[4].binding = 4;
            entries[4].buffer = doorHeaderBuffer_;
            entries[4].size = sizeof(GPUDoorHeader);
            
            entries[5].binding = 5;
            entries[5].buffer = doorArrayBuffer_;
            entries[5].size = MAX_DOORS * sizeof(GPUPortableDoor);
            
            wgpu::BindGroupDescriptor desc{};
            desc.label = "Gallery Compute BindGroup";
            desc.layout = computeBindGroupLayout_;
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            computeBindGroup_ = device_.CreateBindGroup(&desc);
            if (!computeBindGroup_) return false;
        }
        
        {
            std::array<wgpu::BindGroupEntry, 5> entries{};
            
            entries[0].binding = 10;
            entries[0].buffer = signalBuffer_;
            entries[0].size = sizeof(GPUFrameSignal);
            
            entries[1].binding = 11;
            entries[1].buffer = pawnBuffer_;
            entries[1].size = sizeof(GPUPawnState);
            
            entries[2].binding = 12;
            entries[2].buffer = cameraBuffer_;
            entries[2].size = sizeof(GPUCameraState);
            
            entries[3].binding = 13;
            entries[3].buffer = doorHeaderBuffer_;
            entries[3].size = sizeof(GPUDoorHeader);
            
            entries[4].binding = 14;
            entries[4].buffer = doorArrayBuffer_;
            entries[4].size = MAX_DOORS * sizeof(GPUPortableDoor);
            
            wgpu::BindGroupDescriptor desc{};
            desc.label = "Gallery Render BindGroup";
            desc.layout = renderBindGroupLayout_;
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            renderBindGroup_ = device_.CreateBindGroup(&desc);
            if (!renderBindGroup_) return false;
        }
        
        return true;
    }
    
    bool initializeState() {
        wgpu::Queue queue = device_.GetQueue();
        
        config_.mute_dynamics = 0;
        config_.mute_couplings = Coupling::NONE;
        config_.pawn_speed = Initial::PAWN_SPEED;
        queue.WriteBuffer(configBuffer_, 0, &config_, sizeof(config_));
        
        GPUPawnState pawn{};
        pawn.pos[0] = Initial::PAWN_POS_X;
        pawn.pos[1] = Initial::PAWN_POS_Y;
        pawn.pos[2] = Initial::PAWN_POS_Z;
        pawn.heading = Initial::PAWN_HEADING;
        pawn.orientation[1] = 1.0f;
        queue.WriteBuffer(pawnBuffer_, 0, &pawn, sizeof(pawn));
        
        GPUCameraState camera{};
        camera.azimuth = Initial::CAMERA_AZIMUTH;
        camera.elevation = Initial::CAMERA_ELEVATION;
        camera.distance = Initial::CAMERA_DISTANCE;
        queue.WriteBuffer(cameraBuffer_, 0, &camera, sizeof(camera));
        
        GPUDoorHeader header{};
        header.count = 0;
        queue.WriteBuffer(doorHeaderBuffer_, 0, &header, sizeof(header));
        
        return true;
    }
};


} // namespace gallery
} // namespace t7
