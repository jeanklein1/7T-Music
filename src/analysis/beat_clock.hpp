#pragma once

// ─── beat_clock.hpp ──────────────────────────────────────────────
//
// CUT_1c: the analysis intake's successor (ruling R7). MIDI/DAW
// analysis left the build; the render side keeps its two contracts —
// an AnalysisSignal each frame and a StatLayoutView once at bind.
// The BeatClock serves both from nothing but dt: advancing clocks at
// a variable BPM (default 100; this struct is the value's ONE home,
// panel-eligible), and an EMPTY layout.
//
// The empty layout is the audio socket. The render side resolves 12
// live source names against it — all.field, ch1.present_count,
// all.window_length, all.present_count, ch1.window_length,
// ch0.onset .. ch6.onset — and every resolve misses and disables its
// coupling via the graceful path (musical/signal_layout.hpp
// resolve(): one stderr warn, valid=false). A future browser-side
// audio source plugs into this socket by publishing exactly those
// names through a real StatLayoutView.

#include "analysis/analysis_signal.hpp"

namespace t7 {

struct BeatClock {
    float bpm = 100.0f;   // variable BPM — Jean's amendment; one home

    void update(float dt) {
        dt_ = dt;
        seconds_ += dt;
        beats_ += dt * (bpm / 60.0f);
    }

    // Every time-bearing field of the surviving contract, from the
    // clock; everything else (stats, pads) value-initialized to zero.
    // AnalysisSignal carries no transport flag — nothing to default.
    AnalysisSignal output() const {
        AnalysisSignal s{};
        s.t_seconds = seconds_;
        s.t_beats   = beats_;
        s.dt        = dt_;
        return s;
    }

    StatLayoutView stat_layout() const { return StatLayoutView{ nullptr, 0 }; }

private:
    float seconds_ = 0.0f;
    float beats_   = 0.0f;
    float dt_      = 0.0f;
};

} // namespace t7
