> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# COMPAT CONSUMER CENSUS — trajectory.hpp (read-only)

Ruling-1 pass: with `musical.inl` retired in M1 (superseded by
`visual_canvas.hpp`), determine whether the COMPAT section that B3 targets
in `coupling/trajectory.hpp` still has **any** consumer on the live tree.
Read-only; no code changed. Anchored to `FINAL_TOUCH @ c55a67b`
(HEAD verified).

## The deletion target

`coupling/trajectory.hpp` COMPAT section — the `Trajectory` struct
(lines 96-101) + the dt/rate overload `trajectory_release(Trajectory, …)`
(lines 103-106) + the `<cmath>` include (line 37, pulled *only* for the
COMPAT `std::exp` at line 104; the FOLLOW system uses no `std::`).

**Critical structural fact:** there is exactly **one** `trajectory.hpp`
(`src/coupling/trajectory.hpp`) — a **shared** coupling-layer header
`#include`d by both `the_board/cartridge.hpp:90` and
`the_chord/cartridge.hpp:90`. It is not per-cartridge. A deletion in this
file lands in every cartridge that includes it.

## Answer, the_board (the scoped question)

**Nothing on the_board references the COMPAT section.** After campaign B1
inlined pawn.inl's aura ramp, zero the_board C++ code uses the `Trajectory`
struct or the dt/rate overload.

Going further: **the_board uses nothing from `trajectory.hpp` at all.** The
FOLLOW/Segment system (`Segment`, `plan_segment`, `sample_segment`,
`trajectory_release(Segment&, …)`) has **zero** consumers in the_board
too. The `#include "coupling/trajectory.hpp"` at `cartridge.hpp:90` is now
a **dead include** on the_board.

| the_board reference | kind | COMPAT consumer? |
|---|---|---|
| `cartridge.hpp:90` `#include "coupling/trajectory.hpp"` | include | dead (uses no symbol from it) |
| `cartridge.hpp:250` "Trajectory-style 0→1 ramp" | comment | no |
| `cartridge.hpp:2929` "Lives in pawn.inl as a Trajectory-driven tick" | comment | no — **stale after B1** (the tick is now an inlined exponential) |
| `state.hpp` `GPUTrajectory` ×5 | separate GPU struct | no — unrelated 16-byte GPU buffer type |
| `world.wgsl:231/238` `Trajectory` / `trajectory_release` | WGSL shader | no — shader-side primitive, compiled independently; the C++ COMPAT *mirrored* this, not vice-versa |

So on the_board: **B2 dissolves entirely** (no musical.inl, no beat-clock
migration target) and the COMPAT debt has **no board consumer**.

## The wrinkle B3 must clear: the shared file still has a live consumer

Because the file is shared, "delete the COMPAT section" is **not yet a
safe no-op** — **the_chord still consumes it:**

- `the_chord/modules/pawn.inl:149-151` — the exact pre-B1 aura ramp
  (`Trajectory ap{…}; ap = trajectory_release(ap, target, dt, rate);
  aura_presence = ap.value;`). B1 was applied **board-only** (accepted
  lockstep drift), so this twin never received the inline. It is the
  **only surviving live consumer** of the COMPAT section anywhere in the
  two live cartridges.

Deleting the COMPAT section from the shared header **would break
the_chord's build** until its ramp is converged. (`backup_board` also has
consumers — `pawn.inl:158`, `musical.inl` ×5 — but it is a frozen backup,
out of scope.)

## Rewritten Campaign B tail (against the post-M1 world) — for ruling

- **B2 — DISSOLVED.** Its premise (migrate musical.inl's followers to
  beat-clocked `Segment`) no longer describes reality; the file is gone.
- **B3 — two candidate shapes; Jean's call, because the clean end-state
  crosses the board-only boundary:**

  1. **Converge the CPU twin, then delete (full close).** Apply the B1
     inline to `the_chord/pawn.inl` (identical edit — same ruling applies:
     aura is a possession affordance, not a musical gesture), removing the
     last live consumer. Then delete the COMPAT section + `<cmath>` from
     `coupling/trajectory.hpp`, fix the stale board comment
     (`the_board/cartridge.hpp:2929`), drop the dead board include
     (`cartridge.hpp:90`), and take Constitution §5 COMPAT → **zero**.
     This *is* the original B3 "converge the twins" intent, extended to
     the CPU twin. **Requires touching the_chord** (lifts board-only for
     this one convergence).
  2. **Board-local only, defer the shared deletion.** Leave the shared
     COMPAT section in place (the_chord needs it); do only board-local
     cleanup — drop the dead `#include` and fix the stale comment. §5
     cannot truthfully reach zero here (the overload still exists in the
     shared file), so it stays at "one file, no board consumer" until a
     the_chord reconciliation pass. Keeps the board-only ruling intact.

  Recommendation: **(1)** — the_chord's ramp is byte-identical to
  the_board's pre-B1 ramp and the same ruling governs it, so convergence
  is mechanical and low-risk, and only (1) actually retires the debt
  (§5 → zero). But it is a scope decision, so it is flagged, not taken.

## Note on the FOLLOW/Segment system

The original B3 spec said to "converge the board's copy to the analysis
side's post-COMPAT `trajectory.hpp`." That post-COMPAT shape retains the
`Segment` MOVE/FOLLOW system as the go-forward coupling primitive — even
though **neither live cartridge currently has a `Segment` consumer**. B3
should **keep** the FOLLOW system (it is the intended future primitive)
and delete **only** the COMPAT section, per the header's own manifesto.
Whether to also drop the now-unused `#include` from a cartridge that has
no follower at all is a separate, per-cartridge cleanup call.

## Bottom line

On the live the_board tree the COMPAT section has **no consumer**, so
Campaign B's board-side work is essentially done: B2 dissolves and B3 is
reduced to a deletion. The single blocker to actually removing the code is
that `trajectory.hpp` is shared and `the_chord/pawn.inl:149` still uses
COMPAT — so the deletion is gated on either converging that twin
(recommended, full close) or deferring the shared deletion to a the_chord
pass (board-only preserved). Everything needed to decide is above.
