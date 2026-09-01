#pragma once
#include "cartridges/the_board/realization/state.hpp"   // wgpu, GPUState
#include "cartridges/the_board/contracts/wgpu_fwd.hpp"   // wgpu handle fwds (lockstep insurance)
#include <algorithm>   // std::max, std::min   // (impl, merged)
#include <cmath>       // std::sqrt, std::abs, std::acos, std::tan   // (impl, merged)
#include <cstdint>   // (impl, merged)

// ─── render_passes.hpp (MERGED: decls + impl) ──────────────────────
//
// GPU dispatch and draw calls.
//
// THE MACHINE FACE (the B ruling): the realization conductor stands on MachineCtx — its nine
// organ reaches are all machine members, byte-identical through the
// face. The three reaches OUTSIDE the face ride the call site (the B
// read), render_main_pass takes the clear color (const read) + the
// orbs pair (render_orbs — the one sibling door). The module owns no
// state; the two light-matrix helpers are pure math. COHORT: merged
// at the tail after orbs/ribbon/input (render_orbs def +
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// fwd — the machine face + the main pass's orbs pair (reference
// params; complete types precede this file in the cohort).
struct MachineCtx;
struct OrbsState; struct OrbsDeps;

// ═══ MODULE FUNCTIONS — DECLARATIONS ═════════════════════════════

// Pre-render data preparation
void stage_draw_ledger(MachineCtx* c, OrbsState& orbs_state_);
void record_bundles(MachineCtx* c, OrbsState& orbs_state_, OrbsDeps& orbs_deps_);
// GPU compute dispatch
void dispatch_compute(MachineCtx* c, wgpu::CommandEncoder& encoder);
void dispatch_frustum_cull(MachineCtx* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue);
// Render passes (the extras outside the machine face ride the call site)
void render_shadow_pass(MachineCtx* c, wgpu::CommandEncoder& encoder);
template <class Enc>
void draw_shadow_all(MachineCtx* c, Enc& pass, bool cast_terrain);
void render_main_pass(MachineCtx* c, wgpu::CommandEncoder& encoder,
    wgpu::TextureView backbuffer, wgpu::TextureView msaaColor,
    wgpu::TextureView depth,
    const float (&clearColor_)[3], OrbsState& orbs_state_, OrbsDeps& orbs_deps_);
// Light matrix helpers (pure math — no MachineCtx)


// ═══ MODULE IMPLEMENTATION ════════════════════════════════════════
//
// The dispatch/pass bodies + the two pure light-matrix helpers. The
// bodies reach the machine face (c->gpuState_ / c->renderer_ /
// c->entities_state_ /
// c->world_state_ / c->gol_state_ / c->ribbon_state_ /
// c->sky_state_) and the call-site extras
// (clearColor_ / the orbs pair).


// ═══ PRE-RENDER DATA PREP ════════════════════════════════════════

// ═══ THE DRAW LEDGER'S STAGE (BUNDLE_1) ══════════════════════════
//
// ONE SITE that reads every count the CPU authors and stages it. It runs at
// the frame boundary — after the update spine, so every count is this
// frame's, and before the encoder, so the WriteBuffer lands ahead of the
// command buffer on the queue timeline (the ordering upload_patch_params
// relies on).
//
// WHY ONE SITE AND NOT TWELVE. Each of these numbers already has a home —
// the family setters, os.count. What did NOT have a home is
// the set of GUARDS: `if (indexCount == 0) return;`, `if (!os.active)
// return;`, `rendered_slot != UINT32_MAX`. Those lived in the draw verbs as
// encoder-time skips, and an encoder-time skip cannot be recorded into a
// bundle — a bundle taken in a frame where a family was empty would omit it
// forever, and every family is empty at boot. A record of zeros draws
// nothing, so each guard becomes its record's number, and this is where
// "what will be drawn this frame" is said once and read once.
//
// flush_draw_ledger writes only what moved, so a steady frame writes zero
// bytes however many times this stages the same numbers.
inline void stage_draw_ledger(MachineCtx* c, OrbsState& orbs_state_) {
    GPUState& g = c->gpuState_;

    // (The shell's row stood here until ONE_WORLD-II U4.)
    // (maxSlot + 1) * MAX_INDICES_PER_SLOT, zero when nothing is active.

    // The ribbon: RIBBON_1's live vertex count, and its liveness. A ribbon
    // with no rendered slot stages zero — that IS the old guard.
    const bool ribbon_live = c->ribbon_state_.rendered_slot != UINT32_MAX;
    g.stage_draw_verts(GPUState::DR_RIBBON,
        ribbon_live ? ribbon_draw_verts(c->ribbon_state_) : 0u, 1u);

    // The orbs: six indices of a quad, one instance per orb. `os.active`
    // and a zero count are the same fact to the ledger.
    g.stage_draw_indexed(GPUState::DR_ORBS, 6u,
        orbs_state_.active ? orbs_state_.count : 0u);

    // The sun's terrain: ONE draw over both bands (R-G). The instance range
    // is [0, render_patch_count) — the union of the two the fork used to
    // issue — at the LOD1 ring's live index count.
    g.stage_draw_indexed(GPUState::DR_SHADOW_TERRAIN,
        g.patch_index_count_lod1_live(),
        c->world_state_.render_patch_count);
}

// ═══ GPU COMPUTE DISPATCH ════════════════════════════════════════

// Per-frame compute: the live card, ribbon transforms, agents, camera, VP.
//
// ONE PASS WHERE TWO STOOD (SPINE_2 B). The live card write (GROUND_CARD_1)
// held its own pass for one reason: its writes must be visible to the
// consumers below. A pass boundary is one way to get that; DISPATCH ORDER
// INSIDE A PASS is the other, and it is the cheap one — a compute pass
// orders its dispatches and makes an earlier dispatch's writes visible to
// a later one. So the card is simply the FIRST dispatch here, ahead of the
// ribbon (whose fallback ground reads the card through the GoL suppression
// contributor) and ahead of everything else that reads it.
//
// THE REST-LAW SKIP BECAME A DISPATCH SKIP. `write_live_card` carries R8's
// three-conjunct decision (cartridge.hpp, phase_live_card_write): the pass
// opens either way — an empty dispatch slot is cheaper than a boundary —
// and the card's 819,200 invocations are skipped exactly when they were.
//
// PLACEMENT AND THE CULL DID NOT JOIN. Between this pass and theirs stand
// R11's three CopyBufferToBuffer (the witness capture), and a copy cannot
// be encoded inside a pass; O-2 pins it after the compute. Four passes into
// one was the ask; two into one is what the frame's shape allows.
inline void dispatch_compute(MachineCtx* c, wgpu::CommandEncoder& encoder,
                             bool write_live_card) {
    wgpu::ComputePassDescriptor desc{};
    desc.label = "Compute Phase";
    desc.timestampWrites = c->gpuState_.meter_arm_compute(meter_row::DispatchCompute);
    wgpu::ComputePassEncoder compute = encoder.BeginComputePass(&desc);
    // LOOM_2 pass head: WORLD + FRAME are every pipeline's strata 0/1.
    { compute.SetBindGroup(0, c->gpuState_.world_group());
      compute.SetBindGroup(1, c->gpuState_.frame_c_group()); }

    // THE CARD FIRST — before every consumer, on its own group 2/3 pair
    // (ZONES), which the ribbon's binds below replace.
    if (write_live_card) {
        c->renderer_.dispatch_live_card_write(
            compute, c->gpuState_.zones_state_group(), c->gpuState_.zones_textures_group()
        );
    }

    // The ribbon room runs FIRST and on its OWN group 2 (the ribbon state
    // group) — outside the pass-head contract below, which is why it stays
    // ahead of those binds. Head then body, one pair of binds; running first
    // is also what makes the read-only agent/floater windows it binds a
    // one-frame-old read, which is what the Sky Rule wants.
    if (c->ribbon_state_.rendered_slot != UINT32_MAX) {
        c->renderer_.dispatch_ribbon(
            compute,
            c->gpuState_.ribbon_state_group(), c->gpuState_.ribbon_textures_group(),
            GPUState::ribbon_ring_workgroups()
        );
    }

    // LOOM_2 (OIL_1 U11's hoist, restratified): WORLD/FRAME ride the
    // pass head above; the family pairs bind per stratum owner — the
    // AGENTS pair here, FRAME_K inside the camera/VP helpers. The
    // agents pair is restored after the camera for the floaters.
    compute.SetBindGroup(2, c->gpuState_.agents_state_group());
    compute.SetBindGroup(3, c->gpuState_.agents_textures_group());

    c->renderer_.dispatch_update_player_agent(compute);
    c->renderer_.dispatch_update_other_agents(compute);
    c->renderer_.dispatch_update_camera_vp(compute,
        c->gpuState_.frame_k_state_group(), c->gpuState_.frame_k_textures_group());
    compute.SetBindGroup(2, c->gpuState_.agents_state_group());
    compute.SetBindGroup(3, c->gpuState_.agents_textures_group());
    c->renderer_.dispatch_update_sphere(compute);
    c->renderer_.dispatch_update_cube(compute);

    compute.End();

    // CHORD_3 — the GPU truth reaches the render frame. update_camera_vp
    // above is the sovereign writer of camera_state AND vp_data — one
    // kernel since SPINE_2, one lane, both writes; frame_r.camera and
    // frame_r.vp are the render stages' windows onto them. The copy is
    // encoded HERE, after the pass closes, because
    // the pass boundary is the ordering guarantee — and by copy rather
    // than by CPU hand because the readback law forbids the other route.
    c->gpuState_.encode_frame_r_main_sync(encoder);
}

inline void dispatch_frustum_cull(MachineCtx* c, wgpu::CommandEncoder& encoder, wgpu::Queue& queue) {
    // THE DRAW PLAN: the kernel runs in EVERY mood now — the finite/
    // indoor path draws through the plan too, so the old indoor skip
    // is retired with the direct path it fed.

    // 0. The plan's inputs: band counts + the active zone rects
    // (world footprints persisted at commit_gol — E1 rev2's four
    // floats), packed dense. Uploaded beside the frustum reset.
    {
        GPUDrawPlanParams plan{};
        plan.lod0_count   = c->world_state_.lod0_patch_count;
        plan.render_count = c->world_state_.render_patch_count;
        uint32_t n = 0;
        for (uint32_t i = 0; i < Dim::MAX_GOL_ZONES && n < 8; i++) {
            const auto& z = c->gol_state_.zones[i];
            if (!z.active) continue;
            plan.rects[n][0] = z.corner_x;
            plan.rects[n][1] = z.corner_z;
            plan.rects[n][2] = z.extent_x;
            plan.rects[n][3] = z.extent_z;
            n++;
        }
        plan.rect_count = n;
        c->gpuState_.upload_draw_plan(queue, plan);
    }

    // 1. Reset compute buffer (constant args + zero instanceCounts)
    c->gpuState_.reset_frustum_indirect(queue);

    // 2. Compute pass — frustum cull writes atomics + visible indices
    {
        wgpu::ComputePassDescriptor cpd{};
        cpd.label = "Frustum Cull Patches";
        cpd.timestampWrites = c->gpuState_.meter_arm_compute(meter_row::FrustumCull);
        wgpu::ComputePassEncoder compute = encoder.BeginComputePass(&cpd);
        // LOOM_2 pass head: WORLD + FRAME are every pipeline's strata 0/1.
            { compute.SetBindGroup(0, c->gpuState_.world_group());
          compute.SetBindGroup(1, c->gpuState_.frame_c_group()); }
        c->renderer_.dispatch_frustum_cull(
            compute, c->gpuState_.cull_state_group(), c->gpuState_.empty_group()
        );
        compute.End();
    }

    // 3. Copy compute buffer → indirect buffer (Dawn D3D12 can't share Storage|Indirect)
    encoder.CopyBufferToBuffer(
        c->gpuState_.frustum_compute_buffer(), 0,
        c->gpuState_.frustum_indirect_lod0(), 0,
        FC_ARGS_BYTES
    );
}

// ═══ SHADOW PASS ═════════════════════════════════════════════════

// ONE SHADOW PASS, ONE LIGHT (ONE_WORLD-II U4). A two-texture atlas arm
// stood in front of this one, opened when the mood set spot_light_active:
// lights 0-1 tiled onto the sun's own map and 2-3 onto a second texture,
// one render pass per TEXTURE with per-light viewports and scissors, the
// light index riding immediate data. It went with the rooms it lit. The
// sun's map is untouched — it wore a second hat as the atlas's first
// texture, and only the hat is gone.
inline void render_shadow_pass(MachineCtx* c, wgpu::CommandEncoder& encoder) {
    {
        // ─── The shadow pass ────────────────────────────────
        wgpu::RenderPassDepthStencilAttachment depthAttachment{};
        depthAttachment.view = c->gpuState_.shadow_map_view();
        depthAttachment.depthLoadOp = wgpu::LoadOp::Clear;
        depthAttachment.depthStoreOp = wgpu::StoreOp::Store;
        depthAttachment.depthClearValue = 1.0f;

        wgpu::RenderPassDescriptor desc{};
        desc.label = "Shadow Pass";
        desc.colorAttachmentCount = 0;
        desc.depthStencilAttachment = &depthAttachment;
        desc.timestampWrites = c->gpuState_.meter_arm_render(meter_row::ShadowPass);

        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);

        // OIL_1 U12 — the pass-head binds (see the atlas arm above).
        // Outdoor draws for the sun, which shadow_light_vp() reads from
        // frame_r.vp.light_vp; shadow_slot is unread on this path
        // (spots.count == 0) and binds record 0 for determinism.
        // BUNDLE_1: the sun's whole draw list is one recorded bundle, head
        // binds included — ExecuteBundles resets pass state, so the bundle
        // carries its own. The direct arm below is not a fallback that can
        // drift: it calls the SAME draw_shadow_all the recorder called.
        if (c->renderer_.shadow_sun_bundle_ready()) {
            wgpu::RenderBundle b = c->renderer_.shadow_sun_bundle();
            pass.ExecuteBundles(1, &b);
        } else {
            pass.SetBindGroup(0, c->gpuState_.world_group());
            pass.SetBindGroup(1, c->gpuState_.frame_r_group());
            pass.SetBindGroup(2, c->gpuState_.shadow_state_group());
            pass.SetBindGroup(3, c->gpuState_.shadow_textures_group());
            draw_shadow_all(c, pass, /*cast_terrain=*/true);
        }
        pass.End();
    }
}

// All shadow draws: terrain (FORK) + the drawable table (shadow filter).
// A shadow pass is DEPTH-ONLY, so draw order is doubly immaterial here.
//
// cast_terrain — true for the sun pass, false for every spot atlas tile
// (UMBRA_4). An indoor spot sits under a shell with a cone that never
// reaches the horizon, and the pass ran ONCE PER LIGHT: up to
// MAX_SPOT_LIGHTS full terrain redraws per frame, inside the known
// bottleneck, to shadow a surface the cone cannot light. No light-volume
// bounding mechanism is built to decide this — no mechanism until a
// measurement asks. The drawable table is untouched: the indoor scene's
// actual occluders all still cast.
template <class Enc>
inline void draw_shadow_all(MachineCtx* c, Enc& pass, bool cast_terrain) {
    // FORK — terrain, both bands at LOD1 density (ECONOMY_1 E2): the
    // shadow target resolves coarser than even the half mesh, and the
    // decode is patch-agnostic. Both bands take the ring IB at the LIVE
    // LOD1 count (OPT_1e): prefix + curtain tail while any zone is
    // active anywhere — the tail is the slab WALLS, the only thing
    // connecting a lifted cell's shadow to its base (the clean-prefix
    // cut detached them and the caps' shadows floated free) — and the
    // clean prefix at true rest, when no cell anywhere can lift.
    //
    // THE CASTER LOD PIN (UMBRA_3, ruled here rather than re-cut). The
    // ladder is two rungs, both terrain-mesh densities of a 50 wu patch:
    // LOD0 at PATCH_MESH_N = 64 (0.781 wu per quad edge) and the ring's
    // stride-2 cap lattice at 1.5625 wu (CELL_1 rev2). Against a
    // post-UMBRA_5 texel of 0.2051 wu, LOD0 is 3.8 texels per edge —
    // finer than the map can resolve, so pure cost — and the ring is
    // 7.6, already coarser than the target. Neither rung satisfies
    // "edge <= 2 x texelWorld", so that rule selects nothing; the pin is
    // nonetheless already at the ladder's COARSEST rung and there is
    // nothing coarser to move to.
    //
    // SCOPE THAT CLAIM CAREFULLY — it is about DENSITY, not about the SET.
    // Nothing here selects a mesh density by distance: both bands take the
    // LOD1 buffer unconditionally, so a caster's silhouette never
    // re-tessellates as the camera nears it. That is what acquits the
    // second of the two suspects the campaign named for the "shadows
    // compose as we approach" artifact.
    //
    // The instance COUNTS below are a different matter and do move with
    // the eye: band_patches (surface/patch_system.hpp) partitions patches
    // against lod0_radius and the veil ring measured from THE POINT, and
    // in camera-host mode the point IS the eye. So WHICH patches cast
    // tracks the viewer even though HOW FINELY they cast does not.
    //
    // That set boundary is the veil ring (325 wu), and post-UMBRA_5 the
    // sun frustum's half-extent is 420 wu — so the cast set now ends
    // strictly INSIDE the shadow map's coverage, where nothing is drawn to
    // receive anyway (the ring is the draw authority). Before UMBRA_5 the
    // radius was 300 and the relation was inverted: the shadow map ran out
    // 25 wu BEFORE the drawn world did, and that visible edge is what the
    // campaign was chasing. It is now structurally gone, not merely
    // pushed.
    // THE SUBTRACTION MASK (PANORAMA_1) — the shadow pass's two halves. Same
    // rule as the main pass: skipped at the encoder, so the pass row reads
    // the absence. `cast_terrain` is the MOOD's word and stays ahead of it.
    const uint32_t smask = c->gpuState_.config().shadow_mask;

    // ONE DRAW, ONE RECORD (BUNDLE_1, R-G). The two bands shared this
    // pipeline and this index buffer and differed only in their instance
    // RANGE: [0, lod0) then [lod0, render). Their union is [0, render) — one
    // draw of render_patch_count instances, staged by stage_draw_ledger.
    if (cast_terrain && (smask & ShadowBit::TERRAIN)) {
        c->renderer_.draw_shadow_patch_terrain(
            pass,
            c->gpuState_.patch_index_buffer_lod1(),
            c->gpuState_.draw_ledger_buffer(),
            GPUState::draw_record_offset(GPUState::DR_SHADOW_TERRAIN)
        );
    }

    // The drawable table — shadow members, canonical order.
    DrawBind b{ /*shadow=*/true, /*ribbon_bit=*/true };
    if (smask & ShadowBit::TABLE)
    draw_table(c->renderer_, c->gpuState_, pass, b, DRAW_SHADOW);

}

// ═══ THE MAIN PASS'S OPAQUE LIST (BUNDLE_1) ══════════════════════
//
// ONE DRAW LIST, TWO ENCODERS. This is the whole of what the main pass
// draws except the fade — and it is called BOTH by render_main_pass (with
// a wgpu::RenderPassEncoder, exactly as it always ran) and by
// record_bundles (with a wgpu::RenderBundleEncoder). There is no second
// list to keep in step, which is R3 written as code.
//
// THE MASKS ARE READ HERE, so under a bundle they are read AT RECORDING.
// PANORAMA_1's rule — a cleared bit is skipped at the encoder so the pass
// row reads the absence — survives that: the skip is still real, it is
// just taken once per dial-turn instead of once per frame. set_draw_mask
// and set_shadow_mask raise bundlesDirty_ so the turn re-records.
//
// NOT the fade: it is alpha-blended, order-sensitive, and gated on a value
// that moves every frame of a transition. A bundle cannot skip itself, so
// the fade stays a direct draw after ExecuteBundles.
template <class Enc>
inline void encode_main_opaque(MachineCtx* c, Enc& pass,
                               OrbsState& orbs_state_, OrbsDeps& orbs_deps_) {
    // Terrain — THE DRAW PLAN (ECONOMY_1 closing arm): the cull kernel
    // authored three lists; the pass executes them as three indirect
    // draws. Outdoor AND finite/indoor go through the same plan (the
    // kernel sees all bands everywhere). The E1 global-flag selection
    // is RETIRED here — the plan is per-patch; the flag survives only
    // with the snapshot pass that was its last carrier (PRUNE_1).
    // THE SUBTRACTION MASK (PANORAMA_1). Each draw below is skipped AT THE
    // ENCODER when its bit is clear — not culled in the shader, which would
    // still pay the pass's vertex work and leave the meter reading no
    // difference. The mask rests open; a cleared bit is a measurement.
    const uint32_t dmask = c->gpuState_.config().draw_mask;

    c->renderer_.begin_patch_terrain_plan(pass);   // OIL_1 U13: one SetPipeline for the three slots
    // DOMESDAY_0 B3: the per-slot list window rides the vertex-buffer
    // offset now (FC_SEG_A/B/C — the same segments the retired g2:62
    // bind windows carved), delivered to the VS as @location(0).
    if (dmask & DrawBit::TERRAIN_A)
    c->renderer_.draw_patch_terrain_plan_slot(pass,
        c->gpuState_.scene_state_group(),
        c->gpuState_.visible_patch_indices_buffer(), FC_SEG_A_OFF, FC_SEG_A_BYTES,   // plan A window
        c->gpuState_.patch_index_buffer(),           // full IB (zone-overlapped)
        c->gpuState_.frustum_indirect_lod0(), 0);
    if (dmask & DrawBit::TERRAIN_B)
    c->renderer_.draw_patch_terrain_plan_slot(pass,
        c->gpuState_.scene_state_group(),            // B5 (R2): the one scene group
        c->gpuState_.visible_patch_indices_buffer(), FC_SEG_B_OFF, FC_SEG_B_BYTES,   // plan B window
        c->gpuState_.patch_index_buffer_cap_only(),  // cap-only IB (clean LOD0)
        c->gpuState_.frustum_indirect_lod0(), 20);
    if (dmask & DrawBit::TERRAIN_C)
    c->renderer_.draw_patch_terrain_plan_slot(pass,
        c->gpuState_.scene_state_group(),            // B5 (R2): the one scene group
        c->gpuState_.visible_patch_indices_buffer(), FC_SEG_C_OFF, FC_SEG_C_BYTES,   // plan C window
        c->gpuState_.patch_index_buffer_lod1(),      // LOD1 IB (culled at last)
        c->gpuState_.frustum_indirect_lod0(), 40);
    // DOMESDAY_1 B5 (R2): the PlanB/PlanC windows collapsed into the
    // one scene group, so plan C left the RIGHT group bound — the old
    // restore is retired with the groups it restored from.

    // The drawable table — main members, canonical order. All opaque and
    // depth-tested, so order among them is immaterial; this is where the
    // ribbon's ordinal drift dies (it now draws with the entities, not late).
    // The ribbon is a table MEMBER, so its bit is subtracted through the
    // bind rather than by skipping the table it shares.
    DrawBind b{ /*shadow=*/false, /*ribbon_bit=*/(dmask & DrawBit::RIBBON) != 0u };
    if (dmask & DrawBit::TABLE)
        draw_table(c->renderer_, c->gpuState_, pass, b, DRAW_MAIN);

    // FORKS — the specials, kept explicit: the ORDER-SENSITIVE blended pair,
    // LAST and in order: orbs (additive), fade (alpha, no depth write).
    //
    // The orb draw is SCENE family (its per-draw binds were hoisted to the
    // strata), and it inherits the pair it needs: group 2 from slot C of the
    // terrain plan, group 3 from the pass head. Nothing forks the pair between
    // there and here, so nothing has to restore it.
    if (dmask & DrawBit::ORBS)
        render_orbs(orbs_state_, &orbs_deps_, pass);

}

// ═══ THE BUNDLES, RECORDED (BUNDLE_1) ════════════════════════════
//
// Called at the frame boundary when bundlesDirty_, AFTER stage_draw_ledger
// and its flush — not because the bundle needs the contents (it does not:
// an indirect draw reads its count at execution) but because the ledger
// BUFFER must exist before a bundle can capture it, and because recording
// with the frame's masks already staged keeps the two in step.
//
// The head binds are the pass heads', moved inside: ExecuteBundles resets
// the pass's bind state, so a bundle must carry its own. That is also why
// the fade, which draws after ExecuteBundles, rebinds for itself.
inline void record_bundles(MachineCtx* c, OrbsState& orbs_state_, OrbsDeps& orbs_deps_) {
    {   // MAIN — the opaque canonical order, the same list render_main_pass
        // encodes directly when it has no bundle.
        wgpu::RenderBundleEncoder e = c->renderer_.make_main_bundle_encoder();
        e.SetBindGroup(0, c->gpuState_.world_group());
        e.SetBindGroup(1, c->gpuState_.frame_r_group());
        e.SetBindGroup(3, c->gpuState_.scene_textures_group());
        encode_main_opaque(c, e, orbs_state_, orbs_deps_);
        wgpu::RenderBundleDescriptor bd{};
        bd.label = "Main Bundle";
        c->renderer_.set_main_bundle(e.Finish(&bd));
    }
    {   // SHADOW SUN — cast_terrain = true, which is the outdoor arm's word.
        // The indoor spot atlas is NOT bundled: it sets a viewport and a
        // scissor per tile and rebinds group 1 at each light's record, and
        // a bundle can carry none of those.
        wgpu::RenderBundleEncoder e = c->renderer_.make_shadow_sun_bundle_encoder();
        e.SetBindGroup(0, c->gpuState_.world_group());
        e.SetBindGroup(1, c->gpuState_.frame_r_group());
        e.SetBindGroup(2, c->gpuState_.shadow_state_group());
        e.SetBindGroup(3, c->gpuState_.shadow_textures_group());
        draw_shadow_all(c, e, /*cast_terrain=*/true);
        wgpu::RenderBundleDescriptor bd{};
        bd.label = "Shadow Sun Bundle";
        c->renderer_.set_shadow_sun_bundle(e.Finish(&bd));
    }
    c->gpuState_.clear_bundles_dirty();
}

// ═══ MAIN PASS ═══════════════════════════════════════════════════

inline void render_main_pass(MachineCtx* c, wgpu::CommandEncoder& encoder,
    wgpu::TextureView backbuffer, wgpu::TextureView msaaColor,
    wgpu::TextureView depth,
    const float (&clearColor_)[3], OrbsState& orbs_state_, OrbsDeps& orbs_deps_) {

    // DOMESDAY_2 B10 — the msaa arm: when the boot param created a
    // multisampled color target, the pass renders into it and RESOLVES
    // into the backbuffer; the multisampled contents themselves are
    // discarded (resolve is independent of storeOp — tiler-ideal).
    // msaaColor null (msaa=1) leaves every field byte-identical to the
    // pre-B10 descriptor.
    wgpu::RenderPassColorAttachment colorAttachment{};
    colorAttachment.view = backbuffer;
    colorAttachment.loadOp = wgpu::LoadOp::Clear;
    colorAttachment.storeOp = wgpu::StoreOp::Store;
    colorAttachment.clearValue = { (double)clearColor_[0], (double)clearColor_[1], (double)clearColor_[2], 1.0 };
    if (msaaColor) {
        colorAttachment.view = msaaColor;
        colorAttachment.resolveTarget = backbuffer;
        colorAttachment.storeOp = wgpu::StoreOp::Discard;
    }

    wgpu::RenderPassDepthStencilAttachment depthAttachment{};
    depthAttachment.view = depth;
    depthAttachment.depthLoadOp = wgpu::LoadOp::Clear;
    // DISCARD_0 (PASS_0 F1) — Discard, not Store. The console's depth
    // texture is created with usage RenderAttachment ALONE
    // (console.hpp createDepthBuffer): no TextureBinding, so it cannot
    // enter a bind group and no shader can sample it; no CopySrc, so
    // nothing reads it back. Its contents are unreachable the instant
    // this pass ends, and Store writes the whole attachment to main
    // memory anyway — 4·W·H bytes per frame, every arm, for a resource
    // with no reader. Discard is the op for that case.
    //
    // THE SAFETY PROOF IS THE USAGE MASK, not this comment. If
    // createDepthBuffer ever gains TextureBinding or CopySrc, a reader
    // becomes possible and this line must go back to Store in the same
    // commit that grants it.
    depthAttachment.depthStoreOp = wgpu::StoreOp::Discard;
    depthAttachment.depthClearValue = 1.0f;

    wgpu::RenderPassDescriptor desc{};
    desc.label = "Rasterized Scene";
    desc.colorAttachmentCount = 1;
    desc.colorAttachments = &colorAttachment;
    desc.depthStencilAttachment = &depthAttachment;
    desc.timestampWrites = c->gpuState_.meter_arm_render(meter_row::MainPass);

    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&desc);

    // OIL_1 U13 (ledger: R19, C7) — THE PASS-HEAD BIND, restated for
    // the LOOM_2 numbering and the B5 collapse: groups 0 (WORLD),
    // 1 (FRAME) and 3 (scene textures) are the same for every draw in
    // this pass, so they bind once here. Group 2 is set by the plan
    // slot helper — since B5 all three slots bind the ONE scene group,
    // and the table draws inherit it after slot C.
    // Group 1 carries the shadow_slot dynamic seat, so the bind passes
    // one offset; nothing outside the shadow atlas reads it.
    // BUNDLE_1: the opaque list is one recorded bundle, head binds included
    // — ExecuteBundles resets pass state, so the bundle carries its own.
    // Nothing draws after it now (ONE_WORLD-I took the fade), so the reset
    // is the pass's last word. The direct arm cannot drift from the
    // bundle: both call encode_main_opaque.
    if (c->renderer_.main_bundle_ready()) {
        wgpu::RenderBundle mb = c->renderer_.main_bundle();
        pass.ExecuteBundles(1, &mb);
    } else {
        pass.SetBindGroup(0, c->gpuState_.world_group());
        pass.SetBindGroup(1, c->gpuState_.frame_r_group());
        pass.SetBindGroup(3, c->gpuState_.scene_textures_group());
        encode_main_opaque(c, pass, orbs_state_, orbs_deps_);
    }

    // THE FADE REBOUND FOR ITSELF HERE — a world_group() restatement after
    // ExecuteBundles, the three EMPTY strata its layout wanted, its own
    // draw_mask read and the LATTICE_4 rest gate. All of it left with the
    // overlay (ONE_WORLD-I). The bundle's binds are the pass's last word
    // now, and nothing draws after it.

    pass.End();
}

// ═══ LIGHT MATRIX COMPUTATION ════════════════════════════════════



} // namespace the_board
} // namespace t7
