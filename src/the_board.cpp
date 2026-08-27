/**
 * THE BOARD -- Render Cartridge Development Harness
 * =================================================
 *
 * Minimal runtime for the render cartridge. Cartridge selection is
 * controlled from CMakeLists.txt:
 *
 *   set(T7_RENDER_CARTRIDGE "the_board")
 *
 * CMake passes it as a compile definition (INCUBATE_RENDER).
 * No need to edit this file to switch cartridges.
 *
 * CONVENTION:
 *   Render cartridges:
 *   | Folder name          | Namespace              | Class     |
 *   |----------------------|------------------------|-----------|
 *   | the_board/           | t7::the_board          | Cartridge |
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
#include "core/boot_params.hpp"   // DOMESDAY_1 B9 — parse_boot_params at the top of main
#include <iostream>
#include <chrono>
#include <emscripten.h>   // emscripten_set_main_loop / cancel — the rAF driver

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
// one struct so the loop body could live in frame() and be driven
// either by main()'s while (native) or by the browser's rAF (PORT_1c).
// Member order IS the old construction order; init calls stay in
// main(), verbatim and in sequence.

// ═══ THE READY OFFER'S FLOOR (OVERTURE_0) ════════════════════════════════
//
// "Controls:" is the line the shell dismisses the veil on — it either
// presents the world or offers the door, depending on whether the visitor
// has already tapped. The world is LIVE behind that veil, so the line is not
// "the world is ready", it is "you may look now".
//
// It used to print the instant the frame loop went live, which is before any
// painting can have landed: the eager tapper watched the boot ring dress in
// front of them. The offer now waits for a floor of paintings, or for a
// timeout, whichever comes first.
//
// THE TRADE, NAMED: the eager tapper waits at most OVERTURE_READY_TIMEOUT_S
// behind a veil that is already saying "Hanging the paintings (n/57)", and
// nobody sees the ring dress in front of them. A manifest that never lands is
// the timeout's case and needs no third arm.
inline constexpr uint32_t OVERTURE_READY_FLOOR     = 6;     // two galleries of three
inline constexpr float    OVERTURE_READY_TIMEOUT_S = 5.0f;  // a slow phone is not held hostage

struct App {
    t7::Console console;
    t7::BeatClock clock;
    RenderCartridge render;
    wgpu::Queue queue;
    bool world_ready = false;   // PORT_1c: init_world() ran (post-device init)
    // U9's two: when the frame loop went live, and whether the offer is spent.
    // One-shot by construction — once printed, the test is never evaluated again.
    std::chrono::steady_clock::time_point world_live{};
    bool controls_offered = false;
};

static App* app = nullptr;

// =========================================================================
// WORLD INIT -- everything that needs the DEVICE (PORT_1c binding)
// =========================================================================
//
// One home for the post-console init sequence, verbatim from main().
// Native called it from main() exactly where those lines were — same
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

    // THE OFFER IS NOT MADE HERE ANY MORE (OVERTURE_0). This is the instant
    // the frame loop goes live, which is the instant the wait is measured
    // from — see offer_controls_when_ready(), called from frame().
    app->queue = app->console.queue();
    app->world_live = std::chrono::steady_clock::now();
    app->world_ready = true;
    return true;
}

// ── the offer, once the exhibition has a floor under it ────────────
//
// Called every frame until it fires. The line's TEXT is load-bearing at both
// ends: web/index.html dismisses the veil on a line beginning "Controls:",
// so the string and its position stay exactly as they were.
static void offer_controls_when_ready() {
    if (app->controls_offered) return;
    const float waited = std::chrono::duration<float>(
        std::chrono::steady_clock::now() - app->world_live).count();
    // ORGANS ARE PUBLIC (the composition root's law): the gallery's own tally
    // is readable here without a face cut for it.
    const bool floor_met =
        app->render.gallery_state_.authored_staged_count >= OVERTURE_READY_FLOOR;
    if (!floor_met && waited < OVERTURE_READY_TIMEOUT_S) return;
    app->controls_offered = true;
    std::cout << "Controls: WASD=move, Mouse=camera, 5-8=moods, Esc=quit\n\n";
}

// =========================================================================
// FRAME -- the loop body, verbatim (the one token change: the acquire
// failure's `continue` is `return` here — same skip-this-frame meaning)
// =========================================================================

static void frame() {
    // --- Boot gate (PORT_1b/1c) ------------------------------------------
    // Web: rAF turns pump the boot until the device lands, then the world
    // initializes once. Native: boot_state() was Ready before the loop
    // ever ran — fell through immediately.
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
            emscripten_cancel_main_loop();
            return;
        }
    }

    // THE READY OFFER (OVERTURE_0) — the veil lifts on a world that has its
    // first paintings up, not on a world that has merely started.
    offer_controls_when_ready();

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
    // ORGAN — THE DIRTY FLUSH, at the frame boundary and nowhere else.
    // begin_frame has polled events and reconciled, so every writer for this
    // frame has spoken and the panel's edits are bits: this turns them into
    // at most one WriteBuffer per block (docs/ORGAN.md).
    app->render.organ_flush(app->queue);
    if constexpr (t7::INSTRUMENTS.frame_meter)
        s_begin = std::chrono::duration<float, std::milli>(
            std::chrono::steady_clock::now() - s_t0).count();


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

    app->render.render(encoder, app->console.backbuffer(),
        app->console.msaa_color_view(),   // B10: null at msaa=1
        app->console.depth_view());

    wgpu::CommandBufferDescriptor cmdDesc{};
    if constexpr (t7::INSTRUMENTS.frame_meter) s_t0 = std::chrono::steady_clock::now();
    wgpu::CommandBuffer commands = encoder.Finish(&cmdDesc);
    app->queue.Submit(1, &commands);
    // RIBBON_2 P0 1.2b: an updated-but-unrendered frame adds its dt to the
    // next rendered one — a dropped acquire stretches a step, never deletes
    // it. This is the moment the GPU has actually been handed the time the
    // cartridge was holding, and the only place the accumulator may clear.
    // The early return at acquire_surface_texture() above never reaches it.
    app->render.frame_submitted();
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
    // DOMESDAY_1 B9 — the parameter surface, parsed before ANY
    // consumer: the cartridge ctor (inside `new App()` below) reads
    // seed and mood; the console reads cap. One read, never again.
    t7::parse_boot_params(argc, argv);
    std::cout << "\n";
    std::cout << "========================================\n";
    // PORT_2d — the banner states what this build actually has. The
    // FileWatcher class, its member, the watch() call and the per-frame
    // check went with the native arm (SUNSET_1), so there is no hot
    // reload to announce and no longer any twin to contrast against.
    std::cout << "  THE BOARD (web twin — no hot reload)\n";
    std::cout << "  Clock:    BeatClock\n";
    std::cout << "  Render:   " << RENDER_NAME << "\n";
    std::cout << "========================================\n";
    std::cout << "\n";

    // --- Initialize Console -------------------------------------------------
    app = new App();
    if (!app->console.init("The Board", 1280, 720)) {
        std::cerr << "Failed to initialize console\n";
        // EXHIBIT_0 — A REFERENCE OUTLIVES ITS REFERENT (LAWS L15), and
        // here the reference is a fetch. The cartridge constructor ran
        // inside `new App()` above, BEFORE this init — and on this twin
        // it started the exhibition.json request, holding &gallery_state_
        // as its userdata. Returning from main does not tear the browser
        // runtime down, so that request still lands, and freeing App
        // first would make it land in freed memory. The App is
        // deliberately leaked: the page is over, the leak is the last
        // thing that dies, and it is what keeps the callback honest.
        return 1;
    }
    app->console.set_cursor_grab(true);   // the exhibition holds the pointer

    // --- The Clock -----------------------------------------------------------
    // BeatClock needs no initialization: it starts at zero and advances
    // from dt alone. No command-line input either.
    (void)argc; (void)argv;

    // --- The rAF loop (PORT_1c) ----------------------------------------------
    // Boot continues asynchronously from here; frame() pumps the boot state
    // and runs init_world() once the device lands. 0 = rAF-paced; true =
    // this call never returns.
    emscripten_set_main_loop(frame, 0, true);
    return 0;
}
