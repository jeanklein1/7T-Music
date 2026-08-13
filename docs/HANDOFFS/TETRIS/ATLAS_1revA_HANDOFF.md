# ATLAS_1revA — ONE PASS PER SHADOW TEXTURE, RE-RULED ON THE GATE'S EVIDENCE

Authority: `audit/ATLAS_1_RECON.md` @ `095bd61`. The original D2 is
**struck**: `light_vp` is a single slot time-multiplexed within the
frame — `compute_vp` (GPU, R10, ungated in practice) writes the sun VP
every frame, and the per-tile copy was both delivery and protection.
D2's array would have given slot 0 two owners at two cadences and
rendered light 0's tile from the sun's viewpoint. **D2′ is ruled in its
place**, and it is leaner than the original: it widens nothing and
deletes a duplicate — the spot VPs already live on the GPU, per light,
in `render_lighting.spots.lights[i].view_proj`, filled by
`upload_lights` from the same `cpuSpotLights_` that fills the staging
buffer. One fact, one home; the staging home dies.

Pricing unchanged: 96 → 32 MiB/frame at 4 lights, 48 → 16 at 2 (PASS_0
Q3.3). Tile geometry, PCF math, caster sets, formats: untouched (D5).

## THE DESIGN, RE-RULED

- **D1 — pass-per-texture.** Unchanged from ATLAS_1. Indoor: one pass on
  `shadowMapTexture_` for lights 0–1, one on `spotShadowMapTexture_` for
  2–3, opened only if ≥ 1 light owned; `Clear`/`Store`, no `Load`.
  Outdoor unchanged.
- **D2′ — the matrix comes from where it already lives.** No struct
  widening, no new storage. Each shadow VS selects its matrix through
  one helper, declared once in `world.wgsl`:
  `fn shadow_light_vp(li: u32) -> mat4x4<f32>` returning
  `render_vp.light_vp` when `render_lighting.spots.count == 0u` (the
  outdoor arm's own predicate, uniform across the draw), else
  `render_lighting.spots.lights[li].view_proj`. Cost: the
  `render_lighting` layout entry's visibility goes `F` → `VF` (one word,
  `state.hpp`; binding number untouched per L6). Render-family V stages
  go uniform 5 → 6 of 12; the gate row is a compute row and does not move.
- **D3 — the draw names its light.** Unchanged: every shadow draw passes
  `firstInstance = li` (core for direct draws); each shadow VS gains
  `@builtin(instance_index) li : u32`. Outdoor passes 0.
- **D4 — the duplicate dies.** `spotVPStagingBuffer_`, its accessor,
  `stage_spot_vps`, its call in `apply_mood_spot_lights`
  (`direction/mood.hpp` — the recon's corrected symbol), the per-tile
  copy, and the registry constant are all retired. The recon confirmed
  no other reader exists.
- **D5 — untouched by decree:** as in ATLAS_1, plus explicitly:
- **D6 — the sun's slot keeps its one owner.** `compute_vp`'s write and
  `sample_shadow_pcf`'s fragment read of `render_vp.light_vp` are not
  edited, not gated, not moved. The 14th read site needs no work under
  D2′. The `spots.count == 0u` guard in `calc_directional_light` stays
  exactly as written — its future is a separate ruling (see close).

## U0′ — DELTA RECON (the gate already ran; verify only what can have moved)

1. Git hygiene; record HEAD. If HEAD ≠ `17a0faa` lineage plus Jean's
   gate-session commits, re-verify verbatim: the staging-buffer toucher
   table (recon U0.5), the indoor loop (recon U0.2), and the
   `calc_directional_light` guard comment. Unchanged HEAD → proceed on
   the recon's findings as standing.
2. **Witness availability:** run `naga --version`. If absent, attempt
   install (`cargo install naga-cli`, or the container's package route);
   record the outcome. naga green is CC's per-commit gate when present;
   its absence does not unblock landing on master — see WITNESSES.
3. Count check before editing: `world.wgsl` has 19 textual `light_vp`
   references — 1 struct field, 13 shadow-VS reads, 1 fragment read,
   2 compute writes, 2 comments. The subject set is exactly the 13.
   Any other tally → STOP.

## THE UNITS (held branch; one commit each)

- **U1′ — visibility + selection, one commit** (binding closure demands
  they land together): `state.hpp` `render_lighting` entry `F` → `VF`;
  `world.wgsl` gains `shadow_light_vp(li)` once, the 13 shadow VSes gain
  the `instance_index` builtin and their `render_vp.light_vp *` becomes
  `shadow_light_vp(li) *`. Expected after-counts: `light_vp` textual
  references 19 → 7 (field, helper's sun branch, fragment read, 2
  writes, 2 comments); `view_proj` gains exactly one new read site (the
  helper). Mismatch → STOP.
- **U2 — thread `firstInstance`.** As in ATLAS_1: `draw_shadow_all`
  gains the light parameter; all 13 shadow draw functions pass it into
  `DrawIndexed`'s `firstInstance`; outdoor call sites pass 0.
- **U3 — restructure the passes.** As in ATLAS_1: per texture, one
  `BeginRenderPass`, per owned light `SetViewport` + `SetScissorRect` +
  `draw_shadow_all(c, pass, false, li)`; delete the per-tile copy.
- **U4 — retire the staging trio** per D4.
- **U5 — post-merge, on master:** `tools/binding_ledger.py` regen.
  Expected delta: the `render_lighting` visibility cell and the
  render-family V-stage uniform counts 5 → 6; the staging buffer had no
  binding row, so nothing else moves. Any other delta is a FINDING.

## WITNESSES — the clause this revision adds

The units land on a **held branch** (the harness's pinned branch),
regardless of naga's availability — this campaign is visual-gated by
construction. naga runs per WGSL-touching commit when installable and
its result is recorded in the commit body. Jean then runs glaw1 + boot
and the **three visual gates**: outdoor with terrain shadows; indoor 2
lights (Gallery); indoor 4 lights — with one named check: **light 0's
shadow direction indoors**, because a sun-viewpoint tile in the left
half of the sun map is exactly the failure the struck D2 would have
shipped, and its absence is the gate. Merge is Jean's action; the branch
dies; U5 follows on master. Then preview deploy + one Pixel window per
the spend governor — expected observable: indoor `shadow_pass` **cpu**
falls from ~0.29–0.41 toward the outdoor ~0.15–0.2; GPU row compared
against itself only.

## STOP CONDITIONS

- Any U0′ verbatim or count mismatch, before or after edits.
- Any edit reaching `compute_vp`, `sample_shadow_pcf`,
  `calc_directional_light`, the PCF banner, tile rectangles, caster
  sets, or texture formats.
- naga red on any commit (fix or STOP; never land red).
- Any pressure to merge to master from the container — the merge is
  Jean's, after the walk.

## RECORDED FOR A LATER RULING, NOT PERFORMED

Once the tiles stop clobbering `light_vp`, the guard's *stated* reason
in `calc_directional_light` evaporates — but restoring indoor sun
shadow is more than deleting the guard: indoors the sun-map texture's
content is spot tiles 0–1, so a live indoor sun PCF would sample spot
depths under a sun matrix. Restoration needs its own content (a
dedicated tile or arm) and its own semiotic case. Jean's ruling, another
day.
