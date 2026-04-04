#pragma once

/**
 * PLAYGROUND HYBRID — GPU State Management
 * ========================================
 *
 * Branching architecture with interference modulation.
 * Precomputed branch field texture for fast raymarching.
 *
 * TEXTURE ARCHITECTURE:
 *   branch_field (256×256, RGBA16Float):
 *     .r = distance to nearest wall centerline
 *     .g = wall height at this point
 *     .b = wall thickness at this point
 *     .a = interference value
 *
 * See world.wgsl for the GPU scroll.
 */

#include <webgpu/webgpu_cpp.h>
#include "cartridges/playground_hybrid/branches.hpp"
#include <cstring>
#include <cmath>
#include <array>
#include <iostream>

namespace t7 {
namespace playground_hybrid {


// ═══════════════════════════════════════════════════════════════════════════════
// §1 INITIAL STATE
// ═══════════════════════════════════════════════════════════════════════════════

namespace Initial {
    constexpr float CAMERA_AZIMUTH = 0.5f;
    constexpr float CAMERA_ELEVATION = 0.6f;
    constexpr float CAMERA_DISTANCE = 35.0f;
    constexpr float CAMERA_PAN_X = 0.0f;
    constexpr float CAMERA_PAN_Y = 0.0f;
}

namespace Texture {
    constexpr uint32_t BRANCH_FIELD_SIZE = 256;  // 256×256 texels
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

// Branch segment — matches BranchSegment in branches.hpp
struct alignas(16) GPUBranchSegment {
    float start_x, start_z;
    float end_x, end_z;
    float height_start;
    float height_end;
    float _pad0, _pad1;
};

// Wave source — matches WaveSource in branches.hpp
struct alignas(16) GPUWaveSource {
    float x, z;
    float phase;
    float _pad0;
};

// Branch rendering parameters
struct alignas(16) GPUBranchParams {
    float wave_frequency;
    float height_amplitude;
    float thickness_base;
    float thickness_amplitude;
    float min_thickness;
    float max_thickness;
    float min_height;
    float max_height;
    float perimeter_radius;
    float center_x;
    float center_z;
    uint32_t segment_count;
    uint32_t wave_count;
    float world_size;        // Size of world in units (for UV mapping)
    float _pad0;
    float _pad1;
};

static_assert(sizeof(GPUFrameSignal) == 48, "GPUFrameSignal must be 48 bytes");
static_assert(sizeof(GPUCameraState) == 32, "GPUCameraState must be 32 bytes");
static_assert(sizeof(GPUBranchSegment) == 32, "GPUBranchSegment must be 32 bytes");
static_assert(sizeof(GPUWaveSource) == 16, "GPUWaveSource must be 16 bytes");
static_assert(sizeof(GPUBranchParams) == 64, "GPUBranchParams must be 64 bytes");


// ═══════════════════════════════════════════════════════════════════════════════
// §3 GPU STATE CLASS
// ═══════════════════════════════════════════════════════════════════════════════

class GPUState {
public:
    GPUState() = default;
    
    bool init(wgpu::Device device) {
        device_ = device;
        
        BranchConfig config = default_branch_config();
        config.seed = 12345;
        network_ = generate_branches(config);
        print_network_stats(network_);
        
        if (!createBuffers()) return false;
        if (!createTextures()) return false;
        if (!createSamplers()) return false;
        if (!createBindGroups()) return false;
        if (!initializeState()) return false;
        
        return true;
    }
    
    bool init(wgpu::Device device, const BranchConfig& config) {
        device_ = device;
        
        network_ = generate_branches(config);
        print_network_stats(network_);
        
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
    
    bool regenerate(const BranchConfig& config) {
        network_ = generate_branches(config);
        print_network_stats(network_);
        
        wgpu::Queue queue = device_.GetQueue();
        uploadBranchData(queue);
        needsFieldUpdate_ = true;
        
        return true;
    }
    
    // ─── Bind Group Layouts ─────────────────────────────────────────────────
    wgpu::BindGroupLayout compute_buffer_layout() const { return computeBufferLayout_; }
    wgpu::BindGroupLayout compute_texture_layout() const { return computeTextureLayout_; }
    wgpu::BindGroupLayout render_buffer_layout() const { return renderBufferLayout_; }
    wgpu::BindGroupLayout render_texture_layout() const { return renderTextureLayout_; }
    
    // ─── Bind Groups ────────────────────────────────────────────────────────
    wgpu::BindGroup compute_buffer_bind_group() const { return computeBufferBindGroup_; }
    wgpu::BindGroup compute_texture_bind_group() const { return computeTextureBindGroup_; }
    wgpu::BindGroup render_buffer_bind_group() const { return renderBufferBindGroup_; }
    wgpu::BindGroup render_texture_bind_group() const { return renderTextureBindGroup_; }
    
    const BranchNetwork& network() const { return network_; }
    
    bool needs_field_update() const { return needsFieldUpdate_; }
    void clear_field_update_flag() { needsFieldUpdate_ = false; }
    
    uint32_t branch_field_workgroups() const { 
        return Texture::BRANCH_FIELD_SIZE / 8;
    }
    
private:
    wgpu::Device device_;
    BranchNetwork network_;
    bool needsFieldUpdate_ = true;
    
    // Buffers
    wgpu::Buffer signalBuffer_;
    wgpu::Buffer cameraBuffer_;
    wgpu::Buffer branchSegmentBuffer_;
    wgpu::Buffer waveSourceBuffer_;
    wgpu::Buffer branchParamsBuffer_;
    
    // Textures
    wgpu::Texture branchFieldTexture_;
    wgpu::TextureView branchFieldWriteView_;
    wgpu::TextureView branchFieldReadView_;
    
    // Samplers
    wgpu::Sampler bilinearSampler_;
    
    // Bind Group Layouts
    wgpu::BindGroupLayout computeBufferLayout_;
    wgpu::BindGroupLayout computeTextureLayout_;
    wgpu::BindGroupLayout renderBufferLayout_;
    wgpu::BindGroupLayout renderTextureLayout_;
    
    // Bind Groups
    wgpu::BindGroup computeBufferBindGroup_;
    wgpu::BindGroup computeTextureBindGroup_;
    wgpu::BindGroup renderBufferBindGroup_;
    wgpu::BindGroup renderTextureBindGroup_;
    
    bool createBuffers() {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        
        desc.label = "Signal";
        desc.size = sizeof(GPUFrameSignal);
        signalBuffer_ = device_.CreateBuffer(&desc);
        if (!signalBuffer_) return false;
        
        desc.label = "Camera";
        desc.size = sizeof(GPUCameraState);
        cameraBuffer_ = device_.CreateBuffer(&desc);
        if (!cameraBuffer_) return false;
        
        size_t segmentCount = std::max(network_.segments.size(), size_t(1));
        desc.label = "Branch Segments";
        desc.size = segmentCount * sizeof(GPUBranchSegment);
        branchSegmentBuffer_ = device_.CreateBuffer(&desc);
        if (!branchSegmentBuffer_) return false;
        
        size_t waveCount = std::max(network_.wave_sources.size(), size_t(1));
        desc.label = "Wave Sources";
        desc.size = waveCount * sizeof(GPUWaveSource);
        waveSourceBuffer_ = device_.CreateBuffer(&desc);
        if (!waveSourceBuffer_) return false;
        
        desc.label = "Branch Params";
        desc.size = sizeof(GPUBranchParams);
        branchParamsBuffer_ = device_.CreateBuffer(&desc);
        if (!branchParamsBuffer_) return false;
        
        return true;
    }
    
    bool createTextures() {
        wgpu::TextureDescriptor desc{};
        desc.label = "Branch Field";
        desc.size = { Texture::BRANCH_FIELD_SIZE, Texture::BRANCH_FIELD_SIZE, 1 };
        desc.format = wgpu::TextureFormat::RGBA16Float;
        desc.usage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::TextureBinding;
        desc.dimension = wgpu::TextureDimension::e2D;
        desc.mipLevelCount = 1;
        desc.sampleCount = 1;
        
        branchFieldTexture_ = device_.CreateTexture(&desc);
        if (!branchFieldTexture_) return false;
        
        wgpu::TextureViewDescriptor viewDesc{};
        viewDesc.format = wgpu::TextureFormat::RGBA16Float;
        viewDesc.dimension = wgpu::TextureViewDimension::e2D;
        
        viewDesc.label = "Branch Field Write";
        branchFieldWriteView_ = branchFieldTexture_.CreateView(&viewDesc);
        if (!branchFieldWriteView_) return false;
        
        viewDesc.label = "Branch Field Read";
        branchFieldReadView_ = branchFieldTexture_.CreateView(&viewDesc);
        if (!branchFieldReadView_) return false;
        
        return true;
    }
    
    bool createSamplers() {
        wgpu::SamplerDescriptor desc{};
        desc.label = "Bilinear Sampler";
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
        {
            std::array<wgpu::BindGroupLayoutEntry, 5> entries{};
            
            entries[0].binding = 0;
            entries[0].visibility = wgpu::ShaderStage::Compute;
            entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[1].binding = 1;
            entries[1].visibility = wgpu::ShaderStage::Compute;
            entries[1].buffer.type = wgpu::BufferBindingType::Storage;
            
            entries[2].binding = 2;
            entries[2].visibility = wgpu::ShaderStage::Compute;
            entries[2].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[3].binding = 3;
            entries[3].visibility = wgpu::ShaderStage::Compute;
            entries[3].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[4].binding = 4;
            entries[4].visibility = wgpu::ShaderStage::Compute;
            entries[4].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
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
        {
            std::array<wgpu::BindGroupLayoutEntry, 3> entries{};
            
            entries[0].binding = 10;
            entries[0].visibility = wgpu::ShaderStage::Fragment;
            entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[1].binding = 11;
            entries[1].visibility = wgpu::ShaderStage::Fragment;
            entries[1].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[2].binding = 12;
            entries[2].visibility = wgpu::ShaderStage::Fragment;
            entries[2].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
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
        
        // Compute buffer bind group
        {
            std::array<wgpu::BindGroupEntry, 5> entries{};
            
            entries[0].binding = 0;
            entries[0].buffer = signalBuffer_;
            entries[0].size = sizeof(GPUFrameSignal);
            
            entries[1].binding = 1;
            entries[1].buffer = cameraBuffer_;
            entries[1].size = sizeof(GPUCameraState);
            
            size_t segSize = std::max(network_.segments.size(), size_t(1)) * sizeof(GPUBranchSegment);
            entries[2].binding = 2;
            entries[2].buffer = branchSegmentBuffer_;
            entries[2].size = segSize;
            
            size_t waveSize = std::max(network_.wave_sources.size(), size_t(1)) * sizeof(GPUWaveSource);
            entries[3].binding = 3;
            entries[3].buffer = waveSourceBuffer_;
            entries[3].size = waveSize;
            
            entries[4].binding = 4;
            entries[4].buffer = branchParamsBuffer_;
            entries[4].size = sizeof(GPUBranchParams);
            
            wgpu::BindGroupDescriptor desc{};
            desc.label = "Compute Buffer BindGroup";
            desc.layout = computeBufferLayout_;
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            computeBufferBindGroup_ = device_.CreateBindGroup(&desc);
            if (!computeBufferBindGroup_) return false;
        }
        
        // Compute texture bind group
        {
            std::array<wgpu::BindGroupEntry, 1> entries{};
            
            entries[0].binding = 0;
            entries[0].textureView = branchFieldWriteView_;
            
            wgpu::BindGroupDescriptor desc{};
            desc.label = "Compute Texture BindGroup";
            desc.layout = computeTextureLayout_;
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            computeTextureBindGroup_ = device_.CreateBindGroup(&desc);
            if (!computeTextureBindGroup_) return false;
        }
        
        // Render buffer bind group
        {
            std::array<wgpu::BindGroupEntry, 3> entries{};
            
            entries[0].binding = 10;
            entries[0].buffer = signalBuffer_;
            entries[0].size = sizeof(GPUFrameSignal);
            
            entries[1].binding = 11;
            entries[1].buffer = cameraBuffer_;
            entries[1].size = sizeof(GPUCameraState);
            
            entries[2].binding = 12;
            entries[2].buffer = branchParamsBuffer_;
            entries[2].size = sizeof(GPUBranchParams);
            
            wgpu::BindGroupDescriptor desc{};
            desc.label = "Render Buffer BindGroup";
            desc.layout = renderBufferLayout_;
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            renderBufferBindGroup_ = device_.CreateBindGroup(&desc);
            if (!renderBufferBindGroup_) return false;
        }
        
        // Render texture bind group
        {
            std::array<wgpu::BindGroupEntry, 2> entries{};
            
            entries[0].binding = 0;
            entries[0].textureView = branchFieldReadView_;
            
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
        
        uploadBranchData(queue);
        
        return true;
    }
    
    void uploadBranchData(wgpu::Queue& queue) {
        const BranchConfig& cfg = network_.config;
        
        if (!network_.segments.empty()) {
            queue.WriteBuffer(
                branchSegmentBuffer_, 0,
                network_.segments.data(),
                network_.segments.size() * sizeof(GPUBranchSegment)
            );
        }
        
        if (!network_.wave_sources.empty()) {
            queue.WriteBuffer(
                waveSourceBuffer_, 0,
                network_.wave_sources.data(),
                network_.wave_sources.size() * sizeof(GPUWaveSource)
            );
        }
        
        GPUBranchParams params{};
        params.wave_frequency = cfg.wave_frequency;
        params.height_amplitude = cfg.height_amplitude;
        params.thickness_base = cfg.thickness_base;
        params.thickness_amplitude = cfg.thickness_amplitude;
        params.min_thickness = cfg.min_thickness;
        params.max_thickness = cfg.max_thickness;
        params.min_height = cfg.min_height;
        params.max_height = cfg.max_height;
        params.perimeter_radius = cfg.perimeter_radius;
        params.center_x = cfg.center_x;
        params.center_z = cfg.center_z;
        params.segment_count = static_cast<uint32_t>(network_.segments.size());
        params.wave_count = static_cast<uint32_t>(network_.wave_sources.size());
        params.world_size = cfg.perimeter_radius * 2.0f + 10.0f;
        queue.WriteBuffer(branchParamsBuffer_, 0, &params, sizeof(params));
    }
};


} // namespace playground_hybrid
} // namespace t7
