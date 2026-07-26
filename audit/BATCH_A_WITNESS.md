# [G:witness] — the bit-identity witness for BATCH A

Defined by CC, **run by Jean**. Batch A @ `d5ea36e`, base `a9fe3aa`.

The riskiest edit in the batch is [A1], the `run_gate` collapse: nine families'
gate path. If it is not bit-identical, every world silently changes and no
compiler says so. This is the experiment that would catch it.

---

## 0. FIRST — PROVE THE WITNESS. Do not skip this.

**Two boots at the same seed, with NO Batch A in between, must produce the same
twelve integers.** Run this on `a9fe3aa` (pre-batch) twice.

- **Same twice** → the witness holds; proceed to §2.
- **Different** → boot spawning is not deterministic. That is a **FINDING in
  its own right**, it invalidates this gate, and [A1] then needs a different
  proof. Report it and stop; do not read a difference in §2 as evidence about
  the collapse.

The check costs one extra boot and is the difference between a gate and a
coincidence.

---

## 1. THE PROCEDURE — identical on both sides

1. **Fixed seed.** Same world seed both runs. Note which.
2. **Boot. Do not move.** No WASD, no mouse-look, no mood change, no possession,
   no console. The camera must sit exactly where boot leaves it — the census's
   `active` column is a function of what streamed in, and streaming is
   distance-driven off `player_.readback_x/z`.
3. **Wait 60 seconds** of wall clock. At rest the population is static (proven
   indoors over 300s with zero arrivals), so by 60s the patch ring around the
   spawn point has settled.
4. **Capture the `trigger=periodic` census** that fires at/after 60s. Not boot,
   not mood-transition — `periodic`, because boot fires before streaming has
   run and would read all zeros in both builds, proving nothing.
5. Record the **`active` column only** — twelve integers, in PopFamily row
   order as printed.

Repeat identically on `d5ea36e`.

---

## 2. THE COMPARISON — twelve integers, this shape

Copy the `active` column straight down. Nothing else in the print is part of
this gate.

| # | fam | active BEFORE (`a9fe3aa`) | active AFTER (`d5ea36e`) | match |
|---|------|---|---|---|
| 0 | pyr  |  |  | |
| 1 | arch |  |  | |
| 2 | col  |  |  | |
| 3 | ant  |  |  | |
| 4 | palm |  |  | |
| 5 | cact |  |  | |
| 6 | blad |  |  | |
| 7 | sph  |  |  | |
| 8 | ribn |  |  | |
| 9 | cube |  |  | |
| 10 | gol  |  |  | |
| 11 | gall |  |  | |
|  | **TOTAL** |  |  | |

**All twelve identical ⇒ the gate math did not move.**

### Why `active` and not `claimed`

`active` is the family's own array, scanned — it is downstream of the gate, the
tier roll, placement and commit, so it moves if any of them move. `claimed` is
the footprint registry, which SPAWN_2 deliberately changed; it is not a
constant across this comparison and is not part of this gate.

### What a mismatch means, by shape

| pattern | reading |
|---|---|
| **one family differs** | that family's traits row disagrees with what its old hand-written gate passed. Should be impossible — F-5's 45 static_asserts pin exactly that, per family — so this would mean the assert set has a hole. Name the family. |
| **several differ** | something shared moved: the composition law, the seed domains, or `PLACEMENT_ORDER` not actually being identity. |
| **all twelve differ** | suspect the experiment, not the code — different seed, or the camera moved. |
| **only `sph`/`cube` differ** | not [A1]. Look at [A2]'s positional cut on `SPHERE_TRAITS`/`CUBE_TRAITS`, though F-5 pins `grounded` on both. |

---

## 3. WHAT IS ALREADY MACHINE-PROVEN, so the witness need not re-check it

[A1] and [A2] carry 72 compile-time asserts (F-5), GREEN under `glaw1`:

- per family, that the traits row holds exactly what the hand-written gate
  passed — `family_id`, `max_instances`, `grounded`, `spawn_roll_prop`,
  `spawn_chance`, `color_part_count`, and `mood_multiplier` by pointer identity;
- that the five-field positional cut did not slide any initialiser: every
  removal point has a pinned field immediately after it, plus a `color_parts`
  pointer-identity check on the final field.

[A5] carries F-6, a constexpr permutation check on `PLACEMENT_ORDER`.

So the witness is testing what static analysis cannot reach: that the composed
float arithmetic and the seeded rolls land on the same side of the same
thresholds. That is the whole residual risk.

---

## 4. THE OTHER GATES IN THIS BATCH

| gate | owner | what |
|---|---|---|
| `[G:glaw1]` | CC | **GREEN at all seven commits**, individually |
| `[G:witness]` | Jean | this document |
| `[G:shader]` | Jean | **[A4] only** — it touches `world.wgsl` (field rename + two write sites). Stride, layout and the 48-byte assert are unchanged, so risk is nil, but it needs a recompile |
| `[G:visual]` | Jean | nothing in Batch A should move a pixel. If one does, something here was not what it claimed |

`[G:runtime]` is not claimed for this batch — there is no behaviour to measure.
