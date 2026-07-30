# UMBRA — shadow quality campaign

Eight handoffs. UMBRA_1 is a read; UMBRA_2–8 are edits whose parameters bind to UMBRA_1's report.
Execute in order, in one session. Recon precedes proposals: no edit is designed from an unread site.

## Git law
Trunk-based on master only. Commit and push directly to master. One commit per handoff — the
campaign must bisect. No `claude/*` branches. Jean holds all build and runtime gates; do not
compile or run. Tags are pushed by Jean from the design machine.

## Register discipline
REPORT findings; never improvise fixes. Verify every named anchor verbatim before editing.
STOP on mismatch and report — a STOP is a finding, not a failure. Census scope:
`src/cartridges/the_board/**` and `src/incubator_dual.cpp`. Generated C++ text inside
`src/tools/*.jsx` is not a live call site.

## FXC law
Before any WGSL edit, read the `world.wgsl` banner FXC block verbatim and obey it. This
campaign's shader edits are designed FXC-conservative by construction: no arrays, no runtime
branching in the sampled chain, unrolled taps, const-expression offsets only. If any edit
below would require growing a **shared** entity/texture bind-group layout, STOP and report —
do not grow shared layouts.

## Global STOP conditions
- A named anchor is absent or appears a different number of times than stated.
- An edit requires growing a shared bind-group layout.
- An edit conflicts with the `world.wgsl` FXC banner.
- The sun frustum construction is not reducible to center + radius + near/far.

---

# UMBRA_1 — recon (report-only; commit `src/docs/UMBRA_REPORT.md` to master)

Report the following, verbatim where the item is code. Read from descriptors and call sites,
never from labels or comments — the 640-pixel card wearing a "512×512" label is the named
failure mode.

- **R1 — Maps.** Sun shadow map: dimensions and depth format from the texture descriptor.
  Spot shadow maps: count, dimensions, format. One texture or array.
- **R2 — Frustum.** What the sun frustum centers on (pawn? camera?), radius, near/far, refit
  cadence (per frame? on event?), any existing texel snapping. File + function.
- **R3 — Light direction.** Static per scene, or animated per frame. (Snapping quantizes
  translation; it cannot quantize rotation. If the sun animates continuously, report it.)
- **R4 — Shadow pipelines.** For every shadow-writing pipeline: `depthBias`,
  `depthBiasSlopeScale`, `depthBiasClamp`, and `cullMode`, verbatim from the
  depth-stencil / primitive state.
- **R5 — Shader-side nudges.** Every WGSL line that biases, offsets, or fudges depth in the
  shadow write or shadow compare path. Verbatim lines with file + function.
- **R6 — Sampling site.** File + function where the shadow term is computed. Sampler type
  (`sampler_comparison` or manual compare via load), filter mode, tap count. Whether
  light-space coords are built in the vertex or fragment stage. Whether the receiver's
  world-space normal is available at that site.
- **R7 — Caster lists.** Does the sun shadow draw list include curtain geometry? What drives
  caster LOD selection (eye distance? pawn distance?) and what is the LOD ladder? Do spot
  shadow draw lists include terrain?
- **R8 — Bind structure.** Group and binding of the shadow texture and its sampler, and which
  layout owns them (shared entity/texture layout, or other). Does a shadow/light uniform
  struct reach the sampling shader, and does it have spare padding (GROWTH LAW applies if we
  add a field). This report decides UMBRA_7/8 growth risk.
- **R9 — FXC banner.** The `world.wgsl` banner FXC block, verbatim.
- **R10 — Matrix site.** File + function where the sun view/ortho matrices are built CPU-side.
  Confirm exactly one site constructs the sun ortho; if more than one, STOP.
- **R11 — Stale labels.** Any comment or string near the shadow chain asserting a dimension,
  count, or behavior. Verify each against the descriptor; list mismatches (they are folded
  into later commits, delete-not-annotate).

---

# UMBRA_2 — freeze + snap the sun frustum
**Commit:** `umbra: freeze and snap sun frustum`
**Site:** R10. **Params:** R2 (call today's radius R0, resolution RES0).

Intent: pawn motion slides the light-space sample grid by sub-texel amounts; every static
edge re-rasterizes at a shifted threshold each frame — the fire. Quantize the grid.

1. **Freeze.** Radius, near, far become named constants (today's values, or the observed
   maxima rounded up if currently refit per frame). Delete the per-frame refit code — delete,
   not comment.
2. **Snap.** After building the light view matrix and before the ortho, quantize the frustum
   center in light space to whole texels. Shape (adapt names to the found site; column-major
   assumed — verify):

   ```cpp
   const float texelWorld = (2.0f * R) / float(RES);
   vec3 cLS = vec3(lightView * vec4(centerWS, 1.0f));
   cLS.x = std::floor(cLS.x / texelWorld) * texelWorld;
   cLS.y = std::floor(cLS.y / texelWorld) * texelWorld;
   // build the symmetric ortho around cLS: [cLS.x - R, cLS.x + R] × [cLS.y - R, cLS.y + R]
   ```

   If the found construction is lookAt-at-center + symmetric ortho, the equivalent snap of
   the target point in light space is acceptable. If the construction is not reducible to
   center + radius, STOP (global condition).
3. If R3 reported an animated sun, note in the commit body that residual tremble during sun
   motion is expected and out of scope — rotation cannot be snapped.

Gate for Jean: pawn laps, camera orbits — static shadow edges must hold still.

---

# UMBRA_3 — sun caster diet: pin LOD, curtains out
**Commit:** `umbra: pin shadow caster LOD; curtains out of sun casters`
**Site + params:** R7.

Intent: the shadow pass draws at eye density against a target that cannot resolve it, and
half the LOD0 triangle budget is curtain geometry, degenerate at rest. Casters denser than
the target are pure cost; casters whose LOD changes with eye distance change their silhouette
— the second candidate for the "shadows compose as we approach" artifact. Stability beats
accuracy in shadow silhouettes: a permanently slightly-simplified silhouette is invisible, a
changing one is glaring.

1. **Pin LOD.** Caster LOD becomes one fixed ladder level, chosen by the rule: the coarsest
   level whose typical triangle edge ≤ 2 × texelWorld (post-UMBRA_5 texelWorld; compute it
   from R1 = 1.4·R0 and RES1 = 2·RES0 now). If the ladder makes the rule ambiguous, STOP and
   report the ladder — Jean rules.
2. **Curtains out.** Exclude curtain geometry from the sun caster list. Make the exclusion a
   single boolean site so the revert is one commit.

Gate for Jean: silhouette light-leaks at terrain edges under low sun. Leak → revert the
curtain boolean only; the LOD pin stands on its own.

---

# UMBRA_4 — spot caster diet: terrain out
**Commit:** `umbra: terrain out of spot caster lists`
**Site + params:** R7.

Intent: indoor scenes multiply the full shadow draw list by the active spot count, terrain
included. Indoor spots shadowing terrain is meaningless work inside the known bottleneck.
Remove terrain from spot caster lists. Do not build a light-volume bounding mechanism — no
mechanism until a measurement asks. Report what remains in a spot list after the cut.

Gate for Jean: indoor scenes, spot shadows intact, frame time down.

---

# UMBRA_5 — resolution × radius
**Commit:** `umbra: sun map RES→2x per side, radius 1.4x`
**Site:** R1 descriptor + UMBRA_2 constants. **Params:** RES1 = 2·RES0 (cap 4096), R1 = 1.4·R0.

Intent: 4× texels, yield split evenly — 1.4× coverage (pushes the composing boundary out) and
1.4× crispness (texelWorld shrinks to ~0.7× today's). The map centers on the pawn and the
camera orbits the pawn, so the map's uniform sharpness lands exactly where the player looks.
Funded by UMBRA_3/4.

1. Update the descriptor dimensions and the RES constant (one fact, one home — if these are
   two homes today, unify them in this commit).
2. Update R in the UMBRA_2 constants.
3. Fix any R11 stale labels touching these values. Delete rather than annotate.

Gate for Jean: coverage reaches ~40% further; near shadows visibly crisper; the METER_1
table's shadow_pass row is the cost witness.

---

# UMBRA_6 — bias moves to the rasterizer
**Commit:** `umbra: bias to rasterizer state; shader nudges deleted`
**Site:** R4 pipelines + R5 lines.

Intent: one constant is doing two jobs and failing both. Slope-scaled bias tracks the
quantity acne actually comes from; the constant shrinks toward zero; shader-side nudges lose
their home. Zero WGSL surface, zero FXC risk.

1. On every shadow-writing pipeline:

   ```cpp
   ds.depthBias           = 2;
   ds.depthBiasSlopeScale = 2.0f;
   ds.depthBiasClamp      = 0.0f;
   ```

   The constant-bias unit is format-dependent (spec §10.3.6: unorm and float formats scale
   differently). If R1 reported a float depth format, keep depthBias = 2 as the start but
   flag it in the report — the tuning ladder below is Jean's dial either way.
2. Delete every R5 shader-side nudge. Bias now has one home: the pipeline.
3. Cull mode: if R4 reported front-face culling in the shadow pass, switch to back-face —
   front-culling is a peter-panning machine and terrain is open. If back or none, leave it.

---

# UMBRA_7 — normal-offset receiver sampling
**Commit:** `umbra: normal-offset receiver sampling`
**Site:** R6 (coordinate construction). **Precondition:** R6 confirms the receiver normal is
available at that site; if not, STOP and report where it could come from.

Intent: this is what glues the pawn's shadow to its feet. The offset moves the *sample
position* along the receiver normal — it removes slope acne without lifting depth at contact,
and it is expressed in texels, so it survived UMBRA_5 by construction. Sized for the 3×3
filter footprint UMBRA_8 installs.

Shape (adapt names; apply where light-space coords are built — vertex stage if that is where
R6 found them):

```wgsl
let ndotl    = clamp(dot(N, L), 0.0, 1.0);
let offsetW  = N * (TEXEL_WORLD * 2.0 * (1.0 - ndotl));
let posShadow = light_view_proj * vec4<f32>(world_pos + offsetW, 1.0);
```

`TEXEL_WORLD` home, in preference order per R8: (a) spare padding in an existing shadow/light
uniform that already reaches this shader — GROWTH LAW applies: same commit, same position,
same type, `sizeof` witness; (b) if no struct or no padding, STOP and report — do not grow a
shared layout and do not create a second home silently.

---

# UMBRA_8 — 3×3 PCF + edge fade
**Commit:** `umbra: 3x3 PCF + edge fade`
**Site:** R6 (sampling function). **Precondition:** R8 growth check passed.

Intent: the beauty spend, and it spends on the fragment side — the side the resize experiment
proved cheap. Hardware-bilinear per tap through a comparison sampler; nine taps, fully
unrolled, const offsets: FXC-clean by construction against the R9 banner.

1. **Sampler.** The shadow map is sampled through a `sampler_comparison` with linear
   min/mag filter and the compare function matching today's test direction. If R6 reported a
   plain sampler bound for the shadow map, *retype that binding in place* (layout entry
   `comparison`, WGSL var to `sampler_comparison`) — replacement, not growth. If R6 reported
   manual `textureLoad` compare with no sampler binding at all, adding one grows the layout:
   STOP if that layout is shared (global condition); proceed only if it is shadow-private.
2. **Taps.** Replace the single compare with nine literal calls — no array, no loop:

   ```wgsl
   var s = 0.0;
   s += textureSampleCompareLevel(shadow_map, shadow_cmp, uv, z, vec2<i32>(-1, -1));
   s += textureSampleCompareLevel(shadow_map, shadow_cmp, uv, z, vec2<i32>( 0, -1));
   s += textureSampleCompareLevel(shadow_map, shadow_cmp, uv, z, vec2<i32>( 1, -1));
   s += textureSampleCompareLevel(shadow_map, shadow_cmp, uv, z, vec2<i32>(-1,  0));
   s += textureSampleCompareLevel(shadow_map, shadow_cmp, uv, z, vec2<i32>( 0,  0));
   s += textureSampleCompareLevel(shadow_map, shadow_cmp, uv, z, vec2<i32>( 1,  0));
   s += textureSampleCompareLevel(shadow_map, shadow_cmp, uv, z, vec2<i32>(-1,  1));
   s += textureSampleCompareLevel(shadow_map, shadow_cmp, uv, z, vec2<i32>( 0,  1));
   s += textureSampleCompareLevel(shadow_map, shadow_cmp, uv, z, vec2<i32>( 1,  1));
   let shadow = s * (1.0 / 9.0);
   ```

   `textureSampleCompareLevel`, not `textureSampleCompare` — no implicit derivatives, no
   uniformity diagnostics, legal in any stage.
3. **Edge fade.** Distant shadows materialize instead of assembling. Pure arithmetic, outer
   12% of the map:

   ```wgsl
   let d    = max(abs(uv.x * 2.0 - 1.0), abs(uv.y * 2.0 - 1.0));
   let fade = clamp((1.0 - d) / 0.12, 0.0, 1.0);
   let lit  = mix(1.0, shadow, fade);   // unshadowed beyond the frustum
   ```

---

# Campaign ledger (fill at end, append to UMBRA_REPORT.md)

| Handoff | Commit | State (landed / held / dead) | Notes |
|---|---|---|---|
| UMBRA_1 | | | |
| UMBRA_2 | | | |
| UMBRA_3 | | | |
| UMBRA_4 | | | |
| UMBRA_5 | | | |
| UMBRA_6 | | | |
| UMBRA_7 | | | |
| UMBRA_8 | | | |

# Tuning ladder (Jean's dial, at the visual gate)

- Acne survives on slopes → `depthBiasSlopeScale` +0.5 per step.
- Shadow detaches at the pawn's feet → normal-offset scale 2.0 → 1.5 first, then
  `depthBias` 2 → 1.
- Penumbra reads mushy near the camera → claw radius back: R1 −10% per step (crispness is
  bought from radius, never from taps).
- Light leaks at terrain silhouettes under low sun → revert the UMBRA_3 curtain boolean only.
- Composing still visible at the far edge → widen the fade band 0.12 → 0.20 before anything
  structural.
