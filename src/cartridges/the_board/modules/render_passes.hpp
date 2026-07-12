#pragma once
#include "cartridges/the_board/state.hpp"   // wgpu, GPUSpotLight (the light-VP helper's parameter)
#include "cartridges/the_board/modules/keyhole.hpp"  // Cartridge + wgpu::Queue fwds (the keyhole)

// ─── render_passes.hpp (HEADER: decls) ────────────────────────────
// Converted (LADDER-3 c7): history in audit/LADDER.md.
//
// GPU dispatch and draw calls.
//
// The impl reaches the keyhole's organs (gpuState_ / renderer_ /
// entities_state_ / patch_system_state_.cpuPiers_ / cpuSpotLights_ / world_state_ /
// gol_state_ / ribbon_state_ / gallery_state_ / orbs_state_ /
// mood_state_ / clearColor_) through the complete type, and the
// converted modules' surfaces (render_orbs from orbs.hpp).
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════

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
