> **ARCHIVAL** — this document describes the tree as of its own
> date and is superseded. Do not cite as current. Live facts live
> in the code; adjudications live in `audit/` and
> `docs/HANDOFFS/OPTIMIZATION 1/`.

# BATCH E — THE RIDERS + THE SLOPE LAW (the collision arc opens) — report

Cartridge: `the_board` (`incubator_dual`). Riders 3/4 delegated by Jean to
chat-Claude's judgment (July 2026); the judgments arrived stamped in the
handoff and are executed as stamped. Deviations are REPORT, never
improvisation.

**Preflight.** `git rev-parse --is-shallow-repository` → `false` (the
unshallow was done in the Batch C/D session and holds). Base:
`05e0df18c8628e4d110515c8f01d1662b194d564`, `git rev-list --count HEAD` =
995. Working tree clean at base. **Refined git law honored: this batch works
directly on `master`** — no transient branch. The one standing exception,
`claude/batch-c5-prepared`, stays held (see [E4]). Encoding: LF-only, no BOM,
no CR introduced. glaw1 GREEN at base. Citations are symbols, never
FILE:LINE. Part 0 below was gathered and written whole before the first edit.

---

## PART 0 — READ-ONLY CENSUS

### [E0-a] The sky-exit skip census

**`evict_ribbon` (whole):**

```cpp
inline void evict_ribbon(MachineCtx* self,
    uint32_t slot, wgpu::Queue& queue) {
    auto& ar = self->ribbon_state_.active[slot];
    if (!ar.active) return;

    // Sky mode: the flown ribbon is pinned for the flight's duration.
    // ... SEAM[ribbon:sky-mode].
    if (slot == self->ribbon_state_.rendered_slot
        && (self->ribbon_state_.sky.mode || ar.wander)) {
        return;
    }

    // Decrement ref count — one anchor patch has been evicted.
    // Only fully evict when all referencing patches are gone.
    if (ar.ref_count > 1) {
        ar.ref_count--;
        return;
    }

    // Final reference gone — full eviction. ...
    unregister_footprint_for(self, PopFamily::RIBBON, slot);
    ar = ActiveRibbon{};
    self->ribbon_state_.gpu[slot] = GPURibbonState{};
    self->ribbon_state_.active_count--;
    if (self->ribbon_state_.rendered_slot == slot) {
        GPURibbonState empty{};
        self->gpuState_.upload_ribbon(queue, empty);
        self->ribbon_state_.rendered_slot = UINT32_MAX;
        ribbon_invalidate_head(self->ribbon_state_);
    }
    std::cout << "[Ribbon] EVICT slot=" << slot << "\n";
}
```

**The sky-exit edge block (whole), inside `ribbon_frame_tick`:**

```cpp
    // Sky mode just ended — release the pinned (now anchor-less)
    // ribbon so a fresh one can spawn. SEAM[ribbon:sky-mode].
    if (rs.sky.mode_prev && !rs.sky.mode) {
        uint32_t s = rs.rendered_slot;
        if (s != UINT32_MAX && rs.active[s].active) {
            rs.active[s] = ActiveRibbon{};
            rs.gpu[s] = GPURibbonState{};
            if (rs.active_count > 0) rs.active_count--;
            GPURibbonState empty{};
            c->gpuState_.upload_ribbon(queue, empty);
            rs.rendered_slot = UINT32_MAX;
            ribbon_invalidate_head(rs);
        }
    }
    rs.sky.mode_prev = rs.sky.mode;
```

**What the direct clear performs vs. what a real ribbon death performs:**

| duty (evict_ribbon full-eviction path) | direct clear | verdict |
|---|---|---|
| `unregister_footprint_for(RIBBON, slot)` | **absent** | **THE LEAK** — the rider's whole reason |
| `ar = ActiveRibbon{}` | present | ✓ |
| `gpu[slot] = GPURibbonState{}` | present | ✓ |
| `active_count--` | present (guarded `> 0`) | ✓ |
| `upload_ribbon(empty)` + `rendered_slot = UINT32_MAX` + `ribbon_invalidate_head` | present | ✓ (the exit is always the rendered slot) |
| `[Ribbon] EVICT` stdout | absent | diagnostic only |

**THE TIP-REF PROTOCOL — stated explicitly, as required.** Tip refs are
**patch-side** (`ActivePatch.entity_refs`, written by `record_entity` at
`dispatch_commit_ribbon` for each live tip patch, counted into
`ar.ref_count`). **`evict_ribbon` never touches patch-side refs either** — it
decrements only its own `ar.ref_count`, and the patch's refs are wiped
wholesale by `evict_patch_entities` (`entity_ref_count = 0`) in the eviction
that carried them. So on the ref question the direct clear and the evictor
behave **identically**: neither leaks a ref the other reclaims, and the rider
does NOT need to cover refs.

There IS a shared, pre-existing hazard worth naming (out of E1's scope, not
introduced by it): a ribbon that dies by *any* path other than its last
referencing patch's eviction leaves that patch holding a ref to a now-free
slot. With `MAX_RIBBON_INSTANCES == 1` the slot is certainly reused, so a
still-alive old tip patch evicting later calls `evict_ribbon` on the
*successor* ribbon — one premature decrement. Not a leak (the successor's own
footprint is released by that evictor), not a crash: a ribbon that dies one
patch-eviction early. Reported, not fixed here.

**THE PRIMARY ROUTE IS BLOCKED — twice — so E1 lands the stamped FALLBACK:**

1. **The pin.** `evict_ribbon` returns early when
   `slot == rendered_slot && (sky.mode || ar.wander)`. At the exit edge
   `sky.mode` is already false, but **`ar.wander` may be true** — the pin's
   own comment says a rendered wanderer is pinned the same way. Routed
   through the evictor, a wandering sky-ribbon would survive the exit. Today
   it is cleared. That is a behavior change, not a release.
2. **The stale refcount — decisive.** During the flight, every anchor-patch
   eviction calls `evict_ribbon` and hits the pin, which returns **before**
   the `ref_count` decrement. So at the exit `ref_count` still holds its
   commit-time value (1 or 2) even though those patches are gone. Routed
   through the evictor with `ref_count == 2`, the exit would merely decrement
   to 1 and return — **the ribbon would not be released at all**, defeating
   the exact purpose of the block.

Both are the handoff's named fallback trigger ("the evictor route contorted
by the sky pin, or missing a duty the exit needs"). Making the evictor fit
would mean pre-setting `ref_count = 1` and suppressing the wander pin at the
call — contortion, not a door.

**One mechanical fact the fallback must solve:** `ribbon_frame_tick` takes
`RibbonDeps*`, which carries `gpuState_` but **no `MachineCtx`** — and
`unregister_footprint_for` is declared on `MachineCtx*`. The release cannot
be written where the block currently sits without threading the machine face.
`sky.mode_prev` is read and written **only** by this block (verified
tree-wide), and `sky.mode` is written **only** by the input toggle — never
inside `ribbon_frame_tick` — so the edge block is relocatable without
changing the edge it detects. E1's shape follows from that.

*(A third direct-clear path exists — `release_finite_ribbons` — which also
skips the footprint release. It is NOT a leak: it runs at mood transition,
after `reset_surface` has already wiped `footprints_` wholesale. Recorded so
the next reader doesn't re-find it as a defect.)*

### [E0-b] Birth-into-mode reachability

The cube spawn-commit context is `cube_write_gpu(MachineCtx* c, const
EntityInstance& inst, wgpu::Queue& queue)` — the machine face, whole.

- **(i) `kite_mode` reachable?** **YES.** `MachineCtx` carries
  `CubeBehaviorsState& cube_behaviors_state_` (non-const), so
  `c->cube_behaviors_state_.kite_mode` is readable at spawn. Its sibling
  `cube_write_active` already reaches `c->cube_behaviors_state_.activeCubes_`
  through the same door.
- **(ii) The point reachable?** **YES.** `MachineCtx` carries
  `const PlayerState& player_`, so `c->player_.readback_x/z` — the same
  host-authored snapshot `corral_cubes` and the old toggle read — is
  available at spawn.
- **(iii) What does spawn write into `fe.pos`?**
  `fe.pos[0] = inst.cx; fe.pos[1] = fe.orbit_height; fe.pos[2] = inst.cz;`
  — the spawn position, with drift zero (`fe.drift` / `fe.drift_vel` are
  written all-zero two lines below).

**Verdict: (i) and (ii) both hold ⇒ E2 lands the PRIMARY mechanism**
(birth-into-mode at spawn). The fallback (3u at spawn) is not needed; the
sentinel round-trip is avoided, and because drift is exactly zero at birth
the captured offset is exact rather than algebraically-exact.

### [E0-c] PAWN_STEP_HEIGHT reader census

Every occurrence tree-wide (`rg PAWN_STEP_HEIGHT src/ --glob '!src/docs/**'`):

| site | kind |
|---|---|
| `const PAWN_STEP_HEIGHT: f32 = 0.5;` (pawn constants block, world.wgsl) | declaration |
| `pawn_ground_resolve` happy-path gate — `!moved \|\| y_tilt - prev_y_tilt <= PAWN_STEP_HEIGHT` | **comparison 1** |
| `pawn_ground_resolve` x-slide — `x_ok = (x_pair.y - prev_y_tilt) <= PAWN_STEP_HEIGHT` | **comparison 2** |
| `pawn_ground_resolve` z-slide — `z_ok = (z_pair.y - prev_y_tilt) <= PAWN_STEP_HEIGHT` | **comparison 3** |
| TUNING SURFACE DIRECTORY, Pawn §2.2 — "Max terrain step" | prose |
| STEER_GRAD block — "The wall is pawn_ground_resolve's PAWN_STEP_HEIGHT gate (a height STEP, not a gradient-magnitude const), so there is no shared steepness truth to re-point to" | prose ("the wall") |
| `pawn_ground_resolve`'s POLICY_WALKER_TILT header — "step-climb-safe heights for the PAWN_STEP_HEIGHT comparison" | prose |

**Exactly the expected three comparisons plus prose — no other reader.
PAWN_STEP_HEIGHT DIES in E3.** Three prose sites need the truth-fix, one more
than the handoff named (the POLICY_WALKER_TILT header); the extra is
recorded here rather than improvised past.

Note for the STEER_GRAD truth-fix: that comment's stated reason for
authoring `STEER_GRAD_LO/HI` locally — *"there is no shared steepness truth
to re-point to"* — **stops being true in this batch.** `PAWN_MAX_SLOPE` is a
gradient magnitude, directly comparable to the 0.7 / 1.4 whisper band. The
fix says so.

### [E0-d] The pier cost census (read-only — the erasure batch's input)

**Capacity and memory.**

| quantity | value | recipe |
|---|---|---|
| pier slots | **68** (`Dim::PIER_TOTAL`) | 4 head + 32 arch (`PIER_ARCH_BASE = 4`, slots 4–35 = 16 arches × 2 piers) + 32 column (`PIER_COLUMN_BASE = 36`, slots 36–67 = 32 columns × 1 pier) |
| `sizeof(GPUPierInstance)` | **48 B** (pinned by static_assert) | 44 data bytes + 4 pad — the comment calls it "compliance, not capacity" |
| pier buffer | **3,264 B** | 68 × 48 |
| binding row | **26** (`pier_instances`, binding_registry) | L6 mirror |

**Live write sites.** `write_pier` — **6 callers**: the portal arch's pier
pair in `force_spawn_portal_arch` (×2), and in the generic pipeline the
column post-commit, the antenna post-commit, and the arch pier pair (×2).
`clear_pier` — **7 call sites** (the six evictor/teardown counterparts plus
the `reset_surface` sweep over all `Dim::PIER_TOTAL` slots).
`recompute_and_upload_pier_count` is deferred through
`world_state_.pier_count_dirty`; `config.pier_count` is a **high-water mark**
(highest active slot + 1), so a fragmented slot table over-scans.

**Pier-driven regen.** `mark_patches_for_regen` has **2 pier-driven call
sites** — the column/antenna pier AABB and the arch pier-pair AABB, each
inflating by `half + edge_blend` and re-baking every patch the box touches.
This is the pier's *hidden* cost: every pier write dirties a
heightfield-sized region, not a slot.

**The bake's per-texel loop.**

```wgsl
fn structure_height_at(world_xz: vec2<f32>) -> f32 {
    var best = 0.0;
    let count = min(config.pier_count, PIER_TOTAL);
    for (var i = 0u; i < count; i++) {
        let h = evaluate_pier(world_xz, pier_instances[i]);
        best = max(best, h);
    }
    return best;
}
```

Called **once per texel** from `ground_formed_with_complexity`
(`... + structure_height_at(world_xz) + contrib_pyramids_at(world_xz)`).
Patch heightfield is `Dim::PATCH_HEIGHTFIELD_N = 256` per side ⇒
**65,536 texels per patch**; the live ring is
`Dim::MAX_ACTIVE_PATCHES = 289`.

`evaluate_pier`'s body per texel per slot: an `is_active` early-out; then
**`cos(-rotation)` + `sin(-rotation)` — two transcendentals evaluated per
texel per active pier BEFORE any spatial reject** — a rotate, a bbox reject,
up to **4 `smoothstep`s** for the blended mask, a `mix` and a `clamp`.

**The reclaimed-budget figure.** Per patch bake, worst case (all 68 slots
active): 65,536 × 68 = **4,456,448 `evaluate_pier` invocations**, of which
~8.9 M transcendental calls (2 per invocation) land *before* the spatial
reject. At a typical live count (`pier_count` ≈ the handful of arches and
columns near the player, say 12): 65,536 × 12 ≈ **786 K invocations per patch
bake**, ~1.6 M transcendentals. Multiply by however many patches a single
pier write dirties through `mark_patches_for_regen` — that product, not the
3,264 B of instance data, is what the erasure batch reclaims.

### [E0-e] FXC posture (L2, as it bears on E3)

`pawn_ground_resolve` **is** the collision/ground chain L2 point 2 names:
*"The collision/ground chain admits no new runtime branching."* Also in
force: instance structs in hot loops stay lean and byte-pinned (untouched
here), no texture-array stamps near the chain (none added), one
`DrawIndexedIndirect` per pass (untouched), buffer counts (untouched).

E3 is a **condition-expression swap**: the same three `if`/`let` sites, the
same branch count, no new control flow — each comparison gains one divide and
one `max`. The landed diff is stated against this in the E3 section below.

---

## ANCHOR VERIFICATION

| anchor (from the handoff's spec text) | status |
|---|---|
| `rs.active[s] = ActiveRibbon{}` (the sky-exit direct clear) | **MATCHED** byte-for-byte |
| `evict_ribbon`'s `unregister_footprint_for(self, PopFamily::RIBBON, slot);` | **MATCHED** |
| `if (!moved \|\| y_tilt - prev_y_tilt <= PAWN_STEP_HEIGHT) {` | **MATCHED** |
| `let x_ok = (x_pair.y - prev_y_tilt) <= PAWN_STEP_HEIGHT;` | **MATCHED** |
| `let z_ok = (z_pair.y - prev_y_tilt) <= PAWN_STEP_HEIGHT;` | **MATCHED** |
| `const PAWN_STEP_HEIGHT: f32 = 0.5;` (its "old home" — the pawn constants block) | **MATCHED** |
| `if (fe.follow_pawn == 2u) {` … `fe.drift = vec3(0.0); fe.drift_vel = vec3(0.0);` (E5's target) | **MATCHED** |
| `fe.follow_pawn = 0;` + `fe.pawn_offset[...] = 0` + `fe.target_x/z = inst.cx/cz` (E2's target, `cube_write_gpu`) | **MATCHED** |

---

## THE MECHANISM ACTUALLY LANDED (E1 / E2), and why

**[E1] — the stamped FALLBACK, not the primary.** E0-a ruled the evictor
route out on two structural grounds, either sufficient:

1. `evict_ribbon`'s pin spares a rendered **wanderer**. At the exit edge
   `sky.mode` is already false, but `ar.wander` may be true, so a wandering
   sky-ribbon would *survive* the exit that exists to end it.
2. Decisive: every anchor-patch eviction during the flight hit that same pin
   and returned **before** the `ref_count` decrement, so `ref_count` reaches
   the exit stale-high. The evictor would decrement it to 1 and return — the
   ribbon never released, the block's purpose defeated.

Fitting the evictor would have required pre-setting `ref_count = 1` and
suppressing the wander pin at the call: contortion, exactly the case the
handoff named. So the minimal owner-release landed instead — as one named
verb, `release_sky_exit_ribbon`, because the release needs the **machine
face** (`unregister_footprint_for` is declared on `MachineCtx*`) and
`ribbon_frame_tick` carries only `RibbonDeps`. The block moved verbatim plus
one line (the release) and now runs at the head of `phase_ribbon_tick`. The
edge is unchanged: `sky.mode_prev` is the verb's private state (sole reader,
sole writer) and `sky.mode` is written only by the input toggle — never
inside the tick the verb precedes. `evict_ribbon`'s pin comment is
truth-fixed to name the new releaser *and* to record why the pin makes it
mandatory (returning there skips the decrement, so the refcount protocol can
never finish a flown ribbon's death).

**[E2] — the PRIMARY.** E0-b confirmed both preconditions: `MachineCtx`
carries `cube_behaviors_state_` (so `kite_mode` is readable at spawn) and
`player_` (so the point is). `cube_write_gpu` now writes the live mode
directly — kited spawns get `follow_pawn = 1u`, `pawn_offset = (cx − px, 0,
cz − pz)`, `target := that offset`. Because drift is exactly zero at birth
the offset is **exact**, not algebraically-exact: no sentinel round-trip, no
capture frame. The fallback's precondition (E0-b(iii), `fe.pos` = the spawn
position) went unused and is recorded only as confirmed-true.

## THE ADAPTED PROBE (J2) — re-derived, applies clean, compiles

The dossier's preserved diff is a record of its day and no longer applies:
the `n=` column read `activeSphereCount_` / `activeCubeCount_`, **which died
in SPAWN_C1**. That is not a loss — the drift that column hunted (stored
count vs. live `.active` scan) is now *structurally impossible*, because the
stored counts no longer exist. `f=` / `gpu=` / `cb=` / `st=` remain, and
they are exactly the bridge-alive question: does the readback callback ever
run, and does the GPU's active set reach the CPU?

**Verified against the live tree**: `git apply --check` clean at the Batch E
head, and `glaw1` **GREEN with the instrument applied**.

```diff
diff --git a/src/cartridges/the_board/cartridge.hpp b/src/cartridges/the_board/cartridge.hpp
--- a/src/cartridges/the_board/cartridge.hpp
+++ b/src/cartridges/the_board/cartridge.hpp
@@ -1085,6 +1085,30 @@ namespace t7 {
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
+                        std::cout << "[FLOATER] sph f=" << fs
+                                  << " gpu=" << dbg_fb_gsph
+                                  << " | cub f=" << fc
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
@@ -1105,6 +1129,18 @@ namespace t7 {
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

**How to read it.** `cb` rising ⇒ the readback callback runs at all. `gpu=`
tracking `f=` ⇒ the GPU's active set reaches the CPU and the bridge is
ALIVE — §1 of the dossier closes. `cb` stuck at 0, or `gpu=` frozen while
`f=` moves, is the finding. Remove after reading: search
`DIAG_FLOATER_BRIDGE`, delete both blocks. **A dead bridge is a finding, not
a failure.**

## COMMIT TABLE

| commit | hash | glaw1 | encoding |
|---|---|---|---|
| BATCH E: the five censuses, before the first edit | `56ac819` | GREEN (base) | LF, no BOM, no CR |
| RIDER[ribbon:sky-exit-release] — the sky-exit owes the ground back | `715633f` | **GREEN** | LF, no BOM, no CR |
| RIDER[cube:spawn-mode-desync] — newborns join the live mode | `1c6cca6` | **GREEN** | LF, no BOM, no CR |
| SLOPE_LAW: the dune stops being a pier | `6452794` | **GREEN** | LF, no BOM, no CR |
| ANCHOR_E5: drift.y walks home at kite release | `015065c` | **GREEN** | LF, no BOM, no CR |
| SPAWN_C5 **(held, rebased — NOT landed)** | `822c56e` on `claude/batch-c5-prepared` | **GREEN** | LF, no BOM, no CR |

Base `05e0df1`. All five Batch E commits are **on master directly**, per the
refined git law. Encoding verified: LF-only, no BOM, no CR byte introduced in
any file this batch touched.

## [E4] HELD-BRANCH MAINTENANCE

`claude/batch-c5-prepared` was rebased onto the Batch E head and is **one
clean commit** (`822c56e`) on top of master — `git log master..` shows
exactly one. glaw1 **GREEN** on the rebased head. It remains **UNLANDED**.

Its stop conditions are now:

1. **E1 landed** — ✅ done in this batch (`715633f`). The sky-exit orphan
   that the Batch C/D verification pass found is closed, so deleting the
   per-patch sweep no longer strands the flown ribbon's ground.
2. **J1 silent** — ⏳ Jean's. Both halves: `entity_ref OVERFLOW` absent from
   every captured session log, AND one smallest-room session
   (`finite_radius = 1`), ≥5 min wandering. Silence on both ⇒ one word merges
   the branch and the sweep dies. Either firing ⇒ C5 stays out, and the
   overflow finding outranks it — that condition is unchanged and unweakened
   by E1.

## GATE STATUS

- **[G:glaw1]** — CC, per commit: the table above, all GREEN. The adapted
  probe also compiles GREEN with the instrument applied.
- **[G:shader]** — Jean's: full FXC recompile. E3 touches the sensitive
  chain by **condition swap only** — same three test sites, same branch
  count (`slope_passable` uses the non-short-circuiting bool `|` precisely so
  it adds none), one divide + one `max` per comparison, plus one
  `distance()` on the happy path. No new bindings, no new control flow,
  nothing added to the texture path. **If FXC hangs, the divide is the first
  suspect — and here is the pre-derived remedy, so nobody thins the
  mechanism under pressure:**

  ```wgsl
  // Zero-divide equivalent. blocked iff dh > max(floor, slope*dxz):
  fn slope_passable(dh: f32, dxz: f32) -> bool {
      return dh <= max(PAWN_SLOPE_NOISE_FLOOR, PAWN_MAX_SLOPE * dxz);
  }
  ```

  It is the same law — one comparison, no divide, no boolean combinator,
  and identical for every `dxz ≥ 0` except sub-1e-4 moves, where the divide
  form is marginally stricter. **Reported, not landed**: the handoff stamped
  the divide form and E0-e asked the landed diff to honor it. This is the
  swap to make if [G:shader] demands one.
- **[G:runtime-J]** — Jean's, three scripted gates:
  1. **Sky-exit census** — fly the ribbon in and out mid-world, then read
     the entity census: `ribn` delta 0 after exit, no orphaned claim.
     Prediction: delta 0. Before E1 the footprint outlived the body until
     its host patch happened to evict.
  2. **Newborn corral** — F7 ON → walk until cubes spawn → F6. Prediction:
     newborns corral **with** the flock; no origin-bound gliders.
  3. **The dune script** — one steep dune walked at two frame rates (vsync
     on/off): climbs both, same feel. One pier face: blocks both. One GoL
     zone walk: bow-wave unchanged (suppression is untouched by this batch).
     Prediction: the two frame rates now agree, which is the whole point —
     the old height test had `v·dt` inside the verdict.
  Plus Batch D's four moves if not yet run — with move 1 (F7 OFF) now
  **improved by E5**: the vertical no longer snaps, it settles.
- **[G:visual]** — nothing else moves a pixel.

## CARRIED FORWARD

- **RIDER[cube:spawn-mode-desync] is CLOSED** by E2 — remove it from the
  open-rider list.
- **The stale tip-ref hazard** (E0-a, reported not fixed): a ribbon dying by
  any path other than its last referencing patch's eviction leaves that patch
  holding a ref to a freed slot, and with `MAX_RIBBON_INSTANCES == 1` the
  slot is certainly reused — so a still-alive old tip patch evicting later
  decrements the *successor* ribbon. Not a leak, not a crash: one premature
  death. Pre-existing, shared by every non-patch-driven death path, and
  untouched by this batch. A candidate for the next rider list.
- **The pier cost numbers (E0-d)** are the erasure batch's input, above.
