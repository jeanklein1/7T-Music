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
#include "core/boot_params.hpp"   // DOMESDAY_1 B9 — --seed= / --msaa= / --probe=, read once at boot
#include "core/instruments.hpp"  // WIT_2 — t7::g_dropped_submits, the frame-validity witness

#include <webgpu/webgpu_cpp.h>

// ── PORT_1b Region 1: platform includes ──────────────────────────
// The program links dawn::native and exposes the OS window handle for
// the surface. The expose macro must be defined between <GLFW/glfw3.h>
// and <GLFW/glfw3native.h> — upstream requires that order, and so does
// the gate's stub of it (tools/gates/console_gate/PROVENANCE.md).
#include <dawn/native/DawnNative.h>
#include <dawn/dawn_proc.h>
#if __has_include("dawn/common/Version_autogen.h")
#include "dawn/common/Version_autogen.h"
#define T7_DAWN_VERSION 1
#else
#define T7_DAWN_VERSION 0
#endif
#include <GLFW/glfw3.h>

#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#elif defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>

#include <algorithm>
#include <vector>
#include <chrono>
#include <cmath>       // std::sqrt — the stick's magnitude (SHIP_1); std::round/fabs — RIBBON_6's presentation law
#include <cstdio>      // std::printf — RIBBON_6's [PRESENT] histogram (meter builds)
#include <cstdint>     // uint64_t / UINT64_MAX — the touch table's birth counter (SHIP_1)
#include <iostream>
#include <string>
#include <string_view>
#include <optional>

namespace t7 {

    // ═══ THE FLOORS' ONE HOME (DOMESDAY_2 A12) ═══════════════════════
    // The granted-vs-floor line used to hand-carry its six literals;
    // they live in the schema's NEEDS table now, emitted here. This
    // closes LANTERN's print-vs-enforce hazard: the log reads the
    // wallet's own statement of need on the actual device.
#include "console/limits_floor.gen.inc"

    // ═══ THE FEATURE WALLET'S ONE HOME (PROBATE_F) ═══════════════════
    // The device-request site hand-carried its feature list the same way
    // the floor line once hand-carried its literals. The schema's
    // FEATURES table is its home now: nineteen optional features are
    // offered, one is granted, five are vaulted with a price, and
    // witness F-1 holds the emitted request to the schema's granted set.
    // A grant is a schema edit plus Jean's gate — never an edit here.
#include "console/features_wallet.gen.inc"

    // ═══ THE COMPILER PLAN (PIVOT_0, 2026-08-12 · re-ruled at EMBER_0)
    //
    // world.wgsl is single-source across all three values; the plan
    // chooses only which compiler Dawn hands it to. The resident plan
    // is Vulkan (Tint→SPIR-V), and it is what carries the music today.
    // The floor is WebGPU core through modern compilers — Tint→DXC
    // (SM6.0+), Tint→MSL, Tint→SPIR-V, naga.
    //
    // WHY THE PLAN EXISTS, as history: WALLET_0 demoted the occupier
    // windows into the uniform address space; FXC stalled
    // update_player_agent at 20,227 ms and then D3DCompiler_47
    // access-violated on the next room kernel. Jean ruled the floor up
    // rather than the shader down. Those two windows are themselves
    // gone now — occupier_cmg at PRUNE_2 U4, occupier_amg at
    // ONE_WORLD-I U3 — and the cliff is not.
    //
    // D3D12_Fxc IS A DOCUMENTED-UNSUPPORTED LANE, BLOCKED BY SHADER
    // SHAPE. It is no longer archaeology: it is a real lane of this
    // fork carrying a named, standing block. EMBER_0's RECON.8 refresh
    // surveyed every uniform block at master and found SIX carrying
    // the cliff's actual mechanism — an array of structs in the
    // uniform address space subscripted by a non-constant expression.
    // Worst is TileGrid: array<TileGridEntry, 1024>, 16,400 B, read at
    // tile_grid.entries[lz * s + lx] with s the runtime side. Then
    // SceneConstants' array<PawnFigure, 14>, 288 B each, indexed by a
    // storage-buffer-derived skin id in pawn_vs AND shadow_pawn_vs;
    // then AgentRoomConstants, DesignConfig, PyramidArray,
    // DrawPlanParams. The fork does not reshape its shaders to court a
    // legacy compiler (EMBER_0 RULING.4) — the shader shape is the
    // program's law and FXC is the old road. So the lane stays
    // selectable and stays unsupported: choose it and expect a stall
    // or a death inside D3DCompiler_47 — loud and named, never
    // mysterious. STATED HONESTLY: 20,227 ms was MEASURED; these six
    // are shape-matched PREDICTIONS. The measurement would be the boot
    // this ruling declines to spend. The laws FXC used to impose stay
    // an archive in docs/FXC_LAWS_RECORD.md — that record's own text
    // demands re-witnessing on the new floor, and this is a census.
    //
    // D3D12_Dxc IS NOT YET TRUE ON THIS MACHINE. Dawn at the pin puts
    // EnsureDXCLibraries — the only site that opens dxcompiler.dll and
    // dxil.dll — inside #if defined(DAWN_USE_BUILT_DXC), with no #else,
    // and C:/dev/dawn/out is built with that option OFF. The symbol is
    // therefore absent from the dawn_native this program links, and no
    // placement of DLLs can reach code that was never compiled.
    // Selecting DXC today reproduces PIVOT_0a's signature — the log
    // says DXC, GetTogglesUsed lacks use_dxc — for a third reason
    // neither PIVOT_0a nor TOGGLE_0 faced: not mis-chained, not
    // driver-refused, compiled out. The acquisition is scripted at
    // tools/ember_route_a.py and waits on Jean (docs/OPEN.md: EMBER_0).
    //
    // Plan B is one line: if a lane fails on a given driver, set this
    // to Vulkan, rebuild, boot. That IS the fallback, not a failure.
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

    // ═══ THE FEATURE NAME TABLE (DOMESDAY_1 A8, R6) ══════════════════
    //
    // A switch over wgpu::FeatureName ENUMERATOR IDENTIFIERS — never
    // numeric values, so the compiling header supplies every value and
    // the build gate is the witness that each identifier exists. Names
    // are the WebGPU spec's kebab-case feature strings. An id the
    // switch does not know returns nullptr and the caller prints the
    // number — unknown ids stay honest.
    inline const char* feature_name(wgpu::FeatureName f) {
        switch (f) {
        case wgpu::FeatureName::CoreFeaturesAndLimits:         return "core-features-and-limits";
        case wgpu::FeatureName::DepthClipControl:              return "depth-clip-control";
        case wgpu::FeatureName::Depth32FloatStencil8:          return "depth32float-stencil8";
        case wgpu::FeatureName::TimestampQuery:                return "timestamp-query";
        case wgpu::FeatureName::TextureCompressionBC:          return "texture-compression-bc";
        case wgpu::FeatureName::TextureCompressionBCSliced3D:  return "texture-compression-bc-sliced-3d";
        case wgpu::FeatureName::TextureCompressionETC2:        return "texture-compression-etc2";
        case wgpu::FeatureName::TextureCompressionASTC:        return "texture-compression-astc";
        case wgpu::FeatureName::TextureCompressionASTCSliced3D: return "texture-compression-astc-sliced-3d";
        case wgpu::FeatureName::IndirectFirstInstance:         return "indirect-first-instance";
        case wgpu::FeatureName::ShaderF16:                     return "shader-f16";
        case wgpu::FeatureName::RG11B10UfloatRenderable:       return "rg11b10ufloat-renderable";
        case wgpu::FeatureName::BGRA8UnormStorage:             return "bgra8unorm-storage";
        case wgpu::FeatureName::Float32Filterable:             return "float32-filterable";
        case wgpu::FeatureName::Float32Blendable:              return "float32-blendable";
        case wgpu::FeatureName::ClipDistances:                 return "clip-distances";
        case wgpu::FeatureName::DualSourceBlending:            return "dual-source-blending";
        case wgpu::FeatureName::Subgroups:                     return "subgroups";
        case wgpu::FeatureName::TextureFormatsTier1:           return "texture-formats-tier1";
        case wgpu::FeatureName::TextureFormatsTier2:           return "texture-formats-tier2";
        // DOMESDAY_2 A13 — spec-cited additions, glaw1-pruned (F1-a):
        // SubgroupSizeControl was rejected by the native Dawn header
        // and removed per A8's standing protocol — its id keeps
        // printing as a number; re-add the case when the
        // Dawn checkout's wgpu::FeatureName learns the identifier. The
        // Pixel's unknown ids 21/22 are expected to resolve to the two
        // below by enum order.
        case wgpu::FeatureName::PrimitiveIndex:                return "primitive-index";
        case wgpu::FeatureName::TextureComponentSwizzle:       return "texture-component-swizzle";
        default:                                               return nullptr;
        }
    }

    // SUNRISE_0 N2 — THE DIALECT REGISTRY DOES NOT RETURN EITHER.
    // The arm carried a native-only wgsl_language_feature_name() switch
    // over 17 WGSLLanguageFeatureName enumerators, whose ONLY caller was
    // the dialect reporter removed above. COMPAT_1 (ee970995) retired
    // both together when the immediate lane left the program. Restoring
    // it would re-add dead code, and nine of those enumerators do not
    // exist in the emdawnwebgpu generation, so the helper was also a live
    // one-generation-law hazard. WEB_SUNSET's tiered gate would catch it
    // now: console.hpp type-checks against third_party/dawn_native_headers
    // per commit, which is the witness docs/OPEN.md said did not exist.


    // ═══ WIT_2 — IS THIS ERROR A DROPPED FRAME? ══════════════════════
    //
    // Dawn reports a submit of an invalidated command buffer as a
    // VALIDATION error naming the command buffer as invalid. Matching on
    // both halves — the type AND the two words — keeps the count off every
    // other validation error the program could ever raise, which matters
    // because the number's whole value is that zero means one specific
    // thing. Substring matching is the honest tool here: the message text is
    // Dawn's to word, so a stricter parse would be a guess about a string we
    // do not own, and a looser one would count the wrong frames.
    inline void note_if_dropped_submit(wgpu::ErrorType type, std::string_view msg) {
        if (type != wgpu::ErrorType::Validation) return;
        if (msg.find("CommandBuffer") == std::string_view::npos &&
            msg.find("command buffer") == std::string_view::npos) return;
        if (msg.find("invalid") == std::string_view::npos &&
            msg.find("Invalid") == std::string_view::npos) return;
        ++t7::g_dropped_submits;
    }
    // WIT_2 — the dropped-submit counter, called from the device error
    // callback. It counts the submits the device refused, which is the
    // frame-validity witness g_dropped_submits was always for.


    class Console {

        // ═══ §1 IDENTITY ═════════════════════════════════════════

    public:
        Console() = default;
        ~Console() { shutdown(); }

        Console(const Console&) = delete;
        Console& operator=(const Console&) = delete;

        // ── PORT_1b: the boot grammar ────────────────────────────
        // Boot is a state machine. Native traversed it synchronously
        // inside init() (RequestingAdapter..Configuring never observed;
        // init() ended at Ready). Web starts an async request chain in
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
        // frame clock. Native never needed it (init() reached Ready
        // synchronously) but it was callable there harmlessly:
        // every other state is a no-op.
        void pump_boot() {
            if (bootState_ != BootState::Configuring) return;
            if (!initSurface()) { bootState_ = BootState::Failed; return; }
            // ACQ_0: no depth buffer here. It is built at the first acquire,
            // at the size that acquire reports — a placeholder created now
            // could only be a guess, and a guess is what dropped the frames.
            lastTime_ = std::chrono::high_resolution_clock::now();
            bootState_ = BootState::Ready;
        }


        // ═══ §2 INITIALIZATION ═══════════════════════════════════
        //
        // Call order: initGLFW → initWebGPU → initSurface. Each step depends
        // on the previous. If any fails, init returns false. The depth
        // buffer is NOT part of boot any more (ACQ_0) — the first acquire
        // builds it, because the first acquire is the first honest size.

    public:
        bool init(const char* title, uint32_t width, uint32_t height) {
            initialWidth_ = width;
            initialHeight_ = height;
            currentWidth_ = width;
            currentHeight_ = height;

            if (!initGLFW(title))   { bootState_ = BootState::Failed; return false; }
            if (!initWebGPU())      { bootState_ = BootState::Failed; return false; }
            // Native: the device exists synchronously — finish boot here,
            // exactly the pre-PORT_1b sequence, ending Ready.
            if (!initSurface())     { bootState_ = BootState::Failed; return false; }
            // ACQ_0: the depth buffer is built at the first acquire (see
            // acquire_surface_texture). Nothing sized by the frame is
            // allocated before there is a frame to size it by.
            lastTime_ = std::chrono::high_resolution_clock::now();
            bootState_ = BootState::Ready;
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

                if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                    console->request_close();
                    return;
                }

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


        bool initWebGPU() {
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
            // LIFETIME: `kDxcToggle` is static; `toggles`, `wgslControl`
            // (F3-a) and `idesc` need only outlive the emplace() call,
            // and they do — Dawn copies what it needs out of the
            // descriptor during construction.
            // TOGGLE_0 (debt 12) — THE CONTROL RODE THIS ROAD, AND THE
            // ROAD WAS THE ANSWER.
            //
            // Boot 2 proved use_dxc did not take; it did not prove why.
            // Either Dawn validated and refused DXC on this Kepler-era
            // driver, or the chain never propagated at all — a refused
            // toggle and an unchained toggle both read as absent.
            //
            // TOGGLE_0 chained disable_symbol_renaming HERE to separate
            // them: a Tint toggle with no backend of its own, able to ride
            // the working Vulkan boot, travelling the identical road so
            // that the road itself was what got tested. It is
            // ToggleStage::Device, one hop further down than use_dxc's
            // Adapter stage, so its arrival would have proven the whole
            // instance -> adapter -> device chain.
            //
            // IT DID NOT ARRIVE. The boot read (9). That is branch (b):
            // the inheritance this site depends on does not deliver on
            // this Dawn, and Boot 2's verdict softens accordingly — DXC
            // may never have reached Dawn at all. The control has moved to
            // its consuming stage (the device descriptor) and the history
            // is kept here because it is the reason this site is no longer
            // trusted with anything that must arrive.
            //
            // TOGGLE_1revA U1' — THE CONTROL HAS MOVED DOWNSTREAM.
            // TOGGLE_0 chained disable_symbol_renaming here, at the
            // instance, and the boot read (9): it never arrived. Branch
            // (b) — instance -> adapter -> device inheritance does not
            // deliver on this Dawn. The control now chains at the stage
            // that CONSUMES it, the device descriptor, which is the ruling
            // this campaign adopts as standing law.
            //
            // use_dxc STAYS HERE, plan-gated and untouched. It is
            // ToggleStage::Adapter, so the device descriptor would be too
            // late for it — PIVOT_0a proved that, silently. If the D3D12
            // plan is ever revived, its consuming root is
            // RequestAdapterOptions (dawn.json's third chain root), not
            // this one and not the device's. Left as configuration rather
            // than deleted: removing it would erase PIVOT_0d-ii's finding
            // along with its subject.
            // SUNSET_0 — the dev-tier instance toggle is gone with the
            // twin it served. F5-d opened it for exactly one purpose,
            // reaching the immediate lane on the old native generation,
            // and it cost real validation on other not-yet-secure entry
            // points for as long as it was open. The one-generation law's
            // other half never came; the twin was archived instead
            // (docs/LAWS.md SUNSET_0, tag `native-sunset`). `use_dxc`
            // stays, plan-gated, for the reason its own banner above
            // gives.
            static const char* const kDxcToggle[] = { "use_dxc" };
            wgpu::DawnTogglesDescriptor toggles{};
            wgpu::InstanceDescriptor idesc{};

            // ── DOMESDAY_2 F3-a — THE WIRE CONTROL, CORRECTLY LABELLED ──
            //
            // var<immediate> (world.wgsl) is gated by the WGSL language
            // feature immediate_address_space, which is instance-scoped.
            // F3-a and F5-b reached for DawnWireWGSLControl
            // (wgpu::DawnWireWGSLControl; enableExperimental /
            // enableUnsafe / enableTesting) because its chain root IS the
            // instance descriptor and its three members name exactly the
            // three tiers. The header made it look like the control.
            // (The line number that stood here cited Dawn's NATIVE
            // generation, which this tree could not open until
            // third_party/dawn_native_headers — symbols, not line numbers.)
            //
            // IT WAS NEVER THE CONTROL ON THIS PATH. Read out of Dawn's own
            // sources at the archived native revision f0bf8ab:
            // DawnWireWGSLControl is consumed in exactly one place —
            // src/dawn/wire/client/Instance.cpp — and nothing under
            // src/dawn/native/ reads it. A tree linking dawn::native
            // directly has no wire, so the struct is unpacked by
            // ValidateAndUnpack, found to be a legal chain root, and then
            // read by no one. Accepted and ignored: PIVOT_0a's exact
            // species. The native gate was a TOGGLE instead
            // (InstanceBase::GatherWGSLFeatures reads
            // Toggle::AllowUnsafeAPIs for a kUnsafeExperimental feature),
            // which is what F5-d enabled and SUNSET_0 removed with the
            // twin.
            //
            // IT STAYS, for the reason use_dxc's banner above gives of
            // PIVOT_0d-ii: deleting it would erase the finding along with
            // its subject. What it must never be read as is the thing
            // this boot's dialect line credits.
            wgpu::DawnWireWGSLControl wgslControl{};
            wgslControl.enableExperimental = true;
            wgslControl.enableUnsafe = true;   // wire-path only — INERT on dawn::native

            if constexpr (kCompilerPlan == CompilerPlan::D3D12_Dxc) {
                toggles.enabledToggleCount = 1;
                toggles.enabledToggles = kDxcToggle;
            } else if constexpr (kCompilerPlan == CompilerPlan::D3D12_Fxc) {
                // EMBER_0 RULING.3 — THE FXC ARM DECLARES ITS COMPILER.
                // Until now this arm chained nothing and rested on Dawn's
                // DEFAULT for use_dxc. A default is Dawn's to flip and a
                // disabled toggle is ours: resting on one makes an
                // unstated intent indistinguishable from an unnoticed
                // change of default, and there is no witness that can
                // tell them apart after the fact. Same toggle name, same
                // descriptor, opposite field.
                toggles.disabledToggleCount = 1;
                toggles.disabledToggles = kDxcToggle;
            }
            // WHERE THIS CHAIN BELONGS, AND WHY IT IS STILL HERE.
            // EMBER_0 RECON.3 re-read the registry at pin 56f332d7:
            // use_dxc is ToggleStage::Adapter (Toggles.cpp, beside
            // disable_robustness at ToggleStage::Device). L21's citation
            // holds. But this root is the INSTANCE descriptor, one
            // inheritance hop above that stage, and L21 itself records
            // that the hop has never been witnessed carrying a toggle —
            // debt 12 closed MOOT, branch (b) unproven. The consuming
            // root with no hop at all is RequestAdapterOptions, which
            // this program does not construct: EnumerateAdapters() is
            // called bare, deliberately unfiltered so the boot log lists
            // every adapter. EMBER_0 stamps the re-siting of BOTH arms
            // onto RequestAdapterOptions as UNIT.1's work, WITH a boot to
            // prove it — because a change on the boot path that no
            // witness ever runs is exactly what PIVOT_0a was. Until then
            // the declaration stands where its sibling stands, and the
            // FXC lane's block (the banner above) means no boot is owed.
            // The toggles ride BEHIND the WGSL control: independent links
            // in one chain, not two WGSL enablement mechanisms. Named so
            // a future edit that guards or deletes the F3-a block cannot
            // silently take the toggles with it — PIVOT_0a already paid
            // for one toggle that did not arrive.
            wgslControl.nextInChain = &toggles;
            idesc.nextInChain = &wgslControl;
            // THE CHAIN, POST-SUNSET_0: head is always wgslControl and
            // `toggles` is always behind it, but the DXC plan is the only
            // one that puts a string in the array — every other plan
            // chains a toggle-less descriptor, which is the state
            // TOGGLE_0's U2 restored and F5-d had made unreachable.

            // Construct instance in place (non-copyable, non-movable)
            instance_.emplace(reinterpret_cast<const WGPUInstanceDescriptor*>(&idesc));

            // SUNRISE_0 N2 — THE ONE LINE OF THE ARM THAT DOES NOT RETURN.
            // The arm called the dialect's testimony here (F3-a). COMPAT_1
            // (ee970995) retired the immediate lane from schema, module and
            // C++ in one commit, taking that reporter and all four of its
            // dependencies: wgslImmediate_, the feature-name helper,
            // SupportedWGSLLanguageFeatures and HasWGSLLanguageFeature.
            // Restoring the call would not compile, and restoring the
            // reporter would re-open a lane the program closed on BOTH
            // twins. Convergent, not divergent: the native arm does not
            // want it back either.

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
                // THE PROBE'S LADDER (the device gate). An ordinary boot
                // wants the fastest adapter. The probe wants the one that
                // VALIDATES and does nothing else: Dawn's frontend raises
                // the errors the probe hunts before any backend is
                // reached, so a Null or CPU adapter returns the same
                // verdict for none of the wall clock — and a machine with
                // no GPU could return it at all.
                //
                // DEFAULTED OFF, deliberately, and boot_params.hpp's
                // banner says why: `--probe=N` alone takes the ordinary
                // pick, which is the one configuration the probe is
                // already known to boot in. `--probe-backend=null` asks
                // the question; one boot answers it.
                //
                // The ladder is a SCORE, not a filter — an absent Null or
                // CPU adapter costs nothing and the run falls through to
                // the best thing enumerated, which is the honest meaning
                // of "Null -> CPU -> any".
                const t7::BootParams& bp = t7::boot_params();
                const bool probeNull = bp.has_probe && bp.probe_backend == t7::ProbeBackend::Null;
                const bool probeCPU  = bp.has_probe && bp.probe_backend == t7::ProbeBackend::CPU;
                int best = -1;
                for (size_t i = 0; i < adapters.size(); i++) {
                    wgpu::Adapter a = wgpu::Adapter(adapters[i].Get());
                    wgpu::AdapterInfo info{};
                    a.GetInfo(&info);
                    const bool isNull = (info.backendType == wgpu::BackendType::Null);
                    const bool isCPU  = (info.adapterType == wgpu::AdapterType::CPU);
                    int score;
                    if (probeNull) {
                        score = (isNull ? 4 : 0) + (isCPU ? 2 : 0);
                    } else if (probeCPU) {
                        score = (isCPU ? 4 : 0) + (isNull ? 2 : 0);
                    } else {
                        score =
                            (info.adapterType == wgpu::AdapterType::DiscreteGPU ? 2 : 0)
                          + (info.backendType == kPreferredBackend              ? 1 : 0);
                    }
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

            // TOGGLE_0 U2 — THE CONTROL IS RETIRED. VERDICT: BRANCH (b).
            //
            // What stood here: a two-name DawnTogglesDescriptor on
            // deviceDesc.nextInChain — disable_symbol_renaming as the
            // control, t7_not_a_toggle as the garnish. It is gone because
            // it has done its work, and a control left armed becomes dead
            // code, and dead code is a liar.
            //
            // THE READING (Jean, native Vulkan, Dawn f0bf8ab, 2026-08-13,
            // binary built at 9489b8d — i.e. AFTER the chain was re-sited
            // to this device descriptor):
            //
            //     [Console] Toggles used (10): … disable_symbol_renaming …
            //
            // Count 9 → 10, the control present. THE POSITIVE HALF IS
            // PROVEN: chained at the stage that consumes it, a Dawn toggle
            // arrives, and it arrives visibly at GetTogglesUsed.
            //
            // WHAT IS NOT PROVEN, STATED PLAINLY. TOGGLE_0's table read a
            // present control as branch (a) — "the instance chain works,
            // Boot 2 was Dawn refusing DXC". That inference is void here,
            // because the binary that produced this reading does not chain
            // the control on the instance descriptor. TOGGLE_0 U1 armed it
            // there at 30c9a7c; no boot ever ran that binary (the `(9)`
            // reading predates the control entirely — TOGGLE_1 U0 proved
            // the `if constexpr` left idesc.nextInChain null on the Vulkan
            // plan). So the instance chain has never been tested with a
            // toggle in it, and BRANCH (b) IS UNPROVEN, NOT CONFIRMED.
            //
            // Debt 12 therefore closes as MOOT rather than as (a) or (b):
            // the question it asked — "does the instance chain propagate?"
            // — no longer gates anything, because L21 routes every future
            // toggle to its consuming stage and never relies on
            // inheritance. Reopening it costs one boot with the control
            // re-armed on idesc; nothing in the queue wants that boot.
            //
            // The general fact, now L21: a Dawn toggle must be chained at
            // the descriptor of the stage that CONSUMES it. use_dxc is
            // ToggleStage::Adapter and stays on the instance/adapter path
            // above; disable_symbol_renaming is ToggleStage::Device and
            // took from here on the first boot that asked it to.
            //
            // The garnish testified to nothing and was always expected to:
            // its silence was ruled inadmissible when it was armed, and it
            // stayed silent. No inference is drawn from it.
            //
            // The witness that outlives both is GetTogglesUsed below. It
            // is not the control; it is the readout, and it stays.

            // THE PROBE'S EAR IS THIS CALLBACK (the device gate). It was
            // already the only place the program hears the device object;
            // the probe adds a count and a copy of the FIRST message, so a
            // run can end in a verdict instead of a log a human must read.
            // The print stays exactly as it was — an ordinary boot is
            // unchanged, and the probe's verdict is composed from the same
            // words the operator sees.
            deviceDesc.SetUncapturedErrorCallback(
                [](const wgpu::Device&, wgpu::ErrorType type, wgpu::StringView message) {
                    const std::string_view text(message.data, message.length);
                    std::cerr << "WebGPU Error (" << static_cast<int>(type) << "): "
                        << text << std::endl;
                    note_if_dropped_submit(type, text);   // WIT_2
                    t7::note_device_error(static_cast<int>(type),
                                          message.data, message.length);
                });
            // PORT_3a — the loss door. Loss is rare but real (TDR, GPU
            // reset, driver update), and the honest-death policy does not
            // depend on which of them caused it.
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
            // DOMESDAY_2 F3-b — the wrong enum is gone. The immediate
            // lane needs no device feature (there is none); it needs the
            // instance's WGSL dialect (F3-a) and the limit, which rides
            // full passthrough here — the adapter's own maxImmediateSize,
            // never a floor we have to ask for. The log reads in one
            // order: what this request carries, then what the instance
            // either has or lacks.
            // SUNRISE_0 N2 — THE IMMEDIATE-LANE READOUT DOES NOT RETURN.
            // The arm printed maxImmediateSize's floor and the instance's
            // immediate_address_space verdict here. COMPAT_1 (ee970995)
            // retired that lane from schema, module and C++ together, taking
            // FLOOR_MAX_IMMEDIATE_SIZE and wgslImmediate_ with it. Third and
            // last of this file's retired-lane references; all three were
            // found by the native type-check, none by any gate.
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
        }

        bool initSurface() {
            wgpu::SurfaceDescriptor surfaceDesc{};
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

            wgpu::SurfaceCapabilities caps;
            surface_.GetCapabilities(adapter_, &caps);

            colorFormat_ = caps.formats[0];

            surfaceConfig_.device = device_;
            surfaceConfig_.format = colorFormat_;
            // CAP_1 — THE FIRST CONFIGURE TAKES TARGET, NOT THE BOOT ARGS.
            // currentWidth_/currentHeight_ were seeded from init()'s
            // arguments, which are glfwCreateWindow's arguments and nothing
            // else. Configuring the surface with them put a number nobody
            // measured into the one place the frame is defined. The same
            // expression the resize path uses answers it here, once, from
            // the framebuffer that actually exists by now, so boot enters
            // the frame loop with the surface already agreeing with it.
            surfaceConfig_.width = currentWidth_;
            surfaceConfig_.height = currentHeight_;
            surfaceConfig_.presentMode = wgpu::PresentMode::Fifo;
            surfaceConfig_.alphaMode = wgpu::CompositeAlphaMode::Opaque;
            surface_.Configure(&surfaceConfig_);

            return true;
        }

        // THE FRAME-SIZED ATTACHMENTS (ACQ_0). Every texture whose size is
        // defined as "the frame's size" is created here and nowhere else, so
        // there is exactly one place that can disagree with the acquired
        // texture — and reconcile_frame_attachments is its only caller.
        //
        // The census that fixed this membership: the depth buffer and, at
        // msaa=4, the MSAA color target. Nothing else is frame-sized.
        //
        // DOMESDAY_2 B10: the depth buffer carries the boot-read sample
        // count, and the msaa color target rides this same recreate path.
        // With msaa=1 every descriptor below is byte-identical to the
        // pre-B10 shape and no msaa color texture exists.
        void createDepthBuffer(uint32_t w, uint32_t h) {
            // Destroy before recreate: the old textures are a frame size
            // that no longer exists, and holding them is how a stale view
            // finds its way into an encoder.
            if (depthTexture_)     depthTexture_.Destroy();
            if (msaaColorTexture_) msaaColorTexture_.Destroy();
            msaaColorTexture_ = nullptr;
            msaaColorView_    = nullptr;
            wgpu::TextureDescriptor depthDesc{};
            depthDesc.label = "Depth Texture";
            depthDesc.size = { w, h, 1 };
            depthDesc.format = depthFormat_;
            depthDesc.sampleCount = effective_msaa();
            depthDesc.usage = wgpu::TextureUsage::RenderAttachment;
            depthTexture_ = device_.CreateTexture(&depthDesc);
            depthView_ = depthTexture_.CreateView();
            if (effective_msaa() == 4u) {
                wgpu::TextureDescriptor msaaDesc{};
                msaaDesc.label = "MSAA Color Target";
                msaaDesc.size = { w, h, 1 };
                msaaDesc.format = surfaceConfig_.format;
                msaaDesc.sampleCount = 4;
                msaaDesc.usage = wgpu::TextureUsage::RenderAttachment;
                msaaColorTexture_ = device_.CreateTexture(&msaaDesc);
                msaaColorView_ = msaaColorTexture_.CreateView();
            }
        }


        // ═══ §3 FRAME LIFECYCLE ══════════════════════════════════
        //
        // Call order each frame:
        //   begin_frame() → [update cartridges] → acquire_surface_texture()
        //   → [encode & submit] → present() → [back to running()]

    private:

    public:
        float begin_frame() {
            ++probeTurns_;   // the device gate: every turn, presented or not
            glfwPollEvents();

            // Handle resize
            int fbWidth = 0, fbHeight = 0;
            glfwGetFramebufferSize(window_, &fbWidth, &fbHeight);
            // DOMESDAY_1 B7 (R4) — THE SETTLE WINDOW. A new size must hold
            // still for RECONFIGURE_SETTLE_FRAMES consecutive frames before
            // the surface is reconfigured; the accepted cost is ≤100 ms of
            // scale softness while a resize animates.
            //
            // ACQ_0 — ITS JURISDICTION IS CONFIGURE INTENT, AND NOTHING ELSE.
            // This branch decides what the app ASKS the surface for: format,
            // present mode, and the size it would like. It may NEVER again
            // gate the recreation of a frame-sized attachment. It used to
            // call createDepthBuffer, and that was the defect — a debounced
            // attachment is by construction a frame or more behind a surface
            // that resizes on its own clock, and the gap between them is a
            // dropped submit. Attachments follow the acquired texture now
            // (acquire_surface_texture), which cannot lag because it IS the
            // frame. Debouncing intent is still worth doing; debouncing truth
            // never was.
            //
            // CAP_2 — THE SETTLE WINDOW IS THE WHOLE PATH AGAIN. The
            // frame-boundary canvas reassertion that used to reconcile
            // target and surface before control reached here was the web
            // twin's, and went with it at web-sunset; this debounce is once
            // more the one thing that decides a reconfigure. Its ≤100 ms of
            // scale softness while a resize animates is the stated price.
            if (fbWidth > 0 && fbHeight > 0 &&
                (static_cast<uint32_t>(fbWidth) != currentWidth_ ||
                    static_cast<uint32_t>(fbHeight) != currentHeight_)) {
                if (static_cast<uint32_t>(fbWidth) == pendingWidth_ &&
                    static_cast<uint32_t>(fbHeight) == pendingHeight_) {
                    if (++stableFrames_ >= RECONFIGURE_SETTLE_FRAMES) {
                        currentWidth_ = static_cast<uint32_t>(fbWidth);
                        currentHeight_ = static_cast<uint32_t>(fbHeight);
                        surfaceConfig_.width = currentWidth_;
                        surfaceConfig_.height = currentHeight_;
                        surface_.Configure(&surfaceConfig_);
                        // No attachment recreation here — see the banner above.
                        stableFrames_ = 0;
                    }
                } else {
                    pendingWidth_ = static_cast<uint32_t>(fbWidth);
                    pendingHeight_ = static_cast<uint32_t>(fbHeight);
                    stableFrames_ = 1;
                }
            } else {
                stableFrames_ = 0;
            }

            // Delta time
            auto currentTime = std::chrono::high_resolution_clock::now();
            float measured = std::chrono::duration<float>(currentTime - lastTime_).count();
            lastTime_ = currentTime;

            // PORT_1b: the dt clamp, lifted verbatim from the dormant
            // core/clock.hpp (retired this commit) — "Clamp dt to avoid
            // spiral of death", cap 0.1f (100 ms). Inert at ordinary frame
            // rates; the browser tab-suspend it was essential for went with
            // the twin at web-sunset, and a multi-second gap is still a
            // multi-second gap when a debugger or a stalled driver hands
            // one back.
            const float raw = std::clamp(measured, 0.0f, 0.1f);

            // THE PRESENTATION LAW (RIBBON_6). A frame is DISPLAYED for a
            // whole number of refreshes, so it must be INTEGRATED for a whole
            // number of refreshes. The steady clock (RIBBON_3) smoothed the
            // callback's arrival, which is the right cure while every frame
            // makes its refresh and the wrong one the moment a frame misses:
            // a 25 ms frame is shown for two refreshes, and integrating 25 ms
            // of world into 33.3 ms of display is a lurch the eye reads as a
            // block.
            //
            // So: estimate the refresh period from the frames that make it;
            // express the measurement as a multiple of it; serve that multiple
            // exactly. Where nothing is ever dropped this is the steady clock,
            // unchanged — every frame is 1x and the served value is the running
            // mean. Where a frame is dropped it serves what the eye saw. A law
            // that is a no-op when it is not needed.
            //
            // THE PERIOD IS PINNED, NOT TRACKED (WRAP_0 U1). Everything above
            // is still true; what was wrong was WHERE the period came from. It
            // was an EWMA over in-band frames plus a relock that adopted `raw`
            // outright — an estimator that follows the FRAME RATE, and there
            // is nothing in it that can tell "the display is slow" from "the
            // world is slow". So it learned the judder and then hid it: on the
            // laptop it printed 16.66 at boot and 25.4-25.7 for the rest of
            // the session, with every frame labelled 1x. On a 60 Hz panel 25.0
            // is exactly the mean of a 16.67/33.33 alternation — the estimator
            // had adopted the average of the stutter as the refresh, and the
            // histogram, which divides by it, could then only ever read 1x.
            //
            // A PRESENT CAN BE LATE BUT NEVER EARLY. You can miss a vblank;
            // you cannot beat one. So the display period is the SMALLEST
            // stable delta, not the mean: the frames that made their refresh
            // are the ones telling the truth about the panel, and the mean is
            // the judder averaged in. P is the 5th percentile of the trailing
            // PRESENT_FIT_WINDOW deltas — a percentile and not the minimum,
            // because one early timer reading must not redefine the display.
            //
            // AND IT MAY ONLY EVER DECREASE, once seeded. A faster display
            // discovered later is real (a 120 Hz panel whose first seconds
            // were slow); a slower one is a world that has fallen behind, and
            // that is precisely what must NOT be mistaken for a refresh.
            //
            // THE LIMITATION, STATED. A device that never fits a single vblank
            // from its very first frame pins to its slowest honest period, and
            // `refresh` prints 33.3 — which is a recognizable number and not a
            // hidden one. Read it as "this machine never once made 60".
            //
            // THE FLOOR IS NOT DECORATION, and it is the one line that is
            // mine. The period is a DIVISOR now, which the mean never was, so
            // a relock onto a zero measurement is not a degradation but a
            // permanent NaN. The reader that made raw == 0 ordinary rather
            // than hypothetical was the browser's coarsened performance.now()
            // — Firefox at privacy.resistFingerprinting quantises it to
            // 100 ms, so most consecutive calls returned the SAME value —
            // and it went with the twin at web-sunset. The floor stays: a
            // divisor that can reach zero is worth one comparison whoever
            // is holding the clock. A refresh period cannot be shorter than
            // PRESENT_MIN_PERIOD.
            // The fit's sample set: deltas only, artefacts dropped. A reading
            // under PRESENT_FIT_FLOOR is a coarsened or duplicated timer value,
            // never a display that refreshes faster than 250 Hz.
            if (raw >= PRESENT_FIT_FLOOR) {
                presentFit_[presentFitCursor_] = raw;
                presentFitCursor_ = (presentFitCursor_ + 1u) % PRESENT_FIT_WINDOW;
                if (presentFitCount_ < PRESENT_FIT_WINDOW) presentFitCount_++;
            }
            // Refit on a cadence, not per frame: a percentile over the window
            // is a selection, and the panel's period does not move between
            // frames. Cheap either way — one 300-float copy, ~1 kB, 1 Hz.
            if (presentFitCount_ >= PRESENT_FIT_MIN
                    && ++presentFitTick_ >= PRESENT_FIT_REFIT) {
                presentFitTick_ = 0;
                float scratch[PRESENT_FIT_WINDOW];
                std::memcpy(scratch, presentFit_, presentFitCount_ * sizeof(float));
                const size_t idx = (size_t)(presentFitCount_ * 5u / 100u);
                std::nth_element(scratch, scratch + idx, scratch + presentFitCount_);
                const float p5 = std::max(scratch[idx], PRESENT_MIN_PERIOD);
                if (!presentFitSeeded_) { refreshPeriod_ = p5; presentFitSeeded_ = true; }
                else if (p5 < refreshPeriod_) { refreshPeriod_ = p5; }
            }

            const float k_raw = std::round(raw / refreshPeriod_);
            const float k = std::clamp(k_raw, 1.0f, PRESENT_MAX_MULTIPLE);
            const float model = k * refreshPeriod_;
            float dt;
            if (std::fabs(raw - model) < PRESENT_BAND * refreshPeriod_) {
                // EVERY in-band frame teaches the period, and it teaches
                // raw / k — the refresh this frame implies, not the frame.
                //
                // THE STUCK MULTIPLE, and it is why this is not `if (k == 1)`.
                // Gating the gain on unit frames leaves a k >= 2 frame with NO
                // feedback path at all: it does not move the period, and the
                // in-band arm clears dtStrangers_, so it never relocks either.
                // A display whose frame time lands anywhere in 29.2-37.5 ms —
                // 26.7 to 34.3 Hz — then reads as a permanent 2x against a
                // 16.67 ms period and is served 33.3 ms for every frame. At
                // 34 Hz that is the world running 13.4% FAST, forever, with a
                // clean meter. raw / k fixes it without costing anything: a
                // GENUINE 30-on-60 has raw / k == the panel period exactly, so
                // the term is zero and the model is untouched; a false multiple
                // converges until served == raw. A no-op when it is not needed,
                // which is the property the whole law is built on.
                // The period is the percentile's; an in-band frame teaches it
                // nothing any more. What the band still decides is what is
                // SERVED — a frame that fits its multiple is integrated for
                // the multiple, which is the presentation law itself.
                dtStrangers_ = 0;
                dt = model;
            } else {
                // Out of band: a hitch, a tab-resume, a stall. Serve its true
                // time. It no longer relocks the period — a slow frame is not
                // evidence about the display, which is the whole of U1.
                dtStrangers_++;
                dt = raw;
            }
            // THE SERVED VALUE CARRIES THE CLAMP TOO. `model` is k x period and
            // can exceed the 100 ms ceiling `raw` was clamped to: at a 57 ms
            // period a 100 ms measurement reads k=2 in-band and would serve
            // 114.3 ms. The CPU's beat clock would then advance t_seconds by
            // 114 ms while dtPending_ clips the GPU's copy to 100 — two
            // integrators handed different amounts of time for one frame, and
            // the cartridge's own comment about "the same 100 ms ceiling"
            // made false. One line, and the ceiling means what it says.
            dt = std::min(dt, 0.1f);

            // THE PRESENT HISTOGRAM (meter builds, 1 Hz). k IS the reading
            // that settles the class: a 2x or 3x column is a DROPPED FRAME —
            // a stutter with a mechanism outside the simulation — and a
            // near-pure 1x column with judder on screen means the cause is
            // upstream in the integrators, not in presentation.
            if constexpr (t7::INSTRUMENTS.frame_meter) {
                // The index is bounded on its own terms, not on the floor's:
                // k is clamped to [1, 4] above, but an index derived from a
                // float must not depend on that clamp being right (RIBBON_5's
                // lesson — the array bound is stated, never derived).
                const uint32_t bucket = (k >= 1.0f && k <= 4.0f) ? (uint32_t)k : 1u;
                presentBuckets_[bucket <= 4u ? bucket - 1u : 3u]++;
                if (dtStrangers_ != 0u || (dt == raw && k != 1.0f)) presentStrangers_++;
                const float d = std::fabs(dt - raw) * 1000.0f;
                presentDeltaSum_ += d;
                if (d > presentDeltaMax_) presentDeltaMax_ = d;
                if (++presentFrames_ >= PRESENT_REPORT_FRAMES) {
                    std::printf("[PRESENT] refresh %.2f ms | 1x %u  2x %u  3x %u  4x+ %u"
                                "  strangers %u | served-raw |d| mean %.1f max %.1f ms\n",
                                (double)(refreshPeriod_ * 1000.0f),
                                presentBuckets_[0], presentBuckets_[1],
                                presentBuckets_[2], presentBuckets_[3],
                                presentStrangers_,
                                (double)(presentDeltaSum_ / (float)presentFrames_),
                                (double)presentDeltaMax_);
                    presentBuckets_[0] = presentBuckets_[1] = 0;
                    presentBuckets_[2] = presentBuckets_[3] = 0;
                    presentStrangers_ = 0; presentFrames_ = 0;
                    presentDeltaSum_ = 0.0f; presentDeltaMax_ = 0.0f;
                }
            }


            // RIBBON_6: the canvas, published for the meter's window line.
            // Written HERE rather than at the four sites that assign
            // currentWidth_/currentHeight_, because by this point in the frame
            // every one of them has spoken and the settle path above is the
            // last word (CAP_2). One store a frame, no branch.
            t7::g_canvas_w = currentWidth_;
            t7::g_canvas_h = currentHeight_;

            return dt;
        }

        // ═══ ACQ_0 — THE ACQUIRED TEXTURE IS THE ONLY WITNESS OF FRAME SIZE ═══
        //
        // An older generation handed back surface textures at the last
        // CONFIGURED size, so a stale depth buffer and a stale swapchain
        // stayed consistent with each other and the debounce was safe. The
        // generation this tree pins tracks the surface's actual size at
        // acquire (found on the web twin, true of both). Two
        // writers, one of them now moving on its own clock: at boot and around
        // every resize there was a window where the depth attachment and the
        // backbuffer disagreed, Dawn invalidated the whole "frame" encoder, and
        // the ENTIRE submit dropped. Not a dropped draw — a dropped frame. The
        // one-shot GPU work that happened to be encoded in those frames (spawn
        // patch generation, the ground atlas, live-card seeding) was lost
        // silently, and patch caching then preserved the loss. That is the flat
        // black spawn region, and it is the whole of it.
        //
        // So validity may never again depend on two writers agreeing. The size
        // is read off the texture we were just handed, every frame, before any
        // encoding — not from the configure intent, not from the framebuffer
        // query, not from a cached value that was true when it was written.
        //
        // This is also the ONLY creation path for the depth buffer now: the
        // boot placeholder is gone, and the first acquire builds it. There is
        // no frame on which an encoder can be created whose depth size was not
        // read from that frame's own acquired texture, because the one consumer
        // of depth_view() runs after this returns true.
        bool acquire_surface_texture() {
            surface_.GetCurrentTexture(&surfaceTexture_);
            if (surfaceTexture_.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal &&
                surfaceTexture_.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
                // Item 5: no acquire, no encoding. The caller returns before it
                // creates the frame encoder, so a failed acquire costs a frame
                // and nothing else.
                return false;
            }
            const uint32_t aw = surfaceTexture_.texture.GetWidth();
            const uint32_t ah = surfaceTexture_.texture.GetHeight();
            if (aw != acquiredWidth_ || ah != acquiredHeight_ || !depthTexture_) {
                acquiredWidth_  = aw;
                acquiredHeight_ = ah;
                createDepthBuffer(aw, ah);   // synchronous, before any encoding
            }
            backbuffer_ = surfaceTexture_.texture.CreateView();
            return true;
        }

        // ── PORT_1b Region 4 — presentation is a call again. On the web
        // twin this body was a deliberate no-op: emdawnwebgpu's
        // wgpuSurfacePresent existed but ABORTED, because presentation
        // there happened implicitly at rAF return. That twin is attic'd
        // (tag web-sunset) and Present() is simply the swap.
        void present() {
            surface_.Present();
            ++probePresented_;   // the device gate: this frame reached the swap
        }

        // ONE DOOR OUT OF THE LOOP (L10). The probe does not get a frame
        // loop of its own — it gets a reason for the one loop to stop, so
        // every frame it runs is a frame the exhibition runs, encoded by
        // the same code in the same order. Two counters, because a probe
        // whose acquire never succeeds must still terminate: the budget is
        // spent when N frames have PRESENTED, and the patience runs out at
        // probe_turn_budget() turns regardless. Which one ended the run is
        // the verdict's business (the_board.cpp) — exhausted patience is a
        // RED, because a program that cannot present did not pass.
        bool running() const {
            if (!window_ || glfwWindowShouldClose(window_)) return false;
            if (t7::boot_params().has_probe) {
                if (probePresented_ >= t7::boot_params().probe_frames) return false;
                if (probeTurns_     >= t7::probe_turn_budget())        return false;
            }
            return true;
        }

        // The probe's two readings, for the verdict.
        uint32_t probe_presented() const { return probePresented_; }
        uint32_t probe_turns()     const { return probeTurns_; }


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
        // B10: null when msaa=1 — the main pass reads the null as
        // "render straight into the backbuffer, exactly as before".
        wgpu::TextureView msaa_color_view() const { return msaaColorView_; }
        wgpu::TextureFormat color_format() const { return colorFormat_; }
        wgpu::TextureFormat depth_format() const { return depthFormat_; }

        uint32_t width() const { return currentWidth_; }
        uint32_t height() const { return currentHeight_; }

        float aspect_ratio() const {
            // ACQ_0: the acquired size is the frame's real shape, so the
            // projection follows it rather than the configure intent. Before
            // the first acquire there is nothing to report and the intent is
            // the best available answer.
            const uint32_t w = acquiredWidth_  ? acquiredWidth_  : currentWidth_;
            const uint32_t h = acquiredHeight_ ? acquiredHeight_ : currentHeight_;
            if (h == 0) return 1.0f;
            return static_cast<float>(w) / static_cast<float>(h);
        }

        GLFWwindow* window() const { return window_; }


        // ═══ §6 SHUTDOWN ═════════════════════════════════════════

    public:
        void shutdown() {
            // WIT_2b — the session's verdict, through the one formatting
            // site (core/instruments.hpp), so teardown and the window close
            // cannot drift into two spellings of one witness.
            t7::print_dropped_submits("session total");
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
        // CONFIGURE INTENT — what the app asks the surface for (ACQ_0).
        uint32_t currentWidth_ = 0;
        uint32_t currentHeight_ = 0;
        // FRAME TRUTH — the last acquired texture's own size. Written only by
        // acquire_surface_texture, read by the attachments and the aspect.
        uint32_t acquiredWidth_ = 0;
        uint32_t acquiredHeight_ = 0;
        // DOMESDAY_1 B7 (R4) — the reconfigure settle window: a changed
        // framebuffer size must hold still this many consecutive frames
        // before the surface reconfigures (begin_frame). Boot configures
        // immediately (initSurface); this gates the per-frame path only.
        static constexpr uint32_t RECONFIGURE_SETTLE_FRAMES = 6;
        uint32_t pendingWidth_ = 0;
        uint32_t pendingHeight_ = 0;
        uint32_t stableFrames_ = 0;

        // RIBBON_3 — THE STEADY CLOCK's three numbers and its two words.
        // BAND: how far from the mean a measurement may sit and still be
        // called the same cadence (fraction of the mean). GAIN: how fast
        // the mean follows an in-band measurement. RELOCK: how many
        // consecutive out-of-band frames adopt the new cadence outright.
        // RIBBON_6 — THE PRESENTATION LAW's numbers. BAND: how far from a
        // whole multiple of the refresh a measurement may sit and still be
        // called that multiple (fraction of one period). GAIN: how fast the
        // period follows a UNIT frame. MAX_MULTIPLE: the deepest drop the law
        // will model rather than pass through. MIN_PERIOD: the divisor's
        // floor — 1 ms, far above any real display, and the only thing between
        // a coarsened clock and a permanent NaN. RELOCK keeps RIBBON_3's name
        // and value: it is the same idea, one layer down.
        static constexpr float    PRESENT_BAND         = 0.25f;
        // WRAP_0 U1 — THE FIT. The window is 300 presents (~5 s at 60 Hz):
        // long enough that a burst of dropped frames cannot move the 5th
        // percentile, short enough to seed within the first walk. PRESENT_GAIN
        // and STEADY_CLOCK_RELOCK are retired with the estimator they drove.
        static constexpr uint32_t PRESENT_FIT_WINDOW  = 300;
        static constexpr uint32_t PRESENT_FIT_MIN     = 60;    // seed no earlier
        static constexpr uint32_t PRESENT_FIT_REFIT   = 60;    // refit ~1 Hz
        static constexpr float    PRESENT_FIT_FLOOR   = 0.004f;   // s — 250 Hz
        static constexpr float    PRESENT_MAX_MULTIPLE = 4.0f;
        static constexpr float    PRESENT_MIN_PERIOD   = 0.001f;   // s
        static constexpr uint32_t PRESENT_REPORT_FRAMES = 60;
        float    refreshPeriod_ = 1.0f / 60.0f;   // the panel's period; a divisor, so it is named as one
        float    presentFit_[PRESENT_FIT_WINDOW] = {};   // the trailing deltas the percentile is taken over
        uint32_t presentFitCursor_ = 0;
        uint32_t presentFitCount_  = 0;
        uint32_t presentFitTick_   = 0;
        bool     presentFitSeeded_ = false;
        uint32_t dtStrangers_   = 0;
        // The [PRESENT] histogram's counters (meter builds only; the arithmetic
        // is gated at its site, these are four words that cost nothing).
        uint32_t presentBuckets_[4] = {};
        uint32_t presentStrangers_ = 0;
        uint32_t presentFrames_    = 0;
        float    presentDeltaSum_  = 0.0f;
        float    presentDeltaMax_  = 0.0f;

        // ── Gpu Device ───────────────────────────────────────────
        std::optional<dawn::native::Instance> instance_;
        wgpu::Adapter adapter_;
        wgpu::Device device_;
        wgpu::Queue queue_;

        // ── Boot (PORT_1b) ───────────────────────────────────────
        BootState bootState_ = BootState::RequestingAdapter;
        bool deviceLost_ = false;   // PORT_3a — set by the loss callback, read by the frame gate

        // ── The probe's budget (the device gate) ─────────────────
        // Counted unconditionally: two increments a frame is not a cost
        // worth a branch, and a counter that only runs under a switch is a
        // counter nobody has ever seen run.
        uint32_t probeTurns_ = 0;       // begin_frame — every loop turn
        uint32_t probePresented_ = 0;   // present — frames that reached the swap

        // ── Surface & Presentation ───────────────────────────────
        wgpu::Surface surface_;
        wgpu::SurfaceConfiguration surfaceConfig_{};
        wgpu::TextureFormat colorFormat_;
        wgpu::SurfaceTexture surfaceTexture_;
        wgpu::TextureView backbuffer_;

        // ── Depth ────────────────────────────────────────────────
        wgpu::TextureFormat depthFormat_ = wgpu::TextureFormat::Depth24Plus;
        wgpu::Texture depthTexture_;
        // B10 — the msaa color target (created only when ?msaa=4; the
        // frame resolves into the backbuffer). Rides createDepthBuffer.
        wgpu::Texture msaaColorTexture_;
        wgpu::TextureView msaaColorView_;
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
