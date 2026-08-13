# TOGGLE_0 — WITNESS THE TOGGLE CHAIN (debt 12)

Boot 2 proved `use_dxc` did not take; it did not prove why. Two
branches: (a) Dawn validated-and-refused DXC on the Kepler-era driver —
benign, book closed; or (b) PIVOT_0d-ii's instance-descriptor chain
never propagates — in which case **every future Dawn toggle fails the
same silent way**. This campaign is the control test debt 12 designed,
with one improvement the Vulkan console licenses: **the control runs on
the working Vulkan boot** — `disable_symbol_renaming` is a
backend-agnostic Tint toggle, and the readout line already exists
(`[Console] Toggles used (9): …`, CONSOLE_VULKAN). Jean never has to
boot the crashing D3D12 path again.

The control cannot grant the variable under test — that is what makes
it an instrument. Instrument only; no subject edits. Master, two
commits, both tiny.

## U1 — ARM THE CONTROL (one commit, master)

In `console.hpp`, chain `disable_symbol_renaming` through the exact
same instance-descriptor path PIVOT_0d-ii used for `use_dxc` — same
mechanism, same site, nothing bespoke. Verify verbatim first that the
PIVOT_0d-ii chain site exists as CC's P1 assumed; if the site cannot be
found as described → **STOP, report** — that absence is itself branch
(b) evidence. Commit:
`TOGGLE_0: arm disable_symbol_renaming as toggle-chain control`.

## THE READING (Jean, one Vulkan boot, one line)

Boot native (Vulkan, as the machine now is) and read
`[Console] Toggles used (N)`:

| observation | verdict |
|---|---|
| `disable_symbol_renaming` **present** (count 9 → 10) | chain works → Boot 2's failure was Dawn refusing DXC on this driver. Branch (a). **Book closed**, debt 12 retired. |
| **absent** (count stays 9) | chain broken → branch (b): PIVOT_0d-ii never propagates. A fix commit is owed as its own ruled unit — **not improvised here**. |

The reading can ride any native boot — including the ATLAS_1revA gate
session, since the held branch will contain master's U1 if branched
after it.

## U2 — RETIRE THE CONTROL (one commit, master, after the reading)

Remove the control chaining regardless of outcome, same session or
next. A control left armed becomes dead code, and dead code is a liar.
Commit: `TOGGLE_0: retire control; verdict <a|b> — <one line>`.
Record the verdict where debt 12 lives, striking or promoting it.

## STOP CONDITIONS

- The PIVOT_0d-ii chain site not found as described (report as branch-b
  evidence; touch nothing else).
- Any temptation to also "fix" the chain in this campaign — the fix, if
  owed, is its own unit with its own gate.
- Any edit outside `console.hpp`.
