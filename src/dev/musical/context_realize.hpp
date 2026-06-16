#pragma once

// ─── context_realize.hpp ─────────────────────────────────────────
//
// The realizer: it reads a ContextSpec and configures a Context to match.
// The spec is data and the Context is mechanism; this is the one place
// that knows both, turning a description into the live windows and
// memories it names. Keeping it apart is what lets the spec be included
// anywhere — it pulls in nothing — and the Context be used without a spec.
//
// It configures, it does not attach. Channel, retention, the windows, and
// the two memories are set here; what reads a configured context — the
// operations — is a later layer. Realize once, into a fresh context: the
// windows accumulate, so a second pass over a used context would double
// them.
//
// The present is not toggled here. The Playhead is the view layer's floor
// and the cheapest of the views — a stateless snapshot, costing nothing
// to keep fresh for a channel that does not read it — so the Context
// maintains it always. A spec's `present` is a read-side declaration: the
// canvas consults has_present() when it decides which operations a channel
// may run, and that is where a dropped present takes effect. The memories
// are gated instead, both because their feeding has cost and because an
// un-fed memory is correctly empty, which is what "off" must mean.
//
// Retention is set to cover the windows whatever the spec names, so a
// window can never out-reach the history behind it.
//
// Depends on: musical/context.hpp, musical/context_spec.hpp,
//             musical/spine.hpp, <algorithm>.

#include "musical/context.hpp"
#include "musical/context_spec.hpp"
#include "musical/spine.hpp"

#include <algorithm>

namespace t7 {

// Map a spec's oracle binding to one of the spine's injectable oracles.
// Lowest is the only binding bound so far; an unbound selector falls back
// to lowest rather than leaving the oracle null.
inline Spine::Oracle oracle_for(OracleBinding binding) {
    switch (binding) {
        case OracleBinding::Lowest: return &Spine::oracle_lowest;
    }
    return &Spine::oracle_lowest;
}

// Configure `ctx` to match `spec`. Realize into a fresh context.
inline void realize(const ContextSpec& spec, Context& ctx) {
    ctx.set_channel(spec.channel);

    // Retention covers the deepest window, never less.
    const float retention =
        std::max(spec.stream_retention_beats, spec.required_retention_beats());
    ctx.set_retention_beats(retention);

    // View layer: the windows. The present is the always-maintained floor
    // (see the file header for why it is not toggled here).
    for (const auto& w : spec.windows) {
        if (w.active) ctx.add_wagon(w.span_beats, w.offset_beats);
    }

    // Memory layer: each enabled only if the spec asks for it.
    if (spec.event.active) {
        ctx.enable_previous(spec.event.tolerance_beats);
    }
    if (spec.crossings.active) {
        ctx.enable_spine(spec.crossings.tolerance_beats,
                         oracle_for(spec.crossings.oracle));
    }
}

} // namespace t7
