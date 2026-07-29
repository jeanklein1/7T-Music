> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# CC AUDIT REPORT — the_board, INDEPENDENT STRUCTURAL AUDIT
Auditor: Claude Code (second, independent). Read-only; the only write is this file.
Method: fingerprint first; three passes (structure, depth, cross-file) executed as
13 end-to-end assigned readings covering every in-scope file completely; all counts
executed with commands recorded; laws cited by ID per the mission rubric.
Anti-contamination honored: no other audit was read, requested, or inferred.

---

## L0 FINGERPRINT

Tree: commit `dd2b3edf4dcbce45cf1610bb728999ae107fe072`, branch `FINAL_LAPS`,
`git status --short` clean (no unstaged/uncommitted changes at audit time).
Commands: `git rev-parse HEAD`; `git status --short`; per file
`wc -l`, byte length + first-3-bytes + CRLF/LF classification via one python pass
(`open(p,'rb')`, count `\r\n` vs `\n`).

| file | lines | bytes | first3 (hex) | endings |
|---|---:|---:|---|---|
| cartridge.hpp | 4251 | 249665 | 237072 (`#pr`, no BOM) | LF (0 crlf) |
| state.hpp | 5874 | 329799 | 237072 (`#pr`, no BOM) | LF |
| renderer.hpp | 2905 | 146757 | efbbbf (BOM) | LF |
| world.wgsl | 11401 | 479628 | 2f2f20 (`// `, no BOM) | LF |
| modules/seed_utils.inl | 95 | 4303 | 2f2f20 | LF |
| modules/entities.inl | 758 | 38347 | 2f2f20 | LF |
| modules/pawn.inl | 167 | 8743 | efbbbf (BOM) | LF |
| modules/ground_architecture.inl | 306 | 16350 | 2f2f20 | LF |
| modules/orbs.inl | 1038 | 48886 | efbbbf (BOM) | LF |
| modules/agents.inl | 990 | 49422 | efbbbf (BOM) | LF |
| modules/spawn_engine.inl | 1131 | 52386 | 2f2f20 | LF |
| modules/floater_vocabulary.inl | 207 | 12280 | 2f2f20 | LF |
| modules/ribbon.inl | 1426 | 78321 | efbbbf (BOM) | LF |
| modules/gol_zones.inl | 632 | 31350 | 2f2f20 | LF |
| modules/gallery.inl | 1901 | 86775 | 2f2f20 | LF |
| modules/entity_pipeline.inl | 2277 | 111707 | efbbbf (BOM) | LF |
| modules/entity_types.inl | 200 | 10173 | 2f2f20 | LF |
| modules/cube_behaviors.inl | 523 | 26740 | 2f2f20 | LF |
| modules/mood.inl | 1337 | 63980 | 2f2f20 | LF |
| modules/render_passes.inl | 802 | 32302 | efbbbf (BOM) | LF |
| modules/input.inl | 348 | 16684 | 2f2f20 | LF |

[TREE-MOVED] deltas vs the snapshot baseline: **NONE.** All twenty baseline line
counts match exactly; the parallel audit read this same tree. renderer.hpp (2905
lines) is the file absent from the baseline; every finding sourced from it below
is tagged [NO-BASELINE].

Environment note for diff reconciliation: this is a Linux checkout; if the
recorded encoding law (LAW-8) was authored against a Windows working copy,
git line-ending normalization could account for the systematic CRLF absence —
that possibility is flagged, not judged (see L7).

LAW-8 ACTUAL MATRIX (executed, table above): 21 of 21 files are pure LF; zero
CRLF anywhere (`tr -dc '\r' | wc -c` = 0 on sampled files). BOM present on
exactly 7 files: renderer.hpp, agents.inl, entity_pipeline.inl, orbs.inl,
pawn.inl, render_passes.inl, ribbon.inl. BOM absent on state.hpp, cartridge.hpp,
world.wgsl and the other 11 modules. The recorded law ("state.hpp,
cartridge.hpp, world.wgsl are UTF-8 BOM + CRLF; input.inl UTF-8 LF no BOM")
matches the tree on exactly one point (input.inl: LF, no BOM — conforms) and is
contradicted on all six other clauses. The divergence is SYSTEMATIC —
**RULING-NEEDED, not judged** (law changed vs tree drifted vs checkout
normalization; see L7). No in-tree statement of encoding/BOM law exists
(`grep -rni 'utf|BOM|encoding'` hits only data-encoding comments:
world.wgsl:1591, 3663; cartridge.hpp:386). The in-repo constitution
(src/docs/cartridge_constitution.md §0) records a THIRD convention — "ribbon.inl
carries a UTF-8 BOM; world.wgsl is BOM-free LF" — which the tree does satisfy;
it is silent on the other 19 files.

---

## L1 CENSUS

Numbers only; command beside each. Full address lists in the body ledgers.

**Per-family address count, generic pipeline (family sampled: CACTUS).**
Command: `grep -rn -i cactus` over the board tree, hand-consolidated to section
granularity. Files touched: **8** (entities.inl, entity_pipeline.inl,
spawn_engine.inl, cartridge.hpp, state.hpp, renderer.hpp, render_passes.inl,
world.wgsl); line mentions 328. **Distinct sections a NEW generic family must
author end-to-end (CPU spawn → GPU render): ~50** — entities.inl 7 (tier enum,
color bases, config, prop registry, active struct, EntitiesState fields, mesh
preparer), entity_pipeline.inl 1 (the 10-element family block), spawn_engine.inl
6 (PopFamily, short name, Spawn Config Summary row, Property Index Registry row,
4 PROXIMITY columns + affinity), cartridge.hpp ~10 (6 dispatch wrappers,
FAMILY_DISPATCH row, MIN_SEPARATION row+column, POP_CROSS_AFFINITY row+column,
TIER_SCALE array + switch case, THEMES tier weight × 5 rows), state.hpp ~8 (Dim
capacities, GPU mesh params struct + asserts, buffers, layout, bind group,
upload fns), renderer.hpp ~5 (entry-point constants, compute+render+shadow
pipelines, dispatch + draw methods), render_passes.inl 2 (main + shadow draw
calls), world.wgsl ~6 (MeshParams struct, bindings triple, mesh-gen kernel,
VS + shadow VS, atlas slot). See L6-1.

**Per-family address count, bespoke (RIBBON).** Command: `grep -rnic ribbon`
per file + per-hit triage. Files functionally involved: **10** (ribbon.inl,
cartridge.hpp, state.hpp, world.wgsl, renderer.hpp, mood.inl, spawn_engine.inl,
render_passes.inl, gallery.inl, input.inl); 4 more comment-only; 750 mentioning
lines tree-wide. See L6-8.

**cartridge.hpp** (`wc -l` 4251): 16 module include splices (lines 503–4228;
ordering constraints recorded in body); 27 `═══` section banners; 43 static
dispatch wrappers (block 1010–1414 = 405 lines); FAMILY_DISPATCH rows = 12
(= PopFamily::COUNT, spawn_engine.inl:694); conductors update()/render()/
stream_patches() = 212/356/460 lines (3144–3355, 3372–3727, 3735–4194);
static_assert count **0** (`grep -c static_assert` = 1, a comment);
static constexpr = 89; `visual_canvas_` touch points = 3 in-file + 1 in
ribbon.inl.

**state.hpp** (5874): struct definitions 53 (`grep -cE '^\s*struct\s+\w+'`);
upload_* function definitions 66; static_asserts **64**; makeBuffer calls 77 +
3 direct CreateBuffer staging = **80 buffer members** (reconciled exactly);
offsetof partial-write seams 36; texture/view members 32; 24 named
BindGroupLayouts / 23 BindGroups.

**world.wgsl** (11401): kernels — `@compute` **31**, `@vertex` **32** (15
shadow_* variants), `@fragment` **8**; 71 entry points total (list recorded);
unique @group/@binding pairs: group(0)=89, group(1)=14 (103 total; 5 deliberate
read-only aliases for the frustum-cull pipeline); struct definitions 74;
`must match`-family comments **27** in this file alone; FXC mentions 15;
DRIVERLESS tags **11** (matches constitution §5 exactly).

**renderer.hpp** [NO-BASELINE] (2905): Entry namespace = **64** entry-point
strings, 64/64 match a `fn` in world.wgsl (`comm` executed, empty diff);
compute pipelines created 31; render pipelines 34; tPipe-timed creations 65/65;
pipeline members 65 (1:1 with creations); dispatch/draw methods 31/34; public
methods with **zero external callers: 4** (draw_pyramid, draw_shadow_pyramid,
draw_shadow_gallery_frames, draw_shadow_wall_paintings); CreatePipelineLayout
26 (23 null-checked, 3 not); arrayStride literals 9, none `sizeof`-derived;
header table rows 18 vs 31 actual compute pipelines.

**Tag censuses** (commands: `grep -rn "<TAG>" src/cartridges/the_board/`):
TESTING **1** (ribbon.inl:88). TODO **2** (entities.inl:117 NAMED TODO;
world.wgsl:7673 TODO[seam-map:cleanup]). FIXME **0**. HACK **0**. PENDING **8**
(all the PATCH_PENDING_TIER_* constant family — zero doc-leading PENDING tags).
DONE **26** lines (22 primary DONE[] + 3 cross-refs + renderer DONE[renderer:L2]).
SEAM **88** (per file: cartridge.hpp 25, ribbon.inl 13, world.wgsl 8, mood.inl 6,
spawn_engine.inl 6, gallery.inl 5, floater_vocabulary.inl 5, others ≤3).
DIAG **70** (cartridge.hpp 51, mostly `#ifdef`-guarded). DIAG-unwrapped tags
**5** + ribbon SEAM[ribbon:L1] = the constitution's 6 census sites, all located.
`must match` comments **50** tree-wide; `keep/stay in sync` 18;
`MIRRORED MANUALLY` 2 (one twin verified equal; one twin PHANTOM — L3).
static_assert lines **100** (state.hpp 64, agents.inl 14,
ground_architecture.inl 12, cube_behaviors.inl 8, cartridge.hpp 1-comment,
entity_pipeline/entity_types/spawn_engine/entities/floater_vocabulary **0**).
std::cout sites 141. Citations to design docs that do not exist in the tree:
**18** (cpu_gpu_pair_manifest.md ×12, agent_system_design.md ×4,
ground_hierarchy_design.md + ground_refactor_claude_code_brief.md ×2;
`find` returns none).

**GPU binding usage vs stated limits**: compute-entity layout binds 10 storage
buffers (state.hpp; the app requests full adapter limits at console.hpp:169-175
precisely because the WebGPU default 8 is too tight); in-tree stated law
"storage buffers / stage = 10; uniform buffers / stage = 12" (world.wgsl:55);
agent registries forced onto Uniform bindings 110/111 by the storage cap
(state.hpp:2872-2875). One DrawIndexedIndirect per pass (Dawn D3D12) —
LOD0-indirect/LOD1-direct split (state.hpp:1713-1714, renderer.hpp:801-803).

**Consoles**: 8 modules carry a TUNING CONSOLE/TUNING DATA banner (agents,
cube_behaviors, gallery, gol_zones, orbs, pawn, ribbon, mood); entities.inl,
floater_vocabulary.inl, ground_architecture.inl, input.inl, render_passes.inl,
renderer.hpp have none. The GPU panel assembles (world.wgsl:61-112 TUNING
SURFACE DIRECTORY); **no CPU-side global panel assembles anywhere** — the
directory's promised "companion directory in cartridge.hpp" does not exist
(L3/L4).

**Audited surface**: 21 files, 38,569 lines (`wc -l` sum).

---

## L2 STRENGTHS

Patterns worth propagating, with addresses. [TASTE] where the entry is
taste, not law.

1. **LAW-4 spec-as-data at scale** — FamilyDispatch: declarative fn-pointer row
   schema (cartridge.hpp:989-997) precedes wrapper realizers (1010-1414) and the
   constexpr table (1426-1463); the whole entity lifecycle drives from one data
   table. Same shape at the pipeline core: TRAITS/PARAM_DEFS/TIERS constexpr
   data consumed by three shared realizers (entity_pipeline.inl:193-266 with
   entity_types.inl:115-139).
2. **LAW-5 by construction where the language allows** — GPURibbonState pins
   sizeof AND individual retired-slot offsets (state.hpp:1415-1419,
   "checker_scatter must sit at twist_amp's retired slot (28)"); WGSL twin names
   it back byte-for-byte (world.wgsl:826-827). agents.inl:125-128 + 406/414-419:
   behavior/tier counts and per-row mood_id order are static_asserted; the
   cartridge banner cites the enforcement accurately (cartridge.hpp:366-371).
   ground_architecture.inl:272-303: ASSERT_POLICY_DAG_CLOSED expands 54
   compile-time closure checks over the policy registry, macro rationale stated.
3. **LAW-5 by refactor instead of mirror** — ribbon_phase_age is "THE one
   expression" shared by spine echo and ring motor (world.wgsl:4238-4241);
   lod_pawn converts a must-agree into shared data pushed through the config
   (state.hpp:457-467) so CPU and GPU "partition with the same yardstick by
   construction".
4. **LAW-11 model twin** — MOUNT_TANGENT_ALIGN/BANK_GAIN/BANK_MAX
   (ribbon.inl:131-138) ↔ RIBBON_* (world.wgsl:4272-4275): equal values
   (1.0/0.9/0.6), twin named at BOTH sites, tuning authority declared ("The GPU
   set is the tuning authority (hot-reload)"), drift test stated and once fired
   for real (the rider's lean; BNK-1 sign resolution recorded in the datasheet
   ledger). Same discipline: AGENT_EVICTION_RADIUS (agents.inl:181-196 ↔
   world.wgsl:6114-6120, 360==360, both-way naming, why-apart stated);
   meshGenEntityLayout_ reuse named at both renderer.hpp:179 and state.hpp:1670;
   UnifiedPaintingSlot ↔ GPUPaintingSlot (world.wgsl:8118 ↔ state.hpp:1430/1468).
5. **LAW-7 law-stated silences** — INDOOR_ENTITY_WALL_MARGIN documents
   clamp-not-reject with the failure mode rejection would cause
   (cartridge.hpp:419-440; consumed spawn_engine.inl:191-227); readback closures
   capture world_gen and document the stale-drop where it happens
   (cartridge.hpp:3394-3402, 3441-3444); sphere commit's deliberate
   non-registration quantifies its own silence bound ("8 slots and a 1.5% spawn
   chance", entity_pipeline.inl:1770-1785); sample_terrain_y_at declares its 0.0
   out-of-window default at the function head (world.wgsl:7747-7770);
   FLOATER_EVICTION_RADIUS derives 400 = 350 + 50 with the observed failure of
   the earlier value (world.wgsl:6128-6148); the sky-pose neutral-zero fill is
   engineered so "losing the resync fails LOUD (pawn to origin)"
   (cartridge.hpp:3168-3177); gallery staging ring's unconditional-overwrite law
   stated at the cursor (gallery.inl:702).
6. **LAW-9 posture** — renderer.hpp realizes zero geometry CPU-side in 2905
   lines [NO-BASELINE]; §9 mesh-gen states "CPU authors intent (params), GPU
   realizes mesh" and all six families honor it (world.wgsl:8580-8591); the one
   exception (catenary parameter `a` bisected on CPU) is law-stated at both
   sites (world.wgsl:8782-8783, state.hpp:845-846); CPU commit sites state
   "ground_y=0; GPU compute pass realizes true height" per family
   (entity_pipeline.inl:291-294, 1538-1540, 2146-2148).
7. **LAW-3 model parking** — every DRIVERLESS shader read tagged in place with
   a revive-or-delete bound; file-wide count (11) matches the constitution
   census exactly (world.wgsl:4922... 10606-10610; orbs.inl:439 the CPU landing
   site with census citation). Binding 144 retained with reason + bounded
   removal recipe (world.wgsl:7673-7679). COMPAT Trajectory tagged at both ends
   with a death condition (pawn.inl:142-144 ↔ coupling/trajectory.hpp:19-23).
   ENVIRONMENTAL gallery tier parked at weight 0.01 with deletion cost and
   revival path stated (gallery.inl:119-128).
8. **LAW-10 model consoles** — agents.inl:154-200 (dials with coupling prose:
   spawn 340 vs evict 360 "20-unit alive band"); cube_behaviors.inl:96-143
   (declares the dials it cannot own, with edit address and promotion cost
   "~16 bytes of uniform space + 4 lines"); the GPU TUNING SURFACE DIRECTORY
   assembles with values and section pointers (world.wgsl:61-112; 15 of 16
   value rows verified current).
9. **LAW-1 single doors** — request_mood_transition is the one transition entry
   (mood.inl:73-79; five key cases collapsed, input.inl:219-225, same DONE tag
   both sites); Entry namespace is the single address for all 64 entry-point
   strings, zero bypasses [NO-BASELINE] (renderer.hpp:56-155);
   draw_shadow_indexed_mesh collapses 9 shadow wrappers with an honest list of
   what stays individual (renderer.hpp:1063-1092); populate_agent_slot_ is the
   single spec→slot realizer (agents.inl:516-604); property-index registries
   with verified-disjoint decades replace literal hash indices (entities.inl:
   127-129 and 8 siblings; gallery.inl:365-380; ribbon.inl:260-273).
10. **LAW-6 width discipline where it matters** — ribbon_advance_head takes
    RibbonState&, GPUState&, queue, and the ground sample explicitly; the head-law
    cluster never touches Cartridge* (ribbon.inl:655-659); Renderer receives
    device + 25 layouts + 2 formats through init() and nothing else
    [NO-BASELINE] (renderer.hpp:333-364); collect_sorted_patches is the spine
    helper that takes everything explicitly and is const (cartridge.hpp:2844-2877).
11. **Determinism as practiced law** — seed_utils FXC mirrors verified
    constant-for-constant against WGSL (hash chain, lattice seed, gaussian
    clamps, ±3σ, +1000 pair offset); seed-stability WHY-comments guard against
    renames that would shift every rendered world (floater_vocabulary.inl:87-89).
12. [TASTE] **tPipe instrumentation** — all 65 pipeline creations individually
    timed with a boot leaderboard (renderer.hpp:197-212, 376-386) [NO-BASELINE].
13. [TASTE] **Negative-knowledge comments** — "Why no constexpr helper
    builders" records the MSVC incomplete-class failure that killed the obvious
    refactor (agents.inl:324-338); TierRow name-collision notes prevent a
    cleanup-rename regression (entity_pipeline.inl:1004-1006).
14. [TASTE] **Aligned tuning matrices** — TERRAIN_BANDS/GOL_TIERS/PULSE_TIERS/
    OVERLAY_WAVES/SHOT_PARAMS readable as matrices with verified row sums
    (world.wgsl:322-331 etc.; gallery.inl:118 weights sum exactly 1.00).
15. [TASTE] **The saddle law** — ONE s_age serves position, frame, and tube,
    with the prior failure mode quantified at the site ("shears the seat ...
    the float/dip this block once had", ribbon.inl:774-806).

---

## L3 DISEASES

Law violations only. Format: LAW-ID — file:line — evidence — quote (≤2 lines)
where textual. Grouped by law. Where two readings exist, both are stated.

### LAW-1 (one concept, one address)

1. LAW-1 — cartridge.hpp:1426 (+999-1414, 2115, 1871, 1917-1949, 1508-1519,
   1546-1624) — the "entity family" concept spans ≥8 addresses in this file
   alone (dispatch row, 6-wrapper cluster, MIN_SEPARATION row+col,
   POP_CROSS_AFFINITY row+col, TIER_SCALE array, tier_scale switch, tier_wt
   field, 5 THEMES weights) + PopFamily in spawn_engine.inl:681; the header
   recipe names only 3.
   > // Adding a new family means: write select/place/commit/
2. LAW-1 — cartridge.hpp:396 (+372-377, 409-417, 296-303, 884-900) — the "mood"
   concept needs 5 in-file addresses (MOOD_TABLE row, MOOD_* id, mood_name,
   PORTAL_COLORS row, pick_portal_mood thresholds) + 3 assert-locked parallel
   tables elsewhere; the in-file trio drifts freely.
3. LAW-1 — mood.inl:536,576,717,1022,1154 — finite-world bounds
   (bmin/bmax from finite_radius × PATCH_EXTENT) re-derived at five addresses
   in one file (grep-executed).
   > float bmin = -(float)world_state_.finite_radius * PATCH_EXTENT;
4. LAW-1 — mood.inl:1032-1044 vs 1163-1175 — the doorway-footprint wall margin
   (same three ARCH_TIERS reads, same 8.0f fallback) duplicated verbatim
   between the two portal spawners.
5. LAW-1 — ribbon.inl:1283-1294 + mood.inl:622-643 — the sel→plan field copy
   exists at two addresses (dispatch path, mood forced-spawn path); one added
   seed field means three synchronized edits (plus commit's plan→GPU copy);
   mood.inl:624's comment is the fossil of the bug this already caused.
   > plan.seed = sel.seed;   // wander channels sample this — without it every mood-forced ribbon draws from seed 0
6. LAW-1 — ribbon.inl:505-506 vs cartridge.hpp:164-168 — four ribbon-named
   TargetBindings + player_.sky_yaw_eased live on Cartridge while the struct doc
   claims "All ribbon-owned state lives in this struct"; either coupling-layer
   property by design or split ownership — the claim over-states either way.
7. LAW-1 — state.hpp:2394 vs 2175-2386 — config mutation at two addresses:
   ~25 dirty-flagged setters AND a raw `config()` reference that bypasses
   configDirty_ entirely.
   > GPUDesignConfig& config() { return config_; }
8. LAW-1 — world.wgsl:3814-3881 vs 3977-4021 — pawn-mesh vid→position decode
   duplicated verbatim between pawn_vs and shadow_pawn_vs; no cross-reference
   binds them. Same class: gallery_frame_vs vs shadow_gallery_frame_vs quad
   decode (~28 lines, 8216-8243 vs 8290-8313) — in the same file whose
   wall-painting pair correctly shares compute_wall_painting_geometry (8550-8576).
9. LAW-1 — world.wgsl:69,1412,1465 + state.hpp:430 — palette slot 3's identity
   at four addresses, three stale ("grey" ×3 vs the live "warm — dusty
   terracotta"); the "(was grey 0.07)" aside is also LAW-2 archaeology.
10. LAW-1 — world.wgsl:282, 10023, 10349 — hash_property duplicated verbatim as
    cactus_hash and blade_hash (same constants 747796405/2891336453/2654435769);
    neither copy names the original. Golden angle re-derived at 9827/10142/10386.
11. LAW-1 — world.wgsl:9283-9295 vs 9452-9457 — antenna drum sizing authored
    twice inside one kernel (profile builder and disc builder restate
    content_h/drum_start_y/drum-radius formula).
12. LAW-1 — world.wgsl:5059-5060 — aura grid geometry at four addresses
    (render-side literals `aura_cs = 3.125; aura_n = 64`, PAWN_AURA_N
    world.wgsl:5013, per-frame PawnAuraConfig, CPU PAWN_AURA_N state.hpp:1062);
    the render literals name none of them. Two readings: stage isolation vs
    plain duplication; nothing at the site says which.
13. LAW-1 — entity_pipeline.inl:400-405, 614-617, 833-836 vs entities.inl:
    360-362, 430-431, 498-499 — blade/palm/cactus color-base RGB values exist
    twice: named constants with ZERO code consumers (dead twins) and live
    literals whose comments cite the constant names without referencing them.
    > { { 0.28f, 0.52f, 0.22f },   // BLADE_BODY_BASE
14. LAW-1 — entity_pipeline.inl:2153, 2156 — arch portal props derived as
    ArchProp::ROTATION+100u=703 and +200u=803 collide on the same seed source
    with ColumnProp::TIER (703) and PyramidProp::ROTATION (803) — exactly the
    correlated-roll hazard the registry itself brands "a subtle bug"
    (spawn_engine.inl:735-738); the 803 collision is currently masked by
    PORTAL_DENSITY=1.00 (roll always passes).
15. LAW-1 — cube_behaviors.inl:128-143 — eight WGSL kernel force literals
    value-copied into CPU console comments (verified equal today, world.wgsl:
    6414-6417, 6443-6446); every WGSL tune stales the console silently. Two
    readings recorded; the copies are data, not pointers.
16. LAW-1/LAW-11 — gol_zones.inl:187-195, 255-258 + world.wgsl:1652-1662,
    1706-1710 — GOL_TIERS (7×13) and PULSE_TIERS (3×18) duplicated IN FULL,
    both copies live (CPU: tier pick/tick/seed density; GPU: zone_derive_params
    + terrain FS), ~145 literals, neither side names the other, and each side's
    banner claims authority. Row-by-row equality verified today.
    > gol_zones.inl:61: those are the per-tier consoles ‖ world.wgsl:1667: single source of truth

### LAW-2 (comments state present behavior)

17. LAW-2 — cartridge.hpp:961-969 — DENSITY control-surface table says
    MIN 0.1 / MAX 3.0; the constants four lines below are 1.0/1.0, making the
    whole density field a constant 1.0 (consumers multiply by 1).
18. LAW-2 — cartridge.hpp:1483-1489 vs 1544-1625 — THEME table weights
    0.40/0.12/0.18/0.15/0.15, densities ×1.0/×0.7/×1.5/×1.2/×0.3 vs live
    0.21/0.30/0.31/0.18/0.04 and density_mult 1.0 in every row (the second
    boxed table at 1532-1542 IS current, isolating the first as drift).
19. LAW-2 — cartridge.hpp:1827-1850 — POP BATCH table wrong on six of seven
    documented rows (4→16, 3.0→0.0, 2.0→0.0, 2→1, 0.50→0.0, 0.25→0.0).
20. LAW-2 — cartridge.hpp:2101-2118 — MIN_SEPARATION prose contradicts the
    matrix (Pyramid 15/10/5 vs 65/60/5), and the named "Key exception:
    Arch→Pyramid = 0 … explicitly allowed on top of pyramids" is contradicted
    by MIN_SEPARATION[ARCH][PYRAMID]=60.0f — documented intent is the opposite
    of enforced behavior.
21. LAW-2 — cartridge.hpp:3709 — "// DIAG: frustum cull bypassed — direct draw
    active": dispatch_frustum_cull runs whenever use_indirect_terrain() is true
    (mood-profile-driven, moods 0/1/4/5) and LOD0 draws indirect
    (render_passes.inl:227, 466-474).
22. LAW-2 — cartridge.hpp:3379 — "(MAX_AGENTS × 80 bytes)": the_board's
    GPUAgentState is 96 bytes (static_assert state.hpp:1407; world.wgsl:654
    says 96). 80 is the sibling cartridge's size.
23. LAW-2 — state.hpp:11-50 — header BINDING MAP stale on multiple rows
    (26 "solid_instances Uniform" vs pier_instances ReadOnlyStorage :3736-3738;
    300 and 360 listed ReadOnlyStorage but built Uniform :3834-3856) and omits
    whole systems (110/111, 140-152, 160-172, 180-198, 390-391, 400-414,
    500-501).
24. LAW-2 — state.hpp:1540, 1545, 1551 — three texture members annotated
    "49 layers"; created with 225 layers (label itself says "225x256x256",
    :3544-3545). 49 = retired 7² grid.
25. LAW-2 — state.hpp:279 + 820, 842, 870, 905, 940, 974 + world.wgsl:30 —
    rotted line-number cross-references: coupling bits cited at wgsl 1675-1696
    are at 1732-1752; all six *MeshParams cites are ~240-260 lines stale;
    "line ~631" is at 689. The file's own navigation rule (world.wgsl:14-15)
    says to search by section precisely because line refs rot.
26. LAW-2 — state.hpp:705 — is_roaming "(0 = stationary spine = today)":
    commit_ribbon writes 1u unconditionally (ribbon.inl:1346); the WGSL mirror
    already states the truth ("constant 1 … field retained until the next
    struct relayout", world.wgsl:845).
27. LAW-2/LAW-3 — state.hpp:820-822 (6 sites) + world.wgsl:8608 (6 sites) —
    twelve comments order same-commit updates to `cpu_gpu_pair_manifest.md`,
    which does not exist anywhere in the tree (`find` executed). Plus
    agent_system_design.md ×4 and the two ground_* design docs ×2 — 18 total
    citations to ghost documents.
    > If this struct gains/loses a field, the WGSL side and cpu_gpu_pair_manifest.md must be updated together.
28. LAW-2 — world.wgsl:89 vs 1523 — TUNING SURFACE DIRECTORY advertises
    MODE_COUPLING_MAGNITUDE 0.25; the constant is 0.0 "DISABLED". The full
    coupling machinery still evaluates per-sample and multiplies by zero
    (also LAW-3). The other 15 value rows verified correct.
29. LAW-2 — world.wgsl:113-114 — the GPU directory closes by pointing at "the
    companion directory in cartridge.hpp"; no tuning directory exists anywhere
    in cartridge.hpp (grep = 0). Also LAW-10: the promised global CPU panel
    never assembles.
30. LAW-2 — world.wgsl:1666-1667 — pulse banner claims "CPU … uploads
    parameters via GoLZoneConfig; these definitions are the single source of
    truth" — contradicted by the derive-request pipeline (gol_zones.inl:588,
    state.hpp:2781) and by the CPU's own live duplicate tables.
31. LAW-2 — world.wgsl:3682 — "Shadow pass variant — same geometry": shadow VS
    omits pawn-aura lift and radial pulses that the main VS adds (3557-3566 vs
    3708); the file's own rule (3509-3510) demands divergence be documented.
    Also LAW-7 (silent divergence at a boundary).
32. LAW-2 — world.wgsl:4296-4298 — BNK-1 comment still says the saddle rides
    "GIMBAL-level … pending Jean's ruling"; BNK-2 landed (CPU computes
    mount_yaw_off/pitch/roll ribbon.inl:795-806; kernel composes them
    5529-5543). Stale pending on a resolved ruling.
33. LAW-2 — world.wgsl:5979-5983 (behavior_flock2d) — "Random direction noise"
    computes `theta * noise_arc`, confining noise to the sector [0, ~0.78 rad]
    (persistent NE bias); every sibling behavior adds noise to a base angle.
    Two readings: comment misstates behavior, or the multiply is the bug the
    comment indicts — comment and code disagree either way.
34. LAW-2 — world.wgsl:8732, 8751 — "Winding matches CPU": no CPU pyramid mesh
    generator or emit_quad exists anywhere (grep) — a lockstep claim against an
    absent counterpart.
35. LAW-2 — world.wgsl:8740, 8770 vs 8773 — pyramid index-fill comments
    ("vi = 18 … 12→30") describe a pre-bottom-cap geometry two branches above
    the current code.
36. LAW-2 — world.wgsl:10560-10565 vs 10606-10610 — §ORB header claims four
    couplings "uploaded per-frame"; configure_orbs zeros all of them and the
    four upload helpers (state.hpp:2677-2746) have zero call sites (grep). The
    struct's own DRIVERLESS tag contradicts the header; field comments at
    10610/10711 still say "polyphony-coupled".
37. LAW-2 — entities.inl:86-87, 551-552 — arch "CPU-generated barrel vault" and
    pyramid "CPU-generated … mesh": both are GPU compute kernels
    (world.wgsl:9060, 8694; dispatched cartridge.hpp:1015-1023). The code obeys
    LAW-9; the prose misstates sovereignty in the wrong direction.
38. LAW-2 — entities.inl:669-671 vs 712-758 — "Counterparts … currently live in
    spawn_engine.inl; they will be hoisted here in migration #10": the hoist
    happened — the preparers are 40 lines below. spawn_engine.inl:29-32 still
    advertises the moved functions with pre-migration signatures (banner
    contradicting its own body note at 260-265).
39. LAW-2 — entities.inl:194 vs 205-217 — "column palette (seven entries)" vs
    ten rows and COLUMN_PALETTE_COUNT=10.
40. LAW-2 — entities.inl:37-41 — public-surface banner lists 3 mesh preparers;
    six are defined and all six are consumed externally (cartridge.hpp:1013-1156).
41. LAW-2 — entity_pipeline.inl:3-5, 34-35, 2271-2274 — the file's own family
    counts lag reality twice over: "seven cookie-cutter families" (listing
    eight names), "eight families: 24 entries total" vs nine families × 3 = 27
    wrappers (executed count) and 9 generic rows in FAMILY_DISPATCH;
    spawn_engine.inl:1028 ("all 9 migrated families") is the only current number.
42. LAW-2 — entity_pipeline.inl:347-348 — BLADE_PARAM_DEFS floors bound to "the
    std::max() in select_blade_for_patch": the function no longer exists
    anywhere (grep; removed in migration) — the comment points at a ghost.
43. LAW-2 — entity_pipeline.inl:1784 — "(cartridge.hpp ~7990)" points ~3,700
    lines past EOF (4251); the referenced machinery is at cartridge.hpp:3391.
44. LAW-2 — entity_pipeline.inl:71-73 — dependency banner attributes
    select_tier_biased to seed_utils.inl; it lives at cartridge.hpp:2058
    (seed_utils provides only select_tier).
45. LAW-2 — spawn_engine.inl:714 — the "single-glance" spawn table states
    ribbon CHANCE 0.400; compiled value is 0.900f (ribbon.inl:88). No pending
    tag; the table's own preamble concedes it is a copy.
46. LAW-2 — spawn_engine.inl:732-777 — the Property Index Registry ("Check this
    table before allocating") omits four in-use allocations on its own seed
    sources (355u ×2 at entity_pipeline.inl:1080/1092; antenna drum props
    850-868 at 1307-1319; rescale 7777u at 171; cube phase 0xF10A7E70 at 1955)
    and never mentions the +1000 Gaussian pair shadow occupying 1400-1475
    inside space it marks "free". RibbonProp row claims 400-443; actual spread
    400-475 (39 channels, executed).
47. LAW-2 — agents.inl:426-427 — "read … from GPU storage buffers (bindings
    110/111)": both are Uniform (state.hpp:2871-2875 with the storage-cap
    rationale; world.wgsl:739/754 `var<uniform>`). Bindings hold; class stale.
48. LAW-2 — mood.inl:487-491 (and DONE[mood:K2] at 66-70) — "Five named
    sub-functions … band motion": four exist; numbered comments run 1,2,3,5;
    band motion lives at cartridge.hpp:3005. SEAM[mood:K1] (53-55) already says
    four — two headers in one file disagree.
49. LAW-2 — mood.inl:611-612 — "head init in state.hpp": head init lives in
    ribbon.inl::ribbon_advance_head (662-693); ribbon.inl:1089-1090 carries the
    correct sibling comment.
50. LAW-2 — gallery.inl:249-256 vs 258-260, 360-362 — header narrates outdoor
    70/20/10 and indoor 40/20/40; constants execute 0.80/0.05/0.15 and
    0.15/0.05/0.80. The constants' own band comments agree with the code.
51. LAW-2 — gallery.inl:575, 1384, 1532 vs 1421-1434, 1443-1446 — three
    addresses say the authored manifest is "scanned at startup, sorted
    alphabetically"; the code sorts numerically ("not lexicographic") and scans
    lazily on first call (header :43 agrees with lazy).
52. LAW-2 — gallery.inl:1189 — "Centers are evicted by distance sweep in
    stream_patches": centers clear via dispatch_evict_gallery fired by
    host-patch entity_refs (cartridge.hpp:1294-1300, 3814) — true only at one
    remove; no dedicated center sweep exists.
53. LAW-2 — orbs.inl:24, 787 vs input.inl:226 — palette cycling bound to
    "KP_0" twice; the actual key is top-row GLFW_KEY_0, outside input.inl's
    numpad orb block.
54. LAW-2 — orbs.inl:580 — pack_tiers_'s exception comment names offset 428
    only; the i==1 write also lands on repurposed pad 444 (speed_mult), rescued
    only by pack_flocking_ running later — order-dependent behavior half-stated.
55. LAW-2 — pawn.inl:99 — "toggled by numpad 3": binding is main-row
    GLFW_KEY_3 (input.inl:214).
56. LAW-2 — ribbon.inl:56-62 — dependency list names pawnReadback_* (migrated
    to player_.readback_* per cartridge.hpp:481) and two functions the module
    never calls (evaluate_spawn_gate, jittered_position — internals of the
    helpers it does call); omits visual_canvas_ + 4 bindings, inputState_,
    player_ sky fields, terrain services, select_tier_biased, and four upload
    wires (executed c-> census).
57. LAW-2 — seed_utils.inl:9 vs 41 — cpu_hash_f documented "[0, 1)": the
    quotient reaches exactly 1.0f for h in the top rounding band (~3e-8/draw);
    downstream sites defend individually (ribbon.inl:1112 clamp; select_tier
    count-1 fallback), showing the code does not trust the stated contract.
58. LAW-2 — cube_behaviors.inl:47-48, 452 vs cartridge.hpp:3509 — "Called from
    update()" twice; the sole call site is render() (begins :3372).
    floater_vocabulary.inl:202 states the render() home correctly — the two
    files disagree and cube_behaviors is wrong twice.
59. LAW-2 — cube_behaviors.inl:157-158, 202-203 — "Character pass will populate
    with real values": doc leads code with no pending tag, in a repo whose
    convention for exactly this (NAMED TODO / SEAM) is visible one module over.
60. LAW-2 — floater_vocabulary.inl:43 — "Depends on: … entities.inl
    (MOOD_COUNT)": MOOD_COUNT is defined at cartridge.hpp:362; entities.inl
    only consumes it.
61. LAW-2 — render_passes.inl:722-723, 727-730, 779-783 — sun-shadow frustum
    comment sized to "the 11×11 patch grid (radius 5)": live constants are
    GRID_RADIUS=3 / PREGEN_RADIUS=7 (state.hpp:106,110); the ±250/±275/±350
    arithmetic no longer bounds the ]-key radius-7 case (~±375). Also LAW-10:
    altitude/extent/far dials inline, no console.
62. LAW-2 — renderer.hpp:8-31 [NO-BASELINE] — header tables claim 18 compute
    pipelines (31 exist) and 9 render families "+ shadow variants for each"
    (34 exist; pyramid is dead, gallery-frame shadow never drawn); 13 compute
    names missing entirely.
63. LAW-2 — renderer.hpp:223 [NO-BASELINE] — "Legacy cell system — compiled but
    not dispatched": zero legacy-cell pipeline members exist beneath it
    (S2 census executed) — a past-tense ghost stated as present.
64. LAW-2 — renderer.hpp:24 vs 653 [NO-BASELINE] — header row "zone_gol_sync
    2D (8x8x8)" matches neither the WGSL workgroup (8,8,1) nor the dispatch
    (4,4,zone_count); sibling rows prove the column means workgroup size.
65. LAW-2 — gol_zones.inl:538 + gallery.inl:1223, 1335, 1379, 1406, 1436, 1462 —
    autonomous stdout sites NOT in the constitution §5 DIAG-unwrapped census
    and carrying no in-place tag ([GoL] commit print fires per zone spawn
    during streaming; [Photographer] Rendering fires on capture;
    five [Authored] load-path prints), while four gallery siblings ARE tagged.
    By the census's own closing law these are "a bug in this document first."

### LAW-3 (YAGNI / parked capability must be tagged+bounded+cited)

66. LAW-3 — cartridge.hpp:1844-1885 — with POP_MODE_*_CHANCE = 0.0 and both
    strengths 0.0, every batch is NEUTRAL and population_type_affinity always
    returns 1.0: the 12×12 POP_CROSS_AFFINITY matrix and the TIER_SCALE batch
    role are machinery with permanently neutral output — no tag, bound, or
    citation. Two readings: retired-after-tuning (then deletable) or dormant
    experiment (then tag required).
67. LAW-3 — cartridge.hpp:334-335 — MoodProfile.fog_density/fog_color authored
    in all six rows, consumed nowhere (fog is canvas-driven; mood.inl:521-523
    states the retirement) — dead data columns with live-sounding comments.
68. LAW-3 — state.hpp:220-221 — MAX_ZONE_MESH_VERTICES/INDICES referenced
    nowhere (the live pair is ZONE_MESH_MAX_*); name-colliding dead constants.
69. LAW-3 — state.hpp:375-377 vs 2183, 2366-2368, 5658-5662 — inert config
    fields (wave masks, active_cell_size, mute_dynamics_2d) with live-sounding
    declarations while initializeState writes fake-live values
    ("0x7 // All 3 waves enabled") into fields nothing reads — status told
    contradictorily at three addresses (LAW-1 co-violation).
70. LAW-3 — world.wgsl:1721 — PAWN_GOL_GROUND_ENABLED declared, referenced
    nowhere (grep = 1 hit); the banner claims it prunes dependency chains, but
    query_ground_walker includes GoL unconditionally (2687-2690). Also LAW-2
    and LAW-10.
71. LAW-3 — world.wgsl:616-619 — FrameSignal.stats[64] (256 B) uploaded every
    frame with zero shader consumers; tagged DRIVERLESS but bounded by nothing
    and cited to no design doc (sister parks carry revive-or-delete bounds).
72. LAW-3 — world.wgsl:1739-1747 — six coupling bits "(reserved — legacy …)"
    tagged but unbounded and uncited; CPU twin block (state.hpp:280-282)
    explains reuse risk. Two readings recorded (bit-position stability vs
    unmet park test).
73. LAW-3 — world.wgsl:7275 — config.mode_gol_height_scale read carries no
    DRIVERLESS tag while its sibling read (4922-4924) does — the parked
    capability is untagged at one of its two shader read sites, so the census
    undercounts its surface.
74. LAW-3 — world.wgsl:8602, 8800, 9684 — PMG/AMG/PALMG_TOTAL_INDICES declared,
    never referenced (grep = 1 hit each); unnamed mirrors of CPU draw-count
    constants; cactus/blade have no analog.
75. LAW-3 — entity_pipeline.inl:368, 578, 803, 1453-1454 — color_over dead in
    three TierRows (grep: definitions only) and PyramidTierRow
    color_override/color_variance never read — untagged reserved fields
    carrying live-looking values inside the very blocks the header tells
    new-family authors to copy.
76. LAW-3 — entity_pipeline.inl:1651-1669 — SphereTierRow's 15 "dead-but-
    preserved extras": tagged and bounded, but cited to "the migration spec"
    with no findable path — an IOU instead of a citation.
77. LAW-3 — gol_zones.inl:144-155 — GoLColorMode (both weight tables) and six
    GoLZoneProp indices have zero CPU consumers (executed grep); live consumers
    are the WGSL twins. Untagged/uncited. Two readings: dead code vs the CPU
    allocation map of a shared seed-space — either reading requires the tag it
    lacks.
78. LAW-3 — gallery.inl:408, 434 — AUTH_STG_PICK tagged "unused — see Q30 in
    rollout report"; no rollout report exists in-tree (grep Q30 = these
    comments only). Tag+bound present; citation dangles.
79. LAW-3 — gallery.inl:926-930 + gol_zones.inl:512 — commit_* trigger_gx/gz
    parameters never read (bodies re-derive from plan.*); callers pass
    redundantly — untagged reserved parameters at two addresses.
80. LAW-3 — gallery.inl:1219-1220 — dead second guard on
    pending_snapshot.active with no intervening write.
81. LAW-3 — input.inl:119-142 — eight GLFW_KEY_KP_0..7 fallback macros defined
    "so the switch below compiles"; the switch never references them
    (grep-executed).
82. LAW-3 — mood.inl:1303-1305 — upload_lights re-uploads a zero-count
    GPUPointLightArray on every lights_dirty; nothing anywhere authors a point
    light (grep) — reserved capability, untagged, uncited (struct state.hpp:1283,
    binding 321).
83. LAW-3 — orbs.inl:737 + cartridge.hpp:3000, 3140 + ribbon.inl:996 — parked
    couplings cited to `coupling_layer_migration_map.md`, which does not exist
    in the tree (find executed) — tag and bound present, citation chain
    dead-ends.
84. LAW-3 — renderer.hpp:2244-2251, 2742-2749, 980-994, 1228-1239 [NO-BASELINE]
    — pyramidPipeline_/shadowPyramidPipeline_ + draw methods have zero callers
    repo-wide (executed); the why lives only in render_passes.inl:598-600; both
    pipelines recompile on every boot AND hot reload. No tag, bound, or
    citation at the site.
85. LAW-3 — renderer.hpp:2436-2458, 2533-2556, 1256-1280 [NO-BASELINE] —
    shadow gallery-frame and shadow wall-painting pipelines + draw methods:
    zero callers (executed; the shadow pass draws 12 families, paintings not
    among them — gallery.inl:69's NOTE says so); both compile every boot/reload;
    untagged.
86. LAW-3/LAW-11 — entities.inl:80 — PAWN_HEIGHT_UNITS: zero consumers
    repo-wide; kept solely to mirror WGSL PAWN_HEIGHT (world.wgsl:1539), which
    does not name it back — an orphan half-mirror.
87. LAW-3/LAW-2 — floater_vocabulary.inl:77, 154 vs banner 26, 34 —
    SPHERE_TIER_NAMES/CUBE_TIER_NAMES have zero consumers, yet the banner lists
    both under "Public surface (consumed by other files)".
88. LAW-3 — cube_behaviors.inl:153-155, 165 — CubeTierGain::behavior_amp_mult
    reserved ("not yet consumed by kernel"); tagged and bounded but cited to no
    design doc; verified unconsumed in WGSL.

### LAW-5 (lockstep by construction)

89. LAW-5 — **world.wgsl:627 ↔ state.hpp:343 — THE CROSS-BUS DEFECT: the field
    the shader names `dt_beats` ("beat-time delta") is `_pad1` CPU-side and is
    hard-zeroed every frame (cartridge.hpp:3166); no CPU writer exists (grep
    executed; the computed beat delta goes only to time_state_.beat_rate).
    step_trigger (world.wgsl:5439-5445) consumes it in 8 beat-gated agent
    behaviors — with dt_beats always 0, `fired` can never be true.** Two
    readings: a live defect silencing every step impulse, or a
    demolition-orphaned capability whose WGSL comment states non-present
    behavior — either way the pair is unenforced and mis-named across the bus,
    the exact "silent runtime corruption — no compile error" the file predicts
    at 703-704. Sizes still match (336==336) so no assert can catch it.
    > dt_beats: f32,  // beat-time delta (currentBeats_ - prevBeats_)  ⇄  float _pad1; … gpuSignal._pad1 = 0.0f;
90. LAW-5 — entity_pipeline.inl:363 (also 568, 797, 1000, 1445, 1647, 1826,
    2021) — eight "params[] order MUST match *_PARAM_DEFS" comment-locksteps
    across three parallel lists per family; **zero static_asserts in the file**
    (executed); a silent reorder produces wrong-but-plausible geometry. The
    repo's own per-row assert idiom (agents.inl:414-419, cube_behaviors.inl:
    234-239) goes unapplied.
91. LAW-5 — entity_pipeline.inl:372, 586, 808, 1015, 1457, 2034 vs 1482 —
    seven of nine tier tables are unsized `[]` arrays whose row counts live as
    constants in a different file; PYRAMID_TRAITS hardcodes literal 3. Only
    Sphere/Cube size their tables with the shared constant (the two good
    instances recorded in L2).
92. LAW-5 — cartridge.hpp:1416-1426 — FAMILY_DISPATCH "(order matches PopFamily
    enum)" enforced by comment in a file with zero static_asserts; eviction
    routes by index (684) — a swapped row evicts through the wrong family.
93. LAW-5 — cartridge.hpp:296-303 — PORTAL_COLORS sized by literal [6];
    consumers index `% MOOD_COUNT` — raising MOOD_COUNT compiles clean and
    reads out of bounds.
94. LAW-5 — cartridge.hpp:409-417 — mood_name claims "compiler catches a
    missing entry": a sized aggregate only rejects EXCESS initializers; a
    missing entry value-initializes to nullptr, which mood_name returns.
95. LAW-5 — cartridge.hpp:2065-2078 + entity_pipeline.inl:206-216 — the
    max-tier capacity 8 is an unnamed magic number at two sites that must stay
    equal; generic_select zero-weights tiers ≥8 silently, then passes the
    UNCLAMPED count to select_tier_biased whose normalize/roll loops run to
    `count` over the 8-slot array — latent OOB (current max 4; TIER_SCALE_GOL
    declares 10). Also LAW-7.
96. LAW-5 — mood.inl:572-584 vs 738-750 (+ gallery.inl:351 as third authority)
    — the vault crown derivation ({0.45, 5.5, 8.0, 0.30, 5.0}) typed twice
    under "(matches crown computation in generate_indoor_shell)", with
    gallery's paint_y_frac/heights as a shadowed third address.
97. LAW-5 — mood.inl:426-429 vs render_passes.inl:689-690 — the spot-shadow
    FOV formula (2·outer+0.2, cap 2.8) exists as prose in one file and code in
    the other; no shared constants, no twin naming on the render side.
98. LAW-5 — input.inl:96-103 vs 250 — the KEY BINDING REGISTRY is a declared
    comment-mirror of the dispatch ("change both") and has already drifted:
    F8 → toggle_sky_mode exists in the dispatch; the registry ends at F7.
99. LAW-5 — orbs.inl:449-464 — orb_tier_block_ptr reinterpret_casts at literal
    byte offsets (192 + i·40; 416 + i·16) under a must-match comment; sole
    enforcement is sizeof==480, which cannot catch size-preserving reorders;
    offsetof is available and used elsewhere (state.hpp:2714) but not here.
    The fragility is live: two pads (428/444) are already repurposed with
    special cases.
100. LAW-5 — ribbon.inl:490-492 — HIST_CAP×HIST_DT > body-age invariant
     enforced by comment though every term is constexpr in the same file;
     violation is masked by the silent clamp at ribbon_history_sample:600-601.
101. LAW-5 — state.hpp:277-306 ↔ world.wgsl:1732-1752 — 21 coupling bits as
     independent literals both sides, no shared source, values verified equal
     today; the comment itself names the silent-corruption failure mode.
102. LAW-5 — state.hpp:556-560 et al. — every CPU↔WGSL struct mirror is
     size-pinned only ("field order is on the human"): GPUAgentBehaviorDef/
     TierDef, GPUPierInstance, GPUPaintingSlot, GPUTileGrid, six *MeshParams.
     The dt_beats defect (89) is this failure mode realized.
103. LAW-5 — world.wgsl:2115-2149 — POLICY_*_MASK consts must equal
     POLICIES[].contributors; C++ has DAG-closure asserts but nothing reaches
     the WGSL consts; each query_ground_* body re-states the contributor list
     by hand. Two readings recorded (docs-as-constants vs two-address problem).
104. LAW-5 — world.wgsl:8881, 9153-9154, 9497 — named bounds declared
     (AMG_MAX_PROFILE=49, CMG_MAX_PROFILE=32, CMG_MAX_DISCS=12, CMG_MAX_SA=29)
     while the arrays and guards they name use literals; WGSL accepts
     const-expressions in array sizes — construction enforcement available,
     unused.
105. LAW-5 — world.wgsl:7418-7431, 7488-7496 vs 2083 — the GoL suppression
     smoothstep SHAPE written three times, synchronized by prose; radii are
     shared constants (half the enforcement) but the shape could be a pure
     function of distance; the "cannot easily share" comment justifies not
     sharing the pawn READ, not the shape.
106. LAW-5 — renderer.hpp:53 [NO-BASELINE] — "S1 ENTRY POINTS — Must match
     world.wgsl §7": 64 lockstep strings verified in-step today, but not
     enforced by construction, and (per 118) creation failure likely cannot
     fire the nullptr checks — mismatch surfaces as unlabeled async error spam.
107. LAW-5 — renderer.hpp:574-575, 653/664/685, 446 [NO-BASELINE] — hardcoded
     dispatch factors under must-match comments: frustum "4" (=ceil(225/64)),
     zone "(4,4)" ×3 (vs @workgroup_size(8,8,1) and DYNAMIC z.grid_size — a
     zone with grid_size > 32 would silently part-evolve), agents "32" — three
     unlinked addresses each.
108. LAW-5/LAW-1 — renderer.hpp:2053, 2122, 2177, 2268 (+shadow twins 2828,
     2578, 2683, 2766) [NO-BASELINE] — vertex strides/attribute tables spelled
     twice (main vs shadow) as integer literals; the 44-byte zone vertex exists
     at FOUR addresses (2 renderer literals, state.hpp:3432 `* 44`,
     world.wgsl:4795 struct) with no shared constant or sizeof at any; only
     MeshVertex has a CPU struct assert.
109. LAW-5 — renderer.hpp:2438, 2536, 2564 ↔ state.hpp:3607/3619 [NO-BASELINE]
     — shadow depth format Depth32Float spelled raw at 5 sites, no shared
     constant, against the file's own better pattern (injected
     colorFormat_/depthFormat_).
110. LAW-5 — world.wgsl:737, 739, 752, 754 ↔ agents.inl:92-128 —
     AGENT_BEHAVIOR_COUNT_WGSL=10u / array<...,10> / tier 4 are unnamed twins
     of the CPU counts; the static_assert chain reaches state.hpp only — an
     11th behavior compiles clean on CPU while WGSL silently indexes 10.
111. LAW-5 — entities.inl:109, 217 — ARCH/COLUMN_PALETTE_COUNT hand-maintained
     beside literal arrays; zero asserts in the file; consumers modulo by COUNT
     (too small strands rows; too large reads OOB at agents.inl:600's runtime
     index).

### LAW-7 (silence at a boundary law-stated at the site)

112. LAW-7 — cartridge.hpp:629-633 — record_entity silently ignores the 11th
     ref (MAX_ENTITY_REFS=10); downstream, GoL/gallery commits assume success
     and ribbon late-registration increments ref_count even when the record was
     dropped (2914-2926) — unevictable slots; the only detector is compiled out
     (DIAG_ENTITY_LIFECYCLE).
113. LAW-7 — entity_pipeline.inl:519 (+8 sibling commit wrappers) — all nine
     dispatch_commit_*_generic silently drop the entity when find_patch fails:
     active cleared, but the footprint registered at placement is never
     unregistered and the population observation is already counted — a
     phantom footprint blocks nearby spawns; the place-phase drop IS law-stated
     (spawn_engine.inl:1071-1072), the commit-phase one is not.
114. LAW-7 — gallery.inl:607 — queue_promotion silently drops the staging→
     exhibition copy at MAX_PROMOTIONS_PER_FRAME (32) AFTER all four enqueue
     sites have already marked the exhibition layer occupied and the staging
     record consumed — a dropped promotion leaves an active slot displaying a
     stale texture. No law at the site.
115. LAW-7 — gallery.inl:1694 vs 346 — per-wall painting arrays hard-capped at
     8 (`std::min(count, 8u)`); the console dial per_wall_count_hi (5) is not
     linked to the bound; raising it past 8 silently discards. Also LAW-5
     (unlinked pair).
116. LAW-7 — gallery.inl:1732, 1788, 1840 — placement aborts mid-wall/
     mid-painting on slot/layer exhaustion with no statement at the site.
117. LAW-7 — gallery.inl:644 — photographer discards any frame where the pawn
     moved >5.0 units (teleport filter), undocumented; 5.0f is also an
     unconsoled dial (LAW-10).
118. LAW-7 — gallery.inl:1493 vs 1476 — manifest dedup bound `disk_idx < 256`:
     indices ≥256 silently exempt from dedup; the bound is commented, the
     exemption is not.
119. LAW-7 — gol_zones.inl:528 — commit_gol silently skips the GPU derive
     request when the request queue is full — zone stays CPU-active with
     underived GPU params. Two readings (practically unreachable vs defensive
     guard); undocumented either way.
120. LAW-7 — mood.inl:659 vs 1318 — the same out-of-range-mood boundary gets
     two different silent treatments (apply_mood clamps; request_mood_transition
     drops); neither is law-stated and the two policies disagree.
121. LAW-7 — mood.inl:855-858 + state.hpp:2568-2576 — shell mesh counts
     silently min'd against SHELL_MAX_*; the upload clamps bytes but records
     the UNCLAMPED index count as the draw count (stale-tail indexing);
     VAULT_N and SHELL_MAX_* are uncoupled dials.
122. LAW-7 — state.hpp:2260 vs 1928-1961 — upload_agent_slot silently swallows
     out-of-range slots while the cube partial-writers do no bounds check at
     all — the same error class takes two different silent paths.
123. LAW-7 — state.hpp:2466-2490, 2557-2560, 1853-1858 — origin/spot-VP uploads
     silently truncate via std::min with no site statement.
124. LAW-7 — state.hpp:2968-2978, 3397 — createBuffers validation omits
     ribbonBuffer_, agent registry buffers, floater readback staging;
     createGoLZoneBuffers omits zoneDeriveRequestBuffer_ — creation failure
     passes boot silently.
125. LAW-7 — world.wgsl:7250-7252 — zone_emit_quad reserves counters BEFORE the
     capacity check; on overflow it returns without writing, but the indirect
     draw consumes the already-incremented indexCount — silently draws indices
     never written. Neither stated nor safe.
126. LAW-7 — world.wgsl:9829 — palm frond clamp `min(…, 18u)` silently
     truncates authored intent (ROYAL palms author mean 15 σ2 → ~4% of rolls
     capped); sibling clamps equally undocumented; cactus arms have NO clamp
     (10144) where 8 arms would exceed slot capacity; blade clamps nothing —
     five sites, no policy stated anywhere (also L6-13).
127. LAW-7 — world.wgsl:2044-2062 — contrib_gol_zones_at returns on the FIRST
     zone containing xz; overlap silently dropped; no non-overlap precondition
     stated at the boundary.
128. LAW-7 — world.wgsl:172-174 — point_to_vec3 silently collapses near-ideal
     points to origin; callers propagate without note.
129. LAW-7 — ribbon.inl:1081 (+622) — silent clamp to RIBBON_MAX_RINGS with no
     law at the site — contrast the length cap two lines below carrying its
     rationale; currently unreachable, which is exactly why a future tier edit
     would never notice it engaging.
130. LAW-7 — renderer.hpp:1373-1378 [NO-BASELINE] — shader COMPILE failure is
     silent at the site (Dawn returns a non-null error module; loadShader
     returns true; hot reload proceeds to build 65 pipelines against a broken
     module); the missing-FILE case in the same function is handled loudly.
131. LAW-7 — renderer.hpp:203-212, 1287-1293 + incubator.cpp:202 [NO-BASELINE]
     — pipeline failure is boolean-swallowed at every level: tPipe prints the
     same line on success and failure; reload()'s false return is discarded at
     the call site; the 65 `!= nullptr` checks likely cannot fire for
     validation errors (non-null error objects), leaving only the unlabeled
     device callback.
132. LAW-7 — renderer.hpp:2394, 2470, 2861 [NO-BASELINE] — 23 of 26
     CreatePipelineLayout results are null-checked; these 3 are not — either
     the checks are the law (3 sites break it) or theater (23 sites carry dead
     ritual); the file states neither.
133. LAW-7 — cube_behaviors.inl:250 — out-of-range mood silently substitutes
     STATIONARY with no site comment, in a function whose other two fallbacks
     ARE law-stated (:265, :269).

### LAW-9 (GPU sovereignty)

134. LAW-9 — mood.inl:716-868 (indoor shell) — catenary vault tessellated
     CPU-side (33×33 grid, finite-difference normals, full VB/IB built and
     uploaded) with no law statement, while the same file's arches are
     GPU-realized and the cartridge header declares world.wgsl the single
     source of truth for geometry. Two readings (once-per-transition pragmatism
     vs unstated exception); the convention is stated nowhere at the site.
135. LAW-9 — render_passes.inl:643-713 — spot-light view-projection matrices
     computed CPU-side while camera VP and sun shadow VP are GPU-computed; the
     split is workable (once-per-transition) but nowhere law-stated.

### LAW-10 (control panel)

136. LAW-10 — ribbon.inl:88 — a TESTING value living in the shipped control
     panel: SPAWN_CHANCE 0.900f ("was 0.400f — revert before ship"). Ledgered
     in the constitution (TESTING(1), dies at ship) — the tag is the
     mitigation, not an erasure; the spawn-summary table still says 0.400
     (LAW-2 #45), so the two panels disagree.
    > static constexpr float SPAWN_CHANCE = 0.900f;   // TESTING: was 0.400f
137. LAW-10 — cartridge.hpp:293 — PORTAL_DENSITY=1.00f saturates its own dial:
     the consumer roll (entity_pipeline.inl:2157) is always true (dead code),
     the comment still says "fraction", and nothing marks 1.00 as final vs
     testing.
138. LAW-10 — state.hpp:96-100, 5849 — a test rig permanently occupies pier
     slots 0-2 under the S1 DIMENSIONS panel ("cartridge uploads test rig at
     setup"); slot 3 silently skipped (PIER_ARCH_BASE=4) with no comment.
139. LAW-10 — entity_pipeline.inl:1080, 1092 — rotation-hash property 355u as a
     bare literal inside two TRAITS tables, findable under no registry (the
     Property Index Registry omits it — LAW-2 #46). No console assembles for
     the pipeline/spawn engine at all.
140. LAW-10 — world.wgsl:6414-6417, 6443-6446, 6253, 6274, 6286, 6291, 7506,
     7587-7590 — design dials as function-local lets with no banner (curl/
     phase-wave force shapes, camera aim tau, clearance margins, aura shape
     numbers) — in a file that demonstrates the pattern (GoL visual dials ARE
     bannered at 4849-4880).
141. LAW-10 — world.wgsl:8017-8018, 8071, 8177-8190, 10878, 11248-11250, 11325,
     11330, 11336 — none of this range's dials appear in the file's TUNING
     SURFACE DIRECTORY (which registers dials only through §3.5).
142. LAW-10 — gallery.inl:285-286 vs 1610, 1688, 1694, 1712, 973-977, 1076,
     1108 — the WALL_ART console claims completeness ("No other edits needed")
     while the functions carry local dials (WALL_OFFSET, span floor, 8-cap,
     aspect estimate, FAVORITE_TIERS, frame heights, size floor).
143. LAW-10 — input.inl:269, 286 — mouse sensitivity 0.005f and scroll scale
     2.0f buried in handlers; input.inl has no tuning banner (one of the
     assigned modules with none).
144. LAW-10 — renderer.hpp:1052 [NO-BASELINE] — fade overlay visibility
     threshold as a bare literal in a draw method; findable under neither the
     GPU directory nor any renderer banner (the file has none).
145. LAW-10 (+LAW-1) — entity_pipeline.inl:1892-1894 — cube color character
     (three scale/offset literal pairs) inline in the adapter; cube is the only
     family of nine whose visual-identity dials have no vocabulary address.

### LAW-11 (mirror constants)

146. LAW-11 — **world.wgsl:5114, 5288 ↔ state.hpp:111-112 — UNEQUAL TWIN:
     zone_patch_instances declared `array<PatchInstance, 169>` and scanned
     `i < 169u` while the CPU binds the buffer at MAX_ACTIVE_PATCHES = 225;
     169 = 13² is the pre-gen window of an older radius. Patches in slots
     169-224 silently fall to the analytic fallback.** Two readings (fallback
     claimed value-equivalent vs bake/analytic drift over high slots); the twin
     is unequal and names no CPU constant either way.
    > @group(0) @binding(165) var<storage, read> zone_patch_instances: array<PatchInstance, 169>;
147. LAW-11 — **world.wgsl:6145-6148 — PHANTOM TWIN: FLOATER_EVICTION_RADIUS
     claims "MIRRORED MANUALLY in modules/cube_behaviors.inl"; no such constant
     exists anywhere on the C++ side (grep executed — only prose mentions in
     cartridge.hpp).** Also LAW-2.
    > // MIRRORED MANUALLY in modules/cube_behaviors.inl — currently
148. LAW-11 — state.hpp:89 ↔ world.wgsl:4378, 4756, 4770, 4776 — ring capacity
     400 as bare literals at four WGSL sites (8 occurrences in range, executed);
     no WGSL site names Dim::RIBBON_MAX_RINGS; a CPU bump silently truncates
     via the un-annotated zero-fill guard. Same genus:
     RIBBON_TUBE_VERTS_PER_SEG=24 ↔ TUBE_VERTS_PER_SEGMENT=24u and CAP 12 —
     no naming either side; the CPU set sizes the draw call, the WGSL set
     derives geometry: unequal edits over/under-draw silently.
149. LAW-11 — gol_zones.inl:65-70 ↔ world.wgsl:1488 — MODE_LATTICE_SPACING:
     CPU names the twin with full form; the WGSL site names nothing back.
     Half-named. Same one-sidedness: PULSE_ALGORITHM_CHANCE (WGSL names CPU at
     1712-1713; the CPU console site is silent), GoLZoneProp/SpawnConfig/
     ColorMode (WGSL 5143/5153 name CPU; CPU names nothing), seed_utils twins
     (CPU names GAUSSIAN_PAIR_OFFSET/hash_property/lattice_node_seed; WGSL side
     silent all three).
150. LAW-11 — orbs.inl:932, 949, 964, 979 ↔ world.wgsl:10914-11052 — workgroup
     width 64 hard-coded as `(count + 63u)/64u` at four dispatch sites
     mirroring @workgroup_size(64); neither side names the other; changing the
     WGSL size under-dispatches silently (orbs freeze). Same genus, CPU-side:
     the state.hpp dispatch divisors (/8, /16, /64) at 2833-2837, 2659.
151. LAW-11 — cartridge.hpp:2762 — `p.resolution = 256` shadows
     Dim::PATCH_HEIGHTFIELD_N (state.hpp:104) which sizes the textures,
     scratch, and workgroups; the authoring site names no twin.
152. LAW-11 — cartridge.hpp:3661 — `auraCfg.aura_n = 64` raw while
     GPUState::PAWN_AURA_N (state.hpp:1062) and WGSL PAWN_AURA_N
     (world.wgsl:5013) both exist; this is the one site that would silently
     disagree.
153. LAW-11 — state.hpp:3432 ↔ world.wgsl:4795 — CPU stride literal 44 twins
     struct CellMeshVertex; no C++ struct, no shared constant, no assert —
     a WGSL field addition silently desizes the buffer.
154. LAW-11 — world.wgsl:4404 ↔ ribbon.inl:242-243 ↔
     src/coupling/visual_canvas.hpp:139-141 — the Rodrigues chroma basis
     (0.8165, −0.4082, −0.4082) is a three-way twin: both CPU sites gesture at
     the shader without naming a symbol; the WGSL site names neither CPU twin;
     no tuning authority stated for the trio. Values equal today.
155. LAW-11 — world.wgsl:8019 ↔ cartridge.hpp:2806 — FRUSTUM_LOD0_RADIUS_SQ
     hardcodes 3.5 twinning LOD_FULL_RADIUS = 3.5f; the adjacent comment
     itself warns divergence causes boundary flicker — the exact risk
     twin-naming exists to prevent.
156. LAW-11 — world.wgsl:8162 ↔ state.hpp:139 — GALLERY_QUAD_N=8 twins
     PAINTING_QUAD_N=8 under a DIFFERENT NAME with no cross-reference (the CPU
     draw count breaks silently if either changes); plus 18+ same-name-but-
     unnamed *_MAX_SLOTS/_MAX_VERTS/_MAX_INDICES pairs (8595-10327 ↔
     state.hpp:144-188), all verified equal, zero twin-naming.
157. LAW-11 — world.wgsl:7106 ↔ state.hpp:216 — GOL_ZONE_STRIDE 7168u equal by
     arithmetic to GOL_ZONE_LIFE_STRIDE; the 7-plane layout stated
     independently at both, neither names the other.
158. LAW-11 — world.wgsl:6114-6118 ↔ agents.inl:185-195 — AGENT_EVICTION_RADIUS
     twins are named both sides and equal (the L2 exemplar) but neither states
     tuning authority — the one element of the MOUNT_* pattern they lack.
159. LAW-11 — renderer.hpp:2438 et al. ↔ state.hpp:3607/3619 [NO-BASELINE] —
     shadow depth format (see 109; dual-filed as the LAW-11 unnamed-twin case).

---

## L4 DRIFT

Statement-vs-reality pairs; both addresses, both quotes (condensed), and the
side judged current. Deduplicated across auditors.

1. A: cartridge.hpp:61-64, 233-236 "aura_presence is scheduled to migrate here"
   ↔ B: cartridge.hpp:249-254 "Migrated here from pawn_state_" (+pawn.inl:141).
   CURRENT: B — the field exists; both header SEAM[spine:P8] blocks still
   announce a completed migration as future. (Micro-drift at 254: "(was
   player_.aura_presence)" names the field's current name as its past.)
2. A: cartridge.hpp:961-962 (DENSITY table 0.1/3.0) ↔ B: 968-969 (both 1.0f).
   CURRENT: B — entity_density is provably constant 1.0.
3. A: cartridge.hpp:1829-1834 (POP BATCH table) ↔ B: 1844-1850 (constants).
   CURRENT: B — six of seven documented rows wrong.
4. A: cartridge.hpp:1486-1487 (THEME weights table) ↔ B: 1575, 1591 (live
   weights; density_mult 1.0 every row). CURRENT: B — the second boxed table
   (1532-1542) already agrees with code, isolating the first as stale.
5. A: cartridge.hpp:2110-2112 ("Arch→Pyramid = 0 … explicitly allowed") ↔
   B: 2118 (matrix 60.0f). CURRENT: B — enforced behavior is the opposite of
   the documented exception.
6. A: cartridge.hpp:3709 ("frustum cull bypassed") ↔ B: render_passes.inl:227,
   466-474 + mood.inl:664 + world.wgsl:8092 ("Re-enabled"). CURRENT: B — the
   cull runs and LOD0 draws indirect for outdoor moods.
7. A: mood.inl:487-491 + DONE[mood:K2] ("five sub-functions … band motion") ↔
   B: mood.inl:53-56 + bodies (four helpers; numbering skips 4; band motion at
   cartridge.hpp:3005). CURRENT: B — SEAM[mood:K1] is current; the banner and
   DONE tag were not updated same-commit.
8. A: state.hpp:484 (TILE_GRID_SIDE … // 17) ↔ B: state.hpp:489 ("// grid
   dimension (13)"). CURRENT: 17 — "(13)" dates from PREGEN_RADIUS=5.
9. A: state.hpp:1540/1545/1551 ("49 layers") ↔ B: 3544-3545 (label
   "225x256x256"). CURRENT: 225.
10. A: state.hpp:3777-3780 ("10-per-stage … 12-per-stage") ↔ B: 3906-3908
    ("8 storage buffer per-stage limit"). CURRENT: neither number alone —
    console.hpp:169-175 requests full adapter limits because the default 8 is
    too tight, and the compute-entity layout binds 10; "8" is only the WebGPU
    default. Both comments survive uncorrected.
11. A: state.hpp:21 ("26 solid_instances Uniform") ↔ B: 3736-3738
    (pier_instances, ReadOnlyStorage). CURRENT: B — doubly stale row.
12. A: state.hpp:4707 ("17 entries") ↔ B: 4709 (19 entries). CURRENT: B —
    agent registries appended without updating the count. Same class:
    state.hpp:5446 ("2 entries") vs 5448 (3 — orb_state_prev added).
13. A: world.wgsl:1426 (pulse onset "seconds") ↔ B: state.hpp:448/456
    ("onset_beats"). CURRENT: the shader contract (consumer computes
    `t_seconds - p.z`, world.wgsl:2503-2505); no live CPU writer exists to
    adjudicate — the park itself is the DRIVERLESS state (L3 #71 family).
14. A: world.wgsl:89 (directory row 0.25) ↔ B: world.wgsl:1523 (0.0 DISABLED).
    CURRENT: B.
15. A: state.hpp:453 ("kept size 384") ↔ B: state.hpp:1383
    (static_assert == 400). CURRENT: 400 — comment predates the lod_pawn block.
16. A: world.wgsl:69 + 1412 + state.hpp:430 ("3=grey") ↔ B: world.wgsl:1465
    ("3: warm — dusty terracotta … (was grey 0.07)"). CURRENT: B — three
    addresses stale.
17. A: world.wgsl:627 (dt_beats consumed by step_trigger) ↔ B: state.hpp:343
    (_pad1, zeroed every frame). CURRENT: B — runtime behavior is dt_beats==0;
    the WGSL name and its consumer describe a value that never arrives
    (the L3 #89 defect, recorded here as the drift pair).
18. A: world.wgsl:4296-4298 (BNK-1 "pending Jean's ruling") ↔
    B: world.wgsl:5527-5543 + ribbon.inl:131/795-806 (BNK-2 landed; saddle
    wears the full frame). CURRENT: B — the pending clause was resolved and
    never retired.
19. A: world.wgsl:10560-10565 ("uploaded per-frame") ↔ B: 10606-10610
    (DRIVERLESS tag) + orbs.inl zeros + zero upload call sites. CURRENT: B.
20. A: world.wgsl:10611 (motion_rule "3=Flocking") ↔ B: state.hpp:1132
    (comment lists 0-2 only). CURRENT: A — flocking is implemented
    (world.wgsl:11172-11277) and CPU cycles four rules; the CPU twin's comment
    predates Pass 9.
21. A: state.hpp:820 et al. ("around line 8369" etc.) ↔ B: world.wgsl actual
    struct addresses (8610, 8814, 9163, 9659, 9981, 10311). CURRENT: B —
    uniformly ~250 lines stale; layouts themselves still match.
22. A: entity_pipeline.inl:3-5 ("seven cookie-cutter families") ↔
    B: spawn_engine.inl:1028 ("all 9 migrated families") + executed counts
    (27 wrappers, 9 generic rows). CURRENT: B.
23. A: entities.inl:669-671 + spawn_engine.inl:29-32 (preparers "currently live
    in spawn_engine.inl") ↔ B: entities.inl:712-758 (+cartridge.hpp:1013-1027).
    CURRENT: B — migration #10 landed; two files still narrate the dead state.
24. A: cube_behaviors.inl:47-48, 452 ("Called from update()") ↔
    B: cartridge.hpp:3509 (inside render()). CURRENT: B.
25. A: input.inl:97-103 (registry ends at F7) ↔ B: input.inl:250 (F8 →
    toggle_sky_mode). CURRENT: B — the registry's own header demands both be
    changed together.
26. A: input.inl:102 ("F6 … teleport cubes") ↔ B: cube_behaviors.inl:294-298 +
    429 (4s smooth-step glide). CURRENT: B. (Related: cartridge.hpp:3506
    "~3s" vs CUBE_CORRAL_DURATION = 4.0f.)
27. A: orbs.inl:24, 787 ("KP_0") ↔ B: input.inl:226 (GLFW_KEY_0). CURRENT: B.
28. A: gallery.inl:249-256 (70/20/10, 40/20/40) ↔ B: 258-260, 360-362
    (80/5/15, 15/5/80). CURRENT: B.
29. A: gallery.inl:575, 1384, 1532 ("alphabetical, at startup") ↔ B: 1421-1434,
    1443-1446 (numeric, lazy). CURRENT: B.
30. A: gol_zones.inl:60-61 ("those are the per-tier consoles") ↔
    B: world.wgsl:1666-1667 ("single source of truth"). CURRENT: NEITHER
    exclusively — both tables are live consumers (CPU: tick/seed; GPU:
    visuals); the WGSL claim is the more falsified; the CPU claim omits that a
    tuner must edit the twin. The authority contradiction is itself the
    finding.
31. A: ribbon.inl:995-997 ("the gen-2 color coupling … will write through") ↔
    B: ribbon.inl:924-936 (the flush computes the lerp now). CURRENT: B —
    future tense on a landed feature.
32. A: input.inl:327-330 ("Stage 1 … fixed sky altitude … Stages 2-3 add pawn
    snap, camera follow, fade") ↔ B: cartridge.hpp:3565-3580 + wgsl:5515-5535
    (snap + frame landed) + ribbon.inl:725-747 (altitude pen, not fixed).
    CURRENT: B — only the fade transition is unfound in-tree; ribbon.inl:
    697-702 carries the same fossil, contradicted 25 lines below.
33. A: renderer.hpp:8-31 (header pipeline tables) ↔ B: creation bodies
    (31 compute, 34 render; pyramid dead; painting shadows never drawn).
    CURRENT: B [NO-BASELINE].
34. A: src/docs/cartridge_constitution.md §5 "EVICTION THUNKS (1 class, 13
    functions)" ↔ B: the_board cartridge.hpp — `grep -c "static void
    dispatch_evict_"` = **12** (no dispatch_evict_noop; the 13th exists only in
    the sibling cartridge). CURRENT: B — under the doc's own per-cartridge
    counting convention the number is wrong for the_board and the divergence is
    not declared. (Independently re-executed by the synthesizer: 12 vs 13.)
35. A: src/docs/ribbon_color_coupling_datasheet.md:37-38 ("written at commit,
    static per life today") ↔ B: ribbon.inl:908-936, 998-1004 (time, color,
    amps flushed per frame). CURRENT: B — the §1 banner lags the LIVE rows
    beneath it; alternative reading (the "noted where" clause delegates) makes
    it misleading rather than false.
36. A: datasheet BNK-2 ledger ("frame at seat age, position at head age — a
    deliberate seam") ↔ B: ribbon.inl:774-783 (ONE age; SNAP-1) + the
    datasheet's own later SNAP-1 entry ("Ruling reversed on the record").
    CURRENT: B — superseded narration, no forward pointer at the stale
    paragraph.
37. A: agents.inl:426-427 (storage buffers) ↔ B: state.hpp:2872-2875 + WGSL
    var<uniform>. CURRENT: B (dual-filed with L3 #47 as its disease form).

---

## L5 CONSTRAINTS

The hard-limit list any future refactor must honor. Addresses in-tree.

**GPU backend (Windows D3D12 / FXC):**
- Storage buffers/stage = 10; uniform buffers/stage = 12 (world.wgsl:55; the
  app requests full adapter limits, console.hpp:169-175, because default 8 is
  too tight; compute-entity layout binds 10 — state.hpp:2872-2875, 3777-3780).
- One DrawIndexedIndirect per render pass (world.wgsl:54; state.hpp:1713-1714;
  renderer.hpp:801-803) — forces the LOD0-indirect/LOD1-direct split.
- Texture-array stamps in the collision chain hang FXC (world.wgsl:53).
- Collision/ground chain admits no new runtime branching; loops bounded by
  uniform counts; dispatch by uniform function choice (world.wgsl:49-52,
  1950-1954, 2569-2573, cited to the ground design docs).
- Hot fused paths must stay hand-fused (patch_terrain_vs ~256×256
  invocations/patch; ground_formed_with_complexity per-texel)
  (world.wgsl:2270-2282); FXC loop unrolling compounds repeated zone-loop
  evaluation (2074-2079).
- Instance structs in hot loops stay lean and byte-pinned: GPUPierInstance
  48 B static_assert (state.hpp:738; world.wgsl:46-48 — successor of the
  retired 32-byte SolidInstance rule).
- FXC requires compile-time constants — no runtime upload for eviction radii;
  the stated cause of the manual mirrors and the agent-kernel split
  (world.wgsl:6084-6118).
- WGSL vec3 16-byte alignment: pawn_offset must sit at offset 176 or GPU
  struct grows 208→224 (state.hpp:673-679). WebGPU access mode must match
  layout exactly — aliased orb bindings 413/414 (state.hpp:4537-4543).
- WGSL has no function pointers — cube behavior dispatch stays a hand switch
  (cube_behaviors.inl:75-80; world.wgsl:6462-6468).
- Swapchain format unknown until init — two-phase texture init; BGRA8 forces
  CPU R↔B swizzle per authored painting (state.hpp:2015-2063, 5139-5140).

**Capacities (all verified against both sides where twinned):**
- MAX_ACTIVE_PATCHES 225 (15² pregen); render 11×11; priority 7×7;
  PATCH_EXTENT 50 (state.hpp:103-112). Patch streaming budgets: SPAWN/ALLOC/
  EVICT 4/frame; heightfield 1-6/frame escalating (cartridge.hpp:831-866).
- MAX_ENTITY_REFS 10/patch, silent overflow (cartridge.hpp:625-633 — L3 #112).
- alloc_layer exhaustion recycles layer 0, documented (cartridge.hpp:2776-2783).
- MAX_TERRAIN_TOKENS 8; evicts min budget (comment says oldest —
  cartridge.hpp:2419-2431).
- Tiers per family ≤8 implicit (weights[8], two unnamed sites — L3 #95).
- MAX_AGENTS 32, slot 0 = player (state.hpp:229; world.wgsl:4688);
  agent evict 360 / spawn 340 (20-unit alive band, agents.inl:190-196);
  floater evict 400 = 350 + 50 headroom (world.wgsl:6128-6148).
- Floating slots 8 spheres + 256 cubes = 264 (state.hpp:204-212 ↔
  world.wgsl:1573-1576, verified); update_cube @workgroup_size(1) ≈ 7.5K
  ops/frame (state.hpp:205-210); MAX_ORBS 256, O(N²) flocking budget stated
  (world.wgsl:11174-11177).
- RIBBON_MAX_RINGS 400 (state.hpp:89 ↔ four bare WGSL literals);
  RIBBON_MAX_LENGTH 700 enforced at fill (ribbon.inl:430, 1086-1087);
  MAX_RIBBON_INSTANCES 1 (ribbon.inl:429); history 1024×0.05s = 51.2s > ~29.2s
  body age, comment-enforced (ribbon.inl:490-492); GPURibbonState frozen at
  112 B with offset pins (state.hpp:1415-1419); steering R_MIN 40, no reverse
  (ribbon.inl:713-715); ±3σ Gaussian truncation both sides (seed_utils.inl:75 ↔
  world.wgsl:299); Gaussian pair offset +1000 (seed_utils.inl:73 ↔ wgsl:291).
- GoL: 8 zones max; 32×32 cells; 7-plane life stride 7168 (state.hpp:201-216 ↔
  world.wgsl:7106); zone mesh 50k verts/75k indices per zone, 200k/300k total
  (state.hpp:218-222); one zone per patch (gol_zones.inl:453); zone terrain
  sampling scans slots 0-168 only (world.wgsl:5114 — L3 #146).
- Gallery: STAGING_LAYERS 16; EXHIBITION_LAYERS 32; PAINTING_MAX_SLOTS 32;
  MAX_GALLERIES 48; MAX_PROMOTIONS_PER_FRAME 32 (silent drop — L3 #114);
  8 paintings/wall hard cap (unlinked to console — L3 #115); burst 4 shots,
  12-frame cooldown; PAINTING_RESOLUTION 1024², downscale-only.
- Mesh-gen slot budgets: arch 2000 v/7500 i vs monumental worst case 1470/7488
  — 12 indices of headroom (world.wgsl:8796-8797, state.hpp:146-147,
  recomputed exact); arch profile arrays hold su+1 ≤ 49, largest authored 48 —
  one future tier overflows silently (8895-8898); column pc ≤ 32 / discs ≤ 12
  holds distributionally, not by construction (9240-9245); palm all-clamps-max
  1549 > 1200 slot — safety rests on authored tiers (9733-9734); cactus arms
  unclamped (10144); blade unclamped (10378-10379); per-slot index budgets
  divisible by 3 (state.hpp:147-186).
- Shell: SHELL_MAX 2048 v/8192 i vs VAULT_N=32 worst 1105/6168 — uncoupled
  dials (state.hpp:131-132; mood.inl:855-858). Spots: MAX_SPOT_LIGHTS 4
  (2 per 4096-map atlas); outer cone ≤1.3 rad so FOV ≤2.8 (mood.inl:426-429 ↔
  render_passes.inl:689-690).
- Aura: PAWN_AURA_N 64 × 3.125 → ±100 u influence window (world.wgsl:5059-5064).
- Pier slot map: PIER_TOTAL 68 = rig 3 (+1 skipped) + 32 + 32
  (state.hpp:96-100).
- MAX_FOOTPRINTS 128 — full registry refuses spawns (spawn_engine.inl:516);
  ENTITY_CULL_BASE 350 + 50 hysteresis (318-321); catenary bisection 50 iters
  (876-885); MAX_ENTITY_PARAMS 32; MAX_COLOR_CHANNELS 12 (entity_types.inl:
  53-54); property 7777u reserved (entity_pipeline.inl:149-171).
- Readback: SPAWN_PROTECTION_S 0.10s race window (cartridge.hpp:3458); never
  force readback IDLE during teardown (3263-3267); frustum cull dispatch covers
  256 patches (4×64) vs 225 — 31 headroom, comment-enforced
  (renderer.hpp:574-575) [NO-BASELINE]; zone dispatch covers 32×32 cells
  regardless of dynamic grid_size (renderer.hpp:653) [NO-BASELINE]; shader
  search fixed to 2 cwd-relative paths (renderer.hpp:1336-1339) [NO-BASELINE].
- Determinism freeze: FloatingEntityProp name and all property indices frozen
  (renaming shifts every rendered world — floater_vocabulary.inl:87-89);
  MOOD_COUNT=6 sizes every MOOD_MULTIPLIER row asymmetrically (deficit
  zero-fills silently — entities.inl:122 et al.); promoting a WGSL dial to CPU
  costs ~16 B uniform + 4 lines (cube_behaviors.inl:142-143).

---

## L6 COLLAPSE TARGETS

One law, N authors. The law, the author list, the count. No fix designs.

1. **The generic-family concept** — ~50 authoring sections across 8 files for
   one new family (executed cactus enumeration; L1). Authors: entities.inl;
   entity_pipeline.inl; spawn_engine.inl; cartridge.hpp; state.hpp;
   renderer.hpp; render_passes.inl; world.wgsl. Count: 50.
2. **The family concept inside cartridge.hpp alone** — dispatch row, wrapper
   cluster, MIN_SEPARATION, POP_CROSS_AFFINITY, TIER_SCALE + switch, tier_wt
   field, 5 THEMES weights (+PopFamily elsewhere). Count: 8.
3. **The mood concept** — MOOD_TABLE, MOOD_* ids, mood_name, PORTAL_COLORS,
   pick_portal_mood thresholds (+3 assert-locked satellite tables). Count: 5+3.
4. **GOL_TIERS/PULSE_TIERS** — full dual-home tables, both live, contradictory
   authority banners (gol_zones.inl:187/255; world.wgsl:1652/1706). Count: 2
   homes × ~145 literals. Related: zone seed-prop indices ×4 and zone scalar
   consts ×3 addresses; patch cell size 3.125 at 3 unlinked addresses
   (gol_zones.inl:71, world.wgsl:5145, state.hpp:105); corner cell-snap formula
   CPU+GPU (gol_zones.inl:383-386, world.wgsl:5185-5186).
5. **Cumulative-weight picker** — canonical select_tier + 11 hand-rolled
   copies (executed grep; SEAM[seed_utils:Q10-target] tags the duplication as
   pending). Authors: seed_utils.inl:91; agents.inl:547,560; gallery.inl:478,
   1704,1742; gol_zones.inl:407,425; orbs.inl:565; ribbon.inl:1106,1171.
   Count: 12.
6. **Lockstep-by-prose census** — 50 `must match` + 18 `keep in sync` comments
   tree-wide; 27 in world.wgsl alone; 13 positional-lockstep tables with the
   per-row assert idiom available but unapplied. Count: 68 comment-enforced
   invariants.
7. **Ghost-document protocol** — 18 citations to four design docs absent from
   the tree (12 of them mandatory-update instructions). Authors: state.hpp;
   world.wgsl; agents.inl; cartridge.hpp; ground_architecture.inl. Count: 18.
8. **The ribbon (bespoke) concept** — 10 functional files to exist end-to-end;
   the sel→plan field list hand-transcribed ×3 (place, mood, commit — the mood
   seed omission is the recorded casualty). Count: 10 files / 3 copies.
9. **Main/shadow VS twin duplication** — pawn, gallery-frame, ribbon
   vid-decodes duplicated verbatim; renderer vertex layouts spelled twice
   (zone/mesh/arch/shell) [NO-BASELINE]; 15 shadow VS re-derive placement
   [TASTE for the family, LAW for the three verbatim decode copies].
   Count: 3 decode pairs + 4 layout pairs.
10. **Finite-world bounds formula** — mood.inl ×5 (536, 576, 717, 1022, 1154).
    Count: 5.
11. **Deterministic-randomness re-authorship** — hash_property cloned ×2
    (cactus_hash, blade_hash); golden angle ×3. Count: 5.
12. **Six-family mesh-gen scaffolding** — per-family 10-float vertex writer ×6
    (binding-forced), buffer-creation methods ×6 (state.hpp:3151-3319), slot
    upload methods ×6 (state.hpp:1970-2547), layout/group pairs ×12, max-active
    scan ×6 (entities.inl:673-758). Counts: 6/6/6/12/6. [Partly WGSL-forced;
    the CPU-side sextuplication is not.]
13. **Boundary clamp policy across sibling generators** — palm clamps four
    counts, cactus clamps segs not arms, blade nothing, column guards antenna
    discs only — five sites, no stated policy. Count: 5.
14. **Binding numbers hand-written twice** — every layout entry vs bind-group
    entry across ~19 pairs (state.hpp:3683-5642). Count: 19 pairs.
15. **Config mutation** — ~25 dirty-flagged setters + raw config() bypass.
    Count: 2 doors, 26 authors.
16. **The vault crown formula** — mood.inl ×2 + gallery.inl shadowed authority.
    Count: 3.
17. **Doorway wall margin** — mood.inl ×2. Count: 2.
18. **Renderer whole-file cloning** [NO-BASELINE] — the Renderer concept at 3
    addresses (the_board; backup_board tagged frozen, 34-line diff; the_chord
    divergent 2001-line sibling). Count: 3.
19. **Antenna tier offset convention** — +COLUMN_TIER_COUNT applied/subtracted
    at 5 sites in one family block, sequenced only by call order. Count: 5.
20. **Vegetation color bases** — named dead constants + live literals, 6 values
    × 2 addresses. Count: 12.
21. **SNAP-1/render-order law** — narrated at 4 prose addresses, enforced only
    by call order at one code site (constitution §3; datasheet M5;
    cartridge.hpp:3168-3177, 3565-3572). Count: 4 narrations / 1 enforcement.
22. **Chroma basis** — 3 independent constant sets, one-directional naming
    (ribbon.inl, visual_canvas.hpp, world.wgsl). Count: 3.
23. **Agent-state byte size** — 3 statements, one false (96/96/80). Count: 3.
24. [TASTE] **Cookie-cutter per-family blocks** — 9 parallel 10-element blocks,
    self-declared intentional ("Don't fight the cookie-cutter"); every
    cross-cutting change touches 9 blocks. Count: 9.
25. [TASTE] **Migration tombstones** — 12 vacated-address breadcrumbs
    (cartridge.hpp). Count: 12.
26. [TASTE] **Setter boilerplate** — compare-set-dirty ×25 (state.hpp S9).
    Count: 25.
27. [TASTE] **Literal hash-salt numerology in mood.inl** — 8 salts against the
    house registry convention. Count: 8.
28. [TASTE] **Key-map dual address** — registry comment table + dispatch switch
    (input.inl), already drifted once. Count: 2.
29. [TASTE] **100 BPM calibration anchor** — two independent code constants +
    three doc statements, authored per-module by design (contract R20), no site
    names the others. Count: 5.
30. [TASTE] **Pipeline-layout boilerplate** [NO-BASELINE] — the same 6-line
    block ×26, including 3 byte-identical rebuilds of the patch-gen layout.
    Count: 26.

---

## L7 RULINGS NEEDED

Questions only the operator can answer; both options' costs stated.

1. **LAW-8 encoding law (from L0).** The recorded law (BOM+CRLF for state.hpp/
   cartridge.hpp/world.wgsl) is systematically contradicted by the tree (all
   LF; those three files BOM-less; 7 other files BOM'd). Options: (a) the law
   changed — re-record it to match the tree (and reconcile with the in-repo
   constitution §0, which asserts only ribbon.inl-BOM + world.wgsl-BOM-free-LF,
   both of which hold); cost: the old law's rationale is lost. (b) the tree
   drifted — normalize 21 files; cost: whole-file churn on every mirror,
   BOM-sensitive tooling risk, and git-blame noise. (c) checkout normalization
   artifact — verify on a Windows clone with `git config core.autocrlf` known;
   cost: the audit's matrix must be re-run there before either side is judged.
2. **The dt_beats/_pad1 pair (L3 #89).** Park or defect? (a) CPU writes the
   beat delta: all eight step-driven agent behaviors activate at once — a
   visible behavior change that may be intended-off; touches the byte-for-byte
   contract at three addresses. (b) WGSL renames to a pad and step_trigger is
   re-derived later: agents stay step-less; loses the field name documenting
   intent. (c) leave with a DRIVERLESS tag at world.wgsl:627: cheapest; a
   structurally-unfireable trigger keeps compiling and every future reader
   re-derives this audit.
3. **Zeroed population-batch machinery + saturated/neutralized dials
   (L3 #66, #137, #17).** Final design or parked experiment? Final: stale
   tables rewritten, matrix + density lattice deletable (~200 lines, the
   retune capability lost). Parked: LAW-3 tags/bounds/citations required, and
   the per-tile density lattice keeps computing a constant 1.0.
4. **SPAWN_CHANCE 0.900 (L3 #136).** Revert now (loses ribbon-dev visibility
   mid-branch), keep until certification closes (ships 2.25× density if the
   tag is missed — it is the tree's only TESTING tag, so the sweep is cheap),
   or gate behind a build flag (one more #ifdef in a plain-constants panel).
5. **zone_patch_instances 169 vs 225 (L3 #146).** Intentional scan budget or
   stale 13²? Keep: slots 169-224 pay analytic fallback and silently trust its
   value-equality with the bake. Raise: 56 extra iterations per corner sample
   in a hot loop, and the literal must be twinned to prevent recurrence.
6. **Ghost design docs (L6-7).** Resurrect the four cited docs into src/docs/
   (docs may never have existed in this repo's history; writing effort; keeps
   the cross-side update protocol meaningful) vs rewrite 18 citations to
   in-tree anchors (mechanical; the pair-manifest idea dies undocumented).
7. **GOL_TIERS/PULSE_TIERS dual home (L6-4).** Keep dual derivation (~145
   literals ×2, silent drift risk between CPU life seeding and GPU visuals);
   upload once (uniform cost + loses FXC const-inlining the fxc banner
   protects; CPU still needs tick_period); or cross-name + guard (cheapest;
   both authority banners must be rewritten truthfully regardless).
8. **CPU↔WGSL mirror enforcement depth (L3 #102).** Per-field offsetof asserts
   (~hundreds of lines, still one-sided); a generator emitting both sides
   (build machinery grafted onto a hand-authored header ethos); or status quo
   (a self-documented silent-corruption channel across ~14 structs — the
   dt_beats defect is this channel realized once).
9. **Pipeline failure detection** [NO-BASELINE] (L3 #130-131). Keep bool+null
   checks (likely cannot fire for validation errors; hot-reload typo reads as
   success + unlabeled async spam); adopt error scopes wrapped once inside
   tPipe (async plumbing; failures become label-attributable; reload can
   report honestly); or minimum: print FAILED-label on ok==false and stop
   discarding reload()'s return at incubator.cpp:202.
10. **Dead pyramid + painting-shadow pipelines** [NO-BASELINE] (L3 #84-85).
    Delete (4 fewer compiles per boot/reload; restoring mesh-drawn pyramids
    later means ~60 lines + config re-derivation) vs tag as parked (keeps
    compile cost; requires the LAW-3 citation the sites lack; the header table
    must be corrected either way).
11. **record_entity's silent 10-ref cap (L3 #112).** Raise (real cap unknown
    without a worst-case census); surface (touches every commit path + ribbon
    late-registration accounting); or document as accepted loss (legal under
    LAW-7 only with the statement at the site; the unevictable-slot leak
    becomes accepted behavior).
12. **Commit-phase host-miss leak (L3 #113).** Add unregister + diagnostics
    (widens the generic contract; footprint index must travel through
    PlacementEntry) vs leave (phantom footprints block spawns until an
    unrelated eviction; population biasing fed observations of entities that
    do not exist).
13. **Arch portal prop collisions 703/803 (L3 #14).** Re-key (changes portal
    placement for every existing world seed — breaks the determinism promise
    across versions) vs accept (keeps a correlation the registry brands a bug;
    the mask hiding half of it un-masks if PORTAL_DENSITY is ever lowered).
14. **Indoor shell CPU tessellation (L3 #134).** Law-state the exception
    (cheapest; the crown-height twin stays comment-locked; LAW-9 gains a
    citable precedent) vs migrate to GPU (unifies sovereignty, kills the crown
    duplicate; a new compute path for once-per-transition geometry).
15. **Eviction-radius mirrors' tuning authority (L3 #158, #147).** Adopt the
    MOUNT_* declared-authority pattern (one sentence per site; the CPU
    spawn-headroom coupling then depends on a value settled elsewhere) vs
    leave undeclared (two-sided edits race; the floater mirror currently
    points at a file with no constant).
16. **Mood out-of-range policy (L3 #120).** Clamp everywhere (corrupt ids
    degrade gracefully, authoring bugs masked) vs reject everywhere (surfaces
    bugs; a hard reject inside the portal flow would strand a mid-fade
    transition) — either way one line at each site; today the two sites
    disagree silently.
17. **The 405-line wrapper layer.** Keep explicit glue (every new family adds
    ~35 boilerplate lines; the evict wrappers hand-repeat choreography where a
    missed dirty-flag is a silent bug — dispatch_evict_arch omits
    ground_entries_dirty while four siblings set it: deliberate or drift,
    undecidable in-file) vs generate (greppability lost; three genuinely
    bespoke families resist uniform generation).
18. **weights[8] tier capacity (L3 #95).** Name and share the constant (an
    include-order question: entity_types.inl is included after cartridge.hpp
    declares select_tier_biased) vs leave (first >8-tier family gets silent
    zero-weights and an OOB read with no diagnostic at either site).
19. **Header BINDING MAP (L3 #23).** Hand-maintain (history says it rots);
    generate (tooling absent; layouts live in one 2000-line function); delete
    (loses the only overview of a 500-number binding namespace).
20. **KEY BINDING REGISTRY table (L3 #98).** Keep prose (the drift class
    already recurred), generate, or delete in favor of the dispatch — the
    header's own "bindings are temporary" doctrine argues against investing.
21. **Palm/cactus/blade count-clamp policy (L3 #126, L6-13).** CPU ceilings in
    TierParamDef (single authored source; GPU trusts uploads; every param needs
    a hand-derived ceiling) vs GPU clamps (defense in depth; today's clamps
    already silently rewrite ~4% of ROYAL palms and are unregistered dials).
22. **MODE_COUPLING_MAGNITUDE=0.0 (L3 #28).** Permanently dead (delete dial,
    directory row, and the per-sample machinery — clean YAGNI, loses a
    documented compositional axis) vs dormant (retag as parked with revival
    condition; the 0.25 directory row must not survive as-is either way).
23. **CPU-side GPU-only property indices in gol_zones (L3 #77).** Delete (CPU
    loses its only record of which band-250 indices are taken; next CPU roll
    risks silent seed collision) vs keep-as-registry (requires the LAW-11 twin
    tags they lack; keep-as-is satisfies neither law).
24. **Vault crown math (L3 #96).** Extract a shared helper (trivially cheap
    in-file; sourcing gallery's constants adds a cross-module dependency) vs
    keep (retuning gallery heights silently desynchronizes the camera clamp
    from the vault; the ≈5.5 half-height already understates gallery's 7.0).
25. **Spot/point-light residue (L3 #82).** Delete the zero-count re-upload and
    possibly buffer/binding 321 (cross-file surgery for a feature no one
    authors) vs tag as parked (one comment + move the zero-write to init).
26. **Per-family sub-banners vs TUNING CONSOLE convention (L1 consoles).**
    Rule the P10 sub-banners sufficient (two coexisting conventions; a
    'TUNING CONSOLE' grep misses every grounded-family dial) vs insufficient
    (consolidation breaks the block locality that SEAM[entities:P10] declares
    intentional).
27. **Suppression-shape triple (L3 #105).** Accept prose contract (radii
    already shared; shape edits must land at three addresses across two
    stages) vs factor the shape into a pure distance function (touches three
    shaders for a one-line function; removes the last unshared component).
28. **Vertex-layout derivation** [NO-BASELINE] (L3 #108). Derive strides/attrs
    from state.hpp structs + asserts (8 sites here + a new CPU struct for the
    zone vertex + twin naming) vs accept (deferred cost lands on whoever adds
    a vertex field and updates 3 of 4 addresses).
29. **From the in-repo docs' own open ledger** (recorded as operator questions
    the tree cannot answer): F3 — strip or keep the 33 HISTORICAL NARRATION
    tags (strip: one sweep commit + mirrors, archaeology moves to git; keep: 33
    past-tense comments permanently violate the present-behavior rule);
    BREATH constant — give the re-articulation dip its own span (one constant
    + call-site selection + datasheet row) or keep it sharing
    RIBBON_SWELL_RELEASE (two perceptually distinct gestures stay coupled);
    circle-of-fifths re-seating — one line at the tint decode re-maps harmonic
    distance to angular distance vs keeping semitone-adjacent hues;
    CB-2 idiom fork — evolve-in-place (two time regimes on one tube) vs
    travel-at-P (grid becomes a delay line); certification C1-C6 — the
    transcript's performed half awaits the operator's build/screen gates.

---

## L8 SELF-CLAIM VERDICTS

File-header self-claims, DONE tags, and dependency lists verified against the
bodies they describe. 35 verified; 21 HOLD, 14 DRIFTED.

| # | claim (address) | verdict | evidence (executed) |
|---|---|---|---|
| 1 | SEAM[spine:K2-related]: wrappers "~400 lines below FamilyDispatch" (cartridge.hpp:51-53) | HOLDS | sed 1010-1414 → 405 lines; 43 wrappers counted; bodies bind-only |
| 2 | FAMILY_DISPATCH "ties the 12 entity families together" (cartridge.hpp:10,46) | HOLDS | 12 rows counted; PopFamily::COUNT=12 |
| 3 | DONE[mood:L1]: has_anchor_ribbon replaced scattered `mood == 5` checks (cartridge.hpp:388-394) | HOLDS | zero live `mood == 5` checks (grep); profile-driven gating at mood.inl:596, cartridge.hpp:3307 |
| 4 | Mood-ID banner: AGENT_POPULATIONS per-row asserts catch reordering (cartridge.hpp:366-371) | HOLDS | agents.inl:406, 414-419 |
| 5 | Terrain-mirror tombstone: only estimate_terrain_height survives, for ribbon (cartridge.hpp:505-508) | HOLDS | definition + exactly 2 callers, both ribbon.inl (1019, 1050) |
| 6 | SEAM[spine:owns]: stream_patches "~460 lines" with the advertised phases (cartridge.hpp:3731-3734) | HOLDS | awk 3735-4194 → 460; phases in stated order |
| 7 | SEAM[spine:P8]: aura_presence migration "scheduled" (cartridge.hpp:61-64, 233-236) | DRIFTED | migration already landed at :254; pawn.inl reads player_ exclusively |
| 8 | mood_name: "compiler catches a missing entry" (cartridge.hpp:409-411) | DRIFTED | sized aggregate rejects only excess; deficit yields nullptr return |
| 9 | GPURibbonState "112 total (mirrors world.wgsl RibbonState)" (state.hpp:711) | HOLDS | static_assert 112 + 4 offset pins; WGSL names it back byte-for-byte (827) |
| 10 | pier_count at config offset 124 (state.hpp:1814-1816) | HOLDS | manual field walk = 124; adjacent static_assert |
| 11 | GPUPatchGrid sized to 15² = 225 (state.hpp:1369) | HOLDS | constants chain + size assert 16+225·4 |
| 12 | Cube kernel "~7.5K ops/frame"; TOTAL_FLOATING_SLOTS 264 (state.hpp:205-212) | HOLDS | 256·30=7680; 8+256=264 |
| 13 | upload_cube_behavior_id avoids re-uploading "the whole 192-byte struct" (state.hpp:1926) | DRIFTED | struct is 208 B (assert :1414); 192 predates kite fields |
| 14 | possessed_slot "piggybacks … (kept size 384)" (state.hpp:453) | DRIFTED | assert says 400; lod_pawn block occupies 384-400 |
| 15 | heightfield arrays "49 layers" (state.hpp:1540) | DRIFTED | created with 225 (label agrees) |
| 16 | world.wgsl:46-48: GPUPierInstance 48 B pinned in state.hpp | HOLDS | static_assert :738; WGSL struct sums to 48 |
| 17 | world.wgsl:7-8: §2.1 structs mirror C++ "byte-for-byte" | DRIFTED | sizes hold (336==336) but field semantics broken at offset 300 (dt_beats ↔ _pad1) — the mirror's stated purpose fails |
| 18 | world.wgsl:10-12: §1.5 randomness bit-identical to seed_utils | HOLDS | all magic constants equal (hash, lattice, gaussian, +1000, 1e-6); caveat: transcendental ULP unverifiable from source |
| 19 | query_ground_walker_pair ≡ separate walker+tilt calls (world.wgsl:2735-2738) | HOLDS | term-by-term identical contributor sums and suppression |
| 20 | RIBBON frame-law mirror + authority + drift test (world.wgsl:4272 ↔ ribbon.inl:131-138) | HOLDS | values 1.0/0.9/0.6 both sides; formulas match incl. negations; GPU authority stated |
| 21 | FLOATER_EVICTION_RADIUS "MIRRORED MANUALLY in cube_behaviors.inl" (world.wgsl:6145) | DRIFTED | no such constant exists anywhere C++-side (grep) |
| 22 | agent_state array sized to Dim::MAX_AGENTS 32 "keep in sync" (world.wgsl:4687) | HOLDS | state.hpp:229 = 32; loops/workgroups consistent |
| 23 | orb speed_mult at offset 444, applied to each rule's dominant speed (world.wgsl:10733) | HOLDS | byte-walk + CPU twin comment + writer + 3 consumers |
| 24 | UnifiedPaintingSlot must match GPUPaintingSlot, 128 B (world.wgsl:8118) | HOLDS | field-for-field equal; CPU assert + back-naming |
| 25 | §ORB "musical couplings uploaded per-frame" (world.wgsl:10560-10565) | DRIFTED | all four zeroed at configure; upload helpers have zero call sites; struct's own DRIVERLESS tag is current |
| 26 | arch mesh-gen dispatch "(16, 4, 1)" (world.wgsl:8786-8792) | HOLDS | renderer.hpp:719 + MAX_ARCH_INSTANCES=16 + workgroup (1,1,1); siblings verified too |
| 27 | flocking O(N²) tractable at MAX_ORBS=256 (world.wgsl:11174-11177) | HOLDS | constant + clamp + loop bound verified |
| 28 | SEAM[spawn_engine:P11]: run_spawn_preamble "One implementation, ten callers" (spawn_engine.inl:61-63) | HOLDS | 1 template definition + exactly 10 call sites (grep) |
| 29 | "Three per family, eight families: 24 entries total" (entity_pipeline.inl:2271-2273) | DRIFTED | 27 wrappers = 9 families × 3 (executed); the file's opening banner says "seven" while listing eight names |
| 30 | DONE[floaters:L4]: CUBE_BEHAVIOR_COUNT twins named and equal (cube_behaviors.inl:85-89) | HOLDS | 3 == 3u; both sites name each other |
| 31 | DIAGNOSTICS banner: below-the-line is diagnostics-only (cube_behaviors.inl:273-278) | HOLDS | spawn path consumes only above-banner fns; below-banner reached only from F4-F7 + corral tick (caveat: literal commenting-out breaks those two callers) |
| 32 | DONE[entities:K1]: TierParams/TIERS gone; per-family TierRow in pipeline (entities.inl:69-74) | HOLDS | 8 TierRow structs found; only the K1 comment retains the token |
| 33 | floater_vocabulary "Depends on: … entities.inl (MOOD_COUNT)" (:43) | DRIFTED | MOOD_COUNT defined at cartridge.hpp:362; entities.inl only consumes |
| 34 | entities.inl:669-671: preparers "currently live in spawn_engine.inl … will be hoisted" | DRIFTED | they live 40 lines below (712-758); migration #10 landed |
| 35 | cube_behaviors F-key table F4-F7 (cube_behaviors.inl:52-55) | HOLDS | input.inl:246-249 binds exactly those, matching shapes |
| 36 | ribbon.inl public-surface box "called from outside this file" (18-53) | DRIFTED | ribbon_head_frame (called at cartridge.hpp:3577) missing; RIBBON_MAX_LENGTH has zero external consumers; head_pen/head_is are in-file only |
| 37 | ribbon.inl dependency list (56-62) | DRIFTED | names pawnReadback_* (migrated), two never-called helpers; omits canvas bindings, input, player sky fields, terrain services, select_tier_biased, four upload wires (executed c-> census) |
| 38 | seed_utils FXC-mirror claim (24-28) | HOLDS | constant-for-constant vs WGSL (one-sided naming filed under LAW-11) |
| 39 | RibbonState "All ribbon-owned state lives in this struct" (ribbon.inl:503-506) | DRIFTED | four canvas bindings on Cartridge + player_.sky_yaw_eased written only by the conductor |
| 40 | orbs.inl:39-40 "orbs_state_ fully encapsulated" | HOLDS | all 12 external references pass the struct whole (grep) |
| 41 | gol_zones/gallery header boxes: external surface is "reads" (gol_zones.inl:33-35; gallery.inl:46-49) | DRIFTED | spine WRITES zones[].active / gallery_centers[].active on rollback + eviction + teardown; mood writes mood_allowed |
| 42 | NOTE[gallery:shadows-missing] (gallery.inl:69-73) | HOLDS | shadow pass draws 12 families, no paintings; main-pass draws where claimed |
| 43 | gallery.inl:23 "take GalleryState& explicitly (const when read-only)" | DRIFTED (minor) | pick_authored_staging is read-only but takes non-const; three siblings honor the rule |
| 44 | agents.inl:185-188 eviction-radius mirror | HOLDS | 360 == 360; named both directions |
| 45 | input.inl:41-45 A-Z claimed by piano layer | HOLDS | boot banner + keyboard_midi.hpp; zero letter keys in dispatch |
| 46 | mood.inl:521-523 fog retired from apply_mood; canvas flushes per frame | HOLDS | no fog write in apply_mood_lighting; flush at cartridge.hpp:3204-3206 |
| 47 | renderer.hpp:574 "ceil(225/64) = 4" [NO-BASELINE] | HOLDS | constants chain verified; the enforcement gap filed separately (L3 #107) |
| 48 | renderer.hpp:179 meshGenEntityLayout_ "binding 1 only — reused by fade overlay" [NO-BASELINE] | HOLDS | state.hpp:3910-3921 + 1670 back-naming; fade layout built from it (2858) |
| 49 | renderer.hpp:53 "Entry points must match world.wgsl §7" [NO-BASELINE] | DRIFTED (address) | 64/64 names match (comm executed) but ~two-thirds live in §6/§8/§9 per the file's own section map — the section claim is stale |
| 50 | constitution: "CC audits every claim … §5 counts" (cartridge_constitution.md:3-4) | DRIFTED (one count) | 9 of 10 §5 classes verified EXACT against the_board (DIAG 6, DRIVERLESS 11+1, DONE 22+11=33, TESTING 1, COMPAT 2 files, NAMED TODO 3, BOOT-NEUTRAL 1, sweeps 2, whitelist 1 — all executed); EVICTION THUNKS "13 functions" is 12 in the_board (13 only in the sibling; independently re-executed) |
| 51 | datasheet standing rule: same-commit updates (datasheet:8-9) | HOLDS | git log: both surface-changing commits shipped datasheet together; the one datasheet-less ribbon commit was a constant hoist |
| 52 | contract + transcript: "STATIC HALF — COMPLETE" (transcript:12-47) | HOLDS | independent re-verification of R1-R16/H1-H5 static items all check (anatomy order, hoist, byte-identity+BOM, offsetof wires, twins, adjacency sweep 0 violations) |
| 53 | transcript: "PERFORMED HALF — OPEN" (transcript:49-66) | HOLDS | all six checkboxes unchecked at HEAD |
| 54 | constitution §7: docs lead only under PENDING[stage] (constitution:145-146) | HOLDS | grep: only the rule's own statements; no doc currently leads |
| 55 | datasheet wiggle-test standard (datasheet:246-249) | HOLDS | every §0 row traceable to visual_canvas.hpp/ribbon.inl without further reading; the "future panel" does not yet assemble (consistently so named) |

---

*End of report. 21 files / 38,569 lines read end-to-end; 13 assigned readings;
all counts executed with commands recorded beside them; zero writes outside
this file.*
