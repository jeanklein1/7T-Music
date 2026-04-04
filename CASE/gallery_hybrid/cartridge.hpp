#pragma once

/**
 * GALLERY HYBRID CARTRIDGE
 * ========================
 *
 * Rasterized navigation hub. Walk through doors to transition to other cartridges.
 *
 * EXECUTION ORDER:
 *   1. Upload signal (CPU → GPU)
 *   2. Compute: update pawn + camera
 *   3. Render: room mesh
 *   4. Render: pawn mesh
 *   5. CPU: check door collision
 *
 * DOOR COLLISION:
 *   CPU mirrors pawn position (same movement logic as GPU).
 *   When pawn crosses commitment threshold, fires transition callback.
 */

#include "render/render_cartridge.hpp"
#include "core/input_event.hpp"
#include "cartridges/gallery/state.hpp"
#include "cartridges/gallery/renderer.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <functional>
#include <vector>

#ifndef GLFW_KEY_UP
#define GLFW_KEY_UP 265
#define GLFW_KEY_DOWN 264
#define GLFW_KEY_LEFT 263
#define GLFW_KEY_RIGHT 262
#endif

namespace t7 {
namespace gallery {


using TransitionCallback = std::function<void(uint32_t target_cartridge_id)>;

// Colors
constexpr float FOG_COLOR_R = 0.88f;
constexpr float FOG_COLOR_G = 0.90f;
constexpr float FOG_COLOR_B = 0.92f;

// Pawn
constexpr float PAWN_RADIUS = 0.5f;


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
        
        // Configure doors
        configure_doors();
        
        // Initialize GPU state with door config
        gpuState_.init(device, doors_);
        
        // Initialize CPU pawn tracking
        cpuPawnX_ = Initial::PAWN_POS_X;
        cpuPawnZ_ = Initial::PAWN_POS_Z;
        cpuCameraAzimuth_ = Initial::CAMERA_AZIMUTH;
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
        gpuSignal._pad0 = 0.0f;
        gpuSignal.move_x = inputState_.move_x;
        gpuSignal.move_z = inputState_.move_z;
        gpuSignal.look_az_delta = inputState_.look_az_delta;
        gpuSignal.look_el_delta = inputState_.look_el_delta;
        gpuSignal.zoom_delta = inputState_.zoom_delta;
        gpuSignal.pan_x_delta = inputState_.pan_x_delta;
        gpuSignal.pan_y_delta = inputState_.pan_y_delta;
        gpuSignal._pad1 = 0.0f;
        
        // Mirror pawn movement on CPU
        update_cpu_pawn_position(signal.dt);
        
        gpuState_.upload_signal(queue, gpuSignal);
        clear_input_deltas();
    }
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §2 RENDER
    // ═══════════════════════════════════════════════════════════════════════════
    
    void render(wgpu::CommandEncoder& encoder,
                wgpu::TextureView backbuffer,
                wgpu::TextureView depth) override {
        
        // ─── COMPUTE PHASE ────────────────────────────────────────────────────
        {
            wgpu::ComputePassDescriptor desc{};
            desc.label = "Gallery Compute";
            wgpu::ComputePassEncoder compute = encoder.BeginComputePass(&desc);
            
            renderer_.dispatch_update_world(
                compute,
                gpuState_.compute_bind_group()
            );
            
            compute.End();
        }
        
        // ─── RENDER PHASE ─────────────────────────────────────────────────────
        {
            wgpu::RenderPassColorAttachment colorAttachment{};
            colorAttachment.view = backbuffer;
            colorAttachment.loadOp = wgpu::LoadOp::Clear;
            colorAttachment.storeOp = wgpu::StoreOp::Store;
            colorAttachment.clearValue = { FOG_COLOR_R, FOG_COLOR_G, FOG_COLOR_B, 1.0 };
            
            wgpu::RenderPassDepthStencilAttachment depthAttachment{};
            depthAttachment.view = depth;
            depthAttachment.depthLoadOp = wgpu::LoadOp::Clear;
            depthAttachment.depthStoreOp = wgpu::StoreOp::Store;
            depthAttachment.depthClearValue = 1.0f;
            
            wgpu::RenderPassDescriptor desc{};
            desc.label = "Gallery Render";
            desc.colorAttachmentCount = 1;
            desc.colorAttachments = &colorAttachment;
            desc.depthStencilAttachment = &depthAttachment;
            
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);
            
            // Draw room
            renderer_.draw_room(
                pass,
                gpuState_.render_bind_group(),
                gpuState_.room_vertex_buffer(),
                gpuState_.room_index_buffer(),
                gpuState_.room_index_count()
            );
            
            // Draw pawn
            renderer_.draw_pawn(
                pass,
                gpuState_.render_bind_group(),
                gpuState_.pawn_vertex_buffer(),
                gpuState_.pawn_index_buffer(),
                gpuState_.pawn_index_count()
            );
            
            pass.End();
        }
    }
    
    // ─── Post-frame: Check Door Transition ───────────────────────────────────
    
    void check_door_transition() {
        uint32_t target = check_door_commitment();
        if (target != DoorTarget::NONE && transitionCallback_) {
            std::cout << "[Gallery] Pawn committed to door → target " << target << "\n";
            transitionCallback_(target);
        }
    }
    
    void reset_pawn(wgpu::Queue queue) {
        cpuPawnX_ = Initial::PAWN_POS_X;
        cpuPawnZ_ = Initial::PAWN_POS_Z;
        cpuCameraAzimuth_ = Initial::CAMERA_AZIMUTH;
        
        keys_ = KeyState{};
        inputState_ = InputState{};
        
        gpuState_.reset(queue);
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
    
    // Door configs (for collision)
    std::vector<DoorConfig> doors_;
    
    // CPU pawn tracking
    float cpuPawnX_ = 0.0f;
    float cpuPawnZ_ = 0.0f;
    float cpuCameraAzimuth_ = 0.0f;
    
    
    // ═══════════════════════════════════════════════════════════════════════════
    // §5 DOOR CONFIGURATION
    // ═══════════════════════════════════════════════════════════════════════════
    
    void configure_doors() {
        doors_.clear();
        
        // Back wall door → N-Dimensional 2
        doors_.push_back({
            { 0.0f, 0.0f, -ROOM_HALF_DEPTH },  // position
            { 0.0f, 0.0f, -1.0f },              // normal (through door, out of room)
            3.0f,                               // width
            4.0f,                               // height
            2.0f,                               // commitment_depth
            DoorTarget::N_DIMENSIONAL_2
        });
        
        // Left wall door → Playground Raymarch
        doors_.push_back({
            { -ROOM_HALF_WIDTH, 0.0f, -5.0f },
            { -1.0f, 0.0f, 0.0f },
            2.5f,
            3.5f,
            2.0f,
            DoorTarget::PLAYGROUND_RAYMARCH
        });
        
        // Left wall door → Playground Rasterize
        doors_.push_back({
            { -ROOM_HALF_WIDTH, 0.0f, 5.0f },
            { -1.0f, 0.0f, 0.0f },
            2.0f,
            3.0f,
            2.0f,
            DoorTarget::PLAYGROUND_RASTERIZE
        });
        
        // Right wall door → Playground Hybrid
        doors_.push_back({
            { ROOM_HALF_WIDTH, 0.0f, 0.0f },
            { 1.0f, 0.0f, 0.0f },
            3.0f,
            4.0f,
            2.0f,
            DoorTarget::PLAYGROUND_HYBRID
        });
    }
    
    
    // ─────────────────────────────────────────────────────────────────────────
    // INPUT STATE
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
            cpuCameraAzimuth_ += dx * sensitivity;
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
    
    
    // ─────────────────────────────────────────────────────────────────────────
    // CPU PAWN TRACKING
    // ─────────────────────────────────────────────────────────────────────────
    
    void update_cpu_pawn_position(float dt) {
        float cos_az = std::cos(cpuCameraAzimuth_);
        float sin_az = std::sin(cpuCameraAzimuth_);
        
        float world_vel_x = inputState_.move_x * cos_az + inputState_.move_z * sin_az;
        float world_vel_z = -inputState_.move_x * sin_az + inputState_.move_z * cos_az;
        
        constexpr float speed = 8.0f;
        cpuPawnX_ += world_vel_x * speed * dt;
        cpuPawnZ_ += world_vel_z * speed * dt;
        
        constexpr float margin = PAWN_RADIUS + 0.1f;
        
        // Start with default room bounds
        float minX = -ROOM_HALF_WIDTH + margin;
        float maxX = ROOM_HALF_WIDTH - margin;
        float minZ = -ROOM_HALF_DEPTH + margin;
        float maxZ = ROOM_HALF_DEPTH - margin;
        
        // Expand bounds for doors we can pass through
        for (const auto& door : doors_) {
            if (cpu_point_in_door_column(cpuPawnX_, cpuPawnZ_, door)) {
                if (std::abs(door.normal[2]) > 0.5f) {
                    if (door.normal[2] < 0) {
                        minZ = -ROOM_HALF_DEPTH - 10.0f;
                    } else {
                        maxZ = ROOM_HALF_DEPTH + 10.0f;
                    }
                } else {
                    if (door.normal[0] < 0) {
                        minX = -ROOM_HALF_WIDTH - 10.0f;
                    } else {
                        maxX = ROOM_HALF_WIDTH + 10.0f;
                    }
                }
            }
        }
        
        cpuPawnX_ = std::clamp(cpuPawnX_, minX, maxX);
        cpuPawnZ_ = std::clamp(cpuPawnZ_, minZ, maxZ);
    }
    
    
    // ─────────────────────────────────────────────────────────────────────────
    // DOOR CHECKS
    // ─────────────────────────────────────────────────────────────────────────
    
    bool cpu_point_in_door_column(float x, float z, const DoorConfig& door) const {
        float local_x;
        
        if (std::abs(door.normal[2]) > 0.5f) {
            local_x = x - door.position[0];
        } else {
            local_x = z - door.position[2];
        }
        
        float half_w = door.width * 0.5f;
        return std::abs(local_x) < half_w;
    }
    
    bool cpu_point_past_commitment(float x, float z, const DoorConfig& door) const {
        if (!cpu_point_in_door_column(x, z, door)) return false;
        
        float local_z;
        if (std::abs(door.normal[2]) > 0.5f) {
            local_z = (z - door.position[2]) * door.normal[2];
        } else {
            local_z = (x - door.position[0]) * door.normal[0];
        }
        
        return local_z > door.commitment_depth;
    }
    
    uint32_t check_door_commitment() {
        for (const auto& door : doors_) {
            if (cpu_point_past_commitment(cpuPawnX_, cpuPawnZ_, door)) {
                return door.target_id;
            }
        }
        return DoorTarget::NONE;
    }
};


} // namespace gallery
} // namespace t7
