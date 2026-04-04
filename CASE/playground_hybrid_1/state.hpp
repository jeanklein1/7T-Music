#pragma once

/**
 * PLAYGROUND HYBRID — GPU State Management
 * ========================================
 *
 * Raymarched terrain + rasterized mesh entity.
 * Foundation for terrain-based worlds.
 *
 * DIMENSIONALITY:
 *   0D — Buffers: signal, camera, entity_state
 *   Geometry — vertex buffer, index buffer (for entity mesh)
 *
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include <webgpu/webgpu_cpp.h>
#include <cstring>
#include <cmath>
#include <array>
#include <vector>
#include <iostream>

namespace t7 {
namespace playground_hybrid {


// ═══════════════════════════════════════════════════════════════════════════════
// §1 INITIAL STATE
// ═══════════════════════════════════════════════════════════════════════════════

namespace Initial {
    constexpr float CAMERA_AZIMUTH = 0.5f;
    constexpr float CAMERA_ELEVATION = 0.6f;
    constexpr float CAMERA_DISTANCE = 25.0f;
    constexpr float CAMERA_PAN_X = 0.0f;
    constexpr float CAMERA_PAN_Y = 0.0f;
    
    // Entity starts at origin
    constexpr float ENTITY_X = 0.0f;
    constexpr float ENTITY_Y = 3.0f;  // Above terrain
    constexpr float ENTITY_Z = 0.0f;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §2 GPU STRUCTURES — Must match world.wgsl exactly
// ═══════════════════════════════════════════════════════════════════════════════

struct alignas(16) GPUFrameSignal {
    float t_seconds;
    float dt;
    float aspect_ratio;
    float _pad0;
    float look_az_delta;
    float look_el_delta;
    float zoom_delta;
    float pan_x_delta;
    float pan_y_delta;
    float _pad1;
    float _pad2;
    float _pad3;
};

struct alignas(16) GPUCameraState {
    float pos[3];
    float azimuth;
    float elevation;
    float distance;
    float pan_x;
    float pan_y;
};

struct alignas(16) GPUEntityState {
    float pos[3];
    float _pad0;
};

struct Vertex {
    float position[3];
    float normal[3];
};

static_assert(sizeof(GPUFrameSignal) == 48, "GPUFrameSignal must be 48 bytes");
static_assert(sizeof(GPUCameraState) == 32, "GPUCameraState must be 32 bytes");
static_assert(sizeof(GPUEntityState) == 16, "GPUEntityState must be 16 bytes");
static_assert(sizeof(Vertex) == 24, "Vertex must be 24 bytes");


// ═══════════════════════════════════════════════════════════════════════════════
// §3 MESH GENERATION
// ═══════════════════════════════════════════════════════════════════════════════

namespace MeshGen {

inline void generate_cone(
    std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices,
    float base_radius,
    float tip_radius,
    float height,
    int segments
) {
    // Base center
    int base_center_idx = static_cast<int>(vertices.size());
    vertices.push_back({{ 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }});
    
    // Base ring
    int base_ring_start = static_cast<int>(vertices.size());
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * 3.14159f * i / segments;
        float x = base_radius * std::cos(angle);
        float z = base_radius * std::sin(angle);
        vertices.push_back({{ x, 0.0f, z }, { 0.0f, -1.0f, 0.0f }});
    }
    
    // Base triangles
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        indices.push_back(base_center_idx);
        indices.push_back(base_ring_start + i);
        indices.push_back(base_ring_start + next);

    }
    
    // Tip
    int tip_center_idx = static_cast<int>(vertices.size());
    if (tip_radius < 0.001f) {
        // Point tip (cone)
        vertices.push_back({{ 0.0f, height, 0.0f }, { 0.0f, 1.0f, 0.0f }});
        
        int side_ring_start = static_cast<int>(vertices.size());
        float slope = base_radius / height;
        float ny = slope / std::sqrt(1.0f + slope * slope);
        float nxz = 1.0f / std::sqrt(1.0f + slope * slope);
        
        for (int i = 0; i < segments; ++i) {
            float angle = 2.0f * 3.14159f * i / segments;
            float x = base_radius * std::cos(angle);
            float z = base_radius * std::sin(angle);
            float nx = nxz * std::cos(angle);
            float nz = nxz * std::sin(angle);
            vertices.push_back({{ x, 0.0f, z }, { nx, ny, nz }});
        }
        
        for (int i = 0; i < segments; ++i) {
            int next = (i + 1) % segments;
            indices.push_back(side_ring_start + i);
            indices.push_back(tip_center_idx);
            indices.push_back(side_ring_start + next);
        }
    } else {
        // Frustum (truncated cone)
        vertices.push_back({{ 0.0f, height, 0.0f }, { 0.0f, 1.0f, 0.0f }});
        
        int top_ring_start = static_cast<int>(vertices.size());
        for (int i = 0; i < segments; ++i) {
            float angle = 2.0f * 3.14159f * i / segments;
            float x = tip_radius * std::cos(angle);
            float z = tip_radius * std::sin(angle);
            vertices.push_back({{ x, height, z }, { 0.0f, 1.0f, 0.0f }});
        }
        
        for (int i = 0; i < segments; ++i) {
            int next = (i + 1) % segments;
            indices.push_back(tip_center_idx);
            indices.push_back(top_ring_start + i);
            indices.push_back(top_ring_start + next);
        }
        
        // Side
        int side_base_start = static_cast<int>(vertices.size());
        int side_top_start = side_base_start + segments;
        
        float slope = (base_radius - tip_radius) / height;
        float ny = slope / std::sqrt(1.0f + slope * slope);
        float nxz = 1.0f / std::sqrt(1.0f + slope * slope);
        
        for (int i = 0; i < segments; ++i) {
            float angle = 2.0f * 3.14159f * i / segments;
            float nx = nxz * std::cos(angle);
            float nz = nxz * std::sin(angle);
            
            float bx = base_radius * std::cos(angle);
            float bz = base_radius * std::sin(angle);
            vertices.push_back({{ bx, 0.0f, bz }, { nx, ny, nz }});
        }
        for (int i = 0; i < segments; ++i) {
            float angle = 2.0f * 3.14159f * i / segments;
            float nx = nxz * std::cos(angle);
            float nz = nxz * std::sin(angle);
            
            float tx = tip_radius * std::cos(angle);
            float tz = tip_radius * std::sin(angle);
            vertices.push_back({{ tx, height, tz }, { nx, ny, nz }});
        }
        
        for (int i = 0; i < segments; ++i) {
            int next = (i + 1) % segments;
            indices.push_back(side_base_start + i);
            indices.push_back(side_base_start + next);
            indices.push_back(side_top_start + i);
            
            indices.push_back(side_top_start + i);
            indices.push_back(side_base_start + next);
            indices.push_back(side_top_start + next);
        }
    }
}

inline void generate_entity_mesh(
    std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices
) {
    // Pawn: cone with base_radius=0.5, tip_radius=0, height=1.5
    generate_cone(vertices, indices, 0.5f, 0.0f, 1.5f, 32);
}

} // namespace MeshGen


// ═══════════════════════════════════════════════════════════════════════════════
// §4 GPU STATE CLASS
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
    
    void upload_signal(wgpu::Queue& queue, const GPUFrameSignal& signal) {
        queue.WriteBuffer(signalBuffer_, 0, &signal, sizeof(GPUFrameSignal));
    }
    
    void upload_entity_state(wgpu::Queue& queue, const GPUEntityState& entity) {
        queue.WriteBuffer(entityBuffer_, 0, &entity, sizeof(GPUEntityState));
    }
    
    // Mesh
    wgpu::Buffer vertex_buffer() const { return vertexBuffer_; }
    wgpu::Buffer index_buffer() const { return indexBuffer_; }
    uint32_t index_count() const { return indexCount_; }
    
    // Bind groups
    wgpu::BindGroupLayout compute_bind_group_layout() const { return computeBindGroupLayout_; }
    wgpu::BindGroupLayout render_bind_group_layout() const { return renderBindGroupLayout_; }
    wgpu::BindGroup compute_bind_group() const { return computeBindGroup_; }
    wgpu::BindGroup render_bind_group() const { return renderBindGroup_; }
    
private:
    wgpu::Device device_;
    
    // Buffers
    wgpu::Buffer signalBuffer_;
    wgpu::Buffer cameraBuffer_;
    wgpu::Buffer entityBuffer_;
    wgpu::Buffer vertexBuffer_;
    wgpu::Buffer indexBuffer_;
    uint32_t indexCount_ = 0;
    
    // Bind groups
    wgpu::BindGroupLayout computeBindGroupLayout_;
    wgpu::BindGroupLayout renderBindGroupLayout_;
    wgpu::BindGroup computeBindGroup_;
    wgpu::BindGroup renderBindGroup_;
    
    bool createBuffers() {
        wgpu::BufferDescriptor desc{};
        
        // Signal
        desc.label = "Playground Hybrid Signal";
        desc.size = sizeof(GPUFrameSignal);
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        signalBuffer_ = device_.CreateBuffer(&desc);
        if (!signalBuffer_) return false;
        
        // Camera
        desc.label = "Playground Hybrid Camera";
        desc.size = sizeof(GPUCameraState);
        cameraBuffer_ = device_.CreateBuffer(&desc);
        if (!cameraBuffer_) return false;
        
        // Entity
        desc.label = "Playground Hybrid Entity";
        desc.size = sizeof(GPUEntityState);
        entityBuffer_ = device_.CreateBuffer(&desc);
        if (!entityBuffer_) return false;
        
        // Generate mesh
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        MeshGen::generate_entity_mesh(vertices, indices);
        indexCount_ = static_cast<uint32_t>(indices.size());
        
        std::cout << "[Playground Hybrid] Generated entity mesh: " 
                  << vertices.size() << " vertices, " 
                  << indices.size() << " indices\n";
        
        // Vertex buffer
        desc.label = "Playground Hybrid Vertex Buffer";
        desc.size = vertices.size() * sizeof(Vertex);
        desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
        vertexBuffer_ = device_.CreateBuffer(&desc);
        if (!vertexBuffer_) return false;
        
        wgpu::Queue queue = device_.GetQueue();
        queue.WriteBuffer(vertexBuffer_, 0, vertices.data(), vertices.size() * sizeof(Vertex));
        
        // Index buffer
        desc.label = "Playground Hybrid Index Buffer";
        desc.size = indices.size() * sizeof(uint32_t);
        desc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
        indexBuffer_ = device_.CreateBuffer(&desc);
        if (!indexBuffer_) return false;
        
        queue.WriteBuffer(indexBuffer_, 0, indices.data(), indices.size() * sizeof(uint32_t));
        
        return true;
    }
    
    bool createBindGroups() {
        // ─── Compute Layout: bindings 0, 1 ──────────────────────────────────
        {
            std::array<wgpu::BindGroupLayoutEntry, 2> entries{};
            
            entries[0].binding = 0;
            entries[0].visibility = wgpu::ShaderStage::Compute;
            entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[1].binding = 1;
            entries[1].visibility = wgpu::ShaderStage::Compute;
            entries[1].buffer.type = wgpu::BufferBindingType::Storage;
            
            wgpu::BindGroupLayoutDescriptor desc{};
            desc.label = "Playground Hybrid Compute Layout";
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            computeBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
            if (!computeBindGroupLayout_) return false;
        }
        
        // ─── Render Layout: bindings 10, 11, 12 ─────────────────────────────
        // Both terrain and entity passes share this layout
        {
            std::array<wgpu::BindGroupLayoutEntry, 3> entries{};
            
            // Signal (terrain + entity need aspect ratio, time)
            entries[0].binding = 10;
            entries[0].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
            entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            // Camera (terrain + entity need camera)
            entries[1].binding = 11;
            entries[1].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
            entries[1].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            // Entity state (only entity vertex shader needs it, but must be in layout)
            entries[2].binding = 12;
            entries[2].visibility = wgpu::ShaderStage::Vertex;
            entries[2].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            wgpu::BindGroupLayoutDescriptor desc{};
            desc.label = "Playground Hybrid Render Layout";
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            renderBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
            if (!renderBindGroupLayout_) return false;
        }
        
        // ─── Compute Bind Group ─────────────────────────────────────────────
        {
            std::array<wgpu::BindGroupEntry, 2> entries{};
            
            entries[0].binding = 0;
            entries[0].buffer = signalBuffer_;
            entries[0].size = sizeof(GPUFrameSignal);
            
            entries[1].binding = 1;
            entries[1].buffer = cameraBuffer_;
            entries[1].size = sizeof(GPUCameraState);
            
            wgpu::BindGroupDescriptor desc{};
            desc.label = "Playground Hybrid Compute BindGroup";
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
            entries[2].buffer = entityBuffer_;
            entries[2].size = sizeof(GPUEntityState);
            
            wgpu::BindGroupDescriptor desc{};
            desc.label = "Playground Hybrid Render BindGroup";
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
        
        // Camera
        GPUCameraState camera{};
        camera.pos[0] = 0.0f;
        camera.pos[1] = 0.0f;
        camera.pos[2] = Initial::CAMERA_DISTANCE;
        camera.azimuth = Initial::CAMERA_AZIMUTH;
        camera.elevation = Initial::CAMERA_ELEVATION;
        camera.distance = Initial::CAMERA_DISTANCE;
        camera.pan_x = Initial::CAMERA_PAN_X;
        camera.pan_y = Initial::CAMERA_PAN_Y;
        queue.WriteBuffer(cameraBuffer_, 0, &camera, sizeof(camera));
        
        // Entity
        GPUEntityState entity{};
        entity.pos[0] = Initial::ENTITY_X;
        entity.pos[1] = Initial::ENTITY_Y;
        entity.pos[2] = Initial::ENTITY_Z;
        entity._pad0 = 0.0f;
        queue.WriteBuffer(entityBuffer_, 0, &entity, sizeof(entity));
        
        return true;
    }
};


} // namespace playground_hybrid
} // namespace t7
