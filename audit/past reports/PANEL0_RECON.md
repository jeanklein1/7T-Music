> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# PANEL-0 — THE AUTHORING-SURFACE RECON (report-first; ONE STOP)

Read-only campaign product. Nothing moved, nothing gated, nothing merged.
The recon sizes the campaign that makes demos EASY TO AUTHOR at development
time — a pieces × demos MATRIX (existence) paired with per-module PARAMETER
PANELS (tuning), both compile-time, both edited by Jean before any request
reaches Claude. Jean's stamped payoff path: matrix-shape + panels one at a
time → the TERRAIN revised first, in isolation. This report returns R1 (the
perf locus), the pawn fusion map, the proposed matrix shape, the terrain
panel map, and the three sized movements. FULL STOP for the stamp after R5;
p1 cuts only after.

METHOD. Two parallel readers censused (a) the slow-shader locus — the full
world.wgsl call-tree arithmetic, entry-point by entry-point (R1); and (b)
per-module constant readiness across all 16 non-terrain-detail modules (R4
grades). The spine's own reading covered the pawn fusion (PlayerState +
agents slot-0 + pawn aura + the anchor coupling, R2), the existence surface
(roster.hpp / demo_config.hpp / the two demo sentences, R3), and the terrain
tuning surface in full (the four homes, R4). Every load-bearing claim carries
a file:line citation. v3 §§9/11/13 (the driver law, the witness, the
direction layer) and §7 (the covenant) are the standing lens.

---

## R1 — THE SLOW-SHADER LOCUS (the honest perf question, settled first)

**The shader.** `realization/world.wgsl` — 11,468 lines, one monolithic WGSL
module, loaded from disk as raw source and recompiled fresh EVERY boot
(`renderer.hpp:1264-1327` `loadShader()` → `CreateShaderModule` at
`renderer.hpp:1320-1325`, timed on the spot). ~31 compute pipelines +
render pipelines, each re-running FXC at boot. **There is no pipeline or
shader blob cache anywhere in the tree** — every launch pays full
compilation.

**The kernel is already split** — and the split was already a perf cut.
The agent update runs as two entry points (`renderer.hpp:25-26`):
`update_player_agent` (`world.wgsl:6217-6236`, `@workgroup_size(1)`, runs
only on `config.possessed_slot` — the pawn) and `update_other_agents`
(`world.wgsl:6242-6283`, `@workgroup_size(32)`, switching over the 9
autonomous behaviors). The header note (`world.wgsl:6144-6161`) records
that the original UNIFIED kernel inlined the heavy player path for all 32
threads and "produced a pipeline compile that landed at 48s." Splitting the
pawn's kernel out of the 32-thread kernel already dropped it — the current
~34s is what remains in the player kernel alone.

**The hot locus — "7 chains × 24 lattice unrolls."** Inside
`behavior_player_controlled` (`world.wgsl:5574-5700`):
- `pawn_ground_resolve` (`world.wgsl:5438-5477`) calls the walker
  contributor chain **4×** — `new_xz`, `prev_xz`, `slide_x`, `slide_z`
  (:5443/:5444/:5459/:5463): the step-climb candidate positions.
- `terrain_normal_at` (`world.wgsl:5406-5414`) calls it **3×** — center +
  two finite-difference taps (:5408-5410): the draw-orientation tilt.
- 4 + 3 = **7 full chains**. Each chain → `terrain_height_at`
  (`world.wgsl:576`) loops `TERRAIN_BAND_COUNT = 6u` bands (:581), and each
  band unrolls a 2×2 lattice-node blend (`world.wgsl:551-560`) → **24
  `evaluate_lattice_wave` unrolls per chain**. Every bound is a compile-time
  constant, so FXC fully unrolls: **7 × 24 = 168 inlined lattice evals**,
  plus each chain's pyramid / gol-zone / wave / pulse / aura contributors.
  `update_other_agents` snaps each agent with a SINGLE chain (1 × 24), not
  7 — the multiplier is unique to the pawn kernel.

**Cost attribution — the BODY carries it, not the driver, not the gait.**
Within `behavior_player_controlled`:
- **PLAYER / driver (cheap):** input-intent → velocity, camera-azimuth
  heading, world-boundary clamp, portal detection (`world.wgsl:5613-5642,
  5684-5698`) — plain arithmetic, no chain.
- **BODY (the whole 7× cost):** `pawn_ground_resolve` step-climb (4 chains,
  :5650-5659) + `terrain_normal_at` tilt (3 chains, :5662-5682) — placement
  / terrain-snap / draw-orientation. **All 7 chains originate here.**
- **BEHAVIOR / gait (cheap, and elsewhere):** the autonomous walk-chains
  (`world.wgsl:5718-6129`) live in `update_other_agents`, each a single
  ground snap. The pawn's own locomotion is input-driven, not autonomous.

**Two conclusions, kept separate (per the order's demand):**

1. **Does the pawn decomposition touch boot time?** ONLY if it extracts or
   bakes the BODY's terrain-resolve chain. A pure CPU-side rename of
   player/body/behavior ownership that leaves the same WGSL entry point
   compiling the 7×24 chain does **not** touch boot at all. The runtime
   gates that guard the heavy path (`coupling_active` reads
   `config.mute_couplings`, `world.wgsl:1764-1766, 5650, 5662`) are RUNTIME,
   so FXC cannot dead-code-eliminate them — both branches always compile.
   The lever that WOULD cut boot is the same one already proven: a
   pipeline-boundary extraction of the BODY chain (as `update_player_agent`
   was extracted from the 32-thread kernel), or a **lattice bake** — a
   baked-heightfield LUT that collapses the 24-unroll (the shader already
   has a partial baked path, `world.wgsl:3653` "skips 3 lattice noise
   chains", :7163). Both are R7's named levers — distinct movements, not p1.

2. **Is the blob cache the answer regardless?** YES, and it is INDEPENDENT
   of the decomposition. Because the ~34s is one-time COMPILATION,
   serializing the compiled pipelines to disk and reloading them eliminates
   the recompile on every warm boot no matter how the pawn is organized.
   Caveat: a blob cache only helps cache HITS — the first boot and any
   post-shader-edit boot still pay full FXC, whereas chain-multiplier /
   lattice-bake reduce the compile cost itself. (Audit ledger names all
   three as parked peers: `LADDER.md:1387-1388`.)

**R1 verdict for the stamp.** The pawn decomposition's value is
COMPREHENSION (v3 §9), not perf; its perf payoff is zero unless paired with
a BODY-chain extraction or lattice-bake — a separate movement. The pipeline
blob cache is the orthogonal warm-boot lever and exists nowhere today. Do
not let p1 carry a perf promise it cannot keep.

---

## R2 — THE PAWN FUSION MAP (the decomposition's real surface)

**The finding: "the pawn" is already three things wearing one name.** The
concept is CONFLATED in vocabulary but only lightly fused in structure — the
clean lines already very nearly exist.

**(a) The PLAYER — the driver + the anchor (v3 §9 Act I & III, §11).**
Lives in `PlayerState` (`contracts/spine_state.hpp:66`): `possessed_slot`
(the ANCHOR — a pointer at a body, default slot 0), `fpv_mode` (the camera),
`readback_x/z` + `readback_portal_trigger` (the WITNESS harvest),
`aura_presence` (the aura ramp, P8). The intent is authored in
`direction/input.hpp` (the driver's face — already dissolved to InputDeps in
DISSOLVE-1 Batch C). The camera couples to the anchor GPU-side:
`config.possessed_slot` selects `agent_state[possessed_slot]` for the camera
position (`state.hpp:2378, 3976-3980`, `world.wgsl compute_pawn_pos`).
**Possession is re-anchoring** — `try_possess_nearest` (`agents.hpp:284`)
moves `possessed_slot` to another slot; the body it leaves keeps living.
STATUS: the player is ALREADY a distinct organ — it is a pointer at a body,
not fused into one.

**(b) The BODY — the drawn, terrain-snapped thing.** The pawn's body IS
agent slot 0: `PLAYER_SLOT = 0` (`agents.hpp:105`), a `GPUAgentState` in
`agent_state_.slots[0]`, seeded by `seed_player_body` (`agents.hpp:289`,
called UNCONDITIONALLY at boot — `cartridge.hpp:476-477`) and reseeded
across worlds (`reseed_player_body`, `cartridge.hpp:719`). There is NO
separate "pawn body" module — the body is one slot of the shared agent
array, drawn and terrain-snapped by the same machinery as every agent. This
is where R1's 7×24 cost lives (the BODY chain in the WGSL).

**(c) The BEHAVIOR / gait — the walk translation.** `AGENT_BEHAVIOR_PLAYER_
CONTROLLED = 0` (`agents.hpp:48`) — slot 0's gait, dispatched by the WGSL
kernel's behavior switch to `behavior_player_controlled` (input-driven
locomotion) rather than an autonomous gait. The other 9 behaviors
(random_walk … levy_flight, `agents.hpp:49-57`) are the NPC gaits. Per v3
§13: the body owns HOW (the gait/translation); direction owns WHEN and WHOM.

**The separation lines (what moves, and its cost):**

| Concept | Home today | Clean home | Cost to separate |
|---|---|---|---|
| PLAYER (driver+anchor) | `PlayerState` + `input.hpp` | already distinct — a witness/anchor organ | ~NIL. Rename/relabel; possibly graduate the anchor fields out of the readback trio's record for legibility. |
| BODY (slot 0) | `agents.hpp` slot 0 (foundational) | stays in agents as "the anchored body" — a named slot, not a new module | LOW (C++). The body is shared-array machinery; naming slot 0 as the body costs nothing structural. Perf is R1's BODY-chain, a SEPARATE lever. |
| BEHAVIOR (gait 0) | `agents.hpp` behavior 0 + WGSL `behavior_player_controlled` | the gait table row + its WGSL kernel | LOW (C++ row is already named); the WGSL kernel is already its own entry point. |

**THE KNOWN CONSTRAINT — the pawn is FOUNDATIONAL.** `seed_player_body` is
UNCONDITIONAL (`cartridge.hpp:476`, "the player body is unconditional");
only `wanderers` (agent slots 1+) is roster-gated. There is ALWAYS a
playable body + a camera + a gait. So the player/body/behavior rows the
decomposition exposes are **always-on** — they cannot be "ticked off" like a
family. Their value in the matrix (R3) is comprehension (three named
concepts), not three new existence toggles. The arrow law requires no new
teeth: the anchor fields are already the witness contract (census Direction
W, sole-author-guarded); the split relabels, it does not re-author.

**R2 verdict.** The decomposition is a clean, low-cost COMPREHENSION cut —
the three concepts are already 80% structurally separate (an organ, a slot,
a gait row). No weld resists. The one thing it does NOT buy on its own is
boot time (R1). This is the movement that names the matrix's player-side
rows — foundational rows, all green.

---

## R3 — THE MATRIX CENSUS (the existence surface)

**Today's surface.** `struct Roster` (`roster.hpp:59`) = 12 family bits +
7 feature bits = **19 booleans**. `struct DemoConfig` (`demo_config.hpp`) =
`{ Roster roster; uint32_t seed; uint32_t boot_mood; }` — the D1 manifest +
D2 world params. A demo is a hand-written header: `demos/full.hpp` (all 19
true, seed 42, MOOD_OPEN_DEFAULT — the golden twin, byte-identical to head)
and `demos/minimal.hpp` (all 19 false). Selection is COMPILE-TIME:
`-DINCUBATE_DEMO=<basename>` (default full); the selected `DEMO.roster`
becomes the namespace-scope `inline constexpr Roster ROSTER`
(`demo.hpp:33`), folded at every gate.

**The dependency edges — there is exactly ONE.** THE FIRST EDGE:
`static_assert(!ROSTER.transitions || ROSTER.portal, …)` (`demo.hpp:39`) —
transitions REQUIRE portal (the trigger in, the return out). No other roster
edge exists. (The `ground_architecture.hpp` DAG edges are a DIFFERENT graph
— terrain contributor policies, not roster.) The other known "edge" is the
FOUNDATIONAL floor (R2): the pawn/agent-machinery is unconditional — a
constraint on which ROWS can be false, not a cross-row edge.

**The proposed MATRIX SHAPE (a proposal for the stamp).**
The 19-bit vector, today written per-demo as a flat initializer list,
becomes a legible **pieces (rows) × demos (columns)** grid. The two existing
sentences (full, minimal) are the first two columns. Concretely:
- Keep `DemoConfig` and `Roster` EXACTLY as the compile-time boolean carriers
  (the pipeline gate depends on `constexpr` folding — non-negotiable; RIDER
  A). The matrix is a PRESENTATION over the same bits, not a new runtime.
- The grid is a table an author reads down (every piece) and across (every
  demo) — e.g. a single annotated header where each row is a piece and each
  column a demo's cell, still resolving to the per-demo `Roster` literal the
  compiler folds. No boot-time registry (that stays parked — the maturity
  dial, `demo_config.hpp`: compile-time → boot-time table → panel).
- THE FIRST EDGE generalizes to a per-column validation: each demo column
  re-asserts `!transitions || portal` (already true; the grid makes the edge
  visible beside the cells it constrains). The FOUNDATIONAL floor becomes a
  documented "always-green" band (surface, sun, the player/body/gait rows
  from R2).
- YAGNI, per the order and the demo contract: scope to what TWO demos + a
  decomposed pawn need. The grid grows BY PULL (a new column when a new demo
  is authored; a new row when a piece is decomposed). Not a maximal tool.

**The honest re-rank (disclosed).** The order frames p1 as giving the matrix
its "PLAYER-SIDE ROWS." True — but R2 shows those rows are FOUNDATIONAL
(always-on). So p1's contribution to p2 is COMPREHENSION (three named rows in
the always-green band), not new tickable cells. The matrix's genuinely
new-tickable growth is elsewhere (e.g. the photographer split from gallery's
bit, `roster.hpp:LATENT[roster-split:photographer]`; the aura is already its
own `pawn_aura` bit). p2 is worth doing for legibility regardless of p1; p1
sharpens WHICH concepts the always-green band names.

---

## R4 — THE PANEL CENSUS (the tuning surface)

### TERRAIN FIRST, IN FULL — the four-home map

Terrain's tunables are **already gathered within each home, but scattered
across FOUR homes, and split C++ / WGSL.** The panel challenge is the
scatter and the language boundary, NOT within-file hunting.

| Home | Language | What it holds | Grade | Where |
|---|---|---|---|---|
| `contracts/surface_services.hpp` | C++ | streaming/extent: `PATCH_EXTENT`, `PATCH_CELL_SIZE`, radii (`GRID/RENDER/PREGEN`), `MAX_PATCHES`, the frame budgets (`SPAWN/ALLOC/EVICT` + tiers), the visibility cylinder (`VISIBLE/LOD` radii) | GATHERED | `:75-151` |
| `surface/tile_world.hpp` | C++ | generation: `ARCHETYPES` (4 profiles), the coherence multipliers, the `DENSITY_*` field (lattice/seed/exponent/min/max), `TERRAIN_EMISSION`, `AMP_MOMENTUM` | GATHERED | `:24-98` |
| `surface/population_themes.hpp` | C++ | theme/palette SELECTION: `THEME_LATTICE_SPACING`, `THEME_SEED_BAND`, `THEMES` (5) | GATHERED | `:19-57` |
| `realization/world.wgsl` | WGSL | **the visually-dominant half:** the wave-band GEOMETRY (`TERRAIN_BANDS`, 6 bands × 10 fields — freq/amp/damping μ&σ + temporal_freq, `:320-337`), mesh resolution (`TERRAIN_MESH_N=256`, `PATCH_HEIGHTFIELD_N=256`, `PATCH_MESH_N=64`, `:248-255`), the PALETTE colors (`PALETTE_CENTER/LIGHT/VARIANCE/WEIGHT` + `PALETTE_LATTICE_SPACING`, `:1467-1502`), the DRIVERLESS VOICE (the activity lattice — `ACTIVITY_LATTICE_SPACING`, `ACTIVITY_BEAT_FREQ_LO/HI`, `:347-352` — v3 §D2 "the surface's voice") | GATHERED (in the shader's own SECTION MAP, `:64-95`) | `:64-352, 1467-1502` |

`surface/patch_system.hpp` (the merged bodies) is now nearly constant-free —
its dials graduated to `surface_services.hpp` in DISSOLVE-1 Batch D; only a
hardcoded rig-pier fixture geometry block remains inline (`:322-348`).

**The C++↔WGSL bridge that exists today is thin:** a handful of terrain
dials are config-uniform-driven from C++ (`config.terrain_amp_ceiling` set
by mood, `config.world_seed`, `config.terrain_time`, `config.pawn_aura_
height` — `world.wgsl:470, 2324, 2413, 2571`). The BANDS, PALETTE, VOICE,
and mesh-N — the dials that shape the terrain the eye actually sees — are
WGSL const tables, NOT reachable from a C++ panel.

**What terrain's panel would gather — and the scope question for the stamp.**
An author wanting to shape a demo's terrain must today edit up to four files
in two languages. Three options, in ascending cost:
- **(a) The panel INDEX (v1, recommended).** One terrain panel block that
  CONSOLIDATES the three C++ homes into one named region and INDEXES the
  WGSL dials (a cited table pointing at `world.wgsl`'s bands/palette/voice).
  The author sees every terrain dial in one place; WGSL edits still happen
  in the shader. Low cost, no mirror risk, honest about the boundary.
- **(b) The uniform BRIDGE.** Promote the WGSL geometry dials (bands,
  palette, voice) to `config` uniforms the C++ panel writes. Makes them
  live-tunable and per-demo, but it is the bigger lift and re-opens the
  C++/WGSL MIRROR problem (a dial authored once must reach both sides). Pull
  when per-demo terrain SHAPE authoring is actually demanded (the demo
  contract's D3 "design-table overrides, pulled when a demo demands them").
- **(c) A shared constants header + WGSL codegen.** The full mirror-law
  solution; no WGSL codegen exists today (the shader is raw source). Out of
  scope for PANEL-0; named for completeness.

RECOMMENDATION: terrain panel = **(a)** — the C++ three consolidated + the
WGSL index. The bridge (b) is a named sub-movement, not pre-built.

### THE OTHER MODULES — one-line readiness grades

Reference exemplars (the target shape): **`entities.hpp`** and
**`ribbon.hpp`** (ribbon's block is literally labelled "control-panel
constant"). Grades:

| Module | Grade | ~Count | Note |
|---|---|---|---|
| bodies/entities.hpp | GATHERED | ~200 | archetypal per-family panel (SpawnConfig / param-defs / tiers / traits) |
| bodies/ribbon.hpp | GATHERED | ~90 | best-in-class; bodies hold only math |
| bodies/orbs.hpp | GATHERED | ~60 | dome/flock defaults + palettes + `ORB_MOOD_TABLE` |
| bodies/gol_zones.hpp | GATHERED | ~60 | spawn config + `GOL_TIERS`/`PULSE_TIERS` |
| bodies/agents.hpp | GATHERED | ~40 | possession/eviction radii + behavior/tier/population tables |
| bodies/cube_behaviors.hpp | GATHERED | ~30 | spring/drag/corral + tier gains |
| machine/spawn_engine.hpp | GATHERED | ~30 | cull margins + proximity matrix |
| bodies/spheres.hpp | GATHERED | ~18 | param-defs + tier table |
| bodies/pawn.hpp | GATHERED | ~14 | explicit "TUNING CONSOLE"; nothing scattered |
| machine/entity_pipeline.hpp | MIXED | ~150 / ~6 | param-defs gathered; only color-jitter helpers stranded |
| bodies/gallery.hpp | MIXED | ~90 / ~12 | strong block; ~12 painting-layout dims stranded in build fns (`:773-1591`) |
| direction/mood.hpp | MIXED | ~60 / ~35 | **biggest scattered surface** — indoor/vault/portal/spotlight builders riddled with inline cone angles, ambient colors, margins, scheme thresholds (`:539-541, 961, 1264-1268`) |
| direction/input.hpp | SCATTERED | ~2 | trivial — mouse sensitivity `0.005f` (`:292`), scroll rate (`:309`) |
| realization/render_passes.hpp | SCATTERED | ~8 | no block; shadow/spot camera extents (altitude 300, ortho 350, far 800 — `:695, 742, 744`) buried in matrix math — load-bearing "look" dials |

**Hardest future panel work:** mood (~35 lighting dials to gather), then
render_passes (small but 100% scattered, and they are visual "look" dials),
then gallery (~12 layout dims). Nine modules are already de-facto panels —
the work there is renaming to a uniform block shape, not hunting.

---

## R5 — THE THREE MOVEMENTS, SIZED (Jean's stamped order)

### p1 — THE PAWN DECOMPOSITION (player / body / behavior)
- **What it is:** name the three concepts R2 exposes — the PLAYER (driver +
  anchor organ), the BODY (the anchored slot-0), the BEHAVIOR (gait 0) — as
  distinct pieces with their own roster presence (always-green rows) and
  their own panel blocks. v3 §9 made concrete.
- **Size:** SMALL–MEDIUM, C++-side. The three are already 80% separate (an
  organ, a slot, a gait row); the cut is relabelling + possibly graduating
  the anchor fields into a legible "anchor" grouping within the witness
  record. No module is created or destroyed; no weld resists (R2).
- **Risk:** LOW. The arrow law needs no new teeth (the anchor fields are
  already sole-author-guarded, census Direction W). The FOUNDATIONAL floor
  means the rows are always-on — no gate logic changes.
- **PERF — disclosed separately (R1):** p1 as stamped is COMPREHENSION-ONLY
  and touches boot time ZERO. The perf payoff (the 7×24 BODY chain) requires
  a DISTINCT movement — a BODY-chain pipeline extraction or a lattice-bake
  (R7's levers) — and the pipeline blob cache is the orthogonal warm-boot
  lever. NONE of these three perf levers is p1; do not fold them in.
- **Gates:** prime invariant, score census (A/B/W — the witness contract
  must stay sole-authored through the anchor relabel), the minimal witness.

### p2 — THE MATRIX SURFACE (DemoConfig as pieces × demos)
- **What it is:** the 19-bit vector, today per-demo flat lists, becomes a
  legible pieces × demos grid (R3). Composing a sentence becomes editing
  cells. The two demos are the first two columns; the FIRST EDGE generalizes
  to per-column validation; the FOUNDATIONAL floor becomes the always-green
  band (p1's three concepts named there).
- **Size:** SMALL–MEDIUM. A presentation over the same `constexpr` bits — no
  runtime change, no boot-time registry (parked). Two columns exist.
- **Risk:** LOW, with one hard invariant: the compile-time boolean nature is
  NON-NEGOTIABLE (the pipeline gate folds `ROSTER`; RIDER A). The grid must
  still resolve to the per-demo `Roster` literal. YAGNI — grow by pull.
- **Re-rank disclosed (R3):** p1's rows are foundational, so p1 feeds p2
  comprehension (named always-green rows), not new toggles. p2 stands on its
  own legibility value.
- **NEW STANDING WITNESS:** once p2 lands, a **terrain-only sentence** (all
  families + features off, surface + pawn + sun only — today's `minimal`)
  becomes a first-class named column and a standing rig witness for p3.
- **Gates:** prime invariant (full column byte-identical to head), score
  census, glaw1 at every demo column, the minimal/terrain witness.

### p3 — THE PARAMETER PANELS (per module, one at a time, TERRAIN FIRST)
- **What it is:** each module's tunables gathered into one named control-panel
  block, reachable per demo. TERRAIN FIRST (R4): consolidate the three C++
  homes + index the WGSL dials (option a); the WGSL→uniform bridge (option
  b) is a named sub-movement pulled on demand.
- **Size:** per module — SMALL for the 9 GATHERED (rename to the uniform
  block shape; `entities`/`ribbon` are the templates), MEDIUM for the 3
  MIXED (mood is the largest single gather, ~35 dials), and terrain is its
  own shape (four-home consolidation + the C++/WGSL boundary decision).
- **Risk:** the C++/WGSL MIRROR (terrain option b, and any dial that must
  reach both sides). Option (a) avoids it; the bridge is where the risk
  concentrates — hence deferred, pulled per demand.
- **Discipline:** pull-per-module (one module per cut, terrain first), the
  demo-contract cadence — each panel arrives with the demo that wants to
  tune it.
- **Gates:** prime invariant (a panel is a re-home of existing constants —
  byte-identical values, gated), score census, the terrain-only witness
  (p2's new standing witness proves terrain still streams/culls/snaps after
  its dials are re-homed).

### The perf re-rank, stated plainly for the stamp
The order pairs the campaign with "the honest perf question." R1 settles it:
the pawn split (p1) is comprehension; boot time is untouched by p1 alone.
Three separate perf levers exist, none inside p1/p2/p3: (i) BODY-chain
pipeline extraction, (ii) lattice-bake (both cut the ~34s COMPILE), (iii)
the pipeline blob cache (cuts warm-boot RECOMPILE, orthogonal, exists
nowhere). If perf is wanted alongside the authoring campaign, it rides as a
fourth, independently-stamped movement — recommended AFTER p1 (which names
the BODY whose chain (i) would extract), but it is Jean's call whether and
when.

---

## STOP — THE STAMP REQUEST

The recon returns: R1 (the perf locus — BODY-side, split ≠ perf, blob cache
orthogonal), R2 (the pawn is three near-separate things; foundational), R3
(19 bits, one edge, the pieces × demos grid preserving constexpr), R4 (the
terrain four-home map + the C++/WGSL scope question; the others' readiness
table), R5 (p1/p2/p3 sized in the stamped order, with the perf re-rank
disclosed). Open questions for the stamp:

1. **The matrix shape** (R3) — is the pieces × demos grid the right first
   form (presentation over the same constexpr bits, grow-by-pull), or is a
   different legibility target wanted?
2. **The terrain panel scope** (R4) — v1 = option (a) the index+consolidation
   (recommended), or straight to (b) the WGSL uniform bridge?
3. **The perf movement** — do you want a fourth stamped movement (BODY-chain
   extraction / lattice-bake / blob cache), and in what order relative to
   p1–p3? Or does perf stay parked while the authoring surface lands?
4. **p1's roster form** — do the player/body/behavior rows join the roster as
   always-green FOUNDATIONAL entries (comprehension), or stay documented
   concepts without roster bits (since they cannot be ticked off)?

Nothing cut. p1 cuts only after the stamp.
