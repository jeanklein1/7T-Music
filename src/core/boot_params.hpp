#pragma once
// ═══ THE PARAMETER SURFACE (DOMESDAY_1 B9 — LANTERN's deferred U2) ═══
//
// Measurement first. LANTERN_CENSUS §L2 recorded the gap this fills:
// the four moods were diegetically reachable but not addressable — a
// soak walk could not drive the twin deterministically to a named arm,
// so no capture could be known repeatable. The MOODS themselves left at
// ONE_WORLD-II U2 and `--mood=` went with them: there is one world, and
// its seed is the whole of what a walk needs to name. The values, read
// ONCE at boot before the device request, never reread, never mutated
// mid-run, invisible to ordinary visitors:
//
//   seed — u32, overrides the drawn boot seed (the [World] line then
//          says "(param)" instead of "(drawn)")
//   msaa — {1, 4}, the multisample count the pipelines are created with
//   scene — a scene FILE, applied through the organ once the ABI is
//          bound and re-applied on every save (THE_PANEL II U1)
//   probe — THE DEVICE GATE. Run N frames and exit on the device's own
//          verdict. See the banner below; it is the one parameter that
//          changes what the program DOES rather than what it looks like.
//   probe-backend — {any, null, cpu}, the probe's adapter ladder.
//
// Channel: `--seed= --msaa= --scene= --probe= --probe-backend=` on argv, read ONCE at boot before
// the device request, never reread, never mutated mid-run. Absent or
// malformed values are silently ignored; anything accepted prints one
// [Params] line (P6 — a switch that fired is visible). The URL channel
// (`?seed=` …), the pixel cap and the pace lever went with the web twin
// at tag web-sunset — their only consumers were its presentation layer.

#include <cstdint>
#include <string>   // --scene='s path (THE_PANEL II U1)
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace t7 {

    // ═══ THE DEVICE GATE — WHY THE PROGRAM HAS A PROBE MODE ══════════
    //
    // Every gate in this tree reads TEXT. G-LAW 2 parses WGSL for dangling
    // names, the TU gate type-checks C++, the mirror census diffs idioms,
    // binding_gen proves the schema against the tree. Not one of them runs
    // a shader on a device, and there is a whole class of defect that only
    // a device can see: a number that is legal C++, legal WGSL, and wrong
    // ACROSS the two.
    //
    // The class has a name in this repo's history now. ONE_SURFACE-I U5
    // folded a draw-plan segment, shrank the frustum arg buffers to 40
    // bytes in state.hpp, and left world.wgsl declaring
    // `array<atomic<u32>, 15>` — 60 bytes, and that literal is what Dawn
    // computes the pipeline's minimum binding size from. The full battery
    // was GREEN. Every frame of the first native boot failed validation
    // and the world could not draw.
    //
    // So: `--probe=N` boots the program, runs N frames, and exits on what
    // the DEVICE said — zero uncaptured errors is PROBE GREEN and exit 0;
    // anything else prints the first error verbatim and exits nonzero. It
    // is one command, and no constructive GPU work ships without it.
    //
    // THE LADDER (`--probe-backend=`). The probe wants an adapter that
    // VALIDATES and does nothing else — Dawn's frontend raises the errors
    // the probe hunts before any backend is reached, so a Null or CPU
    // adapter returns the same verdict for none of the wall clock, and a
    // machine with no GPU could run it. That is the goal, and it is
    // DEFAULTED OFF: whether Dawn's null backend can serve this console's
    // real GLFW surface is not knowable from the tree, and an instrument
    // that will not boot is worse than no instrument. `--probe=N` alone
    // runs the ordinary adapter pick — exactly the configuration whose log
    // opened this commission, and therefore the one configuration the
    // probe is already known to work in. One boot with
    // `--probe-backend=null` settles the rest; nothing depends on the
    // answer until it is asked.
    enum class ProbeBackend { Any, Null, CPU };

    struct BootParams {
        bool has_seed = false; uint32_t seed = 0;
        // THE SCENE ROAD (THE_PANEL II U1). A path, not a name: the web
        // channel's `?preset=<name>` indexed a shelf the page fetched, and
        // a native program with a file system does not need an index to
        // open a file. It is also WATCHED — the same FileWatcher the
        // shader reload uses, a second instance — so `--scene=` is both
        // "apply this at boot" and "apply this again whenever it is
        // saved". One flag, one road.
        bool has_scene = false; std::string scene;
        bool has_msaa = false; uint32_t msaa = 1;   // DOMESDAY_2 B10: 1 or 4; anything else -> 1
        bool has_probe = false; uint32_t probe_frames = 120;
        ProbeBackend probe_backend = ProbeBackend::Any;
    };

    // Set once by parse_boot_params (main, before any consumer);
    // read-only ever after.
    inline BootParams& boot_params() {
        static BootParams p;
        return p;
    }

    // B10 — the walk's last instrument: multisampling as a boot-read
    // measurement affordance. {1, 4} only; the default stays 1 until
    // the soak walk prices the matrix, and the default flip afterward
    // is one constant. Pipelines are created once with this value —
    // no mid-run mutation, per the surface's law. (F1-b: defined
    // BELOW the accessor it reads — glaw1 caught the original order.)
    inline uint32_t effective_msaa() {
        return boot_params().has_msaa ? boot_params().msaa : 1u;
    }

    // THE PROBE'S PATIENCE. The frame budget counts PRESENTED frames, the
    // same convention the meter's window uses — a frame that fails its
    // acquire is not a frame. That alone cannot terminate a probe whose
    // acquire never succeeds, so the loop also spends a turn budget, and
    // exhausting it is a RED with its own sentence: a program that cannot
    // present is not a program that passed. Generous on purpose — boot,
    // the first-acquire depth build and any driver warm-up all spend turns
    // that present nothing.
    inline uint32_t probe_turn_budget() {
        return 4u * boot_params().probe_frames + 240u;
    }

    inline void boot_params_announce_() {
        BootParams& p = boot_params();
        if (p.has_msaa && p.msaa != 4u) {
            p.msaa = 1u;   // B10: {1, 4} only; anything else -> 1
        }
        if (p.has_seed || p.has_msaa || p.has_probe || p.has_scene) {
            std::cout << "[Params]";
            if (p.has_seed) std::cout << " seed=" << p.seed;
            if (p.has_scene) std::cout << " scene=" << p.scene;
            if (p.has_msaa) std::cout << " msaa=" << p.msaa;
            if (p.has_probe) {
                std::cout << " probe=" << p.probe_frames << " probe-backend="
                    << (p.probe_backend == ProbeBackend::Null ? "null"
                      : p.probe_backend == ProbeBackend::CPU  ? "cpu" : "any");
            }
            std::cout << "\n";
        }
    }

    inline void parse_boot_params(int argc, char** argv) {
        BootParams& p = boot_params();
        for (int i = 1; i < argc; i++) {
            const char* a = argv[i];
            char* end = nullptr;
            if (std::strncmp(a, "--seed=", 7) == 0) {
                unsigned long long v = std::strtoull(a + 7, &end, 10);
                if (end && *end == '\0' && end != a + 7 && v <= 0xFFFFFFFFull) {
                    p.has_seed = true; p.seed = static_cast<uint32_t>(v);
                }
            } else if (std::strncmp(a, "--scene=", 8) == 0) {
                // A PATH IS ACCEPTED AS TYPED and never validated here.
                // Boot params are read before the device request; whether
                // the file opens, parses and resolves is the import walk's
                // question, asked once the ABI is bound and answered out
                // loud (console/organ_scene.hpp). A silent drop here would
                // hide a typo behind a world that looks merely default.
                if (a[8] != '\0') { p.has_scene = true; p.scene = a + 8; }
            } else if (std::strncmp(a, "--msaa=", 7) == 0) {
                unsigned long long v = std::strtoull(a + 7, &end, 10);
                if (end && *end == '\0' && end != a + 7 && v <= 0xFFFFFFFFull) {
                    p.has_msaa = true; p.msaa = static_cast<uint32_t>(v);
                }
            } else if (std::strcmp(a, "--probe") == 0) {
                p.has_probe = true;   // bare: the default frame budget
            } else if (std::strncmp(a, "--probe=", 8) == 0) {
                unsigned long long v = std::strtoull(a + 8, &end, 10);
                if (end && *end == '\0' && end != a + 8 && v >= 1ull && v <= 1000000ull) {
                    p.has_probe = true; p.probe_frames = static_cast<uint32_t>(v);
                }
            } else if (std::strcmp(a, "--probe-backend=null") == 0) {
                p.probe_backend = ProbeBackend::Null;
            } else if (std::strcmp(a, "--probe-backend=cpu") == 0) {
                p.probe_backend = ProbeBackend::CPU;
            } else if (std::strcmp(a, "--probe-backend=any") == 0) {
                p.probe_backend = ProbeBackend::Any;
            }
        }
        boot_params_announce_();
    }

} // namespace t7
