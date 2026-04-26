// ─── floaters.inl ────────────────────────────────────────────────
//
// Cube behavior registry and coordination knob. Spheres do their own
// thing (analytical PGA orbit, no behavior layer); this module is
// cube-only despite the "floaters" name — the file is named for the
// system the cubes live in (the floating-entity buffer they share
// with spheres) rather than for who consumes the registry.
//
// Phase 3 ships three behaviors on top of the drift-integrator
// substrate (Phase 1) and pawn-distance lifecycle (Phase 2):
//
//   STATIONARY  identical to pre-Phase-3 hover-bob; default for
//               every spawn so day-zero visuals are unchanged
//   CURLFIELD   organic XZ drift sampled from a 2D curl-noise field
//               (no neighbor lookups; emergent flock-like coherence
//               via shared field samples at low coordination spread)
//   PHASEWAVE   vertical sinusoid whose phase is a function of XZ
//               rest position; a traveling wavefront drone-show at
//               high coordination, uncorrelated bobbing at low
//
// The COORDINATION dial in [0,1] is the system-wide synchrony knob.
// At 0, behaviors run with maximum individual variation (independent
// phases, high spatial frequency). At 1, behaviors lock to shared
// parameters (synchronized phases, low spatial frequency). The lerp
// happens inside each behavior's force function in world.wgsl —
// cycling coordination from 0 to 1 transitions a cube population
// smoothly from "scattered drifters" to "drone show."
//
// ┌─── Public surface (called from outside this file) ─────────────┐
// │                                                                 │
// │  Player commands (function keys; chosen to avoid the A-Z piano  │
// │                   range the analysis layer consumes):           │
// │    cycle_cube_behavior_override(queue)   F4  next behavior      │
// │    cycle_floater_coordination()          F5  step coordination  │
// │    corral_cubes(queue)                   F6  ring around pawn   │
// │                                                                 │
// │  Per-frame upload:                                              │
// │    The coordination value lives in GPUDesignConfig and uploads  │
// │    via the existing config-write path; no separate dispatch.    │
// │                                                                 │
// └─────────────────────────────────────────────────────────────────┘
//
// Included inside the Cartridge class body.
// Depends on: state.hpp (GPUFloatingEntityState behavior_id field,
//             GPUDesignConfig::floater_coordination), entity_pipeline.inl
//             (cube_write_gpu seeds behavior_phase at spawn).
// ─────────────────────────────────────────────────────────────────


// ═══ Behavior IDs ═════════════════════════════════════════════════
//
// Mirror the WGSL constants in world.wgsl (search:
// "Cube behavior registry"). Adding a behavior means: a new id here,
// a new const in WGSL, a new force function, a new switch case, a
// new entry in CUBE_BEHAVIOR_NAMES.

static constexpr uint32_t CUBE_BEHAVIOR_STATIONARY = 0;
static constexpr uint32_t CUBE_BEHAVIOR_CURLFIELD  = 1;
static constexpr uint32_t CUBE_BEHAVIOR_PHASEWAVE  = 2;
static constexpr uint32_t CUBE_BEHAVIOR_COUNT      = 3;

static constexpr const char* CUBE_BEHAVIOR_NAMES[CUBE_BEHAVIOR_COUNT] = {
    "stationary", "curlfield", "phasewave"
};


// ═══ Coordination ═════════════════════════════════════════════════
//
// System-wide synchrony knob in [0,1]. Lives in GPUDesignConfig as
// floater_coordination; uploaded each frame through the existing
// config-write path. We expose three discrete steps via the cycling
// helper (0.0 / 0.5 / 1.0) for quick visual inspection; smooth
// musical-input-driven values are a Phase 4 concern.

static constexpr float FLOATER_COORDINATION_STEPS[3] = { 0.0f, 0.5f, 1.0f };
uint32_t floaterCoordinationStep_ = 0;  // index into the steps array

void cycle_floater_coordination() {
    floaterCoordinationStep_ = (floaterCoordinationStep_ + 1) % 3;
    float v = FLOATER_COORDINATION_STEPS[floaterCoordinationStep_];
    gpuState_.config().floater_coordination = v;
    std::cout << "[Floaters] coordination: " << v << "\n";
}


// ═══ Behavior override ════════════════════════════════════════════
//
// Walk every active cube slot and rewrite behavior_id. Mirrors the
// agent-system override pattern: no AGENT_OVERRIDE_NONE sentinel
// because we don't have a population-driven default to revert to
// (Phase 3 spawns every cube as STATIONARY); the cycle just steps
// 0 → 1 → 2 → 0. Cubes spawned mid-cycle pick up whichever value
// is current in the override slot via apply_cube_behavior_override.

uint32_t cubeBehaviorOverride_ = CUBE_BEHAVIOR_STATIONARY;

// Walk every active cube slot, set behavior_id to the current override,
// and re-upload the changed slots. Called by cycle_cube_behavior_override.
void apply_cube_behavior_override(wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        if (!activeCubes_[i].active) continue;
        gpuState_.upload_cube_behavior_id(queue, i, cubeBehaviorOverride_);
    }
}

void cycle_cube_behavior_override(wgpu::Queue& queue) {
    cubeBehaviorOverride_ = (cubeBehaviorOverride_ + 1) % CUBE_BEHAVIOR_COUNT;
    apply_cube_behavior_override(queue);
    std::cout << "[Floaters] cube behavior: "
              << CUBE_BEHAVIOR_NAMES[cubeBehaviorOverride_] << "\n";
}


// ═══ Corral ═══════════════════════════════════════════════════════
//
// Diagnostic: smoothly gather every active cube into a small ring
// around the pawn so behaviors can be inspected without waiting on
// natural spawn distribution. Animates the anchor field over a few
// seconds — instant teleport (initial implementation) yanked the
// drift integrator hard and looked violent, since a sudden anchor
// jump means home jumps means drift suddenly equals (pos - new_home),
// and the spring snaps the cube fast.
//
// Implementation: when corral is invoked, capture each active cube's
// current anchor and target ring position into cubeCorralAnim_, set
// duration. tick_cube_corral_animations() runs each frame from the
// main update path, eases anchor between from→to over duration, and
// writes the interpolated anchor to GPU. Animations expire when
// done; subsequent corral presses re-arm them with new from/to.
//
// One-shot, not stateful: corral does not "follow" the pawn. Once
// the animation finishes, anchor stays where it landed. The cube
// drifts/evicts from there by normal lifecycle.
//
// Cube-only — spheres orbit and would look bizarre clustered.

static constexpr float CUBE_CORRAL_RADIUS   = 30.0f;  // ring radius around pawn
static constexpr float CUBE_CORRAL_DURATION = 2.5f;   // seconds, full glide

struct CubeCorralAnim {
    bool   active;            // animation in progress this slot?
    float  t0;                // currentSeconds_ at start
    float  duration;          // seconds (from CUBE_CORRAL_DURATION)
    float  ax_from, az_from;  // captured anchor at start
    float  ax_to,   az_to;    // target ring position
};
CubeCorralAnim cubeCorralAnim_[Dim::MAX_CUBE_INSTANCES] = {};

// Smooth-step easing, [0,1] → [0,1], C¹ continuous at the endpoints.
// Cheap and looks much better than linear for this kind of glide.
static float corral_ease_(float t) {
    return t * t * (3.0f - 2.0f * t);
}

void corral_cubes(wgpu::Queue& queue) {
    // Pawn's current XZ. cpuAgents_[0] is the player's slot, kept
    // current by the per-frame agent readback.
    const float px = cpuAgents_[0].pos_x;
    const float pz = cpuAgents_[0].pos_z;

    // First pass: count active cubes for stable angular distribution.
    uint32_t active_count = 0;
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        if (activeCubes_[i].active) active_count++;
    }
    if (active_count == 0) {
        std::cout << "[Floaters] corral: no active cubes\n";
        return;
    }

    // Second pass: arm the animation for each active cube. Use a
    // stable index so successive corral presses don't shuffle the
    // ring layout.
    uint32_t armed = 0;
    uint32_t k = 0;
    const float two_pi = 6.28318530718f;
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        if (!activeCubes_[i].active) continue;
        float theta = (float(k) / float(active_count)) * two_pi;
        float ax_to = px + std::cos(theta) * CUBE_CORRAL_RADIUS;
        float az_to = pz + std::sin(theta) * CUBE_CORRAL_RADIUS;

        // Capture starting anchor from CPU mirror — activeCubes_[i].cx/cz
        // is the spawn-time anchor. If a previous corral animation is
        // already running, capture its current interpolated value
        // instead, so re-pressing F6 mid-glide doesn't snap.
        float ax_from, az_from;
        if (cubeCorralAnim_[i].active) {
            // Already animating; capture interpolated current value.
            float t_norm = (currentSeconds_ - cubeCorralAnim_[i].t0)
                           / cubeCorralAnim_[i].duration;
            t_norm = std::clamp(t_norm, 0.0f, 1.0f);
            float eased = corral_ease_(t_norm);
            ax_from = cubeCorralAnim_[i].ax_from
                    + (cubeCorralAnim_[i].ax_to - cubeCorralAnim_[i].ax_from) * eased;
            az_from = cubeCorralAnim_[i].az_from
                    + (cubeCorralAnim_[i].az_to - cubeCorralAnim_[i].az_from) * eased;
        } else {
            ax_from = activeCubes_[i].cx;
            az_from = activeCubes_[i].cz;
        }

        cubeCorralAnim_[i] = {
            /*active=*/   true,
            /*t0=*/       currentSeconds_,
            /*duration=*/ CUBE_CORRAL_DURATION,
            ax_from, az_from,
            ax_to,   az_to,
        };
        // Update CPU mirror so the *next* corral starts from the
        // right place if pressed before this one finishes (handled
        // via the if-active branch above), and so the activeCubes_
        // mirror stays the source of truth for "where this cube is."
        activeCubes_[i].cx = ax_to;
        activeCubes_[i].cz = az_to;
        armed++;
        k++;
    }

    std::cout << "[Floaters] corral: " << armed << " cube(s) gliding to ring around ("
              << std::fixed << std::setprecision(1) << px << "," << pz
              << ") radius " << CUBE_CORRAL_RADIUS
              << " over " << CUBE_CORRAL_DURATION << "s\n";
}

// Per-frame: advance any active animations and push interpolated
// anchor to GPU. Skips cleanly when nothing is animating (the common
// case — animations only run for ~3s after F6 is pressed).
void tick_cube_corral_animations(wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        auto& anim = cubeCorralAnim_[i];
        if (!anim.active) continue;
        // If the slot was evicted mid-animation, just disarm.
        if (!activeCubes_[i].active) { anim.active = false; continue; }

        float t_norm = (currentSeconds_ - anim.t0) / anim.duration;
        if (t_norm >= 1.0f) {
            // Final write at the exact target, then disarm.
            gpuState_.upload_cube_anchor(queue, i, anim.ax_to, 0.0f, anim.az_to);
            anim.active = false;
            continue;
        }
        if (t_norm < 0.0f) t_norm = 0.0f;
        float eased = corral_ease_(t_norm);
        float ax = anim.ax_from + (anim.ax_to - anim.ax_from) * eased;
        float az = anim.az_from + (anim.az_to - anim.az_from) * eased;
        gpuState_.upload_cube_anchor(queue, i, ax, 0.0f, az);
    }
}
