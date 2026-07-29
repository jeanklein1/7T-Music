> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# DISSOLVE-1 — PHASE R: THE FACE CENSUS (report-first; ONE STOP)

Read-only. Puller on record: THE COVENANT (comprehension), Jean's
ruling — "The .inl were not desired from the beginning." d0 rides
this commit: v3 §7 carries the second puller verbatim. The m3 poisons
are cured by ORDER — the services face (MachineCtx) cuts FIRST, so no
module conversion is ever incomplete or fake. This report returns the
MachineCtx composition, the finalized deps table, the merge order
with its BOM plan, and the holdouts. FULL STOP for the stamp after §5.

METHOD. Two parallel readers censused the machine seam (every
dispatch-row function per family; every shared machine verb) function
by function; a mechanical sweep enumerated per-module keyhole reaches
post-campaign (post-m1 types, post-m3b faces, post-m4 doors, post-m6
sky); the gol self-submit was read and its submission order analyzed
first-hand. Citations file:line where load-bearing.

---

## 1 — R1: THE MACHINE CONTEXT (MachineCtx)

### 1.1 The design key (the escape clause, executed)

run_spawn_preamble deduces its cartridge parameter (`template
<typename C> ... C* c`, spawn_engine.hpp:221) — THE TEMPLATE KEYHOLE.
The escape clause: **MachineCtx's members are NAMED AS THE ORGANS**
(`tile_world_state_`, `spawn_engine_state_`, `gpuState_`, ...), each
a reference bound once at the root. Then:

- the preamble deduces `C = MachineCtx` with ZERO body edits — every
  `c->organ_` reach resolves against the context member of the same
  name; the signature shape does not change;
- every dispatch row and machine verb converts by PARAMETER TYPE
  ALONE — `Cartridge* self` becomes `MachineCtx* m` (or `MachineCtx&`)
  and the body stays BYTE-IDENTICAL, which is what the token-identity
  gate will verify per conversion;
- the root builds ONE MachineCtx at boot (reference members bound to
  the organs in the member-init list; the context is an organ of the
  root, wired once).

The dispatch contract (contracts/entity_types.hpp, the row type's
home) thereby SAYS what the machine may hand a family — the
requirements face made literal at the machine's boundary.

### 1.2 The composition (the union of both censuses)

```cpp
// contracts/entity_types.hpp — beside the row type it serves
struct MachineCtx {
    // S1/S2 — the surface the machine stands on
    WorldState&              world_state_;         // seed R, finite R, patch count R, free_layer R/W, 2 flags W
    const TileWorldState&    tile_world_state_;    // const BY CENSUS — only the m3b faces consume it
    const ThemesState&       themes_state_;        // active_theme_idx_ R (the preamble)
    MoodState&               mood_state_;          // active R; portals_dirty W (the arch channel)
    PatchSystemState&        patch_system_state_;  // find_patch / piers / regen / record_entity hosts
    SpawnEngineState&        spawn_engine_state_;  // queues + footprints
    // the family organs the rows own
    EntitiesState&           entities_state_;
    SphereState&             sphere_state_;
    CubeBehaviorsState&      cube_behaviors_state_;
    RibbonState&             ribbon_state_;
    GoLState&                gol_state_;
    GalleryState&            gallery_state_;
    // clock + witness (read-only by census)
    const TimeState&         time_state_;          // seconds R (footprints, spawn protection)
    const PlayerState&       player_;              // readback_x/z R (draw-culling; one DIAG print)
    // realization
    GPUState&                gpuState_;            // 33 named methods (unions below)
    Renderer&                renderer_;            // the six dispatch_*_mesh_gen only
};
```

NOT in the context, by census: `device_` (no row or verb reaches it —
the gol self-submit is score-called, §4.1 puts its device member on
GolDeps), `inputState_` (patches_budget_this_frame reads move_x/z but
is stream-side — patch_system's own deps), `agent_state_` (no row
touches it). The const trio (tiles, time, player) is load-bearing:
the machine READS the surface through the m3b faces, the clock, and
the witness — it never writes them.

THE DUAL-ENTRY RULE (d1 mechanic, for the stamp): four doors are
called from BOTH context worlds — commit_ribbon (the row + mood's
anchor applier, SEAM[ribbon:dual-entry]), load_authored_textures (the
boot movement + the gallery commit row), pick_portal_mood and
derive_finite_radius (the arch row + mood). These convert to the
DEDUCED-CONTEXT template form (the stamped TEMPLATE KEYHOLE law,
generalized: `template<typename C>` deduces Cartridge* today,
MachineCtx*/deps tomorrow — reaches checked at instantiation, bodies
byte-identical). Everything else converts by parameter-type rename
alone.

### 1.3 Per-family row reaches (census digest; the verb census carries the shared services)

| family | select | place (fail path) | commit (+adapters) | evict | mesh pair |
|---|---|---|---|---|---|
| pyramid | pyramids R/W (preamble), mood.active R | pyramids[s].active W | pyramids R/W, count W, cpu_pyramids R/W, pending W, world flag W; gpu upload_pyramids + params_slot | same set | pending R/W + set_index; renderer dispatch |
| arch | arches R/W, mood.active R | arches[s].active W | arches R/W (portal fields), count W, pending W, mood.portals_dirty W, world flag W; write_pier ×2, regen | arches W, count W, pending W, portals_dirty W; clear_pier ×2 | pending R/W + set_index; renderer dispatch |
| column | columns R/W, mood.active R | columns[s].active W | columns R/W, count W, pending W, world flag W; write_pier | columns W, count W, pending W; clear_pier | shared pair (column+antenna) |
| antenna | antennas R/W, mood.active R | antennas[s].active W | antennas R/W, count W, COLUMN pending W, world flag W; write_pier | antennas W, count W, column pending W; clear_pier | rides column's pair |
| palm / cactus / blade | own array R/W, mood.active R | own[s].active W | own R/W, count W, pending W, world flag W | own W, count W, pending W, world flag W | own pending pair |
| sphere | activeFloaters_ R/W, mood.active R | [s].active W | floaters R/W (last_alloc_time), count W, time.seconds R, world flag W; NO record_entity (decoupled) | floaters W, count W | none-adapters |
| cube | activeCubes_ R/W, mood.active R | [s].active W | cubes R/W, count W, time R, mood.active R, world flag W; NO record_entity | cubes W, count W | none-adapters |
| ribbon | active R/W (tip scan), player_.readback R | [s].active W | gpu[s] W, active R/W, count W, time R; find_patch ×2 + record_entity ×2 (two-tip) | ref-count law, sky.mode R, rendered_slot R/W, gpu upload | none-adapters |
| gol | mood_allowed R (gate), zones R/W, mood.active R, tiles const (faces), world.seed R | zones[s].active W | zones W, count W, pending_derive R/W, world.seed R; gpu upload_zone_life | zones W, count W; gpu deactivate | none-adapters |
| gallery | centers R/W, painting_slots R, snapshot_count R, mood.active R, tiles const ×2, world.seed R | centers[s].active W | the widest: centers/slots/staging/exhibition/promotions R/W, world.seed R; load_authored_textures (dual door) | centers/slots/exhibition W; gpu deactivate | none-adapters |

### 1.4 The machine verbs (census digest)

run_spawn_preamble (the P11 template): mood.active R, themes idx R,
tiles const via F3; writes the CALLER's actives array (slot reserve).
evaluate_spawn_gate: tiles const via F4, world.seed R.
proximity_affinity_boost / check_position / dump_entity_census:
footprints R (+ time R). register_footprint: footprints W, time R.
unregister_footprints_for_patch: footprints R/W. negotiate_position:
world finite R, mood.active R (+ the footprint pair).
update_entity_draw_visibility: entities arrays R/W (draw_visible,
pending), player_.readback R; gpu param-slot uploads.
select/place/commit_entity_queue: the queues R/W + FAMILY_DISPATCH
keyhole passthrough (the rows above). generic_select: mood.active R
(indoor rescale gate). generic_commit: world flag W. find_patch:
patches R (returns mutable ActivePatch*). record_entity: mutates its
own patch only. write_pier/clear_pier: cpuPiers_ W + two world flags
W; gpu upload_pier_slot. recompute/flush_pier_count: piers R, flag
R/W; gpu stage+upload. mark_patches_for_regen: patches R/W (.phase).
evict_patch_entities: patch refs R/W + dispatch passthrough (+ DIAG
readback print). alloc/free_layer: free_layer_count R/W, stack R/W.
Handles: wgpu::Queue& ALWAYS by parameter; no verb pulls a handle
from the keyhole; no encoder; no device.

gpuState_ method union (rows + verbs, 33): the seven family
upload_*_mesh_params_slot / upload_pyramids forms, the six
set_*_index_count, the six *_mesh_gen_group getters,
upload_sphere/cube_entity_slot, upload_ribbon, upload_zone_life,
deactivate_zone_slot, upload_painting_slot, deactivate_painting_slot,
upload_pier_slot, stage_pier_count, upload_pier_count.
renderer_ union (6): dispatch_{pyramid,arch,column,palm,cactus,
blade}_mesh_gen.

---

## 2 — R2: THE DEPS TABLE, FINALIZED (post-campaign)

The same naming key applies module-side: **XDeps members are named as
the organs**, so d2 conversions are parameter-type renames with
byte-identical bodies. Where a module's machine-facing functions (its
dispatch rows) already take MachineCtx after d1, the XDeps carries
only the module's NON-machine reaches. Const-qualification encodes
read-only access; the witness clause (deferred at m5) folds in here:
witness fields arrive as `const PlayerState&` and census Direction W
extends to verify no deps struct exposes them writable outside their
declared doors.

Post-campaign mechanical sweep (organ : reach count, per module —
comments stripped, `c->`/`self->` forms):

| module | reaches (post-m6) | XDeps proposal (non-machine reaches) |
|---|---|---|
| population_themes | zero (one DIAG service call) | **none** — the DIAG census call converts to MachineCtx at d1 (dump_entity_census is a machine verb) |
| family_dispatch | zero (address table) | **none** |
| tile_world | world_state_:6 R (seed/radius), mood_state_:1 R (active), gpuState_:1 | TileWorldDeps { const WorldState& world_state_; const MoodState& mood_state_; GPUState& gpuState_ } |
| spheres | gpuState_:2, time_state_:2 R | SphereDeps { GPUState& gpuState_; const TimeState& time_state_ } |
| pawn | player_:11 RW(aura_presence)/R, time_state_:5 R, gpuState_:5, renderer_:1 | PawnDeps { PlayerState& player_; const TimeState& time_state_; GPUState& gpuState_; Renderer& renderer_ } — the P8 write stays declared |
| orbs | gpuState_:16, renderer_:5, player_:2 R(readback), time_state_:2 R, world_state_:1 R(seed) | OrbsDeps { GPUState& gpuState_; Renderer& renderer_; const PlayerState& player_; const TimeState& time_state_; const WorldState& world_state_ } |
| agents | gpuState_:8, player_:5 RW(possessed — the declared door), transitionPhase_:1 R, world_state_:1 R, time_state_:1 R | AgentsDeps { GPUState& gpuState_; PlayerState& player_; const TransitionPhase& transitionPhase_; const WorldState& world_state_; const TimeState& time_state_ } |
| cube_behaviors | gpuState_:9, time_state_:5 R, agent_state_:4 R(slots), player_:4 R(possessed), mood_state_:1 R | CubeDeps { GPUState& gpuState_; const TimeState& time_state_; const AgentState& agent_state_; const PlayerState& player_; const MoodState& mood_state_ } |
| gol_zones | gol own, gpuState_:10, renderer_:5, world_state_:3 R, time_state_:3 R, mood_state_:1 R, tile faces, **device_ (holdout, §4)** | GolDeps { GPUState& gpuState_; Renderer& renderer_; const WorldState& world_state_; const TimeState& time_state_; const MoodState& mood_state_; const TileWorldState& tile_world_state_; wgpu::Device& device_ (per §4 ruling) } |
| entities | entities own:65, gpuState_:31, world_state_:4 W(flags), mood_state_:2 W(portals_dirty) | EntitiesDeps { GPUState& gpuState_; WorldState& world_state_; MoodState& mood_state_ } — the two flag writes are the standing channels |
| ribbon | ribbon own:23, gpuState_:13, time_state_:9 R, tile faces:4, player_:4 R(readback — post-m6 the sky trio is OWN state), inputState_ R(move), visual_canvas_ + 4 TargetBindings R | RibbonDeps { GPUState& gpuState_; const TimeState& time_state_; const TileWorldState& tile_world_state_; const PlayerState& player_; const InputState& inputState_; const VisualCanvas& visual_canvas_; const TargetBinding& amp_lat, amp_vert, tint_stim, tint_mix } |
| gallery | gpuState_:44, gallery own:17, sunDirection_:9 R, renderer_:8, world_state_:5 R, tile faces:3, clearColor_ R, ribbon_state_ R(rendered_slot), player_ R(readback), mood_state_ R | GalleryDeps { GPUState& gpuState_; Renderer& renderer_; const WorldState& world_state_; const TileWorldState& tile_world_state_; const RibbonState& ribbon_state_; const PlayerState& player_; const MoodState& mood_state_; const float (&sunDirection_)[3]; const float (&clearColor_)[3] } |
| input | inputState_:22/keys_:12/mouse_:4 (OWN organs), player_ RW(fpv), world_state_ RW(radius), device_:? (GetQueue), gpuState_:2, + command pass-throughs (orbs/agents/cubes/pawn states) | InputDeps { InputState& inputState_; KeyState& keys_; MouseState& mouse_; PlayerState& player_; WorldState& world_state_; GPUState& gpuState_; wgpu::Device& device_; + the command fan's target organs (OrbsState&, AgentState&, CubeBehaviorsState&, PawnState&, RibbonState&) } |
| mood | mood_state_:30 RW, world_state_:27 R, gpuState_:13, sun/clear/spot/portal-array/backPortal RW (K4 spine organs), transitionPhase_/pendingDestination_ RW, renderer_:1, + door pass-throughs | MoodDeps { the K4 organ set by name — MoodState&, const WorldState&... full list per census; the spine's transition organs arrive as members, K4's residency unchanged (the type is a face, not a move) } |
| render_passes | gpuState_:148, renderer_:41, all read-only organ views (entities:25, world:12, ribbon:3, patches, gol, gallery, cpuSpotLights_, mood fade, clearColor_) + orbs pass-through | RenderDeps { GPUState& gpuState_; Renderer& renderer_; const <each organ viewed>& ... } — the largest const face; zero writes by census |
| spawn_engine | (machine) | MachineCtx native at d1 |
| entity_pipeline | (machine) | MachineCtx native at d1 |
| patch_system | world_state_:93 RW (co-owner), patches own:92, player_:20 R(readback), gpuState_:19, entities/tile/gallery/ribbon/gol/themes/spawn reaches = machine-face + teardown verbs (already doors) | PatchDeps { WorldState& world_state_; const PlayerState& player_; GPUState& gpuState_; Renderer& renderer_; MoodState& mood_state_; ... } + MachineCtx where it drives the S3 loops — the fattest client converts LAST |

WITNESS EXTENSION (census W, at d2): PlayerState arrives const in
every deps struct except pawn (P8 write), agents (possession door),
and input (fpv toggle) — the three declared doors. Direction W gains
the deps-level check once d2 lands: a non-const PlayerState& outside
those three is a census RED.

---

## 3 — R3: MERGE ELIGIBILITY FORECAST

### 3.1 The mechanics finding — THE COHORT LAW

A merged module is ONE PRE-CLASS HEADER. Its impl half calls foreign
services (find_patch, the preamble, the S2 faces) and constructs
foreign vocabulary — so every callee must be DECLARED EARLIER in the
cohort. Today's cohort orders bodies (cartridge.hpp:56-63) BEFORE
surface/machine headers (:67-71): the .inl zone made that legal; the
merges make it illegal. THE COHORT LAW: **each merge lands with a
topological include-order proof** — contracts → spine/state types →
service-DECL headers (surface + machine hpps) → merged body headers
in dependency order → direction → realization → the demos. glaw1 is
the checking gate per merge; the zone census steps DOWN per merge and
the census tool learns the retirement (the 18==18 count becomes a
shrinking ledger, not a fixed number).

### 3.2 Deps-struct residency

Each XDeps lives in its module's hpp (the requirements face made
literal). Reference members tolerate incomplete types; the root
instantiates every deps struct once, in the member-init list, where
all types are complete. No new includes in module headers.

### 3.3 The predicted merge order (smallest residue first, post-d1/d2)

1. **family_dispatch** — the address table; zero reaches; merges into
   its contract home once row decls precede it (they do).
2. **population_themes** — zero reaches post-d1 (the DIAG call rides
   MachineCtx).
3. **tile_world** — three-organ deps; the S2 faces already const-form.
4. **spheres**, **pawn** — two- and four-member deps.
5. **orbs**, **agents**, **cube_behaviors**, **entities** — mid-size.
6. **gol_zones** — after the §4 ruling (device member or park).
7. **ribbon**, **gallery**, **input** — wide but mechanical.
8. **spawn_engine**, **entity_pipeline** — MachineCtx natives; merge
   once the last keyhole row is gone.
9. **mood**, **render_passes** — the two spine-heavy faces.
10. **patch_system** — the fattest client, LAST.

### 3.4 THE BOM PLAN (byte-verified per merge)

RULE: every merged file takes **clean UTF-8, no BOM, LF** — the hpp
side's encoding in every pair (mechanical sweep: all 18 hpps clean;
four .inls carry BOMs: agents.inl, ribbon.inl, entity_pipeline.inl,
render_passes.inl). The BOMs DIE WITH THE .INLs: post-d3 the BOM
census reads **renderer.hpp alone** (deliberate, recorded). Each
merge commit names the module's single file and its verified
encoding; a merged file also absorbs the union of the pair's own
system includes (the .inls carry their own <cmath>/<iostream> per
fix-2 — those lines ride into the merged header).

### 3.5 The halving claim (d4 tests it)

18 pairs (36 files) + cartridge.hpp → 18 merged files + cartridge.hpp
if every module merges; the named remainder (if any) is the d4 close-
out's honest count. LOC today (pairs): ~14.6k across .inls + hpps.

---

## 4 — R4: THE HOLDOUTS, NAMED

### 4.1 The gol device_ self-submit — DISSOLVES BY DECLARATION (stamp item)

flush_zone_derive_requests (gol_zones.inl:291-310) uploads the
request buffer, then encodes the derive pass on ITS OWN encoder and
**submits immediately** (queue.Submit mid-render). Submission-order
analysis, first-hand: the frame encoder (carrying the earlier mesh-gen
pass and dispatch_compute) is submitted by the HOST at end-of-render —
so today the derive pass EXECUTES BEFORE this frame's agent kernels,
and a freshly spawned zone's derived params are visible to the same
frame's ground contributors. Folding the derive pass into the frame
encoder would execute it AFTER them — a one-frame skew on every
zone-spawn frame. **The refactor stays forbidden.** But the hazard was
the refactor, never the handover: giving **GolDeps** (not MachineCtx —
the flush is score-called, and no row or verb reaches device_ by
census) a DECLARED `wgpu::Device& device_` member, consumed by
exactly this one verb and seam-tagged SEAM[gol:derive-submit] (the
immediate-submit protocol named), preserves the byte-exact behavior
while dissolving the keyhole reach. PROPOSED: declare on GolDeps,
don't refactor. Jean stamps or parks.

### 4.2 Rows whose MachineCtx form would change behavior

**NONE.** The census confirms every row body converts by parameter-
type rename alone (the organ-naming key), and every machine verb
takes its Queue by parameter already. The one signature-SHAPE change
beyond the rename is the DUAL-ENTRY RULE (§1.2): four doors
template-ize on the deduced context — bodies byte-identical, reaches
compile-checked at instantiation, the stamped pattern law generalized
rather than a new mechanism.

### 4.3 Standing precedents unaffected

The four deps-form first citizens (clear_spheres, clear_cubes,
ribbon_advance_head, ribbon_rebuild_body_upload) already carry
explicit GPUState& — they are ALREADY in the destination form and do
not convert; the m3 ruling's banners stand.

---

## 5 — THE STAMP REQUEST

- S1: MachineCtx composition (§1.2) + its home (contracts/
  entity_types.hpp) + the organ-naming key (byte-identical bodies) +
  THE DUAL-ENTRY RULE (four doors template-ize on the deduced
  context: commit_ribbon, load_authored_textures, pick_portal_mood,
  derive_finite_radius).
- S2: the deps table (§2) with const-encoding + the Direction W
  deps extension.
- S3: the merge order (§3.3), THE COHORT LAW (§3.1), and the BOM
  plan (§3.4 — BOMs die with the .inls; renderer.hpp remains the
  lone deliberate BOM).
- S4: the gol ruling (§4.1): declared device member vs park.
- S5: d0 is in this commit — v3 §7 carries the second puller
  verbatim; confirm the sentence as landed.

FULL STOP. d1 cuts only after the stamp.
