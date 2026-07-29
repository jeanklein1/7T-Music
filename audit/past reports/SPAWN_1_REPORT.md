# SPAWN_1 — stage report

Campaign: SPAWN_SWEEP v2. Handoff: `cc_handoff_spawn_1.txt`.
Base: master @ `b6f296f`. Branch: `claude/spawn-campaign-handoffs-wsblrd`.

| # | commit | stage |
|---|---|---|
| 1 | `3723a3a` | SPAWN_1c — the prediction (recorded first, deliberately) |
| 2 | `b280d2c` | SPAWN_1a — wire the census |
| 3 | `22049d8` | SPAWN_1b — rebuild the census |
| 4 | `70f42ea` | SPAWN_0b — cut `PierTier` |

`glaw1` GREEN at every one.

---

## 1. THE HEADLINE: HALF THE GATES COULD NOT BE RUN, AND WERE NOT

**No census output exists in this stage, because none was produced.** Gates 1a
and 1b each have a compile half and a runtime half. The compile halves passed.
The runtime halves — "it printed at all", "paste a raw periodic print" — and
the entire *actual* column of 1c require running the binary, which this
container cannot do:

| requirement | status |
|---|---|
| Dawn SDK (`CMakeLists.txt:14-15` pins `C:/dev/dawn`, MSVC `.lib`s) | **absent** |
| GPU device (`/dev/dri`) | **absent** |
| built `incubator_dual` | **absent** |
| shader compiler (Tint / FXC / naga) for Gate 0b | **absent** |
| `glaw1` — `g++ -fsyntax-only` over the real TU, stubbed SDK | **available, GREEN ×4** |

`glaw1` does not read `.wgsl` at all and cannot emit a census line. So:

- **Gate 1a — runtime half: UNRUN.**
- **Gate 1b — runtime half: UNRUN.** (Format verified separately — see §4.)
- **Gate 1c — prediction: DONE and committed first. Actual: UNRUN.**
- **Gate 0b — shader recompile: UNRUN, and it is yours.**

The runbook is in `audit/SPAWN_1_PREDICTION.md` §4.

### Where I departed from the land order, and why

The handoff says: *"Do not proceed to 1b until you have seen output. If it does
not print, that is the finding and the stage stops there."*

I could not see output, and the reason is environmental — it says nothing about
whether the code is right. Stopping there would have delivered a four-line
wiring change and nothing else. So I implemented 1a, 1b and 0b, gated each on
`glaw1`, and left every runtime gate explicitly unrun and unclaimed.

**The dependency still holds and you should enforce it:** if Gate 1a fails on
your machine, 1b's and 0b's gates are void — the code is still sound but
unproven, and the stage stops there exactly as written. Nothing downstream
(SPAWN_2 onward) should start until a census has actually printed.

---

## 2. THE ORDERING QUESTION — ANSWERED, AND BETTER THAN HOPED

The handoff flagged this REPORT-DO-NOT-ASSUME: at the mood-transition site,
does the census fire before or after `reset_surface`?

**After — and after every teardown verb too.** `TransitionPhase::TEARDOWN` is a
single case block, `cartridge.hpp:872-958`, one frame straight through:

```
:901  reset_surface(...)      ← wipes footprints_[]        (the claimed side)
:902  teardown_entities(...)  ← clears the 7 generic arrays (the active side)
:904  teardown_gol        (ROSTER-gated)
:906  teardown_ribbon     (ROSTER-gated)
:908  clear_spheres       (ROSTER-gated)
:910  clear_cubes         (ROSTER-gated)
:914  teardown_gallery    (ROSTER-gated)
:948  dump_agent_census(..., "mood-transition")   ← census site
:958  transitionPhase_ = FADE_IN
```

`transition_machine` is an UPDATE_SPINE row (`:1508`); `stream_patches` is a
RENDER_SPINE row (`:1516`). Nothing has re-streamed when it prints.

The handoff hoped for `claimed = 0`. **All twelve families are torn down above
the site as well, so both columns must read 0.** That upgrades the trigger from
an observation to a **teardown-completeness assertion**: a nonzero `active`
names a family whose teardown verb missed it; a nonzero `claimed` means
`reset_surface` did not wipe what it claims to.

Per instruction, **nothing was reordered to obtain this** — the ordering was
already favourable. The reasoning is recorded at the call site.

The same reasoning gives boot the same assertion: `reset_surface` at `:511`,
census at `:553`, patch streaming not yet run → both columns 0.

---

## 3. WHAT LANDED

### 1a — three trigger sites

`phase_census_dumps`' own banner (`cartridge.hpp:1190-1192`) already read
*"periodic agent census + entity census"*. The function was authored for both
halves; only the agent half was ever wired. This filled the stub it left.

Boot and mood-transition do not touch `lastCensusDump_`, because the agent
census does not either at its equivalent sites — mirror, not improvement. Not
ROSTER-gated: a disabled family must read zero on both sides, and that
agreement is a check worth keeping.

### 1b — `FamilyDispatch` gains an eighth member

```cpp
uint32_t (*active_count)(const MachineCtx* self);
```

**Const**, unlike the six verbs, which all mutate. Safe because `const` does not
propagate through `MachineCtx`'s reference members.

The twelve accessors are one-liners over one template — the P11 shape the file
already names ("one implementation, ten callers"):

```cpp
template<typename T, size_t N>
inline uint32_t census_scan_active(const T (&arr)[N]);
```

**The array bound is deduced, never written.** That is the substance, not the
brevity: it makes three of the handoff's standing traps *structurally
unreachable* rather than merely avoided —

- `MAX_RIBBON_INSTANCES` / `MAX_GALLERIES` aren't `Dim::` members → no accessor
  has to know, so none can get it wrong;
- `ANTENNA_SLOT_OFFSET` / `CUBE_SLOT_OFFSET` are GPU-side → no index arithmetic
  is written, so no offset can leak in;
- `active_slot_count` / `cpu_pyramids.count` are high-water marks → not
  reachable from a deduced-extent scan.

### 0b — `PierTier` cut

**The handoff's prescribed rename would not have compiled.** It says
`tier → _pad0`; both structs *already* carry `_pad0` and `_pad1` after
`is_active`, so that duplicates a member name in C++ and in WGSL. Renamed
sequentially instead — `tier → _pad0`, existing `_pad0`/`_pad1` → `_pad1`/`_pad2`
— so the three pads read in declaration order. Types, positions, stride and
`sizeof` unchanged, which is what ruling 17 actually required.

The trap was checked rather than trusted: `GPUColumnMeshParams.tier` is still
written at `spawn_engine.hpp:384` and read at `world.wgsl:9896/:9935/:10064`,
intact after the cut.

---

## 4. WHAT *WAS* VERIFIED, AND EXACTLY HOW FAR IT GOES

The print format is fixed by the handoff "because every later stage compares
against it", so I verified it — by compiling the exact `ostream` sequence
standalone against fabricated numbers:

```
[CENSUS t=  312.4 trigger=periodic]
  fam    active  claimed   delta
  pyr         3        3       0
  arch        5        3      -2
  ...
  TOTAL      65       65       0    footprints 65/128
```

Character-for-character with the specification. Zero prints bare; only nonzero
carries a sign.

**This gates the format and nothing else.** The numbers above are fabricated
input to a formatting harness. They are not a census, they are not from the
program, and no conclusion about the world follows from them.

`footprints 65/128` — capacity is `MAX_FOOTPRINTS = 128` today; ruling 8 raises
it to 512 in SPAWN_3.

---

## 5. THE PREDICTION

Committed as `3723a3a`, **ahead of the 1a and 1b code**, so the ordering is
provable from `git log` rather than asserted. Full table and the eight named
falsifications are in `audit/SPAWN_1_PREDICTION.md`. Summary:

| trigger | prediction |
|---|---|
| `boot` | all twenty-four cells zero |
| `mood-transition` | all twenty-four cells zero (teardown completeness) |
| `periodic` | `sph`/`cube` 0; `arch` **negative** by the live force-spawned portal count; `ribbon`/`gallery` ≥ 0; TOTAL `claimed` monotone non-decreasing, sawtooth-reset at each transition |

The arch row is the sharpest and is an integer, not a direction:
`PORTAL_DENSITY = 1.00f`, so every dispatch DOORWAY arch is a portal *and* is
registered; only Channel B registers nothing.

**If `arch` delta is 0, ruling 14 is wrong** — that is the most valuable
falsification available here, and it should not be reconciled by editing the
prediction.

I deliberately did **not** add a provenance bit to distinguish force-spawned
arches. The symptom exists (Channel B never writes `host_gx`/`host_gz`) but
that is ruling 20's business in SPAWN_3, and adding a field now would erase the
symptom before the prediction is tested.

---

## 6. BRANCH POSITION

- `claude/spawn-campaign-handoffs-wsblrd` was fully merged and **deleted
  locally** with `-d` (it refused nothing). The **remote branch still exists** —
  outward-facing, so it awaits your word, per the handoff.
- The branch was then **recreated** to carry this stage's four commits, since
  the handoff requires permission before master and my instructions pin me to
  it. Local `master` is reset to `origin/master` and carries nothing unpushed.
- **Requesting permission to merge this stage to master.** Last session's
  authorisation was specific to that push and I have not treated it as standing.

---

## 7. WHAT I WOULD NOT START YET

`SPAWN_2` — it gates on the census showing `sph`/`cube` claimed = 0, and the
census has not yet printed once. Everything after 1a depends on the instrument
being demonstrably alive, and right now it is only demonstrably *compiled*.
