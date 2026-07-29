> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# COMPACT-2 — THE PROTOTYPE COMMENT LAW (close-out)

One sweep, as stamped. Per-file structural prose is retired until the
prototype ships; documentation returns at certification as §6
datasheets against the settled structure. Git keeps every word.
**audit/ was untouched by the sweep and is now the sole map.**

## The one number (the extended totals table)

Scope: the 37 the_board source files (.hpp/.inl). Tokenizer and
recipes: COMPACT-0's, unchanged throughout the series.

| | pre-MOD (3aac3db) | pre-C1 (98dc748) | post-C1 (ca0c483) | **post-C2** |
|---|---|---|---|---|
| files | 20 | 34 | 34 | 37 |
| code tokens | 170,867 | 177,256 | 177,380 | 177,895 |
| comment tokens | 97,306 | 115,024 | 110,052 | **65,608** |
| comment lines | 6,524 | 7,671 | 7,294 | **3,514** |

**The prototype's true reading weight: 65,608 comment tokens** — 42%
below where COMPACT-1 left it, 33% below the pre-MOD baseline, with
the entire MOD campaign's structure in place.

## What was cut, what survived

**Cut (−47,061 tokens / −3,977 comment lines):** overview essays,
family tables, public-surface boxes (content + frames, all of them),
field-role paragraphs, multi-line section narration — the PROSE class
in full; the K2 include-restating clauses (the arrow-law refinement:
the include list is the requirements face, compiler-checked); the
LADDER-5 arc citations, present-ized; the box-frame rows and the
separator collapse.

**Survived (the four keep-classes):** K1 one identity sentence per
file; K2 the reaches-lists (split per the rider — 10 headers keep
their "The impl reaches…" tails, 5 pure-restatements collapsed
whole); K3 the ledger untouched (7,541 tokens of SEAM/STATUS/INTENT/
NOTE/pins, machine-checked); K4 code-adjacent one-liners at D2's
two-line width; section-rule navigation; one provenance pointer per
file (theory references now name v2 — the one v1 citation re-pointed).

**Rescues (D1 + D3, disclosed):** 134 constraint-flagged paragraphs
were held from the engine cut and hand-ruled. The narrative ones were
rewritten to K4 form in place (token flow, tempo follower HELD-LAST,
TWO REGIMES, ORGANS-ARE-PUBLIC, THE FIRST EDGE, MATURITY DIAL, the
Phase-3 cube-count note — each keeps its constraint, loses its essay).
The pin-class ones were RATIFIED AS KEEPS under D3's rescue bias:
state.hpp's ~50 byte-layout/binding/WGSL-mirror pins and
renderer.hpp's pipeline-geometry notes, plus the params[]-order and
count-lockstep pins across modules — these are the K4-named classes
(WGSL-mirror per-function pins, governing-law-beside-constant), kept
whole. Audit harder, not document longer.

**Forecast vs delivered:** stamped ≈−55.3k tokens; delivered −47.1k.
The ≈8k difference is D3's retention, itemized above — the stamp
resolved the GPU contract surface toward keeping its pins, and the
pin-class rescues across modules were ratified as keeps rather than
compressed. Every retained paragraph is a constraint carrier; zero
structural prose survives.

## The amendment (same commit)

Constitution §1 gains THE PROTOTYPE REGIME clause (Jean's ruling,
2026-07-12): per-file documentation is the four keep-classes; the §6
datasheet standard is DORMANT, not violated, reactivating as a §7
certification requirement at ship. Entity contract §6 carries the
matching note; the §5 ledger records the ruling with the date.

## Gates at close

CODE-TOKEN-IDENTICAL per file against the pre-sweep head (37/37
green — 177,895 code tokens byte-stable); encodings match HEAD per
file (BOM set: the impl quartet + renderer.hpp, as found at HEAD —
the fifth BOM carrier is now on the record); zone census 13 == 13.
Recipes: scratchpad compact2_sweep.py + the census; the engine's cut
listing preserved the full paragraph inventory for the record.
