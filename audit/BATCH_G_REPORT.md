# BATCH G — THE WIRE + THE ERASURE — report

Cartridge: `the_board` (`incubator_dual`). The collision arc closes.
Master-direct. Deviations are REPORT, never improvisation.

**Preflight.** Not shallow. Base:
`323d4ae3fac3481fd779dd61b2b512c81d8e830f`, `git rev-list --count HEAD` =
1008 at base. LF-only, no BOM, no CR introduced. glaw1 GREEN at base and
every commit. Symbols, never FILE:LINE. Part 0 written whole before the
first edit.

---

## PART 0 — READ-ONLY

### [G0-a] The possessed path — THE HYPOTHESIS, VERDICT: **CONFIRMED in substance**

The three stations, pasted from the live tree:

**1. Where intent becomes movement** — inside `behavior_player_controlled`:
position is authored DIRECTLY from intent; velocity is bookkeeping:

```wgsl
        let speed = select(PAWN_SPEED, config.pawn_speed, config.pawn_speed > 0.0);
        agent.pos_x += world_vel.x * speed * dt;
        agent.pos_z += world_vel.y * speed * dt;
        ...
        agent.vel_x = world_vel.x * speed;    // ← OVERWRITTEN from intent EVERY frame
        agent.vel_z = world_vel.y * speed;
```

**2. Where the candidate forms** — same function, lines later: the moved
`agent.pos` IS the candidate, resolved and overwritten in place:

```wgsl
    if (coupling_active(COUPLING_TERRAIN_TO_PAWN_Y)) {
        let resolved = pawn_ground_resolve(vec2(agent.pos_x, agent.pos_z), prev_xz, prev_y, qi);
        agent.pos_x = resolved.x;
        agent.pos_y = resolved.y;
        agent.pos_z = resolved.z;
        if (resolved.w < 0.5) { agent.vel_x = 0.0; agent.vel_z = 0.0; }
    }
```

**3. Where occupier Δv lands** — in `update_player_agent`, AFTER the
behavior returns (so after the resolve has already run): the gather adds it
to `agent.vel`, and the K1b imposed-delta block applies
`(vel_now − voluntary_snapshot) · dt` to position once, ground-unresolved:

```wgsl
    let imp_v0 = vec2(agent.vel_x, agent.vel_z);       // before the gather
    ...
        { let o_r = occupier_contact(...); agent.vel_x += o_r.x; agent.vel_z += o_r.y; }
    ...
    agent.pos_x += (agent.vel_x - imp_v0.x) * signal.dt;   // K1b pos-add
    agent.pos_z += (agent.vel_z - imp_v0.y) * signal.dt;
```

**Does the Δv reach the candidate? NO.** `pawn_ground_resolve` has already
consumed a candidate built from intent alone. The Δv's only consumption is
the K1b pos-add — one frame, dt²-scaled, never persisted:
`agent.vel_x = world_vel.x * speed` erases it at the next frame's top.

**The arithmetic of the pass-through** (the hypothesis's mechanism, made
exact): at 60 fps the pawn walks `15 · dt ≈ 0.25 wu/frame` into the shaft;
the occupier push contributes `overlap · CONTACT_SPRING · dt² ≈
3 · 40 / 3600 ≈ 0.033 wu/frame` at deep overlap — out-walked ~8:1, capped
harder still by `CONTACT_IMPULSE_CAP`. A free agent keeps the same Δv in
persistent velocity (drag-decayed, integrated by `agent_post_step` every
frame), so it accumulates to walk-speed scale within ~10 frames — exactly
Jean's observation: visible on free agents, pass-through on the pawn.

**Precision on the hypothesis as worded:** the Δv is not *entirely*
unconsumed — K1b applies it once — but it never reaches the candidate,
never persists, and never meets the resolve. The prescription stands
unrefuted: **G1 wires the push into the candidate before
`pawn_ground_resolve`.** The wire's exact shape, dictated by the paste:
the push lands as `candidate += Δv · dt` after the intent integration
(both arms — an idle pawn should also be eased out) and before the
world-bound clamp, so the clamp clamps it and the slope law resolves it;
and the occupier call LEAVES `update_player_agent`'s gather — otherwise
the possessed slot would consume the push twice (once resolved in the
candidate, once unresolved via K1b). Free agents' path untouched.

### [G0-b] The control group's health — the second face, quantified

Pier ramp slope = `solid_height / (2 × edge_blend)` (the smoothstep rises
over 2·blend). Tier means, against `PAWN_MAX_SLOPE = 1.75`:

| population | height μ | blend μ | ramp slope | vs 1.75 |
|---|---|---|---|---|
| Column PILLAR | 1.5 | 0.4 | 1.88 | blocks — barely |
| Column DORIC | 1.0 | 0.3 | 1.67 | **WALKABLE RAMP** |
| Column ORNATE | 1.5 | 0.5 | 1.50 | **WALKABLE RAMP** |
| Antenna ANTENNA | 1.0 | 0.3 | 1.67 | **WALKABLE RAMP** |
| Antenna SQUAT | 1.5 | 0.4 | 1.88 | blocks — barely |
| Antenna COLOSSAL | 12.0 | 1.0 | 6.00 | blocks |
| Arch DOORWAY | 1.5 | 0.9 | 0.83 | **WALKABLE RAMP** |
| Arch STANDARD | 5.6 | 0.7 | 4.00 | blocks |
| Arch MONUMENTAL | 8.0 | 0.8 | 5.00 | blocks |

**Four of nine pier populations are already walkable ramps under the slope
law** (and the Gaussian σ puts borderline instances on both sides of the
line for two more). The pier "wall" was a step-law artifact; the slope law
correctly reads a plinth as a ramp. Report-only — it dies with the piers,
and it explains why Jean's pass-through has two faces: the wire's absence
(G0-a) AND a control group that was already leaking.

### [G0-c] The death census (every cut census-gated)

- **`structure_height_at`** — definition + ONE caller
  (`ground_formed_with_complexity`'s `+ structure_height_at(world_xz)`
  term). Dies whole.
- **`evaluate_pier`** — definition + ONE caller (`structure_height_at`).
  Dies whole.
- **`pier_instances`** (WGSL decl, binding g0:26) + **`PIER_TOTAL`**
  (WGSL const 68) — readers: `structure_height_at` only. The LAYOUT
  carrying 26 is **"Patch Gen Layout"** (8 entries), shared by exactly
  three pipelines — **11a `generate_patch_heights`, 11b
  `generate_patch_gradients`, 12 `generate_patch_cells`** — THE RECOMPILE
  RADIUS. No other layout carries 26 (it left the agents' layout at
  GROUND_CARD_1 [5c]).
- **`config.pier_count`** — C++ field (GPUDesignConfig), WGSL mirror line,
  `upload_pier_count` (targeted offsetof write), `stage_pier_count`, the
  boot zero. **The lockstep pin analysis:** `pier_count: u32` sits in
  `fade_color: vec3<f32>`'s tail slot; the next member
  `world_bound_min: vec2<f32>` has WGSL align 8, so deleting the WGSL line
  re-pads to the SAME offsets — nothing after it moves, `sizeof == 560`
  stands, `offsetof(lod_point_x) == 352` stands. The C++ twin therefore
  takes an explicit `uint32_t _pad_pier_retired;` in the slot (C++ float[2]
  is 4-aligned and WOULD slide without it — the pad is the honest C++
  mirror of WGSL's implicit one, and the standing pins are the proof
  nothing moved). C++ struct + WGSL mirror line + uploader land as ONE
  lockstep commit (G3) — the paired-commit exception, invoked and stated.
- **`write_pier`** — definition + decl + **6 callers**:
  `force_spawn_portal_arch` ×2 (grounded), `column_post_commit`,
  `antenna_post_commit`, `arch_post_commit` ×2.
- **`clear_pier`** — definition + decl + **7 sites**: `evict_arch` ×2,
  `evict_column`, `evict_antenna` (grounded), `force_spawn_portal_arch`'s
  pre-clear ×2*, and `reset_surface`'s all-slot sweep. (*verified in the
  edit; the census counts every call expression.)
- **`recompute_and_upload_pier_count`** + **`flush_pier_count`** +
  **`WorldState.pier_count_dirty`** — the deferral trio: definitions,
  decls, the streaming-conductor call site, the dirty writes in
  write/clear, and the boot-no-op prose in `reset_surface`.
- **`GPUPierInstance`** — struct + **48-byte static_assert** +
  `pierBuffer_` ("Pier Instances") + `upload_pier_slot` + the boot
  zero-init + the buffer validity check + **`cpuPiers_[PIER_TOTAL]`**
  (PatchSystemState) + `Dim::PIER_ARCH_BASE / PIER_COLUMN_BASE /
  PIER_TOTAL` + the WGSL `PierInstance` mirror struct + binding row 26 out
  of the L6 registry + the Patch Gen Layout/BindGroup entries.
- **`mark_patches_for_regen`, pier-driven sites** — **CENSUS CORRECTION to
  the Batch F report, stated loudly: there is ONE pier-driven site, not
  two.** The two callers tree-wide are `pyramid_post_commit` (the
  heightfield bakes pyramids — KEEP, this is the keep-list's "pyramids use
  the path", now verified by symbol) and `arch_post_commit` (pier-driven —
  dies). `column_post_commit` writes its pier WITHOUT marking regen (the
  patch bakes it at its own generation). The Batch F report's "column/
  antenna pier AABB" attribution was wrong; recorded here.
- **The pier param columns (G4's subject) — CENSUS VERDICT: THEY LIVE.**
  `ArchIdx::PIER_HEIGHT` is read by the arch MESH (`mp.pier_height` into
  `GPUArchMeshParams` — the visual legs), the doorway/portal vocabulary
  (mood's doorway width math, `force_spawn_portal_arch`), and the arch
  indoor rescale; `ArchIdx::PIER_PADDING`/`EDGE_BLEND` by the doorway math
  and the arch solid-half (footprint) path; `ColIdx::SOLID_PADDING/
  SOLID_HEIGHT/EDGE_BLEND` by `column_compute_solid_half` (the footprint),
  `column_write_active` (`ac.solid_height`), the indoor-rescale param
  lists, and the antenna twins. **write_pier was never the sole reader of
  any of them. G4 lands NO CUT** — and consequently the JSX designer
  mirrors of those columns stay too, truth-fix-free (they mirror live
  params). G4 is this paragraph.

### [G0-d] The keep list — verified present, untouched by any planned cut

- `mark_patches_for_regen` — kept; sole surviving caller
  `pyramid_post_commit` (verified by symbol, above).
- `ground_entries_dirty` — **8 writers**; only write_pier/clear_pier die,
  6 non-pier writers survive. Kept.
- The arch doorway vocabulary + portal trigger — `ArchIdx::PIER_*` live in
  mood's doorway math and `force_spawn_portal_arch`; the portal trigger
  reads arch state, not piers. Kept (G4's no-cut makes this automatic).
- The column/antenna/arch MESHES + gen pipelines — `mp.pier_height` feeds
  the arch mesh; the mesh-gen layouts (193–198) carry no pier binding.
  Kept.
- The occupier rows — the successor; untouched by the erasure, rewired
  only for the possessed slot by G1.
- `PatchPhase::NEEDS_REGEN` — stays (pyramids), its comment "(new pier in
  range)" truth-fixed in G3.

### [G0-e] FXC posture

Deletions are the safe direction, but two touches are named: (1) the three
patch-gen pipelines lose binding 26 from their shared layout — a layout
RESHAPE, recompiling exactly that radius; entry counts shrink 8 → 7 and no
pipeline's WGSL references the binding after G2, so Dawn's layout↔shader
validation stays green by construction. (2) G1 touches inside the
collision/ground chain's CALLER: the wire is a VALUE change on the existing
candidate — two `+=` lines and one already-compiled function call moved
earlier in the same kernel — zero new branches (L2's clause), and the
occupier block LEAVING the gather removes one block from the chain's
neighborhood. If [G:shader] hangs, G1 is the first suspect and this report
names it.

---

## ANCHOR VERIFICATION

| anchor | status |
|---|---|
| `agent.pos_x += world_vel.x * speed * dt;` (the intent integration) | **MATCHED** |
| `pawn_ground_resolve(vec2(agent.pos_x, agent.pos_z), prev_xz, prev_y, qi)` (the candidate) | **MATCHED** |
| the K1b imposed pos-add pair (`agent.pos_x += (agent.vel_x - imp_v0.x) * signal.dt;`) | **MATCHED** |
| the occupier block in `update_player_agent`'s gather (G1 removes) | **MATCHED** |
| `+ structure_height_at(world_xz)` in `ground_formed_with_complexity` | **MATCHED** |
| `const PIER_TOTAL: u32 = 68u;` / `struct PierInstance` / `@binding(26)` decl | **MATCHED** |
| "Patch Gen Layout" entries[4] = pier_instances (layout + bind group) | **MATCHED** |
| `static_assert(sizeof(GPUPierInstance) == 48, …)` | **MATCHED** |
| the six write_pier / seven clear_pier call expressions | **MATCHED** (enumerated in G0-c) |
| "the 2 pier-driven mark_patches_for_regen sites" (handoff) | **DIVERGED** — one is `pyramid_post_commit` (keep-list, not pier); ONE pier-driven site exists (`arch_post_commit`). Census correction recorded. |

---

## [G4] THE TIER COLUMNS — the census-gated NO-CUT (the whole of G4)

G0-c's census ruled before the first edit: `write_pier` was never the sole
reader of any pier param column. `ArchIdx::PIER_HEIGHT` feeds the arch MESH
(`mp.pier_height` — the legs' visual height), the doorway/portal vocabulary
(mood's width math, `force_spawn_portal_arch`), and the indoor rescale;
`PIER_PADDING`/`EDGE_BLEND` feed the arch footprint (`solid_half`);
`ColIdx::SOLID_PADDING/SOLID_HEIGHT/EDGE_BLEND` feed
`column_compute_solid_half` (the footprint), `ac.solid_height`, and the
rescale lists. **The tier rows keep every column; no F-5 pins were needed
because nothing positional moved; the JSX designer mirrors stay,
truth-fix-free** — they mirror live parameters. G4 is this paragraph and no
commit.

## COMMIT TABLE

| commit | hash | glaw1 | encoding |
|---|---|---|---|
| BATCH G: Part 0 — the hypothesis confirmed, the census corrected | `0a33733` | GREEN (base) | LF, no BOM, no CR |
| CONTACT_G1: the wire — the pawn's candidate consumes the body's word | `9133460` | **GREEN** | LF, no BOM, no CR |
| ERASE_G2: the bake forgets the piers (WGSL) | `08bf586` | **GREEN** | LF, no BOM, no CR |
| ERASE_G3: the pier machinery leaves the program (C++, lockstep pair) | `d3e8483` | **GREEN** | LF, no BOM, no CR |
| BATCH G: the report | *(this commit — includes the registry's declaration-count bookkeeping: 96 declarations over 93 slots, verified by count)* | **GREEN** | LF, no BOM, no CR |

Base `323d4ae`, master-direct. The paired-commit exception was invoked
exactly once, as Part 0 said it would be: `config.pier_count`'s C++ struct
edit and its WGSL mirror line left together in G3; G2 stayed pure-WGSL.

## THE RECOMPILE RADIUS, actually observed

The Patch Gen Layout reshape (8 → 7 entries, binding 26 retired) touches
exactly the three pipelines Part 0 named: `generate_patch_heights`,
`generate_patch_gradients`, `generate_patch_cells`. G1's value change
touches the two agent pipelines. No other pipeline's layout or shader
interface moved. glaw1 (the C++ TU) is GREEN at every commit; [G:shader]'s
full FXC recompile is Jean's, with G1 named first suspect if it hangs.

## THE RECLAIM, restated

Per patch bake at a typical live pier count (~12): **~786 K `evaluate_pier`
invocations and ~1.6 M transcendentals** (two per invocation, evaluated
BEFORE the spatial reject), across 65,536 texels; worst case 4.46 M
invocations at 68 slots. Plus the regen storm: every arch spawn re-baked
every GENERATED patch inside its inflated AABB. All gone. The 3,264-byte
buffer, binding 26, the 48-byte pin, and the FXC-sensitive bounded loop in
the bake left with it.

## GATE STATUS

- **[G:glaw1]** — table above, all GREEN.
- **[G:shader]** — Jean's full FXC recompile. G1 is the one value change
  inside the sensitive chain's caller — first suspect on a hang, named
  here. The erasure is deletion-direction across the measured radius.
- **[G:runtime-J]** — the five moves, after the full batch, one end build:
  (1) walk into a column — the pawn STOPS at the shaft by the rows alone
  (the piers are gone; this single move proves the wire end-to-end);
  (2) strafe — smooth slide; (3) the arch — clean through the span,
  refused at the legs, the portal still fires (the trigger reads arch
  state, never touched piers); (4) a column/antenna-dense area at speed —
  no regen hitching (the storm is gone; note columns never marked regen,
  so the hitching this move hunts was the ARCH pair's — now also gone);
  (5) the Batch E dune at two frame rates — unchanged.
- **[G:visual]** — ONE deliberate pixel change, named so it is seen and not
  hunted: **the plinth bumps vanish** — columns, antennas, and arch legs
  rise from unraised ground (their meshes are untouched; only the raised
  ground under them is gone). G0-b's table is why some of those plinths
  were walkable ramps anyway. Everything else is pixel-still.
- **[G:census]** — prediction: all twelve integers unchanged. Piers were
  never a census family and footprints never knew them (`register_
  footprint` was never called with pier data; the footprint registry is
  untouched by this batch).

## POST-BATCH FIX (Jean's boot log, the L6 witness firing as designed)

Jean's first boot after the batch failed at `CreateBindGroup("Patch Gen
BindGroup")`: *"In entries[7], binding index 0 not present in the bind group
layout."* The cause was G3's reshape applied asymmetrically — the LAYOUT
array was shrunk 8 → 7, but the BIND GROUP kept `std::array<..., 8>`, and
its zero-initialized 8th entry (binding 0, no buffer) is exactly what Dawn
rejected. Fixed: both arrays 7, indices dense 0–6, header comment corrected.

Worth recording for the next reshape: **glaw1 structurally cannot catch
this class** — an 8-array with 7 entries written is legal C++; the
boot-time bind-group validation (the L6 lockstep witness) is the gate that
exists for it, and it fired on the first boot with a message that named the
exact entry. The witness worked; the report records the debt it caught.

## CARRIED REGISTER (standing, report-only)

- Ribbon-head occupier avoidance — net-new behavior, never served by
  piers; its own session in the ribbon's era.
- [point:stale-readback-after-transition] → POINT_1.
- Forced mood-5 anchor ribbon + [ribbon:stale-tip-ref] → REQUEST_1.
- J2 (the adapted probe) — Jean-side, standing, non-blocking (the verified
  diff in audit/BATCH_E_REPORT.md applies to a cartridge.hpp region the
  erasure did not touch; re-verified: `git apply --check` still clean).
- The stale remote branch — Jean's one optional click, affects nothing.
