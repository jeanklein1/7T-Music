/**
 * INCUBATOR DUAL -- Render Cartridge Development Harness
 * ======================================================
 *
 * Minimal runtime for the render cartridge. Cartridge selection is
 * controlled from CMakeLists.txt:
 *
 *   set(INCUBATOR_DUAL_RENDER_CARTRIDGE "the_board")
 *
 * CMake passes it as a compile definition (INCUBATE_RENDER).
 * No need to edit this file to switch cartridges.
 *
 * CONVENTION:
 *   Render cartridges:
 *   | Folder name          | Namespace              | Class     |
 *   |----------------------|------------------------|-----------|
 *   | the_board/     | t7::the_board    | Cartridge |
 *
 * The analysis side is the BeatClock (CUT_1c, ruling R7): advancing
 * clocks at a variable BPM, empty stat layout. All input goes to the
 * render cartridge.
 */

 // =========================================================================
 // TARGET SELECTION -- Provided by CMake, with fallback for manual override
 // =========================================================================

#ifndef INCUBATE_RENDER
#define INCUBATE_RENDER the_board
#endif

// =========================================================================
// MACRO MACHINERY -- Builds include paths and namespaces from defines
// =========================================================================

#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x

#define CONCAT(a, b) CONCAT2(a, b)
#define CONCAT2(a, b) a ## b

#define RENDER_HEADER(name)   STRINGIFY(cartridges/name/cartridge.hpp)

// =========================================================================
// INCLUDES
// =========================================================================

#include "console/console.hpp"
#include "analysis/beat_clock.hpp"

// IntelliSense cannot resolve macro-expanded #include paths.
// This literal include gives VS navigation (Peek Definition, Go To, etc.).
// The compiler ignores it -- the macro include below pulls in the same file.
#if defined(__INTELLISENSE__)
#include "cartridges/the_board/cartridge.hpp"
#else
#include RENDER_HEADER(INCUBATE_RENDER)
#endif

#include "core/instruments.hpp"   // THE INSTRUMENTS DIAL: INSTRUMENTS.watcher_ticks gates the hot-reload progress dot
#include <iostream>
#include <filesystem>
#include <system_error>   // std::error_code — the watcher's non-throwing stat
#include <chrono>

// =========================================================================
// FILE WATCHER -- Detects shader file changes for hot reload
// =========================================================================

class FileWatcher {
public:
    void watch(const std::string& path) {
        path_ = path;
        if (std::filesystem::exists(path_)) {
            lastWriteTime_ = std::filesystem::last_write_time(path_);
        }
    }

    // ONE stat per check, not two. exists() + last_write_time() was two
    // filesystem round-trips on the render thread, both of the throwing
    // overload, ~2×/s forever; the error_code overload answers "gone" and
    // "unchanged" from the same call. Same behaviour, half the syscalls,
    // and no exception path in the frame loop.
    bool check() {
        if (path_.empty()) {
            return false;
        }

        std::error_code ec;
        auto currentTime = std::filesystem::last_write_time(path_, ec);
        if (ec) {
            return false;   // absent or unreadable — nothing to reload
        }
        if (currentTime != lastWriteTime_) {
            lastWriteTime_ = currentTime;
            return true;
        }
        return false;
    }

private:
    std::string path_;
    std::filesystem::file_time_type lastWriteTime_;
};

// =========================================================================
// ACTIVE CARTRIDGE TYPES -- Derived from defines
// =========================================================================

namespace render_ns = t7::INCUBATE_RENDER;

using RenderCartridge = render_ns::Cartridge;

// Name for display
constexpr const char* RENDER_NAME = STRINGIFY(INCUBATE_RENDER);

// =========================================================================
// MAIN
// =========================================================================

int main(int argc, char* argv[]) {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  INCUBATOR DUAL (Hot Reload Enabled)\n";
    std::cout << "  Clock:    BeatClock\n";
    std::cout << "  Render:   " << RENDER_NAME << "\n";
    std::cout << "========================================\n";
    std::cout << "\n";

    // --- Initialize Console -------------------------------------------------
    t7::Console console;
    if (!console.init("Incubator Dual", 1280, 720)) {
        std::cerr << "Failed to initialize console\n";
        return 1;
    }
    console.set_cursor_grab(true);   // the exhibition holds the pointer

    // --- The Clock -----------------------------------------------------------
    // BeatClock needs no initialization: it starts at zero and advances
    // from dt alone. No command-line input either.
    t7::BeatClock clock;
    (void)argc; (void)argv;

    std::cout << "[Incubator] BeatClock ready (bpm " << clock.bpm << ")\n";

    // --- Initialize Render Cartridge ----------------------------------------
    RenderCartridge render;
    render.initialize(console.device());

    if (!render.init_renderer(console.color_format(), console.depth_format())) {
        std::cerr << "Failed to initialize " << RENDER_NAME << " renderer\n";
        return 1;
    }

    std::cout << "[Incubator] " << RENDER_NAME << " renderer ready\n";

    // Publish the slot map once. The BeatClock's layout is EMPTY by design
    // (CUT_1c): every render-side resolve misses, warns once on stderr, and
    // leaves its coupling disabled — the graceful path in signal_layout.hpp.
    render.bind_signal_layout(clock.stat_layout());

    // --- Setup File Watcher -------------------------------------------------
    FileWatcher watcher;
    watcher.watch(render.shader_path());
    std::cout << "[Incubator] Hot reload enabled: " << render.shader_path() << "\n\n";
    std::cout << "Controls: WASD=move, Mouse=camera, 5-8=moods, Esc=quit\n\n";

    int reload_frame_counter = 0;
    wgpu::Queue queue = console.queue();

    // --- Main Loop ----------------------------------------------------------
    while (console.running()) {
        float dt = console.begin_frame();

        // --- Hot Reload Check (every ~30 frames) ----------------------------
        if (++reload_frame_counter >= 30) {
            reload_frame_counter = 0;
            // The progress dot is on the instruments dial: it is an explicit
            // FLUSH — a blocking console write — twice a second, forever, and
            // it reports only that the loop is still looping. The reload
            // itself still announces itself, loudly, when it happens.
            if constexpr (t7::INSTRUMENTS.watcher_ticks) {
                std::cout << "." << std::flush;
            }
            if (watcher.check()) {
                std::cout << "\n[FileWatcher] Change detected!\n";
                render.reload_shaders();
            }
        }

        // --- Input (all of it is the world's) --------------------------------
        for (const auto& event : console.input_events()) {
            render.on_input(event);
        }
        console.clear_input_events();

        // --- Update ---------------------------------------------------------
        clock.update(dt);
        render.update(clock.output(), console.aspect_ratio(), queue);

        // --- Render ---------------------------------------------------------
        if (!console.acquire_surface_texture()) {
            continue;
        }

        wgpu::CommandEncoderDescriptor encDesc{};
        wgpu::CommandEncoder encoder = console.device().CreateCommandEncoder(&encDesc);

        render.render(encoder, console.backbuffer(), console.depth_view());

        wgpu::CommandBufferDescriptor cmdDesc{};
        wgpu::CommandBuffer commands = encoder.Finish(&cmdDesc);
        queue.Submit(1, &commands);

        console.present();
    }

    std::cout << "[Incubator] Shutdown\n";
    return 0;
}