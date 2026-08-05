# C6 — THE 8-FIT (held branch, land-gated)

One edit: demote the `field_head_poses` window from read-only storage to
uniform in the room-family stack, bringing compute-stage storage entries from
9 to 8 — WebGPU default. Chosen per the C6 note's own table: it is the one
candidate whose shape the note verifies (`array<vec4<f32>, 400>` = 6,400 B,
vec4 stride — uniform-legal by inspection, far under 64 KiB), on the tree's
precedent of `state.hpp:4386-4398` (tier-gains + figure-profiles demoted for
the same reason). `field_forces` stays storage (read_write output); the
occupier windows stay untouched.

## GIT LAW

- Work on the existing held branch `claude/cut-1-limits-fit` (at `f62c519`).
  Commit there; do NOT touch master. Jean authorizes the merge and deletes the
  branch afterward.
- LF-only, no BOM. CC never builds; Jean gates `glaw1` + boot on the branch.

## REGISTER

Bind by content, not line numbers. Read every span before editing; STOP on
mismatch, report, await ruling.

## GATES (report all before editing; STOP if any fails)

1. **Alignment.** Report the `field_head_poses` BufferBinding — parent buffer,
   offset, size. The offset must be ≡ 0 mod 256
   (`minUniformBufferOffsetAlignment`). Misaligned → STOP (the fallback is the
   occupier merge, designed separately).
2. **Reader/binding census of the parent buffer.** Every layout entry and
   BindGroup entry that binds `headPosesBuffer_` (any window), and every WGSL
   declaration reading it. Expected: the g2/Room layout entry is the only
   binding of this window; the WGSL var `field_head_poses` is read by the
   field kernel (`write_field_forces`) and any room-family kernels. All
   readers go through the ONE module-scope declaration, so the address-space
   change is total or nothing — confirm no second WGSL declaration aliases
   the region.
3. **Uniform count.** Room-family compute-stage uniform entries after the
   demotion: report the number; expected 8 of the 12 working cap (the note
   records 7 today).
4. **Writer path.** Confirm the buffer is CPU-written (`WriteBuffer`) and
   never GPU-written; a GPU writer would forbid uniform usage on that region.

## THE EDIT (one commit)

1. `state.hpp` — the Room layout entry for `field_head_poses`:
   `BufferBindingType::ReadOnlyStorage` → `Uniform`.
2. `state.hpp` — `headPosesBuffer_` creation: usage gains `Uniform`
   (keep existing Storage|CopyDst flags — other windows may still bind as
   storage per gate 2's census).
3. `world.wgsl` — the module-scope declaration:
   `var<storage, read> field_head_poses : array<vec4<f32>, 400>` →
   `var<uniform> field_head_poses : array<vec4<f32>, 400>` (bind the exact
   current spelling from the tree).
4. `binding_registry.hpp` — annotate the g2 constant's comment (storage →
   uniform); binding NUMBER untouched per L6.
5. RECOUNT in the commit body: room-family compute-stage storage entries —
   must read **8**. Also restate the uniform count from gate 3.

Commit: `C6: field_head_poses demoted to uniform; room family fits default
limits (8 storage)`

## FOR JEAN

- Gate on the branch: `glaw1` + boot — behavior must be pixel-identical (an
  address-space change only).
- On green: merge to master, delete `claude/cut-1-limits-fit` (and the stray
  `claude/port-0-seam-census-*` if still standing).
- After the merge, exceedance 1 is closed; exceedance 2 closes in OPT_1b
  (radius 8→7 — note: 15², not the 16² previously stated; centered windows
  have odd sides).
