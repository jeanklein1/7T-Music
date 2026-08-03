# SALON_1 — P5 REPORT: does anything distinguish `spring_h` from `ceiling_height`?

Report-first stage. **No edit is in this commit.** Read at `7e1b7f8`.

Amendment II §4 asks one question and forks on it:

> Report whether any consumer distinguishes `spring_h` from `m.ceiling_height`
> on VAULT. The gap is 0.25 against 25.0.

**Answer: no consumer distinguishes them. The fork takes the first branch —
they are one fact.** Evidence below, plus two findings the question did not ask
for, one of which changes what the edit should do.

---

## §1 — THE COMPLETE CENSUS

Every occurrence of `vault_crown`, `VaultCrown`, `spring_h`, `crown_h` and
`wall_h` in `src/`. Twelve sites, all in `mood.hpp`. Nothing in any other file,
nothing in WGSL.

| path:line | kind | what |
|---|---|---|
| `mood.hpp:606` | DECLARATION | `struct VaultCrown { float spring_h; float rise; float crown_h; };` |
| `mood.hpp:607` | DEFINITION | `vault_crown(const MoodProfile&, float bmin, float bmax)` |
| `mood.hpp:623` | WRITE | `const float spring_h = paint_top + SPRING_MARGIN;` |
| `mood.hpp:624` | READ | `const float min_rise = ch - spring_h;` — see §3 |
| `mood.hpp:627` | READ | `return VaultCrown{ spring_h, rise, spring_h + rise };` |
| `mood.hpp:629-630` | DEFINITION | `vault_crown_height()` → returns `.crown_h` only |
| `mood.hpp:651` | **CALL** | `effective_ceiling = vault_crown_height(m, bmin, bmax);` — camera ceiling clamp |
| `mood.hpp:737` | WRITE | `float wall_h = ch;` — the FLAT default |
| `mood.hpp:742` | **CALL** | `const VaultCrown vc = vault_crown(m, bmin, bmax);` |
| `mood.hpp:743-745` | READ | `wall_h = vc.spring_h; rise = vc.rise; crown_h = vc.crown_h;` |
| `mood.hpp:748` | READ | `float wall_top = wall_h + JOINT_OVERLAP;` — the four wall quads |
| `mood.hpp:792` | READ | `return crown_h - cat_a * (std::cosh(dist / cat_a) - 1.0f);` — the vault surface |
| `mood.hpp:812` | READ | `y = wall_h - JOINT_OVERLAP;` — the vault mesh's edge ring |
| `mood.hpp:859` | READ | the `[Shell]` log line |

**Two callers of `vault_crown`, and they take disjoint fields.** The camera
clamp takes `crown_h` alone; `generate_indoor_shell` takes all three.

---

## §2 — THE ANSWER

`spring_h` reaches exactly **three** consumers, all through `wall_h`:

1. **`wall_top = wall_h + JOINT_OVERLAP`** (`:748`) — the top edge of the four
   wall quads.
2. **`y = wall_h - JOINT_OVERLAP`** (`:812`) — the vault mesh's edge ring, where
   the vault meets the wall. The `JOINT_OVERLAP = 3.0` on either side is what
   makes the two surfaces interpenetrate rather than crack.
3. **The log** (`:859`).

Plus `crown_h = spring_h + rise` (`:627`), which reaches the vault surface
(`:792`) and the camera clamp (`:651`).

**Not one of them reads `spring_h` as anything but "the height at which the wall
stops and the vault begins."** No consumer compares it to `ceiling_height`, none
requires a gap between them, and none would be wrong if the two were equal. The
0.25 is not a designed clearance — it is the residue of the arithmetic
`ch·0.45 + 5.5 + 8.0` landing near `ch` by coincidence.

If `spring_h` became `ceiling_height` (25.0), everything shifts by the same
+0.25 and nothing changes shape:

| radius | `wall_top` | vault edge `y` | `crown_h` |
|---:|---|---|---|
| 1 | 27.75 → 28.00 | 21.75 → 22.00 | 47.25 → 47.50 |
| 2 | 27.75 → 28.00 | 21.75 → 22.00 | 62.25 → 62.50 |
| 3 | 27.75 → 28.00 | 21.75 → 22.00 | 77.25 → 77.50 |
| 4 | 27.75 → 28.00 | 21.75 → 22.00 | 92.25 → 92.50 |

**Fork branch one: they are one fact.** The derivation chain in `vault_crown`
dies, and its read of `WALL_ART.paint_y_frac` — P2's wire — goes with it.

---

## §3 — FINDING: `min_rise` IS DEAD ARITHMETIC

`mood.hpp:624-626`:

```cpp
    const float min_rise = ch - spring_h;
    const float rise = std::max(half_span * VAULT_RISE_FRACTION,
                                std::max(min_rise, MIN_RISE_FLOOR));
```

`min_rise` is `0.25`. `MIN_RISE_FLOOR` is `5.0`. `half_span · 0.30` ranges over
`[22.5, 67.5]` across the four legal radii. So:

| radius | `half_span·0.30` | `min_rise` | winner |
|---:|---:|---:|---|
| 1 | 22.50 | 0.25 | `half_span·0.30` |
| 2 | 37.50 | 0.25 | `half_span·0.30` |
| 3 | 52.50 | 0.25 | `half_span·0.30` |
| 4 | 67.50 | 0.25 | `half_span·0.30` |

**Neither `min_rise` nor `MIN_RISE_FLOOR` ever wins, at any reachable radius.**
The span term dominates by a factor of 90 at worst. `min_rise` exists to
guarantee the crown clears `ceiling_height`, and the span term already does so
by an enormous margin.

This makes the fork cheaper than it looks: setting `spring_h = ch` drives
`min_rise` to exactly `0.0`, which is still dominated. **`rise` does not move at
all.** The entire delta of the change is the uniform +0.25 in §2.

Whether the dead terms are then deleted is a separate call and not this
report's to make.

---

## §4 — FINDING THE QUESTION DID NOT ASK: `ceiling_height` MEANS TWO THINGS

Worth having before the edit is written, because the fork's first branch renames
a field and the name has to survive contact.

On FLAT, `m.ceiling_height` **is** the ceiling: `wall_h = ch` (`:737`), the
walls run to it, and the camera clamps to it (`:647`, the `effective_ceiling` default).

On VAULT it is **not** the ceiling. It is an input to a derivation, and the
actual ceiling is `crown_h` — which is what the camera clamp uses (`:651`). At
`ch = 25.0` the crown is **47.25 to 92.25** depending on radius. The field named
`ceiling_height` names something 2–4× below the actual ceiling on that path.

Two consequences:

- **`spring_h` can exceed `ceiling_height`.** It is not a near-identity that
  holds generally. At `ch = 25` the gap is +0.25 in `ch`'s favour, but the
  expression is `ch·0.45 + 13.5`, which crosses `ch` at `ch = 24.55`. Any
  `ceiling_height` below that — including FLAT's 20.0, were VAULT ever given it
  — puts the spring *above* the nominal ceiling, and `min_rise` goes negative.
  Inert today only because `min_rise` is dominated (§3). **The current values
  sit 0.45 wu from a sign change in a term nothing reads.**
- **If the fork collapses `spring_h` into `ceiling_height`,** that field then
  means "wall top" on both paths and "the ceiling" on neither VAULT reading.
  That is a *better* meaning than today's, and consistent with `wall_h` already
  being the variable both branches assign to at `:737` / `:743`. Recorded so the
  naming is chosen deliberately rather than inherited.

---

## §5 — WHAT THIS SETTLES FOR E

Amendment II's second charge against the chain is confirmed:

> `spring_h = paint_center + 5.5 + 8.0` guarantees clearance above a *nominal*
> paint centre. Under E the art fills to `wall_top − top_margin`, so the nominal
> centre stops predicting where the art's top is.

Today the nominal paint centre is `11.25` and `paint_top` is `16.75`, against a
measured actual top of **16.45** including the frame border (SALON_1.md §E.1).
The guarantee holds today **by 0.30 wu** — which is luck, not design, and it is
the same order as the 0.25 residue this whole section is about.

Under E, with the fill tier reaching toward `wall_top − top_margin`, the actual
top becomes a function of `top_margin` and has no relation to `ch·0.45 + 5.5`.
**The chain would not merely be circular; it would be false.**

Reversing the dependency fixes both at once: the room defines the spring, E
bounds the art under it, and the clearance becomes a real constraint on real
geometry rather than a fraction standing in for it.

`wall_top` is then introduced by E, in the commit that has a reader for it —
which is what E0 got wrong.

---

## LEDGER ROW

| Stage | State | Commit | Note |
|---|---|---|---|
| P5-report | **reported** | this commit | **no consumer distinguishes them — fork branch one.** `spring_h` reaches 3 consumers, all as "where the wall stops". `min_rise` is dead at every radius, so `rise` does not move; total delta is a uniform +0.25. Two extras: `ceiling_height` means two different things by path, and the clearance guarantee currently holds by 0.30 wu. |
