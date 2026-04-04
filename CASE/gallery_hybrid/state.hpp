#pragma once

/**
 * GALLERY HYBRID — GPU State Management
 * ======================================
 *
 * Rasterized room with doors. Mesh generated on CPU, rendered on GPU.
 *
 * BUFFERS:
 *   0D: signal, camera, pawn
 *   Geometry: room vertices/indices, pawn vertices/indices
 *
 * See mesh.hpp for geometry generation.
 * See world.wgsl for the GPU scroll.
 */

#include <webgpu/webgpu_cpp.h>
#include <cstring>
#include <cmath>
#include <array>
#include <vector>
#include <iostream>

#include "mesh.hpp"

namespace t7 {
namespace gallery {


// ═══════════════════════════════════════════════════════════════════════════════
// §1 INITIAL STATE
// ═══════════════════════════════════════════════════════════════════════════════

namespace Initial {
    // Pawn
    constexpr float PAWN_POS_X = 0.0f;
    constexpr float PAWN_POS_Y = 0.0f;
    constexpr float PAWN_POS_Z = 5.0f;
    constexpr float PAWN_HEADING = 3.14159f;  // Facing -Z
    
    // Camera
    constexpr float CAMERA_POS_X = 0.0f;
    constexpr float CAMERA_POS_Y = 3.0f;
    constexpr float CAMERA_POS_Z = 12.0f;
    constexpr float CAMERA_AZIMUTH = 0.0f;
    constexpr float CAMERA_ELEVATION = 0.4f;
    constexpr float CAMERA_DISTANCE = 10.0f;
    constexpr float CAMERA_PAN_X = 0.0f;
    constexpr float CAMERA_PAN_Y = 0.0f;
    
    // Config
    constexpr float PAWN_SPEED = 8.0f;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §2 DOOR TARGET IDs
// ═══════════════════════════════════════════════════════════════════════════════

namespace DoorTarget {
    constexpr uint32_t NONE = 0xFFFFFFFF;
    constexpr uint32_t GALLERY = 0;
    constexpr uint32_t N_DIMENSIONAL_2 = 1;
    constexpr uint32_t PLAYGROUND_RAYMARCH = 2;
    constexpr uint32_t PLAYGROUND_RASTERIZE = 3;
    constexpr uint32_t PLAYGROUND_HYBRID = 4;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §3 GPU STRUCTURES — Must match world.wgsl exactly
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

struct alignas(16) GPUCameraState {
    float pos[3];
    float azimuth;
    float elevation;
    float distance;
    float pan_x;
    float pan_y;
};

struct alignas(16) GPUPawnState {
    float pos[3];
    float heading;
    float orientation[4];
};

static_assert(sizeof(GPUFrameSignal) == 48, "GPUFrameSignal must be 48 bytes");
static_assert(sizeof(GPUCameraState) == 32, "GPUCameraState must be 32 bytes");
static_assert(sizeof(GPUPawnState) == 32, "GPUPawnState must be 32 bytes");


// ═══════════════════════════════════════════════════════════════════════════════
// §4 DOOR CONFIG (for collision/transition, not rendering)
// ═══════════════════════════════════════════════════════════════════════════════

struct DoorConfig {
    float position[3];    // Bottom center
    float normal[3];      // Direction through door (out of room)
    float width;
    float height;
    float commitment_depth;
    uint32_t target_id;
};


// ═══════════════════════════════════════════════════════════════════════════════
// §5 GPU STATE CLASS
// ═══════════════════════════════════════════════════════════════════════════════

class GPUState {
public:
    GPUState() = default;
    
    bool init(wgpu::Device device, const std::vector<DoorConfig>& doors) {
        device_ = device;
        doors_ = doors;
        
        if (!createBuffers()) return false;
        if (!createMeshes()) return false;
        if (!createBindGroups()) return false;
        if (!initializeState()) return false;
        
        return true;
    }
    
    // ─── Signal Upload ──────────────────────────────────────────────────────
    
    void upload_signal(wgpu::Queue& queue, const GPUFrameSignal& signal) {
        queue.WriteBuffer(signalBuffer_, 0, &signal, sizeof(GPUFrameSignal));
    }
    
    // ─── Mesh Accessors ─────────────────────────────────────────────────────
    
    wgpu::Buffer room_vertex_buffer() const { return roomVertexBuffer_; }
    wgpu::Buffer room_index_buffer() const { return roomIndexBuffer_; }
    uint32_t room_index_count() const { return roomIndexCount_; }
    
    wgpu::Buffer pawn_vertex_buffer() const { return pawnVertexBuffer_; }
    wgpu::Buffer pawn_index_buffer() const { return pawnIndexBuffer_; }
    uint32_t pawn_index_count() const { return pawnIndexCount_; }
    
    // ─── Bind Group Accessors ───────────────────────────────────────────────
    
    wgpu::BindGroupLayout compute_bind_group_layout() const { return computeBindGroupLayout_; }
    wgpu::BindGroupLayout render_bind_group_layout() const { return renderBindGroupLayout_; }
    wgpu::BindGroup compute_bind_group() const { return computeBindGroup_; }
    wgpu::BindGroup render_bind_group() const { return renderBindGroup_; }
    
    // ─── Reset ──────────────────────────────────────────────────────────────
    
    void reset(wgpu::Queue& queue) {
        GPUPawnState pawn{};
        pawn.pos[0] = Initial::PAWN_POS_X;
        pawn.pos[1] = Initial::PAWN_POS_Y;
        pawn.pos[2] = Initial::PAWN_POS_Z;
        pawn.heading = Initial::PAWN_HEADING;
        // Orientation for facing -Z (180° around Y)
        pawn.orientation[0] = 0.0f;
        pawn.orientation[1] = 1.0f;
        pawn.orientation[2] = 0.0f;
        pawn.orientation[3] = 0.0f;
        queue.WriteBuffer(pawnBuffer_, 0, &pawn, sizeof(pawn));
        
        GPUCameraState camera{};
        camera.pos[0] = Initial::CAMERA_POS_X;
        camera.pos[1] = Initial::CAMERA_POS_Y;
        camera.pos[2] = Initial::CAMERA_POS_Z;
        camera.azimuth = Initial::CAMERA_AZIMUTH;
        camera.elevation = Initial::CAMERA_ELEVATION;
        camera.distance = Initial::CAMERA_DISTANCE;
        camera.pan_x = Initial::CAMERA_PAN_X;
        camera.pan_y = Initial::CAMERA_PAN_Y;
        queue.WriteBuffer(cameraBuffer_, 0, &camera, sizeof(camera));
    }
    
    // ─── Door Access (for collision) ────────────────────────────────────────
    
    const std::vector<DoorConfig>& doors() const { return doors_; }
    
private:
    wgpu::Device device_;
    std::vector<DoorConfig> doors_;
    
    // Buffers
    wgpu::Buffer signalBuffer_;
    wgpu::Buffer cameraBuffer_;
    wgpu::Buffer pawnBuffer_;
    
    // Room mesh
    wgpu::Buffer roomVertexBuffer_;
    wgpu::Buffer roomIndexBuffer_;
    uint32_t roomIndexCount_ = 0;
    
    // Pawn mesh
    wgpu::Buffer pawnVertexBuffer_;
    wgpu::Buffer pawnIndexBuffer_;
    uint32_t pawnIndexCount_ = 0;
    
    // Bind groups
    wgpu::BindGroupLayout computeBindGroupLayout_;
    wgpu::BindGroupLayout renderBindGroupLayout_;
    wgpu::BindGroup computeBindGroup_;
    wgpu::BindGroup renderBindGroup_;
    
    
    bool createBuffers() {
        wgpu::BufferDescriptor desc{};
        
        // Signal buffer
        desc.label = "Gallery Signal";
        desc.size = sizeof(GPUFrameSignal);
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        signalBuffer_ = device_.CreateBuffer(&desc);
        if (!signalBuffer_) return false;
        
        // Camera buffer
        desc.label = "Gallery Camera";
        desc.size = sizeof(GPUCameraState);
        cameraBuffer_ = device_.CreateBuffer(&desc);
        if (!cameraBuffer_) return false;
        
        // Pawn state buffer
        desc.label = "Gallery Pawn State";
        desc.size = sizeof(GPUPawnState);
        pawnBuffer_ = device_.CreateBuffer(&desc);
        if (!pawnBuffer_) return false;
        
        return true;
    }
    
    bool createMeshes() {
        wgpu::Queue queue = device_.GetQueue();
        
        // ─── Room mesh ──────────────────────────────────────────────────────
        {
            MeshBuilder mb;
            
            // Build room config from doors
            RoomConfig config;
            DoorInfo backDoor, frontDoor, leftDoor, rightDoor;
            
            for (const auto& door : doors_) {
                DoorInfo info;
                
                if (std::abs(door.normal[2]) > 0.5f) {
                    // Back or front wall
                    info.center_x = door.position[0];
                    info.width = door.width;
                    info.height = door.height;
                    
                    if (door.normal[2] < 0) {
                        backDoor = info;
                        config.back_door = &backDoor;
                    } else {
                        frontDoor = info;
                        config.front_door = &frontDoor;
                    }
                } else {
                    // Left or right wall
                    info.center_x = door.position[2];
                    info.width = door.width;
                    info.height = door.height;
                    
                    if (door.normal[0] < 0) {
                        leftDoor = info;
                        config.left_door = &leftDoor;
                    } else {
                        rightDoor = info;
                        config.right_door = &rightDoor;
                    }
                }
            }
            
            generate_room(mb, config);
            roomIndexCount_ = static_cast<uint32_t>(mb.indices.size());
            
            std::cout << "[Gallery Hybrid] Room mesh: " 
                      << mb.vertices.size() << " vertices, " 
                      << mb.indices.size() << " indices\n";
            
            // Create buffers
            wgpu::BufferDescriptor desc{};
            desc.label = "Gallery Room Vertices";
            desc.size = mb.vertices.size() * sizeof(Vertex);
            desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
            roomVertexBuffer_ = device_.CreateBuffer(&desc);
            queue.WriteBuffer(roomVertexBuffer_, 0, mb.vertices.data(), mb.vertices.size() * sizeof(Vertex));
            
            desc.label = "Gallery Room Indices";
            desc.size = mb.indices.size() * sizeof(uint32_t);
            desc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
            roomIndexBuffer_ = device_.CreateBuffer(&desc);
            queue.WriteBuffer(roomIndexBuffer_, 0, mb.indices.data(), mb.indices.size() * sizeof(uint32_t));
        }
        
        // ─── Pawn mesh ──────────────────────────────────────────────────────
        {
            MeshBuilder mb;
            generate_pawn(mb);
            pawnIndexCount_ = static_cast<uint32_t>(mb.indices.size());
            
            std::cout << "[Gallery Hybrid] Pawn mesh: " 
                      << mb.vertices.size() << " vertices, " 
                      << mb.indices.size() << " indices\n";
            
            wgpu::BufferDescriptor desc{};
            desc.label = "Gallery Pawn Vertices";
            desc.size = mb.vertices.size() * sizeof(Vertex);
            desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
            pawnVertexBuffer_ = device_.CreateBuffer(&desc);
            queue.WriteBuffer(pawnVertexBuffer_, 0, mb.vertices.data(), mb.vertices.size() * sizeof(Vertex));
            
            desc.label = "Gallery Pawn Indices";
            desc.size = mb.indices.size() * sizeof(uint32_t);
            desc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
            pawnIndexBuffer_ = device_.CreateBuffer(&desc);
            queue.WriteBuffer(pawnIndexBuffer_, 0, mb.indices.data(), mb.indices.size() * sizeof(uint32_t));
        }
        
        return true;
    }
    
    bool createBindGroups() {
        // ─── Compute Bind Group Layout ──────────────────────────────────────
        {
            std::array<wgpu::BindGroupLayoutEntry, 3> entries{};
            
            entries[0].binding = 0;
            entries[0].visibility = wgpu::ShaderStage::Compute;
            entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[1].binding = 1;
            entries[1].visibility = wgpu::ShaderStage::Compute;
            entries[1].buffer.type = wgpu::BufferBindingType::Storage;
            
            entries[2].binding = 2;
            entries[2].visibility = wgpu::ShaderStage::Compute;
            entries[2].buffer.type = wgpu::BufferBindingType::Storage;
            
            wgpu::BindGroupLayoutDescriptor desc{};
            desc.label = "Gallery Compute BindGroup Layout";
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            computeBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
            if (!computeBindGroupLayout_) return false;
        }
        
        // ─── Render Bind Group Layout ───────────────────────────────────────
        {
            std::array<wgpu::BindGroupLayoutEntry, 3> entries{};
            
            entries[0].binding = 10;
            entries[0].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
            entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[1].binding = 11;
            entries[1].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
            entries[1].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[2].binding = 12;
            entries[2].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
            entries[2].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            wgpu::BindGroupLayoutDescriptor desc{};
            desc.label = "Gallery Render BindGroup Layout";
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            renderBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
            if (!renderBindGroupLayout_) return false;
        }
        
        // ─── Compute Bind Group ─────────────────────────────────────────────
        {
            std::array<wgpu::BindGroupEntry, 3> entries{};
            
            entries[0].binding = 0;
            entries[0].buffer = signalBuffer_;
            entries[0].size = sizeof(GPUFrameSignal);
            
            entries[1].binding = 1;
            entries[1].buffer = cameraBuffer_;
            entries[1].size = sizeof(GPUCameraState);
            
            entries[2].binding = 2;
            entries[2].buffer = pawnBuffer_;
            entries[2].size = sizeof(GPUPawnState);
            
            wgpu::BindGroupDescriptor desc{};
            desc.label = "Gallery Compute BindGroup";
            desc.layout = computeBindGroupLayout_;
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            computeBindGroup_ = device_.CreateBindGroup(&desc);
            if (!computeBindGroup_) return false;
        }
        
        // ─── Render Bind Group ──────────────────────────────────────────────
        {
            std::array<wgpu::BindGroupEntry, 3> entries{};
            
            entries[0].binding = 10;
            entries[0].buffer = signalBuffer_;
            entries[0].size = sizeof(GPUFrameSignal);
            
            entries[1].binding = 11;
            entries[1].buffer = cameraBuffer_;
            entries[1].size = sizeof(GPUCameraState);
            
            entries[2].binding = 12;
            entries[2].buffer = pawnBuffer_;
            entries[2].size = sizeof(GPUPawnState);
            
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
        reset(queue);
        return true;
    }
};


} // namespace gallery
} // namespace t7
