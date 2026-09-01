#pragma once
#include <cstdint>
#include <cstdio>   // WIT_2b — the witness prints its own line

// ═══ THE INSTRUMENTS DIAL ════════════════════════════════════════════
//
// The single compile-time door on the program's RECURRING self-measurement:
// the frame meter (per-row CPU clocks + per-pass GPU timestamp pairs + the
// [METER] table), the wall-clock [CENSUS] dump, the [CHECKER] read witness,
// and the harness's hot-reload tick. Every one of them was born inside a
// campaign that needed it and stayed on afterwards, so the shipped frame
// pays for readings nobody is reading.
//
// WHAT THE DIAL IS FOR. An instrument costs three ways, and the third is
// the one that bites:
//   1. per-frame work — 62 steady_clock reads and 20 armed timestamp pairs,
//      each frame, whether or not a window is ever printed;
//   2. per-frame GPU plumbing — ResolveQuerySet + CopyBufferToBuffer +
//      MapAsync, one readback in flight forever, and timestamp writes at
//      every pass boundary the driver would rather have merged;
//   3. the SPIKE — a reading is worthless unless it is printed, and a print
//      is a blocking console write on the render thread. ~50 lines every
//      30 s is one long frame every 30 s. That is the hiccup: not the
//      measuring, the reporting.
// Off, all three go to zero and every branch folds out at compile time.
//
// SHAPE. Same grammar as the demo sentence (demos/demo.hpp): a column
// selected by a define, token-pasted into an enumerator, folded into an
// `inline constexpr` the compiler eliminates against. A bad name
// (T7_INSTRUMENTS=xyz) becomes InstrumentCol::xyz — an unknown enumerator,
// a clean compile error, never a silent default.
//
//   cmake -DT7_INSTRUMENTS=full ...      → every instrument live
//   cmake -DT7_INSTRUMENTS=meter ...     → the frame meter + its table
//   (default, undefined)                 → off; the frame is the program's
//
// WHAT THE DIAL DOES NOT GOVERN. Boot reports and TRANSITION witnesses.
// Those are doctrine, not measurement (P6: every switch has a witness, and
// silence afterwards must mean "no transition", not "no witness"), and they
// cost nothing in steady state — boot is already a stall and a transition
// frame is already long. The census at "boot" and "rebirth" stays;
// only the PERIODIC one answers to the dial. Loud correctness checks
// (the ROSTER gol-residue proof, the entity_ref overflow drop, the SPINE
// boot validation) are witnesses too, and also stay: they print when
// something is WRONG, which is never, so they cost nothing when right.

namespace t7 {

    struct Instruments {
        bool frame_meter;      // per-row CPU clocks, per-pass GPU timestamps, the [METER] table
        bool periodic_census;  // the census CADENCE — and the [METER] table, which rides it
        bool census_entity_dump; // the ~50-line entity [CENSUS] text itself (HEADROOM_0 U3)
        bool checker_witness;  // the [CHECKER] line, one per checker read (~every 4 beats)
        bool zoetrope_witness; // the [ZOETROPE] strike line, one per strike-frame
        bool watcher_ticks;    // the harness's hot-reload progress dot (a flushed write, 2×/s)
        bool stream_witness;   // RIBBON_4 — the streaming path's spawn/evict lines
        // ATRIUM_5's PASSER WITNESS stood here and left with the round
        // (ONE_WORLD-I U4).
        //
        // ATRIUM_11 — THE CAMERA WITNESS, true in every column INCLUDING
        // `off`, on the same standing the passer census once had. The
        // camera's orbit is GPU truth with no CPU mirror anywhere in the
        // tree, so the only way to read the pose Jean has made with his own
        // mouse is to bring it back and print it — and a witness his own
        // build column would silence is not a witness. It prints on CHANGE
        // and no faster than 4 Hz, so a settled camera is silent. RETIRE IT
        // once the arrival row is settled — that is the whole of its
        // warrant, and it is the whole of the passer census's too.
        //
        // The flag also gates the READBACK: the staging buffer, the frame's
        // copy and the map all sit under `if constexpr`, so a build with the
        // witness off pays nothing at all — not a buffer, not a byte of
        // per-frame copy.
        bool camera_witness;
    };

    // THE COLUMNS. `off` is the shipped frame. `meter` is the timing arm
    // alone — what a performance session actually wants, without the
    // musical and harness chatter interleaved in the paste-back. `full` is
    // every instrument, the pre-dial behaviour exactly.
    enum class InstrumentCol : uint32_t { off, meter, full };

    constexpr Instruments instruments_config(InstrumentCol col) {
        switch (col) {
        // HEADROOM_0 U3 — `meter` keeps the cadence and the table and
        // DROPS the entity text. That text is ~50 blocking console writes
        // inside a frame the meter is trying to measure; the 2026-08-13
        // boot read census_dumps max 1051 ms. `full` keeps everything,
        // because `full` is the pre-dial behaviour exactly.
        //
        // RIBBON_4 — stream_witness is the seventh, and it is the reason the
        // steady state can be silent. `[Ribbon] SPAWN/REJECT/EVICT`
        // and `[Agents] Respawn` fire whenever a patch
        // spawns or evicts, which under a rider is several times a second
        // and rises with speed — blocking console writes inside exactly the
        // frames the conductor is trying to keep even. `full` keeps them,
        // because `full` is the pre-dial behaviour exactly; `meter` drops
        // them for the same reason it drops the entity text; `off` — the
        // shipped frame — is silent on the steady path.
        case InstrumentCol::meter: return { true,  true,  false, false, false, false, false, true };
        case InstrumentCol::full:  return { true,  true,  true,  true,  true,  true,  true,  true };
        case InstrumentCol::off:   break;
        }
        return { false, false, false, false, false, false, false, true };
    }

} // namespace t7

#ifndef T7_INSTRUMENTS
#define T7_INSTRUMENTS off
#endif

#define T7_INSTRUMENT_COL2(x) t7::InstrumentCol::x
#define T7_INSTRUMENT_COL(x)  T7_INSTRUMENT_COL2(x)

namespace t7 {

    inline constexpr Instruments INSTRUMENTS =
        instruments_config( T7_INSTRUMENT_COL(T7_INSTRUMENTS) );

    // ═══ PURSE_0 R2 — THE BUILD STAMP, AND WHY IT LIVES HERE ═════════════
    //
    // `git describe --always --dirty` + a UTC minute, generated into the
    // BUILD directory by tools/build_stamp.py and put on the include path
    // by CMake. It sits in this header because this is the one file the
    // console, the cartridge and the harness all already include, and it
    // has exactly the standing g_dropped_submits has below: a witness the
    // instruments DIAL DOES NOT GOVERN. A build that will not say which
    // build it is is the failure this ends.
    //
    // IT NAMES THE TREE, and after WEB_SUNSET that is the only sha there
    // is: the artifact digest it used to sit beside was the deploy's
    // (`web_dist.py`'s __BUILD_ID__), and both went with the web twin at
    // tag web-sunset. A build read off disk has no cached predecessor to
    // be confused with, so the tree fact is the whole question now.
    //
    // __has_include, AND IT IS NOT DEFENSIVENESS FOR ITS OWN SAKE. The TU
    // gate type-checks cartridge.hpp and console.hpp as standalone
    // translation units with NO CMake binary directory on the include
    // path, so the generated file is genuinely absent there. UNKNOWN IS A
    // VALUE: a build with no stamp says so rather than failing, and the
    // one thing a provenance line may never do is print a lie.
#if defined(__has_include)
#  if __has_include("build_stamp.gen.inc")
#    include "build_stamp.gen.inc"
#  else
    inline constexpr const char* BUILD_STAMP = "unknown";
#  endif
#else
    inline constexpr const char* BUILD_STAMP = "unknown";
#endif

    // ═══ WIT_2 — THE DROPPED-SUBMIT WITNESS ══════════════════════════
    //
    // Counts uncaptured VALIDATION errors whose message names an invalid
    // command buffer at submit — the signature of a whole frame being
    // thrown away. ACQ_0 removed the cause that made this happen (a depth
    // attachment that disagreed with the acquired texture), and this is how
    // anyone knows it stayed removed: the failure it witnesses is SILENT
    // otherwise. A dropped submit loses whatever one-shot GPU work happened
    // to be encoded that frame — spawn patch generation, the ground atlas,
    // live-card seeding — and the world simply comes up wrong later, far
    // from here.
    //
    // NOT GOVERNED BY THE DIAL, deliberately, and the banner above says why:
    // this is a loud correctness check, not a recurring measurement. It
    // increments only when something is WRONG, which is never, so it costs
    // nothing when right — the same standing the ROSTER residue proof and
    // the SPINE boot validation have. Turning the meter off must not turn
    // the witness off, because SOAK's gate reads it.
    //
    // SOAK's gate is ZERO. If it ever reads nonzero again, a re-arm
    // mechanism for one-shot GPU work earns its place — and not before,
    // because a re-arm that hides a dropped frame is worse than the frame.
    inline uint32_t g_dropped_submits = 0;

    // RIBBON_6 — THE CANVAS, crossing the console/cartridge boundary the same
    // way dropped_submits does: an instruments global, written where the size
    // is decided and read where the window line is composed. The cartridge is
    // handed an aspect RATIO and never a pixel size, and the canvas is the one
    // variable that changed silently between windows in the recording that
    // opened RIBBON_6 — a GPU budget read against an unknown resolution is not
    // a reading. Two words, written once per resize.
    inline uint32_t g_canvas_w = 0;
    inline uint32_t g_canvas_h = 0;

    // ── WIT_2b — AND IT GETS ITS OWN LINE ─────────────────────────────
    //
    // WIT_2 appended the count to the [METER] window header and it was
    // never once read on a machine that had it to report. The header is
    // formatted with snprintf into char[160]; with the GPU arm armed the
    // line renders 169 characters, ` | dropped_submits` begins at byte
    // 148, and the buffer cuts at 159 — mid-token, at `| dropped_`. The
    // CPU arm fits in 68, so the counter printed on exactly the machines
    // that had no timestamp-query and stayed silent on every machine that
    // did. A witness appended to a crowded line is a witness with a
    // truncation hazard for a mouth.
    //
    // So it speaks alone, and it speaks ALWAYS — including at zero, which
    // is the whole point: a witness that only speaks when guilty cannot be
    // distinguished from one that was never armed. One formatting site,
    // here, beside the counter; the window close and the teardown both
    // call it rather than each spelling the line their own way.
    inline void print_dropped_submits(const char* when) {
        std::printf("[WIT] dropped_submits %u (%s)\n", g_dropped_submits, when);
    }

    // THE FIRST EDGE — a meter that measures and never reports is a cost
    // with no reading. The [METER] table prints on the census cadence (one
    // interval, one paste-back block), so the timing arm REQUIRES the census
    // arm. Conditional, so both-off (the shipped frame) stays legal.
    static_assert(!INSTRUMENTS.frame_meter || INSTRUMENTS.periodic_census,
        "INSTRUMENTS: frame_meter on with periodic_census off — the [METER] "
        "table prints on the census cadence, so the meter would accumulate "
        "every frame and report nothing");

} // namespace t7
