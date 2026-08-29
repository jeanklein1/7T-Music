#pragma once
// ═══ THE PARAMETER SURFACE (DOMESDAY_1 B9 — LANTERN's deferred U2) ═══
//
// Measurement first. LANTERN_CENSUS §L2 recorded the gap this fills:
// the four moods were diegetically reachable but not addressable — a
// soak walk could not drive the twin deterministically to a named arm,
// so no capture could be known repeatable. The values, read ONCE at
// boot before the device request, never reread, never mutated mid-run,
// invisible to ordinary visitors:
//
//   seed — u32, overrides the drawn boot seed (the [World] line then
//          says "(param)" instead of "(drawn)")
//   mood — index into MOOD_TABLE, forces the boot mood at the one
//          site that authors it (the Cartridge ctor; range-checked
//          there against MOOD_COUNT, which this header must not know)
//   msaa — {1, 4}, the multisample count the pipelines are created with
//
// Channel: `--seed= --mood= --msaa=` on argv, read ONCE at boot before
// the device request, never reread, never mutated mid-run. Absent or
// malformed values are silently ignored; anything accepted prints one
// [Params] line (P6 — a switch that fired is visible). The URL channel
// (`?seed=` …), the pixel cap and the pace lever went with the web twin
// at tag web-sunset — their only consumers were its presentation layer.

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace t7 {

    struct BootParams {
        bool has_seed = false; uint32_t seed = 0;
        bool has_mood = false; uint32_t mood = 0;
        bool has_msaa = false; uint32_t msaa = 1;   // DOMESDAY_2 B10: 1 or 4; anything else -> 1
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

    inline void boot_params_announce_() {
        BootParams& p = boot_params();
        if (p.has_msaa && p.msaa != 4u) {
            p.msaa = 1u;   // B10: {1, 4} only; anything else -> 1
        }
        if (p.has_seed || p.has_mood || p.has_msaa) {
            std::cout << "[Params]";
            if (p.has_seed) std::cout << " seed=" << p.seed;
            if (p.has_mood) std::cout << " mood=" << p.mood;
            if (p.has_msaa) std::cout << " msaa=" << p.msaa;
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
            } else if (std::strncmp(a, "--mood=", 7) == 0) {
                unsigned long long v = std::strtoull(a + 7, &end, 10);
                if (end && *end == '\0' && end != a + 7 && v <= 0xFFFFFFFFull) {
                    p.has_mood = true; p.mood = static_cast<uint32_t>(v);
                }
            } else if (std::strncmp(a, "--msaa=", 7) == 0) {
                unsigned long long v = std::strtoull(a + 7, &end, 10);
                if (end && *end == '\0' && end != a + 7 && v <= 0xFFFFFFFFull) {
                    p.has_msaa = true; p.msaa = static_cast<uint32_t>(v);
                }
            }
        }
        boot_params_announce_();
    }

} // namespace t7
