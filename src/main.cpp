/**
 * MAIN - 7T Visualizer Entry Point
 * =================================
 *
 * Minimal entry point. All cartridge and transition logic
 * lives in CartridgeManager.
 *
 * Controls:
 *   Arrows     = Move pawn
 *   Mouse      = Camera orbit
 *   ASDF/ZXCV  = Piano keys
 *   Backspace  = Return to hub (Gallery Raymarch)
 *   F1-F4      = Development shortcuts
 *
 * IMPORTANT: After any transition (keyboard or door-based), we must
 * re-fetch the active cartridge before calling update() or render().
 */

#include "console/console.hpp"
#include "analysis/polyphony_basic/canvas.hpp"
#include "core/cartridge_manager.hpp"

#include <iostream>
#include <string>
#include <GLFW/glfw3.h>

static bool is_music_key(int key) {
    if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) return true;
    if (key == GLFW_KEY_SEMICOLON) return true;
    if (key == GLFW_KEY_LEFT_BRACKET) return true;
    if (key == GLFW_KEY_RIGHT_BRACKET) return true;
    return false;
}


int main(int argc, char* argv[]) {
    std::string midi_path;
    if (argc > 1) midi_path = argv[1];

    // ─── Console ─────────────────────────────────────────────────────────
    t7::Console console;
    if (!console.init("7T Visualizer (WebGPU)", 1280, 720)) {
        return 1;
    }

    // ─── Analysis ────────────────────────────────────────────────────────
    t7::polyphony_basic::Canvas analysis;
    analysis.initialize("assets");
    if (!midi_path.empty()) {
        if (analysis.load_midi(midi_path.c_str())) {
            std::cout << "Loaded MIDI: " << midi_path << "\n";
        }
    }

    // ─── Cartridge Manager ───────────────────────────────────────────────
    t7::CartridgeManager cartridges;
    if (!cartridges.init(console.device(), console.color_format(), console.depth_format())) {
        std::cerr << "Failed to initialize cartridge system\n";
        return 1;
    }

    std::cout << "\n7T Visualizer (WebGPU)\n";
    std::cout << "Controls: Arrows=move, Mouse=camera, ASDF/ZXCV=piano\n";
    std::cout << "          Backspace=Hub, F1-F4=Dev shortcuts\n\n";

    // ─── Main Loop ───────────────────────────────────────────────────────
    while (console.running()) {
        float dt = console.begin_frame();

        // ─── Input ───────────────────────────────────────────────────────
        // Get current renderer for input routing
        t7::RenderCartridge* renderer = cartridges.active();
        
        for (const auto& event : console.input_events()) {
            if (event.type == t7::InputEvent::Type::KeyDown ||
                event.type == t7::InputEvent::Type::KeyUp) {

                // Global shortcuts (KeyDown only)
                if (event.type == t7::InputEvent::Type::KeyDown) {
                    if (event.key == GLFW_KEY_BACKSPACE) {
                        // Only enable BACKSPACE in rooms without exit doors
                        if (renderer->supports_backspace()) {
                            cartridges.return_to_previous();
                            renderer = cartridges.active();  // Re-fetch after transition
                        }
                        // Otherwise: do nothing (room has exit doors, use them)
                        continue;
                    }
                    if (event.key == GLFW_KEY_F1) {
                        cartridges.transition_to(t7::CartridgeId::WORLD_COMPUTE);
                        renderer = cartridges.active();  // Re-fetch after transition
                        continue;
                    }
                    if (event.key == GLFW_KEY_F2) {
                        cartridges.transition_to(t7::CartridgeId::PAWN_RASTERIZE);
                        renderer = cartridges.active();  // Re-fetch after transition
                        continue;
                    }
                    if (event.key == GLFW_KEY_F3) {
                        cartridges.transition_to(t7::CartridgeId::PLAYGROUND_HYBRID);
                        renderer = cartridges.active();  // Re-fetch after transition
                        continue;
                    }
                    if (event.key == GLFW_KEY_F4) {
                        cartridges.transition_to(t7::CartridgeId::SPECIES_STUDIO);
                        renderer = cartridges.active();  // Re-fetch after transition
                        continue;
                    }
                }

                // Route to analysis or renderer
                if (is_music_key(event.key)) {
                    analysis.on_input(event);
                } else {
                    renderer->on_input(event);
                }
            } else {
                renderer->on_input(event);
            }
        }
        console.clear_input_events();

        // ──────────────────────────────────────────────────────────────────────────
        // Update
        // ──────────────────────────────────────────────────────────────────────────
        analysis.update(dt);
        wgpu::Queue queue = console.queue();
        
        // CRITICAL: Re-fetch active before update (in case input triggered transition)
        renderer = cartridges.active();
        renderer->update(analysis.output(), console.aspect_ratio(), queue);
        
        // ─── Check for pending DOOR transition from cartridge ───────────────
        // Cartridges REQUEST door transitions via pending_transition_ flag.
        // This allows update() to complete cleanly (deferred transitions).
        // 
        // IMPORTANT: Mark isDoorTransition=true so keyboard shortcuts (F1-F4)
        // don't overwrite the "previous room" saved by door transitions.
        // This way: Door → F1 → BACKSPACE returns to the door's source,
        // not to the F1 destination.
        uint32_t pending = renderer->get_pending_transition();
        if (pending != t7::CartridgeId::NONE) {
            std::cout << "[MAIN] Executing pending transition → " << pending << "\n";
            cartridges.transition_from(cartridges.active_id(), pending, true);  // isDoorTransition = true
            renderer = cartridges.active();
            renderer->reset_pawn(queue);  // Clean state for new cartridge
        }
        
        // CRITICAL: Re-fetch active after potential transition
        renderer = cartridges.active();
        // ─── Render ──────────────────────────────────────────────────────
        if (!console.acquire_surface_texture()) continue;

        wgpu::CommandEncoderDescriptor encDesc{};
        wgpu::CommandEncoder encoder = console.device().CreateCommandEncoder(&encDesc);
        renderer->render(encoder, console.backbuffer(), console.depth_view());

        wgpu::CommandBufferDescriptor cmdDesc{};
        wgpu::CommandBuffer commands = encoder.Finish(&cmdDesc);
        queue.Submit(1, &commands);

        console.present();
    }

    return 0;
}