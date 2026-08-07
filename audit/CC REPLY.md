Four units. Starting with U1 — reading the function I'm about to make speak:

Implementing U1's five lines — diagnostic only, no behavioral change:

The removed lines are the same condition, split into three named checks — behavior identical. Committing U1:

**U3** — the measurement. First confirming the law's ordinal, since the handoff says L11:

Confirmed — **L11 is THE PAINT ANCHOR LAW; the default-limits law is L14**. Same ordinal divergence as the LANDING ROUND, resolving the same way:

**U4** — census before any edit. The handoff's premise is that transitions budget while boot blocks; that needs verifying:

# PORT_6 — campaign report

**Two landed, one deferred, one STOP.** Pushed `ce015cb..017367e`.

| unit | commit | state |
|---|---|---|
| U1 limits path speaks | `271cc1b` | EXECUTE |
| U2 discard not news | — | **awaiting U1's log** |
| U3 measurement recorded | `017367e` | EXECUTE (L11→L14 resolve) |
| U4 boot streams | — | **STOP — premise falsified** |

## U1 — the exact log shape

Five lines, in occurrence order, all inside the web device request. No behavioral change: the only non-comment lines that moved are the three-clause `if`, now three separately-named checks feeding one `below` flag — same predicate, same branches.

```
[Device] requesting CORE DEFAULTS; exceptions carried: none (C6 cleared maxStorageBuffersPerShaderStage 9->8)
[Device] granted vs floor: maxTextureDimension2D=N/2048 maxStorageBuffersPerShaderStage=N/8 maxUniformBufferBindingSize=N/65536
[Device] BELOW FLOOR: <limit> granted X, floor Y          ← only on disagreement, one per failing limit
[Device] modest device accepted — NO DISCARD              ← or:
[Device] DISCARDING the modest device — its `lost` promise will resolve as a CONSEQUENCE of this discard, not as a failure
[Device] REISSUING request with full adapter passthrough  ← both reissue branches, each saying which
[Device] KEEPING the device from: <path> (this is the one the frame loop runs on)
```

**A defect I noticed while reading, reported and not fixed** (it is U2's subject, and this was a diagnostic unit): the device-lost callback is installed on *every* requested device, including one that would be discarded, and it sets the shared `deviceLost_` flag. So if a discard occurs, the discarded device's lost promise sets `deviceLost_ = true` and PORT_3a's frame gate stops all GPU work — on a healthy device — producing a black screen whose only explanation is a `[Device] LOST` line. That is a real latent bug independent of the current hypothesis.

## U2 — deferred, not decided

I cannot execute this: it is conditional on a log only you can produce. I won't guess which branch it takes. What I can say is that reading the code, the sanity check compares granted-vs-floor at 2048/8/65536 — values a browser granting core defaults meets comfortably (8192/8/65536) — so on that reading no discard should occur and the loss has another cause. But that is the fourth hypothesis from the same source, and the previous three were all falsified by census. Send the log; the `NO DISCARD` / `DISCARDING` line decides it in one read.

## U3 — recorded in both rooms

Numbers stated plainly at the request site (opening with **DO NOT SIMPLIFY THIS BACK**) and as a new paragraph in the law: 62,517 ms with adapter-maximum limits vs 5,609 ms with core defaults, same machine and build, Intel HD 5500 via D3D12.

**Ordinal divergence, resolved:** the handoff cites "the L11 default-limits law". L11 is THE PAINT ANCHOR LAW; the default-limits law is **L14**. The preamble's permanence rule settles it — same correction the LANDING ROUND made to the same citation.

## U4 — STOP, and the premise is wrong in an interesting way

The census contradicts the handoff on both halves:

**Boot does not pre-generate its whole ring.** `fullRegen` generates only the **priority window** — `in_priority_window` = `PATCH_GRID_RADIUS` = 3, so **7×7 = 49 patches** synchronously. The other **176** of the 15×15 pregen ring already stream at `ALLOC_BUDGET_PER_FRAME`/`SPAWN_BUDGET_PER_FRAME` = 4/frame.

**Transitions do not budget differently — they run the identical code.** `reset_surface` sets `last_center_x = INT32_MAX`, and it is called from exactly two places: `cartridge.hpp:526` (boot) and `cartridge.hpp:1018` (TEARDOWN). Both force the same `fullRegen` block on the next frame. The tree says so itself at the boot call site: *"The same door the transition machine uses."*

So there is **no separate transition budget to reuse** — the thing U4 was told to adopt does not exist. `generate_selected_patches` has no internal budget either; it processes every candidate handed to it.

The useful consequence, and why this is worth a design conversation rather than an edit: **the fade does not cover streaming — it covers a freeze.** A mood change pays the same ~5.6 s synchronous block boot does; it is simply camouflaged by fade-out → frozen black → fade-in. Making boot stream means budgeting the `fullRegen` block itself, which changes transition behavior too. That is squarely outside "reuse the transition budget, do not write a second one", so I stopped rather than reaching for it.

If you want it, the shape is one unit: give `fullRegen` a per-frame budget and let the existing `FADE_IN` cover the fill — it would fix boot and transitions together. But it needs your ruling, because it changes how every mood change feels.

## Housekeeping

The orphan rescue (`git branch rescue-175fd17 175fd17`) is yours to run — I'd need you to confirm before I create branches. Both stale branches (`claude/cut-1-limits-fit`, `claude/port-0-seam-census-5z0at8`) are still on the remote awaiting your go.