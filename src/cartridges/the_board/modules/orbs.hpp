#pragma once
#include <cstdint>
#include "cartridges/the_board/modules/mood_constants.hpp"   // MOOD_COUNT (sizes ORB_MOOD_TABLE)

// ─── orbs.hpp (HEADER: console + registries + state + decls) ─────
//
// Sky orb layer — luminous points on a dome above the world. A fixed
// population of billboarded quads sampled from a dome of radius
// ORB_DOME_RADIUS, driven by CPU-authored mood config, updated by a
// compute kernel, rendered additively.
//
// CONVERTED (LADDER-2 c3, header/impl split): this header owns the tuning
// console, the registries (palettes / tier sets / gestures), OrbMoodConfig
// + ORB_MOOD_TABLE, OrbsState, and the function DECLARATIONS; the
// cartridge declares the instance (orbs_state_) at its COMPOSITION ROOT.
// The DEFINITIONS dereference the complete Cartridge (the keyhole:
// c->gpuState_, c->renderer_, c->world_state_, c->player_, c->time_state_),
// so they live in orbs.inl, included at FILE SCOPE in the post-class
// MODULE IMPLEMENTATIONS zone (the proven self-wrapping form). Namespace
// t7::the_board. Move, don't improve: the open ORB-1 ruling (anchor
// semantics) governs update_orb_anchor / toggle_orb_anchor — both moved
// verbatim.
//
// The control surfaces (tuning console, palette registry, tier sets,
// flocking gestures, ORB_MOOD_TABLE below) are intended to be read as
// matrices: one row per item, aligned columns, minimum narrative. All
// runtime levers (cycle keys, toggles) are exposed as named functions
// with a single-line purpose comment.
//
// ┌─── Public surface (called from outside this module) ───────────┐
// │                                                                 │
// │  Module functions take OrbsState& explicitly                    │
// │  (or const OrbsState& when read-only).                          │
// │                                                                 │
// │  Lifecycle:                                                     │
// │    configure_orbs(os, c, cfg, queue) — mood entry: upload + arm │
// │    teardown_orbs(os, c)              — mood exit: disable       │
// │                                                                 │
// │  Player commands:                                               │
// │    cycle_orb_palette(os, c, queue)     — 0     next palette     │
// │    cycle_orb_motion_rule(os, c, queue) — KP_8  next rule        │
// │    cycle_orb_gesture(os, c, queue)     — KP_DECIMAL next gest.  │
// │    toggle_orb_anchor(os, c)            — KP_9  world↔pawn       │
// │                                                                 │
// │  Per-frame updates:                                             │
// │    update_orb_anchor(os, c, x, z, q)   — dirty-flagged push     │
// │                                                                 │
// │  GPU dispatches (called from render tick):                      │
// │    dispatch_orb_init(os, c, encoder)        — one-shot seed     │
// │    dispatch_orb_recolor(os, c, encoder)     — palette resample  │
// │    dispatch_orb_copy_prev(os, c, encoder)   — snapshot prev     │
// │    dispatch_orb_dynamics(os, c, enc, q)     — rule + couplings  │
// │    render_orbs(os, c, pass)                 — additive draw     │
// │                                                                 │
// │  Cross-module reads: orbs_state_ is fully encapsulated; no      │
// │  external module reaches into its fields.                       │
// │                                                                 │
// └─────────────────────────────────────────────────────────────────┘
//
// Depends on: mood_constants.hpp (MOOD_COUNT); forward declarations of
// Cartridge (keyhole) and the wgpu handle classes used by reference in
// the declarations. The impl additionally needs state.hpp (GPUOrbConfig,
// Dim::MAX_ORBS) and renderer.hpp — both precede it in the TU.
// ─────────────────────────────────────────────────────────────────

// LOCKSTEP INSURANCE (same construct as entity_types.hpp): mirrors
// webgpu_cpp.h's declaration forms (`class Queue` / `class CommandEncoder`
// / `class RenderPassEncoder`, in namespace wgpu). If Dawn ever changes
// those forms, replace these forward declarations with the include.
namespace wgpu { class Queue; class CommandEncoder; class RenderPassEncoder; }

namespace t7 {
namespace the_board {

class Cartridge;  // fwd — the module functions take the keyhole Cartridge* (defined in orbs.inl, post-class)

// ═══ TUNING CONSOLE ══════════════════════════════════════════════
//
// All orb-system tuning dials in one place. Anything that shapes
// the feel of the sky — dome size, sprite size, coupling rates,
// sanitization defaults — is here. Mood-authored values live in
// ORB_MOOD_TABLE (end of this header); these are system-level dials
// that apply across every mood.

// ── Dome geometry ────────────────────────────────────────────────
inline constexpr float ORB_DOME_RADIUS = 450.0f;
inline constexpr float ORB_BASE_SIZE = 3.0f;

// ── Noise floor ──────────────────────────────────────────────────
// Barely perceptible drift in silence. The steady noise amplitude
// since the gen-1 orb coupling retired (M1-B) — the kernel's noise
// input rests here; a gen-2 coupling may lerp it again.
inline constexpr float ORB_NOISE_FLOOR    = 0.3f;

// ── Rule-critical parameter floors ───────────────────────────────
// Applied in configure_orbs when the mood authors 0.0 for a given
// field: zero reads as "no opinion, use system default" so every
// rule has working parameters regardless of mood authorship. A mood
// wanting "almost zero" should author a tiny non-zero (e.g. 0.001f),
// which reads as intentional.
inline constexpr float ORB_DEFAULT_DRAG = 0.5f;
inline constexpr float ORB_DEFAULT_ORBITAL_SPEED = 0.15f;
inline constexpr float ORB_DEFAULT_FLOCK_SEP_R = 50.0f;
inline constexpr float ORB_DEFAULT_FLOCK_ALIGN_R = 120.0f;
inline constexpr float ORB_DEFAULT_FLOCK_COH_R = 200.0f;
inline constexpr float ORB_DEFAULT_FLOCK_SEP_W = 30.0f;
inline constexpr float ORB_DEFAULT_FLOCK_ALIGN_W = 8.0f;
inline constexpr float ORB_DEFAULT_FLOCK_COH_W = 15.0f;
inline constexpr float ORB_DEFAULT_FLOCK_MAX_SPEED = 60.0f;


// ═══ REGISTRY: PALETTES ══════════════════════════════════════════
//
// Each palette is up to 4 weighted HSV "pockets" — a mood's sky can
// be mostly one color with rare accents. Selection weights across
// pockets should sum ≈ 1.0 (the last pocket catches the tail).
// Indexed by palette_id in mood config.

inline constexpr uint32_t MAX_ORB_PALETTE_ENTRIES = 4;

struct OrbPaletteEntry {
    float hue;         // HSV hue center (0..1)
    float hue_var;     // spread around center
    float saturation;  // base saturation
    float weight;      // selection probability
};

struct OrbPalette {
    uint32_t count;
    float    value_variance;   // per-orb HSV value (brightness) spread
    OrbPaletteEntry entries[MAX_ORB_PALETTE_ENTRIES];
};

// Stellar classification — diagnostic baseline with bold equally-
// weighted pockets (O/B hot blue, F/G yellow-white, K orange, M red).
inline constexpr OrbPalette ORB_PALETTE_JWST_DEEP = {
    4, 0.15f,
    {
        //  hue    hue_var  sat    weight
        { 0.60f, 0.03f, 0.90f, 0.25f },   // hot blue
        { 0.12f, 0.04f, 0.60f, 0.25f },   // yellow-white
        { 0.07f, 0.04f, 0.85f, 0.25f },   // orange
        { 0.01f, 0.03f, 0.95f, 0.25f },   // deep red
    }
};

// Hubble SHO (Pillars of Creation): teal and gold with copper.
inline constexpr OrbPalette ORB_PALETTE_PILLARS = {
    4, 0.30f,
    {
        //  hue    hue_var  sat    weight
        { 0.10f, 0.03f, 0.65f, 0.40f },   // gold
        { 0.48f, 0.04f, 0.55f, 0.35f },   // teal
        { 0.02f, 0.02f, 0.70f, 0.15f },   // copper accent
        { 0.55f, 0.02f, 0.35f, 0.10f },   // pale blue
    }
};

// Carina Nebula: rich blues and amber-orange, no greens.
inline constexpr OrbPalette ORB_PALETTE_CARINA = {
    3, 0.30f,
    {
        //  hue    hue_var  sat    weight
        { 0.60f, 0.05f, 0.55f, 0.45f },   // rich blue
        { 0.08f, 0.04f, 0.60f, 0.40f },   // amber-orange
        { 0.55f, 0.03f, 0.30f, 0.15f },   // desaturated blue-white
        { 0.00f, 0.00f, 0.00f, 0.00f },   // (unused)
    }
};

// Single-pocket warm (legacy-equivalent, for testing).
inline constexpr OrbPalette ORB_PALETTE_WARM_MONO = {
    1, 0.20f,
    {
        //  hue    hue_var  sat    weight
        { 0.08f, 0.06f, 0.60f, 1.00f },
        { 0.00f, 0.00f, 0.00f, 0.00f },
        { 0.00f, 0.00f, 0.00f, 0.00f },
        { 0.00f, 0.00f, 0.00f, 0.00f },
    }
};

inline constexpr uint32_t ORB_PAL_JWST_DEEP = 0;
inline constexpr uint32_t ORB_PAL_PILLARS = 1;
inline constexpr uint32_t ORB_PAL_CARINA = 2;
inline constexpr uint32_t ORB_PAL_WARM_MONO = 3;
inline constexpr uint32_t ORB_PAL_COUNT = 4;

inline constexpr OrbPalette ORB_PALETTES[ORB_PAL_COUNT] = {
    ORB_PALETTE_JWST_DEEP,
    ORB_PALETTE_PILLARS,
    ORB_PALETTE_CARINA,
    ORB_PALETTE_WARM_MONO,
};

inline constexpr const char* ORB_PAL_NAMES[ORB_PAL_COUNT] = {
    "jwst_deep", "pillars", "carina", "warm_mono"
};


// ═══ REGISTRY: TIER SETS ═════════════════════════════════════════
//
// A tier classifies "what kind of orb is this" — at init each orb
// rolls into a tier by weight, then samples its physics (mass, drag,
// size, brightness) from that tier's ranges and carries the tier's
// gains into the dynamics kernel (noise, force, color, flocking).
//
// tierset_id = 0xFFFFFFFFu (ORB_TIERSET_NONE) → uniform population,
// ignores every tier field (see configure_orbs + orb_init).

inline constexpr uint32_t MAX_ORB_TIERS = 4;

struct OrbTier {
    // Physics (ranges sampled per-orb at init)
    float mass_mult = 1.0f;
    float drag_mult = 1.0f;
    float size_min = 0.7f;   // multiplier on base_size
    float size_max = 1.3f;
    float brightness_min = 0.7f;   // multiplier on palette value
    float brightness_max = 1.0f;
    // Coupling gains (apply each frame in dynamics kernel)
    float noise_gain = 1.0f;   // 0..1+
    float force_gain = 1.0f;
    float color_gain = 1.0f;
    // Selection
    float weight = 0.25f;  // relative selection probability
    // Flocking gains (used when motion_rule == 3)
    float flock_sep_gain = 1.0f;
    float flock_align_gain = 1.0f;
    float flock_coh_gain = 1.0f;
};

struct OrbTierSet {
    uint32_t count;
    OrbTier tiers[MAX_ORB_TIERS];
};

// "JWST Stars" — classic deep-field population.
//   giants  : rare, eye-catchers, slow to settle, full color, lead the flock
//   main    : the population baseline
//   faint   : numerous, small, reduced color; follows flow without clustering
//   flickers: hyperactive tiny ones; high noise, edge-scouts
inline constexpr OrbTierSet ORB_TIERSET_JWST_STARS = {
    4,
    {
        //  mass   drag   s_min  s_max  b_min  b_max  n_g    f_g    c_g    w        fs     fa     fc
        {   1.8f,  0.6f,  1.2f,  1.6f,  0.9f,  1.0f,  0.6f,  0.8f,  1.0f,  0.10f,   0.6f,  0.8f,  0.3f  },  // giants
        {   1.0f,  1.0f,  0.8f,  1.1f,  0.6f,  0.9f,  1.0f,  1.0f,  1.0f,  0.60f,   1.0f,  1.0f,  1.0f  },  // main
        {   0.6f,  1.2f,  0.5f,  0.8f,  0.4f,  0.7f,  1.0f,  0.9f,  0.3f,  0.25f,   0.8f,  1.2f,  0.5f  },  // faint
        {   0.3f,  0.8f,  0.4f,  0.6f,  0.5f,  0.8f,  1.8f,  0.5f,  0.8f,  0.05f,   1.8f,  0.5f,  0.0f  },  // flickers
    }
};

// "Resonant" — voiced chamber-like sky.
//   drones : heavy, slow, convergence-responsive, strong cohesion (anchor)
//   voices : balanced baseline
//   sparks : pure motion, no color, high separation (edge agitators)
inline constexpr OrbTierSet ORB_TIERSET_RESONANT = {
    3,
    {
        //  mass   drag   s_min  s_max  b_min  b_max  n_g    f_g    c_g    w        fs     fa     fc
        {   2.5f,  0.5f,  1.3f,  1.7f,  0.7f,  1.0f,  0.3f,  0.6f,  1.0f,  0.20f,   0.5f,  0.7f,  1.5f  },  // drones
        {   1.0f,  1.0f,  0.8f,  1.2f,  0.6f,  0.9f,  1.0f,  1.0f,  0.7f,  0.50f,   1.0f,  1.0f,  1.0f  },  // voices
        {   0.4f,  0.9f,  0.4f,  0.7f,  0.5f,  0.8f,  1.8f,  1.2f,  0.0f,  0.30f,   1.8f,  0.6f,  0.2f  },  // sparks
        {   0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,    0.0f,  0.0f,  0.0f  },  // (unused)
    }
};

inline constexpr uint32_t ORB_TIERSET_JWST = 0;
inline constexpr uint32_t ORB_TIERSET_RES = 1;
inline constexpr uint32_t ORB_TIERSET_COUNT = 2;
inline constexpr uint32_t ORB_TIERSET_NONE = 0xFFFFFFFFu;  // → uniform population

inline constexpr OrbTierSet ORB_TIERSETS[ORB_TIERSET_COUNT] = {
    ORB_TIERSET_JWST_STARS,
    ORB_TIERSET_RESONANT,
};

inline constexpr const char* ORB_TIERSET_NAMES[ORB_TIERSET_COUNT] = {
    "jwst_stars", "resonant"
};


// ═══ REGISTRY: FLOCKING GESTURES ═════════════════════════════════
//
// Eight named sign combinations over the 2³ space (separation,
// alignment, cohesion). Cycled by KP_DECIMAL at runtime. Each row
// is one recognizable behavior; sign flipping reverses the
// direction of that force in the dynamics kernel.
//
//   flock    (+++)  standard cohere-align-space
//   antiflock(---)  full dispersal
//   swirl    (+-+)  cohere but counter-flow within the cluster
//   orbit    (-+-)  ring around empty core
//   huddle   (-++)  dense aligned clump
//   flee     (++-)  spaced + aligned, runs from the center
//   chaos    (+--)  spaced + counter-flow, fleeing
//   trap     (--+)  pulled in, counter-flow, cohesive knot

struct OrbFlockGesture {
    float       sep_sign;
    float       align_sign;
    float       coh_sign;
    const char* name;
};

inline constexpr uint32_t ORB_FLOCK_GESTURE_COUNT = 8;
inline constexpr OrbFlockGesture ORB_FLOCK_GESTURES[ORB_FLOCK_GESTURE_COUNT] = {
    //  sep    align  coh    name
    { +1.0f, +1.0f, +1.0f, "flock"     },  // 0
    { -1.0f, -1.0f, -1.0f, "antiflock" },  // 1
    { +1.0f, -1.0f, +1.0f, "swirl"     },  // 2
    { -1.0f, +1.0f, -1.0f, "orbit"     },  // 3
    { -1.0f, +1.0f, +1.0f, "huddle"    },  // 4
    { +1.0f, +1.0f, -1.0f, "flee"      },  // 5
    { +1.0f, -1.0f, -1.0f, "chaos"     },  // 6
    { -1.0f, -1.0f, +1.0f, "trap"      },  // 7
};


// ═══ REGISTRY: BROWNIAN GESTURES ═════════════════════════════════
//
// Six variations over three binary dimensions — radial sign
// (expansion vs contraction on music), vertical bias (isotropic
// vs upward), and coherence (per-orb independent vs per-block
// shared). Cycled by the gesture key when the active rule is
// Brownian; player state persists across mood transitions.

struct OrbBrownianGesture {
    float       radial_sign;   // ±1 — polyphony expands or contracts
    float       vert_bias;     // 0 = isotropic, 1 = upward-biased noise
    float       coherence;     // 0 = per-orb, 1 = per-block ("wind gusts")
    const char* name;
};

inline constexpr uint32_t ORB_BROWNIAN_GESTURE_COUNT = 6;
inline constexpr OrbBrownianGesture ORB_BROWNIAN_GESTURES[ORB_BROWNIAN_GESTURE_COUNT] = {
    //  radial  vert   coh    name
    { +1.0f,   0.0f,  0.0f,  "drift"  },  // 0  default wandering diffusion
    { -1.0f,   0.0f,  0.0f,  "gather" },  // 1  music pulls orbs inward
    { +1.0f,   1.0f,  0.0f,  "rise"   },  // 2  upward drift (embers)
    { +1.0f,   0.0f,  1.0f,  "gust"   },  // 3  wind-gust group motion
    { -1.0f,   0.0f,  1.0f,  "tide"   },  // 4  coherent inward pull
    { +1.0f,   1.0f,  1.0f,  "swell"  },  // 5  upward column with coherence
};


// ═══ REGISTRY: ORBITAL GESTURES ══════════════════════════════════
//
// Four variations over axis alignment + speed variance. Cycled
// by the gesture key when the active rule is Orbital.

struct OrbOrbitalGesture {
    float       alignment_mode;   // 0 scatter, 1 parallel, 2 mirror
    float       speed_var_mult;   // multiplier on per-orb speed spread
    const char* name;
};

inline constexpr uint32_t ORB_ORBITAL_GESTURE_COUNT = 4;
inline constexpr OrbOrbitalGesture ORB_ORBITAL_GESTURES[ORB_ORBITAL_GESTURE_COUNT] = {
    //  align  var    name
    { 0.0f,   1.0f,  "scatter"  },  // 0  random axes, chaotic rotation
    { 1.0f,   0.0f,  "parallel" },  // 1  unified shell around Y
    { 2.0f,   0.0f,  "mirror"   },  // 2  two counter-rotating streams
    { 1.0f,   3.0f,  "shear"    },  // 3  parallel axis, layered sheets
};


// ═══ MOOD CONFIG (authoring surface) ═════════════════════════════
//
// Declared here; instantiated per-mood in ORB_MOOD_TABLE (end of this
// header). configure_orbs consumes this and sanitizes rule-critical
// zeros against the ORB_DEFAULT_* floors above.

struct OrbMoodConfig {
    // Population
    bool     enabled = false;
    uint32_t count = 0;        // clamped to Dim::MAX_ORBS
    // Color (legacy — superseded by palette_id when a palette is set)
    float    base_hue = 0.08f;
    float    hue_variance = 0.05f;
    float    brightness = 0.8f;     // value center; palette spreads around it
    // Motion
    float    drag = ORB_DEFAULT_DRAG;
    uint32_t motion_rule = 0;                       // 0=Brownian 1=Orbital 2=Frozen 3=Flocking
    float    rotation_speed = 0.0f;                   // rad/s
    float    rotation_axis[3] = { 0.0f, 1.0f, 0.0f };   // normalized in configure_orbs
    float    orbital_base_speed = 0.0f;               // rad/s, rule 1 only
    // Color palette
    uint32_t palette_id = ORB_PAL_JWST_DEEP;
    float    hue_converge_target = 0.12f;
    // Anchor (mood default applies only on first configure; player wins after)
    bool     anchor_to_pawn_default = false;
    // Population variety (ORB_TIERSET_NONE = uniform population)
    uint32_t tierset_id = ORB_TIERSET_NONE;
    // Flocking (used when motion_rule == 3)
    float    flock_sep_radius = 50.0f;
    float    flock_align_radius = 120.0f;
    float    flock_coh_radius = 200.0f;
    float    flock_sep_weight = 30.0f;
    float    flock_align_weight = 8.0f;
    float    flock_coh_weight = 15.0f;
    float    flock_max_speed = 60.0f;
    // Flock gesture seed (player cycle persists across transitions)
    uint32_t flock_gesture_default = 0u;
    // Per-rule drag multipliers (0 = pass-through, 1.0×)
    float    rule_drag_brownian = 0.0f;
    float    rule_drag_orbital = 0.0f;
    float    rule_drag_frozen = 0.0f;
    float    rule_drag_flocking = 0.0f;
};


// ═══ ORBS MODULE STATE (Scope B migration #7) ════════════════════
//
// All orb-owned state lives in this struct, accessed via orbs_state_
// on the Cartridge (declared at the composition root). Module functions
// take `OrbsState& os` explicitly rather than reaching via Cartridge*,
// making ownership language-visible and dependencies explicit in
// signatures.
//
// Sub-grouped by role:
//   • lifecycle      — kernel arming flags, palette
//   • anchor         — player-owned dome-center follow
//   • motion         — current rule + per-rule gesture indices
//   • speed          — population speed multiplier
//
// Motion rule identifiers, named for legibility at gesture-dispatch
// sites. Index into gesture_idx / gesture_initialized.

inline constexpr uint32_t ORB_RULE_BROWNIAN = 0u;
inline constexpr uint32_t ORB_RULE_ORBITAL = 1u;
inline constexpr uint32_t ORB_RULE_FROZEN = 2u;
inline constexpr uint32_t ORB_RULE_FLOCKING = 3u;

struct OrbsState {
    // ── Lifecycle / kernel arming ────────────────────────────────
    bool     active = false;
    uint32_t count = 0;
    bool     init_pending = false;
    bool     recolor_pending = false;
    uint32_t current_palette_id = ORB_PAL_JWST_DEEP;

    // ── Anchor (dome-center follow) ──────────────────────────────
    // Player state: persists across mood transitions. Dirty-flag cache
    // means an idle or unanchored dome produces no per-frame queue traffic.
    bool     pawn_anchored = false;
    bool     anchor_initialized = false;   // seeded by mood default on first configure
    float    last_dome_center_x = 0.0f;
    float    last_dome_center_z = 0.0f;
    bool     dome_center_initialized = false;

    // ── Motion rule + flocking gesture ───────────────────────────
    // Motion rule and flock gesture are both player-owned: they persist
    // across mood transitions. The rule seeds to Brownian on first run.
    uint32_t current_motion_rule = 0u;
    bool     motion_rule_initialized = false;  // one-time Brownian seed guard
    // Per-rule gesture indices. Index 2 (Frozen) is vestigial
    // — Frozen has no gestures; cycle_orb_gesture short-circuits. All
    // four indices persist across mood transitions (player state).
    uint32_t gesture_idx[4]         = { 0u, 0u, 0u, 0u };
    bool     gesture_initialized[4] = { false, false, false, false };

    // ── Speed ────────────────────────────────────────────────────
    // Population speed multiplier. Smoothed on the CPU, uploaded via
    // upload_orb_speed_mult only when it moves.
    float    speed_mult_current = 1.0f;
    // DRIVERLESS (CPU landing site; census: constitution §5): rests at
    // 1.0 (identity); writer retired with gen-1 — the gen-2 "orb.speed"
    // pipe lands here when its coupling is designed. Dies by
    // revive-or-delete when this region is next worked.
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════
//
// DEFINED in orbs.inl (post-class, self-wrapping) — each dereferences the
// keyhole (gpuState_ uploads, renderer_ dispatches, world_state_ seed,
// player_ readback, time_state_ clocks), which requires the complete
// Cartridge. ORB-1's anchor semantics are untouched by the conversion.

// Lifecycle
void configure_orbs(OrbsState& os, Cartridge* c, const OrbMoodConfig& cfg, wgpu::Queue& queue);
void teardown_orbs(OrbsState& os, Cartridge* c);
// Player commands
void cycle_orb_palette(OrbsState& os, Cartridge* c, wgpu::Queue& queue);
void cycle_orb_motion_rule(OrbsState& os, Cartridge* c, wgpu::Queue& queue);
void cycle_orb_gesture(OrbsState& os, Cartridge* c, wgpu::Queue& queue);
void toggle_orb_anchor(OrbsState& os, const Cartridge* c);
// Per-frame updates
void update_orb_anchor(OrbsState& os, Cartridge* c, float pawn_x, float pawn_z, wgpu::Queue& queue);
// GPU dispatches
void dispatch_orb_init(OrbsState& os, Cartridge* c, wgpu::CommandEncoder& encoder);
void dispatch_orb_recolor(OrbsState& os, Cartridge* c, wgpu::CommandEncoder& encoder);
void dispatch_orb_copy_prev(OrbsState& os, Cartridge* c, wgpu::CommandEncoder& encoder);
void dispatch_orb_dynamics(OrbsState& os, Cartridge* c, wgpu::CommandEncoder& encoder,
    wgpu::Queue& queue);
// Render
void render_orbs(OrbsState& os, Cartridge* c, wgpu::RenderPassEncoder& pass);


// ─── Orb Mood Table ─────────────────────────────────────────────
//
// Lives at end-of-header because OrbMoodConfig is defined above and
// MOOD_COUNT comes from mood_constants.hpp. Resolves orbs:D2.
//
// DONE[orbs:D2] per-mood orb authoring data lives with the
//   OrbMoodConfig struct that defines its shape.
// SEAM[mood:K4] mood-5 row is bit-identical to mood-0 (open_default)
//   except for the implicit context that mood-5 is finite_outdoor_ref.
//   Mirrors the same MOOD_TABLE pattern. Resolves with the
//   has_anchor_ribbon flag (mood:L1).
//
// Per-mood orb config. Indexed by the same mood index as MOOD_TABLE.
// See OrbMoodConfig above for field semantics. Zero-valued rule-critical
// fields (drg, orbS, flock radii/weights) are sanitized to system
// defaults in configure_orbs — "0 = no opinion, system picks a working
// value." Explicit small non-zero reads as deliberate authorship.
//
//  Column legend (short → field name):
//    en      enabled              rotAxis rotation_axis[3]
//    n       count                orbS    orbital_base_speed (rule 1)
//    hueB    base_hue (legacy)    pal     palette_id (ORB_PAL_*)
//    hueV    hue_variance         hct     hue_converge_target
//    bri     brightness           anc     anchor_to_pawn_default
//    drg     drag (1/s)           trs     tierset_id (0xFFFFFFFFu = legacy)
//    rul     motion_rule
//    rotS    rotation_speed
//    sepR/alnR/cohR/sepW/alnW/cohW/maxS   flocking parameters
//    gst     flock_gesture_default (0..7, ORB_FLOCK_GESTURES index)
//    drgB/drgO/drgF/drgK          per-rule drag multipliers (0 = 1.0× pass-through)
//
//                                              en     n    hueB   hueV   bri    drg   rul  rotS    rotAxis                  orbS  pal  hct    anc    trs           sepR   alnR    cohR    sepW   alnW   cohW   maxS   gst  drgB  drgO  drgF  drgK
inline constexpr OrbMoodConfig ORB_MOOD_TABLE[MOOD_COUNT] = {
    /* 0 open_default        */ {  true,  128, 0.08f, 0.06f, 0.85f, 0.4f,  0u,  0.012f, {0.15f, 0.97f, 0.10f},  0.0f, 0u,  0.12f, true,  0u,           50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
    /* 1 open_sunset         */ {  true,  128, 0.08f, 0.06f, 0.85f, 0.4f,  3u,  0.012f, {0.15f, 0.97f, 0.10f},  0.0f, 0u,  0.08f, false, 0u,           50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
    /* 2 indoor_flat         */ {  false, 0,   0.08f, 0.05f, 0.80f, 0.5f,  0u,  0.000f, {0.00f, 1.00f, 0.00f},  0.0f, 0u,  0.12f, false, 0xFFFFFFFFu,  50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
    /* 3 indoor_vault        */ {  false, 0,   0.08f, 0.05f, 0.80f, 0.5f,  0u,  0.000f, {0.00f, 1.00f, 0.00f},  0.0f, 0u,  0.12f, false, 0xFFFFFFFFu,  50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
    /* 4 finite_outdoor      */ {  true,  128, 0.08f, 0.06f, 0.85f, 0.4f,  0u,  0.012f, {0.15f, 0.97f, 0.10f},  0.0f, 0u,  0.12f, true,  0u,           50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
    /* 5 finite_outdoor_ref  */ {  true,  128, 0.08f, 0.06f, 0.85f, 0.4f,  0u,  0.012f, {0.15f, 0.97f, 0.10f},  0.0f, 0u,  0.12f, true,  0u,           50.0f, 120.0f, 200.0f, 30.0f, 8.0f,  15.0f, 60.0f, 0u,  0.0f, 0.0f, 0.0f, 0.0f },
};

} // namespace the_board
} // namespace t7
