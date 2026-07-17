#pragma once

// ─── visual_params.hpp ───────────────────────────────────────────
//
// The target surface for couplings — the dual of the analysis contract.
// Where AnalysisSignal + SignalLayout let a coupling read a musical stat
// by name, VisualParams + ParamLayout let a coupling write a visual
// parameter by name. Mirror images:
//
//   analysis (source)               visual (target)
//   ─────────────────               ───────────────
//   AnalysisSignal.stats[]     ↔     VisualParams.v[]
//   StatGroup / StatShape      ↔     ParamSlot (width = count)
//   StatLayoutView             ↔     ParamLayoutView
//   SignalLayout/SourceBinding ↔     ParamLayout/TargetBinding
//
// CATEGORICAL BOUNDARY (mirrored)
//   The musical Canvas writes stats[]; couplings read them, never reaching
//   into analysis internals. Here the visual canvas writes v[]; entities
//   read their pipes from it, never learning what drove them. The bank is
//   the contract — an entity is a pure consumer of its slots, all coupling
//   logic lives on the writing side.
//
// CPU-SIDE (a difference that matters)
//   AnalysisSignal is mirrored into a GPU uniform and uploaded. VisualParams
//   is NOT — it stays CPU-side. Each entity reads its slots and uploads the
//   ones that moved through its own typed GPU config (its flush). The bank
//   never touches the GPU directly, so it carries no alignment contract.
//
// REST (a field the source side has no need of)
//   A musical stat is recomputed every frame; a visual parameter sits at a
//   value. ParamSlot carries that rest — the value a pipe holds at boot, and
//   the value it keeps if no coupling drives it (a control-panel design
//   constant). reset() lays the whole bank to it. For a coupled pipe the rest
//   is an idle — a value it returns to — only when the source can fall quiet;
//   a held source never quiets, so its coupling moves value-to-value and the
//   pipe never returns to rest after boot.
//
// HETEROGENEOUS (the layout is flat, not a grid)
//   The analysis signal is a channel × slot grid because MIDI channels are
//   uniform. Entities are not: each exposes its own pipes. So the bank is
//   one flat space and slots are laid out freely by the layout, by name.
//
// USAGE (mirrors SignalLayout — resolve ONCE, store the binding):
//   TargetBinding t = param_layout_.resolve("orb.speed");
//   if (t.valid) params_.set(t.base, value);   // canvas, per frame
//   ...
//   float v = params_.get(t.base);             // entity flush, per frame
//
// The static PARAM_LAYOUT array (the master control panel) declares every
// pipe's name, slot, width, and rest value; this header is only the
// infrastructure that addresses it.
//
// Depends on: <string_view>, <cstdio>, <cstdint>.

#include <string_view>
#include <cstdio>
#include <cstdint>

namespace t7 {

    // ═══ PARAMETER BANK ══════════════════════════════════════════════
    // Flat CPU-side store of every exposed visual parameter.

    constexpr int VISUAL_PARAM_SLOTS = 256;

    struct VisualParams {
        float v[VISUAL_PARAM_SLOTS] = {};

        // Scalar access.
        float get(int base) const { return v[base]; }
        void  set(int base, float x) { v[base] = x; }

        // Vector access — a contiguous run of `count` floats from `base`.
        const float* run(int base) const { return &v[base]; }
        float* run(int base) { return &v[base]; }
    };

    // ═══ LAYOUT DESCRIPTOR ═══════════════════════════════════════════
    // Dual of StatGroup / StatShape / StatLayoutView. A cartridge (the master
    // control panel) declares a static constexpr PARAM_LAYOUT array of these;
    // the view is the key it publishes so the canvas and entities resolve pipes
    // by name at runtime.


    struct ParamSlot {
        const char* name;    // entity-namespaced, e.g. "orb.speed"
        int         base;    // first slot in the bank
        int         count;   // number of slots (1 for scalar; width IS the shape)
        float       rest;    // value at boot or when uncoupled; reset() lays the bank to it
    };

    struct ParamLayoutView {
        const ParamSlot* slots;
        uint32_t         count;
    };

    // ═══ RESOLVE BY NAME ═════════════════════════════════════════════
    // Dual of SignalLayout / SourceBinding. Resolve once, store the
    // TargetBinding, write/read by base — never resolve per frame.

    struct TargetBinding {
        int  base = 0;
        int  count = 0;
        bool valid = false;
    };

    class ParamLayout {
    public:
        void bind(ParamLayoutView v) { view_ = v; }

        // Lay the bank to each pipe's rest value. Called once at boot, and on any
        // reload of the control panel. Vector pipes fill uniformly.
        void reset(VisualParams& p) const {
            for (uint32_t i = 0; i < view_.count; ++i) {
                const ParamSlot& s = view_.slots[i];
                for (int k = 0; k < s.count; ++k) p.v[s.base + k] = s.rest;
            }
        }

        // Look up a pipe by name. Returns {valid=false} and warns on stderr if
        // the name is absent — callers leave the coupling unbound rather than
        // writing a wrong slot.
        TargetBinding resolve(std::string_view name) const {
            for (uint32_t i = 0; i < view_.count; ++i) {
                const ParamSlot& s = view_.slots[i];
                if (name == s.name) {
                    return TargetBinding{ s.base, s.count, true };
                }
            }
            std::fprintf(stderr,
                "[ParamLayout] pipe '%.*s' not in layout (coupling unbound)\n",
                (int)name.size(), name.data());
            return TargetBinding{};
        }

        uint32_t         count() const { return view_.count; }
        const ParamSlot* slots() const { return view_.slots; }

    private:
        ParamLayoutView view_{ nullptr, 0 };
    };

} // namespace t7