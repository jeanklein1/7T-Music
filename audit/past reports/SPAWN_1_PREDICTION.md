# SPAWN_1c — the prediction, recorded before the instrument exists

Campaign: SPAWN_SWEEP v2. Handoff: `cc_handoff_spawn_1.txt`.
Base: master @ `b6f296f`. Written **before** any SPAWN_1 code was edited.

> "An instrument that reports is worth less than an instrument that can be
> wrong." — cc_handoff_spawn_1.txt, SPAWN_1c

This file is committed **ahead of the 1a and 1b code commits** so that the
ordering is provable from `git log` rather than asserted. A prediction recorded
after the fact is not a test.

---

## 0. THE HONEST CONSTRAINT ON THIS STAGE

**Gates 1a, 1b and the *actual* half of 1c cannot be run in this container, and
I have not run them.**

| requirement | status |
|---|---|
| Dawn SDK (`C:/dev/dawn`, MSVC `.lib`s — `CMakeLists.txt:14-15`) | **absent** |
| GPU device (`/dev/dri`) | **absent** |
| built `incubator_dual` binary | **absent** |
| `glaw1` (syntax-only, stubbed SDK) | **available, and run** |

`audit/tools/glaw1/run.sh` stubs the absent SDK surface and runs
`g++ -fsyntax-only` over the whole real cartridge TU. It proves the tree parses,
scopes and looks up clean. **It cannot produce a single line of census output.**

So no census print appears anywhere in this stage's report, and none should.
Every "actual" column below is empty and stays empty until Jean runs the build.
The runbook for filling them is at the end of this file.

This is the one place the handoff's land order cannot be followed literally:
"Do not proceed to 1b until you have seen output." The blocker is environmental,
not informational — it says nothing about whether the code is right, and holding
1b would have delivered a four-line wiring change and nothing else. I
implemented 1a, 1b and 0b, gated each on `glaw1`, and left every runtime gate
explicitly unrun. **If Gate 1a fails on Jean's machine, 1b and 0b are still
sound code but their gates are void and the stage stops there**, exactly as the
handoff says.

---

## 1. THE THREE TRIGGERS PREDICT DIFFERENT THINGS

The handoff asks about the periodic trigger. Boot and mood-transition turn out
to carry **sharper, harder assertions**, so they are predicted separately.

### Trigger `"boot"` — PREDICT: all twenty-four cells zero

`reset_surface` runs at `cartridge.hpp:511` (MOVEMENT: BOOT — S2 THE SURFACE),
which bulk-wipes `footprints_[]` (`patch_system.hpp:136-137`). The boot census
site is `:552`, inside MOVEMENT: BOOT — S3 PLACEMENT. Patch streaming happens in
the render loop (`phase_stream_patches`, RENDER_SPINE row at `cartridge.hpp:1516`),
which has not run yet.

**Prediction: `active` = 0 and `claimed` = 0 for all twelve families, delta 0.**

Anything nonzero at boot means something spawned before the surface existed.

### Trigger `"mood-transition"` — PREDICT: all twenty-four cells zero

**This is the ordering question the handoff flagged as REPORT-DO-NOT-ASSUME, and
the answer makes the assertion stronger than the handoff hoped for.**

`TransitionPhase::TEARDOWN` is a single case block spanning
`cartridge.hpp:872-958` — one frame, one straight line. Within it:

```
:901  reset_surface(...)        ← bulk-wipes footprints_[]  (the claimed side)
:902  teardown_entities(...)    ← clears the 7 generic arrays (the active side)
:904  teardown_gol           (ROSTER-gated)
:906  teardown_ribbon        (ROSTER-gated)
:908  clear_spheres          (ROSTER-gated)
:910  clear_cubes            (ROSTER-gated)
:914  teardown_gallery       (ROSTER-gated)
...
:948  dump_agent_census(..., "mood-transition")   ← the census site
:958  transitionPhase_ = FADE_IN
```

The census fires **after** the wipe — so the handoff's hoped-for assertion
(`claimed` = 0) holds. But all twelve families are *also* torn down above it:
the seven generic ones by `teardown_entities`, and gol / ribbon / sphere / cube /
gallery by their own verbs. And `transition_machine` is an UPDATE_SPINE row
(`:1508`) while `stream_patches` is a RENDER_SPINE row (`:1516`), so nothing has
re-streamed when the census prints.

**Prediction: `active` = 0 AND `claimed` = 0 for all twelve, delta 0.**

That makes the mood-transition census a **teardown-completeness test**, not an
observation. A nonzero `active` names a family whose teardown verb missed it. A
nonzero `claimed` means `reset_surface` did not wipe what it claims to.

Per the handoff: I did **not** reorder anything to obtain this. The ordering was
already favourable.

### Trigger `"periodic"` — the interesting one

Everything below concerns this trigger only.

---

## 2. PREDICTED PERIODIC DELTAS

`delta = claimed − active`.

| family | predicted delta | why | actual |
|---|---|---|---|
| `sph` | **0** | registers today via `negotiate_position`; SPAWN_2 later drives `claimed` to 0 | _(unrun)_ |
| `cube` | **0** | same | _(unrun)_ |
| `arch` | **NEGATIVE**, equal to −(live force-spawned portal count) | ruling 14: Channel B (`force_spawn_portal_arch`) registers nothing; Channel A already registers | _(unrun)_ |
| `ribbon` | **≥ 0**; positive iff a 0-tip reject has occurred | leak path 3 (`dispatch_commit_ribbon`, `ribbon.hpp:1336`) | _(unrun)_ |
| `gall` | **≥ 0**; positive iff a 0-painting abort has occurred | leak path 2 (`gallery.hpp:900/904/908`) | _(unrun)_ |
| `pyr` `col` `ant` `palm` `cact` `blad` `gol` | **≥ 0**, starting at 0 and drifting up | leak path 1 only | _(unrun)_ |
| any ROSTER-disabled family | **0 on both sides** | never selected (`spawn_engine.hpp:680`) | _(unrun)_ |
| **TOTAL, over time** | **MONOTONE NON-DECREASING** | leak path 1 is unbounded on a traverse | _(unrun)_ |

### The arch row is the sharpest, and it is an integer

`PORTAL_DENSITY = 1.00f` (`mood.hpp:111`), so **every** DOORWAY-tier arch that
comes through dispatch becomes a portal — and those went through
`generic_place` → `negotiate_position` → `register_footprint`, so they are
registered. Only `force_spawn_portal_arch` (`grounded.hpp:626`, the sole
`is_portal = true` write outside the generic path at `:716`) registers nothing.

So:

```
predicted arch delta  =  − count of live arches with is_portal == true
                          that were created by force_spawn_portal_arch
```

This is not a direction, it is a number, and it is separable from the
dispatch-created portals only by provenance — which the `ActiveArch` record does
not currently store. **Cheapest honest instrumentation:** force-spawned portals
are exactly the arches whose `host_gx`/`host_gz` were never written (ruling 20 —
`force_spawn_portal_arch` never writes them). That is a *symptom*, not a
provenance field, and I have deliberately **not** added a provenance bit in this
stage: it is SPAWN_3's business (ruling 20 writes those fields, at which point
the symptom disappears). Predicting the integer therefore needs one number Jean
can read off the back-portal machinery, not a new field.

**If arch delta is 0**, ruling 14 is wrong and Channel B is registering somehow —
that would be the most valuable falsification available in this stage.

### The drift prediction needs wall-clock

Leak path 1 keys a footprint to a host patch that does not exist, so
`unregister_footprints_for_patch` (sole caller `patch_system.hpp:47`) can never
match it. On a wandering camera it is eventually reclaimed only if that exact
cell streams in *and evicts again*; on a linear traverse, never.

**Prediction: over several minutes of continuous traverse the TOTAL `claimed`
column rises monotonically and never falls, except at a mood transition (where
`reset_surface` zeroes it).** The `active` column should stay bounded by the
per-family caps.

That sawtooth — monotone rise, hard reset at each transition — is the signature
of leak path 1, and it is what SPAWN_3 has to flatten.

---

## 3. WHAT WOULD FALSIFY THE MODEL

Named in advance, because reconciling a surprise by editing the prediction is
how a test becomes an observation.

| observation | what it would mean |
|---|---|
| `arch` delta = 0 | ruling 14 wrong; Channel B registers after all |
| `arch` delta positive | arches leak faster than portals under-register; leak path 1 dominates on this family |
| `sph`/`cube` delta ≠ 0 | floaters are leaking or double-registering — SPAWN_2's premise needs re-checking before it subtracts |
| TOTAL `claimed` falls without a transition | something *does* release footprints that this audit did not find |
| TOTAL `claimed` flat over a long traverse | leak path 1 is not reachable in practice; SPAWN_3's promotion to priority-2 is unjustified |
| a ROSTER-disabled family nonzero on either side | the select-time roster gate is not the only door |
| boot or mood-transition not all-zero | teardown is incomplete; names the family |
| `active` exceeds a family's cap | slot accounting is broken |

The last three are assertions about the *program*, not about the campaign, and
any of them would outrank the campaign's current plan.

---

## 4. RUNBOOK — what Jean runs, and what to paste back

```
cmake --preset the-board-full
cmake --build --preset the-board-full
```

Then run and capture stdout. Three prints are needed:

1. **Gate 1a** — the first `[CENSUS ...]` line for each of `trigger=boot`,
   `trigger=mood-transition`, `trigger=periodic`. Raw paste. If none appears,
   **stop** — that is the finding and 1b/0b gates are void.
2. **Gate 1b** — one full periodic print, all twelve rows plus TOTAL.
3. **Gate 1c** — the TOTAL row from several periodic prints spread over a few
   minutes of continuous traverse, so the drift trend is visible.

Also worth capturing, cheap and diagnostic: whether any of the seven write-only
`EntitiesState` count fields disagrees with its scan at first print. The census
does not print that comparison (deliberately — the handoff says note it, do not
add it yet), but a disagreement would mean an evictor is leaking.

`glaw1` is already GREEN at every commit in this stage; it is not the gate that
matters here.
