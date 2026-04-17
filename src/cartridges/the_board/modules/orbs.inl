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
static constexpr float ORB_DOME_RADIUS   = 800.0f;
static constexpr float ORB_DEFAULT_DRAG  = 0.5f;
static constexpr float ORB_DEFAULT_NOISE = 0.0f;
static constexpr float ORB_BASE_SIZE     = 3.0f;

struct OrbMoodConfig {
    bool     enabled       = false;
    uint32_t count         = 0;        // clamped to Dim::MAX_ORBS
    float    base_hue      = 0.08f;    // warm amber default
    float    hue_variance  = 0.05f;
    float    brightness    = 0.8f;
    float    drag          = ORB_DEFAULT_DRAG;
    float    noise_amp     = ORB_DEFAULT_NOISE;
};

bool     orbsActive_     = false;
uint32_t orbCount_       = 0;
bool     orbInitPending_ = false;

void configure_orbs(const OrbMoodConfig& cfg, wgpu::Queue& queue) {
    orbsActive_ = cfg.enabled;
    orbCount_ = std::min(cfg.count, (uint32_t)Dim::MAX_ORBS);

    if (orbsActive_ && orbCount_ > 0) {
        GPUOrbConfig gpuCfg{};
        gpuCfg.count        = orbCount_;
        gpuCfg.seed         = activeSeed_;
        gpuCfg.base_hue     = cfg.base_hue;
        gpuCfg.hue_variance = cfg.hue_variance;
        gpuCfg.brightness   = cfg.brightness;
        gpuCfg.drag         = cfg.drag;
        gpuCfg.noise_amp    = cfg.noise_amp;
        gpuCfg.dome_radius  = ORB_DOME_RADIUS;
        gpuCfg.base_size    = ORB_BASE_SIZE;
        gpuCfg.dt           = 0.0f;
        gpuCfg.t_seconds    = 0.0f;
        gpuCfg._pad         = 0.0f;
        gpuState_.upload_orb_config(queue, gpuCfg);
        orbInitPending_ = true;

        std::cout << "[Orbs] Configured: count=" << orbCount_
            << " hue=" << cfg.base_hue
            << " var=" << cfg.hue_variance
            << " drag=" << cfg.drag << "\n";
    }
}

void teardown_orbs() {
    orbsActive_ = false;
    orbCount_ = 0;
    orbInitPending_ = false;
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
