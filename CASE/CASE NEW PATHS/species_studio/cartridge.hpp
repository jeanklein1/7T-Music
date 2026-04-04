#pragma once

/**
 * SPECIES STUDIO — Cartridge
 * ===========================
 *
 * Live design environment for creating pawn species.
 * 
 * CONTROLS:
 *   ─── Camera ───
 *   Mouse drag     Orbit camera
 *   Scroll         Zoom
 * 
 *   ─── Parameter Navigation ───
 *   Tab / Shift+Tab    Next/prev parameter group
 *   W / S              Next/prev parameter in group
 *   A / D              Decrease/increase value
 *   Shift + A/D        Fine adjustment (1/5 step)
 *   Ctrl + A/D         Coarse adjustment (5x step)
 * 
 *   ─── Actions ───
 *   R                  Reset all to defaults
 *   P                  Print current values to console
 *   Backspace          Return to gallery
 *
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include "render/render_cartridge.hpp"
#include "core/input_event.hpp"
#include "cartridges/species_studio/state.hpp"
#include "cartridges/species_studio/renderer.hpp"
#include <cmath>
#include <iostream>

namespace t7 {
namespace species_studio {


class Cartridge : public RenderCartridge {
public:
    Cartridge() = default;

    bool supports_backspace() const override {
        return true;  // Enable BACKSPACE
    }
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §1 LIFECYCLE
    // ═══════════════════════════════════════════════════════════════════════════
    
    void initialize(wgpu::Device device) override {
        device_ = device;
        gpuState_.init(device);
        designParams_.reset();
        
        print_controls();
    }
    
    bool init_renderer(
        wgpu::TextureFormat colorFormat,
        wgpu::TextureFormat depthFormat
    ) {
        return renderer_.init(
            device_,
            gpuState_.compute_bind_group_layout(),
            gpuState_.render_bind_group_layout(),
            colorFormat,
            depthFormat
        );
    }
    
    void update(const AnalysisSignal& signal,
                float aspect_ratio,
                wgpu::Queue& queue) override {
        
        GPUFrameSignal gpuSignal{};
        gpuSignal.t_seconds = signal.t_seconds;
        gpuSignal.dt = signal.dt;
        gpuSignal.aspect_ratio = aspect_ratio;
        gpuSignal.look_az_delta = inputState_.look_az_delta;
        gpuSignal.look_el_delta = inputState_.look_el_delta;
        gpuSignal.zoom_delta = inputState_.zoom_delta;
        gpuSignal.pan_x_delta = inputState_.pan_x_delta;
        gpuSignal.pan_y_delta = inputState_.pan_y_delta;
        
        gpuState_.upload_signal(queue, gpuSignal);
        gpuState_.upload_design_params(queue, designParams_.gpu);
        
        clear_deltas();
    }
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §2 RENDER
    // ═══════════════════════════════════════════════════════════════════════════
    
    void render(wgpu::CommandEncoder& encoder,
                wgpu::TextureView backbuffer,
                wgpu::TextureView depth) override {
        
        // Compute: update camera
        {
            wgpu::ComputePassDescriptor desc{};
            desc.label = "Species Compute";
            wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&desc);
            renderer_.dispatch_update_camera(pass, gpuState_.compute_bind_group());
            pass.End();
        }
        
        // Render: raymarch subject
        {
            wgpu::RenderPassColorAttachment colorAttachment{};
            colorAttachment.view = backbuffer;
            colorAttachment.loadOp = wgpu::LoadOp::Clear;
            colorAttachment.storeOp = wgpu::StoreOp::Store;
            colorAttachment.clearValue = { 0.02, 0.02, 0.03, 1.0 };
            
            wgpu::RenderPassDescriptor desc{};
            desc.label = "Species Render";
            desc.colorAttachmentCount = 1;
            desc.colorAttachments = &colorAttachment;
            
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);
            renderer_.draw_world(pass, gpuState_.render_bind_group());
            pass.End();
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §3 INPUT
    // ═══════════════════════════════════════════════════════════════════════════
    
    void on_input(const InputEvent& event) override {
        switch (event.type) {
            case InputEvent::Type::KeyDown:
                on_key_down(event.key);
                break;
            case InputEvent::Type::KeyUp:
                on_key_up(event.key);
                break;
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
        r = 0.02f; g = 0.02f; b = 0.03f;
    }
    
    wgpu::TextureFormat depth_format() const override {
        return wgpu::TextureFormat::Depth24Plus;
    }
    
    // ─── Hot Reload ──────────────────────────────────────────────────────────
    
    bool reload_shaders() override {
        return renderer_.reload();
    }
    
    const std::string& shader_path() const {
        return renderer_.shader_path();
    }
    
private:
    wgpu::Device device_;
    GPUState gpuState_;
    Renderer renderer_;
    DesignParams designParams_;
    
    // ─── Input State ─────────────────────────────────────────────────────────
    
    struct InputState {
        float look_az_delta = 0.0f;
        float look_el_delta = 0.0f;
        float zoom_delta = 0.0f;
        float pan_x_delta = 0.0f;
        float pan_y_delta = 0.0f;
    } inputState_;
    
    bool leftDragging_ = false;
    bool rightDragging_ = false;
    bool shiftHeld_ = false;
    bool ctrlHeld_ = false;
    
    void clear_deltas() {
        inputState_.look_az_delta = 0.0f;
        inputState_.look_el_delta = 0.0f;
        inputState_.zoom_delta = 0.0f;
        inputState_.pan_x_delta = 0.0f;
        inputState_.pan_y_delta = 0.0f;
    }
    
    // ─── Key Handlers ────────────────────────────────────────────────────────
    
    void on_key_down(int key) {
        // Track modifier keys
        if (key == 340 || key == 344) { shiftHeld_ = true; return; }  // GLFW_KEY_LEFT_SHIFT / RIGHT_SHIFT
        if (key == 341 || key == 345) { ctrlHeld_ = true; return; }   // GLFW_KEY_LEFT_CONTROL / RIGHT_CONTROL
        
        // Tab: next/prev group
        if (key == 258) {  // GLFW_KEY_TAB
            if (shiftHeld_) {
                designParams_.prev_group();
            } else {
                designParams_.next_group();
            }
            print_selection();
            return;
        }
        
        // W/S: next/prev parameter
        if (key == 87) {  // W
            designParams_.prev_param();
            print_selection();
            return;
        }
        if (key == 83) {  // S
            designParams_.next_param();
            print_selection();
            return;
        }
        
        // A/D: decrease/increase value
        if (key == 65) {  // A
            int steps = ctrlHeld_ ? 5 : 1;
            float mult = shiftHeld_ ? 0.2f : 1.0f;
            for (int i = 0; i < steps; ++i) {
                float* p = designParams_.current_ptr();
                if (p) *p -= designParams_.step_size() * mult;
            }
            print_value();
            return;
        }
        if (key == 68) {  // D
            int steps = ctrlHeld_ ? 5 : 1;
            float mult = shiftHeld_ ? 0.2f : 1.0f;
            for (int i = 0; i < steps; ++i) {
                float* p = designParams_.current_ptr();
                if (p) *p += designParams_.step_size() * mult;
            }
            print_value();
            return;
        }
        
        // R: reset
        if (key == 82) {
            designParams_.reset();
            std::cout << "[Species] Reset to defaults\n";
            return;
        }
        
        // P: print all
        if (key == 80) {
            designParams_.print_all();
            return;
        }
    }
    
    void on_key_up(int key) {
        if (key == 340 || key == 344) { shiftHeld_ = false; }
        if (key == 341 || key == 345) { ctrlHeld_ = false; }
    }
    
    void on_mouse_move(float dx, float dy) {
        constexpr float SENS = 0.005f;
        if (leftDragging_) {
            inputState_.look_az_delta += dx * SENS;
            inputState_.look_el_delta += dy * SENS;
        }
        if (rightDragging_) {
            inputState_.pan_x_delta += dx * SENS;
            inputState_.pan_y_delta -= dy * SENS;
        }
    }
    
    void on_mouse_button(int button, bool pressed) {
        if (button == 0) leftDragging_ = pressed;
        if (button == 1) rightDragging_ = pressed;
    }
    
    void on_scroll(float delta) {
        inputState_.zoom_delta -= delta * 0.5f;
    }
    
    // ─── Console Output ──────────────────────────────────────────────────────
    
    void print_controls() {
        std::cout << "\n";
        std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
        std::cout << "║  SPECIES STUDIO                                              ║\n";
        std::cout << "╠══════════════════════════════════════════════════════════════╣\n";
        std::cout << "║  Tab/Shift+Tab   Switch parameter group                      ║\n";
        std::cout << "║  W/S             Navigate parameters                         ║\n";
        std::cout << "║  A/D             Adjust value                                ║\n";
        std::cout << "║  Shift+A/D       Fine adjustment                             ║\n";
        std::cout << "║  Ctrl+A/D        Coarse adjustment                           ║\n";
        std::cout << "║  R               Reset to defaults                           ║\n";
        std::cout << "║  P               Print all values                            ║\n";
        std::cout << "║  Mouse           Orbit / Pan / Zoom                          ║\n";
        std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
        std::cout << "\n";
        print_selection();
    }
    
    void print_selection() {
        std::cout << "[" << designParams_.group_name() << "] → " 
                  << designParams_.param_name() << " = "
                  << std::fixed << std::setprecision(3)
                  << designParams_.current_value() << "\n";
    }
    
    void print_value() {
        std::cout << "  " << designParams_.param_name() << " = "
                  << std::fixed << std::setprecision(3)
                  << designParams_.current_value() << "\n";
    }
};


} // namespace species_studio
} // namespace t7
