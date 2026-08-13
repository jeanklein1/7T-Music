# TOGGLE_0 — THE VERDICT, AND DEBT 12 CLOSED AS MOOT

TOGGLE_0 U2 asks that the verdict be recorded "where debt 12 lives".
**Debt 12 does not live anywhere in this tree** — a grep for it across
`docs/` and `audit/` returns only the two handoffs that reference it and
one passing mention in `ATLAS_1revA_GATE.md`. The debt ledger is in
`docs/ESTATE.md`, which has never been delivered to a session. So the
verdict is recorded here, and this file is what ESTATE should absorb.

## THE READING

Jean, native Vulkan, Dawn `f0bf8ab`, 2026-08-13, binary built at
`9489b8d`:

```
[Console] Toggles used (10): … disable_symbol_renaming …
```

Count 9 → 10, the control present.

## THE VERDICT: THE POSITIVE HALF ONLY

**Proven.** A Dawn toggle chained at the descriptor of the stage that
consumes it arrives, and arrives visibly at `GetTogglesUsed`.
`disable_symbol_renaming` is `ToggleStage::Device`; it was chained on
`deviceDesc.nextInChain`; it took on the first boot that asked it to.

**Not proven, and this corrects an earlier claim of mine in-session.**
TOGGLE_0's table reads a present control as branch (a) — "the instance
chain works, so Boot 2 was Dawn refusing DXC on this driver". That
inference does not survive the commit order:

| commit | what it did | ever booted? |
|---|---|---|
| `30c9a7c` — TOGGLE_0 U1 | armed the control on the **instance** descriptor | **no** |
| `c8ad699` — CLOSE_0 A4 | moved the chain to the **device** descriptor | yes, `9489b8d` |

The `(9)` baseline predates the control existing in any binary at all —
`audit/TOGGLE_1_U0.md` proved `idesc.nextInChain` stayed `nullptr` on
the Vulkan plan, because PIVOT_0d-ii left the assignment inside an
`if constexpr (kCompilerPlan == D3D12_Dxc)` and set the plan to Vulkan
in the same commit. **No boot has ever run a binary that chains a toggle
on the instance descriptor.** Branch (a) is unsupported and branch (b)
is unproven. The control test, as TOGGLE_0 designed it, did not run.

## WHY DEBT 12 CLOSES ANYWAY

Not because it was answered — because L21 dissolved the question.

Debt 12 asks whether the instance-descriptor chain propagates to a
device two inheritance hops down. Under L21 no toggle is ever sited
anywhere but its own stage, so nothing in the program's future depends
on that inheritance behaving. The debt is **moot**, not retired.

Reopening it is cheap and specified, should a reason ever appear: re-arm
one Device-stage toggle on `idesc.nextInChain` only, boot once, read the
count. Present → inheritance carries; absent → branch (b) at last
confirmed. Nothing in the queue wants that boot.

## THE GARNISH

`t7_not_a_toggle` testified to nothing, which is what it was armed
under: its silence was ruled inadmissible in advance (Dawn's
warn-on-unknown-name behaviour was believed, never confirmed), and it
stayed silent. No inference is drawn from it. Retired with the control.

## WHAT LANDED

| | |
|---|---|
| control + garnish | removed from `console.hpp`; the site now carries the verdict |
| `GetTogglesUsed` readout | **kept** — it is the witness, not the control |
| `use_dxc` on the instance/adapter path | **untouched**, and correct there: it is `ToggleStage::Adapter` |
| the general fact | `docs/LAWS.md` L21 |
