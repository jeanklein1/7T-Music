#pragma once

// ─── cartridge.hpp ───────────────────────────────────────────────
// MOD campaign (ROSTER-1a/1b + LADDER-1..4): conversion history in audit/LADDER.md.
//
// THE_BOARD — Generative world engine.
//
// TWO REGIMES (constitution §1): CONVERTED modules are file-scope
// headers above the class; UNCONVERTED remain class-body includes —
// both lawful until the last module converts.
//
// SEAM[spine:owns] FAMILY_DISPATCH is genuinely spine work — the
//   integration hub that ties the 12 families together. Each row's
//   body lives in the family's owning module. Per Ch. 15 of the seam
//   map. Adding a new family means: write select/place/commit/
//   evict/prepare_mesh in the owning module, add wrappers below,
//   add 1 row to FAMILY_DISPATCH.
// SEAM[spine:K2-related] the dispatch_prepare_mesh_*,
//   dispatch_mesh_gen_* wrappers (~400 lines below FamilyDispatch)
//   are integration glue, not module work. They live here correctly.
//   NOTE[seam-map] keep wrappers here; they're the integration layer
//   between FAMILY_DISPATCH and per-family modules.
// SEAM[spine:P5] readback state machines + world_state_.world_gen counter are
//   pattern P5 (release-pending sentinel) at the spine level. Pawn +
//   floater readbacks each protect against stale callbacks from
//   previous worlds via world_state_.world_gen capture in the closure. Genuinely
//   spine-owned, not a leak.
// SEAM[spine:P8] PlayerState's commented "Future (deferred)" fields
//   are explicit latent infrastructure: aura_presence is live here;
//   the other deferred fields await the unified entity layer.
//   Pattern P8 visible in source.
// SEAM[spine:active-patch-system] the ActivePatch struct,
//   patches_[MAX_PATCHES] array, find_patch / evict_patch /
//   evict_patch_entities / audit_entity_integrity, plus the entity_refs
//   registry on each ActivePatch. The lifecycle hub for streamed-in
//   patches and their entity ownership records. Cross-module readers:
//   spawn_engine.inl (commit functions call host->record_entity),
//   ribbon.inl (two-tip late registration), gallery.inl
//   (evict_paintings_for_patch).
// SEAM[spine:portal-system] portal/transition state machine. Owns
//   TransitionPhase, mood_state_.transition_timer, pendingDestination_, the
//   PORTAL_COLORS table, the back-portal pending state, and the
//   trigger-detection hooks called by readback. Mood.inl drives portal
//   spawning (force_spawn_portal_at, force_spawn_back_portal,
//   force_spawn_finite_portals); spine owns the request → activation
//   flow.
// SEAM[spine:family-dispatch] all evict_<family> (owner-side),
//   dispatch_prepare_mesh_<family>, dispatch_mesh_gen_<family>
//   wrapper functions land here — referenced by FAMILY_DISPATCH and
//   by spawn_engine.inl's commit/evict pipelines.
// ─────────────────────────────────────────────────────────────────

#include "render/render_cartridge.hpp"
#include "core/input_event.hpp"
#include "cartridges/the_board/roster.hpp"
#include "cartridges/the_board/modules/seed_utils.hpp"           // hash/gaussian/tier-select helpers (pure-math leaf)
#include "cartridges/the_board/modules/ground_architecture.hpp"  // ground contributor/policy tables + compile-time DAG checks
#include "cartridges/the_board/modules/entity_types.hpp"         // THE CONTRACT HOME: pipeline contracts + boundary DTOs + queue unions + dispatch row/table decl
#include "cartridges/the_board/modules/mood_constants.hpp"       // MOOD_COUNT + the Mood IDs + PortalDestination
#include "cartridges/the_board/modules/floater_vocabulary.hpp"   // floater TYPES (ActiveFloater/ActiveCube), file scope
#include "cartridges/the_board/modules/pawn.hpp"                 // PawnState + configs + decls (impl is pawn.inl, post-class)
#include "cartridges/the_board/state.hpp"
#include "cartridges/the_board/modules/spheres.hpp"              // SphereState + evictor/funnel decls (impl is spheres.inl, post-class)
#include "cartridges/the_board/modules/entities.hpp"             // grounded-family vocabulary + EntitiesState + preparer decls (impl is entities.inl, post-class)
#include "cartridges/the_board/modules/orbs.hpp"                 // orb console/registries + OrbsState + ORB_MOOD_TABLE + decls (impl is orbs.inl, post-class)
#include "cartridges/the_board/modules/gol_zones.hpp"            // GoL vocabulary + GoLState + decls (impl is gol_zones.inl, post-class)
#include "cartridges/the_board/modules/agents.hpp"               // agent registries + console + AgentState + decls (impl is agents.inl, post-class)
#include "cartridges/the_board/modules/cube_behaviors.hpp"       // cube behavior registry + CubeBehaviorsState + decls (impl is cube_behaviors.inl, post-class)
#include "cartridges/the_board/modules/gallery.hpp"              // shot vocabulary + console + GalleryState + decls (impl is gallery.inl, post-class)
#include "cartridges/the_board/modules/ribbon.hpp"               // ribbon console + color vocabulary + tiers + RibbonState + decls (impl is ribbon.inl, post-class; pairing suspension named in its banner)
#include "cartridges/the_board/modules/input.hpp"                // InputState/KeyState/MouseState + decls (impl is input.inl, post-class; carries its own GLFW include)
#include "cartridges/the_board/modules/render_passes.hpp"        // the nine pass/dispatch + light-VP decls (impl is render_passes.inl, post-class; module owns no state)
#include "cartridges/the_board/modules/mood.hpp"                 // MoodProfile + MOOD_TABLE + portal colors + palettes + door/applier/deriver decls (impl is mood.inl, post-class; mood owns no state)
#include "cartridges/the_board/modules/population_themes.hpp"  // S2: THEMES + ThemeEnvelope + ThemesState + decls (impl is population_themes.inl, post-class)
#include "cartridges/the_board/modules/tile_world.hpp"          // S2: archetypes + tokens + TileState/cache + TileWorldState + decls (impl is tile_world.inl, post-class)
#include "cartridges/the_board/renderer.hpp"
#include "coupling/visual_canvas.hpp"
#include <cmath>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <filesystem>
#include <algorithm>
#include <string>
#include <vector>
#include "external/stb_image.h"

namespace t7 {
    namespace the_board {

        class Cartridge : public RenderCartridge {

        // COMPOSITION ROOT — ORGANS ARE PUBLIC: sight is free; writes pass
        // through declared seams; the census enforces the seam law, not
        // access control.
        public:

            wgpu::Device device_;
            wgpu::TextureFormat colorFormat_;
            wgpu::TextureFormat depthFormat_;

            GPUState gpuState_;
            Renderer renderer_;

            // ═══ COMPOSITION ROOT — MODULE STATE ════════════════════════
            //
            SphereState sphere_state_;

            //   cube_behaviors_state_ — CubeBehaviorsState (cube_behaviors.hpp),
            //     the cube diagnostics + the cube active-slot mirror.
            CubeBehaviorsState cube_behaviors_state_;

            //   pawn_state_ — PawnState (pawn.hpp), the pawn aura + presence state.
            PawnState pawn_state_;

            EntitiesState entities_state_;

            //   orbs_state_ — OrbsState (orbs.hpp), the sky-dome lifecycle +
            //     player-owned anchor/rule/gesture state.
            OrbsState orbs_state_;

            //   gol_state_ — GoLState (gol_zones.hpp), the zone slots + counts +
            //     mood gate + derive-request queue.
            GoLState gol_state_;

            //   agent_state_ — AgentState (agents.hpp), the 32-slot CPU mirror +
            //     respawn counters + diagnostic overrides.
            AgentState agent_state_;

            GalleryState gallery_state_;

            RibbonState ribbon_state_;

            //   themes_state_ — ThemesState (population_themes.hpp), the theme
            //     envelope machine + the per-patch selection.
            ThemesState themes_state_;

            //   tile_world_state_ — TileWorldState (tile_world.hpp), the tile
            //     cache + the terrain tokens (what the terrain remembers).
            TileWorldState tile_world_state_;

            InputState inputState_;
            KeyState keys_;
            MouseState mouse_;

            // ═══ TIME STATE ═════════════════════════════════════════════
            // Per-frame clock state used everywhere. beats/seconds advance
            // monotonically; dt is the most recent frame delta.
            struct TimeState {
                float beats   = 0.0f;
                float seconds = 0.0f;
                float dt      = 0.016f;
                // Musical tempo follower: beats/sec, HELD-LAST through silence
                // and stopped transport; defaults to 100 BPM (the calibration
                // anchor for the authored idle motion).
                float beat_rate   = 100.0f / 60.0f;
                float prev_beats  = 0.0f;
            };
            TimeState time_state_;

            VisualCanvas  visual_canvas_;
            TargetBinding fog_density_dst_{};   // resolved "fog.density" pipe
            // Ribbon amp pipes (pitch compass) — resolved once at bind.
            TargetBinding ribbon_amp_lat_dst_{};
            TargetBinding ribbon_amp_vert_dst_{};
            TargetBinding ribbon_tint_stim_dst_{};
            TargetBinding ribbon_tint_mix_dst_{};
            TargetBinding fog_color_dst_{};      // resolved "fog.color" pipe (3 wide)

            // Sun + atmosphere (driven by active mood — see apply_mood)
            float sunDirection_[3] = { 0.69f, -0.71f, -0.14f };
            float sunColor_[3] = { 1.0f, 0.95f, 0.9f };
            float clearColor_[3] = { 0.85f, 0.78f, 0.72f };

            // ═══ MOOD STATE ═════════════════════════════════════════════
            //
            // Owned by mood.inl semantically, but lives spine-resident
            // because mood-applied values feed every other subsystem.
            struct MoodState {
                // ── Currently active mood ──
                uint32_t active = 0;

                // ── Mood-applied values (re-set on each apply_mood) ──
                float sun_intensity = 0.8f;
                float sun_ambient   = 0.25f;
                float terrain_amp_ceiling = 0.0f;       // mirrors GPU config.terrain_amp_ceiling
                bool  spot_light_active = false;

                // ── Transition machinery ──
                float transition_timer         = 0.0f;
                float transition_fade_duration = 0.5f;  // seconds per fade direction
                float transition_fade_alpha    = 0.0f;

                // ── Portal upload flag ──
                bool portals_dirty = true;              // true at boot → first upload guaranteed

                // ── Back-portal return state ──
                bool     back_portal_pending       = false;
                uint32_t back_portal_return_seed   = 0;
                uint32_t back_portal_return_mood   = 0;
                uint32_t back_portal_return_radius = 2;

                // ── Sun orbit (musical coupling) ──
                float sun_orbit_phase = 0.0f;
            };
            MoodState mood_state_;

            // ═══ PLAYER STATE ════════════════════════════════════════════
            //
            // SEAM[spine:P8] PlayerState commented "Future (deferred)" fields
            //   are explicit latent infrastructure: aura_presence is live
            //   here; the other deferred fields await the unified entity
            //   layer. Pattern P8 visible in source.
            struct PlayerState {
                uint32_t possessed_slot = 0;   // slot in agent_state[] that the player inhabits

                // ── Camera + readback ──
                bool    fpv_mode = false;                // first-person view toggle
                bool    sky_mode = false;                // sky-flight: arrows drive the rendered ribbon's head (SEAM[ribbon:sky-mode])
                bool    sky_mode_prev = false;           // previous-frame sky_mode — drives the exit edge (ribbon release)
                float   sky_yaw_eased = 0.0f;            // player's eased yaw (curvature continuity)
                float   readback_x = 0.0f;               // GPU readback of pawn world X
                float   readback_z = 0.0f;               // GPU readback of pawn world Z
                int32_t readback_portal_trigger = -1;    // set by readback callback when pawn hits portal

                // ── Aura presence (closes SEAM[spine:P8]) ──
                float aura_presence = 0.0f;                  // pawn aura ramp (was pawn_state_.aura_presence)

                // Future (deferred):
                //   uint32_t active_couplings;         // COUPLING_* bitmask owned by player
            };
            PlayerState player_{};

            GPUSpotLightArray cpuSpotLights_{};  // count=0 disables (outdoor)

            // ═══ PORTAL & TRANSITION STATE MACHINE ═══════════════════════
            //
            // SEAM[spine:transitions] (K4, Jean, 2026-07-11): the transition
            //   machine and its working members — transitionPhase_,
            //   pendingDestination_, backPortalPosition_, cpuPortalArray_,
            //   MoodState/mood_state_ and kin — are DECLARED SPINE-OWNED
            //   ORCHESTRATION per the §2 residency law, the same legitimacy
            //   class as the P5 readbacks. Mood (mood.hpp/.inl) supplies
            //   vocabulary + appliers + six doors and owns NO state; no
            //   MoodState exists module-side or at the composition root.
            //   Constitution §2 carries the ruling's line.
            // SEAM[spine:portal-system] consumed by the mood module
            //   (force_spawn_* functions read pendingDestination_), input.inl
            //   (keypress mood transitions request via mood.hpp's
            //   request_mood_transition), render() (readback callback drives
            //   portal trigger detection). PORTAL_COLORS lives in mood.hpp —
            //   portal color is mood vocabulary; the machine keeps the
            //   pending state and the trigger hooks.

            enum class TransitionPhase { IDLE, FADE_OUT, TEARDOWN, FADE_IN };
            TransitionPhase transitionPhase_ = TransitionPhase::IDLE;

            PortalDestination pendingDestination_{};

            GPUPortalArray cpuPortalArray_{};

            // ── Back-portal (guaranteed exit from finite worlds) ──
            // Position is configurable so special-case layouts can relocate it.
            float backPortalPosition_[2] = { 10.0f, 0.0f };   // world XZ

            // ═══ GPU READBACK + WORLDGEN ═════════════════════════════════
            //
            // SEAM[spine:P5] readback state machines + world_state_.world_gen counter are
            //   pattern P5 (release-pending sentinel) at the spine level.
            //   Pawn + floater readbacks each protect against stale callbacks
            //   from previous worlds via world_state_.world_gen capture in the closure.
            //   Genuinely spine-owned, not a leak.

            enum class PawnReadbackState { IDLE, COPIED, MAPPING };
            PawnReadbackState pawnReadbackState_ = PawnReadbackState::IDLE;
            enum class FloaterReadbackState { IDLE, COPIED, MAPPING };
            FloaterReadbackState floaterReadbackState_ = FloaterReadbackState::IDLE;

            // ROSTER-RESIDUE gol (2e) instrumentation: count of frames the GoL
            // zone-compute block ran (the sole writer of the zone GPU buffers),
            // and the residue-report cadence timer. Read only by the residue
            // check when ROSTER.gol is disabled (proves the buffers pristine).
            uint64_t rosterGolZoneRuns_ = 0;
            float    rosterGolResidueDump_ = 0.0f;

            // ═══ UNIFIED PIER SYSTEM ═════════════════════════════════════
            //
            GPUPierInstance cpuPiers_[Dim::PIER_TOTAL]{};

            // ── Terrain CPU mirror deleted ────────────────────────────────

// ── Spawn Engine & Entity Lifecycle (modules/spawn_engine.inl) ──
#include "modules/spawn_engine.inl"

            // ═══ ACTIVE PATCH SYSTEM ═════════════════════════════════════
            //
            // SEAM[spine:active-patch-system] cross-module readers:
            //   spawn_engine.inl (commit functions call host->record_entity),
            //   ribbon.inl (two-tip late registration), gallery.inl
            //   (evict_paintings_for_patch via the owner-side evict_gallery), and
            //   the family dispatch eviction wrappers below.

            struct WorldState {
                // ── Seed + dimensions ──
                uint32_t active_seed   = 42;     // world master seed (mutable for world transitions)
                uint32_t active_radius = Dim::PATCH_PREGEN_RADIUS;
                bool     finite_mode   = false;
                uint32_t finite_radius = 2;      // 2 → 5×5 = 25 patches
                uint32_t world_gen     = 0;

                // ── Recenter cursor ──
                int32_t last_center_x = INT32_MAX;  // force full regeneration on first frame
                int32_t last_center_z = INT32_MAX;

                // ── Patch counts (this frame) ──
                uint32_t active_patch_count = 0;
                uint32_t render_patch_count = 0;    // visible patches (within circular VISIBLE_RADIUS)
                uint32_t lod0_patch_count   = 0;    // subset of rendered: within LOD_FULL_RADIUS (full mesh)
                uint32_t all_patch_count    = 0;    // all generated patches (including pre-gen ring)
                uint32_t entities_culled    = 0;    // entities hidden by distance culling this frame

                // ── Dirty flags (deferred GPU uploads) ──
                bool pier_count_dirty       = false;  // defer recompute_and_upload_pier_count
                bool ground_entries_dirty   = true;   // defer upload_ground_entries (true at boot)
                bool patch_instances_dirty  = true;   // defer LOD sort + upload_patch_instances
                bool placement_dirty        = true;   // defer dispatch_placement_correction

                // ── Free-layer pool ──
                uint32_t free_layer_count = Dim::MAX_ACTIVE_PATCHES;
            };
            WorldState world_state_;

            // Patch dimensions aliased from Dim:: for local readability
            static constexpr float    PATCH_EXTENT = Dim::PATCH_EXTENT;
            static constexpr uint32_t GRID_RADIUS = Dim::PATCH_GRID_RADIUS;   // inner priority (3 → 7×7)
            static constexpr uint32_t GRID_SIDE = Dim::PATCH_GRID_SIDE;
            static constexpr uint32_t RENDER_RADIUS = Dim::PATCH_RENDER_RADIUS;  // visible radius (5)
            static constexpr uint32_t RENDER_SIDE = Dim::PATCH_RENDER_SIDE;
            static constexpr uint32_t PREGEN_RADIUS = Dim::PATCH_PREGEN_RADIUS; // deep pre-gen buffer (7)
            static constexpr uint32_t MAX_PATCHES = Dim::MAX_ACTIVE_PATCHES;    // 225

            enum class PatchPhase : uint8_t {
                ALLOCATED,      // layer assigned, tile cached, no entities yet
                SPAWNED,        // entities selected + placed + committed
                GENERATED,      // heightfield computed, gallery + GoL spawned
                NEEDS_REGEN,    // heightfield stale (new pier in range)
            };

            struct ActivePatch {
                int32_t grid_x = 0;
                int32_t grid_z = 0;
                uint32_t layer = 0;
                bool valid = false;
                PatchPhase phase = PatchPhase::ALLOCATED;
                bool animated = false;   // true if patch overlaps an active pool

                // Entity ownership (recorded at commit, read at eviction)
                struct EntityRef {
                    uint32_t family;   // PopFamily index
                    uint32_t slot;     // index into Active* array
                };
                static constexpr uint32_t MAX_ENTITY_REFS = 10;
                EntityRef entity_refs[MAX_ENTITY_REFS]{};
                uint32_t entity_ref_count = 0;

                void record_entity(uint32_t family, uint32_t slot) {
                    if (entity_ref_count < MAX_ENTITY_REFS) {
                        entity_refs[entity_ref_count++] = { family, slot };
                    }
                }

                void unregister_entity(uint32_t family, uint32_t slot) {
                    for (uint32_t i = 0; i < entity_ref_count; i++) {
                        if (entity_refs[i].family == family && entity_refs[i].slot == slot) {
                            entity_refs[i] = entity_refs[--entity_ref_count];
                            return;
                        }
                    }
                }

            };

            ActivePatch patches_[MAX_PATCHES]{};

            ActivePatch* find_patch(int32_t gx, int32_t gz) {
                for (uint32_t i = 0; i < world_state_.active_patch_count; i++) {
                    if (patches_[i].valid && patches_[i].grid_x == gx && patches_[i].grid_z == gz)
                        return &patches_[i];
                }
                return nullptr;
            }

            // Hook: full eviction of a single patch.
            void evict_patch(uint32_t pi, wgpu::Queue& queue) {
                free_layer(patches_[pi].layer);
                // Painting eviction now handled by evict_gallery (gallery.inl) via entity_refs
                evict_patch_entities(patches_[pi], queue);
                unregister_footprints_for_patch(patches_[pi].grid_x, patches_[pi].grid_z);
                patches_[pi].valid = false;
            }

            void evict_patch_entities(ActivePatch& patch, wgpu::Queue& queue) {
#ifdef DIAG_ENTITY_LIFECYCLE
                if (patch.entity_ref_count > 0) {
                    float wx = (patch.grid_x + 0.5f) * PATCH_EXTENT;
                    float wz = (patch.grid_z + 0.5f) * PATCH_EXTENT;
                    float dx = wx - player_.readback_x, dz = wz - player_.readback_z;
                    std::cout << "[DIAG:EVICT] patch(" << patch.grid_x << "," << patch.grid_z
                        << ") dist=" << std::sqrt(dx * dx + dz * dz)
                        << " refs=" << patch.entity_ref_count << "\n";
                }
#endif
                for (uint32_t i = 0; i < patch.entity_ref_count; i++) {
                    auto& ref = patch.entity_refs[i];
                    FAMILY_DISPATCH[ref.family].evict_slot(this, ref.slot, queue);
                }

                patch.entity_ref_count = 0;
            }

            void audit_entity_integrity() {
#ifdef DIAG_ENTITY_LIFECYCLE
                //
                uint32_t act_a = 0, act_c = 0, act_n = 0, act_p = 0;
                for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) if (entities_state_.arches[i].active) act_a++;
                for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++) if (entities_state_.columns[i].active) act_c++;
                for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++) if (entities_state_.antennas[i].active) act_n++;
                for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) if (entities_state_.pyramids[i].active) act_p++;

                // Count consistency
                if (act_a != entities_state_.arch_count)
                    std::cout << "[DIAG:AUDIT] ARCH COUNT active=" << act_a << " tracked=" << entities_state_.arch_count << "\n";
                if (act_c != entities_state_.column_count)
                    std::cout << "[DIAG:AUDIT] COL COUNT active=" << act_c << " tracked=" << entities_state_.column_count << "\n";
                if (act_n != entities_state_.antenna_count)
                    std::cout << "[DIAG:AUDIT] ANT COUNT active=" << act_n << " tracked=" << entities_state_.antenna_count << "\n";
                if (act_p != entities_state_.pyramid_count)
                    std::cout << "[DIAG:AUDIT] PYR COUNT active=" << act_p << " tracked=" << entities_state_.pyramid_count << "\n";

                // Collect refs from all patches
                bool ra[Dim::MAX_ARCH_INSTANCES]{};
                bool rc[Dim::MAX_COLUMN_ONLY]{};
                bool rn[Dim::MAX_ANTENNA_ONLY]{};
                bool rp[Dim::MAX_PYRAMID_INSTANCES]{};
                for (uint32_t p = 0; p < world_state_.active_patch_count; p++) {
                    if (!patches_[p].valid) continue;
                    for (uint32_t r = 0; r < patches_[p].entity_ref_count; r++) {
                        auto& ref = patches_[p].entity_refs[r];
                        switch (ref.family) {
                        case PopFamily::PYRAMID:
                            if (ref.slot < Dim::MAX_PYRAMID_INSTANCES) {
                                if (rp[ref.slot]) std::cout << "[DIAG:AUDIT] DUP REF pyr slot=" << ref.slot << " patch=(" << patches_[p].grid_x << "," << patches_[p].grid_z << ")\n";
                                rp[ref.slot] = true;
                            } break;
                        case PopFamily::ARCH:
                            if (ref.slot < Dim::MAX_ARCH_INSTANCES) {
                                if (ra[ref.slot]) std::cout << "[DIAG:AUDIT] DUP REF arch slot=" << ref.slot << " patch=(" << patches_[p].grid_x << "," << patches_[p].grid_z << ")\n";
                                ra[ref.slot] = true;
                            } break;
                        case PopFamily::COLUMN:
                            if (ref.slot < Dim::MAX_COLUMN_ONLY) {
                                if (rc[ref.slot]) std::cout << "[DIAG:AUDIT] DUP REF col slot=" << ref.slot << " patch=(" << patches_[p].grid_x << "," << patches_[p].grid_z << ")\n";
                                rc[ref.slot] = true;
                            } break;
                        case PopFamily::ANTENNA:
                            if (ref.slot < Dim::MAX_ANTENNA_ONLY) {
                                if (rn[ref.slot]) std::cout << "[DIAG:AUDIT] DUP REF ant slot=" << ref.slot << " patch=(" << patches_[p].grid_x << "," << patches_[p].grid_z << ")\n";
                                rn[ref.slot] = true;
                            } break;
                        }
                    }
                }

                // Ghost: active but no ref (will never be evicted)
                for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++)
                    if (entities_state_.arches[i].active && !ra[i])
                        std::cout << "[DIAG:AUDIT] GHOST arch slot=" << i << " host=(" << entities_state_.arches[i].host_gx << "," << entities_state_.arches[i].host_gz << ")\n";
                for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++)
                    if (entities_state_.columns[i].active && !rc[i])
                        std::cout << "[DIAG:AUDIT] GHOST col slot=" << i << " host=(" << entities_state_.columns[i].host_gx << "," << entities_state_.columns[i].host_gz << ")\n";
                for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++)
                    if (entities_state_.antennas[i].active && !rn[i])
                        std::cout << "[DIAG:AUDIT] GHOST ant slot=" << i << " host=(" << entities_state_.antennas[i].host_gx << "," << entities_state_.antennas[i].host_gz << ")\n";
                for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++)
                    if (entities_state_.pyramids[i].active && !rp[i])
                        std::cout << "[DIAG:AUDIT] GHOST pyr slot=" << i << " host=(" << entities_state_.pyramids[i].host_gx << "," << entities_state_.pyramids[i].host_gz << ")\n";

                // Orphan: ref but not active (ref points to freed slot)
                for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++)
                    if (!entities_state_.arches[i].active && ra[i])
                        std::cout << "[DIAG:AUDIT] ORPHAN arch slot=" << i << "\n";
                for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++)
                    if (!entities_state_.columns[i].active && rc[i])
                        std::cout << "[DIAG:AUDIT] ORPHAN col slot=" << i << "\n";
                for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++)
                    if (!entities_state_.antennas[i].active && rn[i])
                        std::cout << "[DIAG:AUDIT] ORPHAN ant slot=" << i << "\n";
                for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++)
                    if (!entities_state_.pyramids[i].active && rp[i])
                        std::cout << "[DIAG:AUDIT] ORPHAN pyr slot=" << i << "\n";

                // Ref overflow: any patch at capacity
                for (uint32_t p = 0; p < world_state_.active_patch_count; p++) {
                    if (patches_[p].valid && patches_[p].entity_ref_count >= ActivePatch::MAX_ENTITY_REFS)
                        std::cout << "[DIAG:AUDIT] REF FULL patch=(" << patches_[p].grid_x << "," << patches_[p].grid_z << ") count=" << patches_[p].entity_ref_count << "\n";
                }
#endif
            }

            // Free-list of available texture layers
            uint32_t freeLayerStack_[MAX_PATCHES]{};

            // ═══ DYNAMIC BUDGETS ═════════════════════════════════════════
            //
            static constexpr uint32_t SPAWN_BUDGET_PER_FRAME = 4;    // max patches to spawn entities for
            static constexpr uint32_t ALLOC_BUDGET_PER_FRAME = 4;    // max patches to allocate per frame
            static constexpr uint32_t EVICT_BUDGET_PER_FRAME = 4;    // max patches to evict per frame
            static constexpr uint32_t PATCH_BUDGET_MIN = 1;
            static constexpr uint32_t PATCH_BUDGET_MAX = 6;
            static constexpr uint32_t PATCH_PENDING_TIER_1 = 3;
            static constexpr uint32_t PATCH_PENDING_TIER_2 = 8;
            static constexpr uint32_t PATCH_PENDING_TIER_3 = 20;
            static constexpr uint32_t PATCH_PENDING_TIER_4 = 40;
            static constexpr uint32_t PATCH_BUDGET_MOVE_THRESHOLD = 4;

            uint32_t count_pending_patches() const {
                uint32_t n = 0;
                for (uint32_t i = 0; i < world_state_.active_patch_count; i++) {
                    if (!patches_[i].valid) continue;
                    if (patches_[i].phase == PatchPhase::SPAWNED ||
                        patches_[i].phase == PatchPhase::NEEDS_REGEN) n++;
                }
                return n;
            }

            uint32_t patches_budget_this_frame() const {
                uint32_t pending = count_pending_patches();
                uint32_t budget = PATCH_BUDGET_MIN;
                if (pending >= PATCH_PENDING_TIER_4) budget = 6;
                else if (pending >= PATCH_PENDING_TIER_3) budget = 4;
                else if (pending >= PATCH_PENDING_TIER_2) budget = 3;
                else if (pending >= PATCH_PENDING_TIER_1) budget = 2;

                bool moving = (std::abs(inputState_.move_x) > 0.01f ||
                    std::abs(inputState_.move_z) > 0.01f);
                if (moving && pending > PATCH_BUDGET_MOVE_THRESHOLD)
                    budget += 1;

                return std::min(budget, PATCH_BUDGET_MAX);
            }

            // ═══ FAMILY DISPATCH TABLE ═══════════════════════════════════
            //
            // SEAM[spine:owns] FAMILY_DISPATCH is the integration hub that
            //   ties the 12 families together. Each row's body lives in
            //   the family's owning module.
            // SEAM[spine:K2-related] the six real dispatch_prepare_mesh_* /
            //   dispatch_mesh_gen_* adapter pairs below are integration glue
            //   between FAMILY_DISPATCH and the per-family modules (their
            //   signatures adapt module preparers and renderer dispatches to
            //   the row slots). The bespoke select/place/commit funnels AND
            //   the twelve evictors live with their owners (§5 EVICTION
            //   THUNKS: retirement fulfilled); the no-op mesh adapters are
            //   shared (family_dispatch.inl).
            // SEAM[spine:family-dispatch] anchor for cross-file references —
            //   eviction routes through FAMILY_DISPATCH[f].evict_slot to the
            //   owner-side evict_<family> functions.
            //
            // The row type (struct FamilyDispatch) and the queue-entry
            // unions it walks (EntityQueueEntry / PlacementEntry) live in
            // entity_types.hpp — the contract home.

            // ═══ DISPATCH WRAPPERS ═══════════════════════════════════════

            // ── Mesh gen wrappers ──

            static bool dispatch_prepare_mesh_pyramid(Cartridge* self, wgpu::Queue& queue) {
                return prepare_pyramid_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_pyramid(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_pyramid_mesh_gen(pass, self->gpuState_.pyramid_mesh_gen_group());
            }

            static bool dispatch_prepare_mesh_arch(Cartridge* self, wgpu::Queue& queue) {
                return prepare_arch_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_arch(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_arch_mesh_gen(pass, self->gpuState_.arch_mesh_gen_group());
            }

            static bool dispatch_prepare_mesh_column(Cartridge* self, wgpu::Queue& queue) {
                return prepare_column_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_column(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_column_mesh_gen(pass, self->gpuState_.column_mesh_gen_group());
            }

            // ── Mesh gen dispatch wrappers (palm) ──

            static bool dispatch_prepare_mesh_palm(Cartridge* self, wgpu::Queue& queue) {
                return prepare_palm_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_palm(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_palm_mesh_gen(pass, self->gpuState_.palm_mesh_gen_group());
            }

            // ── Mesh gen dispatch wrappers (cactus) ──

            static bool dispatch_prepare_mesh_cactus(Cartridge* self, wgpu::Queue& queue) {
                return prepare_cactus_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_cactus(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_cactus_mesh_gen(pass, self->gpuState_.cactus_mesh_gen_group());
            }

            static bool dispatch_prepare_mesh_blade(Cartridge* self, wgpu::Queue& queue) {
                return prepare_blade_mesh_gen(self->entities_state_, self, queue);
            }
            static void dispatch_mesh_gen_blade(Cartridge* self, wgpu::ComputePassEncoder& pass) {
                self->renderer_.dispatch_blade_mesh_gen(pass, self->gpuState_.blade_mesh_gen_group());
            }

            // ── The dispatch table (FAMILY_DISPATCH) is defined at file

            // ── Generic Entity Pipeline (modules/entity_pipeline.inl) ──
#include "modules/entity_pipeline.inl"

            //
            // The spine-owned piece-enable manifest (struct Roster, the
            // ROSTER constant, the transitions=>portal edge, and the full doc
            // block — RIDER A / MATURITY DIAL / FOUNDATIONAL / LATENT /
            // gate-(a) status column) now lives in
            // cartridges/the_board/roster.hpp. It met its SECOND CONSUMER —
            // GPUState::init (state.hpp) gates creation on the feature bits —
            // so the reading publishes at the shared header (the standing
            // law). ROSTER is visible here by namespace lookup
            // (t7::the_board::ROSTER); every ROSTER-GATE / ROSTER-RESIDUE
            // consult below is unchanged.

            // ═══ WORLD LIFECYCLE ═════════════════════════════════════════

            void teardown_world(wgpu::Queue& queue) {
                // Patches + tile cache
                init_patch_system();
                world_state_.last_center_x = INT32_MAX;  // force full regen on next frame
                world_state_.last_center_z = INT32_MAX;

                // Terrain tokens
                for (uint32_t t = 0; t < MAX_TERRAIN_TOKENS; t++) {
                    tile_world_state_.terrainTokens_[t] = TerrainToken{};
                }

                entityQueue_.clear();
                placementResults_.clear();

                // Theme envelope
                themes_state_ = ThemesState{};

                // Clear all entity piers (keep test rig at slots 0-2)
                for (uint32_t i = Dim::PIER_ARCH_BASE; i < Dim::PIER_TOTAL; i++) {
                    clear_pier(queue, i);
                }

                // Arches
                for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
                    entities_state_.arches[i] = ActiveArch{};
                }
                entities_state_.arch_count = 0;
                mood_state_.portals_dirty = true;
                gpuState_.set_arch_index_count(0);
                // Clear all arch mesh gen param slots
                {
                    GPUArchMeshParams emptyParams{};
                    for (uint32_t i = 0; i < Dim::MAX_ARCH_INSTANCES; i++) {
                        gpuState_.upload_arch_mesh_params_slot(queue, i, emptyParams);
                    }
                    entities_state_.arch_mesh_gen_pending = true;
                }

                // Columns + Antennas
                for (uint32_t i = 0; i < Dim::MAX_COLUMN_ONLY; i++) {
                    entities_state_.columns[i] = ActiveColumn{};
                }
                for (uint32_t i = 0; i < Dim::MAX_ANTENNA_ONLY; i++) {
                    entities_state_.antennas[i] = ActiveColumn{};
                }
                entities_state_.column_count = 0;
                entities_state_.antenna_count = 0;
                gpuState_.set_column_index_count(0);
                // Clear all column mesh gen param slots
                {
                    GPUColumnMeshParams emptyParams{};
                    for (uint32_t i = 0; i < Dim::MAX_COLUMN_INSTANCES; i++) {
                        gpuState_.upload_column_mesh_params_slot(queue, i, emptyParams);
                    }
                    entities_state_.column_mesh_gen_pending = true;
                }

                // Palms
                for (uint32_t i = 0; i < Dim::MAX_PALM_INSTANCES; i++) {
                    entities_state_.palms[i] = ActivePalm{};
                }
                entities_state_.palm_count = 0;
                gpuState_.set_palm_index_count(0);
                {
                    GPUPalmMeshParams emptyParams{};
                    for (uint32_t i = 0; i < Dim::MAX_PALM_INSTANCES; i++) {
                        gpuState_.upload_palm_mesh_params_slot(queue, i, emptyParams);
                    }
                    entities_state_.palm_mesh_gen_pending = true;
                }

                // Cacti
                for (uint32_t i = 0; i < Dim::MAX_CACTUS_INSTANCES; i++) {
                    entities_state_.cacti[i] = ActiveCactus{};
                }
                entities_state_.cactus_count = 0;
                gpuState_.set_cactus_index_count(0);
                {
                    GPUCactusMeshParams emptyParams{};
                    for (uint32_t i = 0; i < Dim::MAX_CACTUS_INSTANCES; i++) {
                        gpuState_.upload_cactus_mesh_params_slot(queue, i, emptyParams);
                    }
                    entities_state_.cactus_mesh_gen_pending = true;
                }

                // Blade clusters
                for (uint32_t i = 0; i < Dim::MAX_BLADE_INSTANCES; i++) {
                    entities_state_.blades[i] = ActiveBlade{};
                }
                entities_state_.blade_count = 0;
                gpuState_.set_blade_index_count(0);
                {
                    GPUBladeClusterMeshParams emptyParams{};
                    for (uint32_t i = 0; i < Dim::MAX_BLADE_INSTANCES; i++) {
                        gpuState_.upload_blade_mesh_params_slot(queue, i, emptyParams);
                    }
                    entities_state_.blade_mesh_gen_pending = true;
                }

                // Pyramids
                for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) {
                    entities_state_.pyramids[i] = ActivePyramid{};
                }
                entities_state_.pyramid_count = 0;
                entities_state_.cpu_pyramids = GPUPyramidArray{};
                gpuState_.upload_pyramids(queue, entities_state_.cpu_pyramids);
                gpuState_.set_pyramid_index_count(0);
                // Clear all mesh gen param slots (inactive → degenerates on next dispatch)
                {
                    GPUPyramidMeshParams emptyParams{};
                    for (uint32_t i = 0; i < Dim::MAX_PYRAMID_INSTANCES; i++) {
                        gpuState_.upload_pyramid_mesh_params_slot(queue, i, emptyParams);
                    }
                    entities_state_.pyramid_mesh_gen_pending = true;
                }

                // GoL zones
                for (uint32_t i = 0; i < Dim::MAX_GOL_ZONES; i++) {
                    gol_state_.zones[i] = GoLZoneState{};
                }
                gol_state_.zone_count = 0;
                gol_state_.active_slot_count = 0;
                gol_state_.pending_derive_requests.count = 0;
                GPUGoLZoneArray emptyZones{};
                gpuState_.upload_zone_config(queue, emptyZones);

                // Ribbon — clear all slots
                {
                    for (uint32_t i = 0; i < MAX_RIBBON_INSTANCES; i++) {
                        ribbon_state_.active[i] = ActiveRibbon{};
                        ribbon_state_.gpu[i] = GPURibbonState{};
                    }
                    ribbon_state_.active_count = 0;
                    ribbon_state_.rendered_slot = UINT32_MAX;
                    GPURibbonState empty{};
                    gpuState_.upload_ribbon(queue, empty);
                }

                // Sphere + cube clears are per-owner functions (clear_spheres /
                // clear_cubes) — CPU + per-slot-GPU paired. See §5 TEARDOWN BULK SWEEPS.
                clear_spheres(sphere_state_, gpuState_, queue);
                clear_cubes(cube_behaviors_state_, gpuState_, queue);

                // Gallery / paintings — clear all exhibition + slots, keep staging intact
                for (uint32_t i = 0; i < MAX_GALLERIES; i++) {
                    gallery_state_.gallery_centers[i] = GalleryCenter{};
                }
                gallery_state_.pending_snapshot.active = false;
                gallery_state_.pending_promotion_count = 0;
                gallery_state_.wall_frame_count = 0;
                gallery_state_.active_painting_count = 0;
                // Clear all painting slots (CPU + GPU)
                for (uint32_t i = 0; i < Dim::PAINTING_MAX_SLOTS; i++) {
                    gallery_state_.painting_slots[i] = GPUPaintingSlot{};
                }
                {
                    GPUPaintingSlot empty[Dim::PAINTING_MAX_SLOTS]{};
                    gpuState_.upload_painting_slots(queue, empty, Dim::PAINTING_MAX_SLOTS);
                }
                // Free all exhibition layers (staging persists across worlds)
                for (uint32_t i = 0; i < Dim::EXHIBITION_LAYERS; i++) gallery_state_.exhibition_occupied[i] = false;
                gallery_state_.exhibition_count = 0;
                rotate_authored_staging(gallery_state_, this, queue);
                for (uint32_t i = 0; i < Dim::STAGING_LAYERS; i++) gallery_state_.authored_staging[i].consumed = false;

                // Footprints
                for (uint32_t i = 0; i < MAX_FOOTPRINTS; i++) {
                    footprints_[i] = GroundFootprint{};
                }

                // Aura
                if constexpr (ROSTER.pawn_aura) {  // ROSTER-GATE pawn_aura (c) — teardown clear skipped when disabled (no aura to clear)
                    pawn_state_.aura_needs_clear = true;
                    pawn_state_.aura_cfg_dirty = true;
                }

                // Sky orbs: apply_mood re-enables + re-seeds as needed
                if constexpr (ROSTER.orbs)  // ROSTER-GATE orbs (c) — teardown one-shot skipped when disabled
                    teardown_orbs(orbs_state_, this);

                // Indoor shell
                gpuState_.set_shell_index_count(0);

                // Lights need re-upload with potentially new config
                entities_state_.lights_dirty = true;

                // New world decides its own upload frequency policy
                gpuState_.set_config_dynamic(false);
            }

            // ═══ PATCH SUBSYSTEM SETUP ═══════════════════════════════════

            void init_patch_system() {
                for (uint32_t i = 0; i < MAX_PATCHES; i++) {
                    freeLayerStack_[i] = MAX_PATCHES - 1 - i;
                }
                world_state_.free_layer_count = MAX_PATCHES;
                world_state_.active_patch_count = 0;
                world_state_.render_patch_count = 0;
                world_state_.lod0_patch_count = 0;
                world_state_.all_patch_count = 0;
                gpuState_.config().placement_patch_count = 0;
                tile_world_state_.tileCache_.clear();
                world_state_.pier_count_dirty = true;
                world_state_.ground_entries_dirty = true;
                world_state_.patch_instances_dirty = true;
                world_state_.placement_dirty = true;
            }

            // Test rig piers: ramp + plateau + block at pier slots 0-2.
            // Same geometry as the old test rig solids, now as GPUPierInstance.
            // TESTING[test-rig-piers] (ROSTER-1a §1 ruling): a debug ground
            //   fixture, NOT a roster piece (roster rows are design pieces,
            //   not scaffolds). Mortal retirement: dies at ship (checklist).
            //   Joins the future exhibition-guard discussion alongside
            //   SEAM[spawn_engine:L1]'s DIAG_ENTITY_LIFECYCLE. Constitution §5
            //   TESTING class.
            void setup_test_rig_piers(wgpu::Queue queue) {
                // Ramp: height 0→3 along +X.
                GPUPierInstance ramp{};
                ramp.origin[0] = 12.0f;  ramp.origin[1] = 0.0f;
                ramp.half_size[0] = 6.5f; ramp.half_size[1] = 3.0f;
                ramp.height_near = 0.0f;  ramp.height_far = 3.0f;
                ramp.rotation = 0.0f;
                ramp.edge_blend = 0.5f;
                ramp.tier = PierTier::TEST_RIG;
                ramp.is_active = 1;
                write_pier(queue, 0, ramp);

                // Plateau: flat at height 3, overlaps ramp at x=18.
                GPUPierInstance plat{};
                plat.origin[0] = 21.0f;  plat.origin[1] = 0.0f;
                plat.half_size[0] = 3.5f; plat.half_size[1] = 3.0f;
                plat.height_near = 3.0f;  plat.height_far = 3.0f;
                plat.rotation = 0.0f;
                plat.edge_blend = 0.5f;
                plat.tier = PierTier::TEST_RIG;
                plat.is_active = 1;
                write_pier(queue, 1, plat);

                // Block: sharp edges → step-height walls (impassable).
                GPUPierInstance block{};
                block.origin[0] = 21.0f;  block.origin[1] = 0.0f;
                block.half_size[0] = 1.2f; block.half_size[1] = 1.2f;
                block.height_near = 5.0f;  block.height_far = 5.0f;
                block.rotation = 0.0f;
                block.edge_blend = 0.0f;
                block.tier = PierTier::TEST_RIG;
                block.is_active = 1;
                write_pier(queue, 2, block);
            }

            // ═══ PATCH GENERATION ════════════════════════════════════════
            //
            void generate_patch_batch(wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
                const GPUPatchParams* params, uint32_t count,
                uint32_t stagingOffset = 0) {
                if (count == 0) return;

                // One WriteBuffer: all params into staging at the given offset
                gpuState_.upload_patch_staging(queue, params, count, stagingOffset);

                for (uint32_t i = 0; i < count; i++) {
                    // Copy this patch's params from staging slot → active params buffer
                    encoder.CopyBufferToBuffer(
                        gpuState_.patch_staging_buffer(), (stagingOffset + i) * sizeof(GPUPatchParams),
                        gpuState_.patch_params_buffer(), 0,
                        sizeof(GPUPatchParams));

                    // Pass 1: heights only (one ground_formed_with_complexity per texel)
                    {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "Patch Heights (pass 1)";
                        wgpu::ComputePassEncoder cp = encoder.BeginComputePass(&cpd);
                        renderer_.dispatch_generate_patch_heights(cp, gpuState_.patch_gen_group(), GPUState::patch_heightfield_workgroups());
                        cp.End();
                    }

                    // Pass 2: gradients from neighbor reads + complexity + cell colors
                    {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "Patch Gradients + Cells (pass 2)";
                        wgpu::ComputePassEncoder cp = encoder.BeginComputePass(&cpd);
                        renderer_.dispatch_generate_patch_gradients(cp, gpuState_.patch_gen_group(), GPUState::patch_heightfield_workgroups());
                        renderer_.dispatch_generate_patch_cells(cp, gpuState_.patch_gen_group(), GPUState::patch_cell_workgroups());
                        cp.End();
                    }
                }
            }

            GPUPatchParams make_patch_params(int32_t gx, int32_t gz, uint32_t layer) const {
                GPUPatchParams p{};
                p.origin[0] = (gx + 0.5f) * PATCH_EXTENT;
                p.origin[1] = (gz + 0.5f) * PATCH_EXTENT;
                p.extent = PATCH_EXTENT;
                p.resolution = 256;
                p.master_seed = world_state_.active_seed;
                p.time = 0.0f;
                p.layer = layer;
                p._pad1 = 0.0f;
                return p;
            }

            // ═══ LAYER ALLOCATOR ═════════════════════════════════════════

            uint32_t alloc_layer() {
                if (world_state_.free_layer_count == 0) {
                    // Safety: no free layers — recycle layer 0 rather than crash.
                    // This shouldn't happen if eviction works correctly.
                    return 0;
                }
                return freeLayerStack_[--world_state_.free_layer_count];
            }

            void free_layer(uint32_t layer) {
                freeLayerStack_[world_state_.free_layer_count++] = layer;
            }

            // Check if grid coordinate is within the allocation window (world_state_.active_radius = PREGEN_RADIUS)
            bool in_render_window(int32_t gx, int32_t gz, int32_t cx, int32_t cz) {
                int32_t r = (int32_t)world_state_.active_radius;
                return gx >= cx - r && gx <= cx + r &&
                    gz >= cz - r && gz <= cz + r;
            }

            static constexpr float VISIBLE_RADIUS = 5.5f;
            static constexpr float VISIBLE_RADIUS_SQ = VISIBLE_RADIUS * VISIBLE_RADIUS;

            static constexpr float LOD_FULL_RADIUS = 3.5f;
            static constexpr float LOD_FULL_RADIUS_SQ = LOD_FULL_RADIUS * LOD_FULL_RADIUS;

            // ═══ VISIBILITY CYLINDER ═════════════════════════════════════
            //
            // Grid-based allocation/eviction is unchanged; only the
            // draw-list gate uses world-space distance.
            static constexpr float VISIBILITY_CYLINDER_RADIUS = VISIBLE_RADIUS * PATCH_EXTENT;
            static constexpr float VISIBILITY_CYLINDER_RADIUS_SQ = VISIBILITY_CYLINDER_RADIUS * VISIBILITY_CYLINDER_RADIUS;
            static constexpr float LOD0_CYLINDER_RADIUS = LOD_FULL_RADIUS * PATCH_EXTENT;
            static constexpr float LOD0_CYLINDER_RADIUS_SQ = LOD0_CYLINDER_RADIUS * LOD0_CYLINDER_RADIUS;

            // Distance² from point (px,pz) to nearest edge of a patch AABB.
            // Zero when the point is inside the patch.
            static float patch_distance_sq(float px, float pz,
                float origin_x, float origin_z, float half) {
                float dx = std::max(0.0f, std::abs(px - origin_x) - half);
                float dz = std::max(0.0f, std::abs(pz - origin_z) - half);
                return dx * dx + dz * dz;
            }

            // ═══ PATCH STREAMING HELPERS ═════════════════════════════════

            // ── Distance-sorted patch scan helper ──

            struct PatchCandidate {
                uint32_t idx;
                float dist2;
            };

            template<typename Pred>
            uint32_t collect_sorted_patches(
                PatchCandidate* out,
                float pawn_wx, float pawn_wz,
                Pred&& pred,
                bool nearest_first) const
            {
                float half = PATCH_EXTENT * 0.5f;
                uint32_t count = 0;
                for (uint32_t i = 0; i < world_state_.active_patch_count; i++) {
                    if (!patches_[i].valid) continue;
                    if (!pred(patches_[i])) continue;
                    float ox = (patches_[i].grid_x + 0.5f) * PATCH_EXTENT;
                    float oz = (patches_[i].grid_z + 0.5f) * PATCH_EXTENT;
                    float d2 = patch_distance_sq(pawn_wx, pawn_wz, ox, oz, half);
                    out[count++] = { i, d2 };
                }
                for (uint32_t i = 1; i < count; i++) {
                    PatchCandidate key = out[i];
                    uint32_t j = i;
                    if (nearest_first) {
                        while (j > 0 && out[j - 1].dist2 > key.dist2) {
                            out[j] = out[j - 1]; j--;
                        }
                    }
                    else {
                        while (j > 0 && out[j - 1].dist2 < key.dist2) {
                            out[j] = out[j - 1]; j--;
                        }
                    }
                    out[j] = key;
                }
                return count;
            }

            // Check if grid coordinate is within the priority window (GRID_RADIUS)
            bool in_priority_window(int32_t gx, int32_t gz, int32_t cx, int32_t cz) {
                int32_t r = (int32_t)GRID_RADIUS;
                return gx >= cx - r && gx <= cx + r &&
                    gz >= cz - r && gz <= cz + r;
            }

            // Process entity spawn for pre-collected patch candidates.
            void spawn_selected_patches(
                const PatchCandidate* candidates, uint32_t count,
                wgpu::Queue& queue)
            {
                for (uint32_t s = 0; s < count; s++) {
                    uint32_t pi = candidates[s].idx;
                    themes_state_.active_theme_idx_ = evaluate_theme_envelope(themes_state_, this, 
                        tile_seed(world_state_.active_seed, patches_[pi].grid_x, patches_[pi].grid_z));
                    select_entities_for_patch(patches_[pi].grid_x, patches_[pi].grid_z);
                    patches_[pi].phase = PatchPhase::SPAWNED;
                }
                place_entity_queue();
                commit_entity_queue(queue);

                for (uint32_t s = 0; s < count; s++) {
                    uint32_t pi = candidates[s].idx;
                    int32_t gx = patches_[pi].grid_x;
                    int32_t gz = patches_[pi].grid_z;
                    for (uint32_t r = 0; r < MAX_RIBBON_INSTANCES; r++) {
                        auto& ar = ribbon_state_.active[r];
                        if (!ar.active) continue;
                        // Check near tip
                        if (!ar.near_tip_registered &&
                            ar.near_tip_gx == gx && ar.near_tip_gz == gz) {
                            patches_[pi].record_entity(PopFamily::RIBBON, r);
                            ar.near_tip_registered = true;
                            ar.ref_count++;
                        }
                        // Check far tip
                        if (!ar.far_tip_registered &&
                            ar.far_tip_gx == gx && ar.far_tip_gz == gz) {
                            patches_[pi].record_entity(PopFamily::RIBBON, r);
                            ar.far_tip_registered = true;
                            ar.ref_count++;
                        }
                    }
                }
            }

            // Hook: fires once when a patch transitions SPAWNED → GENERATED.
            void on_patch_first_generated(uint32_t pi, wgpu::Queue& queue) {
                // Galleries → entity pipeline (select_gallery_for_patch)
                // GoL zones → entity pipeline (select_gol_for_patch)
                (void)pi; (void)queue;
            }

            // Process heightfield generation for pre-collected patch candidates.
            void generate_selected_patches(
                const PatchCandidate* candidates, uint32_t count,
                wgpu::CommandEncoder& encoder, wgpu::Queue& queue,
                uint32_t& patchStagingOffset, bool& tileGridDirty)
            {
                if (count == 0) return;
                if (tileGridDirty) {
                    upload_tile_grid_now(tile_world_state_, this, queue, world_state_.last_center_x, world_state_.last_center_z);
                    tileGridDirty = false;
                }
                GPUPatchParams batchParams[MAX_PATCHES];
                uint32_t batchIdx[MAX_PATCHES];
                for (uint32_t i = 0; i < count; i++) {
                    uint32_t pi = candidates[i].idx;
                    batchParams[i] = make_patch_params(
                        patches_[pi].grid_x, patches_[pi].grid_z, patches_[pi].layer);
                    batchIdx[i] = pi;
                }
                generate_patch_batch(encoder, queue, batchParams, count, patchStagingOffset);
                patchStagingOffset += count;
                for (uint32_t b = 0; b < count; b++) {
                    uint32_t pi = batchIdx[b];
                    bool first_gen = (patches_[pi].phase == PatchPhase::SPAWNED);
                    patches_[pi].phase = PatchPhase::GENERATED;
                    if (first_gen) {
                        on_patch_first_generated(pi, queue);
                    }
                }
                world_state_.patch_instances_dirty = true;
            }

        public:

            // ═══ PUBLIC: CARTRIDGE LIFECYCLE ═════════════════════════════

            Cartridge() = default;

            Cartridge(const Cartridge&) = delete;
            Cartridge& operator=(const Cartridge&) = delete;

            void initialize(wgpu::Device device) override {
                device_ = device;
                auto tGpu0 = std::chrono::high_resolution_clock::now();
                gpuState_.init(device);
                auto tGpu1 = std::chrono::high_resolution_clock::now();
                std::cout << "[Cartridge] GPUState init:    "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(tGpu1 - tGpu0).count()
                    << " ms\n";

                {
                    float inactive[6] = { -1.f, -1.f, -1.f, -1.f, -1.f, -1.f };
                    float zeros6[6] = {};
                    gpuState_.set_band_motion(inactive, zeros6);
                    gpuState_.set_terrain_time(0.0f);
                    gpuState_.set_mode_color_shift(0.0f);
                    gpuState_.set_mode_checker_scatter(0.0f);
                    gpuState_.set_mode_palette_drift(0.0f, 0.0f, 0.0f);
                    gpuState_.set_mode_gol_scales(1.0f, 1.0f);
                    float zero_pulses[32] = {};
                    gpuState_.set_pulse_data(0, zero_pulses);
                }
            }

            bool init_renderer(
                wgpu::TextureFormat colorFormat,
                wgpu::TextureFormat depthFormat
            ) {
                colorFormat_ = colorFormat;
                depthFormat_ = depthFormat;

                auto t0 = std::chrono::high_resolution_clock::now();
                if (!renderer_.init(
                    device_,
                    gpuState_,
                    colorFormat,
                    depthFormat
                )) return false;

                // Create offscreen textures with the actual swapchain format
                if (!gpuState_.initOffscreenResources(colorFormat)) {
                    std::cerr << "[Cartridge] Failed to init offscreen resources\n";
                    return false;
                }

                auto t1 = std::chrono::high_resolution_clock::now();

                // --- One-shot: generate terrain index buffer on GPU -----------------
                {
                    wgpu::CommandEncoder encoder = device_.CreateCommandEncoder();
                    wgpu::ComputePassDescriptor desc{};
                    desc.label = "Terrain Index Gen (one-shot)";
                    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&desc);
                    renderer_.dispatch_generate_terrain_indices(
                        pass,
                        gpuState_.terrain_index_gen_group(),
                        GPUState::terrain_mesh_workgroups()
                    );
                    pass.End();
                    wgpu::CommandBuffer cmd = encoder.Finish();
                    device_.GetQueue().Submit(1, &cmd);
                }
                auto t2 = std::chrono::high_resolution_clock::now();

                init_patch_system();
                setup_test_rig_piers(device_.GetQueue());

                // Sky orbs for the initial mood (apply_mood runs only on transitions).
                if constexpr (ROSTER.orbs) {  // ROSTER-GATE orbs (c) — boot one-shot skipped when disabled
                    wgpu::Queue q = device_.GetQueue();
                    configure_orbs(orbs_state_, this, ORB_MOOD_TABLE[mood_state_.active], q);
                }

                // Agent registries — single source of truth in modules/agents.inl
                // (AGENT_BEHAVIORS / AGENT_TIER_GAINS), uploaded once to GPU
                // storage buffers at bindings 110 + 111. Values are
                // constexpr-equivalent and never change during a session,
                // so this is a one-shot write at boot.
                {
                    wgpu::Queue q = device_.GetQueue();
                    upload_agent_registries_to_gpu(this, q);
                }

                {
                    agent_state_.slots[0].pos_x = Idle::PAWN_POS_X;
                    agent_state_.slots[0].pos_y = Idle::PAWN_POS_Y;
                    agent_state_.slots[0].pos_z = Idle::PAWN_POS_Z;
                    agent_state_.slots[0].heading = Idle::PAWN_HEADING;
                    agent_state_.slots[0].orient_w = 1.0f;
                    agent_state_.slots[0].is_active = 1u;
                    agent_state_.slots[0].behavior_id = AGENT_BEHAVIOR_PLAYER_CONTROLLED;
                    agent_state_.slots[0].tier_idx = AGENT_TIER_WORKER;
                    agent_state_.slots[0].portal_trigger = -1;

                    wgpu::Queue q = device_.GetQueue();
                    // ROSTER-GATE wanderers (c) — boot population (agent slots
                    // 1+). Slot 0 (the pawn, seeded just above) is untouched.
                    if constexpr (ROSTER.wanderers)
                        spawn_population_for_mood(agent_state_, this, mood_state_.active, world_state_.active_seed,
                            Idle::PAWN_POS_X, Idle::PAWN_POS_Z, q);
                    dump_agent_census(agent_state_, this, "boot");
                }

                // Eager-load authored paintings at boot (avoids mid-frame stall on first gallery)
                {
                    wgpu::Queue q = device_.GetQueue();
                    load_authored_textures(gallery_state_, this, q);
                }

                auto t3 = std::chrono::high_resolution_clock::now();

                std::cout << "[Cartridge] Renderer init:    "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << " ms\n";
                std::cout << "[Cartridge] Terrain gen:      "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << " ms\n";
                std::cout << "[Cartridge] Patch system:     "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t2).count() << " ms\n";
                std::cout << "[Cartridge] Total init:       "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t0).count() << " ms\n";

                if constexpr (!ROSTER.all_enabled()) {
                    std::string off;
                    auto mark = [&](bool enabled, const char* name) {
                        if (!enabled) { if (!off.empty()) off += ", "; off += name; }
                    };
                    mark(ROSTER.pyramid, "pyramid");     mark(ROSTER.arch, "arch");
                    mark(ROSTER.column, "column");       mark(ROSTER.antenna, "antenna");
                    mark(ROSTER.palm, "palm");           mark(ROSTER.cactus, "cactus");
                    mark(ROSTER.blade, "blade");         mark(ROSTER.sphere, "sphere");
                    mark(ROSTER.ribbon, "ribbon");       mark(ROSTER.cube, "cube");
                    mark(ROSTER.gol, "gol");             mark(ROSTER.gallery, "gallery");
                    mark(ROSTER.pawn_aura, "pawn_aura"); mark(ROSTER.orbs, "orbs");
                    mark(ROSTER.spot_lights, "spot_lights");
                    mark(ROSTER.indoor_shell, "indoor_shell");
                    mark(ROSTER.portal, "portal");       mark(ROSTER.transitions, "transitions");
                    mark(ROSTER.wanderers, "wanderers");
                    // Only indoor_shell (SEP) skips creation in v0.
                    const char* skipped = ROSTER.indoor_shell
                        ? "(none — every disabled piece is SH-shared, created-pristine)"
                        : "indoor_shell (shell VB/IB + shell/shadow pipelines)";
                    std::cout << "[ROSTER] pieces disabled: " << off
                        << " | creations skipped: " << skipped << "\n";
                }

                return true;
            }

            void bind_signal_layout(StatLayoutView v) {
                visual_canvas_.bind(v);
                fog_density_dst_ = visual_canvas_.layout().resolve("fog.density");
                fog_color_dst_   = visual_canvas_.layout().resolve("fog.color");
                ribbon_amp_lat_dst_  = visual_canvas_.layout().resolve("ribbon.amp_lateral_mult");
                ribbon_amp_vert_dst_ = visual_canvas_.layout().resolve("ribbon.amp_vertical_mult");
                ribbon_tint_stim_dst_ = visual_canvas_.layout().resolve("ribbon.color_stim");
                ribbon_tint_mix_dst_  = visual_canvas_.layout().resolve("ribbon.color_mix");
                std::fprintf(stderr,
                    "[the_board] fog.density base=%d valid=%d | fog.color base=%d count=%d valid=%d\n",
                    fog_density_dst_.base, (int)fog_density_dst_.valid,
                    fog_color_dst_.base, fog_color_dst_.count, (int)fog_color_dst_.valid);
            }

            void update(const AnalysisSignal& signal,
                float aspect_ratio,
                wgpu::Queue& queue) override {
                // --- Build GPU signal from analysis + input -------------------------
                GPUFrameSignal gpuSignal;

                gpuSignal.t_seconds = signal.t_seconds;
                gpuSignal.t_beats = signal.t_beats;
                gpuSignal.dt = signal.dt;
                gpuSignal.aspect_ratio = aspect_ratio;

                for (size_t i = 0; i < gpuSignal.stats.size(); ++i) {
                    gpuSignal.stats[i] = signal.stats[i];
                }

                gpuSignal.move_x = inputState_.move_x;
                gpuSignal.move_z = inputState_.move_z;
                gpuSignal.look_az_delta = inputState_.look_az_delta;
                gpuSignal.look_el_delta = inputState_.look_el_delta;
                gpuSignal.zoom_delta = inputState_.zoom_delta;
                gpuSignal.pan_x_delta = inputState_.pan_x_delta;
                gpuSignal.pan_y_delta = inputState_.pan_y_delta;
                gpuSignal.dt_beats = signal.t_beats - time_state_.prev_beats;  // beats since last frame -> step_trigger

                // Sky mode (SNAP-1 / M5 same-frame coherence): the pose/frame
                // words here are NEUTRAL placeholders. The authoritative
                // author is resync_sky_head, ordered AFTER ribbon_frame_tick
                // and BEFORE dispatch_compute — queue writes apply in
                // submission order, so the kernel always reads THIS frame's
                // advance, the same advance the ring transforms render.
                // Filling from ribbon_head_pose here would carry the PREVIOUS
                // frame's pose (the M5 skew); zeros instead make any future
                // loss of the resync fail LOUD (pawn to origin) rather than
                // silently one frame late. SEAM[ribbon:sky-mode].
                {
                    gpuSignal.sky_mode    = player_.sky_mode ? 1u : 0u;
                    gpuSignal.sky_head_x  = 0.0f;
                    gpuSignal.sky_head_y  = 0.0f;
                    gpuSignal.sky_head_z  = 0.0f;
                    gpuSignal.sky_heading = 0.0f;
                    gpuSignal.sky_yaw_off = 0.0f;
                    gpuSignal.sky_pitch   = 0.0f;
                    gpuSignal.sky_roll    = 0.0f;
                }

                time_state_.beats = signal.t_beats;
                time_state_.seconds = signal.t_seconds;
                time_state_.dt = signal.dt;
                {
                    const float db = signal.t_beats - time_state_.prev_beats;
                    if (db > 1e-6f && time_state_.dt > 1e-6f)
                        time_state_.beat_rate = db / time_state_.dt;
                    time_state_.prev_beats = signal.t_beats;
                }

                visual_canvas_.tick(signal);
                if (fog_density_dst_.valid && fog_color_dst_.valid) {
                    const VisualParams& fp = visual_canvas_.params();
                    gpuState_.set_fog(fp.get(fog_density_dst_.base),
                                      fp.get(fog_color_dst_.base + 0),
                                      fp.get(fog_color_dst_.base + 1),
                                      fp.get(fog_color_dst_.base + 2));
                }

                // Pawn presence ramp + aura height computation.
                // Lives in pawn.inl as a real-time exponential tick (closes pawn:K1).
                if constexpr (ROSTER.pawn_aura)  // ROSTER-GATE pawn_aura (b) — disabled: presence never raised, no aura height
                    tick_pawn_couplings(pawn_state_, this, queue);
                gpuState_.set_world_seed(world_state_.active_seed);
                if (world_state_.finite_mode) {
                    float bmin = -(float)world_state_.finite_radius * PATCH_EXTENT;
                    float bmax = ((float)world_state_.finite_radius + 1.0f) * PATCH_EXTENT;
                    gpuState_.set_world_bounds(bmin, bmin, bmax, bmax);
                }
                else {
                    gpuState_.set_world_bounds(0.0f, 0.0f, 0.0f, 0.0f);
                }

                // --- Transition state machine ---
                if (transitionPhase_ != TransitionPhase::IDLE) {
                    mood_state_.transition_timer += signal.dt;
                    switch (transitionPhase_) {
                    case TransitionPhase::FADE_OUT:
                        mood_state_.transition_fade_alpha = std::min(1.0f, mood_state_.transition_timer / mood_state_.transition_fade_duration);
                        if (mood_state_.transition_fade_alpha >= 1.0f) {
                            transitionPhase_ = TransitionPhase::TEARDOWN;
                        }
                        break;
                    case TransitionPhase::TEARDOWN:
                    {
                        // This TEARDOWN block stays focused on the
                        //   integration concerns it correctly owns: worldGen
                        //   bump (P5 stale-callback guard), return-state
                        //   capture, agent reset, ribbon cleanup, patch
                        //   teardown.
                        // SEAM[spine:P5] world_state_.world_gen++ at top of TEARDOWN is the
                        //   stale-callback guard (P5 family). Genuinely
                        //   spine-owned.

                        world_state_.world_gen++;

                        // Capture return seed + mood + radius before overwrite
                        mood_state_.back_portal_return_seed = world_state_.active_seed;
                        mood_state_.back_portal_return_mood = mood_state_.active;
                        mood_state_.back_portal_return_radius = world_state_.finite_radius;

                        world_state_.active_seed = pendingDestination_.seed;
                        world_state_.finite_mode = pendingDestination_.finite;
                        world_state_.finite_radius = pendingDestination_.finite_radius;
                        teardown_world(queue);
                        player_.readback_portal_trigger = -1;
                        player_.readback_x = 0.0f;
                        player_.readback_z = 0.0f;
                        uint32_t preserved_tier = agent_state_.slots[player_.possessed_slot].tier_idx;
                        float preserved_color_r = agent_state_.slots[player_.possessed_slot].color_r;
                        float preserved_color_g = agent_state_.slots[player_.possessed_slot].color_g;
                        float preserved_color_b = agent_state_.slots[player_.possessed_slot].color_b;

                        gpuState_.reset_player_agent(queue, preserved_tier,
                            preserved_color_r, preserved_color_g, preserved_color_b);
                        gpuState_.set_possessed_slot(0);
                        // Keep agent_state_.slots in sync with the GPU reset so
                        // patch streaming + ribbon + Caps Lock see current state.
                        std::memset(agent_state_.slots, 0, sizeof(agent_state_.slots));
                        agent_state_.slots[0].pos_x = 0.0f;  // Idle::PAWN_POS_X
                        agent_state_.slots[0].pos_y = 0.0f;
                        agent_state_.slots[0].pos_z = 0.0f;
                        agent_state_.slots[0].orient_w = 1.0f;
                        agent_state_.slots[0].is_active = 1u;
                        agent_state_.slots[0].behavior_id = AGENT_BEHAVIOR_PLAYER_CONTROLLED;
                        agent_state_.slots[0].tier_idx = preserved_tier;
                        agent_state_.slots[0].color_r = preserved_color_r;
                        agent_state_.slots[0].color_g = preserved_color_g;
                        agent_state_.slots[0].color_b = preserved_color_b;
                        agent_state_.slots[0].portal_trigger = -1;
                        player_.possessed_slot = 0;
                        gpuState_.set_world_seed(world_state_.active_seed);
                        apply_mood(this, pendingDestination_.mood, queue);
                        // ROSTER-GATE wanderers (c) — transition population (slots 1+); slot 0 preserved above.
                        if constexpr (ROSTER.wanderers)
                            spawn_population_for_mood(agent_state_, this, pendingDestination_.mood, world_state_.active_seed,
                                Idle::PAWN_POS_X, Idle::PAWN_POS_Z, queue);
                        dump_agent_census(agent_state_, this, "mood-transition");
                        // Deactivate ribbons in finite mode unless the mood
                        // spawns its own anchor ribbon in apply_mood.
                        if (world_state_.finite_mode && ribbon_state_.active_count > 0 && !MOOD_TABLE[mood_state_.active].has_anchor_ribbon) {
                            for (uint32_t i = 0; i < MAX_RIBBON_INSTANCES; i++) {
                                ribbon_state_.active[i] = ActiveRibbon{};
                                ribbon_state_.gpu[i] = GPURibbonState{};
                            }
                            ribbon_state_.active_count = 0;
                            ribbon_state_.rendered_slot = UINT32_MAX;
                            GPURibbonState empty{};
                            gpuState_.upload_ribbon(queue, empty);
                        }
                        // Schedule guaranteed back-portal in finite worlds
                        mood_state_.back_portal_pending = world_state_.finite_mode;

                        transitionPhase_ = TransitionPhase::FADE_IN;
                        mood_state_.transition_timer = 0.0f;
                        uint32_t side = world_state_.finite_mode ? 2 * world_state_.finite_radius + 1 : 0;
                        std::cout << "[World] Teardown complete, seed=" << world_state_.active_seed
                            << " mode=" << (world_state_.finite_mode ? "finite" : "open")
                            << (world_state_.finite_mode ? " " + std::to_string(side) + "x" + std::to_string(side) : "")
                            << "\n";
                    }
                    break;
                    case TransitionPhase::FADE_IN:
                        mood_state_.transition_fade_alpha = std::max(0.0f, 1.0f - mood_state_.transition_timer / mood_state_.transition_fade_duration);
                        if (mood_state_.transition_fade_alpha <= 0.0f) {
                            transitionPhase_ = TransitionPhase::IDLE;
                            mood_state_.transition_fade_alpha = 0.0f;
                        }
                        break;
                    default: break;
                    }
                }
                gpuState_.set_fade(mood_state_.transition_fade_alpha, 0.0f, 0.0f, 0.0f);

                gpuState_.upload_signal(queue, gpuSignal);

                gpuState_.upload_config(queue);

                // Orb dome anchor: follow pawn when toggled on. Uses
                // last-frame pawn readback — one-frame lag is imperceptible.
                if constexpr (ROSTER.orbs)  // ROSTER-GATE orbs (b)
                    update_orb_anchor(orbs_state_, this, player_.readback_x, player_.readback_z, queue);

                // --- Clear deltas for next frame ------------------------------------
                update_photographer(gallery_state_, this, queue);
                clear_input_deltas(this);
            }

            // SEAM[spine:owns] render() is genuinely spine work: readback state
            //   machines, stale-callback guards, portal trigger handling,
            //   patch streaming, photographer cadence. The K1 observation
            //   doesn't apply to render() the same way it applies to update();
            //   render() mixes orchestration (correct) with smaller per-module
            //   GPU upload calls (each lives in its module already).
            void render(wgpu::CommandEncoder& encoder,
                wgpu::TextureView backbuffer,
                wgpu::TextureView depth) override {

                wgpu::Queue queue = device_.GetQueue();

                //
                if (pawnReadbackState_ == PawnReadbackState::COPIED) {
                    pawnReadbackState_ = PawnReadbackState::MAPPING;
                    gpuState_.agent_state_readback_staging().MapAsync(
                        wgpu::MapMode::Read, 0, GPUState::agent_state_buffer_size(),
                        wgpu::CallbackMode::AllowSpontaneous,
                        [this, gen = world_state_.world_gen](wgpu::MapAsyncStatus status, wgpu::StringView) {
                            if (status == wgpu::MapAsyncStatus::Success) {
                                // Drop stale callbacks from a previous world: gen
                                // captured at issue time differs from current
                                // world_state_.world_gen if a teardown happened in between.
                                // Buffer is still successfully mapped though, so
                                // we Unmap unconditionally (mapping contract is
                                // independent of whether we read the data).
                                if (gen == world_state_.world_gen) {
                                    const auto* data = static_cast<const GPUAgentState*>(
                                        gpuState_.agent_state_readback_staging().GetConstMappedRange(
                                            0, GPUState::agent_state_buffer_size()));
                                    if (data) {
                                        std::memcpy(agent_state_.slots, data,
                                            GPUState::agent_state_buffer_size());
                                        const auto& p = agent_state_.slots[player_.possessed_slot];
                                        player_.readback_x = p.pos_x;
                                        player_.readback_z = p.pos_z;
                                        player_.readback_portal_trigger = p.portal_trigger;
                                    }
                                }
                                gpuState_.agent_state_readback_staging().Unmap();
                            }
                            pawnReadbackState_ = PawnReadbackState::IDLE;
                        });
                }

                //
                if (floaterReadbackState_ == FloaterReadbackState::COPIED) {
                    floaterReadbackState_ = FloaterReadbackState::MAPPING;
                    gpuState_.floating_entity_readback_staging().MapAsync(
                        wgpu::MapMode::Read, 0, GPUState::floating_entity_buffer_size(),
                        wgpu::CallbackMode::AllowSpontaneous,
                        [this, gen = world_state_.world_gen](wgpu::MapAsyncStatus status, wgpu::StringView) {
                            if (status == wgpu::MapAsyncStatus::Success) {
                                // Drop stale callbacks from a previous world.
                                // Buffer is still mapped, so Unmap unconditionally.
                                if (gen == world_state_.world_gen) {
                                    const auto* data = static_cast<const GPUFloatingEntityState*>(
                                        gpuState_.floating_entity_readback_staging().GetConstMappedRange(
                                            0, GPUState::floating_entity_buffer_size()));
                                    if (data) {
                                        static constexpr float SPAWN_PROTECTION_S = 0.10f;
                                        float now = time_state_.seconds;
                                        // Spheres: slots [0, MAX_SPHERE_INSTANCES)
                                        for (uint32_t i = 0; i < Dim::MAX_SPHERE_INSTANCES; i++) {
                                            bool gpu_active = (data[i].is_active != 0u);
                                            // sphere active-slot mirror owned by SphereState (spheres.hpp)
                                            if (sphere_state_.activeFloaters_[i].active && !gpu_active &&
                                                (now - sphere_state_.activeFloaters_[i].last_alloc_time) > SPAWN_PROTECTION_S) {
                                                sphere_state_.activeFloaters_[i].active = false;
                                                if (sphere_state_.activeFloaterCount_ > 0) sphere_state_.activeFloaterCount_--;
                                            }
                                        }
                                        // Cubes: slots [CUBE_SLOT_OFFSET, TOTAL_FLOATING_SLOTS)
                                        for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
                                            bool gpu_active = (data[Dim::CUBE_SLOT_OFFSET + i].is_active != 0u);
                                            // cube active-slot mirror owned by CubeBehaviorsState (cube_behaviors.hpp)
                                            if (cube_behaviors_state_.activeCubes_[i].active && !gpu_active &&
                                                (now - cube_behaviors_state_.activeCubes_[i].last_alloc_time) > SPAWN_PROTECTION_S) {
                                                cube_behaviors_state_.activeCubes_[i].active = false;
                                                if (cube_behaviors_state_.activeCubeCount_ > 0) cube_behaviors_state_.activeCubeCount_--;
                                            }
                                        }
                                    }
                                }
                                gpuState_.floating_entity_readback_staging().Unmap();
                            }
                            floaterReadbackState_ = FloaterReadbackState::IDLE;
                        });
                }

                // Check if GPU reported a portal trigger
                // ROSTER-GATE transitions (b) — ENTRY door #2 (portal trigger).
                // With transitions off + portal on (a legal config), portal
                // arches exist but stepping through must NOT start a transition.
                if constexpr (ROSTER.transitions)
                if (player_.readback_portal_trigger >= 0 && transitionPhase_ == TransitionPhase::IDLE) {
                    uint32_t arch_idx = static_cast<uint32_t>(player_.readback_portal_trigger);
                    player_.readback_portal_trigger = -1;
                    if (arch_idx < Dim::MAX_ARCH_INSTANCES &&
                        entities_state_.arches[arch_idx].active &&
                        entities_state_.arches[arch_idx].is_portal) {
                        pendingDestination_ = entities_state_.arches[arch_idx].destination;
                        transitionPhase_ = TransitionPhase::FADE_OUT;
                        mood_state_.transition_timer = 0.0f;
                        std::cout << "[Portal] GPU trigger: arch " << arch_idx
                            << " -> seed=" << pendingDestination_.seed
                            << " finite=" << pendingDestination_.finite << "\n";
                    }
                }

                // Refill any agent slots the GPU evicted last frame.
                // No-op when no slots were evicted — just a 32-slot scan.
                // ROSTER-GATE wanderers (b) — per-frame refill of evicted NPC slots (1+); slot 0 never evicted.
                if constexpr (ROSTER.wanderers)
                    respawn_evicted_agents(agent_state_, this, mood_state_.active, world_state_.active_seed, queue);

                tick_cube_corral_animations(cube_behaviors_state_, this, queue);

                stream_patches(encoder, queue);

                // DIAG-unwrapped (census: constitution §5): autonomous
                // stdout — wrap in #ifdef DIAG_AGENT_CENSUS at ship.
                // Periodic agent census dump — followed by the player's
                // last-known position from the GPU readback. The pos line
                // tells us at-a-glance whether the readback is current
                // (a stuck readback would freeze the position; an idle
                // player would do the same — pair the two by visiting
                // the world manually if you need to disambiguate).
                // ROSTER-RESIDUE gol (2e) — residue recipe. When gol is
                // disabled it is never selected (b), so zone_count stays 0 and
                // the sole writer of the zone GPU buffers (the compute block
                // above, guarded by zone_count>0) never runs. Prove it across
                // frames: report pristine, and fail LOUD if either invariant
                // is ever violated. Used at gate G3. Zero effect when enabled.
                if constexpr (!ROSTER.gol) {
                    if (time_state_.seconds - rosterGolResidueDump_ >= AGENT_CENSUS_INTERVAL) {
                        if (gol_state_.zone_count != 0 || rosterGolZoneRuns_ != 0) {
                            std::cerr << "[ROSTER residue] VIOLATION: gol disabled but zone_count="
                                << gol_state_.zone_count << " runs=" << rosterGolZoneRuns_ << "\n";
                        } else {
                            std::cout << "[ROSTER residue] gol disabled: zone buffers pristine"
                                << " (zone_count=0, zone-compute runs this session=0)\n";
                        }
                        rosterGolResidueDump_ = time_state_.seconds;
                    }
                }

                if (time_state_.seconds - agent_state_.last_census_dump >= AGENT_CENSUS_INTERVAL) {
                    dump_agent_census(agent_state_, this, "periodic");
                    const auto& player = agent_state_.slots[0];
                    std::cout << "[Player] pos=(" << std::fixed << std::setprecision(1)
                        << player.pos_x << "," << player.pos_z
                        << ") slot=" << player_.possessed_slot
                        << " behavior=" << player.behavior_id
                        << "\n";
                    agent_state_.last_census_dump = time_state_.seconds;
                }

                // Periodic entity census dump
#ifdef DIAG_ENTITY_CENSUS
                if (time_state_.seconds - lastCensusDump_ >= CENSUS_DUMP_INTERVAL) {
                    dump_entity_census("periodic");
                    lastCensusDump_ = time_state_.seconds;
                }
#endif

                // ─── Ribbon per-frame ── one call; the conductor lives in
                // modules/ribbon.inl (FRAME ORCHESTRATION). SEAM[ribbon:sky-mode].
                ribbon_frame_tick(ribbon_state_, this, queue);

                // ─── Entity mesh gen: single compute pass for all dirty families ──
                {
                    bool dirty[PopFamily::COUNT] = {};
                    bool anyDirty = false;
                    // ROSTER-GATE family (b) — the per-frame mesh-prepare
                    // loop folds over the roster at COMPILE TIME (R1 hot-path
                    // caveat): a disabled family's prepare_mesh is eliminated
                    // (no call, no runtime branch on a disabled piece).
                    // All-enabled unrolls to the original 12 calls in order.
                    #define ROSTER_PREP_FAMILY(F) \
                        if constexpr (ROSTER.family_enabled(F)) { \
                            dirty[F] = FAMILY_DISPATCH[F].prepare_mesh(this, queue); \
                            anyDirty = anyDirty || dirty[F]; \
                        }
                    ROSTER_PREP_FAMILY(PopFamily::PYRAMID);
                    ROSTER_PREP_FAMILY(PopFamily::ARCH);
                    ROSTER_PREP_FAMILY(PopFamily::COLUMN);
                    ROSTER_PREP_FAMILY(PopFamily::ANTENNA);
                    ROSTER_PREP_FAMILY(PopFamily::PALM);
                    ROSTER_PREP_FAMILY(PopFamily::CACTUS);
                    ROSTER_PREP_FAMILY(PopFamily::BLADE);
                    ROSTER_PREP_FAMILY(PopFamily::SPHERE);
                    ROSTER_PREP_FAMILY(PopFamily::RIBBON);
                    ROSTER_PREP_FAMILY(PopFamily::CUBE);
                    ROSTER_PREP_FAMILY(PopFamily::GOL);
                    ROSTER_PREP_FAMILY(PopFamily::GALLERY);
                    #undef ROSTER_PREP_FAMILY
                    if (anyDirty) {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "Entity Mesh Gen";
                        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                        // dispatch skips disabled families structurally:
                        // dirty[f] stays false for a disabled family (never
                        // set above), so this branches on dirty-ness, not on
                        // the enable bit.
                        for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
                            if (dirty[f]) FAMILY_DISPATCH[f].dispatch_mesh(this, pass);
                        }
                        pass.End();
                    }
                }
                upload_portal_array(this, queue);
                upload_lights(this, queue);

                // Re-sync the pawn mount to THIS frame. ribbon_advance_head (in the
                // ribbon block above) just recomputed the head mount, but the signal
                // uploaded earlier — and therefore the pawn — still carries the previous
                // frame's mount. Re-write the sky_* block so the pawn and the ribbon are
                // sampled at the same frame: the one-frame lag (a ~throttle·MAX_SPEED·dt
                // slide along the tube) disappears, leaving MOUNT_SETBACK as the sole
                // seat offset. Ordered before dispatch_compute, which runs the player
                // agent kernel that reads sky_*. SEAM[ribbon:sky-mode].
                {
                    float hx, hy, hz, hh;
                    ribbon_head_pose(ribbon_state_, hx, hy, hz, hh);
                    float fyaw, fpitch, froll;
                    ribbon_head_frame(ribbon_state_, fyaw, fpitch, froll);
                    gpuState_.resync_sky_head(queue, player_.sky_mode ? 1u : 0u,
                                              hx, hy, hz, hh, fyaw, fpitch, froll);
                }

                dispatch_compute(this, encoder);

                // Copy full agent buffer from GPU to staging (for readback next frame)
                if (pawnReadbackState_ == PawnReadbackState::IDLE) {
                    encoder.CopyBufferToBuffer(
                        gpuState_.agent_state_buffer(), 0,
                        gpuState_.agent_state_readback_staging(), 0,
                        GPUState::agent_state_buffer_size());
                    pawnReadbackState_ = PawnReadbackState::COPIED;
                }

                if (floaterReadbackState_ == FloaterReadbackState::IDLE) {
                    encoder.CopyBufferToBuffer(
                        gpuState_.floating_entity_buffer(), 0,
                        gpuState_.floating_entity_readback_staging(), 0,
                        GPUState::floating_entity_buffer_size());
                    floaterReadbackState_ = FloaterReadbackState::COPIED;
                }

                // GoL zone compute — derive params + sync + evolve (separate passes for barrier)
                if (gol_state_.zone_count > 0) {
                    rosterGolZoneRuns_++;  // ROSTER-RESIDUE gol (2e) — the only writer of the zone GPU buffers; counted so the disabled-piece residue check can prove pristine
                    flush_zone_derive_requests(gol_state_, this, queue);
                    upload_gol_zone_config(gol_state_, this, queue);

                    {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "GoL Zone Sync";
                        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                        renderer_.dispatch_zone_gol_sync(pass,
                            gpuState_.zone_gol_compute_group(), gol_state_.active_slot_count);
                        pass.End();
                    }
                    {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "GoL Zone Evolve";
                        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                        renderer_.dispatch_zone_gol_evolve(pass,
                            gpuState_.zone_gol_compute_group(), gol_state_.active_slot_count);
                        pass.End();
                    }

                    // Mesh gen pass (Group 0 = compute entity, Group 1 = zone mesh gen)
                    {
                        wgpu::ComputePassDescriptor cpd{};
                        cpd.label = "GoL Zone Mesh Gen";
                        wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                        renderer_.dispatch_zone_mesh_reset(pass,
                            gpuState_.zone_mesh_gen_group());
                        renderer_.dispatch_zone_mesh_gen(pass,
                            gpuState_.zone_mesh_gen_group(),
                            gol_state_.active_slot_count);
                        pass.End();
                    }
                }

                // Pawn aura compute — persistent terrain influence
                // Run while presence > 0 (ramping down after toggle-off) or clearing
                // ROSTER-GATE pawn_aura (b) — disabled: the whole aura compute
                // (config upload + dispatch) is eliminated; zero GPU writes.
                if constexpr (ROSTER.pawn_aura)
                if (player_.aura_presence > 0.0f || pawn_state_.aura_needs_clear) {
                    if (pawn_state_.aura_cfg_dirty) {
                        // Full config upload — profile changed or first frame
                        pawn_state_.aura_cfg_dirty = false;
                        const auto& ap = pawn_state_.active_aura_profile;

                        // Presence scales all aura params for smooth raise/lower
                        float p = player_.aura_presence;

                        GPUPawnAuraConfig auraCfg{};
                        auraCfg.cell_size = PATCH_CELL_SIZE;
                        auraCfg.influence_radius = ap.influence_radius * p;
                        auraCfg.attack_stiffness = ap.attack_stiffness;
                        auraCfg.attack_damping = ap.attack_damping;
                        auraCfg.release_rate = (p > 0.01f) ? ap.release_rate : 999.0f;
                        auraCfg.dt = time_state_.dt;
                        auraCfg.effect_mask = ap.effect_mask;
                        auraCfg.aura_n = 64;
                        auraCfg.tint_strength = std::min(ap.tint_strength * p, 1.0f);
                        auraCfg.tint_r = ap.tint_r;
                        auraCfg.tint_g = ap.tint_g;
                        auraCfg.tint_b = ap.tint_b;
                        auraCfg.delta_mode = ap.delta_mode;
                        auraCfg.delta_magnitude = ap.delta_magnitude;
                        auraCfg.t_beats = time_state_.beats;
                        // height_scale gates the compute shader's R channel write (> 0.01 = enabled).
                        // Actual terrain extrusion magnitude comes from config.pawn_aura_height in the VS.
                        auraCfg.height_scale = (pawn_state_.aura_height_enabled && p > 0.01f) ? ap.height_scale : 0.0f;
                        gpuState_.upload_pawn_aura_config(queue, auraCfg);
                    }
                    else {
                        // Steady state — only dt and t_beats change per frame
                        gpuState_.upload_pawn_aura_frame(queue, time_state_.dt, time_state_.beats);
                    }

                    wgpu::ComputePassDescriptor cpd{};
                    cpd.label = "Pawn Aura";
                    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                    renderer_.dispatch_compute_pawn_aura(pass,
                        gpuState_.pawn_aura_compute_group(),
                        GPUState::pawn_aura_workgroups());
                    pass.End();

                    // After one cleanup frame with release_rate=999, all cells are zero
                    if (pawn_state_.aura_needs_clear) { pawn_state_.aura_needs_clear = false; }
                }

                // Orb sky layer: one-shot init, optional color-only refresh,
                // snapshot previous state for flocking neighbor reads, then
                // advance dynamics.
                // ROSTER-GATE orbs (b) — disabled: no orb compute dispatched.
                if constexpr (ROSTER.orbs) {
                    dispatch_orb_init(orbs_state_, this, encoder);
                    dispatch_orb_recolor(orbs_state_, this, encoder);
                    dispatch_orb_copy_prev(orbs_state_, this, encoder);
                    dispatch_orb_dynamics(orbs_state_, this, encoder, queue);
                }

                if (world_state_.ground_entries_dirty) {
                    world_state_.ground_entries_dirty = false;
                    world_state_.placement_dirty = true;
                    upload_ground_entries(this, queue);
                }
                if (world_state_.placement_dirty) {
                    world_state_.placement_dirty = false;
                    dispatch_placement_correction(this, encoder);
                }

                // DIAG: frustum cull re-enabled — indirect draw active
                dispatch_frustum_cull(this, encoder, queue);

                render_shadow_pass(this, encoder);
                render_main_pass(this, encoder, backbuffer, depth);
                render_snapshot_pass(gallery_state_, this, encoder);

                for (uint32_t i = 0; i < gallery_state_.pending_promotion_count; i++) {
                    auto& p = gallery_state_.pending_promotions[i];
                    wgpu::Texture src = p.is_snapshot
                        ? gpuState_.snapshot_staging_texture()
                        : gpuState_.authored_staging_texture();
                    gpuState_.promote_to_exhibition(encoder, src, p.staging_layer, p.exhibition_layer);
                }
                gallery_state_.pending_promotion_count = 0;
            }

            // --- Patch streaming: determine active 7×7 grid, generate new patches ---
            // SEAM[spine:owns] stream_patches is the patch-streaming integration
            //   backbone — ~460 lines covering allocation budgets, generation
            //   phases, eviction. Genuinely spine work; modules consume but
            //   don't own pieces. Per Ch. 15.
            void stream_patches(wgpu::CommandEncoder& encoder, wgpu::Queue& queue) {
                // ─── Patch Generation Pipeline ─────────────────────────────────

                int32_t centerX, centerZ;
                uint32_t patchStagingOffset = 0;  // running offset into staging buffer (multiple batches per frame)
                bool tileGridDirty = false;        // coalesce tile grid uploads to one per frame
                if (world_state_.finite_mode) {
                    centerX = 0;
                    centerZ = 0;
                }
                else {
                    centerX = (int32_t)std::floor(player_.readback_x / PATCH_EXTENT);
                    centerZ = (int32_t)std::floor(player_.readback_z / PATCH_EXTENT);
                }

                // In finite mode, cap the effective radius
                uint32_t savedRadius = world_state_.active_radius;
                if (world_state_.finite_mode && world_state_.active_radius > world_state_.finite_radius) {
                    world_state_.active_radius = world_state_.finite_radius;
                }

                bool gridChanged = (centerX != world_state_.last_center_x || centerZ != world_state_.last_center_z);

                if (gridChanged) {
                    int32_t oldCX = world_state_.last_center_x;
                    int32_t oldCZ = world_state_.last_center_z;
                    world_state_.last_center_x = centerX;
                    world_state_.last_center_z = centerZ;

                    bool fullRegen = (oldCX == INT32_MAX);  // first frame

                    // Lightweight cache maintenance (no GPU buffer writes)
                    evict_distant_tiles(tile_world_state_, centerX, centerZ);

                    if (!fullRegen) {
                        tileGridDirty = true;
                    }

                    // ─── FULLREGEN: synchronous bootstrap ────────────────────
                    //
                    if (fullRegen) {
                        int32_t rr = (int32_t)world_state_.active_radius;
                        static constexpr int32_t TILE_PAD = 1;
                        int32_t rp = rr + TILE_PAD;
                        for (int32_t gz = centerZ - rp; gz <= centerZ + rp; gz++) {
                            for (int32_t gx = centerX - rp; gx <= centerX + rp; gx++) {
                                GridKey key{ gx, gz };
                                if (tile_world_state_.tileCache_.find(key) == tile_world_state_.tileCache_.end()) {
                                    TileState ts = generate_tile_state(tile_world_state_, this, gx, gz);
                                    tick_terrain_tokens(tile_world_state_, ts, tile_seed(world_state_.active_seed, gx, gz));
                                    tile_world_state_.tileCache_[key] = ts;
                                }
                            }
                        }

                        // NOW spawn portals — tile cache is populated, terrain heights are correct
                        if (mood_state_.back_portal_pending) {
                            force_spawn_back_portal(this, queue);
                        }
                        for (int32_t gz = centerZ - rr; gz <= centerZ + rr; gz++) {
                            for (int32_t gx = centerX - rr; gx <= centerX + rr; gx++) {
                                bool found = false;
                                for (uint32_t i = 0; i < world_state_.active_patch_count; i++) {
                                    if (patches_[i].grid_x == gx && patches_[i].grid_z == gz) {
                                        found = true; break;
                                    }
                                }
                                if (!found && world_state_.free_layer_count > 0) {
                                    uint32_t layer = alloc_layer();
                                    patches_[world_state_.active_patch_count] = ActivePatch{};
                                    patches_[world_state_.active_patch_count].grid_x = gx;
                                    patches_[world_state_.active_patch_count].grid_z = gz;
                                    patches_[world_state_.active_patch_count].layer = layer;
                                    patches_[world_state_.active_patch_count].valid = true;
                                    world_state_.active_patch_count++;
                                }
                            }
                        }
                        tileGridDirty = true;

                        // Spawn inner patches
                        PatchCandidate spawnCands[MAX_PATCHES];
                        uint32_t spawnCount = collect_sorted_patches(spawnCands,
                            player_.readback_x, player_.readback_z,
                            [&](const ActivePatch& p) {
                                return p.phase == PatchPhase::ALLOCATED &&
                                    in_priority_window(p.grid_x, p.grid_z, centerX, centerZ);
                            }, true);
                        spawn_selected_patches(spawnCands, spawnCount, queue);

                        // Generate inner patches
                        PatchCandidate genCands[MAX_PATCHES];
                        uint32_t genCount = collect_sorted_patches(genCands,
                            player_.readback_x, player_.readback_z,
                            [&](const ActivePatch& p) {
                                return p.phase == PatchPhase::SPAWNED &&
                                    in_priority_window(p.grid_x, p.grid_z, centerX, centerZ);
                            }, true);
                        generate_selected_patches(genCands, genCount,
                            encoder, queue, patchStagingOffset, tileGridDirty);
                    }
                }

                // ─── CONTINUOUS PATCH EVICTION ────────────────────────────────
                //
                {
                    PatchCandidate candidates[MAX_PATCHES];
                    uint32_t count = collect_sorted_patches(candidates,
                        player_.readback_x, player_.readback_z,
                        [&](const ActivePatch& p) {
                            return !in_render_window(p.grid_x, p.grid_z,
                                world_state_.last_center_x, world_state_.last_center_z);
                        }, false);  // farthest first

                    uint32_t evictThisFrame = std::min(count, EVICT_BUDGET_PER_FRAME);
                    for (uint32_t e = 0; e < evictThisFrame; e++) {
                        evict_patch(candidates[e].idx, queue);
                    }

                    if (evictThisFrame > 0) {
                        uint32_t write = 0;
                        for (uint32_t i = 0; i < world_state_.active_patch_count; i++) {
                            if (patches_[i].valid) patches_[write++] = patches_[i];
                        }
                        world_state_.active_patch_count = write;
                        world_state_.patch_instances_dirty = true;
                    }
                }

                // ─── CONTINUOUS PATCH ALLOCATION ──────────────────────────────
                //
                {
                    int32_t pawnGX = (int32_t)std::floor(player_.readback_x / PATCH_EXTENT);
                    int32_t pawnGZ = (int32_t)std::floor(player_.readback_z / PATCH_EXTENT);
                    int32_t rr = (int32_t)world_state_.active_radius;
                    float pawn_wx = player_.readback_x;
                    float pawn_wz = player_.readback_z;
                    float half = PATCH_EXTENT * 0.5f;

                    // O(1) patch existence lookup (replaces O(N) inner scan)
                    std::unordered_set<GridKey, GridKeyHash> activePatchSet;
                    activePatchSet.reserve(world_state_.active_patch_count);
                    for (uint32_t i = 0; i < world_state_.active_patch_count; i++) {
                        activePatchSet.insert({ patches_[i].grid_x, patches_[i].grid_z });
                    }

                    struct AllocCandidate { int32_t gx, gz; float dist2; };
                    AllocCandidate candidates[MAX_PATCHES];
                    uint32_t candidateCount = 0;

                    for (int32_t gz = pawnGZ - rr; gz <= pawnGZ + rr; gz++) {
                        for (int32_t gx = pawnGX - rr; gx <= pawnGX + rr; gx++) {
                            // Must be within allocation window of grid center
                            if (!in_render_window(gx, gz, world_state_.last_center_x, world_state_.last_center_z)) continue;
                            bool found = activePatchSet.count({ gx, gz }) > 0;
                            if (!found && world_state_.free_layer_count > 0 && candidateCount < MAX_PATCHES) {
                                float ox = (gx + 0.5f) * PATCH_EXTENT;
                                float oz = (gz + 0.5f) * PATCH_EXTENT;
                                float d2 = patch_distance_sq(pawn_wx, pawn_wz, ox, oz, half);
                                candidates[candidateCount++] = { gx, gz, d2 };
                            }
                        }
                    }

                    // Sort by distance (nearest first)
                    for (uint32_t i = 1; i < candidateCount; i++) {
                        AllocCandidate key = candidates[i];
                        uint32_t j = i;
                        while (j > 0 && candidates[j - 1].dist2 > key.dist2) {
                            candidates[j] = candidates[j - 1];
                            j--;
                        }
                        candidates[j] = key;
                    }

                    bool allocated_any = false;
                    uint32_t allocThisFrame = std::min(candidateCount, ALLOC_BUDGET_PER_FRAME);
                    for (uint32_t a = 0; a < allocThisFrame; a++) {
                        int32_t gx = candidates[a].gx;
                        int32_t gz = candidates[a].gz;
                        // Ensure tile cache entry (primary — ticks terrain tokens)
                        GridKey key{ gx, gz };
                        if (tile_world_state_.tileCache_.find(key) == tile_world_state_.tileCache_.end()) {
                            TileState ts = generate_tile_state(tile_world_state_, this, gx, gz);
                            tick_terrain_tokens(tile_world_state_, ts, tile_seed(world_state_.active_seed, gx, gz));
                            tile_world_state_.tileCache_[key] = ts;
                        }
                        // Also cache neighbors for tile grid padding
                        for (int dz = -1; dz <= 1; dz++) for (int dx = -1; dx <= 1; dx++) {
                            GridKey nk{ gx + dx, gz + dz };
                            if (tile_world_state_.tileCache_.find(nk) == tile_world_state_.tileCache_.end()) {
                                tile_world_state_.tileCache_[nk] = generate_tile_state(tile_world_state_, this, gx + dx, gz + dz);
                            }
                        }
                        uint32_t layer = alloc_layer();
                        patches_[world_state_.active_patch_count] = ActivePatch{};
                        patches_[world_state_.active_patch_count].grid_x = gx;
                        patches_[world_state_.active_patch_count].grid_z = gz;
                        patches_[world_state_.active_patch_count].layer = layer;
                        patches_[world_state_.active_patch_count].valid = true;
                        world_state_.active_patch_count++;
                        allocated_any = true;
                    }

                    // Mark tile grid and patch instances dirty whenever new patches were allocated
                    if (allocated_any) {
                        tileGridDirty = true;
                        world_state_.patch_instances_dirty = true;
                    }
                }

                // ─── DISTANCE-DRIVEN ENTITY SPAWNING ─────────────────────────
                //
                {
                    PatchCandidate candidates[MAX_PATCHES];
                    uint32_t count = collect_sorted_patches(candidates,
                        player_.readback_x, player_.readback_z,
                        [](const ActivePatch& p) {
                            return p.phase == PatchPhase::ALLOCATED;
                        }, true);
                    spawn_selected_patches(candidates,
                        std::min(count, SPAWN_BUDGET_PER_FRAME), queue);
                }

                // ─── DISTANCE-DRIVEN HEIGHTFIELD GENERATION ──────────────────
                //
                {
                    PatchCandidate candidates[MAX_PATCHES];
                    uint32_t count = collect_sorted_patches(candidates,
                        player_.readback_x, player_.readback_z,
                        [](const ActivePatch& p) {
                            return p.phase == PatchPhase::SPAWNED ||
                                p.phase == PatchPhase::NEEDS_REGEN;
                        }, true);
                    generate_selected_patches(candidates,
                        std::min(count, patches_budget_this_frame()),
                        encoder, queue, patchStagingOffset, tileGridDirty);
                }

                {
                    GPUPatchInstance instances[MAX_PATCHES]{};
                    uint32_t lod0Count = 0;
                    uint32_t lod1Count = 0;
                    uint32_t pregenCount = 0;

                    // Temporary arrays for each band
                    GPUPatchInstance lod0[MAX_PATCHES]{};
                    GPUPatchInstance lod1[MAX_PATCHES]{};
                    GPUPatchInstance pregen[MAX_PATCHES]{};

                    float pawn_wx = player_.readback_x;
                    float pawn_wz = player_.readback_z;
                    float half = PATCH_EXTENT * 0.5f;

                    for (uint32_t i = 0; i < world_state_.active_patch_count; i++) {
                        if (patches_[i].phase != PatchPhase::GENERATED &&
                            patches_[i].phase != PatchPhase::NEEDS_REGEN) continue;

                        float ox = (patches_[i].grid_x + 0.5f) * PATCH_EXTENT;
                        float oz = (patches_[i].grid_z + 0.5f) * PATCH_EXTENT;

                        GPUPatchInstance inst{};
                        inst.origin[0] = ox;
                        inst.origin[1] = oz;
                        inst.extent = PATCH_EXTENT;
                        inst.layer = patches_[i].layer;

                        float d2 = patch_distance_sq(pawn_wx, pawn_wz, ox, oz, half);

                        // Finite mode: all patches visible (walls define boundary, not fog)
                        if (world_state_.finite_mode || d2 <= VISIBILITY_CYLINDER_RADIUS_SQ) {
                            if (d2 <= LOD0_CYLINDER_RADIUS_SQ) {
                                lod0[lod0Count++] = inst;
                            }
                            else {
                                lod1[lod1Count++] = inst;
                            }
                        }
                        else {
                            pregen[pregenCount++] = inst;
                        }
                    }

                    // Pack: LOD-0, then LOD-1, then pregen
                    uint32_t w = 0;
                    std::memcpy(instances + w, lod0, lod0Count * sizeof(GPUPatchInstance)); w += lod0Count;
                    std::memcpy(instances + w, lod1, lod1Count * sizeof(GPUPatchInstance)); w += lod1Count;
                    std::memcpy(instances + w, pregen, pregenCount * sizeof(GPUPatchInstance)); w += pregenCount;

                    gpuState_.upload_patch_instances(queue, instances, w);
                    world_state_.lod0_patch_count = lod0Count;
                    world_state_.render_patch_count = lod0Count + lod1Count;
                    world_state_.all_patch_count = w;

                    // Sync placement_patch_count so compute_entity_placement
                    // can sample heightfields from the current frame's patch set.
                    gpuState_.config().placement_patch_count = w;
                    gpuState_.upload_placement_patch_count(queue);

                    gpuState_.config().lod_pawn_x = pawn_wx;
                    gpuState_.config().lod_pawn_z = pawn_wz;
                    gpuState_.upload_lod_pawn(queue);

                    // ─── Patch grid: O(1) spatial index for sample_terrain_y_at ────────
                    {
                        GPUPatchGrid grid{};
                        grid.side = Dim::PATCH_PREGEN_SIDE;
                        grid.cell_extent = PATCH_EXTENT;

                        int32_t min_gx = INT32_MAX;
                        int32_t min_gz = INT32_MAX;
                        for (uint32_t i = 0; i < world_state_.active_patch_count; i++) {
                            if (!patches_[i].valid) continue;
                            if (patches_[i].phase != PatchPhase::GENERATED &&
                                patches_[i].phase != PatchPhase::NEEDS_REGEN) continue;
                            min_gx = std::min(min_gx, patches_[i].grid_x);
                            min_gz = std::min(min_gz, patches_[i].grid_z);
                        }
                        if (min_gx == INT32_MAX) { min_gx = 0; min_gz = 0; }
                        grid.origin_x = min_gx;
                        grid.origin_z = min_gz;

                        for (uint32_t i = 0; i < world_state_.active_patch_count; i++) {
                            if (!patches_[i].valid) continue;
                            if (patches_[i].phase != PatchPhase::GENERATED &&
                                patches_[i].phase != PatchPhase::NEEDS_REGEN) continue;
                            int32_t lx = patches_[i].grid_x - grid.origin_x;
                            int32_t lz = patches_[i].grid_z - grid.origin_z;
                            if (lx < 0 || lz < 0 ||
                                lx >= int32_t(grid.side) || lz >= int32_t(grid.side)) continue;
                            grid.entries[lz * grid.side + lx] = patches_[i].layer + 1u;
                        }

                        gpuState_.upload_patch_grid(queue, grid);
                    }
                }
                // GPU Y-correction is additive (ground_y += terrain), so ground
                // entries must be re-uploaded with offset-only values whenever
                // the heightfield changes. Tie groundEntriesDirty to patch changes.
                world_state_.ground_entries_dirty = world_state_.ground_entries_dirty || world_state_.patch_instances_dirty;
                world_state_.placement_dirty = world_state_.placement_dirty || world_state_.patch_instances_dirty;
                world_state_.patch_instances_dirty = false;

                // ─── Entity distance culling ─────────────────────────────
                world_state_.entities_culled = update_entity_draw_visibility(queue);

                // ─── Deferred uploads (one per frame max) ────────────────
                if (tileGridDirty) upload_tile_grid_now(tile_world_state_, this, queue, world_state_.last_center_x, world_state_.last_center_z);
                flush_pier_count(queue);

                audit_entity_integrity();

                // Restore radius if we capped it for finite mode
                if (world_state_.finite_mode) { world_state_.active_radius = savedRadius; }
            }

            // Mood is VOCABULARY + APPLIERS + SIX DOORS: CeilingType /
            // MoodProfile / MOOD_TABLE / portal colors / indoor palettes +
            // the door, applier, and deriver declarations are in mood.hpp
            // (file scope, above the class); the definitions (which reach
            // the spine-owned state + in-class statics via the complete
            // type) are in mood.inl, included at FILE SCOPE in the
            // post-class MODULE IMPLEMENTATIONS zone. MOOD OWNS NO STATE —
            // nothing at the COMPOSITION ROOT; mood_state_ and the
            // transition machine are spine-resident
            // (SEAM[spine:transitions], constitution §2). The force-spawn
            // mutation belongs to the arch's owner: entities'
            // force_spawn_portal_arch (the ROSTER portal door lives
            // there). The lighting-scheme tables stay impl-side. See §1.

        public:

            void on_input(const InputEvent& event) override {
                switch (event.type) {
                case InputEvent::Type::KeyDown:
                    on_key_down(this, event.key);
                    break;
                case InputEvent::Type::KeyUp:
                    on_key_up(this, event.key);
                    break;
                case InputEvent::Type::MouseMove:
                    on_mouse_move(this, event.x, event.y);
                    break;
                case InputEvent::Type::MouseButton:
                    on_mouse_button(this, event.button, event.pressed);
                    break;
                case InputEvent::Type::Scroll:
                    on_scroll(this, event.y);
                    break;
                }
            }

        private:

        public:

            void get_clear_color(float& r, float& g, float& b) const override {
                r = clearColor_[0];
                g = clearColor_[1];
                b = clearColor_[2];
            }

            wgpu::TextureFormat depth_format() const override {
                return wgpu::TextureFormat::Depth24Plus;
            }

            bool supports_backspace() const override {
                return true;
            }

            bool reload_shaders() override { return renderer_.reload(); }
            const std::string& shader_path() const { return renderer_.shader_path(); }

        };

    } // namespace the_board
} // namespace t7

// ═══ MODULE IMPLEMENTATIONS (post-class, FILE SCOPE) ══════════════════
//
// WIRING FORM (fix-2): SELF-WRAPPING — the zone includes impls at FILE SCOPE; law in audit/LADDER.md.
#include "modules/pawn.inl"       // tick_pawn_couplings
#include "modules/entities.inl"   // the six preparers + the seven grounded-family evictors + the blade/palm/cactus recipes
#include "modules/spheres.inl"    // the sphere evictor + recipe
#include "modules/orbs.inl"       // orb lifecycle/commands/dispatches/render
#include "modules/gol_zones.inl"  // GoL three-phase lifecycle + per-frame uploads/dispatch
#include "modules/agents.inl"     // agent registry upload + spawn/respawn/possession/diagnostics
#include "modules/cube_behaviors.inl"  // cube registry upload + corral/kite/coordination + clear + evictor + recipe
#include "modules/gallery.inl"    // photographer + gallery sites + authored loading + wall paintings
#include "modules/ribbon.inl"     // author seats + head laws + frame conductor + three-phase lifecycle
#include "modules/input.inl"      // key/mouse dispatch + movement intent + camera commands (own GLFW include)
#include "modules/render_passes.inl"  // ground-entry prep + compute dispatch + shadow/main passes + light VPs
#include "modules/mood.inl"       // indoor light derivation + appliers + apply_mood + shell + portals + uploads + transition request + derivers
#include "modules/population_themes.inl"  // the envelope machine per-patch step
#include "modules/tile_world.inl"  // the four verbs over what the terrain remembers
#include "modules/family_dispatch.inl"  // THE TABLE — FAMILY_DISPATCH definition + shared no-op mesh adapters
