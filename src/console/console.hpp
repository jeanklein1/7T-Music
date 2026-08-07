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
#endif

#include <algorithm>
#include <vector>
#include <chrono>
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
        // read-only storage to uniform and the room family now counts 8.
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
            deviceDesc.requiredLimits = &limits;

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
                            std::cerr << "[Device] retrying with full adapter passthrough\n";
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
                        if (got.maxTextureDimension2D < 2048u ||
                            got.maxStorageBuffersPerShaderStage < 8u ||
                            got.maxUniformBufferBindingSize < 65536u) {
                            std::cerr << "[Device] modest request returned limits below the"
                                         " censused floor — discarding, retrying passthrough\n";
                            request_device_web(true);
                            return;
                        }
                    }
                    device_ = std::move(device);
                    queue_ = device_.GetQueue();
                    std::cout << "[Device] limits path: " << which << "\n";
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
            instance_.RequestAdapter(nullptr, wgpu::CallbackMode::AllowSpontaneous,
                [this](wgpu::RequestAdapterStatus status, wgpu::Adapter adapter,
                       wgpu::StringView message) {
                    if (status != wgpu::RequestAdapterStatus::Success) {
                        std::cerr << "RequestAdapter failed: "
                            << std::string_view(message.data, message.length) << "\n";
                        bootState_ = BootState::Failed;
                        return;
                    }
                    adapter_ = std::move(adapter);
                    bootState_ = BootState::RequestingDevice;
                    // PORT_5d — ask modestly first; the helper owns the
                    // descriptor, the limits census and the one retry.
                    request_device_web(/*passthrough=*/false);
                });
            return true;
#else
            dawnProcSetProcs(&dawn::native::GetProcs());

            // Construct instance in place (non-copyable, non-movable)
            instance_.emplace();

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
            // outranks integrated; D3D12 breaks ties. Falls back to
            // index 0.
            size_t adapterPick = 0;
            {
                int best = -1;
                for (size_t i = 0; i < adapters.size(); i++) {
                    wgpu::Adapter a = wgpu::Adapter(adapters[i].Get());
                    wgpu::AdapterInfo info{};
                    a.GetInfo(&info);
                    int score =
                        (info.adapterType == wgpu::AdapterType::DiscreteGPU ? 2 : 0)
                      + (info.backendType == wgpu::BackendType::D3D12       ? 1 : 0);
                    if (score > best) { best = score; adapterPick = i; }
                }
            }
            dawn::native::Adapter& nativeAdapter = adapters[adapterPick];
            wgpu::Adapter adapter = wgpu::Adapter(nativeAdapter.Get());
            std::cout << "[Console] Adapter selected: index=" << adapterPick << "\n";

            wgpu::DeviceDescriptor deviceDesc{};
            deviceDesc.label = "7T Device";
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
            surfaceConfig_.width = initialWidth_;
            surfaceConfig_.height = initialHeight_;
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
        // The canvas ELEMENT still fills the page — web/index.html sizes
        // it with CSS (width/height 100%), which is independent of the
        // backing-store size this configures. Fewer pixels, same layout.
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
#endif

    public:
        float begin_frame() {
            glfwPollEvents();

            // Handle resize
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
#ifdef __EMSCRIPTEN__
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