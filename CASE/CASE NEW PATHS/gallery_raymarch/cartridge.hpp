#pragma once

/**
 * GALLERY_RAYMARCH CARTRIDGE — Render Cartridge
 * ==============================================
 *
 * Precomputed room field for fast raymarching.
 *
 * DOOR TRANSITION FIX (v6) — Deferred Transition Model:
 *   
 *   KEY INSIGHT: Firing transitions during update() creates corruption
 *   when chaining transitions (Gallery→Gallery_Raymarch→Gallery).
 *   The old cartridge's state machine is mid-execution when the
 *   CartridgeManager switches the active cartridge.
 *   
 *   SOLUTION: Transitions are now REQUESTED, not executed.
 *   
 *   Flow:
 *     1. update_door_states() detects commitment
 *     2. Sets pending_transition_ = target ID
 *     3. Returns normally (state machine completes)
 *     4. main.cpp checks get_pending_transition() AFTER update completes
 *     5. main.cpp calls CartridgeManager to execute transition
 *     6. New cartridge gets clean state on next frame
 *   
 *   This eliminates mid-update state corruption and simplifies the
 *   cartridge interface (no callbacks needed).
 *   
 *   State machine per door:
 *     IDLE     → (pawn crosses commitment) → CROSSING
 *     CROSSING → (pawn exits door volume)  → COOLDOWN  
 *     COOLDOWN → (pawn reaches safe zone)  → IDLE
 *
 * See world.wgsl for the GPU scroll.
 */

#include "render/render_cartridge.hpp"
#include "core/input_event.hpp"
#include "cartridges/gallery_raymarch/state.hpp"
#include "cartridges/gallery_raymarch/renderer.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

#ifndef GLFW_KEY_UP
#define GLFW_KEY_UP 265
#define GLFW_KEY_DOWN 264
#define GLFW_KEY_LEFT 263
#define GLFW_KEY_RIGHT 262
#endif

namespace t7 {
    namespace gallery_raymarch {
        // ═══════════════════════════════════════════════════════════════════════════
        // DOOR STATE MACHINE
        // ═══════════════════════════════════════════════════════════════════════════
        //
        // Three phases per door:
        //   IDLE     - Ready to trigger. Watching for commitment crossing.
        //   CROSSING - Transition fired (one-shot). Waiting to exit door volume.
        //   COOLDOWN - Exited door. Waiting to reach safe zone before rearming.
        //

        enum class DoorPhase {
            IDLE,       // Ready to trigger
            CROSSING,   // Transition fired, waiting to exit door
            COOLDOWN    // Waiting for pawn to reach safe zone
        };

        inline const char* door_phase_name(DoorPhase p) {
            switch (p) {
                case DoorPhase::IDLE: return "IDLE";
                case DoorPhase::CROSSING: return "CROSSING";
                case DoorPhase::COOLDOWN: return "COOLDOWN";
                default: return "?";
            }
        }


        class Cartridge : public RenderCartridge {
        public:
            Cartridge() = default;

            Cartridge(const Cartridge&) = delete;
            Cartridge& operator=(const Cartridge&) = delete;

            // ─── Transition Request ──────────────────────────────────────────────
            // Check if this cartridge is requesting a transition.
            // Returns the target cartridge ID, or CartridgeId::NONE if no transition.
            // Clears the pending request after returning.
            uint32_t get_pending_transition() {
                uint32_t result = pending_transition_;
                pending_transition_ = CartridgeId::NONE;
                return result;
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

                // All doors start in COOLDOWN with grace period
                for (uint32_t i = 0; i < MAX_DOORS; ++i) {
                    doorPhase_[i] = DoorPhase::COOLDOWN;
                }
                doorGraceFrames_ = 5;  // Initial grace period

                // Mark field dirty to compute on first frame
                gpuState_.mark_field_dirty();
            }

            bool init_renderer(
                wgpu::TextureFormat colorFormat,
                wgpu::TextureFormat depthFormat
            ) {
                colorFormat_ = colorFormat;
                depthFormat_ = depthFormat;

                return renderer_.init(
                    device_,
                    gpuState_.compute_buffer_layout(),
                    gpuState_.compute_texture_layout(),
                    gpuState_.render_buffer_layout(),
                    gpuState_.render_texture_layout(),
                    colorFormat
                );
            }

            // ─────────────────────────────────────────────────────────────────────────
            // UPDATE — All logic happens here, atomically
            // ─────────────────────────────────────────────────────────────────────────
            //
            // Order of operations:
            //   1. Update pawn position
            //   2. Update door state machines (may fire transition callback)
            //   3. Upload GPU state
            //
            // The transition callback fires from WITHIN this call, immediately after
            // the pawn crosses commitment. No external timing dependencies.
            //

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

                // 1. Update pawn position
                update_pawn(signal.dt);
                
                // 2. Update door state machines — transition fires here if triggered
                update_door_states();

                // 3. Upload to GPU
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

                // ─── COMPUTE PASS ───────────────────────────────────────────────────
                    {
                        wgpu::ComputePassDescriptor desc{};
                        desc.label = "Gallery Compute";
                        wgpu::ComputePassEncoder compute = encoder.BeginComputePass(&desc);

                        // 1. Update world (pawn/camera) - always
                        renderer_.dispatch_update_world(
                            compute,
                            gpuState_.compute_buffer_bind_group(),
                            gpuState_.compute_texture_bind_group()
                        );

                        // 2. Update room field - only when doors change
                        if (gpuState_.needs_field_update()) {
                            renderer_.dispatch_update_room_field(
                                compute,
                                gpuState_.compute_buffer_bind_group(),
                                gpuState_.compute_texture_bind_group(),
                                gpuState_.room_field_workgroups()
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
                        colorAttachment.clearValue = { FOG_COLOR_R, FOG_COLOR_G, FOG_COLOR_B, 1.0 };

                        wgpu::RenderPassDescriptor desc{};
                        desc.label = "Gallery Raymarch";
                        desc.colorAttachmentCount = 1;
                        desc.colorAttachments = &colorAttachment;

                        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);
                        renderer_.draw_world(
                            pass,
                            gpuState_.render_buffer_bind_group(),
                            gpuState_.render_texture_bind_group()
                        );
                        pass.End();
                    }
            }

            // DEPRECATED: Transition check now happens inside update()
            // Kept for API compatibility but does nothing
            void check_door_transition() {
                // No-op: transitions now fire from within update()
            }

            void reset_pawn(wgpu::Queue queue) {
                std::cout << "[GR RESET] pawn=(" << Initial::PAWN_POS_X << "," << Initial::PAWN_POS_Z << ")\n";
                
                cpuPawnX_ = Initial::PAWN_POS_X;
                cpuPawnZ_ = Initial::PAWN_POS_Z;
                cpuCameraAzimuth_ = Initial::CAMERA_AZIMUTH;
                keys_ = {};
                inputState_ = {};
                
                // All doors to COOLDOWN — they'll transition to IDLE once
                // pawn reaches safe zone (which it will be at initial position)
                for (uint32_t i = 0; i < MAX_DOORS; ++i) {
                    doorPhase_[i] = DoorPhase::COOLDOWN;
                }
                
                // CRITICAL: Set grace period to skip door checks
                doorGraceFrames_ = 5;
                
                gpuState_.reset_pawn(queue);
            }

 /*           void spawn_pawn(float x, float z, float heading, wgpu::Queue queue) {*/

            void spawn_pawn(float x, float z, float heading, wgpu::Queue queue) {
                std::cout << "[GR SPAWN] pos=(" << x << "," << z << ") heading=" << heading << "\n";
                
                cpuPawnX_ = x;
                cpuPawnZ_ = z;
                cpuCameraAzimuth_ = heading;
                keys_ = {};
                inputState_ = {};
                
                for (uint32_t i = 0; i < MAX_DOORS; ++i) {
                    doorPhase_[i] = DoorPhase::COOLDOWN;
                }
                
                // CRITICAL: Set grace period to skip door checks
                doorGraceFrames_ = 5;
                
                gpuState_.set_pawn_position(x, z, heading, queue);
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
            uint32_t pending_transition_ = CartridgeId::NONE;

            float cpuPawnX_ = 0.0f;
            float cpuPawnZ_ = 0.0f;
            float cpuCameraAzimuth_ = 0.0f;

            PortableDoor doors_[MAX_DOORS];
            uint32_t doorCount_ = 0;
            
            // State machine per door
            DoorPhase doorPhase_[MAX_DOORS] = {};
            
            // Grace period - skip door processing for N frames after spawn/reset
            uint32_t doorGraceFrames_ = 0;


            // ═══════════════════════════════════════════════════════════════════════════
            // §5 DOOR CONFIGURATION — Explicit portable door placement
            // ═══════════════════════════════════════════════════════════════════════════

            void configure_doors() {
                doorCount_ = 0;

                // Wall positions (room bounds)
                constexpr float BACK = -ROOM_HALF_DEPTH;  // Z = -15
                constexpr float FRONT = ROOM_HALF_DEPTH;  // Z = +15
                constexpr float LEFT = -ROOM_HALF_WIDTH;  // X = -15
                constexpr float RIGHT = ROOM_HALF_WIDTH;  // X = +15

                // ─── Door 0: N-Dimensional 2 (back wall) ──────────────────────────────
                //     ARCH — elegant rounded top
                //
                doors_[doorCount_++] = PortableDoorBuilder()
                    .at(0.0f, 0.0f, BACK)
                    .facing(0.0f, 0.0f, -1.0f)
                    .shape(DoorShape::ARCH)
                    .size(5.0f, 4.5f)
                    .target(DoorTarget::N_DIMENSIONAL_2)
                    .color(0.9f, 0.8f, 0.6f)
                    .build();

                // ─── Door 1: World Compute (left wall, back) ──────────────────────────
                //     RECTANGLE — clean industrial
                //
                doors_[doorCount_++] = PortableDoorBuilder()
                    .at(LEFT, 0.0f, -5.0f)
                    .facing(-1.0f, 0.0f, 0.0f)
                    .shape(DoorShape::RECTANGLE)
                    .size(4.0f, 4.0f)
                    .target(DoorTarget::WORLD_COMPUTE)
                    .color(0.6f, 0.85f, 0.9f)
                    .build();

                // ─── Door 2: Terrain Pawn (left wall, front) ─────────────────────────
                //     RECTANGLE — clean industrial
                //
                doors_[doorCount_++] = PortableDoorBuilder()
                    .at(LEFT, 0.0f, 5.0f)
                    .facing(-1.0f, 0.0f, 0.0f)
                    .shape(DoorShape::RECTANGLE)
                    .size(4.0f, 4.0f)
                    .target(DoorTarget::TERRAIN_PAWN)
                    .color(0.6f, 0.9f, 0.7f)
                    .build();

                // ─── Door 3: Playground Hybrid (right wall) ───────────────────────────
                //     ARCH — elegant rounded top
                //
                doors_[doorCount_++] = PortableDoorBuilder()
                    .at(RIGHT, 0.0f, 0.0f)
                    .facing(1.0f, 0.0f, 0.0f)
                    .shape(DoorShape::ARCH)
                    .size(5.0f, 4.5f)
                    .target(DoorTarget::GALLERY)
                    .color(0.9f, 0.7f, 0.8f)
                    .build();
            
               

            }
            
            // ═══════════════════════════════════════════════════════════════════════════
            // §6 PAWN MOVEMENT
            // ═══════════════════════════════════════════════════════════════════════════

            void update_pawn(float dt) {
                float cos_az = std::cos(cpuCameraAzimuth_);
                float sin_az = std::sin(cpuCameraAzimuth_);
                float world_vx = inputState_.move_x * cos_az + inputState_.move_z * sin_az;
                float world_vz = -inputState_.move_x * sin_az + inputState_.move_z * cos_az;

                constexpr float SPEED = 8.0f;
                float new_x = cpuPawnX_ + world_vx * SPEED * dt;
                float new_z = cpuPawnZ_ + world_vz * SPEED * dt;

                // Check if in any door volume using actual door geometry
                constexpr float CHECK_Y = PAWN_HEIGHT * 0.5f;
                bool in_door = false;
                
                for (uint32_t i = 0; i < doorCount_; ++i) {
                    if (doors_[i].point_in_volume(new_x, CHECK_Y, new_z)) {
                        in_door = true;
                        break;
                    }
                }

                if (!in_door) {
                    constexpr float margin = PAWN_RADIUS + 0.1f;
                    new_x = std::clamp(new_x, -ROOM_HALF_WIDTH + margin, ROOM_HALF_WIDTH - margin);
                    new_z = std::clamp(new_z, -ROOM_HALF_DEPTH + margin, ROOM_HALF_DEPTH - margin);
                }

                cpuPawnX_ = new_x;
                cpuPawnZ_ = new_z;
            }


            // ═══════════════════════════════════════════════════════════════════════════
            // §6b DOOR STATE MACHINE — Called from update(), fires callback atomically
            // ═══════════════════════════════════════════════════════════════════════════
            //
            // For each door, the state machine:
            //
            //   IDLE:     Check if pawn committed → fire callback, go to CROSSING
            //   CROSSING: Check if pawn exited door volume → go to COOLDOWN
            //   COOLDOWN: Check if pawn reached safe zone → go to IDLE
            //
            // The transition callback fires ONE TIME when crossing from IDLE to CROSSING.
            // After that, the door is locked until pawn fully exits and reaches safe zone.
            //

            // Check if pawn is in the "safe zone" for rearming
            // Safe zone = well inside room, at least SAFE_MARGIN away from the door's wall
            bool pawn_in_safe_zone(uint32_t door_index) const {
                constexpr float SAFE_MARGIN = 3.0f;
                
                const auto& door = doors_[door_index];
                float fx = door.gpu.forward[0];
                float fz = door.gpu.forward[2];
                
                if (std::abs(fz) > 0.5f) {
                    // Z-axis wall (back or front)
                    float wall_z = door.gpu.position[2];
                    if (fz < 0) {
                        return cpuPawnZ_ > wall_z + SAFE_MARGIN;  // Back wall
                    } else {
                        return cpuPawnZ_ < wall_z - SAFE_MARGIN;  // Front wall
                    }
                } else {
                    // X-axis wall (left or right)
                    float wall_x = door.gpu.position[0];
                    if (fx < 0) {
                        return cpuPawnX_ > wall_x + SAFE_MARGIN;  // Left wall
                    } else {
                        return cpuPawnX_ < wall_x - SAFE_MARGIN;  // Right wall
                    }
                }
            }

            void update_door_states() {
                // Grace period - skip ALL door processing for a few frames after spawn/reset
                if (doorGraceFrames_ > 0) {
                    doorGraceFrames_--;
                    std::cout << "[GR GRACE] " << doorGraceFrames_ << " frames remaining\n";
                    return;
                }
                
                constexpr float CHECK_Y = PAWN_HEIGHT * 0.5f;
                
                for (uint32_t i = 0; i < doorCount_; ++i) {
                    bool in_volume = doors_[i].point_in_volume(cpuPawnX_, CHECK_Y, cpuPawnZ_);
                    bool committed = doors_[i].point_committed(cpuPawnX_, CHECK_Y, cpuPawnZ_);
                    bool in_safe = pawn_in_safe_zone(i);
                    
                    switch (doorPhase_[i]) {
                        
                        case DoorPhase::IDLE: {
                            if (committed) {
                                std::cout << "[GR Door " << i << "] IDLE → CROSSING at ("
                                        << cpuPawnX_ << ", " << cpuPawnZ_ << ")\n";
                                doorPhase_[i] = DoorPhase::CROSSING;
                                
                                uint32_t target = doors_[i].target();
                                std::cout << "[GR] Requesting transition → " << target << "\n";
                                pending_transition_ = target;
                                
                                // CRITICAL: Stop processing doors after requesting transition!
                                // Let main.cpp execute the transition after update completes.
                                return;
                            }
                            break;
                        }
                        
                        case DoorPhase::CROSSING: {
                            if (!in_volume) {
                                std::cout << "[GR Door " << i << "] CROSSING → COOLDOWN\n";
                                doorPhase_[i] = DoorPhase::COOLDOWN;
                            }
                            break;
                        }
                        
                        case DoorPhase::COOLDOWN: {
                            if (in_safe) {
                                std::cout << "[GR Door " << i << "] COOLDOWN → IDLE\n";
                                doorPhase_[i] = DoorPhase::IDLE;
                            }
                            break;
                        }
                    }
                }
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


    } // namespace gallery_raymarch
} // namespace t7