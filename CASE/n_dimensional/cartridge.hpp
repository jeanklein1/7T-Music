#pragma once

/**
 * N-DIMENSIONAL CARTRIDGE — Render Cartridge
 * ==========================================
 * 
 * Compute-first architecture with Design Mode support and 2D trajectory fields.
 * 
 * EXECUTION ORDER (mirrors world.wgsl §6 COMPOSE):
 *   1. Upload signal (CPU → GPU)
 *   2. Upload config (CPU → GPU)
 *   3. Compute: update_world        [0D]
 *   4. Compute: update_height_field [2D]
 *   5. Compute: update_tiles        [2D] — includes trajectory field update
 *   6. Render:  fullscreen raymarch
 * 
 * DESIGN MODE:
 *   enter_design_mode()  — Mute signal, see pure idle state
 *   enter_performance_mode() — All couplings active
 *   set_mute_coupling(bit, muted) — Control individual wires
 * 
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include "render/render_cartridge.hpp"
#include "core/input_event.hpp"
#include "cartridges/n_dimensional/state.hpp"
#include "cartridges/n_dimensional/renderer.hpp"
#include <cmath>
#include <iostream>

#ifndef GLFW_KEY_UP
#define GLFW_KEY_UP 265
#define GLFW_KEY_DOWN 264
#define GLFW_KEY_LEFT 263
#define GLFW_KEY_RIGHT 262
#endif

namespace t7 {
namespace n_dimensional {


// ═══════════════════════════════════════════════════════════════════════════════
// CARTRIDGE CLASS
// ═══════════════════════════════════════════════════════════════════════════════

class Cartridge : public RenderCartridge {
public:
    Cartridge() = default;
    
    Cartridge(const Cartridge&) = delete;
    Cartridge& operator=(const Cartridge&) = delete;
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §1 LIFECYCLE — Init, update, teardown
    // ═══════════════════════════════════════════════════════════════════════════
    
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
        // ─── Build GPU signal from analysis + input ─────────────────────────
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
        
        // ─── Upload to GPU ──────────────────────────────────────────────────
        gpuState_.upload_signal(queue, gpuSignal);
        gpuState_.upload_config(queue);
        
        // ─── Clear deltas for next frame ────────────────────────────────────
        clear_input_deltas();
    }
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §2 COMPOSE — Execution order (mirrors world.wgsl §6)
    // ═══════════════════════════════════════════════════════════════════════════
    //
    // ORDER:
    //   1. Compute: update_world        [0D] — entities, trajectories
    //   2. Compute: update_height_field [2D] — height texture
    //   3. Compute: update_tiles        [2D] — tile texture
    //   4. Render:  fullscreen raymarch
    
    void render(wgpu::CommandEncoder& encoder,
                wgpu::TextureView backbuffer,
                wgpu::TextureView depth) override {
        
        // ─── COMPUTE PHASE ──────────────────────────────────────────────────
        {
            wgpu::ComputePassDescriptor desc{};
            desc.label = "Compute Phase";
            wgpu::ComputePassEncoder compute = encoder.BeginComputePass(&desc);
            
            // Pass 1: Update world (0D)
            renderer_.dispatch_update_world(
                compute,
                gpuState_.compute_entity_bind_group(),
                gpuState_.compute_texture_bind_group()
            );
            
            // Pass 2: Update height field (2D)
            renderer_.dispatch_update_height_field(
                compute,
                gpuState_.compute_entity_bind_group(),
                gpuState_.compute_texture_bind_group(),
                GPUState::height_field_workgroups()
            );
            
            // Pass 3: Update tiles (2D)
            renderer_.dispatch_update_tiles(
                compute,
                gpuState_.compute_entity_bind_group(),
                gpuState_.compute_texture_bind_group(),
                GPUState::tile_texture_workgroups()
            );
            
            compute.End();
        }
        
        // ─── RENDER PHASE ───────────────────────────────────────────────────
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
            
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);
            
            renderer_.draw_world(
                pass,
                gpuState_.render_entity_bind_group(),
                gpuState_.render_texture_bind_group()
            );
            
            pass.End();
        }
    }
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §3 DESIGN MODE — Muting and isolation
    // ═══════════════════════════════════════════════════════════════════════════
    //
    // The cartridge is an instrument. Design mode lets you examine it.
    // Performance mode lets you play it.
    
    /**
     * Enter design mode: mute signal, mute all couplings.
     * See the instrument at rest.
     */
    void enter_design_mode() {
        gpuState_.enter_design_mode();
        std::cout << "[Design Mode] Signal muted, all couplings disabled\n";
    }
    
    /**
     * Enter performance mode: all couplings active.
     * The instrument responds to music.
     */
    void enter_performance_mode() {
        gpuState_.enter_performance_mode();
        std::cout << "[Performance Mode] All couplings enabled\n";
    }
    
    /**
     * Mute/unmute the musical signal.
     */
    void set_mute_signal(bool muted) {
        gpuState_.set_mute_signal(muted);
    }
    
    /**
     * Mute/unmute 0D dynamics (entity physics freeze).
     */
    void set_mute_dynamics_0d(bool muted) {
        gpuState_.set_mute_dynamics_0d(muted);
    }
    
    /**
     * Mute/unmute 2D dynamics (terrain becomes static sculpture).
     */
    void set_mute_dynamics_2d(bool muted) {
        gpuState_.set_mute_dynamics_2d(muted);
    }
    
    /**
     * Mute/unmute a specific coupling by bit.
     * Use Coupling:: constants from state.hpp.
     */
    void set_mute_coupling(uint32_t coupling_bit, bool muted) {
        gpuState_.set_mute_coupling(coupling_bit, muted);
    }
    
    /**
     * Set multiple couplings at once via bitmask.
     */
    void set_mute_couplings(uint32_t mask) {
        gpuState_.set_mute_couplings(mask);
    }
    
    // ─── Isolation helpers (convenience) ────────────────────────────────────
    
    /**
     * Isolate terrain coupling: only terrain→pawn flows active.
     * Good for testing terrain following without input.
     */
    void isolate_terrain_coupling() {
        gpuState_.set_mute_couplings(Coupling::ALL & ~Coupling::TERRAIN);
    }
    
    /**
     * Isolate input coupling: only input→entity flows active.
     * Good for testing controls without musical influence.
     */
    void isolate_input_coupling() {
        gpuState_.set_mute_couplings(Coupling::ALL & ~Coupling::INPUT);
    }
    
    /**
     * Isolate signal coupling: only signal→substrate flows active.
     * Good for testing musical response without movement.
     */
    void isolate_signal_coupling() {
        gpuState_.set_mute_couplings(Coupling::ALL & ~Coupling::SIGNAL);
    }
    
    /**
     * Isolate field coupling: only pawn→field color flows active.
     * Good for testing the 2D trajectory field in isolation.
     */
    void isolate_field_coupling() {
        gpuState_.set_mute_couplings(Coupling::ALL & ~Coupling::FIELD);
    }
    
    // ─── Design parameters (tuning without recompile) ───────────────────────
    
    void set_wave_time_scale(float scale) {
        gpuState_.set_wave_time_scale(scale);
    }
    
    void set_pawn_speed(float speed) {
        gpuState_.set_pawn_speed(speed);
    }
    
    void set_camera_sensitivity(float sensitivity) {
        gpuState_.set_camera_sensitivity(sensitivity);
    }
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §4 INPUT — Mechanical infrastructure (not artistic)
    // ═══════════════════════════════════════════════════════════════════════════
    //
    // Converts platform events → signal deltas.
    // No coupling logic here; just data marshaling.
    
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
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §5 PROPERTIES
    // ═══════════════════════════════════════════════════════════════════════════
    
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
    
    
    // ─────────────────────────────────────────────────────────────────────────
    // INPUT STATE — Accumulated between frames
    // ─────────────────────────────────────────────────────────────────────────
    
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
    
    
    // ─────────────────────────────────────────────────────────────────────────
    // INPUT HANDLERS
    // ─────────────────────────────────────────────────────────────────────────
    
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

} // namespace n_dimensional
} // namespace t7
