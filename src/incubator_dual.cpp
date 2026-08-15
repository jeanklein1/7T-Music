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
#ifdef __EMSCRIPTEN__
#include <emscripten.h>   // emscripten_set_main_loop / cancel — the rAF driver
#endif

// =========================================================================
// FILE WATCHER -- Detects shader file changes for hot reload
// (R6: a native instrument — the browser has no mtime to watch)
// =========================================================================

#ifndef __EMSCRIPTEN__
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
#endif // __EMSCRIPTEN__

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
#ifndef __EMSCRIPTEN__
    FileWatcher watcher;
    int reload_frame_counter = 0;
#endif
    wgpu::Queue queue;
    bool world_ready = false;   // PORT_1c: init_world() ran (post-device init)
};

static App* app = nullptr;

// =========================================================================
// WORLD INIT -- everything that needs the DEVICE (PORT_1c binding)
// =========================================================================
//
// One home for the post-console init sequence, verbatim from main().
// Native calls it from main() exactly where those lines were — same
// calls, same order, same failure handling. Web cannot: the device
// arrives asynchronously, so frame() calls this ONCE when boot reaches
// Ready. (The forced consequence of async boot: render.initialize
// needs console.device(), which does not exist at web main().)

static bool init_world() {
    std::cout << "[Incubator] BeatClock ready (bpm " << app->clock.bpm << ")\n";

    // --- Initialize Render Cartridge ----------------------------------------
    app->render.initialize(app->console.device());

    if (!app->render.init_renderer(app->console.color_format(), app->console.depth_format())) {
        std::cerr << "Failed to initialize " << RENDER_NAME << " renderer\n";
        return false;
    }

    std::cout << "[Incubator] " << RENDER_NAME << " renderer ready\n";

    // Publish the slot map once. The BeatClock's layout is EMPTY by design
    // (CUT_1c): every render-side resolve misses, warns once on stderr, and
    // leaves its coupling disabled — the graceful path in signal_layout.hpp.
    app->render.bind_signal_layout(app->clock.stat_layout());

#ifndef __EMSCRIPTEN__
    // --- Setup File Watcher (native instrument, R6) --------------------------
    app->watcher.watch(app->render.shader_path());
    std::cout << "[Incubator] Hot reload enabled: " << app->render.shader_path() << "\n\n";
#endif
    std::cout << "Controls: WASD=move, Mouse=camera, 5-8=moods, Esc=quit\n\n";

    app->queue = app->console.queue();
    app->world_ready = true;
    return true;
}

// =========================================================================
// FRAME -- the loop body, verbatim (the one token change: the acquire
// failure's `continue` is `return` here — same skip-this-frame meaning)
// =========================================================================

static void frame() {
    // --- Boot gate (PORT_1b/1c) ------------------------------------------
    // Web: rAF turns pump the boot until the device lands, then the world
    // initializes once. Native: boot_state() is Ready before the loop ever
    // runs — falls through immediately.
    if (app->console.boot_state() != t7::Console::BootState::Ready) {
        app->console.pump_boot();
        return;
    }

    // --- Device-loss gate (PORT_3a) --------------------------------------
    // Once the device is lost every wgpu object below is dead. Driving
    // them is what turns a normal web event into heap corruption, so the
    // frame stops here — before begin_frame, before any encoder, before
    // any submit. The reason already printed from the loss callback; this
    // gate stays silent so a lost device does not spam the console once
    // per rAF turn. No recovery attempt by design.
    if (app->console.device_lost()) return;
    if (!app->world_ready) {
        if (!init_world()) {
#ifdef __EMSCRIPTEN__
            emscripten_cancel_main_loop();
#endif
            return;
        }
    }

    // ═══ THE FRAME METER — S rows (OIL_1a; ledger: S0 host tail, C10) ══
    // TIMER LAW: these rows name where a wait SURFACES, not where the
    // cost lives — begin_frame carries the event pump, acquire and
    // present carry swapchain backpressure, finish_submit carries
    // command-buffer validation. Every clock pair sits behind the
    // instruments dial (folds to zero off, locals unused) and behind
    // world_ready (this line is below the boot and world-init gates).
    // A frame that fails the acquire notes NOTHING, so the S means stay
    // consistent with window_frames — which counts rendered frames only.
    std::chrono::steady_clock::time_point s_frame0{}, s_t0{};
    float s_begin = 0.0f, s_acquire = 0.0f, s_submit = 0.0f;
    if constexpr (t7::INSTRUMENTS.frame_meter) {
        s_frame0 = std::chrono::steady_clock::now();
        s_t0 = s_frame0;
    }

    float dt = app->console.begin_frame();
    if constexpr (t7::INSTRUMENTS.frame_meter)
        s_begin = std::chrono::duration<float, std::milli>(
            std::chrono::steady_clock::now() - s_t0).count();

#ifndef __EMSCRIPTEN__
    // --- Hot Reload Check (every ~30 frames; native instrument, R6) ------
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
#endif

    // --- Input (all of it is the world's) --------------------------------
    for (const auto& event : app->console.input_events()) {
        app->render.on_input(event);
    }
    app->console.clear_input_events();

    // --- Update ---------------------------------------------------------
    app->clock.update(dt);
    app->render.update(app->clock.output(), app->console.aspect_ratio(), app->queue);

    // --- Render ---------------------------------------------------------
    if constexpr (t7::INSTRUMENTS.frame_meter) s_t0 = std::chrono::steady_clock::now();
    if (!app->console.acquire_surface_texture()) {
        return;   // skipped frame — no S notes (see the S-row comment above)
    }
    if constexpr (t7::INSTRUMENTS.frame_meter)
        s_acquire = std::chrono::duration<float, std::milli>(
            std::chrono::steady_clock::now() - s_t0).count();

    wgpu::CommandEncoderDescriptor encDesc{};
    encDesc.label = "frame";   // DOMESDAY_1 A9 (label law): named at creation
    wgpu::CommandEncoder encoder = app->console.device().CreateCommandEncoder(&encDesc);

    app->render.render(encoder, app->console.backbuffer(), app->console.depth_view());

    wgpu::CommandBufferDescriptor cmdDesc{};
    if constexpr (t7::INSTRUMENTS.frame_meter) s_t0 = std::chrono::steady_clock::now();
    wgpu::CommandBuffer commands = encoder.Finish(&cmdDesc);
    app->queue.Submit(1, &commands);
    if constexpr (t7::INSTRUMENTS.frame_meter)
        s_submit = std::chrono::duration<float, std::milli>(
            std::chrono::steady_clock::now() - s_t0).count();

    if constexpr (t7::INSTRUMENTS.frame_meter) s_t0 = std::chrono::steady_clock::now();
    app->console.present();
    if constexpr (t7::INSTRUMENTS.frame_meter) {
        const auto s_end = std::chrono::steady_clock::now();
        const float s_present = std::chrono::duration<float, std::milli>(s_end - s_t0).count();
        const float s_total = std::chrono::duration<float, std::milli>(s_end - s_frame0).count();
        using HostRow = RenderCartridge::HostRow;
        app->render.meter_note_host(HostRow::Begin, s_begin);
        app->render.meter_note_host(HostRow::Acquire, s_acquire);
        app->render.meter_note_host(HostRow::FinishSubmit, s_submit);
        app->render.meter_note_host(HostRow::Present, s_present);
        app->render.meter_note_host(HostRow::FrameTotal, s_total);
    }
}

// =========================================================================
// MAIN
// =========================================================================

int main(int argc, char* argv[]) {
    std::cout << "\n";
    std::cout << "========================================\n";
    // PORT_2d — one line per twin, and each states what its own build
    // actually has. The FileWatcher class, the member, the watch() call
    // and the per-frame check are all #ifndef __EMSCRIPTEN__ (PORT_1c),
    // so the web twin has no hot reload to announce.
#ifdef __EMSCRIPTEN__
    std::cout << "  INCUBATOR DUAL (web twin — no hot reload)\n";
#else
    std::cout << "  INCUBATOR DUAL (Hot Reload Enabled)\n";
#endif
    std::cout << "  Clock:    BeatClock\n";
    std::cout << "  Render:   " << RENDER_NAME << "\n";
    std::cout << "========================================\n";
    std::cout << "\n";

    // --- Initialize Console -------------------------------------------------
    app = new App();
    if (!app->console.init("Incubator Dual", 1280, 720)) {
        std::cerr << "Failed to initialize console\n";
#ifndef __EMSCRIPTEN__
        delete app;
#else
        // EXHIBIT_0 — A REFERENCE OUTLIVES ITS REFERENT (LAWS L15), and
        // here the reference is a fetch. The cartridge constructor ran
        // inside `new App()` above, BEFORE this init — and on this twin
        // it started the exhibition.json request, holding &gallery_state_
        // as its userdata. Returning from main does not tear the browser
        // runtime down, so that request still lands, and freeing App
        // first would make it land in freed memory. The App is
        // deliberately leaked: the page is over, the leak is the last
        // thing that dies, and it is what keeps the callback honest.
#endif
        return 1;
    }
    app->console.set_cursor_grab(true);   // the exhibition holds the pointer

    // --- The Clock -----------------------------------------------------------
    // BeatClock needs no initialization: it starts at zero and advances
    // from dt alone. No command-line input either.
    (void)argc; (void)argv;

#ifdef __EMSCRIPTEN__
    // --- The rAF loop (PORT_1c) ----------------------------------------------
    // Boot continues asynchronously from here; frame() pumps the boot state
    // and runs init_world() once the device lands. 0 = rAF-paced; true =
    // this call never returns.
    emscripten_set_main_loop(frame, 0, true);
    return 0;
#else
    // --- World init (device exists — native boot is synchronous) -------------
    if (!init_world()) {
        delete app;
        return 1;
    }

    // --- Main Loop ----------------------------------------------------------
    while (app->console.running()) {
        frame();
    }

    std::cout << "[Incubator] Shutdown\n";
    delete app;
    return 0;
#endif
}
