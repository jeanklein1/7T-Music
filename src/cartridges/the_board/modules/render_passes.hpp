#pragma once
#include "cartridges/the_board/state.hpp"   // wgpu, GPUSpotLight (the light-VP helper's parameter)

// ─── render_passes.hpp (HEADER: decls) ────────────────────────────
// Converted (LADDER-3 c7): history in audit/LADDER.md.
//
// GPU dispatch and draw calls. The speaker at the end of the
// signal chain: reads final state, issues compute and render passes.
//
// The module owns no state — all verbs. The seven pass/dispatch
// functions take the keyhole (Cartridge* c); the two light-matrix
// helpers are pure math and take none.
//
// ┌─── Public surface (called from outside this module) ────────────┐
// │                                                                  │
// │  Pre-render data preparation:                                    │
// │    upload_ground_entries(c, queue)      — pack entity positions  │
// │    dispatch_placement_correction(c, enc)— Y-correct on heightfld │
// │                                                                  │
// │  GPU compute dispatch:                                           │
// │    dispatch_compute(c, enc)             — ribbon, pawn, agents,  │
// │                                           camera, sphere, cube,  │
// │                                           VP matrix              │
// │    dispatch_frustum_cull(c, enc, queue) — outdoor patch culling  │
// │                                                                  │
// │  Render passes:                                                  │
// │    render_shadow_pass(c, enc)           — depth-only             │
// │    draw_shadow_all(c, pass)             — shared shadow drawlist │
// │    render_main_pass(c, enc, color, depth) — full color pass      │
// │                                                                  │
// │  Light matrix helpers (pure; called from the update path and     │
// │  mood.inl's light-scheme builder):                               │
// │    compute_spot_light_vp(light, vp_out)                          │
// │    compute_sun_matrices(direction, vp_out, center_x, center_z)   │
// │                                                                  │
// └──────────────────────────────────────────────────────────────────┘
//
// Depends on: state.hpp (wgpu, GPU* entry/light types, Dim::*,
// MAX_SPOT_LIGHTS). The impl reaches the keyhole's organs
// (gpuState_ / renderer_ / entities_state_ / cpuPiers_ /
// cpuSpotLights_ / world_state_ / gol_state_ / ribbon_state_ /
// gallery_state_ / orbs_state_ / mood_state_ / clearColor_) through
// the complete type, and the converted modules' surfaces
// (render_orbs from orbs.hpp).
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

class Cartridge;  // fwd — module fns take the keyhole (defined in render_passes.inl, post-class)

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════
//
// DEFINED in render_passes.inl (post-class, self-wrapping).

// Pre-render data preparation
void upload_ground_entries(Cartridge* c, wgpu::Queue& queue);
void dispatch_placement_correction(Cartridge* c, wgpu::CommandEncoder& encoder);
// GPU compute dispatch
void dispatch_compute(Cartridge* c, wgpu::CommandEncoder& encoder);
void dispatch_frustum_cull(Cartridge* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue);
// Render passes
void render_shadow_pass(Cartridge* c, wgpu::CommandEncoder& encoder);
void draw_shadow_all(Cartridge* c, wgpu::RenderPassEncoder& pass);
void render_main_pass(Cartridge* c, wgpu::CommandEncoder& encoder,
    wgpu::TextureView backbuffer, wgpu::TextureView depth);
// Light matrix helpers (pure math — no keyhole)
void compute_spot_light_vp(const GPUSpotLight& light, float* view_proj_out);
void compute_sun_matrices(const float* direction, float* view_proj_out,
    float center_x = 0.0f, float center_z = 0.0f);

} // namespace the_board
} // namespace t7
