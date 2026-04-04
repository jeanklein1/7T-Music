#pragma once

/**
 * PLAYGROUND RASTERIZE — GPU State Management
 * ============================================
 *
 * Minimal state for mesh viewing. One static mesh, orbital camera.
 *
 * DIFFERENCE FROM RAYMARCH:
 *   - Has vertex buffer and index buffer
 *   - Mesh generated procedurally at init time
 *   - No SDF, shapes are triangles
 *
 * DIMENSIONALITY:
 *   0D — Buffers: signal, camera
 *   Geometry — vertex buffer, index buffer
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
    namespace playground_rasterize {


        // ═══════════════════════════════════════════════════════════════════════════════
        // §1 INITIAL STATE
        // ═══════════════════════════════════════════════════════════════════════════════

        namespace Initial {
            constexpr float CAMERA_AZIMUTH = 0.5f;
            constexpr float CAMERA_ELEVATION = 0.3f;
            constexpr float CAMERA_DISTANCE = 5.0f;
            constexpr float CAMERA_PAN_X = 0.0f;
            constexpr float CAMERA_PAN_Y = 0.0f;
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

        // ─── Vertex format ──────────────────────────────────────────────────────────

        struct Vertex {
            float position[3];
            float normal[3];
        };

        static_assert(sizeof(GPUFrameSignal) == 48, "GPUFrameSignal must be 48 bytes");
        static_assert(sizeof(GPUCameraState) == 32, "GPUCameraState must be 32 bytes");
        static_assert(sizeof(Vertex) == 24, "Vertex must be 24 bytes");


        // ═══════════════════════════════════════════════════════════════════════════════
        // §3 MESH GENERATION
        // ═══════════════════════════════════════════════════════════════════════════════
        //
        // ┌─────────────────────────────────────────────────────────────────────────────┐
        // │ THE MESH FACTORY                                                            │
        // │                                                                             │
        // │ Modify generate_subject_mesh() to create different shapes.                  │
        // │ Or replace with mesh loading from file.                                     │
        // └─────────────────────────────────────────────────────────────────────────────┘

        namespace MeshGen {

            // ─── Cone mesh (approximates the pawn from raymarch) ────────────────────────

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
                vertices.push_back({ { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } });

                // Base ring
                int base_ring_start = static_cast<int>(vertices.size());
                for (int i = 0; i < segments; ++i) {
                    float angle = 2.0f * 3.14159f * i / segments;
                    float x = base_radius * std::cos(angle);
                    float z = base_radius * std::sin(angle);
                    vertices.push_back({ { x, 0.0f, z }, { 0.0f, -1.0f, 0.0f } });
                }

                // Base triangles
                for (int i = 0; i < segments; ++i) {
                    int next = (i + 1) % segments;
                    indices.push_back(base_center_idx);
                    indices.push_back(base_ring_start + i);
                    indices.push_back(base_ring_start + next);  // Reverse winding for bottom

                }

                // Tip center (or tip ring if tip_radius > 0)
                int tip_center_idx = static_cast<int>(vertices.size());
                if (tip_radius < 0.001f) {
                    // Point tip
                    vertices.push_back({ { 0.0f, height, 0.0f }, { 0.0f, 1.0f, 0.0f } });

                    // Side triangles (cone)
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
                        vertices.push_back({ { x, 0.0f, z }, { nx, ny, nz } });
                    }

                    for (int i = 0; i < segments; ++i) {
                        int next = (i + 1) % segments;
                        indices.push_back(side_ring_start + i);
                        indices.push_back(tip_center_idx);
                        indices.push_back(side_ring_start + next);
                    }
                }
                else {
                    // Frustum (truncated cone)
                    // Top cap
                    vertices.push_back({ { 0.0f, height, 0.0f }, { 0.0f, 1.0f, 0.0f } });

                    int top_ring_start = static_cast<int>(vertices.size());
                    for (int i = 0; i < segments; ++i) {
                        float angle = 2.0f * 3.14159f * i / segments;
                        float x = tip_radius * std::cos(angle);
                        float z = tip_radius * std::sin(angle);
                        vertices.push_back({ { x, height, z }, { 0.0f, 1.0f, 0.0f } });
                    }

                    for (int i = 0; i < segments; ++i) {
                        int next = (i + 1) % segments;
                        indices.push_back(tip_center_idx);
                        indices.push_back(top_ring_start + i);
                        indices.push_back(top_ring_start + next);
                    }

                    // Side triangles (frustum)
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
                        vertices.push_back({ { bx, 0.0f, bz }, { nx, ny, nz } });
                    }
                    for (int i = 0; i < segments; ++i) {
                        float angle = 2.0f * 3.14159f * i / segments;
                        float nx = nxz * std::cos(angle);
                        float nz = nxz * std::sin(angle);

                        float tx = tip_radius * std::cos(angle);
                        float tz = tip_radius * std::sin(angle);
                        vertices.push_back({ { tx, height, tz }, { nx, ny, nz } });
                    }

                    for (int i = 0; i < segments; ++i) {
                        int next = (i + 1) % segments;
                        // Two triangles per quad
                        indices.push_back(side_base_start + i);
                        indices.push_back(side_base_start + next);
                        indices.push_back(side_top_start + i);

                        indices.push_back(side_top_start + i);
                        indices.push_back(side_base_start + next);
                        indices.push_back(side_top_start + next);
                    }
                }
            }

            // ─── Subject mesh (same shape as raymarch pawn) ─────────────────────────────

            inline void generate_subject_mesh(
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

            // ─── Signal Upload ──────────────────────────────────────────────────────

            void upload_signal(wgpu::Queue& queue, const GPUFrameSignal& signal) {
                queue.WriteBuffer(signalBuffer_, 0, &signal, sizeof(GPUFrameSignal));
            }

            // ─── Mesh Info ──────────────────────────────────────────────────────────

            wgpu::Buffer vertex_buffer() const { return vertexBuffer_; }
            wgpu::Buffer index_buffer() const { return indexBuffer_; }
            uint32_t index_count() const { return indexCount_; }

            // ─── Bind Group Accessors ───────────────────────────────────────────────

            wgpu::BindGroupLayout compute_bind_group_layout() const { return computeBindGroupLayout_; }
            wgpu::BindGroupLayout render_bind_group_layout() const { return renderBindGroupLayout_; }
            wgpu::BindGroup compute_bind_group() const { return computeBindGroup_; }
            wgpu::BindGroup render_bind_group() const { return renderBindGroup_; }

        private:
            wgpu::Device device_;

            // ─── Buffers ────────────────────────────────────────────────────────────
            wgpu::Buffer signalBuffer_;
            wgpu::Buffer cameraBuffer_;
            wgpu::Buffer vertexBuffer_;
            wgpu::Buffer indexBuffer_;
            uint32_t indexCount_ = 0;

            // ─── Bind Groups ────────────────────────────────────────────────────────
            wgpu::BindGroupLayout computeBindGroupLayout_;
            wgpu::BindGroupLayout renderBindGroupLayout_;
            wgpu::BindGroup computeBindGroup_;
            wgpu::BindGroup renderBindGroup_;

            // ─────────────────────────────────────────────────────────────────────────
            // CREATION METHODS
            // ─────────────────────────────────────────────────────────────────────────

            bool createBuffers() {
                wgpu::BufferDescriptor desc{};

                // Signal buffer
                desc.label = "Playground Rasterize Signal";
                desc.size = sizeof(GPUFrameSignal);
                desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
                signalBuffer_ = device_.CreateBuffer(&desc);
                if (!signalBuffer_) return false;

                // Camera state buffer
                desc.label = "Playground Rasterize Camera State";
                desc.size = sizeof(GPUCameraState);
                desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
                cameraBuffer_ = device_.CreateBuffer(&desc);
                if (!cameraBuffer_) return false;

                // Generate mesh
                std::vector<Vertex> vertices;
                std::vector<uint32_t> indices;
                MeshGen::generate_subject_mesh(vertices, indices);
                indexCount_ = static_cast<uint32_t>(indices.size());

                std::cout << "[Playground Rasterize] Generated mesh: "
                    << vertices.size() << " vertices, "
                    << indices.size() << " indices\n";

                // Vertex buffer
                desc.label = "Playground Rasterize Vertex Buffer";
                desc.size = vertices.size() * sizeof(Vertex);
                desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
                vertexBuffer_ = device_.CreateBuffer(&desc);
                if (!vertexBuffer_) return false;

                wgpu::Queue queue = device_.GetQueue();
                queue.WriteBuffer(vertexBuffer_, 0, vertices.data(), vertices.size() * sizeof(Vertex));

                // Index buffer
                desc.label = "Playground Rasterize Index Buffer";
                desc.size = indices.size() * sizeof(uint32_t);
                desc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
                indexBuffer_ = device_.CreateBuffer(&desc);
                if (!indexBuffer_) return false;

                queue.WriteBuffer(indexBuffer_, 0, indices.data(), indices.size() * sizeof(uint32_t));

                return true;
            }

            bool createBindGroups() {
                // ─── Compute Bind Group Layout ──────────────────────────────────────
                {
                    std::array<wgpu::BindGroupLayoutEntry, 2> entries{};

                    entries[0].binding = 0;
                    entries[0].visibility = wgpu::ShaderStage::Compute;
                    entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

                    entries[1].binding = 1;
                    entries[1].visibility = wgpu::ShaderStage::Compute;
                    entries[1].buffer.type = wgpu::BufferBindingType::Storage;

                    wgpu::BindGroupLayoutDescriptor desc{};
                    desc.label = "Playground Rasterize Compute BindGroup Layout";
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    computeBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
                    if (!computeBindGroupLayout_) return false;
                }

                // ─── Render Bind Group Layout ───────────────────────────────────────
                {
                    std::array<wgpu::BindGroupLayoutEntry, 2> entries{};

                    entries[0].binding = 10;
                    entries[0].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
                    entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

                    entries[1].binding = 11;
                    entries[1].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
                    entries[1].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

                    wgpu::BindGroupLayoutDescriptor desc{};
                    desc.label = "Playground Rasterize Render BindGroup Layout";
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
                    desc.label = "Playground Rasterize Compute BindGroup";
                    desc.layout = computeBindGroupLayout_;
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    computeBindGroup_ = device_.CreateBindGroup(&desc);
                    if (!computeBindGroup_) return false;
                }

                // ─── Render Bind Group ──────────────────────────────────────────────
                {
                    std::array<wgpu::BindGroupEntry, 2> entries{};

                    entries[0].binding = 10;
                    entries[0].buffer = signalBuffer_;
                    entries[0].size = sizeof(GPUFrameSignal);

                    entries[1].binding = 11;
                    entries[1].buffer = cameraBuffer_;
                    entries[1].size = sizeof(GPUCameraState);

                    wgpu::BindGroupDescriptor desc{};
                    desc.label = "Playground Rasterize Render BindGroup";
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

                return true;
            }
        };


    } // namespace playground_rasterize
} // namespace t7