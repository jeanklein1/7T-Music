#pragma once

/**
 * WORLD COMPUTE - GPU State Management
 * =====================================
 * 
 * Compute-first architecture: every piece of data has a natural dimensionality.
 * Compute at that dimensionality. Render samples it.
 * 
 * DIMENSIONALITY:
 *   0D — Buffers (scalar/struct): world state, pawn, camera
 *   1D — Buffers (array): trajectories, notes
 *   2D — Textures: height field, tile state
 * 
 * BIND GROUPS:
 *   Group 0: Entity state (buffers)
 *   Group 1: 2D textures
 */

#include "analysis/analysis_signal.hpp"
#include <webgpu/webgpu_cpp.h>
#include <cstring>
#include <array>

namespace t7 {
namespace world_compute {

// =============================================================================
// GPU STRUCTURES - 0D State
// =============================================================================

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
static_assert(sizeof(GPUTrajectory) == 16, "GPUTrajectory must be 16 bytes");
static_assert(sizeof(GPUGroundState) == 16, "GPUGroundState must be 16 bytes");
static_assert(sizeof(GPUPawnState) == 32, "GPUPawnState must be 32 bytes");
static_assert(sizeof(GPUCameraState) == 32, "GPUCameraState must be 32 bytes");

// =============================================================================
// CONSTANTS
// =============================================================================

constexpr int MAX_TRAJECTORIES = 16;
constexpr uint32_t HEIGHT_FIELD_SIZE = 256;  // 256×256 texels
constexpr uint32_t TILE_TEXTURE_SIZE = 64;   // 64×64 tiles

// Fog color for clear
constexpr float FOG_COLOR_R = 0.85f;
constexpr float FOG_COLOR_G = 0.78f;
constexpr float FOG_COLOR_B = 0.72f;

// =============================================================================
// GPU STATE CLASS
// =============================================================================

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
    
    // ─── Bind Groups ─────────────────────────────────────────────────────────
    
    // Group 0: Entity state
    wgpu::BindGroup compute_entity_bind_group() const { return computeEntityBindGroup_; }
    wgpu::BindGroup render_entity_bind_group() const { return renderEntityBindGroup_; }
    wgpu::BindGroupLayout compute_entity_bind_group_layout() const { return computeEntityBindGroupLayout_; }
    wgpu::BindGroupLayout render_entity_bind_group_layout() const { return renderEntityBindGroupLayout_; }
    
    // Group 1: Textures
    wgpu::BindGroup compute_texture_bind_group() const { return computeTextureBindGroup_; }
    wgpu::BindGroup render_texture_bind_group() const { return renderTextureBindGroup_; }
    wgpu::BindGroupLayout compute_texture_bind_group_layout() const { return computeTextureBindGroupLayout_; }
    wgpu::BindGroupLayout render_texture_bind_group_layout() const { return renderTextureBindGroupLayout_; }
    
    // ─── Dispatch Dimensions ─────────────────────────────────────────────────
    
    static constexpr uint32_t height_field_workgroups() { 
        return HEIGHT_FIELD_SIZE / 8; // 32 workgroups of 8×8
    }
    
    static constexpr uint32_t tile_texture_workgroups() { 
        return TILE_TEXTURE_SIZE / 8; // 8 workgroups of 8×8
    }
    
private:
    wgpu::Device device_;
    
    // ─── Buffers (0D and 1D state) ───────────────────────────────────────────
    wgpu::Buffer signalBuffer_;
    wgpu::Buffer groundBuffer_;
    wgpu::Buffer pawnBuffer_;
    wgpu::Buffer cameraBuffer_;
    wgpu::Buffer trajectoriesBuffer_;
    
    // ─── Textures (2D state) ─────────────────────────────────────────────────
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
    
    // ─── Creation Methods ────────────────────────────────────────────────────
    
    bool createBuffers() {
        auto createBuffer = [&](const char* label, size_t size) -> wgpu::Buffer {
            wgpu::BufferDescriptor desc{};
            desc.label = label;
            desc.size = size;
            desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
            return device_.CreateBuffer(&desc);
        };
        
        signalBuffer_ = createBuffer("FrameSignal", sizeof(GPUFrameSignal));
        groundBuffer_ = createBuffer("GroundState", sizeof(GPUGroundState));
        pawnBuffer_ = createBuffer("PawnState", sizeof(GPUPawnState));
        cameraBuffer_ = createBuffer("CameraState", sizeof(GPUCameraState));
        trajectoriesBuffer_ = createBuffer("Trajectories", sizeof(GPUTrajectory) * MAX_TRAJECTORIES);
        
        return signalBuffer_ && groundBuffer_ && pawnBuffer_ && 
               cameraBuffer_ && trajectoriesBuffer_;
    }
    
    bool createTextures() {
        // Height field: RGBA16Float, 256×256
        // Note: R32Float is UnfilterableFloat in WebGPU - doesn't support linear sampling
        // RGBA16Float supports filtering and has good precision for height values
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
        
        // Compute layout: bindings 0-4 (read/write)
        {
            std::array<wgpu::BindGroupLayoutEntry, 5> entries{};
            
            // 0: signal (read-only)
            entries[0].binding = 0;
            entries[0].visibility = wgpu::ShaderStage::Compute;
            entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            // 1-4: state buffers (read-write)
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
            if (!computeEntityBindGroupLayout_) return false;
        }
        
        // Render layout: bindings 10-13 (read-only)
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
            if (!renderEntityBindGroupLayout_) return false;
        }
        
        // Compute entity bind group
        {
            wgpu::Buffer buffers[] = { signalBuffer_, groundBuffer_, pawnBuffer_, 
                                       cameraBuffer_, trajectoriesBuffer_ };
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
            if (!computeEntityBindGroup_) return false;
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
            if (!renderEntityBindGroup_) return false;
        }
        
        // ═══════════════════════════════════════════════════════════════════
        // GROUP 1: 2D Textures
        // ═══════════════════════════════════════════════════════════════════
        
        // Compute texture layout: bindings 0-1 (storage write)
        {
            std::array<wgpu::BindGroupLayoutEntry, 2> entries{};
            
            // 0: height_field_write (RGBA16Float for filtering support)
            entries[0].binding = 0;
            entries[0].visibility = wgpu::ShaderStage::Compute;
            entries[0].storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
            entries[0].storageTexture.format = wgpu::TextureFormat::RGBA16Float;
            entries[0].storageTexture.viewDimension = wgpu::TextureViewDimension::e2D;
            
            // 1: tile_state_write
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
            
            // 10: height_field_read
            entries[0].binding = 10;
            entries[0].visibility = wgpu::ShaderStage::Fragment;
            entries[0].texture.sampleType = wgpu::TextureSampleType::Float;
            entries[0].texture.viewDimension = wgpu::TextureViewDimension::e2D;
            
            // 11: tile_state_read
            entries[1].binding = 11;
            entries[1].visibility = wgpu::ShaderStage::Fragment;
            entries[1].texture.sampleType = wgpu::TextureSampleType::Float;
            entries[1].texture.viewDimension = wgpu::TextureViewDimension::e2D;
            
            // 12: bilinear_sampler
            entries[2].binding = 12;
            entries[2].visibility = wgpu::ShaderStage::Fragment;
            entries[2].sampler.type = wgpu::SamplerBindingType::Filtering;
            
            // 13: nearest_sampler
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
        
        GPUGroundState ground{};
        ground.amplitude_scale = 1.0f;
        ground.max_amplitude = 2.0f;
        ground.size = 100.0f;
        ground.lipschitz_factor = 1.0f;
        queue.WriteBuffer(groundBuffer_, 0, &ground, sizeof(ground));
        
        GPUPawnState pawn{};
        pawn.pos[0] = 0.0f;
        pawn.pos[1] = 0.0f;
        pawn.pos[2] = 0.0f;
        pawn.heading = 0.0f;
        pawn.orientation[0] = 0.0f;
        pawn.orientation[1] = 0.0f;
        pawn.orientation[2] = 0.0f;
        pawn.orientation[3] = 1.0f;
        queue.WriteBuffer(pawnBuffer_, 0, &pawn, sizeof(pawn));
        
        GPUCameraState camera{};
        camera.pos[0] = 0.0f;
        camera.pos[1] = 15.0f;
        camera.pos[2] = 30.0f;
        camera.azimuth = 0.0f;
        camera.elevation = 0.4f;
        camera.distance = 30.0f;
        camera.pan_x = 0.0f;
        camera.pan_y = 0.0f;
        queue.WriteBuffer(cameraBuffer_, 0, &camera, sizeof(camera));
        
        GPUTrajectory trajectories[MAX_TRAJECTORIES]{};
        for (int i = 0; i < MAX_TRAJECTORIES; ++i) {
            trajectories[i].value = 1.0f;
            trajectories[i].velocity = 0.0f;
        }
        queue.WriteBuffer(trajectoriesBuffer_, 0, trajectories, sizeof(trajectories));
        
        return true;
    }
};

} // namespace world_compute
} // namespace t7
