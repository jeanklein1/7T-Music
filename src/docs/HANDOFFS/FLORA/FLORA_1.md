# FLORA_1 — palm proportions, flora geometry audit, arm repair

Revision 4. This file REPLACES all earlier revisions in full. There is no
addendum and no second document for this campaign. If a second FLORA_1 file
exists anywhere, it is stale — delete it.

Changed in rev 4: the campaign moves to a held branch. Master is under
concurrent work; nothing here goes to master until Jean authorizes.
Changed in rev 3: F6 and F7 promoted from HELD to executable (slot budget
caps). F8 restated — the bark groove term is dead, see F1 item 12.

## Git law — BRANCH EXCEPTION IN FORCE

Standing law is trunk-based on master. This campaign invokes the land-gated
exception: the work sits on a held branch until Jean authorizes the merge,
then the branch dies. Master is under concurrent work right now; that is
the whole reason.

**Branch: `claude/flora-1`**, cut from master at the head CC sees after
`git fetch --unshallow`. The namespace follows the existing exception,
which names `claude/*`; the campaign is the suffix. Overrule to a bare
`flora-1` if preferred.

CC starts shallow — run `git fetch --unshallow` before any ancestry claim.

CC does NOT merge and does NOT write to master. No `git merge`, no
`git push origin HEAD:master`, no fast-forward, no cherry-pick onto master.
Push the branch and stop. The merge is Jean's, and only Jean's.

CC cannot delete the branch — the proxy returns 403 on branch deletion, as
it does on `refs/tags`. Jean kills the branch from the design machine after
the merge lands.

Everything in this campaign goes on the branch, F1's ledger included. One
campaign, one branch, one home for its state. A ledger on master while the
code sits on a branch is two homes for one fact.

Separate commits per numbered unit, for bisection. Unchanged.

### Rebase discipline

Master moves under this branch. Rebase onto master — never merge master
into the branch — so the per-unit commits stay linear and bisectable.

**After ANY rebase, every verbatim anchor in F0 and F2–F7 is stale until
re-verified.** Re-check each FIND block character-for-character against the
rebased file before continuing. STOP and report on mismatch. Do not
reconstruct an anchor from surrounding context; an anchor that has to be
re-derived is not an anchor.

On a rebase CONFLICT: stop. Report the conflicting hunks verbatim and name
which units they touch. Do not resolve by improvisation. A conflict in
`world.wgsl` is the expected one — F2 through F7 all live in that file, and
concurrent master work is most likely there too.

Report the branch's rebase state on every push: base commit, master head,
units landed.

## Register

REPORT findings. Never improvise a fix. Verify every named anchor verbatim
before editing; STOP and report on mismatch. F1 is read-only. F0 and F2–F7
are edits, all of them on the branch. F8–F10 are HELD — do not touch them.

"Commit alone" below means one commit per unit ON THE BRANCH. It does not
mean push to master. Nothing in this campaign reaches master without Jean.

Encoding: LF-only, no BOM.

## Order

F0 lands first. It changes palm proportions, so every later visual gate is
read against the new bodies rather than a moving target. F0 also has the
smallest conflict surface in the campaign — `grounded.hpp` and `src/tools/`
only, no `world.wgsl` — so it is the unit most likely to survive a rebase
untouched and the one worth gating early. Every other edit unit, F2 through
F7, is in `world.wgsl`, which is where the concurrent master work most
likely is. A held branch gets more expensive the longer it is held; the
order below is also the order of decreasing rebase safety.

F1 is read-only and can run at any point; run it early, since F6 and F7
cannot be written without it.

F6 depends on F1 item 3 (the zero-fill gate) and must not land before it.
F7a depends on F5 — the arm cap block F5 deletes is part of the per-arm
cost F7a must compute. Land F5 first, then derive F7a against the result.

---

## F0 — Palm height and frond length (grounded.hpp)

Heights reduced 10% across all three tiers. Frond lengths reduced 15% on
SAPLING and COASTAL, 20% on ROYAL. Each sigma scaled by the same factor as
its mu, preserving the coefficient of variation.

Neither parameter affects vertex count. This unit does not change any
budget. `burial` is consumed as an absolute wu offset (`y = t * p.height -
burial`), not a fraction, so sink depth is unchanged.

File: `src/cartridges/the_board/bodies/grounded.hpp`, table `PALM_TIERS`.

FIND (expect exactly 1 occurrence). Verify character-for-character
including leading whitespace. STOP on mismatch:

```
        { 0.45f, 0.0f, { {28.0f, 2.0f}, {0.55f, 0.06f}, {0.28f, 0.03f}, {0.08f, 0.04f}, {0.0f, 0.0f},
```

REPLACE — change only the first pair:

```
        { 0.45f, 0.0f, { {25.2f, 1.8f}, {0.55f, 0.06f}, {0.28f, 0.03f}, {0.08f, 0.04f}, {0.0f, 0.0f},
```

FIND (expect exactly 1 occurrence):

```
                   {12.0f, 3.0f}, {0.04f, 0.01f}, {20.0f, 3.0f},  {8.0f, 1.0f},   {0.80f, 0.15f},
```

REPLACE — change only the FROND_LEN pair (fourth on the line):

```
                   {12.0f, 3.0f}, {0.04f, 0.01f}, {20.0f, 3.0f},  {6.8f, 0.85f},  {0.80f, 0.15f},
```

FIND (expect exactly 1 occurrence):

```
        { 0.35f, 0.0f, { {32.0f, 4.0f}, {0.55f, 0.08f}, {0.28f, 0.04f}, {0.14f, 0.06f}, {0.0f, 0.0f},
```

REPLACE:

```
        { 0.35f, 0.0f, { {28.8f, 3.6f}, {0.55f, 0.08f}, {0.28f, 0.04f}, {0.14f, 0.06f}, {0.0f, 0.0f},
```

FIND (expect exactly 1 occurrence):

```
                   {20.0f, 4.0f}, {0.06f, 0.01f}, {24.0f, 5.0f},  {8.0f, 1.50f},  {1.00f, 0.20f},
```

REPLACE:

```
                   {20.0f, 4.0f}, {0.06f, 0.01f}, {24.0f, 5.0f},  {6.8f, 1.28f},  {1.00f, 0.20f},
```

FIND (expect exactly 1 occurrence):

```
        { 0.20f, 0.0f, { {42.0f, 8.0f}, {0.90f, 0.12f}, {0.40f, 0.06f}, {0.06f, 0.03f}, {0.0f, 0.0f},
```

REPLACE:

```
        { 0.20f, 0.0f, { {37.8f, 7.2f}, {0.90f, 0.12f}, {0.40f, 0.06f}, {0.06f, 0.03f}, {0.0f, 0.0f},
```

FIND (expect exactly 1 occurrence):

```
                   {30.0f, 5.0f}, {0.08f, 0.02f}, {30.0f, 7.0f},  {11.0f, 2.50f}, {1.40f, 0.35f},
```

REPLACE:

```
                   {30.0f, 5.0f}, {0.08f, 0.02f}, {30.0f, 7.0f},  {8.8f, 2.0f},   {1.40f, 0.35f},
```

Column alignment is cosmetic; the values are the contract. Six edits, six
pairs changed, nothing else. After editing, re-verify arity: 16 pairs and 6
trailing scalars on every row. Report the full three-row table back.

Commit alone. Message names the change as a proportion pass, not a tweak —
every palm body ever born changes.

---

## F0b — Palm designer defaults refreshed (src/tools/7t_palm_designer.jsx)

The designer's stored defaults are stale against every landed value:
height 13/22/42 vs 25.2/28.8/37.8, frond_count 6/10/16 vs 20/24/30,
frond_len 5/8/12 vs 6.8/6.8/8.8, and so on down all three tiers. Reopening
the designer today shows a world that does not exist.

Rewrite `defaultTier`'s `D` array so every field equals the landed
`PALM_TIERS` value after F0. Map by NAME, not position. Delete the
`solid_h` and `solid_h_s` keys entirely — `palm_compute_solid_half`
computes a radius (`base_r + pad + blend`) and has no height concept;
`PalmProp` has no such member. Nothing is held.

Before editing, report the current `D` array in full and the landed table
side by side, one line per differing field. STOP if any landed value has no
named counterpart in the designer.

The JSX is a sketch program — generated C++ text is not a live call site.
No build gate applies.

Commit alone.

---

## F0c — Palm designer emits the tree's row shape

After F0b the designer still emits a row the tree cannot take verbatim:
`weight` last instead of first, no `LEAN_DIR` placeholder. Every future
paste needs a hand mapping, and a hand mapping is where silent corruption
lives — the F-4 arity assert counts, it does not order.

In `genCpp`:

1. Move `T.weight` and `T.color_var` to the FRONT of the emitted row, in
   that order, matching `TierProfile { weight, color_var, pairs[] }`.
   Note: the palm designer currently has no `color_var` field — palm's
   landed `color_var` is `0.0f` on all three rows. Emit the literal `0.0f`
   and report this; do not invent a UI control for it.
2. Emit `{0.0f, 0.0f}` between `lean` and `bark_rings`, at `PalmIdx::
   LEAN_DIR [4]`. Annotate it in the emitted header comment as the
   UNIFORM_TAU placeholder.
3. Remove `T.solid_h, T.solid_h_s` from the `vals` array and `sH_μ σ` from
   the emitted header comment string.
4. Update the emitted type name from `PalmTierParams` to `PalmTierRow`.

Result: the exported text is pasteable into `PALM_TIERS` unchanged.
Verify by round-tripping the landed table through the new exporter and
diffing against `grounded.hpp`. Report the diff. It must be empty.

Do NOT apply the same treatment to the cactus or blade designers in this
unit. Report whether they need it; hold the edit.

Commit alone.

---

## F1 — RECON (read-only, report, zero edits)

Report each item as a verbatim quote plus file and line. Do not interpret.

1. `src/cartridges/the_board/realization/world.wgsl`, `palm_mesh_gen`:
   quote the line defining `n_fronds`. Report the literal clamp value.
2. Same function: report the literal values of `PALMG_MAX_VERTS_PER_SLOT`,
   `PALMG_MAX_INDICES_PER_SLOT`, and every `min(...)` clamp applied to
   `trunk_rings`, `trunk_segs`, `frond_segs`.
3. Same function: report whether ANY bounds guard exists on `vi` or `ii`
   before a write. Quote it, or state "none".
4. Compute and report, for each of SAPLING / COASTAL / ROYAL, using the
   post-F0 `PALM_TIERS` mu values and the clamps found above:
       verts = (trunk_rings + 1) * trunk_segs + 1 + trunk_segs
             + n_fronds * (frond_segs + 1) * 2
   For each tier report: cost at mu bark_rings with the AUTHORED frond
   count; cost with the clamp applied; the bark_rings value at which the
   tier crosses 1200; and the maximum frond count the tier can afford both
   at mu bark_rings and at the 40-ring hard clamp.
   Expected shape of the answer, to be confirmed or refuted:
   Sapling ~529 with headroom for ~57 fronds; Coastal ~881, safe at 24
   until bark_rings ~39; Royal ~1421, OVER BUDGET at mu, affording ~21 at
   mu and ~13 at the ring clamp. If your arithmetic disagrees with any of
   these, STOP and report the disagreement before proceeding.
5. `world.wgsl`, `cactus_mesh_gen`, arm loop: quote the full
   `if (abs(ndy) > 0.95)` block and the `cactusg_write_vertex` call inside
   the ring loop. Report the literal normal arguments passed.
6. Same loop: quote the `fork_x` / `fork_z` assignment and the trunk's
   `lean_mag` / `lx` / `lz` assignment. Report both lean coefficients.
7. Same function: quote the arm cap block from `let arm_cap_r` through the
   closing of the cap index loop.
8. `grounded.hpp`: quote the full `PalmProp` struct. Report whether any
   member equals `931u`. Report the lowest and highest property index in
   the palm band.
9. Report `Dim::MAX_BLADE_INSTANCES`, `Dim::MAX_PALM_INSTANCES`,
   `Dim::MAX_CACTUS_INSTANCES`, and the matching `*_MAX_SLOTS` constants in
   `world.wgsl`. State whether each CPU/GPU pair agrees.
10. Report every site that multiplies blade spawn probability. Start from
    `BladeClusterConfig::SPAWN_CHANCE` and follow `compose_spawn_chance`.
    Name each site, its file, and its current literal value for BLADE.
11. `7t_cactus_designer.jsx` and `7t_blade_cluster_designer.jsx`: report
    stored default tier values diffed against the landed tables, one line
    per differing parameter. (Palm is handled by F0b.)
12. `palm_mesh_gen`, trunk loop: quote the `t`, `trunk_rings`, `ring_phase`
    and `r +=` lines together. Then evaluate `sin(ring_phase)` for
    ring = 0..trunk_rings at bark_rings = 12, 20, 30, and report the
    results. Claim to confirm or refute: because `bark_rings` is
    `do_round` and `trunk_rings == clamp(bark_rings, 8, 40)`, the
    expression reduces to `sin(ring * 2 * PI)`, which is zero at every
    ring, so `bark_depth` contributes nothing except when a clamp bites
    (bark_rings < 8 or > 40). If your evaluation disagrees, STOP and
    report before F6 is touched.
13. `cactus_mesh_gen` and `blade_cluster_mesh_gen`: report the exact
    per-arm and per-blade vertex and index cost as expressions in the
    variables in scope, and the exact trunk/base cost including every cap
    and top-ring block. These are the inputs to F7 and must come from
    reading, not from arithmetic on this document.

Commit: one markdown at `src/docs/FLORA_1_LEDGER.md`. No code touched.

---

## F2 — Cactus arm ring basis (world.wgsl)

The arm path lies entirely in the vertical plane spanned by
`out = (cos az, 0, sin az)` and world up. A single horizontal vector
perpendicular to that plane is valid for every ring. The runtime branch is
therefore unnecessary, and it flips the reference axis mid-arm whenever
`blend` exceeds 0.7526 — which `arm_curve` mu = 1.00 now guarantees at
t ~ 0.75. The old designer defaults (0.60/0.70/0.75) peaked at ndy = 0.949,
one hundredth below the branch.

Deleting the branch REMOVES runtime branching from the mesh-gen chain.
Confirm against the `world.wgsl` banner FXC block before committing.

FIND (expect exactly 1 occurrence):

```
            var rx: f32; var rz: f32;
            if (abs(ndy) > 0.95) {
                rx = 1.0; rz = 0.0;
            } else {
                rx = ndz; rz = -ndx;
```

Report the remainder of the `else` branch verbatim before editing. STOP if
the block does not match character-for-character.

REPLACE with a single unconditional assignment derived from the arm
azimuth. `out_x` and `out_z` are already in scope and already unit:

```
            let rx = out_z;
            let rz = -out_x;
```

Do not re-normalise. Report the diff before pushing.

Commit alone.

---

## F3 — Cactus arm normals (world.wgsl)

The ring vertex writes `(ca, 0.0, sa)` as its world normal. That is the
ring's local parameter, not a world direction. `r` and `f` are orthonormal
by construction (`f = r x nd`, both unit), so `r*ca + f*sa` is unit and
needs no normalisation.

FIND (expect exactly 1 occurrence, inside the arm ring loop):

```
                cactusg_write_vertex(vb_base + vi,
                    vx, vy, vz, ca, 0.0, sa, cr, cg, cb, slot);
```

REPLACE the three normal arguments with the transformed normal:
`rx * ca + fx * sa`, `fy * sa`, `rz * ca + fz * sa`.

Before editing: report the trunk's own `cactusg_write_vertex` call and
which convention it uses. If the trunk also writes an untransformed normal,
STOP and report — the scope is larger than this unit.

Commit alone.

---

## F4 — Cactus arm fork lean (world.wgsl)

The trunk centre at height `fork_y` is displaced by the full
`p.lean * p.height * fork_frac * fork_frac`. The arm fork applies `* 0.3`
of it. The designer's own 2D preview applies the full offset. Engine and
designer disagree; the designer is the shape authority.

FIND (expect exactly 1 occurrence each):

```
        let fork_x = cx + cos(arm_az) * p.radius * p.taper * 0.9 + lean_at_fork * lean_cos * 0.3;
        let fork_z = cz + sin(arm_az) * p.radius * p.taper * 0.9 + lean_at_fork * lean_sin * 0.3;
```

REPLACE: drop `* 0.3` from both.

Do NOT touch `p.radius * p.taper * 0.9`. That term uses the trunk's TOP
radius rather than the radius at `fork_frac` — a separate defect, currently
forgiving (the fork sits inset, not proud), and shared by engine and
designer alike. Report it in the ledger, change nothing.

Commit alone. Visual gate: FINGER cacti at high lean rolls, where the error
is largest relative to trunk radius and where arms are newly present.

---

## F5 — Cactus arm tip closure (world.wgsl)

The cap emits its own ring at `arm_r * 0.6` while the last body ring sits
at `arm_r * 0.7` with rib modulation. Two concentric unstitched rings at
the same height leave an open annulus.

Do not adjust the radius. Delete the separate cap ring and fan the cap tip
directly to the last body ring. This closes the hole AND deletes
`arm_cap_segs` vertices per arm.

Before editing, report:
- the vertex index of the last body ring's first vertex, as an expression
  in `arm_vi_start`, `arm_segs_u`, `arm_around`;
- whether `arm_cap_segs` (`min(arm_around, 8u)`) ever differs from
  `arm_around` under the landed `RIBS` mu/sigma. If it can differ, the fan
  must use `arm_around`, not `arm_cap_segs`.

FIND (expect exactly 1 occurrence): the block from `let arm_cap_r` through
the closing brace of the cap index loop. Quote it in full before editing.

Commit alone.

---

## F6 — Palm frond count capped by the slot budget (world.wgsl)

`n_fronds` clamps at a literal 18 for every tier, but cost is per-tier. The
constant cuts SAPLING (20 authored) and COASTAL (24 authored) for no reason
while permitting ROYAL 18 where the budget affords 13. One family constant
cannot serve three tiers. The trunk and crown cap are written BEFORE the
fronds and their cost is exactly known at that point, so the frond count is
whatever the remainder affords.

Pure `min`/`max` arithmetic. No new runtime branch — confirm against the
`world.wgsl` banner FXC block before committing.

### GATE — do not edit until this passes

Capping lowers `ii`. If the ACTIVE path does not zero its unused index
range, a slot that previously held a high-frond palm keeps stale indices
pointing at vertices the new occupant has overwritten, and the cap turns
latent garbage into visible ghost triangles.

Report: does `palm_mesh_gen`'s active path end with a trailing index
zero-fill of the form `for (var i = ii; i < PALMG_MAX_INDICES_PER_SLOT;
i++) { palmg_indices[ib_base + i] = vb_base; }`? Quote it, or state
"none". `cactus_mesh_gen` and `column_mesh_gen` both have one — quote
whichever you use as the model.

If it is MISSING, it lands in this same commit, mirroring the cactus form
exactly. The cap and the zero-fill are one change.

### Verify the cost model before writing the cap

Report each of these as an expression read from the code, not assumed:
- trunk vertices written by the ring loop;
- crown cap vertices (tip + ring);
- trunk indices and crown fan indices;
- vertices written per frond by the frond segment loop;
- indices written per frond.

Claim to confirm or refute: `(trunk_rings + 1) * trunk_segs` trunk verts,
`1 + trunk_segs` crown verts, `trunk_rings * trunk_segs * 6` trunk indices,
`trunk_segs * 3` crown indices, `(frond_segs + 1) * 2` verts per frond,
`frond_segs * 6` indices per frond. STOP on any disagreement.

### Edit

FIND (expect exactly 1 occurrence), verbatim including whitespace:

```
    let golden_angle = PI * (3.0 - sqrt(5.0));
    let n_fronds = min(u32(max(3.0, p.frond_count)), 18u);
    let frond_segs = min(p.frond_segs, 14u);
    let crown_frond_y = crown_y + crown_r * 0.3;
```

REPLACE:

```
    let golden_angle = PI * (3.0 - sqrt(5.0));
    let frond_segs = min(p.frond_segs, 14u);

    // THE SLOT IS THE AUTHORITY. Trunk and crown are already written, so
    // the frond count is whatever the remaining vertex and index budgets
    // afford — never a per-family constant, because the cost is per-tier.
    // The authored floor of 3 yields to the ceiling: a floor that can
    // overrun the slot is not a guard. The two saturating min() calls keep
    // the subtraction total rather than dependent on the ring/seg clamps
    // above staying where they are.
    let trunk_verts   = (trunk_rings + 1u) * trunk_segs + 1u + trunk_segs;
    let trunk_indices = trunk_rings * trunk_segs * 6u + trunk_segs * 3u;
    let verts_left    = PALMG_MAX_VERTS_PER_SLOT   - min(trunk_verts,   PALMG_MAX_VERTS_PER_SLOT);
    let indices_left  = PALMG_MAX_INDICES_PER_SLOT - min(trunk_indices, PALMG_MAX_INDICES_PER_SLOT);
    let frond_ceiling = min(verts_left / ((frond_segs + 1u) * 2u),
                            indices_left / max(frond_segs * 6u, 1u));

    let n_fronds = min(u32(max(3.0, p.frond_count)), frond_ceiling);
    let crown_frond_y = crown_y + crown_r * 0.3;
```

`frond_segs` moves ABOVE `n_fronds` because the ceiling depends on it.
Confirm no other statement between the old and new positions reads
`n_fronds`.

### Report after landing

For each tier at mu bark_rings, report `frond_ceiling` and the resulting
`n_fronds`. Expected: SAPLING 57 / 20, COASTAL 38 / 24, ROYAL 21 / 21. Also
report ROYAL at bark_rings 40. Expected ceiling 13. Disagreement means the
cost model is wrong — report, do not adjust the numbers to fit.

Commit alone (with the zero-fill if it was missing).

---

## F7 — Unclamped trip counts into fixed slots (world.wgsl)

`n_arms = u32(max(0.0, p.arm_count))` and the blade equivalent have NO
upper bound. Every other quantity feeding these loops is clamped —
`arm_segs_u`, `arm_around`, `around`, `trunk_steps` — and the trip count is
the one thing that is not. `ARM_COUNT` and `BLADE_COUNT` both carry
`1e30f` as their parameter ceiling in `*_PARAM_DEFS`, so the distribution
tail is the only thing holding the slot.

This is insurance, not a live failure. Current exposure, to be confirmed by
CC's own arithmetic from F1 item 13, not taken from this document:
CANDELABRA overflows at 9 arms against mu 4.0 sigma 1.0 (~5 sigma);
THICKET at ~31 blades against mu 7.0 sigma 3.0 (~8 sigma). Neither fires.
The defect is that an unbounded loop writing into a fixed slot is a
correctness hole whose probability is a property of the current table, not
of the code — a later table edit moves it without warning.

Jean may drop F7 and wait for a measurement. If it is taken:

### F7a — cactus (lands AFTER F5)

F5 deletes the separate arm cap ring, so the per-arm cost changes. Derive
against the POST-F5 code. Apply the same shape as F6:

- compute the trunk + top-cap vertex and index cost from the code;
- compute per-arm vertex and index cost from the code;
- `arm_ceiling = min(verts_left / verts_per_arm, indices_left / indices_per_arm)`
  with saturating subtraction;
- `n_arms = min(u32(max(0.0, p.arm_count)), arm_ceiling)`.

`n_arms` must move below `arm_segs_u` and `arm_around` in the source, since
the ceiling depends on both. Report the reordered block before editing.

There is no floor to preserve — `max(0.0, ...)` already permits zero arms,
which is a valid Finger cactus.

### F7b — blade

Same shape against `blade_cluster_mesh_gen` and `BLADEG_MAX_*`. Report the
full function first; I have not read it, so no anchor is given here and
none may be assumed. Recon precedes the proposal without exception.

Separate commits, F7a then F7b.

---

## HELD — no edits, ruling pending

### F8 — `bark_rings` is two facts and one of them is dead
It is simultaneously the trunk's ring COUNT (tessellation) and the bark
groove FREQUENCY. Pending F1 item 12: the frequency term evaluates to zero
at every ring for all three tiers at mu, so the grooves the tessellation
exists to resolve are not being drawn at all. Royal spends ~620 verts on a
smooth cone and starves its crown against the same budget. Splitting the
two facts is now cheap — the frequency side has no live output to preserve.
Held: what the trunk's ring count SHOULD be is a design ruling.

### F9 — Palm crown is a point
All fronds share one origin vertex, and the width ramp
`(0.3 + 0.7 * min(1.0, t * 3.0))` starves the ribbon to 30% width exactly
at the base. The crown has no mass where the fronds should merge. F0
reduces crown reach relative to height (Royal 26% -> 23%), which pushes
further in this direction. Held.

### F10 — Blade abundance
`BladeClusterConfig::SPAWN_CHANCE = 0.025f` against Cactus `0.100f` and
Palm `0.200f`. `BLADE_TIERS` weights are within-family and do not affect
abundance. `BLADEG_MAX_SLOTS = 32` is the live ceiling; raising the rate
alone exhausts slots and the gate silently starts failing. A blade cluster
costs ~112 verts against a Royal palm's ~1100 — the cheapest flora per
body by an order of magnitude, and the only family that can be multiplied
in a geometry-bound frame. Rate and ceiling move together or not at all.
Held.

---

## Stale comments — fold into the next handoff that touches these files

`grounded.hpp`:
- Palm UNITS block: `FROND_DROOP/FROND_ARCH = radians` is false. Both are
  dimensionless multipliers of `frond_len` in `palm_mesh_gen` and in the
  designer. Delete the claim.
- Palm UNITS block: `burial = fraction sunk` is false. It is consumed as an
  absolute wu offset (`y = t * p.height - burial`, values 0.15/0.15/0.20).
  Delete or correct.
- Cactus UNITS block: `ARM_CURVE = radians` is false. It is a blend
  fraction in [0,1] (`blend = t * p.arm_curve`). Delete the claim.
- Blade UNITS block: `CURVE = radians` — verify against
  `blade_cluster_mesh_gen` and report before changing.
- `PALM_INDOOR_RESCALE_PARAMS` comment calls FROND_DROOP/FROND_ARCH
  "angles". The behaviour (not rescaling them) is correct — they multiply
  `frond_len`, which IS rescaled — but the stated reason is wrong. Correct
  the reason or delete it.

Delete rather than annotate.
