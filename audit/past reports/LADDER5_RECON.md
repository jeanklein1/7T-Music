# LADDER-5 RECON — THE HUBS (read-only)

Scope: spawn_engine.inl (1,054 L / 550 code), entity_pipeline.inl
(2,258 L / 1,672 code), and every consumer of what they export. No
cuts, no moves — this is the map. Recipes disclosed inline; counts are
grep-reproducible at this head.

---

## §1 — THE CONTRACT CENSUS

The file-scope half of the contract already stands in entity_types.hpp:
EntityFamilyTraits, EntityFamilyAdapter, EntityInstance, TierProfile,
TierParamDef, ColorPartDef, ParamDist, SpawnGateOutput. The buried half:

### 1a. PopFamily

**Definition:** spawn_engine.inl:692 — a 16-line `struct PopFamily`
of `static constexpr uint32_t` indices (PYRAMID=0 … GALLERY=11,
COUNT=12). Class-nested today because spawn_engine.inl is included
inside the class body (cartridge.hpp:462).

**Consumers** (occurrence counts; recipe: `grep -c PopFamily`):

| file | count | mode |
|---|---|---|
| cartridge.hpp | 48 | unqualified (in-class): FAMILY_DISPATCH sizing, theme arrays, MIN_SEPARATION, audit switch, record_entity calls, static_asserts |
| entity_pipeline.inl | 34 | unqualified (in-class): TRAITS family_id, run_gate args, dispatch commits |
| spawn_engine.inl | 18 | unqualified (in-class): definition + footprint/proximity tables + select loop |
| gol_zones.inl | 4 code + 2 banner | **qualified `Cartridge::PopFamily`** (post-class keyhole) |
| gallery.inl | 4 code + 2 banner | qualified |
| ribbon.inl | 3 code + 2 banner | qualified |
| roster.hpp | 17 | **comments only** — the literal-index switch mirrors it blind |
| gol_zones.hpp / gallery.hpp / ribbon.hpp | 1 each | banner reach-lists |

**The retirement it buys (roster.hpp's own seam-note):** graduate
PopFamily to file scope, visible before roster.hpp's switch. Forecast
edit set:

1. Move the 16-line struct to the file-scope home (below).
2. roster.hpp `family_enabled`: 12 literal `case 0:` labels become
   `case PopFamily::PYRAMID:` — the literal-index switch retires; the
   12 mirror comments die with it (~12 lines simplified).
3. cartridge.hpp:1359–1373: the PopFamily BINDING static_assert block
   + its 5-line comment **retires entirely** (~20 lines) — the enum and
   the switch now share one authority.
4. The 11 qualified sites in gol_zones/gallery/ribbon .inls drop
   `Cartridge::` (11 one-token edits) + 6 banner lines update.
   (`Cartridge::PopFamily` stops resolving once the struct leaves the
   class, so these edits are mandatory, not cosmetic.)
5. spawn_engine.inl / entity_pipeline.inl / cartridge.hpp unqualified
   uses resolve by namespace lookup unchanged — zero edits (the G1
   precedent, exactly).

**Landing options:** (i) **roster.hpp itself** — the family manifest
and the family indices become one document; family_enabled's switch
labels are self-binding; no new file; roster.hpp already reaches every
consumer (state.hpp and cartridge.hpp include it; module .inls sit in
the TU after it). (ii) The dispatch-contract header — keeps spawn
vocabulary together but adds a roster → contract dependency edge for
one struct. Recommendation: (i); the contract header can re-export by
inclusion if it wants the name locally.

### 1b. FAMILY_DISPATCH — row type, table, thunks

**Row type:** `struct FamilyDispatch` (cartridge.hpp:876–884) — seven
fields: try_select / try_place / try_commit / evict_slot /
prepare_mesh / dispatch_mesh (all plain function pointers taking
`Cartridge*`) + name. **Table:** `static constexpr
FAMILY_DISPATCH[PopFamily::COUNT]` (cartridge.hpp:1308–1348), twelve
rows; the antenna row reuses column's prepare/mesh pair — the weld
made visible in the table itself.

**Wrapper inventory (70 functions today):**

| class | count | where | body shape |
|---|---|---|---|
| generic select/place/commit | 27 (9 fam × 3) | entity_pipeline.inl, per family block | 2–3 lines: funnel into generic_select/place/commit + record_entity |
| bespoke select/place/commit | 9 (gol, gallery, ribbon) | cartridge.hpp 1089–1254 | 2–6 lines: delegate to module fns + record_entity |
| evict_slot | 12 | cartridge.hpp 922–1088 (generic nine) + 1124/1181/1255 (bespoke three) | **real lifecycle bodies** mutating owner organs (the §5 trespass) |
| prepare_mesh | 11 (antenna rides column) | cartridge.hpp | 1 line: `return prepare_X_mesh_gen(self->entities_state_, self, queue);` |
| dispatch_mesh | 11 (antenna rides column) | cartridge.hpp | 1 line: `self->renderer_.dispatch_X_mesh_gen(pass, self->gpuState_.X_group());` |

**Where the table should live:** the row type belongs in the contract
header (it needs EntityQueueEntry, PlacementEntry, and the Cartridge
fwd — all contract vocabulary once 1c graduates; keyhole.hpp supplies
the fwd). The TABLE cannot precede its wrappers: today the wrappers
are static members, so the table sits mid-class. Once wrappers are
file-scope functions **declared in their owners' headers** (pre-class)
and defined in their .inls (post-class), a `constexpr` table of their
addresses is legal at file scope — function addresses are link-time
constants; only declarations are needed at the table's definition
point. Two lawful homes then: file scope beside the machine (one
table, spawn_engine's loops read it by namespace lookup), or
row-per-owner composition (each family header exports its row; a zone
file aggregates). The first is the smaller step.

**Thunk-collapse forecast:**
- prepare_mesh (11): already one-line adapters to entities.inl
  preparers. Collapse = the module exports the keyhole-shaped
  signature the table wants (`bool(Cartridge*, Queue&)`) or the
  one-liners move to entities.inl beside their targets. Either way
  cartridge.hpp sheds ~33 lines.
- dispatch_mesh (11): one-line renderer dispatches — render-shaped;
  natural home is with the owners beside their write_gpu (or a
  render_passes-side ladder). ~33 lines.
- evict_slot (12): the §5 EVICTION THUNKS class. Bodies mutate owner
  organs (entities_state_, sphere_state_, cube_behaviors_state_,
  ribbon_state_, gol_state_, gallery_state_) + gpuState_ doors —
  eviction IS lifecycle, and lifecycle belongs to owners. Absorption:
  each owner exports `evict_<family>(Cartridge* c, uint32_t slot,
  Queue&)` (or State&-first with a one-line adapter). **Does the
  contract header arrive at the §5 retirement condition ("die as
  entities absorb their evictors")? Yes** — the condition's blocker is
  purely declarative: an owner header cannot today declare a function
  taking `EntityQueueEntry&` or matching the row signature, because
  those types are class-nested. The contract header (1c) is exactly
  the visibility that lets owners declare table-shaped functions.
  After absorption the 12 thunks die and the table points at owner
  exports directly.
- bespoke select/place/commit (9): already delegations; they follow
  their owners the same way (gol/gallery/ribbon .inls), each keeping
  its `record_entity` bookkeeping call via the keyhole.

Net forecast: DISPATCH WRAPPERS chapter (385 code lines) collapses to
the table + nothing, with bodies redistributed to owners. Report-only;
no design beyond what the table's own shape mandates.

### 1c. The rest of the buried half — disposition table

The primitive law governs: share functions and vocabulary freely,
never state.

| item | site | disposition | reason |
|---|---|---|---|
| EntityQueueEntry | spawn_engine.inl:947 | **contract header** | pure tagged-union vocabulary; all four union members (RibbonSelection, GoLSelection, GallerySelection, EntityInstance) are ALREADY file-scope (ribbon.hpp / gol_zones.hpp / gallery.hpp / entity_types.hpp) — the type graduates by moving 12 lines |
| PlacementEntry | spawn_engine.inl:967 | **contract header** | same shape, same reason (union members likewise file-scope) |
| entityQueue_, placementResults_ | spawn_engine.inl:959/979 | **machine** | std::vector STATE — never shared |
| FamilyDispatch row type | cartridge.hpp:876 | **contract header** | needs the two entries + keyhole fwd only |
| FAMILY_DISPATCH table | cartridge.hpp:1308 | machine-adjacent file scope (post-graduation) | see 1b |
| generic_select / generic_place / generic_commit | entity_pipeline.inl:191/271/302 | **stays with the machine** | they read spine state ambiently (mood_state_, MOOD_TABLE, world_state_.ground_entries_dirty) and call the machine services; they are the machine's verbs. Signatures go to the contract header only if/when the hubs convert (LADDER-5 proper) |
| run_spawn_preamble\<ActiveT\> | spawn_engine.inl:101 | **stays with the machine** | template over Active* arrays but reads mood_state_, tileCache_, active_theme_idx_, proximity tables — a function over spine state. 10 callers (9 adapters + ribbon) all reach it via the keyhole already |
| negotiate_position | spawn_engine.inl:175 | **stays with the machine** | reads world_state_, MOOD_TABLE, INDOOR_ENTITY_WALL_MARGIN; drives check/register on footprints_ |
| check_position / register_footprint / unregister_footprints_for_patch | spawn_engine.inl:533/553/565 | **stays with the machine** | accessors over footprints_[128] — state |
| record_placement_bookkeeping | spawn_engine.inl:247 | machine | documented vacant seam; travels with its callers |
| GroundFootprint + footprints_ | spawn_engine.inl:517/528 | machine | state |
| MIN_SEPARATION / PROXIMITY_* tables | cartridge.hpp:1746 / spawn_engine.inl:812–817 | vocabulary — eligible to ride PopFamily's graduation (indexed [PopFamily::COUNT]) or stay | constexpr spawn vocabulary consumed only by the machine; no urgency, but they are the natural second passengers |
| THEMES / PopulationTheme | cartridge.hpp:1418/1454 | **finding — see §4** | class-nested vocabulary read by all nine adapters (get_theme_tier_weights); the envelope tick beside it is machine state |

**Contract header forecast** (name per tree idiom — the dispatch
contract home): PopFamily (or included from roster.hpp), EntityQueueEntry,
PlacementEntry, FamilyDispatch. Include-position constraint: it must
follow ribbon.hpp / gol_zones.hpp / gallery.hpp / entity_types.hpp in
the cohort (the unions embed their Selection types) and precede the
class. That dependency is honest — the union IS the coupling between
the machine and the three bespoke subsystems.

---

## §2 — THE THREE CLEAN RECIPES (cactus, blade, palm)

Family blocks in entity_pipeline.inl (banner-to-banner):

| family | pipeline block | lines | entities.hpp vocabulary | adapter extras |
|---|---|---|---|---|
| BLADE | 321–517 | 197 | VOCABULARY: BLADE 485–544 (~60 L) | none — generic colors (Q24), no rescale, no post_commit |
| PALM | 518–750 | 233 | VOCABULARY: PALM 352–420 (~69 L) | custom compute_colors + apply_indoor_rescale (both pure over inst) |
| CACTUS | 751–931 | 181 | VOCABULARY: CACTUS 421–484 (~64 L) | none — generic colors, no rescale, no post_commit |

**Boundary census — what each touches beyond its own tables** (blade
shown; palm/cactus identical shape):

- `c->run_spawn_preamble(...)` — machine service (keyhole) ✓ lawful
- `c->entities_state_.blades / blade_count / blade_mesh_gen_pending` —
  its own organ (entities-owned) ✓
- `c->gpuState_.upload_blade_mesh_params_slot` — GPU organ door ✓
- `THEMES[theme_idx].tier_wt_blade` — **a Cartridge class-static**
  (the one buried read; becomes `Cartridge::THEMES` post-class per the
  c4 statics precedent, or graduates — §4)
- `PopFamily::BLADE` — 1a
- `BladeProp / BladeClusterConfig / BladeIdx / BLADE_TIER_COUNT` —
  entities.hpp, file scope ✓
- `Dim::* / GPUBladeClusterMeshParams` — state.hpp ✓
- contract types (TRAITS/ADAPTER/instance/gate) — entity_types.hpp ✓
- wrappers additionally: `self->generic_select/place/commit`,
  `self->find_patch` + `host->record_entity` — machine services via
  keyhole ✓

**"Clean" costs, in moved lines:** pipeline block + nothing else —
the vocabulary is already home in entities.hpp. Blade 197, cactus 181,
palm 233 (~611 total). Prerequisites: EntityQueueEntry/PlacementEntry
+ FamilyDispatch visible at file scope (1c) so the wrappers can be
declared in the owner's header; THEMES readable post-class
(`Cartridge::` qualification suffices — no graduation strictly
required); PopFamily (1a).

**Natural landing:** entities.hpp/.inl. The active arrays
(EntitiesState.blades/cacti/palms) already live there, the preparers
already live there, and the recipes write only their own EntitiesState
fields + their own GPU doors. A per-family home (blade.hpp/.inl)
would relocate vocabulary + recipe but CANNOT take the state —
EntitiesState is one organ; splitting it is a different, bigger ruling.
Recommendation: recipes land in entities.inl beside their preparers;
per-family homes deferred until (if ever) EntitiesState itself splits.

**Welded families — what welds them:**

- **COLUMN + ANTENNA (one block, 932–1409):** three welds. (1) Shared
  GPU mesh store: antennas write column mesh slots at
  `slot + Dim::ANTENNA_SLOT_OFFSET`; (2) shared index count:
  prepare_column_mesh_gen (entities.inl:78) scans BOTH columns[] and
  antennas[] to compute ONE set_column_index_count; (3) the
  FAMILY_DISPATCH antenna row reuses column's prepare/mesh pair. The
  shared FUNCTIONS are lawful; **the finding is the shared GPU
  resource** — two families co-own one mesh buffer + one index count.
  They relocate together or not at all.
- **ARCH (1975–2258):** welded to the world engine, not to a family —
  arch_post_commit writes two piers (c->write_pier) and calls
  c->mark_patches_for_regen. Also the LADDER-4 channel
  (force_spawn_portal_arch) sits in its owner. Clean-capable only if
  pier/regen services stay reachable via the keyhole (they are).
- **PYRAMID (1410–1609):** same class of weld — pyramid_post_commit
  calls c->mark_patches_for_regen (heightfield bake); write_active
  also mirrors into cpu_pyramids for the bake.
- **SPHERE (1610–1774) / CUBE (1775–1974):** cross-organ writes — the
  pipeline writes c->sphere_state_.activeFloaters_ and
  c->cube_behaviors_state_.activeCubes_, organs owned by OTHER modules
  (spheres.hpp / cube_behaviors). Lawful under the access law, but
  these recipes' natural landing is their owners, not entities.
- No hidden CPU-state weld among the clean three: cactus, blade, palm
  share only EntityState-shaped fields inside the one organ and the
  machine services everyone shares.

---

## §3 — THE SPINE CHAPTER MAP (feeds LADDER-6; no design)

cartridge.hpp proper: 2,423 code lines (4,011 total). The two hub
includes add 550 (spawn_engine) + 1,672 (entity_pipeline) code lines
into the class at lines 462 and 1306. Classes: CR = composition root,
WE = world-engine, DI = dispatch.

| chapter | lines | code | class | note |
|---|---|---|---|---|
| FILE PREAMBLE (banner, includes, class open) | 1–148 | 46 | CR | |
| COMPOSITION ROOT — MODULE STATE | 149–204 | 12 | CR | the organ instances |
| TIME STATE | 205–238 | 18 | CR | |
| MOOD STATE | 239–283 | 17 | CR | instance + doors; the machine is next chapter |
| PLAYER STATE | 284–322 | 13 | CR | |
| PORTAL & TRANSITION STATE MACHINE | 323–366 | 5 | WE | SEAM[spine:transitions]; small state, big banner |
| GPU READBACK + WORLDGEN | 367–412 | 6 | WE | P5 sentinels |
| UNIFIED PIER SYSTEM | 413–480 | 2 | WE | cpuPiers_ mirror; **spawn_engine.inl included here (462, +550 code)** |
| ACTIVE PATCH SYSTEM | 481–740 | 170 | WE | ActivePatch, find/evict, entity_refs, audit |
| DYNAMIC BUDGETS | 741–783 | 32 | WE | |
| TILE WORLD SYSTEM | 784–857 | 28 | WE | tileCache_ |
| FAMILY DISPATCH TABLE (row type) | 858–885 | 9 | DI | |
| DISPATCH WRAPPERS | 886–1374 | 385 | DI | bespoke + evict/prepare/mesh thunks; **entity_pipeline.inl included here (1306, +1,672 code)**; table at 1308 |
| POPULATION THEMES | 1375–1630 | 174 | WE | PopulationTheme + THEMES + envelope |
| TERRAIN TOKENS | 1631–1995 | 220 | WE | includes MIN_SEPARATION |
| TERRAIN TOKEN TICK + EMISSION | 1996–2070 | 48 | WE | |
| WORLD LIFECYCLE | 2071–2273 | 144 | WE | teardown/regen |
| PATCH SUBSYSTEM SETUP | 2274–2338 | 45 | WE | test-rig piers live here |
| PATCH GENERATION | 2339–2399 | 39 | WE | |
| LAYER ALLOCATOR | 2400–2438 | 18 | WE | |
| VISIBILITY CYLINDER | 2439–2461 | 10 | WE | |
| PATCH STREAMING HELPERS | 2462–2600 | 112 | WE | |
| PUBLIC: CARTRIDGE LIFECYCLE — orchestration | 2601–3451 | 544 | CR | initialize / bind_signal_layout / update / render |
| PUBLIC — stream_patches | 3452–3931 | 277 | WE | patch pipeline, FULLREGEN, evict/alloc, distance-driven spawning + heightfield, patch grid, culling — a world-engine machine behind one public method |
| PUBLIC — render tail + on_input + teardown | 3932–3981 | 38 | CR | |
| MODULE IMPLEMENTATIONS zone | 3982–4011 | 11 | CR | the 11 includes |

**The confession's size:** CR 699 code lines · WE 1,330 · DI 394 —
plus the two hubs (2,222 code lines) included mid-class, both WE/DI in
character. If the spine walked the ladder, the world-engine class is
the module: ~1,330 spine lines + spawn_engine's 550 already read as
one subsystem (patches, tiles, tokens, themes, footprints, streaming);
the composition root proper is ~700 lines. No design — the number is
the report.

---

## §4 — HOUSEKEEPING (report-first; nothing changed)

1. **THEMES is class-nested vocabulary with nine module-side readers.**
   Every generic family's get_theme_tier_weights returns
   `THEMES[theme_idx].tier_wt_<family>` — a Cartridge static consumed
   from entity_pipeline.inl (in-class today; would read
   `Cartridge::THEMES` post-class). PopulationTheme also carries
   per-family arrays (spawn_weight[PopFamily::COUNT], tier_wt_* ×
   seven) — adding a family edits the theme struct: a closed set worth
   naming when the contract arc lands. STATUS-shaped observation; no
   ledger line written (read-only order).
2. **MIN_SEPARATION lives in the TERRAIN TOKENS chapter**
   (cartridge.hpp:1746) while its siblings (PROXIMITY_*) live in
   spawn_engine.inl:812 — the pair-separation vocabulary is split
   across the two hubs. Rides PopFamily's graduation naturally.
3. **audit_entity_integrity's ref audit covers four families**
   (pyramid/arch/column/antenna — the count-twin families);
   entity_refs are recorded for all twelve. Diagnostic scope choice,
   not a bug (eviction routes through FAMILY_DISPATCH for all
   families); worth one line when the audit is next touched.
4. **record_placement_bookkeeping stands vacant** (documented seam,
   spawn_engine.inl:241–249). Unchanged finding, restated because the
   contract census walked over it.
5. **A seed_utils relocation note sits inside the UNIFIED PIER SYSTEM
   chapter** (cartridge.hpp:420–424) — present phrasing, correct
   content, odd address (it describes seed_utils, not piers). Cosmetic;
   one-line move whenever a code commit next touches that region.

Standing invariants unaffected: zone census green (11 == 11, once
each; the two hubs remain the lawful class-body pair), encodings
untouched — read-only.

**Next:** the LADDER-5 handoff follows this report (Jean's move).
