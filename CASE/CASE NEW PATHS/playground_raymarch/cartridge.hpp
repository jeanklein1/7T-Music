#pragma once

/**
 * PLAYGROUND RAYMARCH CARTRIDGE — Render Cartridge
 * ========================================
 *
 * Minimal sculpture viewer. One static object (pawn) in void space.
 * Orbital camera with full spherical rotation.
 *
 * PURPOSE:
 *   Rapid iteration on procedural shapes. The world.wgsl §4 SUBJECT
 *   section is the creative canvas—modify SDFs there, see results
 *   immediately.
 *
 * EXECUTION ORDER:
 *   1. Upload signal (CPU → GPU)
 *   2. Compute: update_camera [0D]
 *   3. Render:  fullscreen raymarch
 *
 * NO:
 *   - Pawn movement
 *   - Musical input
 *   - Doors/transitions
 *   - Floor/walls
 *
 * See world.wgsl for the GPU scroll (single source of truth).
 */

#include "render/render_cartridge.hpp"
#include "core/input_event.hpp"
#include "cartridges/playground_raymarch/state.hpp"
#include "cartridges/playground_raymarch/renderer.hpp"
#include <cmath>
#include <iostream>

namespace t7 {
    namespace playground_raymarch {


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
            // §1 LIFECYCLE
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
                    gpuState_.compute_bind_group_layout(),
                    gpuState_.render_bind_group_layout(),
                    colorFormat,
                    depthFormat
                );
            }

            void update(const AnalysisSignal& signal,
                float aspect_ratio,
                wgpu::Queue& queue) override {

                // ─── Build GPU signal ─────────────────────────────────────────────────
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

                // ─── Upload to GPU ────────────────────────────────────────────────────
                gpuState_.upload_signal(queue, gpuSignal);

                // ─── Clear deltas for next frame ──────────────────────────────────────
                clear_input_deltas();
            }


            // ═══════════════════════════════════════════════════════════════════════════
            // §2 COMPOSE — Execution order
            // ═══════════════════════════════════════════════════════════════════════════

            void render(wgpu::CommandEncoder& encoder,
                wgpu::TextureView backbuffer,
                wgpu::TextureView depth) override {

                // ─── COMPUTE PHASE ────────────────────────────────────────────────────
                    {
                        wgpu::ComputePassDescriptor desc{};
                        desc.label = "Playground Raymarch Compute Phase";
                        wgpu::ComputePassEncoder compute = encoder.BeginComputePass(&desc);

                        renderer_.dispatch_update_camera(
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
                        colorAttachment.clearValue = { 0.02, 0.02, 0.03, 1.0 };  // Near-black void

                        wgpu::RenderPassDescriptor desc{};
                        desc.label = "Playground Raymarch";
                        desc.colorAttachmentCount = 1;
                        desc.colorAttachments = &colorAttachment;

                        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);

                        renderer_.draw_world(
                            pass,
                            gpuState_.render_bind_group()
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
                    break;  // Ignore keyboard (no movement)
                }
            }


            // ═══════════════════════════════════════════════════════════════════════════
            // §4 PROPERTIES
            // ═══════════════════════════════════════════════════════════════════════════

            void get_clear_color(float& r, float& g, float& b) const override {
                r = 0.02f;
                g = 0.02f;
                b = 0.03f;
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
            // INPUT STATE
            // ─────────────────────────────────────────────────────────────────────────

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


            // ─────────────────────────────────────────────────────────────────────────
            // INPUT HANDLERS
            // ─────────────────────────────────────────────────────────────────────────

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
        };


    } // namespace playground_raymarch
} // namespace t7