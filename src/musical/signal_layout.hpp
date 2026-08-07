#pragma once

// ─── signal_layout.hpp ───────────────────────────────────────────
//
// Render-side resolve-by-name for analysis stat groups. The render side
// receives a StatLayoutView (the analysis cartridge's published slot
// map) once at startup and resolves coupling sources by name through it.
// This keeps the categorical boundary intact: the render side imports
// only the generic StatGroup contract (analysis_signal.hpp), never the
// analysis cartridge's own headers.
//
// Generic over any layout — both render cartridges and the_lab can use
// it (the_board adopts when it migrates).
//
// USAGE (B2): resolve ONCE, store the SourceBinding, never per-frame:
//   SourceBinding src = signal_layout_.resolve("abbott.lowest_pc");
//   if (src.valid) { ... read stats[src.channel][src.base + i], i < count ... }
//
// resolve() returns whole-StatGroup addressing — {channel, base, count};
// the coupling indexes within the group.
//
// Depends on: analysis/analysis_signal.hpp (StatGroup, StatLayoutView),
//             <string_view>, <cstdio>.

#include "analysis/analysis_signal.hpp"   // StatGroup, StatLayoutView
#include <string_view>
#include <cstdio>

namespace t7 {

// One resolved stat group: where a named source lives in the signal.
// Held once after resolve(); width is irrelevant (never per-frame), so
// the fields mirror StatGroup's own int types — no narrowing on build.
struct SourceBinding {
    int  channel = 0;
    int  base    = 0;   // = StatGroup slot_base
    int  count   = 0;
    bool valid   = false;
};

class SignalLayout {
public:
    void bind(StatLayoutView v) { view_ = v; misses_ = 0; }

    // PORT_4c — how many resolves missed since the last bind(). The
    // release twin prints ONE summary line from this instead of one
    // line per source: with no audio source every resolve misses, and
    // twenty stderr lines read like twenty faults to a visitor who
    // opened DevTools out of curiosity. It is one fact — the socket is
    // empty — so it gets one line. The debug twin still names each.
    uint32_t misses() const { return misses_; }

    // Look up a source by name. Returns {valid=false} and warns on stderr
    // if the name is absent — callers leave the coupling disabled rather
    // than reading a wrong slot.
    SourceBinding resolve(std::string_view name) const {
        for (uint32_t i = 0; i < view_.count; ++i) {
            const StatGroup& g = view_.groups[i];
            if (name == g.name) {            // g.name is const char*
                return SourceBinding{ g.channel, g.slot_base, g.count, true };
            }
        }
        ++misses_;
#ifndef NDEBUG
        // Debug twin only (the-board-web-debug / native Debug): the full
        // list, one line per source, unchanged. NDEBUG is the gate
        // because CMake defines it for Release and not for Debug, which
        // is exactly the two-preset split PORT_2c installed.
        std::fprintf(stderr,
            "[SignalLayout] source '%.*s' not in layout (coupling disabled)\n",
            (int)name.size(), name.data());
#endif
        return SourceBinding{};              // valid = false
    }

    uint32_t         count()  const { return view_.count; }
    const StatGroup* groups() const { return view_.groups; }

private:
    StatLayoutView view_{ nullptr, 0 };
    mutable uint32_t misses_ = 0;   // resolve() is const; the tally is not state the caller owns
};

} // namespace t7
