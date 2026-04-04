#pragma once

/**
 * N-DIMENSIONAL_3 CARTRIDGE — GPU State Management
 * =================================================
 *
 * Trajectory-driven cell grid. Each cell holds a color trajectory that
 * springs toward goals or releases to idle.
 *
 * CORE PRINCIPLE:
 *   Every parameter is always under trajectory. The viewer sees value,
 *   which emerges from the dynamics. Goals are set by couplings (musical,
 *   entity, pattern). Idle is the artist's deliberate choice.
 *
 * DIMENSIONALITY:
 *   0D — Buffers (scalar/struct): ground, pawn, camera, config
 *   1D — Buffers (array): trajectories (16 slots)
 *   2D — Buffers (array): cells (64×64 CellState)
 *   2D — Textures: height field, tile state (renderer observes)
 *
 * BIND GROUPS:
 *   Group 0: Entity state (buffers) + cell buffer
 *   Group 1: 2D textures
 *
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include "analysis/analysis_signal.hpp"
#include <webgpu/webgpu_cpp.h>
#include <cstring>
#include <array>
#include <vector>
#include <cmath>

namespace t7 {
namespace n_dimensional_3 {


// ═══════════════════════════════════════════════════════════════════════════════
// §1 CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════════

constexpr uint32_t CELL_GRID_SIZE = 64;
constexpr uint32_t CELL_COUNT = CELL_GRID_SIZE * CELL_GRID_SIZE;  // 4096

// Trajectory field (for entity proximity effects)
constexpr uint32_t TRAJECTORY_FIELD_SIZE = 64;
constexpr uint32_t TRAJECTORY_FIELD_COUNT = TRAJECTORY_FIELD_SIZE * TRAJECTORY_FIELD_SIZE;  // 4096

constexpr uint32_t HEIGHT_FIELD_SIZE = 256;
constexpr uint32_t TILE_TEXTURE_SIZE = 64;  // Matches cell grid

constexpr int MAX_TRAJECTORIES = 16;

// Chart extent (world units)
constexpr float CHART_EXTENT = 100.0f;


// ═══════════════════════════════════════════════════════════════════════════════
// §2 IDLE STATE — Artist's deliberate choices
// ═══════════════════════════════════════════════════════════════════════════════

namespace Idle {
    // ─── Terrain ─────────────────────────────────────────────────────────────
    constexpr float AMPLITUDE_SCALE = 1.0f;
    constexpr float MAX_AMPLITUDE = 2.0f;
    constexpr float SIZE = 100.0f;
    constexpr float LIPSCHITZ_FACTOR = 1.0f;

    // ─── Cell Colors (for reference — now computed in shader) ──────────────────
    // Base terrain color (used by idle_color function in shader)
    constexpr float BASE_R = 0.55f;
    constexpr float BASE_G = 0.45f;
    constexpr float BASE_B = 0.35f;
    
    // Checkerboard contrast (now computed in shader)
    constexpr float CHECKER_LIGHT = 1.15f;
    constexpr float CHECKER_DARK = 0.85f;

    // ─── Pawn ────────────────────────────────────────────────────────────────
    constexpr float PAWN_POS_X = 0.0f;
    constexpr float PAWN_POS_Y = 0.0f;
    constexpr float PAWN_POS_Z = 0.0f;
    constexpr float PAWN_HEADING = 0.0f;

    // ─── Camera ──────────────────────────────────────────────────────────────
    constexpr float CAMERA_POS_X = 0.0f;
    constexpr float CAMERA_POS_Y = 15.0f;
    constexpr float CAMERA_POS_Z = 30.0f;
    constexpr float CAMERA_AZIMUTH = 0.0f;
    constexpr float CAMERA_ELEVATION = 0.4f;
    constexpr float CAMERA_DISTANCE = 30.0f;

    // ─── Sphere ──────────────────────────────────────────────────────────────
    constexpr float SPHERE_RADIUS = 2.5f;
    constexpr float SPHERE_ORBIT_RADIUS = 25.0f;
    constexpr float SPHERE_ORBIT_SPEED = 0.3f;
    constexpr float SPHERE_HOVER_HEIGHT = 8.0f;
    constexpr float SPHERE_INFLUENCE_RADIUS = 12.0f;

    // ─── Trajectory Dynamics ─────────────────────────────────────────────────
    constexpr float TRAJECTORY_VALUE = 1.0f;
    constexpr float TRAJECTORY_VELOCITY = 0.0f;
    
    // 2D field offset (stimulus: entity proximity)
    constexpr float TRAJECTORY_FIELD_VALUE = 0.0f;
    constexpr float TRAJECTORY_FIELD_VELOCITY = 0.0f;

    // ─── Config ──────────────────────────────────────────────────────────────
    constexpr float WAVE_TIME_SCALE = 1.0f;
    constexpr float PAWN_SPEED = 15.0f;
    constexpr float CAMERA_SENSITIVITY = 0.005f;
    constexpr float ACTIVE_CELL_SIZE = 64.0f;  // Default: full resolution
}


// ═══════════════════════════════════════════════════════════════════════════════
// §3 COUPLING BITS — Muting control
// ═══════════════════════════════════════════════════════════════════════════════

namespace Coupling {
    // ─── Stimulus Couplings (drive trajectories) ──────────────────────────────
    constexpr uint32_t POLYPHONY_TO_AMPLITUDE     = 1u << 0;
    constexpr uint32_t PAWN_TO_FIELD_COLOR        = 1u << 7;
    constexpr uint32_t SPHERE_TO_FIELD_COLOR      = 1u << 8;
    constexpr uint32_t POLYPHONY_TO_CELL_COLOR    = 1u << 9;
    constexpr uint32_t PAWN_TO_CELL_COLOR         = 1u << 10;
    constexpr uint32_t SPHERE_TO_CELL_COLOR       = 1u << 11;
    constexpr uint32_t POLYPHONY_TO_SPHERE_COLOR  = 1u << 12;  // NEW: music → sphere appearance
    constexpr uint32_t SPHERE_TO_TERRAIN_TINT     = 1u << 13;  // NEW: sphere orbit → world color

    // ─── Derived Couplings ────────────────────────────────────────────────────
    constexpr uint32_t TERRAIN_TO_PAWN_Y          = 1u << 1;
    constexpr uint32_t TERRAIN_TO_PAWN_TILT       = 1u << 2;
    constexpr uint32_t PAWN_TO_CAMERA_TARGET      = 1u << 3;

    // ─── Accumulated Couplings ────────────────────────────────────────────────
    constexpr uint32_t INPUT_MOVES_PAWN           = 1u << 4;
    constexpr uint32_t INPUT_ORBITS_CAMERA        = 1u << 5;
    constexpr uint32_t INPUT_ZOOMS_CAMERA         = 1u << 6;

    // ─── Groups for Convenience ───────────────────────────────────────────────
    constexpr uint32_t ALL = 0x3FFFu;  // 14 bits now
    constexpr uint32_t NONE = 0u;
    
    // ─── By Category ──────────────────────────────────────────────────────────
    constexpr uint32_t STIMULUS = POLYPHONY_TO_AMPLITUDE | PAWN_TO_FIELD_COLOR | 
                                  SPHERE_TO_FIELD_COLOR | POLYPHONY_TO_CELL_COLOR |
                                  PAWN_TO_CELL_COLOR | SPHERE_TO_CELL_COLOR |
                                  POLYPHONY_TO_SPHERE_COLOR | SPHERE_TO_TERRAIN_TINT;
    constexpr uint32_t DERIVED = TERRAIN_TO_PAWN_Y | TERRAIN_TO_PAWN_TILT | PAWN_TO_CAMERA_TARGET;
    constexpr uint32_t ACCUMULATED = INPUT_MOVES_PAWN | INPUT_ORBITS_CAMERA | INPUT_ZOOMS_CAMERA;
    
    // ─── By Subsystem ─────────────────────────────────────────────────────────
    constexpr uint32_t SIGNAL = POLYPHONY_TO_AMPLITUDE | POLYPHONY_TO_CELL_COLOR | POLYPHONY_TO_SPHERE_COLOR;
    constexpr uint32_t TERRAIN = TERRAIN_TO_PAWN_Y | TERRAIN_TO_PAWN_TILT;
    constexpr uint32_t INPUT = INPUT_MOVES_PAWN | INPUT_ORBITS_CAMERA | INPUT_ZOOMS_CAMERA;
    constexpr uint32_t CAMERA = PAWN_TO_CAMERA_TARGET | INPUT_ORBITS_CAMERA | INPUT_ZOOMS_CAMERA;
    constexpr uint32_t FIELD = PAWN_TO_FIELD_COLOR | SPHERE_TO_FIELD_COLOR;
    constexpr uint32_t CELLS = POLYPHONY_TO_CELL_COLOR | PAWN_TO_CELL_COLOR | SPHERE_TO_CELL_COLOR;
    
    // ─── Entity-specific ──────────────────────────────────────────────────────
    constexpr uint32_t PAWN_INFLUENCE = PAWN_TO_FIELD_COLOR | PAWN_TO_CELL_COLOR;
    constexpr uint32_t SPHERE_INFLUENCE = SPHERE_TO_FIELD_COLOR | SPHERE_TO_CELL_COLOR | SPHERE_TO_TERRAIN_TINT;
    constexpr uint32_t SPHERE_APPEARANCE = POLYPHONY_TO_SPHERE_COLOR;  // NEW group
}


// ═══════════════════════════════════════════════════════════════════════════════
// §4 GPU STRUCTURES — Must match world.wgsl exactly
// ═══════════════════════════════════════════════════════════════════════════════

// ─── Frame Signal (CPU → GPU each frame) ─────────────────────────────────────

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

// ─── Design Config ───────────────────────────────────────────────────────────

struct alignas(16) GPUDesignConfig {
    uint32_t mute_dynamics_0d;
    uint32_t mute_dynamics_2d;
    uint32_t mute_signal;
    uint32_t mute_couplings;
    float wave_time_scale;
    float pawn_speed;
    float camera_sensitivity;
    uint32_t freeze_sphere;
    float active_cell_size;      // Animatable: 8.0 → 64.0 for resolution effects
    float _cfg_pad0;
    float _cfg_pad1;
    float _cfg_pad2;
};

// ─── Trajectory (1D, for amplitude etc.) ─────────────────────────────────────

struct alignas(16) GPUTrajectory {
    float value;
    float velocity;
    float _pad0;
    float _pad1;
};

// ─── Ground State ────────────────────────────────────────────────────────────

struct alignas(16) GPUGroundState {
    float amplitude_scale;
    float max_amplitude;
    float size;
    float lipschitz_factor;
    float tint[3];             // NEW: global color modifier from sphere position
    float _pad;
};

// ─── Pawn State ──────────────────────────────────────────────────────────────

struct alignas(16) GPUPawnState {
    float pos[3];
    float heading;
    float orientation[4];
};

// ─── Camera State ────────────────────────────────────────────────────────────

struct alignas(16) GPUCameraState {
    float pos[3];
    float azimuth;
    float elevation;
    float distance;
    float pan_x;
    float pan_y;
};

// ─── Sphere State ────────────────────────────────────────────────────────────

struct alignas(16) GPUSphereState {
    float pos[3];
    float radius;
    float orientation[4];
    float influence_radius;
    float t;
    float _pad1;
    float _pad2;
    float color[3];            // NEW: current appearance (driven by polyphony)
    float _pad3;
};

// ─── Cell State ─────────────────────────────────────────────────────────────
//
// Multi-channel cell trajectory. 48 bytes per cell.
// IDLE IS COMPUTED IN SHADER, NOT STORED.
//
// value:    vec4 — .rgb=color, .a=height_mod (what the renderer observes)
// velocity: vec4 — rate of change per channel
// goal:     vec4 — where stimulus wants it (when active)
// is_active: mode flag (1 = spring toward goal, 0 = release to idle)

struct alignas(16) GPUCellState {
    float value[4];           // 16 bytes: rgb=color, a=height_mod
    float velocity[4];        // 16 bytes: rate of change
    float goal[4];            // 16 bytes: stimulus target
    float is_active;          // 4 bytes
    float _pad0;
    float _pad1;
    float _pad2;              // 12 bytes padding to align to 16
};  // 64 bytes total (keeps alignment, idle computed not stored)

// ─── Size Verification ───────────────────────────────────────────────────────

static_assert(sizeof(GPUFrameSignal) == 304, "GPUFrameSignal must be 304 bytes");
static_assert(sizeof(GPUDesignConfig) == 48, "GPUDesignConfig must be 48 bytes");
static_assert(sizeof(GPUTrajectory) == 16, "GPUTrajectory must be 16 bytes");
static_assert(sizeof(GPUGroundState) == 32, "GPUGroundState must be 32 bytes");  // Was 16, added tint
static_assert(sizeof(GPUPawnState) == 32, "GPUPawnState must be 32 bytes");
static_assert(sizeof(GPUCameraState) == 32, "GPUCameraState must be 32 bytes");
static_assert(sizeof(GPUSphereState) == 64, "GPUSphereState must be 64 bytes");  // Was 48, added color
static_assert(sizeof(GPUCellState) == 64, "GPUCellState must be 64 bytes");


// ═══════════════════════════════════════════════════════════════════════════════
// §5 ATMOSPHERE — Colors
// ═══════════════════════════════════════════════════════════════════════════════

constexpr float FOG_COLOR_R = 0.85f;
constexpr float FOG_COLOR_G = 0.78f;
constexpr float FOG_COLOR_B = 0.72f;


// ═══════════════════════════════════════════════════════════════════════════════
// §6 GPU STATE CLASS
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

    // ─── Signal Upload ───────────────────────────────────────────────────────

    void upload_signal(wgpu::Queue& queue, const GPUFrameSignal& signal) {
        queue.WriteBuffer(signalBuffer_, 0, &signal, sizeof(GPUFrameSignal));
    }

    // ─── Config Upload ───────────────────────────────────────────────────────

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

    void set_mute_couplings(uint32_t coupling_mask) {
        config_.mute_couplings = coupling_mask;
    }

    void set_wave_time_scale(float scale) {
        config_.wave_time_scale = scale;
    }

    void set_pawn_speed(float speed) {
        config_.pawn_speed = speed;
    }

    void set_camera_sensitivity(float sensitivity) {
        config_.camera_sensitivity = sensitivity;
    }

    void toggle_freeze_sphere() {
        config_.freeze_sphere = config_.freeze_sphere ? 0 : 1;
    }

    void set_freeze_sphere(bool frozen) {
        config_.freeze_sphere = frozen ? 1 : 0;
    }

    void set_active_cell_size(float size) {
        // Clamp to valid range [8, 64]
        config_.active_cell_size = std::max(8.0f, std::min(64.0f, size));
    }

    float get_active_cell_size() const {
        return config_.active_cell_size;
    }

    GPUDesignConfig& config() { return config_; }
    const GPUDesignConfig& config() const { return config_; }

    // ─── Bind Groups ─────────────────────────────────────────────────────────

    wgpu::BindGroup compute_entity_bind_group() const { return computeEntityBindGroup_; }
    wgpu::BindGroup render_entity_bind_group() const { return renderEntityBindGroup_; }
    wgpu::BindGroupLayout compute_entity_bind_group_layout() const { return computeEntityBindGroupLayout_; }
    wgpu::BindGroupLayout render_entity_bind_group_layout() const { return renderEntityBindGroupLayout_; }

    wgpu::BindGroup compute_texture_bind_group() const { return computeTextureBindGroup_; }
    wgpu::BindGroup render_texture_bind_group() const { return renderTextureBindGroup_; }
    wgpu::BindGroupLayout compute_texture_bind_group_layout() const { return computeTextureBindGroupLayout_; }
    wgpu::BindGroupLayout render_texture_bind_group_layout() const { return renderTextureBindGroupLayout_; }

    // ─── Dispatch Dimensions ─────────────────────────────────────────────────

    static constexpr uint32_t height_field_workgroups() {
        return HEIGHT_FIELD_SIZE / 8;
    }

    static constexpr uint32_t cell_grid_workgroups() {
        return CELL_GRID_SIZE / 8;
    }

    static constexpr uint32_t tile_texture_workgroups() {
        return TILE_TEXTURE_SIZE / 8;
    }

private:
    wgpu::Device device_;
    GPUDesignConfig config_{};

    // ─── Buffers ─────────────────────────────────────────────────────────────
    wgpu::Buffer signalBuffer_;
    wgpu::Buffer configBuffer_;
    wgpu::Buffer groundBuffer_;
    wgpu::Buffer pawnBuffer_;
    wgpu::Buffer cameraBuffer_;
    wgpu::Buffer sphereBuffer_;
    wgpu::Buffer trajectoriesBuffer_;
    wgpu::Buffer trajectoryFieldBuffer_;
    wgpu::Buffer cellsBuffer_;

    // ─── Textures ────────────────────────────────────────────────────────────
    wgpu::Texture heightFieldTexture_;
    wgpu::TextureView heightFieldWriteView_;
    wgpu::TextureView heightFieldReadView_;

    wgpu::Texture tileStateTexture_;
    wgpu::TextureView tileStateWriteView_;
    wgpu::TextureView tileStateReadView_;

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
    wgpu::BindGroup computeTextureBindGroup_;
    wgpu::BindGroup renderTextureBindGroup_;

    // ─────────────────────────────────────────────────────────────────────────
    // CREATION METHODS
    // ─────────────────────────────────────────────────────────────────────────

    bool createBuffers() {
        // Signal buffer (uniform for compute, storage for render)
        {
            wgpu::BufferDescriptor desc{};
            desc.label = "Frame Signal";
            desc.size = sizeof(GPUFrameSignal);
            desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            signalBuffer_ = device_.CreateBuffer(&desc);
            if (!signalBuffer_) return false;
        }

        // Config buffer (uniform - read only, small)
        {
            wgpu::BufferDescriptor desc{};
            desc.label = "Design Config";
            desc.size = sizeof(GPUDesignConfig);
            desc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
            configBuffer_ = device_.CreateBuffer(&desc);
            if (!configBuffer_) return false;
        }

        // Ground buffer
        {
            wgpu::BufferDescriptor desc{};
            desc.label = "Ground State";
            desc.size = sizeof(GPUGroundState);
            desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            groundBuffer_ = device_.CreateBuffer(&desc);
            if (!groundBuffer_) return false;
        }

        // Pawn buffer
        {
            wgpu::BufferDescriptor desc{};
            desc.label = "Pawn State";
            desc.size = sizeof(GPUPawnState);
            desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            pawnBuffer_ = device_.CreateBuffer(&desc);
            if (!pawnBuffer_) return false;
        }

        // Camera buffer
        {
            wgpu::BufferDescriptor desc{};
            desc.label = "Camera State";
            desc.size = sizeof(GPUCameraState);
            desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            cameraBuffer_ = device_.CreateBuffer(&desc);
            if (!cameraBuffer_) return false;
        }

        // Sphere buffer (binding 5)
        {
            wgpu::BufferDescriptor desc{};
            desc.label = "Sphere State";
            desc.size = sizeof(GPUSphereState);
            desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            sphereBuffer_ = device_.CreateBuffer(&desc);
            if (!sphereBuffer_) return false;
        }

        // Trajectories buffer (binding 6)
        {
            wgpu::BufferDescriptor desc{};
            desc.label = "Trajectories";
            desc.size = sizeof(GPUTrajectory) * MAX_TRAJECTORIES;
            desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            trajectoriesBuffer_ = device_.CreateBuffer(&desc);
            if (!trajectoriesBuffer_) return false;
        }

        // Trajectory field buffer (binding 7) - for entity proximity effects
        {
            wgpu::BufferDescriptor desc{};
            desc.label = "Trajectory Field";
            desc.size = sizeof(GPUTrajectory) * TRAJECTORY_FIELD_COUNT;
            desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            trajectoryFieldBuffer_ = device_.CreateBuffer(&desc);
            if (!trajectoryFieldBuffer_) return false;
        }

        // Cells buffer (binding 8)
        {
            wgpu::BufferDescriptor desc{};
            desc.label = "Cell States";
            desc.size = sizeof(GPUCellState) * CELL_COUNT;
            desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            cellsBuffer_ = device_.CreateBuffer(&desc);
            if (!cellsBuffer_) return false;
        }

        return true;
    }

    bool createTextures() {
        // Height field texture
        {
            wgpu::TextureDescriptor desc{};
            desc.label = "Height Field";
            desc.size = { HEIGHT_FIELD_SIZE, HEIGHT_FIELD_SIZE, 1 };
            desc.format = wgpu::TextureFormat::RGBA16Float;
            desc.usage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::TextureBinding;
            heightFieldTexture_ = device_.CreateTexture(&desc);
            if (!heightFieldTexture_) return false;

            heightFieldWriteView_ = heightFieldTexture_.CreateView();
            heightFieldReadView_ = heightFieldTexture_.CreateView();
        }

        // Tile state texture (renderer observes cell colors here)
        {
            wgpu::TextureDescriptor desc{};
            desc.label = "Tile State";
            desc.size = { TILE_TEXTURE_SIZE, TILE_TEXTURE_SIZE, 1 };
            desc.format = wgpu::TextureFormat::RGBA8Unorm;
            desc.usage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::TextureBinding;
            tileStateTexture_ = device_.CreateTexture(&desc);
            if (!tileStateTexture_) return false;

            tileStateWriteView_ = tileStateTexture_.CreateView();
            tileStateReadView_ = tileStateTexture_.CreateView();
        }

        return true;
    }

    bool createSamplers() {
        // Bilinear sampler (for height field)
        {
            wgpu::SamplerDescriptor desc{};
            desc.label = "Bilinear Sampler";
            desc.magFilter = wgpu::FilterMode::Linear;
            desc.minFilter = wgpu::FilterMode::Linear;
            desc.addressModeU = wgpu::AddressMode::ClampToEdge;
            desc.addressModeV = wgpu::AddressMode::ClampToEdge;
            bilinearSampler_ = device_.CreateSampler(&desc);
            if (!bilinearSampler_) return false;
        }

        // Nearest sampler (for tile state - preserve cell boundaries)
        {
            wgpu::SamplerDescriptor desc{};
            desc.label = "Nearest Sampler";
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
        // COMPUTE ENTITY LAYOUT (Group 0)
        // ═══════════════════════════════════════════════════════════════════
        {
            std::array<wgpu::BindGroupLayoutEntry, 9> entries{};

            // @binding(0) signal (uniform)
            entries[0].binding = 0;
            entries[0].visibility = wgpu::ShaderStage::Compute;
            entries[0].buffer.type = wgpu::BufferBindingType::Uniform;

            // @binding(1) config (uniform)
            entries[1].binding = 1;
            entries[1].visibility = wgpu::ShaderStage::Compute;
            entries[1].buffer.type = wgpu::BufferBindingType::Uniform;

            // @binding(2) ground_state
            entries[2].binding = 2;
            entries[2].visibility = wgpu::ShaderStage::Compute;
            entries[2].buffer.type = wgpu::BufferBindingType::Storage;

            // @binding(3) pawn_state
            entries[3].binding = 3;
            entries[3].visibility = wgpu::ShaderStage::Compute;
            entries[3].buffer.type = wgpu::BufferBindingType::Storage;

            // @binding(4) camera_state
            entries[4].binding = 4;
            entries[4].visibility = wgpu::ShaderStage::Compute;
            entries[4].buffer.type = wgpu::BufferBindingType::Storage;

            // @binding(5) sphere_state
            entries[5].binding = 5;
            entries[5].visibility = wgpu::ShaderStage::Compute;
            entries[5].buffer.type = wgpu::BufferBindingType::Storage;

            // @binding(6) trajectories
            entries[6].binding = 6;
            entries[6].visibility = wgpu::ShaderStage::Compute;
            entries[6].buffer.type = wgpu::BufferBindingType::Storage;

            // @binding(7) trajectory_field
            entries[7].binding = 7;
            entries[7].visibility = wgpu::ShaderStage::Compute;
            entries[7].buffer.type = wgpu::BufferBindingType::Storage;

            // @binding(8) cells
            entries[8].binding = 8;
            entries[8].visibility = wgpu::ShaderStage::Compute;
            entries[8].buffer.type = wgpu::BufferBindingType::Storage;

            wgpu::BindGroupLayoutDescriptor desc{};
            desc.label = "Compute Entity Layout";
            desc.entryCount = entries.size();
            desc.entries = entries.data();

            computeEntityBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
            if (!computeEntityBindGroupLayout_) return false;
        }

        // ═══════════════════════════════════════════════════════════════════
        // RENDER ENTITY LAYOUT (Group 0)
        // ═══════════════════════════════════════════════════════════════════
        {
            std::array<wgpu::BindGroupLayoutEntry, 5> entries{};

            // @binding(10) render_signal
            entries[0].binding = 10;
            entries[0].visibility = wgpu::ShaderStage::Fragment;
            entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

            // @binding(11) render_ground
            entries[1].binding = 11;
            entries[1].visibility = wgpu::ShaderStage::Fragment;
            entries[1].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

            // @binding(12) render_pawn
            entries[2].binding = 12;
            entries[2].visibility = wgpu::ShaderStage::Fragment;
            entries[2].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

            // @binding(13) render_camera
            entries[3].binding = 13;
            entries[3].visibility = wgpu::ShaderStage::Fragment;
            entries[3].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

            // @binding(14) render_sphere
            entries[4].binding = 14;
            entries[4].visibility = wgpu::ShaderStage::Fragment;
            entries[4].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;

            wgpu::BindGroupLayoutDescriptor desc{};
            desc.label = "Render Entity Layout";
            desc.entryCount = entries.size();
            desc.entries = entries.data();

            renderEntityBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
            if (!renderEntityBindGroupLayout_) return false;
        }

        // ═══════════════════════════════════════════════════════════════════
        // COMPUTE TEXTURE LAYOUT (Group 1)
        // ═══════════════════════════════════════════════════════════════════
        {
            std::array<wgpu::BindGroupLayoutEntry, 2> entries{};

            // @binding(0) height_field_write
            entries[0].binding = 0;
            entries[0].visibility = wgpu::ShaderStage::Compute;
            entries[0].storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
            entries[0].storageTexture.format = wgpu::TextureFormat::RGBA16Float;
            entries[0].storageTexture.viewDimension = wgpu::TextureViewDimension::e2D;

            // @binding(1) tile_state_write
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

        // ═══════════════════════════════════════════════════════════════════
        // RENDER TEXTURE LAYOUT (Group 1)
        // ═══════════════════════════════════════════════════════════════════
        {
            std::array<wgpu::BindGroupLayoutEntry, 4> entries{};

            // @binding(10) height_field_read
            entries[0].binding = 10;
            entries[0].visibility = wgpu::ShaderStage::Fragment;
            entries[0].texture.sampleType = wgpu::TextureSampleType::Float;
            entries[0].texture.viewDimension = wgpu::TextureViewDimension::e2D;

            // @binding(11) tile_state_read
            entries[1].binding = 11;
            entries[1].visibility = wgpu::ShaderStage::Fragment;
            entries[1].texture.sampleType = wgpu::TextureSampleType::Float;
            entries[1].texture.viewDimension = wgpu::TextureViewDimension::e2D;

            // @binding(12) bilinear_sampler
            entries[2].binding = 12;
            entries[2].visibility = wgpu::ShaderStage::Fragment;
            entries[2].sampler.type = wgpu::SamplerBindingType::Filtering;

            // @binding(13) nearest_sampler
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

        // ═══════════════════════════════════════════════════════════════════
        // BIND GROUPS
        // ═══════════════════════════════════════════════════════════════════

        // Compute entity bind group
        {
            std::array<wgpu::BindGroupEntry, 9> entries{};

            entries[0].binding = 0;
            entries[0].buffer = signalBuffer_;
            entries[0].size = sizeof(GPUFrameSignal);

            entries[1].binding = 1;
            entries[1].buffer = configBuffer_;
            entries[1].size = sizeof(GPUDesignConfig);

            entries[2].binding = 2;
            entries[2].buffer = groundBuffer_;
            entries[2].size = sizeof(GPUGroundState);

            entries[3].binding = 3;
            entries[3].buffer = pawnBuffer_;
            entries[3].size = sizeof(GPUPawnState);

            entries[4].binding = 4;
            entries[4].buffer = cameraBuffer_;
            entries[4].size = sizeof(GPUCameraState);

            entries[5].binding = 5;
            entries[5].buffer = sphereBuffer_;
            entries[5].size = sizeof(GPUSphereState);

            entries[6].binding = 6;
            entries[6].buffer = trajectoriesBuffer_;
            entries[6].size = sizeof(GPUTrajectory) * MAX_TRAJECTORIES;

            entries[7].binding = 7;
            entries[7].buffer = trajectoryFieldBuffer_;
            entries[7].size = sizeof(GPUTrajectory) * TRAJECTORY_FIELD_COUNT;

            entries[8].binding = 8;
            entries[8].buffer = cellsBuffer_;
            entries[8].size = sizeof(GPUCellState) * CELL_COUNT;

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
            std::array<wgpu::BindGroupEntry, 5> entries{};

            entries[0].binding = 10;
            entries[0].buffer = signalBuffer_;
            entries[0].size = sizeof(GPUFrameSignal);

            entries[1].binding = 11;
            entries[1].buffer = groundBuffer_;
            entries[1].size = sizeof(GPUGroundState);

            entries[2].binding = 12;
            entries[2].buffer = pawnBuffer_;
            entries[2].size = sizeof(GPUPawnState);

            entries[3].binding = 13;
            entries[3].buffer = cameraBuffer_;
            entries[3].size = sizeof(GPUCameraState);

            entries[4].binding = 14;
            entries[4].buffer = sphereBuffer_;
            entries[4].size = sizeof(GPUSphereState);

            wgpu::BindGroupDescriptor desc{};
            desc.label = "Render Entity BindGroup";
            desc.layout = renderEntityBindGroupLayout_;
            desc.entryCount = entries.size();
            desc.entries = entries.data();

            renderEntityBindGroup_ = device_.CreateBindGroup(&desc);
            if (!renderEntityBindGroup_) return false;
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

        // ─── Config ──────────────────────────────────────────────────────────
        config_.mute_dynamics_0d = 0;
        config_.mute_dynamics_2d = 0;
        config_.mute_signal = 0;
        config_.mute_couplings = Coupling::NONE;
        config_.wave_time_scale = Idle::WAVE_TIME_SCALE;
        config_.pawn_speed = Idle::PAWN_SPEED;
        config_.camera_sensitivity = Idle::CAMERA_SENSITIVITY;
        config_.freeze_sphere = 0;
        config_.active_cell_size = Idle::ACTIVE_CELL_SIZE;
        config_._cfg_pad0 = 0.0f;
        config_._cfg_pad1 = 0.0f;
        config_._cfg_pad2 = 0.0f;
        queue.WriteBuffer(configBuffer_, 0, &config_, sizeof(config_));

        // ─── Ground ──────────────────────────────────────────────────────────
        GPUGroundState ground{};
        ground.amplitude_scale = Idle::AMPLITUDE_SCALE;
        ground.max_amplitude = Idle::MAX_AMPLITUDE;
        ground.size = Idle::SIZE;
        ground.lipschitz_factor = Idle::LIPSCHITZ_FACTOR;
        ground.tint[0] = 1.0f;  // Neutral tint at start
        ground.tint[1] = 1.0f;
        ground.tint[2] = 1.0f;
        ground._pad = 0.0f;
        queue.WriteBuffer(groundBuffer_, 0, &ground, sizeof(ground));

        // ─── Pawn ────────────────────────────────────────────────────────────
        GPUPawnState pawn{};
        pawn.pos[0] = Idle::PAWN_POS_X;
        pawn.pos[1] = Idle::PAWN_POS_Y;
        pawn.pos[2] = Idle::PAWN_POS_Z;
        pawn.heading = Idle::PAWN_HEADING;
        pawn.orientation[0] = 0.0f;
        pawn.orientation[1] = 0.0f;
        pawn.orientation[2] = 0.0f;
        pawn.orientation[3] = 1.0f;
        queue.WriteBuffer(pawnBuffer_, 0, &pawn, sizeof(pawn));

        // ─── Camera ──────────────────────────────────────────────────────────
        GPUCameraState camera{};
        camera.pos[0] = Idle::CAMERA_POS_X;
        camera.pos[1] = Idle::CAMERA_POS_Y;
        camera.pos[2] = Idle::CAMERA_POS_Z;
        camera.azimuth = Idle::CAMERA_AZIMUTH;
        camera.elevation = Idle::CAMERA_ELEVATION;
        camera.distance = Idle::CAMERA_DISTANCE;
        camera.pan_x = 0.0f;
        camera.pan_y = 0.0f;
        queue.WriteBuffer(cameraBuffer_, 0, &camera, sizeof(camera));

        // ─── Sphere ────────────────────────────────────────────────────────────
        GPUSphereState sphere{};
        sphere.pos[0] = Idle::SPHERE_ORBIT_RADIUS;  // Start at angle=0
        sphere.pos[1] = Idle::SPHERE_HOVER_HEIGHT;
        sphere.pos[2] = 0.0f;
        sphere.radius = Idle::SPHERE_RADIUS;
        sphere.orientation[0] = 0.0f;
        sphere.orientation[1] = 0.0f;
        sphere.orientation[2] = 0.0f;
        sphere.orientation[3] = 1.0f;
        sphere.influence_radius = Idle::SPHERE_INFLUENCE_RADIUS;
        sphere.t = 0.0f;
        sphere._pad1 = 0.0f;
        sphere._pad2 = 0.0f;
        sphere.color[0] = 0.95f;  // SPHERE_BASE_COLOR (warm gold)
        sphere.color[1] = 0.75f;
        sphere.color[2] = 0.4f;
        sphere._pad3 = 0.0f;
        queue.WriteBuffer(sphereBuffer_, 0, &sphere, sizeof(sphere));

        // ─── Trajectories ────────────────────────────────────────────────────
        GPUTrajectory trajectories[MAX_TRAJECTORIES]{};
        for (int i = 0; i < MAX_TRAJECTORIES; ++i) {
            trajectories[i].value = Idle::TRAJECTORY_VALUE;
            trajectories[i].velocity = Idle::TRAJECTORY_VELOCITY;
        }
        queue.WriteBuffer(trajectoriesBuffer_, 0, trajectories, sizeof(trajectories));

        // ─── Trajectory Field ─────────────────────────────────────────────────
        // Initialize all cells to idle (0.0 value, 0.0 velocity)
        std::vector<GPUTrajectory> trajectoryField(TRAJECTORY_FIELD_COUNT);
        for (uint32_t i = 0; i < TRAJECTORY_FIELD_COUNT; ++i) {
            trajectoryField[i].value = 0.0f;     // IDLE_FIELD_OFFSET
            trajectoryField[i].velocity = 0.0f;
            trajectoryField[i]._pad0 = 0.0f;
            trajectoryField[i]._pad1 = 0.0f;
        }
        queue.WriteBuffer(trajectoryFieldBuffer_, 0, trajectoryField.data(), 
                          sizeof(GPUTrajectory) * TRAJECTORY_FIELD_COUNT);

        // ─── Cells ────────────────────────────────────────────────────────────
        // Initialize cells. IDLE IS COMPUTED IN SHADER, not stored.
        // Value starts at computed idle (checkerboard color, 0 height).
        std::vector<GPUCellState> cells(CELL_COUNT);
        for (uint32_t y = 0; y < CELL_GRID_SIZE; ++y) {
            for (uint32_t x = 0; x < CELL_GRID_SIZE; ++x) {
                uint32_t idx = y * CELL_GRID_SIZE + x;
                GPUCellState& cell = cells[idx];

                // Compute idle color (matches shader's idle_color function)
                bool light = ((x + y) & 1) == 0;
                float factor = light ? Idle::CHECKER_LIGHT : Idle::CHECKER_DARK;
                float idle_r = Idle::BASE_R * factor;
                float idle_g = Idle::BASE_G * factor;
                float idle_b = Idle::BASE_B * factor;
                float idle_height = 0.0f;  // idle_height_mod returns 0

                // Value starts at computed idle (vec4: rgb + height_mod)
                cell.value[0] = idle_r;
                cell.value[1] = idle_g;
                cell.value[2] = idle_b;
                cell.value[3] = idle_height;

                // No motion initially
                cell.velocity[0] = 0.0f;
                cell.velocity[1] = 0.0f;
                cell.velocity[2] = 0.0f;
                cell.velocity[3] = 0.0f;

                // Goal starts at idle
                cell.goal[0] = idle_r;
                cell.goal[1] = idle_g;
                cell.goal[2] = idle_b;
                cell.goal[3] = idle_height;

                // Inactive (releasing to computed idle)
                cell.is_active = 0.0f;

                // Padding
                cell._pad0 = 0.0f;
                cell._pad1 = 0.0f;
                cell._pad2 = 0.0f;
            }
        }
        queue.WriteBuffer(cellsBuffer_, 0, cells.data(), sizeof(GPUCellState) * CELL_COUNT);

        return true;
    }
};

} // namespace n_dimensional_3
} // namespace t7
