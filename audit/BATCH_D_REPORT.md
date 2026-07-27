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

## COMMIT TABLE

*(appended as the commits land — Part 0 above was committed before the first
edit, per the order law)*
