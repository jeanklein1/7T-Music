#pragma once

/**
 * PLAYGROUND HYBRID — Render Cartridge
 * ====================================
 * 
 * Branching architecture with interference modulation.
 * Precomputed branch field for fast raymarching.
 * 
 * CONTROLS:
 *   Mouse drag     Orbit camera
 *   Right drag     Pan
 *   Scroll         Zoom
 * 
 * Hot reload enabled — edit world.wgsl to change shading.
 * 
 * See world.wgsl for the GPU scroll.
 */

#include "render/render_cartridge.hpp"
#include "core/input_event.hpp"
#include "cartridges/playground_hybrid/state.hpp"
#include "cartridges/playground_hybrid/renderer.hpp"
#include "cartridges/playground_hybrid/branches.hpp"
#include <cmath>
#include <iostream>

namespace t7 {
namespace playground_hybrid {


class Cartridge : public RenderCartridge {
public:
    Cartridge() = default;
    
    Cartridge(const Cartridge&) = delete;
    Cartridge& operator=(const Cartridge&) = delete;
    
    bool supports_backspace() const override {
        return true;  // Enable BACKSPACE
    }


    // ═══════════════════════════════════════════════════════════════════════════
    // §1 LIFECYCLE
    // ═══════════════════════════════════════════════════════════════════════════
    
    void initialize(wgpu::Device device) override {
        device_ = device;
        
        // Configure branch generation
        BranchConfig config = default_branch_config();
        config.seed = 12345;
        
        // Spatial bounds
        config.perimeter_radius = 25.0f;
        
        // Branch topology - reduced for performance
        config.branch_count_min = 3;
        config.branch_count_max = 5;
        config.bifurcation_depth = 3;
        config.branch_length_min = 4.0f;
        config.branch_length_max = 8.0f;
        config.branch_angle_variance = 0.45f;
        
        // Height distribution
        config.height_at_perimeter = 2.5f;
        config.height_at_center = 14.0f;
        
        // Interference waves
        config.wave_frequency = 0.6f;
        config.height_amplitude = 2.5f;
        config.thickness_base = 1.0f;
        config.thickness_amplitude = 0.5f;
        config.min_thickness = 0.5f;
        config.max_thickness = 2.5f;
        config.min_height = 1.5f;
        config.max_height = 16.0f;
        
        branchConfig_ = config;
        gpuState_.init(device, config);
        
        print_welcome();
    }
    
    bool init_renderer(
        wgpu::TextureFormat colorFormat,
        wgpu::TextureFormat depthFormat
    ) {
        return renderer_.init(
            device_,
            gpuState_.compute_buffer_layout(),
            gpuState_.compute_texture_layout(),
            gpuState_.render_buffer_layout(),
            gpuState_.render_texture_layout(),
            colorFormat
        );
    }
    
    void update(const AnalysisSignal& signal,
                float aspect_ratio,
                wgpu::Queue& queue) override {
        
        GPUFrameSignal gpuSignal{};
        gpuSignal.t_seconds = signal.t_seconds;
        gpuSignal.dt = signal.dt;
        gpuSignal.aspect_ratio = aspect_ratio;
        gpuSignal._pad0 = 0.0f;
        gpuSignal.look_az_delta = inputState_.look_az_delta;
        gpuSignal.look_el_delta = inputState_.look_el_delta;
        gpuSignal.zoom_delta = inputState_.zoom_delta;
        gpuSignal.pan_x_delta = inputState_.pan_x_delta;
        gpuSignal.pan_y_delta = inputState_.pan_y_delta;
        gpuSignal._pad1 = 0.0f;
        gpuSignal._pad2 = 0.0f;
        gpuSignal._pad3 = 0.0f;
        
        gpuState_.upload_signal(queue, gpuSignal);
        
        clear_input_deltas();
    }
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §2 RENDER
    // ═══════════════════════════════════════════════════════════════════════════
    
    void render(wgpu::CommandEncoder& encoder,
                wgpu::TextureView backbuffer,
                wgpu::TextureView depth) override {
        
        // ─── COMPUTE PASS ───────────────────────────────────────────────────
        {
            wgpu::ComputePassDescriptor desc{};
            desc.label = "Playground Hybrid Compute";
            wgpu::ComputePassEncoder compute = encoder.BeginComputePass(&desc);
            
            // 1. Update camera (0D)
            renderer_.dispatch_update_camera(
                compute,
                gpuState_.compute_buffer_bind_group(),
                gpuState_.compute_texture_bind_group()
            );
            
            // 2. Update branch field (2D) - only when needed
            if (gpuState_.needs_field_update()) {
                renderer_.dispatch_update_branch_field(
                    compute,
                    gpuState_.compute_buffer_bind_group(),
                    gpuState_.compute_texture_bind_group(),
                    gpuState_.branch_field_workgroups()
                );
                gpuState_.clear_field_update_flag();
            }
            
            compute.End();
        }
        
        // ─── RENDER PASS ────────────────────────────────────────────────────
        {
            wgpu::RenderPassColorAttachment colorAttachment{};
            colorAttachment.view = backbuffer;
            colorAttachment.loadOp = wgpu::LoadOp::Clear;
            colorAttachment.storeOp = wgpu::StoreOp::Store;
            colorAttachment.clearValue = { 0.85, 0.88, 0.92, 1.0 };
            
            wgpu::RenderPassDescriptor desc{};
            desc.label = "Playground Hybrid Terrain";
            desc.colorAttachmentCount = 1;
            desc.colorAttachments = &colorAttachment;
            
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);
            renderer_.draw_terrain(
                pass,
                gpuState_.render_buffer_bind_group(),
                gpuState_.render_texture_bind_group()
            );
            pass.End();
        }
    }
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §3 INPUT
    // ═══════════════════════════════════════════════════════════════════════════
    
    void on_input(const InputEvent& event) override {
        switch (event.type) {
            case InputEvent::Type::MouseMove:
                on_mouse_move(event.x, event.y);
                break;
            case InputEvent::Type::MouseButton:
                on_mouse_button(event.button, event.pressed);
                break;
            case InputEvent::Type::Scroll:
                on_scroll(event.y);
                break;
            default:
                break;
        }
    }
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §4 PROPERTIES
    // ═══════════════════════════════════════════════════════════════════════════
    
    void get_clear_color(float& r, float& g, float& b) const override {
        r = 0.85f; g = 0.88f; b = 0.92f;
    }
    
    wgpu::TextureFormat depth_format() const override {
        return wgpu::TextureFormat::Undefined;
    }
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §5 HOT RELOAD
    // ═══════════════════════════════════════════════════════════════════════════
    
    bool reload_shaders() override {
        return renderer_.reload();
    }
    
    const std::string& shader_path() const {
        return renderer_.shader_path();
    }
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §6 BRANCH CONTROL
    // ═══════════════════════════════════════════════════════════════════════════
    
    void regenerate_branches(uint32_t new_seed) {
        branchConfig_.seed = new_seed;
        gpuState_.regenerate(branchConfig_);
        std::cout << "[Playground Hybrid] Regenerated branches with seed: " << new_seed << "\n";
    }
    
    void regenerate_branches(const BranchConfig& config) {
        branchConfig_ = config;
        gpuState_.regenerate(config);
        std::cout << "[Playground Hybrid] Regenerated branches with new config\n";
    }
    
    BranchConfig& branch_config() { return branchConfig_; }
    const BranchConfig& branch_config() const { return branchConfig_; }
    const BranchNetwork& branch_network() const { return gpuState_.network(); }
    
    
private:
    wgpu::Device device_;
    GPUState gpuState_;
    Renderer renderer_;
    BranchConfig branchConfig_;
    
    struct InputState {
        float look_az_delta = 0.0f;
        float look_el_delta = 0.0f;
        float zoom_delta = 0.0f;
        float pan_x_delta = 0.0f;
        float pan_y_delta = 0.0f;
    } inputState_;
    
    struct MouseState {
        bool left_dragging = false;
        bool right_dragging = false;
    } mouse_;
    
    void on_mouse_move(float dx, float dy) {
        constexpr float sensitivity = 0.005f;
        if (mouse_.left_dragging) {
            inputState_.look_az_delta += dx * sensitivity;
            inputState_.look_el_delta += dy * sensitivity;
        }
        if (mouse_.right_dragging) {
            inputState_.pan_x_delta += dx * sensitivity;
            inputState_.pan_y_delta -= dy * sensitivity;
        }
    }
    
    void on_mouse_button(int button, bool pressed) {
        if (button == 0) mouse_.left_dragging = pressed;
        if (button == 1) mouse_.right_dragging = pressed;
    }
    
    void on_scroll(float delta) {
        inputState_.zoom_delta -= delta * 2.0f;
    }
    
    void clear_input_deltas() {
        inputState_.look_az_delta = 0.0f;
        inputState_.look_el_delta = 0.0f;
        inputState_.zoom_delta = 0.0f;
        inputState_.pan_x_delta = 0.0f;
        inputState_.pan_y_delta = 0.0f;
    }
    
    void print_welcome() {
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
        std::cout << "║  PLAYGROUND HYBRID — Branching Architecture                  ║\n";
        std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
        std::cout << "║  Mouse drag      Orbit camera                                ║\n";
        std::cout << "║  Right drag      Pan                                         ║\n";
        std::cout << "║  Scroll          Zoom                                        ║\n";
        std::cout << "║                                                              ║\n";
        std::cout << "║  [Hot reload enabled — edit world.wgsl for shading]          ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
        std::cout << "\n";
    }
};


} // namespace playground_hybrid
} // namespace t7
