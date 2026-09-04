#pragma once

// ─── cartridge.hpp ───────────────────────────────────────────────
// glaw1 — the compile gate: the C++ compiler as witness-runner. Every
// static_assert in the tree is a glaw1 check; "glaw1 catches X" means
// the build fails loud. WGSL sits outside its reach — the two-rooms
// mirror rule and the boot rig are the nets there.
//
// THE_BOARD — Generative world engine.
//
// ONE REGIME (L38, the composition law): every module is a file-scope pair around the class;
// the cartridge is the composition root alone.
//
// SEAM[spine:owns] FAMILY_DISPATCH is genuinely spine work — the
//   integration hub that ties the six families together. Each row's
//   body lives in the family's owning module. Per Ch. 15 of the seam
//   map. Adding a new family means: write select/place/commit/
//   evict/prepare_mesh in the owning module, add wrappers below,
//   add 1 row to FAMILY_DISPATCH.
// SEAM[spine:K2-related] the dispatch_prepare_mesh_*,
//   dispatch_mesh_gen_* wrappers are integration glue, not module
//   work. They live here correctly.
//   NOTE[seam-map] keep wrappers here; they're the integration layer
//   between FAMILY_DISPATCH and per-family modules.
// SEAM[spine:P5] readback state machines + world_state_.world_gen counter are
//   pattern P5 (release-pending sentinel) at the spine level. Pawn +
//   floater readbacks each protect against stale callbacks from
//   previous worlds via the issue-time generation recorded at the
//   machine (pawnReadbackGen_ / floaterReadbackGen_ — OIL_1c moved it
//   out of the closure). Genuinely spine-owned, not a leak.
// SEAM[spine:P8] PlayerState's commented "Future (deferred)" fields
//   are explicit latent infrastructure: aura_presence is live here;
//   the other deferred fields await the unified entity layer.
//   Pattern P8 visible in source.
// SEAM[spine:P8] rebirth_world — RETIRED AT THE_PANEL I U1, the cue
//   answered. It stood UNCALLED AND MARKED for two campaigns as the
//   file's second P8 and the larger one, and the mark named its
//   caller in advance: "the panel's seed dial (gen cadence,
//   C3-destructive), arriving with the PANEL campaign". That caller
//   exists — block 15's WORLD_LIVE.next_seed and the REBIRTH door,
//   pressed at the frame boundary (organ_boundary.inc). Ten
//   teardown/reseed verbs came out of latency with it. Boot still
//   walks become_world alone and by ruling always will, which is the
//   half of the mark that was never about latency.
//   A TOMBSTONED GRADUATION, not a deletion: the seam is kept here,
//   struck, because a forward cue that came true is the evidence the
//   category works. The first P8 above is untouched and still latent.
// SEAM[spine:family-dispatch] all evict_<family> (owner-side),
//   dispatch_prepare_mesh_<family>, dispatch_mesh_gen_<family>
//   wrapper functions land here — referenced by FAMILY_DISPATCH and
//   by machine/spawn_engine.hpp's commit/evict pipelines.
// ─────────────────────────────────────────────────────────────────

#include "render/render_cartridge.hpp"
#include "core/input_event.hpp"
#include "core/boot_params.hpp"                                    // DOMESDAY_1 B9 — the ?seed= boot override (ctor, the one authoring site)
#include "core/instruments.hpp"                                    // THE INSTRUMENTS DIAL: INSTRUMENTS.frame_meter / .periodic_census gate the recurring self-measurement (compile-time, T7_INSTRUMENTS; default off)
#include "cartridges/the_board/contracts/roster.hpp"
#include "cartridges/the_board/demos/demo.hpp"             // THE SELECTED SENTENCE: DEMO + ROSTER (compile-time, INCUBATE_DEMO; default full)
#include "cartridges/the_board/primitives/seed_utils.hpp"           // hash/gaussian/tier-select helpers (pure-math leaf)
#include "cartridges/the_board/contracts/ground_architecture.hpp"  // ground contributor/policy tables + compile-time DAG checks
#include "cartridges/the_board/contracts/entity_types.hpp"         // THE CONTRACT HOME: pipeline contracts + boundary DTOs + queue unions + dispatch row/table decl
#include "cartridges/the_board/contracts/spawn_services.hpp"      // THE MACHINE'S DECL TIER: spawn/pipeline service decls + boundary DTOs + MIN_SEPARATION (bodies ride the merged machine headers at the cohort tail)
#include "cartridges/the_board/contracts/spine_state.hpp"          // TimeState + PlayerState + InputState + SkyState (spine organ TYPES; instances stay at the root)
#include "cartridges/the_board/contracts/point.hpp"                // THE POINT: the parent of the player system — host enum + the bubble decl; instance at the root
#include "cartridges/the_board/contracts/control_panel.hpp"        // THE PANEL: the field's dials + the beacon rests — one home, every room
#include "cartridges/the_board/contracts/driver_surface.hpp"       // THE DRIVERS' ROOM: rests and gains at the seams; phase_motion_drivers reads DRIVER_LIVE.fog
#include "cartridges/the_board/contracts/floaters.hpp"   // floater TYPES (ActiveSphere/ActiveCube), file scope
#include "cartridges/the_board/realization/state.hpp"
#include "console/organ_registry.hpp"   // the compiled dial registry + its C ABI (needs the home types above)
#include "cartridges/the_board/surface/population_themes.hpp"  // THE POPULATION PANEL: GLOBAL_ENTITY_DENSITY, and the spawn dials to come (ONE_WORLD-II U3)
#include "cartridges/the_board/contracts/surface_services.hpp"  // THE SURFACE'S DECL TIER: WorldState + the patch registry + budgets/visibility + PatchSystemState + the surface service decls (bodies ride surface/patch_system.hpp at the cohort tail)
#include "cartridges/the_board/surface/tile_world.hpp"          // S2: archetypes + tokens + TileState/cache + TileWorldDeps + impl — MERGED single file; after patch_system for WorldState/Dim::PATCH_EXTENT
#include "cartridges/the_board/bodies/grounded.hpp"             // grounded-family vocabulary + EntitiesState + impl — MERGED; after entity_pipeline for generic_*
#include "cartridges/the_board/bodies/agents.hpp"               // AgentState + AgentsDeps + impl — MERGED; AGENT_PALETTE lives here
#include "cartridges/the_board/bodies/cube_behaviors.hpp"       // CubeBehaviorsState + CubeDeps + impl — MERGED; after agents for AgentState
#include "cartridges/the_board/bodies/spheres.hpp"              // SphereState + SphereDeps + impl — MERGED single file; after entity_pipeline for the generic funnels
#include "cartridges/the_board/realization/renderer.hpp"
#include "cartridges/the_board/realization/drawable_table.hpp"  // The drawable table (one row per drawable; the two passes iterate it filtered) — after renderer/state, before render_passes
#include "cartridges/the_board/bodies/pawn.hpp"                 // PawnState + PawnDeps + impl — MERGED single file; after renderer for Renderer/GPUState complete
#include "cartridges/the_board/bodies/orbs.hpp"                 // OrbsState + OrbsDeps + impl — MERGED; after renderer for Renderer
#include "cartridges/the_board/surface/automaton.hpp"            // AutomatonState + AutomatonDeps (S5) + impl — the ground's own
#include "coupling/visual_canvas.hpp"
#include "cartridges/the_board/bodies/ribbon.hpp"               // RibbonState + RibbonDeps + impl — MERGED; after visual_canvas for the coupling face; after agents/cubes/spheres for the FIELD_2 mirror deps
#include "cartridges/the_board/direction/input.hpp"             // KeyState/MouseState + InputDeps + impl — MERGED; after ribbon for RibbonState (the sky fixture); InputState graduated to spine_state
#include "cartridges/the_board/realization/render_passes.hpp"   // the pass/dispatch bodies on THE MACHINE FACE + light-VP helpers — MERGED; before sky
#include "cartridges/the_board/direction/sky.hpp"              // SkyDeps + the draw + the world's birth + impl — MERGED; after ribbon/input (fan targets), before the machine natives (they call its derivers); SkyState graduated to spine_state
#include "cartridges/the_board/machine/spawn_engine.hpp"        // S3: the spawn engine — footprint registry, the composition law, mesh-param rebuilds, the census dumps — MERGED; cohort tail
#include "cartridges/the_board/machine/entity_pipeline.hpp"     // S3: the three-phase verbs + the welded four — MERGED; after spawn_engine (services) + entities (vocab)
#include "cartridges/the_board/surface/patch_system.hpp"        // S2: the active-patch machine's bodies on THE MACHINE FACE — MERGED; decl tier in contracts/surface_services.hpp
#include <cmath>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <algorithm>
#include <utility>    // std::pair — world_box_()'s return (HEM_1)
#include <string>
#include <vector>

namespace t7 {
    namespace the_board {

        // ═══ THE BOOT DRAW (DRAW_0) ══════════════════════════════════
        //
        // THE WORLD'S MASTER SEED IS CHOSEN AT BOOT, NOT AUTHORED AS A
        // CONSTANT. Each visit is a draw from the same latent space, the
        // way each of them is. One number changes and the generator
        // does the rest: terrain, the activity lattice, the waves, the
        // automaton's seeding, agent spawn and every entity placement all hang
        // off world_state_.active_seed and follow it without a further
        // edit anywhere.
        //
        // THE PIN (T7_WORLD_SEED, CMakeLists.txt) is empty by default,
        // so this macro is UNDEFINED and the seed is DRAWN. A number
        // defines it and the seed IS that number — the world becomes
        // exactly reproducible. -DT7_WORLD_SEED=42 is the authored world
        // (DEMO_SEED, demos/matrix.hpp) typed back in: the pin's default
        // value, and the acceptance test for this campaign. The dial is
        // a RESTORE, not a revert — there is nothing to undo.
        //
        // A DRAWN WORLD STAYS REPORTABLE because of the pin and the
        // witness line the root prints beside this call. A visitor's
        // "the ribbon speared something" is reproducible by reading the
        // seed off the console and pinning it.
        inline uint32_t boot_seed() {
#ifdef T7_WORLD_SEED
            return static_cast<uint32_t>(T7_WORLD_SEED);
#else
            // WALL TIME, AND THE REASON IS NOT STYLE. std::chrono::
            // steady_clock is performance.now() on Emscripten, and that
            // starts near ZERO at every page load — a steady_clock draw
            // would hand nearly every visitor the same world, silently,
            // and the only place the defect could surface is a user
            // report. system_clock is Date.now() there and the real wall
            // clock here: distinct per visit and per visitor.
            const uint64_t t = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count());

            // THE MIX. The raw clock's low bits are ADJACENT between two
            // visitors who arrive in the same second, and adjacent
            // numbers are not different worlds: tile_seed and
            // cpu_lattice_node_seed take the master seed through one xor
            // before their finalizer, so near inputs stay near through
            // the first round. cpu_hash (primitives/seed_utils.hpp) is
            // the tree's existing two-input scalar mixer — the same
            // multiply/xorshift idiom lattice_node_seed uses, three
            // rounds — and is shaped for exactly this: fold the 64-bit
            // clock's two halves into one avalanched u32. No new
            // mechanism was invented here.
            //
            // DEMO.seed rides the fold, and that is what keeps it live:
            // two demo columns with different authored seeds draw from
            // different streams, and the column's seed stays the number
            // the pin restores. XOR with a constant is a bijection, so
            // it costs the draw no entropy.
            return cpu_hash(static_cast<uint32_t>(t),
                            static_cast<uint32_t>(t >> 32) ^ DEMO.seed);
#endif
        }

        // The witness's second word. Boot-only; no frame reads it.
#ifdef T7_WORLD_SEED
        inline constexpr const char* BOOT_SEED_ORIGIN = "pinned";
#else
        inline constexpr const char* BOOT_SEED_ORIGIN = "drawn";
#endif

        class Cartridge : public RenderCartridge {

            // COMPOSITION ROOT — ORGANS ARE PUBLIC: sight is free; writes pass
            // through declared seams; the census enforces the seam law, not
            // access control.
        public:

            wgpu::Device device_;
            // OIL_1 U1: the queue, cached once at initialize() — the same
            // singleton object device_.GetQueue() returns; render() reads
            // this instead of re-fetching per frame (update() already
            // rides the harness-cached queue).
            wgpu::Queue queue_;
            wgpu::TextureFormat colorFormat_;
            wgpu::TextureFormat depthFormat_;

            GPUState gpuState_;
            Renderer renderer_;

            // ═══ COMPOSITION ROOT — MODULE STATE ════════════════════════
            //
            SphereState sphere_state_;

            //   cube_behaviors_state_ — CubeBehaviorsState:
            //     the cube diagnostics + the cube active-slot mirror.
            CubeBehaviorsState cube_behaviors_state_;

            //   pawn_state_ — PawnState: the pawn aura + presence state.
            PawnState pawn_state_;

            EntitiesState entities_state_;

            //   orbs_state_ — OrbsState: the sky-dome lifecycle +
            //     player-owned anchor/rule/gesture state.
            OrbsState orbs_state_;

            //   automaton_state_ — AutomatonState: the drawn world and
            //     its tick cursor. GoLState stood here with eight zone
            //     slots, two counters, a spawn gate and a derive queue;
            //     U1 took the machinery and U2 the family, so the whole
            //     organ is these four words now.
            AutomatonState automaton_state_;

            //   agent_state_ — AgentState: the 32-slot CPU mirror +
            //     respawn counters + diagnostic overrides.
            AgentState agent_state_;


            RibbonState ribbon_state_;

            //   tile_world_state_ — TileWorldState: the tile
            //     cache + the terrain tokens (what the terrain remembers).
            TileWorldState tile_world_state_;

            //   patch_system_state_ — PatchSystemState: the
            //     active-patch registry + the free-layer pool.
            PatchSystemState patch_system_state_;

            //   world_state_ — WorldState: the world seed +
            //     radii + patch counts + dirty flags. ROOT ORGAN: the struct
            //     lives in contracts/surface_services.hpp; the
            //     instance stays here at the root.
            WorldState world_state_;

            //   spawn_engine_state_ — SpawnEngineState: the
            //     two dispatch queues + the footprint registry + the census clock.
            SpawnEngineState spawn_engine_state_;

            InputState inputState_;
            KeyState keys_;
            MouseState mouse_;
            TouchMoveState touch_;   // SHIP_1 — the stick's organ; never written on native
            CameraControls camera_;   // the panel: look_sensitivity is live (KP_+/KP_-)

            // ═══ TIME STATE ═════════════════════════════════════════════
            // Struct TimeState lives in contracts/spine_state.hpp;
            // the instance stays here.
            TimeState time_state_;

            VisualCanvas  visual_canvas_;
            TargetBinding fog_density_dst_{};   // resolved "fog.density" pipe
            // Ribbon amp pipes (pitch compass) — resolved once at bind.
            TargetBinding ribbon_amp_lat_dst_{};
            TargetBinding ribbon_amp_vert_dst_{};
            TargetBinding ribbon_tint_stim_dst_{};
            TargetBinding ribbon_tint_mix_dst_{};
            TargetBinding fog_color_dst_{};      // resolved "fog.color" pipe (3 wide)
            // Checker pipes (CHECKER-1) — resolved once at bind.
            TargetBinding checker_mean_dst_{};
            TargetBinding checker_var_dst_{};
            // The choir's pipe (CHOIR_0) — CHOIR_LANES wide, resolved once.
            TargetBinding cube_light_dst_{};
            // The ground's two SOURCE pipes (GROUND_VOICE_0) — the law that
            // turns them into the automaton's two multipliers is at the seam.
            TargetBinding ground_energy_dst_{};
            TargetBinding ground_density_dst_{};

            // Sun + atmosphere — authored solely by stage_world_birth, at boot
            // and at every rebirth. No boot literals: ATMOS_LIVE is the one
            // source.
            float sunDirection_[3] = {};
            float sunColor_[3] = {};
            float clearColor_[3] = {};

            // ═══ SKY STATE ══════════════════════════════════════════════
            //
            // Struct SkyState lives in contracts/spine_state.hpp.
            // The INSTANCE stays spine-resident because the values the
            // world's birth draws feed every other subsystem
            // (SEAM[spine:transitions], K4).
            SkyState sky_state_;

            // ═══ PLAYER STATE ════════════════════════════════════════════
            //
            // Struct PlayerState lives in contracts/spine_state.hpp
            // — SEAM[spine:P8] rides with the
            // struct; the instance stays here.
            PlayerState player_{};

            // ═══ THE POINT ═══════════════════════════════════════════════
            //
            // The parent of the player system (contracts/point.hpp): the
            // camera is its permanent witness; the pawn is its default
            // host (the kite); free-fly re-hosts it on the camera. The
            // GPU mirror is config.point_host; the toggle is input's
            // point-host command (key 4).
            PointState point_{};
            // The mount's edge and its ease (RIBBON_1) — beside the point,
            // because a host change is what raises it. possess() writes it;
            // FillSignal ships and advances it; nothing reads it back.
            MountState mount_{};
            // THE PENDING dt (RIBBON_3) — the frame's dt as the GPU will see
            // it: the sum of every update since the last submitted frame,
            // capped at the same 100 ms the measurement is. Written by
            // phase_fill_signal, cleared by frame_submitted(); nothing else
            // touches it. time_state_.dt keeps the per-update value, because
            // the CPU's own integrators run on every update, rendered or not.
            float dtPending_ = 0.0f;

            // ═══ THE MACHINE FACE ═══════════════════════════════════════
            // The one declared context the dispatch contract hands the
            // rows — references bound once, in the constructor, to the
            // organs above (contracts/entity_types.hpp owns the type).
            MachineCtx machine_ctx_;

            // Per-module deps faces — bound once
            // in the ctor, each the module's requirements made literal.
            TileWorldDeps tile_world_deps_;
            SphereDeps    sphere_deps_;
            PawnDeps      pawn_deps_;
            OrbsDeps      orbs_deps_;
            AgentsDeps    agents_deps_;
            CubeDeps      cube_deps_;
            AutomatonDeps automaton_deps_;
            RibbonDeps    ribbon_deps_;
            InputDeps     input_deps_;
            SkyDeps      sky_deps_;


            // SEAM[spine:transitions] (K4, Jean, 2026-07-11): the transition
            //   machine and its working members were DECLARED SPINE-OWNED
            //   ORCHESTRATION per the §2 residency law, the same legitimacy
            //   class as the P5 readbacks. ONE_WORLD-I took the machine and
            //   then the doors; the ruling stands over what remains —
            //   sky_state_ — and over rebirth_world, the machine's one
            //   survivor. The sky (direction/sky.hpp) supplies the draw, the
            //   applier and the birth sequence and owns NO instance; struct
            //   SkyState's TYPE lives in contracts/spine_state.hpp.
            //   Constitution §2 carries the K4 line.
            // SEAM[spine:portal-system] CLOSED at ONE_WORLD-I U2 — its whole
            //   subject (the portal arrays, the back-portal anchor, the
            //   force-spawn channel) left with the doors.


            // ═══ GPU READBACK + WORLDGEN ═════════════════════════════════
            //
            // SEAM[spine:P5] readback state machines + world_state_.world_gen counter are
            //   pattern P5 (release-pending sentinel) at the spine level.
            //   Pawn + floater readbacks each protect against stale callbacks
            //   from previous worlds via the issue-time generation recorded
            //   at the machine (the *ReadbackGen_ members below — OIL_1c
            //   moved it out of the closure, where nothing could address it).
            //   Genuinely spine-owned, not a leak.

            enum class PawnReadbackState { IDLE, COPIED, MAPPING };
            PawnReadbackState pawnReadbackState_ = PawnReadbackState::IDLE;
            // OIL_1c — THE GENERATION BELONGS TO THE MACHINE, not to a
            // closure. Written at ISSUE time (the COPIED→MAPPING arm),
            // read in the callback: the same fact the capture carried,
            // now somewhere addressable, beside the state it qualifies.
            // EQUIVALENT TO THE CAPTURE ONLY BECAUSE AT MOST ONE READBACK
            // PER MACHINE IS EVER IN FLIGHT — only IDLE arms a copy, only
            // COPIED issues a map, and the callback restores IDLE on its
            // way out, so no second issue can overwrite this while a
            // callback is pending. If that ever became two-in-flight, the
            // second issue would hand its gen to the first callback and
            // the P5 stale-world guard would pass where it must drop.
            uint32_t pawnReadbackGen_ = 0;

            // OPT_1a's TWO BOOLS LEFT WITH THE REST LAW (ONE_SURFACE-II
            // U1) — `liveCardRestClean_` and `liveCardWritePending_`, the
            // second of which carried R8's verdict to R10 as SPINE_2 B's
            // one-row hand-off. There is no verdict to carry: the card is
            // written every frame. The tombstone at phase_live_card_write
            // holds the whole retirement.
            enum class FloaterReadbackState { IDLE, COPIED, MAPPING };
            FloaterReadbackState floaterReadbackState_ = FloaterReadbackState::IDLE;
            uint32_t floaterReadbackGen_ = 0;   // OIL_1c — same grammar as pawnReadbackGen_ above
            // ATRIUM_11 — THE CAMERA WITNESS'S MACHINE, the pawn's and the
            // floaters' grammar exactly: skip-if-busy, the issue-time
            // generation, the stale callback dropped. It exists because the
            // orbit has NO CPU mirror — compose_camera_position_from_orbit is
            // WGSL and the CHORD_3 block copy is GPU-to-GPU — so a pose made
            // with the mouse is unreadable from here any other way. Every
            // line of it, and the buffer behind it, is `if constexpr`-gated
            // on INSTRUMENTS.camera_witness, so an unarmed build has no third
            // readback at all.
            enum class CameraReadbackState { IDLE, COPIED, MAPPING };
            CameraReadbackState cameraReadbackState_ = CameraReadbackState::IDLE;
            uint32_t cameraReadbackGen_ = 0;
            // The point readback (option A): runs ONLY in
            // camera-host — the camera's GPU position IS the point's, so
            // it must reach the CPU for the viewpoint set (streaming,
            // LOD, cull, orb). Pawn-host never arms this machine.
            // THE FRAME METER's timestamp readback rides the same P5
            // grammar (skip-if-busy: at most one in flight; unsampled
            // frames still write timestamps, they just aren't resolved).
            // Timing is world-agnostic — no world_gen capture needed.
            enum class MeterReadbackState { IDLE, COPIED, MAPPING };
            MeterReadbackState meterReadbackState_ = MeterReadbackState::IDLE;
            // WRAP_0 U4 — THE SLOT LINE'S SAMPLE. The draw plan's two
            // instance counters live only on the GPU, so the terrain's
            // milliseconds could never be divided by its geometry. Same
            // grammar as the meter's own readback and the same SKIP-IF-BUSY,
            // which is right here rather than wrong: one sample is wanted per
            // window, not per frame, and a count of visible patches is
            // geometry — it moves at walking pace, so a sample a second or two
            // old is still a true reading, where a timing sample would not be.
            MeterReadbackState slotReadbackState_ = MeterReadbackState::IDLE;
            // TWO, not three (the device gate's hotfix). It was three, and
            // the callback read a[11] out of a 40-byte mapped range after
            // U5 folded segment C — past the end of the map, in the one
            // build that arms it.
            uint32_t slotInstances_[2] = {};
            bool     slotSampleValid_ = false;
            bool meter_gpu_ = false;   // device carries timestamp-query (set at initialize)

            // ROSTER-RESIDUE gol's two members (rosterGolZoneRuns_ and its
            // cadence timer) left with the check that read them — see the
            // tombstone at phase_census_dumps.

            // ═══ FAMILY DISPATCH TABLE ═══════════════════════════════════
            //
            // SEAM[spine:owns] FAMILY_DISPATCH is the integration hub that
            //   ties the five families together. Each row's body lives in
            //   the family's owning module.
            // SEAM[spine:K2-related] CLOSED. The last real
            //   dispatch_prepare_mesh_* / dispatch_mesh_gen_* adapter pair
            //   was the arch's, and it left with the family (ONE_WORLD-I
            //   U3): PRUNE_2 had already taken the other four. Every row's
            //   mesh hook is the none-fork now. The bespoke
            //   select/place/commit funnels AND the evictors live with their
            //   owners (§5 EVICTION THUNKS: retirement fulfilled); the no-op
            //   mesh adapters are shared (inlined beside the table,
            //   post-class).
            // SEAM[spine:family-dispatch] anchor for cross-file references —
            //   eviction routed through FAMILY_DISPATCH[f].evict_slot to the
            //   owner-side evict_<family> functions.
            //
            // The row type (struct FamilyDispatch) and the queue-entry
            // unions it walks (EntityQueueEntry / PlacementEntry) live in
            // entity_types.hpp — the contract home.

            // ═══ DISPATCH WRAPPERS ═══════════════════════════════════════

            // ── Mesh gen wrappers: NONE REMAIN ──
            // The pyramid never had one: it is the first entity whose
            // realization IS the terrain — it keeps its
            // select/place/commit/evict verbs (placement feeds the
            // heightfield) but has no mesh realization of its own. The arch
            // held the last real pair and left at ONE_WORLD-I U3, so every
            // surviving family's FAMILY_DISPATCH mesh hook routes to the
            // none-fork.

            // ── The dispatch table (FAMILY_DISPATCH) is defined at file
            //    scope after the class, beside the shared no-op adapters
            //    (declared in entity_types.hpp). ──
            //
            // The spine-owned piece-enable manifest (struct Roster, the
            // ROSTER constant and the full doc
            // block — RIDER A / MATURITY DIAL / FOUNDATIONAL / LATENT /
            // gate-(a) status column) now lives in
            // cartridges/the_board/contracts/roster.hpp. It met its SECOND CONSUMER —
            // GPUState::init (state.hpp) gates creation on the feature bits —
            // so the reading publishes at the shared header (the standing
            // law). ROSTER is visible here by namespace lookup
            // (t7::the_board::ROSTER); every ROSTER-GATE / ROSTER-RESIDUE
            // consult below is unchanged.

        public:

            // ═══ PUBLIC: CARTRIDGE LIFECYCLE ═════════════════════════════

            Cartridge()
                : machine_ctx_{ world_state_, tile_world_state_,
                                sky_state_, patch_system_state_, spawn_engine_state_,
                                entities_state_, sphere_state_, cube_behaviors_state_,
                                ribbon_state_,
                                time_state_, player_, point_, gpuState_, renderer_ }
                , tile_world_deps_{ world_state_, gpuState_ }
                , sphere_deps_{ time_state_ }
                , pawn_deps_{ player_, time_state_, gpuState_, renderer_ }
                , orbs_deps_{ gpuState_, renderer_, player_, time_state_, world_state_ }
                , agents_deps_{ gpuState_, player_, point_, world_state_, time_state_ }
                , cube_deps_{ gpuState_, time_state_, player_, point_ }
                , automaton_deps_{ gpuState_, renderer_, time_state_ }
                , ribbon_deps_{ gpuState_, time_state_, tile_world_state_, player_, point_, inputState_, world_state_, sky_state_, visual_canvas_, ribbon_amp_lat_dst_, ribbon_amp_vert_dst_, ribbon_tint_stim_dst_, ribbon_tint_mix_dst_ }
                , input_deps_{ inputState_, keys_, mouse_, touch_, player_, world_state_, ribbon_state_, gpuState_, device_, point_, mount_, camera_ }
                , sky_deps_{ sky_state_, world_state_, gpuState_, sunDirection_, sunColor_, clearColor_ } {
                // THE ROOT AUTHORS THE BOOT VALUES (the demo sentence lands
                // here, not via in-struct defaults — no include-order cable).
                // DRAW_0: the seed is DRAWN, not authored — boot_seed()
                // (above this class) decides, and DEMO.seed is what the pin
                // restores. This line is the campaign's whole edit site.
                world_state_.active_seed = boot_seed();
                // DOMESDAY_1 B9 — the parameter surface: a seed present at
                // boot (?seed= / --seed=) overrides the draw at this one
                // authoring site. Measurement first; boot-read; no mid-run
                // mutation.
                const char* seed_origin = BOOT_SEED_ORIGIN;
                if (boot_params().has_seed) {
                    world_state_.active_seed = boot_params().seed;
                    seed_origin = "param";
                }
                // THE WITNESS (P6). One line, at boot, immediately after the
                // choice and before any consumer — zero frame cost. This line
                // is what keeps a randomized world reportable.
                std::cout << "[World] Boot seed=" << world_state_.active_seed
                          << " (" << seed_origin << ")\n";
                // THE SEED DIAL IS SEATED FROM THE SEED THAT WAS DRAWN
                // (THE_PANEL I U1). WORLD_LIVE has no design table — a
                // world's seed was never authored, so there is no constant
                // to graduate from — and this is the one line that can know
                // the answer: after boot_seed(), after the `--seed=`
                // override, before any consumer can read the manifest.
                //
                // It is the manifest's own law applied: a surface opens
                // showing the PROGRAM, not its own defaults. And it makes
                // the untouched door a REDRAW — press rebirth without
                // turning the dial and the same world is torn down and
                // built again, which is the honest first half of the walk.
                WORLD_LIVE.next_seed = world_state_.active_seed;
                // AND THE RADIUS RANGE — PINNED SHUT (STAGE_0 U1). It was
                // seated to FINITE_RADIUS_MIN/MAX, the full design range,
                // and become_world drew this world's radius inside it. The
                // STAGE LAW authors that fact instead: min == max ==
                // WORLD_RADIUS_PIN, so derive_finite_radius returns the pin
                // through its own `lo >= hi` arm and never reaches the
                // hash. The seat must still precede become_world, which is
                // the same reason the seed's seat does.
                //
                // THE TWO DIALS STAY LIVE, and that is deliberate: a hand
                // that widens the range at the panel gets a drawn radius
                // again, which is how the pin is INSPECTED rather than
                // merely trusted. The default is authored; the door is not
                // welded.
                WORLD_LIVE.radius_min = WORLD_RADIUS_PIN;
                WORLD_LIVE.radius_max = WORLD_RADIUS_PIN;

                // BOOT IS A BIRTH FROM NOTHING — IN FACT (ATRIUM_0).
                // The SEED is settled above and is now the whole of what a
                // world is chosen by: the mood that used to be settled
                // beside it, and the override that forced it, both left at
                // ONE_WORLD-II U2. Boot walks the L10 door and
                // nothing else: there is no world behind it to tear down,
                // which is exactly what rebirth_world's body is for.
                become_world(world_state_.active_seed);

            }

            Cartridge(const Cartridge&) = delete;
            Cartridge& operator=(const Cartridge&) = delete;

            void initialize(wgpu::Device device) override {
                device_ = device;
                queue_ = device_.GetQueue();   // OIL_1 U1 — one fetch, one home
                auto tGpu0 = std::chrono::high_resolution_clock::now();
                gpuState_.init(device);
                auto tGpu1 = std::chrono::high_resolution_clock::now();
                std::cout << "[Cartridge] GPUState init:    "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(tGpu1 - tGpu0).count()
                    << " ms\n";

                // THE FRAME METER — GPU half arms only on timestamp-query
                // (the console requests it when the adapter has it).
                // Absent → degrade loudly; CPU rows are unaffected.
                // Behind the instruments dial: off, the probe never runs and
                // meter_gpu_ stays false, so the harvest/resolve block folds
                // out with it (core/instruments.hpp).
                if constexpr (INSTRUMENTS.frame_meter) {
                    meter_gpu_ = device_.HasFeature(wgpu::FeatureName::TimestampQuery);
                    if (!meter_gpu_)
                        std::cout << "[METER] timestamp-query unavailable on this adapter — CPU rows only\n";
                }

                {
                    // The surface voice's terrain rows read THE
                    // TERRAIN_LOOKS PANEL ROW 2 (surface/
                    // terrain_looks.hpp) — the rest column lives where
                    // the parameters live. Values unchanged: blend -1
                    // = inactive, everything else 0.
                    gpuState_.set_band_motion(terrain_looks::REST_BAND_BLEND,
                        terrain_looks::REST_BAND_PHASE_ORIGIN);
                    gpuState_.set_terrain_time(terrain_looks::REST_TERRAIN_TIME);
                    gpuState_.set_mode_color_shift(terrain_looks::REST_MODE_COLOR_SHIFT);
                    gpuState_.set_mode_checker_scatter(terrain_looks::REST_MODE_CHECKER_SCATTER);
                    gpuState_.set_mode_palette_drift(terrain_looks::REST_MODE_PALETTE_DRIFT_TARGET,
                        terrain_looks::REST_MODE_PALETTE_DRIFT_INTENSITY,
                        terrain_looks::REST_MODE_PALETTE_DRIFT_TIER);
                    gpuState_.set_checker_color_field(terrain_looks::REST_CHECKER_RESULTANT,
                        terrain_looks::REST_CHECKER_AMOUNT,
                        terrain_looks::REST_CHECKER_VARIANCE);
                    gpuState_.set_mode_gol_scales(1.0f, 1.0f);   // GoL's jurisdiction — stays inline (ROW 9 pointer)
                    // Pulse ring rest — the count is a ROW 2 pin; the
                    // zeroed ring is the rest (Phase 1, C4-F1).
                    float zero_pulses[32] = {};
                    gpuState_.set_pulse_data(terrain_looks::REST_PULSE_COUNT, zero_pulses);
                    // The CameraControls panel authors the fly speed
                    // — one dial, one writer, at boot.
                    gpuState_.set_point_fly_speed(CameraControls::MOVE_SPEED);
                    // Tilt lag rest = the pawn's response (CLOSURE_PAWN [6]).
                    // Matches zero-init; stated here so the rest lives with the
                    // other rest pins rather than in the struct. U1 re-authors
                    // it from the possessed figure every frame.
                    gpuState_.set_pawn_tilt_tau(PAWN_FIGURES[0].tilt_tau);
                    // FPV eye rest = the conventional figure's eye (TUNE_1 A3).
                    // Same shape as the tilt pin above; U1 re-authors it from
                    // the possessed figure every frame. U1 runs before the
                    // first upload (UPDATE_SPINE[0] vs [6]), so a zero could
                    // not actually reach the GPU — this declares the rest
                    // value where the other rest pins live (L10), it is not a
                    // guard against a reachable frame-1 hazard.
                    gpuState_.config().fpv_eye_height =
                        FPV_EYE_RATIO * PAWN_FIGURES[0].height;
                    gpuState_.mark_config_dirty();
                }

                // RIBBON_1: the mount block rides the whole-struct signal drain
                // like every other word, so there is no boot-neutral to write —
                // MountState rests at phase 1, kind 0 (arrived, nothing in
                // flight) and the first frame ships that.
            }

            bool init_renderer(
                wgpu::TextureFormat colorFormat,
                wgpu::TextureFormat depthFormat
            ) {
                colorFormat_ = colorFormat;
                depthFormat_ = depthFormat;

                validate_spine();  // BOOT: table-order + O-5b/c face law (the O-#/RC laws are static-asserted)

                auto t0 = std::chrono::high_resolution_clock::now();
                if (!renderer_.init(
                    device_,
                    gpuState_,
                    colorFormat,
                    depthFormat
                )) return false;

                auto t1 = std::chrono::high_resolution_clock::now();

                // ═══ MOVEMENT: BOOT — S2 THE SURFACE ════════════════════════
                // The same door rebirth_world uses. reset_surface opens
                // with init_patch_system, so boot's order is unchanged; what boot
                // gains is the rest of the reset, which it previously received
                // only as in-struct defaults that HAPPENED to match.
                {
                    wgpu::Queue q = device_.GetQueue();
                    reset_surface(&machine_ctx_, q, tile_world_state_);
                }

                // ═══ MOVEMENT: BOOT — PER-PIECE BOOT VERBS (part one) ═══════
                // Order is today's, preserved byte-for-byte (PRIME INVARIANT);
                // one conductor call per piece, presence constexpr-gated.
                // BOOT IS A TRANSITION FROM NOTHING. The world has one way to come
                // into being; the only difference between boot and a rebirth is
                // what came before. stage_world_birth is that one way — it subsumes
                // the frustum-cull row, the orb one-shot, and every atmospheric
                // value boot used to hand-copy from a table row.
                {
                    wgpu::Queue q = device_.GetQueue();
                    stage_world_birth(&sky_deps_, q,
                        machine_ctx_,
                        orbs_state_, orbs_deps_,
                        pawn_state_);
                }

                // Agent registries — behaviors from AGENT_BEHAVIORS
                // (bodies/agents.hpp), tiers from the world's definition
                // bank TIER_LIVE (contracts/agent_tiers.hpp), uploaded to
                // GPU storage buffers at bindings 110 + 111. Behaviors are
                // constexpr-equivalent, so for them this stays a one-shot
                // write at boot; the bank is a live surface, and the frame
                // boundary re-speaks this same author whenever the panel
                // edits it.
                {
                    wgpu::Queue q = device_.GetQueue();
                    upload_agent_registries_to_gpu(&agents_deps_, q);
                }

                // ═══ MOVEMENT: BOOT — S3 PLACEMENT ══════════════════════════
                {
                    // Slot 0, the pawn — ungated: the player body is
                    // unconditional (owner verb).
                    seed_player_body(agent_state_, &agents_deps_);

                    wgpu::Queue q = device_.GetQueue();
                    // ROSTER-GATE wanderers (c) — boot population (agent slots
                    // 1+). Slot 0 (the pawn, seeded just above) is untouched.
                    if constexpr (ROSTER.wanderers)
                        spawn_population(agent_state_, &agents_deps_, world_state_.active_seed,
                            Idle::PAWN_POS_X, Idle::PAWN_POS_Z,
                            world_box_().first, world_box_().second, q);
                    dump_agent_census(agent_state_, &agents_deps_, "boot");
                    dump_entity_census(&machine_ctx_, "boot");
                }

                // ═══ MOVEMENT: BOOT — S2 THE WORLD IS BUILT ═════════════════
                // ONE_SURFACE-I U1. The grid is allocated, spawned, baked,
                // banded and uploaded here, once, before the first frame —
                // the same door rebirth_world walks. It stands AFTER the
                // "boot" entity census on purpose: that census reads zero by
                // construction and says so, and a world built ahead of it
                // would have made it a lie. The "born" census the builder
                // prints is the first count of a world that exists.
                {
                    wgpu::Queue q = device_.GetQueue();
                    build_world(&machine_ctx_, device_, q, tile_world_state_, tile_world_deps_);
                    birth_the_automaton(q);
                }

                auto t3 = std::chrono::high_resolution_clock::now();

                std::cout << "[Cartridge] Renderer init:    "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count() << " ms\n";
                std::cout << "[Cartridge] Patch system:     "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t1).count() << " ms\n";
                std::cout << "[Cartridge] Total init:       "
                    << std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t0).count() << " ms\n";

                // PORT_4b — THE BUDGET, once, after the LAST allocation.
                // Every GPU maker has now run: GPUState::init's five
                // creators, which since PRUNE_1 are the whole of them —
                // the late offscreen creator that forced this site went
                // with the gallery, and the rule it established stands.
                // Placed after the timings so it reads beneath "Total
                // init", and BEFORE the ROSTER + [Ground] block so that
                // block's claim to be the cartridge's last init line
                // stays true.
                gpuState_.report_gpu_budget();
                // ORGAN — the homes exist by now, so bind them; the ABI is
                // inert until this runs. The point is BORROWED, so the
                // panel cannot name a host the program has left.
                t7::organ::bind_home(&gpuState_);
                t7::organ::bind_point(&point_);   // RIBBON_1 — the panel's host row

                if constexpr (!ROSTER.all_enabled()) {
                    std::string off;
                    auto mark = [&](bool enabled, const char* name) {
                        if (!enabled) { if (!off.empty()) off += ", "; off += name; }
                        };
                    mark(ROSTER.pyramid, "pyramid");
                    mark(ROSTER.sphere, "sphere");
                    mark(ROSTER.ribbon, "ribbon");       mark(ROSTER.cube, "cube");
                    mark(ROSTER.pawn_aura, "pawn_aura"); mark(ROSTER.orbs, "orbs");
                    mark(ROSTER.wanderers, "wanderers");
                    // EVERY REMAINING PIECE IS SH-SHARED (ONE_WORLD-II U4).
                    // The shell was the roster's ONE SEPARABLE piece — the
                    // only bit whose buffers were skipped rather than created
                    // pristine — so the ternary that named it has no live
                    // branch and the classification is empty.
                    std::cout << "[ROSTER] pieces disabled: " << off
                        << " | buffer creations skipped: (none — every disabled piece is SH-shared, created-pristine)" 
                        << " | pipelines skipped: " << Renderer::pipelines_skipped() << "\n";
                }

                // P6 COROLLARY — a transition witness prints its state ONCE
                // at boot, so that silence afterwards means "no transition",
                // not "no witness". The Release boot carried no [Ground]
                // line at all and the two causes were indistinguishable;
                // this is that defect's cure. Same form as the transition,
                // marked (boot). Placed as the cartridge's LAST init line —
                // after the timings and the ROSTER block, before the meter
                // restamp — so it reports the state the first frame will
                // actually see.
                {
                    // THE TWO [Ground] SWITCH LINES ARE RETIRED, and P6 is
                    // the reason rather than an exception to it. A witness
                    // exists so silence means "no transition" instead of "no
                    // witness" — but both switches are PINNED now
                    // (zone_rects_in_core and zones_active_anywhere each
                    // return 1, and the tombstones at their definitions say
                    // why), so neither can transition and a per-boot line
                    // announcing a constant is noise pretending to be
                    // evidence. The pin's own reason is at the functions;
                    // when the panel's pause dial unpins them, the lines
                    // come back with the switch.
                    // The [Card] boot line left with the rest law it
                    // witnessed (P6's corollary applies to a SWITCH, and
                    // there is no switch). The automaton's own line below
                    // is what the card's state is now derived from.
                    std::cout << "[Ground] automaton: "
                              << automaton_state_.cfg.grid_size << "x"
                              << automaton_state_.cfg.grid_size << " cells, rule=0x"
                              << std::hex << automaton_state_.cfg.rule_mask << std::dec
                              << " period=" << automaton_state_.cfg.tick_period
                              << " density=" << automaton_state_.cfg.density
                              << " height=" << automaton_state_.cfg.alive_height
                              << " (boot)\n";
                }

                // THE FRAME METER: boot ends here — restamp the window so
                // the first census fps excludes init wall-time.
                if constexpr (INSTRUMENTS.frame_meter) meter_.reset();

                return true;
            }

            void bind_signal_layout(StatLayoutView v) {
                visual_canvas_.bind(v);
                fog_density_dst_ = visual_canvas_.layout().resolve("fog.density");
                fog_color_dst_ = visual_canvas_.layout().resolve("fog.color");
                ribbon_amp_lat_dst_ = visual_canvas_.layout().resolve("ribbon.amp_lateral_mult");
                ribbon_amp_vert_dst_ = visual_canvas_.layout().resolve("ribbon.amp_vertical_mult");
                ribbon_tint_stim_dst_ = visual_canvas_.layout().resolve("ribbon.color_stim");
                ribbon_tint_mix_dst_ = visual_canvas_.layout().resolve("ribbon.color_mix");
                checker_mean_dst_ = visual_canvas_.layout().resolve("terrain.checker_mean");
                checker_var_dst_ = visual_canvas_.layout().resolve("terrain.checker_var");
                cube_light_dst_ = visual_canvas_.layout().resolve("cube.light");
                ground_energy_dst_  = visual_canvas_.layout().resolve("ground.energy");
                ground_density_dst_ = visual_canvas_.layout().resolve("ground.density");
                std::fprintf(stderr,
                    "[the_board] fog.density base=%d valid=%d | fog.color base=%d count=%d valid=%d\n",
                    fog_density_dst_.base, (int)fog_density_dst_.valid,
                    fog_color_dst_.base, fog_color_dst_.count, (int)fog_color_dst_.valid);
                std::fprintf(stderr,
                    "[the_board] terrain.checker_mean base=%d count=%d valid=%d | terrain.checker_var base=%d valid=%d\n",
                    checker_mean_dst_.base, checker_mean_dst_.count, (int)checker_mean_dst_.valid,
                    checker_var_dst_.base, (int)checker_var_dst_.valid);
                std::fprintf(stderr,
                    "[the_board] cube.light base=%d count=%d valid=%d | choir %u key(s), %u rank(s)\n",
                    cube_light_dst_.base, cube_light_dst_.count, (int)cube_light_dst_.valid,
                    CUBE_CHOIR_N, CUBE_CHOIR_RANKS);
                std::fprintf(stderr,
                    "[the_board] ground.energy base=%d valid=%d | ground.density base=%d valid=%d"
                    " | gains %.2f lift, %.2f quicken\n",
                    ground_energy_dst_.base,  (int)ground_energy_dst_.valid,
                    ground_density_dst_.base, (int)ground_density_dst_.valid,
                    (double)DRIVER_LIVE.ground.height_gain,
                    (double)DRIVER_LIVE.ground.tick_gain);
            }

            // ═══════════════════════════════════════════════════════════════
            // THE FRAME SPINE — the program's
            // temporal dispatch table.
            //
            // The frame is an AUTHORED order, CHECKED by validation, never
            // COMPUTED. update() and render() are LOOPS over two constexpr spine
            // tables (UPDATE_SPINE / RENDER_SPINE, below the phase methods); each
            // row is {phase id, name, member fn, driver (§9), roster gate, face
            // tags}. The row order IS the frame order; the O-# / RC laws are
            // static_asserts over the row indices (see § SPINE VALIDATION). No
            // topo-solver — the deliberate stale-reads and write-order designs
            // are LAWS, declared here (recon §2):
            //
            //   LAW E-4 (witness lag): the readback is 1 frame stale BY DESIGN.
            //     R1 WitnessHarvest consumes the capture R10 WitnessCapture wrote
            //     LAST frame (O-2). Player pos / owner mirrors
            //     all lag one frame — every downstream consumer is written to
            //     eat a one-frame-old point.
            //   E-9 (portal spans a frame) — DEAD WITH ITS SUBJECT
            //     (ONE_WORLD-I). It described the two-row handshake by which a
            //     GPU-reported portal trigger armed the transition machine on
            //     the NEXT update. Both rows left; nothing spans a frame here
            //     any more.
            //   E-3 (sky write-order) — DEAD WITH ITS SUBJECT (RIBBON_1). It
            //     was a three-writer relay over a POSE the ribbon tick had to
            //     re-write after the drain; the split drain that mechanized it
            //     was the second cure. The pose is the GPU's now
            //     (ribbon_body_read.saddle) and the trailing words carry only
            //     the mount EDGE, which the signal's one author fills like any
            //     other word. One writer, one whole-struct write, no ordering to
            //     preserve and nothing left to say.
            //
            // Gates are ROW COLUMNS: a disabled family's row is skipped at
            // runtime (row.enabled folds from its constexpr ROSTER bit). Runtime
            // data-guards (dirty flags) live INSIDE their phase.
            // ═══════════════════════════════════════════════════════════════

            // Frame-transient inputs, bundled so every phase has ONE uniform
            // signature (the row's fn type). A phase reads only what it needs.
            struct UpdateCtx {
                const AnalysisSignal& signal;
                float                 aspect_ratio;
                wgpu::Queue& queue;
                GPUFrameSignal& gpuSignal;   // U1 fills it; U8 drains it (sky_* excluded — E-3 mechanized)
            };
            struct RenderCtx {
                wgpu::CommandEncoder& encoder;
                wgpu::Queue& queue;
                wgpu::TextureView     backbuffer;
                wgpu::TextureView     msaaColor;   // B10: null at msaa=1
                wgpu::TextureView     depth;
            };

            // §9 driver law (input.hpp:102 — a driver writes intents through
            // bodies it does not own). None = foundational spine work (no bit).
            enum class Driver : uint8_t { Input, Algo, Music, WallClock, Mixed, None };

            // Coarse face tags — the frame-truth axes a phase touches (recon §3).
            enum FaceTag : uint32_t {
                F_NONE = 0,
                F_SIGNAL = 1u << 0,   // the GPU signal buffer (clock/input/stats)
                F_CONFIG = 1u << 1,   // the GPU config buffer (fog/world/bands/...)
                F_CLOCK = 1u << 2,   // time_state_ (beats/seconds/dt/prev_beats)
                F_WITNESS = 1u << 3,   // the readback record (agent/floater/camera)
                F_COMPUTE = 1u << 5,   // encodes a GPU compute pass
                F_DRAW = 1u << 6,   // encodes a GPU render pass
                F_SUBMIT = 1u << 7,   // issues its OWN queue submit (hidden)
                F_STREAM = 1u << 9,   // patch streaming (S2)
            };

            // Phase ids — DECLARATION ORDER == AUTHORED ORDER == ROW INDEX.
            // (The spine tables are asserted dense + in this order; the O-#/RC
            //  laws are static_asserts over these indices.)
            enum class UPhase : uint32_t {
                FillSignal, AdvanceClock, MotionDrivers, MotionBodies,
                StageWorld, StageUpload,
                ClearInputDeltas, COUNT
            };
            enum class RPhase : uint32_t {
                WitnessHarvest, SurfaceVisibility,
                CensusDumps, RibbonTick, EntityMeshGen, UploadLights, LiveCardWrite, DispatchCompute,
                WitnessCapture, AutomatonStep, PawnAura, OrbSky,
                FrustumCull, ShadowPass, MainPass,
                COUNT
            };

            // Row shapes (the FAMILY_DISPATCH shape, one clock per conductor).
            struct URow {
                UPhase                          id;
                const char* name;
                void (Cartridge::* fn)(UpdateCtx&);
                Driver                          driver;
                bool                            enabled;   // roster gate (constexpr-folded)
                uint32_t                        face;
            };
            struct RRow {
                RPhase                          id;
                const char* name;
                void (Cartridge::* fn)(RenderCtx&);
                Driver                          driver;
                bool                            enabled;
                uint32_t                        face;
            };

            // U1 — SIGNAL FILL (music+input+wall-clock). Build the GPU signal
            // from analysis + input. O-5a: dt_beats reads prev_beats BEFORE the
            // clock advances it at U3. Input deltas were harvested by on_input.
            void phase_fill_signal(UpdateCtx& c) {
                auto& gpuSignal = c.gpuSignal;
                auto& signal = c.signal;
                auto aspect_ratio = c.aspect_ratio;
                gpuSignal.t_seconds = signal.t_seconds;
                gpuSignal.t_beats = signal.t_beats;
                // RIBBON_2 P0 1.2b — THE PENDING dt (RIBBON_3). The signal is
                // written every update; the compute pass that consumes it is
                // encoded only on a RENDERED frame. Writing signal.dt here
                // meant a dropped acquire's dt was overwritten by the next
                // update and DELETED from the GPU's integrators. It
                // accumulates instead, and is cleared by frame_submitted().
                // The same 100 ms ceiling the raw measurement carries applies
                // to the sum: a stretch is a stretch, a teleport is not.
                dtPending_ = std::min(dtPending_ + signal.dt, 0.1f);
                gpuSignal.dt = dtPending_;
                gpuSignal.aspect_ratio = aspect_ratio;

                gpuSignal.move_x = inputState_.move_x;
                gpuSignal.move_z = inputState_.move_z;
                gpuSignal.look_az_delta = inputState_.look_az_delta;
                gpuSignal.look_el_delta = inputState_.look_el_delta;
                gpuSignal.zoom_delta = inputState_.zoom_delta;
                gpuSignal.pan_x_delta = inputState_.pan_x_delta;
                gpuSignal.pan_y_delta = inputState_.pan_y_delta;
                gpuSignal.dt_beats = signal.t_beats - time_state_.prev_beats;  // beats since last frame -> step_trigger

                // THE MOUNT BLOCK (RIBBON_1) — ship the edge, then advance the
                // ease. Shipping FIRST is the point: the frame the host changed
                // on must reach the GPU at phase 0, or the body starts the
                // trajectory already partway along it. possess() authored the
                // edge; this is its only carrier; nothing reads it back.
                gpuSignal.mount_phase        = mount_.phase;
                gpuSignal.mount_kind         = mount_.kind;
                gpuSignal.mount_from[0]      = mount_.from[0];
                gpuSignal.mount_from[1]      = mount_.from[1];
                gpuSignal.mount_from[2]      = mount_.from[2];
                gpuSignal.mount_from_heading = mount_.from_heading;
                if (mount_.kind != 0u) {
                    const float secs = (mount_.kind == 1u) ? RIBBON_LIVE.board_seconds
                                                           : RIBBON_LIVE.land_seconds;
                    mount_.phase += signal.dt / (secs > 1e-3f ? secs : 1e-3f);
                    if (mount_.phase >= 1.0f) { mount_.phase = 1.0f; mount_.kind = 0u; }
                }

                // Possessed body's tilt lag rides the config's slow-dial cadence
                // (CLOSURE_PAWN [6]). Idempotent: set_pawn_tilt_tau only dirties on a
                // real change, so the per-frame call costs nothing while the figure
                // stays put.
                //
                // NOT A GATED BLOCK — the braces are scope, not a condition, and
                // no enclosing one exists. All three fields below are authored on
                // every frame the update spine runs: this is UPDATE_SPINE[0]
                // (FillSignal) behind a literal-true roster gate, upload_config
                // rides UPDATE_SPINE[5] (StageUpload) behind another, and the
                // agent kernels dispatch later still from the RENDER spine. No
                // dispatch can therefore read a value this block did not author,
                // and config_{}'s zero-init is unreachable by the GPU — the same
                // ordering the fpv_eye_height rest pin in initialize() already
                // states. Recorded here because for pawn_body_radius a zero is
                // not a degraded margin, it IS HEM_0's defect, and the next
                // reader should not have to re-derive that it cannot happen.
                {
                    const uint32_t sid = agent_state_.slots[player_.possessed_slot].skin_id;
                    gpuState_.set_pawn_tilt_tau(
                        sid < PAWN_FIGURE_COUNT ? PAWN_FIGURES[sid].tilt_tau : 0.0f);

                    // The possessed figure's own radius, on the same wire and
                    // the same guard (HEM_0). The boundary clamp insets the
                    // legal box by this, so the body — not its centre — stops
                    // at the wall, and the sample never lands on bmax, the
                    // EXCLUSIVE edge of the patch set. Derived here for the
                    // same reason the eye height below is: the compute stage
                    // cannot see scene_constants.figure_profiles.
                    // The out-of-range arm reads figure 0's radius rather than a
                    // literal: pawn_figures.hpp's static_assert proves every ROW
                    // of the table positive, and a bare 0.0f would sit outside
                    // that proof's reach — the guard belongs inside the thing
                    // that makes the law true.
                    gpuState_.set_pawn_body_radius(
                        sid < PAWN_FIGURE_COUNT ? PAWN_FIGURES[sid].radius
                                                : PAWN_FIGURES[0].radius);

                    // FPV eye height follows the possessed figure (TUNE_1 A3).
                    // Derived here and not in the shader because
                    // scene_constants.figure_profiles rides a render-VS-only
                    // uniform block (g2:200) — update_camera_vp's compute layout
                    // does not carry it, and giving it one would be a new
                    // binding. Same
                    // out-of-range fallback as the tilt above, so an unknown
                    // skin lands on the conventional figure rather than at
                    // ground level. Guarded like set_pawn_tilt_tau: the config
                    // only dirties when the possessed figure actually changes.
                    const float eye = FPV_EYE_RATIO
                        * (sid < PAWN_FIGURE_COUNT ? PAWN_FIGURES[sid].height
                                                   : PAWN_FIGURES[0].height);
                    if (gpuState_.config().fpv_eye_height != eye) {
                        gpuState_.config().fpv_eye_height = eye;
                        gpuState_.mark_config_dirty();
                    }
                }
            }

            // U3 — ADVANCE CLOCK (music+wall-clock). The tempo follower; bumps
            // prev_beats (the O-5a partner of U1's dt_beats read).
            void phase_advance_clock(UpdateCtx& c) {
                auto& signal = c.signal;
                time_state_.beats = signal.t_beats;
                time_state_.seconds = signal.t_seconds;
                time_state_.dt = signal.dt;
                // The frame's third clock (PANORAMA_1) — advanced HERE, with
                // the other two, so there is one place a frame begins.
                world_state_.frame_index++;
                {
                    const float db = signal.t_beats - time_state_.prev_beats;
                    if (db > 1e-6f && time_state_.dt > 1e-6f)
                        time_state_.beat_rate = db / time_state_.dt;
                    time_state_.prev_beats = signal.t_beats;
                }
            }

            // U4 — MOTION DRIVERS (music). The music driver authors params
            // through the canvas; fog is its first staged consumer. (Input was
            // harvested by the on_input callbacks; its deltas rode U1.)
            // ── FIELD_4: THE BEACON (the first authored emitter) ──
            // The four dials moved to the panel
            // (contracts/control_panel.hpp), where the S < FIELD_K
            // self-spacing ruling compiles as a static_assert beside
            // them — the writer below reads them from there (FIELD_2a).

            void phase_motion_drivers(UpdateCtx& c) {
                auto& signal = c.signal;
                visual_canvas_.tick(signal);
                // ORGAN — the drivers' room sits at this seam:
                // out = rest + gain·deviation. The REST is ATMOS_LIVE's,
                // drawn per world into sky_state_.fog_rest_* by
                // stage_sky; the DEVIATION is the canvas's,
                // measured from its anchor row. Gain 1 is the coupling
                // verbatim, gain 0 is the bank's own fog, and with no
                // bindings the rest alone speaks — so the dial works
                // headless too.
                //
                // set_fog GUARDS — it compares all four lanes and dirties
                // only on a change — so both arms call it unconditionally
                // and the silent case costs no dirty.
                {
                    const auto& drv = DRIVER_LIVE.fog;
                    const auto& ms  = sky_state_;
                    if (fog_density_dst_.valid && fog_color_dst_.valid) {
                        const VisualParams& fp = visual_canvas_.params();
                        gpuState_.set_fog(
                            std::max(0.0f, ms.fog_rest_density + drv.gain * fp.get(fog_density_dst_.base)),
                            std::clamp(ms.fog_rest_color[0] + drv.gain * fp.get(fog_color_dst_.base + 0), 0.0f, 1.0f),
                            std::clamp(ms.fog_rest_color[1] + drv.gain * fp.get(fog_color_dst_.base + 1), 0.0f, 1.0f),
                            std::clamp(ms.fog_rest_color[2] + drv.gain * fp.get(fog_color_dst_.base + 2), 0.0f, 1.0f));
                    } else {
                        gpuState_.set_fog(ms.fog_rest_density, ms.fog_rest_color[0],
                                          ms.fog_rest_color[1], ms.fog_rest_color[2]);
                    }
                }
                // CHECKER-REBUILD: the pc-color field's flush — one setter,
                // the fan (resultant rgb + music amount + music variance).
                // ORGAN — the drivers' room sits at this seam too:
                // out = rest + gain·(driven − rest), the fog recipe verbatim.
                // Gain 1 is the coupling byte-for-byte; gain 0 is the rest,
                // which terrain_looks calls law — amount 0 returns each cell
                // to its SEED colour, not to gray. With no bindings the rest
                // alone speaks, so the dial works headless.
                //
                // The resultant is a run of three, so it blends per lane
                // against rest_resultant[] rather than against one scalar.
                if (checker_mean_dst_.valid && checker_var_dst_.valid) {
                    const VisualParams& cp = visual_canvas_.params();
                    const auto& ck = DRIVER_LIVE.checker;
                    const float* mean = cp.run(checker_mean_dst_.base);
                    const float blended[3] = {
                        ck.rest_resultant[0] + ck.gain * (mean[0] - ck.rest_resultant[0]),
                        ck.rest_resultant[1] + ck.gain * (mean[1] - ck.rest_resultant[1]),
                        ck.rest_resultant[2] + ck.gain * (mean[2] - ck.rest_resultant[2]),
                    };
                    gpuState_.set_checker_color_field(blended,
                        ck.rest_amount   + ck.gain * (cp.get(checker_var_dst_.base)     - ck.rest_amount),
                        ck.rest_variance + ck.gain * (cp.get(checker_var_dst_.base + 1) - ck.rest_variance));
                    // [FLUSH] one-shot: fires the first time a live resultant
                    // crosses the CPU->GPU seam. If [CHECKER] is singing in the
                    // console and this line never prints, the bindings above are
                    // invalid or the two params_ objects disagree — name it.
                    static bool checker_flush_seen = false;
                    if (!checker_flush_seen
                        && cp.get(checker_var_dst_.base) > 0.05f) {   // music_amount up
                        std::fprintf(stderr,
                            "[FLUSH] checker -> config: resultant=(%.2f %.2f %.2f) amount=%.2f var=%.2f\n",
                            cp.get(checker_mean_dst_.base),
                            cp.get(checker_mean_dst_.base + 1),
                            cp.get(checker_mean_dst_.base + 2),
                            cp.get(checker_var_dst_.base),
                            cp.get(checker_var_dst_.base + 1));
                        checker_flush_seen = true;
                    }
                } else {
                    // No bindings: the rest alone speaks, so the dials still
                    // reach the picture with the music silent — the fog
                    // seam's headless arm again.
                    // set_checker_color_field guards, so this costs no dirty.
                    const auto& ck = DRIVER_LIVE.checker;
                    gpuState_.set_checker_color_field(ck.rest_resultant,
                                                      ck.rest_amount, ck.rest_variance);
                }

                // ── THE GROUND'S VOICE (GROUND_VOICE_0 U2) ──────────────
                // The automaton already carried two multipliers on config
                // and both were PINNED NEUTRAL at boot, at one call site,
                // waiting for a driver. This is that driver, and it adds
                // no mechanism: two ruled expressions and the setter that
                // was already there.
                //
                //   height_mul = clamp(1 + height_gain · energy,   0.25, 4)
                //   tick_mul   = clamp(1 / (1 + tick_gain · dens), 0.25, 4)
                //
                // THE RECIPROCAL IS THE POINT of the second: tick_scale
                // multiplies the automaton's tick PERIOD, so SMALLER IS
                // FASTER — denser music, faster life. Getting that
                // backwards would be legal C++, legal WGSL, and visible
                // only on the device.
                //
                // THE FIELD HAS THREE READERS AND ONLY ONE OF THEM RUNS
                // ON THIS GROUND. Two are in world.wgsl's
                // `pulse_cell_target` (the PULSE field's per-cell target,
                // with divisor floors of 0.01 and 0.1 — deliberately not
                // the same number, and neither is this seam's clamp); the
                // bank boots CONWAY, so both are dormant. The one that
                // runs is `upload_automaton_header` in surface/
                // automaton.hpp, the CPU's own step gate, which
                // GROUND_VOICE_0 taught to read it — see that site for
                // why the coupling would otherwise have been inert.
                //
                // GAIN 0 IS HANDS OFF, PER TERM. Every other seam composes
                // a driven value against a rest that lives somewhere else,
                // so it can write unconditionally at any gain. These two
                // rests ARE the driven fields — config's mode_gol_*_scale
                // are boot-pinned to 1.0 and are still WRITABLE organ
                // dials — so writing unconditionally would overwrite the
                // dial every frame and quietly kill it. At gain 0 the term
                // therefore passes the dial's own value straight back, the
                // setter's inequality gate sees no change, and the dial is
                // the author again. "0 manual … 1 coupling verbatim",
                // meant literally.
                //
                // The setter gates on inequality (state.hpp
                // set_mode_gol_scales), so this runs every frame and a
                // still room costs no dirty. NOTE THE ARGUMENT ORDER:
                // (tick, height) — the two are same-typed and both rest at
                // 1.0, so a swap compiles and only the device sees it.
                {
                    const auto& g   = DRIVER_LIVE.ground;
                    const auto& cfg = gpuState_.config();
                    const VisualParams& gp = visual_canvas_.params();
                    float tick_mul   = cfg.mode_gol_tick_scale;
                    float height_mul = cfg.mode_gol_height_scale;
                    if (g.height_gain != 0.0f && ground_energy_dst_.valid) {
                        const float energy = gp.get(ground_energy_dst_.base);
                        height_mul = std::clamp(1.0f + g.height_gain * energy,
                                                GROUND_SCALE_MIN, GROUND_SCALE_MAX);
                    }
                    if (g.tick_gain != 0.0f && ground_density_dst_.valid) {
                        const float dens = gp.get(ground_density_dst_.base);
                        tick_mul = std::clamp(1.0f / (1.0f + g.tick_gain * dens),
                                              GROUND_SCALE_MIN, GROUND_SCALE_MAX);
                    }
                    gpuState_.set_mode_gol_scales(tick_mul, height_mul);
                }

                // THE CHOIR (CHOIR_0 U4): the canvas envelopes one light
                // per key; this seam composes it against the drivers' room
                // and MIRRORS the result into the cube body's own state.
                // ONE AUTHOR PER FRAME, here — the mirror is the prior, so
                // the body file's readers (the newborn's dress, the swell,
                // the projector) never reach into the coupling layer.
                //
                // The recipe is the fog's and the checker's verbatim,
                // out = rest + gain·(driven − rest), with the rest DARK:
                // rest 0 collapses it to gain·I, and gain 0 is a dark
                // instrument wearing its seed draws exactly.
                //
                // THE TWO WIDTHS MEET HERE and nowhere else — the canvas
                // owns the pipe's, the body owns the population's, and
                // this is the one room that can see both.
                static_assert((int)CUBE_CHOIR_N <= CHOIR_LANES,
                    "the choir has more keys than the cube.light pipe has lanes");
                {
                    const float g = DRIVER_LIVE.cube.gain;
                    if (cube_light_dst_.valid) {
                        const VisualParams& lp = visual_canvas_.params();
                        for (uint32_t k = 0; k < CUBE_CHOIR_N; ++k)
                            cube_behaviors_state_.choir_I[k] = g * lp.get(cube_light_dst_.base + (int)k);
                    } else {
                        // No binding: the rest alone speaks, and the rest
                        // is dark — the fog seam's headless arm again.
                        for (uint32_t k = 0; k < CUBE_CHOIR_N; ++k)
                            cube_behaviors_state_.choir_I[k] = 0.0f;
                    }
                }
                // The projector's own home, poke-on-change: only a key
                // whose light MOVED reaches the GPU.
                //
                // THE ORDER'S HAZARD IS GONE (WHEEL_0 U3). It used to
                // matter that the projector ran before THE CLIMB, because
                // the climb walked body_radius on the CPU and the swell
                // wrote it too — two writers on one scalar in one frame.
                // The climb retired with the walk, so the projector is
                // the only writer of body_radius outside the birth and
                // these two calls no longer contend for anything: one
                // authors LOOK (colour, variance, swell) and the other
                // authors GOALS (glide targets). The order is now free,
                // and is kept as written only because there is no reason
                // to move it.
                choir_project(cube_behaviors_state_, gpuState_, c.queue,
                    world_state_.active_seed);
                // THE WHEEL'S SERVE — the formation machine, entire. It
                // kept a musical clock and a world seed for the lattice
                // and the flush (both left at CHOIR_0 U5), then dt and
                // the point mirror for the climb and the reseat watch
                // (both left at WHEEL_0 U3). It reads the panel and the
                // choir's own state now, and takes neither.
                zoetrope_service(cube_behaviors_state_, gpuState_, c.queue);

                // ── THE BEACON (FIELD_4): row 0, rewritten hot each
                // frame — the point moved. point y is DERIVED (the
                // point's house carries no y): host-routed — pawn
                // mirror y (PAWN and RIBBON alike — the possessed body IS
                // where the point is in both, riding the seat in one and
                // walking in the other) / ground under the point in
                // camera-host (the camera has no CPU y mirror; the harvest
                // discards cam pos[1]).
                {
                    GPUFieldAuthored fa{};
                    const float coord = gpuState_.config().floater_coordination;
                    float py;
                    if (point_.host != PointHost::CAMERA) {
                        py = agent_state_.slots[player_.possessed_slot].pos_y;
                    } else {
                        py = estimate_terrain_height(tile_world_state_, point_.x, point_.z);
                    }
                    // ORGAN — the beacon reads its BANK, PANEL_LIVE, and
                    // not the design table: a bank nothing reads is a dial
                    // that writes nothing.
                    const auto& bcn = PANEL_LIVE.beacon;
                    // THE RING SELF-SPACES, AT RUNTIME TOO.
                    // control_panel.hpp's static_assert proves the AUTHORED
                    // pair; this clamp guards the DIALED one, reading
                    // config's LIVE field_k rather than the constexpr
                    // because field_k is a dial too and lowering it breaks
                    // the same ruling from the other end. It sits at the
                    // writer, not at the panel, so every author is
                    // guarded by it.
                    const float ceiling = gpuState_.config().field_k - 1.0f;
                    float s = bcn.s;
                    if (s > ceiling) s = ceiling;
                    if (s < 0.0f)    s = 0.0f;
                    fa.count = 1u;
                    fa.rows[0][0] = point_.x;
                    fa.rows[0][1] = py + bcn.lift;
                    fa.rows[0][2] = point_.z;
                    fa.rows[0][3] = s * coord;
                    fa.rows[1][0] = bcn.r0;
                    fa.rows[1][1] = bcn.r;
                    fa.rows[1][2] = (coord > 0.0f) ? 1.0f : 0.0f;
                    gpuState_.upload_field_authored(c.queue, fa);
                }
            }

            // U5 — MOTION BODIES (wall-clock). Pawn presence ramp + aura height
            // (bodies/pawn.hpp real-time exponential tick; closes pawn:K1).
            // ROSTER-GATE pawn_aura (b) — guarded at the call site.
            void phase_motion_bodies(UpdateCtx& c) {
                auto& queue = c.queue;
                tick_pawn_couplings(pawn_state_, &pawn_deps_, queue);
            }

            // U6 — STAGE WORLD (seed + finite bounds, algo). Stays PRE-machine
            // (RC policy): the TEARDOWN case re-stages the seed itself, and
            // moving the bounds after the machine would ship the NEW world's
            // bounds one frame early on the teardown frame.
            // THE BOX, ONCE (HEM_1). phase_stage_world derived these two
            // floats inline and handed them to the GPU; the agents'
            // placement needs the same pair on the CPU side, and a second
            // derivation is how two rooms start disagreeing about where the
            // wall is. Returns (0,0) in an infinite world — the SAME
            // all-zero sentinel world_box_clamp_xz reads as "no bounds", so
            // the two rooms share one convention as well as one number.
            // (A third copy lives at surface/patch_system.hpp's patch walk;
            // it is out of this helper's reach — patch_system.hpp is
            // included AFTER bodies/agents.hpp in the cohort — and is left
            // alone.)
            std::pair<float, float> world_box_() const {
                if (!world_state_.finite_mode) return {0.0f, 0.0f};
                return { -(float)world_state_.finite_radius * Dim::PATCH_EXTENT,
                         ((float)world_state_.finite_radius + 1.0f) * Dim::PATCH_EXTENT };
            }

            void phase_stage_world(UpdateCtx&) {
                gpuState_.set_world_seed(world_state_.active_seed);
                {
                    const auto [bmin, bmax] = world_box_();
                    gpuState_.set_world_bounds(bmin, bmin, bmax, bmax);
                }
                // THE VEIL'S STRENGTH was staged here, 0 in a finite world.
                // It left at ONE_SURFACE-I U4 with the icing it scaled — the
                // fold table found every reader of it multiplying by zero.
                // The RING is untouched and is not the veil's: it is the draw
                // authority, and it still culls inside the wall.
            }

            // L10 — A WORLD BECOMES THE WORLD THROUGH ONE DOOR. Boot walks
            // it from nothing; rebirth_world walks it again with a fresh
            // seed. The three facts ARE the world's identity: which seed
            // drew it, whether it is walled, how far the walls stand
            // (ATRIUM_0: finite_mode and finite_radius were in-struct
            // defaults at boot, correct by luck while the boot world was
            // open). The radius is DERIVED from the seed within the pin's
            // dials, never authored: DemoConfig does not grow, and its
            // parked D2 axis stays parked. Boot is a birth from nothing
            // in fact, not in doctrine.
            void become_world(uint32_t seed) {
                world_state_.active_seed   = seed;
                world_state_.finite_mode   = WORLD_FINITE;
                world_state_.finite_radius = derive_finite_radius(seed);
                // SMALL AND LOUD (ONE_WORLD-II U5). The pin is one
                // constant and one line: a world that is bounded says so,
                // once, at the door both births walk. The side is the
                // patch count across the box — 2r+1 — which is the number
                // a walk can actually count.
                const uint32_t side = 2u * world_state_.finite_radius + 1u;
                // THE WITNESS SAYS WHICH IT WAS. The radius is authored
                // while the range is pinned shut and drawn while it is not,
                // and a boot line that could not tell them apart would make
                // the stage law unfalsifiable from the log.
                const bool pinned = (WORLD_LIVE.radius_min >= WORLD_LIVE.radius_max);
                std::cout << "[World] Born FINITE radius=" << world_state_.finite_radius
                          << " (" << side << "x" << side << " patches)"
                          << (pinned ? " [PINNED]" : " [drawn]") << "\n";
            }

            // THE REBIRTH (ONE_WORLD-I — graduated whole from the transition
            // machine's TEARDOWN arm, which left with the doors). One verb,
            // one world: the SAME fixed sequence the machine ran (O-3),
            // minus the fade and the portal choreography that had no subject
            // left. It owns what the arm owned — the worldGen bump (P5
            // stale-callback guard), the per-owner teardown verbs, the
            // authored present, the agent reset, the repopulation.
            //
            // A REBIRTH IS A HARD CUT. The machine crossfaded through
            // FADE_OUT/TEARDOWN/FADE_IN; there is no fade and no phase to
            // hang one on. FLAGGED for Jean's visual gate.
            //
            // SPINE-RESIDENT, as the machine was (K4): every line reaches a
            // root organ, so the verb IS the assembly and cannot live
            // anywhere else without a deps face wider than the root itself.
            //
            // SEAM[spine:P8] — RETIRED, THE CUE ANSWERED (THE_PANEL I
            // U1). The mark stood here for two campaigns: EXPLICIT LATENT
            // INFRASTRUCTURE, the same category PlayerState's deferred
            // fields carry (contracts/spine_state.hpp), zero callers,
            // MARKED and not dead, because a zero-caller verb may not
            // stand UNMARKED.
            //
            // AND THE FORWARD CUE CAME TRUE VERBATIM. It read: "The
            // caller is THE PANEL'S SEED DIAL — cadence GEN (once per
            // rebirth, never per frame), C3-DESTRUCTIVE (it tears the
            // standing world down before it re-draws one) — and it
            // arrives with the PANEL campaign. The six keys that used to
            // ignite a transition are gone and are not coming back; the
            // dial is the door that replaces them."
            //
            // What arrived is that sentence and nothing else: block 15,
            // `WORLD_LIVE.next_seed` (contracts/world_surface.hpp), a GEN
            // row over a bank with no boundary wiring, and
            // ORGAN_DOOR_REBIRTH pressing THIS verb at the frame boundary
            // (organ_boundary.inc). The verb did not move a line to
            // receive its caller, which is the whole claim the category
            // was making.
            //
            // THE TOMBSTONE IS THE POINT, so the mark is struck rather
            // than deleted (L41): a P8 is a bet that a named future
            // caller will arrive, and this is the file's evidence that
            // the bet paid. The FIRST P8 above — PlayerState's deferred
            // fields — is untouched and still latent, still waiting on
            // the unified entity layer.
            //
            // BOOT-AS-CALLER IS REFUSED, NOT DEFERRED. Boot and mid-run
            // rebirth are different operations sharing one door: a birth
            // from nothing seeds slot 0 over pristine organs and has
            // nothing to tear down; a rebirth tears the world down first
            // and then zeroes the whole slot array on BOTH sides of the
            // mirror (reset_player_agent writes a zeroed Dim::MAX_AGENTS
            // buffer; reseed_player_body memsets AgentState::slots
            // whole) before re-authoring slot 0. One wrapper over both
            // is false unification — a mode flag, or changed boot
            // behaviour, and both lose to the PRIME INVARIANT's
            // byte-for-byte boot order, and to the ROSTER gates that
            // exist to eliminate exactly the zero-writes a boot-side
            // teardown would re-introduce. become_world above is the L10
            // door the two operations DO share, and it is the whole of
            // what they share.
            //
            // THE CHAIN THAT WAS LATENT WITH IT. Each verb below has
            // exactly one call site tree-wide and it is inside this body.
            // Until U1 that made each of them latent WITH the seam, read
            // as MARKED by this note rather than as an orphan a sweep may
            // take; the door gave all ten a live caller in one press, so
            // the list is now an ownership map and not a shield. It is
            // KEPT because the single-call-site fact is what makes this
            // body the whole of their lifetime, which is still what a
            // sweep needs to know:
            //   teardown_entities (bodies/grounded.hpp)
            //   teardown_automaton (surface/automaton.hpp)
            //   teardown_ribbon, release_finite_ribbons (bodies/ribbon.hpp)
            //   clear_spheres (bodies/spheres.hpp)
            //   clear_cubes (bodies/cube_behaviors.hpp)
            //   teardown_pawn_aura (bodies/pawn.hpp)
            //   teardown_orbs (bodies/orbs.hpp)
            //   GPUState::reset_player_agent (realization/state.hpp)
            //   reseed_player_body (bodies/agents.hpp)
            // One hop deeper: GPUState::upload_automaton_config has TWO
            // callers and only one is in the chain — teardown_automaton
            // and birth_automaton (at every world's birth) — which is why
            // it never read as latent even while the chain did.
            // That changed at ONE_SURFACE-II U1: the zones' equivalent
            // (upload_zone_config) was teardown's alone, because the zones
            // were born a slot at a time through a derive queue rather
            // than all at once with the world. Its near-twin
            // upload_automaton_header is LIVE, per frame, from the
            // automaton's spine row.
            // NOT LATENT, though this body calls them: become_world,
            // reset_surface, stage_world_birth, spawn_population,
            // dump_agent_census, dump_entity_census, set_world_seed,
            // set_possessed_slot — every one has a live caller at boot or
            // in the frame.
            //
            // THE CHAIN IS GATE-HELD, not merely tolerated. The score
            // census (tools/gates/score/run.py, Direction A) names six
            // of them — the sphere/ribbon/cube/pawn_aura/orbs teardowns
            // and the ribbon finite release — and asserts each roster
            // bit's gated call site in THIS file. Those sites are the ones
            // below. (The seventh was the GoL teardown; the ground took
            // its place at ONE_SURFACE-II U1 and is UNGATED, for the
            // reason given at its line.) Deleting the seam takes the
            // roster/spine bijection with it, which is why the graduation
            // kept the machine's ordering knowledge as living code instead
            // of attic archaeology (L30).
            //
            // MAINTAINED, NOT FROZEN — latency was not exemption, and the
            // rule outlived the latency it was written for. Campaigns
            // treated this body as a first-class reader while it had no
            // caller: its bank reads rewired like any live caller's, its
            // narration rode probate, twin-room discipline applied. That
            // discipline is why a door could be pointed at it with no
            // repair round first.
            //
            // THE VOICE IS SPOKEN NOW. The [World] Rebirth complete line
            // at the tail was the honest voice for a caller that was
            // coming; the caller is here and the line prints for a living
            // reason. It still never prints at boot, because boot never
            // enters here — which is what makes it a REBIRTH witness and
            // not a birth one.
            void rebirth_world(uint32_t seed, wgpu::Queue& queue) {
                // ═══ THE FIXED SEQUENCE (O-3) ══════════════════════
                // SEAM[spine:P5] world_state_.world_gen++ at the top is the
                //   stale-callback guard (P5 family). Genuinely spine-owned.
                world_state_.world_gen++;
                // OPT_1a's "the new world's rest field must be written once"
                // reset stood here. The card is written every frame, so the
                // first frame of a new world writes it like any other.
                // THE FIRST-CAPTURE GATE (POINT_1, the measured seam):
                // the harvest closures bind their gen at MAP time, so a
                // copy STAGED in the old world (state COPIED) and
                // mapped after this ++ would deliver old bytes under
                // the fresh gen, passing the guard — the far first
                // arrivals in Jean's log. Cancel stale staged copies
                // here; a MAPPING machine is left alone: its callback
                // bound the OLD gen and drops itself, and forcing it
                // IDLE would let the poll double-map an in-flight
                // buffer.
                if (pawnReadbackState_ == PawnReadbackState::COPIED)
                    pawnReadbackState_ = PawnReadbackState::IDLE;
                if (floaterReadbackState_ == FloaterReadbackState::COPIED)
                    floaterReadbackState_ = FloaterReadbackState::IDLE;
                if constexpr (INSTRUMENTS.camera_witness) {   // ATRIUM_11 — same cancel
                    if (cameraReadbackState_ == CameraReadbackState::COPIED)
                        cameraReadbackState_ = CameraReadbackState::IDLE;
                }

                become_world(seed);   // L10 — the one door

                // The surface core first, then one teardown verb per
                // owner. The per-organ clears are independent (each
                // touches only its organ + its own GPU slots), so the
                // owner-verb order is free; the new gates eliminate
                // only zeros-over-pristine GPU writes (disclosed at
                // the ladder).
                reset_surface(&machine_ctx_, queue, tile_world_state_);
                teardown_entities(&machine_ctx_, queue);
                // UNGATED (ONE_SURFACE-II U1): the ROSTER-GATE that stood
                // here skipped the clear when the GoL FAMILY was disabled.
                // The ground has no such switch, and P8's law is explicit
                // that latency is not exemption — the verb runs on the
                // teardown path whether or not a caller exists yet.
                teardown_automaton(automaton_state_, &automaton_deps_, queue);
                if constexpr (ROSTER.ribbon)   // ROSTER-GATE ribbon (c) — same zero-write elimination
                    teardown_ribbon(ribbon_state_, &ribbon_deps_, queue);
                if constexpr (ROSTER.sphere)   // ROSTER-GATE sphere (c)
                    clear_spheres(sphere_state_, gpuState_, queue);
                if constexpr (ROSTER.cube)     // ROSTER-GATE cube (c)
                    clear_cubes(cube_behaviors_state_, gpuState_, queue);
                if constexpr (ROSTER.pawn_aura)  // ROSTER-GATE pawn_aura (c) — teardown clear skipped when disabled (no aura to clear)
                    teardown_pawn_aura(pawn_state_);
                // Sky orbs: stage_world_birth re-enables + re-seeds as needed
                if constexpr (ROSTER.orbs)  // ROSTER-GATE orbs (c) — teardown one-shot skipped when disabled
                    teardown_orbs(orbs_state_, &orbs_deps_);

                // THE AUTHORED PRESENT (POINT_1): at a rebirth the
                // CPU is the author of the new present — the same
                // position reset_player_agent / reseed_player_body
                // write below. Idle::PAWN_POS is (0,0) today, so the
                // old zero reset was this value BY LUCK; naming it
                // makes the equality enforced (the reset_surface
                // precedent), and every streaming consumer that runs
                // before the first fresh harvest reads the true point.
                point_.x = Idle::PAWN_POS_X;
                point_.z = Idle::PAWN_POS_Z;
                // THE WHOLE POSE, NOT HALF OF IT (RIBBON_5). POINT_1
                // authored x and z here; RIBBON_1 added y and heading
                // to the mirror and this block never grew to match, so
                // a rebirth carried the DEAD world's altitude and
                // bearing into the new one until the first live
                // readback. Heading is not cosmetic: RIBBON_4's
                // look-ahead reads it — gen_cx = x - cos(heading) x
                // look — so a stale bearing aims the first frames'
                // streaming up to PATCH_LOOK_AHEAD wu into a direction
                // the body is not facing. Both are the spawn pose now,
                // by the same argument POINT_1 made for x and z.
                point_.y = 0.0f;
                point_.heading = 0.0f;
                uint32_t preserved_tier = agent_state_.slots[player_.possessed_slot].tier_idx;
                float preserved_color_r = agent_state_.slots[player_.possessed_slot].color_r;
                float preserved_color_g = agent_state_.slots[player_.possessed_slot].color_g;
                float preserved_color_b = agent_state_.slots[player_.possessed_slot].color_b;
                // The figure travels with the body you inhabit (CLOSURE_PAWN [5]).
                uint32_t preserved_skin = agent_state_.slots[player_.possessed_slot].skin_id;

                gpuState_.reset_player_agent(queue, preserved_tier,
                    preserved_color_r, preserved_color_g, preserved_color_b,
                    preserved_skin);
                gpuState_.set_possessed_slot(0);
                // CPU mirror reseed rides with its owner (agents).
                reseed_player_body(agent_state_, &agents_deps_, preserved_tier,
                    preserved_color_r, preserved_color_g, preserved_color_b,
                    preserved_skin);
                gpuState_.set_world_seed(world_state_.active_seed);
                // THE SKY IS THE WORLD'S, AND IT IS DRAWN AGAIN. The machine
                // carried a destination mood once; there is one world now, so
                // a rebirth re-draws the SAME bank under a new seed.
                stage_world_birth(&sky_deps_, queue,
                    machine_ctx_,
                    orbs_state_, orbs_deps_,
                    pawn_state_);
                // ROSTER-GATE wanderers (c) — rebirth population (slots 1+); slot 0 preserved above.
                if constexpr (ROSTER.wanderers)
                    spawn_population(agent_state_, &agents_deps_, world_state_.active_seed,
                        Idle::PAWN_POS_X, Idle::PAWN_POS_Z,
                        world_box_().first, world_box_().second, queue);
                dump_agent_census(agent_state_, &agents_deps_, "rebirth");
                // Fires AFTER reset_surface and every teardown verb above,
                // and before `band_patches` (the RENDER_SPINE row
                // `RPhase::SurfaceVisibility`) can re-band. Both columns
                // must therefore read 0 for all six — a
                // teardown-completeness assertion, not an observation.
                // (This read "before stream_patches (a RENDER_SPINE row)"
                // until TENSE_0 U4, and was doubly false: the conductor
                // left at ONE_SURFACE-I U2, and there was never a spine
                // row of that name — `stream_patches` was called BY one.)
                dump_entity_census(&machine_ctx_, "rebirth");
                // ROSTER-GATE ribbon (c) — finite-mode release, owner
                // verb. Zero effect when ribbon is off (active_count
                // stays 0). ORDER (O-3): after the world's birth has
                // staged the sky.
                if constexpr (ROSTER.ribbon)
                    release_finite_ribbons(ribbon_state_, &ribbon_deps_, queue);

                // ── THE WORLD IS BUILT (ONE_SURFACE-I U1) ────────────────
                // The same door boot walks, and the last thing a rebirth
                // does before it announces itself. It stands AFTER the
                // "rebirth" entity census for the reason that census gives:
                // both columns must read 0 there, which is a teardown-
                // completeness assertion — a world rebuilt ahead of it would
                // be asserting nothing.
                build_world(&machine_ctx_, device_, queue, tile_world_state_, tile_world_deps_);
                // Rung 4 (the persistence ladder): the automaton is REBORN
                // with the world, from the world's own seed — the same seed
                // draws the same automaton, which is what makes a rebirth
                // reproducible rather than merely new.
                birth_the_automaton(queue);

                uint32_t side = world_state_.finite_mode ? 2 * world_state_.finite_radius + 1 : 0;
                std::cout << "[World] Rebirth complete, seed=" << world_state_.active_seed
                    << " mode=" << (world_state_.finite_mode ? "finite" : "open")
                    << (world_state_.finite_mode ? " " + std::to_string(side) + "x" + std::to_string(side) : "")
                    << "\n";
            }

            // U8 — THE TWO UPLOADS (O-5b/c). upload_signal then upload_config
            // AFTER all staging setters — the O-5b/c face law, enforced by
            // validate_spine at boot. The fade staging that shared this phase
            // left with the transition machine (ONE_WORLD-I), and the veil it
            // staged for followed it out of the config mirror.
            void phase_stage_upload(UpdateCtx& c) {
                auto& gpuSignal = c.gpuSignal;
                auto& queue = c.queue;
                gpuState_.upload_signal(queue, gpuSignal);
                gpuState_.upload_config(queue);
            }

            // U9 — DRIVER BOOKKEEPING (O-5e, dead-last): U1's signal fill
            // consumed the deltas.
            void phase_clear_input_deltas(UpdateCtx&) {
                clear_input_deltas(&input_deps_);
            }

            // ORGAN — the frame boundary: doors, definition re-speaks, the masks,
            // the rule window, the flush. Member functions, in their own file.
            #include "cartridges/the_board/organ_boundary.inc"

            // ── THE CONDUCTOR (update) — a LOOP over UPDATE_SPINE (§1a) ─────
            void update(const AnalysisSignal& signal,
                float aspect_ratio,
                wgpu::Queue& queue) override {
                GPUFrameSignal gpuSignal{};   // the mount block is filled below, with the rest of the signal — one author, one write
                UpdateCtx ctx{ signal, aspect_ratio, queue, gpuSignal };
                for (const URow& row : UPDATE_SPINE) {
                    if (!row.enabled) continue;   // gated-off rows are never timed
                    // The clock pair is the METER's, not the conductor's: with
                    // the instrument off the loop is the bare dispatch it was
                    // before the meter existed (core/instruments.hpp).
                    if constexpr (INSTRUMENTS.frame_meter) {
                        auto t0 = std::chrono::steady_clock::now();
                        (this->*row.fn)(ctx);
                        auto t1 = std::chrono::steady_clock::now();
                        float ms = std::chrono::duration<float, std::milli>(t1 - t0).count();
                        auto& s = meter_.u_rows[(size_t)row.id];
                        s.sum_ms += ms; if (ms > s.max_ms) s.max_ms = ms;
                    }
                    else {
                        (this->*row.fn)(ctx);
                    }
                }
            }

            // SEAM[spine:owns] render() is genuinely spine work: readback state
            //   machines, stale-callback guards, patch streaming. The K1
            //   observation
            //   doesn't apply to render() the same way it applies to update();
            //   render() mixes orchestration (correct) with smaller per-module
            //   GPU upload calls (each lives in its module already).
            // ═══════════════════════════════════════════════════════════════
            // THE FRAME SPINE — render() phases
            //
            // THE EXTRACTION: the movements R1..R21 are now
            // named phase methods; render() at the tail of this block is a page
            // of calls. PURE LIFT — no reordering, no logic change. A whole-
            // movement `if constexpr(ROSTER.x)` gate stays at the CALL SITE
            // (→ CUT-2 spine-row column); runtime data-guards live inside the
            // phase. (R12's SPLIT — the hidden submit as its own named phase,
            // distinct from the sim passes — was the derive flush's, and
            // both halves left at ONE_SURFACE-II U1: one row remains.)
            // ═══════════════════════════════════════════════════════════════

            // R1 — WITNESS HARVEST (algo; P5 maps; consumes LAST frame's
            // capture). Leads the score: every downstream consumer (stream
            // center, corral, sorts) eats its output. The CAPTURE
            // half (R11) sits after dispatch_compute (O-2).
            void phase_witness_harvest(RenderCtx&) {
                if (pawnReadbackState_ == PawnReadbackState::COPIED) {
                    pawnReadbackState_ = PawnReadbackState::MAPPING;
                    pawnReadbackGen_ = world_state_.world_gen;   // OIL_1c — the issue-time generation, at its machine
                    // OIL_1c: CAPTURELESS by requirement, not by taste. The
                    // wrapper's typed-userdata overload converts the callback
                    // with unary + to a plain function pointer, so a capture
                    // would fail the conversion — loudly, at compile time.
                    // `this` rides the trailing userdata slot; the lambda is
                    // written inside a member function, so it keeps the
                    // class's access rights and reaches privates through
                    // `self` with no friend declaration.
                    gpuState_.agent_state_readback_staging().MapAsync(
                        wgpu::MapMode::Read, 0, GPUState::agent_state_buffer_size(),
                        wgpu::CallbackMode::AllowSpontaneous,
                        [](wgpu::MapAsyncStatus status, wgpu::StringView, Cartridge* self) {
                            if (status == wgpu::MapAsyncStatus::Success) {
                                // Drop stale callbacks from a previous world: the
                                // generation recorded at issue time differs from
                                // world_state_.world_gen if a teardown happened in
                                // between. Buffer is still successfully mapped
                                // though, so we Unmap unconditionally (mapping
                                // contract is independent of whether we read).
                                if (self->pawnReadbackGen_ == self->world_state_.world_gen) {
                                    const auto* data = static_cast<const GPUAgentState*>(
                                        self->gpuState_.agent_state_readback_staging().GetConstMappedRange(
                                            0, GPUState::agent_state_buffer_size()));
                                    if (data) {
                                        std::memcpy(self->agent_state_.slots, data,
                                            GPUState::agent_state_buffer_size());
                                        const auto& p = self->agent_state_.slots[self->player_.possessed_slot];
                                        // THE POINT: point_.x/z is the
                                        // POINT's position — the body authors it
                                        // whenever the body hosts (PAWN, or
                                        // RIBBON — the possessed body rides the
                                        // seat). IN CAMERA-HOST NOTHING AUTHORS IT
                                        // (RIBBON_6): this comment named a "camera
                                        // harvest below" that does not exist
                                        // anywhere in src/, and x/z are simply
                                        // HELD-LAST while the witness hosts —
                                        // which is what PointState's own contract
                                        // says. The two writers are this block,
                                        // gated off for CAMERA, and
                                        // rebirth_world's authored present.
                                        if (self->point_.host != PointHost::CAMERA) {
                                            self->point_.x = p.pos_x;
                                            self->point_.z = p.pos_z;
                                            // RIBBON_1: y and heading join the
                                            // mirror. possess() captures the EDGE
                                            // from them — where the body was when
                                            // the host changed — and the GPU eases
                                            // the trajectory from there.
                                            self->point_.y = p.pos_y;
                                            self->point_.heading = p.heading;
                                        }
                                    }
                                }
                                self->gpuState_.agent_state_readback_staging().Unmap();
                            }
                            self->pawnReadbackState_ = PawnReadbackState::IDLE;
                        },
                        this);
                }

                //
                if (floaterReadbackState_ == FloaterReadbackState::COPIED) {
                    floaterReadbackState_ = FloaterReadbackState::MAPPING;
                    floaterReadbackGen_ = world_state_.world_gen;   // OIL_1c — see the pawn arm above
                    gpuState_.floating_entity_readback_staging().MapAsync(
                        wgpu::MapMode::Read, 0, GPUState::floating_entity_buffer_size(),
                        wgpu::CallbackMode::AllowSpontaneous,
                        [](wgpu::MapAsyncStatus status, wgpu::StringView, Cartridge* self) {
                            if (status == wgpu::MapAsyncStatus::Success) {
                                // Drop stale callbacks from a previous world.
                                // Buffer is still mapped, so Unmap unconditionally.
                                if (self->floaterReadbackGen_ == self->world_state_.world_gen) {
                                    const auto* data = static_cast<const GPUFloatingEntityState*>(
                                        self->gpuState_.floating_entity_readback_staging().GetConstMappedRange(
                                            0, GPUState::floating_entity_buffer_size()));
                                    if (data) {
                                        // Owner mirror reconciliation (funnels
                                        // live with spheres / cube_behaviors).
                                        if constexpr (ROSTER.sphere)  // ROSTER-GATE sphere (b) — no spheres, no mirror to release
                                            reconcile_sphere_mirror(self->sphere_state_, &self->sphere_deps_, data);
                                        if constexpr (ROSTER.cube)    // ROSTER-GATE cube (b)
                                            reconcile_cube_mirror(self->cube_behaviors_state_, &self->cube_deps_, data);
                                    }
                                }
                                self->gpuState_.floating_entity_readback_staging().Unmap();
                            }
                            self->floaterReadbackState_ = FloaterReadbackState::IDLE;
                        },
                        this);
                }

                // ATRIUM_11 — THE CAMERA WITNESS. Third arm, the two above
                // in grammar and in every guard: the issue-time generation,
                // the stale callback dropped, Unmap unconditional on a
                // successful map. The whole arm compiles out when the
                // witness is unarmed.
                //
                // THE BASE IS Idle::PAWN_HEADING, not point_.heading: the
                // arrival row's azimuth is an offset on the ARRIVAL gaze, a
                // constant, and the arrival applier added it to exactly this.
                // A printed offset against the live heading would be a
                // different number from the one the panel takes, which is
                // the one way this instrument could lie.
                if constexpr (INSTRUMENTS.camera_witness) {
                    if (cameraReadbackState_ == CameraReadbackState::COPIED) {
                        cameraReadbackState_ = CameraReadbackState::MAPPING;
                        cameraReadbackGen_ = world_state_.world_gen;
                        gpuState_.camera_readback_staging().MapAsync(
                            wgpu::MapMode::Read, 0, GPUState::camera_state_buffer_size(),
                            wgpu::CallbackMode::AllowSpontaneous,
                            [](wgpu::MapAsyncStatus status, wgpu::StringView, Cartridge* self) {
                                if (status == wgpu::MapAsyncStatus::Success) {
                                    if (self->cameraReadbackGen_ == self->world_state_.world_gen) {
                                        const auto* cam = static_cast<const GPUCameraState*>(
                                            self->gpuState_.camera_readback_staging().GetConstMappedRange(
                                                0, GPUState::camera_state_buffer_size()));
                                        if (cam)
                                            dump_camera_orbit(*cam, Idle::PAWN_HEADING,
                                                              self->time_state_.seconds);
                                    }
                                    self->gpuState_.camera_readback_staging().Unmap();
                                }
                                self->cameraReadbackState_ = CameraReadbackState::IDLE;
                            },
                            this);
                    }
                }
            }

            // R3 — SURFACE VISIBILITY (S2, algo). What the conductor did that
            // was never streaming's: BAND the draw set as the point moves, and
            // CULL the entity draw set by distance. Both are functions of a
            // moving POINT; the conductor's other work was a function of a
            // moving WINDOW, and a finite window does not move
            // (ONE_SURFACE-I U2).
            //
            // IT TOOK NO ENCODER. Every line here is CPU plus a WriteBuffer,
            // which is why the row is Driver::Algo with no GPU pass and its
            // meter slot is never armed — phase_respawn_agents' precedent.
            void phase_surface_visibility(RenderCtx& c) {
                auto& queue = c.queue;
                band_patches(&machine_ctx_, queue);
                // `update_entity_draw_visibility` stood beside it, filling
                // `entities_culled`. It has returned a constant 0 since the
                // ARCH loop left at ONE_WORLD-I U3 — the only family whose
                // mesh could be zeroed at range — and nothing read the
                // field. Both left at ONE_SURFACE-I U6.
            }

            // R4 — RESPAWN AGENTS stood here (STAGE_0 U2), a render spine row
            // that refilled the slots the GPU had evicted. With eviction
            // retired the row has nothing to do, so it leaves the SPINE as
            // well as the file: RPhase::RespawnAgents, its RENDER_SPINE row
            // and the RC-1 ordering assert go with it, and the render spine
            // is fifteen rows.

            // R6 — CENSUS DUMPS (wall-clock interval, diagnostic). GoL residue
            // proof (G3, constexpr-gated intra-movement) + entity census.
            // Autonomous stdout (P6: every switch has a witness).
            void phase_census_dumps(RenderCtx&) {
                // ROSTER-RESIDUE gol (2e) STOOD HERE AND WAS ALREADY A LIE
                // BEFORE THIS UNIT (ONE_SURFACE-II U2). It proved, across
                // frames, that a disabled gol left its GPU buffers pristine:
                // zone_count 0 and zone-compute runs 0. Its evidence was
                // rosterGolZoneRuns_, incremented in phase_gol_derive_flush —
                // and U1 retired that phase with the derive seam, so the
                // counter has had no writer since. A witness whose evidence
                // cannot move reports PRISTINE unconditionally, which is the
                // exact failure mode P6 exists to prevent. It leaves with the
                // family whose residue it watched, not quietly with the
                // phase, because saying so is the point.
                // THE DIAL (core/instruments.hpp). Everything below this line
                // is the PERIODIC instrument — the recurring dump and the
                // meter table that rides its cadence — and a print is a
                // blocking console write on the render thread, so ~50 lines
                // every 30 s IS the long frame every 30 s. Off, the row above
                // (the residue proof, a correctness witness) still runs and
                // this returns. The "boot" and "rebirth" dumps are
                // NOT on the dial: those are P6 transition witnesses, and
                // their frames are already long.
                if constexpr (!INSTRUMENTS.periodic_census) return;

                // Periodic entity census dump — its own interval, its own
                // gate. THE INSTRUMENT the batch witnesses read; untouched.
                if (time_state_.seconds - spawn_engine_state_.lastCensusDump_ >= CENSUS_DUMP_INTERVAL) {
                    // TIDY_0c-ii — IS THIS THE FIRST DUMP? lastCensusDump_ is
                    // seeded NEGATIVE (spawn_engine.hpp) so the first census
                    // fires immediately rather than 30 s in. time_state_
                    // .seconds is monotonic and non-negative, so a negative
                    // value here is the sentinel and can be nothing else.
                    const bool first_dump = spawn_engine_state_.lastCensusDump_ < 0.0f;
                    // HEADROOM_0 U3 — THE ENTITY TEXT IS ON ITS OWN DIAL.
                    // ~50 blocking std::cout writes, and the 2026-08-13 boot
                    // read census_dumps max 1051 ms: an instrument spending
                    // over a second inside frames it exists to measure.
                    //
                    // It could not simply be gated off with periodic_census,
                    // because instruments.hpp asserts frame_meter REQUIRES
                    // periodic_census — the [METER] table below rides this
                    // same cadence, so turning the dial off would turn off
                    // the meter. The dial is split instead: the cadence and
                    // the table stay on periodic_census, the TEXT answers to
                    // census_entity_dump. `meter` drops it; `full` keeps it.
                    //
                    // Silence rather than buffering: flushing the text
                    // "outside the frame" moves the same blocking write to
                    // the same thread microseconds later. Not writing it is
                    // both cheaper and a smaller edit.
                    //
                    // The cadence bookkeeping below is NOT gated — it must
                    // advance whether or not the text prints, or the meter
                    // window it drives would never close.
                    if constexpr (INSTRUMENTS.census_entity_dump) {
                        dump_entity_census(&machine_ctx_, "periodic");
                    }
                    spawn_engine_state_.lastCensusDump_ = time_state_.seconds;

                    // THE FRAME METER — the timing census rides the same
                    // cadence. Print ALL enabled rows (completeness feeds
                    // the suspect table; Jean pastes this block back
                    // verbatim). mean = sum/frames; fps = frames/wall.
                    // The dial conjunct is for ELIMINATION, not correctness:
                    // with the meter off nothing increments window_frames, so
                    // the block is already inert — the constant lets the
                    // compiler drop the formatting with it.
                    // TIDY_0c-ii — THE FIRST WINDOW IS NOT A MEASUREMENT.
                    // The seeded lastCensusDump_ fires this dump on frame 1,
                    // and render() increments window_frames at its head, so
                    // the guard below sees 1 and prints a ONE-FRAME table
                    // whose wall clock runs from FrameMeter's construction —
                    // i.e. across the whole boot, pipeline compilation
                    // included. That window reads fps ~0.0, means taken from
                    // a single cold frame, and a residue computed from it.
                    // Pasted into an A/B it is not noise, it is a wrong
                    // number wearing the right format.
                    //
                    // SKIPPED, NOT SILENT (P6): the skip prints its own line,
                    // so a missing first window is never confused with a
                    // meter that failed to arm. reset() restamps
                    // window_start, so the NEXT window is a true 30 s span.
                    if (INSTRUMENTS.frame_meter && first_dump) {
                        char line[160];
                        std::snprintf(line, sizeof line,
                            "[METER] first window SKIPPED — %u frame(s), wall clock spans "
                            "the boot; window starts now\n", meter_.window_frames);
                        std::cout << line;
                        meter_.reset();
                    }
                    else if (INSTRUMENTS.frame_meter && meter_.window_frames > 0) {
                        // WIT_2b — 256, not 160. Pulling dropped_submits out of
                        // this header brought the realistic line back to 149 of
                        // 160, and a 10-byte margin is precisely how the last
                        // one was lost: a hostile window (seven-digit frame
                        // counts, four-digit fps, a wide envelope) still renders
                        // 169. Sizing for the worst case retires the CLASS of
                        // defect instead of this instance of it. A char[320] on
                        // the stack, once per 30 s window, costs nothing worth
                        // counting.
                        //
                        // RIBBON_6 widened this line by two columns (canvas WxH
                        // and the over count) — ~24 more characters at ordinary
                        // values, ~40 at hostile ones. 256 would still have fit;
                        // 320 is taken by the paragraph above's own argument,
                        // whose whole point is not to re-audit the margin every
                        // time a column is added.
                        char line[320];
                        const float wall_s = std::chrono::duration<float>(
                            std::chrono::steady_clock::now() - meter_.window_start).count();
                        const float fps = wall_s > 0.0f
                            ? (float)meter_.window_frames / wall_s : 0.0f;
                        if (meter_gpu_)
                            std::snprintf(line, sizeof line,
                                // TIDY_0d: the gpu columns say what they are.
                                // Both are folded from the PER-FRAME SUM over a
                                // row's pass pairs, not from a single pair — so
                                // a multi-pass row's max can exceed any one pass
                                // it contains. The accumulation site carries the
                                // full note.
                                // WRAP_0 U3 — THE WINDOW DESCRIBES ITSELF. A
                                // capture used to say what it measured and not
                                // what it measured it UNDER, so a mask table
                                // had to be transcribed by hand beside the log
                                // and trusted. These are the dials that
                                // change what a window means.
                                "[METER] window %uf  fps %.1f  canvas %ux%u  gpu sampled %uf"
                                " | draw=0x%02X shadow=0x%X pcf=%u"
                                " | budget %.1f ms | over %uf"
                                " | envelope mean %.2f max %.2f ms -> purse %.2f ms"
                                " | gpu mean/max (per-frame sum)\n",
                                meter_.window_frames, fps,
                                // RIBBON_6: a GPU budget read against an unknown
                                // resolution is not a reading, and the canvas was
                                // the one variable that moved silently between
                                // windows in the recording that opened the round.
                                t7::g_canvas_w, t7::g_canvas_h,
                                meter_.gpu_sampled_frames,
                                gpuState_.config().draw_mask,
                                gpuState_.config().shadow_mask,
                                gpuState_.config().shadow_pcf_taps,
                                FrameMeter::FRAME_BUDGET_MS,
                                meter_.gpu_over_budget_frames,
                                // HEADROOM_0 U1 — the purse. The envelope is a
                                // per-frame SPAN, so its mean is over sampled
                                // frames, not over the window's frames; and the
                                // purse quotes the MEAN, because a budget spent
                                // against the worst frame is not a budget.
                                meter_.gpu_sampled_frames > 0
                                    ? meter_.gpu_envelope.sum_ms / meter_.gpu_sampled_frames : 0.0,
                                (double)meter_.gpu_envelope.max_ms,
                                FrameMeter::FRAME_BUDGET_MS -
                                    (meter_.gpu_sampled_frames > 0
                                        ? meter_.gpu_envelope.sum_ms / meter_.gpu_sampled_frames : 0.0));
                        else
                            std::snprintf(line, sizeof line,
                                "[METER] window %uf  fps %.1f | budget %.1f ms\n",
                                meter_.window_frames, fps, FrameMeter::FRAME_BUDGET_MS);
                        std::cout << line;
                        // WIT_2b — alone, and unconditional. Never appended to
                        // a formatted line again.
                        t7::print_dropped_submits("window");
                        // PANORAMA_0 §5.5 — THE MESH-GEN FIRING COUNT. One
                        // line, only when something fired: on a still world it
                        // is silent, and on a ride it is the number the
                        // EntityMeshGen row cannot give — how many times a
                        // family regenerated EVERY slot because one slot
                        // changed. Names only the families that fired.
                        {
                            uint32_t fired = 0;
                            for (uint32_t f = 0; f < PopFamily::COUNT; f++)
                                fired += meter_.mesh_gen_firings[f];
                            if (fired > 0) {
                                std::string mg = "[METER] mesh-gen firings";
                                for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
                                    if (meter_.mesh_gen_firings[f] == 0) continue;
                                    mg += "  ";
                                    mg += family_short_name(f);
                                    mg += " ";
                                    mg += std::to_string(meter_.mesh_gen_firings[f]);
                                }
                                mg += "  | total ";
                                mg += std::to_string(fired);
                                mg += " over ";
                                mg += std::to_string(meter_.window_frames);
                                mg += "f\n";
                                std::cout << mg;
                            }
                        }
                        // WRAP_0 U4 — THE SLOT LINE: milliseconds divided by
                        // geometry. `main_pass` is 11-12 ms on Kepler for a
                        // vertex and pixel count that does not explain it, and
                        // the terrain plan slots are where the vertices
                        // are. n is the VISIBLE INSTANCE count the cull kernel
                        // wrote (its two atomics); I is the slot's index
                        // count — so n x I is the slot's triangles x 3, and the
                        // two products are what the pass is actually drawing.
                        // TERRAIN_0 opens on this line and the mask table.
                        if (slotSampleValid_) {
                            char sl[160];
                            std::snprintf(sl, sizeof sl,
                                "[METER] terrain  A %ux%u  B %ux%u\n",
                                slotInstances_[0], gpuState_.patch_index_count(),
                                slotInstances_[1], gpuState_.patch_index_count_cap_only());
                            std::cout << sl;
                        }
                        double u_sum = 0.0, r_sum = 0.0;
                        for (const URow& row : UPDATE_SPINE) {
                            if (!row.enabled) continue;
                            const auto& s = meter_.u_rows[(size_t)row.id];
                            const double mean = s.sum_ms / meter_.window_frames;
                            u_sum += mean;
                            std::snprintf(line, sizeof line,
                                "[METER] U %-22s  mean %.2f  max %.2f\n",
                                row.name, mean, (double)s.max_ms);
                            std::cout << line;
                        }
                        for (const RRow& row : RENDER_SPINE) {
                            if (!row.enabled) continue;
                            const auto& s = meter_.r_rows[(size_t)row.id];
                            const double mean = s.sum_ms / meter_.window_frames;
                            r_sum += mean;
                            // A row line gains GPU columns when samples exist.
                            const auto& g = meter_.r_gpu[(size_t)row.id];
                            if (meter_.gpu_sampled_frames > 0 &&
                                (g.sum_ms > 0.0 || g.max_ms > 0.0f)) {
                                const double gmean = g.sum_ms / meter_.gpu_sampled_frames;
                                std::snprintf(line, sizeof line,
                                    "[METER] R %-22s  cpu %.2f/%.2f  gpu %.2f/%.2f\n",
                                    row.name, mean, (double)s.max_ms, gmean, (double)g.max_ms);
                            }
                            else {
                                std::snprintf(line, sizeof line,
                                    "[METER] R %-22s  mean %.2f  max %.2f\n",
                                    row.name, mean, (double)s.max_ms);
                            }
                            std::cout << line;
                        }
                        // THE S BLOCK (OIL_1a) — the host rows, HostRow
                        // order. An S row names where a wait SURFACES,
                        // not where the cost lives (the timer law at the
                        // enum). A frame that failed the acquire noted
                        // nothing, so these means stay consistent with
                        // window_frames (rendered frames only).
                        static constexpr const char* S_NAMES[(size_t)HostRow::COUNT] = {
                            "begin_frame", "acquire", "finish_submit", "present", "frame_total"
                        };
                        double s_partials = 0.0, s_frame_total = 0.0;
                        for (size_t i = 0; i < (size_t)HostRow::COUNT; i++) {
                            const auto& s = meter_.s_rows[i];
                            const double mean = s.sum_ms / meter_.window_frames;
                            if (i == (size_t)HostRow::FrameTotal) s_frame_total = mean;
                            else                                  s_partials += mean;
                            std::snprintf(line, sizeof line,
                                "[METER] S %-22s  mean %.2f  max %.2f\n",
                                S_NAMES[i], mean, (double)s.max_ms);
                            std::cout << line;
                        }
                        std::snprintf(line, sizeof line,
                            "[METER] U_SUM %.2f   R_SUM %.2f\n", u_sum, r_sum);
                        std::cout << line;
                        // The residue — the previously unattributable gap,
                        // now named: what frame_total carries that no U, R,
                        // or S bracket does (input drain, encoder create,
                        // glue). Native backpressure surfaced in acquire/
                        // present above; on the web twin the fps line's
                        // remainder beyond frame_total is the rAF interval.
                        std::snprintf(line, sizeof line,
                            "[METER] residue %.2f  (frame_total %.2f - U_SUM - R_SUM - S_partials %.2f)\n",
                            s_frame_total - (u_sum + r_sum + s_partials),
                            s_frame_total, s_partials);
                        std::cout << line;
                        meter_.reset();
                    }
                }
            }

            // R7 — RIBBON TICK (music+wall-clock). One call; the conductor lives
            // in bodies/ribbon.hpp. Its tail is the sky resync — the SOLE author of
            // the sky_* block (E-3 mechanized: the drain skips those 32 bytes,
            // and initialize() boot-neutrals them for the ribbon-off case).
            // ROSTER-GATE ribbon — guarded at the call site.
            void phase_ribbon_tick(RenderCtx& c) {
                auto& queue = c.queue;
                // The sky-exit death first — it releases the ground, so it
                // takes the machine face the tick below does not carry.
                ribbon_on_dismount(&machine_ctx_, queue);
                ribbon_frame_tick(ribbon_state_, &ribbon_deps_, queue);
            }

            // R8 — ENTITY MESH GEN (algo; dirty-driven). Six constexpr-gated
            // prepare lines set dirty[]; one compute pass dispatches the dirty
            // families (branches on dirty-ness, not the enable bit). The
            // per-family gates are intra-movement.
            void phase_entity_mesh_gen(RenderCtx& c) {
                auto& encoder = c.encoder;
                auto& queue = c.queue;
                bool dirty[PopFamily::COUNT] = {};
                bool anyDirty = false;
                // One explicit prepare line per family, each
                // presence constexpr-gated — THE SCORE RULING: the typelist
                // fold dissolved into prose. A disabled
                // family's prepare is eliminated at COMPILE TIME (no call,
                // no runtime branch); all-enabled compiles to the same six
                // calls in the same order.
                if constexpr (ROSTER.pyramid) {   // ROSTER-GATE pyramid (b)
                    dirty[PopFamily::PYRAMID] = FAMILY_DISPATCH[PopFamily::PYRAMID].prepare_mesh(&machine_ctx_, queue);
                    anyDirty = anyDirty || dirty[PopFamily::PYRAMID];
                }
                if constexpr (ROSTER.sphere) {    // ROSTER-GATE sphere (b)
                    dirty[PopFamily::SPHERE] = FAMILY_DISPATCH[PopFamily::SPHERE].prepare_mesh(&machine_ctx_, queue);
                    anyDirty = anyDirty || dirty[PopFamily::SPHERE];
                }
                if constexpr (ROSTER.ribbon) {    // ROSTER-GATE ribbon (b)
                    dirty[PopFamily::RIBBON] = FAMILY_DISPATCH[PopFamily::RIBBON].prepare_mesh(&machine_ctx_, queue);
                    anyDirty = anyDirty || dirty[PopFamily::RIBBON];
                }
                if constexpr (ROSTER.cube) {      // ROSTER-GATE cube (b)
                    dirty[PopFamily::CUBE] = FAMILY_DISPATCH[PopFamily::CUBE].prepare_mesh(&machine_ctx_, queue);
                    anyDirty = anyDirty || dirty[PopFamily::CUBE];
                }
                if (anyDirty) {
                    wgpu::ComputePassDescriptor cpd{};
                    cpd.label = "Entity Mesh Gen";
                    cpd.timestampWrites = gpuState_.meter_arm_compute((uint32_t)RPhase::EntityMeshGen);
                    wgpu::ComputePassEncoder pass = encoder.BeginComputePass(&cpd);
                    // LOOM_2 pass head: WORLD + FRAME are every pipeline's strata 0/1.
                                    { pass.SetBindGroup(0, gpuState_.world_group());
                      pass.SetBindGroup(1, gpuState_.frame_c_group()); }
                    // dispatch skips disabled families structurally:
                    // dirty[f] stays false for a disabled family (never
                    // set above), so this branches on dirty-ness, not on
                    // the enable bit.
                    for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
                        if (!dirty[f]) continue;
                        // PANORAMA_0 §5.5 — one firing, counted where it is
                        // known. Folds to nothing with the dial off.
                        if constexpr (INSTRUMENTS.frame_meter) meter_.mesh_gen_firings[f]++;
                        FAMILY_DISPATCH[f].dispatch_mesh(&machine_ctx_, pass);
                    }
                    pass.End();
                }
            }

            // R9 — LIGHTS UPLOAD (algo). The portal array upload that shared
            // this row left with the doors (ONE_WORLD-I U2).
            void phase_upload_lights(RenderCtx& c) {
                auto& queue = c.queue;
                upload_lights(&sky_deps_, queue);
            }

            // LIVE CARD WRITE (GROUND_CARD_1; between R9 and R10). The per-frame
            // deformation field: the writer calls the existing evaluators at
            // texel centers; every consumer then samples one card. Before
            // DispatchCompute (the consumers).
            //
            // OPT_1a — THE REST SKIP IS RETIRED (ONE_SURFACE-II U1); the
            // tombstone at phase_live_card_write holds the whole account.
            // In short: the dispatch (819,200 invocations) was skipped while
            // the card's field was at rest, on a three-conjunct law
            // evaluated CPU-side and conservative (any doubt => write): no
            // GoL zone live, pulse ring empty, terrain_time <= 0. The first
            // conjunct is false everywhere once the automaton is the ground.
            // The last two remain structurally pinned at rest (O0-d: the
            // ring's only writer is the boot zero-pin; terrain_time's only
            // writers pin 0.0) — so the KERNEL still writes zeros at rest;
            // it is simply no longer skipped. A future re-arming of either
            // conjunct wakes the
            // writer without an edit here. On the transition into rest, ONE
            // final write runs so consumers never read stale non-zero
            // texels; the flag resets at world teardown so a fresh world's
            // rest field is written once too.
            //
            // SPINE_2 B — THE DECISION STAYS, THE PASS LEAVES. The write is
            // now the FIRST DISPATCH of R10's pass, not a pass of its own
            // (render_passes.hpp, dispatch_compute). This row still owns the
            // rest law and the witness; what it hands on is a bool, and the
            // consumers still read a written card because dispatch order
            // inside a pass is a visibility rule.
            // ═══ THE AUTOMATON IS BORN WITH THE WORLD ════════════════
            //
            // ONE DOOR (L10), called from the two places a world comes into
            // being — boot and rebirth_world — immediately after
            // build_world, because the automaton's seed kernel reads the
            // ground's own vocabulary (discrete_visibility_rest) and that
            // reads the config stage_world_birth authored.
            //
            // ITS OWN ENCODER AND SUBMIT, and this is NOT the hidden second
            // submit the derive seam was. That one ran mid-FRAME, every
            // frame a zone spawned, ahead of the host's encoder, and had to
            // be a named spine row so the submit was visible. This runs at
            // BIRTH, outside the frame loop entirely, on the same footing as
            // build_world's own batch submits.
            void birth_the_automaton(wgpu::Queue& queue) {
                draw_automaton(automaton_state_, AUTO_LIVE,
                               world_state_.active_seed, world_state_.finite_radius);
                wgpu::CommandEncoderDescriptor encDesc{};
                encDesc.label = "birth_the_automaton";   // DOMESDAY_1 A9 (label law)
                wgpu::CommandEncoder encoder = device_.CreateCommandEncoder(&encDesc);
                birth_automaton(automaton_state_, &automaton_deps_, queue, encoder);
                wgpu::CommandBuffer cmd = encoder.Finish();
                queue.Submit(1, &cmd);
            }

            // ═══ AN EMPTY ROW, HELD OPEN FOR THE CARD'S RETURN ═══════
            //
            // THIS BODY DOES NOTHING, ON PURPOSE, and TENSE_0 U6 says so
            // plainly rather than leaving a reader to infer it from
            // `(void)c;`.
            //
            // R8 DECIDED, AND THERE IS NOTHING LEFT TO DECIDE. The body
            // read the rest law, printed its transitions under P6, and set
            // the bool R10 consumed. The whole apparatus is tombstoned
            // beside live_card_is_live's grave below; the card became
            // `phase_dispatch_compute`'s first dispatch, every frame, when
            // the rest law died with the zones at ONE_SURFACE-II U1.
            //
            // THE ROW STAYS, AND REMOVING IT WOULD BE A SPINE EDIT RATHER
            // THAN A CLEANUP. Four reasons, each binding on its own, and
            // they do NOT all have the same teeth:
            //   · GROUND_CARD_1's ordering static_assert below names
            //     RPhase::LiveCardWrite by enumerator — delete the row and
            //     the TU does not compile. This is the strongest of the four
            //     and it is independent of both the census and the tombstone;
            //   · the spine tables are DENSE at COMPILE TIME (static_assert
            //     "render spine must be dense", table extent vs RPhase::COUNT)
            //     while row-order integrity is a BOOT-TIME std::abort in
            //     validate_spine — two different teeth, and a reader who
            //     hears "static_assert" for both will look for the wrong
            //     failure. Either way a row is a contract and not a line;
            //   · the score census (tools/gates/score/run.py) asserts this
            //     row by name, with its own sentence about why it is
            //     unconditional;
            //   · it is the TOMBSTONE-HOLDER for GROUND_CARD_1's two
            //     revival conditions — the pulse ring's emptiness and
            //     terrain_time <= 0, both structurally pinned at rest
            //     today, either of which re-arming wakes the writer with
            //     no edit here. render_passes.hpp points AT this row for
            //     exactly that account. THE PULSE RING IS RULED
            //     REACTIVATED as the coupling campaign's first voice, so
            //     one of those two conditions has a date.
            void phase_live_card_write(RenderCtx& c) {
                (void)c;
            }

            // R10 — DISPATCH COMPUTE (music+input+algo). The per-frame world-
            // update compute pass (7 dispatches; render_passes.hpp). O-1 by
            // construction: R7's resync writes before this reads (submission
            // order).
            void phase_dispatch_compute(RenderCtx& c) {
                auto& encoder = c.encoder;
                // R8's verdict rode in here as a bool (SPINE_2 B), and a
                // frame that never reached R10 could not leak the flag into
                // the next one. THERE IS NO VERDICT: the card is this
                // pass's first dispatch, every frame.
                //
                // (Both sentences stood here at once until TENSE_0 U6 —
                // the first still saying the card arrives "when the rest
                // law asks for it", the second saying it always does. The
                // contradiction is two campaigns' prose in one block; the
                // second one is the true one.)
                dispatch_compute(&machine_ctx_, encoder);
            }

            // R11 — WITNESS CAPTURE (O-2: staging copies AFTER compute; feeds
            // next frame's HARVEST). The camera copy is CAMERA-HOST ONLY (the
            // pawn-host frame encodes no camera copy; that path stays
            // byte-untouched).
            void phase_witness_capture(RenderCtx& c) {
                auto& encoder = c.encoder;
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

                // ATRIUM_11 — the camera witness's 48 bytes, on the same
                // encoder and after the same dispatches. update_camera_vp has
                // written camera_state by now, exactly as it has written the
                // agent buffer above.
                if constexpr (INSTRUMENTS.camera_witness) {
                    if (cameraReadbackState_ == CameraReadbackState::IDLE) {
                        encoder.CopyBufferToBuffer(
                            gpuState_.camera_buffer(), 0,
                            gpuState_.camera_readback_staging(), 0,
                            GPUState::camera_state_buffer_size());
                        cameraReadbackState_ = CameraReadbackState::COPIED;
                    }
                }
            }

            // R12a — GOL DERIVE FLUSH STOOD HERE, AND IT WAS THE PROGRAM'S
            // ONLY HIDDEN SECOND SUBMIT (ONE_SURFACE-II U1).
            //
            // It had its own encoder and its own queue.Submit, issued before
            // the host's, and its own named spine row so that the submit was
            // VISIBLE rather than buried inside another phase. That was the
            // right treatment for it, and the thing it treated is gone:
            // SEAM[gol:derive-submit] existed because a zone spawning
            // mid-frame had to derive its parameters before the agent
            // kernels read them, in the same frame. The automaton is seeded
            // at BIRTH, on the birth encoder, and nothing spawns after — so
            // the seam, the second submit, the F_SUBMIT flag on a compute
            // row, and the ordering assert that paired the two rows all
            // retire together. The frame has one submit again.

            // R12 — THE AUTOMATON'S STEP (algo). Header upload + sync +
            // evolve in SEPARATE passes (O-6a barrier by pass boundary).
            // UNGATED: the ground is not a roster family and there is no
            // count to be zero.
            void phase_automaton_step(RenderCtx& c) {
                auto& encoder = c.encoder;
                auto& queue = c.queue;
                upload_automaton_header(automaton_state_, &automaton_deps_, queue);
                dispatch_automaton_sync(&automaton_deps_, encoder);
                dispatch_automaton_evolve(&automaton_deps_, encoder);
            }

            // R13 — PAWN AURA (wall-clock). Persistent terrain influence; the
            // runtime presence/clearing condition lives inside. ROSTER-GATE
            // pawn_aura — guarded at the call site.
            void phase_pawn_aura(RenderCtx& c) {
                auto& encoder = c.encoder;
                auto& queue = c.queue;
                dispatch_pawn_aura(pawn_state_, &pawn_deps_, encoder, queue);
            }

            // R14 — ORB SKY (algo+music). One-shot init, optional recolor,
            // snapshot-prev for flocking, advance dynamics. ROSTER-GATE orbs —
            // guarded at the call site.
            void phase_orb_sky(RenderCtx& c) {
                auto& encoder = c.encoder;
                auto& queue = c.queue;
                dispatch_orb_init(orbs_state_, &orbs_deps_, encoder);
                dispatch_orb_recolor(orbs_state_, &orbs_deps_, encoder);
                dispatch_orb_copy_prev(orbs_state_, &orbs_deps_, encoder);
                dispatch_orb_dynamics(orbs_state_, &orbs_deps_, encoder, queue);
            }

            // R15/R16 — GROUND ENTRIES + PLACEMENT CORRECTION stood here.
            // The pair was the ARCH's ground channel end to end: the CPU
            // staged per-arch leg origins, compute_entity_placement wrote the
            // corrected Y into the entity ground atlas, and arch_vs /
            // shadow_arch_vs were the atlas's only readers. The family left
            // at ONE_WORLD-I U3 and the channel had nothing to correct and
            // no one to read it, so both rows, the O-4 cascade between them
            // and the atlas itself went with it. (PRUNE_2 U4 had already
            // retired the re-raise, when the column stopped feeding
            // corrected ground back into its mesh.)

            // The two P6 witness memories (zoneRectsInCorePrev_ and
            // zonesActiveAnywherePrev_) left with the transitions they
            // remembered — both switches are pinned (ONE_SURFACE-II U3).
            // liveCardLivePrev_ (OPT_1a's witness memory) left with the
            // transition it remembered.

            // OPT_1a — THE REST LAW, one home. The dispatch gate and the P6
            // witness read the SAME function, so the log can never disagree
            // with the skip (the zone_rects_in_core precedent). The three
            // conjuncts in occurrence order, short-circuiting: the two config
            // reads are free, the zone scan only runs if they clear.
            // ═══ THE REST LAW IS RETIRED — TOMBSTONE (GROUND_CARD_1) ═════
            //
            // WHAT STOOD HERE. `live_card_is_live()`, a three-term
            // disjunction read by BOTH the dispatch gate and the P6 witness
            // so the log could never disagree with the skip:
            //     pulse_count > 0                      [MUSICAL]
            //     terrain_time > 0                     [MUSICAL]
            //     any GoL zone active                  [NOT MUSICAL]
            // With it: `liveCardRestClean_`, the entering-rest clearing
            // write, `liveCardWritePending_`, and `live_card_state_label`'s
            // two-word vocabulary ("LIVE — writer runs every frame" /
            // "AT REST — one clearing write, then skipped").
            //
            // WHY IT CANNOT SURVIVE THE AUTOMATON. The third term asked
            // whether any zone covers the texel. With eight islands that is
            // a LOCAL condition and the card reached rest whenever they were
            // quiet or absent. An automaton over the whole cell grid makes
            // it false wherever ANY cell is alive — which, at a seeded
            // density over the world, is everywhere, always. The gate would
            // never close again, and an optimisation that can never fire is
            // worse than none: it is a claim the code makes and does not
            // keep.
            //
            // WHY THE CARD IS NOT A LOSS. Jean's boot log, before this unit,
            // already read "[Card] live-card field: LIVE — writer runs every
            // frame (boot)". The skip was not being taken. This retires an
            // optimisation the program had already stopped reaching, which
            // is a cut and not a loss.
            //
            // TWO CONDITIONS BRING REST BACK, and they are named so that
            // whoever meets one knows where to look:
            //   (1) AN AUTOMATON PAUSE DIAL. If the panel era lets the
            //       ground stop advancing, "not advancing" is a real state
            //       again and the rest law is the right answer to it.
            //   (2) A TICK-CADENCE GATE. The card is clean BETWEEN ticks, so
            //       a gate on the automaton's own clock would still close,
            //       often. It was NOT taken here because conjuncts (1) and
            //       (2) above are musical and move on their own clock: such
            //       a gate would have to COMPOSE with them rather than
            //       replace them, and an honestly per-frame card beats a
            //       three-clock gate nobody can reason about. If the
            //       per-frame cost is ever measured and found real, this is
            //       where to start.
            //
            // The kernel's own banner (world.wgsl, §7.3) carries the same
            // retirement from the shader's side.

            // THE COUNT, one home — the draw plan's classifier input and the
            // P6 witness read the SAME function, so the log can never
            // disagree with the plan. Each active zone's world AABB
            // (persisted at commit_gol) inflated by one patch, tested
            // against the disc of the LIVE RING around the point.
            //
            // THE YARDSTICK WIDENED AND THAT IS THE SAFE DIRECTION
            // (ONE_SURFACE-I U5). It was `lod0_radius` — 175 — because the
            // curtains it switched lived in the LOD0 band, and the band and
            // the flag read one value so they could not disagree. There is
            // no band; the surviving authority is `draw_ring`, 342. A WIDER
            // disc can only find MORE zones in scope, so it can only choose
            // the zoned tail MORE often — and the zoned tail is the
            // conservative arm, the one that seals seams. A widening cannot
            // open one.
            // ONE RECT, AND IT IS THE WORLD (ONE_SURFACE-II U1). The loop
            // that stood here tested each active zone's persisted AABB,
            // inflated by one patch, against the disc of the veil ring
            // around the point, and returned how many were in scope. Every
            // clause of it was about ISLANDS: a zone had a corner, an
            // extent, and a distance from the eye at which it stopped
            // mattering.
            //
            // The automaton has none of those. It covers the ground, so the
            // answer to "does a curtain reach this patch" is YES for every
            // patch that carries discrete cells, and the honest rect is the
            // world box. The classifier is unchanged and its arithmetic is
            // unchanged; what changed is that segment A is now the common
            // case rather than an extreme (recorded at the capacity asserts
            // in contracts/surface_services.hpp — a LOAD question for the
            // walk, not a correctness one).
            //
            // The count is still a COUNT and still the P6 witness's input,
            // so the log and the plan still read one function.
            uint32_t zone_rects_in_core() const { return 1u; }

            // OPT_1e — THE GLOBAL COUNT. The LOD1 ring's two counts (clean
            // prefix / zoned) select on "any zone active ANYWHERE",
            // deliberately NOT geometric: zones outlive the LOD0 core
            // (EXIST reaches past the veil ring) and the curtain tail is
            // the only thing sealing lifted slab walls in the 175–325 wu
            // annulus, so any distance-scoped predicate would open them.
            // Conservative by construction: the prefix draws only at true
            // rest, when no cell anywhere can lift.
            // OPT_1e's SWITCH IS PINNED SHUT, deliberately and on the
            // conservative arm. Its own banner said why the predicate was
            // never geometric: the curtain tail is the only thing sealing
            // lifted slab walls in the 175-325 wu annulus, so the clean
            // prefix may draw ONLY at true rest, when no cell anywhere can
            // lift. The automaton is everywhere and always able to lift, so
            // true rest is unreachable and the zoned tail is the permanent
            // answer.
            //
            // KEPT AS A FUNCTION rather than folded into its callers: it is
            // ONE HOME for a fact two readers share (the LOD1 count and the
            // P6 witness), and the panel's pause dial — the same dial the
            // rest-law tombstone names — is exactly what would make it move
            // again.
            uint32_t zones_active_anywhere() const { return 1u; }

            // R17 — FRUSTUM CULL (algo; O-7 tail). Cull before the draw passes —
            // the indirect draws consume the cull output (recon E-5).
            void phase_frustum_cull(RenderCtx& c) {
                auto& encoder = c.encoder;
                auto& queue = c.queue;
                // ECONOMY_1 E1 rev2 — the LOD0-SCOPED curtain switch, staged
                // once per frame before every LOD0 carrier (R17 precedes the
                // main and snapshot passes; the shadow pass draws LOD1 since
                // E2 and has no curtains to switch).
                //
                // SCOPE: curtains exist ONLY in the LOD0 index buffer. The
                // cap-only choice is correct on a clean LOD0 patch because no
                // cell there lifts — not because a lift without a curtain is
                // harmless. In the LOD1 ring cells lift and own no curtain;
                // what seals those seams is the rim curtain (WALL_1 — skirt
                // ring copies stand on unlifted ground). The rev1 flag asked
                // "any zone anywhere" and was therefore inert: zones are alive
                // globally almost always, so it never released.
                //
                // Conservative by one patch: each zone's world AABB is
                // inflated by PATCH_EXTENT before the disc test. The radius
                // is the LIVE ring since ONE_SURFACE-I U5 — see
                // zone_rects_in_core for why widening is the safe direction.
                {
                    const uint32_t in_core = zone_rects_in_core();
                    // P6's WITNESS IS RETIRED BECAUSE THE SWITCH IS PINNED,
                    // which is P6 applied rather than P6 waived: it reported
                    // the rect count on change of N, and N is 1 forever now
                    // (zone_rects_in_core's tombstone says why). A witness
                    // that cannot report a change is not a witness; it is a
                    // line. When the panel's pause dial unpins the count,
                    // this comes back with it.
                    // THE FLAG HAS NO READER LEFT. The draw plan retired the
                    // global-flag selection for the main pass and the
                    // indirect reset; the shadow pass draws both bands
                    // through patch_index_buffer_lod1(); and PRUNE_1 took
                    // the snapshot pass, which was the last carrier named in
                    // state.hpp beside the flag-selected pair. The switch is
                    // kept as it stood — retiring it is its own reading, not
                    // this campaign's (PRUNE_1 R7), and the witness above
                    // still reports a real fact about the world.
                    gpuState_.set_curtains_active(in_core > 0);
                }
                // OPT_1e — the LOD1 count switch, staged beside the curtain
                // switch: this dispatch's slot-C reset reads it, and so do
                // the sun's two LOD1 draws (R18 follows R17, so the flag is
                // fresh for both same-frame). P6: witness the INPUT that
                // drives the switch, on change of N only.
                {
                    // Same pin, same retirement (see above): the count is 1
                    // forever, so there is no transition to witness.
                    gpuState_.set_zones_active_anywhere(zones_active_anywhere() > 0);
                }
                dispatch_frustum_cull(&machine_ctx_, encoder, queue);
            }

            // R18 — SHADOW PASS. draw_shadow_all into the shadow map(s).
            void phase_shadow_pass(RenderCtx& c) {
                auto& encoder = c.encoder;
                render_shadow_pass(&machine_ctx_, encoder);
            }

            // R19 — MAIN PASS. The rasterized scene into the backbuffer.
            void phase_main_pass(RenderCtx& c) {
                auto& encoder = c.encoder;
                auto backbuffer = c.backbuffer;
                auto depth = c.depth;
                render_main_pass(&machine_ctx_, encoder, backbuffer, c.msaaColor, depth, clearColor_, orbs_state_, orbs_deps_);
            }

            // ═══════════════════════════════════════════════════════════════
            // THE SPINE TABLES — the AUTHORED order (row order == frame order).
            // Row = {phase id, name, member fn, driver(§9), roster gate, face}.
            // A gate is a constexpr-folded bool (ROSTER bit or `true` for
            // foundational spine work). The census (tools/gates/score) audits
            // THESE ROWS: manifest = the table, attribution = row membership.
            // ═══════════════════════════════════════════════════════════════
            static constexpr URow UPDATE_SPINE[] = {
                { UPhase::FillSignal,          "fill_signal",           &Cartridge::phase_fill_signal,           Driver::Mixed,     true,             F_SIGNAL | F_CLOCK },
                { UPhase::AdvanceClock,        "advance_clock",         &Cartridge::phase_advance_clock,         Driver::Music,     true,             F_CLOCK },
                { UPhase::MotionDrivers,       "motion_drivers",        &Cartridge::phase_motion_drivers,        Driver::Music,     true,             F_CONFIG },
                { UPhase::MotionBodies,        "motion_bodies",         &Cartridge::phase_motion_bodies,         Driver::WallClock, ROSTER.pawn_aura, F_NONE },
                { UPhase::StageWorld,          "stage_world",           &Cartridge::phase_stage_world,           Driver::Algo,      true,             F_CONFIG },
                { UPhase::StageUpload,         "stage_upload",          &Cartridge::phase_stage_upload,          Driver::None,      true,             F_SIGNAL | F_CONFIG },
                { UPhase::ClearInputDeltas,    "clear_input_deltas",    &Cartridge::phase_clear_input_deltas,    Driver::None,      true,             F_NONE },
            };
            static constexpr RRow RENDER_SPINE[] = {
                { RPhase::WitnessHarvest,      "witness_harvest",       &Cartridge::phase_witness_harvest,       Driver::Algo,      true,                                   F_WITNESS },
                { RPhase::SurfaceVisibility,   "surface_visibility",    &Cartridge::phase_surface_visibility,    Driver::Algo,      true,                                   F_STREAM },
                { RPhase::CensusDumps,         "census_dumps",          &Cartridge::phase_census_dumps,          Driver::WallClock, true,                                   F_NONE },
                { RPhase::RibbonTick,          "ribbon_tick",           &Cartridge::phase_ribbon_tick,           Driver::Mixed,     ROSTER.ribbon,                          F_SIGNAL },
                { RPhase::EntityMeshGen,       "entity_mesh_gen",       &Cartridge::phase_entity_mesh_gen,       Driver::Algo,      true,                                   F_COMPUTE },
                { RPhase::UploadLights,        "upload_lights",         &Cartridge::phase_upload_lights,         Driver::Algo,      true,                                   F_CONFIG },
                { RPhase::LiveCardWrite,       "live_card_write",       &Cartridge::phase_live_card_write,       Driver::Mixed,     true,                                   F_COMPUTE },
                { RPhase::DispatchCompute,     "dispatch_compute",      &Cartridge::phase_dispatch_compute,      Driver::Mixed,     true,                                   F_COMPUTE },
                { RPhase::WitnessCapture,      "witness_capture",       &Cartridge::phase_witness_capture,       Driver::None,      true,                                   F_WITNESS },
                { RPhase::AutomatonStep,       "automaton_step",        &Cartridge::phase_automaton_step,        Driver::Algo,      true,                                   F_COMPUTE },
                { RPhase::PawnAura,            "pawn_aura",             &Cartridge::phase_pawn_aura,             Driver::WallClock, ROSTER.pawn_aura,                       F_COMPUTE },
                { RPhase::OrbSky,              "orb_sky",               &Cartridge::phase_orb_sky,               Driver::Mixed,     ROSTER.orbs,                            F_COMPUTE },
                { RPhase::FrustumCull,         "frustum_cull",          &Cartridge::phase_frustum_cull,          Driver::Algo,      true,                                   F_COMPUTE },
                { RPhase::ShadowPass,          "shadow_pass",           &Cartridge::phase_shadow_pass,           Driver::None,      true,                                   F_DRAW },
                { RPhase::MainPass,            "main_pass",             &Cartridge::phase_main_pass,             Driver::None,      true,                                   F_DRAW },
            };

            // ═══ THE FRAME METER ════════════════════════════════════════════
            // A view of the spine — it lives beside the tables it reads (a
            // module home waits for a second consumer). CPU ms per executed
            // row (sum+max) over a census window; census_dumps prints the
            // table against FRAME_BUDGET_MS, then reset(). Gated-off rows
            // are never timed.
            //
            // THE DIAL: the whole instrument answers to INSTRUMENTS.frame_meter
            // (core/instruments.hpp), OFF by default. The state below still
            // EXISTS in the off build — a few hundred bytes of zeroed rows —
            // but nothing reads or writes it: no clock pair around a row, no
            // armed timestamp pair, no resolve, no readback, no table. The
            // structure is kept whole so a measurement session is one define
            // (T7_INSTRUMENTS=meter) and a rebuild, never a re-authoring.
            // ═══ THE HOST S ROWS (OIL_1a; ledger: S0 host tail, C10) ═══
            // The harness's stations — the frame's previously unmetered
            // tail. The host feeds the meter through ONE narrow
            // dial-gated door (meter_note_host below): one meter, one
            // table, one printer.
            // TIMER LAW: an S row names where a wait SURFACES, not where
            // the cost lives — Begin carries the event pump, Acquire and
            // Present carry swapchain backpressure, FinishSubmit carries
            // command-buffer validation; FrameTotal brackets the whole
            // frame() body, so the census's residue line is the gap no
            // row carries.
            enum class HostRow : uint32_t {
                Begin, Acquire, FinishSubmit, Present, FrameTotal, COUNT
            };

            struct FrameMeter {
                static constexpr float FRAME_BUDGET_MS = 16.6f;   // the named budget
                struct RowStat { double sum_ms = 0.0; float max_ms = 0.0f; };
                RowStat u_rows[(size_t)UPhase::COUNT];
                RowStat r_rows[(size_t)RPhase::COUNT];
                RowStat s_rows[(size_t)HostRow::COUNT];   // the host S rows (OIL_1a)
                uint32_t window_frames = 0;
                std::chrono::steady_clock::time_point window_start =
                    std::chrono::steady_clock::now();   // for fps
                // GPU half (M2): per-row pass ms from timestamp queries,
                // merged into the census when samples exist.
                RowStat r_gpu[(size_t)RPhase::COUNT];
                uint32_t gpu_sampled_frames = 0;
                // HEADROOM_0 U1 — THE ENVELOPE. Per frame,
                // max(pair end) - min(pair begin) over every resolved pair.
                // NOT a row: it is not a phase, and folding it into r_gpu
                // would put it in a table whose column header says
                // "per-frame sum", which is the one thing it is not.
                //
                // WHY IT IS THE PURSE. §4b forbids summing brackets, and it
                // is right to: pairs overlap, so a sum over-counts and a
                // single row under-counts. The envelope does neither. It is
                // the wall-span of the frame's GPU work, and it BOUNDS
                // occupancy from above regardless of how the brackets lie
                // inside it — additive-safe by construction, because it
                // never adds anything. budget 16.6 - envelope is the
                // headroom the coupling era has to spend.
                //
                // Pure arithmetic over timestamps already resolved for the
                // rows. Zero new queries, zero new GPU cost.
                RowStat gpu_envelope;
                uint32_t gpu_over_budget_frames = 0;   // RIBBON_6
                // Snapshot of the armed pair table, taken at frame close
                // and consumed by the mapped callback. NOT zeroed by
                // reset() — a readback may be in flight across a window.
                MeterPair snap_pairs[GPUState::meter_max_pairs()] = {};
                uint32_t snap_pair_count = 0;
                // PANORAMA_0 §5.5 — HOW OFTEN A FAMILY REGENERATES ITS MESH.
                // The EntityMeshGen row already says what a firing COSTS; it
                // cannot say how many there were, and the two questions have
                // different answers on a ride, where spawns and evictions keep
                // the dispatch firing. This is the instrument that gates the
                // per-slot regeneration campaign (F3) and proves it after: the
                // count should fall to the number of slots that actually
                // changed, and the row's ms with it.
                uint32_t mesh_gen_firings[PopFamily::COUNT] = {};
                void reset() {   // zero rows + frames, restamp window_start
                    for (auto& s : u_rows) s = RowStat{};
                    for (auto& s : r_rows) s = RowStat{};
                    for (auto& s : s_rows) s = RowStat{};
                    for (auto& s : r_gpu) s = RowStat{};
                    gpu_envelope = RowStat{};
                    gpu_over_budget_frames = 0;
                    window_frames = 0;
                    gpu_sampled_frames = 0;
                    for (auto& n : mesh_gen_firings) n = 0;
                    window_start = std::chrono::steady_clock::now();
                }
            };
            FrameMeter meter_;

            // The host door (OIL_1a): the harness clocks its own brackets
            // and notes the ms here. Dial off: the body folds to an empty
            // inline — the same zero-fold standard the ledger verified
            // for the conductors' clock pairs (X meter-off fold, CLEAN).
            void meter_note_host(HostRow row, float ms) {
                if constexpr (!INSTRUMENTS.frame_meter) { (void)row; (void)ms; return; }
                auto& s = meter_.s_rows[(size_t)row];
                s.sum_ms += ms; if (ms > s.max_ms) s.max_ms = ms;
            }

            // The meter_row registry (state.hpp — the GPU half's raw row
            // ids) is pinned to RPhase HERE, at the enum's home. Drift
            // fails glaw1.
            static_assert(meter_row::SurfaceVisibility   == (uint32_t)RPhase::SurfaceVisibility,   "meter_row drift: SurfaceVisibility");
            static_assert(meter_row::EntityMeshGen       == (uint32_t)RPhase::EntityMeshGen,       "meter_row drift: EntityMeshGen");
            static_assert(meter_row::DispatchCompute     == (uint32_t)RPhase::DispatchCompute,     "meter_row drift: DispatchCompute");
            static_assert(meter_row::AutomatonStep       == (uint32_t)RPhase::AutomatonStep,       "meter_row drift: AutomatonStep");
            static_assert(meter_row::PawnAura            == (uint32_t)RPhase::PawnAura,            "meter_row drift: PawnAura");
            static_assert(meter_row::OrbSky              == (uint32_t)RPhase::OrbSky,              "meter_row drift: OrbSky");
            static_assert(meter_row::FrustumCull         == (uint32_t)RPhase::FrustumCull,         "meter_row drift: FrustumCull");
            static_assert(meter_row::ShadowPass          == (uint32_t)RPhase::ShadowPass,          "meter_row drift: ShadowPass");
            static_assert(meter_row::MainPass            == (uint32_t)RPhase::MainPass,            "meter_row drift: MainPass");

            // ═══ SPINE VALIDATION ═══════════════════════
            // Every CROSS-PHASE O-# / RC law the recon named is a static_assert
            // over the row indices: the frame CANNOT be authored out of its
            // lawful order (a reorder fails the BUILD, not the pixel rig). The
            // by-design lags (E-3/E-4/E-9) are declared as law lines in the
            // spine header above. Two laws are INTRA-phase, not row-index laws,
            // enforced by structure inside a single phase: O-3 (the TEARDOWN
            // fixed sequence, inside rebirth_world) and O-6a (the
            // sync->evolve barrier = the SEPARATE compute passes inside
            // phase_automaton_step).
            static constexpr bool spine_ordered_u() {
                for (std::size_t i = 0; i < (std::size_t)UPhase::COUNT; i++)
                    if ((std::size_t)UPDATE_SPINE[i].id != i) return false;
                return true;
            }
            static constexpr bool spine_ordered_r() {
                for (std::size_t i = 0; i < (std::size_t)RPhase::COUNT; i++)
                    if ((std::size_t)RENDER_SPINE[i].id != i) return false;
                return true;
            }
            // O-5b/c (face-based): no SIGNAL/CONFIG staging phase may follow the
            // drain (StageUpload) — a future staging phase placed after it
            // fails to build, not silently drops for the frame.
            static constexpr bool no_staging_after_drain() {
                for (std::size_t i = 0; i < (std::size_t)UPhase::COUNT; i++)
                    if (UPDATE_SPINE[i].id != UPhase::StageUpload &&
                        (UPDATE_SPINE[i].face & (F_SIGNAL | F_CONFIG)) &&
                        i > (std::size_t)UPhase::StageUpload) return false;
                return true;
            }

            static_assert(sizeof(UPDATE_SPINE) / sizeof(URow) == (std::size_t)UPhase::COUNT, "update spine must be dense");
            static_assert(sizeof(RENDER_SPINE) / sizeof(RRow) == (std::size_t)RPhase::COUNT, "render spine must be dense");
            // Table-order integrity + the O-5b/c face law are BOOT asserts
            // (validate_spine, called once at init): a constexpr member fn
            // cannot be static_asserted inside its own incomplete class.
            // update laws:
            static_assert((uint32_t)UPhase::FillSignal < (uint32_t)UPhase::AdvanceClock, "O-5a: dt_beats reads prev_beats before the clock advances it");
            // E-3 (sky write-order) died with the sky block (RIBBON_1): the
            // signal has one author and one whole-struct write again.
            static_assert((uint32_t)UPhase::ClearInputDeltas + 1 == (uint32_t)UPhase::COUNT, "O-5e: clear_input_deltas is dead-last");
            // render laws:
            static_assert((uint32_t)RPhase::RibbonTick < (uint32_t)RPhase::DispatchCompute, "O-1: the ribbon's state write precedes the compute that reads it");
            static_assert((uint32_t)RPhase::WitnessHarvest < (uint32_t)RPhase::DispatchCompute, "O-2: witness harvest before compute");
            static_assert((uint32_t)RPhase::DispatchCompute < (uint32_t)RPhase::WitnessCapture, "O-2: witness capture after compute (feeds next frame's harvest)");
            static_assert((uint32_t)RPhase::FrustumCull < (uint32_t)RPhase::ShadowPass, "O-7: frustum cull precedes the shadow pass (ordering pin)");
            static_assert((uint32_t)RPhase::FrustumCull < (uint32_t)RPhase::MainPass, "O-7: frustum cull before the main pass (indirect draws consume the cull)");
            static_assert((uint32_t)RPhase::LiveCardWrite > (uint32_t)RPhase::UploadLights &&
                          (uint32_t)RPhase::LiveCardWrite < (uint32_t)RPhase::DispatchCompute,
                "GROUND_CARD_1: the card writes before the consumers "
                "(pre-evolve zone read preserved, R10<R13 order intact)");
            // THE ORDERING ASSERT THAT STOOD HERE IS SPENT WITH ITS SUBJECT.
            // It read "gol: the derive flush (hidden submit) precedes the
            // zone compute that reads it" — a real ordering law over two
            // rows, and the whole reason the hidden submit had to be a row
            // at all. One row, nothing to order (ONE_SURFACE-II U1).
            static_assert((uint32_t)RPhase::ShadowPass < (uint32_t)RPhase::MainPass, "draw: shadow before main");

            // BOOT VALIDATION (always-on): table-order integrity + the O-5b/c
            // face law — the checks a constexpr member fn cannot static_assert
            // inside its own incomplete class. Fails LOUD at boot, never silent.
            void validate_spine() const {
                if (!spine_ordered_u() || !spine_ordered_r() || !no_staging_after_drain()) {
                    std::cerr << "[SPINE] VALIDATION FAILED — row order / O-5b/c face law violated\n";
                    std::abort();
                }
                // F-2: FAMILY_DISPATCH rows are
                // POSITIONAL in PopFamily order and each carries its name —
                // check every row's name against the canonical
                // family_short_name list, so a row swap fails LOUD at boot,
                // never silent (the table is inline const, not constexpr, so
                // this cannot be a static_assert).
                for (uint32_t f = 0; f < PopFamily::COUNT; f++) {
                    const char* have = FAMILY_DISPATCH[f].name;
                    const char* want = family_short_name(f);
                    bool eq = (have != nullptr);
                    for (uint32_t i = 0; eq; i++) {
                        if (have[i] != want[i]) { eq = false; break; }
                        if (have[i] == '\0') break;
                    }
                    if (!eq) {
                        std::cerr << "[SPINE] FAMILY_DISPATCH row " << f << " name '"
                            << (have ? have : "<null>") << "' != PopFamily order '"
                            << want << "' (F-2)\n";
                        std::abort();
                    }
                }
                std::cout << "[SPINE] validated: " << (uint32_t)UPhase::COUNT << " update rows + "
                    << (uint32_t)RPhase::COUNT << " render rows + "
                    << (uint32_t)PopFamily::COUNT << " dispatch rows name-checked; "
                    << "O-#/RC laws static-asserted\n";
            }

            // ── THE CONDUCTOR (render) — a LOOP over RENDER_SPINE (§1b) ─────
            void render(wgpu::CommandEncoder& encoder,
                wgpu::TextureView backbuffer,
                wgpu::TextureView msaaColor,
                wgpu::TextureView depth) override {
                RenderCtx ctx{ encoder, queue_, backbuffer, msaaColor, depth };   // OIL_1 U1: the cached queue — no per-frame GetQueue
                if constexpr (INSTRUMENTS.frame_meter) meter_.window_frames++;

                // THE DRAW LEDGER'S FRAME BOUNDARY (BUNDLE_1). Every count
                // the CPU authors is staged here and flushed before a single
                // pass is encoded — the update spine has run, so the numbers
                // are this frame's, and a WriteBuffer issued now lands ahead
                // of the command buffer on the queue timeline. flush writes
                // only what moved; a steady frame writes nothing.
                stage_draw_ledger(&machine_ctx_, orbs_state_);
                gpuState_.flush_draw_ledger(queue_);

                // THE BUNDLES (BUNDLE_1) — recorded here, before the passes
                // that execute them, and only when something a bundle
                // CAPTURED moved: a recreated bind group or buffer (R-B
                // found none post-boot) or a subtraction-mask dial. Not per
                // frame — that is the whole point. The ledger is flushed
                // first because a bundle must capture a buffer that exists;
                // its CONTENTS are read at execution, not at recording.
                if (gpuState_.bundles_dirty())
                    record_bundles(&machine_ctx_, orbs_state_, orbs_deps_);

                // THE FRAME METER — GPU half, harvest side. Mirrors the
                // floater readback grammar exactly (COPIED → MapAsync →
                // MAPPING; callback accumulates, Unmaps, → IDLE;
                // AllowSpontaneous). No world_gen capture: timing rows are
                // world-agnostic. dt discards per the spec's counter-reset
                // note: end ≤ begin, or dt > 100 ms.
                // The whole GPU half — harvest, arming table, frame-close
                // resolve — rides the instruments dial. Off, the frame issues
                // no ResolveQuerySet, no staging copy, and no MapAsync, and
                // keeps no readback in flight (core/instruments.hpp).
                if constexpr (INSTRUMENTS.frame_meter) {
                    if (slotReadbackState_ == MeterReadbackState::COPIED) {
                        slotReadbackState_ = MeterReadbackState::MAPPING;
                        gpuState_.frustum_count_readback().MapAsync(
                            wgpu::MapMode::Read, 0, GPUState::frustum_indirect_size(),
                            wgpu::CallbackMode::AllowSpontaneous,
                            [](wgpu::MapAsyncStatus status, wgpu::StringView, Cartridge* self) {
                                if (status == wgpu::MapAsyncStatus::Success) {
                                    const auto* a = static_cast<const uint32_t*>(
                                        self->gpuState_.frustum_count_readback().GetConstMappedRange(
                                            0, GPUState::frustum_indirect_size()));
                                    if (a) {
                                        // instanceCounts at 1 / 6 — the TWO
                                        // 5-u32 draw-arg slots (state.hpp's
                                        // reset_frustum_indirect names the
                                        // layout). Index 11 was slot C's and
                                        // is past the end of a 40-byte map.
                                        self->slotInstances_[0] = a[1];
                                        self->slotInstances_[1] = a[6];
                                        self->slotSampleValid_ = true;
                                    }
                                    self->gpuState_.frustum_count_readback().Unmap();
                                }
                                self->slotReadbackState_ = MeterReadbackState::IDLE;
                            }, this);
                    }
                    if (meterReadbackState_ == MeterReadbackState::COPIED) {
                        meterReadbackState_ = MeterReadbackState::MAPPING;
                        gpuState_.meter_readback_staging().MapAsync(
                            wgpu::MapMode::Read, 0, GPUState::meter_readback_size(),
                            wgpu::CallbackMode::AllowSpontaneous,
                            // OIL_1c: captureless, `this` on the userdata slot.
                            // No generation member here — timing rows are
                            // world-agnostic, so this machine never carried one.
                            [](wgpu::MapAsyncStatus status, wgpu::StringView, Cartridge* self) {
                                if (status == wgpu::MapAsyncStatus::Success) {
                                    const auto* ts = static_cast<const uint64_t*>(
                                        self->gpuState_.meter_readback_staging().GetConstMappedRange(
                                            0, GPUState::meter_readback_size()));
                                    if (ts) {
                                        // METER_1.1: group pair dts by row into a
                                        // frame-local total FIRST, then fold sum/max
                                        // from the per-frame totals — multi-pass rows
                                        // (orb quartet, shadow atlas, patch batches)
                                        // otherwise printed a per-pass max below their
                                        // per-frame mean.
                                        double frame_ms[(size_t)RPhase::COUNT] = {};
                                        // HEADROOM_0 U1 — the envelope's extremes,
                                        // gathered in the SAME loop and under the
                                        // SAME discard law, so a pair rejected from
                                        // the rows cannot widen the envelope either.
                                        uint64_t env_lo = 0, env_hi = 0;
                                        bool env_any = false;
                                        for (uint32_t p = 0; p < self->meter_.snap_pair_count; p++) {
                                            const uint64_t t0 = ts[self->meter_.snap_pairs[p].begin_idx];
                                            const uint64_t t1 = ts[self->meter_.snap_pairs[p].begin_idx + 1];
                                            if (t1 <= t0) continue;               // counter-reset garbage
                                            const double ms = (double)(t1 - t0) * 1e-6;   // u64 ns per index
                                            if (ms > 100.0) continue;             // same discard law
                                            frame_ms[self->meter_.snap_pairs[p].row] += ms;
                                            if (!env_any) { env_lo = t0; env_hi = t1; env_any = true; }
                                            else {
                                                if (t0 < env_lo) env_lo = t0;
                                                if (t1 > env_hi) env_hi = t1;
                                            }
                                        }
                                        if (env_any) {
                                            // One number per frame: the span from the
                                            // earliest begin to the latest end. Folded
                                            // like a row so the window reports it with
                                            // the same mean/max grammar, but it is a
                                            // SPAN, not a sum -- and the print says so.
                                            const double env_ms = (double)(env_hi - env_lo) * 1e-6;
                                            auto& e = self->meter_.gpu_envelope;
                                            e.sum_ms += env_ms;
                                            if ((float)env_ms > e.max_ms) e.max_ms = (float)env_ms;
                                            // RIBBON_6: the direct measure of what
                                            // [PRESENT] sees downstream. A frame
                                            // whose GPU span exceeded the refresh
                                            // budget is a frame the panel may have
                                            // shown twice; the mean and max cannot
                                            // say how MANY, and the count can.
                                            if (env_ms > FrameMeter::FRAME_BUDGET_MS)
                                                self->meter_.gpu_over_budget_frames++;
                                        }
                                        // THE MAX ASYMMETRY (TIDY_0d, present
                                        // behavior). The discard above is
                                        // PER-PAIR; the max folded below is over
                                        // the PER-FRAME SUM. So a frame arming
                                        // many pairs on one row can post a max
                                        // that no individual pair could produce,
                                        // and no discard applies to the sum at
                                        // any size. WEB_METER_0 saw exactly this
                                        // twice — 1323.04 and 1198.06 gpu max
                                        // on the row that was `stream_patches`
                                        // when WEB_METER_0 measured it (the
                                        // conductor left at ONE_SURFACE-I U2;
                                        // its per-frame survivor is
                                        // `band_patches`, under
                                        // RPhase::SurfaceVisibility). Both in
                                        // boot-adjacent
                                        // frames that upload many patches. The
                                        // asymmetry is deliberate: the per-frame
                                        // sum is the honest per-frame cost of a
                                        // multi-pass row (METER_1.1 above), and
                                        // the per-pair discard is what keeps
                                        // counter-reset garbage out of it. The
                                        // window header names the column so the
                                        // number is not misleading on its face.
                                        for (size_t r = 0; r < (size_t)RPhase::COUNT; r++) {
                                            if (frame_ms[r] <= 0.0) continue;
                                            auto& s = self->meter_.r_gpu[r];
                                            s.sum_ms += frame_ms[r];
                                            if ((float)frame_ms[r] > s.max_ms) s.max_ms = (float)frame_ms[r];
                                        }
                                        self->meter_.gpu_sampled_frames++;
                                    }
                                    self->gpuState_.meter_readback_staging().Unmap();
                                }
                                self->meterReadbackState_ = MeterReadbackState::IDLE;
                            },
                            this);
                    }
                    gpuState_.meter_frame_begin();   // rebuild this frame's arming table
                }

                for (const RRow& row : RENDER_SPINE) {
                    if (!row.enabled) continue;   // gated-off rows are never timed
                    if constexpr (INSTRUMENTS.frame_meter) {
                        auto t0 = std::chrono::steady_clock::now();
                        (this->*row.fn)(ctx);
                        auto t1 = std::chrono::steady_clock::now();
                        float ms = std::chrono::duration<float, std::milli>(t1 - t0).count();
                        auto& s = meter_.r_rows[(size_t)row.id];
                        s.sum_ms += ms; if (ms > s.max_ms) s.max_ms = ms;
                    }
                    else {
                        (this->*row.fn)(ctx);
                    }
                }

                // Frame close (after the last spine row, before the host's
                // Finish/Submit): resolve this frame's armed pairs, stage
                // the copy, snapshot the pair table. SKIP-IF-BUSY is the
                // law — at most one readback in flight.
                if constexpr (INSTRUMENTS.frame_meter) {
                    if (meter_gpu_ && gpuState_.meter_pair_count() > 0 &&
                        meterReadbackState_ == MeterReadbackState::IDLE) {
                        const uint32_t n = 2 * gpuState_.meter_pair_count();
                        encoder.ResolveQuerySet(gpuState_.meter_query_set(), 0, n,
                            gpuState_.meter_resolve_buffer(), 0);
                        encoder.CopyBufferToBuffer(
                            gpuState_.meter_resolve_buffer(), 0,
                            gpuState_.meter_readback_staging(), 0,
                            n * sizeof(uint64_t));
                        meter_.snap_pair_count = gpuState_.meter_pair_count();
                        for (uint32_t p = 0; p < meter_.snap_pair_count; p++)
                            meter_.snap_pairs[p] = gpuState_.meter_pairs()[p];
                        meterReadbackState_ = MeterReadbackState::COPIED;
                    }
                    // U4 — the plan's counters, this frame's. The cull pass
                    // reset and rewrote them earlier in this same encoder, so
                    // the copy takes the frame it was encoded in.
                    if (meter_gpu_ && slotReadbackState_ == MeterReadbackState::IDLE) {
                        encoder.CopyBufferToBuffer(
                            gpuState_.frustum_compute_buffer(), 0,
                            gpuState_.frustum_count_readback(), 0,
                            GPUState::frustum_indirect_size());
                        slotReadbackState_ = MeterReadbackState::COPIED;
                    }
                }
            }

            // THE SKY is THE DRAW + ONE APPLIER + THE BIRTH SEQUENCE: the
            // door, applier and deriver declarations are in sky.hpp (file
            // scope, above the class); the definitions live in the same
            // header's MODULE IMPLEMENTATION zone (the merged file, pre-class
            // in the cohort). THE SKY OWNS NO STATE — nothing at the
            // COMPOSITION ROOT; sky_state_ is spine-resident
            // (SEAM[spine:transitions], L38 — assembly only; this is declared
            // orchestration). It was MOOD, a vocabulary and four appliers; the
            // vocabulary died at ONE_WORLD-II U2 and U4 and the file was
            // renamed at U7. The force-spawn mutation that belonged to the
            // arch's owner left with the doors (ONE_WORLD-I U2). See §1.

        public:

            // RIBBON_2 P0 1.2b: an updated-but-unrendered frame adds its dt to
            // the next rendered one — a dropped acquire stretches a step,
            // never deletes it. The host calls this once the frame's command
            // buffer is submitted, which is the only moment the GPU is known
            // to have been given the time this accumulator was holding.
            void frame_submitted() { dtPending_ = 0.0f; }

            void on_input(const InputEvent& event) override {
                switch (event.type) {
                case InputEvent::Type::KeyDown:
                    // The command fan's TARGET organs ride the call site:
                    // the root addresses the fan's
                    // bodies per event, through the owner doors — the driver
                    // owns none of them (v3 §9 Act I). The F6 socket stays
                    // RESERVED for a real addressing need.
                    on_key_down(&input_deps_, event.key,
                        pawn_state_, pawn_deps_,
                        orbs_state_, orbs_deps_,
                        agent_state_, agents_deps_,
                        cube_behaviors_state_, cube_deps_);
                    break;
                case InputEvent::Type::KeyUp:
                    on_key_up(&input_deps_, event.key);
                    break;
                case InputEvent::Type::MouseMove:
                    on_mouse_move(&input_deps_, event.x, event.y);
                    break;
                case InputEvent::Type::MouseButton:
                    on_mouse_button(&input_deps_, event.button, event.pressed);
                    break;
                case InputEvent::Type::Scroll:
                    on_scroll(&input_deps_, event.y);
                    break;
                // SHIP_1 — the touch doors. Each lands on the organ its
                // mouse/key sibling lands on; the console already
                // resolved which gesture this was.
                case InputEvent::Type::TouchMove:
                    on_touch_move(&input_deps_, event.x, event.y);
                    break;
                case InputEvent::Type::TouchLook:
                    on_touch_look(&input_deps_, event.x, event.y);
                    break;
                case InputEvent::Type::TouchZoom:
                    on_touch_zoom(&input_deps_, event.y);
                    break;
                case InputEvent::Type::TouchTapLeft:
                    on_touch_tap_left(&input_deps_, pawn_state_, pawn_deps_);
                    break;
                case InputEvent::Type::TouchTapRight:
                    on_touch_tap_right(&input_deps_, agent_state_, agents_deps_);
                    break;
                }
            }

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

// ═══ THE POST-CLASS ZONE — EMPTY OF MODULES ═══════════════════════════
//
// Every module rides ONE pre-class header, its decl tier at the
// contracts. What remains below is the
// spine's own table — FAMILY_DISPATCH — which was never a module.
// ═══ THE TABLE — FAMILY_DISPATCH ═══════════════════════════════════
// The definition
// is SEAM[spine:owns] spine work — it takes the Cartridge mesh-wrapper
// static ADDRESSES and the family row addresses, so it lives with its
// owner, the composition root, at the post-class point.
namespace t7 {
    namespace the_board {
        // ─── Shared no-op adapters ────────────────────────────────────────

        inline bool dispatch_prepare_mesh_none(MachineCtx* self, wgpu::Queue& queue) {
            (void)self; (void)queue;
            return false;
        }
        inline void dispatch_mesh_gen_none(MachineCtx* self, wgpu::ComputePassEncoder& pass) {
            (void)self; (void)pass;
        }

        // ─── Census: the per-family active_count + slot_census rows ────────
        //
        // THE COUNT IS A `.active` SCAN. NEVER A STORED FIELD. A stored
        // counter would be a number no consumer has ever validated; the
        // scan is ground truth. (The write-only per-family counters the
        // modules once carried were cut once the scan became the census.)
        // ARCH_2 extends the rule, it does not bend it: slot_census's
        // high-water is scanned too — it is the reach AT THAT SCAN, not a
        // running maximum, and nothing below stores state between dumps.
        //
        // The bound is DEDUCED from the array, never written. That is not
        // brevity — it makes three standing traps structurally unreachable:
        //   · MAX_RIBBON_INSTANCES is a t7::the_board namespace constant,
        //     NOT a Dim:: member. Nothing here has to know that.
        //   · Dim::CUBE_SLOT_OFFSET (8) is GPU-side only; the CPU array is
        //     0-based. No offset can leak in, because no index arithmetic is
        //     written.
        //   · gol_state_.active_slot_count and cpu_pyramids.count are
        //     HIGH-WATER MARKS (highest active slot + 1), not populations.
        //     Both are live-read, which is what makes them tempting; neither
        //     is reachable from here. ARCH_2 makes this bullet sharper, not
        //     stale: slot_census now PRINTS a high-water, so the temptation
        //     is no longer "close enough" but "the same word". It is still
        //     the wrong number — those two fields are maintained for their
        //     own consumers, on their own cadence, for two families out of
        //     six. The census scans; it does not borrow.
        //
        // ROSTER-disabled families are NOT special-cased: a disabled family is
        // never selected, so its array stays empty and it reads zero on both
        // sides. That agreement is itself a check.

        // One implementation, six callers (the P11 shape).
        template<typename T, size_t N>
        inline uint32_t census_scan_active(const T (&arr)[N]) {
            uint32_t n = 0;
            for (size_t i = 0; i < N; i++) if (arr[i].active) n++;
            return n;
        }

        // The occupancy triple (ARCH_2), same shape and same deduced bound.
        // N is the capacity: that is the ONLY way the ceiling reaches the
        // census without a constant being named here, which is what keeps the
        // three traps above unreachable — the ribbon bound is not a
        // Dim:: member, and the cube slot offset is GPU-side.
        //
        // Deliberately a SECOND pass over the array, not a merge with
        // census_scan_active. The two numbers must be able to disagree: `live`
        // feeds the delta column that catches ground/body leaks, and a
        // diagnostic that silently re-derived it would put the leak check
        // downstream of itself. Six arrays at ≤288 entries, once per
        // census dump — the cost is not measurable beside the print.
        template<typename T, size_t N>
        inline SlotCensus census_scan_slots(const T (&arr)[N]) {
            SlotCensus s{ 0u, 0u, static_cast<uint32_t>(N) };
            for (size_t i = 0; i < N; i++) {
                if (!arr[i].active) continue;
                s.live++;
                s.high_water = static_cast<uint32_t>(i) + 1u;   // ascending scan: last write wins
            }
            return s;
        }

        inline uint32_t active_count_pyramid(const MachineCtx* c) { return census_scan_active(c->entities_state_.pyramids); }
        inline uint32_t active_count_sphere (const MachineCtx* c) { return census_scan_active(c->sphere_state_.activeSpheres_); }
        inline uint32_t active_count_ribbon (const MachineCtx* c) { return census_scan_active(c->ribbon_state_.active); }
        inline uint32_t active_count_cube   (const MachineCtx* c) { return census_scan_active(c->cube_behaviors_state_.activeCubes_); }

        // The slot_census row — the SAME six arrays, named once more so the
        // capacity travels with the population. Any divergence between these
        // two lists is a family reporting its live count off one array and its
        // ceiling off another, so they are kept adjacent on purpose.
        inline SlotCensus slot_census_pyramid(const MachineCtx* c) { return census_scan_slots(c->entities_state_.pyramids); }
        inline SlotCensus slot_census_sphere (const MachineCtx* c) { return census_scan_slots(c->sphere_state_.activeSpheres_); }
        inline SlotCensus slot_census_ribbon (const MachineCtx* c) { return census_scan_slots(c->ribbon_state_.active); }
        inline SlotCensus slot_census_cube   (const MachineCtx* c) { return census_scan_slots(c->cube_behaviors_state_.activeCubes_); }

        // ─── The table ─────────────────────────────────────────────────────
        // AXES: one row per family, POSITIONAL in PopFamily order (PYRAMID=0,
        //   SPHERE, RIBBON, CUBE=3) — the enum values are pinned at roster.hpp (F-1)
        //   and every row's trailing name string is boot-checked against
        //   family_short_name by validate_spine (F-2), so a row swap fails
        //   LOUD. Row columns (FamilyDispatch, entity_types.hpp):
        //     { try_select, try_place, try_commit,
        //       prepare_mesh, dispatch_mesh, active_count, slot_census,
        //       grounded, name }
        // CONSUMERS: the machine tail walks select/place/commit per queue
        //   entry; the evict_slot row left at ONE_SURFACE-I U3; the mesh pair feeds the
        //   RENDER_UPDATE mesh phases (none-fork = family has no mesh).
        inline const FamilyDispatch FAMILY_DISPATCH[PopFamily::COUNT] = {
            { dispatch_select_pyramid_generic, dispatch_place_pyramid_generic, dispatch_commit_pyramid_generic,
              dispatch_prepare_mesh_none, dispatch_mesh_gen_none,   // mesh hook → none-fork: pyramid mesh dead-by-design; placement feeds the heightfield
              active_count_pyramid,
              slot_census_pyramid,
              PYRAMID_TRAITS.grounded,
              "pyr" },
            { dispatch_select_sphere_generic, dispatch_place_sphere_generic, dispatch_commit_sphere_generic,
              dispatch_prepare_mesh_none, dispatch_mesh_gen_none,
              active_count_sphere,
              slot_census_sphere,
              SPHERE_TRAITS.grounded,   // false — orbits an anchor, claims no ground
              "sph" },   // no CPU mesh gen — GPU compute handles update_sphere
            { dispatch_select_ribbon, dispatch_place_ribbon, dispatch_commit_ribbon,
              dispatch_prepare_mesh_none, dispatch_mesh_gen_none,
              active_count_ribbon,
              slot_census_ribbon,
              true,   // anchored: the tips touch ground (no TRAITS object)
              "ribn" },  // no CPU mesh gen — GPU compute handles ribbon rendering
            { dispatch_select_cube_generic, dispatch_place_cube_generic, dispatch_commit_cube_generic,
              dispatch_prepare_mesh_none, dispatch_mesh_gen_none,
              active_count_cube,
              slot_census_cube,
              CUBE_TRAITS.grounded,      // false — hovers and drifts, claims no ground
              "cube" },  // no CPU mesh gen — GPU compute handles update_cube
            // THE GOL ROW LEFT AT ONE_SURFACE-II U2, and it was the LAST
            // row — a tail cut, so no surviving row moved. Its own trailing
            // comment had already written this unit's argument a campaign
            // early: "the zone IS the ground". U1 made that literally true,
            // and a family whose members are the ground is not a family.
        };
    } // namespace the_board
} // namespace t7
