# SIBLING PRUNE — RECON (read-only; report + STOP; Jean rules which die)

Scope: the CMake target census, the_chord, backup_board, + the level-2 keyhole
rider. Three scope-readers + the rider, verified at `56d0507`. **Nothing moved.**
The prune CUT waits for the ruling on this report.

---

## §1 THE TARGET CENSUS (what CMake builds; which root is live)

| target | root cpp | render cartridge | invoked by |
|---|---|---|---|
| **incubator_dual** | incubator_dual.cpp | **the_board** (L261) | **THE LIVE TARGET** — CMakePresets `the-board-full` / `the-board-minimal` both build it |
| incubator | incubator.cpp | the_chord (L258) | ONE live wiring: CMakePresets `the-chord` (configure L34-40 + build L45) |
| incubator_backup | incubator_dual.cpp (same root as dual) | backup_board (L267) | no preset; hand-built only — **and currently BROKEN** (§3) |
| the_lab | the_lab.cpp | none (analysis-only ImGui/ImPlot sandbox) | its own target |
| 7t_visualizer | main.cpp | all (GLOB) | **NOT built by default** — gated `if(NOT INCUBATOR_ONLY)`, INCUBATOR_ONLY defaults ON |
| probe_canvas + 3 check_* | canvas_1 roots | none | headless assert tests |

- The old `incubator` target is invoked by NOTHING except the `the-chord` preset
  pair in CMakePresets.json. Everything else tree-wide is prose/source-comment
  references (LADDER, recon reports, old docs), not invocations. No .vscode, no
  CI, no Makefiles, no scripts reference it. glaw1 compiles its own TU; the score
  census is a pure-Python grep — neither invokes any built target (confirmed).
- Defines only apply under `if(MSVC)` — the live Windows/Dawn build.
- FLAG (doc-only): BINDING_REGISTRY_RECON.md:256 says "boots the_board
  (incubator)" — loose; the the_board target is incubator_dual.
- FLAG (asymmetry): incubator_backup does NOT receive INCUBATE_DEMO despite
  sharing incubator_dual.cpp as root — deliberate-looking; confirm before prune.

## §2 THE_CHORD (what references it; is its purpose covered?)

REFERENCES: CMakeLists L246/258/499-503 + CMakePresets `the-chord` (L34-40, 45) +
incubator.cpp (default `#define INCUBATE_RENDER the_chord`, include at L68) +
the_board/bodies/ribbon.hpp:20-28 (the MIRROR BANNER: pairing SUSPENDED at
LADDER-3 c5, "subject to the mirror law's spirit until the pairing is
[reconciled]") + cartridge_constitution.md:9,156 + ~15 audit/ mentions
(COMPAT_CONSUMER_CENSUS names it the LAST LIVE COMPAT consumer —
the_chord/modules/pawn.inl:149-151, cartridge.hpp:90 — with a deferred
"reconciliation pass"; CLOSEOUT_CAMPAIGN_AB, SWEEP_CLOSEOUT concur).

WHAT IT IS: a complete standalone render cartridge (21 files, ~35,612 lines) —
"the lab" to the_board's "the exhibit" (constitution: two instances of one
anatomy). "PAIRING" here = the cross-cartridge MIRROR LAW (byte-identical
mirrored-module deltas: ribbon, pawn, world.wgsl), NOT a test suite — no
discrete "pairing test" artifacts exist anywhere in its tree.

**COVERAGE VERDICT: NOT-COVERED.** The demo-matrix/roster is orthogonal:
matrix.hpp/demo.hpp/roster.hpp contain ZERO references to the_chord, mirror,
pairing, or coupling. The matrix's golden asserts pin the_board's own
full/minimal piece-rosters — a within-cartridge property. Nothing anywhere
asserts cross-cartridge byte-identity, and no document claims the_chord's
function migrated. Its two roles (the couplings-lab second cartridge; the
mirror-law counterpart) either matter — or are formally retired by ruling. The
audit trail currently treats it as DEFERRED, not superseded.

## §3 BACKUP_BOARD (keep-cost vs delete-cost; git history is the real backup)

KEEP: 22 files / 38,232 lines / 1.9M. Whole dedicated CMake block (L246-293
comments+vars, L637-700 target). 7 audit/*.md prose mentions, zero LADDER
mentions. Friction: score/run.py is pinned to the_board to avoid it; every
tree-wide grep double-hits unless siblings are filtered (this campaign's recons
all carry the "NOT readers" disclaimer). Near-frozen: 2 commits ever — a129960
(promotion: namespace rename, ~37 lines) and 4b55d5e (6-line no-op stub,
last touch).

DELETE: removes the incubator_backup target block (the only build consumer) —
**a target that does not currently build anyway**: backup_board/cartridge.hpp:93
still includes `musical/trajectory.hpp`, a path that no longer exists (noted in
4b55d5e's own message). Nothing else in src consumes it. RECOVERABILITY: full —
created at 837d920 ("verbatim frozen snapshot"), moved at a129960; the only two
divergences from a plain historical the_board checkout (the namespace rename +
the 6-line stub) are both in history. Deleting it loses nothing git doesn't hold.

## §4 LEVEL-2 KEYHOLE RIDER — CLEAN, nothing joins T0

All 18 includers of contracts/keyhole.hpp genuinely use it: 16 name both
`Cartridge` and `wgpu::Queue`; spawn_services.hpp + patch_system.hpp name only
`wgpu::Queue` — exactly matching their "insurance form" banner comments. Zero
stale requirements-face lines.

## §5 THE DECISION SURFACE (report only — Jean rules)

| candidate | what dies | what it costs | what protects it |
|---|---|---|---|
| **backup_board** | 22 files/38k lines + the broken incubator_backup target + 2 cache vars | nothing functional (target already doesn't build); full git recovery from 837d920/a129960/4b55d5e | only its A/B-baseline intent — already superseded by history? |
| **the_chord** | 21 files/~36k lines + the incubator target's default + the `the-chord` preset | the LAST second-cartridge instance: the mirror law loses its counterpart; the COMPAT ramp loses its last live consumer (its deferred reconciliation pass dies with it); incubator.cpp needs a new default or dies too | ribbon.hpp's banner says pairing SUSPENDED, not retired — retiring it is a CONSTITUTION amendment, not a delete |
| **incubator (target)** | the target + the `the-chord` preset pair (+ possibly incubator.cpp) | nothing else invokes it; falls naturally IF the_chord falls | it is the_chord's only harness |

Couplings: the_chord ⇒ incubator target ⇒ the `the-chord` preset (one ruling
covers all three). backup_board is independent. Whichever dies, the audit prose
mentions stay (historical record); CMake comments L246-250 need a trim either way.

## §6 DISCIPLINE
Read-only. `git status` clean but for this file. The prune cut — if ruled —
is T-minus-1 class (whole-tree `git rm` + a CMake block; no re-index, no blast
radius inside the_board) but is a SIBLING delete, not a the_board residue cut:
it waits for the explicit stamp, per the handoff.
