#pragma once
// ═══════════════════════════════════════════════════════════════════════
// THE BINDING REGISTRY (C6) — the single source of truth for GPU binding
// NUMBERS. Governed by L6, src/docs/LAWS.md; read it before adding,
// moving, or reusing a number.
//
// The shape of the table, so it reads correctly: numbers are GROUP-SCOPED
// (g0:: and g1:: are separate namespaces — 25 is tile_grid in group 0
// and shadow_map in group 1), AUTHORED (the render = compute
// + 200 band at the bottom is a witness over the literals, never their
// source), and ONE CONSTANT PER SITE rather than per buffer — the same
// buffer wears several names because each name is one (group, slot):
// patch_instances(340) / fc_patches(340); orb_state(410) /
// render_orb_state(400) / orb_state_ro(413).
//
// The WGSL @binding literals in world.wgsl (96 declarations over 93 slots;
// three fc_ aliases share slots: fc_config / fc_vp / fc_patches) are a
// MIRROR of this file, kept in lockstep by the boot-time bind-group and
// pipeline validation, not by the compiler. The names here deliberately
// equal the WGSL variable names so the mirror is greppable in both.
// ═══════════════════════════════════════════════════════════════════════
#include <cstdint>

namespace t7 {
    namespace the_board {
        namespace bind {

            // ─────────────────────────────────────────────────────────────
            // GROUP 0 — the everything-group: uniforms + storage sim state +
            // write storage-textures + mesh-gen scratch + the render_* read
            // mirrors. Numeric range 0–501, banded by subsystem.
            // ─────────────────────────────────────────────────────────────
            namespace g0 {
                // core (0–2)
                inline constexpr uint32_t signal                     = 0;
                inline constexpr uint32_t config                     = 1;
                inline constexpr uint32_t vp_data                    = 2;   // aka fc_vp (frustum-cull alias)

                // terrain / patch lattice (20–30)
                inline constexpr uint32_t fc_draw_plan               = 22;  // ECONOMY_1 draw plan: counts + zone rects (uniform, cull kernel only). Reuses the slot the retired terrain_mesh_indices freed — L6 item 5, the cmg precedent.
                inline constexpr uint32_t patch_params               = 23;
                inline constexpr uint32_t patch_heightfield_array_write = 24;
                inline constexpr uint32_t tile_grid                  = 25;
                inline constexpr uint32_t patch_cell_color_array_write = 27;
                inline constexpr uint32_t patch_height_scratch       = 28;
                inline constexpr uint32_t pyramid_instances          = 30;
                inline constexpr uint32_t live_card_write            = 31;  // GROUND_CARD_1: the live card (storage-tex write; writer kernel)
                inline constexpr uint32_t live_card_scratch          = 32;  // TRUEBAND_CONTACT_1: two-pass writer scratch (stride-2: Δh, gol)

                // agents / camera (60–101)
                inline constexpr uint32_t agent_state                = 60;
                inline constexpr uint32_t portal_array               = 62;
                inline constexpr uint32_t camera_state               = 80;
                inline constexpr uint32_t floating_entities          = 100;

                // agent registries (110–112)
                inline constexpr uint32_t agent_behaviors            = 110;
                inline constexpr uint32_t agent_tier_gains           = 111;
                inline constexpr uint32_t agent_figure_profiles      = 112;  // uniform: PawnFigure[14] (render VS only)

                // ribbon (120–122)
                inline constexpr uint32_t ribbon_state               = 120;
                inline constexpr uint32_t ring_xforms                = 121;
                inline constexpr uint32_t head_poses                 = 122;

                // photographer / placement (140–152)
                inline constexpr uint32_t photographer_config        = 140;
                inline constexpr uint32_t photographer_vp            = 141;
                inline constexpr uint32_t photographer_camera_out    = 142;
                inline constexpr uint32_t photo_painting_slots       = 143;
                inline constexpr uint32_t photo_heightfield          = 145;
                inline constexpr uint32_t photo_sampler              = 146;
                inline constexpr uint32_t arch_ground                = 147;
                inline constexpr uint32_t column_ground              = 148;
                inline constexpr uint32_t plant_ground               = 150;
                inline constexpr uint32_t entity_ground_atlas_write  = 151;
                inline constexpr uint32_t patch_grid                 = 152;

                // GoL zones / pawn aura (160–172)
                inline constexpr uint32_t zone_config                = 160;
                inline constexpr uint32_t zone_life                  = 161;
                inline constexpr uint32_t zone_life_tex_write        = 162;
                inline constexpr uint32_t zone_derive_requests       = 166;
                inline constexpr uint32_t pawn_aura_cfg              = 170;
                inline constexpr uint32_t pawn_aura_cells            = 171;
                inline constexpr uint32_t pawn_aura_tex_write        = 172;

                // mesh-gen scratch: palm/cactus/blade (180–188)
                inline constexpr uint32_t palmg_params               = 180;
                inline constexpr uint32_t palmg_vertices             = 181;
                inline constexpr uint32_t palmg_indices              = 182;
                inline constexpr uint32_t cactusg_params             = 183;
                inline constexpr uint32_t cactusg_vertices           = 184;
                inline constexpr uint32_t cactusg_indices            = 185;
                inline constexpr uint32_t bladeg_params              = 186;
                inline constexpr uint32_t bladeg_vertices            = 187;
                inline constexpr uint32_t bladeg_indices             = 188;

                // mesh-gen scratch: arch/column (190–198; 192 unassigned)
                inline constexpr uint32_t cmg_config                 = 190;  // DesignConfig view for the cmg kernel (the ceiling gate)
                inline constexpr uint32_t cmg_column_ground          = 191;  // read-only column_ground view (the terrain delta)
                inline constexpr uint32_t amg_params                 = 193;
                inline constexpr uint32_t amg_vertices               = 194;
                inline constexpr uint32_t amg_indices                = 195;
                inline constexpr uint32_t cmg_params                 = 196;
                inline constexpr uint32_t cmg_vertices               = 197;
                inline constexpr uint32_t cmg_indices                = 198;

                // render mirrors — the +200 band (200–361)
                inline constexpr uint32_t render_vp                  = 201;
                inline constexpr uint32_t render_agents              = 260;
                inline constexpr uint32_t render_camera              = 280;
                inline constexpr uint32_t render_floating            = 300;
                inline constexpr uint32_t render_light               = 320;
                inline constexpr uint32_t render_point_lights        = 321;
                inline constexpr uint32_t render_spot_lights         = 322;
                inline constexpr uint32_t patch_instances            = 340;   // aka fc_patches
                inline constexpr uint32_t render_ribbon              = 360;
                inline constexpr uint32_t render_ring_xforms         = 361;

                // atlas / cull outputs / orbs (390–501)
                inline constexpr uint32_t entity_ground_atlas        = 390;
                inline constexpr uint32_t visible_patch_indices      = 391;
                inline constexpr uint32_t render_orb_state           = 400;
                inline constexpr uint32_t orb_state                  = 410;
                inline constexpr uint32_t orb_config                 = 411;
                inline constexpr uint32_t orb_state_prev             = 412;
                inline constexpr uint32_t orb_state_ro               = 413;
                inline constexpr uint32_t orb_state_prev_rw          = 414;
                inline constexpr uint32_t fc_visible                 = 500;
                inline constexpr uint32_t fc_indirect                = 501;
            } // namespace g0

            // ─────────────────────────────────────────────────────────────
            // GROUP 1 — shared samplers + read-side sampled textures. Numeric
            // range 22–52. (These numbers are UNRELATED to the group-0 numbers
            // of the same value — that is why the namespace exists.)
            // ─────────────────────────────────────────────────────────────
            namespace g1 {
                inline constexpr uint32_t bilinear_sampler           = 22;
                inline constexpr uint32_t nearest_sampler            = 23;
                inline constexpr uint32_t shadow_map                 = 25;
                inline constexpr uint32_t shadow_sampler             = 26;
                inline constexpr uint32_t spot_shadow_map            = 27;
                inline constexpr uint32_t patch_heightfield_array_read = 28;
                inline constexpr uint32_t patch_cell_color_array_read  = 29;
                inline constexpr uint32_t zone_life_read             = 31;
                inline constexpr uint32_t zone_params                = 32;
                inline constexpr uint32_t pawn_aura_read             = 33;
                inline constexpr uint32_t live_card_read             = 34;  // GROUND_CARD_1: the live card (sampled read; render + compute)
                inline constexpr uint32_t painting_slots             = 50;
                inline constexpr uint32_t painting_array             = 51;
                inline constexpr uint32_t painting_sampler_filt      = 52;
            } // namespace g1

            // ─────────────────────────────────────────────────────────────
            // GROUP 2 — THE AGENTS' ROOM (BATCH F-B, Option B ruling): the
            // agent kernels' own bind group. All future agent-side bindings
            // (the week's musical couplings included) land HERE, without
            // touching the six pipelines sharing the entity layout. Bound
            // only by update_player_agent / update_other_agents.
            // ─────────────────────────────────────────────────────────────
            namespace g2 {
                // Occupier windows — read-only views onto the SAME mesh-param
                // buffers the mesh-gen groups bind (one fact, one home; only
                // reachability topology differs).
                inline constexpr uint32_t occupier_cmg               = 0;   // ColumnMeshParams[32] (columns 0-15, antennas 16-31)
                inline constexpr uint32_t occupier_amg               = 1;   // ArchMeshParams[16]
            } // namespace g2

            // ─────────────────────────────────────────────────────────────
            // WITNESSES — the render = compute + 200 band. These CHECK the
            // authored literals above; they are NOT the source of any value.
            // If a future edit breaks the band, this fails the BUILD (loud),
            // it does not silently re-derive the number.
            // ─────────────────────────────────────────────────────────────
            static_assert(g0::render_vp      == g0::vp_data  + 199, "render band: vp (2 -> 201)");
            static_assert(g0::render_agents  == g0::agent_state  + 200, "render band: agents");
            static_assert(g0::render_camera  == g0::camera_state + 200, "render band: camera");
            static_assert(g0::render_floating == g0::floating_entities + 200, "render band: floating");

        } // namespace bind
    } // namespace the_board
} // namespace t7
