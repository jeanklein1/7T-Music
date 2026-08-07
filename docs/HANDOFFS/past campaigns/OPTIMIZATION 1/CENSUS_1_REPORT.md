# CENSUS_1 — THE CLAIM CENSUS

**The tree's account of itself, audited against the tree.** Reading only; no
code was touched by this census. Every claim carries file + line + verbatim
quote; inferences are marked `[INFERRED]`. Method: one reader, one refuter —
the primary reading below, then one adversarial pass instructed to refute it;
corrections are shown in place.

Scope, as ruled: `src/cartridges/the_board/**`, `src/incubator_dual.cpp`,
`src/docs/**`, `audit/**`, `src/docs/old docs/**` (classified separately),
**plus amendment 1 `src/console/**`** and **amendment 2 `src/render/**`**.
OUT: `src/tools/*.jsx` (the standing exclusion).

Classification: **A** prose only · **B** dead code · **C** true but drifted ·
**D** sound (counted, not listed). Default disposition is DELETE; KEEP and
CORRECT carry reasons; for Class B the reason must name a dated intent with an
owner — "latent" is not a reason.

This census ran AFTER SWEEP_1 (U1–U5), so seed-4's seven homes were already
adjudicated and swept; findings below state what remains true of the
post-sweep tree at its head.

---

## [0] FINDINGS — RANKED BY THE WRONG RULING THE CLAIM WOULD PRODUCE

### R1 — THE LAW WITH TWO GHOST EXEMPLARS (Class C, charter stratum — top)

`LAWS.md` L2 (THE FXC LAW) items 1 and 2, verbatim:

```
1. Instance structs in hot loops stay lean and byte-pinned — `GPUPierInstance`
   is 48 B with a `static_assert` in `state.hpp` (successor of the retired
   32-byte `SolidInstance` rule).
2. The collision/ground chain admits **no new runtime branching**.
   `evaluate_pier`'s caller bounds its loop by a uniform (`config.pier_count`)
   and dispatch is by uniform function choice, never by branch.
```

**Both worked examples are ghosts.** `GPUPierInstance`: zero hits **in code**
— [REFUTER CORRECTION: not "tree-wide"; archival copies survive at
`old docs/entity_contract_v0.md:170`, `old docs/terrain_program_charter.md:96`,
and `audit/tools/glaw2/baseline.json:592`]. `evaluate_pier`: zero hits in
code. `config.pier_count`: retired in BATCH G — its slot survives only as
`uint32_t _pad_pier_retired;` (`state.hpp:433`, WGSL twin comment
`world.wgsl:1554`). The law's *principles* still bind (the collision
chain and byte-pinned instance structs are real and live); its *evidence* is
fiction. **The wrong ruling it invites:** a reader who greps either exemplar,
finds nothing twice, and discounts the FXC law wholesale — on the one law that
fails only on other people's machines, at pipeline creation. This is the same
disease the L6 example had (g0:22, re-aimed in CENSUS_2b), one stratum up.
**DISPOSITION: CORRECT — re-exemplify both items with live anchors** (item 1:
a live byte-pinned struct with its static_assert, e.g. the GPUSpotLightArray
pin at `state.hpp:1388-1389` [REFUTER CORRECTION: was ":1387", the struct's
closing brace] or a hot-loop instance struct; item 2: the live
uniform-bounded loop pattern — `pyramid_instances.count`, live at
`world.wgsl:2732` `min(pyramid_instances.count, MAX_PYRAMID_INSTANCES)`).
Charter text: Jean's stamp, CENSUS_2-style commit.

### R2 — THE "LATENT" EXEMPTION WAS A STRUCTURE, AND IT ALREADY MISLED A CAMPAIGN (Class B + the ruling that shielded it)

Seed 1, extended — see [seeds] below for the full record. The wrong ruling is
not hypothetical: **LEDGER_1's first draft took the sun ortho from the dead
`compute_sun_matrices` (350) instead of the live GPU-side
`coupling_pawn_to_sun_vp` (300).** The archival seam map's L1 ruling blessed
the function as "latent, not dead," prescribed an intent comment that was
NEVER added, and installed a default — *"When in doubt … Default to
'latent'"* — that is an exemption STRUCTURE, not a fact. Under the seam map's
own taxonomy the function is **Dead** (its feature arrived and settled on the
GPU; this is "written for a feature that came and went"), not Latent.
**DISPOSITION: DELETE `compute_sun_matrices` (declaration
`render_passes.hpp:50-51`, definition `:517-588` [REFUTER CORRECTION: the
first draft said ":517-553" — line 553 is mid-body; the wrong-350 line sits
at **:567**, which the wrong range would have LEFT IN THE TREE while breaking
compilation — the exact class of error this census exists to catch, caught by
its own refuter]), and retire the default-to-latent rule as precedent** — a
future sun-expressivity feature starts from the live GPU coupling, not from a
stale CPU twin. The dead function's body is the only carrier of the wrong 350
**in live code** [REFUTER CORRECTION: the archival seam map carries it twice,
`:2630` and `:4595` — consistent with seed 5, which this draft contradicted].
Note also `:520`: the body pairs the *live* altitude (300) with the *dead*
extent (350) — half-right, which is how it fooled three readings.

### R3 — A DEAD SECOND PAINTING ARCHITECTURE, AND THE A2 MEMORY LEDGER (Class B pair)

`src/render/painting_system.hpp` (class `PaintingTextureManager`,
`CANVAS_SIZE = 2048`) and `src/render/painting_types.hpp`
(`PAINTING_CANVAS_SIZE = 2048`): **zero includers, zero instantiations**
(they do not even include each other). [REFUTER CORRECTION: the first draft
added "and no STB_IMAGE_IMPLEMENTATION fulfiller anywhere" — false;
`src/external/stb_image.cpp` is exactly that fulfiller and is in the build
(`CMakeLists.txt:506, :740`). The pair is dead **for want of includers
only**; its stb dependency contract is satisfied.] The 16 MB/layer canvas
manager never runs; as a claim it asserts a second painting architecture
that does not exist, in a directory that had sat outside every census scope
until amendment 2. **DISPOSITION: DELETE the pair** (whole files), pending
Jean.

**THE A2 LEDGER — the LIVE painting system's resident memory**
(`state.hpp:2185-2252`, `initOffscreenResources`; all at
`PAINTING_RESOLUTION = 1024`, swapchain color format, 4 B/texel, mip 1):

| Texture | Layers | Bytes |
|---|---|---|
| Snapshot Staging | 16 | 64 MiB |
| Authored Staging | 16 | 64 MiB |
| Exhibition | 32 | 128 MiB |
| Offscreen Snapshot Color | 1 | 4 MiB |
| Offscreen Snapshot Depth (Depth24Plus) | 1 | 4 MiB |
| **Total resident** | | **264 MiB** |

For ECONOMY_1's ranking: this is **larger than the patch heightfield array
(151.5 MB, E7)** and dwarfs the −96 MB the C2 merge just recovered. How many
of the 64 staging + exhibition layers ever hold a distinct image at once is
the follow-up audit the campaign should ask for.

### R4 — THE CONTRACT THAT ASSERTS THE RETIRED FORK (Class A, contracts stratum)

Seed 2, re-verified at head: `contracts/ground_architecture.hpp:161-162`
still reads *"gradients realized there via texture .yz + the analytic wave
gradient"*. `patch_terrain_vs` has no analytic gradient
(`world.wgsl`: `out.gradients = height_data.yz + live.yz` — two texture
channels). The written form of the exact belief LEDGER_1 was built to test,
in the stratum a future auditor trusts first. **DISPOSITION: CORRECT the
sentence** (drop "the analytic wave gradient"; the adjacent
shadow-subset description is accurate and stays).

### R5 — THE COUNT COMMENT THAT DIDN'T GROW WITH ITS ARRAY (Class C, new find)

`state.hpp:5459`: `// Live card writer bind group (5 entries: 0, 1, 160,
161, 31)` — the block declares `std::array<wgpu::BindGroupEntry, 6>` and
binds six: signal(0), config(1), zone_config(160), zone_life(161),
live_card_write(31), **live_card_scratch(32)** — the sixth appended by
TRUEBAND_CONTACT_1's two-pass writer without the comment moving. Every
sibling bind-group count comment was verified against its array — **16 of 17
count-bearing blocks exact; this is the only mismatch** [REFUTER CORRECTION:
the first draft said "15 blocks"; the refuter's recount found 17, including
the layout-side twin at `:4329`]. The wrong ruling: any binding-budget audit
that counts by header comments (the tree's own registry recount culture).
**DISPOSITION: CORRECT** to "(6 entries: 0, 1, 160, 161, 31, 32)" —
`bind::g0::live_card_scratch = 32` (`binding_registry.hpp:47`) [REFUTER
CORRECTION: the first draft guessed 30].

### R6 — THE 512 LABELS STAND UNFIXED (Class C, seed 3 carried forward)

Still live at head: `state.hpp:1754` (comment), `:3715` (comment), `:3719`
(**the `desc.label` itself**: `"Live Card (512x512, RGBA16Float —
GROUND_CARD_1)"`) against `LIVE_CARD_SIZE = 640`; plus `world.wgsl`'s
"the bake's pass-2 clone at res 512" comment. CENSUS_2 executed rulings on
seeds 7–10 only; **nothing has yet corrected seed 3.** **DISPOSITION:
CORRECT wholesale** in the next Class-A prose commit — the label string is
the one a debugger shows.

### R7 — VERDICTS ALREADY DELIVERED BY THE CAMPAIGN (closed here for the record)

- **Seed 4** (the one-indirect claim): adjudicated **false today** by Boot 1
  (PERMISSIVE, Dawn f0bf8ab5…, D3D12) and swept in U2 — zero live homes
  remain. Whether it was *ever* true is **unsettleable in-tree**: the Dawn
  checkout is unpinned (`CMakeLists.txt:13`) and no measurement before the
  U4 revision line carries its platform.
- **Seed 7–10**: dispositions executed by CENSUS_2a/2b; the record stands in
  the section preserved below.

### R8 — SMALLER TRUE-BUT-DRIFTED, LISTED (Class C, low harm)

- `cartridge.hpp:1722` — `static_assert(FrustumCull < ShadowPass, "O-7:
  frustum cull before the shadow pass")`: the shadow pass consumes no cull
  output (the shadow VS indexes `patch_instances` directly), so the message
  implies a data dependency that does not exist; its sibling `:1723`
  (< MainPass, "indirect draws consume the cull") is exactly true. KEEP the
  ordering pin, CORRECT the message when next touched — it becomes
  load-bearing only if a shadow-side indirect path lands (ECONOMY_1 E3b).
- `renderer.hpp` pipeline comment numbering: `1c` appears twice
  (`:1305`, `:1316`) and CENSUS_2b's deletion left a hole at 10 — the
  sequence reads 1b, 1c, 1c, 1d, 1e, 2, 11a… **The numbering was already
  broken before the hole.** DISPOSITION: de-number ("Pipeline:
  update_camera") rather than renumber, next touch.
- `state.hpp:1754`/`:3715` are counted under R6, not double-counted here.

### R9–R13 — FOUND BY THE REFUTER, NOT THE READER

The census method exists because one reading converges on its own blind
spots; these five entered the report only through the adversarial pass, and
three of them overturn "clean" verdicts the primary reading had issued.

**R9 (Class C, bodies — overturns "clean on detectors" for `bodies/*`).**
`gol_zones.hpp:611` — `// Upload all 7 slots` — is false: there are **5**.
`upload_zone_life` (`state.hpp:2926-2944`) writes slots 0–4; the stride is
`GOL_ZONE_LIFE_STRIDE = GOL_ZONE_CELLS * 5; // 5 slots` (`state.hpp:296`);
`world.wgsl:8298` says "(5 slots × 1024 cells)". "7" matches nothing at
HEAD. An in-class miss — an "N slots" comment inside a swept directory —
so the detector edge is weaker than declared. DISPOSITION: CORRECT.

**R10 (Class A ×3, `incubator_dual.cpp` — overturns its "clean" full read).**
(a) `:187` prints `"Controls: Arrows=move, Mouse=camera, A-Z=piano keys"` —
arrow keys are bound **nowhere** (movement is W/S/A/D, `input.hpp:222-225`),
and "A-Z=piano keys" is contradicted by the same file: `is_music_key` returns
false unconditionally (`:132-135`) and `:126-128` says every key falls to the
world. A runtime banner asserting behavior the code directly contradicts —
the census's exact detection target, in the smallest file in scope.
(b) `:6-9` names CMake variables `ACTIVE_RENDER_CARTRIDGE` /
`ACTIVE_ANALYSIS_CARTRIDGE` that do not exist — the real ones are
`INCUBATOR_DUAL_RENDER_CARTRIDGE` / `INCUBATOR_DUAL_ANALYSIS_CARTRIDGE` /
`INCUBATOR_ANALYSIS_CARTRIDGE` (`CMakeLists.txt:253ff, :523-524`).
DISPOSITION: CORRECT both.

**R11 (Class C, contracts — overturns `point.hpp`'s "clean").**
`contracts/point.hpp:104`: "world units; MUST match world.wgsl
POINT_BUBBLE_RADIUS" — **no such WGSL constant exists**; the live mechanism
is the `config.point_bubble_radius` field (`world.wgsl:1638`) boot-pinned
from the contract (`state.hpp:553, :5740`) — nothing in WGSL to "match". A
ghost mirror-twin: R1's disease one stratum down. Weaker sibling:
`POINT_HOST_TERRAIN_RULE` (`:82-90`) has zero consumers outside the file.
DISPOSITION: CORRECT the comment to name the config field.

**R12 (Class C, `state.hpp:509` — R5's disease, in R5's own file, missed by
the same sweep).** `// 0 = pawn (the kite), 1 = camera (free-fly)` — the
enumeration omits **2 = ribbon**, live at `world.wgsl:2552`
(`config.point_host == 2u`), `point.hpp:61` (`RIBBON = 2`), `input.hpp:267`.
DISPOSITION: CORRECT.

**R13 (Class C, charter stratum — missed by the same full read that produced
R1).** LAWS.md **L1 (`:28`) and L7 (`:148`) cite ghost paths**:
`audit/WEB_PORT_LEDGER.md` and `audit/cc4_wgsl_static_usage.py` both moved
to `audit/past reports/…` in commit b96074f. A reader following L7's pointer
to "the closure tool" gets a dead path. Also for the record: L6's
"render = compute + 200" band carries one self-documented exception —
`g0::render_vp == g0::vp_data + 199` (`binding_registry.hpp:176`) — a
wrinkle the primary reading's "L6 verified live" flattened.
DISPOSITION: CORRECT the two paths (and any sibling doc-path drift from the
b96074f move) in the next Class-A commit.

---

## [REFUTATION] THE SECOND PASS, FOR THE RECORD

One adversarial pass, instructed to refute, ran against the primary reading
at head `7cd47d3`. Verdicts: **R4, R6, R7, R8, seeds 5–6 and the A2
arithmetic survived exactly as written. R1, R2, R3, R5 survived in substance
but each carried at least one falsifiable defect** — every one is now marked
`[REFUTER CORRECTION]` in place above. The one that mattered: R2's deletion
range would have left the wrong-350 line in the tree while breaking the
build. Three corpus verdicts (incubator_dual.cpp, point.hpp,
bodies-on-detectors) and the LAWS.md row were overturned and now read
accordingly; R9–R13 are the refuter's findings. The method held: the shape
that caught `compute_sun_matrices` in LEDGER_1 caught this census's own
errors here.

---

## [CORPUS] FILE BY FILE

Counting boundary, stated: a CLAIM here is a comment line, label string,
banner, or doc sentence asserting present behavior, a dimension/count, or a
constraint — detected by (a) full read for files ≤ ~350 lines, (b) for the
four realization giants, detector sweeps (all `desc.label` strings; all
"N entries"/dimension/count comments; all "every frame / once / never /
only" behavioral assertions; all banner blocks) plus full verification of
every campaign-adjacent band this optimization arc touched. Detector-swept
is NOT a line-by-line read; the edge is declared per file. "clean" =
every detected claim verified true today.

| File | Lines | Method | Verdict |
|---|---|---|---|
| contracts/demo_config.hpp | 35 | full read | clean |
| contracts/entity_types.hpp | 375 | detector + targeted | clean on detectors |
| contracts/floaters.hpp | 140 | full read | clean |
| contracts/ground_architecture.hpp | 218 | full read | **R4** |
| contracts/indoor_module.hpp | 109 | full read | clean |
| contracts/mood_constants.hpp | 34 | full read | clean |
| contracts/point.hpp | 131 | full read | **R11** (ghost mirror-twin — found by the refuter) |
| contracts/roster.hpp | 197 | detector + targeted | clean on detectors |
| contracts/spawn_services.hpp | 303 | detector + targeted | clean on detectors |
| contracts/spine_state.hpp | 240 | detector + targeted | clean on detectors |
| contracts/surface_services.hpp | 235 | full read (census history) | clean |
| contracts/wgpu_fwd.hpp | 16 | full read | clean |
| realization/state.hpp | ~5,800 | detectors + campaign bands | **R5, R6, R12**; 16/17 bind-group counts exact; 4 dimension labels: 3 true, 1 = R6 |
| realization/world.wgsl | ~12,300 | detectors + campaign bands | **R6** (res-512 comment); banner verified post-U2 |
| realization/renderer.hpp | ~2,300 | detectors + campaign bands | **R8** (numbering) |
| realization/render_passes.hpp | ~620 | full read (campaign) | clean post-U2; R2's dead function lives here |
| realization/binding_registry.hpp | ~200 | full read (campaign) | clean post-CENSUS_2b (95/92 verified live) |
| realization/drawable_table.hpp | 125 | detector | clean on detectors |
| machine/entity_pipeline.hpp | 1,106 | detector | clean on detectors |
| machine/spawn_engine.hpp | ~1,050 | detector | clean on detectors |
| surface/patch_system.hpp | ~840 | detector + campaign bands | clean |
| surface/population_themes.hpp | ~470 | detector | clean on detectors |
| surface/terrain_looks.hpp | 152 | full read (campaign) | clean (REST pin verified) |
| surface/tile_world.hpp | ~580 | detector | clean on detectors |
| direction/input.hpp, mood.hpp | ~1,700 | detector + campaign bands | clean on detectors |
| bodies/* (10 files) | ~9,000 | detector | **R9** (gol_zones 7-vs-5 — an in-class detector miss, found by the refuter); gallery.hpp snapshot-draw band fully read (campaign) |
| cartridge.hpp | 2,066 | detector + campaign bands | **R8** (O-7 message); boot-timing chain verified post-CENSUS_2b |
| incubator_dual.cpp | 251 | full read | **R10** — the primary read passed it; the refuter overturned it three times over |
| console/console.hpp (amendment 1) | ~560 | full read (campaign) | clean at head — every claim in it was placed or verified by PROBE_1/SWEEP_1 |
| render/painting_system.hpp (amendment 2) | ~300 | full read | **R3** (whole file is Class B) |
| render/painting_types.hpp (amendment 2) | ~230 | full read | **R3** (Class B pair) |
| render/render_cartridge.hpp (amendment 2) | ~200 | full read | interface-only; clean |
| docs/LAWS.md | ~250 | full read | **R1, R13**; L3/L4/L6 anchors verified live (L6 carries one self-documented +199 wrinkle) |
| docs/7t_program_theory_v3.md | ~250 | full read | lens-stratum by its own declaration; no behavioral claims found presented as code-fact |
| docs/HANDOFFS/** | — | classification only | campaign documents, dated by construction; not claims |
| docs/old docs/** | — | targeted (seeds 1, 5, 6) | archival AND filed as such; **reachable** — see seeds |
| audit/LEDGER_1_REPORT.md | 1,075 | authorship + targeted | dated audit; quotes-as-evidence, not restatements |
| audit/past reports/** | — | classification only | archival, filed as such |

**The declared edge:** the four realization giants and the ten bodies files
were not line-by-line read; their verdicts are detector-scoped. A future
CENSUS_1b owns the residue if Jean wants the exhaustive walk.

---

## [SEEDS 1–6] VERIFIED AND EXTENDED

**SEED 1 — `compute_sun_matrices`.** CONFIRMED Class B, elevated to R2.
Callers at head: declaration `render_passes.hpp:50` and definition `:517`
only. The seam map's OWN record shows the exemption at work three times:
`:2692` ("currently unreferenced — but latent, not dead"), `:2774` (D1:
deletion struck through, "reframed as latent," action reduced to an intent
comment), and the Ch.10 note (":52-:90") that installs *"When in doubt …
Default to 'latent'"*. The prescribed intent comment **does not exist** in
render_passes.hpp. The verdict the census owes: **the exemption was a
STRUCTURE** — a default that converts "unverified" into "keep" without a
dated owner — and its cost is not hypothetical: it produced LEDGER_1's wrong
sun-extent, caught only by the adversarial pass.

**SEED 2 — the analytic-wave-gradient sentence.** CONFIRMED Class A at head;
ranked R4.

**SEED 3 — the 512 sites.** CONFIRMED still live (3 code sites + 1 wgsl
comment); ranked R6. The archival `ground_card_campaign_v2.md:38` copy is
counted under seed 6.

**SEED 4 — the one-indirect claim.** Adjudicated and swept; closed as R7.
The census's assigned question — "settle whether any Dawn version this tree
ever ran had the limitation" — is formally **unsettleable in-tree**: no Dawn
pin existed before U4's revision line. The claim's provenance is comments
only.

**SEED 5 — the seam map vs today's render_passes.hpp.** Three claims tested,
three drifted (all archival-filed): the "single 4096×4096 pass" is 2048² at
head (C2, U1); the "2048×4096 left/right tiles" are 1024×2048; the
"10 distinct shadow draws" list predates the drawable table —
`draw_shadow_all` today is the terrain fork + `draw_table(DRAW_SHADOW)`
(+ its own two-draw terrain split). **No live text restates any of the
three.** Archival stays archival; the file is reachable by grep, which is
precisely how it corroborated LEDGER_1's wrong 350 — the R2 disposition
(delete the dead function) removes the live half of that trap.

**SEED 6 — `ground_card_campaign_v2.md:38`.** CONFIRMED: "512² RGBA16F over
800 wu" vs the live 640² over 1000 wu — two stale numbers in one archival
line, filed under `old docs/`. Archival stays; noted here so the grep trail
ends at this report.

---

# PART II — SEED ADDENDUM DISPOSITIONS (seeds 7–10)

*(The section below is the addendum report as originally delivered —
preserved verbatim; its parent-absence note is superseded by the parent
handoff having since landed in-tree, and its dispositions have since been
EXECUTED by CENSUS_2a/2b.)*

**PARENT NOTE (historical).** The addendum (`src/docs/HANDOFFS/OPTIMIZATION 1/
cc_handoff_census_1_addendum.txt`) says "append to [5]". At the time of the
addendum run the parent CENSUS_1 handoff was not yet in this tree; it has
since landed beside it. Classification scheme used, reconstructed from the
addendum's own usage: **CLASS A** = prose asserting something the code does not
do; **CLASS B** = dead code (buffer/kernel/pipeline with no live consumer);
**CLASS C** = one fact with two+ homes kept true only by manual lockstep.

---

## SEED 7 — `SHADOW_MAP_SIZE`: A DIM FACT WITH TWO HOMES AND NO LOCKSTEP MECHANISM

**VERIFIED.** The two homes:

```
state.hpp:207    constexpr uint32_t SHADOW_MAP_SIZE = 4096;        (under // Lighting)
world.wgsl:3567  const SHADOW_MAP_SIZE: f32 = 4096.0;              (under // --- Shadow constants)
```

Both PCF kernels derive `texel_size` from the WGSL twin: sun
`sample_shadow_pcf` at `world.wgsl:3598`, spot `sample_spot_shadow_pcf` at
`world.wgsl:3715` — both `let texel_size = 1.0 / SHADOW_MAP_SIZE;`. So the
addendum's warning is exact: changing `Dim::` alone resizes the texture and the
atlas tiles while every PCF world-footprint silently shrinks — a *different*
shadow, not a cheaper one.

**Full census of `\bSHADOW_MAP_SIZE\b`, scope `src/**` minus `src/tools`
(the standing exclusion): 8 sites, 3 files.**

| Site | Quote | Class |
|---|---|---|
| `state.hpp:207` | declaration | HOME 1 |
| `world.wgsl:3567` | declaration | HOME 2 |
| `render_passes.hpp:261` | `TILE_W = Dim::SHADOW_MAP_SIZE / 2;  // 2048` | derives |
| `render_passes.hpp:262` | `TILE_H = Dim::SHADOW_MAP_SIZE;      // 4096` | derives |
| `state.hpp:3800` | sun depth texture `desc.size = { Dim::SHADOW_MAP_SIZE, ... }` | derives |
| `state.hpp:3812` | spot depth texture `desc.size = { Dim::SHADOW_MAP_SIZE, ... }` | derives |
| `world.wgsl:3598` | sun PCF `1.0 / SHADOW_MAP_SIZE` | derives (from HOME 2) |
| `world.wgsl:3715` | spot PCF `1.0 / SHADOW_MAP_SIZE` | derives (from HOME 2) |

No third independent home. No independent functional 4096/2048 literal serving a
shadow purpose (LEDGER_1 F-9 re-confirmed). Three **prose** restatements exist:
the trailing `// 2048` / `// 4096` at `render_passes.hpp:261-262` and the atlas
comment `world.wgsl:3657` ("Two 4096×4096 depth textures, each split left/right
into 2048×4096 tiles") — these go stale under any resize.

**THE AGGRAVATION, verified:** nothing keeps the two homes in lockstep — no
`static_assert`, no codegen, and **not even a mirror note**. The tree has a
named convention for exactly this situation and it is absent here:
`TILE_GRID_CAPACITY` (`state.hpp:137-141`) carries *"twin: world.wgsl
TILE_GRID_CAPACITY ... Raise it in BOTH rooms or glaw1/Dawn objects"*, and
`PATCH_CELL_SIZE` / `LIVE_CARD_SIZE` carry explicit *"L3 MIRROR"* notes
(`state.hpp:79, 93-94`). `SHADOW_MAP_SIZE` alone among the mirrored Dim facts is
unmarked on both sides. `[INFERRED]` note: a compile-time assert cannot span
this seam anyway — world.wgsl is runtime-loaded source — so the available
mechanisms are the mirror-note pattern or piping a pipeline-override constant
from `Dim::`.

**CLASS: C** (in spirit even while the numbers agree, per the addendum — and
the census adds: C *without even the mirror note*, the weakest form in the tree).

**DISPOSITION (paper):** PROBE_1 rev2 C2 edits both rooms in lockstep — done on
`claude/probe1-c2`, both homes, one commit. The **permanent** home is a
landing-time ruling for Jean, two arms: (a) pipeline-override constant piped
from `Dim::SHADOW_MAP_SIZE` at pipeline creation — one home, mechanical
lockstep, at the cost of one more override plumb; (b) keep the twin pair and add
the `TILE_GRID_CAPACITY`-pattern mirror note to BOTH declarations — zero code
motion, lockstep stays manual but becomes *named*. Either way the three prose
restatements (`render_passes.hpp:261-262` trailing comments, `world.wgsl:3657`)
should be de-numbered or derived at the same landing.

---

## SEED 8 — "ONE INDIRECT DRAW PER PASS": SEVEN HOMES, ONE OF THEM FALSE, UNRECORDED PROVENANCE, ARCHITECTURE-BEARING

**VERIFIED — the home count: 7 design-side homes in 5 files.** A first sweep
with `grep "one indirect|One DrawIndexedIndirect|only one indirect"` found only
5 — the canonical home escaped because its backticks break the phrase
(`One \`DrawIndexedIndirect\` per`). This is precisely the corollary-boundary
trap the addendum warned about; the count below is from the widened sweep.

The canonical LAW home (1):
```
LAWS.md:43             4. One `DrawIndexedIndirect` per render pass, maximum.        (L2 item 4)
```

States the limit (3):
```
render_passes.hpp:395  // Terrain LOD1 — always direct (Dawn D3D12 limit: only one indirect per pass)
state.hpp:1838         // GPU frustum culling — LOD0 only (Dawn D3D12 limit: one indirect draw per pass).
state.hpp:3106         // GPU frustum culling — LOD0 only (Dawn D3D12 limitation: one indirect draw per pass).
```

The corollary — "budget line" bookkeeping that only makes sense under the
limit (2):
```
render_passes.hpp:342  // One DrawIndexedIndirect budget line free in this pass (L2.4).   (draw_shadow_all)
render_passes.hpp:414  // One DrawIndexedIndirect budget line free in this pass (L2.4).   (main pass draw_table)
```

The L2 pointer (1):
```
world.wgsl:17          //  READ L2 BEFORE adding ... a second indirect draw in one pass.
```

Plus echoes outside code: `audit/LEDGER_1_REPORT.md:855` (which itself says
"three code sites" — now known to be an undercount) and the archival
`src/docs/old docs/the_board_seam_map.md:2613`,
`audit/past reports/CC_AUDIT_REPORT.md:153`.

**ONE OF THE SEVEN IS LITERALLY FALSE TODAY.** The main-pass corollary at
`render_passes.hpp:414` ("One DrawIndexedIndirect budget line free in this
pass") contradicts `render_passes.hpp:374-382`: outdoors,
`use_indirect_terrain()` is true and the main pass **consumes** its one
indirect line on the LOD0 terrain draw — the budget line is not free there.
The shadow-pass twin at `:342` is unconditionally true (that pass issues zero
indirect draws). Treating `:414` as license to add an indirect draw to the
main pass would violate L2.4 outdoors. Do not edit one twin without ruling on
the other.

**A related flag, reported, not fixed:** `cartridge.hpp:1742` —
`static_assert(FrustumCull < ShadowPass, "O-7: frustum cull before the shadow
pass")` — orders the cull before a pass that consumes no cull output (the
shadow VS reads `patch_instances` directly). The ordering assert is broader
than the data dependency; it becomes load-bearing only if a shadow-side
indirect path ever lands.

**Load-bearing: confirmed at both named consequences.** (a) LOD1 draws its full
annulus direct-and-uncullable in the eye pass — `render_passes.hpp:395` is the
stated reason the LOD1 draw never went indirect. (b) The shadow pass culls
nothing and the `:342` comment treats its indirect budget as a scarce resource
(one line, "free"). The actual encoder-call census: **exactly one**
`DrawIndexedIndirect` in the tree (`renderer.hpp:736`,
`draw_patch_terrain_lod0_indirect`), zero `DrawIndirect`.

**What the census cannot settle in-tree, stated plainly:** whether ANY Dawn
version this tree ever ran actually had the limitation. The tree has **no Dawn
version pin** — `CMakeLists.txt:13` points at an unpinned local checkout
(`set(DAWN_DIR "C:/dev/dawn" ...)`) — and `git log -S "one indirect draw per
pass"` dates only the comments' arrival in this repo, not the Dawn they were
written against. The design-side reading (received, not verified here) is that
the WebGPU spec and dawn.json place `drawIndexedIndirect` as an ordinary
per-call encoder method, and that a one-per-CALL batching limit belongs to the
multi-draw FEATURE (value 50) — i.e. the claim as written matches no spec
construct. **PROBE_1 C1's R0 boot log is the instrument that settles the
adjacent live question** (whether the current Dawn exposes multi-draw); whether
the historical limitation ever existed would need the Dawn changelog, which is
outside this tree.

**CLASS: A-or-fossil prose in 7 homes (one already false), ranked at the top beside seed 2**
(per the addendum's own ranking instruction; seed 2 is parent-side, not
visible here). Wrong ruling it produced if false: the draw-submission
architecture itself — LOD1 uncullable, shadow uncled, one cull result serving
one draw (LEDGER_1 H7/F-2 quantify the cost).

**DISPOSITION (paper):** do not edit the comments — the claim must be
**adjudicated, not reworded**. R0 (C1's log) answers whether multi-draw exists
on the shipping Dawn; a five-line held probe issuing TWO plain
`DrawIndexedIndirect` calls in one pass would answer whether the base
limitation exists today. If both come back permissive, all five homes and the
budget-line bookkeeping retire together, and the LOD1/shadow indirect
architecture unlocks (LEDGER_1 L-3/L-7 become schedulable). If the limitation
is real on D3D12, the seven homes should collapse to the ONE law that already
exists (`LAWS.md:43`, L2.4) with pointers — six independent restatements of an
unverifiable constraint, one of them already false, is how fossils breed. The
`:414` falsity should be corrected at the same ruling regardless of which way
the adjudication goes.

---

## SEED 9 — `terrainIndexBuffer_`: DEAD BUFFER + LIVE KERNEL, CARRYING A FALSE "READ EVERY FRAME"

**VERIFIED — the claim:**

```
state.hpp:3132  // Terrain index buffer -- filled once by compute shader, read every frame
state.hpp:3133  terrainIndexBuffer_ = makeBuffer("Terrain IB",
state.hpp:3134      Dim::TERRAIN_INDEX_COUNT * 4,
state.hpp:3135      wgpu::BufferUsage::Storage | wgpu::BufferUsage::Index);
```

**VERIFIED — the reader set is EMPTY.** With word boundaries:
- `terrainIndexBuffer_` appears at exactly 5 sites: member (`state.hpp:1686`),
  accessor definition (`:2552`), creation (`:3133`, `:3136`), bind-group entry
  (`:5151`, the *writer's* bind group). That is: creation + write plumbing only.
- The accessor `terrain_index_buffer()` has **zero callers** (`grep -rn
  "terrain_index_buffer()"` → only its definition).
- **No `SetIndexBuffer` site in the tree binds it.** All 14 index-buffer binds
  were enumerated; every terrain draw uses `patch_index_buffer()` /
  `patch_index_buffer_lod1()` (`render_passes.hpp:328, 333, 380, 389, 401`;
  `gallery.hpp:1275` — the snapshot pass included).
- The `BufferUsage::Index` flag has therefore never been exercised; the buffer
  is written once by `generate_terrain_indices` (`world.wgsl:7863-7879`, via
  `terrain_mesh_indices` `@group(0) @binding(22)`, `world.wgsl:5457`) and never
  read by anything, on either the GPU or the CPU.

"Filled once by compute shader" — TRUE (the one-shot dispatch,
`cartridge.hpp:502-516`, `"Terrain Index Gen (one-shot)"`). "Read every frame"
— **FALSE.** The patch system replaced the consumer: the live index buffers are
`patchIndexBuffer_` / `patchIndexBufferLOD1_` (CPU-built, `state.hpp:3159-3278`).

**THE RIDE-ALONGS (the full dead cluster, if the buffer is ruled dead):**
`terrainIndexBuffer_` + accessor (`state.hpp:1686, 2552`); the
`generate_terrain_indices` kernel (`world.wgsl:7862-7879`) and its constants'
kernel-side uses (`TERRAIN_MESH_N`/`TERRAIN_MESH_STRIDE`, `world.wgsl:215-216` —
declaration shared, only this kernel consumes them); the storage binding
`@group(0) @binding(22)` (`world.wgsl:5457`); the pipeline (one of the boot FXC
compiles — the ~2.4 s figure is the addendum's, **unverifiable in this
container**: no build here); the Terrain Index Gen layout + bind group
(`state.hpp:4213, 5155`) and dispatch helper (`renderer.hpp` /
`state.hpp:2952` `terrain_mesh_workgroups()`); the one-shot boot dispatch block
(`cartridge.hpp:502-516`); `Dim::TERRAIN_INDEX_COUNT` / `TERRAIN_MESH_VERTS`
(`state.hpp:50-52`) — checked: no other consumer.

**CLASS: B** (buffer + kernel + pipeline + bind group + boot dispatch, no live
consumer) **carrying a CLASS A claim** ("read every frame"), exactly as the
addendum suspected.

**DISPOSITION (paper):** retirement candidate, whole-cluster — the recipe is
mechanical because every member is exclusive to the cluster (g0 binding 22
shares nothing; g1:22 is the bilinear sampler, a different group — not a
collision). Savings if retired: one boot pipeline compile (the FXC
ride-along), one storage buffer (`TERRAIN_INDEX_COUNT × 4` = 1,572,864 B), one
bind group + layout, one binding slot in group 0.

**Three retirement-recipe cautions, so the ruling sees them up front:**
1. **L6 registry governance**: `binding_registry.hpp:16` witnesses the count
   *"96 declarations over 93 slots"* in world.wgsl, and `LAWS.md:113` uses
   **g0:22 as its worked example**. Deleting the `@binding(22)` declaration
   stales both — the removal must update the registry count and the law's
   example together.
2. **Boot gate**: `state.hpp:3136` `if (!terrainIndexBuffer_) return false;`
   makes the dead buffer a boot-success gate inside `createMeshBuffers()`.
   Buffer removal must remove this check; draw-side-only removal changes
   nothing.
3. **Timing brackets**: `cartridge.hpp:498/:516` `t1`/`t2`
   high_resolution_clock stamps bracket the one-shot dispatch — trace their
   consumer before deleting the block (not traced by this census).

The census reports; the campaign decides — note this is the same
`generate_terrain_indices` LEDGER_1 F-1 examined when ruling out per-frame IB
rebuild mechanisms, so retiring it also removes a standing source of confusion
between the dead legacy grid IB and the live patch IBs.

---

## SEED 10 — THE SHADOW-SKIP SENTENCE: THE ENABLER EXISTS, THE SKIP WAS NEVER BUILT

**VERIFIED — the claim, verbatim:**

```
world.wgsl:3251  // Snap pawn XZ to shadow grid for temporal stability.
world.wgsl:3252  // Shadow map content is pixel-perfect between grid crossings,
world.wgsl:3253  // enabling the CPU to skip the shadow pass on idle frames.
```

with `SHADOW_SNAP_SIZE = 2.0` (`world.wgsl:3248`) and the snap itself at
`world.wgsl:3255-3256` (`round(pawn_pos / SNAP) * SNAP` on X and Z).

**VERIFIED — no CPU site exercises the skip.** `render_shadow_pass` has exactly
one caller: `phase_shadow_pass` (`cartridge.hpp:1573-1576`), and it is
**unconditional** — no movement gate, no dirty flag, no grid-crossing
detection. Searched: `grep -rni "shadow_dirty|skip.*shadow|shadow.*skip|
grid_cross"` over `src/cartridges/` and `src/console/` → **zero hits**. The
sun VP is likewise recomputed every frame (`compute_vp`, in the per-frame
compute block). The addendum's METER observation (ShadowPass paying full cost
through resting windows) is consistent with, and now explained by, the code:
the pass runs every frame by construction.

**Precision of the verdict:** the sentence is the (b) case — the *enabling
half* is real (the snap exists and does what it says; between grid crossings
the light VP is bit-stable, so map content would indeed be reusable), and the
*enabled half* (the CPU skip) **was never implemented**. The prose reads as a
description of a shipped capability; what shipped is the precondition. The
call sits in the RENDER_SPINE with a constexpr row gate of `true`
(`cartridge.hpp:1637`), and `render_shadow_pass`'s body has no idle/no-op path
— its only branch selects indoor-atlas vs outdoor.

**A correctness bound on the claim itself `[INFERRED]`:** "pixel-perfect
between grid crossings" holds only for **static** casters. Every dynamic
caster in the drawable table (pawn, spheres, cubes, ribbon, agents) moves the
map's content between crossings, so the honest capability was always
"skip when idle AND nothing shadowed moved" — the invalidation set in
disposition arm (b) is not optional garnish, it is the claim's missing half.

**CLASS: A** (a capability presented as enabled), **LEVER-BEARING**: this
sentence, grown up, is GEOMETRY_2's rate-aligned shadow map — cache
static-caster depth, redraw movers only. The quantified upside is LEDGER_1's
~10 ms shadow pass at rest.

**DISPOSITION (paper):** two arms for the campaign to rank, neither taken here.
(a) The honest-comment fix: reword `:3253` to "which would let the CPU skip the
shadow pass on idle frames (skip not yet implemented)" — CLASS A cured for one
comment's cost. (b) The lever: implement the skip — CPU-side, hash the inputs
that invalidate the map (snapped pawn cell, sun direction, any caster
moved/spawned/despawned, GoL tick, spot-light set) and elide
`render_shadow_pass` when unchanged. Arm (b) subsumes arm (a) and is the
cheapest of the GEOMETRY_2 family because it needs no shader work; its risk is
the invalidation set (a missed invalidator = a frozen shadow), which is why it
belongs to the campaign, not this census.

---

## SCOPE NOTE (relayed, not ruled)

The addendum's optional ruling — adding `src/console/**` to the census scope —
costs ~one file and is supported by this census's own evidence: the adapter
finding (PROBE_1) showed the host layer's prose carries the same disease. All
four seeds above fall inside the existing scope regardless. Jean rules.
