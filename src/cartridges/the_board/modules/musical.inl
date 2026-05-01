// ─── musical.inl ─────────────────────────────────────────────────
//
// The coupling layer: connects analysis signal to visual parameters.
// Mode definitions, intensity trajectories, band motion, palette
// drift, radial pulse, per-frame update, teardown reset.
//
// Included inside the Cartridge class body.
// Depends on: pawn.inl (auraCfgDirty_)
// ─────────────────────────────────────────────────────────────────


// ─── Polyphony-driven band motion ────────────────────────────
// Band activation order: fine(4) → detail(3) → local(2) → regional(1) → continental(0) → tectonic(5)
// First note animates fine ripples, full chord reshapes the continent.
static constexpr uint32_t BAND_ACTIVATION_ORDER[6] = { 4, 3, 2, 1, 0, 5 };
static constexpr float BAND_BLEND_ATTACK = 3.0f;   // 1/s — blend ramp up speed
static constexpr float BAND_BLEND_RELEASE = 2.0f;  // 1/s — blend ramp down speed
bool bandMotionActive_ = false;       // true when polyphony drives bands (mood 5 only)
float bandBlend_[6] = { -1.f, -1.f, -1.f, -1.f, -1.f, -1.f };  // per-band blend factor (-1 = activity field)
float bandPhaseOrigin_[6] = {};       // t_beats when band was activated
float bandBlendTarget_[6] = {};       // 0 or 1, driven by polyphony count

// ─── Musical animation modes (numpad toggles) ────────────────
// Each mode is an independently toggleable coupling circuit.
// When on: polyphony drives the mode's intensity through trajectory ramp.
// When off: intensity releases to 0 (idle).
//
// Future: each mode's source can be rewired to any analysis stat.
// Today: all modes read polyphony as their input signal.
//
//   Numpad 1 = terrain waves  (existing band motion — retroactively mode 0)
//   Numpad 2 = color shift    (smooth → discrete mode bias)
//   Numpad 3 = checker scatter (sparse survival threshold bias)
//   Numpad 4 = palette drift   (terrain color drifts toward target palette)
//   Numpad 5 = GoL tempo       (polyphony speeds up zones + scales height)
//   Numpad 6 = aura expansion (influence radius + height + tint intensity)

static constexpr uint32_t MMODE_TERRAIN_WAVES = 0;   // band motion (existing)
static constexpr uint32_t MMODE_COLOR_SHIFT = 1;
static constexpr uint32_t MMODE_CHECKER_SCATTER = 2;
static constexpr uint32_t MMODE_PALETTE_DRIFT = 3;
static constexpr uint32_t MMODE_GOL_TEMPO = 4;
static constexpr uint32_t MMODE_AURA_EXPAND = 5;
static constexpr uint32_t MMODE_COUNT = 6;   // numpad 1–6 (intensity-driven modes)

// Radial pulse mode: event-driven (no intensity trajectory).
// Toggle gates onset detection; existing pulses decay naturally.
static constexpr uint32_t MMODE_RADIAL_PULSE = 7;   // numpad 7 (separate from intensity array)

// DONE[musical:L2] mode-name registry promoted to module-level constant.
//   Was previously a hidden static array inside toggle_mmode. Indexed
//   by mode id [0..7]; index 6 is "UNUSED" because MMODE_RADIAL_PULSE
//   skips to 7 (musical:L4 — semantic-kind off-by-one, folds into K1).
static constexpr const char* MMODE_NAMES[] = {
    "terrain_waves", "color_shift", "checker_scatter",
    "palette_drift", "gol_tempo",   "aura_expand",
    "UNUSED",        "radial_pulse"
};

static constexpr float MMODE_ATTACK = 4.0f;    // 1/s — intensity ramp up
static constexpr float MMODE_RELEASE = 2.5f;   // 1/s — intensity ramp down

uint32_t mmodeMask_ = 0;              // bitfield: which modes are active
float mmodeIntensity_[MMODE_COUNT] = {};  // current [0,1] per mode (trajectory value)

// Palette drift: target palette index ramps smoothly to avoid color snaps
float paletteDriftTarget_ = 0.0f;       // current [0,3] — ramps toward desired
float paletteDriftDesired_ = 0.0f;      // set by polyphony mapping
static constexpr float PALETTE_DRIFT_TARGET_RATE = 2.0f;  // 1/s — smooth target transition

// Radial pulse ring buffer: 8 slots, circular write.
// Each slot: (origin_x, origin_z, onset_seconds, amplitude)
static constexpr uint32_t PULSE_RING_SIZE = 8;
static constexpr float PULSE_AMPLITUDE = 2.5f;     // world units of peak displacement
static constexpr float PULSE_MAX_AGE = 8.0f;       // seconds — must match WGSL
float pulseRing_[32] = {};              // 8 × 4 floats
uint32_t pulseWriteIdx_ = 0;            // next slot to write (wraps at 8)
float prevPolyphony_ = 0.0f;            // previous frame's polyphony (for onset detection)

bool is_mmode_on(uint32_t mode) const {
    if (!moodAllowsMusicalModes_) { return false; }   // mood gate — silences all modes
    return (mmodeMask_ & (1u << mode)) != 0;
}
void toggle_mmode(uint32_t mode) {
    mmodeMask_ ^= (1u << mode);
    bool on = is_mmode_on(mode);
    // Retroactive: mode 0 controls bandMotionActive_
    if (mode == MMODE_TERRAIN_WAVES) {
        bandMotionActive_ = on;
        if (bandMotionActive_) {
            for (int i = 0; i < 6; i++) {
                bandBlend_[i] = 0.0f;
                bandBlendTarget_[i] = 0.0f;
                bandPhaseOrigin_[i] = 0.0f;
            }
            gpuState_.set_band_motion(bandBlend_, bandPhaseOrigin_);
            gpuState_.set_terrain_time(0.0f);
        }
        else {
            float inactive[6] = { -1.f, -1.f, -1.f, -1.f, -1.f, -1.f };
            float zeros[6] = {};
            gpuState_.set_band_motion(inactive, zeros);
            gpuState_.set_terrain_time(0.0f);
        }
    }
    // Mode 5 (aura expand): mark config dirty to push updated aura params
    if (mode == MMODE_AURA_EXPAND) {
        auraCfgDirty_ = true;
    }
    std::cout << "[MMode] " << MMODE_NAMES[mode] << ": " << (on ? "ON" : "OFF") << "\n";
}



// ─── Per-frame musical coupling tick ─────────────────────────────
//
// DONE[musical:K2] All polyphony→intensity ramps moved out of
//   cartridge.hpp::update() into this single named tick. The three
//   blocks that used to live there:
//     1) band motion (BAND_BLEND_ATTACK/RELEASE per band)
//     2) musical mode intensities (MMODE_ATTACK/RELEASE per mode)
//     3) palette drift target + intensity ramp
//   ...plus radial pulse onset detection (musical:K3 site) all
//   happen here in one call, in the same order they used to run.
//   Every exponential ramp uses the same Trajectory primitive shape
//   as WGSL §1.2 (trajectory_release).
//
// Caller: cartridge.hpp::update() — single site, runs every frame
// after the signal has been uploaded and before orb couplings.
void tick_musical_couplings(const AnalysisSignal& signal, wgpu::Queue& queue) {
    const float polyphony = signal.stats[0];
    const float dt        = signal.dt;

    // ─── 1. Polyphony-driven band motion ──────────────────────────
    if (bandMotionActive_) {
        const uint32_t active_count = (uint32_t)std::max(0.0f, std::min(polyphony, 6.0f));

        // Set per-band targets: bands activate fine → tectonic
        for (uint32_t i = 0; i < 6; i++) bandBlendTarget_[i] = 0.0f;
        for (uint32_t i = 0; i < active_count; i++) {
            bandBlendTarget_[BAND_ACTIVATION_ORDER[i]] = 1.0f;
        }

        bool changed = false;
        for (uint32_t i = 0; i < 6; i++) {
            const float prev = bandBlend_[i];
            const float target = bandBlendTarget_[i];

            // Capture phase origin at the moment a band activates
            if (target > 0.5f && prev < 0.01f) {
                bandPhaseOrigin_[i] = currentBeats_;
            }

            const float rate = (target > prev) ? BAND_BLEND_ATTACK : BAND_BLEND_RELEASE;
            Trajectory bb{ prev, 0.0f, 0.0f, 0.0f };
            bb = trajectory_release(bb, target, dt, rate);
            bandBlend_[i] = bb.value;

            // Snap to endpoints to avoid perpetual drift
            if (bandBlend_[i] < 0.001f && target == 0.0f) bandBlend_[i] = 0.0f;
            if (bandBlend_[i] > 0.999f && target == 1.0f) bandBlend_[i] = 1.0f;

            if (bandBlend_[i] != prev) changed = true;
        }

        if (changed) {
            gpuState_.set_band_motion(bandBlend_, bandPhaseOrigin_);
        }
        gpuState_.set_terrain_time(currentBeats_);
    }

    // ─── 2. Musical mode intensities (per-mode polyphony coupling) ─
    bool any_changed = false;
    for (uint32_t m = 0; m < MMODE_COUNT; m++) {
        // Skip mode 0 (terrain waves) — handled by band motion above
        // Skip mode 3 (palette drift) — has its own steeper curve below
        if (m == MMODE_TERRAIN_WAVES || m == MMODE_PALETTE_DRIFT) continue;

        const bool on = is_mmode_on(m);
        const float target = on ? std::min(polyphony / 6.0f, 1.0f) : 0.0f;
        const float prev = mmodeIntensity_[m];
        const float rate = (target > prev) ? MMODE_ATTACK : MMODE_RELEASE;
        Trajectory mi{ prev, 0.0f, 0.0f, 0.0f };
        mi = trajectory_release(mi, target, dt, rate);
        float next = mi.value;

        // Snap to endpoints
        if (next < 0.001f && target == 0.0f) next = 0.0f;
        if (next > 0.999f && target >= 1.0f) next = 1.0f;

        if (next != prev) {
            mmodeIntensity_[m] = next;
            any_changed = true;
        }
    }
    if (any_changed) {
        gpuState_.set_mode_color_shift(mmodeIntensity_[MMODE_COLOR_SHIFT] * 0.6f);
        gpuState_.set_mode_checker_scatter(mmodeIntensity_[MMODE_CHECKER_SCATTER] * 0.5f);

        // Aura expand: intensity scales aura parameters
        if (mmodeIntensity_[MMODE_AURA_EXPAND] > 0.0f || is_mmode_on(MMODE_AURA_EXPAND)) {
            auraCfgDirty_ = true;
        }

        // GoL tempo: more polyphony = slower zones + taller cells.
        // tick_scale > 1 = slower, height_scale > 1 = taller.
        {
            const float gi = mmodeIntensity_[MMODE_GOL_TEMPO];
            const float tick_scale   = 1.0f + gi * 3.0f;   // up to 4× slower
            const float height_scale = 1.0f + gi * 2.0f;   // up to 3× taller
            gpuState_.set_mode_gol_scales(tick_scale, height_scale);
        }
    }

    // ─── 3. Palette drift (target + intensity, both ramped) ───────
    {
        const bool drift_on = is_mmode_on(MMODE_PALETTE_DRIFT);

        // Smooth palette mapping: ordered by contrast from sand baseline.
        //   1 note → green(2), 2 notes → grey(3), 3+ notes → salmon(1).
        static constexpr float SMOOTH_PALETTE_MAP[] = { 0.0f, 2.0f, 3.0f, 1.0f };
        // Discrete tier mapping for drift mode.
        static constexpr float DISCRETE_TIER_MAP[]  = { 0.0f, 1.0f, 4.0f, 2.0f, 3.0f };

        if (drift_on && polyphony >= 1.0f) {
            const uint32_t idx = std::min((uint32_t)polyphony, 3u);
            paletteDriftDesired_ = SMOOTH_PALETTE_MAP[idx];
        }
        if (!drift_on || polyphony < 0.5f) {
            paletteDriftDesired_ = 0.0f;
        }

        // Ramp target smoothly (avoids color snaps)
        const float prev_t = paletteDriftTarget_;
        Trajectory pdt{ prev_t, 0.0f, 0.0f, 0.0f };
        pdt = trajectory_release(pdt, paletteDriftDesired_, dt, PALETTE_DRIFT_TARGET_RATE);
        paletteDriftTarget_ = pdt.value;

        // Intensity: poly/3 partial→full
        const float drift_intensity = drift_on ? std::min(polyphony / 3.0f, 1.0f) : 0.0f;
        const float prev_i = mmodeIntensity_[MMODE_PALETTE_DRIFT];
        const float rate_i = (drift_intensity > prev_i) ? MMODE_ATTACK : MMODE_RELEASE;
        Trajectory pdi{ prev_i, 0.0f, 0.0f, 0.0f };
        pdi = trajectory_release(pdi, drift_intensity, dt, rate_i);
        mmodeIntensity_[MMODE_PALETTE_DRIFT] = pdi.value;
        const float intensity = mmodeIntensity_[MMODE_PALETTE_DRIFT];

        float discrete_tier = 0.0f;
        if (drift_on && polyphony >= 1.0f) {
            const uint32_t tidx = std::min((uint32_t)polyphony, 4u);
            discrete_tier = DISCRETE_TIER_MAP[tidx];
        }

        if (intensity > 0.001f || paletteDriftTarget_ != prev_t) {
            gpuState_.set_mode_palette_drift(paletteDriftTarget_, intensity, discrete_tier);
        }
    }

    // ─── 4. Radial pulse onset detection (musical:K3 site) ────────
    {
        const bool pulse_on = is_mmode_on(MMODE_RADIAL_PULSE);

        if (pulse_on && polyphony > prevPolyphony_ + 0.5f) {
            const float increase = polyphony - std::max(prevPolyphony_, 0.0f);
            const uint32_t slot = pulseWriteIdx_ % PULSE_RING_SIZE;
            const uint32_t base = slot * 4;
            pulseRing_[base + 0] = pawnReadback_x_;
            pulseRing_[base + 1] = pawnReadback_z_;
            pulseRing_[base + 2] = currentSeconds_;
            pulseRing_[base + 3] = PULSE_AMPLITUDE * std::min(increase, 3.0f);
            pulseWriteIdx_++;
            std::cout << "[Pulse] ONSET slot=" << slot
                << " pos=(" << pawnReadback_x_ << "," << pawnReadback_z_ << ")"
                << " t=" << currentSeconds_
                << " amp=" << pulseRing_[base + 3]
                << " poly=" << polyphony << " prev=" << prevPolyphony_
                << "\n";
        }
        prevPolyphony_ = polyphony;

        // Count active (non-expired) pulses and upload
        uint32_t active = 0;
        for (uint32_t i = 0; i < PULSE_RING_SIZE; i++) {
            const float onset = pulseRing_[i * 4 + 2];
            const float amp   = pulseRing_[i * 4 + 3];
            if (amp > 0.001f && (currentSeconds_ - onset) < PULSE_MAX_AGE) {
                active = std::max(active, i + 1);
            }
        }
        gpuState_.set_pulse_data(active, pulseRing_);
    }
}

// ─── Per-mood-transition reset ────────────────────────────────────
//
// DONE[mood:K3] Per-mood-transition musical reset extracted from the
//   two duplicate sites (cartridge.hpp::teardown_world and
//   mood.inl::apply_mood) into this single named function. Both
//   callers now invoke reset_musical_couplings(queue). Mask stays
//   wired (toggles persist across world transitions), values reset.
void reset_musical_couplings(wgpu::Queue& queue) {
    for (uint32_t m = 0; m < MMODE_COUNT; m++) mmodeIntensity_[m] = 0.0f;
    paletteDriftTarget_  = 0.0f;
    paletteDriftDesired_ = 0.0f;
    gpuState_.set_mode_color_shift(0.0f);
    gpuState_.set_mode_checker_scatter(0.0f);
    gpuState_.set_mode_palette_drift(0.0f, 0.0f, 0.0f);
    gpuState_.set_mode_gol_scales(1.0f, 1.0f);
    for (int i = 0; i < 32; i++) pulseRing_[i] = 0.0f;
    pulseWriteIdx_ = 0;
    prevPolyphony_ = 0.0f;
    float zero_pulses[32] = {};
    gpuState_.set_pulse_data(0, zero_pulses);
}
