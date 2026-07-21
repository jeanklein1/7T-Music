# AFTER-READ REPORT — the swept tree, round one (contracts + machine + realization)

Scope: the twenty files delivered post-sweep — binding_registry, cartridge,
demo_config, drawable_table, entity_pipeline, entity_types,
floater_vocabulary, ground_architecture, mood_constants, point,
render_passes, renderer, roster, spawn_engine, spawn_services, spine_state,
state, surface_services, wgpu_fwd, world.wgsl. The bodies/direction wing is
round two; this report extends when it arrives (section H reserved).

Method: the three committed lenses — rule-pass output quality,
consolidation under the minimality clause, friction delta — plus the
landing verification the lenses ride on. Counts ship with recipes
(appendix G). One correction to the numbers spoken in chat: the first
census ran over a directory still holding PRE-SWEEP copies from earlier
rounds; every table below is re-run over exactly the fresh twenty. The
chat-round numbers are superseded.

---

## A — THE GATE (read this first): the tree is not the addendum's end state

Evidence, from the delivered files themselves:

| Addendum step | Expected | Found | Evidence |
|---|---|---|---|
| 03a T1 (TileGrid) | runtime-sized | **fixed 289** | world.wgsl:903 `entries: array<TileGridEntry, 289>` |
| 03a T2 (zone) | runtime-sized | converted | `ZONE_PATCH_CAP` = 0 hits; zone_patch_instances is `array<PatchInstance>` |
| 03a T3 (patch_grid) | runtime-sized | converted | no `array<u32, 225>` remains |
| 03b (the flip) | radius 8 | **radius 7** | state.hpp:65 `PATCH_PREGEN_RADIUS = 7` + "(15×15, 350 world units)" comment; veil note still "flagged, not started" |
| 04 (pulse port) | landed | landed | `select(0.0, 1.0, sin(phase) > 0.0)` present, 1 hit |

This combination is impossible under the addendum's specified order
(03a → 03b → 04, one commit each): 04 present with 03b absent and 03a
two-thirds present. Three hypotheses, no verdict without evidence:
the upload snapshots a mid-run state; a hunk dropped from 03a; an
unreported deviation.

**Required evidence (CC pastes):**

    git log --oneline -8
    git show --stat <each addendum commit>
    git show <03a's commit> -- realization/world.wgsl   # does the T1 hunk exist?

**THE GATE.** Commit 03b (the flip) must not run — and no radius-8 build
can be trusted — while TileGrid declares a fixed 289. This is precisely
the trap the R-e census named: at radius 8 the runtime side becomes 19,
indexing walks to 360, WGSL robustness clamps into the 289 declaration,
and the outer tile ring reads garbage defaults with zero errors anywhere
— Dawn validation passes, FXC passes, the world is silently wrong at the
rim. If 03b has in fact already run somewhere past this snapshot, the
gate applies retroactively: verify T1 before trusting that build.

---

## B — Lens 1: rule-pass output quality

Verdict first: the archaeology collapse **succeeded at the sentence
level**. Banners flow; the `History: audit/LADDER.md` line is uniform;
the truth-fixes landed (entity_types' MachineCtx sentence, the
WorldState fwd, the band_blend sentinel); the cables are verifiably
gone (§E). The residue is one systematic class and a short list of
named fossils.

### B1 — The orphaned-coordinate class (systematic)

Mechanism: commit 03's witness grepped **parent** campaign names only
(`LADDER-|DISSOLVE-1|REBUILD-0|PANEL-0|ROSTER-1|_RECON`). The parents
died; their **child coordinates** survived legally — and a child whose
parent is stripped is *less* resolvable than before. Witnesses are the
spec; the spec under-covered. Second occurrence of the lesson.

Clean census (fresh twenty only; raw line hits, classification pending):

| token class | hits | notes |
|---|---|---|
| p1a / p1b(-a..e) | 40 | point.hpp + spine_state — the point-model stage ids |
| C2–C8 | 25 | includes legitimate "C6" registry-name uses — see boundary below |
| husk sweep | 20 | binding_registry, entity_pipeline, state, world.wgsl |
| d1–d4 | 17 | expect code-identifier false positives; classify |
| K1–K4 | 11 | **KEEP class** — living seam/constitution ids; listed to mark the boundary |
| R1 / R5 | 9 | recon coordinates; R1 also collides with this session's ruling ids |
| residue T2 | 9 | binding_registry and kin |
| m1–m6 | 8 | cartridge 1, spine_state 4, surface_services 3 |
| p2 | 2 | |
| theory v2 | 2 | fossil class — B2.2 |
| V1 | 2 | veil ruling tag — treat as b1/b3 (bare ruling tag, KEEP) |
| Keyhole form | 1 | fossil — B2.1 |
| L2-a / M-j / Q4-5 | 1+1+1 | L2-a, M-j recon coords (strip); Q# = KEEP pattern ids |

Honest size after classification: **~60–110 comment sites** in this
half of the tree. The wing (round two) was m-heavy in its banners;
expect a comparable batch there.

Classifier boundary for the mop (extends the 03 tables):
- **DELETE**: bare m#, p1a/p1b(-x), p2, d#, C2/C5/C7/C8, husk-sweep
  parentheticals (keep the fact "REMOVED — no reader"), residue T2,
  R1/R5, L2-a, M-j, Phase-R children, "this arc"-style unanchored
  campaign language.
- **KEEP**: SEAM[…]/STATUS/LATENT/INTENT/TESTING tags whole; O-#, RC-#,
  E-#, F-#, K1–K4, P#, S1–S5, Q4/Q5/Q10; bare ruling tags b1/b2/b3 and
  V1; constitution § refs; "ratified"/"Jean's stamp" minus coordinates;
  **C6** — see B2.6.
- Safe default unchanged: unsure → keep, mark `[?]`, list.

Disposition: **one comment-only MOP pass** under the extended tables —
a rider (preamble commit) on the next handoff, not a SWEEP-3.

### B2 — Named fossils

1. **"Keyhole form."** — surface_services.hpp:189 (teardown_surface).
   Now *false* vocabulary: the door takes MachineCtx, the anti-keyhole.
   Fix: replace the label with the truth ("Root-called owner verb") or
   delete it. Round two answered: the twin PERSISTS at
   patch_system.hpp:190 — two sites tree-wide, both to the mop.
2. **"theory v2"** ×2 — entity_types.hpp:11 (§8), roster.hpp:29 (§5).
   v3 is ratified. `[?]`-class, not a blind edit: the §-numbers must be
   re-mapped against v3 (Jean or the seed doc supplies the mapping) or
   the citations become versionless ("the program theory").
3. **Doubled trailing comment** — state.hpp:12, the demo.hpp include
   carries two stacked comments ("// ROSTER via the selected sentence
   (…)  // feature bits (…)"). Collapse to one.
4. **SphereProp / CubeEntityProp asymmetry** — created by the 07
   rename. Finish it: `CubeEntityProp → CubeProp`, same numeric-pin
   argument (the static_assert already stands), token sweep in
   floater_vocabulary + the cube body. Free.
5. **POINT_BUBBLE_RADIUS** — the last hand-lockstep mirror outside
   two-rooms discipline ("MUST match world.wgsl"). Pre-existing, not
   sweep damage. Disposition: fold into the point's panel when that
   panel exists (panel-era register, §C).
6. **"C6" accepted as a name** — binding_registry kept its cut-tag as
   a title while drawable_table lost its own; the asymmetry is
   acceptable because C6 now *functions* as the registry's proper name
   (state.hpp and terrain_looks cite "the C6 registry"). Recorded as
   considered; KEEP.
7. **"(R1 — the collapse)"** — spawn_services' composition-law title
   carries a dangling recon coordinate that now also collides with this
   session's R# ruling vocabulary. Mop class; worth naming because of
   the collision.

### B3 — Rule-pass verdict

The two rule-based passes (03 archaeology, 12 underscores) held their
KEEP guards perfectly — zero law damage found: every SEAM/STATUS tag,
every table's WHAT/AXES/UNITS block, every F/O/E/P id intact where I
checked. The failure mode was purely *under-reach*, never over-reach.
That is the right failure direction, and the mop closes it.

---

## C — Lens 2: consolidation (the minimality clause over the contracts tier)

**Verdict: the tier is sound; zero merges warranted.** Keyhole was the
class's one true no-decision header and it is dead. Per header:

- **wgpu_fwd** — owns the lockstep-insurance decision. Earns.
- **mood_constants** — the mood identity leaf; demos/matrix and
  floater_vocabulary need the ids without the organ set.
  PortalDestination's residence here (a DTO among ids) is the shared
  leaf both mood.hpp and spine_state need — acceptable, recorded.
- **point** — the point model contract. Earns clearly.
- **roster** — piece identity + the gate law + F-1. Earns.
- **demo_config** — the sentence *type*; the type-vs-authoring split
  mirrors roster/demo. Earns.
- **floater_vocabulary** — shared floater vocabulary; the SEAM's
  three-concerns-three-files argument holds. Only the *file name* is
  debt (its own LATENT[naming]). Rename → `contracts/floaters.hpp`.
- **entity_types vs spawn_services** — types vs services; the B-split
  earns both. Merging would push MIN_SEPARATION and ARCH_TIERS into
  every body that wants only MachineCtx.
- **spine_state** — the spine's organ types; each occupant is
  spine-resident; coheres.
- **surface_services** — the surface contract; coheres.
- **ground_architecture** — the exemplar; the STATUS convention and
  DAG-closure assert remain the standard the tier converges to.

**One micro-commit** closes the tier's whole remaining self-declared
debt: `floaters.hpp` rename + `CubeProp` rename (B2.4).

**The structural yield — the panel tension.** Three *design tables*
live inside contracts: MIN_SEPARATION + ARCH_TIERS (spawn_services)
and MOOD_TABLE + MoodProfile's authored rows (spine_state). Their
placement is *correct today* — the second-consumer law put them where
their readers meet — but by the terrain_looks doctrine they are PANEL
content: designer surfaces embedded in decl tiers. Register the
destinations now, move nothing yet:

- **Placement panel** (panel era): MIN_SEPARATION, ARCH_TIERS, the
  PROXIMITY_* family — what stands where, one sitting.
- **Atmosphere panel** (panel era): MOOD_TABLE's authored columns —
  suns, fogs (with the INTENT[mood-fog-baseline] revive-or-delete
  decision), ceilings, clear/wall colors.
- **Point panel** (panel era): POINT_BUBBLE_RADIUS folds into
  two-rooms discipline; the hand-lockstep comment dies.

These three registrations feed the master-control-panel roadmap
directly: the panel era's scope is already forming as a named list.

---

## D — Lens 3: friction delta

Materially down. What specifically got cheaper, before → after:

- Keyhole indirection → gone; every dependency is a visible face.
- DEMO include-order cables → cut; spine_state and surface_services
  now read self-sufficiently, with the one *sanctioned* Dim cable
  named in place.
- Seven-spellings disease → one spelling; a dimension is `Dim::X`
  everywhere it is consumed.
- Provenance → one glance (the History line), instead of a paragraph
  per banner.
- glaw1 → defined at the root; all three mention sites (registry,
  panel, world.wgsl:1472 — the count was three, not the two my census
  first claimed) now resolve.

Where a cold reader still stumbles — exactly B1 and B2: every bare
"m4" is a question with no answer, and "Keyhole form" asserts the
opposite of the truth. The mop closes the gap between the sweep's
intent and its floor. Net: the sweep achieved its aim.

---

## E — Verified-landed ledger (do not re-touch)

Confirmed in this upload: ctor root seeds (cartridge:396–397);
glaw1 definition (cartridge:5–8); BINDING-MAP husk → one-line pointer
(state.hpp:9); band_blend sentinel comment (world.wgsl:1395);
overlay prop consts + `overlay_band_params` helper (3 sites — first
sweep 13/14); pulse port (addendum 04); T2/T3 runtime-sized;
loud drop + cap 16 + `<cstdio>` (surface_services); wgpu_fwd verbatim;
floater pins + Sphere renames; PATCH_CELL_SIZE survivor with the
one-spelling law line; entity_types banner truth + local fwd removal
(0 `namespace wgpu` blocks); zero `entities.hpp` ghosts; zero BOMs
anywhere including renderer.hpp; LF-only throughout; ground_architecture
untouched and exemplary.

---

## F — Dispositions and order of operations

1. **CC's git evidence** (§A commands) resolves the addendum state.
   The T1/flip gate holds until then. Nothing else proceeds first.
2. **The FINISHER** (authored against the verified tree, one small
   handoff): step 0 = state reconciliation from the git evidence;
   T1 conversion completing 03a; 03b the flip (with its R-d comment
   sweep — the veil note resolves); **the MOP** (B1 tables, `[?]`
   guard, theory-v2 held `[?]` unless the v3 §-mapping is supplied);
   the fossil fixes (B2.1, B2.3); the renames (floaters.hpp, CubeProp).
   Ordering and witnesses follow the house pattern: comment passes
   first, the sole code edits last, solo-revertible.
3. **Panel-era register** (§C) — recorded, not executed: placement
   panel, atmosphere panel, point panel.
4. **Round two** (the wing) extends this report at §H: same lenses,
   same census recipes, expecting the coordinate class to be m/d-heavy
   in the body banners, and the pre-sweep twins of B2.1's patch_system
   fossil to show their post-sweep state. Environment note for my own
   process: purge stale pre-sweep copies from the reading directory
   before the round-two census (the contamination caught in G).

---

## G — Appendix: census recipes

Fresh set (the exact twenty, named in Scope). Token census:

    for pat in '\bm[1-6]\b' 'p1[ab]' '\bp2\b' '\bd[1-4]\b' '\bC[2-8]\b' \
               'husk sweep' '\bR[15]\b' 'residue T2' '\bR-[abc]\b' \
               'theory v2' 'Keyhole form' 'L2-a' 'M-j' '\bK[1-4]\b' \
               '\bQ[45]\b' '\bV1\b'; do
      echo -n "$pat: "
      grep -rEc "$pat" $FRESH | awk -F: '{s+=$2} END {print s+0}'
    done

Gate checks: `grep -n "array<TileGridEntry" world.wgsl` (fixed 289 =
gate closed); `grep -n "PATCH_PREGEN_RADIUS" state.hpp` (7 = flip
absent); `grep -c "select(0.0, 1.0, sin(phase) > 0.0)" world.wgsl`
(1 = port present). Contamination guard: run every census against an
explicit file list, never a directory glob — the reading directory
accretes stale rounds.

---

## H — Round two: the wing (bodies + direction + demos + panels)

### H0 — Scope and freshness

Eighteen files, all timestamped as one delivery: agents, cube_behaviors,
demo, gallery, gol_zones, **grounded** (the rename, confirmed on disk),
input, matrix, mood, orbs, patch_system, pawn, population_themes,
ribbon, seed_utils, spheres, terrain_looks, tile_world. The reading
directory still holds stale `entities.hpp` and `keyhole.hpp` from the
pre-sweep rounds — excluded per the §G contamination guard; they are
artifacts of my environment, not of the tree.

### H1 — Verified-landed ledger (wing)

Zero keyhole ghosts; zero `.inl` mentions; zero local `namespace wgpu`
blocks; 13/18 carry the wgpu_fwd include (the five without name no
handles — correct). mood's shadowed loop is `k`; input's registry husk
is gone and `SCROLL_ZOOM_SCALE` exists as definition + use; ribbon's
"ride RibbonDeps" truth landed; parameter underscores are bare in the
patch_system definitions (teardown_surface sampled: `tile_world_state`,
`themes_state`); grounded.hpp is titled clean with the worst-named flag
dead; the Sphere renames run consistently through traits, gates, and
color sites; **pawn.hpp is fully clean — the wing's exemplar**, not one
residue token.

### H2 — Wing census and fossils

Clean census (the eighteen only; raw hits, classification pending):

| token class | hits | notes |
|---|---|---|
| m1–m6 | 31 | the predicted m-heavy wing — body banners narrate stage history |
| p1a/p1b(-x) | 19 | matrix LOCKED banner + point-adjacent narration |
| d1–d4 | 13 | classify; code false-positives expected |
| R1 / R5 | 5 | recon coordinates |
| C2–C8 | 4 | |
| p2 | 3 | the Jean's-stamp cluster — see fossil 10 |
| theory v2 / A-era / Keyhole form | 1+1+1 | fossils 9 and B2.1's twin |
| husk sweep / residue T2 | 0 | realization-side classes; absent here as expected |
| K# 7 · Q# 15 · V1 1 · O-# 4 | — | **KEEP class**, listed as boundary proof |

Post-classification wing estimate: **~40–65 sites**. Combined tree mop:
**~100–175 comment sites**, one pass.

New fossils:

8. **The self-referential rename casualty** — spheres.hpp banner:
   "renamed from ActiveSphere this sweep." The token pass renamed the
   *historical* name inside the sentence documenting the rename; it
   must read "renamed from ActiveFloater." One word — the sweep's most
   elegant artifact, and a textbook token-pass hazard: rename sweeps
   must exempt the sentence that records the old name.
9. **theory v2, third site + A-era** — population_themes.hpp:66
   ("theory v2 §4; formalized at the A-era"). Joins B2.2's `[?]` class
   for the v3 §-mapping; "A-era" enters the DELETE table.
10. **The p2-stamp cluster** — matrix ×2, demo ×1, plus "(p1b)" in the
    LOCKED banner: the mop's authority-keep/coordinate-strip exemplars.
    "Jean's ratified grid" survives; "p2" dies.

Boundary ruling recorded: **"Lifecycle Phase 2"** (spheres, ×2) is
KEEP — it names a live design state of sphere lifetime, not a campaign
coordinate.

### H3 — Consolidation (wing)

**Zero merges.** Every body owns a family; the direction pair
(input/mood) owns driving; the surface trio splits cleanly —
patch_system the conductor, tile_world the cache, population_themes the
envelope *and already a panel*. gallery is the tree's largest file yet
coheres as one composite family plus its photographer; the roster's
LATENT[roster-split:photographer] remains its only registered fission,
correctly parked.

Panel-era register grows by one: the **zone/pulse panel** — terrain_looks
ROW 9 already points at "GoL keeps its own panel"; formalize it at the
panel era. The register now stands at four: **placement, atmosphere,
point, zone/pulse** — the master control panel's scope, assembling
itself as a named list.

### H4 — Friction delta (wing)

Matches round one: ownership reads at a glance, tuning consoles sit at
the top of each body, the History line costs one look. The residual
friction concentrates exactly where the census says — fifty bare m/p1
coordinates in banners that narrate stage history no reader can now
resolve. The mop's territory, nothing more.

### H5 — Dispositions (delta over §F)

Unchanged in structure. The mop's tables extend: **A-era → DELETE**;
**"Lifecycle Phase N" → KEEP**; the p2 cluster under
authority-keep/coordinate-strip; both Keyhole-form sites enumerated
(surface_services.hpp:189, patch_system.hpp:190); the spheres
one-word truth-fix (fossil 8) joins the fossil list. The FINISHER's
contents are now fully enumerated across both rounds.

**The §A gate still stands.** Nothing in round two touches it: TileGrid's
fixed 289 and the absent flip remain the tree's one hazard, and CC's git
evidence remains the next physical step before anything else moves.

---

*Report complete — both rounds read. Next: the §A git evidence, then
the FINISHER against the verified tree, then row one.*
