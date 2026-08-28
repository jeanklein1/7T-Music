#pragma once
// ═══ THE PARAMETER SURFACE (DOMESDAY_1 B9 — LANTERN's deferred U2) ═══
//
// Measurement first. LANTERN_CENSUS §L2 recorded the gap this fills:
// the four moods were diegetically reachable but not addressable — a
// soak walk could not drive the twin deterministically to a named arm,
// so no capture could be known repeatable. Three values, read ONCE at
// boot before the device request, never reread, never mutated mid-run,
// invisible to ordinary visitors:
//
//   seed — u32, overrides the drawn boot seed (the [World] line then
//          says "(param)" instead of "(drawn)")
//   mood — index into MOOD_TABLE, forces the boot mood at the one
//          site that authors it (the Cartridge ctor; range-checked
//          there against MOOD_COUNT, which this header must not know)
//   cap  — float clamped to [0.5, 3.0] at parse, overriding
//          MAX_DEVICE_PIXEL_RATIO for this run (console.hpp
//          effective_pixel_cap) — the soak walk's key and the frame's
//          largest lever
//
// Channel: `?seed=&mood=&cap=` — ONE URLSearchParams read of
// location.search, in one EM_ASM block. Absent or malformed values
// are silently ignored; anything accepted prints one [Params] line
// at parse time (P6 — a switch that fired is visible). No UI, no
// mid-run reread.

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace t7 {

    struct BootParams {
        bool has_seed = false; uint32_t seed = 0;
        bool has_mood = false; uint32_t mood = 0;
        bool has_cap  = false; float    cap  = 1.0f;
        bool has_msaa = false; uint32_t msaa = 1;   // DOMESDAY_2 B10: 1 or 4; anything else -> 1
        // PANORAMA_1 U6 — THE METRONOME, forced. rAF callbacks per presented
        // frame: 1 = every vblank, 2 = every second one (a steady 30 on a
        // 60 Hz panel). Present ONLY to make the taste gate takeable — Jean
        // rides the same world at each and says which is the piece. When the
        // param is present the governor never engages: a forced pace is an
        // instruction, not a starting point.
        bool has_pace = false; uint32_t pace = 1;   // {1, 2}; anything else -> 1
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
        if (p.has_cap) {
            if (p.cap < 0.5f) p.cap = 0.5f;
            if (p.cap > 3.0f) p.cap = 3.0f;
        }
        if (p.has_msaa && p.msaa != 4u) {
            p.msaa = 1u;   // B10: {1, 4} only; anything else -> 1
        }
        if (p.has_pace && p.pace != 2u) {
            p.pace = 1u;   // U6: {1, 2} only; anything else -> 1
        }
        if (p.has_seed || p.has_mood || p.has_cap || p.has_msaa || p.has_pace) {
            std::cout << "[Params]";
            if (p.has_seed) std::cout << " seed=" << p.seed;
            if (p.has_mood) std::cout << " mood=" << p.mood;
            if (p.has_cap)  std::cout << " cap=" << p.cap;
            if (p.has_msaa) std::cout << " msaa=" << p.msaa;
            if (p.has_pace) std::cout << " pace=" << p.pace;
            std::cout << "\n";
        }
    }

#ifdef __EMSCRIPTEN__

    // One location.search read; three typed extractions into HEAPF64.
    // NaN = absent-or-malformed; integer/range checks land on the C++
    // side so the rule has one spelling per twin.
    inline void parse_boot_params(int, char**) {
        double vals[5];
        EM_ASM({
            var q = new URLSearchParams(location.search);
            var o = $0 >> 3;
            function num(k) {
                var v = q.get(k);
                if (v === null || v === "") return NaN;
                var n = Number(v);
                return isFinite(n) ? n : NaN;
            }
            HEAPF64[o]     = num('seed');
            HEAPF64[o + 1] = num('mood');
            HEAPF64[o + 2] = num('cap');
            HEAPF64[o + 3] = num('msaa');
            HEAPF64[o + 4] = num('pace');
        }, vals);
        BootParams& p = boot_params();
        if (!std::isnan(vals[0]) && vals[0] >= 0.0 && vals[0] <= 4294967295.0
                && vals[0] == std::floor(vals[0])) {
            p.has_seed = true; p.seed = static_cast<uint32_t>(vals[0]);
        }
        if (!std::isnan(vals[1]) && vals[1] >= 0.0 && vals[1] <= 4294967295.0
                && vals[1] == std::floor(vals[1])) {
            p.has_mood = true; p.mood = static_cast<uint32_t>(vals[1]);
        }
        if (!std::isnan(vals[2])) {
            p.has_cap = true; p.cap = static_cast<float>(vals[2]);
        }
        if (!std::isnan(vals[4]) && vals[4] == std::floor(vals[4])
                && vals[4] >= 0.0 && vals[4] <= 4294967295.0) {
            p.has_pace = true; p.pace = static_cast<uint32_t>(vals[4]);
        }
        if (!std::isnan(vals[3]) && vals[3] == std::floor(vals[3])
                && vals[3] >= 0.0 && vals[3] <= 4294967295.0) {
            p.has_msaa = true; p.msaa = static_cast<uint32_t>(vals[3]);
        }
        boot_params_announce_();
    }

#else

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
            } else if (std::strncmp(a, "--cap=", 6) == 0) {
                float v = std::strtof(a + 6, &end);
                if (end && *end == '\0' && end != a + 6 && std::isfinite(v)) {
                    p.has_cap = true; p.cap = v;
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

#endif  // __EMSCRIPTEN__ — the byte source, and only the byte source

} // namespace t7
