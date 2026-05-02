// ─── pawn.inl ────────────────────────────────────────────────────
//
// Player-relative state: aura field (toroidal 64×64 spring grid that
// activates near the pawn and releases when it moves away),
// presence trajectory, per-frame coupling tick.
//
// Phase 4.3 extraction (closes pawn:K1): folds the previous
// pawn_aura.inl contents and adds tick_pawn_couplings(queue) for
// the per-frame ramps that used to live in cartridge.hpp::update().
//
// Architecture:
//   PawnAuraProfile    — declarative parameter table
//   PawnAuraDeltaMode  — color differential strategy
//   GPUPawnAuraConfig  — per-frame GPU config (in state.hpp)
//   GPUPawnAuraCell    — per-cell state (in state.hpp)
//   auraPresence_      — Trajectory-style 0→1 ramp on toggle
//
// Included inside the Cartridge class body.
// Depends on: trajectory.inl (release primitive), musical.inl
//             (mmodeIntensity_[MMODE_AURA_EXPAND] for the expand
//             coupling).
// ─────────────────────────────────────────────────────────────────

struct PawnAuraDeltaMode {
    static constexpr uint32_t CONVERGENT = 0;  // all cells shift toward signature tint
    static constexpr uint32_t RANDOM = 1;  // each cell gets unique random delta
};

struct PawnAuraProfile {
    float influence_radius;
    float attack_stiffness;
    float attack_damping;
    float release_rate;
    float tint_strength;
    float tint_r, tint_g, tint_b;
    uint32_t delta_mode;
    float delta_magnitude;     // random mode: max offset per channel
    uint32_t effect_mask;      // bit 0=color, bit 1=height
    float height_scale;        // height extrusion in world units
};

static constexpr PawnAuraProfile PAWN_AURA_DEFAULT = {
    20.0f,             // influence_radius
    12.0f,             // attack_stiffness
    0.7f,              // attack_damping
    1.5f,              // release_rate
    0.5f,              // tint_strength
    0.4f, 0.2f, 0.5f, // tint RGB (purple)
    PawnAuraDeltaMode::CONVERGENT,
    0.3f,              // delta_magnitude (used in random mode)
    0x3u,              // effect_mask: color tint + height
    3.0f,              // height_scale
};

// Active profile — starts as default, can be swapped by landmarks/commands
PawnAuraProfile activeAuraProfile_ = PAWN_AURA_DEFAULT;
bool auraHeightEnabled_ = true;
bool auraEnabled_ = false;         // default off — numpad 3 toggles
bool auraNeedsClear_ = false;
bool auraCfgDirty_ = true;     // true at boot → first frame uploads full config

// Smooth raise/lower: auraPresence_ ramps 0→1 on enable, 1→0 on disable.
// Scales all aura parameters so terrain and pawn height change gradually.
float auraPresence_ = 0.0f;        // current [0,1] — trajectory value
static constexpr float AURA_PRESENCE_ATTACK = 1.0f;   // 1/s — ~3s to full (spring converges in ~0.5s)
static constexpr float AURA_PRESENCE_RELEASE = 1.5f;  // 1/s — ~2s to zero


// ─── Per-frame pawn coupling tick ────────────────────────────────
//
// DONE[pawn:K1] aura presence ramp + height computation moved out of
//   cartridge.hpp::update() into this single named tick. The presence
//   value uses trajectory_release (mirroring WGSL §1.2). The height
//   computation reads mmodeIntensity_[MMODE_AURA_EXPAND] from
//   musical.inl — kept the cross-module read explicit since the
//   coupling is genuinely musical→aura, not pawn-internal.
//
// Caller: cartridge.hpp::update() — runs once per frame after the
// signal is built and before world bounds are uploaded.
void tick_pawn_couplings(wgpu::Queue& queue) {
    // Aura presence ramp: smooth 0→1 on enable / 1→0 on disable.
    {
        const float target = auraEnabled_ ? 1.0f : 0.0f;
        const float prev   = auraPresence_;
        const float rate   = (target > prev) ? AURA_PRESENCE_ATTACK : AURA_PRESENCE_RELEASE;
        Trajectory ap{ prev, 0.0f, 0.0f, 0.0f };
        ap = trajectory_release(ap, target, currentDt_, rate);
        auraPresence_ = ap.value;

        // Snap to endpoints to avoid perpetual drift
        if (auraPresence_ < 0.001f && target == 0.0f) auraPresence_ = 0.0f;
        if (auraPresence_ > 0.999f && target == 1.0f) auraPresence_ = 1.0f;
        if (auraPresence_ != prev) auraCfgDirty_ = true;
    }

    // Pawn aura height: presence × base height × musical-expand multiplier.
    // Same value is consumed by terrain VS for extrusion, so pawn and
    // terrain always agree.
    const float aura_expand_mult = 1.0f + mmodeIntensity_[MMODE_AURA_EXPAND] * 3.0f;
    const float effective_aura_height = auraHeightEnabled_
        ? activeAuraProfile_.height_scale * auraPresence_ * aura_expand_mult
        : 0.0f;
    gpuState_.set_pawn_aura_height(effective_aura_height);
    // Keep compute running while ramping down (so the trail decays cleanly).
    gpuState_.set_aura_enabled(auraPresence_ > 0.001f);
}
