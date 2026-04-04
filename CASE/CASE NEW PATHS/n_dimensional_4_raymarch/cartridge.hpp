#pragma once

/**
 * N-DIMENSIONAL_4 CARTRIDGE — Render Cartridge
 * =============================================
 * 
 * Trajectory-driven cell grid. Each cell holds a color that springs toward
 * goals or releases to idle. Musical polyphony drives the goals via hue rotation.
 * 
 * EXECUTION ORDER (mirrors world.wgsl §7 COMPOSE):
 *   1. Upload signal (CPU → GPU)
 *   2. Upload config (CPU → GPU)
 *   3. Compute: update_world        [0D] — entities, global trajectories, sphere orbit
 *   4. Compute: update_height_field [2D] — terrain elevation (waves)
 *   5. Compute: sync_life_state     [2D] — Game of Life synchronization
 *   6. Compute: update_cells        [2D] — evolve color trajectories
 *   7. Compute: update_cell_heights [2D] — cell-driven terrain geometry
 *   8. Compute: update_tiles        [2D] — copy cell.value to texture
 *   9. Render:  fullscreen raymarch — includes sphere rendering
 * 
 * CORE PRINCIPLE:
 *   Every parameter is always under trajectory.
 *   The viewer sees VALUE, which emerges from dynamics.
 *   GOALS are set by couplings (here: polyphony → hue rotation).
 *   IDLE is the artist's deliberate choice (here: checkerboard).
 * 
 * DESIGN MODE:
 *   enter_design_mode()  — Mute signal, see pure idle state
 *   enter_performance_mode() — All couplings active
 *   set_mute_coupling(bit, muted) — Control individual wires
 *   isolate_*_coupling() — Focus on specific subsystems
 * 
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include "render/render_cartridge.hpp"
#include "core/input_event.hpp"
#include "cartridges/n_dimensional_4/state.hpp"
#include "cartridges/n_dimensional_4/renderer.hpp"
#include <cmath>
#include <iostream>

// GLFW key codes for Ctrl keys (not included in all GLFW headers)
#ifndef GLFW_KEY_LEFT_CONTROL
#define GLFW_KEY_LEFT_CONTROL   341
#endif
#ifndef GLFW_KEY_RIGHT_CONTROL
#define GLFW_KEY_RIGHT_CONTROL  345
#endif

// GLFW key codes for wave toggles
#ifndef GLFW_KEY_8
#define GLFW_KEY_8              56
#endif
#ifndef GLFW_KEY_9
#define GLFW_KEY_9              57
#endif
#ifndef GLFW_KEY_0
#define GLFW_KEY_0              48
#endif

// GLFW key codes for Shift keys (wave freeze modifiers)
#ifndef GLFW_KEY_LEFT_SHIFT
#define GLFW_KEY_LEFT_SHIFT     340
#endif
#ifndef GLFW_KEY_RIGHT_SHIFT
#define GLFW_KEY_RIGHT_SHIFT    344
#endif

namespace t7 {
namespace n_dimensional_4 {


// ═══════════════════════════════════════════════════════════════════════════════
// CARTRIDGE CLASS
// ═══════════════════════════════════════════════════════════════════════════════

class Cartridge : public RenderCartridge {
public:
    Cartridge() = default;
    
    Cartridge(const Cartridge&) = delete;
    Cartridge& operator=(const Cartridge&) = delete;

    bool supports_backspace() const override {
        return true;  // Enable BACKSPACE
    }
    
    
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
        
        // ─── Store wave time for freeze capture ──────────────────────────────
        float waveTimeScale = gpuState_.config().wave_time_scale;
        if (waveTimeScale <= 0.0f) waveTimeScale = 1.0f;
        lastWaveTime_ = signal.t_beats * waveTimeScale;
        
        // ─── Upload to GPU ──────────────────────────────────────────────────
        gpuState_.upload_signal(queue, gpuSignal);
        gpuState_.upload_config(queue);
        
        // ─── Clear deltas for next frame ────────────────────────────────────
        clear_input_deltas();
    }
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §2 COMPOSE — Execution order (mirrors world.wgsl §7)
    // ═══════════════════════════════════════════════════════════════════════════
    //
    // ORDER:
    //   1. Compute: update_world        [0D] — entities, trajectories
    //   2. Compute: update_height_field [2D] — height texture (waves)
    //   3. Compute: sync_life_state     [2D] — Game of Life sync
    //   4. Compute: update_cells        [2D] — trajectory evolution
    //   5. Compute: update_cell_heights [2D] — cell-driven terrain geometry
    //   6. Compute: update_tiles        [2D] — copy to texture
    //   7. Render:  fullscreen raymarch
    
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
                GPUState::terrain_height_field_workgroups()
            );
            
            // Pass 3: Sync life state (2D) — Game of Life synchronization
            renderer_.dispatch_sync_life_state(
                compute,
                gpuState_.compute_entity_bind_group(),
                gpuState_.compute_texture_bind_group(),
                GPUState::cell_grid_workgroups()
            );
            
            // Pass 4: Update cells (2D) — trajectory evolution
            renderer_.dispatch_update_cells(
                compute,
                gpuState_.compute_entity_bind_group(),
                gpuState_.compute_texture_bind_group(),
                GPUState::cell_grid_workgroups()
            );
            
            // Pass 5: Update cell heights (2D) — cell-driven terrain geometry
            renderer_.dispatch_update_cell_heights(
                compute,
                gpuState_.compute_entity_bind_group(),
                gpuState_.compute_texture_bind_group(),
                GPUState::cell_grid_workgroups()
            );
            
            // Pass 6: Update tiles (2D) — copy cell.value to texture
            renderer_.dispatch_update_tiles(
                compute,
                gpuState_.compute_entity_bind_group(),
                gpuState_.compute_texture_bind_group(),
                GPUState::surface_color_workgroups()
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
     * Isolate field coupling: both pawn→field and sphere→field flows active.
     * Good for testing the 2D trajectory field with all stimulus sources.
     */
    void isolate_field_coupling() {
        gpuState_.set_mute_couplings(Coupling::ALL & ~Coupling::FIELD);
    }
    
    /**
     * Isolate cells coupling: all cell-related couplings active.
     * Good for testing the cell trajectory system in isolation.
     */
    void isolate_cells_coupling() {
        gpuState_.set_mute_couplings(Coupling::ALL & ~Coupling::CELLS);
    }
    
    /**
     * Isolate pawn field coupling: only pawn→field color flow active.
     * Good for testing pawn influence on trajectory_field in isolation.
     */
    void isolate_pawn_field_coupling() {
        gpuState_.set_mute_couplings(Coupling::ALL & ~Coupling::PAWN_TO_FIELD_COLOR);
    }
    
    /**
     * Isolate sphere field coupling: only sphere→field color flow active.
     * Good for testing sphere influence on trajectory_field in isolation.
     */
    void isolate_sphere_field_coupling() {
        gpuState_.set_mute_couplings(Coupling::ALL & ~Coupling::SPHERE_TO_FIELD_COLOR);
    }
    
    /**
     * Isolate pawn influence: pawn affects both field and cells.
     * Good for testing pawn's total visual impact.
     */
    void isolate_pawn_influence() {
        gpuState_.set_mute_couplings(Coupling::ALL & ~Coupling::PAWN_INFLUENCE);
    }
    
    /**
     * Isolate sphere influence: sphere affects both field and cells.
     * Good for testing sphere's total visual impact.
     */
    void isolate_sphere_influence() {
        gpuState_.set_mute_couplings(Coupling::ALL & ~Coupling::SPHERE_INFLUENCE);
    }
    
    /**
     * Isolate sphere terrain coupling: sphere height follows terrain.
     * Good for testing sphere-terrain height interaction.
     */
    void isolate_sphere_terrain_coupling() {
        gpuState_.set_mute_couplings(Coupling::ALL & ~Coupling::TERRAIN_TO_SPHERE_HEIGHT);
    }
    
    // ─── Design parameters (tuning without recompile) ───────────────────────
    
    /**
     * Set wave animation speed multiplier.
     */
    void set_wave_time_scale(float scale) {
        gpuState_.set_wave_time_scale(scale);
    }
    
    /**
     * Set pawn movement speed.
     */
    void set_pawn_speed(float speed) {
        gpuState_.set_pawn_speed(speed);
    }
    
    /**
     * Set camera mouse sensitivity.
     */
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
    
    // ─── Hot Reload ─────────────────────────────────────────────────────────
    //
    // Called by incubator when .wgsl file changes on disk.
    
    bool reload_shaders() override { return renderer_.reload(); }
    const std::string& shader_path() const { return renderer_.shader_path(); }
    
    
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
    
    bool fpvMode_ = false;  // First-person view mode (toggled with Ctrl)
    bool shiftHeld_ = false;  // Shift key held (wave freeze modifier)
    float lastWaveTime_ = 0.0f;  // Last computed wave time (for freeze capture)
    
    
    // ─────────────────────────────────────────────────────────────────────────
    // INPUT HANDLERS
    // ─────────────────────────────────────────────────────────────────────────
    
    void toggle_fpv_mode() {
        fpvMode_ = !fpvMode_;
        gpuState_.set_fpv_mode(fpvMode_ ? 1 : 0);
        std::cout << "[n_dimensional_4] Camera mode: " 
                  << (fpvMode_ ? "First-Person View" : "Orbit") << std::endl;
    }
    
    void toggle_wave(uint32_t wave_index) {
        gpuState_.toggle_wave(wave_index);
        bool enabled = gpuState_.is_wave_enabled(wave_index);
        std::cout << "[n_dimensional_4] Wave " << wave_index << ": " 
                  << (enabled ? "ON" : "OFF") << std::endl;
    }
    
    void toggle_wave_freeze(uint32_t wave_index) {
        gpuState_.toggle_wave_freeze(wave_index, lastWaveTime_);
        bool frozen = gpuState_.is_wave_frozen(wave_index);
        if (frozen) {
            std::cout << "[n_dimensional_4] Wave " << wave_index << ": FROZEN at t=" 
                      << gpuState_.get_wave_frozen_t(wave_index) << std::endl;
        } else {
            std::cout << "[n_dimensional_4] Wave " << wave_index << ": UNFROZEN" << std::endl;
        }
    }
    
    void on_key_down(int key) {
        switch (key) {
            case GLFW_KEY_UP:    keys_.forward = true; break;
            case GLFW_KEY_DOWN:  keys_.backward = true; break;
            case GLFW_KEY_LEFT:  keys_.left = true; break;
            case GLFW_KEY_RIGHT: keys_.right = true; break;
            case GLFW_KEY_1:
                gpuState_.toggle_freeze_sphere();
                break;
            case GLFW_KEY_LEFT_CONTROL:
            case GLFW_KEY_RIGHT_CONTROL:
                toggle_fpv_mode();
                break;
            case GLFW_KEY_LEFT_SHIFT:
            case GLFW_KEY_RIGHT_SHIFT:
                shiftHeld_ = true;
                break;
            case GLFW_KEY_8:
                if (shiftHeld_) {
                    toggle_wave_freeze(0);
                } else {
                    toggle_wave(0);
                }
                break;
            case GLFW_KEY_9:
                if (shiftHeld_) {
                    toggle_wave_freeze(1);
                } else {
                    toggle_wave(1);
                }
                break;
            case GLFW_KEY_0:
                if (shiftHeld_) {
                    toggle_wave_freeze(2);
                } else {
                    toggle_wave(2);
                }
                break;
        }
        update_movement_intent();
    }
    
    void on_key_up(int key) {
        switch (key) {
            case GLFW_KEY_UP:    keys_.forward = false; break;
            case GLFW_KEY_DOWN:  keys_.backward = false; break;
            case GLFW_KEY_LEFT:  keys_.left = false; break;
            case GLFW_KEY_RIGHT: keys_.right = false; break;
            case GLFW_KEY_LEFT_SHIFT:
            case GLFW_KEY_RIGHT_SHIFT:
                shiftHeld_ = false;
                break;
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

} // namespace n_dimensional_4
} // namespace t7

