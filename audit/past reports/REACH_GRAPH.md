> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# DEP-1 — THE REACH GRAPH + KIND MAP (read-only)

the_board, branch `FINAL_TOUCH`, anchored at HEAD `84bc96a` (post-RAD-2;
line numbers below reflect that tree). **No code changed.** Method: one
edge-extraction pass per analysis unit (17 `.inl` + the cartridge.hpp
body), an independent deep-dive on the entity_pipeline seam (Q2), then a
completeness-critic cross-check over the aggregate (it recovered one
missed B-edge, folded in below); pivotal edges spot-re-verified against
the tree by grep before writing. A plain include graph is useless here —
the cartridge is one translation unit — so this is the REACH graph: who
actually touches whom.

Normalizations applied (recorded, so the matrix reads consistently):
- Calls into `seed_utils.inl` helpers are **C-CALL** edges to a
  VOCABULARY-kind module (one first-pass reader tagged them D; normalized
  to C — the *edge* is a call even when the *target's kind* is vocab).
- `state.hpp` / `Dim::` symbols are folded into `cartridge.hpp(body)` per
  the roster (the body + its service/state layer count as one unit).
- `InputState` is *declared* in cartridge.hpp but *authored* by input.inl;
  reads of `inputState_` are edged to **input.inl** (the behavioral owner).

## §1 — THE ADJACENCY MATRIX

Kinds: **A** state-read · **B** state-write · **C** call · **D** vocabulary
· **E** keyhole(service). `·` = no edge. Columns abbreviated:
ety=entity_types, sed=seed_utils, flv=floater_vocabulary, ent=entities,
pwn=pawn, rib=ribbon, orb=orbs, agt=agents, cub=cube_behaviors,
spn=spawn_engine, pip=entity_pipeline, moo=mood, gol=gol_zones,
gal=gallery, rps=render_passes, inp=input, CRT=cartridge.hpp(body).
(ground_architecture emits and receives nothing — omitted as a column.)

| src \ tgt | ety | sed | flv | ent | pwn | rib | orb | agt | cub | spn | pip | moo | gol | gal | rps | inp | CRT |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| entity_types | | | | | | | | | | | | | | | | | D |
| seed_utils | | | | | | | | | | | | | | | | | |
| floater_voc | | | | | | | | | | | | | | | | | D |
| entities | | | | | | | | | | | | | | | | | D,E |
| ground_arch | | | | | | | | | | | | | | | | | |
| pawn | | | | | | | | | | | | | | | | | E |
| ribbon | | C | | | A,**B** | | | | | C | | | | | | A | D,E |
| orbs | | | | | A | | | | | | | | | | | | A,D,E |
| agents | | C | | D | A,**B** | | | | | | | | | | | | A,D,E |
| cube_behav | | C | A,**B**,D | | A | | | A | | | | | | | | | D,E |
| spawn_engine | D | C | | A,**B** | A | | | | | | C | A | | | | | A,**B**,C,D |
| entity_pipe | D | C | A,**B**,D | A,**B**,D | | | | | | C,D | | A,**B** | | | | | **B**,C,D,E |
| mood | | C | | A,**B**,D | **B** | A,**B**,C,D | C,D | | | C,D | D | | **B** | C | C | | A,**B**,C |
| gol_zones | | C | | | | | | | | C,D | | A | | | | | A,E |
| gallery | | C | | | A | A | | | | C,D | | A | | | | | A,E |
| render_passes | | | | A | | A | C | | | | | A | A | A | | | A,C |
| input | | | | | A,**B** | | C | C | C | | | C | | | | | **B**,C |
| cartridge(body) | | C | A,**B**,D | A,**B**,C,D | A,**B**,C | A,**B**,C,D | C,D | A,**B**,C,D | C | **B**,C,D | C | A,**B**,C | A,**B**,C,D | A,**B**,C,D | C | C | — |

### The B-STATE-WRITE catalog (every cross-module write, verified)

These are the significant edges — each is a module mutating state it does
not own:

| # | writer → owner | what | site |
|---|---|---|---|
| 1 | ribbon → pawn | `player_.sky_yaw_eased` (+ reset), `sky_mode_prev` — the shared steering hand | ribbon.inl:978,981,959 |
| 2 | agents → pawn | `player_.possessed_slot = new_slot` — possession transfer | agents.inl:801 |
| 3 | mood → pawn | `pawn_state_.aura_enabled = false` when mood forbids aura | mood.inl:669 |
| 4 | mood → gol_zones | `gol_state_.mood_allowed = m.allow_gol_zones` | mood.inl:668 |
| 5 | mood → ribbon | `ribbon_state_.rendered_slot = 0` after anchor commit | mood.inl:650 |
| 6 | mood → entities | `force_spawn_portal_at` hand-writes ~30 arch fields + `arch_count`/`arch_mesh_gen_pending` | mood.inl:939-993 |
| 7 | mood → spine | `pendingDestination_` + `transitionPhase_ = FADE_OUT` — authors the spine's transition FSM *(critic-recovered edge)* | mood.inl:1323-1324 |
| 8 | entity_pipeline → entities | writes all 7 grounded families' instance arrays, counts, pending flags | entity_pipeline.inl:450+ |
| 9 | entity_pipeline → floater_voc | `activeFloaters_`/`activeCubes_` + counts (spawn + rollback) | entity_pipeline.inl:1720,1897 |
| 10 | entity_pipeline → mood | `mood_state_.portals_dirty = true` after portal-arch commit | entity_pipeline.inl:2170 |
| 11 | entity_pipeline → spine | `world_state_.ground_entries_dirty = true` in generic_commit | entity_pipeline.inl:320 |
| 12 | spawn_engine → entities | cull loop toggles `draw_visible` + `*_mesh_gen_pending` | spawn_engine.inl:422+ |
| 13 | spawn_engine → spine | `patches_[p].phase = NEEDS_REGEN`, pier dirty flags, `cpuPiers_` mirror | spawn_engine.inl:879,274+ |
| 14 | cube_behaviors → floater_voc | `activeCubes_[i].cx/.cz` corral/kite position mirror | cube_behaviors.inl:437,514 |
| 15 | input → pawn | inline pokes: `aura_enabled`, `aura_height_enabled`, `aura_cfg_dirty`, `player_.fpv_mode`, `sky_mode` | input.inl:211-218,323,334 |
| 16 | input → spine | `world_state_.active_radius` + forces `last_center = INT32_MAX` (recenter trick) | input.inl:343-349 |
| 17 | spine → (everything) | readback → `player_`/`agent_state_.slots`; teardown resets every module's state; evict wrappers flip peers' `.active` | cartridge.hpp:3120-3125, 2161+, 1034+ |

## §2 — THE KIND MAP

First-pass hypothesis vs. what the code actually is:

| Module | Measured kind | Hypothesis | Verdict |
|---|---|---|---|
| entity_types | **VOCABULARY** (pure; 0 behavior — the adapter *contract* declares `Cartridge*`/`wgpu::Queue&` in signatures but dereferences nothing) | VOCABULARY | confirmed |
| seed_utils | **VOCABULARY** (zero outbound edges; pure static math) | VOCABULARY | confirmed — the perfect sink |
| floater_vocabulary | **VOCABULARY — compromised**: pure tables *plus* live mutable state (`activeFloaters_/activeCubes_` + counts) that three other modules write (#9, #14, #17) | VOCABULARY | **corrected**: vocab file owning live entity state; the single worst mislabel in the tree |
| entities | **VOCABULARY + thin GPU-REALIZE tail** (7 families' tables/types/EntitiesState + 6 `prepare_*_mesh_gen` preparers reaching `c->gpuState_`) | SCENERY | **corrected**: it is the grounded-entity *vocabulary+state declaration*, not scenery — nothing in it behaves |
| pawn | **AGENT + GPU** — post-B1 it is remarkably clean: 3 outbound edges, all E-service (clock, 2 wires) | AGENT | confirmed |
| ribbon | **AGENT + MANAGEMENT** (head law, flight, spawn recipe) | AGENT | confirmed |
| orbs | **SCENERY + GPU-REALIZE** — decorative sky dome; fully encapsulated state; only relational touch is the *optional* pawn-anchored dome + one log read | AGENT | **corrected**: it doesn't respond to bodies; it decorates. The actual scenery module |
| agents | **MANAGEMENT + AGENT** — population lifecycle, spawn/respawn/possession; the *behavior* runs GPU-side | AGENT | **corrected** (split noted: possession scan is genuinely agent-ish) |
| cube_behaviors | **MANAGEMENT + AGENT** (behavior assignment tables + pawn-aware corral/kite diagnostics) | AGENT | **corrected** (same split) |
| spawn_engine | **MANAGEMENT + GPU** (gates, placement laws, footprint registry, cull) | MANAGEMENT | confirmed |
| entity_pipeline | **MANAGEMENT + per-family geometry/skin** — the split IS the conversion seam (§3-Q2) | MANAGEMENT | confirmed |
| mood | **MANAGEMENT + GPU** — and the highest-degree module bar the spine (19 edges, B-writes into 6 peers + the spine FSM) | MANAGEMENT | confirmed |
| ground_architecture | **VOCABULARY** (pure constexpr DAG/POLICIES tables + static_asserts; zero reaches either way) | MANAGEMENT | **corrected**: it is documentation-as-tables, not orchestration |
| gol_zones | **MANAGEMENT + GPU** | GPU-REALIZE | corrected (mild): lattice lifecycle + placement first, compute wires second |
| gallery | **MANAGEMENT + GPU** (photographer, exhibitions, snapshot pass) | (unlisted) | tagged |
| render_passes | **GPU-REALIZE + MANAGEMENT** (draw-order orchestration + per-frame CPU packing that reads seven peers) | GPU-REALIZE | confirmed |
| input | **MANAGEMENT** (command router; near-zero own logic) | (unlisted) | tagged |
| cartridge(body) | **MANAGEMENT + VOCABULARY + GPU service layer** — spine, world-gen tables, clock/wires, 39 outbound edges | MANAGEMENT | confirmed |

## §3 — THE THREE QUESTIONS

### Q1 — LATENT AWARENESS (the relationships an abstraction would formalize)

What the code already does, grouped by shape — enumerated from evidence,
not defined in advance:

**1. Pawn-proximity (the dominant pattern — six independent copies):**
- spawn_engine entity cull: dist(entity, `player_.readback_x/z`) vs the
  visible edge (spawn_engine.inl:408-410 — the RAD-2 invariant).
- Patch streaming: the whole allocate/evict/generate pipeline is centered
  and budget-sorted on the pawn (cartridge.hpp:3505-3506, 3617, 3790).
- Gallery photographer: capture cadence = f(pawn walk distance), each
  photo stamped with pawn capture pos (gallery.inl:626-639, 706-707).
- Ribbon: rendered slot = nearest active ribbon to pawn (1038-1039);
  spawning body orients *away* from the pawn (1240).
- Orbs: optional dome-anchor tracks pawn XZ (orbs.inl:908; fed by
  cartridge.hpp:3060).
- GPU-side: floater eviction at 400 wu from pawn (world.wgsl:6316+);
  pawn-aura compute gated on `player_.aura_presence` (cartridge.hpp:3355).

**2. Mount/possession coupling (the strongest, bidirectional cluster):**
- ribbon ⇄ pawn: head computes the saddle pose the pawn rides; writes the
  shared steering hand `player_.sky_yaw_eased` from live `inputState_`
  (ribbon.inl:833, 971-978); spine re-syncs mount to head pose each frame
  (cartridge.hpp:3288-3290) and pins the flown ribbon against eviction
  while `player_.sky_mode` (1377).
- agents → pawn: possession scans every peer slot for the nearest body
  within POSSESSION_RADIUS of the player (peer-array proximity,
  agents.inl:766) and writes `player_.possessed_slot` (801); gated on the
  spine's `transitionPhase_ == IDLE` (749).
- cube_behaviors → pawn-via-agents: corral/kite anchors on
  `agent_state_.slots[possessed_slot].pos_x/z` (376-377, 481-482) — note
  it resolves the pawn through the *agents* array, not the published
  `player_.readback_*`.

**3. Peer-entity spatial awareness — ALREADY REIFIED as a service:** the
spawn_engine **footprint registry** is an existing "awareness" mechanism:
`check_position` rejects placements against every registered peer
footprint (534-547), `proximity_affinity_boost` *attracts* kinds to
related kinds (847-857), and gol_zones/gallery/ribbon all
publish+query through it (gol_zones.inl:466-472; gallery.inl:884-890).
Any "placeable/awareness" abstraction should be shaped from this — it is
the one place peer-spatial awareness already has an API instead of a
field poke.

**4. Mood gates (world-state awareness):** every spawner reads
`mood_state_.active` to scale/suppress (spawn_engine.inl:120,
gol_zones.inl:320, gallery.inl:765, entity_pipeline.inl:254 indoor
rescale, 1954 cube behavior pick); mood writes gates back into peers
(#3, #4 in the B-catalog).

**5. Presence gates (renderer reading peers):** render_passes gates whole
subsystem draws on peers' live state — `ribbon_state_.rendered_slot`
(164), `gol_state_.zone_count` (336/500), gallery counts (595/616),
`mood_state_.spot_light_active` topology switch (259). Gallery's snapshot
pass does the same peer read (gallery.inl:1260).

### Q2 — THE PIPELINE SEAM (confirmed, with a relocatability gradient)

**Confirmed at the structural level.** The generic machine is real:
`generic_select` (193), `generic_place` (273), `generic_commit` (304) name
no family and run entirely off the `EntityFamilyTraits` data table + the
9-pointer `EntityFamilyAdapter` vtable (both in entity_types.inl), plus
two shared helpers (`generic_compute_colors` 94, `rescale_to_rolled_target`
166). All nine family blocks follow one 10-element template.

**Complicated by three frictions:**
1. *The machine is not hermetic*: generic_select reads
   `mood_state_/MOOD_TABLE` (254); generic_commit writes
   `world_state_.ground_entries_dirty` (320).
2. *Recipes are bidirectionally bound*: invoked via the vtable AND calling
   back into machine helpers (4/9 use the rescale helper, 2/9 the color
   default) — relocation converts intra-file cohesion into cross-file
   back-references.
3. *"Pure geometry" holds for only 3 of 9 families.* The gradient:

| Family | Relocation | Tether |
|---|---|---|
| Cactus | **CLEAN** — the textbook recipe | universal tethers only |
| Blade | **CLEAN(ish)** | no piers, no post_commit |
| Palm | **CLEAN** + 1 machine helper | rescale_to_rolled_target |
| Column | MESSY | creates a pier (post_commit 1212); welded to Antenna |
| Antenna | **MESSIEST-to-separate** | shares ColumnTierRow/ColIdx/GPU buffer/pending flag with Column — inseparable siblings |
| Pyramid | MESSY | heightfield bake: `mark_patches_for_regen` (1590), `cpu_pyramids` mirror |
| Sphere | MESSY-moderate | state lives in floater_vocabulary, clock stamp (1724), decoupled GPU lifecycle |
| Cube | MESSY | calls cube_behaviors (1942-1954), reads mood, clock |
| Arch | **MESSIEST** | portal/world system: writes `mood_state_.portals_dirty` (2170), `pick_portal_mood`/`derive_finite_radius`, two piers, heightfield regen |

Conversion consequence: peel the three vegetation recipes first; treat
Column+Antenna as one unit; leave Arch with the machine until the portal
subsystem has its own seam.

### Q3 — KIND VIOLATIONS (the conversion's known friction)

Ranked by how hard each will fight a header conversion:

1. **floater_vocabulary is not a pure sink** — a VOCABULARY-rostered file
   owns live per-entity state that entity_pipeline (spawn/rollback),
   cube_behaviors (corral/kite mirror), and the spine (evict/teardown/
   readback-sync) all mutate. Any conversion must first split the
   `ActiveFloater/ActiveCube` arrays out of the vocabulary (into a
   floater-state owner) or accept a vocab+state hybrid.
2. **Direct peer-internals WRITES without a service** (the B-catalog):
   the worst offenders are mood (#3-#7 — five distinct peers plus the
   spine FSM; `force_spawn_portal_at` authoring ~30 fields of another
   module's instance is peer-surgery), input's inline pawn/world pokes
   (#15, #16 — inconsistent with its own clean delegation pattern
   elsewhere), and spawn_engine's cull writing `entities_state_` flags
   (#12).
3. **Peer-internals READS that bypass published surfaces**: gallery and
   render_passes read `ribbon_state_.rendered_slot` raw (gallery.inl:1260,
   render_passes.inl:164); cube_behaviors reads the pawn through
   `agent_state_.slots[...]` instead of the published
   `player_.readback_*`; render_passes hand-packs GPU buffers from
   intimate knowledge of every entity family's field layout
   (upload_ground_entries, 49-98) plus spawn_engine's `cpuPiers_` mirror.
4. **entities.inl's GPU tail** — a vocabulary file with six
   `c->gpuState_.set_*_index_count` reaches (681-755). Small, but it
   blocks calling entities.inl a pure header.
5. **State-ownership inversions**: `MoodState` and
   `PlayerState.readback_*` are rostered to mood/pawn but *declared* in
   cartridge.hpp and *authored* by spine machines (transition FSM,
   readback callback) — the struct's home, its declaration site, and its
   author are three different places.
6. Minor: agents colors bodies from entities' `COLUMN_PALETTE`
   (agents.inl:596) — cross-family vocab borrow; entity_types declares
   `Cartridge*`/`wgpu::Queue&` in its adapter signatures (benign but the
   contract names the keyhole); ActiveArch embeds portal/`PortalDestination`
   fields (entities.inl:177-180), tying arch vocabulary to the
   world-transition subsystem; input.inl:56 doc misattributes
   `cycle_floater_coordination` to floater_vocabulary (it lives in
   cube_behaviors.inl:348) — comment drift, flagged not fixed.

## §4 — DEPENDENCY DEPTH (the conversion order)

Layered by outbound reach (convert lowest first):

- **L0 — pure leaves (zero or D-only outbound):** `seed_utils` (zero),
  `ground_architecture` (zero), `entity_types` (D: a `Cartridge*`
  forward-decl in signatures), `floater_vocabulary` (D-only outbound —
  but see the *inbound* state problem, Q3-1, which must be resolved
  first or carried knowingly).
- **L1 — vocab+state with a thin wire tail:** `entities` (D + one
  E-service tail).
- **L2 — self-contained agents/scenery:** `pawn` (3 edges, all
  E-service — the B1/B3 campaign already made it a leaf-adjacent module),
  `orbs` (services + 1 pawn read).
- **L3 — single-domain managers:** `gol_zones`, `gallery`,
  `cube_behaviors`, `agents`, `ribbon` (ribbon carries the pawn-mount
  B-write — needs a mount-service decision or an accepted keyhole).
- **L4 — cross-domain machines:** `spawn_engine` + `entity_pipeline`
  **as a pair** (see cycle below), `render_passes` (reads seven peers),
  `input` (writes/calls six).
- **L5 — hubs, last:** `mood` (19 edges, writes 6 peers + the spine FSM),
  then `cartridge.hpp(body)` (39 edges — the spine converts by shedding,
  not by moving).

**Cycles (mutual reaches, excluding the sanctioned spine⇄everything hub):**

1. **spawn_engine ⇄ entity_pipeline** — the load-bearing one.
   spawn_engine drives the select/place/commit loops through
   `FAMILY_DISPATCH` (spawn_engine.inl:1075-1110 → pipeline's generics);
   the pipeline calls back into spawn_engine's placement laws
   (`negotiate_position`, `run_spawn_preamble`, `record_placement_bookkeeping`,
   `write_pier`) and uses its DTO types (`PopFamily`, queue/placement
   entries). The cycle is *by design* through a vtable — converting to
   headers means extracting the dispatch contract (the FAMILY_DISPATCH
   table type + `EntityQueueEntry`/`PlacementEntry`/`PopFamily`) into a
   shared interface header both include. These two convert together or
   not at all.
2. **mood ⇄ {spawn_engine, gol_zones, gallery, render_passes,
   entity_pipeline}** — a hub-and-backedge pattern rather than five true
   pairwise knots: mood writes gates/calls forward; peers read
   `mood_state_.active` back. One extraction breaks all five backedges: a
   read-only *mood-id/profile query* (the current mood + its MOOD_TABLE
   row) as a service. The forward writes (#3-#7) then become the
   remaining, explicit coupling.
3. **ribbon ⇄ pawn (via spine)** — ribbon writes the steering hand,
   the spine re-syncs the mount each frame, pawn state gates ribbon
   eviction. Not a direct 2-cycle in the matrix but a real runtime loop;
   the mount is the seam to name (it is also the constitution's known
   coupling).

**Sequence, stated once:** L0 → L1 → L2 → L3 → (extract dispatch
interface) → spawn_engine+entity_pipeline → render_passes/input →
(extract mood query) → mood → spine. The awareness/surface/placeable
interfaces should be shaped from §3-Q1's measured relationships — the
footprint registry (already an API), the pawn-proximity pattern (six
copies of one idea), and the mount coupling (one bidirectional seam) —
rather than defined ahead of the evidence.
