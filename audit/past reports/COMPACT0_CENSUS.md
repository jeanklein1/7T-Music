> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# COMPACT-0 — THE GROWTH CENSUS (read-only; no cuts)

Head measured: `98dc748` (LADDER-4). Pre-MOD base: `3aac3db` ("campaign:
L0 alignment" — the parent of ROSTER-1a Phase R). Scope: every
the_board `.hpp`/`.inl` (spine + headers + impls + remaining class-body;
world.wgsl excluded — untouched by the campaign, byte-verified so at
both ends). Nothing was cut; this report is the numbers Jean rules from.

## 0. THE HEADLINE ARITHMETIC

|  | files | code lines | comment lines | blank | code tokens | comment tokens | total tokens |
|---|---|---|---|---|---|---|---|
| BASE (pre-MOD) | 20 | 17,468 | 6,524 | 2,990 | 170,867 | 97,306 | 268,173 |
| HEAD (LADDER-4) | 34 | 18,012 | 7,671 | 3,154 | 177,256 | 115,024 | 292,280 |
| **growth** | **+14** | **+544 (+3.1%)** | **+1,147 (+17.6%)** | +164 | **+6,389 (+3.7%)** | **+17,718 (+18.2%)** | **+24,107 (+9.0%)** |

**The observation is real and it is 73% COMMENTARY.** Of the +24.1k
token growth, +17.7k is comment tokens and +6.4k is code tokens. The
code growth is the split pattern's structural price — declaration
blocks, per-file includes, namespace wrappers, `inline` + keyhole
signatures spread across the 29 converted-module files (11 pairs + 4
reborn headers + 3 born citizens), ≈ 220 code tokens per file on
average — priced, transitional, self-retiring at M-j. The comment
growth is textual and is what COMPACT-1 can cut.

## 1. FILE-COUNT ARITHMETIC (structural growth, separated)

- **20 → 34 files (+14).** Decomposition:
  - **11 split pairs** (+11): pawn, entities, orbs, gol_zones, agents,
    cube_behaviors, gallery, ribbon, input, render_passes, mood — each
    one class-body `.inl` became `.hpp` + post-class `.inl`. This is
    the transitional pair model; each pair merges back to one file only
    if/when a future ruling says so (M-j territory), so the +11 is the
    campaign's deliberate structural cost.
  - **4 dissolved-and-reborn** (net 0): seed_utils, ground_architecture,
    entity_types, floater_vocabulary — `.inl` deleted, `.hpp` born
    (header-only citizens; floater_vocabulary's `.inl` died at c4).
  - **3 born citizens** (+3): roster.hpp (the manifest's graduation),
    mood_constants.hpp (G1's home), spheres.hpp (M-c's sphere owner).
  - **2 remaining class-body** (net 0): spawn_engine.inl,
    entity_pipeline.inl — the hubs, queued as LADDER-5.

## 2. RECIPES (how every number was made — reproducible)

- **Tokens**: `\w+|[^\w\s]` regex count (word ≈ 1 token, punctuation ≈ 1
  token each). Approximate but consistent across BASE/HEAD, so deltas
  are honest even where absolutes drift from any given tokenizer.
- **Line classes**: first non-ws `//` → COMMENT; empty → BLANK; else
  CODE. Trailing `//` comments on code lines: the line counts CODE, its
  comment-portion tokens count COMMENT (unbucketed — see limitations).
- **Comment buckets** (precedence: PRESENT-override → LEDGER →
  DUPLICATE → NARRATION → PRESENT), applied per BLOCK — a tagged or
  marked line captures following same-or-deeper-indent comment lines
  until a blank comment, banner art, code, or a new tag:
  - **PRESENT-override**: blocks opening `WRAPPING FORM` /
    `WIRING FORM` / `SECTION ORDER` / `Depends on:` / `Public surface`
    — the order names wrapping-form notes lawful residents even where
    they cite provenance.
  - **LEDGER** starts: `SEAM[` `NOTE[` `INTENT[` `STATUS:` `LATENT`
    `TESTING` `DIAG-unwrapped` `RETIREMENT:` `LOCKSTEP` `ROSTER-GATE`
    `ROSTER-RESIDUE` `HOME (`.
  - **NARRATION** starts: `DONE[`, `LADDER-<n>`/`(LADDER`/`LADDER.md`,
    `ROSTER-1a/b`, CONVERTED/REPURPOSED/RELOCATED/GRADUATED/MIGRATED
    (any case), `Scope B migration`, `per Jean`/`Jean's`, ruling and
    graduation names (K4/G1/G2), `fix-2`/`LNK2019`/`C2248`,
    `precedent`, `came home`, `was declared in`/`was a Cartridge`,
    `used to`/`previously`, retirement-fulfilled phrasing, `the c<n>
    retrofit`, `rig-found`, `header/impl split`, `class-body`,
    `keyhole's static form`, amendment language.
  - **DUPLICATE**: a comment line (normalized ws, ≥25 chars) appearing
    verbatim in BOTH halves of a module's `.hpp`/`.inl` pair (the
    `.inl` copy counts), or verbatim from roster.hpp or audit/LADDER.md.
  - **PRESENT**: everything else — identity sentences, public-surface
    boxes, governing-law-beside-constant, parameter docs.

## 3. THE PER-FILE CENSUS (HEAD)

| file | code L | comment L | blank L | code tok | comment tok | PRESENT | LEDGER | NARRATION | DUP |
|---|---|---|---|---|---|---|---|---|---|
| cartridge.hpp | 2423 | 1351 | 412 | 23514 | 19063 | 923 | 148 | 280 | 0 |
| modules/agents.hpp | 185 | 288 | 43 | 2037 | 3498 | 263 | 3 | 22 | 0 |
| modules/agents.inl | 311 | 172 | 67 | 3170 | 2172 | 165 | 0 | 6 | 1 |
| modules/cube_behaviors.hpp | 94 | 188 | 35 | 952 | 2531 | 168 | 0 | 20 | 0 |
| modules/cube_behaviors.inl | 175 | 101 | 31 | 1845 | 1378 | 87 | 0 | 14 | 0 |
| modules/entities.hpp | 378 | 259 | 77 | 3038 | 4711 | 224 | 12 | 23 | 0 |
| modules/entities.inl | 185 | 50 | 30 | 1806 | 761 | 17 | 6 | 21 | 6 |
| modules/entity_pipeline.inl | 1672 | 382 | 225 | 22194 | 4985 | 372 | 6 | 4 | 0 |
| modules/entity_types.hpp | 95 | 106 | 22 | 491 | 1747 | 80 | 11 | 15 | 0 |
| modules/floater_vocabulary.hpp | 80 | 124 | 17 | 622 | 1861 | 90 | 28 | 6 | 0 |
| modules/gallery.hpp | 314 | 307 | 76 | 2465 | 4087 | 256 | 26 | 25 | 0 |
| modules/gallery.inl | 980 | 185 | 182 | 9993 | 2281 | 163 | 8 | 13 | 1 |
| modules/gol_zones.hpp | 167 | 164 | 57 | 1652 | 2979 | 124 | 17 | 23 | 0 |
| modules/gol_zones.inl | 230 | 68 | 50 | 2389 | 908 | 58 | 0 | 9 | 1 |
| modules/ground_architecture.hpp | 145 | 225 | 27 | 900 | 2771 | 150 | 47 | 28 | 0 |
| modules/input.hpp | 36 | 92 | 12 | 217 | 1187 | 77 | 0 | 15 | 0 |
| modules/input.inl | 192 | 103 | 33 | 1603 | 1758 | 84 | 1 | 16 | 2 |
| modules/mood.hpp | 87 | 205 | 23 | 1648 | 2782 | 135 | 36 | 34 | 0 |
| modules/mood.inl | 788 | 300 | 139 | 9366 | 4237 | 255 | 5 | 37 | 3 |
| modules/mood_constants.hpp | 19 | 39 | 7 | 90 | 596 | 18 | 0 | 21 | 0 |
| modules/orbs.hpp | 246 | 240 | 58 | 2922 | 3344 | 216 | 8 | 16 | 0 |
| modules/orbs.inl | 386 | 123 | 75 | 4010 | 1897 | 114 | 0 | 8 | 1 |
| modules/pawn.hpp | 46 | 70 | 15 | 206 | 1150 | 49 | 6 | 15 | 0 |
| modules/pawn.inl | 22 | 27 | 7 | 255 | 462 | 20 | 1 | 5 | 1 |
| modules/render_passes.hpp | 18 | 54 | 7 | 189 | 708 | 45 | 0 | 9 | 0 |
| modules/render_passes.inl | 572 | 130 | 97 | 5512 | 1814 | 121 | 0 | 8 | 1 |
| modules/ribbon.hpp | 295 | 300 | 57 | 2552 | 5402 | 232 | 20 | 48 | 0 |
| modules/ribbon.inl | 581 | 281 | 70 | 6511 | 3960 | 242 | 25 | 13 | 1 |
| modules/seed_utils.hpp | 59 | 48 | 13 | 564 | 657 | 27 | 15 | 6 | 0 |
| modules/spawn_engine.inl | 550 | 402 | 115 | 5786 | 5873 | 369 | 21 | 12 | 0 |
| modules/spheres.hpp | 19 | 33 | 6 | 130 | 493 | 12 | 4 | 17 | 0 |
| renderer.hpp | 2291 | 292 | 337 | 18107 | 4989 | 278 | 7 | 7 | 0 |
| roster.hpp | 47 | 107 | 9 | 310 | 1509 | 68 | 15 | 24 | 0 |
| state.hpp | 4324 | 855 | 723 | 40210 | 16473 | 819 | 29 | 7 | 0 |
| **TOTAL (34 files)** | **18012** | **7671** | **3154** | **177256** | **115024** | **6321** | **505** | **827** | **18** |

**Bucket totals (comment lines / tokens):** PRESENT 6,321 L / 86,055 t;
LEDGER 505 L / 6,970 t; NARRATION 827 L / 10,833 t (181 blocks);
DUPLICATE 18 L / 816 t (14 blocks). Trailing-comment tokens on code
lines (unbucketed, predominantly governing-law-beside-constant →
present class): 10,350 t.

**Reading the buckets against the growth:** comment tokens grew +17.7k
over the campaign; the NARRATION bucket alone holds 10.8k tokens at
HEAD (the base had a small pre-existing stratum — 25 `DONE[` blocks +
11 Scope-B mentions — so roughly 8–9k of the narration is
campaign-written). NARRATION + DUPLICATE together ≈ **11.6k tokens ≈
two-thirds of the campaign's comment growth**, and that is the
compressible class. LEDGER (7.0k) and PRESENT stay, per the order.

## 4. THE FORECAST (NARRATION + DUPLICATE → one-line-plus-pointer)

Form priced: each contiguous narration/duplicate block compresses to
ONE pointer line (`// Converted: see audit/LADDER.md` style, ~10
tokens). Per file:

| file | narration L (blocks) | dup L (blocks) | line savings | token savings (est) |
|---|---|---|---|---|
| cartridge.hpp | 280 (64) | 0 (0) | 216 | 2836 |
| modules/agents.hpp | 22 (6) | 0 (0) | 16 | 247 |
| modules/agents.inl | 6 (1) | 1 (1) | 5 | 139 |
| modules/cube_behaviors.hpp | 20 (6) | 0 (0) | 14 | 203 |
| modules/cube_behaviors.inl | 14 (4) | 0 (0) | 10 | 151 |
| modules/entities.hpp | 23 (6) | 0 (0) | 17 | 281 |
| modules/entities.inl | 21 (3) | 6 (3) | 21 | 377 |
| modules/entity_pipeline.inl | 4 (2) | 0 (0) | 2 | 36 |
| modules/entity_types.hpp | 15 (3) | 0 (0) | 12 | 140 |
| modules/floater_vocabulary.hpp | 6 (2) | 0 (0) | 4 | 65 |
| modules/gallery.hpp | 25 (5) | 0 (0) | 20 | 257 |
| modules/gallery.inl | 13 (2) | 1 (1) | 11 | 186 |
| modules/gol_zones.hpp | 23 (6) | 0 (0) | 17 | 236 |
| modules/gol_zones.inl | 9 (1) | 1 (1) | 8 | 153 |
| modules/ground_architecture.hpp | 28 (5) | 0 (0) | 23 | 301 |
| modules/input.hpp | 15 (3) | 0 (0) | 12 | 193 |
| modules/input.inl | 16 (3) | 2 (2) | 13 | 318 |
| modules/mood.hpp | 34 (5) | 0 (0) | 29 | 340 |
| modules/mood.inl | 37 (9) | 3 (2) | 29 | 471 |
| modules/mood_constants.hpp | 21 (4) | 0 (0) | 17 | 243 |
| modules/orbs.hpp | 16 (4) | 0 (0) | 12 | 200 |
| modules/orbs.inl | 8 (2) | 1 (1) | 6 | 151 |
| modules/pawn.hpp | 15 (3) | 0 (0) | 12 | 200 |
| modules/pawn.inl | 5 (2) | 1 (1) | 3 | 111 |
| modules/render_passes.hpp | 9 (1) | 0 (0) | 8 | 109 |
| modules/render_passes.inl | 8 (1) | 1 (1) | 7 | 155 |
| modules/ribbon.hpp | 48 (7) | 0 (0) | 41 | 582 |
| modules/ribbon.inl | 13 (2) | 1 (1) | 11 | 202 |
| modules/seed_utils.hpp | 6 (2) | 0 (0) | 4 | 67 |
| modules/spawn_engine.inl | 12 (4) | 0 (0) | 8 | 113 |
| modules/spheres.hpp | 17 (5) | 0 (0) | 12 | 183 |
| renderer.hpp | 7 (1) | 0 (0) | 6 | 68 |
| roster.hpp | 24 (5) | 0 (0) | 19 | 316 |
| state.hpp | 7 (2) | 0 (0) | 5 | 69 |
| **TOTAL** | 827 (181) | 18 (14) | **650** | **9699** |

- **Block-pointer form (above): −650 lines / ≈ −9.7k tokens** — recovers
  ~55% of the campaign's comment-token growth, ~40% of its total token
  growth, with every block leaving a breadcrumb.
- **File-pointer form** (one `Converted: see audit/LADDER.md` line per
  FILE instead of per block): −811 lines / ≈ −11.3k tokens — recovers
  ~64% / ~47%. The delta between forms is small because narration
  blocks are many and short (181 blocks avg 4.6 lines); the file-pointer
  form is where the real compression lives on cartridge.hpp (114
  narration lines, 73 blocks — the graduation-note archipelago).

**The pre-MOD stratum, included as ordered:** `DONE[` tags at HEAD: 26
blocks / 120 lines (base had 25 — the campaign added ~1 and carried the
rest); `Scope B migration` mentions: 11 lines (all pre-MOD). Measured
DONE+Scope-B ≈ 36 tag sites against the order's remembered 33 — the
delta is bucket-edge (a few DONE tags ride inside SEAM blocks and were
counted LEDGER-side). These ~131 lines / ≈1.6k tokens are already
inside the NARRATION totals above, so ONE ruling covers both strata, as
intended.

## 5. WHAT THE NUMBERS SAY (no action taken)

1. Code growth is small and structural (+3.7% tokens for 14 more
   files, 11 of them the priced transitional pairs). The composition
   goal was not betrayed by code — the growth is textual.
2. The textual growth is concentrated exactly where the order guessed:
   conversion narration (which rung, whose ruling, what carried) that
   git and audit/LADDER.md already hold verbatim. cartridge.hpp,
   ribbon.hpp, mood.hpp/.inl, gallery.hpp, roster.hpp are the top
   holders.
3. Pair-banner DUPLICATION is nearly absent in the verbatim sense (18
   lines) — the practiced form already keeps public-surface boxes
   `.hpp`-side. The order's dedup target is thus mostly PARAPHRASE
   duplication (impl banners restating organ lists the header's
   Depends-on paragraph carries) — not measured by the verbatim recipe
   (limitation below), but it lives inside the impl banners' NARRATION/
   PRESENT blocks and the COMPACT-1 pair-banner rule ("the .inl keeps
   only the wrapping-form note") would collect it without needing a
   separate count.
4. LEDGER is 6% of comment tokens — cheap, and the order keeps it.

## 6. LIMITATIONS (disclosed)

- Bucket assignment is block-granular by opening line: a paragraph that
  MIXES provenance and identity (several impl banners open "Included
  AFTER the Cartridge class (LADDER-3 cN...)" then list the organs the
  impl reaches) classifies whole as NARRATION. COMPACT-1's pair-banner
  rule would split such paragraphs (keep the reaches-list, drop the
  rung cite), so the forecast slightly OVERSTATES narration savings on
  impl banners and UNDERSTATES the surviving present-behavior text —
  the two errors point in opposite directions on the same lines.
- The TWO-REGIMES note (cartridge.hpp file header) counts NARRATION by
  its markers but governs current structure until the last module
  converts; borderline — 5 lines either way.
- Paraphrase duplication is not detected (verbatim ≥25-char lines
  only). Trailing comments on code lines are counted in comment TOKENS
  but not bucketed (10.3k t — overwhelmingly per-constant law, the
  keep class).
- Token counts are regex-approximate (`\w+|[^\w\s]`), consistent across
  both ends; ratios and deltas are the reliable figures.

## 7. WHAT THIS FEEDS

Jean rules the sweep. The numbers support the shape the order sketched
as COMPACT-1: comment-only commits, code-token-identical (the
normalized compare from the rung recipe verifies code tokens byte-for-
byte), narration → pointers (−9.7k to −11.3k tokens), pair banners
deduplicated by the wrapping-form rule, F3 executed across both strata
(the 26 DONE blocks ride the same ruling), LEDGER untouched. Nothing is
cut until Jean stamps the classes.

*Recipe artifacts: the classifier script is reproducible from §2's
rules; run it against any head to re-derive every number in this
report.*
