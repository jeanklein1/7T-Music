#pragma once

// ─── coupling/visual_canvas.hpp ──────────────────────────────────────────────
//
// The visual canvas — the coupling layer's binding surface, dual of the
// musical Canvas. It consumes the analysis contract and writes the visual
// parameter bank; it touches no GPU. Where the musical Canvas reads MIDI,
// composes readings, and publishes AnalysisSignal, this consumes that signal,
// runs each coupling, and drives VisualParams. Entities read the bank and
// upload what moved — they never learn what drove them.
//
// THREE REGIONS
//   couplings            — the decode tuning each coupling reads (tables,
//       spans). The decode itself is inline simple-math in tick(), not a goal
//       object: AffineGoal serves the affine cases, custom decodes stay inline.
//   master control panel — PARAM_LAYOUT: every exposed pipe, its slot, width,
//       and rest. Slots are laid by hand in this one table, so no collisions.
//   the canvas           — bind() resolves every source and target by name
//       once; tick() runs the couplings each frame. Resolve once, never per
//       frame.
//
// FIRST COUPLING — fog. The held field (a one-based rank, 0 = none) selects a
// fog density from FOG_BY_FIELD and an atmospheric tint from FOG_COLOR_BY_FIELD;
// the canvas emits each as a DEVIATION from the anchor row (ATMOS_1), and the
// world's own rest is composed in at the cartridge's seam — v3 §2, scalar
// deviations over inviolate idleness. Segments carry both, so density and
// color drift across a modulation instead of snapping. The source,
// "all.field", is already published, so the analysis side is untouched.
//
// WIRING (live). The cartridge owns a VisualCanvas, binds it once in
// bind_signal_layout with the analysis layout, ticks it each frame in
// update() after the signal, and flushes fog — density and color — from
// params() to set_fog as the world's rest + gain · deviation. Fog has one driver:
// the field.
//
// CHECKER-REBUILD — THE PITCH-CLASS COLOR FIELD (the terrain's checker
// voice). The voice's WINDOW pc-LENGTH vector — pc_length(playhead,
// wagon(0)): the Playhead + Wagon compound, duration-weighted, dressed
// to D — is read every canvas::CANVAS_LIVE.checker_read_span beats. NO DFT, no interval
// math: each ABSOLUTE pitch class (index 0 = D) has an authored RGB
// (PC_COLOR), and the decode is the length-weighted average color,
// resultant = (Σ length_i · PC_COLOR[i]) / Σ length_i. Two enveloped
// scalars ride with it: music_amount (presence [0,1]) and music_variance
// (distinct-pc count). The ENVELOPE is Jean's: LINEAR 2-beat attack to
// the new 4-beat target, LINEAR 8-beat release to rest — a return to
// SEED (music_amount → 0, which the GPU maps to the cell's own seed
// color), not to gray. The cartridge flushes terrain.checker_* through
// set_checker_color_field in U4; the GPU (discrete_cell_color) pulls
// each discrete cell toward the resultant, wanders each region around
// it (re-rolled per window), and widens each region's own spread by the
// count — applied DIRECTLY to the checker cells. Console witness:
// [CHECKER] per read.
//
// THE CUBE CHOIR — THE LIGHT INSIDE THE CUBES. Channel 6 is cast as the
// cube voice, and its PRESENT COUNT (twelve lanes, sounding notes per
// pitch class) plays a keyboard of CHOIR_LANES keys read as stacked
// pianos: key k is rank k/12 of raw pitch class k%12, and key k IS cube
// slot k BY ASSIGNMENT — the cartridge's boot-born choir writes the
// identity (it was a consequence of the lowest-free-slot reservation
// until the spawn law was repealed at STAGE_0 R5; the reservation and
// the eviction it answered are both gone). ACTIVATION AND DEACTIVATION
// ONLY — no held-length book is kept;
// the envelope is the memory. While a key sounds its light climbs the
// house's saturating-approach curve, 1 − e^(−t/τ) with τ =
// light_plateau/6, so ≈ 99.75% at the plateau — steepest at switch-on,
// which IS the fast attack the commission asked for; when
// it falls silent the light falls on a FIXED SLOPE, full brightness to
// dark in light_release beats. Fast attack, slow decay, simple. The run
// leaves through "cube.light"; the cartridge's motion-drivers phase
// composes it against the drivers' room and mirrors it to the cubes.
// Console witness: [CHOIR] on the activation edge.
//
// USAGE
//   visual_canvas_.bind(analysis_layout);          // startup
//   visual_canvas_.tick(signal);                   // per frame, after analysis
//   ...
//   auto d = visual_canvas_.layout().resolve("fog.density");   // entity, once
//   auto k = visual_canvas_.layout().resolve("fog.color");     // base..base+2
//   float density = visual_canvas_.params().get(d.base);       // entity flush
//
// Depends on: coupling/visual_params.hpp, coupling/trajectory.hpp,
//             musical/signal_layout.hpp, analysis/analysis_signal.hpp,
//             <string>, <cmath>, <algorithm>.

#include "coupling/visual_params.hpp"
#include "coupling/trajectory.hpp"
#include "core/instruments.hpp"   // THE INSTRUMENTS DIAL: INSTRUMENTS.checker_witness gates the [CHECKER] line
#include "musical/signal_layout.hpp"
#include "analysis/analysis_signal.hpp"
#include <string>    // casting-sheet name composition ("<voice>.present_count")
#include <array>     // the hue unit-vector table (OIL_1 U5)
#include <cstddef>   // size_t — the table's index casts (OIL_1 U5)
#include <cmath>     // std::floor / cos / sin / sqrt / exp — decode math (exp: the choir's attack)
#include <algorithm> // std::min/std::max — decode clamps
#include <cstdio>    // std::fprintf — the [CHECKER] and [CHOIR] witness lines
#include "coupling/canvas_surface.hpp"   // ORGAN_3b P2 — CANVAS_LIVE: the envelope authorities' live surface

namespace t7 {

    // ═══ COUPLINGS ═══════════════════════════════════════════════════════════════
    // Each coupling's tuning sits with the coupling; the decode runs inline in
    // tick(). Whether a coupling has an idle depends on its source: a source that
    // can fall quiet (a count, a magnitude) has a rest the value returns to; a
    // held source never quiets, so its coupling goes value-to-value, no idle.

    // Fog — the held field selects a density; the canvas emits the DEVIATION of
    // that density from the anchor row (ATMOS_1). The field is a held source:
    // once a scale is established it persists through silence, so fog never returns
    // to a rest — it moves from one field's density to the next. FIELD 1 IS THE
    // ANCHOR: the value index 0 also carries, and the zero point of the deviation
    // the pipe now carries (ATMOS_1) — field 1 reads as "no deviation", and every
    // other field as a shift away from it, composed at the seam over whatever rest
    // the WORLD wears. The absolute values stay in the
    // table because that is how a composer reads them; the subtraction is one
    // line in tick(). Fields 5/6 sit in the dense band, 2/3/4 in the light.
    // Index 0 is "no field yet" — the value at boot, before any scale is held, not
    // an idle. Tunable.
    inline constexpr int   FOG_FIELD_COUNT = 7;          // index 0 = none, 1..6 fields
    // THE ANCHOR — one home for both rows that wear it. Twinned by the boot
    // config in realization/state.hpp (config_.fog_density / fog_color) and by
    // ATMOS_TABLE's fog rest (contracts/atmosphere_surface.hpp): the bank's rest
    // and the canvas's zero point are the same number by construction, which
    // is what keeps gain 1 on the sunset the pre-ATMOS_1 picture exactly.
    inline constexpr float FOG_DENSITY_NONE  = 0.0030f;
    inline constexpr float FOG_COLOR_NONE[3] = { 0.85f, 0.78f, 0.72f };
    inline constexpr float FOG_BY_FIELD[FOG_FIELD_COUNT] = {
        FOG_DENSITY_NONE,   // 0  none   — no field yet (boot)
        FOG_DENSITY_NONE,   // 1  anchor — the open outdoor atmosphere
        0.0022f,            // 2  light
        0.0026f,            // 3  light
        0.0020f,            // 4  light
        0.0050f,            // 5  dense
        0.0058f,            // 6  dense
    };

    // Fog color — the same held field selects an atmospheric tint, carried per
    // channel so the hue drifts with the density. Tiers 0 and 1 both wear the
    // anchor; the rest are shifts away from it. Same held source, so the same
    // value-to-value behavior — no idle. Tunable.
    //                                                  R       G       B
    inline constexpr float FOG_COLOR_BY_FIELD[FOG_FIELD_COUNT][3] = {
        { FOG_COLOR_NONE[0], FOG_COLOR_NONE[1], FOG_COLOR_NONE[2] },   // 0  none   — no field yet
        { FOG_COLOR_NONE[0], FOG_COLOR_NONE[1], FOG_COLOR_NONE[2] },   // 1  anchor — the open outdoor atmosphere
        { 0.78f, 0.80f, 0.82f },   // 2  cool pale
        { 0.80f, 0.82f, 0.76f },   // 3  faint sage
        { 0.74f, 0.78f, 0.86f },   // 4  soft blue
        { 0.92f, 0.72f, 0.55f },   // 5  warm amber
        { 0.70f, 0.68f, 0.80f },   // 6  muted violet
    };

    // One span carries both fog pipes across a field change; split into a second
    // constant if color should lead or lag density.

    // ── Casting (the avatar principle) ── one voice per entity; the set
    // of these is the CASTING SHEET. The ribbon is the chordal piano.
    inline constexpr const char* RIBBON_VOICE = "ch1";   // live prefix verified: chN (canvas_1 NAME_* tables)

    // ── THE CUBE CHOIR ── ONE VOICE, CAST (the avatar principle again):
    // channel 6 is the cube voice. `ZOETROPE_EARS = 0b0111'1111` — a
    // seven-channel listener SET — retired here with the lattice it fed;
    // the narrowing to {ch6} that band already named as the ruling is
    // what a cast voice IS, so the set collapses to a name.
    inline constexpr const char* CHOIR_VOICE = "ch6";   // chN = wire = Ableton − 1

    // THE KEYBOARD'S WIDTH, canvas-side. The PIPE is this wide; the
    // POPULATION cap that reads it is the cartridge's own
    // (the_board::CUBE_CHOIR_N), and the seam static_asserts that the
    // choir fits the pipe. They are deliberately two constants: the
    // canvas may not name a cartridge, and a pipe wider than the choir
    // costs only rest-valued lanes, so flipping the choir to 24 stays
    // the one token the commission asked for.
    inline constexpr int CHOIR_LANES = 36;
    static_assert(CHOIR_LANES % 12 == 0, "the choir is stacked pianos");

    // ── The keyboard fold (the un-dressing) ── THE INVERSE PAIR.
    // Published vectors ship DRESSED to D: index 0 = D, so dressed
    // index i reads RAW pitch class (i + 2) % 12 — the fold the
    // zoetrope's ears used, and the contract PC_COLOR binds too. The
    // keyboard is authored the other way round, in RAW pitch class
    // (0 = C, the bottom of a rank), so it needs that map INVERTED:
    // raw pc p sits at dressed index (p + 10) % 12. Read the two
    // together — (i + 2) and (p + 10) are inverses mod 12 — and the
    // pair is checkable by eye rather than by trust.
    inline constexpr int dressed_of_pc(int raw_pc) { return (raw_pc + 10) % 12; }
    static_assert(dressed_of_pc(2) == 0, "D is the dressed origin");
    // THE ROUND TRIP, ALL TWELVE — a spot check would pass on a fold that
    // is right for two values and wrong for ten. This walks every dressed
    // index through the ears' `(i + 2) % 12` and back, and demands the
    // identity on all of them.
    static_assert([] {
        for (int i = 0; i < 12; ++i)
            if (dressed_of_pc((i + 2) % 12) != i) return false;
        return true;
        }(), "dressed_of_pc must invert the ears' (i + 2) % 12 fold, on every lane");

    // ── Sustain swell (movement) ── PURE ADDITIVE: the dance is the seed
    // idle PLUS the chord's contribution. goal = 1 + (CEILING−1)·t where
    // t ramps over the hold; silence gives 1 from the formula itself —
    // no branch, identity by construction. Music only ever gives;
    // idleness is inviolate. RULED: ceiling 2× idle at 8 beats.
    // Envelope: the swell's goal is continuous during the ramp, so ATTACK
    // engages only at discontinuities (rare); RELEASE governs the breath
    // on re-articulation and the let-go after silence. Fast catch, slow
    // let-go. (A separate BREATH span for re-articulation is one line if
    // the dip wants independence from the final release — say the word.)

    // canvas::CANVAS_LIVE.pitch_vec_origin survives the compass redesign: the tint's angle
    // law and the swappable seating live on it.

    // ── Line tint (color gen-2) ── the melody paints the ribbon: the
    // line's degree sets a hue by the SAME 30°-per-semitone law as the
    // compass (shared ORIGIN ⇒ cross-channel equivariance). The stimulus
    // is a TINTING VOICE at authored luma/chroma, mixed over the spawn
    // color; mix rises while the line sounds, releases to 0 in silence —
    // rest = the seed-drawn ribbon exactly. Compositional dials.
    // Envelope: the mix catches the room quickly and fades long on its
    // last hue; the hue itself re-aims between actives on one span.
    // Rodrigues basis about the gray axis (canvas-side twin of the skin's):
    inline constexpr float TINT_D1[3] = { 0.8165f, -0.4082f, -0.4082f };
    inline constexpr float TINT_D2[3] = { 0.0f,     0.7071f, -0.7071f };

    // ── DOOR AXES (Movement 1 harvest) ── signed coverage spans from the
    // door algebra at the tested ±0.80 sweep; the Movement-2 coupling
    // maps goals into these. Dials — nudge by taste.
    inline constexpr float TIDE_SHIFT_MIN   = -0.65f;  // zones fully closed
    inline constexpr float TIDE_SHIFT_MAX   =  0.75f;  // full flood
    inline constexpr float RAIN_SCATTER_MIN = -0.80f;  // countryside extinct (densest clusters linger)
    inline constexpr float RAIN_SCATTER_MAX =  0.25f;  // storm saturation

    // ── CHECKER-REBUILD — the pitch-class color field (the checker voice) ──
    // Source: the voice's WINDOW pc-LENGTH vector (window_length, 12-wide,
    // duration-weighted, dressed to D — the Playhead + Wagon compound,
    // pc_length(playhead, wagon(0))). NO DFT, no interval math: each PITCH
    // CLASS (ABSOLUTE, index 0 = D after the dress) has an authored RGB, and
    // the decode is the length-weighted average color —
    //     resultant = ( Σ_i length_i · PC_COLOR[i] ) / max(Σ length_i, eps)
    // Read every canvas::CANVAS_LIVE.checker_read_span beats. ENVELOPE (per Jean): LINEAR 2-beat
    // attack to the new target; LINEAR 8-beat release to rest (music_amount →
    // 0, which the GPU maps to each cell's OWN seed color — a return to seed,
    // not to gray). Two enveloped scalars ride alongside the resultant:
    //   music_amount  = presence [0,1] — the GPU's S1 pull + S2 wander scale;
    //   music_variance = distinct-pc count — the GPU's S3 within-patch spread.
    // The GPU (world.wgsl discrete_cell_color) owns pull / wander / spread;
    // this side ships the resultant + the two scalars through terrain.checker_*.
    // PC_COLOR is JEAN'S — twelve hues, one per pitch class. Tune it here.
    inline constexpr const char* CHECKER_VOICE = "ch1";   // the chordal piano; chN = wire = Ableton − 1
    //                          pc (dressed, 0 = D)      R      G      B    — Jean's twelve hues
    inline constexpr float PC_COLOR[12][3] = {
        /*  0  D  */ { 0.85f, 0.20f, 0.20f },   // red
        /*  1  D# */ { 0.85f, 0.45f, 0.15f },   // orange
        /*  2  E  */ { 0.90f, 0.80f, 0.20f },   // yellow
        /*  3  F  */ { 0.55f, 0.80f, 0.25f },   // yellow-green
        /*  4  F# */ { 0.25f, 0.75f, 0.30f },   // green
        /*  5  G  */ { 0.20f, 0.75f, 0.65f },   // teal
        /*  6  G# */ { 0.20f, 0.65f, 0.85f },   // cyan
        /*  7  A  */ { 0.25f, 0.40f, 0.85f },   // blue
        /*  8  A# */ { 0.45f, 0.30f, 0.85f },   // indigo
        /*  9  B  */ { 0.65f, 0.25f, 0.85f },   // violet
        /* 10  C  */ { 0.85f, 0.25f, 0.70f },   // magenta
        /* 11  C# */ { 0.85f, 0.25f, 0.45f },   // crimson
    };
    // Within-patch spread (S3): this side ships the ENVELOPED distinct-pc
    // count surplus (max(0, n-1), glided). The per-note gain and the ceiling
    // live GPU-side (world.wgsl §2.2 ROW 5: CHECKER_VAR_PER_NOTE /
    // CHECKER_VAR_MAX) so they hot-reload. 0 or 1 distinct → 0 extra spread.

    // ═══ MASTER CONTROL PANEL ════════════════════════════════════════════════════
    // The one place every exposed pipe is declared — name, slot, width, and the
    // value it rests at. Slots are assigned here, by hand, in this single table, so
    // there are no collisions across entities. Read it as a register map; every
    // coupling and every entity flush resolves against it by name. (A vector's rest
    // is one value across its channels. Both fog pipes rest at 0 since ATMOS_1:
    // the canvas emits DEVIATIONS from its anchor row, and the world's own rest is
    // composed in at the U4 seam — the same shape the ribbon pipes below wear.)
    //
    //                          name           base count   rest
    inline constexpr ParamSlot PARAM_LAYOUT[] = {
        { "fog.density",          0,    1,    0.0f },   // deviation from the anchor (ATMOS_1)
        { "fog.color",            1,    3,    0.0f },   // per channel, same law
        // ── ribbon (pitch compass) ── deviations composed over the seed
        // draws at the entity flush; rest = identity (1 = the seed's dance).
        { "ribbon.amp_lateral_mult",  4, 1, 1.0f },
        { "ribbon.amp_vertical_mult", 5, 1, 1.0f },
        { "ribbon.color_stim", 6, 3, 0.0f },
        { "ribbon.color_mix",  9, 1, 0.0f },
        // ── terrain (CHECKER-REBUILD, the pc-color field) ── checker_mean
        // now carries the resultant COLOR (rgb); checker_var widens to TWO —
        // [0] = music_amount (presence), [1] = music_variance (distinct-pc
        // count). All rest at 0 (amount 0 → the GPU shows each cell's seed
        // color; the_board's authored rests: terrain_looks ROW 2 REST_CHECKER_*).
        { "terrain.checker_mean", 10, 3, 0.0f },
        { "terrain.checker_var",  13, 2, 0.0f },
        // ── the cube choir ── one lane per key, the ENVELOPED light the
        // cartridge mirrors into the cubes. Rest 0 is DARK, and dark is
        // the seed draw exactly: the projector's silent path composes
        // nothing at I = 0. Base 15 is the first free slot after the
        // terrain run; the bank goes 15/256 → 51/256 allocated.
        { "cube.light", 15, CHOIR_LANES, 0.0f },
        // ── the ground's voice ── TWO PIPES THAT CARRY SOURCES, not
        // targets, and they are the only two in this table that do.
        // Every other pipe carries a decoded, target-shaped value and
        // the seam composes it LINEARLY (`rest + gain·(driven − rest)`).
        // The ground's law is not linear in its gain: the gain sits
        // INSIDE the expression (`1 + g·field`, `1/(1 + g·dens)`), so
        // splitting it canvas-side would change what the gain MEANS —
        // at the authored tick gain the linear form could only ever
        // shorten the period by 15%, where the ruled form reaches the
        // clamp on a dense chord. The law and its desk numbers were
        // authored together, so the law stays whole at the seam and
        // these two carry what it needs.
        // REST 0 IS THE NEUTRALITY, and it is the fog's convention: the
        // `1 +` in the law is what makes silence a multiplier of exactly
        // one, so no rest constant has to promise it.
        { "ground.energy",  51, 1, 0.0f },   // all.field — the room's held energy
        { "ground.density", 52, 1, 0.0f },   // Σ all.current_pc — voices sounding
        // ── the strike ── twelve one-frame impulse lanes, one per pitch
        // class: 1.0 on any rank's activation edge, else 0. A
        // SOURCE-shaped pipe like the ground's two; the cartridge maps
        // pc → wheel station and stamps the terrain ring (STRIKE_0).
        { "ground.strike", 53, 12, 0.0f },
    };
    inline constexpr uint32_t PARAM_LAYOUT_COUNT =
        sizeof(PARAM_LAYOUT) / sizeof(PARAM_LAYOUT[0]);

    // WITNESS — the register map's teeth: every pipe within the bank,
    // no two pipes overlapping. Hand-laying stays; a collision is now a
    // build error, not a silent cross-write.
    static_assert([] {
        for (uint32_t i = 0; i < PARAM_LAYOUT_COUNT; ++i) {
            const ParamSlot& a = PARAM_LAYOUT[i];
            if (a.count < 1) return false;
            if (a.base < 0 || a.base + a.count > VISUAL_PARAM_SLOTS) return false;
            for (uint32_t j = i + 1; j < PARAM_LAYOUT_COUNT; ++j) {
                const ParamSlot& b = PARAM_LAYOUT[j];
                if (a.base < b.base + b.count && b.base < a.base + a.count) return false;
            }
        }
        return true;
        }(), "PARAM_LAYOUT: a pipe leaves the bank or two pipes overlap");

    // ═══ VISUAL CANVAS ═══════════════════════════════════════════════════════════

    class VisualCanvas {
    public:
        // Startup wiring: publish the control panel, lay the bank to its rests,
        // adopt the analysis layout, and resolve every coupling's source and target
        // once. tick() then never resolves.
        void bind(StatLayoutView analysis_layout) {
            param_layout_.bind(ParamLayoutView{ PARAM_LAYOUT, PARAM_LAYOUT_COUNT });
            param_layout_.reset(params_);

            signal_layout_.bind(analysis_layout);

            // fog: the held field → a density and a tint, each a DEVIATION from
            // the anchor row (ATMOS_1); index 0 is the anchor, so the Segments
            // start at 0 — no deviation yet.
            fog_field_ = signal_layout_.resolve("all.field");
            fog_density_ = param_layout_.resolve("fog.density");
            fog_color_ = param_layout_.resolve("fog.color");
            fog_seg_ = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };
            for (int c = 0; c < 3; ++c)
                fog_color_seg_[c] = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };

            // ribbon sources (the casting sheet): the voice's Playhead drives
            // the sustain swell; the room's Wagon aims the tint's hue; the
            // room's Playhead gates the tint's mix.
            {
                std::string v(RIBBON_VOICE);
                voice_playhead_ = signal_layout_.resolve((v + ".present_count").c_str());
            }
            room_wagon_ = signal_layout_.resolve("all.window_length");
            room_playhead_ = signal_layout_.resolve("all.present_count");
            amp_lat_ = param_layout_.resolve("ribbon.amp_lateral_mult");
            amp_vert_ = param_layout_.resolve("ribbon.amp_vertical_mult");
            amp_lat_seg_ = Segment{ 1.0f, 1.0f, 0.0f, 0.0f };
            amp_vert_seg_ = Segment{ 1.0f, 1.0f, 0.0f, 0.0f };
            tint_stim_ = param_layout_.resolve("ribbon.color_stim");
            tint_mix_ = param_layout_.resolve("ribbon.color_mix");
            for (int c2 = 0; c2 < 3; ++c2)
                tint_stim_seg_[c2] = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };
            tint_mix_seg_ = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };

            // CHECKER-REBUILD source + targets (the terrain's checker voice):
            // the voice's WINDOW pc-length vector becomes the resultant color;
            // presence + distinct-pc count envelope the pull and the spread.
            {
                std::string v(CHECKER_VOICE);
                checker_win_ = signal_layout_.resolve((v + ".window_length").c_str());
            }
            checker_mean_ = param_layout_.resolve("terrain.checker_mean");   // 3: resultant rgb
            checker_var_  = param_layout_.resolve("terrain.checker_var");    // 2: amount, variance
            for (int c2 = 0; c2 < 3; ++c2) {
                checker_res_goal_[c2] = 0.0f;                     // resultant color, held between reads
                checker_res_seg_[c2] = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };
            }
            checker_amount_goal_ = 0.0f;                          // presence (rest 0 → seed)
            checker_amount_seg_  = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };
            checker_var_goal_ = 0.0f;                             // distinct-pc spread (rest 0)
            checker_var_seg_  = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };
            checker_next_read_ = 0.0f;   // first frame reads, then grid-locks

            // THE GROUND'S EARS. `all.field` is NOT re-resolved: the fog
            // already binds it as fog_field_, and one home means one
            // resolve — the ground is its SECOND READER, not a second
            // binding. `all.current_pc` is new: the room's per-pc voice
            // count, one of the atlas's unheard names, summed in the
            // decode into a polyphonic density.
            room_current_pc_ = signal_layout_.resolve("all.current_pc");
            ground_energy_  = param_layout_.resolve("ground.energy");
            ground_density_ = param_layout_.resolve("ground.density");

            // THE CHOIR'S ONE EAR (the casting sheet): the cube voice's
            // PRESENT COUNT — twelve lanes, the count of sounding notes
            // per pitch class, zero in silence. Already published by
            // canvas_1 for every voice; the analysis side is untouched.
            {
                std::string v(CHOIR_VOICE);
                choir_ear_ = signal_layout_.resolve((v + ".present_count").c_str());
            }
            for (int k = 0; k < CHOIR_LANES; ++k) choir_I_[k] = 0.0f;
            choir_sounding_ = 0ull;
            choir_target_ = param_layout_.resolve("cube.light");
            strike_target_ = param_layout_.resolve("ground.strike");
            // Boot witness — doctrine, not measurement (P6): one line,
            // always, so a deaf choir names its fault at the seam.
            // THREE WIDTHS NOW, AND THIS LINE MAY ONLY CLAIM THE TWO IT
            // OWNS. The EAR is twelve lanes (a pitch-class vector); the
            // PIPE is CHOIR_LANES lanes stacked over it, and this canvas
            // envelopes every one of them. THE KEYBOARD IS NEITHER — it
            // is CUBE_CHOIR_N, the cartridge's own fact, which this tier
            // may not name and which the seam's own witness prints
            // ("[the_board] cube.light … | choir N key(s), R rank(s)").
            // CHOIR_1 pulled those apart: at 24 keys against 36 lanes,
            // a line here saying "36 keys" would be false about the
            // instrument, so it says LANES and leaves keys to the room
            // that knows. The twelve unread lanes are envelope work
            // nobody reads — real and cheap, flagged not fixed, because
            // narrowing the pipe is what CHOIR_0 banked against.
            std::fprintf(stderr, "[CHOIR] ear %s: %s.present_count (12 pc lanes)"
                                 " -> %d envelope lane(s); keys are the seam's\n",
                choir_ear_.valid ? "bound" : "UNBOUND", CHOIR_VOICE,
                CHOIR_LANES);

            // PORT_4c — THE SOCKET, in one line. Every signal-side
            // resolve above happens here. Against canvas_1's published
            // layout all twelve bind and this line stays silent; it
            // speaks only for a name absent from whatever layout the
            // console bound. The release twin
            // prints this summary; the debug twin has already printed
            // each source by name. Placed last, after the resolves it
            // counts, beside the Zoetrope witness it deliberately does
            // not replace — that line reports a different fact.
            if (signal_layout_.misses() > 0) {
                std::fprintf(stderr,
                    "[SignalLayout] %u sources unbound (no audio source)\n",
                    signal_layout_.misses());
            }
        }

        // One frame: run every coupling — read its source, decode inline, carry the
        // value on its Segment, write the bank. No GPU.
        void tick(const AnalysisSignal& signal) {
            const float beat = signal.t_beats;

            // ── fog ──────────────────────────────────────────────────────────────
            // The held field selects a density and an atmospheric tint; the canvas
            // emits each as a DEVIATION from the anchor row (ATMOS_1), and the
            // cartridge's seam composes it over the world's own rest. Segments carry
            // both so they drift across a modulation rather than snapping. One
            // source, two pipes. Decode is a table index — inline, not a goal
            // object.
            if (fog_field_.valid) {
                const int f = (int)signal.stat(fog_field_.channel, fog_field_.base);
                const int idx = (f >= 0 && f < FOG_FIELD_COUNT) ? f : 0;

                if (fog_density_.valid) {
                    params_.set(fog_density_.base,
                        trajectory_release(fog_seg_, FOG_BY_FIELD[idx] - FOG_BY_FIELD[0],
                                           beat, canvas::CANVAS_LIVE.fog_span));
                }
                if (fog_color_.valid) {
                    for (int c = 0; c < 3; ++c) {
                        params_.set(fog_color_.base + c,
                            trajectory_release(fog_color_seg_[c],
                                FOG_COLOR_BY_FIELD[idx][c] - FOG_COLOR_BY_FIELD[0][c],
                                beat, canvas::CANVAS_LIVE.fog_span));
                    }
                }
            }

            // ── sustain swell (movement = TIME, the ribbon's voice) ─────
            // The dance swells with how long the current chord has held,
            // uninterrupted, on ch1's Playhead. Any change to the sounding
            // SET re-articulates: breathe to baseline (1), regrow.
            // Ruled: 1→2× over 8 beats. Silence ⇒ 1 ⇒ the seed dance.
            if (voice_playhead_.valid && amp_lat_.valid && amp_vert_.valid) {
                uint32_t mask = 0u;
                for (int i = 0; i < 12; ++i)
                    if (signal.stat(voice_playhead_.channel,
                        voice_playhead_.base + i) > 0.0f)
                        mask |= (1u << i);
                const float dbeats = beat - last_beat_;
                if (mask == 0u || mask != hold_mask_) hold_beats_ = 0.0f;
                else if (dbeats > 0.0f)               hold_beats_ += dbeats;
                hold_mask_ = mask;

                // One expression: hold==0 (silence or fresh chord) gives
                // goal 1 by itself. Re-articulation breathes to BASELINE.
                const float t = (hold_beats_ < canvas::CANVAS_LIVE.swell_ramp)
                    ? hold_beats_ / canvas::CANVAS_LIVE.swell_ramp : 1.0f;
                const float goal = 1.0f + (canvas::CANVAS_LIVE.swell_ceiling - 1.0f) * t;
                params_.set(amp_lat_.base,
                    trajectory_release(amp_lat_seg_, goal, beat,
                        (goal == 1.0f ? canvas::CANVAS_LIVE.swell_release : canvas::CANVAS_LIVE.swell_attack)));
                params_.set(amp_vert_.base,
                    trajectory_release(amp_vert_seg_, goal, beat,
                        (goal == 1.0f ? canvas::CANVAS_LIVE.swell_release : canvas::CANVAS_LIVE.swell_attack)));
            }

            // ── room tint (color = the room) ────────────────────────────
            // The Wagon AIMS the hue (remembered center of mass — no argmax
            // flicker on chords); the Playhead GATES the mix (sounding ⇒
            // worn; silence ⇒ fades on its last hue).
            if (room_wagon_.valid && tint_stim_.valid && tint_mix_.valid) {
                // Unit-vector seating: the SWAPPABLE TABLE (one line).
                // Chromatic today (i·30°); circle of fifths ((7i mod 12)
                // ·30°) or an authored ordering are one-line futures —
                // the circle rework is PARKED with Jean's name on it; the
                // swappable line is now the th expression in the fill.
                // OIL_1 U5 (ledger: U4 hue loop, C4): the 12 angles are
                // compile-time-stable, so the vectors are seated ONCE — a
                // function-local static filled by the SAME std::cos/std::sin
                // expressions (identical bits by construction; cos/sin are
                // not constexpr in C++20). The per-frame loop reads the
                // table; only the weights vary.
                static const std::array<std::array<float, 2>, 12> PITCH_VECS = [] {
                    std::array<std::array<float, 2>, 12> t{};
                    for (int j = 0; j < 12; ++j) {
                        const float th = canvas::CANVAS_LIVE.pitch_vec_origin + (float)j * 0.523598776f;
                        t[(size_t)j] = { std::cos(th), std::sin(th) };
                    }
                    return t;
                }();
                float vx = 0.0f, vy = 0.0f, energy = 0.0f;
                for (int i = 0; i < 12; ++i) {
                    const float w = signal.stat(room_wagon_.channel, room_wagon_.base + i);
                    if (w <= 0.0f) continue;
                    vx += w * PITCH_VECS[(size_t)i][0]; vy += w * PITCH_VECS[(size_t)i][1];
                    energy += w;
                }
                const float len = std::sqrt(vx * vx + vy * vy);
                if (len > 1e-4f) {
                    const float ca = vx / len, sa = vy / len;   // hue direction, no atan needed
                    for (int c2 = 0; c2 < 3; ++c2) {
                        const float v = canvas::CANVAS_LIVE.tint_luma
                            + (TINT_D1[c2] * ca + TINT_D2[c2] * sa) * canvas::CANVAS_LIVE.tint_chroma;
                        params_.set(tint_stim_.base + c2,
                            trajectory_release(tint_stim_seg_[c2], v, beat, canvas::CANVAS_LIVE.tint_hue_span));
                    }
                }
                else {
                    // window drained: stim segments hold their last hue; the
                    // MIX below is what releases — fade, not gray-out.
                    for (int c2 = 0; c2 < 3; ++c2)
                        params_.set(tint_stim_.base + c2,
                            trajectory_release(tint_stim_seg_[c2],
                                tint_stim_seg_[c2].to, beat, canvas::CANVAS_LIVE.tint_hue_span));
                }

                float room_sounding = 0.0f;
                if (room_playhead_.valid)
                    for (int i = 0; i < 12; ++i)
                        room_sounding += signal.stat(room_playhead_.channel,
                            room_playhead_.base + i);
                const float mix_goal = (room_sounding > 0.0f) ? canvas::CANVAS_LIVE.tint_mix_max : 0.0f;
                params_.set(tint_mix_.base,
                    trajectory_release(tint_mix_seg_, mix_goal, beat,
                        (mix_goal == 0.0f ? canvas::CANVAS_LIVE.tint_mix_release : canvas::CANVAS_LIVE.tint_mix_attack)));
            }

            // ── CHECKER-REBUILD (the window pc-lengths → a resultant color) ──
            // SAMPLE-AND-HOLD on the absolute beat grid: at each crossing,
            // read the voice's 12-pc WINDOW LENGTH vector, form the length-
            // weighted average color over Jean's PC_COLOR table (NO DFT, no
            // interval math — absolute pitch class → hue). Presence sets the
            // pull; distinct-pc count sets the spread. The ENVELOPE is on the
            // OUTPUT (below): each component glides LINEAR 2-beat attack to the
            // new target, and amount/variance release LINEAR over 8 beats to
            // rest — which the GPU maps to each cell's seed color. [CHECKER]
            // prints one witness line per read.
            if (checker_win_.valid && checker_mean_.valid && checker_var_.valid) {
                // CADENCE: re-anchor on a BACKWARD beat jump (a transport loop
                // back below next_read would otherwise freeze the reader).
                if (beat < checker_next_read_ - canvas::CANVAS_LIVE.checker_read_span)
                    checker_next_read_ = std::floor(beat / canvas::CANVAS_LIVE.checker_read_span) * canvas::CANVAS_LIVE.checker_read_span;
                if (beat >= checker_next_read_) {
                    float acc[3] = { 0.0f, 0.0f, 0.0f };
                    float total = 0.0f;
                    int   n = 0;
                    for (int i = 0; i < 12; ++i) {
                        const float w = signal.stat(checker_win_.channel,
                            checker_win_.base + i);
                        if (w <= 0.0f) continue;
                        acc[0] += w * PC_COLOR[i][0];
                        acc[1] += w * PC_COLOR[i][1];
                        acc[2] += w * PC_COLOR[i][2];
                        total += w;
                        ++n;
                    }
                    const bool present = (total > 1e-6f);
                    if (present) {
                        // The length-weighted average color. On silence we hold
                        // the last resultant (amount fades it to seed anyway).
                        checker_res_goal_[0] = acc[0] / total;
                        checker_res_goal_[1] = acc[1] / total;
                        checker_res_goal_[2] = acc[2] / total;
                    }
                    // Presence drives the pull; distinct-pc count surplus the
                    // spread (raw — the GPU scales + clamps it, hot-reloadable).
                    // 0 or 1 distinct → 0 (a lone note keeps the seed spread).
                    checker_amount_goal_ = present ? 1.0f : 0.0f;
                    checker_var_goal_ = present ? (float)std::max(0, n - 1) : 0.0f;
                    // THE WITNESS: one line per read, upstream of the GPU.
                    // On the instruments dial (core/instruments.hpp): a read
                    // lands every canvas::CANVAS_LIVE.checker_read_span beats — at 120 BPM that
                    // is an UNBUFFERED stderr write every two seconds, on the
                    // beat, which is exactly where a hitch is most visible.
                    if constexpr (INSTRUMENTS.checker_witness) {
                        std::fprintf(stderr,
                            "[CHECKER] n=%d total=%.2f resultant=(%.2f %.2f %.2f) distinct-1=%.0f\n",
                            n, total,
                            checker_res_goal_[0], checker_res_goal_[1], checker_res_goal_[2],
                            checker_var_goal_);
                    }
                    checker_next_read_ =
                        (std::floor(beat / canvas::CANVAS_LIVE.checker_read_span) + 1.0f) * canvas::CANVAS_LIVE.checker_read_span;
                }
                // ENVELOPE. Resultant glides on the 2-beat attack span (its goal
                // holds through silence, so no re-aim there). Amount + variance
                // rise on 2 beats and release to rest on 8 — the return-to-seed.
                for (int c2 = 0; c2 < 3; ++c2)
                    params_.set(checker_mean_.base + c2,
                        trajectory_release(checker_res_seg_[c2],
                            checker_res_goal_[c2], beat, canvas::CANVAS_LIVE.checker_attack));
                params_.set(checker_var_.base,
                    trajectory_release(checker_amount_seg_, checker_amount_goal_, beat,
                        (checker_amount_goal_ > 0.0f ? canvas::CANVAS_LIVE.checker_attack : canvas::CANVAS_LIVE.checker_release)));
                params_.set(checker_var_.base + 1,
                    trajectory_release(checker_var_seg_, checker_var_goal_, beat,
                        (checker_var_goal_ > 0.0f ? canvas::CANVAS_LIVE.checker_attack : canvas::CANVAS_LIVE.checker_release)));
            }

            // ── THE CUBE CHOIR (the light inside the cubes) ─────────────
            // The keyboard: key k is rank k/12 of raw pitch class k%12,
            // and KEY k IS SLOT k by assignment (the cartridge's birth
            // writes it; STAGE_0 R5). ACTIVATION AND
            // DEACTIVATION ONLY — no held-length book; the envelope is
            // the memory.
            //
            //   active(k) ⇔ present_count[dressed(k%12)] > k/12
            //
            // THE DOUBLING LIGHTS THE NEXT RANK: one sounding D lights
            // rank 0's D; a second D in another octave lights rank 1's;
            // a third lights rank 2's. (Octave-true ranking is PARKED —
            // it would need a note-domain reading, and present_count is
            // a pitch-class vector by construction.)
            //
            // ATTACK  — the glide law's own integrator aimed at 1, the
            //   house's standing exponential-approach idiom
            //   (CUBE_GLIDE_TAU is the same k-form; ZOETROPE_LIFT_TAU
            //   was the third and retired at WHEEL_0 U3 with the CPU
            //   walk it timed).
            //   τ = plateau/6 puts I(plateau) = 1 − e⁻⁶ ≈ 0.9975.
            //   CHOIR_1 MADE IT SNAPPIER by moving the DIVISOR and
            //   nothing else — 4 → 6, so the switch-on slope 1/τ goes
            //   0.50 → 0.75 per beat, +50%. Note the direction: a
            //   sharper attack makes the row MORE literally "plateaus
            //   at 8", not less, because the same 8 beats now buys
            //   99.75% instead of 98.2%. The divisor is the only knob
            //   here — light_plateau itself is the dial, and the shape
            //   of the curve is the protected law.
            //   The commission's word for the shape is LOGARITHMIC and it
            //   reads as one — concave, decelerating; the curve is in fact
            //   1 − e^(−t/τ), whose inverse is the log, and unlike a true
            //   logarithm it has an asymptote at 1. Either way the fast
            //   attack IS the curve's own first derivative at t = 0, not
            //   a separate rule bolted on.
            // RELEASE — a FIXED SLOPE, 1/light_release per beat: full
            //   brightness to dark in exactly light_release beats, and a
            //   dimmer key proportionally sooner.
            // RE-ARTICULATION resumes from the present: the ODE reads
            //   its own state, so a key struck mid-fall climbs from
            //   where it is. Goals may leap; values may only walk.
            // THE LOOP SEAM re-anchors Δbeats only — I PERSISTS across a
            //   backward jump, the cells-persist precedent.
            if (choir_ear_.valid && choir_target_.valid) {
                const float dch  = beat - last_beat_;
                const float dbe  = (dch > 0.0f) ? dch : 0.0f;   // a backward jump costs no time
                const float tau  = canvas::CANVAS_LIVE.light_plateau / 6.0f;
                // THE TWO DEGENERATES, taken at their own limits rather
                // than left to the arithmetic: a zero plateau is τ → 0,
                // which the law itself answers with an instant climb to
                // 1; a zero release is an infinite slope, which is an
                // instant fall to dark. A frozen clock (dbe = 0) moves
                // neither — a stopped transport holds the light where it
                // stands, which is the loop seam's rule at zero length.
                const float rise = (dbe <= 0.0f) ? 0.0f
                    : (tau > 0.0f ? 1.0f - std::exp(-dbe / tau) : 1.0f);
                const float fall = (dbe <= 0.0f) ? 0.0f
                    : (canvas::CANVAS_LIVE.light_release > 0.0f
                        ? dbe / canvas::CANVAS_LIVE.light_release : 1.0f);
                const int lanes = (choir_target_.count < CHOIR_LANES)
                    ? choir_target_.count : CHOIR_LANES;
                // ── the strike (STRIKE_0) ── the pipe rests at 0 every
                // tick; an activation edge below raises its pc lane for
                // exactly this frame.
                if (strike_target_.valid)
                    for (int pc = 0; pc < 12; ++pc)
                        params_.set(strike_target_.base + pc, 0.0f);
                for (int k = 0; k < lanes; ++k) {
                    const int   rank  = k / 12;
                    const int   pc    = k % 12;
                    const float count = signal.stat(choir_ear_.channel,
                        choir_ear_.base + dressed_of_pc(pc));
                    const bool  on    = (count > (float)rank);
                    float I = choir_I_[k];
                    if (on) I += (1.0f - I) * rise;             // saturating climb to 1
                    else    I  = (I > fall) ? I - fall : 0.0f;  // fixed slope to dark
                    choir_I_[k] = I;
                    params_.set(choir_target_.base + k, I);
                    // The witness rides the ACTIVATION EDGE — the strike
                    // frame of an instrument that has no strike. On the
                    // zoetrope's dial (rename PARKED; churn minimized).
                    // IT REPORTS A LANE, NOT A KEY: this tier cannot see
                    // CUBE_CHOIR_N, so with the choir narrower than the
                    // pipe (CHOIR_1: 24 of 36) a lane at or above the
                    // choir's width prints here and lights no cube. Read
                    // it against the seam's boot line, which names how
                    // many of these lanes are keys.
                    const unsigned long long bit = 1ull << k;
                    // The edge, hoisted (STRIKE_0): one home for the
                    // strike fact — the witness prints it, the strike
                    // pipe publishes it. Any rank's edge fires its pc
                    // lane; a second rank of one pc re-writes the same 1.0.
                    const bool struck = on && !(choir_sounding_ & bit);
                    if (struck && strike_target_.valid)
                        params_.set(strike_target_.base + pc, 1.0f);
                    if constexpr (INSTRUMENTS.zoetrope_witness) {
                        if (struck)
                            std::fprintf(stderr, "[CHOIR] key=%02d I=%.2f\n", k, I);
                    }
                    if (on) choir_sounding_ |=  bit;
                    else    choir_sounding_ &= ~bit;
                }
            }

            // ── THE GROUND'S VOICE ───────────────────────────────────
            // TWO SOURCES, NO ENVELOPE, NO SEGMENT — as ruled, and the
            // ruling's reason is HALF TRUE, which is worth writing down
            // rather than repeating. The commission says the smoothing
            // question does not arise because `field` is already EMA'd
            // analysis-side. IT IS NOT: canvas_1 publishes it as
            // `field_index(p.field)`, a discrete held ELECTION, so it
            // steps. The fog reads the same source and answers exactly
            // that by carrying its table lookup on a Segment over
            // fog_span — "so density and color drift across a modulation
            // instead of snapping", in its own band's words. Without one
            // here, the WHOLE GROUND changes height in a single frame
            // when the field elects.
            //
            // BUILT AS RULED ANYWAY, and flagged rather than fixed. A
            // ground that jumps on a modulation may be exactly the
            // percept — the harmony moves and the world moves with it —
            // and that is a desk question, not a correctness one. The
            // one-idiom fix is one line per pipe, the fog's own:
            //   trajectory_release(ground_energy_seg_, v, beat, span)
            // with a span on CANVAS_LIVE beside fog_span. Same for the
            // density if the tick reads twitchy, which the commission
            // already flagged.
            //
            // NEITHER PIPE IS DECODED HERE. The law that turns energy
            // into a lift and density into a tick lives at the seam,
            // with the gains — see the PARAM_LAYOUT rows above for why
            // that split falls here and not in the usual place.
            if (fog_field_.valid && ground_energy_.valid) {
                // The fog's own binding, read a second time. A held rank
                // 0..6: 0 is "no field yet", 1 is the anchor, and the
                // ground rises with the rest exactly as the fog thickens.
                params_.set(ground_energy_.base,
                    signal.stat(fog_field_.channel, fog_field_.base));
            }
            if (room_current_pc_.valid && ground_density_.valid) {
                // The room's current notes are a ONE-HOT PER VOICE summed
                // per pitch class, so the sum over the twelve lanes is the
                // count of voices sounding a note right now — 0 in silence,
                // and the room's polyphony otherwise.
                float dens = 0.0f;
                for (int i = 0; i < 12; ++i)
                    dens += signal.stat(room_current_pc_.channel,
                        room_current_pc_.base + i);
                params_.set(ground_density_.base, dens);
            }

            last_beat_ = beat;   // single write, shared by the swell's hold clock
        }

        // Consumers read the bank (and resolve their pipe once through layout()).
        const VisualParams& params() const { return params_; }
        const ParamLayout& layout() const { return param_layout_; }

        // A `choir_light()` accessor stood here, the `zoetrope_rows()`
        // pattern carried forward. It was born an ORPHAN and is not kept:
        // `zoetrope_rows()` existed because the lattice's strike took the
        // run as an ARGUMENT, so the cartridge had to reach the canvas
        // directly. The choir's run leaves through the BANK instead
        // ("cube.light"), because the drivers' room has to compose against
        // it before anything sees it — and a second door onto the same
        // floats, with no caller, is exactly the corpse U5 spent itself
        // removing. The bank IS the accessor.

    private:
        VisualParams params_;
        ParamLayout  param_layout_;
        SignalLayout signal_layout_;

        // ── fog coupling state ───────────────────────────────────────────────────
        SourceBinding fog_field_{};
        TargetBinding fog_density_{};
        TargetBinding fog_color_{};
        Segment       fog_seg_{};
        Segment       fog_color_seg_[3]{};

        // ── ribbon coupling state (sustain swell + room tint) ───────────────────
        SourceBinding voice_playhead_{};   // "<RIBBON_VOICE>.present_count" — the chord's sounding set
        SourceBinding room_wagon_{};       // "all.window_length" — the room's remembered chroma (aims the hue)
        SourceBinding room_playhead_{};    // "all.present_count" — the room sounding (gates the mix)
        uint32_t hold_mask_ = 0u;          // sustain state: the chord's set signature
        float    hold_beats_ = 0.0f;       //   and how long it has held, in beats
        float    last_beat_ = 0.0f;
        TargetBinding amp_lat_{};
        TargetBinding amp_vert_{};
        Segment       amp_lat_seg_{};
        Segment       amp_vert_seg_{};
        TargetBinding tint_stim_{};
        TargetBinding tint_mix_{};
        Segment       tint_stim_seg_[3]{};
        Segment       tint_mix_seg_{};

        // ── checker coupling state (CHECKER-REBUILD: the pc-color field) ─
        SourceBinding checker_win_{};         // "<CHECKER_VOICE>.window_length" — the 12-pc length vector
        float    checker_next_read_ = 0.0f;   // next absolute grid beat (sample-and-hold cursor)
        float    checker_res_goal_[3] = {};   // resultant color goal, held between reads
        float    checker_amount_goal_ = 0.0f; // presence goal [0,1]
        float    checker_var_goal_ = 0.0f;    // distinct-pc spread goal
        TargetBinding checker_mean_{};        // "terrain.checker_mean" (3): resultant rgb
        TargetBinding checker_var_{};         // "terrain.checker_var"  (2): [0]=amount [1]=variance
        Segment       checker_res_seg_[3]{};
        Segment       checker_amount_seg_{};
        Segment       checker_var_seg_{};

        // ── ground coupling state (two sources, no envelope) ─────────────
        SourceBinding room_current_pc_{};   // "all.current_pc" — the room's voices, summed to a density
        TargetBinding ground_energy_{};     // "ground.energy"  — carries all.field, read through fog_field_
        TargetBinding ground_density_{};    // "ground.density" — carries the summed polyphony

        // ── choir coupling state (one ear, one envelope per key) ─────────
        SourceBinding choir_ear_{};              // "<CHOIR_VOICE>.present_count" — the sounding count per pc
        TargetBinding choir_target_{};           // "cube.light" — CHOIR_LANES wide
        TargetBinding strike_target_{};          // "ground.strike" — 12 one-frame impulse lanes (STRIKE_0)
        float         choir_I_[CHOIR_LANES] = {};  // the enveloped light, one per key; state, not an impulse
        unsigned long long choir_sounding_ = 0ull; // the activation edge's memory — one bit per key (36 ≤ 64)
        static_assert(CHOIR_LANES <= 64,
            "choir_sounding_ is one bit per key — widen it past 64 lanes");
    };

} // namespace t7
