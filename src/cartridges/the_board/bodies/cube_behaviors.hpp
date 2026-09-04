#pragma once
#include <cstdint>
#include <array>
#include "cartridges/the_board/realization/state.hpp"                       // Dim::MAX_CUBE_INSTANCES, GPUState, GPUFloatingEntityState, wgpu
#include "cartridges/the_board/contracts/floaters.hpp"  // ActiveCube, CUBE_TIER_COUNT
#include "cartridges/the_board/contracts/driver_surface.hpp"  // THE DRIVERS' ROOM: DRIVER_LIVE.cube — the choir's incandescence + gain
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)
#include "cartridges/the_board/contracts/entity_types.hpp"   // queue types (the funnel signatures)
#include "cartridges/the_board/contracts/control_panel.hpp"  // WHEEL_0: PANEL_LIVE.wheel — the interval wheel's five axes

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
// <numeric> stood here for std::gcd — the helix coprimality witness.
// The helix retired with the screen it seated (WHEEL_0 U3); the wheel
// addresses a key by its PITCH CLASS and its RANK, which is division,
// not a stride, and needs nothing proved coprime.
#include <algorithm>  // std::max — the scatter's altitude floor and the settle norm
                      // (std::min left with the lattice: it clamped the
                      // cell intensity, and there is no cell to clamp)
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
// ─ THE LATTICE band STOOD HERE (WHEEL_0 U3) ─────────────────────
// LATTICE_ROWS / LATTICE_COLS / LATTICE_CELLS and the helix pair
// (ZOETROPE_CELL_STRIDE / ZOETROPE_CELL_UNSTRIDE) with both of their
// witnesses — the inverse and the coprimality. Seven ranked rows of
// thirty-six bearings, and the stride that walked a slot onto one of
// them so consecutive spawns landed one column over and one row up.
//
// THE WHEEL ADDRESSES A KEY DIRECTLY. Key k IS pitch class k % 12 and
// rank k / 12, and those two numbers are the whole seating law — there
// is no cell to map onto, so there is no map, no stride, and nothing
// to prove a bijection. The lattice's last reader was the pair of
// station functions it seated, and they retire with the screen.
//
// The capacity that actually bounds the choir was never the lattice's
// 252 either: it is Dim::MAX_CUBE_INSTANCES, the slot count, and the
// static_assert below now says so.

// ─ THE CHOIR band ─ the boot choice: how many keys the instrument has.
// Not random, chosen before boot (Jean). Two ranks or three of twelve.
//
// KEY k = SLOT k, BY ASSIGNMENT (STAGE_0 R5). It used to be a
// CONSEQUENCE: run_spawn_preamble reserved the lowest free slot, so
// capping the family's max_instances kept the population dense in
// 0..N-1 and an evicted key's refill relit the same dark key. Neither
// mechanism exists — the gate refuses, so nothing reserves, and
// eviction went at STAGE_0 U2, so nothing is refilled. birth_the_choir
// writes `inst.slot = k` for k in 0..N-1 and that is the whole law.
// An assignment is STRONGER than a consequence: there is no mechanism
// left to drift out from under it. No mapping table, no registry.
//
// The lattice geometry that stood above was untouched by this: its 36
// columns were arithmetic (256/7) and CUBE_CHOIR_N is a CHOICE. The two
// shared a number at CHOIR_0 and no longer did — which is exactly why
// they were never the same constant, and why the lattice could retire
// at WHEEL_0 U3 without the choir noticing.
//
// CHOIR_1: TWO RANKS. Rank 0 lights on one sounding note per pitch
// class, rank 1 on a doubling; a third voice of the same class now has
// no key to light, where three ranks gave it one. The PIPE does not
// narrow with the keyboard — CHOIR_LANES stays 36 and lanes 24..35 sit
// at their rest, dark, which is what makes this the one token CHOIR_0
// banked it as (the seam's static_assert reads ≤, not ==).
inline constexpr uint32_t CUBE_CHOIR_N = 24;
static_assert(CUBE_CHOIR_N == 24u || CUBE_CHOIR_N == 36u,
    "the choir is stacked pianos: two ranks or three, nothing else");
static_assert(CUBE_CHOIR_N % 12u == 0u, "ranks are whole pianos");
static_assert(CUBE_CHOIR_N <= Dim::MAX_CUBE_INSTANCES,
    "a key is a SLOT (key k = slot k, by construction) — the choir may not outrun the slots");
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
// estimated, and RE-COUNTED at CHOIR_1's sharper τ rather than scaled.
// ATTACK: ΔI = (1 − I)·(1 − e^(−Δ/τ)) = 0.02469·(1 − I) at τ = 8/6 —
// **158 pokes across the plateau's 240 frames** (it was 199 at τ = 2),
// the per-frame step crossing under this gate at frame 129 / I = 0.960
// (was frame 169 / I = 0.940), then thinning with the residual: about
// 2 more over the following ten plateaus, where the old τ spent 17.
// THE SNAP IS CHEAPER, WHICH IS NOT OBVIOUS: a steeper climb reaches
// the flat part sooner, and the flat part is where the gate stops
// paying. RELEASE: unmoved — the slope is 1/8 per beat = 0.00417 a
// frame, four times this gate, so a fall still pokes **every one of
// its 240 frames**, and the release is now the expensive half by a
// wide margin. WORST CASE is one poke per SOUNDING key per frame with
// the whole choir falling at once: at CUBE_CHOIR_N = 24 that is 24
// twelve-byte colour writes and 24 four-byte variance writes — and,
// WHEN THE SCREEN STANDS, 24 four-byte radius writes as well, because
// the swell rides the same poke. 480 B a frame (it was 720 at three
// ranks) against the lattice's ≤252-slot sweep every quarter beat.
// Silence pokes NOTHING.
inline constexpr float CHOIR_FLUSH_EPS = 1e-3f;

// ─ THE CHOIR'S OWN SEED (STAGE_0 U3) ─ the instrument is AUTHORED, so
// its bodies do not draw from the world. Every Gaussian a key takes —
// radius, bob, aspects, face variance — hashes off THIS constant and the
// key index, so the same twenty-four cubes stand in every world and in
// every run. Not the ground's seed: the ground keeps its own by ruling.
inline constexpr uint32_t CHOIR_SEED = 0x7C0119E5u;

// THE ROSTER, tier per key, at the old weights' proportions
// ({0.40, 0.32, 0.20, 0.08} over 24 = 9.6 / 7.7 / 4.8 / 1.9). Rounded to
// 10 / 8 / 5 / 1 — the Monolith is rounded DOWN to one because two would
// be 8% of a 24-key instrument reading as 4%, and one tall thing is a
// landmark where two are a pair. ITS PLACEMENT IS JEAN'S DESK; ruled
// default key 18, which is rank 1's pitch class 6 — the tritone, and the
// far side of the wheel from key 0.
inline constexpr uint8_t CHOIR_TIERS[CUBE_CHOIR_N] = {
    0,0,0,0,0,0,0,0,0,0,      // keys  0-9  SmallCube  (10)
    1,1,1,1,1,1,1,1,          // keys 10-17 MedCube    ( 8)
    3,                        // key  18    Monolith   ( 1)  ← Jean's desk
    2,2,2,2,2,                // keys 19-23 LargeCube  ( 5)
};
static_assert(sizeof(CHOIR_TIERS) / sizeof(CHOIR_TIERS[0]) == CUBE_CHOIR_N,
    "one tier per key, hand-authored — a keyboard has no weights to roll");

// CHOIR_BIRTH_RADIUS stood here (WHEEL_0 U2) — 60 wu, the placeholder
// circle's reach, declared a placeholder on the day it was written. Its
// successor is PANEL_TABLE.wheel.radius, which is the same 60 wu and is
// a DIAL rather than a constant: the birth stands on the REST wheel and
// the wheel's inner radius is the number it stands at.

// CHOIR_PATCH_ROW stood here (STAGE_0 R4) — a synthetic patch row at
// -30000, one distinct pair per key, invented because the projector
// recomputed a key's colour from tile_seed(world seed, patch_gx,
// patch_gz) and boot-born cubes have no patch. Left at the 0,0 default
// every key hashed the SAME seed and the whole choir came out ONE FLAT
// COLOUR; the synthetic row was the fix, and it was a workaround for a
// seed the birth did not choose.
//
// THE BIRTH CHOOSES IT NOW. choir_slot_seed(k) is cpu_hash(CHOIR_SEED, k)
// — per-key distinct by construction, and world-independent, so a key
// wears the same colour in every world and in every run exactly as it
// wears the same body. No patch coordinates are consulted by anything.
//
// ── AND IT IS PROVED, WHICH IT NEVER WAS ────────────────────────
// The monochrome failure was found by hand at STAGE_0 and asserted by
// nothing: 24 keys hashing to one seed is legal C++ that simply looks
// wrong, and no gate in an eleven-row battery reads a colour. With the
// seed now a pure function of a compile-time key, the COMPILER can
// settle it — 276 pairs, checked at build time, on the one draw where a
// collision is invisible until a device draws it.
inline constexpr bool choir_palette_is_distinct() {
    for (uint32_t a = 0; a < CUBE_CHOIR_N; ++a)
        for (uint32_t b = a + 1u; b < CUBE_CHOIR_N; ++b)
            if (cpu_hash(CHOIR_SEED, a) == cpu_hash(CHOIR_SEED, b)) return false;
    return true;
}
static_assert(choir_palette_is_distinct(),
    "two keys share a colour seed — the choir would come out part monochrome, "
    "and nothing but a device would say so. Move CHOIR_SEED.");

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
// ─ THE SCREEN, THE SCATTER AND THE WALK STOOD HERE (WHEEL_0 U3) ──
// RING_RADIUS / H_BASE / H_STEP / PIXEL_RADIUS with the spacing note
// that derived them; the screen's three loosening jitters and their
// seed; the whole scatter band — reach, three jitters, the size bias
// and its own seed; and the walk's three numbers, LIFT_TAU, SETTLE_EPS
// and RESEAT_JUMP.
//
// WHAT THEY SERVED IS GONE, ALL OF IT AT ONCE: two resting formations
// and the three walks between them, seated by hand on the CPU with a
// per-slot shadow set and a settle test. THE GLIDE DOOR IS THE
// TRANSITION NOW — goals may leap, values may only walk — so a
// formation is a STATION FUNCTION and nothing else, and there is no
// walk left to own a tau, no arrival left to test an epsilon, and no
// staged frame for a sentinel that no longer flies.
//
// THE HEIGHTS WENT WITH THEM, and that is the campaign's own ruling:
// the wheel is XZ ONLY. A key stands at the altitude its TIER drew
// (CHOIR_1's calm band), so the screen's ranked rows have no successor
// — the wheel ranks in RADIUS, outward, where the screen ranked in
// height.

// ─ THE SERVE'S GATE (WHEEL_0 U2) ────────────────────────
// The station is recomputed every frame for every key, but a key only
// POKES when its station moved past this. A steady wheel is silent; a
// turning phase pokes all twenty-four.
//
// LIKE THE LIGHT'S GATE, IT COMPARES AGAINST THE LAST SERVE, not the
// last frame — which is the difference between thinning and stopping.
// A wheel turning slower than epsilon per frame still accumulates
// against the stored shadow and pokes when the sum crosses, so no
// speed of turn can leave a key behind.
inline constexpr float WHEEL_SERVE_EPS = 0.05f;  // wu — below this the station has not moved

// ─ THE EXPRESSION band ──────────────────────────────────────────
// One light, three expressions: the colour mix, the swell, and the
// CONVERGENCE of the face. All read the same I through choir_light's
// one door, so a lit key is one gesture.
// ZOETROPE_REST_DIM stood here (WHEEL_0 U3) — 0.30, the SCREEN's rest
// brightness, on the reading that an instrument is dark until played.
// It had exactly two edges (the two dim transitions) and both were
// TO_ states; with the screen gone there is no dark screen to rest, and
// a wheel of cubes standing in the world is the world's swarm, which
// C6R already ruled must not be dimmed. The base is the mirror's own
// draw again, at full.
inline constexpr float ZOETROPE_SWELL_GAIN = 0.60f;  // × THE MIRROR'S OWN body radius at full I (was × the screen's uniform pixel)
// PIGMENT_R/G/B/WEIGHT stood here (CHOIR_0 U5) — the ethereal ice the
// mix aimed at, and the stain under the flash. Their successor is
// DRIVER_LIVE.cube.light_color, which belongs in the DRIVERS' ROOM
// rather than in a module console: a driven parameter wears no dial on
// its value, it wears one on its driver (contracts/driver_surface.hpp).
// IT IS NOT A REACHABLE DIAL YET — its organ row is PARKED under the
// ORGAN_REST registry freeze, so today it is edited here-and-recompile
// exactly as the constants it replaces were. What changed is its HOME
// and therefore what it will cost to enrol: one organ_params.inc line,
// not a graduation.
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
// stands. The helix survived CHOIR_0 as formation geometry and retired
// at WHEEL_0 U3 with the screen it seated — the wheel addresses a key by
// its pitch class and its rank, so there is no cell and no map.

struct CubeBehaviorsState {
    uint32_t   behavior_override  = CUBE_BEHAVIOR_STATIONARY;
    // `kite_mode` stood here (STAGE_0 U4) — one CPU flag for the flock,
    // against a per-cube truth that lived on the GPU. Formations anchor in
    // the world now, so there is no frame to choose between.
    ActiveCube activeCubes_[Dim::MAX_CUBE_INSTANCES]{};

    // ── THE MODE (WHEEL_0 U3) ── two states and no walk between them.
    // WHEEL: every key is served its station on the interval wheel,
    // every frame, through the glide-target door. ROAM: the targets go
    // back to the birth anchors and DRIFT OWNS THE PICTURE — the
    // behaviour kernels have the cubes again.
    //
    // THERE ARE NO TO_ STATES AND THERE IS NOTHING TO WALK, which is the
    // whole point of goals-leap-values-walk: the door moves the GOAL and
    // the kernel walks the VALUE at the standing tau. What used to be
    // three transitions with a staged frame, a shadow set, a settle test
    // and an arrival edge is now one assignment.
    //
    // REST IS WHEEL. The instrument stands assembled at boot — the birth
    // lays every key on the rest wheel already in position (U2), so the
    // first frame serves nothing and the wheel is simply THERE.
    enum class Formation : uint8_t { ROAM, WHEEL };
    Formation formation = Formation::WHEEL;
    // THE FORCE FLAG. A cube's look changes with the MODE as well as
    // with the music, and the projector is gated on the music — so a
    // mode change that moves the look has to say so here, or the new
    // look waits for a note that may never come (V1 E3). The raiser is a
    // door verb with no GPU hand of its own, so it raises this instead
    // and choir_project spends it as a FORCE on its next pass. (It used
    // to be that the raiser "could not reach the world seed the projector
    // needs" — the projector needs no world seed since STAGE_0 R4; the
    // flag survives its own justification because the door still cannot
    // write a colour.)
    // TWO RAISERS, TWO EDGES (WHEEL_0 R7):
    //   · reveal_zoetrope, on the ROAM↔WHEEL flip, BOTH DIRECTIONS — the
    //     swell's jurisdiction changes there, and the raise is
    //     unconditional so leaving the wheel restores the mirror's own
    //     radius as surely as entering it asserts the swell.
    //   · REBIRTH, at the tail of rebirth_world (cartridge.hpp), after
    //     build_world has borne the new choir. clear_cubes flips the mode
    //     to its rest SILENTLY, which is the same class of change; the
    //     raise is not made there because choir_project spends the flag
    //     even on an empty roster, so a teardown-time raise survives only
    //     by this frame's atomicity. The site carries the argument.
    // (The dim's two edges retired with the dim; the arrival edge
    // retired with the walk that used to snap a radius under it.)
    bool  repaint_all   = false;
    // ── THE SERVE'S SHADOW (WHEEL_0 U2) ── the station each key was
    // last actually served at, in world XZ. Same shape and same reading
    // as choir_flushed one field down: the gate compares against THIS,
    // not against the previous frame, so a wheel turning slower than
    // WHEEL_SERVE_EPS per frame still accumulates and pokes.
    //
    // IT DIFFS AUTHORED INTENT AGAINST LAST-SENT, NEVER AGAINST
    // GPU-WALKED STATE (WHEEL_0 R7, confirmed against the shipped code).
    // The whole census of writers is three and there is no fourth path:
    // the SERVE writes it immediately after each poke, the BIRTH seeds it
    // from fe.target_x/z — the same struct member that went to the GPU,
    // not a recomputed station — and clear_cubes RESETS it, because the
    // slots it shadowed have just been wiped. The serve reads no
    // live_pos, no live_body_radius, no readback and no mapped buffer;
    // `gpu` is used for exactly one thing, the write.
    //
    // AND THAT IS NOT MERELY UNREAD BUT UNREADABLE-BY-ANYONE now:
    // ActiveCube::live_pos and live_body_radius have no reader left in
    // the tree at all — their last was the hand-back that retired at
    // WHEEL_0 U3 — so the harvest exists and nothing consumes it.
    // A gate that diffed against walked state would chase the walk it
    // caused and never settle; this one is a goal against a goal.
    float wheel_sx[CUBE_CHOIR_N]{};
    float wheel_sz[CUBE_CHOIR_N]{};
    // ── THE BIRTH ANCHOR (WHEEL_0 U3) ── where the key was actually
    // BORN, in world XZ, recorded once and never rewritten. This is
    // ROAM's target: let go of the wheel and a key walks home to the
    // shape it was laid in.
    //
    // IT IS A RECORD, NOT A RECOMPUTATION, and that distinction is the
    // whole reason it is a field rather than a call to birth_station.
    // birth_station reads the LIVE wheel, so recomputing it would make
    // ROAM follow the dials — the one thing ROAM exists not to do. The
    // anchor is the wheel as it stood ON THE DAY, frozen.
    float birth_ax[CUBE_CHOIR_N]{};
    float birth_az[CUBE_CHOIR_N]{};

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
// The mode door (WHEEL_0 U3): ROAM ↔ WHEEL. Its two trailing
// parameters are the door table's shape, not its needs — the flip
// writes no GPU state at all.
void reveal_zoetrope(CubeBehaviorsState& cbs, CubeDeps* c, wgpu::Queue& queue);
// `set_cube_kite` (the ONE kite home, G3) and `toggle_cube_kite_mode`
// stood here (STAGE_0 U4), with door 6 that pressed the second.
// Per-frame
void reconcile_cube_mirror(CubeBehaviorsState& cs, CubeDeps* c, const GPUFloatingEntityState* data);
// THE SERVE — the wheel's per-frame pass, and all that is left of the
// formation machine. `zoetrope_strike` stood beside it (CHOIR_0 U5) and
// went with the lattice it fed; the reseat watch and the climb went at
// WHEEL_0 U3 with the walk and the host-following screen, which took
// `dt` and the point mirror with them. What remains reads the panel,
// computes a station per key and pokes the ones that moved — no clock,
// no world seed, no host, no time.
void zoetrope_service(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue);
// THE PROJECTOR — one home: the light reaches pixels here and nowhere
// else. `zoetrope_cell_intensity`, `project_cell_color` and `zoetrope_
// project_slot` stood here and are superseded by these. The seed
// recompute crossed over intact as choir_slot_seed at CHOIR_0 and stopped
// being a recompute at STAGE_0 R4 — it authors now, off CHOIR_SEED and
// the key, and takes neither a mirror nor a world. choir_light is I's ONE
// computation (the G6 door, inherited whole).
uint32_t choir_slot_seed(uint32_t key);
float choir_light(const CubeBehaviorsState& cbs, uint32_t slot);
void choir_project_color(const CubeBehaviorsState& cbs, uint32_t slot,
    float& out_r, float& out_g, float& out_b);
void choir_project(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue);
// THE BIRTH (STAGE_0 U3): the choir is authored, not spawned.
void birth_the_choir(MachineCtx* c, wgpu::Queue& queue);

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
    //
    // THE MODE RESETS TO ITS REST (WHEEL_0 U3), not to ROAM: the wheel is
    // the instrument's standing shape, and a world torn down mid-roam
    // that came back roaming would be a world whose choir has to be
    // re-assembled by hand. The birth that follows lays every key on the
    // rest wheel, so the reset and the birth agree.
    cbs.formation     = CubeBehaviorsState::Formation::WHEEL;
    for (uint32_t i = 0; i < CUBE_CHOIR_N; i++) {
        cbs.choir_flushed[i] = 0.0f;
        cbs.wheel_sx[i] = 0.0f;
        cbs.wheel_sz[i] = 0.0f;
        cbs.birth_ax[i] = 0.0f;
        cbs.birth_az[i] = 0.0f;
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

// ── THE INTERVAL WHEEL — the ONE home for geometry (WHEEL_0) ────
//
// THE LAW, whole:
//
//     pc  = k % 12          the key's PITCH CLASS
//     r   = k / 12          the key's RANK  (N = 24 → two ranks)
//     θ(k)      = phase + twist·r + (2π/12)·wrap12(step · pc)
//     radius(k) = radius + rank_sep·r
//     station(k)= centre + radius(k)·(cos θ, sin θ)
//
// Concentric rings about the world's centre, one ring per rank, a key
// seated on its ring by the pitch class it IS. Key k was already a
// pitch class and a rank by construction (key k = slot k, and the
// choir is stacked pianos); the wheel is what makes the FORMATION say
// so out loud.
//
// STEP IS THE TRANSFORMATION AXIS. At step 1 the ring is the CHROMATIC
// CIRCLE — the twelve classes in order round the clock. Walked
// continuously toward 7 it passes through every star polygon {12/step}
// and arrives at the CIRCLE OF FIFTHS, where a fifth is a neighbour
// pair and a semitone is a near-diameter. What the eye reads is the
// interval content of whatever is sounding, DRAWN: a fifth is a
// near-diameter at step 1 and a neighbour pair at step 7, and the
// chord's shape on the floor changes meaning as the wheel is turned.
//
// AND IT IS A CONTINUOUS AXIS, not a switch. `step` is a float and
// wrap12 is applied to the CONTINUOUS PRODUCT — no rounding, no
// shortest-arc logic, nothing that could make a key jump the seam.
// A key crossing the wrap seam WALKS the long way round, because the
// station is a goal and the glide door is what turns a goal into a
// path. THE BRAID IS THE PERCEPT: a moving step sends twenty-four
// cubes weaving past each other across the floor of the world, and
// that motion — not the endpoints — is the thing worth watching.
//
// NO NEW MOTION MECHANISM EXISTS ANYWHERE IN THIS CAMPAIGN. Every
// station reaches a cube through upload_cube_glide_target and is
// walked in-kernel at the standing CUBE_GLIDE_TAU, exactly as a corral
// target was. Goals may leap; values may only walk.
//
// XZ ONLY (the campaign's ruling). Heights stay the tiers' own calm
// band — CHOIR_1 tuned them and the wheel has no business in them. The
// screen ranked in HEIGHT; the wheel ranks in RADIUS, outward.
struct WheelStation { float off_x; float off_z; };
// `ZoetropeStation` stood here with a third member, `h` — the seat's
// height, which every walk read and no wheel does.

// The wrap, and it is the law's one subtlety. std::fmod keeps the sign
// of its left operand, so a negative step (a legal setting: the wheel
// may be turned backwards) would come back negative and put the key on
// the wrong side of the circle. One conditional lift puts it in [0,12).
inline float wrap12(float x) {
    const float w = std::fmod(x, 12.0f);
    return (w < 0.0f) ? w + 12.0f : w;
}

// The station of key k on a GIVEN wheel. Taking the axes by parameter
// rather than reading PANEL_LIVE is what lets the birth stand on the
// REST wheel while the serve stands on the LIVE one, through one law.
inline WheelStation wheel_station(const PanelSurface::Wheel& w, uint32_t k) {
    const float two_pi = 6.28318530718f;
    const float pc = float(k % 12u);
    const float rank = float(k / 12u);
    const float theta  = w.phase + w.twist * rank
                       + (two_pi / 12.0f) * wrap12(w.step * pc);
    const float radius = w.radius + w.rank_sep * rank;
    return { std::cos(theta) * radius, std::sin(theta) * radius };
}

// KEY k'S BIRTH POSITION — no longer a placeholder: it is the wheel's
// own station, on the wheel AS THE HAND HAS IT.
//
// IT READS PANEL_LIVE, AND IT MUST. PanelSurface is a GRADUATED PAIR and
// the law of one is that the design table is a SEED and an assert
// subject and nothing else — a runtime read of PANEL_TABLE is precisely
// the defect organ_gap's reader witness exists to catch, and it caught
// this function reading it. (It was written against the table on a
// determinism argument; the argument was answering the wrong question.
// What the protect list guards is the BODY draw — CHOIR_SEED and the
// tier table — and that is untouched here. Where a key STANDS is the
// wheel's business, and the wheel is a dial.)
//
// BORN IN POSITION, which is the thing the birth actually owes. At boot
// PANEL_LIVE is PANEL_TABLE — step 1, the chromatic circle — so the
// choir stands assembled from its first frame, nothing walks in from
// anywhere, and the serve's first pass finds every station already met
// and pokes nothing at all. Boot from a scene that has already turned
// the wheel and the choir is laid on THAT wheel, in position, which is
// the same sentence.
inline WheelStation birth_station(uint32_t k) {
    return wheel_station(PANEL_LIVE.wheel, k);
}

// `cell_of_slot`, `zoetrope_station` and `station_scatter` stood here
// (WHEEL_0 U3) — the helix that walked a slot onto a lattice cell, and
// the two seatings that read the cell back: the SCREEN (uniform pixels,
// seven ranked rows, tight fixed-seed jitter) and the SCATTER (the
// flock — own bodies, own altitudes, deep jitter, and the size bias
// that seated big cubes outward so the depth read).
//
// THE SEATING LAW (K2) THEY SERVED — two cubes may share a column,
// nothing may share a bearing — IS KEPT BY CONSTRUCTION HERE and needs
// no jitter to keep it. Two keys share a bearing only when their pitch
// classes coincide, and same-pc keys are on DIFFERENT RANKS, which is
// to say different radii: rank_sep is asserted positive on the panel.
// At step 1 the twelve bearings are exactly the twelve hours.

inline void reveal_zoetrope(CubeBehaviorsState& cbs, CubeDeps*, wgpu::Queue&) {
    using Formation = CubeBehaviorsState::Formation;

    // ONE DOOR, TWO STATES (WHEEL_0 U3). It was a three-destination
    // cycle — roam → gather → reveal → release — and the two middle
    // destinations were the screen and the flock that fed it. What is
    // left is the only distinction the wheel admits: the wheel HAS the
    // choir, or the choir is LET GO.
    //
    // IT WRITES NOTHING. The old press staged a frame, re-armed a
    // shadow set and cleared a settle flag for every seat, because the
    // walk it started was the CPU's to run. The serve reads `formation`
    // every frame and aims accordingly, so flipping the field IS the
    // transition — the goal leaps here, the values walk in-kernel, and
    // no cube can be caught mid-anything.
    const bool to_wheel = (cbs.formation == Formation::ROAM);
    cbs.formation = to_wheel ? Formation::WHEEL : Formation::ROAM;

    // THE ONE REPAINT EDGE (WHEEL_0 U3). The swell is gated on WHEEL, so
    // the flip changes what a lit key LOOKS LIKE without moving its
    // LIGHT — exactly the class of change the poke gate cannot see. One
    // forced pass on the projector's next run restores the mirror's own
    // radius going out, and re-asserts the swell coming in.
    cbs.repaint_all = true;

    uint32_t keys = 0;
    for (uint32_t k = 0; k < CUBE_CHOIR_N; k++)
        if (cbs.activeCubes_[k].active) keys++;

    std::cout << (to_wheel ? "[Wheel] the wheel takes the choir"
                           : "[Wheel] release: the choir is let go to drift")
              << " (" << keys << " key(s))\n";
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
// THE KITE'S TWO VERBS STOOD HERE (STAGE_0 U4). `set_cube_kite` wrote the
// 2u/3u sentinel to every active cube — capture (offset := the true
// present) or release (anchor := current pos, drift.xz zeroed) — and both
// were xz-position-preserving even under drift, because the capture
// happened where drift lives. `toggle_cube_kite_mode` was door 6's verb
// and the flag's one writer; the F7 key its comments named had already
// left at ONE_WORLD-II, so the door was the only way in.
//
// THE LAW THEY SERVED IS THE ONE THE STAGE RETIRES: "F6 chooses shape,
// F7 chooses whether it follows". A formation now always stands in the
// world, so there is no second frame for a shape to be expressed in.

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
// SIZE μ (CHOIR_1): the three big tiers came DOWN — Med 4.0 → 3.0, Large
// 8.0 → 5.0, Monolith 3.0 → 2.2 with its ASPECT_Y 5.0 → 3.5, so its slab
// is ≈ 7.7 wu where it was ≈ 15. SmallCube did not move: it was already
// the scale the others were being brought toward. SCALE_0's law is kept
// at the new means rather than inherited — σ moved WITH μ, so the two
// tiers that carried CV 0.20 still carry it exactly (Med 0.60/3.0,
// Large 1.00/5.0, tightened from 0.209) and the Monolith sits at 0.182.
// A retune that moved μ alone would have widened every silhouette in
// relative terms, which is the compounding trap the paragraph above
// names.
// HEIGHT μ,σ (CHOIR_1 — A NEW AUTHORED CHOICE, beside SCALE_0's and
// TEMPO_0's): the flock flies LOWER and, deliberately, in a NARROWER
// BAND — 25/45/75/12 → 12/16/22/10, with CV taken from 0.60–0.80 down
// to ≈ 0.50 on every row. Narrower is the half that is not implied by
// "lower": scaling σ proportionally would have kept the old spread's
// character at a lower centre, and the ruling is a CALMER band, not
// just a lower one. A side effect worth having: every tier now clears
// the CUBE_PARAM_DEFS floor (3.0 wu) by more than it did, so the low
// tail clips LESS than before — Small 13.6% → 6.7%, Monolith 13.0% →
// 8.1% — and the drawn distribution is closer to the authored one.
// Biography determinant — frozen biography (§12).
inline constexpr CubeTierRow CUBE_TIERS[CUBE_TIER_COUNT] = {
    /* 0: SmallCube */ {
        { 0.40f, 0.0f, { {1.8f, 0.33f}, {12.0f, 6.0f}, {6.0f, 1.5f},  {0.04f, 0.015f},
                   {1.0f, 0.3f}, {5.0f, 0.6f},
                   {1.0f, 0.10f}, {1.0f, 0.10f}, {0.18f, 0.06f} }},
        0.12f
    },
    /* 1: MedCube   */ {
        { 0.32f, 0.0f, { {3.0f, 0.60f}, {16.0f, 8.0f}, {10.0f, 2.0f}, {0.03f, 0.01f},
                   {1.5f, 0.4f}, {6.0f, 0.9f},
                   {1.0f, 0.13f}, {1.0f, 0.13f}, {0.20f, 0.07f} }},
        0.10f
    },
    /* 2: LargeCube */ {
        { 0.20f, 0.0f, { {5.0f, 1.00f}, {22.0f, 10.0f}, {14.0f, 3.0f}, {0.02f, 0.008f},
                   {2.0f, 0.5f}, {8.0f, 1.4f},
                   {1.0f, 0.17f}, {1.0f, 0.17f}, {0.16f, 0.05f} }},
        0.08f
    },
    /* 3: Monolith  */ {
        { 0.08f, 0.0f, { {2.2f, 0.40f}, {10.0f, 5.0f}, {12.0f, 3.0f}, {0.015f, 0.005f},
                   {1.2f, 0.3f}, {6.0f, 0.9f},
                   {3.5f, 0.55f}, {0.15f, 0.02f}, {0.20f, 0.06f} }},
        0.10f
    },
};

inline const TierProfile& cube_get_tier_profile(uint32_t tier_idx) {
    return CUBE_TIERS[tier_idx].profile;
}

inline constexpr EntityFamilyTraits CUBE_TRAITS = {
    PopFamily::CUBE, CUBE_CHOIR_N,   // THE CHOIR is the living ceiling now (the lattice's 252 was); capacity stays 256
    // THREE FIELDS BELOW ARE DEAD SINCE STAGE_0 U3 and are kept as VALUES,
    // not as law: spawn_roll_prop, spawn_chance and position_jitter were
    // read only by the gate this campaign retired. They stay because
    // EntityFamilyTraits is a POSITIONAL AGGREGATE — removing a field
    // silently slides every later initialiser up one slot, which is the
    // exact hazard T7_GATE_PIN exists to catch — and because CubeProp's
    // indices are the FROZEN BIOGRAPHY contract and SPAWN_ROLL is one of
    // them. Zeroing spawn_chance would read as "never spawns" and be just
    // as dead while looking like a dial. Flagged, not cut.
    false,                // NOT grounded — hovers and drifts; claims no ground (ruling 21)
    CubeProp::SPAWN_ROLL, CubeConfig::SPAWN_CHANCE,
    CubeConfig::POSITION_JITTER,
    CUBE_TIER_COUNT, CubeProp::TIER,
    CUBE_PARAM_DEFS, CUBE_PARAM_COUNT,
    CubeProp::ANCHOR_X, CubeProp::ANCHOR_Z, CubeProp::ROTATION,
    0, nullptr,
};

// THE GATE REFUSES (STAGE_0 U3). It delegated to gate_from_traits, which
// rolled CubeConfig::SPAWN_CHANCE per patch and reserved the lowest free
// slot. THE CHOIR IS AUTHORED NOW — birth_the_choir lays twenty-four keys
// at world birth — so there is nothing left to roll, and a gate that could
// still fire would put a twenty-fifth cube in a twenty-four-key
// instrument.
//
// IT REFUSES RATHER THAN VANISHING, and that is deliberate. This function
// is one slot of CUBE_ADAPTER, which is a POSITIONAL AGGREGATE, and the
// three funnels below are three more slots of FAMILY_DISPATCH's cube row.
// Removing any of them slides every later initialiser up one — the exact
// hazard T7_GATE_PIN and the tail-pointer asserts exist to catch, and a
// hazard with no upside here: an early `return {false,…}` is one branch
// the compiler folds, and generic_select exits on it before touching a
// tier weight. The SHAPE stays, the SPAWN goes.
inline SpawnGateOutput cube_run_gate(MachineCtx* c, int32_t gx, int32_t gz) {
    (void)c; (void)gx; (void)gz;
    return { false, 0u, 0u };
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
    // rather than waking dark and catching up (the slot's colour seed is
    // the key itself since STAGE_0 R4, so this needs nothing seated ahead
    // of it; at I = 0 it is inst.colors bit-exactly — the silent path is
    // today's, and R4 did not move it).
    choir_project_color(c->cube_behaviors_state_, inst.slot,
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
    // ═══ THE SPAWN LAW IS REPEALED, AND IT IS SAID HERE BECAUSE HERE
    //     IS WHERE IT WAS STATED (STAGE_0 R5) ══════════════════════
    //
    // IT READ: "THE SPAWN LAW (Jean, non-negotiable): a cube spawns at
    // its DESIGNATED PATCH, outside the render radius — no matter what."
    // It was the strongest sentence in this file and it was RIGHT for
    // the program that had a cube spawn path: a seat that relocated a
    // newborn to point + station put newborns in the lattice beside the
    // player the moment a formation stood, and the law is what forbade
    // it.
    //
    // BOTH OF ITS TERMS ARE GONE, and they went one campaign apart.
    // "OUTSIDE THE RENDER RADIUS" lost its referent at STAGE_0 U2, when
    // the veil chain, both eviction radii and EXIST_RADIUS were excised
    // under THE STAGE LAW — everything computed is visible, so there is
    // no radius to be outside of. "ITS DESIGNATED PATCH" lost its at
    // STAGE_0 U3: cube_run_gate refuses, so no cube is designated
    // anything by the spawn engine, and the only caller left of this
    // function is birth_the_choir.
    //
    // BIRTH STATIONS ARE THE LAW NOW. Where a key stands is authored —
    // birth_station(k), the wheel's own station for the key — and the
    // wheel serves it thereafter. That is not a loophole in the spawn
    // law; it is what replaces it, and the thing the law protected
    // against (a newborn teleported next to the player) is impossible
    // for a different reason: there is no host to be next to. The wheel
    // is anchored at the world's centre.
    //
    // WHAT THE SPAWN LAW STILL GOVERNS: nothing in this tree. It is
    // repealed rather than dormant, and the repeal is recorded in
    // docs/OPEN.md beside THE STAGE LAW, which is the commission that
    // superseded it.
    //
    // THE RIDER THAT RODE IT, retired with the same sentence:
    // RIDER[cube:spawn-mode-desync] held that a newborn must join the
    // mode live at its spawn, because a cube left in anchor mode while
    // the flock was kited would be handed ring offsets and glide to a
    // ring around the world origin. There are no modes and no kite
    // (STAGE_0 U4). Drift is still exactly zero at birth, and target is
    // still initialised to the param, so update_cube's glide term is
    // exactly zero on the first frame — that half was never about the
    // kite and it stands.
    // WHAT THE SEAT DRESSES IS THE SWELL, AND ONLY THE SWELL (WHEEL_0
    // U3). Under the screen it also overwrote the height with the seat's
    // rank and the body with the uniform pixel and both aspects with 1 —
    // a newborn wore the instrument's shape. THE WHEEL TAKES NONE OF
    // THAT: it is XZ only, so a key keeps its tier's height, its tier's
    // body and its tier's aspects, and the only thing the mode changes
    // about a newborn's LOOK is whether the light swells it.
    //
    // IT STILL HAS TO, though, for the reason CHOIR_0 U6 found the hard
    // way: a cube born into a HELD chord must be born swollen, or it
    // stands at its bare draw until that key next moves — and a key
    // being held is precisely a key that is not moving.
    using Formation = CubeBehaviorsState::Formation;
    if (c->cube_behaviors_state_.formation == Formation::WHEEL) {
        fe.body_radius *= (1.0f + ZOETROPE_SWELL_GAIN
            * choir_light(c->cube_behaviors_state_, inst.slot));
    }

    // ONE ARM (STAGE_0 U4). The kite arm wrote pawn_offset and the 1u
    // sentinel so a newborn joined whatever frame was live at its spawn;
    // there is one frame now. The param is anchor.xz, written above from
    // the BIRTH position, and target is initialised to match so the glide
    // term is exactly zero on the first frame. This block used to credit
    // the spawn law for that; the law is repealed (the banner above) and
    // the practice stands on its own — a body should not be walking
    // anywhere before anything has asked it to.
    fe.target_x = inst.cx;
    fe.target_z = inst.cz;

    c->gpuState_.upload_cube_entity_slot(queue, inst.slot, fe);

    // ZOETROPE (C6R + G5): birth bookkeeping — the walker's shadows
    // mirror the scalars just written; a newborn is born settled — no
    // walk owed, in any formation state. The PRIOR needs no record
    // here: the mirror is the prior (write_active seated the draws).
    auto& zcbs = c->cube_behaviors_state_;
    // THE SERVE'S SHADOW IS SEEDED HERE (WHEEL_0 U2), from the target
    // that was just written rather than from the wheel — they are the
    // same point for a boot-born key (birth_the_choir sets inst.cx/cz
    // from birth_station) and for a spawned one the target is the patch,
    // which is where the serve must believe it is. Either way the shadow
    // records what the GPU was actually handed, which is the only thing
    // an epsilon gate may ever compare against.
    // THE BIRTH ANCHOR IS TAKEN FROM THE SAME WRITE, and it is the last
    // time either is authored from anything but the serve: the shadow
    // moves with every poke, the anchor never moves again.
    if (inst.slot < CUBE_CHOIR_N) {
        zcbs.wheel_sx[inst.slot] = fe.target_x;
        zcbs.wheel_sz[inst.slot] = fe.target_z;
        zcbs.birth_ax[inst.slot] = fe.target_x;
        zcbs.birth_az[inst.slot] = fe.target_z;
    }
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

// THE THREE DISPATCH FUNNELS — select, place and commit, the cube's arm of
// the three-phase pipeline. They stand unchanged and UNREACHABLE: the gate
// above refuses, so generic_select returns false before any of this runs.
// They keep their slots in FAMILY_DISPATCH's cube row for the positional
// reason stated at the gate, and the row's other slots — the census pair,
// the mesh pair — are live as ever, because a family that no longer spawns
// still has a population to count.
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
    if (host) { generic_commit(self, CUBE_TRAITS, CUBE_ADAPTER, pe.generic, queue); }
    else { self->cube_behaviors_state_.activeCubes_[pe.generic.slot].active = false; }
}


// ═══ THE CHOIR'S BIRTH (STAGE_0 U3) ══════════════════════════════
//
// The cube spawn path is gone. Cubes are not scattered by a patch roll
// any more — the instrument is AUTHORED, twenty-four keys born at boot
// and at every rebirth, the same twenty-four in every world.
//
// WHAT A BIRTH OWES, and it is the union of three things the pipeline
// used to do in three places:
//   1. RESERVE the slot. run_spawn_preamble set activeCubes_[k].active
//      at GATE time, which is why "KEY k = SLOT k by construction" was
//      ever true. With no gate, the birth reserves — and the law becomes
//      an assignment rather than a consequence, which is stronger.
//   2. cube_write_active — the MIRROR, the PRIOR every release walks back
//      to.
//   3. cube_write_gpu — the whole GPU slot, plus the choir bookkeeping
//      (the serve's shadow and the poke gate's seed; the walk shadow
//      and the settled flag retired at WHEEL_0 U3).
// Cubes touch no footprint registry (they are not grounded) and no patch
// registry (record_entity retired at ONE_SURFACE-I U3), so that is all of
// it.
//
// ONE SEED, AND THE SECOND ONE WAS THE WORLD'S (STAGE_0 R4).
//
// This block read "TWO SEEDS, AND THE SECOND ONE IS NOT OPTIONAL", and
// the second was tile_seed(world seed, patch_gx, patch_gz) — NOT ours to
// choose, it said, because the projector recomputed it from the mirror's
// patch coordinates on every poke, and that function was on the choir
// light's PROTECT LIST. So the birth seated a synthetic patch the
// projector could recompute from, and dressed inst.colors through the
// same seed or the first poke would have repainted every cube.
//
// THE BLOCK ENDED BY NAMING ITS OWN SUCCESSOR: "the BODIES are
// world-independent and the PALETTE is not — the same instrument wears a
// different finish in each world. That reads well and it is one line to
// change (CHOIR_SEED in place of the world seed), but the call site is
// protected and the choice is Jean's."
//
// ── THE PROTECT-LIST ENTRY IS FORMALLY AMENDED ──────────────────
// The choice is made and it is Jean's: the determinism commission
// outranks the protect list, and the protect list's own subject was the
// LIGHT — the envelope, the mix law, the silence bit-exactness — not the
// arithmetic that picks a base colour. WHEEL_0 already drew this line
// once, when organ_gap refused birth_station's PANEL_TABLE read: "what
// the protect list guards is the BODY draw; where a key STANDS is the
// wheel's business". Where a key's FINISH comes from is the same class
// of question, and the same answer.
//
// WHAT SURVIVES UNTOUCHED, because the amendment is narrow: the mix law
// (base + (light - base)*I), the variance's (1 - I) close, the swell,
// and THE SILENCE BIT-EXACTNESS — at I = 0 the projector still returns
// the mirror's own draw to the last bit. Only the arithmetic upstream of
// "base" moved.
//
// ONE SEED NOW: cpu_hash(CHOIR_SEED, k), for the body AND the palette.
// A key is the same key in every world, finish included, and the
// compiler proves the twenty-four are distinct (THE CHOIR band).

// `birth_station`'s PLACEHOLDER BODY stood here (WHEEL_0 U2) — angle
// 2πk/N on a 60 wu circle, declared a placeholder the day it was
// written. It moved up beside the geometry it now belongs to, where it
// is one line: the REST wheel's station for key k. The seam it was cut
// at held exactly: the birth still asks a function rather than computing
// a position inline, and nothing else in this file changed to re-aim it.

inline void birth_the_choir(MachineCtx* c, wgpu::Queue& queue) {
    for (uint32_t k = 0; k < CUBE_CHOIR_N; ++k) {
        const uint32_t tier = CHOIR_TIERS[k];
        const TierProfile& profile = cube_get_tier_profile(tier);

        // NO PATCH, AND THAT IS THE TRUTH ABOUT A BOOT-BORN CUBE
        // (STAGE_0 R4). A synthetic pair was seated here so the
        // projector's colour recompute would differ per key; the colour
        // is authored now, so the coordinates go back to the honest 0,0
        // that "belongs to no patch" means. Nothing reads them — the
        // mirror's patch_gx/gz had exactly one reader tree-wide and it
        // was the recompute.
        EntityInstance inst{};
        inst.family_id  = PopFamily::CUBE;
        inst.slot       = k;
        inst.tier_idx   = tier;
        inst.seed       = cpu_hash(CHOIR_SEED, k);   // THE SEED — body AND palette

        // The tier draws, off the body seed — the same sampler the
        // pipeline used, so a key's body is what the tier table says.
        for (uint32_t i = 0; i < CUBE_PARAM_COUNT; ++i) {
            const auto& pd = CUBE_PARAM_DEFS[i];
            float v = cpu_sample_gaussian(inst.seed, pd.prop,
                                          profile.params[i].mean, profile.params[i].sigma);
            // generic_select's clamp order, verbatim — round, floor,
            // then the ceiling only if it is a real one.
            if (pd.do_round) v = std::round(v);
            v = std::max(pd.floor, v);
            if (pd.ceiling < 1e29f) v = std::min(pd.ceiling, v);
            inst.params[i] = v;
        }
        cube_compute_solid_half(inst, profile);

        const WheelStation st = birth_station(k);
        inst.cx = st.off_x;
        inst.cz = st.off_z;

        // THE COLOURS, off the same seed the projector will use — which
        // is now inst.seed itself, so the borrowed-instance dance this
        // block used to do is gone. The invariant it protected is
        // unchanged and is what makes the silence bit-exact: base_color
        // and the projector's recompute must be ONE hash, or the first
        // poke repaints every cube.
        cube_compute_colors(inst, CUBE_TRAITS, cube_get_tier_profile(0));

        // THE MIRROR, THEN THE GPU SLOT — generic_commit's own two calls,
        // in its own order, minus the post_commit the cube adapter never
        // had. THE ORDER IS STILL LOAD-BEARING, for a narrower reason
        // than it was: cube_write_gpu's projector call no longer needs
        // the patch coordinates write_active seated (STAGE_0 R4 authored
        // the colour seed), but it DOES read the mirror's own body_radius
        // for the swell and its face_variance for the (1 - I) close, and
        // write_active is what seats those. write_active also sets
        // `active` and `last_alloc_time`, so it IS the reservation the
        // gate used to make.
        cube_write_active(c, inst);
        cube_write_gpu(c, inst, queue);
    }
    std::cout << "[CHOIR] born: " << CUBE_CHOIR_N << " keys, "
              << CUBE_CHOIR_RANKS << " rank(s), seed 0x"
              << std::hex << CHOIR_SEED << std::dec
              << " (bodies AND palette authored — one seed, 24 distinct keys)\n";
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
// seed recompute, renamed choir_slot_seed and standing below. It kept the
// lattice's arithmetic for two more campaigns and lost it at STAGE_0 R4:
// the name is all that is left of the inheritance now.

// ═══ THE CHOIR'S PROJECTOR — ONE HOME ════════════════════════════
//
// The successor to the lattice's projector. The base is DERIVED through
// the seed fn rather than cached — that half is the inheritance and it
// has not moved — but what the seed fn reads has: it drew
// tile_seed(active world seed, trigger patch) and reconstructed from the
// mirror's own coordinates until STAGE_0 R4 authored it as
// cpu_hash(CHOIR_SEED, key). The silent path is still bit-exact
// WHERE IT WAS BEFORE: at I = 0 the mix returns its base unchanged and
// the variance returns the spawn draw, both to the last bit. The base is
// the seed colour itself, in both modes, since WHEEL_0 U3 — the SCREEN
// dim it also carried retired with the screen, and with it the one place
// the silent path was NOT the mirror's own draw.
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

// THE COLOUR SEED, AUTHORED (STAGE_0 R4). It was inherited from the
// spawn gate — tile_seed(active world seed, trigger patch), reconstructed
// on every poke from the mirror's own patch coordinates — and that shape
// was a RECOMPUTE of something the pipeline had chosen. There is no
// pipeline: the choir is authored, so its palette is authored too.
//
// ONE KEY, ONE SEED, EVERY WORLD. cpu_hash(CHOIR_SEED, k) is the same
// hash the BODY draws from, which is the second half of the commission:
// the instrument is the same instrument everywhere, finish included.
//
// THE BODY AND THE PALETTE NOW SHARE ONE STREAM, and that is safe by
// property index rather than by luck: the colour triple draws at
// CubeProp COLOR_R/G/B (150/151/152), the nine tier params at
// {140,142,144,145,146,147,153,154,155} with their Gaussian partners at
// +1000, and the behaviour picks at 0xBEEF11A0 / 0xF10A7E70. Disjoint,
// all of them. It is still a collapse of what STAGE_0 called "two seeds,
// and the second one is not optional" into one, and it is named in the
// campaign report as such.
//
// IT TAKES A KEY AND NOTHING ELSE. No mirror, no world, no state — which
// is what let the distinctness witness up in THE CHOIR band be written at
// all.
inline uint32_t choir_slot_seed(uint32_t key) {
    return cpu_hash(CHOIR_SEED, key);
}

inline void choir_project_color(const CubeBehaviorsState& cbs, uint32_t slot,
    float& out_r, float& out_g, float& out_b) {
    const float I = choir_light(cbs, slot);
    EntityInstance tmp{};
    tmp.seed = choir_slot_seed(slot);
    // The seed fn's exact signature takes traits + tier; it reads neither
    // (both unnamed) — the call adapts, the law does not. G5 V2 verdict:
    // profile-INVARIANT — no profile field is consulted (and CUBE_TIERS'
    // color_var column is 0.0 in every row), so profile(0) is bit-exact
    // for every tier.
    cube_compute_colors(tmp, CUBE_TRAITS, cube_get_tier_profile(0));
    // THE DARK REST (V1) STOOD HERE and retired with the screen (WHEEL_0
    // U3). It dimmed the base to 0.30 in the two SCREEN states on the
    // reading that an instrument is dark until played — and C6R had
    // already ruled the other way for every state that is the WORLD's:
    // dimming a roaming swarm darkens the world, not an instrument. The
    // wheel is a formation IN the world, standing on its floor among the
    // ground and the ribbon, so it takes the world's ruling. The base is
    // the mirror's own draw, at full, in both modes.
    const float br = tmp.colors[0];
    const float bg = tmp.colors[1];
    const float bb = tmp.colors[2];
    const float* lc = DRIVER_LIVE.cube.light_color;
    out_r = br + (lc[0] - br) * I;
    out_g = bg + (lc[1] - bg) * I;
    out_b = bb + (lc[2] - bb) * I;
}

// THE ONE POKE HOME: every projector write for a slot goes through here,
// so no two paths can disagree about what a lit cube looks like.
//
// ALL THREE EXPRESSIONS POKE IN BOTH MODES NOW (WHEEL_0 U3). The light
// is the music's, not the formation's, and a roaming cube is as entitled
// to it as a seated one; colour and variance always were. THE SWELL
// JOINS THEM, because the thing that used to hold it back is gone: it
// spoke only when the screen STOOD, since the CPU walk owned body_radius
// through every TO_ state and two writers on one scalar in one frame is
// a fight. There is no walk and there are no TO_ states, so the
// projector is the ONE writer of body_radius outside the birth.
//
// WHAT THE MODE STILL DECIDES IS THE GAIN, not the write. On WHEEL the
// swell is live; on ROAM it is zero — and because the write happens
// either way it is SELF-RESTORING, exactly as the variance is: a cube
// swollen when the mode flips is walked back to the mirror's own draw by
// the forced pass the flip raises, instead of standing swollen forever
// waiting for a writer that no longer exists.
//
// AND THE SWELL IS RELATIVE NOW. It multiplied the screen's UNIFORM
// PIXEL, because on the screen every cube was one; the wheel does not
// touch bodies, so it multiplies THE MIRROR'S OWN DRAW and a Monolith
// swells like a Monolith.
inline void choir_project_slot(const CubeBehaviorsState& cbs, GPUState& gpu,
    wgpu::Queue& queue, uint32_t slot) {
    float cr, cg, cb;
    choir_project_color(cbs, slot, cr, cg, cb);
    gpu.upload_cube_color(queue, slot, cr, cg, cb);

    // GLOW UNIFIES: the spawn draw is the REST and the light closes it.
    // At I = 0 this is the mirror's bare draw to the bit — the silent
    // path — and at I = 1 one flat face.
    const float I = choir_light(cbs, slot);
    gpu.upload_cube_face_variance(queue, slot,
        cbs.activeCubes_[slot].face_variance * (1.0f - I));

    using Formation = CubeBehaviorsState::Formation;
    const float swell = (cbs.formation == Formation::WHEEL) ? ZOETROPE_SWELL_GAIN : 0.0f;
    gpu.upload_cube_body_radius(queue, slot,
        cbs.activeCubes_[slot].body_radius * (1.0f + swell * I));
}

// THE PER-FRAME FLUSH, POKE-ON-CHANGE. The lattice's flush hid behind a
// tick (0.25 beats); the choir has no tick to hide behind, so the gate is
// the light itself: a slot pokes only when its light MOVED past epsilon.
// A silent room pokes nothing at all, a sustained chord pokes only while
// it climbs, and the release pokes for exactly light_release beats. The
// repaint edge rides through as a FORCE. ONE THING RAISES IT NOW
// (WHEEL_0 U3), and it has the property all three of its predecessors
// shared: it changes what a cube LOOKS LIKE without moving its LIGHT, so
// it cannot be gated on the light.
//   · THE MODE FLIP, ROAM↔WHEEL, in reveal_zoetrope — the swell's gain
//     changes under the projector, and the whole reason the swell is now
//     written unconditionally is so this one edge can restore it.
// The three it replaces: the dim's two edges (V1 E3), which went with
// the dim, and THE ARRIVAL at every settle (CHOIR_0 U6b) — the walk had
// just snapped body_radius to the bare pixel and the formation changed
// underneath the projector, so the swell had to be re-asserted or a
// cube arriving under a HELD key stood unswollen until that key next
// moved. The first two had nothing left to dim; the third had nothing
// left to arrive.
inline void choir_project(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue) {
    const bool force = cbs.repaint_all;
    for (uint32_t slot = 0; slot < CUBE_CHOIR_N; ++slot) {
        if (!cbs.activeCubes_[slot].active) continue;
        const float I = choir_light(cbs, slot);   // the one door, here too
        if (!force && std::fabs(I - cbs.choir_flushed[slot]) <= CHOIR_FLUSH_EPS) continue;
        choir_project_slot(cbs, gpu, queue, slot);
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

inline void zoetrope_service(CubeBehaviorsState& cbs, GPUState& gpu, wgpu::Queue& queue) {
    // ── THE SERVE (WHEEL_0 U2) ── one pass, every frame, every key.
    //
    // Compute the key's station for the mode that is live, compare it
    // against the station the key was LAST SERVED AT, and poke the glide
    // door only if it moved past WHEEL_SERVE_EPS. That is the entire
    // per-frame cost of the formation: twenty-four cosines and a squared
    // distance, and zero bus traffic while the wheel is still.
    //
    // THE TWO MODES DIFFER ONLY IN WHICH WHEEL IS ASKED.
    //   WHEEL : PANEL_LIVE — the wheel as the hand has it. Turn `step`
    //           and the stations move continuously, so the keys BRAID.
    //   ROAM  : the recorded BIRTH ANCHORS — no wheel is read at all.
    //           The target stops chasing the dials and the cube walks
    //           back to where it was laid, after which the glide term
    //           decays to zero and DRIFT OWNS THE PICTURE.
    //
    // WHICH MEANS THE BOOT WHEEL IS THE FIXED POINT OF BOTH: until a
    // dial is turned the two arms name the SAME point, the epsilon gate
    // sees no motion, and the door press costs nothing but the
    // projector's forced pass. A mode flip only actually moves cubes
    // once the wheel has been turned away from where the choir was born
    // — which is the correct reading of what ROAM is FOR.
    //
    // XZ ONLY, and that is the whole of the wheel's jurisdiction over a
    // body. Height, radius and aspects belong to the tier draw and to
    // the swell; the wheel never writes one of them.
    //
    // NOTHING IS STAGED AND NOTHING WAITS A FRAME. The staged press
    // existed because a kite sentinel was in flight and would EAT a
    // target written before it was consumed; the sentinels retired at
    // STAGE_0 U4 and the last thing that needed them retires here.
    using Formation = CubeBehaviorsState::Formation;
    const bool on_wheel = (cbs.formation == Formation::WHEEL);
    for (uint32_t k = 0; k < CUBE_CHOIR_N; ++k) {
        if (!cbs.activeCubes_[k].active) continue;
        const WheelStation st = on_wheel ? wheel_station(PANEL_LIVE.wheel, k)
                                         : WheelStation{ cbs.birth_ax[k], cbs.birth_az[k] };
        const float tx = st.off_x, tz = st.off_z;
        const float dx = tx - cbs.wheel_sx[k];
        const float dz = tz - cbs.wheel_sz[k];
        if (dx * dx + dz * dz <= WHEEL_SERVE_EPS * WHEEL_SERVE_EPS) continue;
        // THE GLIDE DOOR IS THE WHOLE MECHANISM. The target is a GOAL in
        // world coordinates; update_cube walks the anchor toward it at
        // CUBE_GLIDE_TAU. No sentinel, no capture, no snap — a station
        // that leaps is a path that is walked.
        gpu.upload_cube_glide_target(queue, k, tx, tz);
        cbs.wheel_sx[k] = tx;
        cbs.wheel_sz[k] = tz;
    }

    // THE RESEAT WATCH, THE STAGE FRAME, THE SEAT PASS, THE CLIMB AND
    // THE HAND-BACK STOOD HERE (WHEEL_0 U3).
    //
    // The watch marked a possession seam — a per-frame step of the point
    // no motion could make — and answered it by re-entering the walk so
    // the screen re-formed around its new host. THE WHEEL HAS NO HOST.
    // It is anchored in the WORLD, about the world's centre, and a
    // possession moves the player, not the instrument; there is nothing
    // for it to follow and therefore nothing to watch.
    //
    // The climb was a CPU flush-walk on four scalars per seat with its
    // own tau, its own settle epsilon, its own shadow set and its own
    // arrival edge — the mechanism that made every transition a walk
    // back when the transition was the CPU's to run. The serve above
    // does the same job in six lines because it moves a GOAL and lets
    // the kernel do the walking, which is the law the climb was written
    // to obey by hand.
    //
    // The hand-back aimed each target at the mirror's live_pos so a
    // released cube stopped where it stood. ROAM aims at the recorded
    // BIRTH ANCHOR instead, which is a stronger let-go: a cube walks
    // home to the shape it was laid in rather than freezing wherever the
    // last turn of the wheel happened to leave it, and drift takes it
    // from there.
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
