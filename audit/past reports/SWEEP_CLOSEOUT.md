> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# DRIFT SWEEP — CLOSE-OUT

Corrective comment-and-doc sweep executed against the work order
(`audit/CC_AUDIT_REPORT.md`, tree `dd2b3ed`) under the DRIFT SWEEP
AUTHORIZATION. Branch `FINAL_TOUCH`. Scope ruling: **the_board only**
(operator ruling — the working cartridge); the two byte-identical twins
(`ribbon.inl`, `gol_zones.inl`) were corrected in the_board alone, so
they now carry an accepted lockstep drift (40 / 8 lines) that a later
the_chord pass reconciles. `the_chord` and `backup_board` untouched.

Discipline gates: **D1 PASS** (every code-file hunk verified comment-text
only — trailing-comment edits keep the code token byte-identical); **D2
PASS** (BOM state + pure-LF preserved on all 18 files, matches L0);
**D3** satisfied (constitution §5 eviction count edited in this commit —
§5 *is* the census; the DONE[mood:K2] content fix changed no §5-tracked
count, so no further §5 edit); **D4** one commit.

---

## 1. FIXED

### L4 DRIFT (33 of 34 in-scope; #10 → SKIPPED under S1)

| # | file | before → after |
|---|------|----------------|
| 1 | cartridge.hpp:61-64, 233-236, 254 | aura_presence "scheduled to migrate" (×2 SEAM[spine:P8]) → "has already migrated"; :254 "(was player_.aura_presence)" → "(was pawn_state_.aura_presence)" |
| 2 | cartridge.hpp:961-962 | DENSITY table 0.1/3.0 → 1.0/1.0; "Ceiling (3× base spawn rates)" → "Ceiling (now 1.0× — no boost)" |
| 3 | cartridge.hpp:1828-1835 | POP BATCH table: 7 stale rows → live constants (16, 0.0, 0.0, 1, 0.0, 0.0, remainder 1.0) |
| 4 | cartridge.hpp:1484-1488 | THEME CONTROL SURFACE weights 0.40/0.12/0.18/0.15/0.15 → 0.21/0.30/0.31/0.18/0.04; density ×0.7/×1.5/×1.2/×0.3 → ×1.0 |
| 6 | cartridge.hpp:3709 | "frustum cull bypassed — direct draw active" → "re-enabled — indirect draw active" (non-census DIAG: preserved) |
| 7 | mood.inl:487, 656, 69 | "five sub-functions … band motion" → "four"; DONE[mood:K2] "five helpers below" → "four helpers below" |
| 8 | state.hpp:489 | "grid dimension (13)" → "(17)" (TILE_GRID_SIDE) |
| 9 | state.hpp:1540/1545/1551 | heightfield/color/LUT "49 layers" → "225 layers" |
| 11 | state.hpp:21 | binding 26 "solid_instances Uniform" → "pier_instances ReadOnlyStorage" |
| 12 | state.hpp:4707, 5446 | "17 entries" → "19"; "2 entries" → "3" |
| 14 | world.wgsl:89 | MODE_COUPLING_MAGNITUDE directory row 0.25 → 0.0 (DISABLED) |
| 15 | state.hpp:453 | "(kept size 384)" → "(struct is now 400 bytes)" |
| 16 | world.wgsl:69, 1412; state.hpp:430 | palette "3=grey" → "3=warm" (per authority world.wgsl:1465 "warm — dusty terracotta") |
| 18 | world.wgsl:4296-4298 | BNK-1 "saddle rides GIMBAL-level … pending Jean's ruling" → "BNK-2 landed: saddle wears the full frame" |
| 19 | world.wgsl:10560 | orb "Musical couplings uploaded per-frame" → "held at neutral (configure_orbs zeros them, no upload)" |
| 20 | state.hpp:1132 | motion_rule "0=Brownian 1=Orbital 2=Frozen" → "+ 3=Flocking" |
| 21 | state.hpp:603, 820, 842, 870, 905, 940, 974 | rotted line refs → section refs (§6.3; §9.0–§9.5 per world.wgsl nav rule) |
| 22 | entity_pipeline.inl:3 | "seven cookie-cutter families" → "nine" |
| 23 | entities.inl:669-671; spawn_engine.inl:30-32 | preparers "currently live in spawn_engine … will be hoisted" → "now live here (migration #10 landed)"; spawn_engine box → "prepare_{arch,column,pyramid}_mesh_gen — now in entities.inl" |
| 24 | cube_behaviors.inl:48, 452 | "Called from update()" → "render()" (call is in render(), cartridge.hpp:3372) |
| 25 | input.inl:104 | registry ended at F7 → added F8 row (toggle_sky_mode) — registry header mandates registry↔dispatch sync |
| 26 | input.inl:102; cartridge.hpp:3508 | F6 "teleport cubes" → "glide cubes around pawn (4s)"; "~3s" → "~4s" (CUBE_CORRAL_DURATION=4.0f) |
| 27 | orbs.inl:24, 787 | cycle_orb_palette "KP_0" → "0" (dispatch = GLFW_KEY_0, main row) |
| 28 | gallery.inl:249-256 | outdoor 70/20/10 → 80/5/15; indoor 40/20/40 → 15/5/80 |
| 29 | gallery.inl:575, 1384, 1532 | "sorted alphabetically / at startup" → "sorted numerically / lazily on first load" |
| 30 | gol_zones.inl:61-62; world.wgsl:1667 | SPECIAL: named the GOL/PULSE twin both ways, both live, "a tuner must edit both"; retired WGSL "single source of truth" — **no single-source elected** (L7 #7 left open) |
| 31 | ribbon.inl:993-997 | color gen-2 "will write through" (future) → "now computes the color per frame in the flush loop above" |
| 32 | input.inl:328-331; ribbon.inl:697-702 | sky-flight "(Stage 1) … fixed sky altitude … Stages 2-3 add snap, camera, fade" → "snap + camera landed; only fade remains; altitude held by the pen" |
| 33 | renderer.hpp:8, 29-31 | COMPUTE table "(18 shown; bodies build 31)"; RENDER "(34 built; pyramid render + wall-painting shadow built but not drawn)" [NO-BASELINE; ruling-neutral, L7 #10 open] |
| 34 | constitution §5:114 | "EVICTION THUNKS (1 class, 13 functions)" → "12 in the_board; 13 in the_chord (keeps dispatch_evict_noop)" [also L8 #50; D3] |
| 35 | datasheet §1 banner | "written at commit, static per life today" → "written at commit, then time + coupled color/amps flush per frame" |
| 36 | datasheet BNK-2 ledger (line 153) | added forward pointer: "(SUPERSEDED by SNAP-1 below … the seam was reversed on the record)" |
| 37 | agents.inl:426 | "GPU storage buffers (bindings 110/111)" → "GPU uniform buffers" (world.wgsl:739/754 var<uniform>) |

### L8 DRIFTED self-claims (in-scope; #17, #43 excluded)

| # | file | before → after |
|---|------|----------------|
| 7 | cartridge.hpp:61-64, 233-236 | (same as L4 #1) |
| 8 | cartridge.hpp:410-411 | mood_name "compiler catches a missing entry" → "catches an EXTRA entry … a missing one is not — zero-fills to nullptr" |
| 13 | state.hpp:1926 | "the whole 192-byte struct" → "208-byte" (assert :1414) |
| 14 | state.hpp:453 | (same as L4 #15) |
| 15 | state.hpp:1540 | (same as L4 #9) |
| 21 | world.wgsl:6145 | FLOATER_EVICTION_RADIUS "MIRRORED MANUALLY in cube_behaviors.inl … change the C++ side too" → "GPU-only: no C++ constant mirrors this; the CPU learns of evictions through the is_active readback" (L7 #15 authority left open) |
| 25 | world.wgsl:10560 | (same as L4 #19) |
| 29 | entity_pipeline.inl:34, 181, 2272 | "eight families) / eight family blocks / eight families: 24 entries" → "nine families / nine families / nine families: 27 entries" |
| 33 | floater_vocabulary.inl:43 | "Depends on: … entities.inl (MOOD_COUNT)" → "cartridge.hpp (MOOD_COUNT)" (defined at cartridge.hpp:362) |
| 34 | entities.inl:669-671 | (same as L4 #23) |
| 36 | ribbon.inl:37, 51 | public-surface box: added ribbon_head_frame (external, was missing); dropped RIBBON_MAX_LENGTH from cross-module reads (zero external consumers) |
| 37 | ribbon.inl:57-62 | dependency list: removed evaluate_spawn_gate + jittered_position (never called) and pawnReadback_* (migrated); added select_tier_biased, four ribbon canvas bindings, player sky fields, estimate_terrain_height, the four upload wires |
| 39 | ribbon.inl:500-504 | "All ribbon-owned state lives in this struct" → "Most … — exceptions: the four ribbon canvas bindings and player_.sky_yaw_eased on the Cartridge" |
| 41 | gol_zones.inl:36; gallery.inl:49 | "reads" boxes noted the writes: gol note "(spine writes zones[].active; mood sets mood_allowed)"; gallery_centers "read by spine" → "read/written by spine" |
| 49 | renderer.hpp:53 | "S1 ENTRY POINTS — Must match world.wgsl §7" → "§6-§9" (64 names match but two-thirds live in §6/§8/§9) [NO-BASELINE] |
| 50 | constitution §5 | (same as L4 #34) |

### ADDENDUM (parallel-audit items)

| id | file | before → after |
|----|------|----------------|
| A1 | spawn_engine.inl:245 | record_placement_bookkeeping "shared by all three families" → "shared by every family (the generic pipeline plus the bespoke gallery/gol/ribbon commits)" |
| A2 | cartridge.hpp:3762, 4020 | spawn order "(pyramids → arches → columns)" → "(families in FAMILY_DISPATCH order)"; "Priority order … pyramids → arches → columns (largest footprint first)" → "follows FAMILY_DISPATCH (families iterated 0..PopFamily::COUNT; pyramid/arch/column lead)" |
| A3 | entity_pipeline.inl | swept every "seven/eight famil" string → "nine" (lines 3, 34, 181, 2272; overlaps L4 #22 / L8 #29) |

---

## 2. SKIPPED

| # | class | reason |
|---|-------|--------|
| L4 #5 | S3 (+ mission-excluded) | intent-vs-code type specimen ("Arch→Pyramid = 0 explicitly allowed" vs matrix 60.0f) — ratify neither side |
| L4 #10 | S1 | report records both readings surviving ("Both comments survive uncorrected"; console.hpp requests full limits, layout binds 10, "8" is only the WebGPU default) |
| L4 #13 | S2 (L7-gated, mission-excluded) | pulse onset seconds↔beats — the DRIVERLESS park is the finding (L3 #71 family) |
| L4 #17 / L8 #17 | S2 (L7-gated, mission-excluded) | dt_beats ↔ _pad1 cross-bus defect (L7 #2) — code fix, not a comment correction |
| L8 #43 | mission-excluded | gallery.inl:23 pick_authored_staging non-const — the fix is a code token edit (S4) |

---

## 3. FOUND-NOT-FIXED

- **mood.inl sub-function numbering skips 4** (comments `// 1)`, `// 2)`, `// 3)`, `// 5)` — no `4)`). The audit put "numbering skips 4" on the CURRENT side (B) of #7, so it is present behavior, not drift; left as-is. Noted here because a reader now sees "four sub-functions" numbered 1/2/3/5.
- **renderer.hpp COMPUTE table is a partial list** (18 shown of 31 built). I annotated the count rather than enumerate all 31 entries — a full per-pipeline table is a larger authoring task and [NO-BASELINE]. The RENDER list similarly annotated (34 built) without expanding every shadow variant.
- **Public-surface box header quirk** (ribbon.inl:18, spawn_engine.inl:8, gol_zones.inl top border): the `┌…┐` border rows are 70 cols while `│…│` content rows are 70+1; a pre-existing cosmetic 1-col corner offset, not introduced here, left untouched.

### Count discrepancies (recorded, not forced — per the authorization)

- **POP BATCH (L4 #3)**: my executed count is **7 of 8 rows stale** (only POP_GOL_SUPPRESSION 0.05 matches); the report said "six of seven." Corrected all 7 stale rows.
- **A1 record_placement_bookkeeping**: the addendum cited "ten callers." My executed count is **4 direct call sites** (entity_pipeline.inl:296 generic_commit — covering all 9 generic families — plus gallery.inl:914, gol_zones.inl:502, ribbon.inl:1296). "Ten callers" is run_spawn_preamble's P11 count (a different helper). I wrote the accurate present fact ("every family: generic pipeline + bespoke gallery/gol/ribbon") rather than force "ten."
- **entity_pipeline.inl:181 "eight family blocks"**: literally accurate as **8 section headers** (Column+Antenna share one block) but **9 families** (distinct dispatch_select_* stems). A3 asked for "nine"; I reworded to "all nine families" to match the family count, which the file's other strings use.

---

## 4. FINGERPRINT (post-edit, beside L0 original — must be identical)

All 18 changed files: pure LF, no CRLF. BOM state matches L0 exactly.

| file | first-3-bytes | BOM | endings | L0 match |
|------|---------------|-----|---------|----------|
| cartridge.hpp | 23 70 72 (`#pr`) | none | LF | ✓ |
| modules/agents.inl | ef bb bf | BOM | LF | ✓ |
| modules/cube_behaviors.inl | 2f 2f 20 (`// `) | none | LF | ✓ |
| modules/entities.inl | 2f 2f 20 | none | LF | ✓ |
| modules/entity_pipeline.inl | ef bb bf | BOM | LF | ✓ |
| modules/floater_vocabulary.inl | 2f 2f 20 | none | LF | ✓ |
| modules/gallery.inl | 2f 2f 20 | none | LF | ✓ |
| modules/gol_zones.inl | 2f 2f 20 | none | LF | ✓ |
| modules/input.inl | 2f 2f 20 | none | LF | ✓ |
| modules/mood.inl | 2f 2f 20 | none | LF | ✓ |
| modules/orbs.inl | ef bb bf | BOM | LF | ✓ |
| modules/ribbon.inl | ef bb bf | BOM | LF | ✓ |
| modules/spawn_engine.inl | 2f 2f 20 | none | LF | ✓ |
| renderer.hpp | ef bb bf | BOM | LF | ✓ |
| state.hpp | 23 70 72 (`#pr`) | none | LF | ✓ |
| world.wgsl | 2f 2f 20 | none | LF | ✓ |
| src/docs/cartridge_constitution.md | 23 20 54 (`# T`) | none | LF | ✓ |
| src/docs/ribbon_color_coupling_datasheet.md | 23 20 52 (`# R`) | none | LF | ✓ |

The LAW-8 encoding ruling (L7 #1) stays open; no normalization performed.

---

*End of close-out. All corrections are comment-text or src/docs/*.md;
no code tokens, consts, static_asserts, tags, or data touched. Operator
gate: build after merge.*
