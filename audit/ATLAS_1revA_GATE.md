# ATLAS_1revA — U0′ DELTA GATE (read-only)

U0′ clears on every count it asks for. **D2′ is sound and needs no
further work.** But the gate found a second collision, in a decree
ATLAS_1revA carried over from ATLAS_1 unchanged and that neither the
first gate nor this handoff re-examined: **D3's `firstInstance` channel
is already occupied, and six of the fourteen shadow draws are
instanced.** D3 as ruled would corrupt them.

No `src/` file was touched by ATLAS_1revA. U1′–U5 are untouched.

**Scope honesty.** This defect was visible in PASS_0's own Table G
extract and in the first gate's evidence. The first gate ran U0's six
listed steps and did not audit D3's mechanism, because D3 was not among
them. It should have been caught a round earlier.

## Provenance

| field | value |
|---|---|
| HEAD at gate | `30c9a7c8118405ed2729520554ebbef8aaf46cc1` |
| lineage since the first gate | `095bd61` (recon) → `aa9d674` (Jean's handoffs) → `30c9a7c` (TOGGLE_0 U1) |
| subject files touched since `17a0faa` | **none** — `world.wgsl`, `state.hpp`, `render_passes.hpp`, `mood.hpp` all byte-identical, so the first gate's findings stand unre-run |
| handoff | `docs/HANDOFFS/TETRIS/ATLAS_1revA_HANDOFF.md` |
| **naga** | **INSTALLED — `naga-cli v30.0.0`** via `cargo install naga-cli --locked`. Baseline: `naga world.wgsl` → **`Validation successful`**, exit 0, 50 ms. The per-commit witness this campaign mandates is live for the first time. |

## U0′ VERDICT TABLE

| step | result |
|---|---|
| U0′.1 hygiene, lineage, subject integrity | **PASS** — subjects untouched since the gate; recon findings stand |
| U0′.2 witness availability (`naga --version`) | **PASS — installed, and the baseline is green** |
| U0′.3 count check: 19 = 1 field + 13 VS + 1 fragment + 2 writes + 2 comments | **PASS — exact** |
| **D3's mechanism** (not a U0′ step) | **STOP — the channel is occupied** |

U0′.3 measured, not asserted:

```
total light_vp refs                                    19
  out.clip_pos = render_vp.light_vp   (shadow VS)      13
  let light_clip = render_vp.light_vp (fragment)        1
  (vp_data|photographer_vp).light_vp = (compute)        2
  light_vp: mat4x4<f32>,              (field)           1
  comments                                              2
```

---

## THE SECOND COLLISION — D3 cannot use `firstInstance`

### `firstInstance` shifts `instance_index`; it does not add a channel

In WebGPU, `@builtin(instance_index)` **starts at `firstInstance`** — it
is the instance ordinal biased by that argument, not a separate value.
There is no way for a WGSL shader to read `firstInstance` on its own and
subtract it back out. So `firstInstance = li` does not *name* the light;
it *renumbers every instance* in the draw.

The codebase already relies on exactly that semantic. `draw_shadow_all`,
band 1 of the terrain:

```
pass.DrawIndexed(c->gpuState_.patch_index_count_lod1_live(),
    c->world_state_.render_patch_count - c->world_state_.lod0_patch_count,
    0, 0, c->world_state_.lod0_patch_count);
                 ^^^^^^^^^^^^^^^^^^^^^^^^^ firstInstance, load-bearing
```

Band 1 uses `firstInstance` as an **index base** so its patches continue
band 0's numbering into `patch_instances[patch_id]`. The channel D3 wants
is already carrying data.

### Five of the 13 shadow VSes consume `instance_index` for indexing

| shadow VS | builtin it already declares | what it indexes with it |
|---|---|---|
| `shadow_patch_terrain_vs` | `@builtin(instance_index) patch_id` | `patch_instances[patch_id]` |
| `shadow_pawn_vs` | `@builtin(instance_index) inst` | `render_agents[inst]` |
| `shadow_sphere_vs` | `@builtin(instance_index) inst` | `render_floating.entities[inst]` |
| `shadow_monolith_vs` | `@builtin(instance_index) inst` | `render_floating.entities[inst]` |
| `shadow_gallery_frame_vs` | `@builtin(instance_index) iid` | the frame slot |

The other eight (`arch`, `column`, `palm`, `cactus`, `blade`, `shell`,
`ribbon`, `wall_painting`) declare no `instance_index` and would take the
builtin cleanly. U1′ would also have hit a hard WGSL error on the five:
a second parameter carrying an already-declared builtin is invalid, so
"each shadow VS gains `@builtin(instance_index) li : u32`" cannot be
applied literally to them in any case.

### Six of the fourteen shadow draws are instanced

D3's sketch is `DrawIndexed(count, 1, 0, 0, li)` — `instanceCount = 1`.
That is false for six shapes (`audit/BINDING_LEDGER.md` Table G,
re-read at HEAD):

| shadow pipeline | `instanceCount` | effect of `firstInstance = li` |
|---|---|---|
| Shadow Patch Terrain, band 0 | `lod0_patch_count` | patches shifted by `li` |
| Shadow Patch Terrain, band 1 | `render_patch_count − lod0_patch_count` | **collides with the existing `firstInstance`** |
| Shadow Pawn | `Dim::MAX_AGENTS` = 32 | reads `render_agents[li … li+31]` — shifted, and past the end |
| Shadow Sphere | `Dim::MAX_SPHERE_INSTANCES` = 8 | shifted, past the end |
| Shadow Monolith | `Dim::MAX_CUBE_INSTANCES` = 256 | shifted, past the end |
| Shadow Gallery Frame | `slot_high_water` | frame slots shifted |

For `li = 0` everything is byte-identical to today, so the **outdoor arm
is unaffected**. The damage is confined to the indoor arm at `li ≥ 1` —
which is precisely the arm ATLAS_1revA exists to fix, and precisely
where the visual gate would have caught it as garbage in the right-hand
tiles. The gate would have worked; the point is that it should not have
had to.

One mitigation is already in the tree: UMBRA_4's `cast_terrain=false`
means terrain is not drawn into spot tiles, so the two terrain rows
above never see `li ≥ 1`. Pawn, sphere, monolith and gallery frames do.

---

## TWO AMENDMENTS, PRICED — Jean's ruling

Both keep D1, D2′, D4, D5 and D6 exactly as ruled. They differ only in
how a draw names its light.

### D3′-A — stride the instance index

`firstInstance = li * LIGHT_STRIDE`, and each VS decodes:

```
let li   = instance_index / LIGHT_STRIDE;
let inst = instance_index % LIGHT_STRIDE;     // today's index
```

`LIGHT_STRIDE` must exceed the largest `instanceCount` (256, Shadow
Monolith); 1024 leaves room. Band 1 passes
`li * LIGHT_STRIDE + lod0_patch_count`, which decodes to today's value
exactly, so the outdoor terrain path is unchanged in behaviour.

| | |
|---|---|
| new bindings | none |
| new buffers | none |
| WGSL touched | the helper + 13 VSes, **5 of which need index decoding** |
| C++ touched | 13 draw sites + `draw_shadow_all`'s parameter |
| ledger delta | the `render_lighting` visibility cell only |
| risk | a wrong modulus is invisible to naga and to the type system; it surfaces only in the visual gate |

### D3′-B — a dynamic offset

WebGPU's own mechanism for "same binding, different window, per draw
inside a pass": mark one uniform binding `hasDynamicOffset = true` and
call `SetBindGroup(0, group, 1, &offset)` with `offset = li * 256`
between draw groups.

| | |
|---|---|
| new bindings | none new, but one existing binding gains `hasDynamicOffset` |
| new buffers | one small aligned VP window (256 B stride × 4) — or the existing lighting buffer if its layout permits a 256-aligned window |
| **WGSL touched** | **none at all** — no builtin, no decode, no helper; the VS reads its matrix as today |
| C++ touched | the layout entry, the bind-group calls in the shadow pass, and every other `SetBindGroup` of that group (the main pass shares it) |
| ledger delta | witness `0d-1` moves **0 of 8 → 1 of 8 uniform** — a headline number that has read zero program-wide since BUDGET_0 |
| risk | C++-side and mechanical; the shader cannot be wrong because it does not change |

**My reading, offered as input and not as a ruling.** D3′-B is the safer
of the two and the only one that leaves WGSL untouched, which matters
when the visual gate is the only instrument that can see a mistake. Its
cost is honest and visible: it spends the program's first dynamic
offset, and `0d-1` stops reading zero. D3′-A spends nothing measurable
but hides its failure mode in arithmetic that no witness in this repo
can check. If the zero in `0d-1` is a value rather than an accident,
D3′-A; if it is an accident, D3′-B.

D2′ itself is unaffected either way: the helper still selects
`render_vp.light_vp` outdoors and
`render_lighting.spots.lights[li].view_proj` indoors, and under D3′-B it
takes `li` from the dynamically-offset window instead of a builtin.

---

## WHAT LANDED THIS SESSION, AND WHAT DID NOT

**Landed on master:**

- `TOGGLE_0` U1 — `30c9a7c`, `console.hpp` only. The PIVOT_0d-ii chain
  site was verified verbatim before editing; it exists as debt 12's P1
  assumed. `disable_symbol_renaming` now rides the identical
  instance-descriptor road on every plan, so the reading is available on
  the working Vulkan boot. One guard is stated in that commit: if the
  toggle already appears in the pre-U1 nine, the control is void and
  another device-stage toggle must be picked. U2 retires it after the
  reading.

**Not landed:** no ATLAS_1revA unit. U1′ cannot be applied literally —
five VSes would take a duplicate builtin, which is a WGSL error — and
would be wrong where it did apply.

**Standing for the next session:** U0′.1, U0′.2 and U0′.3 are complete
and green; naga is installed and the shader baseline validates. D1, D2′,
D4, D5, D6 need no revisiting. The single open question is **which of
D3′-A or D3′-B replaces D3.** Once ruled, the campaign is mechanical:
U1′ (visibility + helper + selection), U2 (name the light, by the ruled
mechanism), U3 (restructure the passes), U4 (retire the staging trio),
U5 (ledger regen on master after merge).
