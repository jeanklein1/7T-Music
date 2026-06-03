#pragma once

// ─── clock.hpp ───────────────────────────────────────────────────
//
// Composition's time authority. Composition owns the clock, which
// ensures determinism: given the same dt sequence, the same outputs are
// produced. The clock tracks both wall-clock time (seconds) and musical
// time (beats); adapters call tick() with dt from their frame timing.
//
// Determinism note: the clock does NOT read system time. It accumulates
// whatever dt the caller provides, so replaying the same dt sequence
// produces identical results, tests can use fixed dt for reproducibility,
// and variable frame rates still work (the adapter provides real dt).
//
// Depends on: <algorithm>.

#include <algorithm>

namespace t7 {

class Clock {
public:
    Clock() = default;
    
    /**
     * Advance the clock by dt seconds.
     * Call once per frame from Composition::update().
     * 
     * @param dt_seconds Delta time in seconds (from adapter's frame timing)
     * @return The dt that was applied (clamped for safety)
     */
    float tick(float dt_seconds) {
        // Clamp dt to avoid spiral of death
        dt_ = std::clamp(dt_seconds, 0.0f, max_dt_);
        
        t_seconds_ += dt_;
        t_beats_ += dt_ * (bpm_ / 60.0f);
        
        return dt_;
    }
    
    // â”€â”€â”€ ACCESSORS â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    
    float t_seconds() const { return t_seconds_; }
    float t_beats() const { return t_beats_; }
    float dt() const { return dt_; }
    float bpm() const { return bpm_; }
    
    /**
     * Beats per second at current tempo.
     */
    float beats_per_second() const { return bpm_ / 60.0f; }
    
    /**
     * Seconds per beat at current tempo.
     */
    float seconds_per_beat() const { return 60.0f / bpm_; }
    
    // â”€â”€â”€ CONFIGURATION â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€â”€
    
    void set_bpm(float bpm) { bpm_ = std::max(1.0f, bpm); }
    void set_max_dt(float max_dt) { max_dt_ = max_dt; }
    
    /**
     * Reset clock to zero.
     */
    void reset() {
        t_seconds_ = 0.0f;
        t_beats_ = 0.0f;
        dt_ = 0.0f;
    }
    
    /**
     * Seek to specific beat position.
     */
    void seek_beats(float beats) {
        t_beats_ = beats;
        t_seconds_ = beats * seconds_per_beat();
    }
    
    /**
     * Seek to specific time position.
     */
    void seek_seconds(float seconds) {
        t_seconds_ = seconds;
        t_beats_ = seconds * beats_per_second();
    }
    
private:
    float t_seconds_ = 0.0f;
    float t_beats_ = 0.0f;
    float dt_ = 0.0f;
    float bpm_ = 120.0f;
    float max_dt_ = 0.1f;  // 100ms max to prevent spiral of death
};

} // namespace t7
