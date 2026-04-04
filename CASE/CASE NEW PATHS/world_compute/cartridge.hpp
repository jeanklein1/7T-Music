#pragma once

/**
 * WORLD COMPUTE - Render Cartridge
 * =================================
 * 
 * Compute-first architecture: every piece of data has a natural dimensionality.
 * Compute at that dimensionality. Render samples it.
 * 
 * EXECUTION ORDER:
 *   1. Compute: update_world()         [1 thread]       → 0D state
 *   2. Compute: update_height_field()  [256×256 threads] → 2D height texture
 *   3. Compute: update_tiles()         [64×64 threads]   → 2D tile texture
 *   4. Render:  fullscreen raymarch    [samples textures]
 * 
 * PRINCIPLE:
 *   Compute prepares. Render observes.
 *   Adding features = adding compute passes at appropriate dimensionality.
 */

#include "render/render_cartridge.hpp"
#include "core/input_event.hpp"
#include "cartridges/world_compute/state.hpp"
#include "cartridges/world_compute/renderer.hpp"
#include <cmath>

#ifndef GLFW_KEY_UP
#define GLFW_KEY_UP 265
#define GLFW_KEY_DOWN 264
#define GLFW_KEY_LEFT 263
#define GLFW_KEY_RIGHT 262
#endif

namespace t7 {
namespace world_compute {

class Cartridge : public RenderCartridge {
public:
    Cartridge() = default;
    
    Cartridge(const Cartridge&) = delete;
    Cartridge& operator=(const Cartridge&) = delete;

    bool supports_backspace() const override {
        return true;  // Enable BACKSPACE
    }
    
    // ─── LIFECYCLE ───────────────────────────────────────────────────────────
    
    void initialize(wgpu::Device device) override {
        device_ = device;
        gpuState_.init(device);
    }
    
    bool init_renderer(
        wgpu::TextureFormat colorFormat,
        wgpu::TextureFormat depthFormat
    ) {
        colorFormat_ = colorFormat;
        depthFormat_ = depthFormat;
        
        return renderer_.init(
            device_,
            gpuState_.compute_entity_bind_group_layout(),
            gpuState_.compute_texture_bind_group_layout(),
            gpuState_.render_entity_bind_group_layout(),
            gpuState_.render_texture_bind_group_layout(),
            colorFormat,
            depthFormat
        );
    }
    
    void update(const AnalysisSignal& signal, 
                float aspect_ratio,
                wgpu::Queue& queue) override {
        GPUFrameSignal gpuSignal;
        
        gpuSignal.t_seconds = signal.t_seconds;
        gpuSignal.t_beats = signal.t_beats;
        gpuSignal.dt = signal.dt;
        gpuSignal.aspect_ratio = aspect_ratio;
        
        for (size_t i = 0; i < signal.stats.size(); ++i) {
            gpuSignal.stats[i] = signal.stats[i];
        }
        
        gpuSignal.move_x = inputState_.move_x;
        gpuSignal.move_z = inputState_.move_z;
        gpuSignal.look_az_delta = inputState_.look_az_delta;
        gpuSignal.look_el_delta = inputState_.look_el_delta;
        gpuSignal.zoom_delta = inputState_.zoom_delta;
        gpuSignal.pan_x_delta = inputState_.pan_x_delta;
        gpuSignal.pan_y_delta = inputState_.pan_y_delta;
        gpuSignal._pad1 = 0.0f;
        
        gpuState_.upload_signal(queue, gpuSignal);
        clear_input_deltas();
    }
    
    void render(wgpu::CommandEncoder& encoder,
                wgpu::TextureView backbuffer,
                wgpu::TextureView depth) override {
        
        // ═══════════════════════════════════════════════════════════════════
        // COMPUTE PHASE: Three passes, ordered by dependency
        // ═══════════════════════════════════════════════════════════════════
        
        {
            wgpu::ComputePassDescriptor desc{};
            desc.label = "Compute Phase";
            wgpu::ComputePassEncoder compute = encoder.BeginComputePass(&desc);
            
            // Pass 1: Update world (0D) — updates ground_state, pawn, camera
            renderer_.dispatch_update_world(
                compute,
                gpuState_.compute_entity_bind_group(),
                gpuState_.compute_texture_bind_group()
            );
            
            // Pass 2: Update height field (2D) — reads ground_state, writes texture
            renderer_.dispatch_update_height_field(
                compute,
                gpuState_.compute_entity_bind_group(),
                gpuState_.compute_texture_bind_group(),
                GPUState::height_field_workgroups()
            );
            
            // Pass 3: Update tiles (2D) — reads ground_state, writes texture
            renderer_.dispatch_update_tiles(
                compute,
                gpuState_.compute_entity_bind_group(),
                gpuState_.compute_texture_bind_group(),
                GPUState::tile_texture_workgroups()
            );
            
            compute.End();
        }
        
        // ═══════════════════════════════════════════════════════════════════
        // RENDER PHASE: Sample precomputed state, observe
        // ═══════════════════════════════════════════════════════════════════
        
        {
            wgpu::RenderPassColorAttachment colorAttachment{};
            colorAttachment.view = backbuffer;
            colorAttachment.loadOp = wgpu::LoadOp::Clear;
            colorAttachment.storeOp = wgpu::StoreOp::Store;
            colorAttachment.clearValue = { FOG_COLOR_R, FOG_COLOR_G, FOG_COLOR_B, 1.0 };
            
            wgpu::RenderPassDescriptor desc{};
            desc.label = "World Raymarch";
            desc.colorAttachmentCount = 1;
            desc.colorAttachments = &colorAttachment;
            // No depth attachment — raymarching handles occlusion
            
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);
            
            renderer_.draw_world(
                pass,
                gpuState_.render_entity_bind_group(),
                gpuState_.render_texture_bind_group()
            );
            
            pass.End();
        }
    }
    
    // ─── INPUT ───────────────────────────────────────────────────────────────
    
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
        }
    }
    
    // ─── PROPERTIES ──────────────────────────────────────────────────────────
    
    void get_clear_color(float& r, float& g, float& b) const override {
        r = FOG_COLOR_R;
        g = FOG_COLOR_G;
        b = FOG_COLOR_B;
    }
    
    wgpu::TextureFormat depth_format() const override {
        return wgpu::TextureFormat::Depth24Plus;
    }
    
private:
    wgpu::Device device_;
    wgpu::TextureFormat colorFormat_;
    wgpu::TextureFormat depthFormat_;
    
    GPUState gpuState_;
    Renderer renderer_;
    
    // ─── Input State ─────────────────────────────────────────────────────────
    
    struct InputState {
        float move_x = 0.0f;
        float move_z = 0.0f;
        float look_az_delta = 0.0f;
        float look_el_delta = 0.0f;
        float zoom_delta = 0.0f;
        float pan_x_delta = 0.0f;
        float pan_y_delta = 0.0f;
    } inputState_;
    
    struct KeyState {
        bool forward = false;
        bool backward = false;
        bool left = false;
        bool right = false;
    } keys_;
    
    struct MouseState {
        bool left_dragging = false;
        bool right_dragging = false;
    } mouse_;
    
    // ─── Input Handlers ──────────────────────────────────────────────────────
    
    void on_key_down(int key) {
        switch (key) {
            case GLFW_KEY_UP:    keys_.forward = true; break;
            case GLFW_KEY_DOWN:  keys_.backward = true; break;
            case GLFW_KEY_LEFT:  keys_.left = true; break;
            case GLFW_KEY_RIGHT: keys_.right = true; break;
        }
        update_movement_intent();
    }
    
    void on_key_up(int key) {
        switch (key) {
            case GLFW_KEY_UP:    keys_.forward = false; break;
            case GLFW_KEY_DOWN:  keys_.backward = false; break;
            case GLFW_KEY_LEFT:  keys_.left = false; break;
            case GLFW_KEY_RIGHT: keys_.right = false; break;
        }
        update_movement_intent();
    }
    
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
    
    void update_movement_intent() {
        inputState_.move_x = 0.0f;
        inputState_.move_z = 0.0f;
        
        if (keys_.forward)  inputState_.move_z -= 1.0f;
        if (keys_.backward) inputState_.move_z += 1.0f;
        if (keys_.left)     inputState_.move_x -= 1.0f;
        if (keys_.right)    inputState_.move_x += 1.0f;
        
        float len = std::sqrt(inputState_.move_x * inputState_.move_x +
                              inputState_.move_z * inputState_.move_z);
        if (len > 1.0f) {
            inputState_.move_x /= len;
            inputState_.move_z /= len;
        }
    }
    
    void clear_input_deltas() {
        inputState_.look_az_delta = 0.0f;
        inputState_.look_el_delta = 0.0f;
        inputState_.zoom_delta = 0.0f;
        inputState_.pan_x_delta = 0.0f;
        inputState_.pan_y_delta = 0.0f;
    }
};

} // namespace world_compute
} // namespace t7
