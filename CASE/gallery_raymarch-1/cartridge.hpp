#pragma once

/**
 * GALLERY CARTRIDGE — Render Cartridge
 * =====================================
 * 
 * Updated to use Portable Door system.
 * 
 * KEY CHANGES:
 *   - Collision logic simplified: check door volume, not wall expansion
 *   - Door configuration uses PortableDoorBuilder
 *   - Transition check uses door.point_committed()
 * 
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include "render/render_cartridge.hpp"
#include "core/input_event.hpp"
#include "cartridges/gallery/state.hpp"
#include "cartridges/gallery/renderer.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <functional>

#ifndef GLFW_KEY_UP
#define GLFW_KEY_UP 265
#define GLFW_KEY_DOWN 264
#define GLFW_KEY_LEFT 263
#define GLFW_KEY_RIGHT 262
#endif

namespace t7 {
namespace gallery {


using TransitionCallback = std::function<void(uint32_t target_cartridge_id)>;


class Cartridge : public RenderCartridge {
public:
    Cartridge() = default;
    
    Cartridge(const Cartridge&) = delete;
    Cartridge& operator=(const Cartridge&) = delete;
    
    void set_transition_callback(TransitionCallback callback) {
        transitionCallback_ = std::move(callback);
    }
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §1 LIFECYCLE
    // ═══════════════════════════════════════════════════════════════════════════
    
    void initialize(wgpu::Device device) override {
        device_ = device;
        gpuState_.init(device);
        
        configure_doors();
        
        cpuPawnX_ = Initial::PAWN_POS_X;
        cpuPawnZ_ = Initial::PAWN_POS_Z;
    }
    
    bool init_renderer(
        wgpu::TextureFormat colorFormat,
        wgpu::TextureFormat depthFormat
    ) {
        colorFormat_ = colorFormat;
        depthFormat_ = depthFormat;
        
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
        gpuSignal.move_x = inputState_.move_x;
        gpuSignal.move_z = inputState_.move_z;
        gpuSignal.look_az_delta = inputState_.look_az_delta;
        gpuSignal.look_el_delta = inputState_.look_el_delta;
        gpuSignal.zoom_delta = inputState_.zoom_delta;
        gpuSignal.pan_x_delta = inputState_.pan_x_delta;
        gpuSignal.pan_y_delta = inputState_.pan_y_delta;
        
        // Move pawn on CPU (simplified collision)
        update_pawn(signal.dt);
        
        // Check door transitions
        check_door_transition();
        
        // Upload
        gpuState_.upload_signal(queue, gpuSignal);
        gpuState_.upload_config(queue);
        gpuState_.upload_doors(queue, doors_, doorCount_);
        
        clear_input_deltas();
    }
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §2 COMPOSE
    // ═══════════════════════════════════════════════════════════════════════════
    
    void render(wgpu::CommandEncoder& encoder,
                wgpu::TextureView backbuffer,
                wgpu::TextureView depth) override {
        
        // Compute pass
        {
            wgpu::ComputePassDescriptor desc{};
            desc.label = "Gallery Compute";
            wgpu::ComputePassEncoder compute = encoder.BeginComputePass(&desc);
            renderer_.dispatch_update_world(compute, gpuState_.compute_bind_group());
            compute.End();
        }
        
        // Render pass
        {
            wgpu::RenderPassColorAttachment colorAttachment{};
            colorAttachment.view = backbuffer;
            colorAttachment.loadOp = wgpu::LoadOp::Clear;
            colorAttachment.storeOp = wgpu::StoreOp::Store;
            colorAttachment.clearValue = { FOG_COLOR_R, FOG_COLOR_G, FOG_COLOR_B, 1.0 };
            
            wgpu::RenderPassDescriptor desc{};
            desc.label = "Gallery Raymarch";
            desc.colorAttachmentCount = 1;
            desc.colorAttachments = &colorAttachment;
            
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);
            renderer_.draw_world(pass, gpuState_.render_bind_group());
            pass.End();
        }
    }
    
    // ─── Post-frame transition check ─────────────────────────────────────────
    
    void check_door_transition() {
        uint32_t target = check_commitment();
        if (target != DoorTarget::NONE && transitionCallback_) {
            std::cout << "[Gallery] Door transition → " << target << "\n";
            transitionCallback_(target);
        }
    }
    
    void reset_pawn(wgpu::Queue queue) {
        cpuPawnX_ = Initial::PAWN_POS_X;
        cpuPawnZ_ = Initial::PAWN_POS_Z;
        cpuCameraAzimuth_ = Initial::CAMERA_AZIMUTH;
        keys_ = {};
        inputState_ = {};
        gpuState_.reset_pawn(queue);
    }
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §3 INPUT
    // ═══════════════════════════════════════════════════════════════════════════
    
    void on_input(const InputEvent& event) override {
        switch (event.type) {
            case InputEvent::Type::KeyDown:   on_key_down(event.key); break;
            case InputEvent::Type::KeyUp:     on_key_up(event.key); break;
            case InputEvent::Type::MouseMove: on_mouse_move(event.x, event.y); break;
            case InputEvent::Type::MouseButton: on_mouse_button(event.button, event.pressed); break;
            case InputEvent::Type::Scroll:    on_scroll(event.y); break;
        }
    }
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §4 PROPERTIES
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
    TransitionCallback transitionCallback_;
    
    // CPU pawn state
    float cpuPawnX_ = 0.0f;
    float cpuPawnZ_ = 0.0f;
    float cpuCameraAzimuth_ = 0.0f;
    
    // Doors
    PortableDoor doors_[MAX_DOORS];
    uint32_t doorCount_ = 0;
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §5 DOOR CONFIGURATION
    // ═══════════════════════════════════════════════════════════════════════════
    
    void configure_doors() {
        doorCount_ = 0;
        
        // ─── Door 1: N-Dimensional 2 (back wall) ─────────────────────────────
        doors_[doorCount_++] = PortableDoorBuilder()
            .in_back_wall(0.0f)
            .shape(DoorShape::ARCH)
            .size(3.0f, 4.0f)
            .target(DoorTarget::N_DIMENSIONAL_2)
            .color(0.7f, 0.7f, 0.8f)
            .build();
        
        // ─── Door 2: Playground Raymarch (left wall) ─────────────────────────
        doors_[doorCount_++] = PortableDoorBuilder()
            .in_left_wall(-5.0f)
            .shape(DoorShape::RECTANGLE)
            .size(2.5f, 3.5f)
            .target(DoorTarget::PLAYGROUND_RAYMARCH)
            .color(0.8f, 0.75f, 0.7f)
            .build();
        
        // ─── Door 3: Playground Rasterize (left wall) ────────────────────────
        doors_[doorCount_++] = PortableDoorBuilder()
            .in_left_wall(5.0f)
            .shape(DoorShape::CIRCLE)
            .size(2.0f, 2.0f)
            .target(DoorTarget::PLAYGROUND_RASTERIZE)
            .color(0.7f, 0.8f, 0.75f)
            .build();
        
        // ─── Door 4: Playground Hybrid (right wall) ──────────────────────────
        doors_[doorCount_++] = PortableDoorBuilder()
            .in_right_wall(0.0f)
            .shape(DoorShape::POINTED_ARCH)
            .size(3.0f, 4.5f)
            .target(DoorTarget::PLAYGROUND_HYBRID)
            .color(0.75f, 0.7f, 0.8f)
            .build();
        
        // ─── EXAMPLE: Freestanding door in the middle of nowhere ─────────────
        // Uncomment to test:
        // doors_[doorCount_++] = PortableDoorBuilder()
        //     .at(5.0f, 0.0f, 0.0f)           // In the middle of the room
        //     .facing(1.0f, 0.0f, 0.0f)       // Face +X
        //     .shape(DoorShape::ARCH)
        //     .size(2.0f, 3.0f)
        //     .target(DoorTarget::N_DIMENSIONAL_2)
        //     .color(0.9f, 0.7f, 0.7f)
        //     .build();
    }
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §6 PAWN MOVEMENT — Simplified collision
    // ═══════════════════════════════════════════════════════════════════════════
    
    void update_pawn(float dt) {
        // Transform input to world velocity
        float cos_az = std::cos(cpuCameraAzimuth_);
        float sin_az = std::sin(cpuCameraAzimuth_);
        float world_vx = inputState_.move_x * cos_az + inputState_.move_z * sin_az;
        float world_vz = -inputState_.move_x * sin_az + inputState_.move_z * cos_az;
        
        constexpr float SPEED = 8.0f;
        float new_x = cpuPawnX_ + world_vx * SPEED * dt;
        float new_z = cpuPawnZ_ + world_vz * SPEED * dt;
        
        // Check Y at chest height for door checks
        constexpr float CHECK_Y = PAWN_HEIGHT * 0.5f;
        
        // ─── THE KEY SIMPLIFICATION ──────────────────────────────────────────
        // 
        // OLD: Expand room bounds when in door column (axis-aligned hack)
        // NEW: If inside any door's passable volume, skip collision
        
        bool in_door = false;
        for (uint32_t i = 0; i < doorCount_; ++i) {
            if (doors_[i].point_in_volume(new_x, CHECK_Y, new_z)) {
                in_door = true;
                break;
            }
        }
        
        if (!in_door) {
            // Apply room bounds
            constexpr float margin = PAWN_RADIUS + 0.1f;
            new_x = std::clamp(new_x, -ROOM_HALF_WIDTH + margin, ROOM_HALF_WIDTH - margin);
            new_z = std::clamp(new_z, -ROOM_HALF_DEPTH + margin, ROOM_HALF_DEPTH - margin);
        }
        
        cpuPawnX_ = new_x;
        cpuPawnZ_ = new_z;
    }
    
    // ─── Commitment check ────────────────────────────────────────────────────
    
    uint32_t check_commitment() {
        constexpr float CHECK_Y = PAWN_HEIGHT * 0.5f;
        
        for (uint32_t i = 0; i < doorCount_; ++i) {
            if (doors_[i].point_committed(cpuPawnX_, CHECK_Y, cpuPawnZ_)) {
                return doors_[i].target();
            }
        }
        return DoorTarget::NONE;
    }
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §7 INPUT STATE
    // ═══════════════════════════════════════════════════════════════════════════
    
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
        constexpr float sens = 0.005f;
        if (mouse_.left_dragging) {
            inputState_.look_az_delta += dx * sens;
            inputState_.look_el_delta += dy * sens;
            cpuCameraAzimuth_ += dx * sens;
        }
        if (mouse_.right_dragging) {
            inputState_.pan_x_delta += dx * sens;
            inputState_.pan_y_delta -= dy * sens;
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


} // namespace gallery
} // namespace t7
