# ATLAS_1revB — ONE PASS PER SHADOW TEXTURE, THIRD RULING

Authority: `audit/ATLAS_1_RECON.md` (first gate) and
`ATLAS_1revA_GATE.md` @ `30c9a7c` (second gate). D2′ stands verified.
**D3 is struck**: `firstInstance` biases `instance_index` rather than
adding a channel (spec: the builtin is set from the instance ordinal,
which begins at `firstInstance`); five shadow VSes already consume
`instance_index` for load-bearing indexing, six shadow draws are
instanced, and terrain band 1 already uses `firstInstance` as an index
base. **D3″ is ruled in its place — the dynamic-offset form, in the
`li`-window shape specified below.**

Why this shape and not the gate's other two candidates, recorded so the
choice has one home: D3′-A (stride + decode) rewrites the five
load-bearing index computations and hides its failure mode in modulus
arithmetic no witness here can check. The pure matrix-in-window variant
of D3′-B would need the **sun's** matrix in window 0 — but the sun VP's
only writer is `compute_vp`, on the GPU, per frame; a CPU-filled window
re-creates the exact two-owner collision D2′ was adopted to escape. The
`li`-window carries only an index, written once at boot, immutable
thereafter: no owners, no cadences, no arithmetic. Its failure modes are
loud — a missed offset is a validation error, not a wrong pixel.

The dynamic-offset wallet reads 0 of 8 today. Ruled: that zero was an
accident of history, not a value. This is the mechanism's textbook case
— same binding, different window, per draw inside one pass — and it is
spent deliberately. Seven seats remain for the control-panel era.

Pricing unchanged: 96 → 32 MiB/frame at 4 lights (PASS_0 Q3.3). Fresh
native before-rows for the after-comparison: Quartet-4 N/S
`shadow 42.60 gpu, fps 11.2`; Cathedral-4 E/W `29.79, fps 13.5`;
Cathedral-3 `32.49, fps 14.7`; outdoor `13.9–14.4`.

## THE DESIGN — FINAL FORM

- **D1 — pass-per-texture.** Unchanged. Indoor: one pass per shadow
  texture for the lights it owns; `Clear`/`Store`, no `Load`. Outdoor
  unchanged.
- **D2′ — the matrix comes from where it lives.** Unchanged: helper
  `shadow_light_vp(li)` returns `render_vp.light_vp` when
  `render_lighting.spots.count == 0u`, else
  `render_lighting.spots.lights[li].view_proj`. `render_lighting`
  visibility `F` → `VF`.
- **D3″ — the draw's light arrives by dynamic offset.**
  - New buffer `lightSlotBuffer_`: 4 windows × 256 B (the
    `minUniformBufferOffsetAlignment` core default). Window `i` holds
    `ShadowSlot { li : u32 }` = `i`. Written **once at boot**; never
    again.
  - New uniform binding `shadow_slot` on the render-entity layout
    (`g0`, next-free registry number per L6), visibility **Vertex**,
    `hasDynamicOffset = true`, minBindingSize = 4.
  - Every `SetBindGroup` of that group, everywhere in the program, now
    passes exactly one dynamic offset: `0` outside the shadow tile
    loop; `li * 256` per light-group inside it.
  - WGSL: one binding declaration; the helper reads `shadow_slot.li`.
    **No builtin additions anywhere. No `DrawIndexed` argument changes
    anywhere. The five `instance_index`-consuming VSes are untouched.**
- **D4 — the staging trio dies.** Unchanged.
- **D5 / D6 — untouched by decree.** Unchanged, including `compute_vp`,
  `sample_shadow_pcf`, `calc_directional_light`, tile geometry, PCF,
  caster sets, formats.

## U0″ — MECHANISM AUDIT (the clause two shipped misses bought; standing
for all future handoffs: every ruled mechanism gets its own recon step)

1. Hygiene; record HEAD; confirm subjects unmoved since `30c9a7c`
   (else re-verify the second gate's tables). naga baseline green at
   the branch point.
2. Registry: the next-free `g0` binding number, read, not assumed.
3. **The offset census:** enumerate every `SetBindGroup` call site of
   the render-entity group across all passes (main, both shadow arms,
   snapshot, any other). Report the count. Each site gains the
   one-element offset array; a missed site fails loudly at validation,
   but the census exists so none is missed.
4. Confirm the request path leaves `minUniformBufferOffsetAlignment` at
   the core default 256 (no limit change is part of this campaign).
5. Confirm by diff-plan that the five VSes
   (`shadow_patch_terrain_vs`, `shadow_pawn_vs`, `shadow_sphere_vs`,
   `shadow_monolith_vs`, `shadow_gallery_frame_vs`) receive **zero
   edits** to their signatures or index expressions.
6. Count check as in revA: 19 `light_vp` refs before; after U1″ the 13
   VS sites read `shadow_light_vp(...)` and refs fall to 7.

## THE UNITS (held branch; one commit each; naga after every WGSL commit)

- **U1″ — closure commit:** `state.hpp` (buffer, boot write, layout
  entry with `hasDynamicOffset`, registry constant, `render_lighting`
  `F`→`VF`) + `world.wgsl` (`shadow_slot` declaration, the helper, 13
  call-site rewrites) in one commit — the tree never validates with
  half of this landed. Immediately after: the offset arrays at every
  site from U0″.3 (same commit if the group cannot bind without them —
  it cannot; a dynamic-offset layout requires the offset at every set).
- **U2″ — restructure the passes:** per texture, one `BeginRenderPass`;
  per owned light `SetViewport` + `SetScissorRect` +
  `SetBindGroup(..., { li * 256 })` + `draw_shadow_all(c, pass, false)`
  — **signature unchanged**; delete the per-tile copy.
- **U3″ — retire the staging trio** (buffer, accessor,
  `stage_spot_vps`, its call in `apply_mood_spot_lights`, registry
  constant).
- **U4″ — post-merge, master: ledger regen.** Expected delta, exactly:
  one new uniform V row (`shadow_slot`, dynamic); `render_lighting`
  visibility cell `F`→`VF`; witness `0d-1` **0 → 1 of 8**; render-family
  V-stage uniforms 5 → 7 of 12. Anything else is a FINDING.

## WITNESSES

Held branch; naga per WGSL commit (now installed and green). Jean:
glaw1 + boot + the three-gate walk with **two named checks** — (a)
light 0's indoor shadow direction (absence of the D2 failure), and (b)
**the right-tile occupants**: pawn, sphere, and monolith shadows
present and correctly placed under lights 1–3 (absence of the D3
failure — shifted indexing would show as missing or misplaced entity
shadows in exactly those tiles). Merge is Jean's; branch dies; U4″
follows; preview deploy + one Pixel window. Expected observables:
indoor `shadow_pass` cpu toward outdoor's ~0.15; native Quartet-4
GPU row measured against 42.60/29.79 with no promise beyond the pass
count halving.

## STOP CONDITIONS

- Any U0″ count or verbatim mismatch, before or after edits.
- Any edit touching the five VSes' signatures or index expressions,
  any `DrawIndexed` argument, or anything in D5/D6's list.
- Validation errors at any set site after U1″ — enumerate, fix within
  the census, never suppress.
- naga red on any commit.
- The merge is Jean's, after the walk.
