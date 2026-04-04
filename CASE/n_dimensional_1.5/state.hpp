#pragma once

/**
 * N-DIMENSIONAL_2 CARTRIDGE — GPU State Management
 * =================================================
 * 
 * Extends n_dimensional with multiple stimulus sources (pawn + sphere).
 * 
 * DIMENSIONALITY:
 *   0D — Buffers (scalar/struct): ground, pawn, camera, sphere, config
 *   1D — Buffers (array): trajectories (16 slots)
 *   2D — Buffers (array): trajectory_field (64×64)
 *   2D — Textures: height field, tile state
 * 
 * BIND GROUPS:
 *   Group 0: Entity state (buffers)
 *   Group 1: 2D textures
 * 
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include "analysis/analysis_signal.hpp"
#include <webgpu/webgpu_cpp.h>
#include <cstring>
#include <array>
#include <vector>

namespace t7 {
namespace n_dimensional_2 {


// ═══════════════════════════════════════════════════════════════════════════════
// §1 IDLE STATE — The design at rest
// ═══════════════════════════════════════════════════════════════════════════════
//
// These values define what you see when all couplings are muted.
// The instrument before it is played.
//
// Must match world.wgsl §1.2

namespace Idle {
    // ─── [FIELD:ground] ─────────────────────────────────────────────────────
    constexpr float AMPLITUDE_SCALE = 1.0f;
    constexpr float MAX_AMPLITUDE = 2.0f;
    constexpr float SIZE = 100.0f;
    constexpr float LIPSCHITZ_FACTOR = 1.0f;
    
    // ─── [ENTITY:pawn] ──────────────────────────────────────────────────────
    constexpr float PAWN_POS_X = 0.0f;
    constexpr float PAWN_POS_Y = 0.0f;
    constexpr float PAWN_POS_Z = 0.0f;
    constexpr float PAWN_HEADING = 0.0f;
    constexpr float PAWN_QUAT_X = 0.0f;
    constexpr float PAWN_QUAT_Y = 0.0f;
    constexpr float PAWN_QUAT_Z = 0.0f;
    constexpr float PAWN_QUAT_W = 1.0f;
    
    // ─── [ENTITY:camera] ────────────────────────────────────────────────────
    constexpr float CAMERA_POS_X = 0.0f;
    constexpr float CAMERA_POS_Y = 15.0f;
    constexpr float CAMERA_POS_Z = 30.0f;
    constexpr float CAMERA_AZIMUTH = 0.0f;
    constexpr float CAMERA_ELEVATION = 0.4f;
    constexpr float CAMERA_DISTANCE = 30.0f;
    constexpr float CAMERA_PAN_X = 0.0f;
    constexpr float CAMERA_PAN_Y = 0.0f;
    
    // ─── [ENTITY:sphere] ───────────────────────────────────────────────────────
    // Sphere orbits at fixed height, position computed from time
    constexpr float SPHERE_RADIUS = 2.5f;           // 5× pawn base diameter
    constexpr float SPHERE_ORBIT_RADIUS = 25.0f;
    constexpr float SPHERE_ORBIT_SPEED = 0.3f;      // rad/sec
    constexpr float SPHERE_HOVER_HEIGHT = 8.0f;
    constexpr float SPHERE_INFLUENCE_RADIUS = 12.0f;
    
    // ─── [TRAJECTORY:amplitude] ─────────────────────────────────────────────
    constexpr float TRAJECTORY_VALUE = 1.0f;
    constexpr float TRAJECTORY_VELOCITY = 0.0f;
    
    // ─── [TRAJECTORY_FIELD:2d] ────────────────────────────────────────────────
    // 2D trajectory field stores displacement from idle (0 = at rest)
    constexpr float TRAJECTORY_FIELD_VALUE = 0.0f;
    constexpr float TRAJECTORY_FIELD_VELOCITY = 0.0f;
    
    // ─── [CONFIG:design] ────────────────────────────────────────────────────
    constexpr float WAVE_TIME_SCALE = 1.0f;
    constexpr float PAWN_SPEED = 15.0f;
    constexpr float CAMERA_SENSITIVITY = 0.005f;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §2 COUPLING BITS — Muting control
// ═══════════════════════════════════════════════════════════════════════════════
//
// Each bit controls one data flow. Muting a coupling breaks that wire.
// Must match world.wgsl §1 CONFIG.
//
// ┌─────┬────────────────────────────────┬───────────────────────────────────────┐
// │ Bit │ Coupling                       │ Flow                                  │
// ├─────┼────────────────────────────────┼───────────────────────────────────────┤
// │  0  │ POLYPHONY_TO_AMPLITUDE         │ signal.poly → terrain.amplitude       │
// │  1  │ TERRAIN_TO_PAWN_Y              │ terrain.h(xz) → pawn.y                │
// │  2  │ TERRAIN_TO_PAWN_TILT           │ terrain.n(xz) → pawn.orientation      │
// │  3  │ PAWN_TO_CAMERA_TARGET          │ pawn.pos → camera.target              │
// │  4  │ INPUT_MOVES_PAWN               │ arrows → pawn.velocity                │
// │  5  │ INPUT_ORBITS_CAMERA            │ mouse.drag → camera.az/el             │
// │  6  │ INPUT_ZOOMS_CAMERA             │ mouse.scroll → camera.distance        │
// │  7  │ PAWN_TO_FIELD_COLOR            │ pawn.proximity → field.color_offset   │
// │  8  │ SPHERE_TO_FIELD_COLOR          │ sphere.proximity → field.color_offset │
// └─────┴────────────────────────────────┴───────────────────────────────────────┘

namespace Coupling {
    constexpr uint32_t POLYPHONY_TO_AMPLITUDE  = 1u << 0;
    constexpr uint32_t TERRAIN_TO_PAWN_Y       = 1u << 1;
    constexpr uint32_t TERRAIN_TO_PAWN_TILT    = 1u << 2;
    constexpr uint32_t PAWN_TO_CAMERA_TARGET   = 1u << 3;
    constexpr uint32_t INPUT_MOVES_PAWN        = 1u << 4;
    constexpr uint32_t INPUT_ORBITS_CAMERA     = 1u << 5;
    constexpr uint32_t INPUT_ZOOMS_CAMERA      = 1u << 6;
    constexpr uint32_t PAWN_TO_FIELD_COLOR     = 1u << 7;
    constexpr uint32_t SPHERE_TO_FIELD_COLOR   = 1u << 8;
    
    // Groups for convenience
    constexpr uint32_t ALL      = 0x1FFu;
    constexpr uint32_t NONE     = 0u;
    constexpr uint32_t SIGNAL   = POLYPHONY_TO_AMPLITUDE;
    constexpr uint32_t TERRAIN  = TERRAIN_TO_PAWN_Y | TERRAIN_TO_PAWN_TILT;
    constexpr uint32_t INPUT    = INPUT_MOVES_PAWN | INPUT_ORBITS_CAMERA | INPUT_ZOOMS_CAMERA;
    constexpr uint32_t CAMERA   = PAWN_TO_CAMERA_TARGET | INPUT_ORBITS_CAMERA | INPUT_ZOOMS_CAMERA;
    constexpr uint32_t FIELD    = PAWN_TO_FIELD_COLOR | SPHERE_TO_FIELD_COLOR;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §3 GPU STRUCTURES — Must match world.wgsl exactly
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Signal (CPU → GPU each frame) ──────────────────────────────────────────

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

// ─── Design Config (muting + tuning) ────────────────────────────────────────

struct alignas(16) GPUDesignConfig {
    uint32_t mute_dynamics_0d;   // Freeze entity physics
    uint32_t mute_dynamics_2d;   // Freeze field evolution
    uint32_t mute_signal;        // Zero all musical input
    uint32_t mute_couplings;     // Bitfield: which wires are cut
    float wave_time_scale;       // Speed of wave animation
    float pawn_speed;            // Movement speed override
    float camera_sensitivity;    // Mouse sensitivity override
    float _pad0;
};

// ─── 1D State (trajectories) ────────────────────────────────────────────────

struct alignas(16) GPUTrajectory {
    float value;
    float velocity;
    float _pad0;
    float _pad1;
};

// ─── 0D State (entities) ────────────────────────────────────────────────────

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

struct alignas(16) GPUSphereState {
    float pos[3];
    float radius;
    float influence_radius;
    float _pad0;
    float _pad1;
    float _pad2;
};

// ─── Size verification ──────────────────────────────────────────────────────

static_assert(sizeof(GPUFrameSignal) == 304, "GPUFrameSignal must be 304 bytes");
static_assert(sizeof(GPUDesignConfig) == 32, "GPUDesignConfig must be 32 bytes");
static_assert(sizeof(GPUTrajectory) == 16, "GPUTrajectory must be 16 bytes");
static_assert(sizeof(GPUGroundState) == 16, "GPUGroundState must be 16 bytes");
static_assert(sizeof(GPUPawnState) == 32, "GPUPawnState must be 32 bytes");
static_assert(sizeof(GPUCameraState) == 32, "GPUCameraState must be 32 bytes");
static_assert(sizeof(GPUSphereState) == 32, "GPUSphereState must be 32 bytes");


// ═══════════════════════════════════════════════════════════════════════════════
// §4 CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════════

constexpr int MAX_TRAJECTORIES = 16;
constexpr uint32_t HEIGHT_FIELD_SIZE = 256;
constexpr uint32_t TILE_TEXTURE_SIZE = 64;

// ─── Trajectory Field (2D) ──────────────────────────────────────────────────
constexpr uint32_t TRAJECTORY_FIELD_SIZE = 64;
constexpr uint32_t TRAJECTORY_FIELD_COUNT = TRAJECTORY_FIELD_SIZE * TRAJECTORY_FIELD_SIZE;

// ─── Terrain Appearance ─────────────────────────────────────────────────────
constexpr float TERRAIN_BASE_COLOR_R = 0.55f;
constexpr float TERRAIN_BASE_COLOR_G = 0.45f;
constexpr float TERRAIN_BASE_COLOR_B = 0.35f;

// ─── Pawn Influence (stimulus parameters) ───────────────────────────────────
constexpr float PAWN_INFLUENCE_RADIUS = 8.0f;

// ─── Field Dynamics ─────────────────────────────────────────────────────────
constexpr float FIELD_ATTACK_STIFFNESS = 12.0f;
constexpr float FIELD_ATTACK_DAMPING = 0.7f;
constexpr float FIELD_RELEASE_RATE = 1.5f;
constexpr float FIELD_MAX_OFFSET = 0.5f;

// ─── Color Shift (how trajectory displaces from base) ───────────────────────
constexpr float COLOR_SHIFT_R = 0.4f;
constexpr float COLOR_SHIFT_G = 0.2f;
constexpr float COLOR_SHIFT_B = 0.5f;

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
        
        return true;
    }
    
    // ─── Signal Upload ──────────────────────────────────────────────────────
    
    void upload_signal(wgpu::Queue& queue, const GPUFrameSignal& signal) {
        queue.WriteBuffer(signalBuffer_, 0, &signal, sizeof(GPUFrameSignal));
    }
    
    // ─── Config Upload ──────────────────────────────────────────────────────
    
    void upload_config(wgpu::Queue& queue) {
        queue.WriteBuffer(configBuffer_, 0, &config_, sizeof(GPUDesignConfig));
    }
    
    // ─── Design Mode Control ────────────────────────────────────────────────
    //
    // These methods manipulate the config_ member.
    // Call upload_config() after changes to push to GPU.
    
    void enter_design_mode() {
        config_.mute_signal = 1;
        config_.mute_couplings = Coupling::ALL;
    }
    
    void enter_performance_mode() {
        config_.mute_signal = 0;
        config_.mute_couplings = Coupling::NONE;
    }
    
    void set_mute_signal(bool muted) {
        config_.mute_signal = muted ? 1 : 0;
    }
    
    void set_mute_dynamics_0d(bool muted) {
        config_.mute_dynamics_0d = muted ? 1 : 0;
    }
    
    void set_mute_dynamics_2d(bool muted) {
        config_.mute_dynamics_2d = muted ? 1 : 0;
    }
    
    void set_mute_coupling(uint32_t coupling_bit, bool muted) {
        if (muted) {
            config_.mute_couplings |= coupling_bit;
        } else {
            config_.mute_couplings &= ~coupling_bit;
        }
    }
    
    void set_mute_couplings(uint32_t mask) {
        config_.mute_couplings = mask;
    }
    
    // ─── Design Parameters ──────────────────────────────────────────────────
    
    void set_wave_time_scale(float scale) {
        config_.wave_time_scale = scale;
    }
    
    void set_pawn_speed(float speed) {
        config_.pawn_speed = speed;
    }
    
    void set_camera_sensitivity(float sensitivity) {
        config_.camera_sensitivity = sensitivity;
    }
    
    GPUDesignConfig& config() { return config_; }
    const GPUDesignConfig& config() const { return config_; }
    
    // ─── Bind Groups ────────────────────────────────────────────────────────
    
    wgpu::BindGroup compute_entity_bind_group() const { return computeEntityBindGroup_; }
    wgpu::BindGroup render_entity_bind_group() const { return renderEntityBindGroup_; }
    wgpu::BindGroupLayout compute_entity_bind_group_layout() const { return computeEntityBindGroupLayout_; }
    wgpu::BindGroupLayout render_entity_bind_group_layout() const { return renderEntityBindGroupLayout_; }
    
    wgpu::BindGroup compute_texture_bind_group() const { return computeTextureBindGroup_; }
    wgpu::BindGroup render_texture_bind_group() const { return renderTextureBindGroup_; }
    wgpu::BindGroupLayout compute_texture_bind_group_layout() const { return computeTextureBindGroupLayout_; }
    wgpu::BindGroupLayout render_texture_bind_group_layout() const { return renderTextureBindGroupLayout_; }
    
    // ─── Dispatch Dimensions ────────────────────────────────────────────────
    
    static constexpr uint32_t height_field_workgroups() {
        return HEIGHT_FIELD_SIZE / 8;
    }
    
    static constexpr uint32_t tile_texture_workgroups() {
        return TILE_TEXTURE_SIZE / 8;
    }
    
private:
    wgpu::Device device_;
    GPUDesignConfig config_{};
    
    // ─── Buffers (0D and 1D state) ──────────────────────────────────────────
    wgpu::Buffer signalBuffer_;
    wgpu::Buffer configBuffer_;
    wgpu::Buffer groundBuffer_;
    wgpu::Buffer pawnBuffer_;
    wgpu::Buffer cameraBuffer_;
    wgpu::Buffer sphereBuffer_;
    wgpu::Buffer trajectoriesBuffer_;
    
    // ─── Buffers (2D state) ──────────────────────────────────────────────────
    wgpu::Buffer trajectoryFieldBuffer_;
    
    // ─── Textures (2D state) ────────────────────────────────────────────────
    wgpu::Texture heightFieldTexture_;
    wgpu::TextureView heightFieldWriteView_;
    wgpu::TextureView heightFieldReadView_;
    
    wgpu::Texture tileStateTexture_;
    wgpu::TextureView tileStateWriteView_;
    wgpu::TextureView tileStateReadView_;
    
    // ─── Samplers ───────────────────────────────────────────────────────────
    wgpu::Sampler bilinearSampler_;
    wgpu::Sampler nearestSampler_;
    
    // ─── Bind Groups ────────────────────────────────────────────────────────
    wgpu::BindGroupLayout computeEntityBindGroupLayout_;
    wgpu::BindGroupLayout renderEntityBindGroupLayout_;
    wgpu::BindGroupLayout computeTextureBindGroupLayout_;
    wgpu::BindGroupLayout renderTextureBindGroupLayout_;
    
    wgpu::BindGroup computeEntityBindGroup_;
    wgpu::BindGroup renderEntityBindGroup_;
    wgpu::BindGroup computeTextureBindGroup_;
    wgpu::BindGroup renderTextureBindGroup_;
    
    // ─────────────────────────────────────────────────────────────────────────
    // CREATION METHODS
    // ─────────────────────────────────────────────────────────────────────────
    
    bool createBuffers() {
        auto createBuffer = [&](const char* label, size_t size) -> wgpu::Buffer {
            wgpu::BufferDescriptor desc{};
            desc.label = label;
            desc.size = size;
            desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            return device_.CreateBuffer(&desc);
        };
        
        signalBuffer_ = createBuffer("FrameSignal", sizeof(GPUFrameSignal));
        configBuffer_ = createBuffer("DesignConfig", sizeof(GPUDesignConfig));
        groundBuffer_ = createBuffer("GroundState", sizeof(GPUGroundState));
        pawnBuffer_ = createBuffer("PawnState", sizeof(GPUPawnState));
        cameraBuffer_ = createBuffer("CameraState", sizeof(GPUCameraState));
        sphereBuffer_ = createBuffer("SphereState", sizeof(GPUSphereState));
        trajectoriesBuffer_ = createBuffer("Trajectories", sizeof(GPUTrajectory) * MAX_TRAJECTORIES);
        trajectoryFieldBuffer_ = createBuffer("TrajectoryField", sizeof(GPUTrajectory) * TRAJECTORY_FIELD_COUNT);
        
        return signalBuffer_ && configBuffer_ && groundBuffer_ && 
               pawnBuffer_ && cameraBuffer_ && sphereBuffer_ && 
               trajectoriesBuffer_ && trajectoryFieldBuffer_;
    }
    
    bool createTextures() {
        // Height field: RGBA16Float, 256×256
        {
            wgpu::TextureDescriptor desc{};
            desc.label = "HeightField";
            desc.size = { HEIGHT_FIELD_SIZE, HEIGHT_FIELD_SIZE, 1 };
            desc.format = wgpu::TextureFormat::RGBA16Float;
            desc.usage = wgpu::TextureUsage::StorageBinding | 
                         wgpu::TextureUsage::TextureBinding;
            desc.dimension = wgpu::TextureDimension::e2D;
            desc.mipLevelCount = 1;
            desc.sampleCount = 1;
            
            heightFieldTexture_ = device_.CreateTexture(&desc);
            if (!heightFieldTexture_) return false;
            
            wgpu::TextureViewDescriptor viewDesc{};
            viewDesc.format = wgpu::TextureFormat::RGBA16Float;
            viewDesc.dimension = wgpu::TextureViewDimension::e2D;
            viewDesc.mipLevelCount = 1;
            viewDesc.arrayLayerCount = 1;
            
            heightFieldWriteView_ = heightFieldTexture_.CreateView(&viewDesc);
            heightFieldReadView_ = heightFieldTexture_.CreateView(&viewDesc);
            
            if (!heightFieldWriteView_ || !heightFieldReadView_) return false;
        }
        
        // Tile state: RGBA8Unorm, 64×64
        {
            wgpu::TextureDescriptor desc{};
            desc.label = "TileState";
            desc.size = { TILE_TEXTURE_SIZE, TILE_TEXTURE_SIZE, 1 };
            desc.format = wgpu::TextureFormat::RGBA8Unorm;
            desc.usage = wgpu::TextureUsage::StorageBinding | 
                         wgpu::TextureUsage::TextureBinding;
            desc.dimension = wgpu::TextureDimension::e2D;
            desc.mipLevelCount = 1;
            desc.sampleCount = 1;
            
            tileStateTexture_ = device_.CreateTexture(&desc);
            if (!tileStateTexture_) return false;
            
            wgpu::TextureViewDescriptor viewDesc{};
            viewDesc.format = wgpu::TextureFormat::RGBA8Unorm;
            viewDesc.dimension = wgpu::TextureViewDimension::e2D;
            viewDesc.mipLevelCount = 1;
            viewDesc.arrayLayerCount = 1;
            
            tileStateWriteView_ = tileStateTexture_.CreateView(&viewDesc);
            tileStateReadView_ = tileStateTexture_.CreateView(&viewDesc);
            
            if (!tileStateWriteView_ || !tileStateReadView_) return false;
        }
        
        return true;
    }
    
    bool createSamplers() {
        // Bilinear sampler for height field
        {
            wgpu::SamplerDescriptor desc{};
            desc.label = "Bilinear";
            desc.magFilter = wgpu::FilterMode::Linear;
            desc.minFilter = wgpu::FilterMode::Linear;
            desc.addressModeU = wgpu::AddressMode::ClampToEdge;
            desc.addressModeV = wgpu::AddressMode::ClampToEdge;
            
            bilinearSampler_ = device_.CreateSampler(&desc);
            if (!bilinearSampler_) return false;
        }
        
        // Nearest sampler for tiles
        {
            wgpu::SamplerDescriptor desc{};
            desc.label = "Nearest";
            desc.magFilter = wgpu::FilterMode::Nearest;
            desc.minFilter = wgpu::FilterMode::Nearest;
            desc.addressModeU = wgpu::AddressMode::ClampToEdge;
            desc.addressModeV = wgpu::AddressMode::ClampToEdge;
            
            nearestSampler_ = device_.CreateSampler(&desc);
            if (!nearestSampler_) return false;
        }
        
        return true;
    }
    
    bool createBindGroups() {
        // ═══════════════════════════════════════════════════════════════════
        // GROUP 0: Entity State (Buffers)
        // ═══════════════════════════════════════════════════════════════════
        
        // Compute layout: bindings 0-7 (signal, config, ground, pawn, camera, sphere, trajectories, trajectory_field)
        {
            std::array<wgpu::BindGroupLayoutEntry, 8> entries{};
            
            // 0: signal (read-only)
            entries[0].binding = 0;
            entries[0].visibility = wgpu::ShaderStage::Compute;
            entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            // 1: config (read-only)
            entries[1].binding = 1;
            entries[1].visibility = wgpu::ShaderStage::Compute;
            entries[1].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            // 2-7: state buffers (read-write)
            for (int i = 2; i < 8; ++i) {
                entries[i].binding = i;
                entries[i].visibility = wgpu::ShaderStage::Compute;
                entries[i].buffer.type = wgpu::BufferBindingType::Storage;
            }
            
            wgpu::BindGroupLayoutDescriptor desc{};
            desc.label = "Compute Entity Layout";
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            computeEntityBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
            if (!computeEntityBindGroupLayout_) return false;
        }
        
        // Render layout: bindings 10-14 (read-only)
        {
            std::array<wgpu::BindGroupLayoutEntry, 5> entries{};
            
            for (int i = 0; i < 5; ++i) {
                entries[i].binding = 10 + i;
                entries[i].visibility = wgpu::ShaderStage::Fragment;
                entries[i].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            }
            
            wgpu::BindGroupLayoutDescriptor desc{};
            desc.label = "Render Entity Layout";
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            renderEntityBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
            if (!renderEntityBindGroupLayout_) return false;
        }
        
        // Compute entity bind group
        {
            wgpu::Buffer buffers[] = { 
                signalBuffer_, configBuffer_, groundBuffer_, 
                pawnBuffer_, cameraBuffer_, sphereBuffer_,
                trajectoriesBuffer_, trajectoryFieldBuffer_
            };
            size_t sizes[] = { 
                sizeof(GPUFrameSignal), sizeof(GPUDesignConfig),
                sizeof(GPUGroundState), sizeof(GPUPawnState), 
                sizeof(GPUCameraState), sizeof(GPUSphereState),
                sizeof(GPUTrajectory) * MAX_TRAJECTORIES,
                sizeof(GPUTrajectory) * TRAJECTORY_FIELD_COUNT
            };
            
            std::array<wgpu::BindGroupEntry, 8> entries{};
            for (int i = 0; i < 8; ++i) {
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
            if (!computeEntityBindGroup_) return false;
        }
        
        // Render entity bind group
        {
            wgpu::Buffer buffers[] = { signalBuffer_, groundBuffer_, pawnBuffer_, cameraBuffer_, sphereBuffer_ };
            size_t sizes[] = { sizeof(GPUFrameSignal), sizeof(GPUGroundState), 
                              sizeof(GPUPawnState), sizeof(GPUCameraState), sizeof(GPUSphereState) };
            
            std::array<wgpu::BindGroupEntry, 5> entries{};
            for (int i = 0; i < 5; ++i) {
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
            if (!renderEntityBindGroup_) return false;
        }
        
        // ═══════════════════════════════════════════════════════════════════
        // GROUP 1: 2D Textures
        // ═══════════════════════════════════════════════════════════════════
        
        // Compute texture layout: bindings 0-1 (storage write)
        {
            std::array<wgpu::BindGroupLayoutEntry, 2> entries{};
            
            entries[0].binding = 0;
            entries[0].visibility = wgpu::ShaderStage::Compute;
            entries[0].storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
            entries[0].storageTexture.format = wgpu::TextureFormat::RGBA16Float;
            entries[0].storageTexture.viewDimension = wgpu::TextureViewDimension::e2D;
            
            entries[1].binding = 1;
            entries[1].visibility = wgpu::ShaderStage::Compute;
            entries[1].storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
            entries[1].storageTexture.format = wgpu::TextureFormat::RGBA8Unorm;
            entries[1].storageTexture.viewDimension = wgpu::TextureViewDimension::e2D;
            
            wgpu::BindGroupLayoutDescriptor desc{};
            desc.label = "Compute Texture Layout";
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            computeTextureBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
            if (!computeTextureBindGroupLayout_) return false;
        }
        
        // Render texture layout: bindings 10-13 (texture read + samplers)
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
            if (!renderTextureBindGroupLayout_) return false;
        }
        
        // Compute texture bind group
        {
            std::array<wgpu::BindGroupEntry, 2> entries{};
            
            entries[0].binding = 0;
            entries[0].textureView = heightFieldWriteView_;
            
            entries[1].binding = 1;
            entries[1].textureView = tileStateWriteView_;
            
            wgpu::BindGroupDescriptor desc{};
            desc.label = "Compute Texture BindGroup";
            desc.layout = computeTextureBindGroupLayout_;
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            computeTextureBindGroup_ = device_.CreateBindGroup(&desc);
            if (!computeTextureBindGroup_) return false;
        }
        
        // Render texture bind group
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
            if (!renderTextureBindGroup_) return false;
        }
        
        return true;
    }
    
    bool initializeState() {
        wgpu::Queue queue = device_.GetQueue();
        
        // ─── Initialize config to performance mode ──────────────────────────
        config_.mute_dynamics_0d = 0;
        config_.mute_dynamics_2d = 0;
        config_.mute_signal = 0;
        config_.mute_couplings = Coupling::NONE;
        config_.wave_time_scale = Idle::WAVE_TIME_SCALE;
        config_.pawn_speed = Idle::PAWN_SPEED;
        config_.camera_sensitivity = Idle::CAMERA_SENSITIVITY;
        queue.WriteBuffer(configBuffer_, 0, &config_, sizeof(config_));
        
        // ─── Initialize ground to idle state ────────────────────────────────
        GPUGroundState ground{};
        ground.amplitude_scale = Idle::AMPLITUDE_SCALE;
        ground.max_amplitude = Idle::MAX_AMPLITUDE;
        ground.size = Idle::SIZE;
        ground.lipschitz_factor = Idle::LIPSCHITZ_FACTOR;
        queue.WriteBuffer(groundBuffer_, 0, &ground, sizeof(ground));
        
        // ─── Initialize pawn to idle state ──────────────────────────────────
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
        
        // ─── Initialize camera to idle state ────────────────────────────────
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
        
        // ─── Initialize sphere to idle state ─────────────────────────────────
        // Position is computed GPU-side from time, but we initialize it at orbit start
        GPUSphereState sphere{};
        sphere.pos[0] = Idle::SPHERE_ORBIT_RADIUS;  // cos(0) * orbit_radius
        sphere.pos[1] = Idle::SPHERE_HOVER_HEIGHT;
        sphere.pos[2] = 0.0f;                        // sin(0) * orbit_radius
        sphere.radius = Idle::SPHERE_RADIUS;
        sphere.influence_radius = Idle::SPHERE_INFLUENCE_RADIUS;
        queue.WriteBuffer(sphereBuffer_, 0, &sphere, sizeof(sphere));
        
        // ─── Initialize trajectories to idle state ──────────────────────────
        GPUTrajectory trajectories[MAX_TRAJECTORIES]{};
        for (int i = 0; i < MAX_TRAJECTORIES; ++i) {
            trajectories[i].value = Idle::TRAJECTORY_VALUE;
            trajectories[i].velocity = Idle::TRAJECTORY_VELOCITY;
        }
        queue.WriteBuffer(trajectoriesBuffer_, 0, trajectories, sizeof(trajectories));
        
        // ─── Initialize trajectory field to idle (zero displacement) ─────────
        // Field idle = 0 (no displacement from base color)
        std::vector<GPUTrajectory> trajectoryField(TRAJECTORY_FIELD_COUNT);
        for (uint32_t i = 0; i < TRAJECTORY_FIELD_COUNT; ++i) {
            trajectoryField[i].value = Idle::TRAJECTORY_FIELD_VALUE;
            trajectoryField[i].velocity = Idle::TRAJECTORY_FIELD_VELOCITY;
        }
        queue.WriteBuffer(trajectoryFieldBuffer_, 0, trajectoryField.data(), 
                          sizeof(GPUTrajectory) * TRAJECTORY_FIELD_COUNT);
        
        return true;
    }
};

} // namespace n_dimensional_2
} // namespace t7
