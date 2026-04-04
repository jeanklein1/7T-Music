#pragma once

/**
 * SPECIES STUDIO — GPU State Management
 * ======================================
 *
 * Live-editable design parameters for pawn species creation.
 * All profile, color, and texture values are GPU-side and tweakable
 * in real-time without rebuild.
 *
 * PARAMETER GROUPS:
 *   Profile  — Shape silhouette control points
 *   Material — Base color, metallic, roughness
 *   Texture  — Procedural pattern parameters
 *
 * WORKFLOW:
 *   1. Tweak parameters with keyboard
 *   2. See changes instantly
 *   3. Press [P] to print current values
 *   4. Copy values to create named presets
 *
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include <webgpu/webgpu_cpp.h>
#include <cstring>
#include <array>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace t7 {
namespace species_studio {


// ═══════════════════════════════════════════════════════════════════════════════
// §1 PARAMETER INDICES — For keyboard navigation
// ═══════════════════════════════════════════════════════════════════════════════

namespace ParamGroup {
    constexpr int PROFILE = 0;
    constexpr int MATERIAL = 1;
    constexpr int TEXTURE = 2;
    constexpr int COUNT = 3;
}

namespace ProfileParam {
    constexpr int BASE_RADIUS = 0;
    constexpr int BASE_RIM_RADIUS = 1;
    constexpr int BASE_HEIGHT = 2;
    constexpr int BODY_BOTTOM_RADIUS = 3;
    constexpr int BODY_TAPER = 4;        // How much body narrows (0-1)
    constexpr int BODY_HEIGHT = 5;
    constexpr int COLLAR_BULGE = 6;
    constexpr int COLLAR_HEIGHT = 7;
    constexpr int NECK_RADIUS = 8;
    constexpr int HEAD_RADIUS = 9;
    constexpr int HEAD_HEIGHT = 10;
    constexpr int SMOOTHING = 11;
    constexpr int COUNT = 12;
}

namespace MaterialParam {
    constexpr int COLOR_R = 0;
    constexpr int COLOR_G = 1;
    constexpr int COLOR_B = 2;
    constexpr int METALLIC = 3;
    constexpr int ROUGHNESS = 4;
    constexpr int ACCENT_R = 5;
    constexpr int ACCENT_G = 6;
    constexpr int ACCENT_B = 7;
    constexpr int ACCENT_BLEND = 8;      // How much accent shows
    constexpr int COUNT = 9;
}

namespace TextureParam {
    constexpr int PATTERN_TYPE = 0;      // 0=none, 1=grain, 2=marble, 3=bands, 4=cells
    constexpr int PATTERN_SCALE = 1;
    constexpr int PATTERN_INTENSITY = 2;
    constexpr int PATTERN_DISTORT = 3;
    constexpr int GRAIN_DIRECTION = 4;   // 0=vertical, 1=radial, 2=spiral
    constexpr int NOISE_OCTAVES = 5;
    constexpr int COUNT = 6;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §2 INITIAL/DEFAULT VALUES
// ═══════════════════════════════════════════════════════════════════════════════

namespace Default {
    // Camera
    constexpr float CAMERA_AZIMUTH = 0.5f;
    constexpr float CAMERA_ELEVATION = 0.3f;
    constexpr float CAMERA_DISTANCE = 5.0f;
    
    // Profile
    constexpr float BASE_RADIUS = 0.58f;
    constexpr float BASE_RIM_RADIUS = 0.47f;
    constexpr float BASE_HEIGHT = 0.31f;
    constexpr float BODY_BOTTOM_RADIUS = 0.36f;
    constexpr float BODY_TAPER = 0.49f;
    constexpr float BODY_HEIGHT = 0.80f;
    constexpr float COLLAR_BULGE = 0.30f;
    constexpr float COLLAR_HEIGHT = 0.10f;
    constexpr float NECK_RADIUS = 0.26f;
    constexpr float HEAD_RADIUS = 0.27f;
    constexpr float HEAD_HEIGHT = 0.57f;
    constexpr float SMOOTHING = 0.02f;  // Keep positive
    
    // Material (neutral white)
    constexpr float COLOR_R = 0.90f;
    constexpr float COLOR_G = 0.90f;
    constexpr float COLOR_B = 0.88f;
    constexpr float METALLIC = 0.0f;
    constexpr float ROUGHNESS = 0.4f;
    constexpr float ACCENT_R = 0.20f;
    constexpr float ACCENT_G = 0.15f;
    constexpr float ACCENT_B = 0.10f;
    constexpr float ACCENT_BLEND = 0.0f;
    
    // Texture (disabled by default)
    constexpr float PATTERN_TYPE = 0.0f;
    constexpr float PATTERN_SCALE = 8.0f;
    constexpr float PATTERN_INTENSITY = 0.3f;
    constexpr float PATTERN_DISTORT = 0.2f;
    constexpr float GRAIN_DIRECTION = 0.0f;
    constexpr float NOISE_OCTAVES = 4.0f;
}


// ═══════════════════════════════════════════════════════════════════════════════
// §3 GPU STRUCTURES — Must match world.wgsl exactly
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

// ─── Design Parameters ──────────────────────────────────────────────────────
//
// All tweakable values in one buffer for live editing.

struct alignas(16) GPUDesignParams {
    // Profile (48 bytes)
    float base_radius;
    float base_rim_radius;
    float base_height;
    float body_bottom_radius;
    
    float body_taper;
    float body_height;
    float collar_bulge;
    float collar_height;
    
    float neck_radius;
    float head_radius;
    float head_height;
    float smoothing;
    
    // Material (48 bytes)
    float color_r;
    float color_g;
    float color_b;
    float metallic;
    
    float roughness;
    float accent_r;
    float accent_g;
    float accent_b;
    
    float accent_blend;
    float _mat_pad0;
    float _mat_pad1;
    float _mat_pad2;
    
    // Texture (32 bytes)
    float pattern_type;
    float pattern_scale;
    float pattern_intensity;
    float pattern_distort;
    
    float grain_direction;
    float noise_octaves;
    float _tex_pad0;
    float _tex_pad1;
};

static_assert(sizeof(GPUFrameSignal) == 48, "GPUFrameSignal must be 48 bytes");
static_assert(sizeof(GPUCameraState) == 32, "GPUCameraState must be 32 bytes");
static_assert(sizeof(GPUDesignParams) == 128, "GPUDesignParams must be 128 bytes");


// ═══════════════════════════════════════════════════════════════════════════════
// §4 PARAMETER METADATA — Names, ranges, step sizes
// ═══════════════════════════════════════════════════════════════════════════════

struct ParamInfo {
    const char* name;
    float min_val;
    float max_val;
    float step;
    float* ptr;  // Pointer into GPUDesignParams
};

class DesignParams {
public:
    GPUDesignParams gpu;
    
    // Current selection
    int group = ParamGroup::PROFILE;
    int param = 0;
    
    DesignParams() {
        reset();
    }
    
    void reset() {
        gpu.base_radius = Default::BASE_RADIUS;
        gpu.base_rim_radius = Default::BASE_RIM_RADIUS;
        gpu.base_height = Default::BASE_HEIGHT;
        gpu.body_bottom_radius = Default::BODY_BOTTOM_RADIUS;
        gpu.body_taper = Default::BODY_TAPER;
        gpu.body_height = Default::BODY_HEIGHT;
        gpu.collar_bulge = Default::COLLAR_BULGE;
        gpu.collar_height = Default::COLLAR_HEIGHT;
        gpu.neck_radius = Default::NECK_RADIUS;
        gpu.head_radius = Default::HEAD_RADIUS;
        gpu.head_height = Default::HEAD_HEIGHT;
        gpu.smoothing = Default::SMOOTHING;
        
        gpu.color_r = Default::COLOR_R;
        gpu.color_g = Default::COLOR_G;
        gpu.color_b = Default::COLOR_B;
        gpu.metallic = Default::METALLIC;
        gpu.roughness = Default::ROUGHNESS;
        gpu.accent_r = Default::ACCENT_R;
        gpu.accent_g = Default::ACCENT_G;
        gpu.accent_b = Default::ACCENT_B;
        gpu.accent_blend = Default::ACCENT_BLEND;
        
        gpu.pattern_type = Default::PATTERN_TYPE;
        gpu.pattern_scale = Default::PATTERN_SCALE;
        gpu.pattern_intensity = Default::PATTERN_INTENSITY;
        gpu.pattern_distort = Default::PATTERN_DISTORT;
        gpu.grain_direction = Default::GRAIN_DIRECTION;
        gpu.noise_octaves = Default::NOISE_OCTAVES;
    }
    
    // ─── Parameter Access ────────────────────────────────────────────────────
    
    int param_count() const {
        switch (group) {
            case ParamGroup::PROFILE:  return ProfileParam::COUNT;
            case ParamGroup::MATERIAL: return MaterialParam::COUNT;
            case ParamGroup::TEXTURE:  return TextureParam::COUNT;
            default: return 0;
        }
    }
    
    const char* group_name() const {
        switch (group) {
            case ParamGroup::PROFILE:  return "PROFILE";
            case ParamGroup::MATERIAL: return "MATERIAL";
            case ParamGroup::TEXTURE:  return "TEXTURE";
            default: return "?";
        }
    }
    
    const char* param_name() const {
        if (group == ParamGroup::PROFILE) {
            switch (param) {
                case ProfileParam::BASE_RADIUS:       return "Base Radius";
                case ProfileParam::BASE_RIM_RADIUS:   return "Base Rim";
                case ProfileParam::BASE_HEIGHT:       return "Base Height";
                case ProfileParam::BODY_BOTTOM_RADIUS:return "Body Bottom R";
                case ProfileParam::BODY_TAPER:        return "Body Taper";
                case ProfileParam::BODY_HEIGHT:       return "Body Height";
                case ProfileParam::COLLAR_BULGE:      return "Collar Bulge";
                case ProfileParam::COLLAR_HEIGHT:     return "Collar Height";
                case ProfileParam::NECK_RADIUS:       return "Neck Radius";
                case ProfileParam::HEAD_RADIUS:       return "Head Radius";
                case ProfileParam::HEAD_HEIGHT:       return "Head Height";
                case ProfileParam::SMOOTHING:         return "Smoothing";
            }
        } else if (group == ParamGroup::MATERIAL) {
            switch (param) {
                case MaterialParam::COLOR_R:      return "Color R";
                case MaterialParam::COLOR_G:      return "Color G";
                case MaterialParam::COLOR_B:      return "Color B";
                case MaterialParam::METALLIC:     return "Metallic";
                case MaterialParam::ROUGHNESS:    return "Roughness";
                case MaterialParam::ACCENT_R:     return "Accent R";
                case MaterialParam::ACCENT_G:     return "Accent G";
                case MaterialParam::ACCENT_B:     return "Accent B";
                case MaterialParam::ACCENT_BLEND: return "Accent Blend";
            }
        } else if (group == ParamGroup::TEXTURE) {
            switch (param) {
                case TextureParam::PATTERN_TYPE:      return "Pattern Type";
                case TextureParam::PATTERN_SCALE:     return "Pattern Scale";
                case TextureParam::PATTERN_INTENSITY: return "Pattern Intensity";
                case TextureParam::PATTERN_DISTORT:   return "Pattern Distort";
                case TextureParam::GRAIN_DIRECTION:   return "Grain Direction";
                case TextureParam::NOISE_OCTAVES:     return "Noise Octaves";
            }
        }
        return "?";
    }
    
    float* current_ptr() {
        if (group == ParamGroup::PROFILE) {
            switch (param) {
                case ProfileParam::BASE_RADIUS:       return &gpu.base_radius;
                case ProfileParam::BASE_RIM_RADIUS:   return &gpu.base_rim_radius;
                case ProfileParam::BASE_HEIGHT:       return &gpu.base_height;
                case ProfileParam::BODY_BOTTOM_RADIUS:return &gpu.body_bottom_radius;
                case ProfileParam::BODY_TAPER:        return &gpu.body_taper;
                case ProfileParam::BODY_HEIGHT:       return &gpu.body_height;
                case ProfileParam::COLLAR_BULGE:      return &gpu.collar_bulge;
                case ProfileParam::COLLAR_HEIGHT:     return &gpu.collar_height;
                case ProfileParam::NECK_RADIUS:       return &gpu.neck_radius;
                case ProfileParam::HEAD_RADIUS:       return &gpu.head_radius;
                case ProfileParam::HEAD_HEIGHT:       return &gpu.head_height;
                case ProfileParam::SMOOTHING:         return &gpu.smoothing;
            }
        } else if (group == ParamGroup::MATERIAL) {
            switch (param) {
                case MaterialParam::COLOR_R:      return &gpu.color_r;
                case MaterialParam::COLOR_G:      return &gpu.color_g;
                case MaterialParam::COLOR_B:      return &gpu.color_b;
                case MaterialParam::METALLIC:     return &gpu.metallic;
                case MaterialParam::ROUGHNESS:    return &gpu.roughness;
                case MaterialParam::ACCENT_R:     return &gpu.accent_r;
                case MaterialParam::ACCENT_G:     return &gpu.accent_g;
                case MaterialParam::ACCENT_B:     return &gpu.accent_b;
                case MaterialParam::ACCENT_BLEND: return &gpu.accent_blend;
            }
        } else if (group == ParamGroup::TEXTURE) {
            switch (param) {
                case TextureParam::PATTERN_TYPE:      return &gpu.pattern_type;
                case TextureParam::PATTERN_SCALE:     return &gpu.pattern_scale;
                case TextureParam::PATTERN_INTENSITY: return &gpu.pattern_intensity;
                case TextureParam::PATTERN_DISTORT:   return &gpu.pattern_distort;
                case TextureParam::GRAIN_DIRECTION:   return &gpu.grain_direction;
                case TextureParam::NOISE_OCTAVES:     return &gpu.noise_octaves;
            }
        }
        return nullptr;
    }
    
    float step_size() const {
        if (group == ParamGroup::PROFILE) {
            if (param == ProfileParam::SMOOTHING) return 0.005f;
            if (param == ProfileParam::BODY_HEIGHT) return 0.05f;
            return 0.02f;
        } else if (group == ParamGroup::MATERIAL) {
            return 0.02f;
        } else if (group == ParamGroup::TEXTURE) {
            if (param == TextureParam::PATTERN_TYPE) return 1.0f;
            if (param == TextureParam::GRAIN_DIRECTION) return 1.0f;
            if (param == TextureParam::NOISE_OCTAVES) return 1.0f;
            if (param == TextureParam::PATTERN_SCALE) return 0.5f;
            return 0.05f;
        }
        return 0.01f;
    }
    
    // ─── Navigation ──────────────────────────────────────────────────────────
    
    void next_group() {
        group = (group + 1) % ParamGroup::COUNT;
        param = 0;
    }
    
    void prev_group() {
        group = (group + ParamGroup::COUNT - 1) % ParamGroup::COUNT;
        param = 0;
    }
    
    void next_param() {
        param = (param + 1) % param_count();
    }
    
    void prev_param() {
        param = (param + param_count() - 1) % param_count();
    }
    
    void increase() {
        float* p = current_ptr();
        if (p) *p += step_size();
    }
    
    void decrease() {
        float* p = current_ptr();
        if (p) *p -= step_size();
    }
    
    // ─── Display ─────────────────────────────────────────────────────────────
    
    float current_value() const {
        float* p = const_cast<DesignParams*>(this)->current_ptr();
        return p ? *p : 0.0f;
    }
    
    void print_current() const {
        std::cout << "[" << group_name() << "] " 
                  << param_name() << " = " 
                  << std::fixed << std::setprecision(3) 
                  << current_value() << "\n";
    }
    
    void print_all() const {
        std::cout << "\n╔══════════════════════════════════════════════════════════════╗\n";
        std::cout << "║  SPECIES DESIGN PARAMETERS                                   ║\n";
        std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
        
        std::cout << "║  PROFILE                                                     ║\n";
        std::cout << "║    base_radius:        " << std::setw(6) << gpu.base_radius << "                              ║\n";
        std::cout << "║    base_rim_radius:    " << std::setw(6) << gpu.base_rim_radius << "                              ║\n";
        std::cout << "║    base_height:        " << std::setw(6) << gpu.base_height << "                              ║\n";
        std::cout << "║    body_bottom_radius: " << std::setw(6) << gpu.body_bottom_radius << "                              ║\n";
        std::cout << "║    body_taper:         " << std::setw(6) << gpu.body_taper << "                              ║\n";
        std::cout << "║    body_height:        " << std::setw(6) << gpu.body_height << "                              ║\n";
        std::cout << "║    collar_bulge:       " << std::setw(6) << gpu.collar_bulge << "                              ║\n";
        std::cout << "║    collar_height:      " << std::setw(6) << gpu.collar_height << "                              ║\n";
        std::cout << "║    neck_radius:        " << std::setw(6) << gpu.neck_radius << "                              ║\n";
        std::cout << "║    head_radius:        " << std::setw(6) << gpu.head_radius << "                              ║\n";
        std::cout << "║    head_height:        " << std::setw(6) << gpu.head_height << "                              ║\n";
        std::cout << "║    smoothing:          " << std::setw(6) << gpu.smoothing << "                              ║\n";
        
        std::cout << "║  MATERIAL                                                    ║\n";
        std::cout << "║    color:              (" << std::setw(4) << gpu.color_r << ", " 
                  << std::setw(4) << gpu.color_g << ", " << std::setw(4) << gpu.color_b << ")                    ║\n";
        std::cout << "║    metallic:           " << std::setw(6) << gpu.metallic << "                              ║\n";
        std::cout << "║    roughness:          " << std::setw(6) << gpu.roughness << "                              ║\n";
        std::cout << "║    accent:             (" << std::setw(4) << gpu.accent_r << ", "
                  << std::setw(4) << gpu.accent_g << ", " << std::setw(4) << gpu.accent_b << ")                    ║\n";
        std::cout << "║    accent_blend:       " << std::setw(6) << gpu.accent_blend << "                              ║\n";
        
        std::cout << "║  TEXTURE                                                     ║\n";
        std::cout << "║    pattern_type:       " << std::setw(6) << (int)gpu.pattern_type << "  (0=none,1=grain,2=marble,3=bands,4=cells) ║\n";
        std::cout << "║    pattern_scale:      " << std::setw(6) << gpu.pattern_scale << "                              ║\n";
        std::cout << "║    pattern_intensity:  " << std::setw(6) << gpu.pattern_intensity << "                              ║\n";
        std::cout << "║    pattern_distort:    " << std::setw(6) << gpu.pattern_distort << "                              ║\n";
        std::cout << "║    grain_direction:    " << std::setw(6) << (int)gpu.grain_direction << "  (0=vertical,1=radial,2=spiral) ║\n";
        std::cout << "║    noise_octaves:      " << std::setw(6) << (int)gpu.noise_octaves << "                              ║\n";
        
        std::cout << "╚══════════════════════════════════════════════════════════════╝\n\n";
    }
};


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
    
    // ─── Uploads ─────────────────────────────────────────────────────────────
    
    void upload_signal(wgpu::Queue& queue, const GPUFrameSignal& signal) {
        queue.WriteBuffer(signalBuffer_, 0, &signal, sizeof(GPUFrameSignal));
    }
    
    void upload_design_params(wgpu::Queue& queue, const GPUDesignParams& params) {
        queue.WriteBuffer(designParamsBuffer_, 0, &params, sizeof(GPUDesignParams));
    }
    
    // ─── Accessors ───────────────────────────────────────────────────────────
    
    wgpu::BindGroupLayout compute_bind_group_layout() const { return computeBindGroupLayout_; }
    wgpu::BindGroupLayout render_bind_group_layout() const { return renderBindGroupLayout_; }
    wgpu::BindGroup compute_bind_group() const { return computeBindGroup_; }
    wgpu::BindGroup render_bind_group() const { return renderBindGroup_; }
    
private:
    wgpu::Device device_;
    
    wgpu::Buffer signalBuffer_;
    wgpu::Buffer cameraBuffer_;
    wgpu::Buffer designParamsBuffer_;
    
    wgpu::BindGroupLayout computeBindGroupLayout_;
    wgpu::BindGroupLayout renderBindGroupLayout_;
    wgpu::BindGroup computeBindGroup_;
    wgpu::BindGroup renderBindGroup_;
    
    bool createBuffers() {
        wgpu::BufferDescriptor desc{};
        desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
        
        desc.label = "Species Signal";
        desc.size = sizeof(GPUFrameSignal);
        signalBuffer_ = device_.CreateBuffer(&desc);
        if (!signalBuffer_) return false;
        
        desc.label = "Species Camera";
        desc.size = sizeof(GPUCameraState);
        cameraBuffer_ = device_.CreateBuffer(&desc);
        if (!cameraBuffer_) return false;
        
        desc.label = "Species Design Params";
        desc.size = sizeof(GPUDesignParams);
        designParamsBuffer_ = device_.CreateBuffer(&desc);
        if (!designParamsBuffer_) return false;
        
        return true;
    }
    
    bool createBindGroups() {
        // Compute: 0=signal, 1=camera(rw), 2=params
        {
            std::array<wgpu::BindGroupLayoutEntry, 3> entries{};
            
            entries[0].binding = 0;
            entries[0].visibility = wgpu::ShaderStage::Compute;
            entries[0].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            entries[1].binding = 1;
            entries[1].visibility = wgpu::ShaderStage::Compute;
            entries[1].buffer.type = wgpu::BufferBindingType::Storage;
            
            entries[2].binding = 2;
            entries[2].visibility = wgpu::ShaderStage::Compute;
            entries[2].buffer.type = wgpu::BufferBindingType::ReadOnlyStorage;
            
            wgpu::BindGroupLayoutDescriptor desc{};
            desc.label = "Species Compute Layout";
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            computeBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
            if (!computeBindGroupLayout_) return false;
        }
        
        // Render: 10=signal, 11=camera, 12=params
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
            desc.label = "Species Render Layout";
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            renderBindGroupLayout_ = device_.CreateBindGroupLayout(&desc);
            if (!renderBindGroupLayout_) return false;
        }
        
        // Create bind groups
        {
            std::array<wgpu::BindGroupEntry, 3> entries{};
            
            entries[0].binding = 0;
            entries[0].buffer = signalBuffer_;
            entries[0].size = sizeof(GPUFrameSignal);
            
            entries[1].binding = 1;
            entries[1].buffer = cameraBuffer_;
            entries[1].size = sizeof(GPUCameraState);
            
            entries[2].binding = 2;
            entries[2].buffer = designParamsBuffer_;
            entries[2].size = sizeof(GPUDesignParams);
            
            wgpu::BindGroupDescriptor desc{};
            desc.label = "Species Compute BindGroup";
            desc.layout = computeBindGroupLayout_;
            desc.entryCount = entries.size();
            desc.entries = entries.data();
            
            computeBindGroup_ = device_.CreateBindGroup(&desc);
            if (!computeBindGroup_) return false;
        }
        
        {
            std::array<wgpu::BindGroupEntry, 3> entries{};
            
            entries[0].binding = 10;
            entries[0].buffer = signalBuffer_;
            entries[0].size = sizeof(GPUFrameSignal);
            
            entries[1].binding = 11;
            entries[1].buffer = cameraBuffer_;
            entries[1].size = sizeof(GPUCameraState);
            
            entries[2].binding = 12;
            entries[2].buffer = designParamsBuffer_;
            entries[2].size = sizeof(GPUDesignParams);
            
            wgpu::BindGroupDescriptor desc{};
            desc.label = "Species Render BindGroup";
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
        camera.azimuth = Default::CAMERA_AZIMUTH;
        camera.elevation = Default::CAMERA_ELEVATION;
        camera.distance = Default::CAMERA_DISTANCE;
        queue.WriteBuffer(cameraBuffer_, 0, &camera, sizeof(camera));
        
        DesignParams defaults;
        queue.WriteBuffer(designParamsBuffer_, 0, &defaults.gpu, sizeof(GPUDesignParams));
        
        return true;
    }
};


} // namespace species_studio
} // namespace t7
