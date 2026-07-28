# POINT_1 — THE POINT GETS ITS HOUSE — report

Cartridge: `the_board` (`incubator_dual`). Master-direct. Deviations are
REPORT, never improvisation.

**Preflight.** Not shallow. Base: `ea1e18d` — **the REQUEST_1 head, as the
handoff requires** (REQUEST_1 landed first: `e212dda` → `33b9a78` →
`f494ba8` → `ea1e18d`). LF-only, no BOM, no CR introduced. glaw1 GREEN at
base and every commit. Symbols, never FILE:LINE. Part 0 written whole
before the first edit. **[G:shader] is n/a — CPU-side entirely; stated
explicitly.**

---

## PART 0 — READ-ONLY

### [P0-a] The writer census, pasted

The documented expectation, quoted from the tree: the trio's declarations
in `PlayerState` read *"THE POINT's world X (host-authored)"* /
*"the point's bubble sensor, on the possessed slot's wire"*, and
`SEAM[spine:P5]` names the readback state machines + gen counter as the
guard. Every author found, and they are exactly the documented set:

1. **The P5 harvest, sole steady-state author** — `phase_witness_harvest`:
   the pawn-host arm (`if (point_.host == PointHost::PAWN)` →
   `readback_x/z = p.pos_x/z`, and `readback_portal_trigger =
   p.portal_trigger` in both hosts) and the camera-host arm
   (`readback_x/z = cam->pos[0]/[2]` under `PointHost::CAMERA`).
2. **The TEARDOWN reset** — present on the transition path (portal
   crossings included), inside the TEARDOWN phase after the teardown
   verbs: `readback_portal_trigger = -1; readback_x = 0.0f;
   readback_z = 0.0f;`. **It writes ZERO, not the authored spawn
   position** — and it is correctly ordered (TEARDOWN runs in the update
   spine; the first `stream_patches` of the new world runs in the render
   spine after it). The reset itself is not the hole.
3. **The portal door's consume** — `readback_portal_trigger = -1` after
   the trigger fires.

### [P0-b] The first-capture order — VERDICT: **hypothesis 2, exactly, with the mechanism one level sharper**

The ordering facts, pasted:

- `world_state_.world_gen++` sits **at the top of TEARDOWN** (its SEAM
  comment says so verbatim), before the mirror reset and before
  `gpuState_.reset_player_agent(...)` uploads the new pawn — so the GPU
  buffer is re-authored synchronously in the same phase, before any
  new-world capture encodes (R11 sits later in the render spine). A
  callback **in flight** (state MAPPING) captured the OLD gen and is
  dropped correctly.
- **The hole**: the readback state machine is COPIED → MAPPING → callback,
  and the gen guard is bound **at MapAsync-issue time**, not at copy time:

  ```cpp
  if (pawnReadbackState_ == PawnReadbackState::COPIED) {
      pawnReadbackState_ = PawnReadbackState::MAPPING;
      gpuState_.agent_state_readback_staging().MapAsync(
          ...,
          [this, gen = world_state_.world_gen](...) {   // ← gen bound HERE
              ...
              if (gen == world_state_.world_gen) {      // ← passes with OLD bytes
                  std::memcpy(agent_state_.slots, data, ...);
                  ...
                  player_.readback_x = p.pos_x;         // ← the poison
  ```

  A copy staged in the OLD world (frame N encodes agent→staging, state :=
  COPIED) that is *mapped* in the NEW world (frame N+1's harvest sees
  COPIED and issues MapAsync with `gen = the NEW gen`) delivers old-world
  bytes **under a fresh gen, passing the guard** — the handoff's words
  made mechanism. The callback then memcpy's the entire old agent buffer
  over the freshly reseeded slots and writes the OLD pawn position
  (868,−873) over the just-reset mirror. Streaming centers there; the
  first arrivals — gallery (868,−873), ribbon (925,−981) — spawn ~1300 wu
  from the true point; the next capture/harvest cycle corrects to origin.
  Every line of Jean's log is accounted for.
- The same COPIED window exists on all THREE machines: the pawn harvest,
  the camera harvest (the camera-host author), and the floater readback
  (whose stale delivery would reconcile fresh mirrors against old-world
  actives — same disease, different organ).

**The fix P0-b's paste dictates (both of the handoff's expected shapes,
each doing the half it can):**

1. **Gate the first capture of a new world**: at TEARDOWN, beside
   `world_gen++`, cancel any stale STAGED copy — for each of the three
   machines, `if (state == COPIED) state = IDLE;`. A COPIED staging is
   never mapped, so no old bytes can arrive under the new gen; a MAPPING
   machine is left alone (its callback carries the old gen and drops
   itself — and forcing IDLE there would let the poll double-map an
   already-mapping buffer). The stale data problem dies at its root.
2. **The reset becomes an authored write**: the teardown writes the point
   mirror synchronously to the authored spawn position —
   `Idle::PAWN_POS_X/Z`, the same values `reset_player_agent` /
   `reseed_player_body` author — not zero. The continuity law: at a
   teleport the CPU is the author of the new present, and every streaming
   consumer that runs before the first fresh harvest reads the true
   position rather than a placeholder.

Zero behavior change outside the transition window; the harvest remains
the sole steady-state author.

### [P0-c] The reader census — **34 sites, 6 modules, classified**

| role | sites | face |
|---|---|---|
| streaming center + recenter + LOD banding/stage + patch sorts + visibility cylinder | 12 (patch_system: the bubble center, `update_center`, the LOD/band/sort calls, `update_entity_draw_visibility`'s cylinder) | MachineCtx |
| entity distance cull (arch/column/antenna reupload culls) | 6 (spawn_engine ×3 pairs) | MachineCtx |
| agent respawn ring + possession search | 4 (agents ×2 pairs — `respawn_evicted_agents`, the possession-nearest scan) | AgentsDeps |
| corral ring + birth-into-mode (E2's offset capture) | 4 (cube_behaviors: `corral_cubes` pair, `cube_write_gpu` pair) | CubeDeps ­/ MachineCtx |
| ribbon: nearest-active adoption + away-orientation at select | 4 (ribbon_frame_tick pair, `select_ribbon_for_patch` pair) | RibbonDeps / MachineCtx |
| gallery wall-frame placement | 2 (`fill_slot_wall_frame`) | GalleryDeps |
| the portal door (bubble sensor read + consume) | 2 (cartridge, spine-resident) | direct member |

*(count: 34 read expressions of `readback_x/z/portal_trigger`; the REQUEST_1
fulfillment verb does NOT read the point — the anchor position is
seed-derived — so it adds nothing to this census.)*

**The funnel's mechanical scope**: the trio moves into `PointState`; the
five faces that carry `player_` for these reads — MachineCtx, AgentsDeps,
CubeDeps, RibbonDeps, GalleryDeps — gain `const PointState& point_`,
threaded at the composition root's five init sites; the InputDeps face
already carries `point_` (the host toggle). PlayerState keeps
`possessed_slot`, `fpv_mode`, `aura_presence`.

### [P0-d] The point record today

`contracts/point.hpp`: `PointState { PointHost host; PointBubble bubble; }`,
instance `point_` at the composition root (spine-resident, beside
PlayerState). The banner's REALIZATION paragraph says *"POSITION lives in
the HOST's GPU storage … read through point_pos() on the GPU and the
host-routed P5 harvest (readback_x/z) on the CPU; no separate point buffer
was needed."* **No naming collision** — `x`/`z`/`portal_trigger` are free
in PointState. **One ordering constraint the move must respect**:
`point.hpp` is a contract with no includes beyond `<cstdint>` — the trio
is three scalars, so the move adds no include and no ordering pressure.
**One truth-fix obligation**: the banner's "(readback_x/z)" parenthetical
and the "no separate point buffer" sentence — the GPU half stays true; the
CPU mirror now lives HERE, and the banner must say so.

---

## ANCHOR VERIFICATION

| anchor | status |
|---|---|
| the teardown reset trio (`readback_* = -1/0/0`) | **MATCHED** (zeros, ordered before new-world streaming) |
| `world_gen++` at top of TEARDOWN + its SEAM[spine:P5] comment | **MATCHED** |
| the harvest's `[this, gen = world_state_.world_gen]` closures ×3 | **MATCHED** — gen bound at MAP time; the finding |
| the COPIED → MAPPING → callback state machines ×3 | **MATCHED** |
| `PlayerState` trio + "THE POINT's world X (host-authored)" comment | **MATCHED** |
| `PointState { host, bubble }` + the REALIZATION banner | **MATCHED** |
| "the P5 harvest is the sole author; teardown reset + portal consume are the spine's only other touches" (documented expectation) | **MATCHED** — the writer census found exactly that set |

## COMMIT TABLE

*(appended as the commits land)*

| hash | step | content |
|---|---|---|
| `b8bab65` | P0 | Part 0 — the four censuses + the P0-b verdict (map-time gen is the hole), committed before any edit |
| `77915e9` | P1 | the seam — three `COPIED → IDLE` cancels at teardown (`world_gen++` side) + the authored reset (`point x/z = 0`); MAPPING left alone (its callback bound the OLD gen and self-rejects) |
| `bf5e35f` | P2 | the funnel — the trio moves home to `PointState`; five faces gain `const PointState& point_`; nine-file rename; every touched comment truth-fixed |

## GATES

| gate | status |
|---|---|
| [G:glaw1] | **GREEN at every commit** (b8bab65 report-only; 77915e9 GREEN; bf5e35f GREEN) |
| [G:shader] | **n/a — CPU-side batch**, stated explicitly per the handoff: no WGSL file touched, no binding change, L6 registry untouched |
| [G:runtime-J] | Jean's two checks at a fixed seed: (1) the log's scenario re-run — walk far, portal-cross, first arrivals all near the true point, none at the old coordinates; (2) ten minutes across hosts — streaming, corral, kite, respawn, portals byte-identical. |
| [G:census] | **Prediction: all integers unchanged, all deltas zero everywhere** (REQUEST_1 already retired the last −1). |

## ENCODING

All touched files LF-only, no BOM, no CR (verified by the house sweep
before each commit).

## THE RG PROOF — zero readers of the old names

```
$ rg -n "readback_x|readback_z|readback_portal_trigger" src/ --glob '!src/docs/**'
(no matches — exit 1)
```

Docs (`src/docs/**`) keep the old names as history — they narrate past
batches and are not readers.

## [P3] THE CHARTER PARAGRAPH — drafted for Jean's stamp

*(report-only; not committed to the charter document without Jean)*

> **THE POINT.** The point is the program's reference position — one
> place on the board that means "where the witness is," host-arbitrated:
> when the pawn hosts, the point is the possessed body's position; when
> the camera hosts, it is the camera's ground shadow (pos.xz — the
> witness altitude stays GPU-only and is not to be invented). Its record
> is `PointState` (contracts/point.hpp): the position mirror `x/z`, the
> bubble sensor `portal_trigger`, the host, and the bubble — one home,
> one face. **Authors**: the P5 HARVEST (sole steady-state author,
> host-routed), the TEARDOWN reset (the authored present at world birth,
> ordered before any new-world streaming, with the first-capture gate
> canceling any stale COPIED capture from the old world), and the portal
> door's consume (`portal_trigger = -1` after acting). Nothing else
> writes it, ever. **Consumers** (the P0-c census, 34 read sites in six
> modules): the streaming bubble (center, recenter, LOD banding, patch
> sorts, visibility cylinder), the entity distance culls, the agent
> respawn ring and possession search, the corral and kite rings and E2's
> birth-into-mode offset capture, the ribbon's nearest-active adoption
> and away-orientation, the gallery's wall-frame placement, and the
> portal door. **The standing rule, now structural**: PRESENCE FOLLOWS
> THE POINT — anything that spawns, streams, culls, or gathers "near the
> player" reads the point through its module face, never a body's
> position directly; the type system now enforces what the comment law
> used to ask politely.

## CLOSING

POINT_1 complete: the seam is closed at the first-capture gate (P1), the
record lives in its semantic home behind one face (P2), and the charter
paragraph awaits Jean's stamp (P3). Behavior contract: byte-identical
outside the transition window; the funnel moved names, not behavior.
