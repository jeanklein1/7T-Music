# the_board — Seam Map

A growing ledger of structural observations across the cartridge. One chapter
per module: what it owns, what it consumes, what it exposes, what it *almost*
owns but leaks elsewhere, plus loose strands and genuine knots.

This is a **description**, not a refactor proposal. Surgery comes after the map
is complete.

---

## How to read this document

The first chapter is an **orientation map** of `cartridge.hpp` — different
shape from the rest. It catalogs structure (members, inline content, lifecycle
surface, per-frame flow, dispatch hub) so the module chapters that follow can
cite into it without re-explaining the spine.

Module chapters use a consistent shape:

- **Owns** — state and invariants the module is the home for.
- **Consumes** — what it reads from outside, and (when relevant) when in the frame.
- **Exposes** — what it writes that's read by outside code.
- **Almost owns** — responsibilities the module is the natural home for but
  whose implementation lives elsewhere. The *leaks*.
- **Loose strands (Ln)** — free wins. Local, reversible, no semantic change.
  Catalogued now, pulled as a batch once the full map is drawn — pulling early
  builds false confidence.
- **Knots (Kn)** — genuine difficulties. Resolving one may depend on resolving
  another in a different chapter.
- **Open decisions (Dn)** — questions that need conversation before any move.

A note on **artistic specificity vs. structural leakage** (introduced in
Ch. 5). Some "generic" functions in the codebase carry mode-specific or
family-specific rules. These are not always seams. *Indoor moods need
indoor-specific rules; finite_outdoor will need its own; specific entity
families need specific behaviors.* That specificity is artistic intent, and
forcing it through a uniform shape is hostile to the work. The seam map's
job for these cases is not to demand uniformity — it's to ask:

- Is the specificity *named* (clear which mode/family the rule serves)?
- Is the *encoding* of the named specificity clean (a profile flag, an
  adapter override, a dispatch table) or muddled (magic number, deep nested
  branch, shared state)?
- Does the encoding survive new modes/families being added?

A function with three well-named branches for three well-named modes is
fine. The same function with an `if (mood == 5)` magic number, or with
shared mutable state that ties unrelated modes together, isn't. The seam
map flags the *encoding* problem, not the specificity itself.

A note on **dead code vs. latent code** (introduced in Ch. 10,
expanded by the world.wgsl audit). Some unreferenced code in this
project isn't dead — it's *latent*: written ahead of the feature that
will use it, sitting ready for the right artistic moment. The sun VP
machinery is one example (the sun's position and intensity will become
musically expressive); reserved trait fields like `behavior_amp_mult`
(Ch. 9) are another; "hygiene rows" in registries (Ch. 9 P4) are a
declarative version. The world.wgsl audit found ~15 explicit
`reserved` / `placeholder` / `future X` annotations on the GPU side —
**the pattern is not something the seam map discovered, it's something
the codebase has been practicing all along, named here only after the
GPU side already had it as everyday practice.**

**Before proposing any deletion of unreferenced code, the seam map
should distinguish:**

- *Dead* — written for a feature that came and went; the feature is
  gone but the code wasn't cleaned up. Safe to delete.
- *Latent* — written for a feature that hasn't arrived yet; the code
  is the artist's note-to-self about what's coming. Keep, ideally with
  a comment naming the intent so future readers don't mistake it for
  dead.
- *Stale documentation* — the code itself is fine, but a TODO or
  comment block claims it needs cleanup that has already happened.
  Delete the documentation, keep the code.
- *Historical breadcrumb* — a comment that names code that was
  deliberately removed, kept so future readers know the code was
  intentionally absent rather than missing. The world.wgsl file uses
  these heavily — `// (verify_motor_norm removed — diagnostic function,
  never called)`, `// (legacy chart constants removed)`, etc.
  **Distinct from stale documentation: the breadcrumb is correct, it's
  pointing at deliberate absence.** Do not delete breadcrumb comments
  during cleanup; they're part of the project's archaeology.

When in doubt, ask. Default to "latent." The cost of carrying unused
code is small; the cost of deleting a future feature's foundation is
large.

A note on **inline tag systems already in the codebase** (introduced
by the world.wgsl audit). The world.wgsl file uses several inline tag
families that pre-date this seam map:

- `[STATE:<name>]` — struct declarations (signal, terrain, agent,
  camera, floating_entity, ribbon, patch, config). 8 instances.
- `[COUPLING:<source>→<target>:<aspect>]` — cross-system coupling
  functions (~14 instances, e.g., `[COUPLING:signal.polyphony→terrain:amplitude]`).
- `[DYNAMICS:*]`, `[BINDINGS:*]`, `[DATA-DRIVEN ...]` — smaller groups.

These are **architectural tags** — permanent labels marking what a
function or struct is *for*. The seam map's proposed
`SEAM[<module>:<kind><n>]` is a third tag family with a different
role: **structural-observation tags** — mostly transient, marking work
to do or decisions to make, removed when the seam closes.

The two systems are visually distinguishable (no `SEAM` prefix on
permanent tags, `SEAM` prefix on observations) and grep-compatible.
Neither system invalidates the other.

The final chapter — the spine seam map — uses the module shape on
`cartridge.hpp` itself, harvesting the leaks the prior chapters fed into it.

---

## Tag conventions (in source)

Tags are single-line comments placed at the relevant code site. They mirror
the chapter ids in this document so `grep` works both directions.

**Format:** `// SEAM[<module>:<kind><n>] <one-line summary>`

- `L<n>` — loose strand
- `K<n>` — knot
- `D<n>` — open decision

**Discoverability:**

- All flags for one module:        `grep "SEAM\[musical:"`
- All knots across the project:    `grep "SEAM\[.*:K"`
- All open decisions:              `grep "SEAM\[.*:D"`

Tags are added in a separate pass after a chapter's seam map is agreed. Each
chapter ends with a **Proposed tags** block listing exact insertion points.
The orientation chapter (Ch. 1) emits no tags — its content gets tagged in
the spine seam map at the end of the tour.

---

# Chapter 1 — `cartridge.hpp` (orientation)

The spine. 9163 lines. One class — `Cartridge : public RenderCartridge` —
spanning the whole file. This chapter maps the structure so module chapters
can refer to line ranges and section names without restating context.

## File layout at a glance

```
        1 ─ 28     namespace + includes
       29          private:  ── members and inline content begin
       29 ─ 553    State declarations (input, transition, mood tables, indoor lights, wall art, lighting schemes)
      554 ─ 626    [INLINED]   modules/seed_utils.inl     ~71 lines
      628 ─ 1693   [INLINED]   modules/entities.inl       ~1059 lines
     1696          #include    modules/pawn_aura.inl
     1699          #include    modules/ground_architecture.inl
     1702 ─ 1737   #include    modules/orbs.inl + ORB_MOOD_TABLE definitions
     1741          #include    modules/agents.inl
     1744 ─ 2928   [INLINED]   modules/spawn_engine.inl   ~1184 lines
     2689          #include    modules/entity_types.inl   (mid-spawn_engine — see ordering note)
     2929 ─ 3347   Sphere + Cube entity systems, ribbon select/place/commit
     3349 ─ 3890   [INLINED]   modules/gol_zones.inl       ~541 lines
     3893 ─ 5460   [INLINED]   modules/gallery.inl         ~1567 lines
     5461 ─ 5815   Patch streaming infrastructure (deferred uploads, budgets, density field)
     5816 ─ 6223   FAMILY_DISPATCH wrappers (mesh-gen, eviction, GoL, gallery, ribbon)
     6227          #include    modules/entity_pipeline.inl
     6232          #include    modules/floaters.inl
     6234 ─ 6271   FAMILY_DISPATCH[] static table (12 rows)
     6273 ─ 7619   Population themes, theme envelope, terrain tokens, batch system,
                   affinity matrix, scale character, separation matrix
     7621 ─ 7775   Visibility cylinder, distance-sorted patch scan helper, patch streaming helpers
     7777          public:  ── lifecycle surface begins
     7778 ─ 7892   initialize() + init_renderer()
     7894 ─ 8255   update()                              ~362 lines
     8257 ─ 8264   ORDER comment block (GPU dispatch order)
     8266 ─ 8643   render()                              ~378 lines
     8647 ─ 9106   stream_patches()                      ~460 lines
     9109          #include    modules/mood.inl          (late — included after stream_patches)
     9113          #include    modules/render_passes.inl (late — included after stream_patches)
     9115          public:  ── on_input()
     9137          private: ── #include modules/input.inl
     9142          public:  ── get_clear_color, depth_format, supports_backspace, reload_shaders, shader_path
     9163          end of class
```

## Class members (instance state)

Per-instance fields that evolve at runtime. Line ranges approximate.

- **GPU connection** (31–35): `device_`, `colorFormat_`, `depthFormat_`,
  `gpuState_`, `renderer_`.
- **Input state** (38–58): `inputState_` (deltas), `keys_` (held), `mouse_`
  (drag flags).
- **Camera mode** (60): `fpvMode_`.
- **Frame timing** (61–63): `currentBeats_`, `currentSeconds_`, `currentDt_`.
- **Sun + atmosphere** (66–72): `sunDirection_`, `sunColor_`, `sunIntensity_`,
  `sunAmbient_`, `clearColor_`, `activeMood_`, `terrainAmpCeiling_`. All
  driven by `apply_mood`.
- **Musical coupling state** (75): `#include "modules/musical.inl"` — see Ch. 2.
- **Player state** (87–94): `PlayerState player_` with `possessed_slot`.
- **Spotlight array** (96–97): `cpuSpotLights_`, `spotLightActive_`.
- **World transition state machine** (100–104): `transitionPhase_`,
  `transitionTimer_`, `transitionFadeDuration_`, `transitionFadeAlpha_`.
- **Portal destination** (107–114): `pendingDestination_` (seed, finite,
  finite_radius, mood) — also used for keys + portal crossings.
- **Finite patch mode** (117–118): `finiteMode_`, `finiteRadius_`.

Instance state ends around line 118. Most additional state lives inside the
inlined modules and the `.inl` includes that follow.

## Static declarative tables (class-scope constants)

Class-scoped `static constexpr` tables that drive runtime behavior via lookup.
Read once at boot or per-mood-transition; never mutated. This is the "control
panel at the top of the file" — most of what an authoring pass would touch.

- **Portal detection constants** (121–132): `PORTAL_DENSITY`,
  `PORTAL_COLORS[6][3]`, `PORTAL_COLOR_BACK`.
- **MoodProfile struct + MOOD_TABLE** (134–218): the canonical 6-row mood
  table. Sun direction, fog, ceiling type, clear/wall/ceiling colors, plus
  per-mood feature flags (`allow_musical_modes`, `allow_gol_zones`,
  `allow_pawn_aura`, `allow_frustum_cull`).
- **Mood IDs** (190–203): named indices into MOOD_TABLE.
- **Indoor palettes** (234–263): 8 designed (wall, ceiling) pairs.
- **Wall art configuration** (265–352): scale buckets, wall participation
  thresholds, per-wall painting count, content×form mixing config.
- **Indoor entity placement** (354–371): geometry constants for placing
  paintings/galleries inside walled moods.
- **Indoor lighting schemes** (373–553): scheme table, scheme definitions,
  `LIGHT_SCHEMES[]` data.

Beyond line 553 the file becomes a sequence of subsystems with their own
state and helpers interleaved with the inlined module content.

## Inlined content (banner-only modules)

Five sections in the class body carry banner-style headers
(`═══ INLINED: modules/<name>.inl ═══`) but live in the file rather than as
separate `.inl` files. They were grouped together during a partial
decomposition pass; the headers exist to make the next extraction trivial.

| Banner module     | Lines       | Size     | What it is |
|-------------------|-------------|----------|------------|
| `seed_utils.inl`  | 554–626     | ~71      | Hashing, Gaussian sampling, tier selection. Pure math. |
| `entities.inl`    | 628–1693    | ~1059    | Vocabulary of forms: Ribbon, Arch, Column, Antenna, Palm, Cactus, Blade, Pyramid — tier profiles, property registries, tracking structs. |
| `spawn_engine.inl`| 1744–2928   | ~1184    | Spawn helpers, pier write, distance culling, footprint registry, census, density, proximity affinity, gallery+gol+ribbon selection, entity queue, sphere+cube systems. |
| `gol_zones.inl`   | 3349–3890   | ~541     | GoL zone system: tier profiles, pulse profiles, runtime state, select/place/commit. |
| `gallery.inl`     | 3893–5460   | ~1567    | Photographer, painting tiers, snapshot/authored/exhibition staging, slot management, gallery select/place/commit, frame styles, wall painting placement. |

Total: ~4422 lines of banner-only content — roughly half the file.

**Ordering subtlety inside `spawn_engine.inl`:** at line 2689,
`modules/entity_types.inl` is `#include`d *before* the `EntityQueueEntry` and
`PlacementEntry` union definitions appear (at ~2691, ~2712). That ordering is
required: `entity_types.inl` declares `EntityInstance`, which is used as a
union member in those queue structs. The matching `entity_pipeline.inl` is
included much later (line 6227), *after* the unions — also intentional
(adapters and family data depend on the queue types). This sandwich is why
the entity machinery is split across two `.inl` files instead of one.

## Real `.inl` modules

Eleven files in `cartridges/the_board/modules/`, each a real include with its
own contents and dependencies declared in its header.

| Module                    | Include line   | Approx size | Role                                      |
|---------------------------|----------------|-------------|-------------------------------------------|
| `musical.inl`             | 75             | 103         | Mode IDs, intensity state, toggle.        |
| `pawn_aura.inl`           | 1696           | 60          | Aura profile, default config, presence ramp constants. |
| `ground_architecture.inl` | 1699           | 271         | Contributor IDs, policy bitmasks, DAG closure validation. |
| `orbs.inl`                | 1702           | 1127        | Sky orbs: palettes, tier sets, gestures, mood configs, kernels. |
| `agents.inl`              | 1741           | 956         | Agent registries (behavior / tier / population) + lifecycle. |
| `entity_types.inl`        | 2689           | 107         | Generic entity type definitions (header sandwich part 1). |
| `entity_pipeline.inl`     | 6227           | 1969        | Generic entity pipeline: select / place / commit + per-family adapters (sandwich part 2). |
| `floaters.inl`            | 6232           | 497         | Cube behavior registry (separate from spheres). |
| `mood.inl`                | 9109           | 1067        | Atmosphere, indoor lighting derivation, shell geometry, portals. |
| `render_passes.inl`       | 9113           | 774         | Pre-render data prep, GPU compute dispatch, shadow + main + snapshot passes, light matrices. |
| `input.inl`               | 9140           | 336         | Keyboard / mouse / scroll handlers, key bindings. |

## Public lifecycle surface

The methods exposed to the engine spine. Everything else in the class is
private.

- **Constructor:** `Cartridge() = default;` non-copyable.
- **`initialize(wgpu::Device)`** (7783) — boots `gpuState_`. Times the GPU init.
- **`init_renderer(colorFormat, depthFormat)`** (7793) — initializes renderer,
  generates terrain index buffer (one-shot compute), bootstraps patch system,
  uploads agent registries, spawns boot mood population, eager-loads authored
  paintings. Logs four timing sections.
- **`update(signal, aspect_ratio, queue)`** (7894) — CPU-side per-frame work.
  See *Per-frame flow / update()* below.
- **`render(encoder, backbuffer, depth)`** (8266) — GPU per-frame work.
- **`on_input(event)`** (9117) — switch over `KeyDown` / `KeyUp` / `MouseMove` /
  `MouseButton` / `Scroll`, dispatching into the handlers from `input.inl`.
- **`get_clear_color(r, g, b)`** (9144), **`depth_format()`** (9150),
  **`supports_backspace()`** (9154), **`reload_shaders()`** (9158),
  **`shader_path()`** (9159) — small accessors.

## Per-frame flow

Three big functions run each frame in this order: `update()` → `render()` →
(internally `render()` calls `stream_patches()`).

### `update()` (7894–8255) — CPU-side state evolution

| Range       | Phase                                                             |
|-------------|-------------------------------------------------------------------|
| 7898–7916   | Build `GPUFrameSignal` from analysis signal + cached input deltas |
| 7918–7920   | Cache `currentBeats_`, `currentSeconds_`, `currentDt_`            |
| 7924–7950   | Aura presence trajectory ramp; world bounds upload                |
| 7952–8045   | Transition state machine (FADE_OUT / TEARDOWN / FADE_IN)          |
| 8046        | `set_fade()` to GPU                                               |
| 8048        | `upload_signal()` — pushes `GPUFrameSignal` to GPU                |
| **8050–8087** | **Polyphony-driven band motion** (musical.inl K2)              |
| **8089–8198** | **Musical animation modes** (mmodes + palette drift) (K2)      |
| **8200–8236** | **Radial pulse onset detection** (K2)                          |
| 8238        | `upload_config()` — pushes `GPUDesignConfig` to GPU               |
| **8240–8243** | **Orb musical coupling** (sibling to musical.inl K2)           |
| 8245–8247   | Orb dome anchor follow                                            |
| 8253        | `update_photographer()`                                           |
| 8254        | `clear_input_deltas()` — zeros input deltas for next frame        |

Bold rows = polyphony consumer sites flagged in Ch. 2's K2.

### `render()` (8266–8643) — GPU dispatch

| Range       | Phase                                                             |
|-------------|-------------------------------------------------------------------|
| 8270        | `queue = device_.GetQueue()`                                      |
| 8272–8313   | Agent buffer readback (state machine: COPIED → MAPPING → IDLE)    |
| 8315–8378   | Floater buffer readback (parallel state machine)                  |
| 8380–8394   | Portal trigger handling (from agent readback `portal_trigger`)    |
| 8398        | `respawn_evicted_agents()`                                        |
| 8403        | `tick_cube_corral_animations()`                                   |
| 8405        | **`stream_patches()`** — see next subsection                       |
| 8413–8430   | Periodic agent + entity census dumps                              |
| 8432–8474   | Ribbon eviction + nearest-rendering selection                     |
| 8476–8493   | **Entity mesh gen** — single compute pass via FAMILY_DISPATCH     |
| 8494        | `upload_portal_array()`                                           |
| 8495        | `upload_lights()`                                                 |
| 8496        | **`dispatch_compute()`** — terrain config, agents, camera, sphere, cube, VP |
| 8498–8505   | Copy agent buffer → readback staging (for next frame)             |
| 8507–8516   | Copy floater buffer → readback staging                            |
| 8519–8552   | GoL zone compute (sync + evolve + mesh gen) — gated on `golZoneCount_` |
| 8556–8605   | Pawn aura compute — gated on `auraPresence_`                      |
| 8610–8613   | Orb dispatches (init, recolor, copy_prev, dynamics)               |
| 8615–8623   | Ground entries upload + placement correction (Y-correct entities) |
| 8626        | `dispatch_frustum_cull()`                                         |
| 8628        | `render_shadow_pass()`                                            |
| 8629        | `render_main_pass()`                                              |
| 8630        | `render_snapshot_pass()` (gallery photographer)                   |
| 8635–8642   | Texture promotions (staging → exhibition layers)                  |

### `stream_patches()` (8647–9106) — patch streaming pipeline

Called from `render()` line 8405. The longest single function in the file.

| Range       | Phase                                                             |
|-------------|-------------------------------------------------------------------|
| 8696–8712   | Compute grid center; cap radius for finite mode                   |
| 8714–8805   | **On grid shift:** evict distant tiles, mark tile grid dirty, FULLREGEN bootstrap on first frame |
| 8807–8835   | **CONTINUOUS PATCH EVICTION** — farthest first, EVICT_BUDGET/frame, compact array |
| 8837–8926   | **CONTINUOUS PATCH ALLOCATION** — nearest first, ALLOC_BUDGET/frame |
| 8928–8947   | **DISTANCE-DRIVEN ENTITY SPAWNING** — sort unspawned patches by distance, spawn up to SPAWN_BUDGET/frame |
| 8949–9087   | **DISTANCE-DRIVEN HEIGHTFIELD GENERATION** — generate up to budget, then build patch instance array (LOD0 / LOD1 / pregen), upload, build O(1) patch grid index |
| 9091–9093   | Mark `groundEntriesDirty_`, `placementDirty_` if patches changed  |
| 9095–9096   | `update_entity_draw_visibility()` — distance culling for draw     |
| 9098–9100   | Deferred uploads (tile_grid, pier count)                          |
| 9102        | `audit_entity_integrity()`                                        |
| 9104–9105   | Restore radius if capped for finite mode                          |

## Cross-module dispatch — `FAMILY_DISPATCH`

The architectural unifier for entity families. Defined at line 6234, twelve
rows, one per `PopFamily` enum value. Each row carries six function pointers:

```cpp
struct FamilyDispatch {
    bool (*try_select)(...);     // select-phase wrapper
    bool (*try_place)(...);      // place-phase wrapper
    void (*try_commit)(...);     // commit-phase wrapper
    void (*evict_slot)(...);     // eviction wrapper
    bool (*prepare_mesh)(...);   // CPU-side mesh prep
    void (*dispatch_mesh)(...);  // GPU mesh-gen dispatch
    const char* name;
};
```

Two wiring patterns visible in the table:

- **Generic pipeline families** (pyramid, arch, column, antenna, palm, cactus,
  blade, sphere, cube): the wrappers call `dispatch_*_generic` functions
  defined in `entity_pipeline.inl`. These families share the
  `EntityFamilyAdapter` machinery.
- **Bespoke pipeline families** (ribbon, gol, gallery): the wrappers call
  hand-written `dispatch_select_<family>` / `dispatch_place_<family>` /
  `dispatch_commit_<family>` defined inline in the banner-only modules. Same
  dispatch *interface*, different *internals*.

The wrapper definitions and the table together span lines 5816–6271. The
table itself is the cleanest expression of "here are all the entity families"
— a 12-row glance.

**Compilation-order constraint:** the table at line 6234 references
`dispatch_*_generic` functions defined in `entity_pipeline.inl` — which is
why that include sits at line 6227, just before the table. The same kind of
ordering constraint as the `entity_types.inl` ↔ queue-union sandwich noted
above (one inside `spawn_engine.inl` at line 2689, one between the dispatch
wrappers and the table at line 6227). Two such constraints in this file,
both load-bearing.

## Per-frame entry into the dispatch hub

Five sites read the table:

1. **Spawning** (during `stream_patches()` → `spawn_selected_patches()` →
   `select_entities_for_patch()`): calls `try_select` per family per patch,
   building an entity queue.
2. **Placement** (`place_entity_queue()`): calls `try_place` for each queued
   entity (no GPU writes — spatial negotiation only).
3. **Commit** (`commit_entity_queue()`): calls `try_commit` for each placed
   entity (GPU writes, footprint registration, ground hierarchy bookkeeping).
4. **Eviction** (during patch eviction in `stream_patches()`): calls
   `evict_slot` for each entity reference owned by the evicted patch.
5. **Mesh gen** (in `render()` line 8480): calls `prepare_mesh` to detect
   dirty families, then `dispatch_mesh` for each within a single compute pass.

Each of these is a `for f in PopFamily::COUNT` loop over the table. Adding a
family is one row plus six wrappers — the rest of the spine adapts
automatically.

## Notable structural observations

These are observations the spine seam map (last chapter of this document)
will return to. Not yet seams; not yet decisions.

- **Inline-ness is visible.** The five `═══ INLINED ═══` banners make the
  partial decomposition state explicit. Trivial to extract; held back by the
  feature work that's still in flight.
- **Late mood/render_passes includes.** `mood.inl` and `render_passes.inl`
  are included at lines 9109/9113 — *after* `update()`, `render()`, and
  `stream_patches()` have already been declared. The functions in those
  modules are class-body member definitions; they show up *after* the
  callers because the includes are placed after them. The compiler doesn't
  care (member function bodies are deferred), but the reading order is
  inverted from logical order.
- **Two orthogonal dialect axes for entity families.** Two splits, not one:
  - *Pipeline dialect:* generic (table-driven `EntityFamilyAdapter` plus
    `dispatch_*_generic`) vs. bespoke (hand-written select / place / commit).
  - *Hosting dialect:* extracted real `.inl` vs. banner-only inline.

  These don't correlate cleanly. Pyramid / arch / column are
  generic-pipeline but their *tier definitions* live in banner-only
  `entities.inl` while their *machinery* is in real `entity_pipeline.inl`.
  Ribbon is bespoke-pipeline AND banner-only (`spawn_engine.inl`).
  Gol / gallery are bespoke-pipeline AND banner-only.

  Also: generic-pipeline families have *two* indirection layers —
  FAMILY_DISPATCH wrappers call `dispatch_*_generic` which reads the
  per-family `EntityFamilyAdapter` (declared in `entity_types.inl`) for
  `run_gate` / `compute_solid_half` / `write_gpu` / `post_commit`. Bespoke
  families have just one (FAMILY_DISPATCH wrapper directly into hand-written
  code). Whether to converge — and along which axis — is a Ch. 13 / 15
  question.
- **The "floater" partition.** Spheres and cubes share one GPU buffer
  (slot-offset partitioned in `state.hpp`), but only cubes have a behavior
  registry — `floaters.inl` is cube-only despite the name, by deliberate
  design (its header comment says so explicitly). Spheres run an analytical
  PGA orbit with no behavior layer. The taxonomy chapter (Ch. 13) will need
  to keep this split visible: shared *hosting*, separate *trajectory
  machinery*.
- **`stream_patches()` is a module waiting to happen.** 460 lines, six
  named phases (grid shift / continuous eviction / continuous allocation /
  distance-driven spawning / distance-driven generation / patch grid build),
  each clearly delineated. It has the shape of an extraction candidate:
  name it `streaming.inl`, promote each phase to a private member, leave a
  thin orchestrator behind. The spine seam map (Ch. 15) will revisit this
  as a knot or a loose strand depending on cross-references discovered
  during the tour.
- **CPU / GPU sovereignty has a textual signature.** `update()` ramps
  trajectories and pushes uniforms; `render()` dispatches kernels that
  consume those uniforms; `render()` also issues the readbacks that *next*
  frame's `update()` will read. Pawn aura is the canonical example:
  presence ramp in `update()` at 7924–7933 (CPU trajectory), compute
  dispatch in `render()` at 8556–8605 (GPU kernel), with `auraPresence_`
  and `auraCfgDirty_` as the wires between them. This pattern recurs
  throughout — naming it once here so module chapters can refer to "the
  sovereignty pattern" without re-explaining.
- **`update()` is the home of the musical couplings (Ch. 2 K2).** Lines
  8050–8243 hold the entire polyphony-consumer set. Untying that knot is
  load-bearing for the next chat.
- **Three per-frame functions, three different concerns.** `update()` =
  "evolve CPU state, push uniforms to GPU." `render()` = "dispatch GPU
  work." `stream_patches()` = "manage the streaming patch lifecycle." Clean
  separation; the names match the work.
- **State is grouped by concern at the top, then interleaved.** The first
  ~553 lines read as a control panel (mood definitions, palettes, lighting
  schemes, wall art). After that the file becomes a sequence of subsystems
  with their own state mixed in among helpers, until the public block at
  7777.

---

# Chapter 2 — `musical.inl`

The coupling layer between analysis signal and visual parameters: mode
definitions, intensity trajectories, band motion, palette drift, radial pulse.
Currently 103 lines. Sole consumer of `signal.stats[0]` (polyphony) — the
prime rewiring target for the next chat.

## Owns

- Mode IDs (`MMODE_TERRAIN_WAVES`..`MMODE_RADIAL_PULSE`).
- Mode activation mask (`mmodeMask_`).
- Per-mode intensity trajectory state (`mmodeIntensity_[MMODE_COUNT]`).
- Band blend state (`bandBlend_[6]`, `bandPhaseOrigin_[6]`, `bandBlendTarget_[6]`).
- Palette drift state (`paletteDriftTarget_`, `paletteDriftDesired_`).
- Radial pulse ring buffer (`pulseRing_[32]`, `pulseWriteIdx_`, `prevPolyphony_`).
- The activation toggle (`is_mmode_on`, `toggle_mmode`).
- Authored coupling rates (`MMODE_ATTACK`, `MMODE_RELEASE`, `BAND_BLEND_*`,
  `PALETTE_DRIFT_TARGET_RATE`, `PULSE_AMPLITUDE`, `PULSE_MAX_AGE`).

## Consumes

- `signal.stats[0]` — polyphony. Read at *seven* sites in `cartridge.hpp::update()`
  (see Ch. 1's update() phase table, rows 8050–8243). The rewiring target.
- `currentDt_` — for trajectory ramps.
- `moodAllowsMusicalModes_` — gate from `mood.inl` (silences all modes per mood).
- `auraCfgDirty_` — written when toggling `MMODE_AURA_EXPAND` (to push updated
  aura params downstream).

## Exposes

- `mmodeIntensity_[]` — read by GoL tempo, palette drift, aura expansion logic;
  uploaded to terrain VS via `gpuState_` setters.
- `bandBlend_[]`, `bandPhaseOrigin_[]` — uploaded via `gpuState_.set_band_motion`
  and read by terrain VS for per-band blending.
- `pulseRing_[]` — uploaded to GPU; read by terrain compute for radial displacement.
- `bandMotionActive_` — read internally by the band-motion update site.

## Almost owns

The **per-frame coupling update** is the module's natural responsibility but
lives in `cartridge.hpp::update()` between roughly lines 8050 and 8243 (Ch. 1
phase table, bold rows). Seven sites, all reading `signal.stats[0]`, each
computing a target and ramping a trajectory toward it.

This is the structural reason "rewire polyphony to chord/BPM/pitch" feels harder
than it should: the *modes* are defined in `musical.inl`, but the *wires* that
read polyphony are in `cartridge.hpp`. One rewiring touches two files for every
mode.

The seven sites are interleaved with `gpuSignal` upload (8048),
`upload_config()` (8238), aura presence ramp (7924–7933), and orb coupling
(8243) — extracting them must preserve frame ordering.

## Loose strands

**L1.** `bandMotionActive_` (line 18) is a pure duplicate of
`is_mmode_on(MMODE_TERRAIN_WAVES)`. The comment in `toggle_mmode` even calls
itself "retroactive" — fossil from before mode 0 joined the mmode family.
Deletable; consumer sites call `is_mmode_on()` directly.

**L2.** `MODE_NAMES[]` (line 100) is a hidden registry sitting inside
`toggle_mmode()`. Used once for diagnostics. Promote to top-level
`MMODE_NAMES[MMODE_COUNT + 1]` (or fold into a registry table — see K1) and the
mode definitions read as one block instead of three fragments.

**L3.** Numpad bindings documented in the header comment (lines 31–36) are
*also* live as `case GLFW_KEY_KP_<n>: toggle_mmode(...)` switches in
`input.inl` (lines 172–178). Two sources of truth for the same mapping.
Either delete the redundant comment (input.inl is canonical) or move the
canonical mapping into a registry that `input.inl` reads.

**L4.** `MMODE_RADIAL_PULSE = 7` sits *outside* `MMODE_COUNT = 6`. This
encodes a real semantic distinction (event-driven onsets vs. continuous
intensity ramps) as an off-by-one. The `MODE_NAMES[]` array even has a
`"UNUSED"` placeholder at index 6 to bridge the gap. Resolvable by registry
shape (K1) — naming the distinction (`kind = INTENSITY_TRAJECTORY` vs
`EVENT_ONSET`) instead of encoding it numerically.

## Knots

**K1.** Three fragments describe one thing — the mode definitions:
mode IDs as constexprs, `MODE_NAMES[]` hidden in `toggle_mmode`, numpad bindings
in a comment. The natural unifier is a single `MMODE_REGISTRY[]` table modelled
on `AGENT_BEHAVIORS[]` / `CUBE_BEHAVIORS[]`. Each row would carry: id, name,
numpad key, kind (intensity vs. onset), source (the rewiring lever), attack,
release.

This is a knot rather than a loose strand because the **registry shape depends
on what we want next chat to look like.** Specifically: do we declare a
`SourceId` enum now (with one value, `POLYPHONY`), so each mode reads from a
registry-declared source — making next chat's edit purely "change rows"? Or
introduce `SourceId` as the first move of the rewiring chat? See D1.

**K2.** Pulling the seven coupling sites out of `cartridge.hpp::update()` into
a local `tick_musical_couplings(signal, dt, queue)` is the move that closes
the leak. The sites are interleaved with non-musical per-frame work (Ch. 1
phase table) and the order matters. **Ordering can now be verified against
Ch. 1**, but the actual extraction is best deferred until after the spine seam
map (final chapter), because other module chapters may add constraints.

**Target shape from world.wgsl audit.** The GPU side has the canonical
abstraction this resolution should mirror — `world.wgsl` §1.2 declares
`struct Trajectory { value, velocity, _pad0, _pad1 }` plus
`fn trajectory_release(t, goal, dt, rate) → Trajectory`, and every
musical coupling on the GPU side uses it (e.g.
`coupling_signal_polyphony_to_terrain_amplitude(polyphony, traj, dt)
→ Trajectory`). The C++ side has nothing analogous — the seven sites
in `update()` are ad-hoc inlined versions of the same exponential
trajectory math. **The end-of-tour resolution should introduce a CPU
counterpart** (a small `cartridges/the_board/modules/trajectory.inl`
or a foundations block declaring `struct Trajectory` + an
`exponential_release` helper). Then `tick_musical_couplings()` is a
thin function over named couplings using a shared primitive — same
shape on both sides.

**K3.** `prevPolyphony_` is consumer state for pulse onset detection. It's
correctly placed *if* `musical.inl` owns the per-frame update (K2 resolves);
it's stranded *if* the update stays in `cartridge.hpp`. Not independently
resolvable — moves with K2.

## Open decisions

**D1.** Define `SourceId` now (single value) so registry rows declare their
source today, or defer the entire source-indirection layer to the rewiring
chat? *Inclining toward defer:* introducing the field with one value is
makework, and the rewiring chat will have full context on what sources the
analysis layer actually exposes. **Decision deferred to end-of-tour.**

**D2.** Should `musical.inl` own the per-frame update (closing K2), or should
the per-frame couplings be a small `couplings.inl` sibling that reads
`musical.inl`'s registry? Argument for inside: state lives here, frame
ordering is local. Argument for sibling: keeps `musical.inl` as a *registry +
state* module (the cockpit) and the *circuit* lives next door. **Decision
deferred until the spine seam map is drawn.**

## Proposed tags

To be applied after the chapter is agreed.

```
File: cartridges/the_board/modules/musical.inl

Near line 18 (bool bandMotionActive_ = false; ...):
  // SEAM[musical:L1] duplicate of is_mmode_on(MMODE_TERRAIN_WAVES) — fossil

Near line 31 (// Numpad 1 = terrain waves ...):
  // SEAM[musical:L3] numpad bindings also live in input.inl — two sources of truth

Near line 38 (static constexpr uint32_t MMODE_TERRAIN_WAVES = 0;):
  // SEAM[musical:K1] mode IDs / names / numpad keys want unification into MMODE_REGISTRY[]
  // SEAM[musical:D1] registry source field — define now or defer to rewiring chat?

Near line 48 (static constexpr uint32_t MMODE_RADIAL_PULSE = 7; // numpad 7 ...):
  // SEAM[musical:L4] sits outside MMODE_COUNT=6 — semantic kind encoded as off-by-one

Near line 68 (float prevPolyphony_ = 0.0f; ...):
  // SEAM[musical:K3] consumer state for pulse onset — moves with K2

Near line 100 (static const char* MODE_NAMES[] = ...):
  // SEAM[musical:L2] hidden registry — promote to top-level or fold into K1
```

```
File: cartridges/the_board/cartridge.hpp

Near line 8050 (// ─── Polyphony-driven band motion ────...):
  // SEAM[musical:K2] band motion coupling — should live in musical.inl (almost-owns leak)

Near line 8089 (// ─── Musical animation modes: per-frame intensity ramp ───):
  // SEAM[musical:K2] mmode intensity coupling — should live in musical.inl

Near line 8200 (// ─── Radial pulse onset detection ────...):
  // SEAM[musical:K2] radial pulse coupling — should live in musical.inl

Near line 8240 (// Orb musical coupling: polyphony → radial expansion):
  // SEAM[musical:K2] orb coupling reads polyphony — sibling to musical.inl couplings
```

---

# Chapter 3 — `ground_architecture.inl`

The contract module for the ground query architecture: contributor IDs,
policy bitmasks, dependency DAG, compile-time DAG-closure validation. 271
lines. Header explicitly declares *"Depends on: nothing — pure enum + table
definitions + macro checks."*

This is not a runtime module. Nothing in the C++ codebase reads `POLICIES[]`,
`CONTRIBUTOR_DAG[]`, or any field on `PolicyDef` at runtime. The file's
runtime-side function is the nine `static_assert` invocations at the bottom.
Everything else is *declarative intent* — a contract the rest of the system
is supposed to honor.

## Owns

- `ContributorId` enum (70–83) — 11 named height/displacement sources.
- `PolicyId` enum (85–96) — 9 named consumer policies.
- `ContributorEdge` struct + `CONTRIBUTOR_DAG[]` (108–122) — 6 dependency
  edges among static_landform contributors.
- `PolicyDef` struct (128–133) — id, name, contributor bitmask,
  `gradient_supported` flag.
- `GROUND_STATIC_BASE_MASK` (137–140) — convenience mask for the three fused
  static-base contributors.
- `POLICIES[]` table (142–218) — 9 rows defining each policy's contributor set.
- `DAG_EDGE_CLOSED` macro + `ASSERT_POLICY_DAG_CLOSED` macro (237–258) — the
  closure-validation harness.
- 9 `static_assert` invocations (260–268) — one per policy; runs at compile time.

## Consumes

Nothing. The header's "Depends on: nothing" is accurate.

## Exposes

- The IDs and the policy table — but in practice these are *referenced in
  comments* throughout the codebase, not read by code:

  | Reader                       | Sites referencing module symbols |
  |------------------------------|----------------------------------|
  | `cartridge.hpp`              | 3 (all in comments)              |
  | `state.hpp`                  | 4 (all in comments)              |
  | `renderer.hpp`               | 5 (all in comments)              |
  | `render_passes.inl`          | 5 (all in comments)              |
  | `world.wgsl`                 | mirror — see *Almost owns* below |

- Compile-time DAG closure: any policy that adds a contributor without its
  ancestors fails to compile, with a named error message
  (`"POLICY_X: includes Y but not Z"`).

## Almost owns

**The C++/WGSL contract.** The header claims this file is "single source of
truth on the C++ side; world.wgsl mirrors the same ids and per-policy
bitmasks as `const` values." That's accurate as far as it goes — and what
*lives where* is the leak.

Three places that must agree:

1. **C++ `POLICIES[]`** (this file) — declarative bitmask per policy.
2. **WGSL `POLICY_*_MASK`** (world.wgsl lines 2069–2100) — same bitmask,
   hand-mirrored as `const u32`.
3. **WGSL `query_ground_<policy>` function bodies** (world.wgsl lines
   2545–2740) — *implementation*. Each function is hand-written; the
   contributors it calls are hardcoded by name.

Sources 1 and 2 are documentation-only. **The WGSL `POLICY_*_MASK` constants
are declared but never indexed or consumed anywhere in the shader.** Each
`query_ground_<policy>` function calls `contrib_*_at()` directly, in a
hardcoded sequence. If a function body adds or drops a contributor, the
matching mask in (1) and (2) does not auto-update; nothing fails.

The C++ compile-time validation only catches DAG-closure violations *inside
the C++ table*. It does not validate:

- That WGSL `POLICY_X_MASK` matches C++ `POLICIES[POLICY_X].contributors`.
- That `query_ground_x()`'s function body calls the contributors declared
  in either mask.

This is the canonical "single source of truth, sort of" pattern: three
places, none reads the others, only the first has compile-time enforcement
and only over a sub-property (DAG closure). The drift surface is not zero.

## Loose strands

**L1.** `gradient_supported` flag on `PolicyDef` is declared (line 132) and
**never read** anywhere — verified by grep across the full project. Pure
declarative metadata. Either delete (loose strand) or make it consumed —
the natural consumer would be a compile-time or runtime check that any
gradient-using consumer (`terrain_normal_at`, step-climb decisions) only
queries policies with `gradient_supported = true`. See D1.

**L2.** WGSL `POLICY_*_MASK` constants (world.wgsl 2069–2100) are declared
but never read by any shader code. They're documentation-as-code. Could be
deleted with no behavior change, or kept as the canonical declaration that
`query_ground_*` bodies are checked against — but no checking exists today,
so as-is they're inert. Folds into K1.

## Knots

**K1 — REFRAMED by world.wgsl audit.** Originally framed as "three-place
agreement, no validation between places" (C++ `POLICIES`, WGSL
`POLICY_*_MASK`, WGSL `query_ground_*` bodies). Reading world.wgsl
§2102–2287 shows this is **a deliberate, well-documented architecture
where each side owns a different concern**, not a leak.

What world.wgsl actually shows:

- **C++ `POLICIES[]` owns the contract** — DAG, contributor masks per
  policy, compile-time closure validation. The authoritative source.
- **WGSL `POLICY_*_MASK` constants are naming bookkeeping** — declared
  for documentation, rarely indexed (correctly noted by L2).
- **WGSL `query_ground_<policy>` functions are per-policy
  implementations**, hand-written with the FXC inlining property
  explicitly named in §2152–2178:

  > *"A policy is a compile-time contributor bitmask. A consumer
  > declares its policy by calling query_ground_<policy>. FXC sees a
  > uniform function choice and dead-code-eliminates contributors
  > outside the mask. The policy is part of the consumer's identity
  > — changing what a consumer sees requires changing its declared
  > policy."*

The "three places" aren't three fragments of the same thing. They're
three concerns owned by three sides. **This is exactly the case the
Ch. 5 vocabulary recognizes — intentional specificity (hardware: FXC
dead-code elimination), with clean encoding via per-policy function
dispatch.**

Plus §2211–2223 documents the **fused inline evaluations**
(`ground_formed_with_complexity` and `patch_terrain_vs`) — hand-fused
copies of policy-equivalent paths kept for per-vertex/per-texel
performance, with comment-as-policy enforcement (P6):

> *"Both must stay consistent with their policy's contributor set. If
> a policy gains or loses a contributor, update the fused function."*

**Action.** Downgrade from a knot to a careful Almost-owns observation
(see expanded *Almost owns* above). The "drift detection" question
remains real — currently rests on developer discipline plus the
fused-inline comment — but it's **discipline at a clean architectural
boundary, not a leak that should be plumbed.**

Update the **Recurring patterns** table to remove ground:K1 from the
"declarative contracts that aren't enforced" row. The remaining
instances in that row (musical:K1, mood:K2) are still correct.

**K2.** The "stub contributor" pattern is structural and intentional but
fragile. `CONTRIB_PAINTINGS_BASES` and `CONTRIB_VEGETATION_BASES` exist as
registry slots whose WGSL implementations (`contrib_paintings_base_at`,
`contrib_vegetation_base_at`) return `0.0` literally. The DAG declares
their dependencies (PYRAMIDS → PAINTINGS_BASES; SOLIDS → PAINTINGS_BASES;
SOLIDS → VEGETATION_BASES) so policies including them are forced to also
include their ancestors.

**Reframe under P8 (latent infrastructure).** This is the GPU-side
canonical example of P8 — the seam map's Ch. 9–10 elaboration of the
pattern was named *after* the GPU side had been doing this. The WGSL
file even self-documents the intent at line 2244:
> *"reserved for a future flat-bases-under-paintings contributor; the
> registry slot exists so policies can declare intent now."*

The pattern lets policies declare intent before implementation lands. When
those contributors gain real implementations:

- The WGSL `contrib_*_at` body changes; nothing else moves.
- Policies that *should* now include them aren't auto-updated — both
  POLICIES and POLICY_*_MASK and query_ground_* bodies need a coordinated
  edit.
- The DAG already encodes the dependency, so forgetting to add the
  contributor to a downstream policy *and* its DAG-required ancestor
  yields a compile error; forgetting *only* to add the new contributor to
  a relevant policy is silent (the DAG only fires when ancestors are
  missing, not when descendants are).

Not a bug, a watch item. Worth a note when implementations land.

## Open decisions

**D1.** `gradient_supported` flag — delete or make consumed?
*Inclining toward consumed:* the field is intent-bearing (some policies
genuinely cannot support gradient evaluation, e.g. `placement_*` policies
which exclude the deformation fields whose derivatives the gradient would
need). Wiring a static_assert that gradient-using callers only read
gradient-supporting policies would catch a real class of bug. Decision
depends on whether that gradient-caller surface is small enough to enumerate.
**Decision deferred until the spine seam map (Ch. 15).**

**D2.** Code-generate the WGSL side from the C++ table?
This is the K1 "heavy" path. Argument for: eliminates the three-way drift
permanently, makes the C++ table an actual single source of truth instead
of "one of three." Argument against: WGSL `query_ground_*` function bodies
have hand-tuned structure (e.g. WALKER's inline suppression) that wouldn't
emerge from a naive `for c in mask: h += contrib_c_at(xz)` template. The
code-gen would need to support pattern overrides, which is a real piece of
engineering. Probably out of scope for the current refactor pass; worth
noting as a long-horizon architectural option.
**Decision deferred — likely a separate future project.**

## Proposed tags

```
File: cartridges/the_board/modules/ground_architecture.inl

Near line 70 (enum ContributorId : uint32_t {):
  // SEAM[ground:K1] C++ POLICIES + WGSL POLICY_*_MASK + WGSL query_ground_* — three-place agreement, no cross-validation

Near line 75 (CONTRIB_PAINTINGS_BASES = 4, // placeholder ...):
  // SEAM[ground:K2] stub contributor — DAG declares deps; policy inclusion is hand-coordinated when implementation lands

Near line 76 (CONTRIB_VEGETATION_BASES = 5, // placeholder ...):
  // SEAM[ground:K2] stub contributor — same pattern as PAINTINGS_BASES

Near line 132 (bool gradient_supported;):
  // SEAM[ground:L1] declared but never read — delete or wire a static_assert (see D1)
  // SEAM[ground:D1] keep with a runtime/compile-time consumer, or remove?
```

```
File: world.wgsl

Near line 2069 (const POLICY_PLACEMENT_PYRAMID_MASK ...):
  // SEAM[ground:L2] POLICY_*_MASK constants are declared but never read in WGSL — documentation-as-code

Near line 2545 (fn query_ground_placement_pyramid ...):
  // SEAM[ground:K1] hand-implemented bodies must match C++ POLICIES + WGSL POLICY_*_MASK; no auto-validation
```

---

# Chapter 4 — `entity_types.inl` + `entity_pipeline.inl`

One chapter. Two files, treated as a unit because the C++ union sandwich
constraint forces the split (Ch. 1): `entity_types.inl` (107 lines) declares
`EntityInstance` and the trait/adapter structs *before* the queue unions in
`spawn_engine.inl` define them as union members at line 2691. `entity_pipeline.inl`
(1969 lines) is the implementation, included *after* the unions at line 6227.

Together: the **generic entity machinery** — the table-driven adapter pattern
that handles 9 of 12 entity families (pyramid, arch, column, antenna, palm,
cactus, blade, sphere, cube). The 3 bespoke families (ribbon, gol, gallery)
do not use this machinery; they live in banner-only modules.

## Owns

### `entity_types.inl` — the vocabulary

- **Capacity constants** (12–13): `MAX_ENTITY_PARAMS = 32`, `MAX_COLOR_CHANNELS = 12`.
- **`ParamDist` enum** (15–19): `GAUSSIAN`, `UNIFORM_01`, `UNIFORM_TAU`.
- **`TierParamDef` struct** (21–27): per-parameter floor / ceiling / rounding /
  distribution. Shape of one parameter sampling rule.
- **`TierMuSigma`** (29–31), **`TierProfile`** (33–36): a tier is a weight plus
  an array of `(mean, sigma)` pairs.
- **`ColorPartDef`** (38–43): default color compute support — base RGB +
  variance + property index pair.
- **`EntityFamilyTraits`** (45–70): the per-family configuration record. 24
  fields covering identity (`family_id`, `short_name`), capacity,
  spawn/density gating, mood multipliers, the tier/param/color tables, and
  a `gpu_ground_y` flag.
- **`SpawnGateOutput`** (72–77): preamble result.
- **`EntityInstance`** (79–95): the work-in-progress entity during
  select / place / commit. **Required as a union member of `EntityQueueEntry`
  and `PlacementEntry`** in `spawn_engine.inl` — the constraint forcing the
  sandwich split.
- **`EntityFamilyAdapter`** (97–105): the function-pointer vtable. 7 slots:
  `run_gate`, `get_theme_tier_weights`, `compute_solid_half`, `compute_colors`,
  `write_active`, `write_gpu`, `post_commit`.

### `entity_pipeline.inl` — the implementation

- **`generic_compute_colors`** (17–29): the default color compute path.
  **Currently dead code** (see L3).
- **`apply_indoor_rescale`** (67–184): rescale entity dimensions to fit
  ceiling height in indoor moods. **Family-aware** (see K1).
- **`generic_select`** (195–266): spawn gate → tier selection (theme-biased)
  → parameter sampling → indoor rescale → derived values via adapter.
- **`generic_place`** (273–298): position negotiation (jitter, footprint,
  host patch resolution).
- **`generic_commit`** (304–321): adapter calls in order — `write_active`
  (CPU mirror), `write_gpu` (GPU mesh params), optional `post_commit`
  (piers, regen, portals).
- **9 family blocks**, each ~150–230 lines, each with the same shape:
  `XxxIdx` struct + `XXX_PARAM_DEFS[]` + `XXX_TIER_TABLE[]` + `XXX_COLOR_PARTS[]`
  + `XXX_TRAITS` constant + 5–7 adapter functions + `XXX_ADAPTER` constant +
  3 dispatch wrappers (`dispatch_select_xxx_generic`, etc.):

  | Family   | Lines       | post_commit? | Notes                              |
  |----------|-------------|--------------|------------------------------------|
  | Blade    | 330–516     | nullptr      | Vegetation                         |
  | Palm     | 520–696     | nullptr      | Vegetation                         |
  | Cactus   | 700–857     | nullptr      | Vegetation                         |
  | Column   | 861–1093    | column_post  | Pillared — writes piers            |
  | Antenna  | 933–1246    | antenna_post | Pillared — writes piers            |
  | Pyramid  | 1250–1409   | pyramid_post | Pillared — writes piers, marks regen |
  | Sphere   | 1413–1538   | nullptr      | Floater (analytical PGA orbit)     |
  | Cube     | 1542–1706   | nullptr      | Floater (behavior registry)        |
  | Arch     | 1710–1942   | arch_post    | Pillared — piers, regen, portals   |

  `post_commit` cleanly splits **pillared** families (4) from
  **non-pillared** (5).

## Consumes

The list is long — full encapsulation isn't achieved. Adapters reach into
specific Cartridge members and into companion declarations that live
elsewhere (mostly banner-only `entities.inl`).

- **From banner-only `entities.inl`** (still inline in cartridge.hpp,
  Ch. 12):
  - `PopFamily` enum.
  - Per-family **legacy compact tier structs**: `ARCH_TIERS[]`, `PALM_TIERS[]`,
    `COLUMN_TIERS[]`, etc. — these carry `burial`, `segs_u/v`, `color_var`,
    `color_override` and other fields that the *adapter functions read* but
    are not in `TierProfile`.
  - Per-family **property-index registries**: `ArchProp`, `PalmProp`, etc.
  - Per-family **config structs**: `ArchConfig::SPAWN_CHANCE`, etc.
  - Per-family **palettes and constants**: `ARCH_PALETTE`, `ARCH_SANDSTONE_BASE`.
  - Some `XxxIdx` structs are defined here in entity_pipeline; some live in
    entities.inl. Mixed.
- **From `cartridge.hpp` core**:
  - `THEMES[]` table (population themes with per-family tier-weight arrays).
  - `MOOD_TABLE[].indoor` / `.ceiling_height` (read by `apply_indoor_rescale`
    and `generic_select`).
  - `Cartridge` members: `activeArches_[]`, `activeColumns_[]`, etc. (each
    family's CPU mirror); `archMeshGenPending_`, `palmMeshGenPending_`, etc.
    (per-family dirty flags); `gpuState_`, `find_patch`, `mark_patches_for_regen`,
    `write_pier`, `pick_portal_mood`, `derive_finite_radius`, `portalsDirty_`.
- **From `spawn_engine.inl`** (banner-only):
  - `run_spawn_preamble()` — the gate logic.
  - `negotiate_position()` — placement spatial negotiation.
  - `record_placement_bookkeeping()` — placement census.
  - `select_tier_biased()` — theme-aware tier selection.
  - `EntityQueueEntry`, `PlacementEntry` — union types whose members include
    `EntityInstance` (the sandwich constraint).
- **From `seed_utils.inl`** (banner-only): `cpu_hash_f`, `cpu_sample_gaussian`.
- **From `state.hpp`**: `Dim::MAX_*_INSTANCES`, `Dim::PIER_ARCH_BASE`,
  `GPUPalmMeshParams`, `GPUArchMeshParams`, etc.

## Exposes

- The struct definitions in `entity_types.inl` are referenced by the queue
  unions in `spawn_engine.inl` and the family adapter implementations here.
- The 9 `dispatch_*_generic` wrapper functions are referenced by the
  `FAMILY_DISPATCH[]` table at `cartridge.hpp:6234` — the hub from Ch. 1.
- Each family's `XXX_TRAITS` and `XXX_ADAPTER` constants are referenced
  internally by the dispatch wrappers; not exported to outside callers.

## Almost owns

- **Per-family tier representation.** Each family currently has *two* tier
  representations: `XXX_TIER_TABLE[]` here (Gaussian sampling parameters,
  shape `TierProfile`) and the legacy compact `XXX_TIERS[]` in banner-only
  `entities.inl` (carrying `burial`, `segs_u/v`, `color_var`, etc.). Adapter
  functions read both. The split is the partial-decomposition state — the
  pipeline owns the *sampling* tier shape; entities.inl still owns the
  *non-sampled* tier extras. Resolves when entities.inl extracts (Ch. 12);
  whether to merge representations or document the split is a Ch. 12 decision.

- **The default color compute path.** `generic_compute_colors` exists as a
  fallback when `adapter.compute_colors == nullptr`, and no current family
  uses the default — every family today has exotic enough color logic to
  override. **This is latent infrastructure, not dead code** (see L3 with
  Ch. 10 conventions reframing). The trait fields `color_part_count` +
  `color_parts` are the data shape for future-default-using families.

- **The `EntityFamilyAdapter` partial encapsulation.** Adapters look like a
  vtable (good), but each adapter function reaches into specific Cartridge
  members (`activePalms_[slot]`, `palmMeshGenPending_`, etc.) — the family
  isn't actually pluggable, just routed. This is inherent to the partial-
  generic approach (see K4). Naming it here so the pattern is visible
  before later chapters compound on it.

## Loose strands

**L1.** Stale migration TODO at lines 1948–1967. Lists ~208 lines of code in
`cartridge.hpp` / `entities.inl` to delete after blade migration validates.
**The migration completed**; the listed structs (`BladeClusterSelection`,
`BladeClusterPlacement`, `select_blade_for_patch`, `place_blade_from_selection`,
`commit_blade`, `dispatch_select_blade`, `dispatch_place_blade`,
`dispatch_commit_blade`) **have already been deleted** — verified via grep.
The TODO is documentation debt. Trivial to remove or rewrite as a historical
note.

**L2.** The "FAMILY_DISPATCH Integration" comment at lines 1942–1946
references `blade_integration_guide.md`. Same staleness — historical
artifact from the migration. Folds into L1.

**L3.** `generic_compute_colors` is **latent default path, not dead code**
(reframed under Ch. 10 conventions update). Function defined at line 17,
conditionally called at line 263 (`if (adapter.compute_colors) ... else
generic_compute_colors(...)`). Verified: every one of the 9 current family
adapters sets `compute_colors` to its own per-family function — the else
branch never fires *today*.

But the comment above the function names the design intent:
> *"Families with exotic color logic provide their own adapter fn."*

The default exists for future families with **non-exotic** color logic
— a family that just rolls base RGB ± variance without per-part
asymmetry, palette lookup, theme override, or palette-cycling, would
take this path. The trait fields `color_part_count` + `color_parts`
aren't dead either; they're the data shape that future-default-using
families would author.

The seam-map action is a tag, not a deletion: name the intent above the
function so a future reader doesn't mistake "no callers today" for
"delete me." The current 9 families all have exotic enough color logic
to override; future families may not.

**L4.** `write_active` boilerplate. Each family's `xxx_write_active` opens
with the same ~10 lines: setting `patch_gx`, `patch_gz`, `host_gx`, `host_gz`,
`active = true`, `draw_visible = true`, `world_x`, `world_z`, `tier_idx`,
`cached_ground_y`, then incrementing the per-family count. Repeated across
9 families = ~90 lines of duplicate. Hoistable to a helper.

**L5.** Dispatch wrapper templates. The 27 functions
`dispatch_{select,place,commit}_xxx_generic` follow near-identical
templates. Differences per family: `XXX_TRAITS`, `XXX_ADAPTER`, family enum,
active-array reference (for the dispatch_place on-fail cleanup). Could
collapse into a macro or template; currently hand-written 27 times.

## Knots

**K1.** `apply_indoor_rescale` is family-aware in a generically-named
function. Three `switch (inst.family_id)` blocks (eligibility / target
range, current-height calculation, length-param scaling), with Columns
getting an early-return special path. The function knows everything
family-specific; it's "generic" only in the sense that one symbol handles
all of them.

**Reframed under the artistic-specificity vocabulary (introduced in
Ch. 5):** the per-family rescale rules are *artistic intent*, not a leak.
Indoor moods change the relationship between entity scale and ceiling
height; Columns *should* snap to the ceiling exactly (architectural anchor),
Palms *should* roll tighter (canopy-defining), Pyramids/Arches/Antennas
*should* roll wider (variety). The specificity is correct.

The remaining seam-map question is whether the **encoding** is clean. The
current encoding mixes concerns: eligibility (in switch A), current-height
indexing (in switch B), length-param indexing (in switch C). Three
parallel switches with the same family list. Two encoding alternatives:

- *Trait approach:* `rescale_eligibility`, `rescale_target_lo / _hi`,
  `current_height_param_indices[]`, `length_param_indices[]` on
  `EntityFamilyTraits`. Each family's rescale rule is one entry in its
  trait declaration; the function becomes a table-driven loop with no
  switches. Columns still need their own snap-exact path (special case),
  but the others fold into the traits.
- *Adapter approach:* `compute_rescale_target(inst, ceiling_h) → float`,
  `apply_rescale_scale(inst, scale)` on `EntityFamilyAdapter`. Each family
  implements its own rescale; the function dispatches.

The current state is *fine for today* (only 4 eligible families, well-
commented), but the encoding doesn't scale: a fifth eligible family means
adding a row to three switches in lockstep. See D1.

**K2.** Tier representation is split across two homes (this file +
banner-only `entities.inl`). Each family's adapter reads from both. **This
is a cross-chapter knot — resolves with Ch. 12 (entities.inl extraction).**
Two resolution shapes:
- Merge `TierProfile` and the compact `XXX_TIERS` struct into one
  representation per family (tier extras become trait/profile fields).
- Document the split explicitly: profiles are *Gaussian-sampled parameters*,
  legacy tiers are *non-sampled tier extras*. Different roles, both legitimate.

**K3.** Arch `SPAN` parameter semantic mutation. `arch_compute_solid_half`
overwrites `inst.params[ArchIdx::SPAN]` to hold *half_span* (line 1764) so
that subsequent adapter calls (`arch_write_active`, `arch_write_gpu`,
`arch_post_commit`) can read the half value directly. Hidden contract:
SPAN's stored meaning changes mid-pipeline. Anyone reading the code
top-down might miss it. Not a bug — the comment flags it — but fragile.
Could be resolved by storing `half_span` in a separate field or by computing
half from full at each read site (small cost, clearer semantics).

**K4.** Adapter functions reach into specific Cartridge member fields. Each
family has a hard-coded path: `c->activePalms_[slot]`, `c->gpuState_.upload_palm_mesh_params_slot`,
`c->palmMeshGenPending_`. The adapter pattern *almost* makes families
pluggable but stops at routing — actual integration is per-field. This is
inherent to the partial-generic approach. **Probably fine** — full
encapsulation would require massive interface changes (every active mirror
becoming a virtual interface, every per-family GPU upload becoming a
generic hook). Documenting as a structural fact, not a fixable seam.

## Open decisions

**D1.** `apply_indoor_rescale` — refactor into traits/adapter, or accept
as-is? *Inclining accept-as-is:* the function is contained, well-commented,
and the family-specific logic genuinely differs (Columns snap exactly).
Pushing into the adapter creates ceremony for a feature that fires only in
indoor moods. **Decision deferred until Ch. 12** (when entities.inl
extraction may make the trait-enrichment path cheaper).

**D2.** ~~`generic_compute_colors` — delete or keep?~~ — **reframed as
latent default path (per Ch. 10 conventions update).** Action is a tag
naming the design intent (default for future families with non-exotic
color logic). Keeps the function for the future, prevents misreading
as dead.

**D3.** Dispatch wrapper macro/template? The 27 wrapper functions are
near-identical. Macroizing buys ~180 lines but adds one indirection layer.
Templating buys cleaner names but requires non-trivial template machinery
(the wrappers reference per-family active arrays). **Inclining no:** the
wrappers are mechanical but readable as-is, and the cost of the macro
indirection (especially for debugging) probably outweighs the dedup. Worth
re-examining once a 10th generic-pipeline family arrives. **Decision
deferred — likely "no" at end-of-tour unless a strong reason emerges.**

**D4.** Cross-references with Ch. 12 (entities.inl). Several K- and
L-strands here defer to Ch. 12 outcomes. The chapter ordering may need to
flip — Ch. 12 first, then a touch-up pass on this chapter — depending on
what's revealed. **Re-evaluate after Ch. 12 is drafted.**

## Proposed tags

Selective — the file is large; tags only at the most consequential sites.

```
File: cartridges/the_board/modules/entity_types.inl

Near line 79 (struct EntityInstance {):
  // SEAM[entity:K2] tier extras (burial, segs, color_var) live in
  //   banner-only entities.inl — adapter reads from both — resolves with Ch. 12

Near line 97 (struct EntityFamilyAdapter {):
  // SEAM[entity:K4] adapter shape implies pluggability but functions reach
  //   into concrete Cartridge fields — partial-generic by design
```

```
File: cartridges/the_board/modules/entity_pipeline.inl

Near line 17 (static void generic_compute_colors ...):
  // SEAM[entity:L3] latent, not dead — default path for future families
  //   with non-exotic color logic; current 9 families all override.
  // SEAM[entity:D2] tag with intent (per Ch. 10 conventions)

Near line 67 (void apply_indoor_rescale ...):
  // SEAM[entity:K1] family-aware "generic" function — three switches on
  //   family_id; candidate for trait/adapter migration
  // SEAM[entity:D1] migrate to trait/adapter, or accept?

Near line 311 (adapter.write_active(this, inst);):
  // SEAM[entity:L4] write_active boilerplate ~10 lines × 9 families —
  //   hoistable to generic helper

Near line 1764 (inst.params[ArchIdx::SPAN] = half_span; // overwrite):
  // SEAM[entity:K3] hidden semantic mutation — SPAN now holds half_span
  //   for subsequent adapter calls

Near line 1942 (// ─── FAMILY_DISPATCH Integration ─────):
  // SEAM[entity:L1] migration completed — referenced structs/functions
  //   already deleted from cartridge.hpp; this comment block is stale

Near line 1948 (// ─── What to delete after blade migration is validated ───):
  // SEAM[entity:L1] stale TODO — verified deletion via grep
```

---

# Chapter 5 — `mood.inl`

The integration point. 1067 lines covering atmosphere, indoor lighting, shell
geometry, portal placement. `apply_mood` — the orchestrator at the center —
touches more subsystems than any other function in the cartridge. Every
mood transition routes through here.

This chapter introduces a vocabulary distinction that the framing in earlier
chapters lacked: **deliberately mode-specific rules** are not seams. Indoor
moods need indoor-specific rules; outdoor finite moods will need their own;
forcing them through a generic shape is hostile to the artistic intent. The
seam map's job for this chapter is to distinguish *artistic specificity*
(named, accommodated) from *structural duplication* (one rule expressed in
multiple places).

## Owns

- **`apply_mood(mood, queue)`** (231–457). The orchestrator. Called once per
  mood transition from `update()`'s TEARDOWN phase. Twelve distinct concerns
  in linear order: feature gates, sun/atmosphere, fog, terrain amplitude
  ceiling, lights dirty flag, indoor light derivation, indoor shell with
  per-seed palette, camera ceiling clamp, band-motion reset, mode-intensity
  reset, mood-5 ribbon spawn, orb configuration, log line.
- **`derive_indoor_lights`** (17–229). Generates the per-mood-per-seed spot
  light array. Reads `LIGHT_SCHEMES`, picks scheme + wall pair, derives each
  light's position / direction / cone / color via Gaussian sampling. Adds a
  vault uplight when ceiling is VAULT and a slot is free.
- **`generate_indoor_shell` / `clear_indoor_shell` / `push_quad`** (467–763).
  Wall + ceiling geometry. 4 wall quads (deep floor to wall_top). Ceiling:
  flat = 1 quad; vault = tessellated catenary (groin vault).
- **`force_spawn_back_portal`** (765–905). Places the return-portal in
  finite worlds. Seed-shuffled wall selection, footprint-aware wall margin,
  origin-distance constraint, fallback for radius-1 worlds. Chains into
  `force_spawn_finite_portals`.
- **`force_spawn_finite_portals`** (918–1014). Places 1–3 forward portals
  along the perimeter (count scales with `finiteRadius_`).
- **`upload_portal_array`** (1017–1040). Per-frame: copies portal arches
  from `activeArches_` into the GPU portal-detection array.
- **`upload_lights`** (1043–1066). Per-frame: uploads sun directional light
  + spot lights.

State owned: `cpuSpotLights_`, `spotLightActive_`, `lightsDirty_`,
`portalsDirty_`, `cpuPortalArray_`, `backPortalPending_`,
`backPortalPosition_`, `backPortalReturnSeed_/Mood_/Radius_`.

## Consumes

A long list — mood is downstream of everything.

- **From `cartridge.hpp` core:** `MOOD_TABLE`, `MOOD_COUNT`, `mood_name`,
  `INDOOR_PALETTES`, `INDOOR_PALETTE_COUNT`, `LIGHT_SCHEMES`,
  `IndoorLightProp`, `INDOOR_ENTITY_WALL_MARGIN`, `MAX_GPU_PORTALS`,
  `Coupling::PAWN_TO_SUN_VP`.
- **From banner-only `entities.inl`:** `ARCH_TIERS` (read for
  footprint-aware margin), `RibbonProp`, `RIBBON_BASE_TIER_WEIGHTS`,
  `RIBBON_TIER_COUNT`, `RibbonSelection`, `RibbonPlacement`.
- **From banner-only `spawn_engine.inl`:** `select_tier_biased`,
  `fill_ribbon_selection_geometry`, `commit_ribbon`,
  `force_spawn_portal_at`, `pick_portal_mood`, `derive_finite_radius`,
  `estimate_terrain_height`.
- **From `seed_utils`:** `cpu_hash`, `cpu_hash_f`, `cpu_sample_gaussian`,
  `tile_seed`.
- **From `musical.inl` state:** `bandMotionActive_`, `bandBlend_`,
  `bandPhaseOrigin_`, `bandBlendTarget_`, `mmodeIntensity_[]`,
  `paletteDriftTarget_`, `paletteDriftDesired_`, `pulseRing_`,
  `pulseWriteIdx_`, `prevPolyphony_`. **Mood transition resets all of
  these** (see K3).
- **From other modules:** `auraEnabled_` (`pawn_aura.inl`),
  `configure_orbs` + `ORB_MOOD_TABLE` (`orbs.inl`).
- **From Cartridge state:** `activeMood_`, `activeSeed_`, `finiteMode_`,
  `finiteRadius_`, `moodRibbonOffset_`, `activeArches_`,
  `moodAllowsMusicalModes_`, `moodAllowsGoLZones_`, `gpuState_`,
  `renderer_.set_frustum_cull_active`, `compute_spot_light_vp`.

## Exposes

- **`apply_mood`** — called from `update()` during TEARDOWN.
- **`force_spawn_back_portal`** — called from `stream_patches()` FULLREGEN
  bootstrap. Internally chains `force_spawn_finite_portals`.
- **`upload_portal_array`** + **`upload_lights`** — called each frame from
  `render()`'s pre-pass.

`derive_indoor_lights`, `generate_indoor_shell`, `clear_indoor_shell` are
exposed but only called from `apply_mood` within this file. Effectively
private.

## Almost owns

- **Per-mood-transition reset of musical-coupling state.** Lines 351–388 of
  `apply_mood` reset `bandBlend_`, `bandPhaseOrigin_`, `mmodeIntensity_[]`,
  `paletteDriftTarget_/Desired_`, `pulseRing_`, `pulseWriteIdx_`,
  `prevPolyphony_`. This is the **mirror of musical:K2** along a different
  temporal axis: musical.inl almost-owns the per-frame update (lives in
  `update()`); mood.inl almost-owns the per-mood-transition reset (lives
  here). Same leak, different time slice. Resolves when musical.inl gains
  `tick_musical_couplings()` AND `reset_musical_couplings()` together.

- **The integration shape.** `apply_mood` is one function, ~250 lines, doing
  a dozen distinct concerns in linear order. Each concern is small (5–40
  lines). The file *contains* the integration but doesn't *structure* it —
  there's no `apply_mood_atmosphere()` / `apply_mood_lighting()` /
  `apply_mood_geometry()` decomposition. The function is correct in scope
  (mood is the integration point) but unstructured in shape.

## Loose strands

**L1.** Mood-5 ribbon is gated by `if (mood == 5)` (line 390) — a magic
number, not a profile flag. The MoodProfile carries feature flags
(`allow_musical_modes`, etc.) but no `has_anchor_ribbon`. With
finite_outdoor and future moods needing similar one-offs, the magic-number
pattern will recur. Adding a flag to MoodProfile generalizes the gate. See
K1 for the broader pattern this is part of.

**L2.** Wall-margin computation duplicated. Lines 793–803 in
`force_spawn_back_portal` and lines 929–940 in `force_spawn_finite_portals`
are identical: indoor → footprint-aware (read `ARCH_TIERS[DOORWAY]`,
compute `doorway_half_span + doorway_pier_half + INDOOR_ENTITY_WALL_MARGIN`);
outdoor finite → `8.0f`. Should be a helper:
`compute_perimeter_wall_margin()` returning a float.

**L3.** Candidate-spot construction + shuffle pattern duplicated.
`force_spawn_back_portal` (lines 809–822) and `force_spawn_finite_portals`
(lines 947–970) both build a 4-element `Spot[]` for the four walls and run
Fisher-Yates with a seed-derived order. Same structure, different seeds and
slightly different per-spot fields. Hoistable to a helper that returns
either an iterator or a shuffled array.

**L4.** Sun direction is normalized twice — once in `apply_mood` (lines
260–266 → uploaded to GPU config), once in `upload_lights` (lines 1048–1051
→ uploaded to directional light buffer). Two normalizations of the same
input. Either compute once and cache, or pick the canonical site.

## Knots

**K1.** The **indoor / outdoor binary doesn't survive contact with
finite_outdoor**. The file branches on `m.indoor` in many places:

  - `set_terrain_amp_ceiling(m.indoor ? 0.5f : 0.0f)` (line 280).
  - Indoor lights vs spot light deactivation (286–309).
  - Indoor shell vs clear (315–329).
  - Camera ceiling clamp (332–349).
  - Wall margin computation (793–803, 929–940).

  Today: `if indoor { A } else { B or fallback }`. With `finite_outdoor`
  needing its own treatment (Jean's note: "we'll do similar things to the
  finite outdoor mood as well in the near future"), each of these branches
  becomes either a three-way cascade or a properties-driven decision.

  **The right framing is artistic, not structural.** Indoor moods *should*
  have specific rules. So should finite_outdoor. So should any future mode.
  The seam-map question is *how those specifics are named*. Two paths:

  - *Properties path:* extend MoodProfile with per-rule feature flags.
    Today it has 4 (`allow_*`); could add `has_indoor_shell`,
    `has_ceiling_clamp`, `uses_footprint_wall_margin`, `has_anchor_ribbon`,
    etc. Each `if (m.indoor)` becomes `if (m.has_<thing>)`. The MoodProfile
    grows; the branches stop checking a meta-property (indoor) and start
    checking the actual property they care about.

  - *Mode-named path:* keep `m.indoor` as today, accept that adding
    finite_outdoor means a parallel `m.finite_outdoor` boolean and
    branches expand to three cases. Lower upfront cost, higher per-mode
    cost as moods proliferate.

  **The properties path is the natural extension of what MoodProfile
  already does.** The 4 existing flags (`allow_*`) are exactly this
  pattern. Extending them is additive. See D1.

**K2.** `apply_mood` is one 250-line function doing twelve concerns. The
function is correct in scope (mood is the integration point) but
unstructured in shape. Splitting into named sub-functions
(`apply_mood_atmosphere(m, queue)`, `apply_mood_indoor_geometry(m, queue)`,
`reset_musical_couplings_on_transition()`, etc.) clarifies the temporal
sequence without changing semantics. **Pure structural** — no behavioral
risk, no cross-module dependency. Loose-strand-shaped if not for size; size
makes it a knot.

**K3.** Per-mood-transition musical reset lives in `apply_mood` (lines
351–388) but should live in `musical.inl`. **Mirror of musical:K2.** Both
resolve together: when `musical.inl` gains `tick_musical_couplings()` for
the per-frame side, it also gains `reset_musical_couplings()` for the
per-transition side. `apply_mood` calls the reset; the state stays inside
musical.inl.

**Target shape inherited from musical:K2 (per world.wgsl audit).** The
reset function takes the per-coupling `Trajectory` state and zeroes the
value (or sets to a per-coupling rest goal). With the CPU-side
`Trajectory` primitive in place, reset becomes mechanical:
each coupling owns a `Trajectory` field, and `reset_musical_couplings()`
walks them.

**K4.** The mood-5 ribbon block (lines 389–447) is *bespoke twice*:
bespoke pipeline (ribbon doesn't go through FAMILY_DISPATCH's generic
machinery; uses `commit_ribbon` directly), and *bespoke trigger*
(magic-number mood ID rather than profile flag). L1 addresses the trigger
half. The pipeline half is a Ch. 13 question (specialized entity families).
Both halves are artistic intent — mood 5 is the reference clone with its
own anchor ribbon — but the encoding mixes intent with mechanism.

## Open decisions

**D1.** Extend MoodProfile with per-rule feature flags? *Inclining yes:*
this is the natural continuation of the existing `allow_*` pattern, it
generalizes the indoor/outdoor binary cleanly to handle finite_outdoor and
beyond, and each `m.indoor` branch becomes a more precisely-named flag
check. Cost: MoodProfile grows; MOOD_TABLE rows get longer. Counter-cost:
`m.indoor` and `m.ceiling_type` already encode some of this; adding flags
parallel to those creates redundancy unless we're careful about which
branches drop `m.indoor` and which retain it. **Decision deferred until
finite_outdoor design lands** — at that point the cost-benefit is concrete.

**D2.** Split `apply_mood` into named sub-functions? *Inclining yes:*
purely structural, no semantic change, immediate readability win, and the
sub-functions become natural homes for K3 (the musical-reset call) and for
future per-mood specifics. Cost: zero behavioral, modest churn.
**Promotable to free-strand pull at end-of-tour or as a small surgical
pass with Claude Code.**

**D3.** Resolve K3 jointly with musical:K2 — confirmed this maps cleanly,
no decision needed beyond timing. When musical.inl owns its per-frame
couplings, also give it `reset_musical_couplings()`. **No standalone
decision; tracked under musical:K2.**

## Proposed tags

```
File: cartridges/the_board/modules/mood.inl

Near line 231 (void apply_mood(uint32_t mood, wgpu::Queue& queue)):
  // SEAM[mood:K2] 250-line integration function — split into named sub-functions
  // SEAM[mood:D2] when (Ch. 15 / end-of-tour / Claude Code surgical pass)?

Near line 280 (gpuState_.set_terrain_amp_ceiling(m.indoor ? 0.5f : 0.0f)):
  // SEAM[mood:K1] indoor/outdoor binary — finite_outdoor needs a third case

Near line 351 (// Polyphony-driven band motion: active when mode toggled on):
  // SEAM[mood:K3] musical state reset — should live in musical.inl as
  //   reset_musical_couplings(), called from here. Resolves with musical:K2.

Near line 390 (if (mood == 5) {):
  // SEAM[mood:L1] magic-number trigger — should be MoodProfile.has_anchor_ribbon
  // SEAM[mood:K4] bespoke trigger AND bespoke pipeline (ribbon)

Near line 793 (if (MOOD_TABLE[activeMood_].indoor) { ... WALL_MARGIN = ...):
  // SEAM[mood:L2] wall-margin computation duplicated at line 929 — extract helper

Near line 809 (Spot candidates[4] = { ... }):
  // SEAM[mood:L3] candidate-spot + shuffle pattern duplicated at line 947 — hoistable

Near line 929 (if (MOOD_TABLE[activeMood_].indoor) { ... margin = ...):
  // SEAM[mood:L2] duplicate of line 793 wall-margin computation

Near line 1048 (float len = std::sqrt(sunDirection_[0] * ...)):
  // SEAM[mood:L4] sun direction normalized twice (apply_mood + upload_lights) —
  //   pick canonical site
```

---

# Chapter 6 — `pawn_aura.inl` (and the case for `pawn.inl`)

60 lines. Smallest module in the cartridge. **Pure declarations** — zero
function definitions, zero behavior, only structs / constants / member
variables. Every behavior that touches its state lives in another file.

This chapter starts with the standard seam map but turns into something
larger: the case for **promoting `pawn_aura.inl` into `pawn.inl`** — a
proper home for everything pawn-related currently scattered across
cartridge.hpp, input.inl, mood.inl, and agents.inl. That promotion is not
a new idea. The codebase already documents it: `PlayerState` (cartridge.hpp
line 87) explicitly carries the deferred-migration list as an inline
comment.

## Owns

- **`PawnAuraDeltaMode`** struct (17–20): two color-differential modes —
  `CONVERGENT` (all cells shift toward signature tint) vs. `RANDOM` (each
  cell gets unique random delta).
- **`PawnAuraProfile`** struct (22–33): the parameter record — radius,
  attack stiffness, attack damping, release rate, tint strength + RGB,
  delta mode, delta magnitude, effect mask (color/height bitfield),
  height scale.
- **`PAWN_AURA_DEFAULT`** constexpr (35–46): the canonical profile —
  20-unit radius, purple tint (`0.4, 0.2, 0.5`), CONVERGENT mode, both
  effects enabled, 3.0 height extrusion.
- **State variables** (49–58):
  - `activeAuraProfile_` — currently mutable, initialized to
    PAWN_AURA_DEFAULT. Comment says "can be swapped by landmarks/commands"
    (aspirational — see L1).
  - `auraHeightEnabled_`, `auraEnabled_`, `auraNeedsClear_`,
    `auraCfgDirty_` — toggle and dirty flags.
  - `auraPresence_` — current ramp value in `[0, 1]`.
  - `AURA_PRESENCE_ATTACK = 1.0f`, `AURA_PRESENCE_RELEASE = 1.5f` — ramp
    rates.

## Consumes

Nothing. The file is pure declarations.

## Exposes

Every declaration is read or written from outside this file:

- `activeAuraProfile_` → read in `update()` (height calc) and `render()`
  (full GPU config upload).
- `auraEnabled_` → toggled in `input.inl` (KEY_3), gated by
  `mood.inl::apply_mood` (`!m.allow_pawn_aura → false`), driving the
  presence ramp in `update()`.
- `auraHeightEnabled_` → toggled in `input.inl` (KEY_2), read in
  `update()`'s `effective_aura_height` computation.
- `auraCfgDirty_` → set in many places (any mutation), cleared once in
  `render()` after the full config upload.
- `auraPresence_` → ramped in `update()`, gates compute dispatch in
  `render()` (`if (auraPresence_ > 0.0f || auraNeedsClear_)`).
- `auraNeedsClear_` → cleared in `update()` upon mood transitions
  (line 7435), checked in `render()` to decide whether to keep the compute
  alive for cleanup.
- `PAWN_AURA_DEFAULT`, `AURA_PRESENCE_ATTACK / _RELEASE` — used at init
  and in `update()` ramp logic.

## Almost owns

**The most extreme case in the seam map so far.** This module declares
state and owns *no* functions. Every behavior that uses any of its
declarations lives elsewhere:

- **Presence trajectory ramp** — `cartridge.hpp::update()` lines 7924–7933.
  Mirrors the `MMODE_*` exponential-ramp pattern (Ch. 2). **Per
  world.wgsl audit, this ramp wants the GPU-side `Trajectory` shape**
  — the same target shape that closes musical:K2 closes this leak too.
  Once `pawn.inl` extracts, `auraPresence_` becomes a `Trajectory`
  field driven by a tick function inside the module.
- **Effective height computation** — `cartridge.hpp::update()` lines
  7937–7940. Multiplies `presence × base_height × (1 + AURA_EXPAND
  intensity × 3)` — couples to musical mode 5.
- **GPU config upload + compute dispatch** — `cartridge.hpp::render()`
  lines 8556–8605. Builds `GPUPawnAuraConfig`, applies aura-expansion
  scaling (`radius_scale`, `tint_scale`), uploads, dispatches the
  `compute_pawn_aura` kernel.
- **Toggle handlers** — `input.inl` lines 84–93 (KEY_2 height extrusion,
  KEY_3 field on/off).
- **Mood gate** — `mood.inl::apply_mood` line 250 (`if (!m.allow_pawn_aura)
  auraEnabled_ = false`).
- **Aura-expand intensity coupling** — `musical.inl` declares
  `MMODE_AURA_EXPAND`; `update()` reads `mmodeIntensity_[AURA_EXPAND]`
  to drive radius/height/tint scaling (lines 7937, 8124, 8562–8564).
- **Init clear on mood transition** — `cartridge.hpp` line 7435
  (`auraNeedsClear_ = true; auraCfgDirty_ = true`).

The module is a control panel without buttons.

## Loose strands

**L1.** `activeAuraProfile_` is declared mutable with the comment "Active
profile — starts as default, can be swapped by landmarks/commands." Verify:
**no code in the project swaps it.** Grep for `activeAuraProfile_ =` shows
only the declaration. Either the swap-mechanism is genuinely planned
(keep mutable, document as aspirational) or it never landed and the field
should be `const PawnAuraProfile&` pointing at PAWN_AURA_DEFAULT. Cheap
either way.

## Knots

**K1. The module is too narrow.** Pawn-related state and behavior is
scattered across at least five files. The natural unifier is a `pawn.inl`
that absorbs `pawn_aura.inl` and pulls the scattered concerns into one
home.

This is **not** a new architectural idea — the codebase documents it. From
`cartridge.hpp` line 87 (`PlayerState` declaration):

> *"Pass 1 only fills possessed_slot; aura/mmodes still live in their
> respective modules and are folded in by later passes."*

And the deferred-future block underneath:

```
// Future (deferred):
//   uint32_t active_couplings;         // COUPLING_* bitmask owned by player
//   float    aura_presence;            // migrated from auraPresence_
//   float    mmode_intensities[MMODE_COUNT];  // migrated from mmodeIntensity_
```

Per Jean's opinion in this chat, the natural module is `pawn.inl` —
"this feature and every other involved with the pawn should be in a
pawn.inl." That matches the comment exactly.

The next subsection inventories what would join.

### Inventory: pawn concerns scattered across the cartridge

| Concern                                   | Currently lives in                  | Lines (approx) | Move to `pawn.inl`? |
|-------------------------------------------|-------------------------------------|----------------|---------------------|
| Aura profile + flags + presence value     | `pawn_aura.inl`                     | 60             | **Yes** — naturally fits |
| Aura presence ramp                        | `cartridge.hpp::update()`           | 7924–7933      | **Yes**             |
| Effective aura height calculation         | `cartridge.hpp::update()`           | 7937–7940      | **Yes**             |
| Aura GPU config upload + compute dispatch | `cartridge.hpp::render()`           | 8556–8605      | **Yes**             |
| Aura toggle handlers (KEY_2, KEY_3)       | `input.inl`                         | 84–93          | Body moves; switch case stays |
| Aura needs-clear hook                     | `cartridge.hpp` (mood transition)   | 7435           | **Yes**             |
| `player_` struct (`possessed_slot`)       | `cartridge.hpp` class member         | 87–94          | **Yes**             |
| Pawn readback state machine declaration   | `cartridge.hpp` class member         | 522–541        | **Yes**             |
| Pawn readback dispatch + callback         | `cartridge.hpp::render()`           | 8272–8313      | **Yes**             |
| Portal-trigger receiver                   | `cartridge.hpp::render()`           | 8380–8394      | **Yes** — pawn-specific |
| FPV mode flag + toggle                    | `cartridge.hpp` (`fpvMode_`) + `input.inl::toggle_fpv_mode` | 60, 318–322 | **Yes** |
| Possession transfer (`try_possess_nearest`) | `agents.inl`                       | 719–780        | **Borderline** — see D2 |
| `cpuAgents_[possessed_slot]` reads        | `cartridge.hpp` (many sites)        | 7992, 8303, 8418, ribbon picks, etc. | accessor in `pawn.inl` |

**Rough size of the unified module:** 300–500 lines (depending on how much
boilerplate compresses), comparable to `agents.inl` (956). All currently
live; none would change semantics.

**What stays elsewhere:**
- The mood gate on aura (`mood.inl::apply_mood` line 250) — *mood drives
  the gate*; the gated state is pawn's, but the gate decision is mood's.
- `MMODE_AURA_EXPAND` declaration (`musical.inl`) — coupling lives in
  musical; `pawn.inl` reads the intensity value.
- `GPUPawnAuraConfig` struct + buffer (`state.hpp`) — the GPU contract.
- `compute_pawn_aura` kernel (`world.wgsl`) — the GPU implementation.
- Pawn rendering (`world.wgsl` + `renderer.hpp` PAWN_VS) — GPU geometry.

The principle: `pawn.inl` owns the *CPU-side player relationship to the
world*. It does not own the GPU contract or the agent-system machinery —
those have their own homes.

## Open decisions

**D1.** When to extract `pawn.inl`? *Inclining: as a coordinated pass with
musical:K2 + mood:K3* (the per-frame and per-transition couplings). Those
three knots are entangled — `auraPresence_` ramp, `mmodeIntensity_[]` ramp,
and `bandBlend_[]` ramp share an exponential-trajectory pattern; they
co-occupy `update()`. Splitting them into three home modules at once
clarifies all three. Alternatively, do `pawn.inl` first as a smaller
proof-of-extraction pass, then handle musical/mood separately. **Decision
deferred until end-of-tour** when the cumulative picture allows the
sequencing call.

**D2.** Should `try_possess_nearest` (and the possession-transfer logic
generally) move from `agents.inl` to `pawn.inl`? It touches both — it
walks the agent registry to find a target, AND it rewrites
`player_.possessed_slot` and `gpuState_.set_possessed_slot()`. The
function leans agent-registry-side (it's about *traversal of the agent
system*, with the pawn's slot id as the pivot). My instinct: **leave in
`agents.inl`** as a public method called from `pawn.inl` (or directly from
`input.inl`'s Caps Lock handler). Agents owns the registry; pawn reads its
own slot from agents. **Decision deferred — re-evaluate at the
extraction.**

**D3.** Should the `PlayerState` struct grow per its deferred-comment list
(absorb `aura_presence`, `mmode_intensities[]`, `active_couplings`)? Or
should `pawn.inl` keep flat module-scoped state like the current pawn_aura
declarations? *Inclining: structify.* The `PlayerState` shape is the
"travels with the player on possession transfer" record — exactly the
right scope for these fields. Promotion to struct fields makes possession
transfer mechanically clean (copy a `PlayerState`) and matches the
comment-documented intent. **Decision deferred until extraction
mechanics are concrete.**

## Proposed tags

```
File: cartridges/the_board/modules/pawn_aura.inl

Near line 1 (// ─── pawn_aura.inl ──────────────):
  // SEAM[pawn:K1] this module is too narrow — promote to pawn.inl
  //   absorbing aura + presence + readback + possession + fpv mode

Near line 49 (PawnAuraProfile activeAuraProfile_ = PAWN_AURA_DEFAULT;):
  // SEAM[pawn:L1] mutable but never reassigned in project — verify intent
```

```
File: cartridges/the_board/cartridge.hpp

Near line 60 (bool fpvMode_ = false;):
  // SEAM[pawn:K1] candidate for pawn.inl — pawn-specific state in spine

Near line 87 (struct PlayerState {):
  // SEAM[pawn:K1] PlayerState comment already documents the migration
  // SEAM[pawn:D3] absorb aura_presence + mmode_intensities into struct?

Near line 522 (enum class PawnReadbackState):
  // SEAM[pawn:K1] readback machine + state — candidate for pawn.inl

Near line 7924 (// Aura presence trajectory: smooth ramp on enable/disable):
  // SEAM[pawn:K1] presence ramp — should live in pawn.inl
  //   (mirrors musical:K2 pattern — ramp-in-spine leak)

Near line 8272 (// --- GPU agent buffer readback (one-frame latency) ---):
  // SEAM[pawn:K1] readback dispatch + callback — should live in pawn.inl

Near line 8380 (// Check if GPU reported a portal trigger):
  // SEAM[pawn:K1] portal trigger receiver — pawn-specific
```

---

# Chapter 7 — `agents.inl`

956 lines. The agent system: 32 fixed slots, 10 behaviors, 4 tiers,
per-mood populations. Slot 0 is the player's home; possession transfer
relocates the player slot id to other slots while in-mood; mood transitions
return the player to slot 0. Has its own GPU kernel (`update_player_agent`
+ `update_other_agents`); deliberately separate from `FAMILY_DISPATCH` per
the in-file rationale at lines 52–69.

This is **the cleanest implementation of the cockpit pattern in the
codebase.** Every section has a header banner; the TUNING CONSOLE is
explicit; registries are wide-formatted with column-header comments; the
shared population helper (`populate_agent_slot_`) is correctly
decomposed; possession/spawn/respawn lifecycle is split into named
sections. The chapter is light on knots because the module is doing it
right.

It also resolves Ch. 6 D2 (where does possession-transfer live).

## Owns

- **IDs and names** (83–142): `AgentBehaviorId` enum (10 values),
  `AgentTierId` enum (4 values), name arrays for diagnostics, plus
  `static_assert`s cross-checking C++ counts against `state.hpp`'s
  `GPU_AGENT_*_COUNT` constants.
- **Tuning console** (145–185): `PLAYER_SLOT = 0`, `POSSESSION_RADIUS`,
  `AGENT_EVICTION_RADIUS`, `AGENT_CENSUS_INTERVAL`. Every authored radius
  in one block.
- **Three registries:**
  - `AgentBehaviorDef` + `AGENT_BEHAVIORS[]` (213–240): per-behavior
    motion parameters. 10 rows, 7 fields each, formatted as a wide table.
  - `AgentTierDef` + `AGENT_TIER_GAINS[]` (252–272): per-tier gain
    multipliers + render color. 4 rows.
  - `AgentPopulationDef` + `AGENT_POPULATIONS[]` (299–404): per-mood
    population authoring — count, behavior weights, tier weights, spawn
    annulus, home seeding radius. One row per mood, each with explicit
    column-header comments and a per-row `static_assert` catching
    reorder.
- **Upload mechanism** (422–452): `upload_agent_registries_to_gpu` —
  one-shot init that translates CPU structs to GPU layouts and uploads
  to bindings 110/111.
- **CPU mirror** (470–471): `cpuAgents_[Dim::MAX_AGENTS]` (shadow of GPU
  agent buffer, refreshed via readback) and `agentRespawnCounters_[]`
  (per-slot respawn count for seed mixing).
- **Diagnostic overrides** (490–492): `agentBehaviorOverride_`,
  `agentTierOverride_`, `AGENT_OVERRIDE_NONE` sentinel.
- **Functions:**
  - `populate_agent_slot_` (private) — the shared helper for filling one
    slot from a population definition.
  - `spawn_population_for_mood` — mood-entry, full 32-slot refill.
  - `respawn_evicted_agents` — per-frame, per-slot refill of evicted slots.
  - `try_possess_nearest` — Caps Lock possession transfer.
  - `apply_agent_overrides_` (private), `cycle_agent_behavior_override`,
    `cycle_agent_tier_override`, `force_respawn_population` — F1/F2/F3
    diagnostic dials.
  - `dump_agent_census` — periodic + on-event log line, plus
    `lastAgentCensusDump_` cooldown timer.

## Consumes

- **From `cartridge.hpp` core:** `player_.possessed_slot`,
  `transitionPhase_`, `activeSeed_`, `currentSeconds_`, `MOOD_COUNT`
  + named mood IDs (`MOOD_OPEN_DEFAULT`, etc.).
- **From `state.hpp`:** `Dim::MAX_AGENTS`, `GPUAgentState`,
  `GPUAgentBehaviorDef`, `GPUAgentTierDef`,
  `GPU_AGENT_BEHAVIOR_COUNT`, `GPU_AGENT_TIER_COUNT`.
- **From `seed_utils.inl`:** `cpu_hash`, `cpu_hash_f`.
- **From Cartridge:** `gpuState_.upload_agent_registries`,
  `upload_agent_state_all`, `upload_agent_slot`, `set_possessed_slot`.
- **From WGSL (manual mirror):** `AGENT_EVICTION_RADIUS = 360.0f` —
  duplicated as `const AGENT_EVICTION_RADIUS: f32 = 360.0` in `world.wgsl`.
  See L2.

## Exposes

- **Boot path:** `upload_agent_registries_to_gpu` (called from
  `init_renderer`), `spawn_population_for_mood` (called from
  `init_renderer` for boot mood, and from `update()` TEARDOWN).
- **Per-frame:** `respawn_evicted_agents` (called from `render()`).
- **Player commands:** `try_possess_nearest` (called from `input.inl`
  Caps Lock); `cycle_agent_behavior_override` /
  `cycle_agent_tier_override` / `force_respawn_population` (F1/F2/F3
  from `input.inl`).
- **Logging:** `dump_agent_census` (called from `init_renderer` "boot",
  TEARDOWN "mood-transition", `render()` periodic).
- **`cpuAgents_[]` is effectively public storage:** read across the
  cartridge for distance-to-player computations (patch streaming, ribbon
  picks, distance culling, photographer position). Owned here, observed
  everywhere.

## Almost owns

Almost nothing — this is an unusually tight module. Three small surfaces:

- **`player_.possessed_slot`** is owned by `cartridge.hpp` (`PlayerState`
  struct, headed for `pawn.inl` per Ch. 6) but written by
  `try_possess_nearest` here. The bridge between agent registry and pawn
  identity. After `pawn.inl` extraction, this stays the same — `agents.inl`
  reads/writes the slot id, `pawn.inl` holds the storage. Clean split.

- **The pawn-position read pattern.** Many sites in the cartridge read
  `cpuAgents_[player_.possessed_slot].pos_x / pos_z` to get the player's
  current XZ. After `pawn.inl` extraction, those reads naturally become
  `pawn_x()` / `pawn_z()` accessors in `pawn.inl`. The data lives here;
  the *named accessor* moves to `pawn.inl`.

- **Tier preservation across mood transitions.** Lines 7992–8005 of
  `cartridge.hpp` (TEARDOWN) read `cpuAgents_[possessed_slot].tier_idx`,
  reset the buffer, then re-establish `cpuAgents_[0]` with the preserved
  tier. The "preserve player identity across moods" logic spans two
  files — written here as documented intent (header line 28: "Tier gains
  are authored for all four tiers"; possession comment line 711: "The new
  slot keeps its tier"), executed there. Splits naturally with `pawn.inl`:
  TEARDOWN block becomes `pawn.inl::reset_for_mood_transition()`,
  `agents.inl` keeps the spawn behavior of skipping slot 0.

## Loose strands

**L1.** The PLAYER_SLOT comment (lines 152–156) is stale:

> "Every spawn/respawn skips this slot; possession transfer rewrites the
> behavior_id of slot 0 vs the destination but never moves the player out
> of slot 0."

This is no longer accurate. `try_possess_nearest` (line 772) does
`player_.possessed_slot = new_slot` — the player *does* move out of slot 0
during in-mood possession. The comment dates from before possession
transfer landed. Correct framing: PLAYER_SLOT is the *home/return slot*
(where the player starts and where mood transitions reset to), not the
slot the player is always at. Trivial fix.

**L2.** `AGENT_EVICTION_RADIUS = 360.0f` is mirrored manually as a `const`
in `world.wgsl`. The comment is self-aware (lines 170–179):

> "MIRRORED MANUALLY in world.wgsl as `const AGENT_EVICTION_RADIUS: f32 =
> 360.0` (the WGSL needs a compile-time const for FXC inlining; no runtime
> upload exists). If you change this value, change the WGSL constant too —
> the compiler will not catch the drift."

**This is intentional specificity, cleanly named.** The FXC backend needs
compile-time constants for inlining; runtime config breaks the constraint.
The mirror is the price of FXC compatibility. Same family of concerns as
the SolidInstance 32-byte limit — hardware-constraint specificity, with
the constraint named at the site. The encoding is the best available
given FXC; the comment IS the enforcement. Worth tagging as a hardware-
mirror site (so future additions follow the pattern), but not a knot.

**L3.** `agentRespawnCounters_[Dim::MAX_AGENTS]` accumulates monotonically
across the session. Never reset, even on mood transition. Intentional
(more diversity for slots that respawn many times) but worth noting in a
comment near the declaration so a future reader doesn't reach for a reset.

## Knots

None. The module is the model.

The closest thing to a knot is the C++ class-body limitation explained in
the "Why no constexpr helper builders" comment (309–323): same restriction
that forced the macro workaround in `ground_architecture.inl`. The
workaround here is plain literal initialization with column-header
comments — perfectly fine. Worth recognizing as a **recurring constraint**
across the codebase: any module that uses class-body `constexpr` or
`static_assert` runs into "enclosing class isn't complete during parsing."
Not a knot in any single chapter; a documented language-level constraint
that this module navigates well.

## Open decisions

**D1.** *(Resolves Ch. 6 D2.)* Does `try_possess_nearest` move to
`pawn.inl`?

**Recommendation: stay in `agents.inl`.** Reading the function body in
full confirms it: 60 lines, of which 50 are *agent-registry traversal*
(walk all slots, check active, check non-player, find nearest within
radius, preserve tier on entry, reset velocity). The pawn-side write at
the end — `player_.possessed_slot = new_slot; gpuState_.set_possessed_slot()`
— is two lines. The function lives in agents.inl, called from input.inl,
mutates pawn state. Three layers, clear roles.

**Resolves: Ch. 6 D2 with "stay in agents.inl."**

**D2.** Should the cross-cartridge reads of `cpuAgents_[player_.possessed_slot]`
become accessors in `pawn.inl`? *Inclining yes:* every site that asks
"where is the player" should be a named call into `pawn.inl`, not a raw
index lookup into the agent mirror. The mirror stays here; `pawn.inl`
publishes `pawn_x()`, `pawn_z()`, `pawn_pos()`, `pawn_active_slot()`.
Mechanical refactor; semantics unchanged. **Decision deferred to
end-of-tour or `pawn.inl` extraction time.**

## Proposed tags

```
File: cartridges/the_board/modules/agents.inl

Near line 152 (// PLAYER_SLOT — slot 0 of agentStateBuffer_ ...):
  // SEAM[agents:L1] comment is stale — possession transfer DOES move
  //   player_.possessed_slot. PLAYER_SLOT is the home/return slot.

Near line 170 (// MIRRORED MANUALLY in world.wgsl as `const AGENT_EVICTION_RADIUS ...):
  // SEAM[agents:L2] hardware-mirror site — FXC constraint, comment is
  //   the enforcement. Same family as ground:L2 (declared but never
  //   read in WGSL). Distinct from ground:K1 — mirror here is intentional
  //   for FXC inlining, not accidental.

Near line 471 (uint32_t agentRespawnCounters_[Dim::MAX_AGENTS] = {};):
  // SEAM[agents:L3] never reset across session — intentional for
  //   respawn diversity; documented to deter future "fix the leak" PRs.

Near line 719 (void try_possess_nearest(wgpu::Queue& queue) {):
  // SEAM[agents:D1] resolves Ch. 6 pawn:D2 — stays here (registry-traversal
  //   dominant; pawn-side write is 2 lines)
```

```
File: cartridges/the_board/cartridge.hpp

Near multiple sites reading cpuAgents_[player_.possessed_slot]:
  // SEAM[agents:D2] candidate for pawn_x()/pawn_z()/pawn_pos() accessor in pawn.inl
  // (sites: 8303, 8418, ribbon picks ~3226, photographer ~4221, distance culling)
```

---

# Chapter 8 — `orbs.inl`

1128 lines. Largest module in the cartridge. **The cockpit pattern at
maximum complexity** — registries (five of them), mood config struct,
runtime state (sub-grouped by lifetime), GPU layout helpers, configure
helpers, lifecycle, player commands, per-frame couplings, GPU dispatches,
and a render call. The full sky orb subsystem, end-to-end, in one file.

This chapter is short on knots because the module is doing it right. The
findings are mostly positive — patterns to *name* so other chapters can
reference them, plus a couple of small loose strands. Where `agents.inl`
showed the pattern at moderate scale, `orbs.inl` shows the pattern at
maximum scale and it still holds.

## Owns

### Tuning console (44–100)

System-level dials. Dome geometry (`ORB_DOME_RADIUS = 450`,
`ORB_BASE_SIZE = 3`), four coupling families with attack/release rates
(force, color, flock, speed), `ORB_NOISE_FLOOR`, `ORB_SPEED_CEILING`,
and ten `ORB_DEFAULT_*` sanitization floors.

### Registries (102–356)

Five tables. Each has its own header banner, named-index constants, and
diagnostic name array.

| Registry         | Type                        | Count | Purpose                                          |
|------------------|-----------------------------|-------|--------------------------------------------------|
| `ORB_PALETTES`   | `OrbPalette` (4 HSV pockets) | 4     | Color authoring (jwst_deep, pillars, carina, warm_mono) |
| `ORB_TIERSETS`   | `OrbTierSet` (≤4 tiers)      | 2     | Population variety (jwst_stars, resonant)        |
| `ORB_FLOCK_GESTURES` | `OrbFlockGesture` (sep/align/coh signs) | 8 | Flocking sign combinations |
| `ORB_BROWNIAN_GESTURES` | `OrbBrownianGesture` (radial/vert/coh) | 6 | Brownian variations |
| `ORB_ORBITAL_GESTURES` | `OrbOrbitalGesture` (alignment/var) | 4 | Orbital variations |

### Mood config (359–405)

`OrbMoodConfig` struct — the per-mood authoring surface. Population,
color, motion, palette, color dynamics, anchor default, tierset,
flocking parameters, gesture seed, per-rule drag multipliers. Struct
defined here; rows instantiated in `cartridge.hpp`'s `ORB_MOOD_TABLE`
(banner-only).

### Runtime CPU state (408–467)

Sub-grouped by lifetime, an explicit design choice the file calls out:

> *"Player-owned state (anchor flag, gesture index) persists across mood
> transitions; mood-owned state (motion rule, active couplings,
> intensities) refreshes on configure."*

- **Lifecycle / kernel arming:** `orbsActive_`, `orbCount_`,
  `orbInitPending_`, `orbRecolorPending_`, `orbCurrentPaletteId_`.
- **Anchor (player-owned):** `orbPawnAnchored_`, `orbAnchorInitialized_`,
  `orbLastDomeCenterX_/Z_`, `orbDomeCenterInitialized_`.
- **Motion rule + gesture (mixed):** `orbCurrentMotionRule_` (mood),
  `orbGestureIdx_[4]` + `orbGestureInitialized_[4]` (player).
- **Couplings (mood-owned, per-frame smoothed):**
  `orbForceIntensity_`, `orbActiveNoiseAmp_`, `orbColorPulseIntensity_`,
  `orbColorConvergeIntensity_`, `orbColorSurgeIntensity_`,
  `orbColorPulseActive_/ConvergeActive_/SurgeActive_`,
  `orbFlockIntensity_`, `orbFlockActive_`, `orbSpeedMultCurrent_`.

### GPU layout helpers (470–487)

`orb_tier_block_ptr` and `orb_tier_flock_ptr` — `reinterpret_cast`
wrappers over `GPUOrbConfig` returning `float*` to specific tier blocks.
The right level of abstraction: layout offsets are explicit, callers
work with pointer arithmetic-free indexing. State.hpp owns the layout;
this file owns the access pattern.

### Configure helpers (490–695)

- `apply_mood_first_run_defaults_` — anchor flag and per-rule gesture
  indices seeded by mood once, then player wins.
- `pack_palette_` — pack ORB_PALETTE into GPU config.
- `pack_tiers_` — pack tierset (or zero for legacy uniform) with
  cumulative weights.
- `pack_flocking_` — pack flocking params, gesture signs (all three
  rules), per-rule drag.
- `log_configure_` — operator-readable summary line.

### Lifecycle (700–828)

- `configure_orbs(cfg, queue)` — sanitize zeros against
  `ORB_DEFAULT_*`, apply first-run defaults, build GPU config, upload,
  arm init kernel.
- `teardown_orbs()` — disable, reset mood-owned intensities, preserve
  player-owned state.

### Player commands (832–940)

- `cycle_orb_palette` (KP_0)
- `cycle_orb_motion_rule` (KP_8)
- `cycle_orb_gesture` (KP_DECIMAL — dispatches to brownian/orbital/flock
  based on current rule)
- `toggle_orb_anchor` (KP_9)

### Per-frame updates (945–1050)

- `update_orb_anchor(pawn_x, pawn_z, queue)` — dirty-flagged dome center.
- `update_orb_coupling(polyphony, dt, queue)` — three independent
  smoothed intensity channels (force/color/flock) plus speed multiplier;
  uploads only when a value moves.

### GPU dispatches (1053–1114)

- `dispatch_orb_init` (one-shot)
- `dispatch_orb_recolor` (palette cycle)
- `dispatch_orb_copy_prev` (per-frame snapshot for neighbor queries)
- `dispatch_orb_dynamics` (per-frame rule + couplings)

### Render (1119–1127)

`render_orbs(pass)` — additive billboard draw.

## Consumes

- **From `cartridge.hpp` core:** `activeSeed_`, `currentDt_`,
  `currentSeconds_`, `pawnReadback_x_/z_` (one log line only).
- **From banner-only:** `ORB_MOOD_TABLE` (per-mood config rows defined
  in cartridge.hpp; struct lives here).
- **From `state.hpp`:** `Dim::MAX_ORBS`, `GPUOrbConfig`.
- **From Cartridge:** `gpuState_.upload_orb_*` family, `orb_compute_group`,
  `orb_copy_group`, `orb_quad_vb/ib`, `render_entity_group`,
  `render_texture_group`; `renderer_.dispatch_orb_*`, `draw_orbs`.

## Exposes

| Function                       | Caller                                  | Line  |
|--------------------------------|-----------------------------------------|-------|
| `configure_orbs`               | boot init path                          | cartridge.hpp:7839 |
| `configure_orbs`               | `mood.inl::apply_mood`                  | mood.inl:450 |
| `teardown_orbs`                | TEARDOWN phase                          | cartridge.hpp:7439 |
| `update_orb_coupling`          | `update()` per-frame                    | cartridge.hpp:8243 |
| `update_orb_anchor`            | `update()` per-frame                    | cartridge.hpp:8247 |
| `dispatch_orb_*` (4 funcs)     | `render()` per-frame                    | cartridge.hpp:8610–8613 |
| `render_orbs`                  | `render_passes.inl` main pass           | render_passes.inl:596 |
| `cycle_orb_palette/motion_rule/gesture`, `toggle_orb_anchor` | `input.inl` numpad keys | — |

Lifecycle is fully wired: boot, mood transition, per-frame updates,
per-frame dispatch, draw. No site is missing.

## Almost owns

- **`ORB_MOOD_TABLE` rows** live in banner-only `entities.inl` in
  `cartridge.hpp`. The struct (`OrbMoodConfig`) is here; the per-mood
  authoring data is there. **Same partial-decomposition pattern as
  Ch. 4 K2** (tier extras split). Resolves with Ch. 12 (entities.inl
  extraction). When `entities.inl` extracts, `ORB_MOOD_TABLE` either
  joins it or moves here as `ORB_MOOD_TABLE` is genuinely orb-specific
  data; either is fine.

- **The polyphony coupling source.** `update_orb_coupling(polyphony,
  dt, queue)` takes `polyphony` as a parameter; the caller in
  `update()` reads it from `signal.stats[0]`. This module is one of
  the consumers Ch. 2 K2 names — it gets polyphony, doesn't care about
  the source. When the Ableton/musical-rewiring chat lands and the
  coupling source switches from polyphony to chord/BPM/pitch, this
  module's signature stays the same. The plumbing is already abstracted
  cleanly. **Confirms the musical:K2 design intent — couplings should
  be parameterized by signal value, not by source.**

## Loose strands

**L1.** `apply_mood_first_run_defaults_` reuses one mood-authored
`flock_gesture_default` for all three rules with per-rule count
clamping. The comment is honest:

> *"A future pass can split this into per-rule mood defaults if wanted."*

Not a bug; a documented simplification. Either keep as-is (simplicity
wins; one default per mood per gesture system is enough) or extend
`OrbMoodConfig` with `brownian_gesture_default`, `orbital_gesture_default`,
`flocking_gesture_default`. **Decision deferred — only act if a mood
specifically wants different defaults across rule types.**

**L2.** **Field-repurposing fragility in `pack_tiers_` / `pack_flocking_`.**
The comment at lines 552–554:

> *"Note: offsets 180-188 used to be `_pad_tier0/1/2` here. They now hold
> Brownian gesture fields (brownian_radial_sign/vert_bias/coherence),
> written by `pack_flocking_`. Do NOT zero them here."*

And at lines 558–559, line 603, line 611:

> *"Skip pf[3] for tier 0 — offset 428 is `orbital_speed_var_mult`
> (repurposed), written later by `pack_flocking_`."*

The two pack functions cooperate through repurposed `_pad` fields. The
comments are the enforcement: a future change to `pack_tiers_` that
"forgets" the carve-out will silently corrupt gesture data. **Same
family as agents:L2 (hardware mirror)** — documented constraint, comment
is the enforcement, intentional. Distinguishable from a knot only
because the constraint is *named at the site* and the cooperation is
*ordering-dependent and verifiable* (pack_tiers_ must run before
pack_flocking_; both run in `configure_orbs` in that order). Worth
tagging.

## Knots

None. The module is the model.

Same finding as Ch. 7 (agents.inl). orbs.inl is *more* self-contained —
it owns its own GPU dispatches and render call, where agents.inl
delegates dispatch to render_passes.inl/render(). When asked "what does
the cockpit pattern look like fully realized," `orbs.inl` is the answer.

## Patterns this chapter names

Three patterns worth surfacing for the cross-chapter index:

**P1 — "Per-frame coupling decomposed into the module."** The
ramp-in-spine leak (Ch. 2 K2, Ch. 6 K1) — exponential trajectories
living in `cartridge.hpp::update()` rather than their owning modules —
has a counter-example here. `update_orb_coupling` lives in `orbs.inl`,
takes `(polyphony, dt, queue)`, runs four independent smoothed channels
inside the module, uploads only when values move. The spine just calls
it. **This is what musical.inl::tick_musical_couplings() and
pawn.inl::tick_aura_presence() should look like.**

**P2 — "0 = no opinion, use system default."** The `ORB_DEFAULT_*`
floors + the `eff()` lambda + the `passthrough()` lambda. Mood authors
zero for fields it has no opinion on; configure_orbs substitutes a
named default. Each default is a constant in the tuning console. Clean
encoding for "every rule has working parameters regardless of mood
authorship." Likely useful in other modules that have similar mood-
authored configs.

**P3 — "Player state vs. mood state, explicit."** The runtime state is
sub-grouped by *who owns it*: anchor flag + gesture indices persist
across mood transitions (player); motion rule + couplings + intensities
refresh on configure (mood). The distinction is artistic — anchor
toggles and gesture cycles are the player's expression; motion rule is
the mood's character. The seam map's framing in Ch. 5 (artistic
specificity, named in code) shows up here as a runtime-state ownership
distinction. Worth naming because the same distinction will appear in
Ch. 9 (floaters) and Ch. 13 (specialized entity families).

## Open decisions

**D1.** Per-rule mood gesture defaults? See L1. Likely no unless a mood
specifically wants the differentiation.

**D2.** Fold `ORB_MOOD_TABLE` rows into `orbs.inl` (alongside the struct
they instantiate), or keep them in `entities.inl` for extraction in
Ch. 12? *Inclining: keep for Ch. 12.* The mood tables belong together
(orb mood config + agent populations + entity tier extras + ground
config) — they're all per-mood authoring data. Ch. 12 (entities.inl
extraction) decides where the *mood-authoring grouping* lives.
**Decision deferred to Ch. 12.**

## Proposed tags

```
File: cartridges/the_board/modules/orbs.inl

Near line 408 (// ═══ RUNTIME CPU STATE ═══ ...):
  // SEAM[orbs:P3] runtime state sub-grouped by ownership lifetime —
  //   player-owned (persists) vs mood-owned (refreshes on configure)

Near line 500 (// The mood carries one default (flock_gesture_default); we reuse...):
  // SEAM[orbs:L1] one default reused across three rule gesture systems —
  //   simplification documented; extend OrbMoodConfig if future moods care

Near line 552 (// Note: offsets 180-188 used to be _pad_tier0/1/2 ...):
  // SEAM[orbs:L2] field repurposing — pack_tiers_ and pack_flocking_
  //   cooperate through carved-out _pad fields. Comment is the enforcement.
  //   Same family as agents:L2 hardware mirror.

Near line 974 (void update_orb_coupling(float polyphony, float dt, ...)):
  // SEAM[orbs:P1] per-frame coupling lives IN the module — model for
  //   resolving musical:K2 (mode intensities) and pawn:K1 (aura presence)
```

---

# Chapter 9 — `floaters.inl`

497 lines. **Cubes only**, despite the name — the file is named for the
GPU buffer (`floating-entity buffer`, shared between spheres and cubes)
rather than for who consumes the registry. Spheres do "their own thing
(analytical PGA orbit, no behavior layer)" per the file header; nothing
sphere-specific lives here.

Smaller cousin of agents.inl (956) and orbs.inl (1128) — same cockpit
pattern at moderate scale. ~3 registries, F4–F7 diagnostics, no
self-owned GPU dispatch (cube kernel rides the floating-entity dispatch
in render_passes.inl). The chapter is mostly positive findings + one
small bug + the partial-population observation that explains why two
out of three registries are placeholder-shaped.

## Owns

### Tuning console (84–131)

- **Substrate (drift integrator):** `CUBE_DEFAULT_SPRING_STIFFNESS = 4.0`,
  `CUBE_DEFAULT_DRAG = 1.5`. Authored constants for cube physics.
- **Diagnostics:** `CUBE_CORRAL_RADIUS = 30.0`, `CUBE_CORRAL_DURATION = 4.0s`
  (F6 corral animation), `FLOATER_COORDINATION_STEPS[3] = {0.0, 0.5, 1.0}`
  (F5 cycle).
- **WGSL kernel constants documented (not here, but pinned):** the file
  enumerates the WGSL-side force amplitudes / wavelengths / time scales for
  curlfield + phasewave kernels with explicit promotion path (~16 bytes
  uniform + 4 lines if CPU adjustability is wanted). **Intentional
  specificity, properly named.** Same family as `agents:L2` and
  `orbs:L2` — comment is the authority.

### Behavior IDs (64–81)

`CUBE_BEHAVIOR_STATIONARY/CURLFIELD/PHASEWAVE` (3) + diagnostic name array.
Header explicitly mirrors the 4-step add-a-behavior procedure that crosses
this file + world.wgsl.

### Three registries

| Registry             | Type                           | Rows  | Purpose                                        |
|----------------------|--------------------------------|-------|------------------------------------------------|
| `CUBE_TIER_GAINS`    | `CubeTierGain`                 | 4     | Per-tier multipliers on spring/drag/amp        |
| `CUBE_POPULATIONS`   | `CubePopulationDef`            | 6 (per-mood) | Per-mood behavior weights                |
| (Behavior IDs — above)                                                                                       |

Two of the three are explicitly placeholder-shaped:

> *"Default (all 1.0) preserves Phase-3-pre-organization behavior;
> character pass populates with real differentiation."*

> *"Default (all-Stationary) preserves Phase-3-pre-organization
> behavior; character pass populates with real per-mood character."*

This is **intentional placeholder** — the cockpit is wired, the dials
are in place, the values await the character pass. Different from dead
code or stale data.

### Spawn-time helpers (167–258)

- `apply_cube_tier_gains(spring, drag, tier)` — called by
  `entity_pipeline.inl::cube_write_gpu`.
- `pick_cube_behavior_for_spawn(mood, seed)` — same caller.

### Diagnostics (261–497)

- **Coordination cycle (F5)** — `cycle_floater_coordination` steps
  `gpuState_.config().floater_coordination` through `FLOATER_COORDINATION_STEPS`.
- **Behavior override (F4)** — `cubeBehaviorOverride_` plus
  `apply_cube_behavior_override` / `cycle_cube_behavior_override`.
- **Corral animation (F6)** — `CubeCorralAnim` + `cubeCorralAnim_[]` per-slot
  array, `corral_ease_` smoothstep, `corral_cubes` arms the animation,
  `tick_cube_corral_animations` advances per-frame.
- **Kite mode (F7)** — `cubeKiteMode_` flag + `cubePawnOffset_[]` per-slot
  array, `toggle_cube_kite_mode` switches anchor space.

## Consumes

- **From `cartridge.hpp`:** `cpuAgents_[]` (pawn position reads — see L1),
  `currentSeconds_` (animation timing), `MOOD_COUNT` + named mood IDs.
- **From `agents.inl`:** the `cpuAgents_` mirror (read pattern; see L1).
- **From `state.hpp`:** `Dim::MAX_CUBE_INSTANCES`, `GPUFloatingEntityState`
  fields, `GPUDesignConfig::floater_coordination`.
- **From `entity_pipeline.inl`:** the call site of
  `pick_cube_behavior_for_spawn` and `apply_cube_tier_gains` —
  `cube_write_gpu`. **Inverted dependency:** entity_pipeline.inl includes
  the entity machinery; floaters.inl provides the cube-specific spawn
  behavior the machinery calls into. Headers cross both ways — note in K1
  framing for orientation maps.
- **From `seed_utils`:** `cpu_hash`.
- **From Cartridge:** `gpuState_.upload_cube_*`, `activeCubes_[]` (active
  mirror — declared in cartridge.hpp class member list).
- **From `world.wgsl`:** the matching `cube_force_<n>` functions (the
  WGSL-side implementation of each behavior id).

## Exposes

| Function                              | Caller                                  |
|---------------------------------------|-----------------------------------------|
| `pick_cube_behavior_for_spawn`        | `entity_pipeline.inl::cube_write_gpu`   |
| `apply_cube_tier_gains`               | `entity_pipeline.inl::cube_write_gpu`   |
| `tick_cube_corral_animations`         | `update()` per-frame                    |
| `cycle_cube_behavior_override` (F4)   | `input.inl`                             |
| `cycle_floater_coordination` (F5)     | `input.inl`                             |
| `corral_cubes` (F6)                   | `input.inl`                             |
| `toggle_cube_kite_mode` (F7)          | `input.inl`                             |

`apply_cube_behavior_override` is private (called only by `cycle_*`).

## Almost owns

- **Cube tier representation is split across THREE homes** (extends Ch. 4
  K2). Each cube has:
  1. `CUBE_TIER_TABLE` in `entity_pipeline.inl` — Gaussian sampling
     profiles for parameter generation.
  2. `CUBE_TIERS[]` in banner-only `entities.inl` — legacy compact tier
     struct (burial, segs, color_var, color_override).
  3. `CUBE_TIER_GAINS` in this file — multipliers on spring/drag for
     dynamics signature.
  
  These are *functionally distinct* (sampling profile vs. render extras
  vs. behavior dynamics gains), so three homes might be correct — but
  `entity:K2` only named two of them. **Reframes Ch. 4 K2:** the split
  isn't a binary entity_pipeline/entities.inl problem; some families
  have a third dimension owned by their behavior module. The Ch. 12
  (entities.inl extraction) decision needs to account for this.

- **The `behavior_amp_mult` field** in `CubeTierGain` is declared but
  never consumed by the kernel. The comment is honest:
  > *"behavior_amp_mult is reserved for future use — the kernel applies
  > forces uniformly today. To wire it up, kernels would need to read
  > tier_idx and multiply force outputs. Defer until a behavior demands
  > per-tier amplitude differentiation."*
  
  **Intentional placeholder, not dead code.** Same family as `orbs:L1`
  (per-rule mood gesture defaults — declared simplification). The
  registry is wired; the consumer awaits.

## Loose strands

**L1.** **Bug — `cpuAgents_[0]` instead of `cpuAgents_[player_.possessed_slot]`.**
`corral_cubes` reads `cpuAgents_[0].pos_x/z` (line 334–335) and
`toggle_cube_kite_mode` does the same (line 455–456). After possession
transfer, the player's body lives at the new slot; slot 0 is now the
autopilot RandomWalk version of the original body. F6 corral and F7 kite
toggle currently center the ring/offsets around slot 0's position, not
the player's current position.

This is exactly the cleanup territory `agents:D2` named: every site that
reads `cpuAgents_[player_.possessed_slot]` should become a
`pawn_x()/pawn_z()` accessor in `pawn.inl`. Floaters.inl is *additionally*
broken — it doesn't even use `player_.possessed_slot`; it hardcodes `0`.
**The fix is one-line per site, plus future-proofing through pawn.inl's
accessors.** Promote to immediate-fix free strand if exhibition-bound
(F6/F7 are diagnostic but visible).

**L2.** Hygiene rows in `CUBE_POPULATIONS` for moods that don't actually
spawn cubes. Comment is explicit:
> *"Cubes are gated by `CubeConfig::MOOD_MULTIPLIER` (in
> entity_pipeline.inl) which is `{1, 1, 0, 0, 1, 0}` — cubes don't
> spawn in indoor moods or in MOOD_FINITE_OUTDOOR_REF, so those rows
> here are never consulted in practice. We declare them anyway for
> hygiene; if the spawn gate ever changes, the populations will already
> exist."*

**Intentional defensive design.** Declare for the full domain even when
some entries are unreachable. Worth tagging as a *named pattern*
(P4 — see below) so other modules can reference it.

**L3.** Two registries (`CUBE_TIER_GAINS`, `CUBE_POPULATIONS`) are at
their pre-character-pass defaults — all-1.0 multipliers, all-Stationary
weights. The dials are wired but not yet authored. Not a problem, just
a state observation: floaters.inl is half-built by intent, with the
cockpit ready for the values.

**L4. (Added by world.wgsl audit.)** `CUBE_BEHAVIOR_COUNT = 3` is
declared on the C++ side of floaters.inl with the four-step
add-a-behavior procedure named in the file header (lines 64–72). The
WGSL side declares the three id constants (`CUBE_BEHAVIOR_STATIONARY/
CURLFIELD/PHASEWAVE`) plus three force functions plus the
`cube_behavior_force` switch, but **has no parallel
`CUBE_BEHAVIOR_COUNT` const**.

Compare with `agents.inl` (the cockpit-pattern model from Ch. 7):
`AGENT_BEHAVIOR_COUNT_WGSL: u32 = 10u` is declared on the WGSL side
(line 680), and a C++ `static_assert` cross-checks the count against
`GPU_AGENT_BEHAVIOR_COUNT` from `state.hpp` (line 88 of agents.inl).
Three places, with C++ as the contract authority — exactly the
intentional-specificity pattern the seam map credits.

Floaters has the four-step procedure documented but the cross-check
enforcement is one-sided. A future addition that does step 3 (WGSL
side) but forgets step 1 (C++ count bump) would compile fine; the
`cycle_cube_behavior_override`'s `% CUBE_BEHAVIOR_COUNT` modulo
arithmetic would silently skip the new behavior.

**Action.** Add a `CUBE_BEHAVIOR_COUNT_WGSL: u32 = 3u` const on the
WGSL side near the id declarations, and a C++ `static_assert` in
floaters.inl matching the pattern from agents.inl. Both sides tagged
with `MUST match`-style comments. Same family as `agents:L2`
(intentional hardware mirror, comment is the enforcement).

## Knots

None. The module is well-structured.

The closest thing to a knot is the multi-file orchestration around cube
tier representation (entity_pipeline + entities.inl + this file), which
folds into the Ch. 12 entities.inl extraction question. Already on the
backlog.

## Patterns this chapter names

Two more patterns for the cross-chapter index, both useful for
recognizing similar shapes elsewhere:

**P4 — "Hygiene rows."** Per-mood (or per-domain) registries declare
entries even for cases the *current* gate skips, so future gate changes
don't require parallel updates. Works when the registry is declarative
and the gate is the discriminator. Seen in `CUBE_POPULATIONS`. Likely
useful if `AGENT_POPULATIONS` ever has zero-count moods that might
become populated.

**P5 — "Release-pending sentinel."** When CPU lacks accurate state to
hand to GPU (the CPU mirror is one frame stale; the GPU has the truth),
encode the *intent* as a sentinel value the kernel reads next frame and
acts on. `toggle_cube_kite_mode` OFF sets `follow_pawn = 2u` — a
"release pending" sentinel. The kernel sees it, snapshots the cube's
current position into anchor, zeros drift, switches `follow_pawn` back
to 0. Visible position preserved; no CPU drift estimation; no readback
latency. **The pattern of choice when CPU and GPU disagree about whose
view is more accurate.** Worth recognizing — likely useful for similar
"GPU has fresher state" cases (agent eviction, ribbon end-of-life).

## Open decisions

**D1.** **Resolve L1 immediately, or fold into pawn.inl extraction?**
The bug is small (two sites, one-line each) and visible (F6/F7 diagnostic
behavior). *Inclining: fix immediately as a tiny surgical pass*, then
let pawn.inl extraction (D2 from Ch. 7) wrap the access into accessors.
The fix doesn't depend on extraction; it just uses
`cpuAgents_[player_.possessed_slot]` instead of `cpuAgents_[0]` until
the accessor exists. **Decision: small immediate fix; track the
accessor-rename under agents:D2.**

**D2.** **When does the character pass land** to populate
`CUBE_TIER_GAINS` (real per-tier dynamics differentiation) and
`CUBE_POPULATIONS` (real per-mood behavior character)? This is a design
question for the artist (Jean), not a code-organization question. Track
as a deferred authoring task — when the answer arrives, one
seam-map-aware edit populates both registries; no structural change
needed.

**D3.** **Should the file be renamed** from `floaters.inl` to `cubes.inl`,
given it's cube-only? *Inclining: defer.* The header comment names the
discrepancy and explains the historical reason; renaming is a small
churn cost with negligible clarity benefit when the file's first ten
lines tell the truth. Re-evaluate if a sphere behavior layer ever
emerges (then the file might genuinely become "the floating-entity
behavior layer"). **Decision deferred indefinitely.**

## Proposed tags

```
File: cartridges/the_board/modules/floaters.inl

Near line 134 (// ═══ REGISTRY: TIER GAINS ═══):
  // SEAM[floaters:placeholder] CUBE_TIER_GAINS at default — character pass
  //   populates real differentiation; reserved field behavior_amp_mult awaits

Near line 153 (float behavior_amp_mult; // reserved):
  // SEAM[floaters:placeholder] reserved field — kernel applies forces
  //   uniformly today; consumer awaits behavior demanding amplitude differentiation

Near line 198 (static constexpr CubePopulationDef CUBE_POPULATIONS[MOOD_COUNT]):
  // SEAM[floaters:P4] hygiene rows — populated for full mood domain even
  //   for moods CubeConfig::MOOD_MULTIPLIER zeroes out

Near line 334 (const float px = cpuAgents_[0].pos_x;):
  // SEAM[floaters:L1] bug — should be cpuAgents_[player_.possessed_slot];
  //   F6 corral centers on slot 0 not current pawn after possession
  //   (cleanup target: agents:D2 pawn_x()/pawn_z() accessors)

Near line 455 (const float px = cpuAgents_[0].pos_x;):
  // SEAM[floaters:L1] same bug as line 334 — kite mode toggle reads
  //   wrong slot under possession

Near line 470 (// Toggle OFF: rather than write anchor on CPU...):
  // SEAM[floaters:P5] release-pending sentinel — follow_pawn = 2u tells
  //   kernel to snapshot position next frame; CPU avoids drift estimation
```

---

# Chapter 10 — `render_passes.inl`

774 lines. **The procedural counterpart to the cockpit modules** — all
behavior, no declarations, no registries, no runtime state owned. Pure
verbs: pack data, dispatch compute, draw shadow, draw color, project
matrices. The file header calls it accurately:

> *"GPU dispatch and draw calls. The speaker at the end of the signal
> chain: reads final state, issues compute and render passes."*

Every other module decides *what should happen*; this module *makes it
happen on the GPU*. Different shape than the chapters before it — the
"Owns" section is a pipeline of named operations rather than a state
ledger; "Almost owns" is mostly empty (a transformer with no state has
nothing to almost-own).

## Owns

Three labelled sections, nine functions:

### Pre-Render Data Preparation (21–132)

- **`upload_ground_entries(queue)`** (27–117). Per-frame: walks the
  per-family `active*_[]` mirrors, packs world positions into
  `GPU{Arch,Column,Pyramid,Palm}GroundEntry` arrays, uploads. Plants
  (palm + cactus + blade) share one combined compute buffer split via
  three `static constexpr` offsets (`PALM_OFF`, `CACT_OFF`, `BLAD_OFF`)
  but keep separate render uniform buffers for VS bindings — encoded
  in-line. **Comment-as-policy:** `ground_y` set to 0 for all families
  because the GPU compute shader replaces it with the heightfield
  sample. The CPU side uploads positions; the GPU side resolves Y.

- **`dispatch_placement_correction(encoder)`** (124–132). Three lines
  of dispatch wrapper: open compute pass, call
  `renderer_.dispatch_entity_placement`, end pass. The Y-correction
  comment is a one-paragraph map of who reads what (paintings + GoL +
  columns + palms + cacti single-point, arches 2-point pier feet min,
  pyramids 5-point corner min).

### GPU Compute Dispatch (134–227)

- **`dispatch_compute(encoder)`** (137–195). The seven-stage compute
  phase, in fixed order:
  1. Ribbon ring transforms (only if a ribbon is renderable)
  2. Terrain config update
  3. Player agent (walker policy on possessed slot)
  4. Other agents (sees player's frame-current position)
  5. Camera (flyer policy)
  6. Sphere (analytical PGA orbit)
  7. Cube (drift integrator)
  8. VP matrix derivation
  
  Order is load-bearing: the player kernel runs first so the
  other-agents kernel sees its current position when computing eviction.
  Comment is the enforcement.

- **`dispatch_frustum_cull(encoder, queue)`** (204–227). 4-step indirect
  draw setup: reset compute buffer → frustum cull compute pass → copy
  buffer-to-buffer (Storage→Indirect, since Dawn D3D12 can't share
  Storage|Indirect usage). Outdoor-only — early-returns on
  `!renderer_.use_indirect_terrain()`.

### Shadow Pass + Main Pass (229–606)

- **`render_shadow_pass(encoder)`** (237–295). Two branches:
  - **Indoor (atlas):** per-spot-light loop, copies VP from staging,
    routes lights 0–1 to the sun map (idle indoor) and lights 2–3 to
    the spot map, splits each map into 2048×4096 left/right tiles.
    `LoadOp::Clear` for `within=0`, `LoadOp::Load` for `within=1`. Per
    the comment: doubles per-tile resolution vs. an old single-texture
    2×2 grid, with zero extra memory.
  - **Outdoor (single):** single 4096×4096 pass with directional sun VP.

- **`draw_shadow_all(pass)`** (298–413). The shared shadow draw list:
  terrain LOD0+LOD1, GoL zones, pawn, sphere, monolith, ribbon (gated
  on `renderedRibbonSlot_`), arch, column, palm, cactus, blade, shell.
  10 distinct shadow draws.

- **`render_main_pass(encoder, backbuffer, depth)`** (416–606). The
  full color pass. Same family-by-family draw list as shadow, plus:
  - Terrain LOD0 has two paths: `draw_patch_terrain_lod0_indirect`
    (outdoor, GPU-culled) vs. `draw_patch_terrain_direct` (indoor, CPU
    count). LOD1 is always direct (Dawn D3D12 limit: only one indirect
    draw per pass).
  - Wall-mounted paintings and gallery frames (separate bind groups
    via `gallery_*_group()`).
  - Sky orbs (additive blending, gated by `render_orbs(pass)` from
    Ch. 8).
  - Fade overlay (alpha-blended, last so it covers the orbs too).
  
  In total **22 entity-side `renderer_.draw_*` calls** (10 shadow + 12
  color, modulo the orbs/paintings/fade non-`renderer_.draw_*` calls).

### Light Matrix Computation (608–774)

- **`compute_spot_light_vp(light, view_proj_out)`** (615–685).
  Per-spot-light perspective projection sized to the cone angle (FOV =
  outer_half × 2 + 0.2, capped at 2.8 rad). Pure CPU math, ~70 lines.
- **`compute_sun_matrices(direction, view_proj_out, center_x, center_z)`**
  (692–774). Orthographic projection, ±350 half-extent, altitude 300.
  Pure CPU math, ~80 lines.

## Consumes

A pipeline reads from many places. Roughly:

- **From the `active*_[]` mirrors** (every entity family): `activeArches_`,
  `activeColumns_`, `activeAntennas_`, `activePyramids_`, `cpuPyramids_.instances`,
  `activePalms_`, `activeCacti_`, `activeBlades_`, `cpuPiers_`,
  `cpuSpotLights_`.
- **From per-frame state:** `renderedRibbonSlot_`, `lod0PatchCount_`,
  `renderPatchCount_`, `golZoneCount_`, `wallFrameCount_`,
  `activePaintingCount_`, `transitionFadeAlpha_`, `clearColor_`,
  `spotLightActive_`, `cubeKiteMode_` (no — only floaters).
- **From `state.hpp`:** every `Dim::*` capacity constant, every
  `GPU*GroundEntry` shape, `GPUSpotLight`, `MAX_SPOT_LIGHTS`,
  `Dim::SHADOW_MAP_SIZE`, `Dim::PIER_ARCH_BASE`, `Dim::ANTENNA_SLOT_OFFSET`.
- **From renderer_:** every `dispatch_*` and `draw_*` call (~30
  distinct entry points).
- **From `gpuState_`:** every bind group accessor, every buffer accessor,
  every staging buffer; `upload_*_origins`, `reset_frustum_indirect`,
  `light_vp_offset/size`, `vp_buffer`, `spot_vp_staging`, etc.
- **From `orbs.inl`:** `render_orbs(pass)`.

## Exposes

| Function                      | Caller (verified by grep)              |
|-------------------------------|----------------------------------------|
| `upload_ground_entries`       | `cartridge.hpp::render()` (4 hits inc. self-ref) |
| `dispatch_placement_correction` | `cartridge.hpp::render()`            |
| `dispatch_compute`            | `cartridge.hpp::render()` (8 hits)     |
| `dispatch_frustum_cull`       | `cartridge.hpp::render()`              |
| `render_shadow_pass`          | `cartridge.hpp::render()`              |
| `render_main_pass`            | `cartridge.hpp::render()`              |
| `compute_spot_light_vp`       | `mood.inl::apply_mood` (1 hit)         |
| `compute_sun_matrices`        | **NONE — see L1**                      |
| `draw_shadow_all`             | private — called only from `render_shadow_pass` |

The module exposes only functions; no struct/constant/state crosses the
boundary. Clean.

## Almost owns

Two narrow surfaces, both shape rather than state:

- **The order of operations in `dispatch_compute`.** The comment names
  the player-first-then-others dependency, but the order is encoded only
  in the C++ statement sequence. If a future kernel introduces a similar
  cross-dispatch dependency, it'll be enforced by the same convention —
  call-site comments, not declarative ordering. Same family as
  `mood:K2` (twelve concerns in linear order, comment-enforced).
  Different in scale: 8 lines vs. 250.

- **The shadow / main draw list parity.** `draw_shadow_all` (10 draws)
  and `render_main_pass`'s entity section (12 draws) have to stay in
  sync — every entity that casts a shadow needs both sites updated when
  added. Today the family count is small enough that drift is
  catchable, but the duplication is a recurring tax. See L2.

## Loose strands

**L1.** **`compute_sun_matrices` is currently unreferenced — but
latent, not dead.** Verified across cartridge.hpp, mood.inl, and every
`.inl`: the only references are the function definition itself
(line 692) and the file-header advertisement (line 14). No caller
today.

**Reframed under the latent-code distinction (introduced in Ch. 10
conventions):** the sun is going to do work in this project. Its
position and intensity are both planned to become musically
expressive — not just static directional light, but a presence that
responds to phrasing. The CPU-side sun VP machinery is the artist's
note-to-self about what's coming. **Keep it.**

The current GPU-derived sun VP path (via `dispatch_compute_vp`) is
fine for today's static-sun use. When the sun starts moving in
response to music, or when shadows need CPU-side projection
(debug overlays, alternate backends, or a feature that wants the
matrix on the CPU side before issuing the dispatch), this function
is ready.

The seam-map action is a tag, not a deletion: add an intent comment
near the function so a future reader doesn't mistake "no callers
today" for "delete me." The same goes for the file header (line 14)
which already advertises the function — that's correct, leave it.

**L2.** **Shadow / main draw list duplication.** `draw_shadow_all`
contains the same family-by-family draw sequence as `render_main_pass`'s
entity section, with parallel calls (`draw_shadow_arch` ↔ `draw_arch`,
etc.). Adding a new entity family requires updating both lists with the
right ordering. Today: 10 shadow + 12 color, drift would be visible.
Tomorrow: more families = more chance of forgetting one.

A registry-driven approach would solve it: a single
`SHADOWED_ENTITIES[]` array of `(shadow_draw_fn, color_draw_fn,
gating_predicate)` tuples, walked twice. But the gating differs (ribbon
needs `renderedRibbonSlot_ != UINT32_MAX`, GoL needs `golZoneCount_ > 0`,
shell is unconditional), and the buffer-list arguments differ per
family. Not trivial. **Defer until family count grows or a missed-draw
bug actually fires.**

**L3.** **Indentation oddity.** This file uses 12-space leading
indentation on every function definition, as if the contents are nested
inside a function body — but they're class-body member functions like
every other `.inl`. Compared with all other `.inl` files (verified by
grep): every other file starts function definitions at column 0 (or in
class-member style at 4). `render_passes.inl` is unique. Cosmetic — the
compiler doesn't care — but git blame and editor folding both treat the
file differently from the rest. Trivial cleanup; would normalize when
the file moves to a real translation unit. Tag-only for now.

## Knots

None. The module is the model for *procedural* rather than declarative
work. Where agents/orbs/floaters showed how to organize state +
registries + diagnostics, render_passes shows how to organize a
pipeline of named GPU operations: each function does one thing,
ordered by dependency, with comment-as-policy where ordering matters.
No knots filed.

The closest thing to a structural concern is the duplication noted in
L2, which is a tax not a tangle.

## Patterns this chapter names

**P6 — "Comment-as-policy ordering."** When a sequence of operations
has a load-bearing dependency, the dependency lives in the comment at
the call site rather than in a declarative structure. Used here for
`dispatch_compute`'s player-first ordering. Used at smaller scale in
`mood.inl` (where 12 concerns flow in fixed order with one short
comment per concern). Same family as the `pack_tiers_` / `pack_flocking_`
cooperation in orbs.inl (Ch. 8 L2). Pattern works when the sequence is
small and rarely changes; breaks when sequences grow large enough to
hide ordering errors (mood:K2's 250-line case).

**P7 — "Speaker at the end of the chain."** The module pattern this
chapter exemplifies — a transformer that reads decisions from
everywhere and writes commands to one downstream surface (GPU). No
state owned. No registries. Pure verbs. Procedurally-shaped, not
declarative. The right shape for this role.

## Open decisions

**D1.** ~~Verify `compute_sun_matrices` is dead, then delete~~ —
**reframed as latent code (per Ch. 10 conventions update).** Action is
a tag, not a deletion: add an intent comment near the function naming
the planned use (sun position + intensity becoming musically
expressive). Keeps the function for the future, prevents misreading
as dead.

**D2.** Draw-list duplication (L2): registry-driven, or accept? Defer
until family growth or bug pressure. **Decision: accept until pressure
arrives.**

**D3.** Indentation cleanup (L3): immediate or defer? Pure cosmetic;
single editor pass to normalize. Could land as a free-strand pull at
end-of-tour. No semantic impact. **Defer to free-strand batch.**

## Proposed tags

```
File: cartridges/the_board/modules/render_passes.inl

Near line 137 (void dispatch_compute(...)):
  // SEAM[render_passes:P6] order is load-bearing — player kernel before
  //   other-agents kernel; comment-as-policy enforcement

Near line 298 (void draw_shadow_all(wgpu::RenderPassEncoder& pass)):
  // SEAM[render_passes:L2] family-by-family draw list duplicated in
  //   render_main_pass — adding an entity family requires both sites

Near line 692 (void compute_sun_matrices(...)):
  // SEAM[render_passes:L1] latent, not dead — currently unreferenced,
  //   but the sun's position and intensity will become musically
  //   expressive. CPU-side sun VP is the foundation for that work.

(File-level cosmetic — no specific line)
  // SEAM[render_passes:L3] 12-space leading indentation unique among .inl
  //   modules — normalize on extraction
```

---

# Chapter 11 — `input.inl`

336 lines. The smallest module after `pawn_aura.inl`. **Another P7
"speaker at the end of the chain"** — but for the *front* of the
chain rather than the back: reads OS events, translates to cartridge
intent. Like `render_passes.inl` it owns no state and exposes only
verbs; unlike render_passes it crosses into virtually every other
module (every command function `cycle_*` / `toggle_*` /
`force_respawn_*` ends up here as a key binding).

Distinctive shape: one ~180-line `on_key_down` switch handles essentially
every player command in the project. Mostly correct in scope, but the
five mood-transition cases (KEY_5–9) are textbook copy-paste duplication
and the keypad bindings duplicate the numpad mapping documented in
`musical.inl` (already on file as musical:L3).

## Owns

### Platform-fallback macros (10–73)

`#ifndef GLFW_KEY_KP_0 ... #endif` for 19 key codes (numpad + control +
caps lock + F1–F7). The comment names the rationale:
> *"Platform workaround: GLFW key codes not available in all header
> configurations."*

Defensive fallback. Workaround for a GLFW header inconsistency, not
something to refactor.

### Event handlers

| Handler                | Lines      | What it does                                     |
|------------------------|-----------|---------------------------------------------------|
| `on_key_down(key)`     | 75–260    | The big switch — every command keypress           |
| `on_key_up(key)`       | 262–270   | Movement-key release only                         |
| `on_mouse_move(dx,dy)` | 272–282   | Look (left-drag) / pan (right-drag) deltas       |
| `on_mouse_button(b,p)` | 284–287   | Drag-flag tracking                                |
| `on_scroll(delta)`     | 289–291   | Zoom delta                                         |

### Input pipeline helpers

- `update_movement_intent()` (293–308) — turns `keys_.forward/back/left/right`
  bools into normalized `inputState_.move_x/z`.
- `clear_input_deltas()` (310–316) — zeros mouse deltas; called once
  per frame from `update()`.

### Local toggles + radius command

- `toggle_fpv_mode()` (318–323) — flips `fpvMode_`, uploads to GPU,
  logs.
- `set_render_radius(r)` (325–336) — clamped 1..PREGEN_RADIUS, triggers
  full re-eval via `lastCenterX_/Z_ = INT32_MAX` sentinel.

### The keymap (in `on_key_down`)

| Range          | Function                                           |
|----------------|----------------------------------------------------|
| ARROW keys     | Movement intent (forward/back/left/right)          |
| 1              | `gpuState_.toggle_freeze_sphere()`                 |
| 2, 3           | Aura height / field toggles                        |
| 5, 6, 7, 8, 9  | Mood transitions (sunset / indoor flat / vault / finite outdoor / finite outdoor ref) |
| KP_1..KP_7     | Musical mode toggles (MMODE_*)                     |
| KP_8           | Cycle orb motion rule                              |
| KP_9           | Toggle orb anchor                                  |
| KP_DECIMAL     | Cycle orb gesture                                  |
| 0              | Cycle orb palette                                  |
| LEFT/RIGHT_CONTROL | Toggle FPV camera mode                         |
| CAPS_LOCK      | Try possess nearest agent                          |
| F1–F3          | Agent diagnostic dials                             |
| F4–F7          | Cube/floater diagnostic dials                      |
| `[`, `]`       | Render radius decrease/increase                    |

## Consumes

The module cuts across virtually everything because every player command
ends here. Specifically:

- **From `cartridge.hpp`:** `keys_`, `mouse_`, `inputState_`,
  `transitionPhase_`, `pendingDestination_`, `transitionTimer_`,
  `activeSeed_`, `activeRadius_`, `lastCenterX_/Z_`, `fpvMode_`,
  `device_`, `MOOD_*` constants, `MOOD_TABLE`, `derive_finite_radius`,
  `mood_name`, `cpu_hash`, `GRID_RADIUS`, `PREGEN_RADIUS`.
- **From `musical.inl`:** `toggle_mmode`, `MMODE_*` IDs.
- **From `pawn_aura.inl`:** `auraEnabled_`, `auraHeightEnabled_`,
  `auraCfgDirty_`.
- **From `agents.inl`:** `try_possess_nearest`,
  `cycle_agent_behavior_override`, `cycle_agent_tier_override`,
  `force_respawn_population`.
- **From `floaters.inl`:** `cycle_cube_behavior_override`,
  `cycle_floater_coordination`, `corral_cubes`,
  `toggle_cube_kite_mode`.
- **From `orbs.inl`:** `cycle_orb_palette`, `cycle_orb_motion_rule`,
  `cycle_orb_gesture`, `toggle_orb_anchor`.
- **From `gpuState_`:** `toggle_freeze_sphere`, `set_fpv_mode`.

## Exposes

| Function                          | Caller                                |
|-----------------------------------|---------------------------------------|
| `on_key_down`, `on_key_up`        | OS event loop (main.cpp / GLFW)       |
| `on_mouse_move`, `on_mouse_button`| OS event loop                         |
| `on_scroll`                       | OS event loop                         |
| `clear_input_deltas`              | `cartridge.hpp::update()` per-frame   |
| `update_movement_intent`          | private — called from on_key_*        |
| `toggle_fpv_mode`                 | private (this file's `LEFT_CONTROL`) — Ch. 6 inventories under pawn.inl candidates |
| `set_render_radius`               | private (this file's `[` / `]`)       |

## Almost owns

**Almost all of player command dispatch** — the dispatch surface is here,
but the mappings between key and command live both here AND elsewhere:

- KP_1..KP_7 in `on_key_down` mirror the `MMODE_*` numpad mappings
  already documented in `musical.inl`'s header comments. **Already on
  file as `musical:L3`.** When musical.inl gains its
  `MMODE_REGISTRY[]` (per `musical:K1`), the natural shape is:

  ```cpp
  case GLFW_KEY_KP_1: toggle_mmode(MMODE_REGISTRY[0].id); break;
  ```

  Or, more declarative: a `static constexpr KeyBinding KEYMAP[]` driven
  from this file using the registry from musical.inl. **Resolves jointly
  with `musical:K1`.**

- `toggle_fpv_mode()` is *defined* here (lines 318–323) but is a pawn
  concern. **Already inventoried in Ch. 6 K1** as a candidate for
  `pawn.inl` — the body migrates with the rest of pawn machinery; the
  switch case in `on_key_down` (`LEFT/RIGHT_CONTROL`) becomes a call
  into pawn.inl's `toggle_fpv_mode()`.

## Loose strands

**L1.** **The five mood-transition cases (KEY_5..9) are copy-paste
duplication.** Each case (47 lines for KEY_5, ~17 lines each for
KEY_6..9, summed ~83 lines) does the same thing with one variable
swapped:

```
if (transitionPhase_ != TransitionPhase::IDLE) break;
uint32_t mood = MOOD_<NAME>;
const auto& mp = MOOD_TABLE[mood];
uint32_t dest_seed = cpu_hash(activeSeed_, 999u);
uint32_t radius = derive_finite_radius(dest_seed, mp);   // KEY_5 omits
pendingDestination_ = { dest_seed, mp.finite, radius, mood };
transitionPhase_ = TransitionPhase::FADE_OUT;
transitionTimer_ = 0.0f;
std::cout << "[World] Transition (" << mood_name(mood) << ...) << "\n";
```

A single helper would dedupe entirely:

```cpp
void request_mood_transition(uint32_t mood) {
    if (transitionPhase_ != TransitionPhase::IDLE) return;
    const auto& mp = MOOD_TABLE[mood];
    uint32_t dest_seed = cpu_hash(activeSeed_, 999u);
    uint32_t radius = derive_finite_radius(dest_seed, mp);
    pendingDestination_ = { dest_seed, mp.finite, radius, mood };
    transitionPhase_ = TransitionPhase::FADE_OUT;
    transitionTimer_ = 0.0f;
    if (mp.finite) {
        uint32_t side = 2 * radius + 1;
        std::cout << "[World] Transition (" << mood_name(mood) << " "
                  << side << "x" << side << "): seed " << activeSeed_
                  << " -> " << dest_seed << "\n";
    } else {
        std::cout << "[World] Transition (" << mood_name(mood) << "): seed "
                  << activeSeed_ << " -> " << dest_seed << "\n";
    }
}
```

Then each case becomes a single line:
`case GLFW_KEY_5: request_mood_transition(MOOD_OPEN_SUNSET); break;`

The helper itself is a Cartridge member — exposed for CAPS_LOCK or other
future triggers (e.g., a portal-detected automatic transition that today
goes through a different path but might want to use this).

**Where does the helper live?** Two reasonable homes — current file
(input.inl), or `mood.inl` (since it's mood-related). Slight inclination
toward `mood.inl` since that file already owns mood machinery, but
either works. **Decision deferred to D1.**

**L2.** Two diagnostic ranges share the F-key palette but **F4–F7 are
cube-specific**, not floater-specific. The header comment is honest:
> *"F4 — cycle cube behavior   (stationary → curlfield → phasewave)"*
> *"F5 — cycle floater coordination   (0.0 → 0.5 → 1.0)"*

So F4 acts on cubes, F5 acts on the system-wide floater coordination
knob (which today only cubes consume since spheres do their own thing
per `floaters.inl` header). The comment is accurate; the framing matches
the floaters.inl naming discrepancy already noted (`floaters:D3` —
file is "cube-only despite the name"). **No new strand — this is
downstream of the floaters.inl naming.**

## Knots

None.

The mood-transition duplication is L1, not a knot — local, mechanical,
no cross-module dependency.

## Patterns this chapter names

No new patterns. The module fits **P7 (Speaker at the end of the
chain)** — exactly the front-of-chain mirror: pure-verbs translator,
reads OS events, dispatches into the rest of the cartridge. Same shape
as `render_passes.inl`, opposite direction of flow.

Worth noting: input.inl + render_passes.inl together form a
**translator pair** at the cartridge boundary. Input speaks OS-event →
cartridge-command; render_passes speaks cartridge-state → GPU-command.
Same job, different sides. This is what the cartridge's I/O surface
looks like as code.

## Open decisions

**D1.** **Refactor the five mood-transition cases into a
`request_mood_transition(mood)` helper?** *Inclining: yes, in mood.inl.*
The helper is small, mechanical, and gives every future mood-transition
trigger one canonical entry point — useful both for input shortcuts and
for mood transitions triggered from other places (portal detection
already does its own variant in mood.inl). Cost: ~25-line helper, 5
single-line case rewrites. Promotable to small Claude Code surgical
pass alongside the floaters bug fix. **Decision: yes, defer site
question to extraction time (mood.inl vs input.inl).**

**D2.** **Fold KP_1..KP_7 into `MMODE_REGISTRY[]` once musical.inl
gains it.** Joint resolution with `musical:K1`. No standalone decision.

## Proposed tags

```
File: cartridges/the_board/modules/input.inl

Near line 75 (void on_key_down(int key) {):
  // SEAM[input:P7] front-of-chain speaker — OS events → cartridge commands;
  //   mirror of render_passes.inl which is cartridge state → GPU commands

Near line 94 (case GLFW_KEY_5: ... mood transition):
  // SEAM[input:L1] five copy-paste mood-transition cases — extract
  //   request_mood_transition(mood) helper (likely lives in mood.inl)

Near line 172 (case GLFW_KEY_KP_1: toggle_mmode(MMODE_TERRAIN_WAVES); break;):
  // SEAM[input:D2] resolves with musical:K1 — KEYMAP[] driven from
  //   MMODE_REGISTRY[] once musical.inl declares it
```

---

# Chapter 12 — banner-only modules in `cartridge.hpp`

Five `.inl` modules are physically inlined inside `cartridge.hpp` as
banner-only blocks rather than `#include`d. Together they account for
~4422 lines — roughly half the spine. Each is a complete, well-named
unit of code. The chapter shape is the same as a real module chapter,
applied five times, then a cross-cutting discussion at the end.

The reason these are inline rather than extracted is **compilation
order** (Ch. 1's central observation): each block needs to be
positioned where its dependencies are already declared and where its
declarations land before later code that uses them. Extracting them
to real `.inl` files is mostly mechanical *if the order is preserved*,
but the order is meaningful — it encodes a buildable layering.

Module sub-sections, smallest first:

  - 12.A — `seed_utils` (~71 lines)
  - 12.B — `gol_zones` (~541)
  - 12.C — `entities` (~1059)
  - 12.D — `spawn_engine` (~1184)
  - 12.E — `gallery` (~1567)

Then 12.F — **cross-module discussion: the entities.inl extraction**.
This is the resolution site for several knots opened across chapters
4 / 8 / 9, made denser by the world.wgsl audit.

---

## 12.A — `seed_utils` (lines 554–626)

The smallest module in the project (the smaller `pawn_aura.inl` is
60 lines but is all declarations). 6 static functions, no state, no
domain knowledge. The header says it directly:

> *"Pure math. Hash, Gaussian, tier selection. No member state. No
> domain knowledge. Every layer below depends on these; they depend
> on nothing."*

### Owns

- `cpu_hash(seed, property) → u32` — the base PCG-style hash.
- `cpu_hash_f(seed, property) → f32` — normalized [0,1] form.
- `tile_seed(master_seed, gx, gz) → u32` — patch-level seed derivation.
- `cpu_lattice_node_seed(master_seed, nx, nz, band) → u32` — lattice
  seed derivation. **Mirrors WGSL `lattice_node_seed`** (intentional
  hardware specificity, comment-as-policy at the site).
- `cpu_smoothstep(e0, e1, x) → f32` — interpolation primitive.
- `cpu_sample_gaussian(seed, property, mean, sigma) → f32` —
  Box-Muller, ±3σ truncation. **Mirrors WGSL `sample_gaussian`**
  (named in comment).
- `select_tier(seed, prop, weights, count) → u32` — weighted
  cumulative tier picker.

### Almost owns

Nothing. The module is the textbook "library without state."

### Loose strands

None.

### Knots

None.

### Patterns this module names

**P9 — "Library without state."** Pure functions, no class members
referenced, no domain assumptions. Same shape as render_passes.inl
(P7) but with a different role: render_passes is a transformer at the
output boundary (verbs that produce GPU commands); seed_utils is a
toolbox for everyone (verbs that produce values). Both have zero
state and zero domain coupling. **Useful pattern to recognize because
it's the easiest module to extract to a real file** — true zero-cost
extraction, no compilation-order constraints to worry about.

### Extraction notes

Mechanical. The module already lives in its own logical block with
its own comment header. Moving to a real `cartridges/the_board/modules/seed_utils.inl`
file requires only:

1. `#include` it where the inline currently sits.
2. Remove the inline block.
3. (Optional) un-indent the function bodies — they currently use
   12-space indentation matching the class-body context.

No semantic change. No ordering risk. **Promotable to immediate-pull
if there's any reason to clean up the spine.** The only argument
against is "if it ain't broke" — but the precedent matters: every
banner-only module that follows is harder to extract, and seed_utils
is the warm-up.

---

## 12.B — `gol_zones` (lines 3349–3890)

A **complete subsystem in one block**: tier profiles, spawn detection,
life buffer seeding, eviction, per-frame config upload, derive
request flush. Not a registry that's shared like `entities` or a
service like `seed_utils` — it owns its own end-to-end pipeline,
distinct from but parallel to the cockpit pattern of agents.inl /
orbs.inl.

The header is explicit about the architecture:
> *"Architecture follows the Column entity pattern: GoLZoneProp,
> GoLZoneSpawnConfig, GoLTierProfile, GoLColorMode, GoLZoneState."*

Plus its own select/place/commit logic since it's a **bespoke**
family (not in FAMILY_DISPATCH's generic machinery).

### Owns

The vocabulary of GoL zones:
- `GoLZoneProp` — property index registry (seed band 250, indices 920–939).
- `GoLZoneSpawnConfig` — spawn chance, mode threshold, footprint
  radius, mood gate `MOOD_MULTIPLIER` (`{1, 1, 0, 0, 1, 0}` —
  suppressed in indoor and finite_outdoor_ref like cubes/spheres).
- `GoLColorMode` — NEUTRAL / LENS / BLACKISH; weight matrices for
  height-on vs height-off.
- `GoLTierProfile` + `GOL_TIER_COUNT = 7` tiers — Gaussian
  mean+sigma for density, tick period, spring stiffness, transition,
  height, target colors.
- `MODE_LATTICE_SPACING = 120.0f` — the spatial scale.
- `ZONE_EXTENT = 100.0f` (32 cells × 3.125) — grid-aligned.

The runtime:
- `GoLZoneState` per-instance state.
- `cpuGolZones_[]` array, `golZoneCount_`.
- `GoLSelection` / `GoLPlacement` for the bespoke pipeline.
- Per-frame `dispatch_*` sites (compute, render).

### Consumes

- **From `seed_utils`:** all of it.
- **From `spawn_engine`:** footprint registry (`record_footprint`,
  `is_footprint_clear`).
- **From `mood.inl`:** `m.allow_gol_zones` flag (mood gate).
- **From `state.hpp`:** `GPUGoLZoneState`, `Dim::MAX_GOL_ZONES`.

### Almost owns

- **The bespoke pipeline shape.** GoL zones use their own
  `select_*` / `place_*` / `commit_*` instead of the generic
  EntityFamilyAdapter. Same as ribbon and gallery. **Three "bespoke"
  families**, each a complete subsystem; the dispatch is hand-wired
  per family rather than table-driven. This is exactly what mood:K4
  noted ("ribbon doesn't go through FAMILY_DISPATCH's generic
  machinery"); gol_zones and gallery share the property.

### Loose strands

**L1.** `MODE_LATTICE_SPACING = 120.0f` is also defined in
world.wgsl as `MODE_LATTICE_SPACING` (search at the top of world.wgsl
TUNING SURFACE DIRECTORY: "MODE_LATTICE_SPACING 120 wu — smooth/discrete
clusters"). **Hardware mirror, intentional, comment-as-policy on the
GPU side, but no `MUST match` annotation on the C++ side.** Same
family as `agents:L2`; should add the `MUST match` comment to align
the practice across the codebase.

### Knots

None. The module is well-shaped.

### Patterns this module names

No new patterns. It's an instance of:
- **P7-variant — "Complete subsystem in one block":** end-to-end
  pipeline, vocabulary + state + lifecycle + dispatch all together,
  bespoke (not table-driven). Distinguishable from cockpit modules
  which expose multiple decoupled commands; complete-subsystem
  modules expose a single lifecycle-scoped service.

### Extraction notes

Mechanical, like seed_utils — the dependencies are already linear
(seed_utils + spawn_engine, both above). Only consideration:
gol_zones is included **after** `entities`, `spawn_engine`, and the
sphere/cube/ribbon families that live in cartridge.hpp itself
(2929–3347). Extraction preserves that order.

---

## 12.C — `entities` (lines 628–1693)

The vocabulary of forms. **The most consequential module in this
chapter** — most cross-chapter knots resolve here. Eight per-family
blocks (Ribbon, Arch, Column, Antenna, Palm, Cactus, Blade, Pyramid),
each with the same template:

  - Tier enum + `XxxTierParams` struct + `XXX_TIERS[]` table
  - Color palette (RGB base + variance, or full HSV palette)
  - `XxxConfig` (SPAWN_CHANCE, MOOD_MULTIPLIER, POSITION_JITTER)
  - `XxxProp` (property index registry — seed band per family)
  - `ActiveXxx` runtime tracking struct
  - `activeXxxs_[]` array + `activeXxxCount_` + `xxxMeshGenPending_`
  - (For some: CPU mirror of GPU instances, e.g., `cpuPyramids_`)

Plus the bespoke ribbon block which adds `RibbonColorMode`,
`RibbonProp`, `HarmonicRatio` palettes (vertical + twist), and
`RibbonTierProfile`.

### Owns

The per-family vocabulary for 8 entity families. **All 9 generic-
pipeline families' tier-extra data lives here**: ribbon (bespoke,
fully here) plus Arch / Column / Antenna / Palm / Cactus / Blade /
Pyramid (generic, shared with entity_pipeline.inl which owns the
sampling profiles). Plus harmonic ratio palettes (ribbon-specific).

### Consumes

- **From `seed_utils`:** `cpu_hash_f`, `cpu_sample_gaussian`,
  `select_tier`.
- **From `MOOD_COUNT`:** declared earlier in cartridge.hpp.

### Exposes

Everything is data-shaped: structs and constexpr tables consumed by
spawn_engine, entity_pipeline, gallery, and per-family commit
helpers. ~30 named exports.

### Almost owns

This is where the leaks of multiple other chapters concentrate:

- **Ch. 4 entity:K2** — tier representation split between this file
  (legacy compact tier struct: burial, segs, color_var,
  color_override) and `entity_pipeline.inl` (Gaussian sampling
  profiles: `XXX_TIER_TABLE[]`). Each family has both; adapter
  functions read from both.
- **Ch. 9 cube three-tier-home reframing** — adds a third home for
  cubes specifically: `floaters.inl::CUBE_TIER_GAINS` (spring/drag
  multipliers for behavior dynamics). So cubes have three places
  where tier data lives: entities (extras + render), entity_pipeline
  (sampling), floaters (behavior gains). Confirmed by the world.wgsl
  audit's spot-check.
- **Ch. 8 orbs:D2** — `ORB_MOOD_TABLE` rows live in `cartridge.hpp`
  immediately after the orbs.inl `#include` (lines 1702–1737 per the
  Ch. 1 layout), but the struct (`OrbMoodConfig`) lives in `orbs.inl`.
  **Important nuance from world.wgsl audit:** ORB_MOOD_TABLE is
  authoring data (per-mood orb config), distinct in role from the
  per-family vocabulary in entities.inl. Question of where it lives
  hasn't changed; the framing has — see 12.F.

### Loose strands

**L1.** RibbonProp's property indices skip values irregularly
(SPAWN_ROLL=400, ANCHOR_X=401 ... PALETTE_IDX=409, then jumps to
CUBE_COUNT=410 ... HEIGHT=412, then LATERAL_AMP=420, VERTICAL_AMP=430,
TWIST_AMP=440). Per-coordinate stride is intentional — leaves room
for future per-axis params without renumbering — but the convention
isn't documented at the site. **Add a header comment naming the
stride convention** so a future reader doesn't try to "compact" the
indices.

**L2.** PYRAMID_TIERS and similar tier tables have `aspect_ratio_mean
= 1.0` for all rows (Pyramid OBELISK/TEMPLE/COLOSSUS all use 1.0).
**Latent infrastructure (P8)**: aspect ratio is wired but tier
authoring hasn't differentiated. Same family as floaters' default
all-1.0 tier gains. Awaits character pass.

### Knots

**K1 — already named as Ch. 4 K2.** The two-home tier representation
(this file + entity_pipeline.inl). After the world.wgsl audit and
Ch. 9 cube reframe, **at least three families have three-home
representations** if you count behavior modules. The extraction
question is what shape resolves it; see 12.F.

### Patterns this module names

**P10 — "Per-family vocabulary block."** A repeated structural
template applied per family: Tier enum / TierParams struct /
TIERS table / Color palette / Config / Prop registry / Active
tracking / runtime arrays. Eight applications in this module. The
template is the codebase's textbook expression of "what a new entity
family needs to declare." A future ninth family would copy the
template and fill it in.

The pattern strongly resembles cookie-cutter — almost every block
has the same shape — and **a registry-of-registries** style could
collapse the duplication. But: (a) each family has small but real
differences (ribbon has harmonic ratios, pyramid has truncation,
columns/antennas share a rendering path with split arrays), and
(b) the registry-of-registries shape makes per-family tweaking
harder, not easier. **Worth recognizing the template; not worth
fighting it.** Same posture as Ch. 4 K1's "intentional specificity"
framing.

### Extraction notes

The most consequential extraction in the project. See **12.F** for the
full discussion. Short version: extracting `entities.inl` is mostly
mechanical, but the *what to put in it* question depends on resolving
the multi-home tier representation. Three options on the table; each
has implications.

---

## 12.D — `spawn_engine` (lines 1744–2928)

The spawn machinery. Where things appear, when, with what
spatial constraints. The largest of the inline blocks (~1184 lines)
and the one with the most internal complexity. Includes the
**entity_types.inl sandwich** at line 2689 — the structural reason
the spawn machinery can't trivially extract.

### Owns

The shared spawn infrastructure:

- **`run_spawn_preamble<ActiveT>(...)`** — idempotency-through-slot-
  reservation gate, templated on the active-array type. Returns a
  `SpawnGatePreambleResult` { seed, slot, theme_idx, ok }. The
  six-step spawn modifier chain (mood × population × density ×
  theme × tile × proximity affinity).
- **`negotiate_position(...)`** — spatial negotiation: world position
  with jitter, footprint clearance check.
- **`record_placement_bookkeeping(...)`** — placement census,
  footprint registry update, neighbor-aware spawn density tracking.
- **`select_tier_biased(...)`** — theme-aware tier selection
  (different from seed_utils' `select_tier` which is uniform).

The select/place/commit pipeline:
- `EntityQueueEntry` and `PlacementEntry` tagged unions with
  `EntityInstance` from entity_types.inl as their generic member,
  plus `RibbonSelection`, `GoLSelection`, `GallerySelection`,
  matching `*Placement` types.
- `entityQueue_` and `placementResults_` vectors.
- `select_entities_for_patch(gx, gz)` — walks `FAMILY_DISPATCH[]`,
  invokes per-family `try_select`.
- `place_entity_queue()` — drains queue through per-family
  `try_place`.
- `commit_entity_queue()` — writes GPU state through per-family
  `try_commit`.

Plus per-family spawn helpers exposed for bespoke families:
- `force_spawn_portal_at(...)` (used by mood.inl).
- `pick_portal_mood(...)`, `derive_finite_radius(...)` (used by
  mood.inl + input.inl).
- `fill_ribbon_selection_geometry(...)`, `commit_ribbon(...)` (used
  by mood.inl for mood-5 ribbon).
- `record_footprint`, `is_footprint_clear` (used by gol_zones, gallery).

The per-family infrastructure:
- Per-family mesh-gen preparation (Column, Pyramid).
- Pier write helper.
- Entity distance culling.
- Ground footprint registry.
- Entity census, spawn utilities, spawn configuration summary.
- Global entity density.
- Property index registry (shared across families).
- Proximity affinity (the "nearby entities attract" rule).

### Consumes

- **From `seed_utils`:** all of it.
- **From `entities.inl`:** `Active*` structs, per-family `*Config`
  (SPAWN_CHANCE, MOOD_MULTIPLIER), per-family `*Prop` (property
  indices), per-family tier tables.
- **From `mood.inl`:** mood-related state.
- **From `terrain_cpu.inl`:** terrain height, footprint queries.
- **From `entity_types.inl` (the mid-block include):**
  `EntityInstance`, `EntityFamilyTraits`, `EntityFamilyAdapter`,
  `SpawnGateOutput`.
- **From `state.hpp`:** all `Dim::MAX_*` capacities.

### Exposes

A wide surface — the helpers above, the queue types, the dispatch
loops. Most other modules in the project consume from spawn_engine.

### Almost owns

- **The mid-block `#include "modules/entity_types.inl"`** (line 2689).
  This is the **textbook example of a compilation-order constraint**:
  EntityQueueEntry's union member `EntityInstance` must be defined
  before the union, so entity_types.inl gets dragged in mid-spawn-engine.
  The Ch. 1 orientation map noted this; here we see why. **Not a
  leak — it's a structural fact of C++ unions.** Worth tagging,
  not fixing.

- **FAMILY_DISPATCH ownership.** The `FAMILY_DISPATCH[12]` table at
  cartridge.hpp:6234 is a hub used by `select_entities_for_patch`
  here. The table's *declaration* is in cartridge.hpp; its *callers*
  are here. Before entity_pipeline.inl, the table can't be populated
  (because the wrappers like `dispatch_select_palm_generic` are
  defined there). Three-place dependency: `spawn_engine` calls →
  table declared in `cartridge.hpp` → wrappers from `entity_pipeline`
  populate. Layering correct; constraint named in Ch. 1.

### Loose strands

**L1.** `// #define DIAG_ENTITY_LIFECYCLE` at line 1755 is a
**latent diagnostic** (P8): commented out, available for spawn/evict
debug. Same family as the `[DIAG:*]` stdout pattern noted in user
memories (compile-time guard needed before exhibition). Worth
documenting alongside any other diagnostic switches when the
exhibition guard discussion happens.

### Knots

None. The module is well-structured given the constraints.

The mid-block include is *unusual* but not a knot — it's the C++
language rule expressed as code. The closest thing to a knot is the
sheer breadth of the module (1184 lines, a dozen named sections).
Splitting it is an option but unwarranted: every section serves the
spawn pipeline, and the spawn pipeline is one thing.

### Patterns this module names

**P11 — "Templated active-array helper."** `run_spawn_preamble<ActiveT>`
takes any of the per-family `Active*` structs (because they all share
`.active`, `.patch_gx`, `.patch_gz`). One implementation, ten callers.
Same family as P10's "per-family vocabulary block" — recognizes that
families share structural shape, but applies it at the algorithm level.
Potentially extensible: any per-family bookkeeping helper that only
needs the shared idempotency fields can use this template.

### Extraction notes

The mid-block sandwich means spawn_engine.inl extracts as **two real
files** OR with the entity_types include preserved mid-extract.
Either:

1. Split spawn_engine into `spawn_engine_pre.inl` (everything before
   line 2689) + `spawn_engine_post.inl` (the queue types and
   dispatch), with `entity_types.inl` `#include`d between them in the
   spine.
2. Keep the mid-block `#include` literally — extract spawn_engine
   into one file that itself contains the entity_types include.

Option 1 is more honest about the structure but has more pieces.
Option 2 keeps the file count down and is what's effectively done
already. Either works mechanically. **Decision deferred — it's a
preference call, not a correctness one.**

---

## 12.E — `gallery` (lines 3893–5460)

The biggest banner-only block (~1567 lines). **A complete art system
in one place:** photographer (the snapshot capture cadence), terrain
paintings (outdoor), wall paintings (indoor), exhibitions (curated
groupings), authored image loading + staging. Bespoke pipeline like
ribbon and gol_zones.

### Owns

- **Photographer system:** capture cadence (trigger distance, burst
  count), the 8 ShotType tiers (PANORAMIC, ENVIRONMENTAL, MEDIUM,
  CLOSE_UP, PORTRAIT, BIRDS_EYE, LOW_ANGLE, CINEMATIC) with full
  parameter tables (distance, elevation, FOV, aspect, tracks_pawn,
  offsets, weight). Camera-to-pawn coupling.
- **Painting spawn:** painting size by canvas area, terrain placement
  rules, frame width and color, snapshot-to-painting transition.
- **Wall paintings:** indoor placement on wall geometry, mood-driven
  placement, authored vs. snapshot mix.
- **Exhibition / authored image staging:** image library, loading,
  GPU upload to texture array.
- **Gallery's bespoke pipeline:** `GallerySelection`, `GalleryPlacement`,
  select/place/commit functions, eviction.
- Runtime: `cpuPaintings_`, `activePaintingCount_`,
  `cpuWallPaintings_`, `wallFrameCount_`, photographer state machine.

### Consumes

- **From `seed_utils`:** all of it.
- **From `entities.inl`:** mood multipliers, palettes (paintings
  borrow the sandstone family).
- **From `spawn_engine`:** footprint registry, patch streaming hooks.
- **From `terrain_cpu`:** terrain height for painting placement.
- **From `mood.inl`:** wall painting placement triggered on mood
  entry (indoor moods).

### Exposes

`render_main_pass` (Ch. 10) calls `draw_wall_paintings`,
`draw_gallery_frames`. Per-frame photographer tick called from
`cartridge.hpp::update()`. Bespoke select/place/commit hooks in
FAMILY_DISPATCH-style dispatch (gallery is family 11 of 12).

### Almost owns

- **Image library and GPU texture array.** Authored images load from
  disk, staging buffers pack them into the GPU texture array.
  Gallery owns the CPU side; `state.hpp` owns the GPU layout
  (texture count, format, mip chain).

- **The dual outdoor/indoor pipeline.** Gallery serves two roles —
  painting-on-terrain (outdoor) and painting-on-wall (indoor) — with
  shared infrastructure (image loading, frame rendering) and
  divergent spawn paths. Not a leak; **two named sub-systems sharing
  resources**. Comment-as-policy at the file header explicitly names
  this division.

### Loose strands

**L1.** ShotType `ENVIRONMENTAL` has weight 0.01 — effectively
disabled at the authoring level. Comment doesn't say why. Either
it's intentionally rare for a reason that should be named, or it's
**latent infrastructure (P8)** for a future authoring change. Add a
header comment naming the choice.

**L2.** Photographer config separates `PhotographerCaptureConfig`
(snapshot cadence) from `GalleryConfig` (painting placement) in
explicit parallel structures. **Good** — same concern-separation
pattern that orbs.inl uses for "player state vs. mood state" (P3).
Worth tagging as a P3 instance.

### Knots

None visible at the structural-read level; gallery is well-organized
internally. Without doing a full deep-dive (which the chapter's scope
doesn't ask for), the structure looks solid.

### Patterns this module names

No new patterns. Two existing patterns named here:
- P7-variant "complete subsystem in one block" (same as gol_zones).
- P3 "concern-separation in named substructures" (`PhotographerCaptureConfig`
  vs `GalleryConfig`, mirroring orbs.inl's player-state vs mood-state).

### Extraction notes

Gallery extracts cleanly because its dependencies are all upstream
and its consumers are all downstream — same compile-order constraints
as gol_zones. Mechanical move, no semantic change.

---

## 12.F — Cross-module discussion: the entities.inl extraction

This is the discussion all the cross-chapter knots in the entity
domain point at. Five things stack here:

1. **Ch. 4 entity:K2** — tier representation split between
   entities.inl (legacy compact: burial, segs, color_var,
   color_override) and entity_pipeline.inl (Gaussian sampling
   profiles).
2. **Ch. 8 orbs:D2** — `ORB_MOOD_TABLE` location.
3. **Ch. 9 cube three-home reframing** — cubes additionally have
   `CUBE_TIER_GAINS` in floaters.inl.
4. **Ch. 4 entity:D4** — defer to Ch. 12 (this discussion).
5. **World.wgsl audit findings** — the per-family blocks here have
   GPU-side counterparts (the `XxxProp` registries are mirrored as
   `[STATE:*]` tag declarations or as policy-mask declarations).

### What entities.inl extraction means structurally

Mechanically: move lines 628–1693 to `cartridges/the_board/modules/entities.inl`,
replace with `#include`. The file is bracketed by clean
boundaries: `seed_utils` ends above; `pawn_aura.inl` is included
below.

The real question is **what stays and what moves with it**.
Concretely:

- **Per-family vocabulary blocks (Ribbon, Arch, Column, Antenna,
  Palm, Cactus, Blade, Pyramid)** — definitely move. They're the
  reason this module exists.

- **Tier "extras" (legacy compact `XXX_TIERS[]` with burial / segs /
  color_var / color_override)** — debatable. Three options:

  - *Option A: Keep the split, document it.* Per-family extras live
    in entities.inl; sampling profiles live in entity_pipeline.inl.
    Adapter functions read from both (current state). Add a comment
    at each `XXX_TIERS[]` naming the partner table in entity_pipeline.
    Cost: zero. Benefit: the split is now explicit, not implicit.

  - *Option B: Merge into entity_pipeline.inl.* The compact tier
    extras become trait fields (`burial`, `segs_u`, `segs_v`,
    `color_variance`, `color_override`) on `EntityFamilyTraits`.
    Each family's row in `XXX_TIER_TABLE[]` gains the extras. Single
    home for tier data per family. Cost: one-time migration of ~8
    families, mechanical but tedious. Benefit: one tier representation
    per family.

  - *Option C: Merge into entities.inl.* The Gaussian sampling
    profiles move from entity_pipeline.inl into entities.inl
    alongside the legacy tier extras. Adapters in entity_pipeline.inl
    keep reading from entities.inl. Cost: comparable to Option B but
    backwards. Benefit: entity_pipeline.inl becomes pure machinery
    with zero data; entities.inl owns all per-family declarative
    state. Conceptually cleaner separation of "machinery" from
    "vocabulary."

  **Inclining toward Option C.** Reason: the cockpit pattern that
  agents/orbs/floaters all use puts vocabulary + state + lifecycle
  in one module. entity_pipeline.inl currently splits "machinery"
  from "vocabulary"; merging vocabulary into entities.inl gives the
  generic-family system the same shape as the cockpit modules, just
  decomposed across files (entities = vocabulary + state, entity_pipeline
  = machinery, FAMILY_DISPATCH in cartridge.hpp = wiring). This
  matches the codebase's existing successful pattern.

- **`ORB_MOOD_TABLE` rows.** Currently in cartridge.hpp at lines
  1702–1737 (per Ch. 1) — *not* inside the entities banner. **Two
  reasonable homes:**

  - With orbs.inl: the table is per-mood orb authoring data; orbs.inl
    owns everything else orb-related; the table is one struct away
    from `OrbMoodConfig` definition. **Inclining here.**
  - With entities.inl-as-data-hub: if entities.inl becomes "all
    per-family declarative state," ORB_MOOD_TABLE could land there
    too. Less compelling because orbs aren't an entity family in
    the FAMILY_DISPATCH sense.

  **Resolves orbs:D2 with "move to orbs.inl."**

- **Cube three-home reframing (Ch. 9).** With Option C, cubes have:
  - entities.inl: legacy compact tier + sampling profiles + Config +
    Prop + ActiveCube + cube color palette.
  - entity_pipeline.inl: cube adapter functions (write_active,
    write_gpu, post_commit are nullptr for non-pillared families).
  - floaters.inl: `CUBE_TIER_GAINS` (spring/drag/amp behavior
    multipliers).

  Floaters' tier gains are *behavior-domain*, not vocabulary-domain.
  They modify how a cube physically responds to forces, not what a
  cube *looks like* or *what tier-bucket it samples from*. Keeping
  them in floaters.inl is correct after the reframe. **No move
  needed.** The "three homes" framing was alarming; the actual
  mapping is "vocabulary in entities, machinery in entity_pipeline,
  domain-specific behavior gains in floaters" — three distinct
  concerns, each in the right place.

- **Harmonic ratio palettes (ribbon).** Specific to ribbon. Ribbon
  is bespoke (not in FAMILY_DISPATCH's generic). After extraction,
  the palettes naturally stay with the rest of the ribbon block in
  entities.inl. No special handling.

### When to do this

The extraction itself is mechanical (a Claude Code surgical pass).
The substantive work is the Option A/B/C choice. **Three approaches
to sequencing:**

- *Now:* settle Option A/B/C, then extract. Cleaner final shape, more
  upfront thinking.
- *In two passes:* extract under Option A first (zero behavioral
  change, just file moves); decide B vs C later. Lower risk, more
  motion overall.
- *After spine seam map (Ch. 15):* the spine seam map may surface
  additional concerns (e.g., the FAMILY_DISPATCH wiring's
  dependencies on entities.inl) that affect the choice.

**Recommendation:** *In two passes.* Extract under Option A
(documented split) as a pre-Ch. 15 prep — it's mechanical, it makes
the spine readable, and it doesn't commit to the Option B vs. C
choice. The choice itself can land after Ch. 15 when the full
picture is on the table.

### Summary of cross-cutting resolutions

| Open item | Resolution |
|-----------|-----------|
| Ch. 4 entity:K2 | Defer to extraction-time choice (A/B/C); current split is documented |
| Ch. 4 entity:D4 | **Resolved via two-pass approach:** extract Option A before Ch. 15 |
| Ch. 8 orbs:D2 | **Move ORB_MOOD_TABLE to orbs.inl** at extraction time |
| Ch. 9 cube three-home | **Reframed:** three concerns, each in the right place; no move needed |
| world.wgsl audit floaters:L4 | Independent of extraction; small tag/static_assert pass |

### Proposed tags (cross-module)

```
Near cartridge.hpp:1689 (cpu mirror of GPU pyramid instances):
  // SEAM[entities:K1] tier representation split with entity_pipeline.inl —
  //   resolves at extraction time; see Ch. 12.F

Near cartridge.hpp:1702 (ORB_MOOD_TABLE definitions):
  // SEAM[orbs:D2] move to orbs.inl at extraction time (12.F resolution)

Near cartridge.hpp:2689 (#include "modules/entity_types.inl"):
  // SEAM[spawn_engine:structural] mid-block include — not a leak,
  //   the EntityInstance union member must be defined before the union
```

---

# Chapter 13 — specialized entity families inline in `cartridge.hpp`

Lines 2929–3347 of `cartridge.hpp`. Three blocks: **Sphere Entity System**
(~88 lines), **Cube Entity System** (~89 lines), **Ribbon Dispatch
Pipeline** (~237 lines). All three sit between the spawn_engine block
ends and the gol_zones banner begins.

This chapter does two jobs:

1. Document each block as a module sub-section (same shape as Ch. 12).
2. **Clarify the entity taxonomy** the seam map has been navigating
   imperfectly across chapters. With this chapter read, the correct
   inventory is:

   - **Generic-pipeline grounded families** (7): Arch, Column, Antenna,
     Palm, Cactus, Blade, Pyramid. Vocabulary in `entities.inl`,
     sampling profile in `entity_pipeline.inl`, machinery in
     `entity_pipeline.inl`.
   - **Generic-pipeline floater families** (2): Sphere, Cube. Vocabulary
     **here** (not in entities.inl), sampling profile in
     `entity_pipeline.inl`, machinery in `entity_pipeline.inl`. Plus
     cube-specific behavior gains in `floaters.inl`.
   - **Bespoke families** (3): Ribbon (vocabulary in `entities.inl`,
     bespoke pipeline **here**), GoL (vocabulary + bespoke pipeline
     in `gol_zones.inl` banner — Ch. 12.B), Gallery (vocabulary +
     bespoke pipeline in `gallery.inl` banner — Ch. 12.E).

   Total: 12 entity families in `FAMILY_DISPATCH[12]`, distributed
   across 6 home concepts. **The "where tier extras live" question
   from Ch. 4 K2 has different answers for different family classes.**

A correction first.

## A correction to Ch. 9 (cube three-tier-home)

Ch. 9 stated cubes have a `CUBE_TIERS[]` in `entities.inl`. **That was
wrong.** Verified by grep: `entities.inl` contains blocks for Ribbon,
Arch, Column, Antenna, Palm, Cactus, Blade, Pyramid — **no Sphere or
Cube**. Cubes have their `CUBE_TIERS[CUBE_TIER_COUNT]` here in the
specialized-families block (`cartridge.hpp:3043`), with a
**`CubeTierProfile`** struct that's distinct from the entity-pipeline
sampling profile (different fields: orbit_height, bob_amplitude,
spin_speed, aspect_y/z, face_variance — the floater physics shape).

The Ch. 9 reframing's *intent* was correct: cubes have three
concerns (vocabulary, sampling profile, behavior gains), each in the
right file. The *file inventory* was wrong — vocabulary lives here
(13), not in `entities.inl` (12.C). 12.F's resolution table needs
updating; that update lands at the end of this chapter.

The same applies to spheres: `SPHERE_TIERS[SPHERE_TIER_COUNT]` lives
here at line 2959, not in `entities.inl`.

---

## 13.A — Sphere Entity System (lines 2930–3017)

The vocabulary block for orbital spheres. Per the file comment:

> *"Orbital spheres. Rare, PGA motor-driven orbits around anchors.
> Slots 0 .. MAX_SPHERE_INSTANCES-1 in the shared floating entity
> buffer. GPU compute: update_sphere. Vertex shader: sphere_vs."*

### Owns

- **`FloatingEntityTierProfile`** struct (lines 2940–2956) — the
  shared per-tier shape for the floater family. 15+ fields covering
  body radius, orbit (radius/height/speed), spin (speed/tilt), bob
  (amplitude/period), aspect ratios, face variance, geometry/motion
  type, weight.
  
  **Naming nit:** declared "FloatingEntityTierProfile" but only
  consumed by spheres in this block. Cubes have their own
  `CubeTierProfile` (different fields). The "FloatingEntity" naming
  suggests shared-across-floaters; the actual usage is sphere-only.
  See L1.

- **`SPHERE_TIERS[2]`** — Sentinel (rare, slow), Anomaly (closer,
  faster). Tier-0 weight 0.65, tier-1 weight 0.35.

- **`SphereConfig`** — `SPAWN_CHANCE = 0.015f`, `MOOD_MULTIPLIER =
  {1, 1, 0, 0, 1, 0}` (suppressed in indoor and finite_outdoor_ref,
  same family as cubes/GoL), `POSITION_JITTER = 0.4f`.

- **`FloatingEntityProp`** — property index registry, range 100–126.
  Single registry shared with cubes (which use 130–156). Comment
  notes the sharing: "Seed source: tile_seed (shared with all
  families). Range: 100–126 (original floating range, preserved for
  seed stability)."

- **`ActiveFloater`** + `activeFloaters_[Dim::MAX_SPHERE_INSTANCES]`
  + `activeFloaterCount_`. Includes `last_alloc_time` race
  protection (see Almost owns).

### Consumes

- `MOOD_COUNT`, `tile_seed`.
- `entity_pipeline.inl`'s sphere adapter functions (`sphere_run_gate`,
  `sphere_compute_solid_half`, `sphere_compute_colors`, etc.).

### Almost owns

- **The `last_alloc_time` race-protection field.** Comment at lines
  3006–3010:
  > *"See ActiveCube::last_alloc_time — same race protection for
  > sphere slots. Spheres rarely evict in practice (orbital, anchored
  > at origin), but the readback path covers them uniformly so the
  > protection covers them uniformly too."*
  
  Same pattern as P5 (release-pending sentinel from Ch. 9
  floaters:P5): the GPU has fresher state than the CPU mirror; the
  race window between "slot allocated this frame" and "GPU readback
  reflects previous-frame state" is closed by a CPU-side timestamp.
  Worth tagging as another P5 instance.

### Loose strands

**L1.** `FloatingEntityTierProfile` is named as if shared but used
only for spheres. Cubes have `CubeTierProfile`. The naming suggests
an architectural intent (one tier shape for all floaters) that didn't
land — likely because spheres and cubes have genuinely different
physics needs (spheres orbit, cubes hover-bob). **Two reasonable
fixes:**
- Rename to `SphereTierProfile`, accept that spheres and cubes have
  separate shapes.
- Migrate cubes to use `FloatingEntityTierProfile` with the orbit
  fields zeroed (the comment at 2938 hints this is what was
  originally planned: "Reuses FloatingEntityTierProfile — orbit
  fields are meaningful, hover-bob fields are zero" — but cubes don't
  actually reuse it; `CubeTierProfile` has different fields).

The naming claims more than the code delivers. **Trivial fix: rename
to `SphereTierProfile`**, accept the divergence as the artistic
reality. Comment can note "Cubes have their own `CubeTierProfile` —
floater physics differs by motion shape."

### Knots

None.

### Patterns this module names

P10 (per-family vocabulary block). Same shape as the entries in
entities.inl (Ch. 12.C) but for floater entities. The template
applies; the file location differs.

P5 (release-pending sentinel) — the `last_alloc_time` race-protection
field is another instance of P5. Different mechanism (CPU timestamp
vs GPU sentinel value) but same structural intent: when CPU mirror
and GPU truth disagree about timing, encode the disagreement so the
later-arriving side knows to re-check.

---

## 13.B — Cube Entity System (lines 3020–3108)

The vocabulary block for hover-bob cubes. Per the file comment:

> *"Hover-bob monoliths. Colorful cubes/slabs floating above terrain.
> Slots 0 .. MAX_CUBE_INSTANCES-1 (buffer offset by CUBE_SLOT_OFFSET).
> GPU compute: update_cube. Vertex shader: monolith_vs."*

### Owns

- **`CubeTierProfile`** struct — distinct from FloatingEntityTierProfile.
  Lacks orbit_radius and orbit_speed (cubes don't orbit; they hover at
  fixed anchor). Has all the bob/spin/aspect/face_variance shared with
  spheres.

- **`CUBE_TIERS[4]`** — SmallCube (40%), MedCube (32%), LargeCube
  (20%), Monolith (8%).

- **`CubeConfig`** — `SPAWN_CHANCE = 0.060f` (4× spheres' 0.015),
  same mood gate.

- **`CubeEntityProp`** — property index registry, range 130–156
  (avoids sphere's 100–126).

- **`ActiveCube`** + `activeCubes_[Dim::MAX_CUBE_INSTANCES]` +
  `activeCubeCount_`. Includes `last_alloc_time` (same race
  protection as spheres) and `cx`, `cz` — CPU mirror of the cube's
  current anchor, captured at spawn so `corral_cubes` (floaters.inl
  Ch. 9) can read without GPU readback.

### Consumes

- `MOOD_COUNT`, `tile_seed`.
- `entity_pipeline.inl`'s cube adapter functions.
- `floaters.inl::CUBE_TIER_GAINS` for behavior dynamics (read by
  `cube_write_gpu` in entity_pipeline).

### Almost owns

- **The `cx`, `cz` mirror fields.** Used by `floaters.inl::corral_cubes`
  and `floaters.inl::toggle_cube_kite_mode` to read the cube's current
  anchor without GPU readback. The fields are *here* but the
  consumers are *there*. Same family as `agents.inl`'s
  `cpuAgents_[possessed_slot].pos_x/z` reads (resolved as
  `agents:D2` to migrate to `pawn_x()/pawn_z()` accessors). Worth
  tagging — the cube floaters could have analogous `cube_anchor(slot)`
  accessors when their machinery extracts.

### Loose strands

None new beyond what 13.A already named (the FloatingEntityTierProfile
naming question from L1 affects cubes by exclusion).

### Knots

None.

### Patterns

P10 again. Same template as 13.A.

---

## 13.C — Ribbon Dispatch Pipeline (lines 3110–3346)

**The bespoke pipeline for ribbons.** This is structurally distinct
from 13.A and 13.B — those are vocabulary blocks; this is *machinery*.
Ribbons are a generic-pipeline outsider: they don't use entity_pipeline's
EntityFamilyAdapter; they have their own select/place/commit functions
defined here.

The vocabulary for ribbons (RibbonTierProfile, RibbonProp, etc.) lives
in `entities.inl` (Ch. 12.C). The machinery lives here. **Ribbon's
split is along vocabulary-vs-machinery axis**, not the floater-physics
axis used by spheres/cubes.

### Owns

Three functions:

- **`fill_ribbon_selection_geometry(seed, tier_idx, terrain_est, sel)`**
  (3118–3181) — Gaussian-samples all ribbon parameters (cube count,
  size, height, lateral/vertical/twist amp/cycles/speed) using the
  `RibbonTierProfile` from entities.inl. Also picks color mode and
  base color. Shared between the dispatch pipeline and mood-5 forced
  spawn (called from `mood.inl::apply_mood`).

- **`select_ribbon_for_patch(gx, gz, sel)`** (3184–3234) — bespoke
  select. Tip-overlap idempotency (rejects if any active ribbon's
  near or far tip falls in the trigger patch), runs the spawn
  preamble (shared with generic families via spawn_engine), picks a
  tier with theme bias, fills geometry. Constrains orientation to
  point away from the pawn ±60°.

- **`place_ribbon_from_selection(sel, plan)`** (3237–3275) — bespoke
  place. Negotiates position via `negotiate_position`, copies
  selection fields into the placement, records bookkeeping.

- **`commit_ribbon(plan, trigger_gx, trigger_gz, queue)`** (3278–3344) —
  bespoke commit. Builds `GPURibbonState`, writes to CPU mirror
  (`ribbonStates_[]`), updates `activeRibbons_[]` with two-tip
  anchoring (near-tip = anchor, far-tip = anchor + length × direction),
  computes patch coordinates for both tips, logs the spawn.

### Consumes

- **Vocabulary from `entities.inl`:** `RIBBON_TIERS`, `RibbonProp`,
  `RibbonConfig`, `RIBBON_BASE_TIER_WEIGHTS`, `RIBBON_TIER_COUNT`,
  `RIBBON_MAX_LENGTH`, `RibbonSelection`, `RibbonPlacement`.
- **From `spawn_engine.inl`:** `run_spawn_preamble`,
  `select_tier_biased`, `negotiate_position`,
  `record_placement_bookkeeping`, `estimate_terrain_height`.
- **From `seed_utils`:** `cpu_hash_f`, `cpu_sample_gaussian`.
- **From `cartridge.hpp` core:** `currentSeconds_`, `pawnReadback_x_/z_`,
  `THEMES`, `PATCH_EXTENT`, `Dim::MAX_RIBBON_INSTANCES`.

### Exposes

- `select_ribbon_for_patch` — called from `FAMILY_DISPATCH[RIBBON].try_select`.
- `place_ribbon_from_selection` — called from `FAMILY_DISPATCH[RIBBON].try_place`.
- `commit_ribbon` — called from `FAMILY_DISPATCH[RIBBON].try_commit` AND
  from `mood.inl::apply_mood` for the mood-5 forced ribbon spawn.
- `fill_ribbon_selection_geometry` — shared helper, called from both
  paths.

### Almost owns

- **The mood-5 ribbon coupling.** `commit_ribbon` is called from two
  places: (1) the standard FAMILY_DISPATCH path during patch streaming,
  and (2) `mood.inl::apply_mood` for the mood-5 reference ribbon
  (anchored ribbon, not a streamed one). The dual entry point is
  flagged in `mood:K4` and `mood:L1`. **The bespoke pipeline doesn't
  cause this leak** — it's a property of mood-5 being a reference
  clone — but the dual entry point makes the bespoke commit function
  more public than the generic commits would be.

### Loose strands

**L1.** Ribbon-anchor logging is unconditional (line 3339). Same family
as the `[DIAG:*]` stdout pattern flagged in user memories (compile-time
guard needed before exhibition). Inventoried alongside other diagnostic
stdout sites for the exhibition guard discussion.

### Knots

None new. The mood-5 dual entry point is owned by `mood:K4` not by
ribbon machinery itself.

### Patterns

No new patterns. **The bespoke-machinery-here pattern matches the
gol_zones (12.B) and gallery (12.E) shape: when a family has machinery
that doesn't fit the generic pipeline, the machinery lives in its own
banner-only block.** Ribbon is unusual in that the *vocabulary* lives
elsewhere (entities.inl) while the *machinery* lives here. The other
two bespoke families (gol, gallery) keep both together.

This is a structural fact worth naming — see the cross-cutting
discussion below.

---

## 13.D — Cross-cutting: the entity taxonomy

The full picture, with this chapter read.

### The three family classes

| Class | Members | Vocabulary location | Machinery location | Notes |
|-------|---------|---------------------|--------------------|-------|
| **Generic-pipeline grounded** | Arch, Column, Antenna, Palm, Cactus, Blade, Pyramid (7) | entities.inl | entity_pipeline.inl | Tier extras + sampling profile in two homes (entities:K1) |
| **Generic-pipeline floater** | Sphere, Cube (2) | **here (Ch. 13.A, 13.B)** | entity_pipeline.inl | Cube has additional behavior gains in floaters.inl |
| **Bespoke** | Ribbon, GoL, Gallery (3) | mixed | mixed (see below) | No use of EntityFamilyAdapter |

For bespoke families, the vocabulary/machinery split varies:

| Family | Vocabulary | Machinery |
|--------|-----------|-----------|
| **Ribbon** | entities.inl (Ch. 12.C) | here (Ch. 13.C) |
| **GoL** | gol_zones.inl banner (Ch. 12.B) | gol_zones.inl banner (Ch. 12.B) |
| **Gallery** | gallery.inl banner (Ch. 12.E) | gallery.inl banner (Ch. 12.E) |

**Ribbon is structurally unique** in keeping vocabulary and machinery in
separate files. The other two bespoke families keep both together.

### What this means for the entities.inl extraction (12.F)

The 12.F discussion analyzed the extraction question for the seven
generic-pipeline grounded families and didn't fully account for the
floater families. With this chapter, the picture clarifies:

- **Generic-pipeline grounded families' tier extras** stay an
  entities.inl question. Three options (A/B/C) still apply.

- **Generic-pipeline floater families' vocabulary** (Sphere, Cube
  blocks here) **is its own extraction decision, parallel to
  entities.inl.** Could:
  - **Option α:** stay inlined in cartridge.hpp (current state)
  - **Option β:** extract to `cartridges/the_board/modules/floater_vocabulary.inl`
    (or merge into floaters.inl, which currently holds only the cube
    behavior layer)
  - **Option γ:** merge into entities.inl (treat all entity vocabulary
    as one home regardless of GPU buffer layout)

  **Inclining toward β with naming care** — adding sphere/cube
  vocabulary to `floaters.inl` would unify "everything about cube
  behavior + cube vocabulary + sphere vocabulary" in one file, but
  the file would then include both spheres (which floaters.inl
  currently disclaims: "Spheres do their own thing — analytical PGA
  orbit, no behavior layer") and the actual cube behavior layer.
  Worth a careful naming pass: maybe rename the existing floaters.inl
  to `cube_behaviors.inl`, then create a new `floater_vocabulary.inl`
  with sphere + cube vocabulary.

- **Ribbon's bespoke pipeline** here in Ch. 13.C **is its own
  extraction decision.** Could:
  - **Stay here** (current state)
  - **Move to ribbon.inl** (new banner-only module joining gol_zones,
    gallery)
  - **Move into entities.inl alongside the ribbon vocabulary** (unifies
    ribbon, but entities.inl becomes mixed vocabulary+machinery for
    one family while staying vocabulary-only for the other seven)

  **Inclining toward extraction to ribbon.inl** — matches the
  bespoke-family pattern (gol_zones, gallery each own their own block).
  Ribbon being uniquely split is the structural fact noted above; the
  extraction would normalize ribbon to the gol/gallery shape.

### Updates to the 12.F resolution table

Restating the resolutions with this chapter's correction:

| Open item | Resolution |
|-----------|-----------|
| Ch. 4 entity:K1 (was K2) | Defer to extraction-time choice (A/B/C); applies to **7 grounded families**, not all 9 |
| Ch. 4 entity:D4 | **Resolved via two-pass approach:** extract Option A (grounded vocabulary) before Ch. 15 |
| Ch. 8 orbs:D2 | **Move ORB_MOOD_TABLE to orbs.inl** at extraction time |
| Ch. 9 cube three-home | **Reframed correctly with corrected inventory:** vocabulary here (13.B), sampling profile in entity_pipeline.inl, behavior gains in floaters.inl. Three concerns, three files, each in the right place. |
| Ch. 13 floater vocabulary | **New decision (D-floater):** Option α/β/γ, inclining toward β with naming care |
| Ch. 13 ribbon machinery | **New decision (D-ribbon):** stay here / extract to ribbon.inl / merge into entities.inl, inclining toward `ribbon.inl` |
| world.wgsl audit floaters:L4 | Independent of extraction; small tag/static_assert pass |

### Patterns this chapter strengthens

**P10 (per-family vocabulary block)** — applied here for spheres
(13.A) and cubes (13.B), matching the eight applications in
entities.inl (Ch. 12.C). Total: **10 instances of P10** across the
generic-pipeline families. The template is genuinely codebase-wide.

**P5 (release-pending sentinel)** — `last_alloc_time` race protection
in `ActiveFloater` and `ActiveCube` is another instance, alongside
the `follow_pawn = 2u` sentinel in floaters.inl (Ch. 9). Same
structural intent: when CPU mirror and GPU truth disagree, encode
the disagreement so the later-arriving side knows to re-check.

**No new patterns named.** Ch. 13 sharpens the inventory rather than
adding categories.

### Proposed tags

```
File: cartridge.hpp

Near line 2940 (struct FloatingEntityTierProfile):
  // SEAM[sphere:L1] naming claims more than code delivers — used only
  //   for spheres; cubes have CubeTierProfile

Near line 2978 (struct FloatingEntityProp):
  // SEAM[sphere/cube:taxonomy] property-index registry shared with cubes
  //   (range 100-126 for sphere, 130-156 for cube — distinct seed bands)

Near line 3043 (CUBE_TIERS table):
  // SEAM[cube:taxonomy] vocabulary lives here, NOT in entities.inl —
  //   floater families parallel grounded families but in different file
  // (corrects Ch. 9 cube three-tier-home claim)

Near line 3093 (ActiveCube cx, cz fields):
  // SEAM[cube:agents-pattern] CPU mirror of GPU anchor for floaters.inl
  //   to read without readback — same family as agents:D2

Near line 3110 (Ribbon Dispatch Pipeline):
  // SEAM[ribbon:taxonomy] bespoke machinery here, vocabulary in
  //   entities.inl — uniquely split among bespoke families;
  //   candidate for ribbon.inl extraction (Ch. 13.D)

Near line 3339 (std::cout << "[Ribbon] SPAWN ...):
  // SEAM[ribbon:L1] unconditional stdout — exhibition guard candidate
```

---

# Chapter 14 — `state.hpp`, `renderer.hpp`, `world.wgsl`

The three "foreign files" of the cartridge — outside the `.inl` tour
proper. Together they define the **C++/GPU contract surface**:
state.hpp owns data contracts and capacities, renderer.hpp owns
pipeline creation and dispatch glue, world.wgsl owns everything that
runs on the GPU. The audit between Ch. 11 and Ch. 12 already
established substantial context for world.wgsl; this chapter does the
formal module-shape pass on all three, harvests new findings the
audit didn't surface, and lands the cross-cutting view of the
contract surface.

Three sub-sections plus a synthesis:
- 14.A — `state.hpp` (5764 lines)
- 14.B — `renderer.hpp` (2762 lines)
- 14.C — `world.wgsl` (11114 lines) — the deliberate seam map
- 14.D — Cross-cutting: the C++/WGSL contract surface

---

## 14.A — `state.hpp`

The CPU/GPU data contract. 5764 lines. One namespace `t7::the_board`,
ten labelled sections (S1–S10), one main class (`GPUState`).

### Owns

**Structural sections:**

| Section | Lines | Content |
|---------|-------|---------|
| S1 — DIMENSIONS | 64–232 | `namespace Dim` — grid sizes, mesh resolutions, buffer capacities. ~120 named constants. |
| S2 — IDLE | 234–272 | `namespace Idle` — default values for state initialization (~25 constants: amplitude scale, pawn position, camera defaults, sphere orbit, etc.) |
| S3 — COUPLING BITS | 275–320 | `namespace Coupling` — 21 bit-flag constants + 11 named groups (ALL, NONE, STIMULUS, DERIVED, etc.) |
| S4 — GPU STRUCTURES | 322–1428 | **49 GPU* structs**, all `alignas(16)`. The data contract proper. |
| S5–S10 — GPUState class | 1436–5762 | Buffers, textures, bind groups, samplers; identity (non-copyable); boot; member methods. |

**Top-of-file BINDING MAP** (lines 11–50): comprehensive table of all
GPU bindings 0/1/2/20/21/25/26/40/60/80/100/101/120/121/200/201/220/260/280/300/320/321/340/360/361 plus texture group bindings, with explicit "(reserved — formerly...)" annotations on bindings 20/21/24/40/40-45.

**The 49 GPU\* structs** are the C++ side of the data contract. Each
has a matching WGSL declaration in world.wgsl. Each tagged
`alignas(16)`. Field-for-field shape mismatch produces silent runtime
corruption (the WGSL side names this constraint explicitly — see
the world.wgsl audit's note about the agent registry).

### Consumes

- **From `analysis/analysis_signal.hpp`** — for the input signal type.
- **From WGSL** — every struct in S4 has a counterpart declaration in
  world.wgsl tagged `[STATE:*]`. The contract goes both ways: C++
  authors values and uploads; WGSL declares the read shape. Both
  must agree.

### Exposes

- `Dim::*` — every capacity constant for cartridge.hpp and modules.
- `Idle::*` — initial values for state init.
- `Coupling::*` — the bit registry.
- `GPUFooBar` structs — uploaded to GPU buffers throughout the
  cartridge.
- `GPUState` class — singleton owning every GPU resource. Used as
  `gpuState_` in cartridge.hpp.

### Almost owns

- **The C++/WGSL field-shape contract.** state.hpp declares the C++
  side; world.wgsl declares the WGSL side; **no automatic
  cross-validation**. The WGSL side names this with an explicit
  schema reminder for AgentBehaviorParams (audit, lines 631–672 of
  world.wgsl) — *"PAIRED DECLARATIONS — KEEP IN SYNC: AgentBehaviorParams
  ↔ GPUAgentBehaviorDef. Both struct shapes must match field-for-field.
  Field-order mismatches produce silent runtime corruption — no
  compile error."* The same constraint applies to all 49 structs.

- **The Coupling bit registry.** The C++ namespace `Coupling`
  (state.hpp:275–320) and the WGSL `COUPLING_*` constants
  (world.wgsl:1672–1696) are paired. The WGSL side documents reserved
  slots for legacy systems (audit findings); the C++ side keeps
  legacy aliases (`PAWN_TO_PROXIMITY_FIELD = PAWN_TO_FIELD_COLOR`)
  but **doesn't tag them as "reserved"** the way WGSL does. Same
  registry, slightly inconsistent annotation discipline.

- **Binding number assignments.** The BINDING MAP at lines 11–50 is
  the C++ side's documentation of what binds where; world.wgsl's
  `[BINDINGS:*]` blocks are the WGSL side. **No cross-validation;
  comment-as-policy on both sides.**

### Loose strands

**L1.** Coupling namespace mirrors WGSL `COUPLING_*` but doesn't
mirror the "reserved — legacy X" annotations. **Trivial**: walk the
~6 reserved slots, add the same `// (reserved — legacy X)` comment
that the WGSL side uses. Aligns annotation discipline across the
contract surface; no semantic change.

**L2.** S4's 49 GPU structs are sequenced roughly by topic (signal,
state, agent, camera, floater, ribbon, ground, ribbons, ...) but the
ordering isn't named. **Tag-only**: section comments could group the
structs by subsystem (Frame & Config / Terrain & Patches / Agents /
Floaters / Ribbons / Ground Architecture / Lighting / Gallery) so
new readers see the grouping. Or split S4 into S4.1, S4.2, etc.

**L3.** Comment typo at line 6: "textAures" (should be "textures").

### Knots

None.

The state-of-the-art on the contract-shape question is "comment-as-
policy on both sides plus discipline" — same posture as `agents:L2`
and the world.wgsl audit's framing. **Genuinely intentional, not a
leak.** A future `static_assert` cross-check (e.g., emitting a small
WGSL-readable file from the C++ struct definitions and parsing it on
WGSL load) is conceivable but heavy for the gain. Discipline is
working today.

### Patterns this chapter strengthens

P8 (latent infrastructure) — the BINDING MAP's "(reserved — formerly
X)" annotations are GPU-side P8. The C++ side has the *Idle*
namespace as another flavor: pre-authored default values for state
init, holding the line until something dynamic sets them.

### Proposed tags

```
File: state.hpp

Near line 11 (// ─── BINDING MAP ───):
  // SEAM[state:contract] BINDING MAP is the C++/WGSL boundary —
  //   matched on the WGSL side by [BINDINGS:*] tags; no cross-validation

Near line 275 (namespace Coupling):
  // SEAM[state:L1] mirror WGSL COUPLING_* annotation discipline —
  //   tag legacy aliases as "reserved" to match GPU-side practice

Near line 322 (S4 GPU STRUCTURES):
  // SEAM[state:contract] 49 GPU* structs — each paired with [STATE:*]
  //   declaration in world.wgsl; field-shape match enforced by discipline
```

---

## 14.B — `renderer.hpp`

The pipeline manager. 2762 lines. One class `Renderer`, three big init
methods, ~65 dispatch/draw methods. **A speaker between cartridge
state and the WebGPU API** — same role as render_passes.inl but at a
lower level: render_passes.inl orchestrates *what* dispatches happen
and *in what order*; renderer.hpp owns *how* each dispatch wires up.

### Owns

**Pipeline state (~65 pipelines):**
- 31 `wgpu::ComputePipeline` members (terrain config, agents, camera,
  sphere, cube, VP, terrain indices, patch heights/gradients/cells,
  ribbon rings, photographer VP, entity placement, frustum cull,
  pawn aura, orb init/dynamics/recolor/copy_prev, zone GoL
  sync/evolve/mesh_reset/mesh_gen/derive_params, pyramid/arch/column/
  palm/cactus/blade mesh gen).
- 34 `wgpu::RenderPipeline` members (pawn, sphere, monolith, ribbon,
  arch, column, pyramid, palm, cactus, blade, shell, gallery_frame,
  zone_extrusion, fade_overlay, orb, plus shadow variants).

**Three init phases:**
- `loadShader()` (line 1268) — reads world.wgsl from disk, creates
  the shader module.
- `createComputePipelines()` (line 1334) — instantiates all 31
  compute pipelines using the shared module.
- `createRenderPipelines()` (line 1861) — instantiates all 34 render
  pipelines.

**The dispatch surface:**
- `dispatch_*` methods (compute) — bind groups + workgroup counts +
  pipeline.SetPipeline + computePass.Dispatch.
- `draw_*` methods (render) — same shape for render passes:
  bindings + pipeline.SetPipeline + pass.Draw[Indexed[Indirect]].

### Consumes

- **From `state.hpp`:** every bind group shape, every buffer view,
  every texture format, every Dim:: capacity.
- **From `world.wgsl`:** the entry point names (`update_terrain_config`,
  `update_player_agent`, `pawn_vs`, `pawn_fs`, etc.) — string-matched
  in `pipelineDescriptor.compute.entryPoint = "..."`.
- **From WebGPU:** the API itself (Device, Queue, Pipeline, BindGroup,
  RenderPass, ComputePass).

### Exposes

- `init(device, ...)` — boot.
- `dispatch_*` and `draw_*` methods consumed by render_passes.inl
  (Ch. 10) and any other module that issues dispatches.
- `use_indirect_terrain()` — reports whether GPU frustum culling is
  enabled (used by render_passes:render_main_pass).
- `ribbon_ring_workgroups()` and similar workgroup-count helpers.

### Almost owns

- **The shader entry-point string contract.** `pipelineDescriptor.entryPoint
  = "update_player_agent"` here at the C++ side; `@compute @workgroup_size(1)
  fn update_player_agent()` over there in world.wgsl. **String-matched,
  no cross-validation.** Typo on either side produces a runtime
  pipeline creation error (caught at boot, not silent corruption — so
  less risky than the struct-shape contract). Comment-as-policy:
  none, just the entry point name's match.

  **Same family as the BINDING MAP question** — boundary contract
  with the GPU side, enforced by discipline. The boot-time error
  surface makes this less load-bearing than struct shapes.

### Loose strands

**L1.** With ~65 pipelines, the create methods are mechanical and
long but not tangled. The pattern is highly repetitive: each pipeline
declares its `BindGroupLayout`s, vertex/fragment state, depth-stencil
state, fragment color targets. **Could be a registry** — a table of
pipeline-creation specs walked by a single helper. Cost vs. benefit:
the table would be dense, the helpers would have to handle every
combination of options, and pipelines have small differences (some
with depth, some without; some with multiple color targets; some with
vertex pulling, some with vertex IDs). **Defer indefinitely** —
discipline is working; the file is mechanical but readable.

**L2.** `loadShader()` reads `world.wgsl` from a hardcoded path. If
that path moves, the build breaks. Worth a comment naming the
expected location (relative to the executable, presumably alongside
the cartridge directory). **Tag-only.**

### Knots

None.

### Patterns

**P7 (Speaker at end of chain) — second instance.** renderer.hpp
plays the same role as render_passes.inl with a different scope:
render_passes.inl is the *per-frame orchestrator* (what dispatches to
issue, in what order); renderer.hpp is the *pipeline owner* (how each
dispatch is wired). Together they form the C++ side of the GPU
boundary. Same shape; different scope.

### Proposed tags

```
File: renderer.hpp

Near line 1268 (bool loadShader()):
  // SEAM[renderer:L2] hardcoded world.wgsl path — document expected location

Near line 1334 (createComputePipelines):
  // SEAM[renderer:contract] entry-point names string-matched against
  //   world.wgsl @compute fn names; runtime error if mismatch
```

---

## 14.C — `world.wgsl`

The GPU side. 11114 lines. One file, nine sections (§1–§9), a
TUNING SURFACE DIRECTORY at the top, a SECTION MAP, and the most
self-organized file in the project (the audit established this).

This is the deliberate module-shape pass. The audit already covered
several specific findings; here we do the formal Owns / Consumes /
Exposes / Almost owns / Loose / Knots inventory across the whole
file.

### Owns

**Section structure:**

| § | Title | Approx lines | Content |
|---|-------|-------------|---------|
| §1 | FOUNDATIONS | ~570 | PGA algebra, trajectory primitives, coordinate systems, utilities, terrain height functions |
| §2 | STATE | ~720 | Struct declarations (`[STATE:*]` tags), constants, muting control, COUPLING_* registry |
| §3 | COUPLINGS | ~1100 | Signal/input/terrain/entity cross-wiring (`[COUPLING:source→target:aspect]` functions) |
| §4 | DYNAMICS | ~250 | PGA motor integration (pawn, camera, ribbon ring motors) |
| §5 | COMPOSITION | ~200 | 0D update logic split across compute entry points |
| §6 | RENDERING | ~3500 | VS/FS pairs for all entities, lighting, terrain, ribbon, shadows |
| §7 | COMPUTE | ~2800 | Compute kernels, bindings, GoL zones, pawn aura, agent kernels |
| §8 | GALLERY | ~700 | Photographer, terrain paintings, wall paintings |
| §9 | ENTITY MESH GEN | ~1300 | GPU-sovereign geometry: pyramid, arch, column, palm, cactus, blade mesh gen |

**Foundational primitives (§1):**
- PGA P(ℝ*₃,₀,₁) algebra implementation: `Motor`, `Line`, `Point`,
  `gp_mm`, `apply_motor`, `motor_translation`, `motor_rotation`, etc.
- **§1.2 TRAJECTORY PRIMITIVES** — `struct Trajectory` + `fn
  trajectory_release`. The abstraction the C++ side wants to mirror
  (musical:K2 / mood:K3 / pawn:K1).
- §1.3 coordinate systems (TERRAIN_MESH_N, PATCH_*).
- §1.4 utilities (quat math, hash, gaussian).
- §1.5 ground architecture — the policy-based dispatch system, with
  contributor/policy DAG documentation in comments.
- §1.6 terrain waves.

**State (§2):**
- 8 `[STATE:*]` tagged structs (FrameSignal, TerrainState, AgentState,
  CameraState, FloatingEntityState, RibbonState, PatchParams,
  DesignConfig).
- COUPLING_* bit registry (mirrored from C++ Coupling namespace).
- POLICY_*_MASK constants.
- Pipeline override constants (`USE_PATCH_INDIRECTION`).
- Tunable kernel constants for compute shaders.

**Couplings (§3):**
- 14+ named `coupling_*` functions tagged
  `[COUPLING:source→target:aspect]`.
- Including the canonical patterns:
  `coupling_signal_polyphony_to_terrain_amplitude(polyphony, traj, dt)
  → Trajectory` — this is what the C++ side will mirror at extraction.

**PGA dynamics (§4):**
- Motor integration for pawn, camera.
- `ribbon_ring_motor` (the function fixed in user-memories' PGA bug
  — `gp_mm(orient, trans)` correct order, line 4198).
- Per-ring composition for ribbons.

**Compute kernels (§7):**
- Per-entity update kernels (`update_terrain_config`, `update_player_agent`,
  `update_other_agents`, `update_camera`, `update_sphere`, `update_cube`,
  `compute_vp`, `compute_pawn_aura`, GoL zone kernels, orb kernels).
- Mesh gen kernels (§9, dispatched from the same compute infrastructure).
- Frustum cull kernel.
- Patch heightfield gen (two-pass).
- Photographer VP compute.

**Rendering (§6):**
- VS/FS pairs for each entity family (pawn, sphere, monolith, ribbon,
  arch, column, pyramid, palm, cactus, blade, shell).
- Shadow VS variants for each.
- Terrain VS (with patch indirection override) + FS.
- Lighting computation (`calc_directional_light`, point/spot light
  handling).
- Zone extrusion VS/FS.
- Gallery frame VS/FS.
- Fade overlay.

### Consumes

- **From the C++ side**, via uploaded buffers:
  - Every `[STATE:*]` struct's data (FrameSignal, TerrainState, ...).
  - Agent registry (AgentBehaviorParams × 10, AgentTierParams × 4).
  - Patch params, tile grid, solid instances, pier instances.
  - Light state, VP matrices, sun direction.
  - Per-family ground entries, pyramid instances, GoL zone configs,
    pawn aura cells, orb state.
- **From the WebGPU runtime:** standard library functions, shader
  override values set at pipeline creation.

### Exposes

- **All `@compute` entry points** — string-matched by renderer.hpp
  pipeline creation.
- **All `@vertex` and `@fragment` entry points** — string-matched by
  renderer.hpp render pipeline creation.
- **All struct shapes** for buffers — string-matched by C++ side.
- **All binding numbers** — string-matched by state.hpp BINDING MAP.

### Almost owns

The "almost owns" surface for world.wgsl is unusual: world.wgsl owns
*all GPU code*, but the **GPU code's correspondence to CPU intent**
is co-owned with the C++ side. Specifically:

- **The §1.2 Trajectory abstraction** (audit). The struct + helper
  exists here; the C++ side has no analog. **The end-of-tour resolution
  is to introduce a CPU-side mirror.** When that lands, world.wgsl
  remains the canonical declarer; the C++ side becomes the consumer
  copy.

- **The COUPLING_* bit registry** (audit). Mirrored on both sides;
  WGSL has reserved-slot annotations the C++ side lacks (state:L1).
  Closer to a leak than the contract-shape question because the
  registry semantics are "what bits mean what" rather than "what
  bytes go where."

- **Hardware mirror constants** (audit). `AGENT_EVICTION_RADIUS` here
  with `MUST match` comment naming agents.inl. `MODE_LATTICE_SPACING`
  here, but C++ side `gol_zones.inl` doesn't have a `MUST match`
  comment yet (gol_zones:L1). World.wgsl is the canonical declarer
  for hardware constants; CPU-side mirrors are the consumers.

- **The `[BINDINGS:*]` block** at §7. Documents which bindings each
  bind group expects; matched on the C++ side by state.hpp's BINDING
  MAP. World.wgsl is the canonical declarer for the binding numbers
  used in `@group(N) @binding(M)` clauses; the C++ side documents
  the same numbers in comments.

### Loose strands

**L1 (= floaters:L4).** `CUBE_BEHAVIOR_COUNT_WGSL` missing — already
on the backlog from the audit. World.wgsl side declares the three
`CUBE_BEHAVIOR_STATIONARY/CURLFIELD/PHASEWAVE` ids and three force
functions plus the dispatch switch, but no count constant. C++ side
has `CUBE_BEHAVIOR_COUNT = 3`. Add the parallel const + a `MUST match`
comment on both sides.

**L2.** `coupling_pawn_to_sun_vp` (line 2836) and CPU
`compute_sun_matrices` have **different constants** (GPU
ALTITUDE=250/HE=300; CPU altitude=300/half_extent=350). Two parallel
implementations of the same matrix, neither cites the other. Latent
because `compute_sun_matrices` is reframed as latent (render_passes:L1).
**Worth a comment in `coupling_pawn_to_sun_vp` naming the CPU
counterpart as the alternate entry path** so a future reader doesn't
update one and miss the other. Tag-only.

**L3.** §6 Rendering is ~3500 lines — the largest section. Per-entity
VS/FS pairs are mostly mechanical with small variations. Same
"could be a registry" pattern as renderer.hpp's pipeline creation
(L1). **Defer indefinitely** for the same reason: the variations are
real (some VSes use vertex pulling, some use vertex IDs, some have
shadow-only modes, some don't), and the registry would need to
encode every variation.

**L4.** §9 Entity Mesh Gen has **dedicated bind groups isolated from
terrain evaluation** (line 8361 commented as such) — likely because
mesh-gen kernels need a smaller bind group than the full terrain
suite (vertex stage 8-storage-buffer limit per user memories). **Not
a leak — intentional binding isolation for a hardware constraint.**
Worth tagging as another instance of intentional-specificity (the
same family as agents:L2 / orbs:L2 / FXC mirroring).

**L5.** §8 GALLERY uses photographer-specific bindings (~22-23) for
its frame VS (audit). Pattern is consistent with the rest of WGSL's
binding-isolation discipline.

### Knots

None at the structural level.

The world.wgsl audit established that world.wgsl is the **most
coherent file in the project** — TUNING SURFACE DIRECTORY,
SECTION MAP, per-function header comments, inline `[STATE:*]` /
`[COUPLING:*]` / `[BINDINGS:*]` tags, removal breadcrumbs. The
absence of structural knots reflects this. **Knots in the seam map
overall point at the C++ side's incomplete grasp of an architecture
the GPU side has more fully realized** (the audit's framing).

### Patterns this chapter recognizes

No new patterns. World.wgsl is the canonical home of every pattern
the seam map has named:
- P7 (Speaker at end of chain) — every kernel and VS/FS function.
- P8 (Latent infrastructure) — ~15 explicit `reserved` / `placeholder`
  / `future X` annotations.
- P10 (Per-family vocabulary block) — every entity family has its
  WGSL-side counterpart of struct + VS/FS + mesh gen kernel.
- Comment-as-policy ordering — fused-inline evaluation comments name
  what must stay in sync.
- Historical breadcrumbs — ~15 `// (X removed — Y)` comments
  preserved deliberately.

### Proposed tags

```
File: world.wgsl

Near line 178 (struct Trajectory):
  // SEAM[wgsl:foundation] §1.2 Trajectory primitives — canonical shape
  //   the C++ side will mirror to close musical:K2 / mood:K3 / pawn:K1

Near line 2836 (fn coupling_pawn_to_sun_vp):
  // SEAM[wgsl:L2] CPU compute_sun_matrices in render_passes.inl is the
  //   alternate entry path (latent — sun's musical expression awaits);
  //   keep these two implementations in sync if either changes

Near line 6146 (CUBE_BEHAVIOR_STATIONARY):
  // SEAM[wgsl:L1] add CUBE_BEHAVIOR_COUNT_WGSL = 3u to mirror C++;
  //   pattern in agents (line 680: AGENT_BEHAVIOR_COUNT_WGSL)

Near line 8361 (// ── Bindings (dedicated layout — isolated)):
  // SEAM[wgsl:L4] mesh-gen bindings isolated for VS 8-storage-buffer
  //   limit; intentional hardware specificity (P7-style isolation)
```

---

## 14.D — Cross-cutting: the C++/WGSL contract surface

The three files together expose a **boundary surface** with several
named concerns. Mapping them explicitly closes a gap the prior
chapters touched only by reference.

### The contract surface in five layers

**Layer 1: Buffer shapes.** state.hpp's 49 `GPU*` structs ↔
world.wgsl's `[STATE:*]` declarations. **Each pair must agree
field-by-field.** No automatic validation. The WGSL side has the
schema reminder for AgentBehaviorParams; the same constraint applies
to all 49 pairs, mostly without explicit reminders.

**Layer 2: Binding numbers.** state.hpp's BINDING MAP comment table
↔ world.wgsl's `@group(N) @binding(M)` clauses + `[BINDINGS:*]`
block. **String-matched at pipeline creation, runtime error if
mismatched.**

**Layer 3: Entry-point names.** renderer.hpp's
`pipelineDescriptor.entryPoint = "..."` strings ↔ world.wgsl's
`@compute @workgroup_size(...) fn name()` and `@vertex fn name()`
declarations. **Boot-time error if mismatched.** Less load-bearing
than Layer 1 because the failure is loud rather than silent.

**Layer 4: Coupling bit semantics.** state.hpp's `Coupling::*` ↔
world.wgsl's `COUPLING_*`. Same bit values, same semantic intent.
WGSL side has reserved-slot annotations the C++ side lacks
(state:L1). **Discipline-enforced; semantic drift would be silent.**

**Layer 5: Hardware mirror constants.** Specific values that must
agree because the GPU and CPU compute the same thing in two places.
`AGENT_EVICTION_RADIUS` (mirrored, named with `MUST match` on both
sides). `MODE_LATTICE_SPACING` (mirrored, no `MUST match` comment on
C++ side — gol_zones:L1). `CUBE_BEHAVIOR_COUNT` (one-sided —
floaters:L4). Lattice/grid constants from world.wgsl's TUNING
SURFACE DIRECTORY (some mirrored on C++ side, some not).

### Why this surface isn't a leak

It would be reasonable to view the five layers as one big "declarative
contract that isn't enforced" knot. The audit and Ch. 14 both rejected
this framing for the same reason: **the layers are deliberate
architectural boundaries between what the CPU owns (decisions,
spawning, mood transitions, input) and what the GPU owns
(transformations, geometry, lighting, integration).** The contract
surface IS the architecture; its absence would mean the GPU running
in a vacuum. Comment-as-policy, manual mirroring, and discipline
are the cost of having two computers cooperate.

The seam map's job is **not** to propose collapsing the contract
surface, but to make sure each layer is named, tagged, and visible
to future readers.

### What the seam map should add to make the surface explicit

The end-of-tour cleanup pass (after Ch. 15) should include:

1. **Tag every struct pair** in S4 of state.hpp with a comment
   pointing to the matching `[STATE:*]` in world.wgsl, mirroring the
   AgentBehaviorParams pattern.

2. **Walk the BINDING MAP** and add `[BINDINGS:*]` cross-references
   so readers can grep both ways.

3. **Resolve the registries layer (Layer 4):** add reserved-slot
   annotations to the C++ Coupling namespace matching WGSL's
   discipline.

4. **Walk the hardware mirrors (Layer 5):** every constant that's
   declared both sides gets a `MUST match` comment naming the partner.
   Today: agents:L2 has it. gol_zones:L1 doesn't. floaters:L4 needs
   adding the const itself plus the comment.

These four passes are mechanical; they make the contract surface
*explicit* without changing anything semantic. **Promotable to one
end-of-tour batch** — a small Claude Code surgical pass after Ch. 15.

### Open decisions

**D1.** Should the seam map propose any cross-validation tooling
(e.g., a small Python script that parses C++ struct definitions and
emits matching WGSL `static_assert`-equivalents)? *Inclining no.* The
discipline is working; the tooling cost outweighs the benefit at the
project's current scale. Re-evaluate if the contract surface grows
substantially or if a struct-shape mismatch ever causes a real
debugging session.

**D2.** Should the seam map include world.wgsl in the Tag conventions
section (Ch. 1 conventions)? **Yes** — already done in the
conventions update after the audit. The `[STATE:*]`, `[COUPLING:*]`,
`[BINDINGS:*]` tag families are mentioned. Worth making it explicit
that *the GPU side practiced inline tagging before the seam map
arrived*, and the seam map's `SEAM[...]` is the third tag family.

### Proposed cross-cutting tag

```
File: state.hpp

Near line 11 (BINDING MAP):
  // SEAM[contract:L1] BINDING MAP is half of the C++/WGSL contract;
  //   the other half is world.wgsl's [BINDINGS:*] blocks at §7.0
  //   The full contract surface is documented in seam map Ch. 14.D.
```

---

# Chapter 15 — `cartridge.hpp` (the spine)

The final chapter. **A harvest more than a discovery.** The structural
observations about cartridge.hpp's spine were named as they appeared
across prior chapters — when each module noticed it was leaking *into*
the spine. Ch. 15 collects them and views cartridge.hpp from its own
perspective.

The Ch. 1 orientation map gave the structural shell — file layout,
members, lifecycle surface, per-frame flow, dispatch hub. That work
isn't repeated here. Ch. 1 emitted no tags by design; Ch. 15 is where
the spine's tags land.

The shape: cartridge.hpp is **a 9163-line single-class file holding
the entire `t7::the_board::Cartridge`**. Eight `.inl` modules `#include`
into it. Five banner-only blocks live inline (Ch. 12.A–E). Three
specialized-family blocks live inline (Ch. 13.A–C). The remaining
~3147 lines are spine — declarations, lifecycle, per-frame
orchestration, FAMILY_DISPATCH wiring, transition state, patch
streaming infrastructure, and the integration code that ties
everything together.

## Owns

The spine genuinely owns:

- **The Cartridge class identity** — inherits `RenderCartridge`,
  exposes the bootstrap/render/update/teardown interface to the host
  application (`main.cpp`).

- **Lifecycle orchestration** — `initialize()` (line 7783),
  `init_renderer()` (line 7793), `init_patch_system()` (line 7475),
  the boot sequence that wires modules together. *This is correctly
  spine work.*

- **The per-frame phase sequence** — `update()` (line 7894) and
  `render()` (line 8266). *The orchestration of when things happen
  is correctly spine work; the per-frame work itself is mostly
  module work that's leaked here (see Almost owns).*

- **The mood transition state machine** —
  `TransitionPhase { IDLE, FADE_OUT, FADE_IN }`, `transitionTimer_`,
  `pendingDestination_`, `transitionFadeAlpha_`. The state machine
  spans cartridge.hpp (state declarations + per-frame advance) and
  mood.inl (the transition action `apply_mood`). *Correctly spine
  work* — the transition state is global, mood-independent
  infrastructure.

- **The FAMILY_DISPATCH wiring** at line 6234. The 12-row dispatch
  table that ties together generic-pipeline families, bespoke
  families, and the floater families. *Correctly spine work* — the
  table is the integration point; each row's body lives in the
  family's owning module (entity_pipeline.inl, gol_zones.inl,
  gallery.inl, the specialized-families block).

- **Patch streaming infrastructure** (lines 5461–5815) — deferred
  uploads, budgets, density field, `tileCache_`, the streaming state
  machine. *Correctly spine work* — patch streaming is the
  integration backbone; modules consume it but don't own pieces of
  it.

- **The `MOOD_TABLE` constant** (per Ch. 5 inventory) — the per-mood
  configuration array. *Correctly spine work* — mood.inl reads it,
  but the table is global integration data, not mood-internal state.

- **The lighting schemes / wall art / indoor lights state**
  declarations (lines 29–553 per Ch. 1 layout). Many are
  per-mood data tables. *Mixed* — some genuinely spine, some leaked
  (see Almost owns).

- **`initialize()`'s role as the integration point.** The boot
  sequence reads cartridge config, creates GPU resources, uploads
  initial buffers, and walks every module's init hook. *Correctly
  spine work.*

## Consumes

Every module that's been mapped consumes from the spine, and the
spine consumes from every module. **The spine is the integration
hub by definition.** Listing every consumption would duplicate every
prior chapter's *Consumes* section in reverse.

The structural facts worth naming:

- **Module includes are ordered.** seed_utils → entities → pawn_aura →
  ground_architecture → orbs (+ ORB_MOOD_TABLE) → agents →
  spawn_engine (+ entity_types mid-include) → specialized families
  → gol_zones → gallery → patch streaming → FAMILY_DISPATCH →
  entity_pipeline → musical → floaters → render_passes → mood →
  input. The order encodes the dependency graph (Ch. 1's
  observation).

- **Every `XXX_TIER_TABLE`, `XXX_TIERS`, `XXX_PROFILES` consumed by
  the spine is owned by an .inl** — the spine pulls them in as
  authored data via include order.

- **`gpuState_` is a member, used everywhere** — the spine owns the
  declaration; modules call accessors. Same with `device_`,
  `currentSeconds_`, `activeMood_`, etc.

## Exposes

The `Cartridge` class to the host application (`main.cpp`):

| Method | Purpose |
|--------|---------|
| `initialize(device)` | Boot — wire module init, create GPU resources |
| `update(signal, dt, ...)` | Per-frame logic update |
| `render(encoder, backbuffer, depth)` | Per-frame draw |
| `on_key_down`, `on_key_up`, `on_mouse_*`, `on_scroll` | Input forwarding (declared here, defined in input.inl) |
| `on_resize(width, height)` | Viewport change |

Also, by virtue of being one big class, **everything declared in any
.inl is technically a Cartridge member**. This is the C++-language
mechanism that lets the .inl modules call each other's members
without explicit dispatch — same-class membership is the wiring.
*Structural fact, not a leak.*

## Almost owns — the harvest

This is the chapter's main work: the leaks every prior module chapter
fed into the spine, organized by category.

### Almost owns A — per-frame ramp-in-spine (`update()`)

**The big one.** Multiple K-tags from prior chapters point at
`update()` as the home for ramps that should live in their owning
modules. Listed in source-order:

| Site | Lines (approx) | Source chapter | Module that should own it |
|------|----------------|----------------|---------------------------|
| 7 polyphony coupling sites | scattered through update() | musical:K2 | musical.inl `tick_musical_couplings()` |
| Aura presence ramp | 7924–7933 | pawn:K1 | pawn.inl (when extracted) |
| Effective aura height computation | 7937–7940 | pawn:K1 | pawn.inl |
| Per-mode intensity ramps | (multiple sites) | musical:K2 | musical.inl |
| Various "smooth toward target" patterns | scattered | (collective) | the owning module via tick functions |

**The unified resolution shape from the world.wgsl audit:** introduce
a CPU-side `Trajectory` primitive (mirror of WGSL §1.2) and a small
`exponential_release(t, goal, dt, rate)` helper. Then each module
gains a `tick_*_couplings()` function. cartridge.hpp::update()
becomes a sequence of named per-module tick calls instead of a
catch-all of inlined ramps.

**Open decision (D-trajectory):** introduce trajectory.inl as a
foundations module mirroring WGSL §1.2, OR add the primitive to
seed_utils.inl, OR declare it inline in cartridge.hpp's foundations
area. *Inclining toward a small `trajectory.inl`* — keeps the parallel
with WGSL §1.2 visible, and seed_utils is "library without state"
(P9), arguably wrong domain for what's effectively math + a state
struct.

### Almost owns B — per-mood-transition logic (`apply_mood`)

`mood.inl::apply_mood` is the integration point for mood transitions,
correctly. But within it, several concerns live that should be in
their owning modules:

| Concern | Lines | Source chapter | Resolution |
|---------|-------|----------------|------------|
| Per-mood-transition musical reset | 351–388 | mood:K3 | musical.inl `reset_musical_couplings()` |
| 12-concern linear sequence | 250-line apply_mood | mood:K2 | split into named sub-functions |
| Mood-5 ribbon forced spawn | 389–447 | mood:K4 / mood:L1 | profile flag, not magic mood ID |
| Indoor lights derivation | (within apply_mood) | (mood-internal) | stays in mood.inl |
| Wall painting placement | (within apply_mood) | gallery integration | stays — gallery.inl exposes the helper |

The K2 split (12 concerns → named sub-functions) is the structural
work; K3 (musical reset) and K4 (mood-5 ribbon) are leaks pointing
at musical.inl and the entity-tier system respectively.

### Almost owns C — member declarations leaking out

Some Cartridge members declared in cartridge.hpp would more naturally
live in their owning modules:

| Declaration | Lines | Source chapter | Resolution |
|-------------|-------|----------------|------------|
| `ORB_MOOD_TABLE` rows | 1702–1737 | orbs:D2 | **Resolved 12.F:** move to orbs.inl at extraction |
| `cubeKiteMode_`, `cubePawnOffset_[]` | declared in floaters | floaters | already in floaters.inl module-local |
| Aura state members | declared in cartridge.hpp | pawn:K1 | move to pawn.inl when extracted |
| `prevPolyphony_` | declared in cartridge.hpp | musical:K3 | move to musical.inl with K2 |
| Various "active*" arrays | declared in cartridge.hpp | per-family chapters | stay — the spine owns the arrays; modules manipulate them |

The pattern: state owned by *one module* tends to leak; state shared
across modules (like `active*_` arrays consumed by multiple commit
paths) correctly stays in the spine.

### Almost owns D — banner-only inline modules (Ch. 12 and 13)

The banner-only blocks in cartridge.hpp are the spine's heaviest
"almost owns" surface in raw line count:

| Block | Lines | Source chapter | Extraction target |
|-------|-------|----------------|-------------------|
| seed_utils | 554–626 | 12.A | seed_utils.inl |
| entities | 628–1693 | 12.C | entities.inl (Option A pre-Ch. 15, B/C later) |
| spawn_engine | 1744–2928 | 12.D | spawn_engine.inl + mid-include preserved |
| Sphere/Cube specialized families | 2929–3108 | 13.A, 13.B | floater_vocabulary.inl (D-floater inclining β) |
| Ribbon bespoke pipeline | 3110–3346 | 13.C | ribbon.inl (D-ribbon inclining yes) |
| gol_zones | 3349–3890 | 12.B | gol_zones.inl |
| gallery | 3893–5460 | 12.E | gallery.inl |

**~4400 lines** of inline content waiting for mechanical extraction.
After all extractions land, cartridge.hpp drops to roughly 4700 lines
— still substantial, but predominantly the genuine spine concerns.

### Almost owns E — magic-number checks in the spine

Magic numbers and bespoke checks scattered through the spine that
should be profile flags or registry lookups:

| Site | Lines | Source chapter | Resolution |
|------|-------|----------------|------------|
| Mood-5 ribbon trigger checks | scattered | mood:L1 | `MOOD_TABLE[mood].has_anchor_ribbon` flag |
| `cpuAgents_[0]` hardcoded slot in floaters | floaters L334, 455 | floaters:L1 | `pawn_x() / pawn_z()` accessors via pawn.inl |
| `cpuAgents_[0]` reads in other places | scattered | agents:D2 | same accessors |
| Various per-mood inline conditionals | scattered | mood:K1, mood:K2 | mood profile flags |

## Loose strands

Spine-specific strands not already attributed to module chapters:

**L1.** **`pawn_aura.inl` is included but is pure declarations** —
all behavior lives in cartridge.hpp::update() / render(). This is
the explicit case for `pawn.inl` extraction (pawn:K1). The current
`pawn_aura.inl` is misnamed — it's a state-only module with no verbs,
and the verbs live wherever they were when the seam map found them.
*Resolution: pawn.inl extraction (already on backlog).*

**L2.** **The `MOOD_TABLE` declaration site** in cartridge.hpp could
move to mood.inl. It's read by mood.inl's `apply_mood` and by
input.inl's mood-transition triggers; declared in cartridge.hpp for
historical reasons. *Trivial extraction* if mood.inl is touched at
end-of-tour.

**L3.** **`THEMES[]` table declaration** is in cartridge.hpp. Themes
are spawn_engine concerns (theme-biased tier selection). *Could move
to spawn_engine* at extraction time. Defer.

**L4.** **The phase table** noted in Ch. 1 — the per-frame ordering
documentation — is split between Ch. 1 prose and per-site comments.
A formal `// Per-frame phase X: <name> (Y lines)` comment block at
the top of `update()` would make the ordering grep-discoverable.
*Tag-only.*

**L5.** **`DIM_INDOOR_LIGHTS_INTENSITY_RAMP` and similar ramp
constants** declared in cartridge.hpp where mood machinery uses them.
Same family as L2 — declarations leaking out of their owning module.
Trivial extraction.

## Knots

**K1. The spine's update() is doing module work.**

This is the consolidated knot — every prior chapter's "ramp-in-spine"
finding (musical:K2, mood:K3, pawn:K1) plus several smaller ones
collapse into one pattern: `update()` is the de-facto home for
per-frame logic that doesn't have an owning module's tick function
yet.

**Resolution shape (from world.wgsl audit):** introduce CPU-side
`Trajectory` primitive (mirror of WGSL §1.2). Each module gains a
`tick_*_couplings()` function. `update()` becomes a sequence of
named per-module tick calls.

**Cost:** moderate — new primitive + ~5 modules gaining tick
functions + walk and replace ~15 inline ramp sites. Mechanical work
once the primitive exists.

**Impact:** closes 3 module knots (musical:K2, mood:K3, pawn:K1).
The spine's `update()` shrinks substantially and becomes
phase-orchestration only, which is its correct role.

**K2. The spine holds banner-only blocks that are entire other
modules waiting to extract.**

Five blocks (Ch. 12.A-E) plus three (Ch. 13.A-C). ~4400 lines of
inline content. Each extraction is mechanical given the dependency
order is preserved.

**Resolution shape:** end-of-tour batch — extract all banner-only
blocks in dependency order, with the per-block decisions already
made (Option A for entities at first, ribbon.inl, floater_vocabulary.inl
with naming care).

**Cost:** moderate — mostly mechanical file moves, but the entities
extraction has the Option A/B/C choice (12.F), the floater
vocabulary has the naming question (rename floaters.inl to
cube_behaviors.inl), and the ribbon extraction makes ribbon
structurally consistent with gol/gallery.

**Impact:** spine drops from 9163 to ~4700 lines. **The spine's
genuine concerns become legible without the inlined content
crowding them.**

**K3. The spine holds member declarations that belong to one
module.**

Less severe than K1 and K2, but a real organizational concern. Once
K2 extracts modules and pawn.inl lands, ~10–15 declarations migrate
to their owning files (ORB_MOOD_TABLE, prevPolyphony_, aura state,
mood machinery's local data, theme tables).

**Resolution shape:** alongside each module extraction, harvest the
declarations that belong with it.

**Cost:** trivial per declaration; easy to forget. Worth a checklist.

**Impact:** minor by itself; cumulative effect is that each module
becomes a closed loop (vocabulary + state + verbs all in one file).

## Patterns this chapter strengthens

No new patterns. The chapter recognizes:

- **P10 (per-family vocabulary block)** — eight inlined banner blocks
  exhibit the template (entities + sphere + cube + the bespoke
  trio). The spine is hosting more P10 instances than any single
  module.

- **P7 (Speaker at end of chain)** — `update()` and `render()` are
  P7 instances at the spine level, with the additional concern that
  they currently mix orchestration (correct) with module work
  (leaked).

- **The cockpit pattern (Ch. 7)** — `cartridge.hpp` *as a whole* is
  attempting to be a maximal cockpit. Where successful (FAMILY_DISPATCH
  wiring, lifecycle, transition state), it works; where it leaks
  (per-frame ramps, member declarations, banner-only blocks), the
  cockpit shape isn't right for the leaked content because the
  content has its own owning module that wants to host it.

## Open decisions

**D1 (D-trajectory).** Where does the CPU-side `Trajectory`
primitive live? Three options:
- **a)** `cartridges/the_board/modules/trajectory.inl` — new
  foundations module, parallel to WGSL §1.2.
- **b)** Add to `seed_utils.inl` — joins the existing library-without-
  state module.
- **c)** Declare inline in cartridge.hpp's foundations area.

*Inclining a)* — keeps parallel with WGSL §1.2 visible; primitive is
small enough that a dedicated file isn't overkill; sets up
"foundations" as a category for future shared abstractions.

**D2.** Do the banner-only extractions land before or after the
`Trajectory` introduction and the `tick_*_couplings()` work?

Two valid orderings:
- *Extract first, then add primitives:* gets the spine to a
  legible baseline, then introduces new abstractions in clean
  files.
- *Primitives first, then extract:* lets the per-module tick
  functions exist in their inline blocks before extraction, so
  extraction moves working code rather than inserting new
  functions at the same time.

*Inclining first ordering* — extraction is mechanical and reversible;
the primitive introduction is design work. Doing the mechanical pass
first means the design pass operates on the cleaner state.

**D3.** Does the seam map propose any **immediate-fix** items now,
before end-of-tour? Two candidates from earlier chapters:
- `floaters:L1` — the `cpuAgents_[0]` two-site bug. Visible in
  diagnostic features (F6/F7) under possession. Two-line fix.
- `floaters:L4` — `CUBE_BEHAVIOR_COUNT_WGSL` add + static_assert.

*Inclining yes for both* — they're small, surgical, low-risk; both
flagged as "immediate" in the backlog already. No reason to wait
for the broader cleanup.

## Proposed tags

```
File: cartridge.hpp

Near line 7894 (void update(...)):
  // SEAM[spine:K1] update() holds per-frame ramps that should live in
  //   their owning modules — see harvest in seam map Ch. 15.A
  //   Resolution: CPU-side Trajectory primitive + tick_*_couplings() per module

Near line 8266 (void render(...)):
  // SEAM[spine:K1-related] render() mixes orchestration (correct) with
  //   module-specific GPU uploads (leaked); same family as update()

Near line 7475 (void init_patch_system()):
  // SEAM[spine:owns] patch streaming infrastructure — genuinely spine work,
  //   no leak; modules consume but don't own

Near line 6234 (FAMILY_DISPATCH[...]):
  // SEAM[spine:owns] FAMILY_DISPATCH wiring — genuinely spine work;
  //   each row's body lives in the family's owning module

Near line 554 (// ── Seed Utilities ──):
  // SEAM[spine:K2] banner-only extraction queue:
  //   554-626 seed_utils, 628-1693 entities, 1744-2928 spawn_engine,
  //   2929-3108 sphere/cube, 3110-3346 ribbon, 3349-3890 gol_zones,
  //   3893-5460 gallery
```

---

## Closing — the resolution sequence

The seam map's purpose was to map before touching. The map is now
drawn. The cleanup order, ordered by risk × dependency:

**Phase 1 — Immediate fixes (small, surgical, low-risk).**

- Fix `floaters:L1` two-site bug (`cpuAgents_[0]` → `[possessed_slot]`).
- Add `CUBE_BEHAVIOR_COUNT_WGSL` to world.wgsl + static_assert
  (`floaters:L4`).
- Apply tag conventions update to source files (the SEAM[...] tags
  proposed throughout the seam map).

**Phase 2 — Banner-only extractions (mechanical).**

- `seed_utils.inl` (warmest warm-up).
- `gol_zones.inl`.
- `gallery.inl`.
- `entities.inl` Option A (vocabulary stays split with entity_pipeline,
  documented).
- `spawn_engine.inl` (with mid-include preserved).
- `floater_vocabulary.inl` (with rename of existing floaters.inl
  to `cube_behaviors.inl` per D-floater).
- `ribbon.inl` (with bespoke pipeline moved out of cartridge.hpp).
- Member declarations migrate alongside (ORB_MOOD_TABLE → orbs.inl,
  etc.).

**Phase 3 — Cleanup batch (cross-cutting).**

- Tag struct pairs in state.hpp S4 with WGSL `[STATE:*]` cross-refs.
- Walk BINDING MAP, add `[BINDINGS:*]` cross-refs.
- Add reserved-slot annotations to C++ Coupling namespace
  (`state:L1`).
- Walk hardware mirrors (`MUST match` comments where missing —
  `gol_zones:L1`, etc.).
- `request_mood_transition()` helper (`input:L1`).
- Various tag-only loose strands.

**Phase 4 — Trajectory primitive + tick functions (design + mechanical).**

- Introduce `trajectory.inl` mirroring WGSL §1.2 (D-trajectory:a).
- musical.inl gains `tick_musical_couplings()` and
  `reset_musical_couplings()`.
- mood.inl uses reset; the K2 split into named sub-functions.
- pawn.inl extraction (folds in pawn_aura.inl + pawn-related state +
  presence ramp).
- update() becomes a sequence of named tick calls; the inlined
  ramps disappear.

**Phase 5 — Open decisions (each its own design pass).**

- entities.inl Option B vs. C (after Phase 2 lets the structure
  settle).
- musical:K1 (MMODE_REGISTRY) — alongside the Ableton rewiring chat.
- musical:D2 (define `SourceId`) — alongside the rewiring chat.
- Other deferred decisions.

The seam map is complete. Phase 1 is small enough to act on at any
time; Phases 2–5 unlock progressively as each completes.

---

# Post-tour resolution log

This section records what landed after the tour and how the open
items resolved. Added as work happened.

## Phases 1–5 — landed via Claude Code sessions

All five phases of the resolution sequence completed cleanly:

- **Phase 1 (immediate fixes):** `spine:L1` (vestigial for-loop fixed),
  `spine:L2` (audit coverage documented).
- **Phase 2 (banner-only extractions):** all 5 banner-only blocks
  extracted into their own modules. cartridge.hpp halved
  (9163 → ~4235 lines). New modules: `seed_utils.inl`, `entities.inl`,
  `spawn_engine.inl`, `gol_zones.inl`, `gallery.inl`. Plus
  `floater_vocabulary.inl` (Sphere + Cube vocabulary), `ribbon.inl`
  (bespoke ribbon machinery extracted from cartridge.hpp). Module
  rename: `floaters.inl` → `cube_behaviors.inl` to reflect actual
  scope (cube behavior layer only).
- **Phase 3 (small-strand cleanup batch):** floaters:L1 (cpuAgents
  possession bug fixed), floaters:L4 / wgsl:L1 (CUBE_BEHAVIOR_COUNT
  mirrored both sides + static_assert), sphere:L1 (struct renamed,
  misleading comment removed), gol_zones:L1 (MUST match comment
  added), input:L1 (request_mood_transition helper extracted),
  mood:L1 (has_anchor_ribbon flag replaces magic-number check),
  state:L1 (Coupling reserved annotations), state:L3 (typo),
  agents:L1, gallery:L1, musical:L2, entities:L1, renderer:L2.
- **Phase 4 (Trajectory primitive + tick functions):** new
  `trajectory.inl` mirroring WGSL §1.2. `update()` reduced from a
  catch-all of inline ramps to phase-orchestration only. New
  functions: `tick_musical_couplings`, `reset_musical_couplings`
  in musical.inl; `tick_pawn_couplings` in new `pawn.inl`. The
  pawn.inl extraction landed (pawn_aura.inl renamed, presence ramp
  + state moved in).
- **Phase 5 (apply_mood split):** `mood:K2` resolved.
  `apply_mood` reduced from 217-line linear soup to 28-line
  orchestrator + 5 named sub-functions
  (`apply_mood_lighting`, `apply_mood_spot_lights`,
  `apply_mood_indoor_shell`, `apply_mood_band_motion`,
  `apply_mood_anchor_ribbon`).

Every knot the seam map's resolution sequence named is closed.

## Post-Phase-5 decisions and follow-ups

### entities:K1 — resolved with Option B (per-family TierRow structs)

Decided after Phase 5 settled. The two-table duplication for the 9
generic-pipeline families (7 grounded + 2 floater) collapses to a
single source of truth.

**The decision history is worth preserving:**

1. *Initial decision: Option C with converters.* Named struct in
   vocabulary files (entities.inl / floater_vocabulary.inl) as
   source; generic `*_TIER_TABLE` derived via `*_to_profile()`
   `constexpr` converter functions. The reasoning was that named
   structs are the artist-facing surface and should hold the source
   of truth.

2. *MSVC build failure — C2131.* The C+converter approach hit
   MSVC's restriction on calling class-static-member `constexpr`
   functions to initialize class-static `constexpr` arrays.
   Approach 2 (lift named structs out of the class to namespace
   scope) was considered as a fix.

3. *Re-evaluation prompted by the build error.* The previous "C is
   clearly better than B" framing was reactive to a flawed version
   of B (wide `EntityFamilyTraits` with all extras). A cleaner B
   shape — per-family struct holding both `TierProfile profile` and
   the family-specific extras, authored once in
   `entity_pipeline.inl` — yields fewer artifacts than C with
   converter, sharper conceptual split, and resolves the build
   issue not by working around it but by removing the duplication
   that required the converter in the first place.

4. *Final decision: Option B.* The named structs in entities.inl
   are deleted. Per-family `*TierRow` structs (`PyramidTierRow`,
   `ArchTierRow`, etc.) live in entity_pipeline.inl alongside the
   sampling machinery. A new `get_tier_profile` accessor on
   `EntityFamilyAdapter` provides per-family tier-profile access;
   `traits.tier_profiles` is removed.

**The naming-convention discovery:** `enum class PyramidTier` already
existed in entities.inl (with `OBELISK`, `TEMPLE`, `COLOSSUS`),
making `PyramidTier` unavailable as a struct name. Resolved by
naming the new struct `*TierRow` ("one row of the tier table") —
slightly clearer than `*Tier` would have been. Pattern applied
consistently across all 9 families.

**Cleaner conceptual split achieved:**

| File | Role |
|------|------|
| `entity_types.inl` | Generic types: `TierProfile`, `TierMuSigma`, `EntityFamilyTraits`, `EntityFamilyAdapter`, `EntityInstance` |
| `entity_pipeline.inl` | All per-family tier data (`*TierRow` structs + `*_TIERS` arrays) + sampling machinery + adapters |
| `entities.inl` | Per-family **non-sampling** vocabulary: color palettes, spawn configs, prop registries, runtime tracking, enum classes |
| `floater_vocabulary.inl` | Floater non-sampling vocabulary mirror (sphere + cube): spawn config, prop registries, runtime tracking, tier metadata (counts, weights, names) |

**Migration breadcrumbs are in source.** Every grounded-family
section in entities.inl has an explanatory comment naming where its
tier values went and why. Floater sections in floater_vocabulary.inl
do the same. The historical-breadcrumb pattern (Ch. 14 conventions)
applied proactively.

**No converter code, no second table.** The ~80 lines of mechanical
derivation that C-with-converter would have introduced don't exist.
Single declaration per family, same file as the machinery that
consumes it.

### Control surface direction

A future direction (post-K1, multiple Claude Code sessions over
time): introduce `control_surface.inl` as a project-wide module that
exposes every *tunable* parameter (not every constant — only the
ones an artist would meaningfully turn during creative iteration),
organized by visual concern, with explicit metadata about:

- **Scope:** spawn-time / per-frame / per-mood-transition.
- **Signal flow:** which downstream consumers read this value, what
  visible effect it has, where in the pipeline the change cascades.

The control surface would *not* invalidate per-module vocabulary
files — those become typed views over the control surface. The
control surface adds a single navigation point for "what can I tune
and what does it affect."

The K1 resolution is structured to be compatible with this direction:
the per-family `*TierRow` structs in entity_pipeline.inl are
themselves a proto-control-surface for sampling parameters. Each row
is a named, addressable container of tunable values; migration to a
project-wide control surface would extract from these rows into named
control-surface entries. The destination is the same as Option C
would have led to; the starting point is per-family structs in
entity_pipeline.inl rather than named structs in vocabulary files.

### Items still open (CC's report named these correctly)

- **musical:K1 / musical:D2** — `MMODE_REGISTRY` + `SourceId`
  abstraction. Paired with the Ableton rewiring conversation since
  the registry's design depends on what kinds of musical inputs we
  want to express. That conversation is the natural next step.
- **contract:cleanup** — bulk WGSL ↔ state.hpp `[STATE:*]`
  cross-ref tagging across all 49 GPU struct pairs. High cost / low
  signal per the seam map. Bundle when convenient; not urgent.
- **mood:L2/L3/L4** — small helpers (perimeter wall margin,
  candidate-spot helper, normalization site). Bundle when mood.inl
  gets touched again for substantive reasons.

---

# Tour complete

All 15 chapters drawn. The map is the document; the source-code tags
are the breadcrumbs. The next move — review, then surgery — happens
outside this document.

---

# Cross-chapter index

(Built incrementally as knots get filed.)

| Knot | Chapter | Depends on |
|------|---------|------------|
| musical:K1 | 2 | musical:D1 |
| musical:K2 | 2 | Chapter 15 (spine seam map) |
| musical:K3 | 2 | musical:K2 |
| ground:K1  | 3 | **reframed by world.wgsl audit** — not a leak, deliberate three-side architecture; downgraded to careful Almost-owns observation |
| ground:K2  | 3 | latent infrastructure (P8); fires when stub contributors gain implementations |
| entity:K1  | 4 | entity:D1 (encoding question; specificity is artistic) |
| entity:K2  | 4 | Chapter 12 (entities.inl extraction); revised by Ch. 9 — three homes for cube tiers, not two |
| entity:K3  | 4 | local resolution — small refactor |
| entity:K4  | 4 | structural fact, not a fixable seam |
| mood:K1    | 5 | mood:D1 (encoding question, surfaces with finite_outdoor) |
| mood:K2    | 5 | mood:D2 (pure structural split) |
| mood:K3    | 5 | musical:K2 (mirror — resolves jointly) |
| mood:K4    | 5 | mood:L1 + Ch. 13 (taxonomy of bespoke entities) |
| pawn:K1    | 6 | pawn:D1 (likely entangled with musical:K2 + mood:K3) |
| (agents:none) | 7 | — module is the model; no knots filed |
| (orbs:none)   | 8 | — module is the model at maximum scale |
| (floaters:none) | 9 | — well-structured; one bug (L1) and intentional placeholder content |
| (render_passes:none) | 10 | — procedural model; pure-verbs module shape |
| (input:none)  | 11 | — front-of-chain speaker; one local duplication (L1) |
| (seed_utils:none)  | 12.A | — pure math; the textbook library-without-state module |
| (gol_zones:none)   | 12.B | — complete subsystem; one C++/WGSL mirror to formalize (L1) |
| entities:K1   | 12.C | resolves at extraction-time choice (12.F Option A/B/C) |
| (spawn_engine:none) | 12.D | — well-structured given mid-block sandwich constraint |
| (gallery:none) | 12.E | — well-organized at structural level; deeper deep-dive deferred |
| (sphere:none) | 13.A | — vocabulary block; one naming nit (L1) |
| (cube:none)   | 13.B | — vocabulary block; pattern matches sphere |
| (ribbon:none) | 13.C | — bespoke pipeline; structural-uniqueness (vocab vs machinery split) |
| (state:none)    | 14.A | — data contract; intentional cross-side discipline |
| (renderer:none) | 14.B | — pipeline manager; pure mechanical creation |
| (wgsl:none)     | 14.C | — most coherent file in the project; canonical home of patterns |
| spine:K1        | 15 | **harvest:** update() ramps → CPU Trajectory primitive + tick_*_couplings() per module (closes musical:K2, mood:K3, pawn:K1) |
| spine:K2        | 15 | **harvest:** banner-only blocks → mechanical extraction batch (Phase 2 of resolution sequence) |
| spine:K3        | 15 | **harvest:** member declarations → migrate alongside module extractions (~10–15 declarations) |

# Recurring patterns

(Architectural patterns that surface across multiple chapters. Pattern
tags are referenced in proposed SEAM tags so cross-chapter readers can
find each instance.)

| Pattern | First named | Instances |
|---------|-------------|-----------|
| **Declarative contracts that aren't enforced** — multiple-place agreements with no cross-validation, only DAG closure. | Ch. 2 (musical 3 fragments) | musical:K1, mood:K2 (12 inline concerns). *(ground:K1 removed by world.wgsl audit — it's not three fragments of one thing, it's three concerns owned by three sides.)* |
| **Intentional specificity (artistic OR hardware)** — named, deliberate, the question is whether the *encoding* is clean. Specificity itself is never the problem. | Ch. 5 (vocabulary added to conventions) | entity:K1 (per-family rescale, artistic), mood:K1 (indoor/outdoor binary, artistic), agents:L2 (FXC mirror, hardware), orbs:L2 (field repurposing, layout), floaters tuning console (WGSL kernel constants documented but unmovable) |
| **Ramp-in-spine** — exponential trajectory `next = prev + (target - prev)·(1 - exp(-rate·dt))` living in `cartridge.hpp::update()` rather than the owning module. | Ch. 2 (musical mode intensities) | musical:K2, mood:K3 (per-transition reset), pawn:K1 (aura presence) |
| **Cockpit pattern** — TUNING CONSOLE → registries → CPU MIRROR → orchestration → diagnostics. Each section banner-titled. | Ch. 7 (named) | agents.inl (model, moderate scale), orbs.inl (model, max scale, owns GPU dispatch), floaters.inl (model, smaller scale, no self-owned dispatch) |
| **P1: Per-frame coupling decomposed into the module** — counter-example to ramp-in-spine. Module exposes `update_*_coupling(signal, dt, queue)`, smoothing lives inside. | Ch. 8 (orbs:P1) | orbs.inl `update_orb_coupling`. Target shape for musical/pawn/mood resets. |
| **P2: "0 = no opinion, use system default"** — mood-authored zero substituted with named tuning-console default at configure time. | Ch. 8 (orbs:P2) | orbs.inl `ORB_DEFAULT_*` + `eff()`/`passthrough()` lambdas. Likely useful in other mood-authored config sites. |
| **P3: Player state vs. mood state, explicit** — runtime state sub-grouped by ownership lifetime. | Ch. 8 (orbs:P3) | orbs.inl runtime state sub-groups (anchor/gesture vs. couplings). Probable analog in floaters (Ch. 9), specialized entities (Ch. 13). |
| **P4: Hygiene rows** — per-mood/per-domain registries declare entries even where the gate skips them, so future gate changes don't require parallel updates. | Ch. 9 (floaters:P4) | floaters.inl `CUBE_POPULATIONS` for indoor / finite_outdoor_ref moods. |
| **P5: Release-pending sentinel** — when CPU lacks accurate state, encode intent as a value the kernel reads next frame and acts on. Avoids CPU drift estimation and readback latency. Generalized: when CPU mirror and GPU truth disagree about timing/state, encode the disagreement so the later-arriving side knows to re-check. | Ch. 9 (floaters:P5) | floaters.inl `toggle_cube_kite_mode` OFF (`follow_pawn = 2u` GPU sentinel). **Ch. 13:** `ActiveFloater::last_alloc_time` and `ActiveCube::last_alloc_time` (CPU timestamp protecting freshly-allocated slots from previous-frame readback). Same intent, different mechanism. |
| **P6: Comment-as-policy ordering** — load-bearing call-site sequence with the dependency named only in the comment. Works at small scale, breaks at large scale. | Ch. 10 (render_passes:P6) | render_passes.inl `dispatch_compute` (player-first); orbs.inl `pack_tiers_` → `pack_flocking_` cooperation; mood.inl `apply_mood` 12-concern sequence (where it crosses from pattern into knot — see mood:K2). |
| **P7: Speaker at the end of the chain** — pure-verbs transformer: reads decisions from many places, writes commands to one downstream surface (GPU). No state, no registries. | Ch. 10 (render_passes:P7) | render_passes.inl (back-of-chain: cartridge state → GPU). input.inl (front-of-chain: OS events → cartridge commands; same shape, opposite flow direction). The two together form the cartridge's I/O translator pair. |
| **P8: Latent infrastructure** — code written ahead of the feature that will use it. Currently unreferenced, but the artist's note-to-self about what's coming. Distinct from dead code (a feature that came and went). Default to "latent" when in doubt; the cost of carrying is small, the cost of deleting a future foundation is large. | Ch. 10 conventions update; **world.wgsl audit: the pattern is already in active practice on the GPU side, named here only after the codebase had been doing it all along.** | C++ side: entity:L3 (`generic_compute_colors` — default for future non-exotic families), render_passes:L1 (`compute_sun_matrices` — sun's musical expression), floaters `behavior_amp_mult` field, `entities:L2` (PYRAMID_TIERS aspect_ratio defaults), `spawn_engine:L1` (`#define DIAG_ENTITY_LIFECYCLE`). WGSL side: ~15 explicit `reserved` / `placeholder` annotations including binding slot reservations (lines 4578, 1528–1529), COUPLING_* bits held for legacy systems (lines 1682–1690), `contrib_paintings_base_at` / `contrib_vegetation_base_at` placeholder stubs (lines 2241–2256, the `ground:K2` instances), `query_ground_celestial` (line 2735), `query_ground_walker_agent` (line 2714). |
| **P9: Library without state** — pure functions, no class members referenced, no domain assumptions. Distinct from P7 (Speaker at end of chain) which is a transformer at a boundary; P9 is a toolbox for everyone. The easiest module shape to extract — true zero-cost extraction with no compilation-order constraints. | Ch. 12.A (seed_utils) | seed_utils.inl. Pattern instances: cpu_hash, cpu_sample_gaussian, select_tier — all candidates for shared library extraction. |
| **P10: Per-family vocabulary block** — a repeated structural template applied per entity family: TierEnum / TierParams / TIERS table / Color palette / Config / Prop registry / Active tracking / runtime arrays. The codebase's textbook expression of "what a new entity family needs to declare." | Ch. 12.C (entities) | **10 total instances:** entities.inl: 8 (Ribbon, Arch, Column, Antenna, Palm, Cactus, Blade, Pyramid). Ch. 13: Sphere (13.A), Cube (13.B). Worth recognizing the template; not worth fighting it (Ch. 4 K1's "intentional specificity" framing applies). |
| **P11: Templated active-array helper** — one helper function templated on the family's `Active*` struct type, reused across all families that share idempotency fields (`.active`, `.patch_gx`, `.patch_gz`). | Ch. 12.D (spawn_engine) | spawn_engine.inl `run_spawn_preamble<ActiveT>`. Same family as P10 at the algorithm level: families share structural shape, helpers exploit the shape with templates. |

# Free-strand backlog

(Loose strands across all chapters. Pulled as a batch *after* the tour ends.)

| Strand | Chapter | Effort |
|--------|---------|--------|
| musical:L1 | 2 | trivial — delete + rename consumer |
| musical:L2 | 2 | trivial — promote constant |
| musical:L3 | 2 | conversational — pick canonical source first |
| musical:L4 | 2 | folds into K1 |
| ground:L1  | 3 | trivial if delete; depends on D1 if wire a consumer |
| ground:L2  | 3 | folds into K1 |
| entity:L1  | 4 | trivial — delete stale comment block |
| entity:L2  | 4 | folds into L1 |
| entity:L3  | 4 | tag-only — latent default path, name the intent |
| entity:L4  | 4 | medium — extract write_active boilerplate to helper |
| entity:L5  | 4 | medium — only if D3 favors macro/template |
| mood:L1    | 5 | trivial — add `has_anchor_ribbon` flag, replace magic number |
| mood:L2    | 5 | small — extract `compute_perimeter_wall_margin()` helper |
| mood:L3    | 5 | small — extract candidate-spot + shuffle helper |
| mood:L4    | 5 | trivial — pick canonical normalization site |
| pawn:L1    | 6 | trivial — verify intent + make const-ref or document |
| agents:L1  | 7 | trivial — fix stale PLAYER_SLOT comment |
| agents:L2  | 7 | tag-only — hardware mirror, comment is the enforcement |
| agents:L3  | 7 | trivial — add comment near declaration |
| orbs:L1    | 8 | tag-only — documented simplification, defer unless mood needs it |
| orbs:L2    | 8 | tag-only — comment is the enforcement |
| floaters:L1 | 9 | **immediate** — two-site bug, one-line each (`cpuAgents_[0]` → `[player_.possessed_slot]`); diagnostic but visible after possession |
| floaters:L2 | 9 | tag-only — defensive design, named pattern P4 |
| floaters:L3 | 9 | observation — placeholder content awaits character pass |
| floaters:L4 | 9 | small — add WGSL `CUBE_BEHAVIOR_COUNT_WGSL` const + C++ static_assert (mirror agents pattern) |
| render_passes:L1 | 10 | tag-only — latent (sun's musical expression awaits) |
| render_passes:L2 | 10 | defer — duplication tax, accept until family count grows |
| render_passes:L3 | 10 | trivial — cosmetic indentation cleanup, defer to free-strand batch |
| input:L1   | 11 | small — extract `request_mood_transition()` helper, removes ~70 lines of dup |
| input:L2   | 11 | downstream of floaters:D3 — no separate strand |
| gol_zones:L1 | 12.B | trivial — add `MUST match` comment to MODE_LATTICE_SPACING (mirror agents:L2 pattern) |
| entities:L1 | 12.C | trivial — document RibbonProp stride convention at site |
| entities:L2 | 12.C | tag-only — latent (P8), aspect ratio awaits character pass |
| spawn_engine:L1 | 12.D | tag-only — latent diagnostic, document with exhibition guard |
| gallery:L1 | 12.E | trivial — document why ENVIRONMENTAL weight is 0.01 |
| gallery:L2 | 12.E | tag-only — recognize as P3 instance |
| sphere:L1  | 13.A | small — rename `FloatingEntityTierProfile` to `SphereTierProfile` + comment naming the cube divergence |
| ribbon:L1  | 13.C | tag-only — exhibition guard candidate (alongside other diagnostic stdouts) |
| state:L1   | 14.A | trivial — mirror WGSL "(reserved — legacy X)" annotations on C++ Coupling namespace |
| state:L2   | 14.A | tag-only — group S4's 49 GPU structs by subsystem with section comments |
| state:L3   | 14.A | trivial — fix "textAures" typo at line 6 |
| renderer:L1 | 14.B | defer — pipeline-creation registry possible but not warranted |
| renderer:L2 | 14.B | tag-only — document hardcoded world.wgsl path |
| wgsl:L1 (= floaters:L4) | 14.C | small — add CUBE_BEHAVIOR_COUNT_WGSL + MUST match comments (already on floaters backlog) |
| wgsl:L2    | 14.C | tag-only — comment naming compute_sun_matrices as alternate path |
| wgsl:L3    | 14.C | defer — §6 RENDERING registry possible but not warranted (~3500 lines mechanical) |
| wgsl:L4    | 14.C | tag-only — recognize §9 mesh-gen binding isolation as intentional hardware specificity |
| wgsl:L5    | 14.C | tag-only — gallery photographer-bindings are consistent with discipline |
| contract:cleanup | 14.D | medium — end-of-tour batch: tag struct pairs, walk BINDING MAP, mirror reserved annotations, walk hardware mirrors |
| spine:L1   | 15 | folds into pawn:K1 — pawn_aura.inl is state-only awaiting pawn.inl extraction |
| spine:L2   | 15 | trivial — MOOD_TABLE declaration could move to mood.inl |
| spine:L3   | 15 | trivial — THEMES[] declaration could move to spawn_engine.inl |
| spine:L4   | 15 | tag-only — formal phase-table comment block at top of update() |
| spine:L5   | 15 | trivial — ramp constants declared in spine where modules use them |

# Open decisions backlog

| Decision | Chapter | Resolves |
|----------|---------|----------|
| musical:D1 | 2 | end-of-tour |
| musical:D2 | 2 | after Chapter 15 |
| ground:D1  | 3 | after Chapter 15 |
| ground:D2  | 3 | likely separate future project — flagged not committed |
| entity:D1  | 4 | after Chapter 12 |
| entity:D2  | 4 | **reframed Ch. 10** — latent, tag with intent |
| entity:D3  | 4 | end-of-tour (likely "no") |
| entity:D4  | 4 | re-evaluate after Chapter 12 |
| mood:D1    | 5 | when finite_outdoor design lands |
| mood:D2    | 5 | end-of-tour or surgical Claude Code pass |
| pawn:D1    | 6 | end-of-tour (sequencing call with musical:K2 + mood:K3) |
| pawn:D2    | 6 | at extraction time |
| pawn:D3    | 6 | at extraction mechanics |
| agents:D1  | 7 | **resolved** — try_possess_nearest stays in agents.inl (closes Ch. 6 D2) |
| agents:D2  | 7 | end-of-tour (folds into pawn.inl extraction) |
| orbs:D1    | 8 | only act if a mood needs differentiation |
| orbs:D2    | 8 | resolves with Chapter 12 (entities.inl decision) |
| floaters:D1 | 9 | **immediate small fix recommended** — two-site `cpuAgents_[0]` → `[possessed_slot]` |
| floaters:D2 | 9 | when artist authors character; no structural change needed |
| floaters:D3 | 9 | deferred indefinitely — header comment names the discrepancy |
| render_passes:D1 | 10 | **reframed** — latent, tag with intent |
| render_passes:D2 | 10 | accept until family growth or bug pressure |
| render_passes:D3 | 10 | folds into end-of-tour free-strand batch |
| input:D1   | 11 | small Claude Code surgical pass; site question deferred (mood.inl vs input.inl) |
| input:D2   | 11 | resolves with musical:K1 |
| entities:D1 (Option A/B/C) | 12.F | **two-pass approach recommended:** Option A pre-Ch. 15 (mechanical extract); Option B vs C decided after Ch. 15 |
| orbs:D2    | 12.F | **resolved** — move ORB_MOOD_TABLE to orbs.inl at extraction time |
| entity:D4  | 12.F | **resolved** — two-pass extraction approach (entity:D2 reframed earlier) |
| spawn_engine:D1 | 12.D | extraction-shape choice (split or keep mid-include) — preference call, defer to extraction time |
| floater-vocab:D | 13.D | **new** — α stay / β extract floater_vocabulary.inl / γ merge into entities.inl. Inclining β with naming care |
| ribbon-machinery:D | 13.D | **new** — stay here / extract ribbon.inl / merge into entities.inl. Inclining ribbon.inl |
| Ch. 9 cube-three-home | 13 | **resolved with corrected inventory:** vocabulary in 13.B (not entities.inl), sampling profile in entity_pipeline.inl, behavior gains in floaters.inl |
| contract:D1 | 14.D | cross-validation tooling — **deferred indefinitely**, discipline working at current scale |
| contract:D2 | 14.D | tag conventions for WGSL — **resolved**, already done in audit conventions update |
| spine:D1 (D-trajectory) | 15 | **inclining a)** — new `trajectory.inl` foundations module mirroring WGSL §1.2 |
| spine:D2 | 15 | extraction order: Phase 2 (banner-only) before Phase 4 (primitives + ticks) |
| spine:D3 | 15 | **inclining yes** — Phase 1 immediate fixes (floaters:L1, floaters:L4) ahead of broader cleanup |
