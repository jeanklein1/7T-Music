// ─── orbs.inl ────────────────────────────────────────────────────
//
// Sky orb layer — luminous points living on a dome above the world.
// Bones pass: per-mood declarative config, one-shot GPU init at mood
// entry, per-frame drag+twinkle dynamics, billboard-quad render.
//
// Architecture:
//   OrbMoodConfig       — per-mood declarative parameters (in MoodProfile)
//   GPUOrbConfig        — per-frame GPU uniform (in state.hpp)
//   GPUOrbState         — per-orb state (in state.hpp)
//   configure_orbs      — apply_mood hook: uploads config, arms init
//   teardown_orbs       — teardown_world hook: disables dispatch
//   dispatch_orb_init   — one-shot seed-to-dome kernel
//   dispatch_orb_dynamics — per-frame drag + twinkle kernel
//   render_orbs         — billboard-quad draw into main render pass
//
// Included inside the Cartridge class body.
// Depends on: state.hpp (GPUOrbConfig/State, MAX_ORBS), renderer.hpp
// ─────────────────────────────────────────────────────────────────

// Dome geometry + bones-pass defaults. Tune here if the sky reads
// too small or too dim on first build.
static constexpr float ORB_DOME_RADIUS   = 450.0f;
static constexpr float ORB_DEFAULT_DRAG  = 0.5f;
static constexpr float ORB_DEFAULT_NOISE = 0.0f;
static constexpr float ORB_BASE_SIZE     = 3.0f;

// ─── Color palettes ──────────────────────────────────────────────
// Orbs sample from up to 4 HSV "pockets" at init. Each pocket has its
// own hue center + spread + saturation + selection weight, so a mood's
// sky can be mostly one color with rare accents (JWST depth feel).
static constexpr uint32_t MAX_ORB_PALETTE_ENTRIES = 4;

struct OrbPaletteEntry {
    float hue;         // HSV hue center (0..1)
    float hue_var;     // spread around center
    float saturation;  // base saturation
    float weight;      // selection probability (weights across entries
                       //  should sum ≈ 1.0 — the last entry catches the tail)
};

struct OrbPalette {
    uint32_t count;
    float    value_variance;   // per-orb HSV value (brightness) spread
    OrbPaletteEntry entries[MAX_ORB_PALETTE_ENTRIES];
};

// Stellar classification palette — diagnostic baseline with bold
// equally-weighted pockets so each color asserts itself. Once we know
// the floor / weight / saturation knobs behave, we'll dial back to a
// proper warm-dominant Deep Field design.
//   O/B  — hot blue
//   F/G  — yellow-white (sun-like)
//   K    — orange
//   M    — deep red
static constexpr OrbPalette ORB_PALETTE_JWST_DEEP = {
    4, 0.15f,
    {
        { 0.60f, 0.03f, 0.90f, 0.25f },  // hot blue
        { 0.12f, 0.04f, 0.60f, 0.25f },  // yellow-white
        { 0.07f, 0.04f, 0.85f, 0.25f },  // orange
        { 0.01f, 0.03f, 0.95f, 0.25f },  // deep red
    }
};

// Hubble SHO (Pillars of Creation): teal and gold with copper.
static constexpr OrbPalette ORB_PALETTE_PILLARS = {
    4, 0.30f,
    {
        { 0.10f, 0.03f, 0.65f, 0.40f },  // gold
        { 0.48f, 0.04f, 0.55f, 0.35f },  // teal
        { 0.02f, 0.02f, 0.70f, 0.15f },  // copper accent
        { 0.55f, 0.02f, 0.35f, 0.10f },  // pale blue
    }
};

// Carina Nebula: rich blues and amber-orange, no greens.
static constexpr OrbPalette ORB_PALETTE_CARINA = {
    3, 0.30f,
    {
        { 0.60f, 0.05f, 0.55f, 0.45f },  // rich blue
        { 0.08f, 0.04f, 0.60f, 0.40f },  // amber-orange
        { 0.55f, 0.03f, 0.30f, 0.15f },  // desaturated blue-white
        { 0.0f,  0.0f,  0.0f,  0.0f  },  // unused
    }
};

// Single warm (legacy-equivalent, for testing).
static constexpr OrbPalette ORB_PALETTE_WARM_MONO = {
    1, 0.20f,
    {
        { 0.08f, 0.06f, 0.60f, 1.0f },
        { 0.0f,  0.0f,  0.0f,  0.0f },
        { 0.0f,  0.0f,  0.0f,  0.0f },
        { 0.0f,  0.0f,  0.0f,  0.0f },
    }
};

// Palette registry — indexed by palette_id in mood config.
static constexpr uint32_t ORB_PAL_JWST_DEEP = 0;
static constexpr uint32_t ORB_PAL_PILLARS   = 1;
static constexpr uint32_t ORB_PAL_CARINA    = 2;
static constexpr uint32_t ORB_PAL_WARM_MONO = 3;
static constexpr uint32_t ORB_PAL_COUNT     = 4;

static constexpr OrbPalette ORB_PALETTES[ORB_PAL_COUNT] = {
    ORB_PALETTE_JWST_DEEP,
    ORB_PALETTE_PILLARS,
    ORB_PALETTE_CARINA,
    ORB_PALETTE_WARM_MONO,
};

static constexpr const char* ORB_PAL_NAMES[ORB_PAL_COUNT] = {
    "jwst_deep", "pillars", "carina", "warm_mono"
};

struct OrbMoodConfig {
    bool     enabled       = false;
    uint32_t count         = 0;        // clamped to Dim::MAX_ORBS
    float    base_hue      = 0.08f;    // legacy — used only if palette selection fails
    float    hue_variance  = 0.05f;    // legacy
    float    brightness    = 0.8f;     // global value center (palette spreads around it)
    float    drag          = ORB_DEFAULT_DRAG;
    float    noise_amp     = ORB_DEFAULT_NOISE;   // ceiling (floor is ORB_NOISE_FLOOR)
    uint32_t motion_rule   = 0;                    // 0=Brownian, 1=Orbital, 2=Frozen
    float    rotation_speed = 0.0f;                // rad/s
    float    rotation_axis[3] = {0.0f, 1.0f, 0.0f};// normalized in configure_orbs
    float    orbital_base_speed = 0.0f;            // rad/s (rule 1 only)
    uint32_t palette_id    = ORB_PAL_JWST_DEEP;    // index into ORB_PALETTES
    // ── Pass 5: color dynamics ────────────────────────────────
    bool  color_pulse_enabled    = false;
    bool  color_converge_enabled = false;
    bool  color_surge_enabled    = false;
    float hue_converge_target    = 0.12f;          // soft warm white-yellow
    // ── Pass 7: pawn-anchored dome (default on first run only) ─
    bool  anchor_to_pawn_default = false;
};

bool     orbsActive_           = false;
uint32_t orbCount_             = 0;
bool     orbInitPending_       = false;
bool     orbRecolorPending_    = false;              // set by cycle_orb_palette
uint32_t orbCurrentPaletteId_  = ORB_PAL_JWST_DEEP;  // cycled by the palette key

// ─── Pass 7: anchor state ────────────────────────────────────────
// Player-controlled toggle; persists across mood transitions. The
// mood's anchor_to_pawn_default seeds this only on first configure;
// thereafter the player's choice wins. Per-frame uploads use the
// dirty-flag pattern so an idle dome produces no queue traffic.
bool  orbPawnAnchored_          = false;
bool  orbAnchorInitialized_     = false;  // set on first configure_orbs
float orbLastDomeCenterX_       = 0.0f;
float orbLastDomeCenterZ_       = 0.0f;
bool  orbDomeCenterInitialized_ = false;

// ─── Orb musical coupling state ──────────────────────────────────
// Polyphony drives a radial force on the orbs AND lerps noise from a
// resting floor up to the mood's configured ceiling. Exponential ramps
// keep onset/release smooth. Attack is faster than release so the sky
// expands quickly but holds position after a phrase ends.
float orbForceIntensity_   = 0.0f;
float orbActiveNoiseAmp_   = 0.0f;  // mood's configured ceiling, captured at configure
static constexpr float ORB_FORCE_ATTACK  = 3.0f;   // 1/s
static constexpr float ORB_FORCE_RELEASE = 1.5f;   // 1/s
static constexpr float ORB_FORCE_SCALE   = 40.0f;  // world-units/s² at full intensity
static constexpr float ORB_NOISE_FLOOR   = 0.3f;   // barely perceptible drift in silence

// ─── Color dynamics coupling state ───────────────────────────────
// Three independent trajectories, each smoothed from polyphony and
// gated by its mood flag. Color couples faster than motion so onsets
// feel punchy; release is a touch quicker too so the sky settles
// back within a beat or two of silence.
float orbColorPulseIntensity_    = 0.0f;
float orbColorConvergeIntensity_ = 0.0f;
float orbColorSurgeIntensity_    = 0.0f;
bool  orbColorPulseActive_       = false;
bool  orbColorConvergeActive_    = false;
bool  orbColorSurgeActive_       = false;
static constexpr float ORB_COLOR_ATTACK  = 5.0f;   // 1/s
static constexpr float ORB_COLOR_RELEASE = 2.5f;   // 1/s

void configure_orbs(const OrbMoodConfig& cfg, wgpu::Queue& queue) {
    orbsActive_ = cfg.enabled;
    orbCount_ = std::min(cfg.count, (uint32_t)Dim::MAX_ORBS);

    if (orbsActive_ && orbCount_ > 0) {
        orbActiveNoiseAmp_ = cfg.noise_amp;  // capture ceiling for the coupling

        // Anchor: mood default applies ONLY on first run. After that,
        // the player's toggle persists across mood transitions.
        if (!orbAnchorInitialized_) {
            orbPawnAnchored_ = cfg.anchor_to_pawn_default;
            orbAnchorInitialized_ = true;
        }

        // Normalize rotation axis on CPU — GPU still renormalizes but
        // keeping the uploaded value unit-length avoids surprises.
        float rx = cfg.rotation_axis[0];
        float ry = cfg.rotation_axis[1];
        float rz = cfg.rotation_axis[2];
        float rlen = std::sqrt(rx * rx + ry * ry + rz * rz);
        if (rlen > 0.001f) { rx /= rlen; ry /= rlen; rz /= rlen; }
        else               { rx = 0.0f; ry = 1.0f; rz = 0.0f; }

        GPUOrbConfig gpuCfg{};
        gpuCfg.count              = orbCount_;
        gpuCfg.seed               = activeSeed_;
        gpuCfg.base_hue           = cfg.base_hue;
        gpuCfg.hue_variance       = cfg.hue_variance;
        gpuCfg.brightness         = cfg.brightness;
        gpuCfg.drag               = cfg.drag;
        gpuCfg.noise_amp          = ORB_NOISE_FLOOR;   // start at floor, coupling lerps up
        gpuCfg.dome_radius        = ORB_DOME_RADIUS;
        gpuCfg.base_size          = ORB_BASE_SIZE;
        gpuCfg.dt                 = 0.0f;
        gpuCfg.t_seconds          = 0.0f;
        gpuCfg.force_radial       = 0.0f;
        gpuCfg.motion_rule        = cfg.motion_rule;
        gpuCfg.rotation_speed     = cfg.rotation_speed;
        gpuCfg.rotation_axis_x    = rx;
        gpuCfg.rotation_axis_y    = ry;
        gpuCfg.rotation_axis_z    = rz;
        gpuCfg.orbital_base_speed = cfg.orbital_base_speed;

        // Palette lookup and field-by-field upload.
        uint32_t pal_id = std::min(cfg.palette_id, ORB_PAL_COUNT - 1u);
        orbCurrentPaletteId_ = pal_id;  // mood wins on entry; O cycles within the mood
        const auto& pal = ORB_PALETTES[pal_id];
        gpuCfg.palette_count  = pal.count;
        gpuCfg.value_variance = pal.value_variance;
        gpuCfg.pal0_hue       = pal.entries[0].hue;
        gpuCfg.pal0_hue_var   = pal.entries[0].hue_var;
        gpuCfg.pal0_sat       = pal.entries[0].saturation;
        gpuCfg.pal0_weight    = pal.entries[0].weight;
        gpuCfg.pal1_hue       = pal.entries[1].hue;
        gpuCfg.pal1_hue_var   = pal.entries[1].hue_var;
        gpuCfg.pal1_sat       = pal.entries[1].saturation;
        gpuCfg.pal1_weight    = pal.entries[1].weight;
        gpuCfg.pal2_hue       = pal.entries[2].hue;
        gpuCfg.pal2_hue_var   = pal.entries[2].hue_var;
        gpuCfg.pal2_sat       = pal.entries[2].saturation;
        gpuCfg.pal2_weight    = pal.entries[2].weight;
        gpuCfg.pal3_hue       = pal.entries[3].hue;
        gpuCfg.pal3_hue_var   = pal.entries[3].hue_var;
        gpuCfg.pal3_sat       = pal.entries[3].saturation;
        gpuCfg.pal3_weight    = pal.entries[3].weight;

        // Color dynamics: start at rest (coupling lifts them on music).
        // hue_converge_target changes only at mood entry, so it goes here.
        gpuCfg.color_pulse         = 0.0f;
        gpuCfg.color_converge      = 0.0f;
        gpuCfg.color_surge         = 0.0f;
        gpuCfg.hue_converge_target = cfg.hue_converge_target;

        // Dome center: start at origin even when anchored. The per-frame
        // update_orb_anchor catches up on the next frame with fresh
        // pawnReadback values, which may not be ready at mood-entry time.
        gpuCfg.dome_center_x = 0.0f;
        gpuCfg.dome_center_y = 0.0f;
        gpuCfg.dome_center_z = 0.0f;
        gpuCfg._pad_anchor   = 0.0f;
        // Force dirty-flag re-eval so the next frame uploads even if
        // the cached last-center matches this zero state.
        orbDomeCenterInitialized_ = false;

        orbColorPulseActive_    = cfg.color_pulse_enabled;
        orbColorConvergeActive_ = cfg.color_converge_enabled;
        orbColorSurgeActive_    = cfg.color_surge_enabled;

        gpuState_.upload_orb_config(queue, gpuCfg);
        orbInitPending_ = true;

        static const char* RULE_NAMES[] = { "brownian", "orbital", "frozen" };
        std::cout << "[Orbs] Configured: count=" << orbCount_
            << " palette=" << ORB_PAL_NAMES[pal_id]
            << " drag=" << cfg.drag
            << " noise=" << ORB_NOISE_FLOOR << ".." << orbActiveNoiseAmp_
            << " rule=" << RULE_NAMES[std::min(cfg.motion_rule, 2u)]
            << " rot=" << cfg.rotation_speed
            << " orbital=" << cfg.orbital_base_speed
            << " color:"
            << (cfg.color_pulse_enabled    ? " pulse" : "")
            << (cfg.color_converge_enabled ? " converge" : "")
            << (cfg.color_surge_enabled    ? " surge" : "")
            << (cfg.color_pulse_enabled || cfg.color_converge_enabled || cfg.color_surge_enabled
                ? "" : " off")
            << " anchor=" << (orbPawnAnchored_ ? "pawn" : "origin")
            << "\n";
    }
}

void teardown_orbs() {
    orbsActive_ = false;
    orbCount_ = 0;
    orbInitPending_ = false;
    orbRecolorPending_ = false;
    orbForceIntensity_ = 0.0f;
    orbActiveNoiseAmp_ = 0.0f;

    orbColorPulseIntensity_    = 0.0f;
    orbColorConvergeIntensity_ = 0.0f;
    orbColorSurgeIntensity_    = 0.0f;
    orbColorPulseActive_       = false;
    orbColorConvergeActive_    = false;
    orbColorSurgeActive_       = false;

    // Per-frame dome-center cache clears (forces a fresh upload on the
    // next configure). NOTE: do NOT touch orbPawnAnchored_ or
    // orbAnchorInitialized_ — anchor is player state and persists
    // across moods.
    orbLastDomeCenterX_ = 0.0f;
    orbLastDomeCenterZ_ = 0.0f;
    orbDomeCenterInitialized_ = false;
}

// Flip the dome anchor between world origin and the pawn. Player
// state: persists across mood transitions.
void toggle_orb_anchor() {
    orbPawnAnchored_ = !orbPawnAnchored_;
    std::cout << "[Orbs] Anchor: "
        << (orbPawnAnchored_ ? "ON — dome follows pawn"
                             : "OFF — dome fixed at world origin")
        << "  (pawn readback: "
        << pawnReadback_x_ << ", " << pawnReadback_z_ << ")"
        << "\n";
}

// Push the current dome center to the GPU. Dirty-flagged so a stationary
// anchored pawn or an unanchored session produces no per-frame traffic.
// Horizontal-only: Y is always 0 regardless of pawn altitude so the sky
// doesn't bob with terrain.
void update_orb_anchor(float pawn_x, float pawn_z, wgpu::Queue& queue) {
    if (!orbsActive_ || orbCount_ == 0) return;

    float target_x = orbPawnAnchored_ ? pawn_x : 0.0f;
    float target_z = orbPawnAnchored_ ? pawn_z : 0.0f;

    bool changed = !orbDomeCenterInitialized_
                || target_x != orbLastDomeCenterX_
                || target_z != orbLastDomeCenterZ_;

    if (changed) {
        gpuState_.upload_orb_dome_center(queue, target_x, 0.0f, target_z);
        orbLastDomeCenterX_ = target_x;
        orbLastDomeCenterZ_ = target_z;
        orbDomeCenterInitialized_ = true;

        // Diagnostic: print on every change so we can verify the upload
        // fires and see the magnitude. Once visual confirms behavior, we
        // can drop this to a single "first upload after toggle" log.
        std::cout << "[Orbs] Dome center -> ("
            << target_x << ", 0, " << target_z
            << ") anchored=" << (orbPawnAnchored_ ? "yes" : "no") << "\n";
    }
}

// Cycle forward through the palette registry. Session-local within a
// mood — the next mood transition resets to that mood's configured
// palette. Re-arms the init kernel so positions and colors both
// refresh visibly on the next frame.
void cycle_orb_palette(wgpu::Queue& queue) {
    if (!orbsActive_ || orbCount_ == 0) {
        std::cout << "[Orbs] Palette cycle ignored (no active dome)\n";
        return;
    }

    orbCurrentPaletteId_ = (orbCurrentPaletteId_ + 1u) % ORB_PAL_COUNT;
    const auto& pal = ORB_PALETTES[orbCurrentPaletteId_];

    float pal_data[16];
    for (uint32_t i = 0; i < 4; i++) {
        pal_data[i * 4 + 0] = pal.entries[i].hue;
        pal_data[i * 4 + 1] = pal.entries[i].hue_var;
        pal_data[i * 4 + 2] = pal.entries[i].saturation;
        pal_data[i * 4 + 3] = pal.entries[i].weight;
    }
    gpuState_.upload_orb_palette(queue, pal.count, pal.value_variance, pal_data);

    // Color-only refresh: positions, velocities, twinkle phase all
    // persist so the sky "holds" and only the hues transition.
    orbRecolorPending_ = true;

    std::cout << "[Orbs] Palette: " << ORB_PAL_NAMES[orbCurrentPaletteId_] << "\n";
}

// Polyphony → motion (force + noise) and color (pulse / converge /
// surge). Each coupling is structurally independent — same source
// for now so they're synced; swapping sources later desyncs.
// Uploads only when a value actually changes.
void update_orb_coupling(float polyphony, float dt, wgpu::Queue& queue) {
    if (!orbsActive_ || orbCount_ == 0) return;

    float target = std::min(polyphony / 6.0f, 1.0f);

    // Local smoother — returns true iff value actually moved.
    auto smooth = [&](bool enabled, float& intensity,
                      float attack, float release) -> bool {
        float t = enabled ? target : 0.0f;
        float prev = intensity;
        float rate = (t > prev) ? attack : release;
        float next = prev + (t - prev) * (1.0f - std::exp(-rate * dt));
        if (next < 0.001f && t == 0.0f) next = 0.0f;
        if (next > 0.999f && t >= 1.0f) next = 1.0f;
        if (next != prev) { intensity = next; return true; }
        return false;
    };

    // ─── Motion couplings ────────────────────────────────────
    if (smooth(true, orbForceIntensity_, ORB_FORCE_ATTACK, ORB_FORCE_RELEASE)) {
        float radial = orbForceIntensity_ * ORB_FORCE_SCALE;
        gpuState_.upload_orb_force(queue, radial);

        float noise = ORB_NOISE_FLOOR
            + orbForceIntensity_ * (orbActiveNoiseAmp_ - ORB_NOISE_FLOOR);
        gpuState_.upload_orb_noise(queue, noise);
    }

    // ─── Color couplings ─────────────────────────────────────
    bool color_changed = false;
    if (smooth(orbColorPulseActive_,    orbColorPulseIntensity_,
               ORB_COLOR_ATTACK, ORB_COLOR_RELEASE))    color_changed = true;
    if (smooth(orbColorConvergeActive_, orbColorConvergeIntensity_,
               ORB_COLOR_ATTACK, ORB_COLOR_RELEASE))    color_changed = true;
    if (smooth(orbColorSurgeActive_,    orbColorSurgeIntensity_,
               ORB_COLOR_ATTACK, ORB_COLOR_RELEASE))    color_changed = true;

    if (color_changed) {
        gpuState_.upload_orb_color_dynamics(queue,
            orbColorPulseIntensity_,
            orbColorConvergeIntensity_,
            orbColorSurgeIntensity_);
    }
}

void dispatch_orb_init(wgpu::CommandEncoder& encoder) {
    if (!orbInitPending_) return;
    orbInitPending_ = false;

    wgpu::ComputePassDescriptor cpd{};
    cpd.label = "Orb Init";
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
    uint32_t wgs = (orbCount_ + 63u) / 64u;
    renderer_.dispatch_orb_init(pass,
        gpuState_.orb_compute_group(), wgs);
    pass.End();

    std::cout << "[Orbs] Init dispatched: " << orbCount_
        << " orbs, " << wgs << " workgroups\n";
}

void dispatch_orb_recolor(wgpu::CommandEncoder& encoder) {
    if (!orbRecolorPending_) return;
    orbRecolorPending_ = false;

    wgpu::ComputePassDescriptor cpd{};
    cpd.label = "Orb Recolor";
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
    uint32_t wgs = (orbCount_ + 63u) / 64u;
    renderer_.dispatch_orb_recolor(pass,
        gpuState_.orb_compute_group(), wgs);
    pass.End();
}

void dispatch_orb_dynamics(wgpu::CommandEncoder& encoder,
                           wgpu::Queue& queue) {
    if (!orbsActive_ || orbCount_ == 0) return;

    gpuState_.upload_orb_frame(queue, currentDt_, currentSeconds_);

    wgpu::ComputePassDescriptor cpd{};
    cpd.label = "Orb Dynamics";
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
    uint32_t wgs = (orbCount_ + 63u) / 64u;
    renderer_.dispatch_orb_dynamics(pass,
        gpuState_.orb_compute_group(), wgs);
    pass.End();
}

void render_orbs(wgpu::RenderPassEncoder& pass) {
    if (!orbsActive_ || orbCount_ == 0) return;
    renderer_.draw_orbs(pass,
        gpuState_.render_entity_group(),
        gpuState_.render_texture_group(),
        gpuState_.orb_quad_vb(),
        gpuState_.orb_quad_ib(),
        orbCount_);
}
