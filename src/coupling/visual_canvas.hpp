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
// USAGE
//   visual_canvas_.bind(analysis_layout);          // startup
//   visual_canvas_.tick(signal);                   // per frame, after analysis
//   ...
//   auto d = visual_canvas_.layout().resolve("fog.density");   // entity, once
//   auto k = visual_canvas_.layout().resolve("fog.color");     // base..base+2
//   float density = visual_canvas_.params().get(d.base);       // entity flush
//
// Depends on: coupling/visual_params.hpp, coupling/trajectory.hpp,
//             musical/signal_layout.hpp, analysis/analysis_signal.hpp.

#include "coupling/visual_params.hpp"
#include "coupling/trajectory.hpp"
#include "musical/signal_layout.hpp"
#include "analysis/analysis_signal.hpp"

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

    // ── Pitch compass ── pc circle → (lateral, vertical) deviation circle.
    // θ = pc·30°; multipliers = 1 + GAIN·(cosθ, sinθ) while stimulated,
    // → 1 on silence. ORIGIN_DEG rotates which degree points pure-lateral
    // (default: degree 0 — the tonic rests in the horizontal). Chords
    // decode as the RESULTANT of their unit vectors: dissonance diffuses
    // the gesture, unison commits it. Compositional dials — tune by ear.
    inline constexpr float PITCH_VEC_GAIN   = 0.35f;   // amp swing ±35%
    inline constexpr float PITCH_VEC_SPAN   = 1.5f;    // beats — re-aim/release glide
    inline constexpr float PITCH_VEC_ORIGIN = 0.0f;    // radians — rotates the compass

    // ── Line tint (color gen-2) ── the melody paints the ribbon: the
    // line's degree sets a hue by the SAME 30°-per-semitone law as the
    // compass (shared ORIGIN ⇒ cross-channel equivariance). The stimulus
    // is a TINTING VOICE at authored luma/chroma, mixed over the spawn
    // color; mix rises while the line sounds, releases to 0 in silence —
    // rest = the seed-drawn ribbon exactly. Compositional dials.
    inline constexpr float TINT_LUMA    = 0.55f;
    inline constexpr float TINT_CHROMA  = 0.35f;
    inline constexpr float TINT_MIX_MAX = 0.85f;
    inline constexpr float TINT_SPAN    = 2.0f;    // beats — attack and release
    // Rodrigues basis about the gray axis (canvas-side twin of the skin's):
    inline constexpr float TINT_D1[3] = { 0.8165f, -0.4082f, -0.4082f };
    inline constexpr float TINT_D2[3] = { 0.0f,     0.7071f, -0.7071f };

    // ═══ MASTER CONTROL PANEL ════════════════════════════════════════════════════
    // The one place every exposed pipe is declared — name, slot, width, and the
    // value it rests at. Slots are assigned here, by hand, in this single table, so
    // there are no collisions across entities. Read it as a register map; every
    // coupling and every entity flush resolves against it by name. (A vector's rest
    // is one value across its channels; for fog.color the bind() seed sets the true
    // per-channel start, so the rest is only the pre-first-tick placeholder.)
    //
    //                          name           base count  shape               rest
    inline constexpr ParamSlot PARAM_LAYOUT[] = {
        { "fog.density",          0,    1,    ParamShape::Scalar, FOG_DENSITY_NONE },
        { "fog.color",            1,    3,    ParamShape::Vector, 0.80f            },
        // ── ribbon (pitch compass) ── deviations composed over the seed
        // draws at the entity flush; rest = identity (1 = the seed's dance).
        { "ribbon.amp_lateral_mult",  4, 1, ParamShape::Scalar, 1.0f },
        { "ribbon.amp_vertical_mult", 5, 1, ParamShape::Scalar, 1.0f },
        { "ribbon.color_stim", 6, 3, ParamShape::Vector, 0.0f },
        { "ribbon.color_mix",  9, 1, ParamShape::Scalar, 0.0f },
    };
    inline constexpr uint32_t PARAM_LAYOUT_COUNT =
        sizeof(PARAM_LAYOUT) / sizeof(PARAM_LAYOUT[0]);

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

            // pitch compass: the Wagon's duration-weighted chroma → amp pipes
            wagon_chroma_ = signal_layout_.resolve("all.window_length");
            amp_lat_  = param_layout_.resolve("ribbon.amp_lateral_mult");
            amp_vert_ = param_layout_.resolve("ribbon.amp_vertical_mult");
            amp_lat_seg_  = Segment{ 1.0f, 1.0f, 0.0f, 0.0f };
            amp_vert_seg_ = Segment{ 1.0f, 1.0f, 0.0f, 0.0f };

            // line tint: the sung line paints the ribbon over its spawn color
            line_pc_   = signal_layout_.resolve("all.current_pc");
            tint_stim_ = param_layout_.resolve("ribbon.color_stim");
            tint_mix_  = param_layout_.resolve("ribbon.color_mix");
            for (int c2 = 0; c2 < 3; ++c2)
                tint_stim_seg_[c2] = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };
            tint_mix_seg_ = Segment{ 0.0f, 0.0f, 0.0f, 0.0f };
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

            // ── pitch compass (X₁ of the Wagon chroma) ──────────────────
            // Duration-weighted resultant: direction = where the remembered
            // harmony's center of mass points; magnitude = its concentration
            // (unison commits, clusters diffuse). Double-smoothed by
            // construction: the window drains in per-beat stairs, the
            // Segments glide between them. Silence = a two-stage release —
            // the window empties, then the dance glides home.
            if (wagon_chroma_.valid && amp_lat_.valid && amp_vert_.valid) {
                float vx = 0.0f, vy = 0.0f, energy = 0.0f;
                for (int i = 0; i < 12; ++i) {
                    const float w = signal.stat(wagon_chroma_.channel, wagon_chroma_.base + i);
                    if (w <= 0.0f) continue;
                    const float th = PITCH_VEC_ORIGIN + (float)i * 0.523598776f; // 30°
                    vx += w * std::cos(th);
                    vy += w * std::sin(th);
                    energy += w;
                }
                float gl = 1.0f, gv = 1.0f;
                if (energy > 0.0f) {
                    const float inv = 1.0f / energy;      // resultant, unit-ish
                    gl = 1.0f + PITCH_VEC_GAIN * (vx * inv);
                    gv = 1.0f + PITCH_VEC_GAIN * (vy * inv);
                }
                params_.set(amp_lat_.base,
                    trajectory_release(amp_lat_seg_,  gl, beat, PITCH_VEC_SPAN));
                params_.set(amp_vert_.base,
                    trajectory_release(amp_vert_seg_, gv, beat, PITCH_VEC_SPAN));
            }

            // ── line tint (hue by the 30° law; mix is the envelope) ─────
            if (line_pc_.valid && tint_stim_.valid && tint_mix_.valid) {
                float best = 0.0f; int deg = -1;
                for (int i = 0; i < 12; ++i) {
                    const float w = signal.stat(line_pc_.channel, line_pc_.base + i);
                    if (w > best) { best = w; deg = i; }
                }
                float mix_goal = 0.0f;
                if (deg >= 0 && best > 0.0f) {
                    const float th = PITCH_VEC_ORIGIN + (float)deg * 0.523598776f;
                    const float ca = std::cos(th), sa = std::sin(th);
                    for (int c2 = 0; c2 < 3; ++c2) {
                        const float v = TINT_LUMA
                            + (TINT_D1[c2]*ca + TINT_D2[c2]*sa) * TINT_CHROMA;
                        params_.set(tint_stim_.base + c2,
                            trajectory_release(tint_stim_seg_[c2], v, beat, TINT_SPAN));
                    }
                    mix_goal = TINT_MIX_MAX;
                } else {
                    // silence: stim segments hold their last hue; only the
                    // MIX releases — the tint fades, it does not gray out.
                    for (int c2 = 0; c2 < 3; ++c2)
                        params_.set(tint_stim_.base + c2,
                            trajectory_release(tint_stim_seg_[c2],
                                tint_stim_seg_[c2].to, beat, TINT_SPAN));
                }
                params_.set(tint_mix_.base,
                    trajectory_release(tint_mix_seg_, mix_goal, beat, TINT_SPAN));
            }
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

        // ── pitch compass coupling state ─────────────────────────────────────────
        SourceBinding wagon_chroma_{};      // "all.window_length" — the Wagon's
                                            // duration-weighted chroma (12-wide);
                                            // X₁ of THIS is the compass: the
                                            // remembered music's center of mass
        TargetBinding amp_lat_{};
        TargetBinding amp_vert_{};
        Segment       amp_lat_seg_{};
        Segment       amp_vert_seg_{};

        // ── line tint coupling state ─────────────────────────────────────────────
        SourceBinding line_pc_{};           // "all.current_pc" — the sung line
        TargetBinding tint_stim_{};
        TargetBinding tint_mix_{};
        Segment       tint_stim_seg_[3]{};
        Segment       tint_mix_seg_{};
    };

} // namespace t7