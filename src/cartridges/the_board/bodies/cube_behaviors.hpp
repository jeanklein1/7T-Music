#pragma once
#include <cstdint>
#include <array>
#include "cartridges/the_board/realization/state.hpp"                       // Dim::MAX_CUBE_INSTANCES, GPUState, GPUFloatingEntityState, wgpu
#include "cartridges/the_board/contracts/floaters.hpp"  // ActiveCube, CUBE_TIER_COUNT
#include "cartridges/the_board/contracts/driver_surface.hpp"  // THE DRIVERS' ROOM: DRIVER_LIVE.cube — the choir's incandescence + gain
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)
#include "cartridges/the_board/contracts/entity_types.hpp"   // queue types (the funnel signatures)

// ─── cube_behaviors.hpp (HEADER: registries + console + state + decls) ─
//
// Cube behavior system.
//
// The impl additionally hashes with seed_utils; the cube recipe
// (below) calls into the two spawn helpers; world.wgsl holds the
// force functions and dispatch switch.
// ──────────────────────────────────────────────────────────────────

#include <cmath>      // std::cos, std::sin, std::exp, std::fabs   // (impl, merged)
#include <iostream>   // diagnostics feedback   // (impl, merged)
#include <numeric>    // std::gcd — the helix coprimality witness
#include <algorithm>  // std::min / std::max — the walk's clamps and settle norm
// <cstdio> and core/instruments.hpp stood here (CHOIR_0 U5) for the
// [ZOETROPE] strike witness. The witness went with the strike, and its
// successor prints from the canvas, where the envelope it reports lives
// — so the dial (INSTRUMENTS.zoetrope_witness, name unchanged, rename
// PARKED) is read one tier down and this file needs neither.

namespace t7 {
namespace the_board {

// ═══ MODULE DEPS ════════════════════════════════════════════════════
// The cube commands' requirements face: corral/kite center on THE
// POINT through the point's own record (point_.x/z — the agent
// slot reach retired with it); all reads except the GPU wire.
struct CubeDeps {
    GPUState&        gpuState_;
    const TimeState& time_state_;
    const PlayerState& player_;
    const PointState&  point_;   // the point's house (position mirror — the corral ring's center)
};

// ═══ BEHAVIOR IDS ════════════════════════════════════════════════

inline constexpr uint32_t CUBE_BEHAVIOR_STATIONARY = 0;
inline constexpr uint32_t CUBE_BEHAVIOR_CURLFIELD  = 1;
inline constexpr uint32_t CUBE_BEHAVIOR_PHASEWAVE  = 2;
// CUBE_BEHAVIOR_COUNT_WGSL exists on the WGSL side (world.wgsl §7
// cube behavior registry). MUST match this value — when adding a
// behavior, bump both. Mirrors the agents pattern
// (AGENT_BEHAVIOR_COUNT_WGSL).
inline constexpr uint32_t CUBE_BEHAVIOR_COUNT      = 3;

inline constexpr const char* CUBE_BEHAVIOR_NAMES[CUBE_BEHAVIOR_COUNT] = {
    "stationary", "curlfield", "phasewave"
};

// ═══ TUNING CONSOLE ══════════════════════════════════════════════

// ─ Substrate (drift integrator) ─────────────────────────────────

inline constexpr float CUBE_DEFAULT_SPRING_STIFFNESS = 4.0f;   // 1/s², ~0.5s settle
inline constexpr float CUBE_DEFAULT_DRAG             = 1.5f;   // 1/s,  gentle damping

// ─ WGSL kernel constants (NOT here, but documented) ─────────────
//
//   cube_force_phasewave (world.wgsl):
//     k_x         = 0.020    wavefront freq in X
//     k_z         = 0.012    wavefront freq in Z (asymmetric)
//     omega       = 1.5      1/s, temporal frequency
//     amplitude   = 30.0     force magnitude (vertical)

// ═══ ZOETROPE — THE CONSOLE ═══ four bands: THE LATTICE, THE CHOIR,
// THE SCREEN, THE EXPRESSION. All Jean-tunable; beats units unless a
// comment says otherwise. (THE AUTOMATON band was the fifth and left
// with the lattice's substrate at CHOIR_0 U5; its tombstone stands
// where it did.)
//
// NONE OF THESE IS MIRRORED TO WGSL. The zoetrope is CPU-resident by
// construction (the GPU is a projector only), so every constant here is
// safe to change live — no shader recompile, no struct, no binding, no
// two-room handshake to keep. The only arithmetic that binds is stated
// in its own band: the helix's coprimality and the screen's spacing.
//
// ─ THE LATTICE band ─────────────────────────────────────────────
// THE FORMATION GEOMETRY OWNS THESE NOW. They were the automaton's
// address space and the screen's shape at once; the automaton is gone,
// so what is left is the SEATING — seven ranked rows of thirty-six
// bearings, and the helix that walks a slot onto one of them
// (zoetrope_station / station_scatter are the only readers). The
// numbers do not move: the screen is the screen it was.
inline constexpr uint32_t LATTICE_ROWS  = 7;   // the mode's degrees
inline constexpr uint32_t LATTICE_COLS  = Dim::MAX_CUBE_INSTANCES / LATTICE_ROWS;          // 36
inline constexpr uint32_t LATTICE_CELLS = LATTICE_ROWS * LATTICE_COLS;  // 252 seats; the LIVING ceiling is CUBE_CHOIR_N now, capacity stays 256
inline constexpr uint32_t ZOETROPE_CELL_STRIDE   = LATTICE_COLS + 1;  // 37 — the helix
inline constexpr uint32_t ZOETROPE_CELL_UNSTRIDE = 109;               // 37⁻¹ mod 252
static_assert((ZOETROPE_CELL_STRIDE * ZOETROPE_CELL_UNSTRIDE) % LATTICE_CELLS == 1u,
              "helix inverse broken — recompute UNSTRIDE for these dims");
static_assert(std::gcd(ZOETROPE_CELL_STRIDE, LATTICE_CELLS) == 1u,
              "helix stride must be coprime to the lattice");

// ─ THE CHOIR band ─ the boot choice: how many keys the instrument has.
// Not random, chosen before boot (Jean). Two ranks or three of twelve.
//
// KEY k = SLOT k, BY CONSTRUCTION. run_spawn_preamble reserves the LOWEST
// FREE SLOT (machine/spawn_engine.hpp, step 8-9), so capping the family's
// max_instances here keeps the population dense in slots 0..N-1 and an
// evicted key's refill takes the lowest free slot — THE SAME DARK KEY
// RELIGHTS. No mapping table, no registry: the identity IS the law.
//
// The lattice geometry above is untouched by this: LATTICE_COLS is 36 by
// arithmetic (256/7) and CUBE_CHOIR_N is 36 by CHOICE. Two facts that
// happen to share a number today; flipping the choir to 24 moves one and
// not the other, which is why they are not the same constant.
inline constexpr uint32_t CUBE_CHOIR_N = 36;
static_assert(CUBE_CHOIR_N == 24u || CUBE_CHOIR_N == 36u,
    "the choir is stacked pianos: two ranks or three, nothing else");
static_assert(CUBE_CHOIR_N % 12u == 0u, "ranks are whole pianos");
static_assert(CUBE_CHOIR_N <= LATTICE_CELLS,
    "the choir seats through the helix bijection — it may not outrun the lattice");
inline constexpr uint32_t CUBE_CHOIR_RANKS = CUBE_CHOIR_N / 12u;
// THE POKE GATE. The projector runs every frame now — the lattice's
// tick is gone and there is nothing left to hide a flush behind — so
// the flush is gated on the LIGHT instead of on a clock: a slot pokes
// only when its light moved past this.
//
// THE GATE COMPARES AGAINST THE LAST FLUSH, NOT THE LAST FRAME, which
// is the difference between thinning and stopping: when a frame's step
// falls under this, the residual keeps ACCUMULATING against the stored
// shadow and pokes when the sum crosses. So the light never sits more
// than one epsilon from what the GPU holds, and a slow drift is
// reported late rather than lost.
//
// THE ARITHMETIC, at 120 BPM / 60 fps (30 frames a beat, Δ = 1/30 beat),
// with the default 8-beat plateau and 8-beat release — COUNTED, not
// estimated. ATTACK: ΔI = (1 − I)·(1 − e^(−Δ/τ)) = 0.01653·(1 − I) at
// τ = 2 — **199 pokes across the plateau's 240 frames**, then it thins
// with the residual: about 17 more over the following ten plateaus as
// I closes on 1. RELEASE: the slope is 1/8 per beat = 0.00417 a frame,
// four times this gate, so a fall pokes **every one of its 240 frames**.
// WORST CASE is one poke per SOUNDING key per frame with the whole
// choir falling at once: 36 twelve-byte colour writes and 36 four-byte
// variance writes — and, WHEN THE SCREEN STANDS, 36 four-byte radius
// writes as well, because the swell rides the same poke. 720 B a frame
// against the lattice's ≤252-slot sweep every quarter beat. Silence
// pokes NOTHING.
inline constexpr float CHOIR_FLUSH_EPS = 1e-3f;

// ─ THE AUTOMATON band stood here (CHOIR_0 U5) ───────────────────
// TICK_BEATS, REV_BEATS, EXCITE_DIFFUSE, ASYMMETRY, EXCITE_HALF_BEATS,
// PIGMENT_GAIN, PIGMENT_HALF_BEATS, STRIKE_SPREAD, WEIGHT_SEED, the two
// derived decays and the diffusion-stability static_assert: the whole
// composite law of a lattice that no longer exists. The choir keeps no
// field to diffuse and no flash to decay — THE ENVELOPE IS THE MEMORY,
// and it lives one tier down, on the canvas, in beats
// (canvas::CANVAS_LIVE.light_plateau / .light_release). The write head
// went with them: a note lights the key it names, wherever that key
// stands, so there is nothing left for a revolution to sweep.
// ─ THE SCREEN band ──────────────────────────────────────────────
// THE SPACING: 9 wu rows against 6.4 wu pixels is the first spacing
// where the rows read as rows; the column arc at R=60 is 2πR/36 ≈
// 10.5 wu, so neighbours in a row clear each other too.
inline constexpr float ZOETROPE_RING_RADIUS = 60.0f;  // arc = 2π·R/36 ≈ 10.5 wu; FOV° ≈ 2·atan(3.5·H_STEP/RADIUS) ≈ 55°
inline constexpr float ZOETROPE_H_BASE      = 8.0f;   // row-0 height above ground (wu)
inline constexpr float ZOETROPE_H_STEP      = 9.0f;   // wu per mode degree — screen ≈ rows 8..62 wu
inline constexpr float ZOETROPE_PIXEL_RADIUS = 3.2f;  // pixel half-size; full ≈ 6.4 wu
// The screen's own loosening — all zero ⇒ the machined ring; raise for
// a hand-placed screen. Bounded by the spacing above: the jitters are
// fractions of the arc, the step and the radius, so no setting of them
// can put two pixels in one place.
inline constexpr float ZOETROPE_JITTER_R     = 0.14f;  // × radius
inline constexpr float ZOETROPE_JITTER_THETA = 0.35f;  // × column arc
inline constexpr float ZOETROPE_JITTER_H     = 0.30f;  // × H_STEP
inline constexpr uint32_t ZOETROPE_STATION_SEED = 0x57A7104Eu;
// ─ the scatter seat (the gathering, not the instrument) ─────────
inline constexpr float ZOETROPE_SCATTER_RADIUS  = 90.0f;  // wu — the gathering's mean reach
inline constexpr float ZOETROPE_SCATTER_JITTER_R = 0.45f; // × radius — deep, this is a flock
inline constexpr float ZOETROPE_SCATTER_JITTER_H = 28.0f; // wu — free vertical spread
inline constexpr float ZOETROPE_SCATTER_JITTER_THETA = 0.90f;  // × column arc — the flock is not spoked
inline constexpr float ZOETROPE_SCATTER_SIZE_BIAS   = 4.0f;   // wu of extra radius per wu of body radius
inline constexpr uint32_t ZOETROPE_SCATTER_SEED = 0x5CA77E12u;
// ─ the walk between seats (every transition is a walk) ──────────
inline constexpr float ZOETROPE_LIFT_TAU    = 1.1f;   // s — the climb's own walk law; birth-equal to CUBE_GLIDE_TAU, independently tunable
inline constexpr float ZOETROPE_SETTLE_EPS  = 0.05f;  // wu — snap-and-stop threshold
inline constexpr float ZOETROPE_RESEAT_JUMP = 40.0f;  // wu/frame — no motion moves the point this far; only possess() does

// ─ THE EXPRESSION band ──────────────────────────────────────────
// One light, three expressions: the colour mix, the swell, and the
// CONVERGENCE of the face. All read the same I through choir_light's
// one door, so a lit key is one gesture.
inline constexpr float ZOETROPE_REST_DIM = 0.30f;  // SCREEN rest brightness — the instrument is dark until played
inline constexpr float ZOETROPE_SWELL_GAIN = 0.60f;  // × pixel radius at full I
// PIGMENT_R/G/B/WEIGHT stood here (CHOIR_0 U5) — the ethereal ice the
// mix aimed at, and the stain under the flash. Their successor is
// DRIVER_LIVE.cube.light_color, which is a DRIVER's dial rather than a
// module constant: a driven parameter wears no dial on its value, it
// wears one on its driver (contracts/driver_surface.hpp).
//
// FACE_SPLAY and FACE_REST stood here too, with the calibration note
// TUNE_2 B2 wrote for the splay's magnitude. Both retire with the law
// they served: the strike SPLAYED the face (a struck cell is a cell
// disturbed) over a rest MULTIPLIED up in formation. The light does the
// opposite — GLOW UNIFIES, so the projector closes the spawn draw by
// (1 − I) and needs neither a rest multiplier nor a splay knob. The
// draw's own σ (CUBE_TIERS FACE_VARIANCE) is the whole rest now, and it
// is self-restoring at I = 0, which is why no restore pass exists.

// ═══ REGISTRY: TIER GAINS ════════════════════════════════════════

struct CubeTierGain {
    uint32_t tier_idx;
    const char* name;
    float spring_stiffness_mult;
    float drag_mult;
    float behavior_amp_mult;   // reserved; not yet consumed by kernel
    float plasticity;          // CONTACT_3 K2c: per-tier RELATIVE λ character;
                               // the live master is config.cube_plasticity.
                               // Baked per-instance at spawn — changing a
                               // ROW needs a respawn; the MASTER is live.
                               // Jean-tunable in-row.
};

inline constexpr CubeTierGain CUBE_TIER_GAINS[CUBE_TIER_COUNT] = {
    //                            spring  drag   amp   λ (relative character — K2c; master = config.cube_plasticity)
    /* 0 SmallCube */ { 0, "SmallCube", 1.0f, 1.0f, 1.0f, 1.0f },
    /* 1 MedCube   */ { 1, "MedCube",   1.0f, 1.0f, 1.0f, 0.8f },
    /* 2 LargeCube */ { 2, "LargeCube", 1.0f, 1.0f, 1.0f, 1.2f },
    /* 3 Monolith  */ { 3, "Monolith",  1.0f, 1.0f, 1.0f, 0.5f },
};

static_assert(sizeof(CUBE_TIER_GAINS) / sizeof(CUBE_TIER_GAINS[0]) == CUBE_TIER_COUNT,
              "CUBE_TIER_GAINS must declare one row per cube tier");

// ═══ REGISTRY: POPULATIONS ═══════════════════════════════════════
//
// Cubes carried a mood term until ONE_WORLD-II U3 — mood_mult_for from
// MOOD_SPAWN_MULT, a column of all 1.0. This banner once claimed
// {1, 1, 0, 0, 1, 0} and concluded that cubes "don't spawn in indoor
// moods"; those zeros never existed in the live table, and the per-row
// "exists for hygiene" notes inherited the error. The term suppressed no
// row then and does not exist now.

// THE SUNSET ROW'S THREE VALUES, named so the bank's witness has a source
// to be proved against after CUBE_POPULATIONS itself left with the moods.
// They are the authored numbers, not a copy of the copy.
inline constexpr float CUBE_POPULATIONS_SUNSET_STAT = 1.0f;
inline constexpr float CUBE_POPULATIONS_SUNSET_CURL = 0.0f;
inline constexpr float CUBE_POPULATIONS_SUNSET_WAVE = 0.0f;

// ─── THE CUBE BANK (ONE_WORLD-II U1c) ────────────────────────────
// CUBE_POPULATIONS was seven rows, one per mood, indexed at every cube
// spawn by the live mood. It is one row now — and it always was one row in
// every way that mattered: ALL SEVEN carried { 1.0f, 0.0f, 0.0f }
// identically, so the mood axis here was fiction and the seeding is
// behaviour-identical for that reason and not merely by the sunset ruling.
//
// The bank owns cube behaviour as the agents own agents, so it stands at
// the cube's own home rather than folding into the population panel.
// Enrollment is NEW (these rows never existed) and stays U6 by ruling; the
// bank therefore has no transport to move with it today.
//
// A CONSEQUENCE, NAMED: `curl` and `wave` carry zero spawn weight and did
// under every mood, so no cube has ever spawned into either. The only
// reachable path to them is cycle_cube_behavior_override, itself a console
// orphan parked with the panel. Reported, not acted on — a taste question,
// and Jean's.
struct CubeBank {
    std::array<float, CUBE_BEHAVIOR_COUNT> behavior_weights;
};

// TRANSCRIBED AND PINNED (the canonized pattern), from the sunset row.
inline constexpr CubeBank CUBE_TABLE = {
    //                    stat  curl  wave
    /*behavior_weights=*/ { 1.0f, 0.0f, 0.0f }
};
static_assert(CUBE_TABLE.behavior_weights[0] == CUBE_POPULATIONS_SUNSET_STAT
           && CUBE_TABLE.behavior_weights[1] == CUBE_POPULATIONS_SUNSET_CURL
           && CUBE_TABLE.behavior_weights[2] == CUBE_POPULATIONS_SUNSET_WAVE,
    "CUBE_TABLE is the sunset row, transcribed (ONE_WORLD-II U1c)");

inline CubeBank CUBE_LIVE = CUBE_TABLE;

// ═══ DIAGNOSTIC STATE (owned by the tools) ═══════════════════════

// `ZoetropeCell` stood here (CHOIR_0 U5) — one lattice cell, a fast
// excitation and a slow pigment. The choir keeps no field: a key's light
// is its own state, indexed by the key, and the cube reads it where it
// stands. THE HELIX survives as FORMATION geometry (cell_of_slot, below)
// and nothing else; it is a seating law now, not an addressing one.

struct CubeBehaviorsState {
    uint32_t   behavior_override  = CUBE_BEHAVIOR_STATIONARY;
    bool       kite_mode          = false;
    ActiveCube activeCubes_[Dim::MAX_CUBE_INSTANCES]{};

    // ── The reveal machine (C6R; H1: F6 IS A CYCLE) ── staged capture,
    // CPU flush-walk climb. Three resting states — roam, scattered,
    // screen — and a walk between each: roam → scatter → screen → roam.
    // Every transition is a walk; nothing teleports.
    enum class Formation : uint8_t {
        ROAM, TO_SCATTER, SCATTERED, TO_SCREEN, SCREEN, TO_ROAM };
    Formation formation = Formation::ROAM;
    bool  stations_sent = false;
    // The dim changes with the formation, not with the music, so the
    // projector must repaint once at the transition — otherwise the new
    // rest waits for a note that may never come (V1 E3). reveal_zoetrope
    // cannot reach the world seed the projector needs, so it raises this
    // and choir_project spends it as a FORCE on its next pass — the one
    // thing that has to outrank the poke gate, because it changes what a
    // cube looks like without moving its light.
    bool  repaint_all   = false;
    // The stage frame: a kite sentinel is in flight and will EAT any
    // target written before it is consumed (V1), so the seat pass waits
    // one frame. THREE writers, two of them load-bearing: F7's toggle
    // and the reseat both send a sentinel and must stage; the reveal no
    // longer sends one (K1) and stages only to keep one press shape.
    bool  stage_wait    = false;
    // The walker's own shadows (G5) — height, radius, aspects; it never
    // reads the GPU values; the walker owns the scalars while the walk
    // lives. The PRIOR needs no stage: THE MIRROR IS THE PRIOR
    // (activeCubes_ keeps every tier draw; release targets read it).
    struct ZoeWalk { float h, r, ay, az; } walk_[LATTICE_CELLS]{};
    bool  settled[LATTICE_CELLS]{};
    // ── The reseat watch (G4) ── the point's last seen position; a
    // per-frame step no motion can make marks a possession seam.
    float last_px = 0.0f, last_pz = 0.0f;
    bool  point_seen = false;

    // ── THE CHOIR'S LIGHT (the mirror, and the poke gate) ──────────
    // choir_I is written by ONE AUTHOR, once per frame: the cartridge's
    // motion-drivers phase, which reads the canvas's "cube.light" run
    // and composes it against the drivers' room (gain·I over a DARK
    // rest). THE MIRROR IS THE PRIOR — every reader in this file (the
    // newborn's dress, the swell, the projector) reads it here and none
    // of them reaches back into the coupling layer.
    //
    // choir_flushed is the poke gate: the last light each slot was
    // actually flushed at. Steady state pokes NOTHING, which is what
    // makes a per-frame projector affordable where the lattice's needed
    // a tick to hide behind. Seeded at birth by cube_write_gpu (which
    // writes the whole slot, light included) and reset by clear_cubes.
    float choir_I[CUBE_CHOIR_N]{};
    float choir_flushed[CUBE_CHOIR_N]{};

    // `cells[]`, `cell_scratch[]`, `wdir[][]`, `last_tick_beat` and
    // `primed` stood here (CHOIR_0 U5) — the lattice's own memory, its
    // diffusion scratch, its fixed-seed weight table and the clock they
    // ran on. The choir's state is choir_I above: 36 floats against
    // 252 cells + 252 scratch + 1008 weights, and no clock at all.
};

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════

// Spawn-side (stateless — consumed by entity_pipeline.hpp's cube_write_gpu)
void apply_cube_tier_gains(float& spring_stiffness, float& drag, uint32_t tier_idx);
uint32_t pick_cube_behavior_for_spawn(uint32_t seed);
// Teardown owner-clear
void clear_cubes(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue);  // DEPS-FORM PRECEDENT: explicit GPUState& param, born-converted
// The evictor — MachineCtx-shaped
// to match the FAMILY_DISPATCH evict slot (table in cartridge.hpp, post-class)
// `evict_cube` stood here — the CUBE family's patch-death evictor. Its one
// reach was FamilyDispatch::evict_slot, which left at ONE_SURFACE-I U3
// with the patch-death sweep that was its only caller.
// Dispatch funnels (table-shaped; defined below beside the recipe)
bool dispatch_select_cube_generic(MachineCtx* self, int32_t gx, int32_t gz, EntityQueueEntry& e);
bool dispatch_place_cube_generic(MachineCtx* self, EntityQueueEntry& e, PlacementEntry& pe);
void dispatch_commit_cube_generic(MachineCtx* self, PlacementEntry& pe, wgpu::Queue& queue);
// Player commands
void cycle_cube_behavior_override(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue);
void reveal_zoetrope(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue);
uint32_t set_cube_kite(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue, bool on);  // the ONE kite home (G3)
void toggle_cube_kite_mode(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue);
// Per-frame
void reconcile_cube_mirror(CubeBehaviorsState& cs, CubeDeps* c, const GPUFloatingEntityState* data);
// The zoetrope — the FORMATION machine's per-frame service. `zoetrope_
// strike` stood beside it (CHOIR_0 U5) and went with the lattice it fed;
// the service survives lighter, carrying the reseat watch and the climb
// and nothing else, so it no longer needs a musical clock or a world
// seed to do either.
void zoetrope_service(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue,
    float dt, float point_x, float point_z);
// THE PROJECTOR — one home: the light reaches pixels here and nowhere
// else. `zoetrope_cell_intensity`, `project_cell_color` and `zoetrope_
// project_slot` stood here and are superseded by these; the seed
// recompute survives the rename intact as choir_slot_seed. choir_light
// is I's ONE computation (the G6 door, inherited whole).
uint32_t choir_slot_seed(const CubeBehaviorsState& cbs, uint32_t active_seed, uint32_t slot);
float choir_light(const CubeBehaviorsState& cbs, uint32_t slot);
void choir_project_color(const CubeBehaviorsState& cbs, uint32_t active_seed, uint32_t slot,
    float& out_r, float& out_g, float& out_b);
void choir_project(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue,
    uint32_t active_seed);

// ═══ IMPL:
// rows deref cube_state(own) + time/world via MachineCtx; corral/kite
// read AgentState + player_ via CubeDeps. COHORT: after agents (AgentState)
// + entity_pipeline (generic_*) + spawn_engine (preamble) + sky/state.

// Apply tier gains to base substrate values. Called by cube_write_gpu
// during spawn — the result is what gets stored on the cube.
inline void apply_cube_tier_gains(float& spring_stiffness, float& drag, uint32_t tier_idx) {
    if (tier_idx >= CUBE_TIER_COUNT) return;  // defensive; tier_idx is bounded at select
    const auto& g = CUBE_TIER_GAINS[tier_idx];
    spring_stiffness *= g.spring_stiffness_mult;
    drag             *= g.drag_mult;
}

//
inline uint32_t pick_cube_behavior_for_spawn(uint32_t seed) {
    const auto& pop = CUBE_LIVE;

    float total = 0.0f;
    for (uint32_t i = 0; i < CUBE_BEHAVIOR_COUNT; i++) total += pop.behavior_weights[i];
    if (total <= 0.0f) return CUBE_BEHAVIOR_STATIONARY;

    uint32_t h = cpu_hash(seed, 0xBEEF11A0u);
    float r = (float(h) / 4294967296.0f) * total;

    float acc = 0.0f;
    for (uint32_t i = 0; i < CUBE_BEHAVIOR_COUNT; i++) {
        acc += pop.behavior_weights[i];
        if (r < acc) return i;
    }
    // Numerical edge case (r == total exactly): return last non-zero.
    for (uint32_t i = CUBE_BEHAVIOR_COUNT; i > 0; i--) {
        if (pop.behavior_weights[i - 1] > 0.0f) return i - 1;
    }
    return CUBE_BEHAVIOR_STATIONARY;  // unreachable; total > 0 was checked
}

// ═══ DIAGNOSTICS ═════════════════════════════════════════════════
//
// ─── Coordination cycle ─────────────────────────────────────────────
//
// ─── Behavior override ──────────────────────────────────────────────
//
// ─── Corral / kite (the anchor law) ────────────────────────────────
//
// The CPU authors TARGETS only; update_cube walks the live param
// toward them (CUBE_GLIDE_TAU). Mode switches ride the follow_pawn
// sentinels (3u kite-capture / 2u kite-release) — captured in-kernel
// from the true present, position-preserving even under drift.

// DEPS-FORM PRECEDENT: explicit GPUState& parameter —
// the deps form's first citizen; not a MachineCtx bypass.
inline void clear_cubes(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        cbs.activeCubes_[i] = ActiveCube{};
        GPUFloatingEntityState empty{};
        gpu.upload_cube_entity_slot(queue, i, empty);
    }
    // THE FORMATION MACHINE RESETS WITH THE WORLD (K1 E5). A portal taken
    // mid-screen used to carry a phantom formation into the new world,
    // where the reseat watch narrated a screen that was not there and the
    // seat pass sprayed ring offsets at strangers.
    //
    // THE CHOIR'S LIGHT IS WORLD-AGNOSTIC and is not reset here: it is
    // the MUSIC's state, not the world's, and it is re-mirrored from the
    // live signal every frame regardless. Only the POKE GATE resets —
    // the slots it shadowed have just been wiped on the GPU, so every
    // shadow it holds is now a lie about an empty slot.
    cbs.formation     = CubeBehaviorsState::Formation::ROAM;
    cbs.stations_sent = false;
    cbs.stage_wait    = false;
    for (uint32_t i = 0; i < CUBE_CHOIR_N; i++) cbs.choir_flushed[i] = 0.0f;
    for (uint32_t i = 0; i < LATTICE_CELLS; i++) {
        cbs.settled[i] = false;
        cbs.walk_[i]   = {};
    }
}

inline void apply_cube_behavior_override(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue) {
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        if (!cbs.activeCubes_[i].active) continue;
        c->gpuState_.upload_cube_behavior_id(queue, i, cbs.behavior_override);
    }
}

inline void cycle_cube_behavior_override(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue) {
    cbs.behavior_override = (cbs.behavior_override + 1) % CUBE_BEHAVIOR_COUNT;
    apply_cube_behavior_override(cbs, c, queue);
    std::cout << "[Floaters] cube behavior: "
              << CUBE_BEHAVIOR_NAMES[cbs.behavior_override] << "\n";
}

// ── The zoetrope's stations (C6R) — the ONE home for geometry ───
// THE HELIX (G2): cell = slot·37 mod 252 — consecutive spawns land one
// column over, one row up; the screen densifies with the population.
// The stride is coprime to the lattice, so slot↔cell is a bijection
// (UNSTRIDE is its inverse); every row is reachable at any count.
inline uint32_t cell_of_slot(uint32_t s) { return (s * ZOETROPE_CELL_STRIDE)   % LATTICE_CELLS; }
// `slot_of_cell` — the inverse crossing — stood here (CHOIR_0 U5). The
// strike was its only caller: a lattice cell had to name the slot it
// pokes, so it needed the map read backwards. Nothing reads a cell now,
// so the crossing is one-way and only the SEATING direction survives.
// BOTH CONSTANTS STAY, and so does the inverse static_assert above — the
// helix is still a bijection and that assert is still what proves it;
// it is simply not walked backwards at runtime any more.

// ── THE TWO STATIONS, side by side (H1b) ────────────────────────
// SCATTER is the flock drawn in: own bodies, own altitudes, deep
// jitter. SCREEN is the instrument: uniform pixels, ranked rows,
// tight jitter. One function each; the cycle chooses.
//
// Both seat from the same construction — cell = cell_of_slot(slot),
// theta from that cell's column — so a cube keeps its angular
// identity across the whole cycle, and only its reach, its rank and
// its body change between the two.
//
// THE SEATING LAW (K2): two cubes may share a column; nothing may
// share a bearing. The screen ranks its seven rows on one bearing per
// column and separates them vertically, so a shared bearing is the
// point there. The gathering has no ranks to separate it, so it
// scatters the bearing itself (JITTER_THETA, most of a column arc) and
// separates by BODY as well — SIZE_BIAS seats big cubes outward and
// small ones close, which reads as depth rather than as a ring.
struct ZoetropeStation { float off_x; float off_z; float h; };

inline ZoetropeStation zoetrope_station(uint32_t slot) {
    const uint32_t cell = cell_of_slot(slot);
    const uint32_t row = cell / LATTICE_COLS;
    const uint32_t col = cell % LATTICE_COLS;
    const float two_pi = 6.28318530718f;
    const float column_arc = two_pi / float(LATTICE_COLS);
    // THE LOOSENING (V4): three fixed-seed offsets, the same grammar the
    // scatter uses, on their own seed so the two seatings never share a
    // hash — a cube's screen seat and its gathering seat are independent
    // draws. All three jitters at zero give back the machined ring
    // exactly; raised, the screen reads as hand-placed rather than
    // milled, without any seat leaving its own cell's neighbourhood.
    const float jt = (cpu_hash_f(ZOETROPE_STATION_SEED, cell * 3u + 2u) - 0.5f) * 2.0f;
    const float jr = (cpu_hash_f(ZOETROPE_STATION_SEED, cell * 3u)      - 0.5f) * 2.0f;
    const float jh = (cpu_hash_f(ZOETROPE_STATION_SEED, cell * 3u + 1u) - 0.5f) * 2.0f;
    const float theta  = column_arc * float(col) + jt * column_arc * ZOETROPE_JITTER_THETA;
    const float radius = ZOETROPE_RING_RADIUS * (1.0f + jr * ZOETROPE_JITTER_R);
    const float h      = ZOETROPE_H_BASE + float(row) * ZOETROPE_H_STEP
                       + jh * ZOETROPE_H_STEP * ZOETROPE_JITTER_H;
    return { std::cos(theta) * radius, std::sin(theta) * radius, h };
}

// The gathering's seat. Fixed-seed jitters (ZOETROPE_SCATTER_SEED), so
// re-pressing lands every cube on the same seat it left — the flock has
// a shape, not a shuffle.
inline ZoetropeStation station_scatter(const CubeBehaviorsState& cbs, uint32_t slot) {
    const uint32_t cell = cell_of_slot(slot);
    const uint32_t col  = cell % LATTICE_COLS;
    const float two_pi = 6.28318530718f;
    const float column_arc = two_pi / float(LATTICE_COLS);   // radians per column
    // Angular jitter (K2): the bearing itself scatters, so the seven
    // cells of a column stop reading as a radial string.
    const float jt = (cpu_hash_f(ZOETROPE_SCATTER_SEED, cell * 3u + 2u) - 0.5f) * 2.0f;
    const float theta = column_arc * float(col) + jt * column_arc * ZOETROPE_SCATTER_JITTER_THETA;
    // Deep radial jitter — this is a flock, not a ring — plus the SIZE
    // BIAS: a big body seats outward, a small one close, so bodies never
    // contend for the same neighbourhood and the depth reads.
    const float jr = (cpu_hash_f(ZOETROPE_SCATTER_SEED, cell * 3u) - 0.5f) * 2.0f;
    const float radius = ZOETROPE_SCATTER_RADIUS * (1.0f + jr * ZOETROPE_SCATTER_JITTER_R)
                       + ZOETROPE_SCATTER_SIZE_BIAS * cbs.activeCubes_[slot].body_radius;
    // Free vertical spread around the cube's OWN spawn altitude: the
    // flock keeps its altitude character — it is gathered, not ranked.
    // Floored at the screen's own base so no gathered cube sinks in.
    const float jh = (cpu_hash_f(ZOETROPE_SCATTER_SEED, cell * 3u + 1u) - 0.5f) * 2.0f;
    const float h = std::max(ZOETROPE_H_BASE,
        cbs.activeCubes_[slot].orbit_height + jh * ZOETROPE_SCATTER_JITTER_H);
    return { std::cos(theta) * radius, std::sin(theta) * radius, h };
}


inline void reveal_zoetrope(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue) {
    using Formation = CubeBehaviorsState::Formation;

    // F6 IS THE CYCLE (H1): roam → scatter → screen → roam. One door,
    // three destinations; F7 never touches it. A press MID-WALK counts as
    // its destination's press — the cycle advances and the walk simply
    // re-aims, because the targets are read from the state every frame.
    Formation next;
    const char* line;
    switch (cbs.formation) {
        case Formation::ROAM:
        case Formation::TO_ROAM:
            next = Formation::TO_SCATTER;
            line = "[Zoetrope] gather: the flock draws in";    break;
        case Formation::SCATTERED:
        case Formation::TO_SCATTER:
            next = Formation::TO_SCREEN;
            line = "[Zoetrope] reveal: the screen assembles";  break;
        default:   // SCREEN | TO_SCREEN
            next = Formation::TO_ROAM;
            line = "[Zoetrope] release: the swarm walks home"; break;
    }

    // F6 CHOOSES SHAPE; F7 CHOOSES WHETHER IT FOLLOWS (K1). The reveal
    // touches the kite NOWHERE — the seat pass has two arms, so a
    // formation seats correctly whether the flock follows the point or
    // stands planted in the world.
    //
    // STAGED (V1): the sentinel eats target_x/z in the frame it is
    // consumed, so the press writes NO targets and NO bodies — it stages,
    // and the service walks from the next frame. Nothing new is recorded
    // either: THE MIRROR IS THE PRIOR (G5). The walk re-arms from its own
    // live shadows; only on the first press out of ROAM are those shadows
    // seeded from the mirror, because in ROAM the GPU scalars ARE the
    // mirror's draws — nothing has walked them yet.
    const bool from_roam = (cbs.formation == Formation::ROAM);
    // Leaving a STANDING screen, the swell (V2) owns the live radius
    // while the walk's shadow still reads the bare pixel — so a lit cube
    // would snap down the instant the walk resumed. Seed the shadow from
    // what the eye is actually seeing: nothing teleports, including out
    // of a gesture.
    const bool from_screen = (cbs.formation == Formation::SCREEN);
    uint32_t staged = 0;
    for (uint32_t i = 0; i < LATTICE_CELLS; i++) {
        if (!cbs.activeCubes_[i].active) continue;
        const ActiveCube& ac = cbs.activeCubes_[i];
        if (from_roam)
            cbs.walk_[i] = { ac.orbit_height, ac.body_radius, ac.aspect_y, ac.aspect_z };
        else if (from_screen)
            cbs.walk_[i].r = ZOETROPE_PIXEL_RADIUS
                * (1.0f + ZOETROPE_SWELL_GAIN * choir_light(cbs, i));
        cbs.settled[i] = false;
        staged++;
    }

    cbs.formation = next;
    cbs.stations_sent = false;
    cbs.stage_wait = true;      // this frame stages; the service acts next frame
    // THE TWO DIM EDGES (V1 E3) — the only two transitions that change
    // the rest brightness: entering TO_SCREEN turns the dim ON, entering
    // TO_ROAM turns it OFF. (TO_SCATTER is reachable only from ROAM and
    // TO_ROAM, both already undimmed, and the settles stay inside their
    // own band — so no other edge moves it.)
    if (next == Formation::TO_SCREEN || next == Formation::TO_ROAM)
        cbs.repaint_all = true;

    std::cout << line << " (" << staged << " cube(s))\n";
}

// ─── Kite mode toggle (F7) ──────────────────────────────────────

// THE ONE KITE HOME (G3): flag write + sentinel loop, extracted whole
// from the toggle; no print — callers narrate. DEPS-FORM PRECEDENT
// (clear_cubes): explicit GPUState& param, so the spine's service can
// call it without a deps face. Returns the affected count.
//   ON  → 3u kite-capture: offset := the true present
//         (pos − point − drift); target := offset.
//   OFF → 2u kite-release: anchor := current pos; target :=
//         anchor; drift zeroed.
// Both xz-position-preserving even under drift — the capture
// happens where drift lives. (Release clears drift.xz only; the
// vertical WALKS home on the existing spring/drag rather than
// snapping. Capture is xz-exact to f32.)
inline uint32_t set_cube_kite(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue, bool on) {
    cbs.kite_mode = on;
    const uint32_t sentinel = on ? 3u : 2u;
    uint32_t affected = 0;
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        if (!cbs.activeCubes_[i].active) continue;
        gpu.upload_cube_follow_pawn(queue, i, sentinel);
        affected++;
    }
    return affected;
}

inline void toggle_cube_kite_mode(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue) {
    // F7 CHANGES THE FRAME, NOT THE SHAPE (K1). Fully live in every
    // state: the flag, the sentinel to every active cube, the print —
    // exactly as before the borrow existed. A standing formation simply
    // re-seats itself in the new coordinate frame; the sentinel must eat
    // before that re-seat, so the stage frame is re-armed with it.
    const uint32_t affected = set_cube_kite(cbs, c->gpuState_, queue, !cbs.kite_mode);

    std::cout << "[Floaters] kite mode: " << (cbs.kite_mode ? "ON" : "OFF")
              << " (" << affected << " cube(s))\n";

    if (cbs.formation != CubeBehaviorsState::Formation::ROAM) {
        cbs.stations_sent = false;
        cbs.stage_wait    = true;
    }
}

// ═══ THE EVICTOR ══════════════════════════════════════════════════


// ═══ THE CUBE RECIPE ══════════════════════════════════════════════
//
// Tier tables, traits, adapter, and dispatch funnels — beside the
// evictor. Funnels declared above; the FAMILY_DISPATCH rows
// (cartridge.hpp, post-class) point here.

// ═══ FAMILY: CUBE ═════════════════════════════════════════════════

struct CubeIdx {
    static constexpr uint32_t BODY_RADIUS      = 0;
    static constexpr uint32_t ORBIT_HEIGHT     = 1;
    static constexpr uint32_t INFLUENCE_RADIUS = 2;
    static constexpr uint32_t SPIN_SPEED       = 3;
    static constexpr uint32_t BOB_AMPLITUDE    = 4;
    static constexpr uint32_t BOB_PERIOD       = 5;
    static constexpr uint32_t ASPECT_Y         = 6;
    static constexpr uint32_t ASPECT_Z         = 7;
    static constexpr uint32_t FACE_VARIANCE    = 8;
    static constexpr uint32_t COUNT            = 9;
};

inline constexpr TierParamDef CUBE_PARAM_DEFS[] = {
    { CubeProp::BODY_RADIUS,      0.5f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeProp::ORBIT_HEIGHT,     3.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeProp::INFLUENCE_RADIUS, 3.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeProp::SPIN_SPEED,       0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeProp::BOB_AMPLITUDE,    0.0f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeProp::BOB_PERIOD,       0.5f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeProp::ASPECT_Y,         0.2f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeProp::ASPECT_Z,         0.1f, 1e30f, false, ParamDist::GAUSSIAN },
    { CubeProp::FACE_VARIANCE,    0.0f, 1e30f, false, ParamDist::GAUSSIAN },
};
inline constexpr uint32_t CUBE_PARAM_COUNT = sizeof(CUBE_PARAM_DEFS) / sizeof(TierParamDef);
static_assert(CUBE_PARAM_COUNT == CubeIdx::COUNT,
    "F-4: CUBE_PARAM_DEFS must cover CubeIdx exactly (row order IS the index)");

// params[] order MUST match CUBE_PARAM_DEFS:
//   [0]BODY_RADIUS [1]ORBIT_HEIGHT [2]INFLUENCE_RADIUS [3]SPIN_SPEED
//   [4]BOB_AMPLITUDE [5]BOB_PERIOD [6]ASPECT_Y [7]ASPECT_Z [8]FACE_VARIANCE
struct CubeTierRow {
    TierProfile profile;
    float       spin_tilt_sigma;
};

// ── Cube tier table ────────────────────────────────────────────────
// Row = cube tier index (0 SmallCube / 1 MedCube / 2 LargeCube /
// 3 Monolith — plain index, no enum class; CUBE_TIER_COUNT pinned in
// floaters.hpp). Each row = { weight, color_var, { 9 {μ,σ}
// pairs in CubeIdx order:
//   BODY_RADIUS ORBIT_HEIGHT INFLUENCE_RADIUS SPIN_SPEED BOB_AMPLITUDE
//   BOB_PERIOD ASPECT_Y ASPECT_Z FACE_VARIANCE } }, spin_tilt_sigma.
// UNITS: radii/height/amplitude = wu; SPIN_SPEED = rad/s; BOB_PERIOD =
//   s; ASPECT_Y/Z / FACE_VARIANCE = multipliers; spin_tilt_sigma =
//   radians. CONSUMERS: cube_get_tier_profile (generic sampling);
//   spin_tilt_sigma at cube write_gpu.
// BOB_PERIOD σ (TEMPO_0): held at CV ≈ 0.15. Bob frequency is 1/period, so a
// wide σ is asymmetric in the percept — −1σ speeds a cube more than +1σ slows
// it, and the low tail ran into the 0.5 s CUBE_PARAM_DEFS floor. A narrow σ
// keeps the Gaussian locally linear in frequency. For a slower field, raise μ;
// do not widen σ.
// SIZE σ (SCALE_0): BODY_RADIUS / ASPECT_Y / ASPECT_Z held at 2/3 of their
// authored spread — ±3σ of these now spans ±2σ of the original. These three are
// INDEPENDENT draws that MULTIPLY into the silhouette (monolith_vs scales by
// vec3(r, r·aspect_y, r·aspect_z)), so their variances compound rather than
// average. Widen one and the height range grows with the product, not the sum.
// Biography determinant — frozen biography (§12).
inline constexpr CubeTierRow CUBE_TIERS[CUBE_TIER_COUNT] = {
    /* 0: SmallCube */ {
        { 0.40f, 0.0f, { {1.8f, 0.33f}, {25.0f, 20.0f}, {6.0f, 1.5f},  {0.04f, 0.015f},
                   {1.0f, 0.3f}, {5.0f, 0.6f},
                   {1.0f, 0.10f}, {1.0f, 0.10f}, {0.18f, 0.06f} }},
        0.12f
    },
    /* 1: MedCube   */ {
        { 0.32f, 0.0f, { {4.0f, 0.8f}, {45.0f, 30.0f}, {10.0f, 2.0f}, {0.03f, 0.01f},
                   {1.5f, 0.4f}, {6.0f, 0.9f},
                   {1.0f, 0.13f}, {1.0f, 0.13f}, {0.20f, 0.07f} }},
        0.10f
    },
    /* 2: LargeCube */ {
        { 0.20f, 0.0f, { {8.0f, 1.67f}, {75.0f, 45.0f}, {14.0f, 3.0f}, {0.02f, 0.008f},
                   {2.0f, 0.5f}, {8.0f, 1.4f},
                   {1.0f, 0.17f}, {1.0f, 0.17f}, {0.16f, 0.05f} }},
        0.08f
    },
    /* 3: Monolith  */ {
        { 0.08f, 0.0f, { {3.0f, 0.53f}, {12.0f, 8.0f}, {12.0f, 3.0f}, {0.015f, 0.005f},
                   {1.2f, 0.3f}, {6.0f, 0.9f},
                   {5.0f, 0.80f}, {0.15f, 0.02f}, {0.20f, 0.06f} }},
        0.10f
    },
};

inline const TierProfile& cube_get_tier_profile(uint32_t tier_idx) {
    return CUBE_TIERS[tier_idx].profile;
}

inline constexpr EntityFamilyTraits CUBE_TRAITS = {
    PopFamily::CUBE, CUBE_CHOIR_N,   // THE CHOIR is the living ceiling now (the lattice's 252 was); capacity stays 256
    false,                // NOT grounded — hovers and drifts; claims no ground (ruling 21)
    CubeProp::SPAWN_ROLL, CubeConfig::SPAWN_CHANCE,
    CubeConfig::POSITION_JITTER,
    CUBE_TIER_COUNT, CubeProp::TIER,
    CUBE_PARAM_DEFS, CUBE_PARAM_COUNT,
    CubeProp::ANCHOR_X, CubeProp::ANCHOR_Z, CubeProp::ROTATION,
    0, nullptr,
};

inline SpawnGateOutput cube_run_gate(MachineCtx* c, int32_t gx, int32_t gz) {
    return gate_from_traits(c, gx, gz, CUBE_TRAITS, c->cube_behaviors_state_.activeCubes_);
}

inline void cube_compute_solid_half(EntityInstance& inst, const TierProfile&) {
    inst.solid_half = inst.params[CubeIdx::BODY_RADIUS];
    inst.ground_y_offset = 0.0f;
    inst.burial = 0.0f;
}

inline void cube_compute_colors(EntityInstance& inst, const EntityFamilyTraits&, const TierProfile& /*tier*/) {
    inst.colors[0] = cpu_hash_f(inst.seed, CubeProp::COLOR_R) * 0.55f + 0.35f;
    inst.colors[1] = cpu_hash_f(inst.seed, CubeProp::COLOR_G) * 0.50f + 0.30f;
    inst.colors[2] = cpu_hash_f(inst.seed, CubeProp::COLOR_B) * 0.60f + 0.20f;
}

inline void cube_write_active(MachineCtx* c, const EntityInstance& inst) {
    auto& ac = c->cube_behaviors_state_.activeCubes_[inst.slot];
    ac.patch_gx = inst.trigger_gx; ac.patch_gz = inst.trigger_gz;
    ac.host_gx = inst.host_gx; ac.host_gz = inst.host_gz;
    ac.last_alloc_time = c->time_state_.seconds;
    ac.orbit_height  = inst.params[CubeIdx::ORBIT_HEIGHT];    // the tier draws — THE MIRROR IS THE
    ac.body_radius   = inst.params[CubeIdx::BODY_RADIUS];     // PRIOR (C6R V2 + G5): release walks
    ac.aspect_y      = inst.params[CubeIdx::ASPECT_Y];        // every possessed scalar back here
    ac.aspect_z      = inst.params[CubeIdx::ASPECT_Z];
    ac.face_variance = inst.params[CubeIdx::FACE_VARIANCE];
    ac.active = true;
}

inline void cube_write_gpu(MachineCtx* c, const EntityInstance& inst, wgpu::Queue& queue) {
    // Spin tilt: custom derivation from tier constant (not a sampled param)
    float tilt_sigma = CUBE_TIERS[inst.tier_idx].spin_tilt_sigma;
    float tilt_x = (cpu_hash_f(inst.seed, CubeProp::SPIN_TILT_X) - 0.5f) * 2.0f * tilt_sigma;
    float tilt_z = (cpu_hash_f(inst.seed, CubeProp::SPIN_TILT_Z) - 0.5f) * 2.0f * tilt_sigma;

    GPUFloatingEntityState fe{};
    fe.anchor[0] = inst.cx; fe.anchor[1] = 0.0f; fe.anchor[2] = inst.cz;
    fe.body_radius = inst.params[CubeIdx::BODY_RADIUS];
    fe.orbit_radius = 0.0f;
    fe.orbit_height = inst.params[CubeIdx::ORBIT_HEIGHT];
    fe.orbit_speed = 0.0f;
    fe.influence_radius = inst.params[CubeIdx::INFLUENCE_RADIUS];
    fe.spin_speed = inst.params[CubeIdx::SPIN_SPEED];
    fe.bob_amplitude = inst.params[CubeIdx::BOB_AMPLITUDE];
    fe.bob_period = inst.params[CubeIdx::BOB_PERIOD];
    fe.spin_tilt_x = tilt_x; fe.spin_tilt_z = tilt_z;
    fe.base_color[0] = inst.colors[0]; fe.base_color[1] = inst.colors[1]; fe.base_color[2] = inst.colors[2];
    // THE CHOIR (U4): a cube BORN MID-NOTE IS BORN LIT. The key's light
    // is already standing in the mirror, so the newborn dresses from it
    // rather than waking dark and catching up (write_active has already
    // seated the mirror, so the slot's seed recomputes; at I = 0 this is
    // inst.colors bit-exactly — the silent path is today's).
    choir_project_color(c->cube_behaviors_state_, c->world_state_.active_seed, inst.slot,
                        fe.color[0], fe.color[1], fe.color[2]);
    fe.aspect_y = inst.params[CubeIdx::ASPECT_Y];
    fe.aspect_z = inst.params[CubeIdx::ASPECT_Z];
    // THE CHOIR (G6, inherited): the same law the projector runs, at
    // birth — GLOW UNIFIES, so the tier draw is CLOSED by the light
    // rather than splayed by it. The mirror keeps the bare draw
    // (write_active); this is the projected face, and at I = 0 it is
    // that draw to the bit.
    fe.face_variance = inst.params[CubeIdx::FACE_VARIANCE]
                     * (1.0f - choir_light(c->cube_behaviors_state_, inst.slot));
    fe.geometry_type = 1; fe.motion_type = 1;
    fe.entity_seed = Dim::CUBE_SLOT_OFFSET + inst.slot;
    fe.t = 0.0f; fe.orientation[3] = 1.0f;
    fe.pos[0] = inst.cx; fe.pos[1] = fe.orbit_height; fe.pos[2] = inst.cz;
    fe.is_active = 1;
    //
    fe.spring_stiffness = CUBE_DEFAULT_SPRING_STIFFNESS;
    fe.drag             = CUBE_DEFAULT_DRAG;
    fe.tier_idx = inst.tier_idx;
    apply_cube_tier_gains(fe.spring_stiffness, fe.drag, inst.tier_idx);
    // CONTACT_2 C1b: bake the tier's plasticity λ per-instance (no GPU
    // cube-tier array + no new bindings this batch — rides the fe pad).
    fe.plasticity = (inst.tier_idx < CUBE_TIER_COUNT)
                        ? CUBE_TIER_GAINS[inst.tier_idx].plasticity : 0.0f;
    fe.drift[0] = 0.0f; fe.drift[1] = 0.0f; fe.drift[2] = 0.0f;
    fe.drift_vel[0] = 0.0f; fe.drift_vel[1] = 0.0f; fe.drift_vel[2] = 0.0f;
    fe.behavior_id    = pick_cube_behavior_for_spawn(inst.seed);
    fe.behavior_phase = cpu_hash(inst.seed, 0xF10A7E70u);
    // BIRTH INTO THE LIVE MODE (RIDER[cube:spawn-mode-desync]).
    // Per-cube mode truth lives on the GPU; kite_mode is one CPU flag
    // for the flock. A newborn left in anchor mode while the flock is
    // kited would be handed RING OFFSETS by the next corral — and a
    // mode-0 cube walks its ANCHOR toward its target, reading those
    // offsets as absolute world coordinates: a smooth glide to a ring
    // around the WORLD ORIGIN. So a newborn joins the mode that is
    // live at its spawn.
    //
    // Drift is exactly zero at birth (set below), so the offset is
    // exact — no sentinel round-trip, no capture frame. This is the
    // ONE init home for target in both arms: at rest target == param,
    // so update_cube's glide term is exactly zero either way.
    // ZOETROPE (C6R E7 + G5 + K1): a newborn under a standing formation
    // wears its cell's LOOK — row height, and PIXEL SCALE for the
    // screen; the gathering's leaves the body its own spawn draw. The
    // mirror keeps its true tier draws (write_active), so the walk home
    // scatters it as if it had always roamed.
    //
    // THE SPAWN LAW (Jean, non-negotiable): a cube spawns at its
    // DESIGNATED PATCH, outside the render radius — no matter what.
    // The seat therefore authors APPEARANCE ONLY. Both arms previously
    // relocated a newborn to point + station (kite via pawn_offset,
    // anchor via an absolute seat), which put newborns in the lattice
    // beside the player the moment a formation stood; that is the
    // violation, and it is gone. A newborn born under a formation
    // stands at its patch wearing the cohort's look and joins the
    // lattice on the next cycle — the service owns the travel, birth
    // does not teleport.
    using Formation = CubeBehaviorsState::Formation;
    const auto formation = c->cube_behaviors_state_.formation;
    const bool born_to_screen  = (formation == Formation::TO_SCREEN
                               || formation == Formation::SCREEN);
    const bool born_to_scatter = (formation == Formation::TO_SCATTER
                               || formation == Formation::SCATTERED);
    const bool born_seated = (born_to_screen || born_to_scatter);
    ZoetropeStation st{};
    if (born_seated) {
        st = born_to_screen ? zoetrope_station(inst.slot)
                            : station_scatter(c->cube_behaviors_state_, inst.slot);
        fe.orbit_height = st.h;
        if (born_to_screen) {
            // THE SWELL DRESSES THE NEWBORN TOO, and only where it has
            // jurisdiction: under a STANDING screen the swell owns the
            // radius, so a cube born into a held chord is born swollen
            // exactly as it is born lit. Under TO_SCREEN the WALK owns it
            // and the newborn takes the bare pixel, which is the target
            // the walk is already carrying every other cube toward.
            fe.body_radius = (formation == Formation::SCREEN)
                ? ZOETROPE_PIXEL_RADIUS * (1.0f + ZOETROPE_SWELL_GAIN
                    * choir_light(c->cube_behaviors_state_, inst.slot))
                : ZOETROPE_PIXEL_RADIUS;
            fe.aspect_y    = 1.0f;
            fe.aspect_z    = 1.0f;
        }
    }

    if (c->cube_behaviors_state_.kite_mode) {
        // Kite arm: the param is the OFFSET from the point, and the
        // point is point_.x/z — the same host-authored snapshot the
        // seat pass rings around.
        // THE SPAWN LAW: the offset is the one that PRESERVES the
        // patch position (inst.cx/cz), formation or not.
        fe.follow_pawn = 1u;
        fe.pawn_offset[0] = inst.cx - c->point_.x;
        fe.pawn_offset[1] = 0.0f;
        fe.pawn_offset[2] = inst.cz - c->point_.z;
        fe.target_x = fe.pawn_offset[0];
        fe.target_z = fe.pawn_offset[2];
    } else {
        // Anchor arm: the param is anchor.xz, written above from the
        // spawn position — and it STAYS that, formation or not (THE
        // SPAWN LAW). K1's absolute-seat arm is retired: it planted
        // newborns where the point stood.
        fe.follow_pawn = 0u;
        fe.pawn_offset[0] = 0.0f; fe.pawn_offset[1] = 0.0f; fe.pawn_offset[2] = 0.0f;
        fe.target_x = inst.cx;
        fe.target_z = inst.cz;
    }
    c->gpuState_.upload_cube_entity_slot(queue, inst.slot, fe);

    // ZOETROPE (C6R + G5): birth bookkeeping — the walker's shadows
    // mirror the scalars just written; a newborn is born settled — no
    // walk owed, in any formation state. The PRIOR needs no record
    // here: the mirror is the prior (write_active seated the draws).
    auto& zcbs = c->cube_behaviors_state_;
    zcbs.walk_[inst.slot] = { fe.orbit_height, fe.body_radius, fe.aspect_y, fe.aspect_z };
    zcbs.settled[inst.slot] = true;
    // THE POKE GATE IS BOOKKEEPING TOO: the whole-slot write above has
    // just flushed this slot at the live light, so the gate records it.
    // Without this a slot inheriting a stale shadow from its previous
    // tenant could sit one epsilon from the truth and never poke — the
    // one way a dark key could fail to relight.
    if (inst.slot < CUBE_CHOIR_N)
        zcbs.choir_flushed[inst.slot] = zcbs.choir_I[inst.slot];
}

// CUBE_INDOOR_RESCALE_PARAMS and its CAP policy note stood here —
// cube_apply_indoor_rescale's table, orphaned when U4 took that adapter
// slot. U7's orphan sweep; the same cut as the sphere's and the pyramid's.

inline constexpr EntityFamilyAdapter CUBE_ADAPTER = {
    cube_run_gate,
    cube_compute_solid_half, cube_compute_colors,
    cube_write_active, cube_write_gpu, nullptr,
    cube_get_tier_profile,
};

inline bool dispatch_select_cube_generic(MachineCtx* self, int32_t gx, int32_t gz, EntityQueueEntry& e) {
    EntityInstance inst{};
    if (!generic_select(self, CUBE_TRAITS, CUBE_ADAPTER, gx, gz, inst)) return false;
    e.family = PopFamily::CUBE; e.gx = gx; e.gz = gz; e.generic = inst; return true;
}
inline bool dispatch_place_cube_generic(MachineCtx* self, EntityQueueEntry& e, PlacementEntry& pe) {
    pe.family = e.family; pe.gx = e.gx; pe.gz = e.gz;
    if (generic_place(self, CUBE_TRAITS, e.generic)) { pe.generic = e.generic; return true; }
    self->cube_behaviors_state_.activeCubes_[e.generic.slot].active = false; return false;
}
inline void dispatch_commit_cube_generic(MachineCtx* self, PlacementEntry& pe, wgpu::Queue& queue) {
    auto* host = find_patch(self, pe.generic.host_gx, pe.generic.host_gz);
    if (host) {
        generic_commit(self, CUBE_TRAITS, CUBE_ADAPTER, pe.generic, queue);
        // Lifecycle Phase 2: cube lifetime decoupled from host patch.
        // See dispatch_commit_sphere_generic for the rationale.
    }
    else { self->cube_behaviors_state_.activeCubes_[pe.generic.slot].active = false; }
}


// ═══ THE ZOETROPE'S LATTICE SUBSTRATE STOOD HERE (CHOIR_0 U5) ═════
//
// A 7×36 cylinder automaton over the cube slots: two fields per cell
// (fast excitation diffusing one cell per tick on fixed-seed asymmetric
// weights, slow pigment integrating it on a long half-life), a stateless
// write head sweeping a column per revolution of the musical clock, and
// a strike that landed a note on its own cell and bled SPREAD into both
// column-neighbours because a note has width. The per-tick decay pair
// (`ZOETROPE_EXCITE_DECAY` / `ZOETROPE_PIGMENT_DECAY`, derived from the
// panel), the weight table's seed grammar (`ZOETROPE_WEIGHT_SEED`),
// `zoetrope_cell_intensity` — I's one computation, min(1, excite +
// WEIGHT·pigment) — `project_cell_color` and `zoetrope_project_slot` all
// lived here, and all of them are gone.
//
// WHAT REPLACED IT IS NOT A SMALLER AUTOMATON — it is no automaton. The
// lattice existed to turn seven row IMPULSES into a field that could
// spread and fade, because an impulse carries no duration. The choir
// reads PRESENCE instead of onsets, so duration arrives already in the
// signal and the only thing left to author is an envelope on it. The
// width the spread gave a note goes with it: a note lights the key it
// names and no other, which is what a keyboard is.
//
// `zoetrope_slot_seed` is the one thing that crossed over intact — the
// seed recompute, renamed choir_slot_seed and standing below.

// ═══ THE CHOIR'S PROJECTOR — ONE HOME ════════════════════════════
//
// The successor to the lattice's projector, and its whole inheritance:
// the base is RECOMPUTED through the seed fn from the slot's TRUE spawn
// seed (never cached — the gate drew tile_seed(active world seed,
// trigger patch) and the mirror keeps the trigger patch, so the seed
// reconstructs bit-exactly), the SCREEN dim survives, and the silent
// path is still bit-exact: at I = 0 the mix returns the seed colour and
// the variance returns the spawn draw, both to the last bit.
//
// WHAT CHANGED IS THE VARIANCE'S DIRECTION. The strike SPLAYED — it
// added face variance, because a struck cell was a cell disturbed. The
// light UNIFIES: a lit cube converges on one face, because incandescence
// is the body glowing through, not the surface breaking up. So variance
// is the spawn draw SCALED DOWN by the light rather than a rest scaled
// up plus a splay — which is also why no restore pass exists: the law is
// self-restoring at I = 0 and needs no separate return.

// I's ONE COMPUTATION (the G6 door, inherited). Every reader — the
// newborn's dress, the swell, the colour mix, the variance — reads the
// light through here and no other way. Slots past the choir are DARK by
// construction rather than by luck: the population cap keeps them
// unallocated, and this door keeps them silent even if one ever were.
inline float choir_light(const CubeBehaviorsState& cbs, uint32_t slot) {
    return (slot < CUBE_CHOIR_N) ? cbs.choir_I[slot] : 0.0f;
}

// THE SEED RECOMPUTE, inherited whole (only the name changed). The base
// is never cached: the gate drew tile_seed(active world seed, trigger
// patch) (machine/spawn_engine.hpp evaluate_spawn_gate) and the mirror
// keeps the trigger patch, so the seed reconstructs bit-exactly from
// what the slot already carries.
inline uint32_t choir_slot_seed(const CubeBehaviorsState& cbs, uint32_t active_seed, uint32_t slot) {
    const ActiveCube& ac = cbs.activeCubes_[slot];
    return tile_seed(active_seed, ac.patch_gx, ac.patch_gz);
}

inline void choir_project_color(const CubeBehaviorsState& cbs, uint32_t active_seed, uint32_t slot,
    float& out_r, float& out_g, float& out_b) {
    const float I = choir_light(cbs, slot);
    EntityInstance tmp{};
    tmp.seed = choir_slot_seed(cbs, active_seed, slot);
    // The seed fn's exact signature takes traits + tier; it reads neither
    // (both unnamed) — the call adapts, the law does not. G5 V2 verdict:
    // profile-INVARIANT — no profile field is consulted (and CUBE_TIERS'
    // color_var column is 0.0 in every row), so profile(0) is bit-exact
    // for every tier.
    cube_compute_colors(tmp, CUBE_TRAITS, cube_get_tier_profile(0));
    // THE DARK REST (V1): the instrument is dark until played. The base
    // dims in the SCREEN states ONLY — a lit rock face spends the whole
    // of I on a tint nobody can see, so the screen makes room for the
    // music first. ROAM and the gathering keep the world's own swarm at
    // full brightness; dimming those would darken the world, not an
    // instrument. At I = 1 the destination is the light either way, so
    // the dim costs the gesture nothing — it only lowers the floor.
    using Formation = CubeBehaviorsState::Formation;
    const bool screen = (cbs.formation == Formation::SCREEN
                      || cbs.formation == Formation::TO_SCREEN);
    const float dim = screen ? ZOETROPE_REST_DIM : 1.0f;
    const float br = tmp.colors[0] * dim;
    const float bg = tmp.colors[1] * dim;
    const float bb = tmp.colors[2] * dim;
    const float* lc = DRIVER_LIVE.cube.light_color;
    out_r = br + (lc[0] - br) * I;
    out_g = bg + (lc[1] - bg) * I;
    out_b = bb + (lc[2] - bb) * I;
}

// THE ONE POKE HOME: every projector write for a slot goes through here,
// so no two paths can disagree about what a lit cube looks like.
//
// COLOUR AND VARIANCE POKE IN EVERY FORMATION STATE, ROAM INCLUDED —
// the light is the music's, not the screen's, and a roaming cube is as
// entitled to it as a seated one. (The lattice's projector returned
// early in ROAM because its variance term was a REST MULTIPLIER that
// only made sense in formation; this one's is self-restoring, so there
// is nothing to hold back.) The SWELL is the exception and keeps its old
// jurisdiction exactly: the walk owns body_radius during every TO_*
// state, so the swell speaks only when the screen STANDS.
inline void choir_project_slot(const CubeBehaviorsState& cbs, GPUState& gpu,
    wgpu::Queue& queue, uint32_t active_seed, uint32_t slot) {
    float cr, cg, cb;
    choir_project_color(cbs, active_seed, slot, cr, cg, cb);
    gpu.upload_cube_color(queue, slot, cr, cg, cb);

    // GLOW UNIFIES: the spawn draw is the REST and the light closes it.
    // At I = 0 this is the mirror's bare draw to the bit — the silent
    // path — and at I = 1 one flat face.
    const float I = choir_light(cbs, slot);
    gpu.upload_cube_face_variance(queue, slot,
        cbs.activeCubes_[slot].face_variance * (1.0f - I));

    using Formation = CubeBehaviorsState::Formation;
    if (cbs.formation == Formation::SCREEN)
        gpu.upload_cube_body_radius(queue, slot,
            ZOETROPE_PIXEL_RADIUS * (1.0f + ZOETROPE_SWELL_GAIN * I));
}

// THE PER-FRAME FLUSH, POKE-ON-CHANGE. The lattice's flush hid behind a
// tick (0.25 beats); the choir has no tick to hide behind, so the gate is
// the light itself: a slot pokes only when its light MOVED past epsilon.
// A silent room pokes nothing at all, a sustained chord pokes only while
// it climbs, and the release pokes for exactly light_release beats. The
// repaint edge (V1 E3) rides through as a FORCE — the two formation
// transitions that move the dim change what a cube looks like without
// moving its light, so they cannot be gated on the light.
inline void choir_project(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue,
    uint32_t active_seed) {
    const bool force = cbs.repaint_all;
    for (uint32_t slot = 0; slot < CUBE_CHOIR_N; ++slot) {
        if (!cbs.activeCubes_[slot].active) continue;
        const float I = choir_light(cbs, slot);   // the one door, here too
        if (!force && std::fabs(I - cbs.choir_flushed[slot]) <= CHOIR_FLUSH_EPS) continue;
        choir_project_slot(cbs, gpu, queue, active_seed, slot);
        cbs.choir_flushed[slot] = I;
    }
    cbs.repaint_all = false;
}

// `zoetrope_strike` stood here (CHOIR_0 U5) — the write head, the
// three-cell hit set with its column-wrapping spread, the immediate
// per-struck-slot poke, and the [ZOETROPE] strike witness. Its successor
// is not a function: the canvas's envelope IS the strike now, and the
// witness moved with it — [CHOIR] key=NN I=X.XX, on the ACTIVATION EDGE,
// still behind INSTRUMENTS.zoetrope_witness (the dial's rename is PARKED
// to keep this campaign's churn down).

inline void zoetrope_service(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue,
    float dt, float point_x, float point_z) {
    // ── The reseat watch (G4) ── possession moves the point in one
    // frame farther than any motion can; a standing formation answers by
    // re-using the capture two-step at the seam — recapture from the
    // true present, restage, resend seats around the new host. A resting
    // state re-enters its own walk so the seat pass runs; the bodies are
    // already settled, so it re-forms in one breath. TO_ROAM/ROAM: watch
    // only — a dissolving formation has no host to follow.
    {
        using Formation = CubeBehaviorsState::Formation;
        if (!cbs.point_seen) {
            cbs.last_px = point_x; cbs.last_pz = point_z; cbs.point_seen = true;
        }
        const float ddx = point_x - cbs.last_px;
        const float ddz = point_z - cbs.last_pz;
        const float delta = std::sqrt(ddx * ddx + ddz * ddz);
        if ((cbs.formation == Formation::TO_SCATTER || cbs.formation == Formation::SCATTERED
             || cbs.formation == Formation::TO_SCREEN || cbs.formation == Formation::SCREEN)
            && cbs.kite_mode && delta > ZOETROPE_RESEAT_JUMP) {
            // Only a FOLLOWING formation reseats (K1 E4): an anchored one
            // is planted in the world and a possession is none of its
            // business. The capture re-derives every offset from the new
            // host, so the seat pass below lands in the new frame.
            set_cube_kite(cbs, gpu, queue, true);   // re-capture at the new host
            if (cbs.formation == Formation::SCATTERED)   cbs.formation = Formation::TO_SCATTER;
            else if (cbs.formation == Formation::SCREEN) cbs.formation = Formation::TO_SCREEN;
            cbs.stations_sent = false;
            cbs.stage_wait = true;
            std::cout << "[Zoetrope] reseat: the screen follows its new host\n";
        }
        cbs.last_px = point_x; cbs.last_pz = point_z;
    }

    // THE PRIME PASS, THE CLOCK GUARDS, THE TICK AND THE PROJECTOR'S
    // FLUSH stood here (CHOIR_0 U5). The prime built `wdir` from the
    // fixed weight seed and anchored `last_tick_beat`; the two guards
    // re-anchored that clock across a loop seam and a transport leap;
    // the while-loop ran the diffusion + decay pass; and the flush spent
    // one ≤252-slot sweep per tick, plus the repaint edge.
    //
    // The choir needs no clock of its own — the canvas advances the
    // envelope on the beat it is already handed — and its flush is
    // poke-on-change rather than per-tick, so it lives at the seam
    // (choir_project) beside the mirror that feeds it. WHAT SURVIVES
    // HERE IS THE FORMATION MACHINE ALONE: the reseat watch above and
    // the climb below.

    // ── THE CLIMB (C6R): the formation walk — a CPU flush-walk on the
    // height scalar, the glide law's grammar; steady state pokes NOTHING.
    using Formation = CubeBehaviorsState::Formation;

    if (cbs.formation == Formation::TO_SCATTER || cbs.formation == Formation::TO_SCREEN
        || cbs.formation == Formation::TO_ROAM) {
        const bool to_screen  = (cbs.formation == Formation::TO_SCREEN);
        const bool to_scatter = (cbs.formation == Formation::TO_SCATTER);
        if (cbs.stage_wait) {
            // The press frame (V1): the sentinel is in flight — no
            // target or body writes until it has eaten.
            cbs.stage_wait = false;
        } else {
            if ((to_screen || to_scatter) && !cbs.stations_sent) {
                // First service after the press: the XZ seats, uniform,
                // one pass. TO_ROAM sends no seats: a release is not a
                // destination. The bodies walk home; the positions are
                // handed back to drift at the settle.
                //
                // TWO ARMS, ONE DOOR (K1) — the corral's absolute arm,
                // restored. C6R deleted it as "absorbed" and that
                // deletion is what forced the kite borrow. A kited cube
                // walks its OFFSET from the point; an anchored one walks
                // its ANCHOR through world coordinates, so the seat is
                // the same ring either way — planted instead of carried.
                for (uint32_t slot = 0; slot < LATTICE_CELLS; slot++) {
                    if (!cbs.activeCubes_[slot].active) continue;
                    const ZoetropeStation st = to_screen ? zoetrope_station(slot)
                                                         : station_scatter(cbs, slot);
                    if (cbs.kite_mode)
                        gpu.upload_cube_glide_target(queue, slot, st.off_x, st.off_z);
                    else
                        gpu.upload_cube_glide_target(queue, slot,
                            point_x + st.off_x, point_z + st.off_z);
                }
                cbs.stations_sent = true;
            }
            // THE WALK'S TARGETS — one table, one home (H1 E3):
            //   TO_SCATTER : the scatter seat's h; body = the MIRROR's own
            //                spawn draw — the flock keeps its variety
            //   TO_SCREEN  : the screen seat's h; body = the uniform pixel
            //   TO_ROAM    : the mirror's four — THE MIRROR IS THE PRIOR
            // The walker owns its four shadows (G5) and never reads the
            // GPU values while the walk lives. One k for all four;
            // aspects weigh 10× in the settle test (they are ratios).
            // Eviction mid-walk: inactive slots drop from the set; the
            // ghost keeps its cell.
            const float k = 1.0f - std::exp(-dt / ZOETROPE_LIFT_TAU);
            bool all_settled = true;
            for (uint32_t slot = 0; slot < LATTICE_CELLS; slot++) {
                if (!cbs.activeCubes_[slot].active) continue;
                if (cbs.settled[slot]) continue;
                const ActiveCube& ac = cbs.activeCubes_[slot];
                float th, tr, tay, taz;
                if (to_screen) {
                    th = zoetrope_station(slot).h; tr = ZOETROPE_PIXEL_RADIUS;
                    tay = 1.0f; taz = 1.0f;
                } else if (to_scatter) {
                    th = station_scatter(cbs, slot).h;
                    tr = ac.body_radius; tay = ac.aspect_y; taz = ac.aspect_z;
                } else {
                    th = ac.orbit_height; tr = ac.body_radius;
                    tay = ac.aspect_y; taz = ac.aspect_z;
                }
                CubeBehaviorsState::ZoeWalk& w = cbs.walk_[slot];
                w.h  += (th  - w.h)  * k;
                w.r  += (tr  - w.r)  * k;
                w.ay += (tay - w.ay) * k;
                w.az += (taz - w.az) * k;
                const float miss = std::max(
                    std::max(std::fabs(th - w.h), std::fabs(tr - w.r)),
                    10.0f * std::max(std::fabs(tay - w.ay), std::fabs(taz - w.az)));
                if (miss <= ZOETROPE_SETTLE_EPS) {
                    w = { th, tr, tay, taz };
                    cbs.settled[slot] = true;
                } else {
                    all_settled = false;
                }
                gpu.upload_cube_orbit_height(queue, slot, w.h);
                gpu.upload_cube_body_radius(queue, slot, w.r);
                gpu.upload_cube_aspects(queue, slot, w.ay, w.az);
            }
            if (all_settled) {
                if (!to_screen && !to_scatter) {
                    // THE HAND-BACK. Re-sending the sentinel it already
                    // wears recaptures target := present in-kernel, so the
                    // glide term goes to zero and the cube is returned to
                    // drift and its behavior force — free where it stands.
                    // kite_mode is UNCHANGED by construction: we resend
                    // what it already is. F6 chooses shape; F7 chooses
                    // frame. (Not set_cube_kite — that would rewrite the
                    // flag and print.)
                    for (uint32_t slot = 0; slot < LATTICE_CELLS; slot++) {
                        if (!cbs.activeCubes_[slot].active) continue;
                        gpu.upload_cube_follow_pawn(queue, slot, cbs.kite_mode ? 3u : 2u);
                    }
                }
                // THE ARRIVAL IS A REPAINT EDGE TOO (CHOIR_0 U6). The
                // walk snapped body_radius to the BARE pixel and the
                // formation changes underneath the projector — but the
                // LIGHT did not move, so the poke gate would skip every
                // slot and a cube arriving under a HELD chord would stand
                // at the bare radius, unswelled, until its key next
                // changed. The lattice's flush hid this: it ran
                // unconditionally every tick, so the swell landed within
                // a quarter beat whatever the gate thought. Poke-on-change
                // has no such backstop, so the arrival has to declare
                // itself. One forced pass, on the frame after the settle.
                cbs.repaint_all = true;
                cbs.formation = to_screen  ? Formation::SCREEN
                              : to_scatter ? Formation::SCATTERED
                                           : Formation::ROAM;
                std::cout << (to_screen  ? "[Zoetrope] screen: the instrument stands\n"
                            : to_scatter ? "[Zoetrope] gathered: the flock stands drawn in\n"
                                         : "[Zoetrope] roam: the swarm is whole\n");
            }
        }
    }
    // SCATTERED, SCREEN and ROAM poke nothing: steady-state traffic is zero.
}

// ─── Readback mirror reconciliation (owner verb) ─
// the cube half of the floater-readback funnel.
inline void reconcile_cube_mirror(CubeBehaviorsState& cs, CubeDeps* c, const GPUFloatingEntityState* data) {
    float now = c->time_state_.seconds;
    // Cubes: slots [CUBE_SLOT_OFFSET, TOTAL_FLOATING_SLOTS)
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        bool gpu_active = (data[Dim::CUBE_SLOT_OFFSET + i].is_active != 0u);
        // cube active-slot mirror owned by CubeBehaviorsState (cube_behaviors.hpp)
        if (cs.activeCubes_[i].active && !gpu_active &&
            (now - cs.activeCubes_[i].last_alloc_time) > SPAWN_PROTECTION_S) {
            cs.activeCubes_[i].active = false;
        }
        // FIELD_2: harvest the live pose the funnel already maps —
        // one frame stale, the ribbon's CPU field reads it. The PRIOR
        // scalars above stay untouched (the release walk's law).
        if (gpu_active) {
            const auto& fe = data[Dim::CUBE_SLOT_OFFSET + i];
            cs.activeCubes_[i].live_pos[0] = fe.pos[0];
            cs.activeCubes_[i].live_pos[1] = fe.pos[1];
            cs.activeCubes_[i].live_pos[2] = fe.pos[2];
            cs.activeCubes_[i].live_body_radius = fe.body_radius;
        } else {
            cs.activeCubes_[i].live_body_radius = 0.0f;  // un-harvested sentinel
        }
    }
}

} // namespace the_board
} // namespace t7
