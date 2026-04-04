#pragma once

/**
 * DESIGN CARTRIDGE 2 — State Management
 * ======================================
 *
 * Based on working design_cartridge (project's state.hpp).
 * ONLY ADDITION: Trajectory field ping-pong textures.
 *
 * All struct layouts IDENTICAL to design_cartridge.
 */

#include "analysis/analysis_signal.hpp"
#include <webgpu/webgpu_cpp.h>
#include <cstring>
#include <array>
#include <iostream>

namespace t7 {
    namespace design_cartridge_2 {


        // ═══════════════════════════════════════════════════════════════════════════════
        // §1 IDLE STATE — IDENTICAL TO DESIGN_CARTRIDGE
        // ═══════════════════════════════════════════════════════════════════════════════

        namespace Idle {
            constexpr float AMPLITUDE_SCALE = 1.0f;
            constexpr float MAX_AMPLITUDE = 2.0f;
            constexpr float SIZE = 100.0f;
            constexpr float LIPSCHITZ_FACTOR = 1.0f;

            constexpr float PAWN_POS_X = 0.0f;
            constexpr float PAWN_POS_Y = 0.0f;
            constexpr float PAWN_POS_Z = 0.0f;
            constexpr float PAWN_HEADING = 0.0f;
            constexpr float PAWN_QUAT_X = 0.0f;
            constexpr float PAWN_QUAT_Y = 0.0f;
            constexpr float PAWN_QUAT_Z = 0.0f;
            constexpr float PAWN_QUAT_W = 1.0f;

            constexpr float CAMERA_POS_X = 0.0f;
            constexpr float CAMERA_POS_Y = 15.0f;
            constexpr float CAMERA_POS_Z = 30.0f;
            constexpr float CAMERA_AZIMUTH = 0.0f;
            constexpr float CAMERA_ELEVATION = 0.4f;
            constexpr float CAMERA_DISTANCE = 30.0f;
            constexpr float CAMERA_PAN_X = 0.0f;
            constexpr float CAMERA_PAN_Y = 0.0f;

            constexpr float TRAJECTORY_VALUE = 1.0f;
            constexpr float TRAJECTORY_VELOCITY = 0.0f;

            constexpr float WAVE_TIME_SCALE = 1.0f;
            constexpr float PAWN_SPEED = 15.0f;
            constexpr float CAMERA_SENSITIVITY = 0.005f;
        }


        // ═══════════════════════════════════════════════════════════════════════════════
        // §2 COUPLING BITS — IDENTICAL TO DESIGN_CARTRIDGE
        // ═══════════════════════════════════════════════════════════════════════════════

        namespace Coupling {
            constexpr uint32_t POLYPHONY_TO_AMPLITUDE = 1u << 0;
            constexpr uint32_t TERRAIN_TO_PAWN_Y = 1u << 1;
            constexpr uint32_t TERRAIN_TO_PAWN_TILT = 1u << 2;
            constexpr uint32_t PAWN_TO_CAMERA_TARGET = 1u << 3;
            constexpr uint32_t INPUT_MOVES_PAWN = 1u << 4;
            constexpr uint32_t INPUT_ORBITS_CAMERA = 1u << 5;
            constexpr uint32_t INPUT_ZOOMS_CAMERA = 1u << 6;

            constexpr uint32_t ALL = 0x7Fu;
            constexpr uint32_t NONE = 0u;
            constexpr uint32_t SIGNAL = POLYPHONY_TO_AMPLITUDE;
            constexpr uint32_t TERRAIN = TERRAIN_TO_PAWN_Y | TERRAIN_TO_PAWN_TILT;
            constexpr uint32_t INPUT = INPUT_MOVES_PAWN | INPUT_ORBITS_CAMERA | INPUT_ZOOMS_CAMERA;
            constexpr uint32_t CAMERA = PAWN_TO_CAMERA_TARGET | INPUT_ORBITS_CAMERA | INPUT_ZOOMS_CAMERA;
        }


        // ═══════════════════════════════════════════════════════════════════════════════
        // §3 GPU STRUCTURES — IDENTICAL TO DESIGN_CARTRIDGE
        // ═══════════════════════════════════════════════════════════════════════════════

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

        struct alignas(16) GPUDesignConfig {
            uint32_t mute_dynamics_0d;
            uint32_t mute_dynamics_2d;
            uint32_t mute_signal;
            uint32_t mute_couplings;
            float wave_time_scale;
            float pawn_speed;
            float camera_sensitivity;
            float _pad0;
        };

        struct alignas(16) GPUTrajectory {
            float value;
            float velocity;
            float _pad0;
            float _pad1;
        };

        struct alignas(16) GPUGroundState {
            float amplitude_scale;
            float max_amplitude;
            float size;
            float lipschitz_factor;
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

        static_assert(sizeof(GPUFrameSignal) == 304, "GPUFrameSignal must be 304 bytes");
        static_assert(sizeof(GPUDesignConfig) == 32, "GPUDesignConfig must be 32 bytes");
        static_assert(sizeof(GPUTrajectory) == 16, "GPUTrajectory must be 16 bytes");
        static_assert(sizeof(GPUGroundState) == 16, "GPUGroundState must be 16 bytes");
        static_assert(sizeof(GPUPawnState) == 32, "GPUPawnState must be 32 bytes");
        static_assert(sizeof(GPUCameraState) == 32, "GPUCameraState must be 32 bytes");


        // ═══════════════════════════════════════════════════════════════════════════════
        // §4 CONSTANTS
        // ═══════════════════════════════════════════════════════════════════════════════

        constexpr int MAX_TRAJECTORIES = 16;
        constexpr uint32_t HEIGHT_FIELD_SIZE = 256;
        constexpr uint32_t TILE_TEXTURE_SIZE = 64;
        constexpr uint32_t TRAJECTORY_FIELD_SIZE = 256;  // ADDITION

        constexpr float FOG_COLOR_R = 0.85f;
        constexpr float FOG_COLOR_G = 0.78f;
        constexpr float FOG_COLOR_B = 0.72f;


        // ═══════════════════════════════════════════════════════════════════════════════
        // §5 GPU STATE CLASS
        // ═══════════════════════════════════════════════════════════════════════════════

        class GPUState {
        public:
            GPUState() = default;

            bool init(wgpu::Device device) {
                device_ = device;

                if (!createBuffers()) return false;
                if (!createTextures()) return false;
                if (!createSamplers()) return false;
                if (!createBindGroups()) return false;
                if (!initializeState()) return false;

                std::cout << "Design Cartridge 2 state initialized\n";
                std::cout << "  Trajectory field: " << TRAJECTORY_FIELD_SIZE << "x" << TRAJECTORY_FIELD_SIZE << "\n";

                return true;
            }

            void upload_signal(wgpu::Queue& queue, const GPUFrameSignal& signal) {
                queue.WriteBuffer(signalBuffer_, 0, &signal, sizeof(GPUFrameSignal));
            }

            void upload_config(wgpu::Queue& queue) {
                queue.WriteBuffer(configBuffer_, 0, &config_, sizeof(GPUDesignConfig));
            }

            // ─── Design Mode Control ─────────────────────────────────────────────────

            void enter_design_mode() {
                config_.mute_signal = 1;
                config_.mute_couplings = Coupling::ALL;
            }

            void enter_performance_mode() {
                config_.mute_signal = 0;
                config_.mute_couplings = Coupling::NONE;
            }

            void set_mute_signal(bool muted) { config_.mute_signal = muted ? 1 : 0; }
            void set_mute_dynamics_0d(bool muted) { config_.mute_dynamics_0d = muted ? 1 : 0; }
            void set_mute_dynamics_2d(bool muted) { config_.mute_dynamics_2d = muted ? 1 : 0; }

            void set_mute_coupling(uint32_t coupling_bit, bool muted) {
                if (muted) {
                    config_.mute_couplings |= coupling_bit;
                }
                else {
                    config_.mute_couplings &= ~coupling_bit;
                }
            }

            void set_mute_couplings(uint32_t mask) { config_.mute_couplings = mask; }

            void set_wave_time_scale(float scale) { config_.wave_time_scale = scale; }
            void set_pawn_speed(float speed) { config_.pawn_speed = speed; }
            void set_camera_sensitivity(float sensitivity) { config_.camera_sensitivity = sensitivity; }

            GPUDesignConfig& config() { return config_; }
            const GPUDesignConfig& config() const { return config_; }

            // ─── Bind Groups ─────────────────────────────────────────────────────────

            wgpu::BindGroup compute_entity_bind_group() const { return computeEntityBindGroup_; }
            wgpu::BindGroup render_entity_bind_group() const { return renderEntityBindGroup_; }
            wgpu::BindGroupLayout compute_entity_bind_group_layout() const { return computeEntityBindGroupLayout_; }
            wgpu::BindGroupLayout render_entity_bind_group_layout() const { return renderEntityBindGroupLayout_; }

            // MODIFIED: Now returns ping-pong bind groups
            wgpu::BindGroup compute_texture_bind_group() const {
                return trajPingPong_ ? computeTextureBindGroupB_ : computeTextureBindGroupA_;
            }
            wgpu::BindGroup render_texture_bind_group() const { return renderTextureBindGroup_; }
            wgpu::BindGroupLayout compute_texture_bind_group_layout() const { return computeTextureBindGroupLayout_; }
            wgpu::BindGroupLayout render_texture_bind_group_layout() const { return renderTextureBindGroupLayout_; }

            // ADDITION: Swap trajectory field ping-pong
            void swap_trajectory_field() { trajPingPong_ = !trajPingPong_; }

            // ─── Dispatch Dimensions ─────────────────────────────────────────────────

            static constexpr uint32_t height_field_workgroups() { return HEIGHT_FIELD_SIZE / 8; }
            static constexpr uint32_t tile_texture_workgroups() { return TILE_TEXTURE_SIZE / 8; }
            static constexpr uint32_t trajectory_field_workgroups() { return TRAJECTORY_FIELD_SIZE / 8; }  // ADDITION

        private:
            wgpu::Device device_;
            GPUDesignConfig config_{};
            bool trajPingPong_ = false;  // ADDITION

            // ─── Buffers ─────────────────────────────────────────────────────────────
            wgpu::Buffer signalBuffer_;
            wgpu::Buffer configBuffer_;
            wgpu::Buffer groundBuffer_;
            wgpu::Buffer pawnBuffer_;
            wgpu::Buffer cameraBuffer_;
            wgpu::Buffer trajectoriesBuffer_;

            // ─── Textures ────────────────────────────────────────────────────────────
            wgpu::Texture heightFieldTexture_;
            wgpu::TextureView heightFieldWriteView_;
            wgpu::TextureView heightFieldReadView_;

            wgpu::Texture tileStateTexture_;
            wgpu::TextureView tileStateWriteView_;
            wgpu::TextureView tileStateReadView_;

            // ADDITION: Trajectory field ping-pong
            wgpu::Texture trajectoryFieldA_;
            wgpu::Texture trajectoryFieldB_;
            wgpu::TextureView trajFieldViewA_;
            wgpu::TextureView trajFieldViewB_;

            // ─── Samplers ────────────────────────────────────────────────────────────
            wgpu::Sampler bilinearSampler_;
            wgpu::Sampler nearestSampler_;

            // ─── Bind Groups ─────────────────────────────────────────────────────────
            wgpu::BindGroupLayout computeEntityBindGroupLayout_;
            wgpu::BindGroupLayout renderEntityBindGroupLayout_;
            wgpu::BindGroupLayout computeTextureBindGroupLayout_;
            wgpu::BindGroupLayout renderTextureBindGroupLayout_;

            wgpu::BindGroup computeEntityBindGroup_;
            wgpu::BindGroup renderEntityBindGroup_;
            // MODIFIED: Two compute texture bind groups for ping-pong
            wgpu::BindGroup computeTextureBindGroupA_;
            wgpu::BindGroup computeTextureBindGroupB_;
            wgpu::BindGroup renderTextureBindGroup_;

            // ─────────────────────────────────────────────────────────────────────────
            // CREATION METHODS
            // ─────────────────────────────────────────────────────────────────────────

            bool createBuffers() {
                wgpu::BufferDescriptor desc{};

                desc.size = sizeof(GPUFrameSignal);
                desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
                desc.label = "Signal Buffer";
                signalBuffer_ = device_.CreateBuffer(&desc);

                desc.size = sizeof(GPUDesignConfig);
                desc.label = "Config Buffer";
                configBuffer_ = device_.CreateBuffer(&desc);

                desc.size = sizeof(GPUGroundState);
                desc.label = "Ground Buffer";
                groundBuffer_ = device_.CreateBuffer(&desc);

                desc.size = sizeof(GPUPawnState);
                desc.label = "Pawn Buffer";
                pawnBuffer_ = device_.CreateBuffer(&desc);

                desc.size = sizeof(GPUCameraState);
                desc.label = "Camera Buffer";
                cameraBuffer_ = device_.CreateBuffer(&desc);

                desc.size = sizeof(GPUTrajectory) * MAX_TRAJECTORIES;
                desc.label = "Trajectories Buffer";
                trajectoriesBuffer_ = device_.CreateBuffer(&desc);

                return true;
            }

            bool createTextures() {
                // Height field
                {
                    wgpu::TextureDescriptor desc{};
                    desc.size = { HEIGHT_FIELD_SIZE, HEIGHT_FIELD_SIZE, 1 };
                    desc.format = wgpu::TextureFormat::RGBA16Float;
                    desc.usage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::TextureBinding;
                    desc.label = "Height Field";
                    heightFieldTexture_ = device_.CreateTexture(&desc);
                    heightFieldWriteView_ = heightFieldTexture_.CreateView();
                    heightFieldReadView_ = heightFieldTexture_.CreateView();
                }

                // Tile state
                {
                    wgpu::TextureDescriptor desc{};
                    desc.size = { TILE_TEXTURE_SIZE, TILE_TEXTURE_SIZE, 1 };
                    desc.format = wgpu::TextureFormat::RGBA8Unorm;
                    desc.usage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::TextureBinding;
                    desc.label = "Tile State";
                    tileStateTexture_ = device_.CreateTexture(&desc);
                    tileStateWriteView_ = tileStateTexture_.CreateView();
                    tileStateReadView_ = tileStateTexture_.CreateView();
                }

                // ADDITION: Trajectory field ping-pong
                {
                    wgpu::TextureDescriptor desc{};
                    desc.size = { TRAJECTORY_FIELD_SIZE, TRAJECTORY_FIELD_SIZE, 1 };
                    desc.format = wgpu::TextureFormat::RG32Float;
                    desc.usage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::TextureBinding;
                    desc.label = "Trajectory Field A";
                    trajectoryFieldA_ = device_.CreateTexture(&desc);
                    trajFieldViewA_ = trajectoryFieldA_.CreateView();

                    desc.label = "Trajectory Field B";
                    trajectoryFieldB_ = device_.CreateTexture(&desc);
                    trajFieldViewB_ = trajectoryFieldB_.CreateView();
                }

                return true;
            }

            bool createSamplers() {
                wgpu::SamplerDescriptor bilinearDesc{};
                bilinearDesc.minFilter = wgpu::FilterMode::Linear;
                bilinearDesc.magFilter = wgpu::FilterMode::Linear;
                bilinearDesc.addressModeU = wgpu::AddressMode::ClampToEdge;
                bilinearDesc.addressModeV = wgpu::AddressMode::ClampToEdge;
                bilinearDesc.label = "Bilinear Sampler";
                bilinearSampler_ = device_.CreateSampler(&bilinearDesc);

                wgpu::SamplerDescriptor nearestDesc{};
                nearestDesc.minFilter = wgpu::FilterMode::Nearest;
                nearestDesc.magFilter = wgpu::FilterMode::Nearest;
                nearestDesc.addressModeU = wgpu::AddressMode::ClampToEdge;
                nearestDesc.addressModeV = wgpu::AddressMode::ClampToEdge;
                nearestDesc.label = "Nearest Sampler";
                nearestSampler_ = device_.CreateSampler(&nearestDesc);

                return true;
            }

            bool createBindGroups() {
                // ═══════════════════════════════════════════════════════════════════
                // GROUP 0: Entity buffers — IDENTICAL TO DESIGN_CARTRIDGE
                // ═══════════════════════════════════════════════════════════════════

                // Compute entity layout: bindings 0-4
                {
                    std::array<wgpu::BindGroupLayoutEntry, 5> entries{};

                    entries[0].binding = 0;
                    entries[0].visibility = wgpu::ShaderStage::Compute;
                    entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

                    for (int i = 1; i < 5; ++i) {
                        entries[i].binding = i;
                        entries[i].visibility = wgpu::ShaderStage::Compute;
                        entries[i].buffer.type = wgpu::BufferBindingType::Storage;
                    }

                    wgpu::BindGroupLayoutDescriptor desc{};
                    desc.label = "Compute Entity Layout";
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    computeEntityBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
                }

                // Render entity layout: bindings 10-13
                {
                    std::array<wgpu::BindGroupLayoutEntry, 4> entries{};

                    for (int i = 0; i < 4; ++i) {
                        entries[i].binding = 10 + i;
                        entries[i].visibility = wgpu::ShaderStage::Fragment;
                        entries[i].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
                    }

                    wgpu::BindGroupLayoutDescriptor desc{};
                    desc.label = "Render Entity Layout";
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    renderEntityBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
                }

                // Compute entity bind group
                {
                    wgpu::Buffer buffers[] = { signalBuffer_, groundBuffer_, pawnBuffer_, cameraBuffer_, trajectoriesBuffer_ };
                    size_t sizes[] = { sizeof(GPUFrameSignal), sizeof(GPUGroundState),
                                      sizeof(GPUPawnState), sizeof(GPUCameraState),
                                      sizeof(GPUTrajectory) * MAX_TRAJECTORIES };

                    std::array<wgpu::BindGroupEntry, 5> entries{};
                    for (int i = 0; i < 5; ++i) {
                        entries[i].binding = i;
                        entries[i].buffer = buffers[i];
                        entries[i].size = sizes[i];
                    }

                    wgpu::BindGroupDescriptor desc{};
                    desc.label = "Compute Entity BindGroup";
                    desc.layout = computeEntityBindGroupLayout_;
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    computeEntityBindGroup_ = device_.CreateBindGroup(&desc);
                }

                // Render entity bind group
                {
                    wgpu::Buffer buffers[] = { signalBuffer_, groundBuffer_, pawnBuffer_, cameraBuffer_ };
                    size_t sizes[] = { sizeof(GPUFrameSignal), sizeof(GPUGroundState),
                                      sizeof(GPUPawnState), sizeof(GPUCameraState) };

                    std::array<wgpu::BindGroupEntry, 4> entries{};
                    for (int i = 0; i < 4; ++i) {
                        entries[i].binding = 10 + i;
                        entries[i].buffer = buffers[i];
                        entries[i].size = sizes[i];
                    }

                    wgpu::BindGroupDescriptor desc{};
                    desc.label = "Render Entity BindGroup";
                    desc.layout = renderEntityBindGroupLayout_;
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    renderEntityBindGroup_ = device_.CreateBindGroup(&desc);
                }

                // ═══════════════════════════════════════════════════════════════════
                // GROUP 1: 2D Textures — MODIFIED FOR TRAJECTORY FIELD
                // ═══════════════════════════════════════════════════════════════════

                // Compute texture layout: bindings 0-3
                {
                    std::array<wgpu::BindGroupLayoutEntry, 4> entries{};

                    // binding 0: height_field_write (storage)
                    entries[0].binding = 0;
                    entries[0].visibility = wgpu::ShaderStage::Compute;
                    entries[0].storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
                    entries[0].storageTexture.format = wgpu::TextureFormat::RGBA16Float;
                    entries[0].storageTexture.viewDimension = wgpu::TextureViewDimension::e2D;

                    // binding 1: tile_state_write (storage)
                    entries[1].binding = 1;
                    entries[1].visibility = wgpu::ShaderStage::Compute;
                    entries[1].storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
                    entries[1].storageTexture.format = wgpu::TextureFormat::RGBA8Unorm;
                    entries[1].storageTexture.viewDimension = wgpu::TextureViewDimension::e2D;

                    // ADDITION: binding 2: trajectory_field_read (texture)
                    // RG32Float is unfilterable - must use UnfilterableFloat
                    entries[2].binding = 2;
                    entries[2].visibility = wgpu::ShaderStage::Compute;
                    entries[2].texture.sampleType = wgpu::TextureSampleType::UnfilterableFloat;
                    entries[2].texture.viewDimension = wgpu::TextureViewDimension::e2D;

                    // ADDITION: binding 3: trajectory_field_write (storage)
                    entries[3].binding = 3;
                    entries[3].visibility = wgpu::ShaderStage::Compute;
                    entries[3].storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
                    entries[3].storageTexture.format = wgpu::TextureFormat::RG32Float;
                    entries[3].storageTexture.viewDimension = wgpu::TextureViewDimension::e2D;

                    wgpu::BindGroupLayoutDescriptor desc{};
                    desc.label = "Compute Texture Layout";
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    computeTextureBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
                }

                // Render texture layout: bindings 10-13 — IDENTICAL TO DESIGN_CARTRIDGE
                {
                    std::array<wgpu::BindGroupLayoutEntry, 4> entries{};

                    entries[0].binding = 10;
                    entries[0].visibility = wgpu::ShaderStage::Fragment;
                    entries[0].texture.sampleType = wgpu::TextureSampleType::Float;
                    entries[0].texture.viewDimension = wgpu::TextureViewDimension::e2D;

                    entries[1].binding = 11;
                    entries[1].visibility = wgpu::ShaderStage::Fragment;
                    entries[1].texture.sampleType = wgpu::TextureSampleType::Float;
                    entries[1].texture.viewDimension = wgpu::TextureViewDimension::e2D;

                    entries[2].binding = 12;
                    entries[2].visibility = wgpu::ShaderStage::Fragment;
                    entries[2].sampler.type = wgpu::SamplerBindingType::Filtering;

                    entries[3].binding = 13;
                    entries[3].visibility = wgpu::ShaderStage::Fragment;
                    entries[3].sampler.type = wgpu::SamplerBindingType::NonFiltering;

                    wgpu::BindGroupLayoutDescriptor desc{};
                    desc.label = "Render Texture Layout";
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    renderTextureBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
                }

                // MODIFIED: Compute texture bind groups (ping-pong A and B)
                {
                    // A: read from A, write to B
                    std::array<wgpu::BindGroupEntry, 4> entriesA{};
                    entriesA[0].binding = 0;
                    entriesA[0].textureView = heightFieldWriteView_;
                    entriesA[1].binding = 1;
                    entriesA[1].textureView = tileStateWriteView_;
                    entriesA[2].binding = 2;
                    entriesA[2].textureView = trajFieldViewA_;  // read A
                    entriesA[3].binding = 3;
                    entriesA[3].textureView = trajFieldViewB_;  // write B

                    wgpu::BindGroupDescriptor descA{};
                    descA.label = "Compute Texture BindGroup A";
                    descA.layout = computeTextureBindGroupLayout_;
                    descA.entryCount = entriesA.size();
                    descA.entries = entriesA.data();

                    computeTextureBindGroupA_ = device_.CreateBindGroup(&descA);

                    // B: read from B, write to A
                    std::array<wgpu::BindGroupEntry, 4> entriesB{};
                    entriesB[0].binding = 0;
                    entriesB[0].textureView = heightFieldWriteView_;
                    entriesB[1].binding = 1;
                    entriesB[1].textureView = tileStateWriteView_;
                    entriesB[2].binding = 2;
                    entriesB[2].textureView = trajFieldViewB_;  // read B
                    entriesB[3].binding = 3;
                    entriesB[3].textureView = trajFieldViewA_;  // write A

                    wgpu::BindGroupDescriptor descB{};
                    descB.label = "Compute Texture BindGroup B";
                    descB.layout = computeTextureBindGroupLayout_;
                    descB.entryCount = entriesB.size();
                    descB.entries = entriesB.data();

                    computeTextureBindGroupB_ = device_.CreateBindGroup(&descB);
                }

                // Render texture bind group — IDENTICAL TO DESIGN_CARTRIDGE
                {
                    std::array<wgpu::BindGroupEntry, 4> entries{};

                    entries[0].binding = 10;
                    entries[0].textureView = heightFieldReadView_;

                    entries[1].binding = 11;
                    entries[1].textureView = tileStateReadView_;

                    entries[2].binding = 12;
                    entries[2].sampler = bilinearSampler_;

                    entries[3].binding = 13;
                    entries[3].sampler = nearestSampler_;

                    wgpu::BindGroupDescriptor desc{};
                    desc.label = "Render Texture BindGroup";
                    desc.layout = renderTextureBindGroupLayout_;
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    renderTextureBindGroup_ = device_.CreateBindGroup(&desc);
                }

                return true;
            }

            bool initializeState() {
                wgpu::Queue queue = device_.GetQueue();

                // Config
                config_.mute_dynamics_0d = 0;
                config_.mute_dynamics_2d = 0;
                config_.mute_signal = 0;
                config_.mute_couplings = Coupling::NONE;
                config_.wave_time_scale = Idle::WAVE_TIME_SCALE;
                config_.pawn_speed = Idle::PAWN_SPEED;
                config_.camera_sensitivity = Idle::CAMERA_SENSITIVITY;
                queue.WriteBuffer(configBuffer_, 0, &config_, sizeof(config_));

                // Ground
                GPUGroundState ground{};
                ground.amplitude_scale = Idle::AMPLITUDE_SCALE;
                ground.max_amplitude = Idle::MAX_AMPLITUDE;
                ground.size = Idle::SIZE;
                ground.lipschitz_factor = Idle::LIPSCHITZ_FACTOR;
                queue.WriteBuffer(groundBuffer_, 0, &ground, sizeof(ground));

                // Pawn
                GPUPawnState pawn{};
                pawn.pos[0] = Idle::PAWN_POS_X;
                pawn.pos[1] = Idle::PAWN_POS_Y;
                pawn.pos[2] = Idle::PAWN_POS_Z;
                pawn.heading = Idle::PAWN_HEADING;
                pawn.orientation[0] = Idle::PAWN_QUAT_X;
                pawn.orientation[1] = Idle::PAWN_QUAT_Y;
                pawn.orientation[2] = Idle::PAWN_QUAT_Z;
                pawn.orientation[3] = Idle::PAWN_QUAT_W;
                queue.WriteBuffer(pawnBuffer_, 0, &pawn, sizeof(pawn));

                // Camera
                GPUCameraState camera{};
                camera.pos[0] = Idle::CAMERA_POS_X;
                camera.pos[1] = Idle::CAMERA_POS_Y;
                camera.pos[2] = Idle::CAMERA_POS_Z;
                camera.azimuth = Idle::CAMERA_AZIMUTH;
                camera.elevation = Idle::CAMERA_ELEVATION;
                camera.distance = Idle::CAMERA_DISTANCE;
                camera.pan_x = Idle::CAMERA_PAN_X;
                camera.pan_y = Idle::CAMERA_PAN_Y;
                queue.WriteBuffer(cameraBuffer_, 0, &camera, sizeof(camera));

                // Trajectories
                GPUTrajectory trajectories[MAX_TRAJECTORIES]{};
                for (int i = 0; i < MAX_TRAJECTORIES; ++i) {
                    trajectories[i].value = Idle::TRAJECTORY_VALUE;
                    trajectories[i].velocity = Idle::TRAJECTORY_VELOCITY;
                }
                queue.WriteBuffer(trajectoriesBuffer_, 0, trajectories, sizeof(trajectories));

                return true;
            }
        };

    } // namespace design_cartridge_2
} // namespace t7