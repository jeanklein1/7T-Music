#pragma once

// ─── transport.hpp ───────────────────────────────────────────────
//
// DAW musical time from MIDI clock + transport. Pure (no RtMidi): fed the
// raw MIDI messages a port receives, it maintains the play state and the
// musical position by counting 0xF8 timing-clock pulses — 24 per quarter
// note. Position is therefore exact and phase-locked to the DAW; tempo is
// estimated only for display and is never used to advance the beat.
//
// Why counting, not timing: the window we analyse is defined in beats, so
// its edges must land on the DAW's beats. A pulse count does that exactly.
// And because a DAW sends clock only while playing, a stop freezes the
// count — the windows stop sliding on their own, no pause logic needed.
//
// Threading: feed() runs on the MIDI input thread; the accessors are read
// on the main thread. The shared state is atomic; the tempo-estimate
// scratch is touched only by feed().
//
// Depends on: <atomic>, <cstdint>, <vector>.

#include <atomic>
#include <cstdint>
#include <vector>

namespace t7 {

constexpr int MIDI_CLOCK_PPQN = 24;   // timing-clock pulses per quarter note

class MidiTransport {
public:
    // Offer one raw MIDI message (with RtMidi's delta-time, seconds since the
    // previous message). Returns true if it was a clock/transport/position
    // message — consumed here, and NOT to be treated as a note by the caller.
    bool feed(double delta_seconds, const std::vector<unsigned char>& m) {
        acc_ += delta_seconds;            // every message spaces the next pulse
        if (m.empty()) return false;

        switch (m[0]) {
            case 0xF8:                    // timing clock
                on_clock();
                return true;
            case 0xFA:                    // start — locate to 0 and run
                pulses_.store(0, std::memory_order_release);
                playing_.store(true, std::memory_order_release);
                return true;
            case 0xFB:                    // continue — run from here
                playing_.store(true, std::memory_order_release);
                return true;
            case 0xFC:                    // stop
                playing_.store(false, std::memory_order_release);
                return true;
            case 0xF2:                    // song position pointer (1/16 notes)
                if (m.size() >= 3) {
                    const uint32_t spp = (uint32_t(m[2]) << 7) | uint32_t(m[1]);
                    pulses_.store(spp * 6, std::memory_order_release);  // 6 pulses / 1/16
                    synced_.store(true, std::memory_order_release);
                }
                return true;
            default:
                return false;             // a note or other channel message
        }
    }

    // ── Read side (main thread) ──────────────────────────────────

    bool     playing() const { return playing_.load(std::memory_order_acquire); }
    uint32_t pulses()  const { return pulses_.load(std::memory_order_acquire); }
    double   beats()   const { return double(pulses()) / MIDI_CLOCK_PPQN; }
    float    bpm()     const { return bpm_.load(std::memory_order_relaxed); }

    // True once any clock or position message has arrived — lets a caller
    // tell "DAW not sending clock yet" from "DAW parked at beat 0".
    bool ever_synced() const { return synced_.load(std::memory_order_acquire); }

    void reset() {
        pulses_.store(0);
        playing_.store(false);
        bpm_.store(0.0f);
        synced_.store(false);
        acc_ = 0.0;
        ema_ = 0.0;
    }

private:
    std::atomic<uint32_t> pulses_{0};
    std::atomic<bool>     playing_{false};
    std::atomic<float>    bpm_{0.0f};
    std::atomic<bool>     synced_{false};

    // Tempo estimate — feed()-thread scratch only, never shared.
    double acc_ = 0.0;    // seconds accumulated since the last pulse
    double ema_ = 0.0;    // smoothed seconds-per-pulse

    void on_clock() {
        pulses_.fetch_add(1, std::memory_order_acq_rel);
        synced_.store(true, std::memory_order_release);

        const double interval = acc_;     // seconds since the previous pulse
        acc_ = 0.0;
        if (interval > 1e-6) {
            ema_ = (ema_ <= 0.0) ? interval : (ema_ * 0.8 + interval * 0.2);
            const double seconds_per_beat = ema_ * MIDI_CLOCK_PPQN;
            if (seconds_per_beat > 1e-6)
                bpm_.store(float(60.0 / seconds_per_beat), std::memory_order_relaxed);
        }
    }
};

} // namespace t7
