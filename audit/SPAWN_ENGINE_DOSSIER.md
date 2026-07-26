# SPAWN ENGINE DOSSIER — what the next campaign inherits

Written at the close of BOOT_ONE_VOICE, anchored at master `f403ab8`.

The spawn engine is the one subsystem BOOT_ONE_VOICE deliberately did not
enter. The campaign reached the reset door — `reset_surface`, now called from
both boot and the transition machine (LAWS L10) — and stopped there. Everything
below is what it saw from the doorway and did not act on.

**This document is a record, not a work order.** Each entry states what is
true, what is open, and what is already closed. Rulings belong to the campaign
that opens this subsystem.

---

## THE CITATION RULE — binding on this document

**Symbol citations only. No `FILE:LINE` anywhere in this file.**

This corpus currently carries roughly 860 line-number citations across its
recon documents, **206 of which address `.inl` files that exist nowhere in the
tree** — the layout moved to `.hpp` and the addresses stayed. They still read
as valid. `TERRAIN_DOSSIER` alone holds 633, and it attributed
`setup_test_rig_piers` to the wrong file entirely with a precise-looking line
range, which is part of how a retired test rig was believed live during this
very campaign.

A line number drifts on every edit above it, and it drifts **silently**. A
symbol citation goes stale only when the symbol dies, and then it fails
**loud**: the grep returns nothing. This document will be read months from now.
Cite `run_spawn_preamble` and `spawn_engine.hpp`, never a line.

---

## 1 — THE FLOATER BRIDGE — OPEN, DEFERRED BY RULING

The CPU↔GPU floater slot-recovery bridge is **structurally intact**: the encode
is unconditional in `phase_witness_capture`, and the poll plus reconcile live in
`phase_witness_harvest`, both under `F_WITNESS`. Whether the `MapAsync` callback
**actually fires at runtime** is unanswered, and it is not answerable by reading
the tree — it needs the application running.

The probe branch's stated hypothesis — that the active-slot counter and the
per-slot flags drift apart as `n > f` — **does not reproduce on this tree.**
Traced end to end:

- `run_spawn_preamble` reserves a slot and sets its `active` flag **before any
  count exists**, so the transient is `f > n`, not the reverse.
- `dispatch_place_sphere_generic` clears the flag on placement failure. The
  count was never incremented at that point, so this restores sync.
- `dispatch_commit_sphere_generic` clears the flag only when the host patch is
  missing — and in that branch `generic_commit` never runs, so
  `sphere_write_active` (the sole increment) never fires. Also in sync.
- The genuine decrements, in the evict path and in `reconcile_sphere_mirror`,
  are each correctly paired with a flag clear.

`cube_behaviors.hpp` carries the identical shape.

So the leak the probe was built to catch is not there. **What remains open is
the runtime question**, and Jean deferred it to this campaign deliberately
rather than dropping it. The probe itself is preserved in the appendix.

Note for whoever picks this up: `reconcile_sphere_mirror` heals only the
**CPU-true / GPU-false** direction. The opposite orphan — GPU slot active with
the CPU census unaware — is invisible to it. BOOT_ONE_VOICE commit B removed one
instance of exactly that (the boot slot-0 sphere), but by deleting the source,
not by teaching the reconciler. If the bridge is found to be broken at runtime,
the reconciler's one-directionality is the second thing to look at.

## 2 — THE WRITE-ONLY COUNTERS

`activeSphereCount_` (`spheres.hpp`) and `activeCubeCount_`
(`cube_behaviors.hpp`) have **ten write sites between them and zero readers
tree-wide.** Every occurrence is a declaration, a reset to zero, an increment, a
decrement, or a `> 0` guard protecting its own decrement. Nothing anywhere
consults either value.

**The lesson is the more valuable half of this entry.** The floater probe's
entire hypothesis was conditioned on the clause *"if a spawn gate consults the
count."* Nothing does. An hour of instrumentation went into a drift that could
not have mattered, and a single grep for a reader would have closed the question
before it opened.

> **INSTRUMENT A VALUE ONLY AFTER CONFIRMING SOMEONE READS IT.**

This is a **distinct species** from BOOT_ONE_VOICE's findings, and the
distinction is worth keeping. Those were **twins** — two places obliged to agree,
with nothing enforcing the agreement: a literal and its table row, a boot copy
and its transition source. This is something else: **a value maintained for an
audience of zero.** Twins fail when they drift. This cannot fail, because
nothing is listening. It is dead weight that looks like state, and the ordinary
tools for finding dead code do not flag it, because it is written constantly.

## 3 — THE CENSUS PAIR — the handoff's entry, corrected

The commission that ordered this document described `dump_entity_census` as an
ungated 30-second stdout dump running for the whole length of a two-hour
recording. **That is not what the tree says**, and the correction matters
because the next campaign would have inherited a wrong subject.

What is actually true, three separate facts:

- **`dump_entity_census` has ZERO callers.** It is defined in `spawn_engine.hpp`
  and declared in `spawn_services.hpp`, and nothing invokes it. It does not run
  at all. It is dead code, not an ungated diagnostic.
- **`CENSUS_DUMP_INTERVAL` (30 s, `spawn_engine.hpp`) has ZERO readers.** It was
  the cadence for the census that no longer runs. The pair is dead together —
  the same species as entry 2, a constant maintained for an audience of zero.
- **The census that genuinely runs every 30 seconds is `dump_agent_census`** — a
  different function, in a different organ (`agents.hpp`), driven by
  `AGENT_CENSUS_INTERVAL`. It is **ungated stdout**, called from the frame loop
  on a `"periodic"` trigger as well as at `"boot"` and `"mood-transition"`. The
  concern behind the original entry is therefore real; it simply belongs to the
  agents organ, not the spawn engine.

**Ruling owed, now on the correct subjects:** `dump_entity_census` and
`CENSUS_DUMP_INTERVAL` are delete-or-wire (they are currently neither);
`dump_agent_census` joins the DIAG census or is gated. It is not among the
constitution's list of six unwrapped DIAG sites.

## 4 — THE O(n) SPAWN SCANS

`check_position` and `proximity_affinity_boost` (both `spawn_engine.hpp`) each
loop the full footprint registry — `for i < MAX_FOOTPRINTS` — per candidate, per
family, per patch.

`PopFamily::COUNT` is **12** (static-asserted in both `roster.hpp` and
`indoor_module.hpp`) and `MAX_FOOTPRINTS` is **128**, so a patch spawn costs on
the order of **12 × 128 = 1536 distance tests**.

**Adequate today.** Named here so it is not discovered under load, by someone
who does not know it was already seen.

## 5 — MAX_FOOTPRINTS = 128

`MAX_FOOTPRINTS` (`spawn_engine.hpp`) has never been examined against the entity
population it is meant to cover. **Stated, not judged.** The number may be
right; nobody has checked, and that is the finding.

## 6 — CLOSED — DO NOT REOPEN — the tile/patch containment seam

The family-0 `tile_apply_spawn_mult` abort is **fixed**, by commit `e1f49b0`
(`F1 FIX_TILE_PATCH_CONTAINMENT: a tile must outlive every patch that stands on
it`), which is on master and permanently reachable.

The diagnosis: the tile cache and the patch registry are two eviction clocks at
different speeds. Tile eviction is unbudgeted and immediate; patch eviction is
budgeted and lags. In steady state the `FORGET_RADIUS` slack keeps tiles
outliving their patches, but a world-transition centre jump consumes that slack
in one unbudgeted call, orphaning a patch allocated in one frame and spawned in
another. The fix made containment structural rather than a slack number:
`evict_distant_tiles` gained a `KeepFn` overload, and the call site supplies
`build_active_patch_set`, so a tile hosting a live patch is spared however far
the centre moved.

**The live tripwire for this class is on master**: `tile_apply_spawn_mult`'s
`std::cerr` plus `std::abort` on a cache miss. It was kept deliberately as the
witness. Do not remove it while opening this subsystem.

The branch `diag/spawn-seam` was this bug's instrument. It is retired, and
**nothing of it is preserved** — deliberately. Its finding is closed, and
`e1f49b0`'s own commit message quotes the instrument's output (the figure
"cache 24" is a line that branch printed). The record already exists, in the
better place, written by the thing that consumed it.

---

## 7 — APPENDIX: THE PRESERVED PROBE

The branch `diag/floater-bridge` was **deleted** at the close of BOOT_ONE_VOICE.
It was never merged, so its commit is unreachable and will eventually be
collected. **This diff is the only surviving copy.**

It is embedded here in full rather than referenced by SHA on purpose: a recorded
SHA pointing at an unreachable commit reads exactly like a live one, which is
this corpus's signature failure mode. The record must contain what it describes,
or it is not a record.

The probe applied cleanly to `f403ab8` and sat in regions BOOT_ONE_VOICE never
touched. **That is a fact about that day, not a promise about yours** — whoever
re-applies it verifies against the tree in front of them.

```
commit 70c415887be00cbd1fef6a10ef6437bbd9b225c2
Author: Claude <noreply@anthropic.com>
Date:   Fri Jul 24 04:29:27 2026 +0000

    DIAG_FLOATER_BRIDGE v2 — temporary floater slot-recovery instrument (DO NOT MERGE)
    
    Throwaway diagnostic cut from master. Two blocks in cartridge.hpp, both tagged
    DIAG_FLOATER_BRIDGE for removal by search:
    
    INSERTION A (phase_witness_harvest, above the COPIED poll): once/second prints
    per-family the COUNTER (activeSphereCount_/activeCubeCount_, n=) AND a live scan
    of the per-slot `active` flags (f=), plus the last-seen GPU-active count (gpu=),
    the callback tick (cb=), and the readback state (st=). n vs f exposes counter
    drift: run_spawn_preamble sets the flag on reservation without touching the
    count; dispatch_place_*/dispatch_commit_* clear the flag on failure, also without
    touching the count -- if a spawn gate consults the count, n>f is the bug.
    
    INSERTION B (MapAsync callback, after reconcile_cube_mirror): scans the mapped
    readback for GPU-active spheres/cubes and stows the counts + bumps cb, so the
    sync poll can print the last delivered snapshot.
    
    STEP 1 closed: bridge structurally intact (encode unconditional in
    phase_witness_capture, poll+reconcile in phase_witness_harvest, both under
    F_WITNESS) -- the break is runtime. glaw1 GREEN with the instrument applied.
    
    Remove after reading: search DIAG_FLOATER_BRIDGE, delete both blocks. This
    branch must not merge into master or the TIDY_1 review branch.
    
    Co-Authored-By: Claude <noreply@anthropic.com>
    Claude-Session: https://claude.ai/code/session_01FJZ6g9sRQgfT8WsKNR6nHv

diff --git a/src/cartridges/the_board/cartridge.hpp b/src/cartridges/the_board/cartridge.hpp
index 1bd3e3d..0ed38d5 100644
--- a/src/cartridges/the_board/cartridge.hpp
+++ b/src/cartridges/the_board/cartridge.hpp
@@ -1051,6 +1051,32 @@ namespace t7 {
                 }
 
                 //
+                // ── DIAG_FLOATER_BRIDGE (temporary) ───────────────────────
+                static uint32_t dbg_fb_cb   = 0;   // times the readback callback ran
+                static uint32_t dbg_fb_gsph = 0;   // GPU-side active spheres, last seen
+                static uint32_t dbg_fb_gcub = 0;   // GPU-side active cubes,   last seen
+                {
+                    static float dbg_fb_last = -1.0f;
+                    const float now_s = time_state_.seconds;
+                    if (now_s - dbg_fb_last >= 1.0f) {
+                        dbg_fb_last = now_s;
+                        uint32_t fs = 0, fc = 0;
+                        for (uint32_t i = 0; i < Dim::MAX_SPHERE_INSTANCES; i++)
+                            if (sphere_state_.activeSpheres_[i].active) fs++;
+                        for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++)
+                            if (cube_behaviors_state_.activeCubes_[i].active) fc++;
+                        std::cout << "[FLOATER] sph n=" << sphere_state_.activeSphereCount_
+                                  << " f=" << fs
+                                  << " gpu=" << dbg_fb_gsph
+                                  << " | cub n=" << cube_behaviors_state_.activeCubeCount_
+                                  << " f=" << fc
+                                  << " gpu=" << dbg_fb_gcub
+                                  << " | cb=" << dbg_fb_cb
+                                  << " st=" << static_cast<int>(floaterReadbackState_)
+                                  << "\n";
+                    }
+                }
+                // ── end DIAG_FLOATER_BRIDGE ─────────────────────────────
                 if (floaterReadbackState_ == FloaterReadbackState::COPIED) {
                     floaterReadbackState_ = FloaterReadbackState::MAPPING;
                     gpuState_.floating_entity_readback_staging().MapAsync(
@@ -1071,6 +1097,18 @@ namespace t7 {
                                             reconcile_sphere_mirror(sphere_state_, &sphere_deps_, data);
                                         if constexpr (ROSTER.cube)    // ROSTER-GATE cube (b)
                                             reconcile_cube_mirror(cube_behaviors_state_, &cube_deps_, data);
+                                        // ── DIAG_FLOATER_BRIDGE (temporary) ───────────────────────
+                                        {
+                                            uint32_t gs = 0, gc = 0;
+                                            for (uint32_t i = 0; i < Dim::MAX_SPHERE_INSTANCES; i++)
+                                                if (data[i].is_active != 0u) gs++;
+                                            for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++)
+                                                if (data[Dim::CUBE_SLOT_OFFSET + i].is_active != 0u) gc++;
+                                            dbg_fb_gsph = gs;
+                                            dbg_fb_gcub = gc;
+                                            dbg_fb_cb++;
+                                        }
+                                        // ── end DIAG_FLOATER_BRIDGE ─────────────────────────────
                                     }
                                 }
                                 gpuState_.floating_entity_readback_staging().Unmap();
```
