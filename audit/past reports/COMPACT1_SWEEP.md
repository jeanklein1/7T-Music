# COMPACT-1 — THE SWEEP (close-out)

Comment-only compaction of `the_board`, per the COMPACT-1 order.
Three commits: (1) narration cut + F3 executed, (2) keyhole.hpp
boilerplate consolidation, (3) this close-out + the code-repetition
appendix. Census tooling and recipes are those of COMPACT-0
(audit/COMPACT0_CENSUS.md) — same tokenizer (`\w+|[^\w\s]`), same
bucket recipes, re-run unchanged at the post-sweep head.

## 1. What was cut, what was kept

**Commit 1 (comment-only; gate = CODE-TOKEN-IDENTICAL per file).**
The sweep ran paragraph-granular: a paragraph is a maximal comment run
unbroken by blank lines, blank comments, box art, or code. Whole
paragraphs classified LEDGER (SEAM/NOTE/INTENT/STATUS/TESTING/
LOCKSTEP/ROSTER-GATE/HOME/PAIRING …) were immune — no SEAM block was
truncated. NARRATION paragraphs were cut, or replaced from a rescue
table where they carried present behavior. 157 paragraphs processed by
the sweep (101 rescued in present phrasing, 56 pure cuts), followed by
a hand pass over ~40 residual sites the paragraph shield had protected
(narration paragraphs citing a SEAM, rung-cite prefixes on include
comments, trailing-comment cites).

**F3 EXECUTED, both strata.** DONE[] blocks: 26 → 0 (120 lines).
'Scope B' banners: 11 → 0. SEAM[] tags stayed. Constitution §5's
HISTORICAL NARRATION entry now records the execution; "PENDING RULING
F3" is struck from the open-rulings list.

**Provenance.** Each swept file carries at most ONE pointer line in
its banner ("Converted <arc>: history in audit/LADDER.md"); the
narrative lives in audit/LADDER.md, unchanged.

**Rescues (the PRESENT-override clause: "if a cut reveals a
misclassified fragment, rescue it and disclose — never delete present
behavior").** Classes rescued, in present phrasing:

- Composition-root ACCESS LAW + the ten organ identity lines
  (cartridge.hpp).
- The MODULE IMPLEMENTATIONS zone law ("still ONE translation unit…").
- All ten impl-banner reaches-lists (what each .inl dereferences).
- Mirror pins: gol_zones MODE_LATTICE_SPACING, cube_behaviors
  CUBE_BEHAVIOR_COUNT_WGSL, state.hpp COUPLING_* block, the ribbon
  stride convention.
- roster.hpp FIRST EDGE / boot-summary / independence notes.
- spawn_engine payload relocation pointers (×3, "live in <hpp> —
  file-scope vocabulary preceding these unions by construction").
- The gallery SHOT-column legend and five width-exact box rows.
- Ordering constraints revealed inside cut narration (apply_mood's
  applier call order is load-bearing; request_mood_transition's
  one-door contract; has_anchor_ribbon's identifier-not-discriminator
  rule; entity tier data's single-source-of-truth home).

Kept verbatim (ledger/override classes): WRAPPING/WIRING FORM
paragraphs with their fix-2 cites, PAIRING blocks (ribbon.hpp/.inl),
HOME blocks (roster.hpp, entities.inl), TESTING[test-rig-piers],
STATUS: LATENT entries, SEAM attributions (e.g. SEAM[spine:transitions]
(K4, Jean, 2026-07-11)).

**Commit 2 (code change, own gate).** modules/keyhole.hpp — the
forward-decl home — now carries the Cartridge fwd + the wgpu::Queue
fwd with the single lockstep-insurance comment. The twelve module
headers include it in place of their per-header ceremony; orbs.hpp
keeps only its two extra encoder fwds. Verification (disclosed in the
commit): fwd-once census, include census (12 × exactly once, each
include preceding the old fwd position), token-diff identity per file
(exactly {+include, −fwd(s)}), zone census, encodings. Repeated
identical forward declarations are legal C++; the TU's declaration set
is unchanged. Compile certification rides the rig as usual.

## 2. Tokens recovered vs forecast

COMPACT-0 forecast (file-pointer form, the stamped target): −11.3k
comment tokens. Delivered:

| measure | pre-sweep (98dc748) | post-sweep | delta |
|---|---|---|---|
| comment tokens | 115,024 | 110,052 | **−4,972** |
| NARRATION bucket | 10,833 t / 827 L / 181 blocks | 1,329 t / 90 L / 44 blocks | −9,504 t |
| DONE[] stratum | 26 blocks / 120 L | 0 | −all |
| Scope-B banners | 11 L | 0 | −all |
| comment lines | 7,671 | 7,294 | −377 |
| blank lines | 3,154 | 2,992 | −162 |
| code tokens | 177,256 | 177,380 | +124 (keyhole.hpp + 12 includes, commit 2) |

**Why −4,972 and not −11.3k: the rescue clause.** The forecast priced
pure file-pointer compression. The NARRATION bucket itself shrank
−9,504 tokens (within ~16% of its share of the forecast), but ~4.4k
tokens of it were carrying present behavior and were REWRITTEN into
the PRESENT bucket (86,055 → 90,486) rather than deleted — the
"never delete present behavior" override, exercised 101+ times. The
archaeology died; its cargo didn't.

**The residual 44 "NARRATION" blocks are not archaeology.** They are:
(a) the sanctioned one-per-file provenance pointer lines, which match
the census regex by construction (`LADDER.md`); (b) the PAIRING and
HOME ledger blocks (ribbon.hpp 13 L, roster.hpp 11 L, entities.inl,
ribbon.inl), ratified as LEDGER classes in the sweep classifier after
COMPACT-0's recipe was frozen — the COMPACT-0 regex, re-run unchanged
for comparability, cannot see them as LEDGER; and (c) DUPLICATE
overlap with audit/LADDER.md lines. Zero unsanctioned narration
remains: `DONE[` = 0, `Scope B` = 0, and every remaining rung cite
sits inside a kept ledger-class block or a provenance pointer.

## 3. Updated totals (pre-MOD → pre-sweep → post-sweep)

Scope: `src/cartridges/the_board/` .hpp/.inl (COMPACT-0's scope).

| | pre-MOD (3aac3db) | pre-sweep (98dc748) | post-sweep |
|---|---|---|---|
| files | 20 | 34 | 35 (+keyhole.hpp) |
| code lines | 17,468 | 18,012 | 18,017 |
| comment lines | 6,524 | 7,671 | 7,294 |
| blank lines | 2,990 | 3,154 | 2,992 |
| code tokens | 170,867 | 177,256 | 177,380 |
| comment tokens | 97,306 | 115,024 | 110,052 |
| **MOD growth, comment** | — | +17,718 (+18.2%) | **+12,746 (+13.1%)** |
| **MOD growth, total** | — | +24,107 | **+19,259** |

The campaign's commentary growth is cut by 28% while every ledger
entry, seam, mirror pin, and present-behavior note survives.

## 4. Per-file bytes (f9ffafc → post-sweep head, all three commits)

Total: −26,705 bytes across changed files (−27,933 recovered from
pre-existing the_board source; +1,034 keyhole.hpp; +194 constitution).

| file | bytes | Δ | | file | bytes | Δ |
|---|---|---|---|---|---|---|
| cartridge.hpp | 234,925 | −12,243 | | modules/input.inl | 14,753 | −535 |
| modules/agents.hpp | 27,814 | −1,349 | | modules/mood.hpp | 19,462 | −527 |
| modules/mood_constants.hpp | 2,263 | −1,045 | | modules/spawn_engine.inl | 51,605 | −400 |
| modules/gallery.hpp | 34,079 | −1,005 | | modules/entities.inl | 10,714 | −382 |
| modules/pawn.hpp | 5,822 | −902 | | modules/render_passes.hpp | 4,419 | −351 |
| modules/entities.hpp | 36,561 | −882 | | modules/spheres.hpp | 2,567 | −301 |
| modules/ground_architecture.hpp | 20,869 | −840 | | modules/orbs.inl | 25,624 | −230 |
| modules/ribbon.hpp | 37,766 | −730 | | modules/seed_utils.hpp | 5,013 | −223 |
| modules/orbs.hpp | 26,975 | −699 | | modules/entity_pipeline.inl | 111,731 | −206 |
| modules/mood.inl | 57,380 | −625 | | renderer.hpp | 147,587 | −204 |
| modules/entity_types.hpp | 10,569 | −608 | | modules/render_passes.inl | 32,038 | −192 |
| modules/input.hpp | 6,514 | −603 | | modules/floater_vocabulary.hpp | 12,633 | −191 |
| modules/gol_zones.hpp | 20,263 | −596 | | modules/pawn.inl | 2,943 | −190 |
| modules/cube_behaviors.inl | 13,498 | −587 | | modules/agents.inl | 24,079 | −184 |
| modules/cube_behaviors.hpp | 17,205 | −570 | | roster.hpp | 8,945 | −129 |
| | | | | modules/ribbon.inl | 47,193 | −104 |
| | | | | modules/gallery.inl | 59,064 | −101 |
| | | | | state.hpp | 334,555 | −100 |
| | | | | modules/gol_zones.inl | 15,004 | −99 |
| | | | | modules/keyhole.hpp | 1,034 | +1,034 |

Encodings held throughout: the BOM quartet (agents.inl, ribbon.inl,
render_passes.inl, entity_pipeline.inl) still ef-bb-bf/LF; all others
no-BOM/LF; world.wgsl untouched.

## 5. CODE-REPETITION APPENDIX (read-only; no action)

Repeated CODE idiom classes observed during the arcs, Q10 style —
locations + counts, no ruling. Where classification is ambiguous, the
default is "contract-mandated idiom" per the order.

**A. prepare_*_mesh_gen sextuplet** — entities.inl:36/50/64/78/99/114.
Six preparers with the same skeleton (pending-flag check → per-family
index-count computation → GPU upload → flag clear). Sits directly on
the P10 cookie-cutter (per-family specificity is intentional); the
skeleton, not the bodies, is the repeated part.

**B. render_passes draw ladders** — render_passes.inl. The shadow pass
walks 12 near-identical `renderer_.draw_shadow_*` call blocks; the
main pass walks ~17 `renderer_.draw_*` call blocks of the same shape
(bind → count check → draw). The ladder shape is the pass contract;
the repetition is the roster of pieces.

**C. Selection→Placement field-copy ladders (×3)** —
gol_zones.inl:193, gallery.inl:397, ribbon.inl:769. Each
`place_*_from_selection` zero-inits a Placement then copies Selection
fields member-by-member (slot / trigger_gx / trigger_gz / …). The
unions in spawn_engine.inl mandate distinct types; the copy ladders
are the price of the select/place boundary.

**D. FAMILY_DISPATCH wrapper thunks** — cartridge.hpp, 34 `static
void/bool dispatch_*` thunks (evict / prepare_mesh / mesh_gen per
family). Named in constitution §5 as EVICTION THUNKS; the fn-ptr table
mandates uniform signatures — contract-mandated.

**E. upload_ground_entries per-family packing (×7)** —
render_passes.inl:38/55/63/76/100/107/114. Seven per-family loops
(arch / column / antenna / pyramid / palm / cactus / blade) packing
instance stores into ground entries with the same loop shape.

**F. Authored-pick fallback double-scan (×2)** — gallery.inl:536–552
(commit_gallery) and 1207–1264 (place_wall_paintings). The
count_unused_authored → fallback → pick_authored_staging sequence is
duplicated across the terrain and wall painting paths.

**G. Doorway wall-margin computation (×2)** — mood.inl ~860–870
(force_spawn_back_portal) and ~991–1001 (force_spawn_finite_portals).
Both compute the INDOOR_ENTITY_WALL_MARGIN clamp for a portal arch
against the finite wall; same arithmetic, two authors.

**H. Ribbon ground-sample block (×2)** — ribbon.inl:526/557. The
estimate_terrain_height sampling block is duplicated across two
frame_tick branches.

**I. R13 flush seams** — the per-frame flush/upload call shape
repeated across modules is the contract-mandated default (the frame
law); listed for completeness, not as a candidate.

## 6. Standing state

PRESENT-BEHAVIOR and LEDGER strata untouched, as ordered. Zone census
green at the close-out head (11 self-wrapping impls == 11 zone
includes, exactly once; spawn_engine.inl + entity_pipeline.inl remain
the two class-body includes, not self-wrapping). Constitution §5
carries F3 EXECUTED. L-HUBS (LADDER-5) remains queued, untouched by
this order.
