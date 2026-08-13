# ATLAS_1revB — U0″ MECHANISM AUDIT

> **ANNOTATION, added after the units landed (`abb7d55`).** Jean's
> direction was to rule the open question myself and finish rather than
> hand back a fourth gate. **G2 was adopted.** U1″, U2″ and U3″ are on
> the held branch `claude/ledger-regen-head-i4svuf`, naga green at every
> WGSL commit. What changed against the text below, and one thing this
> gate did not foresee, are recorded in **THE CLOSE** at the foot of this
> file. Everything above it stands as written at gate time.

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


---

# THE CLOSE — what was ruled, and what the units found

Added after `abb7d55`. The gate above stopped at a three-way question;
this records the answer and the two things the implementation turned up
that the gate had not.

## The ruling: G2

Adopted as recommended. The two **shadow** artwork pipelines now take
`renderEntityBindGroupLayout_ → galleryTextureBindGroupLayout_`; the
colour gallery pipelines keep the gallery entity layout untouched.

It paid what the gate predicted and one thing more: `draw_shadow_all`
no longer rebinds group 0 at all, so group 0 stops moving mid-tile. That
mattered more than it looked — see the second finding below.

| gate prediction | outcome |
|---|---|
| one layout grows, not two | render-entity 14 → 15 entries; gallery entity untouched |
| offset census stays at 7 | 7, closed |
| `draw_shadow_all` sheds a bind per light | it did |
| the type table makes the swap safe | `config` Uniform, `render_vp` / `render_camera` ReadOnlyStorage — identical on both layouts |

## Two things the units found that the gate had not

**1. `shadow_light_vp()` takes no parameter.** revB's D2′ names the
helper `shadow_light_vp(li)` while D3″ says it "reads `shadow_slot.li`".
Those cannot both be literal. It is written as
`fn shadow_light_vp() -> mat4x4<f32>`, reading the binding internally —
which is what makes D3″'s promise of *no signature changes anywhere*
reach the helper too, and what lets `draw_shadow_all` keep its signature
as U2″ requires.

**2. The group-1 bind cannot be hoisted to the merged pass head.** The
obvious economy in U2″ — bind the shadow texture group once per pass,
since it never varies within one — is wrong, and I wrote it that way
before catching it. `draw_shadow_all`'s tail rebinds group 1 to the
gallery **texture** group for the two artwork draws, so a hoisted bind
would be stale for the *second* light sharing that pass: its
non-gallery draws would read the gallery texture group. Both groups are
re-set per light. Bind count is unchanged from the per-tile version;
only the attachment traffic falls.

This is the same class of defect as the gate's own finding — a
group-0/group-1 assumption that holds per-pass but not per-light-group
once passes merge — and it is invisible to naga for the same reason.

## The after-counts

| check | predicted | actual |
|---|---|---|
| `light_vp` references after U1″ | 7 | **8** — the seven predicted, plus one comment line written into the helper's banner. The 13 VS reads are gone. |
| WGSL `@group` declarations | 97 → 98 | 98, matching the registry banner |
| `LoadOp::Load` in the tree | 0 | **0** — every load op in `render_passes.hpp` and `gallery.hpp` reads `Clear` |
| `spot_vp_staging` references | 0 | 0 code references; two comments record the retirement |

## What U4″ will see

Beyond revB's four expected deltas, the three the gate flagged did move
and had to: `binding_registry.hpp`'s banner (97/94 → 98/95, because
witness `0b-1` **parses** it and a failing witness blocks the write),
witness `registry` (94 → 95 constants), and witness `0b-0` (97 → 98
declarations). Two more follow from G2, and are consequences of the
adopted ruling rather than deviations from it:

- Appendix 3's group-layout column for `Shadow Gallery Frame` and
  `Shadow Wall Painting`: `galleryEntityBindGroupLayout_` →
  `renderEntityBindGroupLayout_`.
- Those two pipelines' Table B stage rows move with their new layout.

`0d-1` moves `0 → 1 of 8 uniform` as ruled. The dynamic-offset wallet is
open, deliberately, with seven seats left.

## What is not claimed

No compile gate ran here. naga validates the WGSL module and was green
at every commit that touched it — but naga does not check pipeline
layouts, which is precisely the class this gate's finding belonged to,
and it does not check C++ at all. `glaw1`, the boot, and the three-gate
walk with its two named checks (light 0's indoor shadow direction; the
right-tile occupants) remain the real witnesses, and they are Jean's.
The merge is Jean's after the walk; `U4″` follows on master.


---

# U4″ DRY RUN — run on the branch before the build, `07bb9b0`

The card puts U4″ after the merge. I ran it early, to a scratch path,
because **the ledger tool is a static parser of `state.hpp`,
`binding_registry.hpp`, `renderer.hpp` and `world.wgsl` — it witnesses
exactly the class naga cannot.** Nothing was written to
`audit/BINDING_LEDGER.md`.

**Exit 0. All 41 witnesses PASS.** The ones that earn their keep here:

| witness | what it proves about this campaign |
|---|---|
| `0a-1`, `0a-1b` | the 14 → 15 array-size edits are consistent, and every `entryCount` matches its array |
| `0a-6` | all four render-entity groups are still a bijection with the 15-entry layout |
| `0b-1` | the banner I edited parses and agrees with the census: **98 declarations over 95 slots** |
| `0c-0c`, `0c-4` | **G2's layout swap resolves** — every pipeline resolves to layouts `state.hpp` creates, and every layout is still bound at one group index |
| `0d-3` | **every layout entry has an access-compatible WGSL declaration at its (group, binding)** — `shadow_slot` 362 exists, typed compatibly, reachable |
| `0d-1` | **1 of 8 uniform**, exactly as ruled |

That is a real gate on the naga-blind class, short of a compiler. It does
not replace glaw1 — it parses, it does not compile — but a
bind-group/layout mistake of the kind U0″ found would have shown here.

## The nine expected deltas — all present, all as predicted

Including `render-family V uniforms 5 → 7`, verified on a sample row:
`Pawn Entity (Chess Pawn) / V` moves `uniform 5 / 5 / 3` →
`7 / 7 / 3`, storage unchanged at `6 / 6 / 2`, F stage byte-identical.

## THREE MORE — the tenth, eleventh and twelfth

### 10. The gate row moved. This is G2's cost, and I did not price it.

```
was:  tightest is Update Player Agent (0D, 1 thread) / C at uniform 10 of 12
now:  tightest is Shadow Gallery Frame / V at storage 7 of 8
```

`Shadow Gallery Frame / V` and `Shadow Wall Painting / V` move
`uniform 1 / 1 / 1, storage 2 / 2 / 2` → `uniform 7 / 7 / 3, storage 7 / 7 / 2`.

The arithmetic: a pipeline layout's charge is the concatenation of its
groups. Before, those two were `galleryEntity` (1 V-visible storage:
`render_vp`) + `galleryTexture` (1: `painting_slots`, `Vertex|Fragment`)
= 2. Now they are `renderEntity` (6 V-visible storage) + `galleryTexture`
(1) = **7**.

**They are charged 7 storage seats to use 2** (`actual` reads 2). That is
the charged-vs-actual gap the binding ledger exists to surface, and I
walked into it: my G2 pricing counted layouts, census sites and binds,
and never ran the stage budget. G1 would not have done this — those two
would have stayed at storage 2 and the tightest row would still be the
compute room.

**My ruling: keep G2, and record the constraint.** It passes; `actual` is
2, so there is no real pressure behind the number; and switching now
means growing a second layout and three more offset sites on a branch
that has never been compiled — more surface to get wrong, for headroom
nothing is currently asking for. But the constraint is now real and
narrow, and belongs in front of the next campaign:

> **Adding a Vertex-visible storage binding to EITHER
> `renderEntityBindGroupLayout_` or `galleryTextureBindGroupLayout_`
> pushes `Shadow Gallery Frame / V` and `Shadow Wall Painting / V` to
> 8 of 8.** The next one after that does not fit. If that day comes
> before anything else forces the question, the cheapest answer is to
> give those two pipelines their own five-entry layout (`config`,
> `render_vp`, `render_camera`, `render_lighting`, `shadow_slot`) rather
> than to unwind G2.

### 11 & 12. The tool's prose goes false when the tightest row is not the room family

Two auto-generated sentences template room-family boilerplate around the
**program-wide** tightest number. That was silently correct while the
room family held the tightest row. It no longer does, and the
regenerated ledger would assert two things that are not true:

**Table D**, verbatim from the dry run:

```
The room family's storage stage stands at 7 of 8. Any new storage binding
reachable from update_player_agent / update_other_agents / update_sphere /
update_cube needs a demotion to pay for it.
```

The room family stands at **6** of 8, not 7. Verified directly:
`Update Player Agent (0D, 1 thread) / C` reads `storage 6 / 6 / 4` in
**both** the committed ledger and the dry run — the compute room did not
move at all this campaign.

**Findings §, the tightest-row paragraph**, verbatim:

```
**Shadow Gallery Frame / V at storage 7 of 8** — the ROOM FAMILY, over the
three concatenated groups … roomLayout_, shared by shadow_gallery_frame_vs,
shadow_wall_painting_vs.
```

Wrong family, wrong layout (`roomLayout_` is the compute room's; these
two are on `renderEntityBindGroupLayout_ → galleryTextureBindGroupLayout_`),
and wrong group count (two, not three).

**This is an instrument defect, not a campaign one** — it is in
`tools/binding_ledger.py`'s emitter, it predates this branch, and it was
merely dormant. It is reported and **not fixed**: the tool is outside
this campaign's scope, and a prose fix wants its own unit and its own
regen. But it means **U4″ as written would commit two false sentences**,
so it should be fixed before or with the regen rather than after.

## What this changes for the sitting

Nothing about the build or the walk — those stand exactly as the card
writes them. What it buys is that U4″ is now a known quantity: twelve
deltas, nine expected, three named above, and every witness green before
the compiler has ever seen the branch.
