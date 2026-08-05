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
// APP -- the loop-carried state (PORT_1a)
// =========================================================================
//
// The six locals that persisted across frame-loop iterations, homed in
// one struct so the loop body can live in frame() and be driven either
// by main()'s while (native) or by the browser's rAF (PORT_1c).
// Member order IS the old construction order; init calls stay in
// main(), verbatim and in sequence.

struct App {
    t7::Console console;
    t7::BeatClock clock;
    RenderCartridge render;
    FileWatcher watcher;
    int reload_frame_counter = 0;
    wgpu::Queue queue;
};

static App* app = nullptr;

// =========================================================================
// FRAME -- the loop body, verbatim (the one token change: the acquire
// failure's `continue` is `return` here — same skip-this-frame meaning)
// =========================================================================

static void frame() {
    float dt = app->console.begin_frame();

    // --- Hot Reload Check (every ~30 frames) ----------------------------
    if (++app->reload_frame_counter >= 30) {
        app->reload_frame_counter = 0;
        // The progress dot is on the instruments dial: it is an explicit
        // FLUSH — a blocking console write — twice a second, forever, and
        // it reports only that the loop is still looping. The reload
        // itself still announces itself, loudly, when it happens.
        if constexpr (t7::INSTRUMENTS.watcher_ticks) {
            std::cout << "." << std::flush;
        }
        if (app->watcher.check()) {
            std::cout << "\n[FileWatcher] Change detected!\n";
            app->render.reload_shaders();
        }
    }

    // --- Input (all of it is the world's) --------------------------------
    for (const auto& event : app->console.input_events()) {
        app->render.on_input(event);
    }
    app->console.clear_input_events();

    // --- Update ---------------------------------------------------------
    app->clock.update(dt);
    app->render.update(app->clock.output(), app->console.aspect_ratio(), app->queue);

    // --- Render ---------------------------------------------------------
    if (!app->console.acquire_surface_texture()) {
        return;
    }

    wgpu::CommandEncoderDescriptor encDesc{};
    wgpu::CommandEncoder encoder = app->console.device().CreateCommandEncoder(&encDesc);

    app->render.render(encoder, app->console.backbuffer(), app->console.depth_view());

    wgpu::CommandBufferDescriptor cmdDesc{};
    wgpu::CommandBuffer commands = encoder.Finish(&cmdDesc);
    app->queue.Submit(1, &commands);

    app->console.present();
}

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
    app = new App();
    if (!app->console.init("Incubator Dual", 1280, 720)) {
        std::cerr << "Failed to initialize console\n";
        delete app;
        return 1;
    }
    app->console.set_cursor_grab(true);   // the exhibition holds the pointer

    // --- The Clock -----------------------------------------------------------
    // BeatClock needs no initialization: it starts at zero and advances
    // from dt alone. No command-line input either.
    (void)argc; (void)argv;

    std::cout << "[Incubator] BeatClock ready (bpm " << app->clock.bpm << ")\n";

    // --- Initialize Render Cartridge ----------------------------------------
    app->render.initialize(app->console.device());

    if (!app->render.init_renderer(app->console.color_format(), app->console.depth_format())) {
        std::cerr << "Failed to initialize " << RENDER_NAME << " renderer\n";
        delete app;
        return 1;
    }

    std::cout << "[Incubator] " << RENDER_NAME << " renderer ready\n";

    // Publish the slot map once. The BeatClock's layout is EMPTY by design
    // (CUT_1c): every render-side resolve misses, warns once on stderr, and
    // leaves its coupling disabled — the graceful path in signal_layout.hpp.
    app->render.bind_signal_layout(app->clock.stat_layout());

    // --- Setup File Watcher -------------------------------------------------
    app->watcher.watch(app->render.shader_path());
    std::cout << "[Incubator] Hot reload enabled: " << app->render.shader_path() << "\n\n";
    std::cout << "Controls: WASD=move, Mouse=camera, 5-8=moods, Esc=quit\n\n";

    app->queue = app->console.queue();

    // --- Main Loop ----------------------------------------------------------
    while (app->console.running()) {
        frame();
    }

    std::cout << "[Incubator] Shutdown\n";
    delete app;
    return 0;
}