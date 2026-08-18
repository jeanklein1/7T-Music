#pragma once

// ─── render_cartridge.hpp ────────────────────────────────────────
//
// Interface for visualization modules. A render cartridge transforms
// musical statistics into visuals; it owns the visual interpretation —
// what shapes to draw, how stats map to movement, what the camera does.
//
// The categorical boundary: the render cartridge knows about
// visualization (terrain, animating a pawn, orbital cameras), but NOT
// about music. When it reads stats[0] it doesn't know that's
// "polyphony" — only "when this rises, terrain breathes."
//
// Lifecycle:
//   1. Console creates cartridge
//   2. Console calls initialize(device)
//   3. Per frame:
//      a. Console routes input events via on_input()
//      b. Console calls update(signal, aspect_ratio)
//      c. Console calls render(encoder, backbuffer, depth)
//
// What the cartridge owns: GPU buffers (state, trajectories, etc.),
// pipelines (compute, render), meshes, input state (key/mouse), and the
// mapping of stats to visual parameters.
//
// Depends on: analysis/analysis_signal.hpp, core/input_event.hpp,
//             core/cartridge_ids.hpp, <webgpu/webgpu_cpp.h>.

#include "analysis/analysis_signal.hpp"
#include "core/input_event.hpp"
#include "core/cartridge_ids.hpp"
#include <webgpu/webgpu_cpp.h>

namespace t7 {

// ═══ RENDER CARTRIDGE INTERFACE ══════════════════════════════════

class RenderCartridge {
public:
    virtual ~RenderCartridge() = default;
    
    // â”€â”€â”€ LIFECYCLE â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    
    /**
     * Initialize the cartridge.
     * Called once after construction.
     * 
     * @param device  WebGPU device for resource creation
     */
    virtual void initialize(wgpu::Device device) = 0;
    
    /**
     * Update the cartridge state.
     * Called once per frame before render.
     * 
     * This is where:
     * - Analysis signal is uploaded to GPU
     * - Input state is processed and uploaded
     * - upload inputs / prep CPU state 
     * 
     * @param signal        Musical analysis output
     * @param aspect_ratio  Window width / height (for projection)
     * @param queue         GPU queue for buffer uploads
     */
    virtual void update(const AnalysisSignal& signal, 
                        float aspect_ratio,
                        wgpu::Queue& queue) = 0;
    
    /**
     * Render the visualization.
     * Called once per frame after update.
     * 
     * @param encoder    Command encoder for recording draw commands
     * @param backbuffer Render target (surface texture view)
     * @param depth      Depth buffer view
     */
    // DOMESDAY_2 B10: msaaColor is null at msaa=1 (render straight into
    // the backbuffer); at msaa=4 it is the multisampled target and the
    // backbuffer becomes the resolve destination.
    virtual void render(wgpu::CommandEncoder& encoder,
                        wgpu::TextureView backbuffer,
                        wgpu::TextureView msaaColor,
                        wgpu::TextureView depth) = 0;
    
    // â”€â”€â”€ INPUT â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    
    /**
     * Handle an input event.
     * The cartridge decides which events it cares about 
     * (e.g., movement keys, mouse for camera).
     * 
     * @param event  The input event
     */
    virtual void on_input(const InputEvent& event) = 0;
    
    // ── Deferred Transitions ─────────────────────────────────────
    // Cartridges request state transitions instead of executing callbacks.
    // This allows the state machine to complete cleanly before transition.
    
    /**
     * Check if this cartridge is requesting a transition.
     * Returns the target cartridge ID, or CartridgeId::NONE if no transition.
     * Clears the pending request after returning.
     * 
     * Default: no transition (returns CartridgeId::NONE).
     * Override in cartridges with door systems.
     * 
     * @return Target cartridge ID, or CartridgeId::NONE
     */
    virtual uint32_t get_pending_transition() {
        return CartridgeId::NONE;
    }
    
    /**
     * Reset pawn position and state to safe defaults.
     * Called after a cartridge transition to ensure clean initial state.
     * Grace period for door detection should be set here.
     * 
     * Default: no-op.
     * Override in cartridges with pawn/door systems.
     * 
     * @param queue  GPU queue for buffer uploads
     */
    virtual void reset_pawn(wgpu::Queue queue) {
        // Default: no-op
    }

    /**
     * Determine if BACKSPACE should return to previous room.
     * 
     * Return true ONLY if this room has no exit doors and the user needs
     * a way to escape (e.g., a dead-end exploration room).
     * 
     * Return false if this room has exit doors (user can navigate normally).
     * 
     * Default: false (most cartridges have doors or don't support BACKSPACE)
     * 
     * @return true if BACKSPACE should work as "return to previous room"
     */
    virtual bool supports_backspace() const {
        return false;  // Default: BACKSPACE disabled (has exit doors or not applicable)
    }

    
    // â”€â”€â”€ PROPERTIES â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    
    /**
     * Get the clear color for this world.
     * Console uses this to clear the framebuffer before rendering.
     * 
     * @param r, g, b  Output: RGB components (0.0 to 1.0)
     */
    virtual void get_clear_color(float& r, float& g, float& b) const = 0;
    
    /**
     * Get the depth format this cartridge expects.
     * Default: Depth24Plus
     */
    virtual wgpu::TextureFormat depth_format() const {
        return wgpu::TextureFormat::Depth24Plus;
    }
    
    // ── Hot Reload (Pawn harness support) ────────────────────────
    
    /**
     * Reload shaders from disk and recreate pipelines.
     * Called by the pawn harness when shader file changes.
     * 
     * Default: no-op (returns false).
     * Override in cartridges that support hot-reload.
     * 
     * @return true if reload succeeded
     */
    virtual bool reload_shaders() { return false; }
};

} // namespace t7
