#pragma once
#include <cstdint>

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
// frame is already long. The census at "boot" and "mood-transition" stays;
// only the PERIODIC one answers to the dial. Loud correctness checks
// (the ROSTER gol-residue proof, the entity_ref overflow drop, the SPINE
// boot validation) are witnesses too, and also stay: they print when
// something is WRONG, which is never, so they cost nothing when right.

namespace t7 {

    struct Instruments {
        bool frame_meter;      // per-row CPU clocks, per-pass GPU timestamps, the [METER] table
        bool periodic_census;  // the wall-clock [CENSUS] dump (the spine's census row)
        bool checker_witness;  // the [CHECKER] line, one per checker read (~every 4 beats)
        bool watcher_ticks;    // the harness's hot-reload progress dot (a flushed write, 2×/s)
    };

    // THE COLUMNS. `off` is the shipped frame. `meter` is the timing arm
    // alone — what a performance session actually wants, without the
    // musical and harness chatter interleaved in the paste-back. `full` is
    // every instrument, the pre-dial behaviour exactly.
    enum class InstrumentCol : uint32_t { off, meter, full };

    constexpr Instruments instruments_config(InstrumentCol col) {
        switch (col) {
        case InstrumentCol::meter: return { true,  true,  false, false };
        case InstrumentCol::full:  return { true,  true,  true,  true };
        case InstrumentCol::off:   break;
        }
        return { false, false, false, false };
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

    // THE FIRST EDGE — a meter that measures and never reports is a cost
    // with no reading. The [METER] table prints on the census cadence (one
    // interval, one paste-back block), so the timing arm REQUIRES the census
    // arm. Conditional, so both-off (the shipped frame) stays legal.
    static_assert(!INSTRUMENTS.frame_meter || INSTRUMENTS.periodic_census,
        "INSTRUMENTS: frame_meter on with periodic_census off — the [METER] "
        "table prints on the census cadence, so the meter would accumulate "
        "every frame and report nothing");

} // namespace t7
