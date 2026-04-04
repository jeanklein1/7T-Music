#pragma once

/**
 * GALLERY CARTRIDGE — GPU State Management
 * =========================================
 *
 * Precomputed room field texture for fast raymarching.
 *
 * TEXTURE ARCHITECTURE:
 *   room_field (128×128, RGBA16Float):
 *     .r = signed distance to walls (with door holes carved)
 *     .g = material hint (wall edge detection)
 *     .b = reserved
 *     .a = reserved
 *
 * See world.wgsl for the GPU scroll.
 */

#include <webgpu/webgpu_cpp.h>
#include <cstring>
#include <cmath>
#include <array>
#include <iostream>

#include "core/cartridge_ids.hpp"

namespace t7 {
    namespace gallery_raymarch {


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

        // Colors
        constexpr float FOG_COLOR_R = 0.88f;
        constexpr float FOG_COLOR_G = 0.90f;
        constexpr float FOG_COLOR_B = 0.92f;

        // Texture
        constexpr uint32_t ROOM_FIELD_SIZE = 128;


        // ═══════════════════════════════════════════════════════════════════════════════
        // §2 DOOR SHAPE TYPES
        // ═══════════════════════════════════════════════════════════════════════════════

        namespace DoorShape {
            constexpr uint32_t RECTANGLE = 0;
            constexpr uint32_t ARCH = 1;
            constexpr uint32_t CIRCLE = 2;
            constexpr uint32_t POINTED_ARCH = 3;
        }

        // DoorTarget is an alias for CartridgeId (single source of truth)
        namespace DoorTarget = t7::CartridgeId;


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
            constexpr uint32_t INPUT_MOVES_PAWN = 1u << 0;
            constexpr uint32_t PAWN_TO_CAMERA_TARGET = 1u << 1;
            constexpr uint32_t INPUT_ORBITS_CAMERA = 1u << 2;
            constexpr uint32_t INPUT_ZOOMS_CAMERA = 1u << 3;

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

        struct alignas(16) GPUPortableDoor {
            float position[3];
            float _pad0;
            float forward[3];
            float _pad1;
            float up[3];
            float _pad2;

            float width;
            float height;
            float thickness;
            float commitment_depth;

            uint32_t shape_type;
            uint32_t target_id;
            uint32_t _pad3;
            uint32_t _pad4;

            float color[3];
            float glow;
        };

        struct alignas(16) GPUDoorHeader {
            uint32_t count;
            uint32_t _pad0;
            uint32_t _pad1;
            uint32_t _pad2;
        };

        struct alignas(16) GPURoomParams {
            float room_half_width;
            float room_height;
            float room_half_depth;
            float field_world_size;
        };

        static_assert(sizeof(GPUFrameSignal) == 48, "GPUFrameSignal must be 48 bytes");
        static_assert(sizeof(GPUDesignConfig) == 16, "GPUDesignConfig must be 16 bytes");
        static_assert(sizeof(GPUPawnState) == 32, "GPUPawnState must be 32 bytes");
        static_assert(sizeof(GPUCameraState) == 32, "GPUCameraState must be 32 bytes");
        static_assert(sizeof(GPUPortableDoor) == 96, "GPUPortableDoor must be 96 bytes");
        static_assert(sizeof(GPUDoorHeader) == 16, "GPUDoorHeader must be 16 bytes");
        static_assert(sizeof(GPURoomParams) == 16, "GPURoomParams must be 16 bytes");


        // ═══════════════════════════════════════════════════════════════════════════════
        // §6 PORTABLE DOOR — CPU-side
        // ═══════════════════════════════════════════════════════════════════════════════

        class PortableDoor {
        public:
            GPUPortableDoor gpu;
            float right[3];
            bool committed = false;

            void compute_derived() {
                right[0] = gpu.up[1] * gpu.forward[2] - gpu.up[2] * gpu.forward[1];
                right[1] = gpu.up[2] * gpu.forward[0] - gpu.up[0] * gpu.forward[2];
                right[2] = gpu.up[0] * gpu.forward[1] - gpu.up[1] * gpu.forward[0];

                float len = std::sqrt(right[0] * right[0] + right[1] * right[1] + right[2] * right[2]);
                if (len > 0.0001f) {
                    right[0] /= len;
                    right[1] /= len;
                    right[2] /= len;
                }
            }

            void world_to_local(float wx, float wy, float wz,
                float& lx, float& ly, float& lz) const {
                float dx = wx - gpu.position[0];
                float dy = wy - gpu.position[1];
                float dz = wz - gpu.position[2];

                lx = dx * right[0] + dy * right[1] + dz * right[2];
                ly = dx * gpu.up[0] + dy * gpu.up[1] + dz * gpu.up[2];
                lz = dx * gpu.forward[0] + dy * gpu.forward[1] + dz * gpu.forward[2];
            }

            bool point_in_volume(float x, float y, float z) const {
                float lx, ly, lz;
                world_to_local(x, y, z, lx, ly, lz);

                float half_w = gpu.width * 0.5f;
                return std::abs(lx) < half_w &&
                    ly > 0.0f && ly < gpu.height &&
                    std::abs(lz) < gpu.thickness * 0.5f;
            }

            bool point_committed(float x, float y, float z) const {
                float lx, ly, lz;
                world_to_local(x, y, z, lx, ly, lz);

                float half_w = gpu.width * 0.5f;
                return std::abs(lx) < half_w &&
                    ly > 0.0f && ly < gpu.height &&
                    lz > gpu.commitment_depth;
            }

            uint32_t target() const { return gpu.target_id; }
        };


        // ═══════════════════════════════════════════════════════════════════════════════
        // §7 DOOR BUILDER
        // ═══════════════════════════════════════════════════════════════════════════════

        class PortableDoorBuilder {
        public:
            PortableDoorBuilder() {
                door_.gpu = {};
                door_.gpu.up[1] = 1.0f;
                door_.gpu.forward[2] = -1.0f;
                door_.gpu.width = 2.0f;
                door_.gpu.height = 3.0f;
                // Door volume extends deep into the void so pawn can walk far past commitment
                // thickness=12 means volume extends ±6 units from door plane
                door_.gpu.thickness = 12.0f;
                // Commitment at 1.5 = ~3 pawn radii past door plane
                // Leaves 4.5 units of margin before exiting volume
                door_.gpu.commitment_depth = 1.5f;
                door_.gpu.shape_type = DoorShape::RECTANGLE;
                door_.gpu.target_id = DoorTarget::NONE;
                door_.gpu.color[0] = 0.8f;
                door_.gpu.color[1] = 0.8f;
                door_.gpu.color[2] = 0.8f;
                door_.gpu.glow = 0.0f;
            }

            PortableDoorBuilder& at(float x, float y, float z) {
                door_.gpu.position[0] = x;
                door_.gpu.position[1] = y;
                door_.gpu.position[2] = z;
                return *this;
            }

            PortableDoorBuilder& facing(float fx, float fy, float fz) {
                float len = std::sqrt(fx * fx + fy * fy + fz * fz);
                door_.gpu.forward[0] = fx / len;
                door_.gpu.forward[1] = fy / len;
                door_.gpu.forward[2] = fz / len;
                return *this;
            }

            PortableDoorBuilder& in_back_wall(float offset_x) {
                door_.gpu.position[0] = offset_x;
                door_.gpu.position[1] = 0.0f;
                door_.gpu.position[2] = -ROOM_HALF_DEPTH;
                door_.gpu.forward[0] = 0.0f;
                door_.gpu.forward[1] = 0.0f;
                door_.gpu.forward[2] = -1.0f;
                return *this;
            }

            PortableDoorBuilder& in_front_wall(float offset_x) {
                door_.gpu.position[0] = offset_x;
                door_.gpu.position[1] = 0.0f;
                door_.gpu.position[2] = ROOM_HALF_DEPTH;
                door_.gpu.forward[0] = 0.0f;
                door_.gpu.forward[1] = 0.0f;
                door_.gpu.forward[2] = 1.0f;
                return *this;
            }

            PortableDoorBuilder& in_left_wall(float offset_z) {
                door_.gpu.position[0] = -ROOM_HALF_WIDTH;
                door_.gpu.position[1] = 0.0f;
                door_.gpu.position[2] = offset_z;
                door_.gpu.forward[0] = -1.0f;
                door_.gpu.forward[1] = 0.0f;
                door_.gpu.forward[2] = 0.0f;
                return *this;
            }

            PortableDoorBuilder& in_right_wall(float offset_z) {
                door_.gpu.position[0] = ROOM_HALF_WIDTH;
                door_.gpu.position[1] = 0.0f;
                door_.gpu.position[2] = offset_z;
                door_.gpu.forward[0] = 1.0f;
                door_.gpu.forward[1] = 0.0f;
                door_.gpu.forward[2] = 0.0f;
                return *this;
            }

            PortableDoorBuilder& shape(uint32_t s) { door_.gpu.shape_type = s; return *this; }
            PortableDoorBuilder& size(float w, float h) { door_.gpu.width = w; door_.gpu.height = h; return *this; }
            PortableDoorBuilder& depth(float thickness, float commitment) {
                door_.gpu.thickness = thickness;
                door_.gpu.commitment_depth = commitment;
                return *this;
            }
            PortableDoorBuilder& target(uint32_t t) { door_.gpu.target_id = t; return *this; }
            PortableDoorBuilder& color(float r, float g, float b) {
                door_.gpu.color[0] = r; door_.gpu.color[1] = g; door_.gpu.color[2] = b;
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
                if (!createTextures()) return false;
                if (!createSamplers()) return false;
                if (!createBindGroups()) return false;
                if (!initializeState()) return false;

                return true;
            }

            void upload_signal(wgpu::Queue& queue, const GPUFrameSignal& signal) {
                queue.WriteBuffer(signalBuffer_, 0, &signal, sizeof(GPUFrameSignal));
            }

            // ─── Door Hash Computation ──────────────────────────────────────────────
            // Compute hash of door properties to detect any changes.
            // This ensures cache is invalidated if doors are:
            //   - Added/removed (count changes)
            //   - Repositioned (position changes)
            //   - Resized (width/height changes)
            //   - Shape changed (shape_type changes)
            //   - Target changed (target_id changes)
            //
            // Previous implementation only tracked count, missing repositioning/resizing.
            uint32_t compute_door_hash(const PortableDoor* doors, uint32_t count) const {
                if (count == 0) return 0;

                uint32_t hash = 0;
                for (uint32_t i = 0; i < count; ++i) {
                    const auto& d = doors[i].gpu;

                    // Hash position (XYZ)
                    uint32_t px = std::hash<float>()(d.position[0]);
                    uint32_t py = std::hash<float>()(d.position[1]);
                    uint32_t pz = std::hash<float>()(d.position[2]);
                    hash ^= px ^ (py << 1) ^ (pz << 2);

                    // Hash dimensions
                    uint32_t w = std::hash<float>()(d.width);
                    uint32_t h = std::hash<float>()(d.height);
                    uint32_t t = std::hash<float>()(d.thickness);
                    hash ^= w ^ (h << 3) ^ (t << 4);

                    // Hash shape and target
                    uint32_t shape = d.shape_type;
                    uint32_t target = d.target_id;
                    hash ^= (shape << 5) ^ (target << 8);

                    // Mix in index to detect reordering
                    hash ^= (i * 73);
                }

                return hash;
            }

            void upload_config(wgpu::Queue& queue) {
                queue.WriteBuffer(configBuffer_, 0, &config_, sizeof(config_));
            }

            void upload_doors(wgpu::Queue& queue, const PortableDoor* doors, uint32_t count) {
                GPUDoorHeader header{};
                header.count = count;
                queue.WriteBuffer(doorHeaderBuffer_, 0, &header, sizeof(header));

                if (count > 0) {
                    std::array<GPUPortableDoor, MAX_DOORS> gpuDoors{};
                    for (uint32_t i = 0; i < count; ++i) {
                        gpuDoors[i] = doors[i].gpu;
                    }
                    queue.WriteBuffer(doorArrayBuffer_, 0, gpuDoors.data(), count * sizeof(GPUPortableDoor));
                }

                // Mark field for update when doors change
                // Use hash of door content (position, size, shape, target)
                // not just count, to catch repositioning/resizing
                uint32_t doorHash = compute_door_hash(doors, count);
                if (doorHash != lastDoorHash_) {
                    needsFieldUpdate_ = true;
                    lastDoorHash_ = doorHash;
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

            void set_pawn_position(float x, float z, float heading, wgpu::Queue& queue) {
                GPUPawnState pawn{};
                pawn.pos[0] = x;
                pawn.pos[1] = 0.0f;
                pawn.pos[2] = z;
                pawn.heading = heading;

                float half_angle = heading * 0.5f;
                pawn.orientation[0] = 0.0f;
                pawn.orientation[1] = std::sin(half_angle);
                pawn.orientation[2] = 0.0f;
                pawn.orientation[3] = std::cos(half_angle);

                queue.WriteBuffer(pawnBuffer_, 0, &pawn, sizeof(pawn));
            }

            void mark_field_dirty() { needsFieldUpdate_ = true; }
            bool needs_field_update() const { return needsFieldUpdate_; }
            void clear_field_update_flag() { needsFieldUpdate_ = false; }

            uint32_t room_field_workgroups() const { return ROOM_FIELD_SIZE / 8; }

            // Layouts
            wgpu::BindGroupLayout compute_buffer_layout() const { return computeBufferLayout_; }
            wgpu::BindGroupLayout compute_texture_layout() const { return computeTextureLayout_; }
            wgpu::BindGroupLayout render_buffer_layout() const { return renderBufferLayout_; }
            wgpu::BindGroupLayout render_texture_layout() const { return renderTextureLayout_; }

            // Bind groups
            wgpu::BindGroup compute_buffer_bind_group() const { return computeBufferBindGroup_; }
            wgpu::BindGroup compute_texture_bind_group() const { return computeTextureBindGroup_; }
            wgpu::BindGroup render_buffer_bind_group() const { return renderBufferBindGroup_; }
            wgpu::BindGroup render_texture_bind_group() const { return renderTextureBindGroup_; }

        private:
            wgpu::Device device_;

            // Buffers
            wgpu::Buffer signalBuffer_;
            wgpu::Buffer configBuffer_;
            wgpu::Buffer pawnBuffer_;
            wgpu::Buffer cameraBuffer_;
            wgpu::Buffer doorHeaderBuffer_;
            wgpu::Buffer doorArrayBuffer_;
            wgpu::Buffer roomParamsBuffer_;

            // Textures
            wgpu::Texture roomFieldTexture_;
            wgpu::TextureView roomFieldWriteView_;
            wgpu::TextureView roomFieldReadView_;

            // Sampler
            wgpu::Sampler bilinearSampler_;

            // Layouts
            wgpu::BindGroupLayout computeBufferLayout_;
            wgpu::BindGroupLayout computeTextureLayout_;
            wgpu::BindGroupLayout renderBufferLayout_;
            wgpu::BindGroupLayout renderTextureLayout_;

            // Bind groups
            wgpu::BindGroup computeBufferBindGroup_;
            wgpu::BindGroup computeTextureBindGroup_;
            wgpu::BindGroup renderBufferBindGroup_;
            wgpu::BindGroup renderTextureBindGroup_;

            GPUDesignConfig config_{};
            bool needsFieldUpdate_ = true;
            uint32_t lastDoorHash_ = 0;

            bool createBuffers() {
                wgpu::BufferDescriptor desc{};
                desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;

                desc.label = "Signal";
                desc.size = sizeof(GPUFrameSignal);
                signalBuffer_ = device_.CreateBuffer(&desc);
                if (!signalBuffer_) return false;

                desc.label = "Config";
                desc.size = sizeof(GPUDesignConfig);
                configBuffer_ = device_.CreateBuffer(&desc);
                if (!configBuffer_) return false;

                desc.label = "Pawn";
                desc.size = sizeof(GPUPawnState);
                pawnBuffer_ = device_.CreateBuffer(&desc);
                if (!pawnBuffer_) return false;

                desc.label = "Camera";
                desc.size = sizeof(GPUCameraState);
                cameraBuffer_ = device_.CreateBuffer(&desc);
                if (!cameraBuffer_) return false;

                desc.label = "Door Header";
                desc.size = sizeof(GPUDoorHeader);
                doorHeaderBuffer_ = device_.CreateBuffer(&desc);
                if (!doorHeaderBuffer_) return false;

                desc.label = "Doors";
                desc.size = MAX_DOORS * sizeof(GPUPortableDoor);
                doorArrayBuffer_ = device_.CreateBuffer(&desc);
                if (!doorArrayBuffer_) return false;

                desc.label = "Room Params";
                desc.size = sizeof(GPURoomParams);
                roomParamsBuffer_ = device_.CreateBuffer(&desc);
                if (!roomParamsBuffer_) return false;

                return true;
            }

            bool createTextures() {
                wgpu::TextureDescriptor desc{};
                desc.label = "Room Field";
                desc.size = { ROOM_FIELD_SIZE, ROOM_FIELD_SIZE, 1 };
                desc.format = wgpu::TextureFormat::RGBA16Float;
                desc.usage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::TextureBinding;
                desc.dimension = wgpu::TextureDimension::e2D;
                desc.mipLevelCount = 1;
                desc.sampleCount = 1;

                roomFieldTexture_ = device_.CreateTexture(&desc);
                if (!roomFieldTexture_) return false;

                wgpu::TextureViewDescriptor viewDesc{};
                viewDesc.format = wgpu::TextureFormat::RGBA16Float;
                viewDesc.dimension = wgpu::TextureViewDimension::e2D;

                viewDesc.label = "Room Field Write";
                roomFieldWriteView_ = roomFieldTexture_.CreateView(&viewDesc);
                if (!roomFieldWriteView_) return false;

                viewDesc.label = "Room Field Read";
                roomFieldReadView_ = roomFieldTexture_.CreateView(&viewDesc);
                if (!roomFieldReadView_) return false;

                return true;
            }

            bool createSamplers() {
                wgpu::SamplerDescriptor desc{};
                desc.label = "Bilinear";
                desc.addressModeU = wgpu::AddressMode::ClampToEdge;
                desc.addressModeV = wgpu::AddressMode::ClampToEdge;
                desc.addressModeW = wgpu::AddressMode::ClampToEdge;
                desc.magFilter = wgpu::FilterMode::Linear;
                desc.minFilter = wgpu::FilterMode::Linear;
                desc.mipmapFilter = wgpu::MipmapFilterMode::Nearest;

                bilinearSampler_ = device_.CreateSampler(&desc);
                return bilinearSampler_ != nullptr;
            }

            bool createBindGroups() {
                // ═══ COMPUTE BUFFER LAYOUT (group 0) ═══
                // 0=signal, 1=config, 2=pawn(rw), 3=camera(rw), 4=doorHeader, 5=doors, 6=roomParams
                {
                    std::array<wgpu::BindGroupLayoutEntry, 7> entries{};

                    for (int i = 0; i < 7; ++i) {
                        entries[i].binding = i;
                        entries[i].visibility = wgpu::ShaderStage::Compute;
                        entries[i].buffer.type = (i == 2 || i == 3)
                            ? wgpu::BufferBindingType::Storage
                            : wgpu::BufferBindingType::ReadOnlyStorage;
                    }

                    wgpu::BindGroupLayoutDescriptor desc{};
                    desc.label = "Compute Buffer Layout";
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    computeBufferLayout_ = device_.CreateBindGroupLayout(&desc);
                    if (!computeBufferLayout_) return false;
                }

                // ═══ COMPUTE TEXTURE LAYOUT (group 1) ═══
                {
                    std::array<wgpu::BindGroupLayoutEntry, 1> entries{};

                    entries[0].binding = 0;
                    entries[0].visibility = wgpu::ShaderStage::Compute;
                    entries[0].storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
                    entries[0].storageTexture.format = wgpu::TextureFormat::RGBA16Float;
                    entries[0].storageTexture.viewDimension = wgpu::TextureViewDimension::e2D;

                    wgpu::BindGroupLayoutDescriptor desc{};
                    desc.label = "Compute Texture Layout";
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    computeTextureLayout_ = device_.CreateBindGroupLayout(&desc);
                    if (!computeTextureLayout_) return false;
                }

                // ═══ RENDER BUFFER LAYOUT (group 0) ═══
                // 10=signal, 11=pawn, 12=camera, 13=doorHeader, 14=doors, 15=roomParams
                {
                    std::array<wgpu::BindGroupLayoutEntry, 6> entries{};

                    for (int i = 0; i < 6; ++i) {
                        entries[i].binding = 10 + i;
                        entries[i].visibility = wgpu::ShaderStage::Fragment;
                        entries[i].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
                    }

                    wgpu::BindGroupLayoutDescriptor desc{};
                    desc.label = "Render Buffer Layout";
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    renderBufferLayout_ = device_.CreateBindGroupLayout(&desc);
                    if (!renderBufferLayout_) return false;
                }

                // ═══ RENDER TEXTURE LAYOUT (group 1) ═══
                {
                    std::array<wgpu::BindGroupLayoutEntry, 2> entries{};

                    entries[0].binding = 0;
                    entries[0].visibility = wgpu::ShaderStage::Fragment;
                    entries[0].texture.sampleType = wgpu::TextureSampleType::Float;
                    entries[0].texture.viewDimension = wgpu::TextureViewDimension::e2D;

                    entries[1].binding = 1;
                    entries[1].visibility = wgpu::ShaderStage::Fragment;
                    entries[1].sampler.type = wgpu::SamplerBindingType::Filtering;

                    wgpu::BindGroupLayoutDescriptor desc{};
                    desc.label = "Render Texture Layout";
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    renderTextureLayout_ = device_.CreateBindGroupLayout(&desc);
                    if (!renderTextureLayout_) return false;
                }

                // ═══ BIND GROUPS ═══

                // Compute buffer
                {
                    std::array<wgpu::BindGroupEntry, 7> entries{};

                    entries[0].binding = 0; entries[0].buffer = signalBuffer_; entries[0].size = sizeof(GPUFrameSignal);
                    entries[1].binding = 1; entries[1].buffer = configBuffer_; entries[1].size = sizeof(GPUDesignConfig);
                    entries[2].binding = 2; entries[2].buffer = pawnBuffer_; entries[2].size = sizeof(GPUPawnState);
                    entries[3].binding = 3; entries[3].buffer = cameraBuffer_; entries[3].size = sizeof(GPUCameraState);
                    entries[4].binding = 4; entries[4].buffer = doorHeaderBuffer_; entries[4].size = sizeof(GPUDoorHeader);
                    entries[5].binding = 5; entries[5].buffer = doorArrayBuffer_; entries[5].size = MAX_DOORS * sizeof(GPUPortableDoor);
                    entries[6].binding = 6; entries[6].buffer = roomParamsBuffer_; entries[6].size = sizeof(GPURoomParams);

                    wgpu::BindGroupDescriptor desc{};
                    desc.label = "Compute Buffer BindGroup";
                    desc.layout = computeBufferLayout_;
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    computeBufferBindGroup_ = device_.CreateBindGroup(&desc);
                    if (!computeBufferBindGroup_) return false;
                }

                // Compute texture
                {
                    std::array<wgpu::BindGroupEntry, 1> entries{};
                    entries[0].binding = 0;
                    entries[0].textureView = roomFieldWriteView_;

                    wgpu::BindGroupDescriptor desc{};
                    desc.label = "Compute Texture BindGroup";
                    desc.layout = computeTextureLayout_;
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    computeTextureBindGroup_ = device_.CreateBindGroup(&desc);
                    if (!computeTextureBindGroup_) return false;
                }

                // Render buffer
                {
                    std::array<wgpu::BindGroupEntry, 6> entries{};

                    entries[0].binding = 10; entries[0].buffer = signalBuffer_; entries[0].size = sizeof(GPUFrameSignal);
                    entries[1].binding = 11; entries[1].buffer = pawnBuffer_; entries[1].size = sizeof(GPUPawnState);
                    entries[2].binding = 12; entries[2].buffer = cameraBuffer_; entries[2].size = sizeof(GPUCameraState);
                    entries[3].binding = 13; entries[3].buffer = doorHeaderBuffer_; entries[3].size = sizeof(GPUDoorHeader);
                    entries[4].binding = 14; entries[4].buffer = doorArrayBuffer_; entries[4].size = MAX_DOORS * sizeof(GPUPortableDoor);
                    entries[5].binding = 15; entries[5].buffer = roomParamsBuffer_; entries[5].size = sizeof(GPURoomParams);

                    wgpu::BindGroupDescriptor desc{};
                    desc.label = "Render Buffer BindGroup";
                    desc.layout = renderBufferLayout_;
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    renderBufferBindGroup_ = device_.CreateBindGroup(&desc);
                    if (!renderBufferBindGroup_) return false;
                }

                // Render texture
                {
                    std::array<wgpu::BindGroupEntry, 2> entries{};
                    entries[0].binding = 0;
                    entries[0].textureView = roomFieldReadView_;
                    entries[1].binding = 1;
                    entries[1].sampler = bilinearSampler_;

                    wgpu::BindGroupDescriptor desc{};
                    desc.label = "Render Texture BindGroup";
                    desc.layout = renderTextureLayout_;
                    desc.entryCount = entries.size();
                    desc.entries = entries.data();

                    renderTextureBindGroup_ = device_.CreateBindGroup(&desc);
                    if (!renderTextureBindGroup_) return false;
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

                GPURoomParams roomParams{};
                roomParams.room_half_width = ROOM_HALF_WIDTH;
                roomParams.room_height = ROOM_HEIGHT;
                roomParams.room_half_depth = ROOM_HALF_DEPTH;
                roomParams.field_world_size = std::max(ROOM_HALF_WIDTH, ROOM_HALF_DEPTH) * 2.0f + 4.0f;
                queue.WriteBuffer(roomParamsBuffer_, 0, &roomParams, sizeof(roomParams));

                return true;
            }
        };


    } // namespace gallery_raymarch
} // namespace t7