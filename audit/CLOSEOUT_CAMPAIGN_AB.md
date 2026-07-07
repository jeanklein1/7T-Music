# CAMPAIGN A + B — CLOSE-OUT

Campaign **A (picker consolidation)** + **B (COMPAT retirement)** on
`the_board`, branch `FINAL_TOUCH`. Executed under the CADENCE-CHANGE
ruling: the closeout + campaign sequence landed as **separate commits,
one per stage, with no inter-stage build/gate** — Jean builds once at the
end. Mandatory dependency order was honoured:

> closeout S2 → S3 → S4 → **A1 → A2 → B1** → [musical.inl gate] → B2 → B3

Stages **A1, A2, B1 landed.** Stages **B2, B3 are GATED** and were not
started — `musical.inl` is absent from the board tree (see §B2). Every
anchor was re-verified against the live tree before editing; where the
tree disagreed with the handoff, the tree won and the difference is
recorded here as a finding (never forced).

## Commit stack (this campaign)

| Stage | Commit | Files | Net Δ |
|---|---|---|---|
| A1 — one picker, every domain | `d248a68` | seed_utils + agents + gol_zones + ribbon + gallery + orbs | +51 / −70 (−19) |
| A2 — retire `select_tier_biased` | `3ab2698` | cartridge.hpp + entity_pipeline + mood + ribbon + seed_utils | +11 / −25 (−14) |
| B1 — aura ramp real-time | `63a1742` | pawn.inl + cartridge_constitution.md | +14 / −12 (+2) |

(Prior batch, for context: `faf26c3` drift sweep, `2345b4c`/`b0b6ab0`/
`83254f3`/`9c073d1` closeout S1–S4.)

## Discipline gates (all landed stages)

- **D2 encoding** — PASS. Post-edit BOM/EOL re-checked against L0 on
  every touched file: BOM preserved on `entity_pipeline.inl`,
  `ribbon.inl`, `pawn.inl`, `agents.inl`, `orbs.inl`; no-BOM preserved on
  `seed_utils.inl`, `mood.inl`, `gol_zones.inl`, `gallery.inl`,
  `cartridge.hpp`, `cartridge_constitution.md`. Pure LF throughout.
- **D4 one-commit-per-stage** — PASS (three commits above).
- **Completeness net** (compiler substitute) — exhaustive grep for every
  retired/renamed symbol returned zero surviving references before each
  commit (evidence per stage below). Brace balance on `cartridge.hpp`
  re-counted after each deletion.

---

## STAGE A1 — ONE PICKER, EVERY DOMAIN  (`d248a68`)

**A1a.** `seed_utils.inl` — factored the canonical into the roll-taking
core `select_weighted(roll, weights, count)` plus the two-line seeded
wrapper `select_tier(seed, prop, weights, count) → select_weighted(
cpu_hash_f(seed, prop), weights, count)`. The `SEAM[seed_utils:Q10-target]`
block was rewritten from a *pending task* into the *landed consolidation
record*.

**A1b.** Nine call sites migrated (see finding F-A1.1 on the count).
DIRECT = weights already a flat array → single call; GATHER = weights in
struct fields / need normalization → gather into a local array
(**divide-then-accumulate order preserved so arithmetic is bit-identical**)
then one call.

| # | site | pattern | prop / count |
|---|---|---|---|
| 1 | agents.inl:547 behavior | GATHER (÷ beh_sum) | 1u / AGENT_BEHAVIOR_COUNT |
| 2 | agents.inl:558 tier | GATHER (÷ tier_sum) | 2u / AGENT_TIER_COUNT |
| 3 | gol_zones.inl:405 GOL tier | GATHER | GoLZoneProp::TIER / GOL_TIER_COUNT |
| 4 | gol_zones.inl:419 Pulse tier | GATHER | PulseZoneProp::PULSE_TIER / PULSE_TIER_COUNT |
| 5 | ribbon.inl:1106 color mode | DIRECT | RibbonProp::COLOR_ROLL / RibbonColorMode::COUNT |
| 6 | ribbon.inl:1168 checker pair | GATHER | RibbonProp::CHECKER_PAIR_ROLL / CHECKER_PAIR_COUNT |
| 7 | gallery.inl:1698 indoor scale | GATHER | WallPaintingProp::SCALE_ROLL / INDOOR_SCALE_COUNT |
| 8 | gallery.inl:1732 indoor scale (2nd copy) | GATHER | same |
| 9 | gallery.inl sample_shot_type | GATHER + `select_weighted(roll,…)` | uses the photographer's own `uniform(0,1)` stream, **not** cpu_hash_f |

**A1c.** Deliberately-distinct annotations added (these are NOT picks):
`orbs.inl` cumulative-table builder ("BUILDS the cumulative table the
shader rolls against; it picks nothing"). The gallery
`cumulative_distance` walk accumulator shares only the word — no
annotation needed.

### Declared-expectation results (A1)

- **Zero residual hand-rolled pick loops** — PASS. `grep 'cumul +=|cum +='`
  across agents / gol_zones / ribbon / gallery = 0 hits; `seed_utils`
  holds the only walk. The two annotated non-picks are not pick loops.
- **Bit-identical picks** — held at every site: same hash, same prop
  salt, same divide-then-accumulate order, same tier-struct lookup after
  the pick.

### A1 fallthrough-unification record (the ONE declared exception class)

The canonical returns `count - 1` on the float-epsilon miss (the roll
lands at/above the cumulative sum by ≤ 1 ulp). Per-site pre-loop defaults
before A1:

- gol / pulse / ribbon / gallery — already defaulted to `count - 1`
  → **identical**, no behavior change.
- agents (sites 1, 2) — defaulted to pre-loop values (behavior default,
  `AGENT_TIER_WORKER`). After A1 the epsilon miss returns `count - 1`
  (the last bucket) instead. **Unified by construction.** Probability is
  ~ulp-scale on a hash-derived roll; **accepted by ruling.** This is the
  only site class where a (vanishingly rare) output could differ.

---

## STAGE A2 — RETIRE `select_tier_biased`  (`3ab2698`)

Hard-depended on closeout Stage 3, which had already stripped
`select_tier_biased` to a base-weights pass-through to `select_tier`
(its family bias was zero-strength once the population-batch mode pinned
NEUTRAL). At that point its body *was* `select_tier` with a dead `family`
parameter and a stale `weights[8]` intermediate.

- **A2a/b** — three callers (see F-A2.1) retargeted to `select_tier`,
  dropping the vestigial 5th `family` argument:
  - `entity_pipeline.inl:215` generic_select
  - `mood.inl:608` apply_mood_anchor_ribbon
  - `ribbon.inl:1223` select_ribbon_for_patch (also dropped the `c->`
    qualifier — `select_tier` is a **static** member, callable unqualified;
    `select_tier_biased` had been a non-static `const` member, which is
    why the enclosing static helper reached it through `c->`).
- **A2c** — definition deleted from `cartridge.hpp` (comment block +
  function; `cartridge.hpp` brace balance 643 → 642, one balanced pair
  removed, consistent).
- **A2d** — comment sweep: dependency-list mentions in
  `entity_pipeline.inl:72` and `ribbon.inl:59`, plus the
  `seed_utils.inl` Q10 SEAM record, all updated to name `select_tier`
  (the SEAM was reworded so it no longer names the retired symbol).

### Declared-expectation results (A2)

- **`grep select_tier_biased` = 0** across the board tree — PASS.
- All retargeted callers verified to have exactly four arguments
  (seed, prop, weights, count) — no orphaned `family` / `PopFamily`.

### A2 L3 #95 retirement note

This stage **retires the audit's L3 #95 latent-OOB hazard by
construction.** The `weights[8]` fixed-size intermediate no longer exists;
the walk now runs over each caller's *actual* table at its *actual*
count. The class of bug (a caller with count > 8 indexing past the
intermediate) is structurally impossible now — there is no intermediate.

---

## STAGE B1 — AURA RAMP GOES HONESTLY REAL-TIME  (`63a1742`)

By ruling, `aura_presence` is a possession-toggle affordance, not a
musical gesture — it keeps real-time exponential semantics, and YAGNI
places the formula at its single call site (no helper, no trajectory.hpp
addition).

- **B1a/b** — the COMPAT overload's exponential step was lifted
  **verbatim** into `pawn.inl` `tick_pawn_couplings`, replacing the
  `Trajectory` struct ceremony (construct → `trajectory_release` → read
  `.value`):

  ```
  c->player_.aura_presence =
      prev + (target - prev) * (1.0f - std::exp(-rate * c->time_state_.dt));
  ```

  Same expression, same order of operations as
  `trajectory.hpp:104` with the substitutions
  `t.value→prev, goal→target, dt→c->time_state_.dt` ⇒ frame-identical.
- **B1c** — comments reconciled to present truth (see F-B1.2 on the
  extra two): header surface bullet (line 12), the removed COMPAT tag at
  the ramp, plus the now-falsified `Depends on:` line (35) and
  `DONE[pawn:K1]` prose (135).

### Declared-expectation results (B1)

- **`pawn.inl` no longer references `Trajectory` or the dt overload** —
  PASS (`grep 'Trajectory|trajectory_release'` in pawn.inl = 0; the two
  surviving lowercase "trajectory" hits are the generic concept word and
  the truthful "lifted inline from the former coupling/trajectory.hpp"
  provenance note).
- **Arithmetic verbatim ⇒ frame-identical aura behavior** — held.

---

## STAGES B2 / B3 — GATED, NOT STARTED

- **B2 (musical.inl onto the beat clock)** — **BLOCKED.** `musical.inl`
  is absent from the board tree; it exists only under
  `src/cartridges/backup_board/modules/musical.inl`. Per the gate ruling
  ("run 1–6 through B1, commit, and STOP — B2/B3 wait for the file; do
  not guess at musical.inl's contents or migrate blind"), no census was
  taken and no caller was migrated. **The B2 per-caller census table
  (caller / driven value / MIGRATED-or-STOPPED-nonmusical / span shipped)
  must be produced when the file is supplied** — it cannot be authored
  against an absent file.
- **B3 (delete COMPAT, converge the twins)** — **BLOCKED** on B2. The
  COMPAT section in `coupling/trajectory.hpp` (the `Trajectory` struct +
  the dt/rate `trajectory_release` overload + `<cmath>`) is **left
  intact**; it now has **no board consumer** after B1 but cannot be
  deleted until B2 confirms no musical follower needs it. Constitution §5
  COMPAT was decremented to *one file* by B1 (below); B3 takes it to
  zero.

---

## FINDINGS (tree vs. handoff — recorded, not forced)

- **F-A1.1** — the A1b prose says "eight sites" but the list enumerates
  **nine** (sites 1–9; the WHY block likewise names nine copies, gallery
  ×2). All nine were migrated. Count discrepancy in the handoff, not the
  tree.
- **F-A2.1** — the A2 WHY says "Two callers exist"; the live tree has
  **three** — the handoff missed `mood.inl:608`
  (`apply_mood_anchor_ribbon`). All three retargeted; `grep = 0`
  confirms none were left behind.
- **F-A2.2** — A2b anchored the ribbon caller at ~:1232; live line was
  **1223**. Text anchor governed.
- **F-B1.1 (scope deviation, with rationale)** — B1's stage header scopes
  it to "file: pawn.inl", and the campaign reserves census work for B3b
  ("COMPAT class goes to zero"). But B1 removes the COMPAT tag that
  Constitution §5 explicitly asserts exists at the ramp
  ("COMPAT-tagged at the ramp"), and §5 states it "updates same-commit
  with any change to its entries" (and §-preamble: CC audits every §5
  claim against the tree). Leaving §5 unedited would make it fail
  ratification the instant B1 landed. **Resolution:** §5 COMPAT was
  decremented in the B1 commit from *two files* to *one file*
  (trajectory.hpp's overload, now with no board consumer). This does not
  usurp B3b — B3b takes the class from one file to **zero**; B1 corrected
  only the pawn.inl half it falsified. A monotonic, ratifiable decrement.
- **F-B1.2** — B1c enumerates two comments to update (header bullet;
  the ramp COMPAT note). Two further references were also falsified by
  the inline and updated for truth: `pawn.inl:35` `Depends on:` (no
  longer depends on trajectory.hpp's release primitive) and
  `pawn.inl:135` `DONE[pawn:K1]` ("uses trajectory_release" → "uses an
  inlined real-time exponential step"). Not census-tracked; corrected to
  avoid leaving fresh drift.

## FOUND-NOT-FIXED

None among the landed stages A1/A2/B1. The only outstanding work is
B2/B3, which are **gated** (blocked on `musical.inl`), not deferred by
choice.
