# ATLAS_1revB — U0″ MECHANISM AUDIT

U0″ is the clause the two shipped misses bought, and it earns itself on
the first run. **D3″ is sound. D2′ is sound. Neither can reach two of
the thirteen shadow vertex shaders**, because those two are drawn on a
different group-0 layout that carries neither binding.

Steps 1, 2, 5 and 6 pass. Step 4's premise needs a correction (harmless).
**Step 3 — the offset census — is the finding.**

No `src/` file was touched. HEAD `6bceca6`, held branch
`claude/ledger-regen-head-i4svuf`.

## VERDICT TABLE

| step | result |
|---|---|
| U0″.1 hygiene, subjects unmoved since `30c9a7c`, naga baseline | **PASS** — `git diff 30c9a7c..HEAD -- src/` empty; `naga world.wgsl` → `Validation successful` |
| U0″.2 next-free `g0` registry number, read not assumed | **PASS — 362** |
| U0″.3 the offset census | **STOP — the census is not closed by the render-entity group alone** |
| U0″.4 `minUniformBufferOffsetAlignment` at core default | **CORRECTION — the program requests adapter limits, not core. Harmless; see below.** |
| U0″.5 the five `instance_index` VSes receive zero edits | **PASS by construction** — D3″ touches no VS signature and no `DrawIndexed` argument |
| U0″.6 count check, before | **PASS — 19 `light_vp` refs** |

## U0″.2 — the registry number

The `g0` render band is documented `200–361` and currently tops out at
`render_ring_xforms = 361`. The next band (`atlas / cull outputs / orbs`)
starts at 390. **Next-free in band: `362`.**

Note the `+200` band is a *mirror* convention with a `static_assert`
behind it (`render_floating == floating_entities + 200`). `shadow_slot`
mirrors no compute binding, so the convention does not bind it; 362 is
free space inside the render band, not a mirror slot.

## U0″.3 — THE OFFSET CENSUS, AND WHAT IT FOUND

### The census as the handoff scoped it: 7 sites

Four bind groups are built on `renderEntityBindGroupLayout_`:
`renderEntityBindGroup_` (plan A), `renderEntityBindGroupPlanB_`,
`renderEntityBindGroupPlanC_` (all three via
`build_render_entity_group`), and `photographerRenderEntityBindGroup_`.

Every `SetBindGroup` that binds one of them:

| # | site | group | pass |
|---|---|---|---|
| 1 | `render_passes.hpp:299` | `render_entity_group()` | shadow, indoor tile head |
| 2 | `render_passes.hpp:330` | `render_entity_group()` | shadow, outdoor head |
| 3 | `render_passes.hpp:510` | `render_entity_group()` | main, restore after plan C |
| 4 | `gallery.hpp:1498` | `photographer_render_entity_group()` | snapshot head |
| 5 | `renderer.hpp:563` in `draw_orbs` | parameter | main (called from `orbs.hpp:806`) |
| 6 | `renderer.hpp:702` in `draw_patch_terrain_plan_slot` | parameter | main — **one site, called 3× with plan A/B/C** |
| 7 | `renderer.hpp:719` in `draw_patch_terrain_direct` | parameter | snapshot (called from `gallery.hpp:1502`) |

`renderer.hpp:944` (`draw_fade_overlay`) is **not** in the census — it
binds `mesh_gen_entity_group()`, a different layout. Verified.

**Seven code sites. That part of the handoff is exactly right.**

### The finding: two shadow VSes are not on this layout at all

`draw_shadow_all` ends by rebinding group 0 to the **gallery** pair for
the two artwork draws, and the ledger's Appendix 3 confirms their
pipeline layouts:

```
| Shadow Gallery Frame  | shadowGalleryFramePipeline_  | shadow_gallery_frame_vs  | galleryEntityBindGroupLayout_ → galleryTextureBindGroupLayout_ |
| Shadow Wall Painting  | shadowWallPaintingPipeline_  | shadow_wall_painting_vs  | galleryEntityBindGroupLayout_ → galleryTextureBindGroupLayout_ |
```

`galleryEntityBindGroupLayout_` has **exactly three entries**:

```
std::array<wgpu::BindGroupLayoutEntry, 3> entries{};
entries[0].binding = bind::g0::config;         Vertex|Fragment, Uniform
entries[1].binding = bind::g0::render_vp;      Vertex,          ReadOnlyStorage
entries[2].binding = bind::g0::render_camera;  Fragment,        ReadOnlyStorage
```

No `render_lighting`. No room for `shadow_slot`.

Both of those entry points are in the thirteen — they read
`out.clip_pos = render_vp.light_vp * …` today and must become
`shadow_light_vp(…)` under D2′. But `shadow_light_vp` reads
**`render_lighting`** (D2′) and **`shadow_slot`** (D3″), and neither is
in their pipeline layout.

**Consequence: Dawn rejects both shadow gallery pipelines at creation.**
A shader may only reference bindings its pipeline layout provides.

**And naga cannot catch this.** naga validates the WGSL *module* — the
declarations and the calls are all well-formed, so it reports
`Validation successful`. Pipeline-layout conformance is checked by Dawn
at `CreateRenderPipeline`, on Jean's machine, after the branch is
pushed. The campaign's own per-commit witness is blind to exactly this
class of error, which is why U0″.3 asked for the census.

## U0″.4 — the alignment premise, corrected

The handoff asks to confirm the request path leaves
`minUniformBufferOffsetAlignment` at the core default 256. **It does
not** — `console.hpp` requests the adapter's full limits wholesale:

```
wgpu::Limits adapterLimits{};
adapter.GetLimits(&adapterLimits);
deviceDesc.requiredLimits = &adapterLimits;
```

So the effective alignment is whatever the adapter reports.

**This is harmless and the 256 stride is safe regardless.** WebGPU
classes this limit as "better is lower", and an adapter must support at
least the 256 default — so the effective value is a power of two ≤ 256,
and 256 is a multiple of every such value. A 256-byte window stride is
legal on any conforming adapter. Reported because the handoff states a
premise the tree does not hold, not because it changes the design.

## U0″.5 — the five VSes

**Zero edits, by construction.** D3″ adds a binding read inside
`shadow_light_vp`; it adds no builtin, changes no signature, and
changes no `DrawIndexed` argument. `shadow_patch_terrain_vs`,
`shadow_pawn_vs`, `shadow_sphere_vs`, `shadow_monolith_vs` and
`shadow_gallery_frame_vs` keep their `instance_index` parameters and
their index expressions untouched. This is precisely the property that
made D3″ the right ruling over D3′-A.

## THREE WAYS OUT — Jean's ruling

### G1 — extend the gallery entity layout

Add `render_lighting` (Vertex, Uniform) and `shadow_slot` (Vertex,
Uniform, dynamic) to `galleryEntityBindGroupLayout_` and to both groups
built on it (`galleryEntityBindGroup_`,
`galleryPhotographerEntityBindGroup_`).

| | |
|---|---|
| layouts grown | **two** — render-entity 14 → 15 entries, gallery-entity 3 → 5 |
| offset census | **7 → 10 sites** (adds `render_passes.hpp:424`, `render_passes.hpp:527`, `gallery.hpp:1525`) |
| budget | gallery V uniform 1 → 3 of 12 (Table B: `Shadow Gallery Frame` / `Shadow Wall Painting` V read `1 / 1 / 1`) |
| ledger delta | beyond the handoff's four: two gallery rows, two gallery group entry counts |
| cost | the main-pass and snapshot gallery colour draws now pay a dynamic offset they never use |

### G2 — move the two shadow gallery pipelines onto the render-entity layout ✦

The gallery entity layout is a strict **subset** of the render-entity
layout for everything those two VSes use, and the binding *types* match
exactly:

| binding | gallery entity layout | render-entity layout |
|---|---|---|
| `config` | V\|F, `Uniform` | V\|F, `Uniform` |
| `render_vp` | V, `ReadOnlyStorage` | V\|F, `ReadOnlyStorage` |
| `render_camera` | F, `ReadOnlyStorage` | V\|F, `ReadOnlyStorage` |

So the two **shadow** gallery pipelines can take
`renderEntityBindGroupLayout_ → galleryTextureBindGroupLayout_` and lose
nothing. Group 1 (`galleryTextureBindGroupLayout_`, where
`painting_slots` and `painting_array` live) is unchanged.

| | |
|---|---|
| layouts grown | **one** — render-entity 14 → 15 |
| offset census | **stays 7** |
| ledger delta | matches the handoff's predicted four, plus the two pipelines' group-layout column |
| bonus | `draw_shadow_all` stops rebinding the gallery pair in the shadow path — **one fewer bind per light**, and it removes the only place group 0 changes mid-tile |
| cost | two pipeline layouts change in `renderer.hpp`; the colour gallery pipelines keep the gallery entity layout and are untouched |

**This is the one I would take.** It shrinks the campaign instead of
growing it, keeps the offset census at the number the handoff predicted,
and the type table above says it is safe.

### G3 — keep the two gallery shadow draws on today's mechanism

Leave them reading `render_vp.light_vp` and keep a per-tile copy alive
for them alone. **Rejected on its face**: it keeps the copy D4 exists to
kill, and it re-opens the two-owner surface D2′ was adopted to escape.
Recorded only so the option set is complete.

## ONE MORE PREDICTED DELTA, BEYOND U4″'s "EXACTLY"

U4″ lists four expected ledger changes and rules anything else a
FINDING. Adding a binding moves three more, unavoidably — and one of
them is a **gate**, not a report:

- `binding_registry.hpp`'s banner states `97 declarations over 94 slots`.
  Witness **`0b-1` parses that banner** and fails if the census
  disagrees. Adding `shadow_slot` makes it 98 / 95, so **the banner must
  be edited in the same commit** or the tool writes nothing (the witness
  gate blocks `emit`).
- witness `registry`: `94 constants over 3 namespaces` → 95.
- witness `0b-0`: `97 @group( occurrences, 97 declarations parsed` → 98.

These are consequences of the ruled design, not deviations from it, but
they are not in U4″'s list and would otherwise read as FINDINGs. Flagged
now so the regen is not misread later.

## STATE

| | |
|---|---|
| edits made | **none** — U0″ is read-only |
| U1″–U3″ | not run; the closure commit cannot land while two pipelines would fail creation |
| naga | installed (`naga-cli v30.0.0`), baseline green, ready as the per-commit witness |
| open question | **G1, G2 or G3.** Everything else in revB is verified and ready; U1″ is mechanical once this is ruled. |
