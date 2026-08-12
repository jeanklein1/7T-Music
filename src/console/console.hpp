#pragma once

// ─── console.hpp ─────────────────────────────────────────────────
//
// The 7T runtime infrastructure: the platform shell for the visualizer.
// Owns the window, GPU device, surface, depth buffer, input collection,
// and frame timing. Does NOT own cartridges, musical interpretation, or
// visual interpretation.
//
// Organized in lifecycle order:
//   §1  IDENTITY       — Constructor, destructor, copy prevention
//   §2  INITIALIZATION — init() and the helpers it calls, in call order
//   §3  FRAME          — begin_frame(), acquire, present, running
//   §4  INPUT          — Injection (producer) then access (consumer)
//   §5  ACCESSORS      — Handles and properties for external use
//   §6  SHUTDOWN       — shutdown(), request_close()
//   §7  STATE          — Member variables, grouped by responsibility
//
// Usage:
//   Console console;
//   if (!console.init("7T Visualizer", 1280, 720)) return 1;
//   while (console.running()) {
//       float dt = console.begin_frame();
//       // ... update cartridges ...
//       console.present();
//   }

#include "core/input_event.hpp"

#include <webgpu/webgpu_cpp.h>

// ── PORT_1b Region 1: platform includes ──────────────────────────
// Native links dawn::native and exposes the OS window handle for the
// surface. Web (emdawnwebgpu) has neither; contrib.glfw3 ships no
// glfw3native.h, so the whole expose block is native-only too.
#ifndef __EMSCRIPTEN__
#include <dawn/native/DawnNative.h>
#include <dawn/dawn_proc.h>
#if __has_include("dawn/common/Version_autogen.h")
#include "dawn/common/Version_autogen.h"
#define T7_DAWN_VERSION 1
#else
#define T7_DAWN_VERSION 0
#endif
#endif
#include <GLFW/glfw3.h>

#ifndef __EMSCRIPTEN__
#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>
#else
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLFW/emscripten_glfw3.h>   // emscripten_glfw_make_canvas_resizable (FRAME_0)
#endif

#include <algorithm>
#include <vector>
#include <chrono>
#include <cmath>       // std::sqrt — the stick's magnitude (SHIP_1)
#include <cstdint>     // uint64_t / UINT64_MAX — the touch table's birth counter (SHIP_1)
#include <iostream>
#include <string>
#include <string_view>
#include <optional>

namespace t7 {

    // ═══ WEB PRESENTATION CONSTANTS (PORT_3c) ════════════════════════
    //
    // THE PIXEL CAP — the largest mobile lever this program has, and it
    // is a scalar. A phone at 390x844 CSS px with devicePixelRatio 3
    // renders 1170x2532 = 2.96 M pixels, nearly TRIPLE a 1366x768
    // laptop: a device three times faster can present itself as slower.
    // The ratio multiplies fragment work, the surface backbuffer, and
    // every full-screen attachment together, so it is also a memory
    // dial, not only a speed one.
    //
    // 1.5 keeps edges visibly crisper than 1.0 at 2.25x the pixels
    // rather than 9x. PANEL-ELIGIBLE and deliberately alone up here:
    // this is the number to move when a device cannot keep up, and
    // moving it changes nothing else. 1.0 = CSS-pixel rendering;
    // a very large value = uncapped, the pre-PORT_3c behavior.
    inline constexpr float MAX_DEVICE_PIXEL_RATIO = 1.5f;

    // ═══ THE COMPILER PLAN (PIVOT_0, 2026-08-12) ═════════════════════
    //
    // PIVOT_0 — the native shader-compiler plan. The web twin's
    // compiler is the browser's own; this constant governs native
    // only. world.wgsl is single-source across all values.
    //
    // Why it exists: WALLET_0's occupier cbuffer arrays stalled
    // update_player_agent at 20,227 ms under FXC and then
    // D3DCompiler_47 access-violated on the next room kernel. Jean
    // ruled the floor up rather than the shader down. The audience
    // floor is WebGPU core through modern compilers — Tint→DXC
    // (SM6.0+), Tint→MSL, Tint→SPIR-V, naga.
    //
    // D3D12_Fxc exists for ARCHAEOLOGY ONLY. It reproduces the retired
    // gate so a historical result can be re-run; it is not a supported
    // floor and nothing should be shaped to satisfy it. The laws it
    // used to impose are in audit/FXC_LAWS_RECORD.md.
    //
    // Plan B is one line: if DXC fails on a given driver, set this to
    // Vulkan, rebuild, boot. That IS the fallback, not a failure.
    enum class CompilerPlan { D3D12_Dxc, Vulkan, D3D12_Fxc };
    inline constexpr CompilerPlan kCompilerPlan = CompilerPlan::Vulkan;

    inline constexpr const char* compiler_plan_name(CompilerPlan p) {
        switch (p) {
        case CompilerPlan::D3D12_Dxc: return "DXC";
        case CompilerPlan::Vulkan:    return "VULKAN";
        case CompilerPlan::D3D12_Fxc: return "FXC";
        }
        return "?";
    }

#ifdef __EMSCRIPTEN__
    // ═══ THE INSTANCE ANCHOR (PORT_4a) ═══════════════════════════════
    //
    // A SECOND external reference to the WebGPU Instance, in static
    // storage, DELIBERATELY NEVER RELEASED. It exists so that no
    // lifetime accident anywhere in the program can take the external
    // count to zero — Dawn destroys every device made from an instance
    // whose last EXTERNAL reference drops, and a device torn out from
    // under a running frame loop is heap corruption a few frames later.
    //
    // This is a BELT, and the reason it is a belt and not a fix is
    // worth recording. PORT_4's census found NO drop in our code:
    // Console::instance_ is assigned once, never released, never moved;
    // Console is non-copyable AND non-movable (a user-declared deleted
    // copy suppresses the implicit move), ~Console() is unreachable on
    // the web path because App is heap-allocated and never deleted, and
    // initWebGPU() has exactly one reachable call. The one raw-handle
    // construction in this file is native-only and refcount-neutral
    // (Dawn's ObjectBase(CType) AddRefs on construction; only Acquire()
    // adopts). So this anchor should be redundant — and if the
    // "[Device] LOST" line survives it, that is PROOF the loss comes
    // from outside this program and the search moves to the browser.
    //
    // Shape borrowed from Dawn's own cross-platform sample, which holds
    // `static wgpu::Instance instance;` for exactly this reason.
    // Nothing releases it: Emscripten does not run static destructors
    // (EXIT_RUNTIME is off and main never returns — it unwinds).
    inline wgpu::Instance g_instanceAnchor;
#endif

#ifdef __EMSCRIPTEN__
    // ═══ SHIP_1 — TOUCH ══════════════════════════════════════════════
    //
    // THE PANEL. CameraControls' form, one module over: one organized
    // block, clear names, editable without hunting. It lives HERE and
    // not beside CameraControls because the gesture machine lives here —
    // the console owns the hand, the cartridge owns what the hand means.
    //
    // EVERY LENGTH IS CSS PIXELS. EmscriptenTouchPoint::targetX/targetY
    // are CSS px (clientX minus the canvas rect), and glfwGetWindowSize
    // reports CSS px too, so the midline and the stick are measured in
    // the same units the finger moves in. Feel therefore survives
    // devicePixelRatio — a 3x phone does not get a 3x-twitchy stick,
    // which is exactly the trap PORT_3c's cap comment warns about from
    // the other side.
    struct TouchControls {
        // The stick's throw: the drag at which the move vector reaches
        // full magnitude. About a thumb's comfortable arc.
        static constexpr float STICK_RADIUS    = 64.0f;
        // Below this the vector is exactly ZERO, not small — a resting
        // thumb must not walk the pawn.
        static constexpr float STICK_DEAD_ZONE = 8.0f;
        // Radians per CSS pixel. SEPARATE from the mouse's
        // CameraControls::look_sensitivity by design: a thumb sweeps a
        // fraction of the arc a mouse does, so one number cannot serve
        // both hands.
        static constexpr float LOOK_SENS_TOUCH = 0.006f;
        // Zoom units per CSS pixel of separation change. Feeds the same
        // zoom_delta channel the scroll wheel feeds.
        static constexpr float PINCH_SENS      = 0.06f;
        // A tap declares itself by a clean quick release; a pinch
        // declares itself by separation change OR by outliving this.
        static constexpr double TAP_MS         = 220.0;
        // Movement past this and the touch was never a tap.
        static constexpr float TAP_SLOP        = 12.0f;
        // Separation change past this declares a pinch immediately,
        // without waiting out TAP_MS.
        static constexpr float PINCH_DECLARE   = 8.0f;
    };

    // ONE TRACKED FINGER. `left` is decided once, at birth, and never
    // again — a thumb that slides across the midline keeps the identity
    // it was born with, so a wide drag cannot silently become a
    // different gesture halfway through.
    struct TouchPoint {
        int      id      = -1;
        bool     active  = false;
        bool     left    = false;
        uint64_t seq     = 0;       // birth order: who is primary, who is second
        float    x = 0.0f,  y = 0.0f;    // current, CSS px, canvas-relative
        float    x0 = 0.0f, y0 = 0.0f;   // where it landed
        double   t0 = 0.0;               // when it landed, ms
        bool     slopped = false;        // has moved past TAP_SLOP since landing
    };

    // The port's default canvas selector (Config.h kDefaultCanvasSelector).
    // lib_emscripten_glfw3.js registers it into specialHTMLTargets at
    // glfwInit, so findEventTarget resolves this exact string to the exact
    // element the port registered its own touch handlers on. That identity
    // is what makes the deregistration below hit its target.
    inline constexpr const char* TOUCH_TARGET = "Module['canvas']";
#endif

    class Console {

        // ═══ §1 IDENTITY ═════════════════════════════════════════

    public:
        Console() = default;
        ~Console() { shutdown(); }

        Console(const Console&) = delete;
        Console& operator=(const Console&) = delete;

        // ── PORT_1b: the boot grammar ────────────────────────────
        // Boot is a state machine. Native traverses it synchronously
        // inside init() (RequestingAdapter..Configuring never observed;
        // init() ends at Ready). Web starts an async request chain in
        // init() and the frame gate pumps Configuring → Ready once the
        // device callback lands. Failed is terminal — the cause has
        // already printed.
        enum class BootState { RequestingAdapter, RequestingDevice, Configuring, Ready, Failed };

        BootState boot_state() const { return bootState_; }

        // ── PORT_3a: the device can be lost ──────────────────────
        // On the web, device loss is NORMAL: tab backgrounding, GPU
        // resets, driver updates. A gallery installation running for
        // hours will see it. Once lost, every wgpu object the program
        // holds is dead, and continuing to drive them is what turns a
        // recoverable event into heap corruption. The policy is visible,
        // honest death — no recovery attempt: the frame gate stops
        // issuing GPU work and the reason is on the console verbatim.
        bool device_lost() const { return deviceLost_; }

        // Advance Configuring → Ready: runs the existing surface +
        // depth-buffer path once the device exists, then seeds the
        // frame clock. Native never needs it (init() reaches Ready
        // synchronously) but it is callable there harmlessly: every
        // other state is a no-op.
        void pump_boot() {
            if (bootState_ != BootState::Configuring) return;
            if (!initSurface()) { bootState_ = BootState::Failed; return; }
            createDepthBuffer(currentWidth_, currentHeight_);
            lastTime_ = std::chrono::high_resolution_clock::now();
            bootState_ = BootState::Ready;
        }


        // ═══ §2 INITIALIZATION ═══════════════════════════════════
        //
        // Call order: initGLFW → initWebGPU → initSurface → createDepthBuffer.
        // Each step depends on the previous. If any fails, init returns false.

    public:
        bool init(const char* title, uint32_t width, uint32_t height) {
            initialWidth_ = width;
            initialHeight_ = height;
            currentWidth_ = width;
            currentHeight_ = height;

            if (!initGLFW(title))   { bootState_ = BootState::Failed; return false; }
            if (!initWebGPU())      { bootState_ = BootState::Failed; return false; }
#ifndef __EMSCRIPTEN__
            // Native: the device exists synchronously — finish boot here,
            // exactly the pre-PORT_1b sequence, ending Ready.
            if (!initSurface())     { bootState_ = BootState::Failed; return false; }
            createDepthBuffer(width, height);

            lastTime_ = std::chrono::high_resolution_clock::now();
            bootState_ = BootState::Ready;
#endif
            // Web: initWebGPU only STARTED the request chain; the frame
            // gate pumps Configuring → Ready when the device lands.
            return true;
        }

    private:
        bool initGLFW(const char* title) {
            if (!glfwInit()) {
                std::cerr << "Failed to initialize GLFW\n";
                return false;
            }

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

            window_ = glfwCreateWindow(initialWidth_, initialHeight_, title, nullptr, nullptr);
            if (!window_) {
                std::cerr << "Failed to create window\n";
                glfwTerminate();
                return false;
            }

            // Set user pointer for callbacks
            glfwSetWindowUserPointer(window_, this);

            // Set callbacks
            glfwSetKeyCallback(window_, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
                (void)scancode; (void)mods;
                auto* console = static_cast<Console*>(glfwGetWindowUserPointer(w));
                if (!console) return;

#ifndef __EMSCRIPTEN__
                // Native-only: the browser owns ESC (pointer-lock exit).
                if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                    console->request_close();
                    return;
                }
#endif

                // Numpad * — the pointer door. A window command of the same
                // class as ESC: it never reaches the cartridge fan, and the
                // cartridge never gains a window handle to serve it.
                if (key == GLFW_KEY_KP_MULTIPLY && action == GLFW_PRESS) {
                    console->toggle_cursor_grab();
                    return;
                }

                console->inject_key_event(key, action);
                });

            glfwSetCursorPosCallback(window_, [](GLFWwindow* w, double xpos, double ypos) {
                auto* console = static_cast<Console*>(glfwGetWindowUserPointer(w));
                if (!console) return;
                console->feed_cursor(xpos, ypos);
                });

            // A focus change un-applies and re-applies the OS cursor mode
            // beneath us; both edges move the pointer. Unprime so the first
            // sample after the seam is a new origin, not a jump.
            glfwSetWindowFocusCallback(window_, [](GLFWwindow* w, int focused) {
                (void)focused;
                auto* console = static_cast<Console*>(glfwGetWindowUserPointer(w));
                if (!console) return;
                console->unprime_cursor();
                });

            glfwSetMouseButtonCallback(window_, [](GLFWwindow* w, int button, int action, int mods) {
                (void)mods;
                auto* console = static_cast<Console*>(glfwGetWindowUserPointer(w));
                if (console) console->inject_mouse_button(button, action == GLFW_PRESS);
                });

            glfwSetScrollCallback(window_, [](GLFWwindow* w, double xoffset, double yoffset) {
                (void)xoffset;
                auto* console = static_cast<Console*>(glfwGetWindowUserPointer(w));
                if (console) console->inject_scroll(static_cast<float>(yoffset));
                });

#ifdef __EMSCRIPTEN__
            // ═══ FRAME_0 — THE LINK THAT WAS NEVER MADE ══════════════
            //
            // The fluid frame was a contract with a missing middle. The
            // shell tracked visualViewport into --app-h correctly; this
            // file reconfigured the surface on any framebuffer change
            // correctly, PORT_3c-capped. Between them, nothing: the
            // library pins the canvas at glfwCreateWindow's size and
            // enforces it with INLINE width/height
            // (lib_emscripten_glfw3.js, emglfw3w_set_size — its own
            // comment says "this will (on purpose) override any css
            // setting"; it asks for !important and does not get it,
            // because ctx.setCSSValue is a two-parameter arrow that
            // drops the priority — which changes nothing, since a plain
            // inline declaration already outranks a non-important author
            // rule). A stylesheet rule cannot win that, so the
            // canvas never moved, glfwGetFramebufferSize never changed,
            // and the per-frame compare below had nothing to compare.
            // One defect, three symptoms: a phone that fills neither
            // orientation, and an F11 that grows the browser but not
            // the world.
            //
            // This is the whole fix. The library observes #frame and
            // sizes the canvas from it, so the chain finally runs end
            // to end:
            //   #frame  <- CSS, from --app-h (the shell, unchanged)
            //   canvas  <- the library, from #frame
            //   backing <- the library, canvas x devicePixelRatio
            //   surface <- begin_frame's compare, PORT_3c-capped
            //
            // NO DPR HINT IS NEEDED, and that was worth checking rather
            // than assuming: GLFW_SCALE_FRAMEBUFFER already defaults to
            // GLFW_TRUE in the pinned port (Config.h:46), so the
            // framebuffer is floor(css x monitorScale) (Window.cpp:326)
            // and glfwGetWindowSize stays CSS px — exactly the two units
            // apply_pixel_cap's ratio math already assumes. Setting the
            // hint would have been a no-op; NOT having checked would
            // have risked a phone rendering at CSS resolution.
            //
            // Failure is non-fatal by choice: a missing #frame leaves
            // the pre-FRAME_0 behaviour (a fixed canvas), which is a
            // worse frame but still a world.
            if (emscripten_glfw_make_canvas_resizable(window_, "#frame", nullptr)
                != EMSCRIPTEN_RESULT_SUCCESS) {
                std::cerr << "[Frame] #frame not found — the canvas cannot track the page\n";
            }
            else {
                std::cout << "[Frame] Canvas tracks #frame\n";
            }

            // SHIP_1 U1 — after the window exists, because the port
            // registered ITS touch handlers inside glfwCreateWindow.
            // Order against the resizable call above is IMMATERIAL, and
            // the reason is worth stating so nobody preserves a
            // constraint that does not exist: that call only QUEUES a
            // resize request (Window.h, fResizeRequest), applied at the
            // first glfwPollEvents; and the port registers its canvas
            // listeners exactly once, at window creation, so no resize
            // path can land between our deregistration and our claim.
            claim_touch_stream();
#endif
            return true;
        }

#ifdef __EMSCRIPTEN__
        // ═══ PORT_5d — THE DEVICE REQUEST, TWICE IF NEEDED ═══════════
        //
        // The web twin asked for the adapter's MAXIMUM limits, the same
        // full passthrough native uses. On a desktop that is harmless;
        // on a constrained device it is backwards — it tells the browser
        // to provision every ceiling at once when the program needs one.
        //
        // THE CENSUS behind the modest set (against WebGPU core
        // defaults; full table in this unit's commit body): the largest
        // uniform binding is GPUTileGrid at 16,400 B of 65,536; the
        // largest storage binding is Live Card Scratch at ~3.3 MiB of
        // 128 MiB; the widest workgroup is 16x16 = 256 invocations,
        // exactly the default and not over; texture array layers peak at
        // 225 of 256 (OPT_1b) and 2D dimension at 2048 of 8192
        // (PORT_5a). The last exceedance was
        // maxStorageBuffersPerShaderStage at 9 against a default of 8;
        // C6 (merged) demoted the field's head-pose window from
        // read-only storage to uniform, and TETRIS WALLET_0 demoted the
        // two occupier windows the same way — the room family now
        // counts 6.
        //
        // SO THIS IS NOW A PURE DEFAULTS REQUEST: nothing is named, and
        // a value-initialised wgpu::Limits means "every limit undefined,
        // use the default". If a future piece ever needs a ceiling above
        // a default, name it here — one line, beside this sentence, so
        // the exception is never silent.
        //
        // SAFETY: a mis-censused limit must degrade to today's behavior,
        // never to a black screen. Two nets. (1) If requestDevice fails,
        // the reason prints verbatim and the request is made again with
        // full passthrough. (2) If it SUCCEEDS but comes back below the
        // floor we censused — the failure mode if a value-initialised
        // wgpu::Limits ever meant "zero" rather than "undefined, use
        // default" — the device is discarded and the passthrough request
        // made anyway, because a device whose ceilings are zero fails at
        // pipeline creation later, far from this line and with nothing
        // pointing back here.
        void request_device_web(bool passthrough) {
            wgpu::DeviceDescriptor deviceDesc{};
            deviceDesc.label = "7T Device";
            // Deliberately unguarded — boot wants verbose errors.
            deviceDesc.SetUncapturedErrorCallback(
                [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView msg) {
                    std::cerr << "WebGPU Error (" << static_cast<int>(type) << "): "
                        << std::string_view(msg.data, msg.length) << std::endl;
                });
            // PORT_3a — the loss door. AllowSpontaneous so it fires from
            // the browser event loop without a pump. `this` is safe to
            // capture: App is heap-allocated and never deleted on the web
            // path, so Console outlives every callback.
            deviceDesc.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous,
                [this](const wgpu::Device&, wgpu::DeviceLostReason reason,
                       wgpu::StringView msg) {
                    deviceLost_ = true;
                    std::cerr << "[Device] LOST reason=" << static_cast<int>(reason)
                        << " : " << std::string_view(msg.data, msg.length) << std::endl;
                });

            wgpu::Limits limits{};
            if (passthrough) {
                adapter_.GetLimits(&limits);          // the old behavior, kept as the net
            }
            // else: every field stays undefined == every limit at its
            // core default. Post-C6 the program needs no exception.
            //
            // DO NOT "SIMPLIFY" THIS BACK TO PASSTHROUGH (PORT_6c). The
            // ground is COMPATIBILITY: a program that asks only for what
            // it uses runs on the widest set of devices, and the phone is
            // the target that decides. Asking the adapter for its maximum
            // narrows that set and buys nothing — the program never uses
            // the ceilings. L14 carries this as law.
            //
            // SHIP_0 U1 — the 11x timing claim that used to sit here
            // (62,517 vs 5,609 ms, "usable boot vs unusable") is
            // WITHDRAWN. It was one bisect on a machine whose runs vary
            // by an order of magnitude on identical code (native pipeline
            // creation has been observed at 70,459 and 205,527 ms). One
            // run from it is not evidence. Compatibility stands on its
            // own; do not re-argue this line with a number from this
            // laptop.
            deviceDesc.requiredLimits = &limits;

            // PORT_6a (1) — the request being issued, with its exceptions.
            if (passthrough) {
                std::cout << "[Device] requesting FULL ADAPTER PASSTHROUGH limits"
                             " (fallback path)\n";
            } else {
                std::cout << "[Device] requesting CORE DEFAULTS; exceptions carried: none"
                             " (C6 cleared maxStorageBuffersPerShaderStage 9->8)\n";
            }

            wgpu::FeatureName requiredFeatures[1] = { wgpu::FeatureName::TimestampQuery };
            if (adapter_.HasFeature(wgpu::FeatureName::TimestampQuery)) {
                deviceDesc.requiredFeatures = requiredFeatures;
                deviceDesc.requiredFeatureCount = 1;
            }

            adapter_.RequestDevice(&deviceDesc, wgpu::CallbackMode::AllowSpontaneous,
                [this, passthrough](wgpu::RequestDeviceStatus status, wgpu::Device device,
                       wgpu::StringView message) {
                    const char* which = passthrough
                        ? "full adapter passthrough"
                        : "core defaults + censused exceptions";
                    if (status != wgpu::RequestDeviceStatus::Success) {
                        std::cerr << "RequestDevice failed (" << which << "): "
                            << std::string_view(message.data, message.length) << "\n";
                        if (!passthrough) {
                            // PORT_6a (4) — the reissue, failure branch.
                            std::cerr << "[Device] REISSUING request with full adapter"
                                         " passthrough (modest request was rejected)\n";
                            request_device_web(true);
                            return;
                        }
                        bootState_ = BootState::Failed;
                        return;
                    }
                    // Net (2) — verify before adopting, while `device` is
                    // still the local (it is moved from just below).
                    if (!passthrough) {
                        wgpu::Limits got{};
                        device.GetLimits(&got);
                        // PORT_6a (2) — granted vs the censused floor, always
                        // printed, so the numbers are on the record whether or
                        // not they disagree.
                        std::cout << "[Device] granted vs floor:"
                            << " maxTextureDimension2D=" << got.maxTextureDimension2D << "/2048"
                            << " maxStorageBuffersPerShaderStage="
                            << got.maxStorageBuffersPerShaderStage << "/8"
                            << " maxUniformBufferBindingSize="
                            << got.maxUniformBufferBindingSize << "/65536\n";
                        bool below = false;
                        if (got.maxTextureDimension2D < 2048u) {
                            std::cerr << "[Device] BELOW FLOOR: maxTextureDimension2D granted "
                                << got.maxTextureDimension2D << ", floor 2048\n";
                            below = true;
                        }
                        if (got.maxStorageBuffersPerShaderStage < 8u) {
                            std::cerr << "[Device] BELOW FLOOR: maxStorageBuffersPerShaderStage"
                                         " granted " << got.maxStorageBuffersPerShaderStage
                                << ", floor 8\n";
                            below = true;
                        }
                        if (got.maxUniformBufferBindingSize < 65536u) {
                            std::cerr << "[Device] BELOW FLOOR: maxUniformBufferBindingSize"
                                         " granted " << got.maxUniformBufferBindingSize
                                << ", floor 65536\n";
                            below = true;
                        }
                        // PORT_6a (3) — the discard decision, BOTH ways. The
                        // no-discard line is the informative one: it is what
                        // says a [Device] LOST later did not come from here.
                        if (below) {
                            std::cerr << "[Device] DISCARDING the modest device — its `lost`"
                                         " promise will resolve as a CONSEQUENCE of this"
                                         " discard, not as a failure\n";
                            // PORT_6a (4) — the reissue, discard branch.
                            std::cerr << "[Device] REISSUING request with full adapter"
                                         " passthrough\n";
                            request_device_web(true);
                            return;
                        }
                        std::cout << "[Device] modest device accepted — NO DISCARD\n";
                    }
                    device_ = std::move(device);
                    queue_ = device_.GetQueue();
                    // PORT_6a (5) — the device the program actually keeps.
                    std::cout << "[Device] KEEPING the device from: " << which
                        << " (this is the one the frame loop runs on)\n";
                    bootState_ = BootState::Configuring;
                });
        }
#endif // __EMSCRIPTEN__

        bool initWebGPU() {
#ifdef __EMSCRIPTEN__
            // ── PORT_1b Region 2 (web): the async boot grammar ────
            // emdawnwebgpu (P0-verified): AllowSpontaneous callbacks fire
            // from the browser event loop between rAF turns — no pump
            // needed for the request chain itself. Adapter::GetLimits
            // exists, so the limits request is the full-adapter
            // passthrough exactly as native. Descriptor locals are
            // serialized during the RequestDevice call, so stack
            // lifetime suffices.
            instance_ = wgpu::CreateInstance(nullptr);
            if (!instance_) {
                std::cerr << "Failed to create WebGPU instance\n";
                return false;
            }
            // PORT_4a — arm the anchor (see its banner above). Copy, not
            // move: the member keeps its own reference and the anchor
            // adds a second one that outlives every object in the
            // program. Both are external references by Dawn's counting.
            g_instanceAnchor = instance_;
            bootState_ = BootState::RequestingAdapter;
            // SHIP_0 U2 — ASK FOR THE REAL GPU. Harmless on single-GPU
            // phones (the only adapter is the only answer); correct for a
            // real-time artwork; on dual-GPU Windows modern Chromium
            // honors it. Stack lifetime suffices for the same reason the
            // device descriptor's does, stated in the banner above: the
            // options are serialized during the call, not held.
            wgpu::RequestAdapterOptions adapterOpts{};
            adapterOpts.powerPreference = wgpu::PowerPreference::HighPerformance;
            instance_.RequestAdapter(&adapterOpts, wgpu::CallbackMode::AllowSpontaneous,
                [this](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter,
                       wgpu::StringView message) {
                    if (status != wgpu::RequestAdapterStatus::Success) {
                        std::cerr << "RequestAdapter failed: "
                            << std::string_view(message.data, message.length) << "\n";
                        bootState_ = BootState::Failed;
                        return;
                    }
                    adapter_ = std::move(adapter);
                    // SHIP_0 U2 — WITNESS IDENTITY, web half. Native
                    // enumerates every adapter and logs its pick
                    // (PROBE_1 C1); the web twin could not name its own
                    // silicon, so "the browser runs the HD 5500" was a
                    // presumption and every web METER number was
                    // uninterpretable without it. Now it is a logged fact
                    // or it is overturned.
                    //
                    // Empty fields print "?" rather than nothing: some
                    // builds redact these strings, and a blank field is
                    // indistinguishable from a line that never ran. A
                    // capture reading all "?" is the RESOLVE case — report
                    // it, do not plumb a fallback.
                    {
                        wgpu::AdapterInfo info{};
                        adapter_.GetInfo(&info);
                        auto sv = [](wgpu::StringView s) {
                            return (s.data && s.length)
                                ? std::string_view(s.data, s.length)
                                : std::string_view("?");
                        };
                        std::cout << "[Device] adapter: " << sv(info.vendor)
                                  << " | " << sv(info.architecture)
                                  << " | " << sv(info.device)
                                  << " | " << sv(info.description) << "\n";
                    }
                    bootState_ = BootState::RequestingDevice;
                    // PORT_5d — ask modestly first; the helper owns the
                    // descriptor, the limits census and the one retry.
                    request_device_web(/*passthrough=*/false);
                });
            return true;
#else
            dawnProcSetProcs(&dawn::native::GetProcs());

            // PIVOT_0d E1 — THE TOGGLE CHAIN, AT THE ROOT.
            //
            // `use_dxc` is ToggleStage::Adapter (dawn/native/Toggles.cpp),
            // and toggles flow DOWNWARD only: instance -> adapter
            // (Instance.cpp, adapterToggles.InheritFrom) -> device
            // (Adapter.cpp, deviceToggles.InheritFrom). PIVOT_0a chained it
            // on the DEVICE descriptor, which is downstream of the stage
            // that reads it, so it was accepted and ignored — no error, no
            // warning. The boot log said DXC and FXC compiled for 19,745 ms
            // and then access-violated. The instance descriptor is upstream
            // of both stages and is the only root that reaches an
            // adapter-stage toggle.
            //
            // The C++ descriptors are used rather than the C ones because
            // wgpu::DawnTogglesDescriptor and these two field names are
            // PROVEN against this Dawn — PIVOT_0a compiled them. Only the
            // cast and the constructor arity are new, and the cast is the
            // documented layout-compatibility between wgpu:: and WGPU
            // structs.
            //
            // LIFETIME: `kDxcToggle` is static; `toggles` and `idesc` need
            // only outlive the emplace() call, and they do — Dawn copies
            // what it needs out of the descriptor during construction.
            static const char* const kDxcToggle[] = { "use_dxc" };
            wgpu::DawnTogglesDescriptor toggles{};
            wgpu::InstanceDescriptor idesc{};
            if constexpr (kCompilerPlan == CompilerPlan::D3D12_Dxc) {
                toggles.enabledToggleCount = 1;
                toggles.enabledToggles = kDxcToggle;
                idesc.nextInChain = &toggles;
            }
            // Vulkan and D3D12_Fxc chain nothing: Vulkan reaches SPIR-V
            // through Tint with no toggle, and Fxc is the untoggled path
            // kept for archaeology.

            // Construct instance in place (non-copyable, non-movable)
            instance_.emplace(reinterpret_cast<const WGPUInstanceDescriptor*>(&idesc));

            std::vector<dawn::native::Adapter> adapters = instance_->EnumerateAdapters();
            if (adapters.empty()) {
                std::cerr << "No WebGPU adapters found\n";
                return false;
            }

            // The platform line: Dawn's own 20-byte SHA1
            // (all-zero when the build is unhashed).
#if T7_DAWN_VERSION
            {
                static constexpr char hexd[] = "0123456789abcdef";
                std::string dawnRev; dawnRev.reserve(40);
                for (uint8_t b : dawn::kDawnVersion) {
                    dawnRev += hexd[b >> 4]; dawnRev += hexd[b & 0x0F];
                }
                std::cout << "[Console] Dawn revision: " << dawnRev << "\n";
            }
#else
            std::cout << "[Console] Dawn revision: unavailable "
                         "(Version_autogen.h not on the include path)\n";
#endif
#ifdef NDEBUG
            std::cout << "[Console] Build: Release\n";
#else
            std::cout << "[Console] Build: Debug\n";
#endif

            // PROBE_1 C1 — the adapter log: every adapter Dawn
            // enumerates, then the pick. The tree records what it
            // runs on; every METER number is uninterpretable
            // without this line.
            auto sv = [](wgpu::StringView s) {
                return std::string_view(s.data, s.length);
            };
            auto backend_name = [](wgpu::BackendType b) {
                switch (b) {
                case wgpu::BackendType::D3D12:    return "D3D12";
                case wgpu::BackendType::D3D11:    return "D3D11";
                case wgpu::BackendType::Vulkan:   return "Vulkan";
                case wgpu::BackendType::Metal:    return "Metal";
                case wgpu::BackendType::OpenGL:   return "OpenGL";
                case wgpu::BackendType::OpenGLES: return "OpenGLES";
                case wgpu::BackendType::Null:     return "Null";
                default:                          return "?";
                }
            };
            auto type_name = [](wgpu::AdapterType t) {
                switch (t) {
                case wgpu::AdapterType::DiscreteGPU:   return "discrete";
                case wgpu::AdapterType::IntegratedGPU: return "integrated";
                case wgpu::AdapterType::CPU:           return "CPU";
                default:                               return "unknown";
                }
            };
            for (size_t i = 0; i < adapters.size(); i++) {
                wgpu::Adapter a = wgpu::Adapter(adapters[i].Get());
                wgpu::AdapterInfo info{};
                a.GetInfo(&info);
                std::cout << "[Console] Adapter " << i << ": "
                    << type_name(info.adapterType) << " / "
                    << backend_name(info.backendType) << " | "
                    << sv(info.device) << " (" << sv(info.description)
                    << ") vendor=" << sv(info.vendor) << "\n";
            }

            // Adapter selection (landed, PROBE_1): DiscreteGPU
            // outranks integrated; the backend breaks ties.
            // Falls back to index 0.
            //
            // Dawn's native Instance accepts wgpu::RequestAdapterOptions
            // (a chain root for toggles; carries backendType). This code
            // deliberately enumerates UNFILTERED so the boot log lists all
            // adapters, and picks by the scorer below; kCompilerPlan's
            // backend preference is a tie-break, not a guarantee — the
            // effect witnesses after CreateDevice report what was actually
            // picked and enabled. Toggles ride the instance descriptor:
            // instance -> adapter -> device inheritance
            // (dawn/native Instance.cpp, Adapter.cpp).
            constexpr wgpu::BackendType kPreferredBackend =
                (kCompilerPlan == CompilerPlan::Vulkan) ? wgpu::BackendType::Vulkan
                                                        : wgpu::BackendType::D3D12;
            size_t adapterPick = 0;
            {
                int best = -1;
                for (size_t i = 0; i < adapters.size(); i++) {
                    wgpu::Adapter a = wgpu::Adapter(adapters[i].Get());
                    wgpu::AdapterInfo info{};
                    a.GetInfo(&info);
                    int score =
                        (info.adapterType == wgpu::AdapterType::DiscreteGPU ? 2 : 0)
                      + (info.backendType == kPreferredBackend              ? 1 : 0);
                    if (score > best) { best = score; adapterPick = i; }
                }
            }
            dawn::native::Adapter& nativeAdapter = adapters[adapterPick];
            wgpu::Adapter adapter = wgpu::Adapter(nativeAdapter.Get());
            // PIVOT_0d E1 — the index alone made the reader cross-reference
            // the enumeration above to learn which backend won. The scorer's
            // backend preference is a TIE-BREAK, not a guarantee, so the
            // backend that was actually picked is the fact worth printing.
            // (The loop's `info` is loop-local; re-fetch for the winner.)
            wgpu::AdapterInfo pickedInfo{};
            adapter.GetInfo(&pickedInfo);
            std::cout << "[Console] Adapter selected: index=" << adapterPick
                << " backend=" << backend_name(pickedInfo.backendType) << "\n";

            wgpu::DeviceDescriptor deviceDesc{};
            deviceDesc.label = "7T Device";

            // PIVOT_0d E2 — the device-level toggle chain that stood here
            // is GONE. It was inert (see the instance construction above,
            // where the chain now lives) and a second chain root would be
            // two homes for one fact.

            deviceDesc.SetUncapturedErrorCallback(
                [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
                    std::cerr << "WebGPU Error (" << static_cast<int>(type) << "): "
                        << std::string_view(message.data, message.length) << std::endl;
                });
            // PORT_3a — the loss door, installed on BOTH twins. Native
            // loss is rarer but real (TDR, GPU reset, driver update), the
            // honest-death policy is the same, and one shape in two
            // branches is cheaper to keep true than two policies.
            deviceDesc.SetDeviceLostCallback(wgpu::CallbackMode::AllowSpontaneous,
                [this](const wgpu::Device&, wgpu::DeviceLostReason reason,
                       wgpu::StringView message) {
                    deviceLost_ = true;
                    std::cerr << "[Device] LOST reason=" << static_cast<int>(reason)
                        << " : " << std::string_view(message.data, message.length) << std::endl;
                });

            // Query adapter limits and request the full capacity.
            // The default maxStorageBuffersPerShaderStage (8) is too tight
            // once generative objects each add render bindings.
            wgpu::Limits adapterLimits{};
            adapter.GetLimits(&adapterLimits);

            deviceDesc.requiredLimits = &adapterLimits;

            // THE FRAME METER (timestamp-query): request the feature when
            // the adapter carries it; consumers check device.HasFeature.
            // Absent → no unsafe-API chasing; downstream degrades loudly
            // to CPU rows only.
            //
            // The REQUEST stays unconditional on purpose. Whether the meter
            // arms is a cartridge decision (INSTRUMENTS.frame_meter,
            // core/instruments.hpp) and this is the host — a granted feature
            // costs nothing until a pass writes a timestamp, and keeping the
            // request here means turning the instrument back on is one define
            // in the cartridge, with no host edit and no second door to find.
            wgpu::FeatureName requiredFeatures[1] = { wgpu::FeatureName::TimestampQuery };
            if (adapter.HasFeature(wgpu::FeatureName::TimestampQuery)) {
                deviceDesc.requiredFeatures = requiredFeatures;
                deviceDesc.requiredFeatureCount = 1;
            }

            std::cout << "[Console] Adapter limits:"
                << " storageBuffers/stage=" << adapterLimits.maxStorageBuffersPerShaderStage
                << " uniformBuffers/stage=" << adapterLimits.maxUniformBuffersPerShaderStage
                << " bindingsPerGroup=" << adapterLimits.maxBindingsPerBindGroup
                << "\n";

            // PIVOT_0 E4 — every future log self-attributes. A [Pipeline]
            // table with no compiler beside it is uninterpretable, and
            // this campaign exists because one was.
            //
            // PIVOT_0d E3 — labelled (request), because that is all it is.
            // This line prints what the program ASKED FOR, before the
            // device exists; it said DXC while FXC compiled, for 19,745 ms,
            // and was not lying — it was answering a different question.
            // The EFFECT is reported after CreateDevice by the toggles
            // witness. P6 wants both halves: the request and the effect.
            std::cout << "[Console] Compiler plan (request): "
                << compiler_plan_name(kCompilerPlan) << "\n";

            // PROBE_1 C1 — the full enumerated feature list (numeric;
            // settles LEDGER_1 F4-2 at zero cost). Nothing is
            // requested here beyond what the tree already requests.
            {
                wgpu::SupportedFeatures feats{};
                adapter.GetFeatures(&feats);
                std::cout << "[Console] Adapter features (" << feats.featureCount << "):";
                for (size_t i = 0; i < feats.featureCount; i++) {
                    std::cout << " " << static_cast<uint32_t>(feats.features[i]);
                }
                std::cout << "\n";
                std::cout << "[Console] feature multi-draw-indirect="
                    << (adapter.HasFeature(wgpu::FeatureName::MultiDrawIndirect) ? "YES" : "no")
                    << " timestamp-query="
                    << (adapter.HasFeature(wgpu::FeatureName::TimestampQuery) ? "YES" : "no")
                    << "\n";
            }

            device_ = adapter.CreateDevice(&deviceDesc);
            if (!device_) {
                std::cerr << "Failed to create WebGPU device\n";
                return false;
            }

            // PIVOT_0d E2 — THE EFFECT WITNESS. Everything above this line
            // is a request; this is the answer. dawn::native::GetTogglesUsed
            // reports the toggles Dawn ACTUALLY enabled on the device, after
            // its own inheritance and after any ForceSet it applied.
            //
            // Paid for by PIVOT_0a: `use_dxc` was chained on the DEVICE
            // descriptor, and it is a ToggleStage::Adapter toggle
            // (dawn/native/Toggles.cpp), so it was silently inert. The boot
            // log said "Compiler plan: DXC" and FXC compiled anyway. A
            // switch that cannot be seen to have fired is indistinguishable
            // from one that never fired (P6) — so the switch testifies now.
            //
            // The full list, not a use_dxc grep: it is one boot line, and
            // the next toggle mystery will not be this one.
            {
                auto used = dawn::native::GetTogglesUsed(device_.Get());
                std::cout << "[Console] Toggles used (" << used.size() << "):";
                for (size_t i = 0; i < used.size(); i++) {
                    std::cout << (i ? ", " : " ") << used[i];
                }
                std::cout << "\n";
            }

            queue_ = device_.GetQueue();
            adapter_ = adapter;

            return true;
#endif // __EMSCRIPTEN__
        }

        bool initSurface() {
            wgpu::SurfaceDescriptor surfaceDesc{};
#ifdef __EMSCRIPTEN__
            // ── PORT_1b Region 3 (web): the canvas surface ────────
            // P0-verified spelling: wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector
            // (dawn.json "emscripten surface source canvas HTML selector",
            // chained into the surface descriptor; member `selector`).
            wgpu::EmscriptenSurfaceSourceCanvasHTMLSelector canvasSource{};
            canvasSource.selector = "#canvas";
            surfaceDesc.nextInChain = &canvasSource;

            surface_ = instance_.CreateSurface(&surfaceDesc);
#else
#if defined(_WIN32)
            wgpu::SurfaceSourceWindowsHWND hwndSource{};
            hwndSource.hwnd = glfwGetWin32Window(window_);
            hwndSource.hinstance = GetModuleHandle(nullptr);
            surfaceDesc.nextInChain = &hwndSource;
#elif defined(__linux__)
            wgpu::SurfaceSourceXlibWindow x11Source{};
            x11Source.display = glfwGetX11Display();
            x11Source.window = glfwGetX11Window(window_);
            surfaceDesc.nextInChain = &x11Source;
#endif

            surface_ = wgpu::Instance(instance_->Get()).CreateSurface(&surfaceDesc);
#endif // __EMSCRIPTEN__

            wgpu::SurfaceCapabilities caps;
            surface_.GetCapabilities(adapter_, &caps);

            colorFormat_ = caps.formats[0];

            surfaceConfig_.device = device_;
            surfaceConfig_.format = colorFormat_;
            // FRAME_0 — CURRENT, not initial. pump_boot allocates the
            // depth buffer from currentWidth_/currentHeight_, so reading
            // a different pair here is a divergence waiting for someone
            // to poll events during boot. Today nothing does and the two
            // are seeded equal in init(), so this is a no-op — but
            // FRAME_0 is what makes a resize reachable before this line,
            // and initialWidth_/Height_ now have exactly one job:
            // glfwCreateWindow's arguments.
            surfaceConfig_.width = currentWidth_;
            surfaceConfig_.height = currentHeight_;
            surfaceConfig_.presentMode = wgpu::PresentMode::Fifo;
            surfaceConfig_.alphaMode = wgpu::CompositeAlphaMode::Opaque;
            surface_.Configure(&surfaceConfig_);

            return true;
        }

        void createDepthBuffer(uint32_t w, uint32_t h) {
            wgpu::TextureDescriptor depthDesc{};
            depthDesc.label = "Depth Texture";
            depthDesc.size = { w, h, 1 };
            depthDesc.format = depthFormat_;
            depthDesc.usage = wgpu::TextureUsage::RenderAttachment;
            depthTexture_ = device_.CreateTexture(&depthDesc);
            depthView_ = depthTexture_.CreateView();
        }


        // ═══ §3 FRAME LIFECYCLE ══════════════════════════════════
        //
        // Call order each frame:
        //   begin_frame() → [update cartridges] → acquire_surface_texture()
        //   → [encode & submit] → present() → [back to running()]

    private:
#ifdef __EMSCRIPTEN__
        // PORT_3c — clamp the EFFECTIVE device-pixel ratio, web only.
        //
        // Under contrib.glfw3 the GLFW contract holds: window size is CSS
        // pixels, framebuffer size is CSS x devicePixelRatio. Their ratio
        // IS the DPR, so the cap needs no browser API of its own — and if
        // Hi-DPI is not in play the two are equal, the ratio is 1, and
        // this is a no-op. That is the 1x-display acceptance, structurally.
        //
        // Applied BEFORE the change comparison in begin_frame, which is
        // load-bearing: the capped size is what lands in currentWidth_,
        // so the next frame compares capped against capped and the
        // resize branch stays quiet. Capping after the comparison would
        // reconfigure the surface every single frame.
        //
        // The canvas ELEMENT is sized by the library from #frame
        // (FRAME_0, in initGLFW above) — NOT by a CSS rule on the canvas,
        // which is what this comment used to claim and what the element's
        // own inline style always overrode. That CSS size is independent
        // of the backing-store size this caps. Fewer pixels, same layout.
        void apply_pixel_cap(int& fbWidth, int& fbHeight) const {
            int winW = 0, winH = 0;
            glfwGetWindowSize(window_, &winW, &winH);
            if (winW <= 0 || winH <= 0 || fbWidth <= 0 || fbHeight <= 0) return;
            const float ratio = static_cast<float>(fbWidth) / static_cast<float>(winW);
            if (ratio <= MAX_DEVICE_PIXEL_RATIO) return;
            fbWidth  = static_cast<int>(static_cast<float>(winW) * MAX_DEVICE_PIXEL_RATIO);
            fbHeight = static_cast<int>(static_cast<float>(winH) * MAX_DEVICE_PIXEL_RATIO);
            if (fbWidth  < 1) fbWidth  = 1;
            if (fbHeight < 1) fbHeight = 1;
        }

        // ═══ FRAME_1 U0 — TEMPORARY INSTRUMENTATION ══════════════════
        //
        // REMOVABLE. This whole method, the two locals that feed it in
        // begin_frame, and its one call site come out together once the
        // phone numbers have named the defect. Nothing depends on it.
        //
        // It prints through out() — Emscripten's Module.print — and NOT
        // console.log, deliberately: index.html's onLine() feeds
        // Module.print into the "details" panel, so these numbers are
        // readable ON THE PHONE by tapping DETAILS. A devtools-only line
        // would be useless on the device that has the defect.
        //
        // Fires only inside the resize branch, so a steady frame loop
        // prints nothing.
        void frame1_report(int fbPreW, int fbPreH, int fbPostW, int fbPostH) const {
            int winW = 0, winH = 0;
            glfwGetWindowSize(window_, &winW, &winH);
            EM_ASM({
                var f  = document.getElementById('frame');
                var c  = document.getElementById('canvas');
                var vv = window.visualViewport;
                var r2 = function (n) { return Math.round(n * 100) / 100; };
                // out() is Emscripten's Module.print. Guarded because this
                // block's only job is to produce numbers on a device that
                // cannot be tested from here — if the symbol is ever
                // absent the line must still reach devtools rather than
                // throw and take the frame with it.
                var say = (typeof out === 'function') ? out : console.log;
                say('[FRAME_1]'
                  + ' glfwWin='   + $0 + 'x' + $1
                  + ' fbPreCap='  + $2 + 'x' + $3
                  + ' fbPostCap=' + $4 + 'x' + $5
                  + ' inner='     + window.innerWidth + 'x' + window.innerHeight
                  + ' visualVP='  + (vv ? r2(vv.width) + 'x' + r2(vv.height) : 'absent')
                  + ' dpr='       + window.devicePixelRatio
                  + ' appH='      + getComputedStyle(document.documentElement)
                                      .getPropertyValue('--app-h').trim()
                  + ' frameClient=' + (f ? f.clientWidth + 'x' + f.clientHeight : 'ABSENT')
                  + ' canvasCSS='   + (c ? (c.style.width || '(none)') + 'x'
                                          + (c.style.height || '(none)') : 'ABSENT')
                  + ' canvasBuf='   + (c ? c.width + 'x' + c.height : 'ABSENT'));
            }, winW, winH, fbPreW, fbPreH, fbPostW, fbPostH);
        }
#endif

    public:
        float begin_frame() {
            glfwPollEvents();
#ifdef __EMSCRIPTEN__
            emit_touch_intents();   // SHIP_1 — the frame tick consumes the gestures
#endif

            // Handle resize
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
#ifdef __EMSCRIPTEN__
            const int fbPreCapW = fbWidth, fbPreCapH = fbHeight;   // FRAME_1 (temporary)
            apply_pixel_cap(fbWidth, fbHeight);   // PORT_3c — before the compare
#endif
            if (fbWidth > 0 && fbHeight > 0 &&
                (static_cast<uint32_t>(fbWidth) != currentWidth_ ||
                    static_cast<uint32_t>(fbHeight) != currentHeight_)) {

                currentWidth_ = static_cast<uint32_t>(fbWidth);
                currentHeight_ = static_cast<uint32_t>(fbHeight);
                surfaceConfig_.width = currentWidth_;
                surfaceConfig_.height = currentHeight_;
                surface_.Configure(&surfaceConfig_);
                createDepthBuffer(currentWidth_, currentHeight_);
#ifdef __EMSCRIPTEN__
                frame1_report(fbPreCapW, fbPreCapH, fbWidth, fbHeight);   // FRAME_1 (temporary)
#endif
            }

            // Delta time
            auto currentTime = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(currentTime - lastTime_).count();
            lastTime_ = currentTime;

            // PORT_1b: the dt clamp, lifted verbatim from the dormant
            // core/clock.hpp (retired this commit) — "Clamp dt to avoid
            // spiral of death", cap 0.1f (100 ms). Inert at native frame
            // rates; essential across a browser tab-suspend, where rAF
            // hands back a multi-second gap.
            dt = std::clamp(dt, 0.0f, 0.1f);

            return dt;
        }

        bool acquire_surface_texture() {
            surface_.GetCurrentTexture(&surfaceTexture_);
            if (surfaceTexture_.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
                surfaceTexture_.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
                return false;
            }
            backbuffer_ = surfaceTexture_.texture.CreateView();
            return true;
        }

        void present() {
#ifndef __EMSCRIPTEN__
            surface_.Present();
#endif
            // ── PORT_1b Region 4 (web): no-op — presentation is implicit
            // at rAF return. P0-verified: emdawnwebgpu's wgpuSurfacePresent
            // exists but ABORTS ("wgpuSurfacePresent is unsupported (use
            // requestAnimationFrame via html5.h instead)"), so it must not
            // be called.
        }

        bool running() const {
            return window_ && !glfwWindowShouldClose(window_);
        }


        // ═══ §4 INPUT ════════════════════════════════════════════
        //
        // Producer: inject_* methods, called by GLFW callbacks during
        //           glfwPollEvents(). These push events into the vector.
        //
        // Consumer: input_events() and clear_input_events(), called by
        //           the main loop after begin_frame().

    public:
        // ── Producer (GLFW callbacks → event vector) ─────────────

        void inject_key_event(int key, int action) {
            InputEvent event{};
            event.type = (action == GLFW_PRESS || action == GLFW_REPEAT)
                ? InputEvent::Type::KeyDown
                : InputEvent::Type::KeyUp;
            event.key = key;

            // Convert to character for printable keys
            if (key >= GLFW_KEY_A && key <= GLFW_KEY_Z) {
                event.character = 'A' + (key - GLFW_KEY_A);
            }
            else if (key == GLFW_KEY_SEMICOLON) {
                event.character = ';';
            }
            else if (key == GLFW_KEY_LEFT_BRACKET) {
                event.character = '[';
            }
            else if (key == GLFW_KEY_RIGHT_BRACKET) {
                event.character = ']';
            }
            else {
                event.character = 0;
            }

            inputEvents_.push_back(event);
        }

        // The one differentiator. GLFW reports absolute positions; the
        // tree consumes only deltas, so the previous position is console
        // state — not a static hiding in a callback body.
        void feed_cursor(double x, double y) {
#ifdef __EMSCRIPTEN__
            // SHIP_1 U1 — THE BACKSTOP. claim_touch_stream() removed the
            // port's touch handlers, so nothing should synthesize a
            // cursor from a finger any more. Should is not a guarantee:
            // the port re-registers its listeners whenever it rebuilds
            // them, and ATMOS_0 made a fullscreen transition part of the
            // normal entry. If that ever resurrects the emulation, this
            // turns a silent double-drive — two consumers fighting over
            // one look delta — into nothing at all. The origin is kept
            // current so a real mouse afterwards does not jump.
            if (any_touch_active()) {
                lastCursorX_ = x;
                lastCursorY_ = y;
                return;
            }
#endif
            if (!cursorPrimed_) {
                lastCursorX_ = x;
                lastCursorY_ = y;
                cursorPrimed_ = true;
                return;                 // no event: the first sample is an origin
            }
            inject_mouse_move(static_cast<float>(x - lastCursorX_),
                              static_cast<float>(y - lastCursorY_));
            lastCursorX_ = x;
            lastCursorY_ = y;
        }

        // Declares a seam: the next sample re-origins instead of differencing.
        void unprime_cursor() { cursorPrimed_ = false; }

        void inject_mouse_move(float dx, float dy) {
            InputEvent event{};
            event.type = InputEvent::Type::MouseMove;
            event.x = dx;
            event.y = dy;
            inputEvents_.push_back(event);
        }

        void inject_mouse_button(int button, bool pressed) {
#ifdef __EMSCRIPTEN__
            if (any_touch_active()) return;   // the backstop's other half
#endif
            InputEvent event{};
            event.type = InputEvent::Type::MouseButton;
            event.button = button;
            event.pressed = pressed;
            inputEvents_.push_back(event);
        }

        void inject_scroll(float delta) {
            InputEvent event{};
            event.type = InputEvent::Type::Scroll;
            event.y = delta;
            inputEvents_.push_back(event);
        }

#ifdef __EMSCRIPTEN__
        // ═══ SHIP_1 U1 — THE TOUCH STREAM, CLAIMED ═══════════════
        //
        // THE PROBLEM, precisely. contrib.glfw3 registers its own
        // touchstart/move/end/cancel handlers on the canvas inside
        // glfwCreateWindow and converts each one to setCursorPos +
        // mouse-button-left — which is why a drag already rotates the
        // camera on a phone today, by accident. The port exposes NO
        // lever to turn that off: not a port option (disableWarning,
        // disableJoystick, disableMultiWindow, disableWebGL2,
        // optimizationLevel — that is the whole list), not a window
        // hint, not a function in emscripten_glfw3.h.
        //
        // WHAT THIS ACTUALLY REMOVES, corrected against the pinned port
        // (FRAME_0 recon). Window.cpp registers exactly ONE touch
        // listener on the canvas — touchstart — while Context.cpp
        // registers touchstart/move/cancel/end on
        // EMSCRIPTEN_EVENT_TARGET_DOCUMENT. Nulling the canvas target
        // therefore removes one handler and leaves four live, so the
        // port's emulation is NOT gone: what actually prevents the
        // double-drive is the any_touch_active() backstop below, plus
        // the ordering. The port's document listeners are BUBBLE phase
        // (Events.h passes useCapture=false), and ours sit on the canvas
        // — the target — so ours run FIRST and the table is populated
        // before the port's synthesis reaches feed_cursor. Jean's
        // ruling to keep the backstop was not a belt; it is the load-
        // bearing half. Nulling the document target as well is a real
        // option and a separate decision, not a silent one.
        //
        // THE LEVER IS HTML5.H'S OWN. In JSEvents.registerOrRemoveHandler
        // a NON-null callback appends a listener and leaves any existing
        // one in place — so simply registering ours would give two live
        // consumers of one finger, which is the failure this whole unit
        // exists to prevent. A NULL callback takes the other branch and
        // removes every handler matching (target, eventType), unbinding
        // with the useCapture each was stored with. So: null first, ours
        // second.
        //
        // The target string is load-bearing. lib_emscripten_glfw3.js
        // does specialHTMLTargets["Module['canvas']"] = Module.canvas at
        // glfwInit, and findEventTarget checks specialHTMLTargets before
        // querySelector — so this literal resolves to the same element
        // the port used, which is the only reason the removal matches.
        void claim_touch_stream() {
            // 1 — the port's handlers, off.
            emscripten_set_touchstart_callback(TOUCH_TARGET, nullptr, true, nullptr);
            emscripten_set_touchmove_callback(TOUCH_TARGET, nullptr, true, nullptr);
            emscripten_set_touchend_callback(TOUCH_TARGET, nullptr, true, nullptr);
            emscripten_set_touchcancel_callback(TOUCH_TARGET, nullptr, true, nullptr);

            // 2 — ours, on. Every handler returns true, which makes
            // html5.h call preventDefault — and THAT is what suppresses
            // the browser's compatibility mouse events. Without it the
            // emulation would come back through the port's MOUSE door
            // after we closed its touch one.
            emscripten_set_touchstart_callback(TOUCH_TARGET, this, true, &Console::touch_cb);
            emscripten_set_touchmove_callback(TOUCH_TARGET, this, true, &Console::touch_cb);
            emscripten_set_touchend_callback(TOUCH_TARGET, this, true, &Console::touch_cb);
            emscripten_set_touchcancel_callback(TOUCH_TARGET, this, true, &Console::touch_cb);

            std::cout << "[Touch] Claimed the canvas touch stream ("
                << TOUCH_TARGET << ")\n";
        }

        bool any_touch_active() const {
            for (const TouchPoint& t : touches_) if (t.active) return true;
            return false;
        }

    private:
        // ── The table ────────────────────────────────────────────
        // Four slots: two per half is every named gesture, and the
        // vocabulary says extras are ignored rather than queued.
        static constexpr int MAX_TRACKED_TOUCHES = 4;

        TouchPoint* find_touch(int id) {
            for (TouchPoint& t : touches_) if (t.active && t.id == id) return &t;
            return nullptr;
        }

        // Primary = earliest born in that half; secondary = next.
        // Birth order, not slot order: a lifted finger frees its slot and
        // the survivors must not be reshuffled by who happens to sit
        // where.
        TouchPoint* half_touch(bool left, int rank) {
            TouchPoint* out = nullptr;
            uint64_t best = UINT64_MAX;
            uint64_t floor_seq = 0;
            for (int r = 0; r <= rank; r++) {
                out = nullptr; best = UINT64_MAX;
                for (TouchPoint& t : touches_) {
                    if (!t.active || t.left != left) continue;
                    if (t.seq < floor_seq) continue;
                    if (t.seq < best) { best = t.seq; out = &t; }
                }
                if (!out) return nullptr;
                floor_seq = best + 1;
            }
            return out;
        }

        int half_count(bool left) const {
            int n = 0;
            for (const TouchPoint& t : touches_) if (t.active && t.left == left) n++;
            return n;
        }

        // THE MIDLINE, in the same CSS pixels the touch reports. Read
        // per event rather than cached: a rotation changes it, and a
        // stale midline would classify a thumb into the wrong half for
        // one gesture — the exact bug the born-in rule exists to avoid.
        float midline_css() const {
            int w = 0, h = 0;
            glfwGetWindowSize(window_, &w, &h);
            (void)h;
            return w > 0 ? static_cast<float>(w) * 0.5f : 0.0f;
        }

        void clear_all_touches() {
            for (TouchPoint& t : touches_) t = TouchPoint{};
            // Every accumulator too: a cancelled gesture must not leave
            // half a look delta behind to arrive on the next tick.
            touchLookDx_ = 0.0f;
            touchLookDy_ = 0.0f;
            touchZoomAccum_ = 0.0f;
            pinchDeclared_ = false;
            rightTapPending_ = false;
            tapAuraPending_ = false;
            tapPossessPending_ = false;
        }

        // ── U4: the right-half pair ──────────────────────────────
        static float separation(const TouchPoint& a, const TouchPoint& b) {
            const float dx = a.x - b.x, dy = a.y - b.y;
            return std::sqrt(dx * dx + dy * dy);
        }

        static bool touch_cb(int eventType, const EmscriptenTouchEvent* e, void* userData) {
            auto* self = static_cast<Console*>(userData);
            if (self && e) self->on_touch(eventType, *e);
            return true;   // preventDefault — see claim_touch_stream
        }

        void on_touch(int eventType, const EmscriptenTouchEvent& e) {
            if (eventType == EMSCRIPTEN_EVENT_TOUCHCANCEL) {
                // U1's rule, flat: cancel clears EVERYTHING. A cancelled
                // gesture has no ending to interpret, so the only honest
                // reading is that no finger is down.
                clear_all_touches();
                return;
            }

            const float mid = midline_css();

            for (int i = 0; i < e.numTouches; i++) {
                const EmscriptenTouchPoint& p = e.touches[i];
                if (!p.isChanged) continue;

                const float px = static_cast<float>(p.targetX);
                const float py = static_cast<float>(p.targetY);

                if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART) {
                    if (find_touch(p.identifier)) continue;      // already tracked
                    TouchPoint* slot = nullptr;
                    for (TouchPoint& t : touches_) if (!t.active) { slot = &t; break; }
                    if (!slot) continue;                          // extras are ignored
                    slot->id      = p.identifier;
                    slot->active  = true;
                    slot->left    = (px < mid);                   // decided ONCE
                    slot->seq     = nextTouchSeq_++;
                    slot->x = slot->x0 = px;
                    slot->y = slot->y0 = py;
                    slot->t0      = e.timestamp;
                    slot->slopped = false;

                    // U4 — THE PAIR FORMS. Undecided on purpose: two
                    // fingers on the right half are a pinch, a possess
                    // tap, or nothing yet, and which one is not knowable
                    // at the moment they land. The pair is born PENDING
                    // and declares itself later, by separating or by
                    // outliving TAP_MS.
                    if (!slot->left && half_count(false) == 2) {
                        TouchPoint* a = half_touch(false, 0);
                        TouchPoint* b = half_touch(false, 1);
                        rightPairT0_   = e.timestamp;
                        rightPairSep_  = (a && b) ? separation(*a, *b) : 0.0f;
                        pinchDeclared_ = false;
                    }
                    continue;
                }

                TouchPoint* t = find_touch(p.identifier);
                if (!t) continue;

                if (eventType == EMSCRIPTEN_EVENT_TOUCHMOVE) {
                    // The step since this finger's own last report. Taken
                    // BEFORE the position is updated, and per-finger — so
                    // a second touch arriving cannot inject a phantom
                    // jump into the first one's delta.
                    const float step_x = px - t->x;
                    const float step_y = py - t->y;
                    t->x = px;
                    t->y = py;

                    // ── U3: LOOK ────────────────────────────────
                    // RIGHT half, ONE touch. The deltas are RAW here and
                    // scaled at consumption (the ledger's sampling law:
                    // accumulate in the callback, spend once on the frame
                    // tick), so a browser that fires three touchmoves in
                    // one frame turns them into one look, not three.
                    //
                    // The one-touch gate is also the pinch suspension:
                    // while two fingers hold the right half this stops
                    // accumulating, and because the delta is measured
                    // from each finger's OWN previous position, the
                    // survivor of a lift resumes with a small step
                    // instead of a jump. No origin to reset by hand.
                    if (!t->left && half_count(false) == 1 && !rightTapPending_) {
                        touchLookDx_ += step_x;
                        touchLookDy_ += step_y;
                    }
                    // The tap window closes on its own: once the pair
                    // has outlived TAP_MS the survivor is just a look
                    // again. Checked on movement because that is the
                    // only moment the answer can matter.
                    if (rightTapPending_
                        && (e.timestamp - rightPairT0_) > TouchControls::TAP_MS) {
                        rightTapPending_ = false;
                    }

                    // ── U4: PINCH ───────────────────────────────
                    // Only the right half, only as a pair. Separation is
                    // read from the pair as a whole rather than from
                    // either finger's motion, so sliding both fingers
                    // across the glass together zooms nothing.
                    if (!t->left && half_count(false) == 2) {
                        TouchPoint* a = half_touch(false, 0);
                        TouchPoint* b = half_touch(false, 1);
                        if (a && b) {
                            const float sep = separation(*a, *b);
                            if (!pinchDeclared_) {
                                // THE DISAMBIGUATOR. A pinch declares
                                // itself two ways — by moving enough to
                                // mean it, or by lasting longer than a
                                // tap could. Until one of them fires the
                                // pair is still a possible tap, and
                                // nothing zooms.
                                if (std::fabs(sep - rightPairSep_) > TouchControls::PINCH_DECLARE
                                    || (e.timestamp - rightPairT0_) > TouchControls::TAP_MS) {
                                    pinchDeclared_ = true;
                                    // NO RE-BASELINE. The travel that
                                    // proved this was a pinch is real
                                    // pinch travel, and it is spent
                                    // below against the separation the
                                    // pair was BORN with. Discarding it
                                    // would be a dead zone rather than a
                                    // classifier: a browser that
                                    // coalesces a whole fast spread into
                                    // one touchmove would declare the
                                    // pinch and zoom nothing, and the
                                    // faster the gesture the more of it
                                    // would vanish.
                                }
                            }
                            if (pinchDeclared_) {
                                const float dsep = sep - rightPairSep_;
                                rightPairSep_ = sep;
                                // SPREAD = IN. zoom_delta adds to camera
                                // distance and closer IS zoomed in, so
                                // growing separation must go negative.
                                touchZoomAccum_ += -dsep * TouchControls::PINCH_SENS;
                            }
                        }
                    }

                    const float ddx = t->x - t->x0, ddy = t->y - t->y0;
                    if (ddx * ddx + ddy * ddy >
                        TouchControls::TAP_SLOP * TouchControls::TAP_SLOP) {
                        t->slopped = true;   // never a tap again
                    }
                    continue;
                }

                if (eventType == EMSCRIPTEN_EVENT_TOUCHEND) {
                    t->x = px;
                    t->y = py;
                    on_touch_lift(*t, e.timestamp);
                    *t = TouchPoint{};
                    continue;
                }
            }
        }

        // ── U5: THE CLEAN TAPS ───────────────────────────────────
        // Both fire on RELEASE, not on press. A press-fired toggle
        // commits before the gesture has said what it is — and the
        // right-half tap in particular shares its opening frames with a
        // pinch, so there is nothing to commit to yet.
        //
        // Called while `t` is still active, so half_count includes it.
        void on_touch_lift(TouchPoint& t, double now_ms) {
            const bool clean = !t.slopped
                && (now_ms - t.t0) <= TouchControls::TAP_MS;

            if (t.left) {
                // AURA — the SECOND left finger. Never the stick: the
                // primary is the stick whether it is dragging or resting,
                // so a tap is only ever a finger that is not it. That is
                // what "the stick is never disturbed by the tap" means
                // mechanically.
                TouchPoint* primary = half_touch(true, 0);
                if (clean && primary != &t) tapAuraPending_ = true;
                return;
            }

            // POSSESS — both of a pair, clean, within one TAP_MS window
            // measured from when the PAIR formed (not from each finger),
            // and only if the pinch never declared itself.
            if (pinchDeclared_) { rightTapPending_ = false; pinchDeclared_ = false; return; }

            const bool in_window = (now_ms - rightPairT0_) <= TouchControls::TAP_MS;
            if (half_count(false) == 2) {
                // First of the pair. Arm, and hold the survivor's look
                // for the rest of the window so a possess cannot nudge
                // the camera on its way out.
                TouchPoint* a = half_touch(false, 0);
                TouchPoint* b = half_touch(false, 1);
                rightTapPending_ = in_window && a && b && !a->slopped && !b->slopped;
            }
            else if (rightTapPending_) {
                // Second of the pair.
                if (in_window && !t.slopped) tapPossessPending_ = true;
                rightTapPending_ = false;
            }
            pinchDeclared_ = false;
        }

        // ── U2: THE STICK ────────────────────────────────────────
        // FLOATING ORIGIN: the stick is born where the thumb lands, so
        // there is no fixed pad to find and no chrome to draw. The
        // vector is the drag from that birthplace.
        //
        // The dead zone RESCALES rather than truncating: past its edge
        // the magnitude starts at 0 and climbs to 1 at STICK_RADIUS. A
        // plain truncation would make the first pixel past the dead zone
        // jump straight to STICK_DEAD_ZONE/STICK_RADIUS of full speed —
        // a lurch exactly where the thumb is trying to be gentle.
        //
        // Screen y grows downward and W is move_z -= 1, so a thumb
        // pushed UP is forward with no sign flip: the drag IS the
        // vector. Camera-relativity is the kernel's
        // (coupling_input_to_pawn_velocity rotates by camera azimuth),
        // and it does not renormalize — so this magnitude survives all
        // the way to the step.
        void stick_vector(float& out_x, float& out_z) {
            out_x = 0.0f; out_z = 0.0f;
            TouchPoint* s = half_touch(true, 0);
            if (!s) return;
            const float dx = s->x - s->x0;
            const float dy = s->y - s->y0;
            const float len = std::sqrt(dx * dx + dy * dy);
            if (len <= TouchControls::STICK_DEAD_ZONE) return;   // exactly zero, not small
            const float span = TouchControls::STICK_RADIUS - TouchControls::STICK_DEAD_ZONE;
            float m = (len - TouchControls::STICK_DEAD_ZONE) / span;
            if (m > 1.0f) m = 1.0f;                              // clamped at STICK_RADIUS
            const float inv = m / len;
            out_x = dx * inv;
            out_z = dy * inv;
        }

        // Called once per frame from begin_frame, before the main loop
        // drains the queue — the ledger's sampling law: a delta
        // accumulated in a browser callback is consumed exactly once,
        // on the frame tick.
        void emit_touch_intents() {
            // MOVE speaks every frame it is held, PLUS exactly one zero
            // on release. A vector that only spoke when it changed would
            // leave the pawn walking after the thumb left the glass.
            const bool stick_live = (half_touch(true, 0) != nullptr);
            if (stick_live || stickWasLive_) {
                InputEvent e{};
                e.type = InputEvent::Type::TouchMove;
                stick_vector(e.x, e.y);
                inputEvents_.push_back(e);
            }
            stickWasLive_ = stick_live;

            // LOOK — spent once, then zeroed. Silence when there is
            // nothing to say: an unchanged camera needs no event.
            if (touchLookDx_ != 0.0f || touchLookDy_ != 0.0f) {
                InputEvent e{};
                e.type = InputEvent::Type::TouchLook;
                // LOOK_SENS_TOUCH applies HERE, at consumption — one
                // multiply per frame instead of one per browser event,
                // and one place to look when the feel is wrong.
                e.x = touchLookDx_ * TouchControls::LOOK_SENS_TOUCH;
                e.y = touchLookDy_ * TouchControls::LOOK_SENS_TOUCH;
                inputEvents_.push_back(e);
                touchLookDx_ = 0.0f;
                touchLookDy_ = 0.0f;
            }

            // ZOOM — the same channel the scroll wheel feeds, already
            // signed and scaled.
            if (touchZoomAccum_ != 0.0f) {
                InputEvent e{};
                e.type = InputEvent::Type::TouchZoom;
                e.y = touchZoomAccum_;
                inputEvents_.push_back(e);
                touchZoomAccum_ = 0.0f;
            }

            // THE TAPS — one event per recognized tap, then the flag is
            // spent. Edge-fired by construction: the flag is only ever
            // set by a release.
            if (tapAuraPending_) {
                InputEvent e{};
                e.type = InputEvent::Type::TouchTapLeft;
                inputEvents_.push_back(e);
                tapAuraPending_ = false;
            }
            if (tapPossessPending_) {
                InputEvent e{};
                e.type = InputEvent::Type::TouchTapRight;
                inputEvents_.push_back(e);
                tapPossessPending_ = false;
            }
        }

    public:
#endif   // __EMSCRIPTEN__

        // ── Consumer (main loop reads then clears) ───────────────

        const std::vector<InputEvent>& input_events() const {
            return inputEvents_;
        }

        void clear_input_events() {
            inputEvents_.clear();
        }

        // ── Cursor grab ──────────────────────────────────────────
        //
        // Two facts, two homes; the GLFW mode is always DERIVED, never
        // set from anywhere else.
        //   grabPolicy_ — this program grabs the pointer. The board's
        //                 boot declares it; the lab never does (ImGui
        //                 needs a live cursor), so the door is inert there.
        //   grabActive_ — the grab is applied right now. KP_* toggles it,
        //                 and the choice persists across focus changes.

        void set_cursor_grab(bool on) {
            grabPolicy_ = on;
            apply_cursor_mode();
        }

        void toggle_cursor_grab() {
            if (!grabPolicy_) return;
            grabActive_ = !grabActive_;
            apply_cursor_mode();
        }

        bool cursor_grabbed() const { return grabPolicy_ && grabActive_; }

    private:
        void apply_cursor_mode() {
            if (!window_) return;
            const bool grab = grabPolicy_ && grabActive_;
            glfwSetInputMode(window_, GLFW_CURSOR,
                grab ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            // Raw motion is only provided while the cursor is disabled, so
            // it travels with the mode and is never set separately. It
            // removes the OS pointer-ballistics curve from the look path.
            if (glfwRawMouseMotionSupported()) {
                glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION,
                    grab ? GLFW_TRUE : GLFW_FALSE);
            }
            unprime_cursor();           // the mode change moves the pointer
        }

    public:

        // ═══ §5 ACCESSORS ════════════════════════════════════════

    public:
        wgpu::Device device() const { return device_; }
        wgpu::Queue queue() { return queue_; }
        wgpu::TextureView backbuffer() const { return backbuffer_; }
        wgpu::TextureView depth_view() const { return depthView_; }
        wgpu::TextureFormat color_format() const { return colorFormat_; }
        wgpu::TextureFormat depth_format() const { return depthFormat_; }

        uint32_t width() const { return currentWidth_; }
        uint32_t height() const { return currentHeight_; }

        float aspect_ratio() const {
            if (currentHeight_ == 0) return 1.0f;
            return static_cast<float>(currentWidth_) / static_cast<float>(currentHeight_);
        }

        GLFWwindow* window() const { return window_; }


        // ═══ §6 SHUTDOWN ═════════════════════════════════════════

    public:
        void shutdown() {
            if (window_) {
                glfwDestroyWindow(window_);
                window_ = nullptr;
            }
            glfwTerminate();
        }

        void request_close() {
            if (window_) {
                glfwSetWindowShouldClose(window_, GLFW_TRUE);
            }
        }


        // ═══ §7 STATE ════════════════════════════════════════════

    private:
        // ── Window ───────────────────────────────────────────────
        GLFWwindow* window_ = nullptr;
#ifdef __EMSCRIPTEN__
        // ── SHIP_1 — the claimed touch stream ────────────────────
        TouchPoint touches_[MAX_TRACKED_TOUCHES]{};
        uint64_t   nextTouchSeq_ = 1;   // 0 stays "never born"
        bool       stickWasLive_ = false;   // so the release emits its zero exactly once
        float      touchLookDx_ = 0.0f;     // raw CSS px, accumulated between ticks
        float      touchLookDy_ = 0.0f;
        float      touchZoomAccum_ = 0.0f;  // already signed + scaled by PINCH_SENS
        double     rightPairT0_ = 0.0;      // when the right-half pair formed
        float      rightPairSep_ = 0.0f;    // its separation at the last reading
        bool       pinchDeclared_ = false;  // pinch, or still a possible tap
        bool       rightTapPending_ = false;// one of a clean pair has lifted; waiting on the other
        bool       tapAuraPending_ = false; // edge-fired verbs, spent on the next tick
        bool       tapPossessPending_ = false;
#endif
        uint32_t initialWidth_ = 0;
        uint32_t initialHeight_ = 0;
        uint32_t currentWidth_ = 0;
        uint32_t currentHeight_ = 0;

        // ── Gpu Device ───────────────────────────────────────────
#ifndef __EMSCRIPTEN__
        std::optional<dawn::native::Instance> instance_;
#else
        wgpu::Instance instance_;   // portable handle; owns the async request chain
#endif
        wgpu::Adapter adapter_;
        wgpu::Device device_;
        wgpu::Queue queue_;

        // ── Boot (PORT_1b) ───────────────────────────────────────
        BootState bootState_ = BootState::RequestingAdapter;
        bool deviceLost_ = false;   // PORT_3a — set by the loss callback, read by the frame gate

        // ── Surface & Presentation ───────────────────────────────
        wgpu::Surface surface_;
        wgpu::SurfaceConfiguration surfaceConfig_{};
        wgpu::TextureFormat colorFormat_;
        wgpu::SurfaceTexture surfaceTexture_;
        wgpu::TextureView backbuffer_;

        // ── Depth ────────────────────────────────────────────────
        wgpu::TextureFormat depthFormat_ = wgpu::TextureFormat::Depth24Plus;
        wgpu::Texture depthTexture_;
        wgpu::TextureView depthView_;

        // ── Timing ───────────────────────────────────────────────
        std::chrono::high_resolution_clock::time_point lastTime_;

        // ── Input ────────────────────────────────────────────────
        std::vector<InputEvent> inputEvents_;

        // ── Cursor ───────────────────────────────────────────────
        bool   grabPolicy_ = false;   // no program has claimed the pointer
        bool   grabActive_ = true;    // if one does, it begins grabbed
        double lastCursorX_ = 0.0;
        double lastCursorY_ = 0.0;
        bool   cursorPrimed_ = false;
    };

} // namespace t7
