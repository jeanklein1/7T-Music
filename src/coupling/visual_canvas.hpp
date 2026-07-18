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
// FIRST COUPLING — fog. The held field (a one-based rank, 0 = none) selects an
// absolute fog density from FOG_BY_FIELD and an atmospheric tint from
// FOG_COLOR_BY_FIELD; Segments carry both, so density and color drift across a
// modulation instead of snapping. The source, "all.field", is already
// published, so the analysis side is untouched.
//
// WIRING (live). The cartridge owns a VisualCanvas, binds it once in
// bind_signal_layout with the analysis layout, ticks it each frame in
// update() after the signal, and flushes fog — density and color —
// from params() to set_fog. Fog has one driver: the field.
//
// CHECKER-1 — the terrain's checker voice. <CHECKER_VOICE>.dft_mag
// (six interval-family magnitudes) → THE SPECTRUM MATRIX → checker
// median offset (rgb) + variance gain, sample-and-held on the
// absolute CHECKER_READ_SPAN grid with a full-span portamento (each
// glide LANDS exactly as the next reading is taken). The cartridge
// flushes terrain.checker_* through set_checker_color_field in U4,
// beside fog; the GPU composes the pair over the seed draws (+ / ×)
// in discrete_cell_color / _at_tier.
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
#include "musical/signal_layout.hpp"
#include "analysis/analysis_signal.hpp"
#include <string>    // casting-sheet name composition ("<voice>.present_count")
#include <cmath>     // std::floor — CHECKER-1's grid cursor
#include <algorithm> // std::min/std::max — CHECKER-1's decode clamps

namespace t7 {

    // ═══ COUPLINGS ═══════════════════════════════════════════════════════════════
    // Each coupling's tuning sits with the coupling; the decode runs inline in
    // tick(). Whether a coupling has an idle depends on its source: a source that
    // can fall quiet (a count, a magnitude) has a rest the value returns to; a
    // held source never quiets, so its coupling goes value-to-value, no idle.

    // Fog — the held field selects an absolute density. The field is a held source:
    // once a scale is established it persists through silence, so fog never returns
    // to a rest — it moves from one field's density to the next. Fields 1/5/6 sit
    // in the dense band, 2/3/4 in the light, with small differences within each.
    // Index 0 is "no field yet" — the value at boot, before any scale is held, not
    // an idle. Tunable; matched to the mood table's outdoor fog scale (0.0003
    // indoor … 0.0050 sunset, ~0.0030 open).
    inline constexpr int   FOG_FIELD_COUNT = 7;          // index 0 = none, 1..6 fields
    inline constexpr float FOG_DENSITY_NONE = 0.0030f;    // index 0 — no field yet (boot)
    inline constexpr float FOG_BY_FIELD[FOG_FIELD_COUNT] = {
        FOG_DENSITY_NONE,   // 0  none — no field yet (boot)
        0.0055f,            // 1  dense
        0.0022f,            // 2  light
        0.0026f,            // 3  light
        0.0020f,            // 4  light
        0.0050f,            // 5  dense
        0.0058f,            // 6  dense
    };

    // Fog color — the same held field selects an atmospheric tint, carried per
    // channel so the hue drifts with the density. Tier 1 is golden hour, kept
    // exactly as the open_sunset look; index 0 is the neutral no-field beige; the
    // rest are gentle shifts away from golden. Same held source, so the same
    // value-to-value behavior — no idle. Tunable.
    //                                                  R       G       B
    inline constexpr float FOG_COLOR_BY_FIELD[FOG_FIELD_COUNT][3] = {
        { 0.85f, 0.78f, 0.72f },   // 0  none        — neutral beige (no field yet)
        { 0.95f, 0.70f, 0.45f },   // 1  golden hour — open_sunset, kept
        { 0.78f, 0.80f, 0.82f },   // 2  cool pale
        { 0.80f, 0.82f, 0.76f },   // 3  faint sage
        { 0.74f, 0.78f, 0.86f },   // 4  soft blue
        { 0.92f, 0.72f, 0.55f },   // 5  warm amber
        { 0.70f, 0.68f, 0.80f },   // 6  muted violet
    };

    // One span carries both fog pipes across a field change; split into a second
    // constant if color should lead or lag density.
    inline constexpr float FOG_SPAN = 2.0f;   // beats — glide into the new field

    // ── Casting (the avatar principle) ── one voice per entity; the set
    // of these is the CASTING SHEET. The ribbon is the chordal piano.
    inline constexpr const char* RIBBON_VOICE = "ch1";   // live prefix verified: chN (canvas_1 NAME_* tables)

    // ── Sustain swell (movement) ── PURE ADDITIVE: the dance is the seed
    // idle PLUS the chord's contribution. goal = 1 + (CEILING−1)·t where
    // t ramps over the hold; silence gives 1 from the formula itself —
    // no branch, identity by construction. Music only ever gives;
    // idleness is inviolate. RULED: ceiling 2× idle at 8 beats.
    inline constexpr float RIBBON_SWELL_CEILING  = 2.00f;  // × idle (ruled)
    inline constexpr float RIBBON_SWELL_RAMP     = 8.0f;   // beats (ruled)
    // Envelope: the swell's goal is continuous during the ramp, so ATTACK
    // engages only at discontinuities (rare); RELEASE governs the breath
    // on re-articulation and the let-go after silence. Fast catch, slow
    // let-go. (A separate BREATH span for re-articulation is one line if
    // the dip wants independence from the final release — say the word.)
    inline constexpr float RIBBON_SWELL_ATTACK   = 0.35f;  // beats
    inline constexpr float RIBBON_SWELL_RELEASE  = 2.0f;   // beats

    // PITCH_VEC_ORIGIN survives the compass redesign: the tint's angle
    // law and the swappable seating live on it.
    inline constexpr float PITCH_VEC_ORIGIN = 0.0f;    // radians — rotates the hue seating

    // ── Line tint (color gen-2) ── the melody paints the ribbon: the
    // line's degree sets a hue by the SAME 30°-per-semitone law as the
    // compass (shared ORIGIN ⇒ cross-channel equivariance). The stimulus
    // is a TINTING VOICE at authored luma/chroma, mixed over the spawn
    // color; mix rises while the line sounds, releases to 0 in silence —
    // rest = the seed-drawn ribbon exactly. Compositional dials.
    inline constexpr float TINT_LUMA    = 0.55f;
    inline constexpr float TINT_CHROMA  = 0.35f;
    inline constexpr float TINT_MIX_MAX = 0.85f;
    // Envelope: the mix catches the room quickly and fades long on its
    // last hue; the hue itself re-aims between actives on one span.
    inline constexpr float TINT_MIX_ATTACK  = 0.5f;   // beats
    inline constexpr float TINT_MIX_RELEASE = 3.0f;   // beats
    inline constexpr float TINT_HUE_SPAN    = 2.0f;   // beats
    // Rodrigues basis about the gray axis (canvas-side twin of the skin's):
    inline constexpr float TINT_D1[3] = { 0.8165f, -0.4082f, -0.4082f };
    inline constexpr float TINT_D2[3] = { 0.0f,     0.7071f, -0.7071f };

    // ── CHECKER-1 (color gen-2, the terrain's checker voice) ─────────
    // THE SPECTRUM MATRIX: <voice>.dft_mag (six interval families,
    // L1-normalized [0,1], origin D) → checker median offset (rgb) +
    // variance gain, composed over the seed draws GPU-side (+ / ×).
    //
    //   offset_c = clamp( Σ_k RGB[c][k]·mag[k],  ±CHECKER_MEAN_MAX )
    //   gain     = clamp( 1 + Σ_k V[k]·mag[k],   GAIN_MIN..GAIN_MAX )
    //
    // SAMPLE-AND-HOLD: goals re-read on the absolute CHECKER_READ_SPAN
    // beat grid; trajectory_release spans the full cycle, so each
    // portamento LANDS exactly as the next reading is taken. Silence
    // publishes zero mags → identity in one cycle, no branch.
    inline constexpr const char* CHECKER_VOICE = "ch2";   // casting sheet; live prefix verified (chN)
    inline constexpr float CHECKER_READ_SPAN = 4.0f;      // beats — read cadence AND glide span (one number)
    //
    // RGB ROWS — the hexagonal seating, stated as a recipe so a
    // reseating regenerates the numbers mechanically:
    //   col_k = CHROMA · ( cos(θ_k)·TINT_D1 + sin(θ_k)·TINT_D2 ),
    //   θ_k = CHECKER_ORIGIN + (k−1)·60°  (shared basis ⇒ cross-voice
    //   hue equivariance with the ribbon's compass).
    // Antipodal columns ⇒ every RGB row sums to ZERO: a lone sustained
    // note (all six bins = 1) displaces NOTHING — only harmonic SHAPE
    // moves the color. The V row is authored free-hand, also zero-sum
    // (same lone-note identity): clusters (f1) scatter the checkers,
    // diatonic clarity (f5) tightens them. Tunable — edit cells
    // directly, or re-run the recipe after moving ORIGIN/CHROMA.
    inline constexpr float CHECKER_ORIGIN = 0.0f;   // radians — rotates the family seating
    inline constexpr float CHECKER_CHROMA = 0.30f;  // hexagon radius (offset strength)
    //                                            f1        f2        f3        f4        f5        f6
    //                                          chroma     dyads    triads     octa    fifths   whole-t
    inline constexpr float CHECKER_MATRIX_RGB[3][6] = {
        /* ΔR */ {  0.2450f,  0.1225f, -0.1225f, -0.2450f, -0.1225f,  0.1225f },
        /* ΔG */ { -0.1225f,  0.1225f,  0.2450f,  0.1225f, -0.1225f, -0.2450f },
        /* ΔB */ { -0.1225f, -0.2450f, -0.1225f,  0.1225f,  0.2450f,  0.1225f },
    };
    inline constexpr float CHECKER_MATRIX_V[6] =
        /* V  */ {  0.60f,    0.10f,   -0.20f,    0.30f,   -0.60f,   -0.20f  };
    // CPU clamps — the pathology fence lives in the decode, never the
    // shader (the shader only adds and multiplies):
    inline constexpr float CHECKER_MEAN_MAX = 0.35f;   // per-channel |offset| ceiling
    inline constexpr float CHECKER_GAIN_MIN = 0.25f;   // variance-gain floor
    inline constexpr float CHECKER_GAIN_MAX = 2.50f;   //   and ceiling (seed var .02–.25 × this)

    // ═══ MASTER CONTROL PANEL ════════════════════════════════════════════════════
    // The one place every exposed pipe is declared — name, slot, width, and the
    // value it rests at. Slots are assigned here, by hand, in this single table, so
    // there are no collisions across entities. Read it as a register map; every
    // coupling and every entity flush resolves against it by name. (A vector's rest
    // is one value across its channels; for fog.color the bind() seed sets the true
    // per-channel start, so the rest is only the pre-first-tick placeholder.)
    //
    //                          name           base count   rest
    inline constexpr ParamSlot PARAM_LAYOUT[] = {
        { "fog.density",          0,    1,    FOG_DENSITY_NONE },
        { "fog.color",            1,    3,    0.80f            },
        // ── ribbon (pitch compass) ── deviations composed over the seed
        // draws at the entity flush; rest = identity (1 = the seed's dance).
        { "ribbon.amp_lateral_mult",  4, 1, 1.0f },
        { "ribbon.amp_vertical_mult", 5, 1, 1.0f },
        { "ribbon.color_stim", 6, 3, 0.0f },
        { "ribbon.color_mix",  9, 1, 0.0f },
        // ── terrain (CHECKER-1) ── the checker vocabulary's deviation,
        // composed GPU-side over the seed draws; rests are the identity
        // elements of + and × (law, not taste — the_board's authored
        // home: terrain_looks ROW 2 REST_CHECKER_*).
        { "terrain.checker_mean", 10, 3, 0.0f },
        { "terrain.checker_var",  13, 1, 1.0f },
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

            // fog: the held field → an absolute density and an atmospheric tint
            fog_field_ = signal_layout_.resolve("all.field");
            fog_density_ = param_layout_.resolve("fog.density");
            fog_color_ = param_layout_.resolve("fog.color");
            fog_seg_ = Segment{ FOG_DENSITY_NONE, FOG_DENSITY_NONE, 0.0f, 0.0f };
            for (int c = 0; c < 3; ++c) {
                fog_color_seg_[c] = Segment{ FOG_COLOR_BY_FIELD[0][c],
                                             FOG_COLOR_BY_FIELD[0][c], 0.0f, 0.0f };
            }

            // ribbon sources (the casting sheet): the voice's Playhead drives
            // the sustain swell; the room's Wagon aims the tint's hue; the
            // room's Playhead gates the tint's mix.
            {
                std::string v(RIBBON_VOICE);
                voice_playhead_ = signal_layout_.resolve((v + ".present_count").c_str());
            }
            room_wagon_    = signal_layout_.resolve("all.window_length");
            room_playhead_ = signal_layout_.resolve("all.present_count");
            amp_lat_  = param_layout_.resolve("ribbon.amp_lateral_mult");
            amp_vert_ = param_layout_.resolve("ribbon.amp_vertical_mult");
            amp_lat_seg_  = Segment{ 1.0f, 1.0f, 0.0f, 0.0f };
            amp_vert_seg_ = Segment{ 1.0f, 1.0f, 0.0f, 0.0f };
            tint_stim_ = param_layout_.resolve("ribbon.color_stim");
            tint_mix_  = param_layout_.resolve("ribbon.color_mix");
            for (int c2 = 0; c2 < 3; ++c2)
                tint_stim_seg_[c2] = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };
            tint_mix_seg_ = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };

            // CHECKER-1 (the terrain's checker voice): the voice's
            // spectrum drives the checker field.
            {
                std::string v(CHECKER_VOICE);
                checker_dft_ = signal_layout_.resolve((v + ".dft_mag").c_str());
            }
            checker_mean_ = param_layout_.resolve("terrain.checker_mean");
            checker_var_  = param_layout_.resolve("terrain.checker_var");
            for (int c2 = 0; c2 < 3; ++c2) {
                checker_mean_goal_[c2] = 0.0f;
                checker_mean_seg_[c2] = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };
            }
            checker_var_goal_ = 1.0f;
            checker_var_seg_  = Segment{ 1.0f, 1.0f, 0.0f, 0.0f };
            checker_next_read_ = 0.0f;   // first frame reads, then grid-locks
        }

        // One frame: run every coupling — read its source, decode inline, carry the
        // value on its Segment, write the bank. No GPU.
        void tick(const AnalysisSignal& signal) {
            const float beat = signal.t_beats;

            // ── fog ──────────────────────────────────────────────────────────────
            // The held field selects an absolute density and an atmospheric tint;
            // Segments carry both so they drift across a modulation rather than
            // snapping. One source, two pipes. Decode is a table index — inline,
            // not a goal object.
            if (fog_field_.valid) {
                const int f = (int)signal.stat(fog_field_.channel, fog_field_.base);
                const int idx = (f >= 0 && f < FOG_FIELD_COUNT) ? f : 0;

                if (fog_density_.valid) {
                    params_.set(fog_density_.base,
                        trajectory_release(fog_seg_, FOG_BY_FIELD[idx], beat, FOG_SPAN));
                }
                if (fog_color_.valid) {
                    for (int c = 0; c < 3; ++c) {
                        params_.set(fog_color_.base + c,
                            trajectory_release(fog_color_seg_[c],
                                FOG_COLOR_BY_FIELD[idx][c], beat, FOG_SPAN));
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
                const float t = (hold_beats_ < RIBBON_SWELL_RAMP)
                    ? hold_beats_ / RIBBON_SWELL_RAMP : 1.0f;
                const float goal = 1.0f + (RIBBON_SWELL_CEILING - 1.0f) * t;
                params_.set(amp_lat_.base,
                    trajectory_release(amp_lat_seg_,  goal, beat,
                        (goal == 1.0f ? RIBBON_SWELL_RELEASE : RIBBON_SWELL_ATTACK)));
                params_.set(amp_vert_.base,
                    trajectory_release(amp_vert_seg_, goal, beat,
                        (goal == 1.0f ? RIBBON_SWELL_RELEASE : RIBBON_SWELL_ATTACK)));
            }

            // ── room tint (color = the room) ────────────────────────────
            // The Wagon AIMS the hue (remembered center of mass — no argmax
            // flicker on chords); the Playhead GATES the mix (sounding ⇒
            // worn; silence ⇒ fades on its last hue).
            if (room_wagon_.valid && tint_stim_.valid && tint_mix_.valid) {
                float vx = 0.0f, vy = 0.0f, energy = 0.0f;
                for (int i = 0; i < 12; ++i) {
                    const float w = signal.stat(room_wagon_.channel, room_wagon_.base + i);
                    if (w <= 0.0f) continue;
                    // Unit-vector seating: the SWAPPABLE TABLE (one line).
                    // Chromatic today (i·30°); circle of fifths ((7i mod 12)
                    // ·30°) or an authored ordering are one-line futures —
                    // the circle rework is PARKED with Jean's name on it.
                    const float th = PITCH_VEC_ORIGIN + (float)i * 0.523598776f;
                    vx += w * std::cos(th); vy += w * std::sin(th);
                    energy += w;
                }
                const float len = std::sqrt(vx*vx + vy*vy);
                if (len > 1e-4f) {
                    const float ca = vx / len, sa = vy / len;   // hue direction, no atan needed
                    for (int c2 = 0; c2 < 3; ++c2) {
                        const float v = TINT_LUMA
                            + (TINT_D1[c2]*ca + TINT_D2[c2]*sa) * TINT_CHROMA;
                        params_.set(tint_stim_.base + c2,
                            trajectory_release(tint_stim_seg_[c2], v, beat, TINT_HUE_SPAN));
                    }
                } else {
                    // window drained: stim segments hold their last hue; the
                    // MIX below is what releases — fade, not gray-out.
                    for (int c2 = 0; c2 < 3; ++c2)
                        params_.set(tint_stim_.base + c2,
                            trajectory_release(tint_stim_seg_[c2],
                                tint_stim_seg_[c2].to, beat, TINT_HUE_SPAN));
                }

                float room_sounding = 0.0f;
                if (room_playhead_.valid)
                    for (int i = 0; i < 12; ++i)
                        room_sounding += signal.stat(room_playhead_.channel,
                                                     room_playhead_.base + i);
                const float mix_goal = (room_sounding > 0.0f) ? TINT_MIX_MAX : 0.0f;
                params_.set(tint_mix_.base,
                    trajectory_release(tint_mix_seg_, mix_goal, beat,
                        (mix_goal == 0.0f ? TINT_MIX_RELEASE : TINT_MIX_ATTACK)));
            }

            // ── CHECKER-1 (spectrum → checker median + spread) ──────
            // SAMPLE-AND-HOLD on the absolute beat grid: at each
            // crossing, read the six family magnitudes, form goals
            // through the matrix (CPU clamps), re-anchor the cursor.
            // Every frame the Segments glide on the full span — MOVE's
            // exact arrival lands each portamento precisely as the
            // next reading is taken. Silence → zero mags → identity in
            // one cycle, no branch.
            if (checker_dft_.valid && checker_mean_.valid && checker_var_.valid) {
                if (beat >= checker_next_read_) {
                    float mag[6];
                    for (int k = 0; k < 6; ++k)
                        mag[k] = signal.stat(checker_dft_.channel, checker_dft_.base + k);
                    for (int c2 = 0; c2 < 3; ++c2) {
                        float o = 0.0f;
                        for (int k = 0; k < 6; ++k) o += CHECKER_MATRIX_RGB[c2][k] * mag[k];
                        checker_mean_goal_[c2] =
                            std::max(-CHECKER_MEAN_MAX, std::min(CHECKER_MEAN_MAX, o));
                    }
                    float g = 1.0f;
                    for (int k = 0; k < 6; ++k) g += CHECKER_MATRIX_V[k] * mag[k];
                    checker_var_goal_ =
                        std::max(CHECKER_GAIN_MIN, std::min(CHECKER_GAIN_MAX, g));
                    checker_next_read_ =
                        (std::floor(beat / CHECKER_READ_SPAN) + 1.0f) * CHECKER_READ_SPAN;
                }
                for (int c2 = 0; c2 < 3; ++c2)
                    params_.set(checker_mean_.base + c2,
                        trajectory_release(checker_mean_seg_[c2],
                            checker_mean_goal_[c2], beat, CHECKER_READ_SPAN));
                params_.set(checker_var_.base,
                    trajectory_release(checker_var_seg_,
                        checker_var_goal_, beat, CHECKER_READ_SPAN));
            }

            last_beat_ = beat;   // single write, shared by the swell's hold clock
        }

        // Consumers read the bank (and resolve their pipe once through layout()).
        const VisualParams& params() const { return params_; }
        const ParamLayout& layout() const { return param_layout_; }

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

        // ── checker coupling state (CHECKER-1: spectrum color field) ─
        SourceBinding checker_dft_{};        // "<CHECKER_VOICE>.dft_mag" — six family magnitudes
        float    checker_next_read_ = 0.0f;  // next absolute grid beat (sample-and-hold cursor)
        float    checker_mean_goal_[3] = {}; // goals held between reads (offset rgb)
        float    checker_var_goal_ = 1.0f;   //   and the variance gain
        TargetBinding checker_mean_{};
        TargetBinding checker_var_{};
        Segment       checker_mean_seg_[3]{};
        Segment       checker_var_seg_{};
    };

} // namespace t7