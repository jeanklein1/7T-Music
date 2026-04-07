// ─── pawn_aura.inl ──────────────────────────────────────────────
//
// Persistent terrain influence field centered on the pawn.
// Toroidal 64×64 grid of spring-driven cells that activate near
// the pawn and release when it moves away, leaving a decaying trail.
//
// Architecture follows the entity pattern:
//   PawnAuraProfile    — declarative parameter table
//   PawnAuraDeltaMode  — color differential strategy
//   GPUPawnAuraConfig  — per-frame GPU config (in state.hpp)
//   GPUPawnAuraCell    — per-cell state (in state.hpp)
//
// Included inside the Cartridge class body.
// Depends on: nothing (pure state + profile definitions)
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
