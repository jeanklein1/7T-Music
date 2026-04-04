#pragma once

/**
 * PLAYGROUND CARTRIDGE — GPU State Management
 * ============================================
 *
 * Minimal state for sculpture viewing. One static object, orbital camera.
 * No movement, no music, no doors—just shape and observation.
 *
 * DIMENSIONALITY:
 *   0D — Buffers: signal, camera
 *   No entity state (pawn position is constant in shader)
 *
 * BIND GROUPS:
 *   Group 0: All buffers (compute and render share layout)
 *
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include <webgpu/webgpu_cpp.h>
#include <cstring>
#include <array>
#include <iostream>

namespace t7 {
    namespace playground_raymarch {


        // ═══════════════════════════════════════════════════════════════════════════════
        // §1 INITIAL STATE
        // ═══════════════════════════════════════════════════════════════════════════════

        namespace Initial {
            // Camera starts at 3/4 view of the subject
            constexpr float CAMERA_AZIMUTH = 0.5f;
            constexpr float CAMERA_ELEVATION = 0.3f;
            constexpr float CAMERA_DISTANCE = 5.0f;
            constexpr float CAMERA_PAN_X = 0.0f;
            constexpr float CAMERA_PAN_Y = 0.0f;
        }


        // ═══════════════════════════════════════════════════════════════════════════════
        // §2 GPU STRUCTURES — Must match world.wgsl exactly
        // ═══════════════════════════════════════════════════════════════════════════════

        // ─── Signal (CPU → GPU each frame) ──────────────────────────────────────────
        // Minimal: just timing and aspect ratio

        struct alignas(16) GPUFrameSignal {
            float t_seconds;
            float dt;
            float aspect_ratio;
            float _pad0;

            // Camera input deltas
            float look_az_delta;
            float look_el_delta;
            float zoom_delta;
            float pan_x_delta;
            float pan_y_delta;
            float _pad1;
            float _pad2;
            float _pad3;
        };

        // ─── Camera State ───────────────────────────────────────────────────────────

        struct alignas(16) GPUCameraState {
            float pos[3];
            float azimuth;
            float elevation;
            float distance;
            float pan_x;
            float pan_y;
        };

        // ─── Size verification ──────────────────────────────────────────────────────

        static_assert(sizeof(GPUFrameSignal) == 48, "GPUFrameSignal must be 48 bytes");
        static_assert(sizeof(GPUCameraState) == 32, "GPUCameraState must be 32 bytes");


        // ═══════════════════════════════════════════════════════════════════════════════
        // §3 GPU STATE CLASS
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
                desc.label = "Playground Signal";
                desc.size = sizeof(GPUFrameSignal);
                desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
                signalBuffer_ = device_.CreateBuffer(&desc);
                if (!signalBuffer_) return false;

                // Camera state buffer
                desc.label = "Playground Camera State";
                desc.size = sizeof(GPUCameraState);
                desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
                cameraBuffer_ = device_.CreateBuffer(&desc);
                if (!cameraBuffer_) return false;

                return true;
            }

            bool createBindGroups() {
                // ─── Compute Bind Group Layout ──────────────────────────────────────
                // Bindings 0-1: signal (read), camera (read_write)
                {
                    std::array<wgpu::BindGroupLayoutEntry, 2> entries{};

                    // Binding 0: signal (read)
                    entries[0].binding = 0;
                    entries[0].visibility = wgpu::ShaderStage::Compute;
                    entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

                    // Binding 1: camera_state (read_write)
                    entries[1].binding = 1;
                    entries[1].visibility = wgpu::ShaderStage::Compute;
                    entries[1].buffer.type = wgpu::BufferBindingType::Storage;

                    wgpu::BindGroupLayoutDescriptor desc{};
                    desc.label = "Playground Compute BindGroup Layout";
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    computeBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
                    if (!computeBindGroupLayout_) return false;
                }

                // ─── Render Bind Group Layout ───────────────────────────────────────
                // Bindings 10-11: signal, camera (read only for fragment)
                {
                    std::array<wgpu::BindGroupLayoutEntry, 2> entries{};

                    // Binding 10: render_signal (read)
                    entries[0].binding = 10;
                    entries[0].visibility = wgpu::ShaderStage::Fragment;
                    entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

                    // Binding 11: render_camera (read)
                    entries[1].binding = 11;
                    entries[1].visibility = wgpu::ShaderStage::Fragment;
                    entries[1].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

                    wgpu::BindGroupLayoutDescriptor desc{};
                    desc.label = "Playground Render BindGroup Layout";
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
                    desc.label = "Playground Compute BindGroup";
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
                    desc.label = "Playground Render BindGroup";
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

                // ─── Initialize camera ──────────────────────────────────────────────
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


    } // namespace playground_raymarch
} // namespace t7