> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# BATCH D — ONE_ANCHOR_1, the anchor law — report

Cartridge: `the_board` (`incubator_dual`). Executed by CC against the handoff
stamped by Jean (chat, July 2026), immediately after Batch C on the same line.

**Preflight** (shared with Batch C, same session): repository was shallow,
unshallowed before any ancestry reasoning (`true` → `false`). Batch C base:
`5a9fafd`; Batch D begins at the Batch C head. LF-only, no BOM, no CR
introduced. glaw1 GREEN at the Batch D base. Citations are symbols, never
FILE:LINE. Part 0 below was gathered and written whole before the first edit.

---

## PART 0 — READ-ONLY CENSUS

### [D0-a] Layout

`GPUFloatingEntityState` (realization/state) is `alignas(16)`, **208 bytes**,
pinned by `static_assert(sizeof(GPUFloatingEntityState) == 208)`. The WGSL
mirror `FloatingEntityState` (world.wgsl §2.1) matches field-for-field at the
same offsets, 208 total, arrayed as `FloatingEntityArray` ×
`TOTAL_FLOATING_SLOTS` (264 = 8 spheres + 256 cubes). The readback staging and
every bind-entry sizing derive from `Dim::TOTAL_FLOATING_SLOTS *
sizeof(GPUFloatingEntityState)` — no independently authored byte count
anywhere.

**Spare fields: the struct tail carries `_pad1` (offset 200) and `_pad2`
(offset 204) — exactly 8 bytes.** (`plasticity` consumed the former `_pad0` at
196 in CONTACT_2.)

**Decision: HEADROOM EXISTS.** `target_x` / `target_z` take the two pads at
200/204. The struct does NOT grow; sizeof stays 208; the sizeof pin stands
unchanged; staging and bind sizings are untouched (they derive from sizeof).
The C++/WGSL rename is still a paired lockstep edit (the L3 mirror law), made
inside D1's commit, with offsetof pins added for the two new fields.

### [D0-b] cx/cz reader census

`ActiveCube.cx / .cz` (contracts/floaters), every read and write tree-wide:

| site | kind |
|---|---|
| `cube_write_active` | spawn-time write (`ac.cx = inst.cx; ac.cz = inst.cz;`) |
| `corral_cubes`, anchor mode | read (glide start) **and** write (ring target) |
| `toggle_cube_kite_mode`, kite-ON arm | read (offset capture `cx − px`) |
| `toggle_cube_kite_mode`, kite-OFF arm | write (`cx = px + offset`) |

Nothing else touches the fields: `reconcile_cube_mirror` reads only `.active`
/ `last_alloc_time` (D0-d), the census reads only `.active`
(`census_scan_active`), the dispatch funnels only `.active`/`.slot`. One WGSL
comment names them (the stale-mirror paragraph in `update_cube` — the defect's
own documentation; dies in D1).

**This is the expected set — with one precision the handoff's parenthetical
missed: the spawn-time write lives in `cube_write_active`, not
`cube_write_gpu`** (the sibling that writes `fe.anchor` on the GPU side; the
two are the same spawn-commit pair). The kite-OFF write is the fourth member,
inside the same corral/kite family the batch rewires. **Verdict: the census is
exactly the corral/kite set ⇒ `ActiveCube.cx/.cz` and
`SEAM[cube:cx-cz-mirror]` DIE in D2.**

### [D0-c] pawn_offset[][] and upload_cube_pawn_offset — caller census

`CubeBehaviorsState.pawn_offset[][]` (CPU): read/written by `corral_cubes`
(kite-mode from + target), read/written by `toggle_cube_kite_mode` (ON
capture, OFF reconstruction), read by `tick_cube_corral_animations` — nothing
else. `GPUState::upload_cube_pawn_offset`: called by
`tick_cube_corral_animations` (kite arm) and `toggle_cube_kite_mode` (ON arm)
— nothing else. **All callers die in D2 ⇒ both die.** (The GPU field
`pawn_offset` inside `GPUFloatingEntityState` is a different thing — the kite
offset PARAM the kernel walks; it stays.)

**Consequence the handoff did not name, recorded before the cut:**
`GPUState::upload_cube_anchor`'s sole caller is
`tick_cube_corral_animations`'s anchor arm. When tick dies, the setter is an
orphan. It dies in D2 with the divergence recorded in the commit message —
same disposition as C3's registry row: the minimal mechanical completion of
the stamped cut, not a new decision.

### [D0-d] reconcile_cube_mirror — the body

```cpp
inline void reconcile_cube_mirror(CubeBehaviorsState& cs, CubeDeps* c, const GPUFloatingEntityState* data) {
    float now = c->time_state_.seconds;
    // Cubes: slots [CUBE_SLOT_OFFSET, TOTAL_FLOATING_SLOTS)
    for (uint32_t i = 0; i < Dim::MAX_CUBE_INSTANCES; i++) {
        bool gpu_active = (data[Dim::CUBE_SLOT_OFFSET + i].is_active != 0u);
        // cube active-slot mirror owned by CubeBehaviorsState (cube_behaviors.hpp)
        if (cs.activeCubes_[i].active && !gpu_active &&
            (now - cs.activeCubes_[i].last_alloc_time) > SPAWN_PROTECTION_S) {
            cs.activeCubes_[i].active = false;
        }
    }
}
```

**It does NOT touch cx/cz.** It reconciles only the active flag (with the P5
spawn-protection window). It is untouched by this batch.

### [D0-e] Sphere same-species

`ActiveSphere` carries **no position mirror at all** — its fields are
patch/host coordinates, `last_alloc_time`, `active`. `SphereProp::ANCHOR_X/Z`
are seed-property indices consumed once at spawn (the position derivation
feeding `sphere_write_gpu`'s one-time `fe.anchor` write); no post-spawn CPU
writer or reader of the sphere anchor exists. The freeze toggle is
`config.freeze_sphere` — a GPU uniform read by the kernels (`sphere_frozen()`)
that pauses motion; it neither reads nor writes anchors. Spheres have no
corral and no kite. **Expected clean, confirmed clean: the ninth twin has no
sphere sibling. Disposition is the D3 paragraph below — no edits, no rider.**

### [D0-f] FXC banner

world.wgsl's header carries the law index; the FXC law is **L2
(src/docs/LAWS.md)**, honored by structure:

1. hot-loop instance structs stay lean and byte-pinned;
2. the collision/ground chain admits no new runtime branching;
3. no texture-array stamps near that chain (they hang FXC);
4. one `DrawIndexedIndirect` per render pass;
5. storage buffers ≤ 10 / uniform buffers ≤ 12 per stage.

Constraints restated against this batch: the floater struct does NOT grow
(D0-a — the 208 pin stands, satisfying 1); `update_cube` is not in the
collision/ground chain, and D1 adds one sentinel branch of the same shape as
the existing `follow_pawn == 2u` block plus a branchless-in-spirit two-arm
walk — no new bindings, no new buffers, no texture stamps, no new indirect
draws. [G:shader] (Jean's full FXC recompile) remains the gate; if it hangs,
D0-a's no-growth verdict means struct size is NOT the suspect — the new
branches are, and the report says so rather than thinning silently.

---

## ANCHOR VERIFICATION

| anchor | status |
|---|---|
| kite-release block beginning `if (fe.follow_pawn == 2u) {` (update_cube) | **MATCHED** byte-for-byte |
| `── Analytical home ───` comment (update_cube) | **MATCHED** — one precision: it sits directly BELOW the release block, not above it; the handoff's "above" reads the other way. Same pair, same place. |
| `corral_cubes` whole body | **MATCHED** (read whole; ring math + anim arming + mirror writes as the stamped evidence describes) |
| `toggle_cube_kite_mode` whole body | **MATCHED** (CPU offset capture ON / 2u sentinel OFF) |
| `tick_cube_corral_animations` whole body | **MATCHED** (eased upload_cube_pawn_offset / upload_cube_anchor per mode) |
| `CubeBehaviorsState` block `CubeCorralAnim corral_anim[` … `pawn_offset[` | **MATCHED** byte-for-byte |

---

## REALIZATION DECISIONS (reported, not improvised)

1. **Plasticity leak vs the walk — the one gap in the stamped design.** The
   CONTACT_2 plasticity leak is itself a GPU-side anchor AUTHOR: each frame it
   moves `anchor.xz` by `drift.xz × λ·dt` (permanent relocation — the shove
   law). If `target` did not move with it, the new walk would read
   `target − anchor ≠ 0` after every shove and drag the cube back — silently
   repealing CONTACT_2 and breaking "hands off ⇒ bit-still" under λ > 0.
   D1 therefore applies the leak to `target_x/z` and `anchor.xz` **as a
   pair**: `target − anchor` is invariant under the leak, so a resting cube
   stays resting (rest identity is preserved structurally), a mid-glide shove
   translates the remaining glide rather than restarting or fighting it, and
   λ = 0 remains bit-exact. This is the minimal completion of "one control
   law, many authors": the leak authors the pair, the target door stays the
   only goal door.

2. **Target init home.** The ONE home is the **spawn-commit write**:
   `cube_write_gpu` initializes `fe.target_x/z := inst.cx/cz` — the same
   values it writes into `fe.anchor.xz`, so at rest target == param and the
   glide term is exactly zero from frame 1. The kernel never re-inits; the
   sentinels RE-TARGET (2u: target := captured anchor; 3u: target := captured
   offset), which is the mode-switch glide-cancel the design states. The init
   rides D1 (the handoff lists "the init path" in D1's contents), and D2's
   commit message records the choice per its bullet.

3. **The corral stdout line** keeps its shape but loses the
   `over CUBE_CORRAL_DURATION s` clause — the constant dies with the CPU
   animation; the pace is now CUBE_GLIDE_TAU, a WGSL module const the C++
   line cannot read. The line still reports count, mode, and radius.

4. **D1→D2 intermediate state.** After D1 alone, the old CPU corral/kite
   authors still run against the new kernel walk (they upload leaping anchors
   the walk then pulls toward stale targets). glaw1 is GREEN at D1 and the
   batch is gated at its head by [G:shader]/[G:runtime]/[G:visual] — the
   sequencing is the handoff's own two-commit structure, noted here so nobody
   bisects into D1 and reads the fight as a defect.

---

## [D3] SPHERE DISPOSITION (the recorded paragraph — no edits)

Spheres do not carry the ninth twin's disease, because they never grew the
organ it lives in. `ActiveSphere` has no position mirror — no cx/cz, nothing
for a kernel write to stale-ify. The sphere anchor is written once, at spawn
(`sphere_write_gpu`, from the `SphereProp::ANCHOR_X/Z` seed draws), and no
CPU code reads or writes it afterward; the GPU side never re-anchors a
sphere (no release sentinel, no plasticity leak on the sphere path — the
leak is inside `update_cube` only). The freeze toggle (`config.freeze_sphere`,
read kernel-side as `sphere_frozen()`) pauses motion without touching
anchors. Spheres have no corral and no kite, so there is no author whose
goal could leap. **Same-species verdict: clean. No rider is needed, and none
is specced.** If spheres ever gain a corral-class author, the target door
built in D1 is the pattern to reuse — the fields are already in the shared
struct, zero for spheres, inert.

## COMMIT TABLE

| commit | hash | glaw1 | encoding |
|---|---|---|---|
| BATCH D: the six censuses and four rulings, before the code | `540d971` | GREEN (base) | LF, no BOM, no CR |
| ANCHOR_D1: the walk — targets in the kernel | `327501c` | **GREEN** | LF, no BOM, no CR |
| ANCHOR_D2: the C++ rewire — authors write targets, nothing else | `1ed6727` | **GREEN** | LF, no BOM, no CR |

## GATE STATUS

- **[G:glaw1]** — CC, per commit: table above, all GREEN.
- **[G:shader]** — Jean's: full FXC recompile. D0-f's posture: the struct
  did NOT grow (the spare-field variant IS the landed variant), so if FXC
  hangs, suspect the new sentinel branch shape in `update_cube` — and
  REPORT; the mechanism is not to be thinned silently.
- **[G:runtime]** — Jean's, the four moves at a fixed seed:
  1. F7 ON → walk 100+ wu → F7 OFF → zero jump (both sentinels capture from
     the true present, drift included).
  2. F6 (anchor mode) → cubes glide FROM WHERE THEY VISIBLY ARE — the walk
     starts at the live anchor; there is no from-field to be stale.
  3. F6 again mid-glide → smooth retarget, no restart snap — exponential
     approach has no clock to reset.
  4. F4 curlfield, let drift build, F7 ON → zero jump (sentinel 3 subtracts
     drift at capture).
  Rest: hands off ⇒ bit-still — target == param at spawn, after both
  sentinels, and under the plasticity pair-leak; the glide term is exactly
  zero everywhere reachable.
- **[G:visual]** — the corral ring forms as before (CUBE_CORRAL_RADIUS
  unchanged); the feel ≈ the old 4 s ease; CUBE_GLIDE_TAU (world.wgsl,
  beside the cube constants) is the dial if not.

---

## VERIFICATION ADDENDUM (post-batch adversarial pass)

Independent verifiers briefed to refute the batch. No blocker on the landed
mechanism; three findings sharpen the predictions, and one is a NAMED RIDER:

- **RIDER[cube:spawn-mode-desync]** — specced, not improvised (the D3
  pattern). `cbs.kite_mode` is a global CPU flag; per-cube mode truth lives
  only on the GPU. A cube SPAWNED while kite mode is ON is born
  `follow_pawn = 0` (`cube_write_gpu` writes it unconditionally), so a
  subsequent corral — which keys target shape on the global flag — hands
  that mode-0 newborn a ring OFFSET, which the mode-agnostic walk applies to
  its ANCHOR as absolute coordinates: it glides toward the ring around the
  WORLD ORIGIN. The desync predates this batch (the old corral uploaded
  `pawn_offset` to a mode-0 cube — a silent no-op); the target door turned
  the dormant desync into visible wrong motion. Two candidate closures,
  Jean's pick: (a) spawn into the live mode — `cube_write_gpu` (or the
  spawn commit) uploads sentinel 3u when `cbs.kite_mode` is on, so newborns
  join the flock; (b) make the target encoding mode-agnostic — always
  absolute, the mode-1 walk subtracting `point_xz` kernel-side. Until the
  rider lands: F6 after spawning-while-kited mis-corrals the newborns
  (smooth glide, wrong destination — no snap, no crash).
- **[G:runtime] move 1 (F7 OFF), prediction scoped**: xz is preserved
  bit-exactly; **drift.y is discarded at release** — the 2u semantics the
  kernel always had, untouched by this batch. Under PhaseWave (a vertical
  force, amplitude 30) a cube can hold ~10 wu of vertical drift, and F7 OFF
  snaps it. The four scripted moves (curlfield, planar) don't reach it;
  PhaseWave + F7 does. Comments truth-fixed to say xz-exact; folding
  drift.y into a decay at release is a design change — a rider if wanted.
- **[G:runtime] move 4 (curlfield + F7 ON), prediction scoped**: the xz
  capture is exact (algebraically; a few f32 ULPs). On SLOPED ground the
  ground query moves from `pos.xz` (anchor arm, ruling 1) to
  `pos.xz − drift.xz` (kite arm) at the toggle frame, so home.y can step by
  the ground difference over drift.xz — the kernel's own F7 paragraph
  already concedes this; it is the ANCHOR_2 seam (ruling 1 on the kite
  arm), not this batch's. Zero-jump holds unconditionally in xz, and in y
  on locally flat ground.
