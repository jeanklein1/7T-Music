#pragma once

/**
 * PAWN RASTERIZE — GPU State Management
 * ======================================
 *
 * Rasterized pawn viewer. Mesh generated from profile math at init.
 *
 * THE PROFILE MATH LIVES HERE (C++ is source of truth).
 * WGSL just renders the mesh — no SDF, no profile logic.
 *
 * See world.wgsl for the GPU scroll.
 */

#include <webgpu/webgpu_cpp.h>
#include <cstring>
#include <cmath>
#include <array>
#include <vector>
#include <iostream>

namespace t7 {
    namespace pawn_rasterize {


        // ═══════════════════════════════════════════════════════════════════════════════
        // §1 INITIAL STATE
        // ═══════════════════════════════════════════════════════════════════════════════

        namespace Initial {
            constexpr float CAMERA_AZIMUTH = 0.5f;
            constexpr float CAMERA_ELEVATION = 0.3f;
            constexpr float CAMERA_DISTANCE = 4.0f;
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

        struct Vertex {
            float position[3];
            float normal[3];
        };

        static_assert(sizeof(GPUFrameSignal) == 48, "GPUFrameSignal must be 48 bytes");
        static_assert(sizeof(GPUCameraState) == 32, "GPUCameraState must be 32 bytes");
        static_assert(sizeof(Vertex) == 24, "Vertex must be 24 bytes");


        // ═══════════════════════════════════════════════════════════════════════════════
        // §3 PAWN PROFILE — The math (C++ is source of truth)
        // ═══════════════════════════════════════════════════════════════════════════════
        //
        // ┌─────────────────────────────────────────────────────────────────────────────┐
        // │ This is THE definition of the pawn shape.                                   │
        // │ Edit here → regenerate mesh → shape changes everywhere.                     │
        // └─────────────────────────────────────────────────────────────────────────────┘

        namespace PawnProfile {

            struct Params {
                float base_radius = 0.58f;
                float base_rim_radius = 0.47f;
                float base_height = 0.31f;
                float body_bottom_radius = 0.36f;
                float body_taper = 0.49f;
                float body_height = 0.80f;
                float collar_bulge = 0.30f;
                float collar_height = 0.10f;
                float neck_radius = 0.26f;
                float head_radius = 0.27f;
                float head_height = 0.57f;
            };

            inline float smoothstep(float t) {
                return t * t * (3.0f - 2.0f * t);
            }

            inline float lerp(float a, float b, float t) {
                return a + (b - a) * t;
            }

            // ─── The profile function ─────────────────────────────────────────────────────
            //
            // Returns radius at given height. This defines the lathe silhouette.

            inline float evaluate(float y, const Params& p) {
                // Key heights
                const float base_rim_y = p.base_height * 0.5f;
                const float base_top_y = p.base_height;
                const float body_top_y = base_top_y + p.body_height;
                const float collar_mid_y = body_top_y + p.collar_height * 0.5f;
                const float collar_top_y = body_top_y + p.collar_height;
                const float neck_top_y = collar_top_y + p.collar_height * 0.5f;
                const float head_center_y = neck_top_y + p.head_height - p.head_radius;

                const float body_top_radius = p.body_bottom_radius * (1.0f - p.body_taper);

                if (y < 0.0f) return 0.0f;

                // Base foot to rim
                if (y < base_rim_y) {
                    float t = y / base_rim_y;
                    return lerp(p.base_radius, p.base_rim_radius, smoothstep(t));
                }

                // Base rim to top
                if (y < base_top_y) {
                    float t = (y - base_rim_y) / (base_top_y - base_rim_y);
                    return lerp(p.base_rim_radius, p.body_bottom_radius, smoothstep(t));
                }

                // Body taper
                if (y < body_top_y) {
                    float t = (y - base_top_y) / p.body_height;
                    return lerp(p.body_bottom_radius, body_top_radius, smoothstep(t));
                }

                // Collar approach
                if (y < collar_mid_y) {
                    float t = (y - body_top_y) / (collar_mid_y - body_top_y);
                    return lerp(body_top_radius, p.collar_bulge, smoothstep(t));
                }

                // Collar peak to top
                if (y < collar_top_y) {
                    float t = (y - collar_mid_y) / (collar_top_y - collar_mid_y);
                    return lerp(p.collar_bulge, p.neck_radius * 1.1f, smoothstep(t));
                }

                // Neck
                if (y < neck_top_y) {
                    float t = (y - collar_top_y) / (neck_top_y - collar_top_y);
                    return lerp(p.neck_radius * 1.1f, p.neck_radius, smoothstep(t));
                }

                // Blend to head sphere
                if (y < head_center_y) {
                    float t = (y - neck_top_y) / (head_center_y - neck_top_y);
                    return lerp(p.neck_radius, 0.0f, t);
                }

                return 0.0f;
            }

            inline float head_center_y(const Params& p) {
                const float base_top_y = p.base_height;
                const float body_top_y = base_top_y + p.body_height;
                const float collar_top_y = body_top_y + p.collar_height;
                const float neck_top_y = collar_top_y + p.collar_height * 0.5f;
                return neck_top_y + p.head_height - p.head_radius;
            }

            inline float total_height(const Params& p) {
                return head_center_y(p) + p.head_radius;
            }

        } // namespace PawnProfile


        // ═══════════════════════════════════════════════════════════════════════════════
        // §4 MESH GENERATION
        // ═══════════════════════════════════════════════════════════════════════════════

        namespace MeshGen {

            constexpr float PI = 3.14159265359f;
            constexpr float TWO_PI = 2.0f * PI;

            struct Config {
                int radial_segments = 32;
                int height_segments = 64;
                int sphere_rings = 16;
            };

            inline void generate_pawn(
                std::vector<Vertex>& vertices,
                std::vector<uint32_t>& indices,
                const PawnProfile::Params& params = PawnProfile::Params{},
                const Config& config = Config{}
            ) {
                using namespace PawnProfile;

                const float hcy = head_center_y(params);
                const float lathe_top_y = hcy - params.head_radius * 0.3f;
                const int n_rings = config.height_segments;
                const int n_segs = config.radial_segments;

                // ─── LATHE BODY ───────────────────────────────────────────────────────────

                for (int ring = 0; ring <= n_rings; ++ring) {
                    float t = static_cast<float>(ring) / n_rings;
                    float y = t * lathe_top_y;
                    float r = evaluate(y, params);

                    if (r < 0.001f && ring > 0) continue;

                    for (int seg = 0; seg < n_segs; ++seg) {
                        float angle = TWO_PI * seg / n_segs;
                        float x = r * std::cos(angle);
                        float z = r * std::sin(angle);

                        // Normal from profile slope
                        float eps = 0.001f;
                        float dr = evaluate(y + eps, params) - evaluate(y - eps, params);
                        float dy = 2.0f * eps;

                        float nx = std::cos(angle);
                        float nz = std::sin(angle);
                        float ny = -dr / dy;
                        float len = std::sqrt(nx * nx + ny * ny + nz * nz);
                        if (len > 0.0001f) { nx /= len; ny /= len; nz /= len; }

                        vertices.push_back({ {x, y, z}, {nx, ny, nz} });
                    }
                }

                // Connect rings
                int rings_created = static_cast<int>(vertices.size()) / n_segs;
                for (int ring = 0; ring < rings_created - 1; ++ring) {
                    for (int seg = 0; seg < n_segs; ++seg) {
                        int next_seg = (seg + 1) % n_segs;

                        uint32_t a = ring * n_segs + seg;
                        uint32_t b = ring * n_segs + next_seg;
                        uint32_t c = (ring + 1) * n_segs + seg;
                        uint32_t d = (ring + 1) * n_segs + next_seg;

                        indices.push_back(a); indices.push_back(c); indices.push_back(b);
                        indices.push_back(b); indices.push_back(c); indices.push_back(d);
                    }
                }

                // ─── BOTTOM CAP ───────────────────────────────────────────────────────────

                uint32_t bottom_center = static_cast<uint32_t>(vertices.size());
                vertices.push_back({ {0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f} });

                for (int seg = 0; seg < n_segs; ++seg) {
                    int next_seg = (seg + 1) % n_segs;
                    indices.push_back(bottom_center);
                    indices.push_back(next_seg);
                    indices.push_back(seg);
                }

                // ─── HEAD SPHERE ──────────────────────────────────────────────────────────
                //
                // Full sphere centered at (0, hcy, 0) with radius head_r.
                // We generate from top pole to bottom pole.

                const int sphere_rings = config.sphere_rings;
                const float head_r = params.head_radius;

                // Top pole
                uint32_t top_pole = static_cast<uint32_t>(vertices.size());
                vertices.push_back({ {0.0f, hcy + head_r, 0.0f}, {0.0f, 1.0f, 0.0f} });

                // Generate rings from just below top to just above bottom
                uint32_t first_ring = static_cast<uint32_t>(vertices.size());
                for (int ring = 1; ring < sphere_rings; ++ring) {
                    float phi = PI * ring / sphere_rings;  // 0 to π
                    float sin_phi = std::sin(phi);
                    float cos_phi = std::cos(phi);
                    float y = hcy + head_r * cos_phi;
                    float ring_r = head_r * sin_phi;

                    for (int seg = 0; seg < n_segs; ++seg) {
                        float theta = TWO_PI * seg / n_segs;
                        float x = ring_r * std::cos(theta);
                        float z = ring_r * std::sin(theta);

                        float nx = sin_phi * std::cos(theta);
                        float ny = cos_phi;
                        float nz = sin_phi * std::sin(theta);

                        vertices.push_back({ {x, y, z}, {nx, ny, nz} });
                    }
                }

                // Bottom pole
                uint32_t bottom_pole = static_cast<uint32_t>(vertices.size());
                vertices.push_back({ {0.0f, hcy - head_r, 0.0f}, {0.0f, -1.0f, 0.0f} });

                // Top cap: connect top pole to first ring (CCW when viewed from above)
                for (int seg = 0; seg < n_segs; ++seg) {
                    int next_seg = (seg + 1) % n_segs;
                    indices.push_back(top_pole);
                    indices.push_back(first_ring + next_seg);
                    indices.push_back(first_ring + seg);
                }

                // Middle rings: connect adjacent rings
                int num_middle_rings = sphere_rings - 2;  // Excludes pole rings
                for (int ring = 0; ring < num_middle_rings; ++ring) {
                    for (int seg = 0; seg < n_segs; ++seg) {
                        int next_seg = (seg + 1) % n_segs;

                        uint32_t a = first_ring + ring * n_segs + seg;
                        uint32_t b = first_ring + ring * n_segs + next_seg;
                        uint32_t c = first_ring + (ring + 1) * n_segs + seg;
                        uint32_t d = first_ring + (ring + 1) * n_segs + next_seg;

                        // CCW winding when viewed from outside
                        indices.push_back(a); indices.push_back(b); indices.push_back(c);
                        indices.push_back(b); indices.push_back(d); indices.push_back(c);
                    }
                }

                // Bottom cap: connect last ring to bottom pole (CCW when viewed from below)
                uint32_t last_ring = first_ring + num_middle_rings * n_segs;
                for (int seg = 0; seg < n_segs; ++seg) {
                    int next_seg = (seg + 1) % n_segs;
                    indices.push_back(last_ring + seg);
                    indices.push_back(last_ring + next_seg);
                    indices.push_back(bottom_pole);
                }
            }

        } // namespace MeshGen


        // ═══════════════════════════════════════════════════════════════════════════════
        // §5 GPU STATE CLASS
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

            // ─── Accessors ────────────────────────────────────────────────────────────

            wgpu::Buffer vertex_buffer() const { return vertexBuffer_; }
            wgpu::Buffer index_buffer() const { return indexBuffer_; }
            uint32_t index_count() const { return indexCount_; }

            wgpu::BindGroupLayout compute_bind_group_layout() const { return computeBindGroupLayout_; }
            wgpu::BindGroupLayout render_bind_group_layout() const { return renderBindGroupLayout_; }
            wgpu::BindGroup compute_bind_group() const { return computeBindGroup_; }
            wgpu::BindGroup render_bind_group() const { return renderBindGroup_; }

        private:
            wgpu::Device device_;

            wgpu::Buffer signalBuffer_;
            wgpu::Buffer cameraBuffer_;
            wgpu::Buffer vertexBuffer_;
            wgpu::Buffer indexBuffer_;
            uint32_t indexCount_ = 0;

            wgpu::BindGroupLayout computeBindGroupLayout_;
            wgpu::BindGroupLayout renderBindGroupLayout_;
            wgpu::BindGroup computeBindGroup_;
            wgpu::BindGroup renderBindGroup_;

            bool createBuffers() {
                wgpu::BufferDescriptor desc{};

                // Signal buffer
                desc.label = "Pawn Signal";
                desc.size = sizeof(GPUFrameSignal);
                desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
                signalBuffer_ = device_.CreateBuffer(&desc);
                if (!signalBuffer_) return false;

                // Camera buffer
                desc.label = "Pawn Camera";
                desc.size = sizeof(GPUCameraState);
                cameraBuffer_ = device_.CreateBuffer(&desc);
                if (!cameraBuffer_) return false;

                // Generate pawn mesh
                std::vector<Vertex> vertices;
                std::vector<uint32_t> indices;
                MeshGen::generate_pawn(vertices, indices);
                indexCount_ = static_cast<uint32_t>(indices.size());

                std::cout << "[Pawn Rasterize] Mesh: "
                    << vertices.size() << " vertices, "
                    << indices.size() << " indices\n";

                // Vertex buffer
                desc.label = "Pawn Vertices";
                desc.size = vertices.size() * sizeof(Vertex);
                desc.usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst;
                vertexBuffer_ = device_.CreateBuffer(&desc);
                if (!vertexBuffer_) return false;

                wgpu::Queue queue = device_.GetQueue();
                queue.WriteBuffer(vertexBuffer_, 0, vertices.data(), vertices.size() * sizeof(Vertex));

                // Index buffer
                desc.label = "Pawn Indices";
                desc.size = indices.size() * sizeof(uint32_t);
                desc.usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst;
                indexBuffer_ = device_.CreateBuffer(&desc);
                if (!indexBuffer_) return false;

                queue.WriteBuffer(indexBuffer_, 0, indices.data(), indices.size() * sizeof(uint32_t));

                return true;
            }

            bool createBindGroups() {
                // Compute layout: 0=signal, 1=camera(rw)
                {
                    std::array<wgpu::BindGroupLayoutEntry, 2> entries{};

                    entries[0].binding = 0;
                    entries[0].visibility = wgpu::ShaderStage::Compute;
                    entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

                    entries[1].binding = 1;
                    entries[1].visibility = wgpu::ShaderStage::Compute;
                    entries[1].buffer.type = wgpu::BufferBindingType::Storage;

                    wgpu::BindGroupLayoutDescriptor desc{};
                    desc.label = "Pawn Compute Layout";
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    computeBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
                    if (!computeBindGroupLayout_) return false;
                }

                // Render layout: 10=signal, 11=camera
                {
                    std::array<wgpu::BindGroupLayoutEntry, 2> entries{};

                    entries[0].binding = 10;
                    entries[0].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
                    entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

                    entries[1].binding = 11;
                    entries[1].visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
                    entries[1].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

                    wgpu::BindGroupLayoutDescriptor desc{};
                    desc.label = "Pawn Render Layout";
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    renderBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
                    if (!renderBindGroupLayout_) return false;
                }

                // Compute bind group
                {
                    std::array<wgpu::BindGroupEntry, 2> entries{};

                    entries[0].binding = 0;
                    entries[0].buffer = signalBuffer_;
                    entries[0].size = sizeof(GPUFrameSignal);

                    entries[1].binding = 1;
                    entries[1].buffer = cameraBuffer_;
                    entries[1].size = sizeof(GPUCameraState);

                    wgpu::BindGroupDescriptor desc{};
                    desc.label = "Pawn Compute BindGroup";
                    desc.layout = computeBindGroupLayout_;
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    computeBindGroup_ = device_.CreateBindGroup(&desc);
                    if (!computeBindGroup_) return false;
                }

                // Render bind group
                {
                    std::array<wgpu::BindGroupEntry, 2> entries{};

                    entries[0].binding = 10;
                    entries[0].buffer = signalBuffer_;
                    entries[0].size = sizeof(GPUFrameSignal);

                    entries[1].binding = 11;
                    entries[1].buffer = cameraBuffer_;
                    entries[1].size = sizeof(GPUCameraState);

                    wgpu::BindGroupDescriptor desc{};
                    desc.label = "Pawn Render BindGroup";
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
                camera.pos[1] = 1.0f;
                camera.pos[2] = Initial::CAMERA_DISTANCE;
                camera.azimuth = Initial::CAMERA_AZIMUTH;
                camera.elevation = Initial::CAMERA_ELEVATION;
                camera.distance = Initial::CAMERA_DISTANCE;
                camera.pan_x = 0.0f;
                camera.pan_y = 0.0f;
                queue.WriteBuffer(cameraBuffer_, 0, &camera, sizeof(camera));

                return true;
            }
        };


    } // namespace pawn_rasterize
} // namespace t7