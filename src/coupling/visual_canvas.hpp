#pragma once

// ─── coupling/visual_canvas.hpp ──────────────────────────────────────
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
// absolute fog density from FOG_BY_FIELD; a Segment carries it, so the density
// drifts across a modulation instead of snapping. The source, "all.field", is
// already published, so the analysis side is untouched.
//
// WIRING (left to the integration)
//   The cartridge owns a VisualCanvas, calls bind() once with the analysis
//   layout it was handed at startup, and tick(signal) each frame. mood.inl
//   reads fog.density from params() and hands it to set_fog with the mood's
//   color — its only change.
//
// USAGE
//   visual_canvas_.bind(analysis_layout);          // startup
//   visual_canvas_.tick(signal);                   // per frame, after analysis
//   ...
//   auto b = visual_canvas_.layout().resolve("fog.density");   // entity, once
//   float density = visual_canvas_.params().get(b.base);       // entity flush
//
// Depends on: coupling/visual_params.hpp, coupling/trajectory.hpp,
//             musical/signal_layout.hpp, analysis/analysis_signal.hpp.

#include "coupling/visual_params.hpp"
#include "coupling/trajectory.hpp"
#include "musical/signal_layout.hpp"
#include "analysis/analysis_signal.hpp"

namespace t7 {

// ═══ COUPLINGS ═══════════════════════════════════════════════════
// Each coupling's tuning sits with the coupling; the decode runs inline in
// tick(). A coupled pipe's rest is its idle — the value it returns to when its
// source is quiet — so the rest the control panel publishes is authored here.

// Fog — the held field selects an absolute density. Fields 1/5/6 sit in the
// dense band, 2/3/4 in the light, with small differences within each. Index 0
// is "no field yet": the idle, and the bank's rest for fog.density. Tunable;
// matched to the mood table's outdoor fog scale (0.0003 indoor … 0.0050
// sunset, ~0.0030 open).
inline constexpr int   FOG_FIELD_COUNT  = 7;          // index 0 = none, 1..6 fields
inline constexpr float FOG_DENSITY_REST = 0.0030f;    // index 0 — idle / no field
inline constexpr float FOG_BY_FIELD[FOG_FIELD_COUNT] = {
    FOG_DENSITY_REST,   // 0  none — idle
    0.0055f,            // 1  dense
    0.0022f,            // 2  light
    0.0026f,            // 3  light
    0.0020f,            // 4  light
    0.0050f,            // 5  dense
    0.0058f,            // 6  dense
};
inline constexpr float FOG_SPAN = 12.0f;   // beats — a slow, atmospheric crossing

// ═══ MASTER CONTROL PANEL ════════════════════════════════════════
// The one place every exposed pipe is declared — name, slot, width, and the
// value it rests at. Slots are assigned here, by hand, in this single table, so
// there are no collisions across entities. Read it as a register map; every
// coupling and every entity flush resolves against it by name.
//
//                          name           base count  shape               rest
inline constexpr ParamSlot PARAM_LAYOUT[] = {
    { "fog.density",          0,    1,    ParamShape::Scalar, FOG_DENSITY_REST },
};
inline constexpr uint32_t PARAM_LAYOUT_COUNT =
    sizeof(PARAM_LAYOUT) / sizeof(PARAM_LAYOUT[0]);

// ═══ VISUAL CANVAS ═══════════════════════════════════════════════

class VisualCanvas {
public:
    // Startup wiring: publish the control panel, lay the bank to its rests,
    // adopt the analysis layout, and resolve every coupling's source and target
    // once. tick() then never resolves.
    void bind(StatLayoutView analysis_layout) {
        param_layout_.bind(ParamLayoutView{ PARAM_LAYOUT, PARAM_LAYOUT_COUNT });
        param_layout_.reset(params_);

        signal_layout_.bind(analysis_layout);

        // fog: held field → absolute density
        fog_field_   = signal_layout_.resolve("all.field");
        fog_density_ = param_layout_.resolve("fog.density");
        fog_seg_     = Segment{ FOG_DENSITY_REST, FOG_DENSITY_REST, 0.0f, 0.0f };
    }

    // One frame: run every coupling — read its source, decode inline, carry the
    // value on its Segment, write the bank. No GPU.
    void tick(const AnalysisSignal& signal) {
        const float beat = signal.t_beats;

        // ── fog ────────────────────────────────────────────────────────────
        // Field selects the target density; the Segment carries it so the
        // density drifts across a modulation rather than snapping. Decode is a
        // table index — inline, not a goal object.
        if (fog_field_.valid && fog_density_.valid) {
            const int   f      = (int)signal.stat(fog_field_.channel, fog_field_.base);
            const float target = FOG_BY_FIELD[(f >= 0 && f < FOG_FIELD_COUNT) ? f : 0];
            params_.set(fog_density_.base,
                        trajectory_release(fog_seg_, target, beat, FOG_SPAN));
        }
    }

    // Consumers read the bank (and resolve their pipe once through layout()).
    const VisualParams& params() const { return params_; }
    const ParamLayout&  layout() const { return param_layout_; }

private:
    VisualParams params_;
    ParamLayout  param_layout_;
    SignalLayout signal_layout_;

    // ── fog coupling state ───────────────────────────────────────────────
    SourceBinding fog_field_{};
    TargetBinding fog_density_{};
    Segment       fog_seg_{};
};

} // namespace t7
