#pragma once
// ═══════════════════════════════════════════════════════════════════════
// THE DRAWABLE TABLE — one row per drawable; the
// bundle and the direct passes iterate it through ONE template
// (BUNDLE_1), FILTERED by membership (shadow / main).
// Kills the duplication (a drawable was enumerated once per
// list, hand-synced) and the ribbon's ordinal DRIFT (the lists had
// ribbon at different positions) — there is now ONE canonical order.
// A new drawable is ONE row instead of a multi-site edit.
//
// PIXEL-SAFETY. Every drawable IN THIS TABLE is OPAQUE (depth-tested,
// depth-write, no blend — or an alpha=1.0 output that makes SrcAlpha a
// no-op): terrain(fork), pawn, sphere, monolith, ribbon,
// shell. Draw
// order among OPAQUE geometry is immaterial — the depth test resolves
// visibility identically regardless of order — so the ONE canonical order
// (the shadow order) reproduces every pass pixel-for-pixel, and the ribbon
// drift dies for free. Verified opaque: ribbon uses ENTITY_FS + the shared
// depthStencil/colorTarget (no blend). The ORDER-SENSITIVE draws are FORKS,
// kept last and in order (orbs additive, fade alpha/no-depth).
//
// FORKS (Discipline 2 — flagged, NOT forced into a uniform row):
//   - terrain: two different per-pass codes (shadow: LOD0 + manual LOD1;
//     main: LOD0 indirect/direct + LOD1 direct).
//   - orbs / fade: blended, ORDER-SENSITIVE, kept LAST in the main pass.
// These stay as explicit code in their pass function; only the uniform
// opaque entity draws live in the table.
//
// PRUNE_1 U2: the snapshot pass and its wall_paintings / gallery_frames
// fork left with the gallery organ. The membership axis is two passes now,
// and the fork the table could not absorb is no longer anyone's debt.
// ═══════════════════════════════════════════════════════════════════════

#include "cartridges/the_board/realization/state.hpp"      // GPUState + wgpu
#include "cartridges/the_board/realization/renderer.hpp"   // Renderer (the draw_* verbs)

namespace t7 {
namespace the_board {

enum DrawPass : uint32_t { DRAW_SHADOW = 1u, DRAW_MAIN = 2u };

// Per-pass context for the thunks: which draw family to call, plus the
// runtime data-guards (precomputed by the pass) so the thunks stay
// ctx-agnostic — MachineCtx exposes Renderer&/GPUState&, nothing more is
// needed.
//
// OIL_1 U11-U13: the entity/texture group pair LEFT this struct. Each
// pass now binds its own groups ONCE at its head (group0 = that pass's
// entity window, group1 = its texture group) and the thunks set only
// pipeline + vertex/index buffers — so the pair is pass state, named
// where the pass begins, not a parameter re-set at every draw.
// BUNDLE_1: the ribbon's LIVENESS and its VERTEX COUNT left this struct —
// both are one record in the draw ledger now, staged at the frame boundary,
// because an encoder-time skip cannot survive being recorded into a bundle.
// Its MASK BIT stays, because a mask bit is a different kind of fact: it is
// a measurement dial, and PANORAMA_1's rule is that a cleared bit is skipped
// AT THE ENCODER so the pass row reads the absence. Under bundles "at the
// encoder" becomes "at the recording", which is still a real skip — the
// mask setters raise bundlesDirty_ so the dial re-records when it turns.
struct DrawBind {
    bool shadow;                // shadow pass -> draw_shadow_X ; else draw_X
    bool ribbon_bit;            // the pass's own say: DrawBit::RIBBON in main, true elsewhere
};

// BUNDLE_1: the row is a TEMPLATE over the encoder type. `Enc` is
// wgpu::RenderPassEncoder for a direct pass and wgpu::RenderBundleEncoder
// for a recorded bundle; both carry SetPipeline / SetBindGroup /
// SetVertexBuffer / SetIndexBuffer / Draw / DrawIndexed / DrawIndirect /
// DrawIndexedIndirect, which is the whole of what a thunk needs.
//
// THE TABLE ITSELF IS WRITTEN ONCE (R-E: the table-template spelling, not
// the pointer-pair one). A pair of function pointers per row would have
// duplicated every row's initialiser to say the same thing twice;
// a variable template instantiates the one list per encoder type. One draw
// list, two encoders, no second list — which is R3's requirement stated as
// code rather than as discipline.
template <class Enc>
struct Drawable {
    const char* name;
    uint32_t    passes;         // bit-OR of DrawPass
    void (*draw)(Renderer&, GPUState&, Enc&, const DrawBind&);
};

// ── The thunks: each knows its buffers and picks draw_X vs draw_shadow_X ──
template <class Enc>
inline void dt_pawn(Renderer& r, GPUState& g, Enc& p, const DrawBind& b) {
    (void)g;
    if (b.shadow) r.draw_shadow_pawn(p, GPUState::pawn_vertex_count());   // OIL_1 U12: pass-head binds
    else          r.draw_pawn       (p, GPUState::pawn_vertex_count());   // OIL_1 U13: pass-head binds
}
template <class Enc>
inline void dt_sphere(Renderer& r, GPUState& g, Enc& p, const DrawBind& b) {
    if (b.shadow) r.draw_shadow_sphere(p, g.sphere_vertex_buffer(), g.sphere_index_buffer(), g.sphere_index_count());
    else          r.draw_sphere       (p, g.sphere_vertex_buffer(), g.sphere_index_buffer(), g.sphere_index_count());
}
template <class Enc>
inline void dt_monolith(Renderer& r, GPUState& g, Enc& p, const DrawBind& b) {
    if (b.shadow) r.draw_shadow_monolith(p, g.monolith_vertex_buffer(), g.monolith_index_buffer(), g.monolith_index_count());
    else          r.draw_monolith       (p, g.monolith_vertex_buffer(), g.monolith_index_buffer(), g.monolith_index_count());
}
template <class Enc>
inline void dt_ribbon(Renderer& r, GPUState& g, Enc& p, const DrawBind& b) {
    // RIBBON_1: the draw is the LIVE count. It used to be the 400-ring
    // ceiling with the VS early-out retiring four fifths of it twice a pass.
    // BUNDLE_1: the count AND the `rendered_slot != UINT32_MAX` guard are
    // the record's now — an inactive ribbon stages zero vertices. The guard
    // could not stay here: an encoder-time skip recorded into a bundle
    // during a rideless frame would omit the ribbon forever.
    if (!b.ribbon_bit) return;
    if (b.shadow) r.draw_shadow_ribbon(p, g.draw_ledger_buffer(),
                                       GPUState::draw_record_offset(GPUState::DR_RIBBON));
    else          r.draw_ribbon       (p, g.draw_ledger_buffer(),
                                       GPUState::draw_record_offset(GPUState::DR_RIBBON));
}

// THE CANONICAL ORDER (== the shadow order). Membership is which passes a
// drawable belongs to.
template <class Enc>
inline const Drawable<Enc> DRAWABLES[] = {
    { "pawn",     DRAW_SHADOW | DRAW_MAIN, dt_pawn<Enc>     },
    { "sphere",   DRAW_SHADOW | DRAW_MAIN, dt_sphere<Enc>   },
    { "monolith", DRAW_SHADOW | DRAW_MAIN, dt_monolith      },
    { "ribbon",   DRAW_SHADOW | DRAW_MAIN, dt_ribbon<Enc>   },
};

// Iterate the table for one pass, in canonical order, filtered by membership.
template <class Enc>
inline void draw_table(Renderer& r, GPUState& g, Enc& p,
                       const DrawBind& b, uint32_t passbit) {
    for (const Drawable<Enc>& d : DRAWABLES<Enc>)
        if (d.passes & passbit) d.draw(r, g, p, b);
}

// THE WITNESS THAT BOTH ENCODERS INSTANTIATE (BUNDLE_1). A function
// template is only type-checked when something asks for it, so a draw list
// that compiles for the pass encoder proves NOTHING about the bundle
// encoder until a bundle is recorded — and the first bundle lands a commit
// later. This asks now: taking the address forces both instantiations, so
// the TU gate reads the whole table twice and Commit B stands on its own.
// Never called; emits no code.
inline void drawable_table_encoder_witness() {
    (void)&draw_table<wgpu::RenderPassEncoder>;
    (void)&draw_table<wgpu::RenderBundleEncoder>;
}

} // namespace the_board
} // namespace t7
