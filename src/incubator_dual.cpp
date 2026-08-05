/**
 * INCUBATOR DUAL -- Analysis + Render Cartridge Development Harness
 * =================================================================
 *
 * Minimal runtime for developing cartridge pairs.
 * Cartridge selection is controlled from CMakeLists.txt:
 *
 *   set(INCUBATOR_DUAL_RENDER_CARTRIDGE "the_board")
 *   set(INCUBATOR_DUAL_ANALYSIS_CARTRIDGE "canvas_1")
 *
 * CMake passes these as compile definitions (INCUBATE_RENDER, INCUBATE_ANALYSIS).
 * No need to edit this file to switch cartridges.
 *
 * CONVENTION:
 *   Analysis cartridges:
 *   | Folder name          | Namespace              | Class  |
 *   |----------------------|------------------------|--------|
 *   | canvas_1/            | t7::canvas_1           | Canvas |
 *
 *   Render cartridges:
 *   | Folder name          | Namespace              | Class     |
 *   |----------------------|------------------------|-----------|
 *   | the_board/     | t7::the_board    | Cartridge |
 *
 * INPUT ROUTING:
 *   - Music keys (A-Z, etc.) -> Analysis cartridge
 *   - Movement/camera        -> Render cartridge
 */

 // =========================================================================
 // TARGET SELECTION -- Provided by CMake, with fallback for manual override
 // =========================================================================

#ifndef INCUBATE_ANALYSIS
#define INCUBATE_ANALYSIS canvas_1
#endif

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

#define ANALYSIS_HEADER(name) STRINGIFY(analysis/name/canvas.hpp)
#define RENDER_HEADER(name)   STRINGIFY(cartridges/name/cartridge.hpp)

// =========================================================================
// INCLUDES
// =========================================================================

#include "console/console.hpp"

// IntelliSense cannot resolve macro-expanded #include paths.
// These literal includes give VS navigation (Peek Definition, Go To, etc.).
// The compiler ignores them -- the macro includes below pull in the same files.
#if defined(__INTELLISENSE__)
#include "analysis/canvas_1/canvas.hpp"
#include "cartridges/the_board/cartridge.hpp"
#else
#include ANALYSIS_HEADER(INCUBATE_ANALYSIS)
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

namespace analysis_ns = t7::INCUBATE_ANALYSIS;
namespace render_ns = t7::INCUBATE_RENDER;

using AnalysisCartridge = analysis_ns::Canvas;
using RenderCartridge = render_ns::Cartridge;

// Names for display
constexpr const char* ANALYSIS_NAME = STRINGIFY(INCUBATE_ANALYSIS);
constexpr const char* RENDER_NAME = STRINGIFY(INCUBATE_RENDER);

// =========================================================================
// INPUT ROUTING -- the keyboard is the WORLD'S by default (PANEL-0 p1a)
// =========================================================================
//
// is_music_key routed the QWERTY letters to the analysis cartridge back
// when the computer keyboard WAS the note source. Ableton is the source
// now, so every key falls to render — the world owns W/A/S/D and the rest.
// (The INCUBATE_MUSIC_KEYS escape hatch was never defined anywhere, so it
//  had never compiled and would not have worked if switched on; deleted
//  PRUNING_1 P1 Step 4. git has the routing if it is ever wanted back.)
static bool is_music_key(int key) {
    (void)key;   // the keyboard is the world's; the note source is Ableton
    return false;
}

// =========================================================================
// MAIN
// =========================================================================

int main(int argc, char* argv[]) {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "  INCUBATOR DUAL (Hot Reload Enabled)\n";
    std::cout << "  Analysis: " << ANALYSIS_NAME << "\n";
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

    // --- Initialize Analysis Cartridge --------------------------------------
    AnalysisCartridge analysis;
    analysis.initialize("assets");

    // canvas_1's source is the DAW transport (loopMIDI), not a MIDI file, so
    // there is no command-line MIDI to load.
    (void)argc; (void)argv;

    std::cout << "[Incubator] " << ANALYSIS_NAME << " analysis ready\n";

    // --- Initialize Render Cartridge ----------------------------------------
    RenderCartridge render;
    render.initialize(console.device());

    if (!render.init_renderer(console.color_format(), console.depth_format())) {
        std::cerr << "Failed to initialize " << RENDER_NAME << " renderer\n";
        return 1;
    }

    std::cout << "[Incubator] " << RENDER_NAME << " renderer ready\n";

    // Both cartridges are initialized; publish the slot map once so the render
    // side can resolve coupling sources by name (the_board's fog coupling reads
    // "all.field"). STAT_LAYOUT is constexpr, valid the moment analysis exists.
    render.bind_signal_layout(analysis.stat_layout());

    // --- Setup File Watcher -------------------------------------------------
    FileWatcher watcher;
    watcher.watch(render.shader_path());
    std::cout << "[Incubator] Hot reload enabled: " << render.shader_path() << "\n\n";
    std::cout << "Controls: WASD=move, Mouse=camera, 4=host, 5-8=moods, Esc=quit\n\n";

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

        // --- Input Routing --------------------------------------------------
        for (const auto& event : console.input_events()) {
            if (event.type == t7::InputEvent::Type::KeyDown ||
                event.type == t7::InputEvent::Type::KeyUp) {

                if (is_music_key(event.key)) {
                    analysis.on_input(event);
                }
                else {
                    render.on_input(event);
                }
            }
            else {
                render.on_input(event);
            }
        }
        console.clear_input_events();

        // --- Update ---------------------------------------------------------
        analysis.update(dt);

        // (canvas_1 publishes the union field; the render side reads it through
        // the bound stat layout — no per-cartridge debug stat to print here.)

        render.update(analysis.output(), console.aspect_ratio(), queue);

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