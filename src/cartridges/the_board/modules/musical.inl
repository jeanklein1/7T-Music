// ─── musical.inl ─────────────────────────────────────────────────
//
// The coupling layer: connects analysis signal to visual parameters.
// Mode definitions, intensity trajectories, band motion, palette
// drift, radial pulse, per-frame update, teardown reset.
//
// Included inside the Cartridge class body.
// Depends on: pawn_aura.inl (auraCfgDirty_)
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
    static const char* MODE_NAMES[] = { "terrain_waves", "color_shift", "checker_scatter", "palette_drift", "gol_tempo", "aura_expand", "UNUSED", "radial_pulse" };
    std::cout << "[MMode] " << MODE_NAMES[mode] << ": " << (on ? "ON" : "OFF") << "\n";
}

