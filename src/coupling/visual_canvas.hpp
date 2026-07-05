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
    };

} // namespace t7