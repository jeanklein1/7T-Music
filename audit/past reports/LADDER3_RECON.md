# LADDER-3 RECON — L-MID BLOCKER FORECAST + THE K4 DOSSIER (read-only)

the_board only. Branch **MOD_1_ROSTER**, on the LADDER-2 close-out head
(`53e1e32`). Census only — no cuts, no source commits; this report is the
only artifact. Method: verify-first, counts with recipes, the tree rules.
This buys before LADDER-3 what LADDER-2 had to buy mid-arc (the MOOD_COUNT
prereq, the conversion-model fork).

---

## 1. THE GRADUATION FORECAST (the MOOD_COUNT-class census)

**Recipe:** per module, (a) grep the declaration side (state structs,
config tables, array sizes, parameter types) against the full in-class
symbol inventory of cartridge.hpp (constants / enums / types, lines
185–2655); (b) collect distinct `c->`/`self->` derefs plus ambient member
reads for the two ambient-style modules; (c) scan for anything the proven
pattern doesn't cover (`template <`, `[this]`, `virtual`, `friend`,
member-defined types consumed elsewhere).

### The graduations (two, total — L-mid's complete forecast)

| # | what graduates | to where | forced by | consumers today |
|---|---|---|---|---|
| G1 | **The six Mood IDs** (`MOOD_OPEN_DEFAULT` … `MOOD_FINITE_OUTDOOR_REF`) | `mood_constants.hpp` (already the mood vocabulary home) | **cube_behaviors** (CUBE_POPULATIONS rows + 6 per-row static_asserts) and **agents** (AGENT_POPULATIONS rows + per-row static_asserts) — DECLARATION side both | cube_behaviors (decl), agents (decl), input (body), mood (body), cartridge (body) |
| G2 | **InputState / KeyState / MouseState** (cartridge.hpp 185–208, instances `inputState_`/`keys_`/`mouse_`) | `input.hpp` — the module's own header becomes their first-scope home (they ARE input's state, currently living on the spine) | **input**'s conversion (D1: struct to header, instance stays at root) | input (writer), cartridge spine (`on_input` feed), ribbon (`c->inputState_` body read — carries) |

Nothing else. Every other declaration side resolves from what already
exists at file scope: `Dim::*`, state.hpp GPU types, `mood_constants.hpp`
(MOOD_COUNT, PortalDestination), the converted headers, and module-own
types.

### Per-module rows

| module | lines | (a) declaration blockers | (b) impl sizing (keyhole/ambient) | grade |
|---|---|---|---|---|
| **gol_zones** | 624 | **NONE** — GoLZoneState/GoLState use Dim:: + module-own AlgorithmType + GPUZoneDeriveRequestArray (state.hpp); its one decl constant reads `Dim::PATCH_EXTENT` directly | keyhole ×10: gpuState_, renderer_, device_, mood_state_, world_state_, time_state_, tileCache_ + spine services (check_position, record_placement_bookkeeping, register_footprint) | **near-verbatim** — 5/6 fns take `(GoLState&, …)`; the 6th is the `(Cartridge*, …)` placer |
| **render_passes** | 802 | **NONE** — no state struct, no tables; pure functions | **THE HEAVY RETROFIT**: ~240 ambient reads over 11 members (gpuState_ ×148, renderer_ ×41, entities_state_ ×25, world_state_ ×12, ribbon/gol/gallery/orbs/mood states, clearColor_, cpuSpotLights_); every fn is an ambient member (`render_main_pass`, `dispatch_compute`, …) called from the spine's `render()` override | **D3 retrofit, real** — fns gain the keyhole; largest mechanical diff of L-mid |
| **input** | 350 | **G2** (its own state lives on the spine); Mood-ID uses are BODY-side (key dispatch) — no decl blocker | ambient ×~41 over 7 members (pawn_state_ ×13, player_ ×7, world_state_ ×6, orbs/cube_behaviors/agent states, gpuState_); fns are ambient members (`on_key_down`/`on_key_up`/`on_mouse_move`) called from the `on_input` override | **D3 retrofit, small** + G2. NOTE: GLFW_KEY_* macros are TU-provided (glfw3.h via the harness), not module-included — unchanged by conversion, recorded for awareness |
| **cube_behaviors** | 545 | **G1** (CUBE_POPULATIONS + its 6 static_asserts name the Mood IDs) — all else module-own or file-scope (ActiveCube already graduated at LADDER-2 c0) | keyhole ×4: agent_state_, gpuState_, player_, time_state_ | **near-verbatim after G1** — 7/10 fns take `(CubeBehaviorsState&, …)`; the 3 others are pure helpers |
| **gallery** | 1889 | **NONE** — PhotographerState/GalleryState build from module-own types (SnapshotStagingRecord, AuthoredStagingRecord, PendingPromotion, GalleryCenter, PendingSnapshot, MAX_PROMOTIONS_PER_FRAME) + Dim:: + GPUPaintingSlot (state.hpp) + std (`<random>` mt19937, `<vector>`, `<string>` — header include notes) | keyhole ×12: gpuState_, renderer_, mood_state_, world_state_, player_, ribbon_state_ (cross-read), clearColor_, sunDirection_, tileCache_ + the 3 spawn services | **near-verbatim, largest module** — 13/19 fns take `(GalleryState&, …)`; the rest are const-readers/helpers/the placer. PhotographerState has member fns (uniform/gaussian on its own rng) — struct-local, moves whole |
| **agents** | 986 | **G1** (AGENT_POPULATIONS + per-row static_asserts) — AgentState itself is clean (GPUAgentState from state.hpp, module-own AGENT_OVERRIDE_NONE); the GPU_AGENT_*_COUNT lockstep asserts check state.hpp constants (file-scope ✓) | keyhole ×5: gpuState_, player_, time_state_, world_state_, **transitionPhase_** (in-class member — BODY-side read through the complete type, no blocker) | **near-verbatim after G1** — 7/10 fns take `(AgentState&, …)` |
| **ribbon** | 1421 | **NONE** — RibbonState builds from module-own (ActiveRibbon, RibbonHead, MAX_RIBBON_INSTANCES) + GPURibbonState (state.hpp). BOM FILE — encoding law applies (mirror deltas byte-identical with the_chord; the constitution's mirrored-module clause makes ribbon a SPECIAL rung: its conversion diverges the mirror unless the_chord converts in step — flagged for the LADDER-3 handoff to rule) | keyhole ×14: gpuState_, player_, time_state_, inputState_, visual_canvas_ + the 4 resolved dst handles, spine terrain services (estimate_terrain_height, terrain_tile_warm, negotiate_position), record_placement_bookkeeping, **run_spawn_preamble** (member template — callable through the complete type post-class; the pattern covers it) | **near-verbatim** — 6/14 take `(RibbonState&, …)`; the other 8 are const-readers (head_pose/head_frame/…), the wander/history helpers, and the placer. THEMES/PATCH_EXTENT reads are body-side |

### (c) The third-corollary scan — NONE found

Recipe: grep `template <` / `[this]` / `virtual` / `friend` across the
seven; inspect every hit. Findings: two local `[&]` lambdas (gallery's
manifest sort comparator, ribbon's median helper) — plain locals, no
`this` capture at namespace scope, no issue. No module defines a member
template (ribbon *calls* one — `run_spawn_preamble<ActiveT>`, P11, owned
by spawn_engine — which a post-class impl invokes through the complete
type exactly like any member; the existing pattern covers it). No virtual,
no friend. The ambient member functions of render_passes/input are NOT a
new corollary: they are non-virtual members called from the spine's
virtual overrides (`render()`, `on_input()`), and the keyhole retrofit —
the D3 path the LADDER-2 charter already priced — converts them.

### Proposed conversion order (proposal only — Jean rules)

Charter listing: cube_behaviors, gol_zones, gallery, ribbon, agents,
input, render_passes. **The census argues for:**

1. **gol_zones** — the cleanest rung (zero blockers, disciplined sigs);
   proves nothing new, lands momentum.
2. **agents** — carries **G1** (the Mood-ID graduation rides its commit).
3. **cube_behaviors** — consumes G1; its state already holds the c0 cube
   array.
4. **gallery** — big but disciplined; after it, every STATE the render
   pass reads is header-owned.
5. **ribbon** — pending the MIRROR ruling above (or deferred to a paired
   the_chord stage).
6. **input** — carries **G2**; small retrofit.
7. **render_passes** — LAST: the heavy retrofit touches every subsystem;
   converting it after the states it reads are all file-scope keeps its
   diff purely mechanical (`X_state_` → `c->X_state_`).

Rationale: blockers land exactly once each (G1 at #2, G2 at #6); the two
retrofit-heavy rungs sit last, after their dependencies stop moving.

---

## 2. THE K4 DOSSIER (mood's boundary — map only, no design)

mood.inl: 1354 lines, class-body include, the largest unconverted module.
**The structural headline for marination:** mood has NO MoodModuleState of
its own — its working state lives spread across SPINE members
(`mood_state_` [MoodState, cartridge.hpp 248], `transitionPhase_`,
`pendingDestination_`, `sunDirection_`, `cpuSpotLights_`,
`cpuPortalArray_`), and the transition MACHINE (phase advance, fade
timers, teardown call, back-portal bookkeeping) runs in cartridge.hpp's
update/render, not in mood.inl. The module is doors + appliers; the spine
is the engine.

### (a) Foreign-state writes (module → member, shape) — recipe: grep
assignment/mutation into non-mood state across mood.inl

| line(s) | target | shape |
|---|---|---|
| 668 | `gol_state_.mood_allowed` | **request-flag** (mood gate, read by spawn detection) |
| 669 | `pawn_state_.aura_enabled = false` | **request-flag** (conditional disallow) |
| 525, 1299 | `entities_state_.lights_dirty` | **request-flag** (set true in apply path; cleared in upload_lights) |
| 949–1005 | `entities_state_.arches[slot].*` (≈24 fields), `arch_count++`, `arch_mesh_gen_pending` | **direct mutation** — force_spawn_portal_at writes an arch in place (bypasses FAMILY_DISPATCH; the ROSTER R3 finding; K4's request-channel candidate) |
| 940/949 | `write_pier(queue, …)` ×2 | **direct mutation** via spine service (portal feet) |
| 650, 649 | `ribbon_state_.rendered_slot = 0` + `gpuState_.upload_ribbon(gpu[0])` | **direct mutation + config push** (anchor-ribbon apply) |
| 678 | `configure_orbs(orbs_state_, this, …)` | **config push** through the converted module's own door ✓ |
| 863 | `place_wall_paintings(gallery_state_, this, …)` | **config push** through gallery's door |
| 1340–1341 | `pendingDestination_`, `transitionPhase_ = FADE_OUT` | **request-flag** (the transition trigger — spine members) |
| 496–498, 505 | `sunDirection_[0..2]`, `gpuState_.set_sun_direction` | **direct mutation of spine member + config push** (the sun author's seat) |
| 278, 452, 476, 532 | `cpuSpotLights_` (reset/count) | **direct mutation of spine member** (the lights CPU mirror) |
| 523, 534/547, 567/569, 585/587, 695, 861, 1004, 1292, 1313–1319 | `gpuState_.set_terrain_amp_ceiling / set_mute_coupling / shell mesh / ceiling_height / arch mesh params / portal array / directional+point+spot lights` | **config push** (the apply surface) |

Reverse boundary (who writes mood's spine state from outside):
`mood_state_.portals_dirty` (entity_pipeline arch-evict + cartridge ×2),
and the transition machine's own fields (`transition_timer/fade_alpha`,
`back_portal_*`) written by cartridge.hpp's update — the machine is
spine-resident today.

### (b) apply_mood fan-out + MoodProfile reads

`apply_mood(mood, queue)` (1 external caller: the spine's TEARDOWN phase;
plus boot path) → sets `mood_state_.active`, gates gol/pawn_aura, then:

| consumer | MoodProfile fields read |
|---|---|
| `apply_mood_lighting` | sun_direction (×12 total in file), sun_color, sun_intensity, sun_ambient, clear_color, indoor (amp ceiling, coupling mute) |
| `apply_mood_spot_lights` (ROSTER-gated) | indoor, ceiling_height, ceiling_type (via derive_indoor_lights) |
| `apply_mood_indoor_shell` (ROSTER-gated) | indoor, ceiling_type, ceiling_height, wall_color, ceiling_color |
| `apply_mood_anchor_ribbon` | has_anchor_ribbon (via MOOD_TABLE row) |
| `configure_orbs` | (reads ORB_MOOD_TABLE, not MoodProfile) |
| `request_mood_transition` | finite, finite_radius_min/max (as `mp.`) |

External MoodProfile/MOOD_TABLE consumers beyond mood.inl (the pull, see
d): entity_pipeline (indoor + ceiling_height for indoor rescale; a
MOOD_TABLE row at arch-evict), spawn_engine (indoor, with finite_mode),
cartridge.hpp (indoor ×2, the machine).

**Census find:** `MoodProfile.fog_density` / `fog_color` have **zero
readers anywhere** (fog is canvas-coupled via the fog.density/fog.color
pipes). Declared-only authoring fields — LATENT[unused] candidates for
the ledger; flag-don't-delete governs; listed in §3.

### (c) Mood's internals nothing else touches (converts trivially)

External-caller census (recipe: grep each mood.inl function across the
tree minus mood.inl): **zero external callers** for derive_indoor_lights,
generate_indoor_shell, clear_indoor_shell, force_spawn_portal_at,
force_spawn_finite_portals, and all four `apply_mood_*` sub-appliers.
Plus the private palettes stretch (mood.inl 102–267: INDOOR_WALL_PALETTES
+ the anchor tables — the class-access residual from the organs-public
fix) and `mood_name`. The externally-called surface is exactly SIX doors:
`apply_mood` (1), `request_mood_transition` (5 lines, input keys),
`force_spawn_back_portal` (1, the spine's regen), `upload_lights` (1, the
render tick), `upload_portal_array` (2), `mood_name` (1, logging).

### (d) The boundary drawing itself toward mood_constants.hpp

Already there: MOOD_COUNT, PortalDestination. Being pulled by existing
consumers (mapped, NOT moved):
1. **The six Mood IDs** — G1 (cube_behaviors/agents decl-side; L-mid
   forces this one regardless of K4).
2. **MoodProfile + CeilingType + MOOD_TABLE** — pulled by entity_pipeline
   and spawn_engine (both L-late rungs read MOOD_TABLE rows in bodies;
   body-side today, decl-side never — the pull matures only if a
   file-scope header ever needs a row, or when mood itself converts).
3. **PORTAL_COLORS / PORTAL_COLOR_BACK / PORTAL_DENSITY** — read only
   inside mood.inl bodies today; they follow whatever home K4 gives the
   portal door.
4. **TransitionPhase** — read by agents (body) and owned by the spine
   machine; it follows the machine, not the vocabulary, unless K4 rules
   otherwise.

---

## 3. HOUSEKEEPING SWEEP (list, don't fix — ledger-ready)

1. **Stale depends-notes naming converted modules' old forms** (3 files):
   `gallery.inl:54` ("Depends on: entities.inl, terrain_cpu.inl,
   seed_utils.inl" — entities/seed_utils now .hpp; entities.inl is
   impl-only), `gol_zones.inl:40` ("seed_utils.inl"), `ribbon.inl:59`
   ("seed_utils.inl"). Each self-heals at its module's conversion; or one
   comment-only sweep fixes all three.
2. **world.wgsl:12** mirror note names `modules/seed_utils.inl` —
   untouchable (scope guard); stands until a world.wgsl-touching arc.
3. **PAWN_HEIGHT_UNITS** — STATUS: LATENT[unused] standing (entities.hpp;
   zero callers; CPU mirror of WGSL PAWN_HEIGHT).
4. **NEW census find:** `MoodProfile.fog_density` + `fog_color` —
   declared, zero readers (fog rides the canvas pipes). LATENT[unused]
   candidates; flag at K4's rung or a comment-only sweep; never silently
   removed.
5. **input.inl GLFW macros are TU-provided** (glfw3.h arrives via the
   harness include order, not the module) — unchanged by conversion;
   input.hpp/.inl must not pretend otherwise when cut.
6. **Standing LATENT census** (tag-greppable): gate-a-shared ×14 (+3 doc
   echoes), policy-surface ×10, tile-activation ×2, complexity ×2,
   roster-split:photographer ×2, naming (ActiveFloater→ActiveSphere) ×2,
   unused ×1 (+ the two fog candidates above), name ×1. No orphans — every
   tag's home region still exists.
7. **Ribbon is a MIRRORED module** (constitution §0: mirrored-module
   deltas byte-identical; ribbon.inl carries a BOM) — its conversion
   either pairs with the_chord's or breaks the mirror convention;
   needs a ruling in the LADDER-3 handoff (flagged in its row above).
8. Arc drift inherited by future rungs: none found beyond the above — the
   `(void)param` parity additions and the orbs commit-message brace-count
   typo (89/89 + 53/53 actual, both balanced) are recorded here for the
   record and change nothing.

---

*Report ends. Nothing else moves; LADDER-3's handoff follows.*
