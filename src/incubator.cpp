/**
 * INCUBATOR — Fast Cartridge Development Harness
 * ================================================
 *
 * Minimal runtime for developing individual cartridges.
 * Change the INCUBATE define → rebuild → see results in seconds.
 *
 * USAGE:
 *   1. Change INCUBATE below to your target cartridge folder name
 *   2. Build: cmake --build . --target incubator
 *   3. Run, iterate, repeat
 *
 * CONVENTION:
 *   For this to work, cartridges must follow the naming convention:
 *
 *   | Folder name          | Namespace             | Class     |
 *   |----------------------|-----------------------|-----------|
 *   | species_studio/      | t7::species_studio    | Cartridge |
 *   | playground_raymarch/ | t7::playground_raymarch| Cartridge |
 *
 *   Folder name must match namespace name exactly.
 */

// ═══════════════════════════════════════════════════════════════════════════════
// TARGET SELECTION — Change this line to switch cartridges
// ═══════════════════════════════════════════════════════════════════════════════

#define INCUBATE gallery

// ═══════════════════════════════════════════════════════════════════════════════
// MACRO MACHINERY — Builds include path and namespace from INCUBATE
// ═══════════════════════════════════════════════════════════════════════════════

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x

#define CONCAT(a, b) CONCAT2(a, b)
#define CONCAT2(a, b) a ## b

#define CARTRIDGE_HEADER(name) STRINGIFY(cartridges/name/cartridge.hpp)

// ═══════════════════════════════════════════════════════════════════════════════
// INCLUDES
// ═══════════════════════════════════════════════════════════════════════════════

#include "console/console.hpp"
#include "analysis/analysis_signal.hpp"

// This expands to: #include "cartridges/species_studio/cartridge.hpp"
#include CARTRIDGE_HEADER(INCUBATE)

#include <iostream>
#include <GLFW/glfw3.h>

// ═══════════════════════════════════════════════════════════════════════════════
// ACTIVE CARTRIDGE TYPE — Derived from INCUBATE
// ═══════════════════════════════════════════════════════════════════════════════

// This expands to: namespace active = t7::species_studio;
namespace active = t7::INCUBATE;

using ActiveCartridge = active::Cartridge;

// Cartridge name for display
constexpr const char* CARTRIDGE_NAME = STRINGIFY(INCUBATE);

// ═══════════════════════════════════════════════════════════════════════════════
// MINIMAL ANALYSIS — Just enough to drive the cartridge
// ═══════════════════════════════════════════════════════════════════════════════

class MinimalAnalysis {
public:
    MinimalAnalysis() {
        signal_.clear_stats();
    }
    
    void update(float dt) {
        signal_.t_seconds += dt;
        signal_.dt = dt;
        signal_.t_beats += dt * 2.0f;  // ~120 BPM
    }
    
    const t7::AnalysisSignal& output() const { return signal_; }
    
private:
    t7::AnalysisSignal signal_{};
};

// ═══════════════════════════════════════════════════════════════════════════════
// MAIN
// ═══════════════════════════════════════════════════════════════════════════════

int main() {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  INCUBATOR\n";
    std::cout << "  Target: " << CARTRIDGE_NAME << "\n";
    std::cout << "========================================\n";
    std::cout << "\n";

    // ─── Initialize Console ─────────────────────────────────────────────────
    t7::Console console;
    if (!console.init("Incubator", 1280, 720)) {
        std::cerr << "Failed to initialize console\n";
        return 1;
    }

    // ─── Initialize Cartridge ───────────────────────────────────────────────
    ActiveCartridge cartridge;
    cartridge.initialize(console.device());
    
    if (!cartridge.init_renderer(console.color_format(), console.depth_format())) {
        std::cerr << "Failed to initialize " << CARTRIDGE_NAME << " renderer\n";
        return 1;
    }
    
    std::cout << "[Incubator] " << CARTRIDGE_NAME << " ready\n\n";

    // ─── Minimal Analysis ───────────────────────────────────────────────────
    MinimalAnalysis analysis;

    // ─── Main Loop ──────────────────────────────────────────────────────────
    while (console.running()) {
        float dt = console.begin_frame();
        
        // ─── Input ──────────────────────────────────────────────────────────
        for (const auto& event : console.input_events()) {
            cartridge.on_input(event);
        }
        console.clear_input_events();
        
        // ─── Update ─────────────────────────────────────────────────────────
        analysis.update(dt);
        wgpu::Queue queue = console.queue();
        cartridge.update(analysis.output(), console.aspect_ratio(), queue);
        
        // ─── Render ─────────────────────────────────────────────────────────
        if (!console.acquire_surface_texture()) {
            continue;
        }
        
        wgpu::CommandEncoderDescriptor encDesc{};
        wgpu::CommandEncoder encoder = console.device().CreateCommandEncoder(&encDesc);
        
        cartridge.render(encoder, console.backbuffer(), console.depth_view());
        
        wgpu::CommandBufferDescriptor cmdDesc{};
        wgpu::CommandBuffer commands = encoder.Finish(&cmdDesc);
        queue.Submit(1, &commands);
        
        console.present();
    }

    std::cout << "[Incubator] Shutdown\n";
    return 0;
}
