# PENUMBRA_1 — shadow residuals and soft-edge restoration

Eight handoffs. P1 is a read; P2–P7 land; P8 is HELD pending P2's visual gate.
Successor to UMBRA. Recon precedes proposals: no edit is designed from an unread site.

## Git law (amended at UMBRA close)
`master` names the most recent version of the program. Commit on the harness-mandated
branch, then **fast-forward master and push after each commit**. Fast-forward only — if a
commit cannot fast-forward, master moved: fetch, rebase onto it, and report. Never merge,
never force. One commit per handoff; the campaign must bisect.

**New clause, adopted from the UMBRA close:** no ancestry, topology, or divergence claim
without a `git fetch` in the same command sequence. A remote-tracking ref is a cached label,
and this campaign family exists because cached labels lie. Fold this clause into
`src/docs/HANDOFFS/PROCESS_LAWS.md` as part of P1's commit.

## Register discipline
REPORT findings; never improvise fixes. Verify every named anchor verbatim before editing.
STOP on mismatch and report — a STOP is a finding. Scope: `src/cartridges/the_board/**`
and `src/incubator_dual.cpp`. Generated C++ in `src/tools/*.jsx` is not a live call site.
Jean holds all build and visual gates. Do not compile, do not run.

## FXC law
Read the `world.wgsl` banner FXC block (and the L2 specifics it points to) before any WGSL
edit. Every shader edit below is arithmetic or const-expression offsets: no arrays, no new
runtime branching, no new bindings, no growth of the shared 11-entry
`renderTextureBindGroupLayout_`. Growing that layout fires a global STOP.

## Global STOP conditions
- A named anchor is absent or appears a different number of times than stated.
- Any edit would grow a shared bind-group layout.
- Any edit conflicts with the FXC banner or L2.
- `depthBiasClamp` proves illegal on this device (see P1-A).

## Standing facts carried from UMBRA (do not re-derive; verify only if an edit touches them)
- Sun map: 4096 × 4096, `Depth32Float`. `Dim::SHADOW_MAP_SIZE` = 4096, four C++ readers.
- `SUN_HALF_EXTENT` = 420.0, `SUN_NEAR` = 0.1, `SUN_FAR` = 600.0, `SUN_ALTITUDE` = 250.0.
- `TEXEL_WORLD` = 2 × 420 / 4096 = **0.20508 wu**.
- Sun ortho depth range 599.9 wu maps to [0,1]; one ULP at z ≈ 0.417 is 2.98e-8.
- Sun PCF is gated on `render_spot_lights.count == 0u` — **outdoor moods only**.
- Eleven shadow pipelines, one shared `makeShadow` builder, one `shadowDepth` state.
  Seven are `CullMode::None`: pawn, column, palm, cactus, blade, shell, ribbon.

---

# P1 — targeted recon (report-only; append to `src/docs/UMBRA_REPORT.md`, new section
# "PENUMBRA_1 — RECON")

Five questions. Each gates a specific later handoff. Read from descriptors and call sites.

- **P1-A — `depthBiasClamp` legality.** Does the device request/hold `core-features-and-limits`?
  Under WebGPU compatibility mode a non-zero `depthBiasClamp` is invalid. Report the device
  feature set as requested at adapter/device creation. **Gates P2.** If compatibility mode is
  in force, STOP P2 and report — the fallback is capping `depthBiasSlopeScale` instead, which
  is a different edit and Jean rules on it.
- **P1-B — terrain normal derivation.** In the terrain FS, is the shading normal a per-cell /
  per-facet flat value, or interpolated/derived from a filtered heightfield gradient? Quote the
  producing lines. **Gates the checker ruling** (see the OBSERVATION GATE below) and tells us
  whether P3's offset inherits a discontinuity.
- **P1-C — sun sampler bounds rejection.** Does `sample_shadow_pcf` reject `light_clip.z`
  outside [0,1] before comparing, the way `sample_spot_shadow_pcf` does via `out_of_bounds`?
  Quote what it does. **Gates the sphere-square second hypothesis.**
- **P1-D — spot projection aspect.** The spot tile is `TILE_W = SHADOW_MAP_SIZE / 2` by
  `TILE_H = SHADOW_MAP_SIZE` — 2048 × 4096, a 1:2 tile. Does the spot projection matrix use
  `aspect = 0.5` to compensate, or 1.0? Report the matrix construction verbatim, plus the spot
  half-FOV and near/far. **Gates P7.** If aspect is uncompensated, spot texels are non-square
  by 2× and that is a finding in its own right — report it, do not fix it here.
- **P1-E — terrain rim.** Quote the visible rim's per-fragment discard and its radius source.
  Confirm whether `shadow_patch_terrain_vs` has any rim path, and report what selects the patch
  set fed to the shadow pass (`band_patches` off THE POINT) and its radius. **Feeds the
  sphere-square first hypothesis; no edit in this campaign.**

Commit the PROCESS_LAWS.md clause from the git law block in the same commit.

---

# P2 — restore the bias ceiling; retire the inert constant
**Commit:** `penumbra: depthBiasClamp restores the SHADOW_BIAS_MAX ceiling`
**Site:** `renderer.hpp`, `shadowDepth` in the shared shadow builder. **Gated by P1-A.**

Intent. UMBRA_6 traded `SHADOW_BIAS_MIN` / `SHADOW_BIAS_MAX` for slope-scale, but only the
floor got a successor. `depthBiasClamp = 0.0` means no ceiling: the ratio against the old term
runs ≈1.1 through the mid-range, 10.1 at 88°, and unbounded past that. Seven pipelines are
`CullMode::None` zero-thickness sheets whose faces reach grazing at every twist. That is the
ribbon chainsaw, and the sphere square is a candidate for the same cause.

Deleting a mechanism means enumerating everything it did, not only its headline job. The old
shader term did two: a floor and a ceiling. This restores the second.

```cpp
shadowDepth.depthBiasSlopeScale = 2.0f;   // unchanged
shadowDepth.depthBiasClamp      = 2.8e-3f;
```

`2.8e-3` is the old `SHADOW_BIAS_MAX` (4.0e-3) scaled by the post-UMBRA_5 texel ratio
(0.20508 / 0.29297 = 0.700). Derivation recorded in the commit body.

**Also in this commit:** delete the `depthBias` line. Under `Depth32Float` the constant is
ULP-relative; at this scene's z ≈ 0.417 a value of 2 buys 6.0e-8 NDC — about 3,355× short of
the floor it replaced. It is not a dial in steps of one and it should not sit in the tree
pretending to be one. The flat-sun-facing-ground case it was nominally covering is picked up
by P3's floor, in texel units, which is the correct unit for it. Record in the ladder: **under
`Depth32Float` the constant term is not a dial; slope-scale and the normal offset are the two
live instruments.**

---

# P3 — filter footprint becomes one fact; offset floor and tap spacing derive from it
**Commit:** `penumbra: PCF_SPACING drives tap offsets and normal-offset magnitude`
**Site:** `world.wgsl`, `sample_shadow_pcf`.

Intent, and why these are one handoff and not two. Measured in world units rather than texels,
UMBRA halved the penumbra: the old 4×4 kernel at 0.29297 wu texels spanned ≈1.47 wu of blur;
the current 3×3 at 0.20508 spans ≈0.82. Nothing new appeared in the terrain — the caster's own
tessellation was always stepping and 1.47 wu of blur was covering it. The fix is filter width,
not caster geometry, because geometry is the frame's bottleneck and the fragment side is the
side the resize experiment proved cheap.

And the normal offset must cover half the filter footprint or the filter reaches back into the
caster surface. So spacing and offset magnitude are **one fact with two expressions**. Give it
one home.

1. Declare, module scope, beside `TEXEL_WORLD`:

   ```wgsl
   const PCF_SPACING: i32 = 2;                             // texels between taps
   const PCF_RADIUS_TEXELS: f32 = f32(PCF_SPACING) + 1.0;  // 3.0 — half the footprint
   ```

   Spacing 2 is the **gapless maximum** at three taps per axis: each
   `textureSampleCompareLevel` tap carries a 2×2 hardware-bilinear footprint, so taps at
   −2 / 0 / +2 cover [−3,−1] [−1,1] [1,3] contiguously. At spacing 3 the footprints stop
   touching and blur becomes banding. Do not raise it without also raising the tap count.

2. Rewrite the nine offsets to use it — still nine literal calls, no array, no loop. WGSL
   `const` is a const-expression, so these remain legal `offset` operands:

   ```wgsl
   var s = 0.0;
   s += textureSampleCompareLevel(shadow_map, shadow_sampler, uv, z, vec2<i32>(-PCF_SPACING, -PCF_SPACING));
   s += textureSampleCompareLevel(shadow_map, shadow_sampler, uv, z, vec2<i32>(           0, -PCF_SPACING));
   s += textureSampleCompareLevel(shadow_map, shadow_sampler, uv, z, vec2<i32>( PCF_SPACING, -PCF_SPACING));
   s += textureSampleCompareLevel(shadow_map, shadow_sampler, uv, z, vec2<i32>(-PCF_SPACING,            0));
   s += textureSampleCompareLevel(shadow_map, shadow_sampler, uv, z, vec2<i32>(           0,            0));
   s += textureSampleCompareLevel(shadow_map, shadow_sampler, uv, z, vec2<i32>( PCF_SPACING,            0));
   s += textureSampleCompareLevel(shadow_map, shadow_sampler, uv, z, vec2<i32>(-PCF_SPACING,  PCF_SPACING));
   s += textureSampleCompareLevel(shadow_map, shadow_sampler, uv, z, vec2<i32>(           0,  PCF_SPACING));
   s += textureSampleCompareLevel(shadow_map, shadow_sampler, uv, z, vec2<i32>( PCF_SPACING,  PCF_SPACING));
   let shadow = s * (1.0 / 9.0);
   ```

   Offsets must stay within [−8, 7]; at spacing 2 the extreme is ±2.

3. Replace UMBRA_7's offset magnitude with one that has a floor and derives its ceiling from
   the footprint:

   ```wgsl
   let ndotl    = clamp(dot(N, L), 0.0, 1.0);
   let offset_w = TEXEL_WORLD * PCF_RADIUS_TEXELS * (0.33 + 0.67 * (1.0 - ndotl));
   let pos_off  = world_pos + N * offset_w;
   ```

   ≈0.99 texel on ground facing the sun, 3.0 texels at grazing. The floor closes the gap
   UMBRA left open — on flat sun-facing ground slope-scale is zero **and** the old
   `(1 - ndotl)` offset was zero, so nothing covered caster/receiver tessellation mismatch
   there. It is covered now, in texel units, so it survives every future radius change.

---

# P4 — the shadow sample reads the geometric normal, not the shaded one
**Commit:** `penumbra: shadow offset uses geometric normal`
**Site:** terrain FS, the aura perturbation and the `calc_directional_light` call.

Intent. `normal = normalize(normal + vec3(0.0, aura.r * 0.3, 0.0))` bends the normal by up to
~17°, and after UMBRA_7 that bend steers the shadow sample as well as the shading. Normal-offset
exists to escape a **geometric** self-shadowing surface; it is now reading a shading fact. Two
facts in one variable, and a coupling nobody designed.

Hold the pre-aura normal in a local, pass that to `calc_directional_light`, and let the aura
keep the shading it was written for. No new binding, no branch, no layout change.

---

# P5 — spot-path parity
**Commit:** `penumbra: normal offset on the spot path`
**Site:** `world.wgsl`, `sample_spot_shadow_pcf`. **Gated by P1-D.**

Intent. UMBRA_6 deleted the pawn lift; on the sun path UMBRA_7 backfilled it, on the spot path
nothing did. Indoor moods therefore run with no offset **and** no constant bias — the campaign's
thinnest ice, and the reason indoor pawn shadows are on the observation gate.

The spot frustum is perspective, so texel world-size is not constant — that is why UMBRA ruled
the spot path offset-free. Derive it per fragment instead:

```wgsl
let spot_texel_world = 2.0 * light_dist * SPOT_TAN_HALF_FOV / SPOT_TILE_TEXELS;
```

`SPOT_TAN_HALF_FOV` and `SPOT_TILE_TEXELS` come from P1-D verbatim; do not invent them. Then
apply the same shape as P3 step 3 with `spot_texel_world` in place of `TEXEL_WORLD`. Pure
arithmetic, no branch.

**If P1-D reports an uncompensated 1:2 aspect**, spot texels are non-square by 2× and a single
scalar is wrong on one axis. In that case land the offset using the **larger** of the two texel
world-sizes (conservative — over-offsets on one axis rather than under-offsetting) and report
the aspect finding as a HORIZON item. Do not fix the projection here.

Also in this commit: confirm the spot kernel's `+ 0.5` centring term still holds after any
edit, and leave it. It is the correction the sun kernel lacked.

---

# P6 — dead code
**Commit:** `penumbra: delete dead depth clamp in sample_spot_shadow_pcf`
**Site:** `world.wgsl`, `sample_spot_shadow_pcf`.

`clamped_depth = clamp(current_depth, 0.0, 1.0)` contained the bias UMBRA_6 deleted. With raw
`light_ndc.z` going in and `out_of_bounds` already rejecting outside [0,1], it is the identity
on every surviving fragment. Residue of our own edit. Delete rather than annotate.

Fold in any stale labels found near the sites this campaign touches, per R11 discipline.

---

# P7 — ladder and control-panel record
**Commit:** `penumbra: tuning ladder rewritten against the new instruments`
**Site:** `src/docs/UMBRA_REPORT.md`, TUNING LADDER section.

Rewrite the ladder against what is now true. Keep the direction table at its head — it was the
adversarial pass's correction and it stays. Then:

| symptom | instrument | direction |
|---|---|---|
| acne on slopes | `depthBiasSlopeScale` | up, +0.5 per step |
| acne on flat sun-facing ground | P3's offset floor (`0.33` term) | up, +0.15 per step |
| shadow off contact at the pawn's feet | P3's offset ceiling (`0.67` term) | down |
| shadow detaches on thin sheets only | `depthBiasClamp` | down from 2.8e-3 |
| penumbra too hard, caster steps visible | `PCF_SPACING` | already at gapless max; next step is 25 taps (P8-adjacent, not free) |
| penumbra too soft near the camera | `SUN_HALF_EXTENT` | down — see the reserve below |

**Record the crispness reserve as a control-panel fact.** `SUN_HALF_EXTENT` is a single WGSL
const from which texel world-size, offset magnitude, and the edge-fade radius all derive. Two
regimes:

- **420 → 369 wu:** the edge fade stays dormant, no shadow horizon is ever visible, ~12% finer
  texels. Risk-free.
- **369 → 325 wu:** the fade goes live and does the job it was written for, softening a horizon
  that now falls inside the 325 wu veil ring. Up to ~23% finer texels, at the cost of a soft
  shadow horizon.

The fade is therefore **not** dormant-and-deletable. It is precisely what makes the reserve
spendable below 369. Record it as such so nobody deletes it as unreachable code.

**Also record:** the sun map's resolution and the spot atlas's resolution are the same fact
(`Dim::SHADOW_MAP_SIZE`) serving two roles. One dial, two jobs — the pattern this campaign
family exists to break. Splitting them would let the sun go finer for less VRAM than the
+100.7 MB UMBRA_5 paid. HORIZON item, dated, not chased here.

---

# P8 — HELD: bias profile fork for zero-thickness sheets
**Do not land.** Designed and held pending P2's visual gate.

If the ribbon chainsaw **survives** P2's clamp, the remaining cause is that a zero-thickness
sheet wants a different bias regime from a closed solid: it has no interior to self-shadow, so
slope-scale buys it nothing and costs it displacement. `makeShadow` already forks on
`cullMode`; adding a bias profile is one more parameter on an existing fork, not a new
mechanism.

```
SOLID  — slopeScale 2.0, clamp 2.8e-3   (terrain, sphere, monolith, arch)
SHEET  — slopeScale 0.5, clamp 1.0e-3   (pawn, column, palm, cactus, blade, shell, ribbon)
```

Second candidate if it still survives: with `CullMode::None` both faces of the sheet rasterize
at near-identical depths, so which one wins the depth test is arbitrary per texel — z-fighting
inside the shadow map, which reads as teeth. That one has no cheap fix and would be reported,
not solved, in this campaign.

Land only on Jean's word.

---

# OBSERVATION GATE — each artifact bound to the commit that should move it

Not a vibe check. Each row is a falsification test; record the outcome in the ledger.

| artifact (screenshot) | hypothesis | commit that should move it | if it does not move |
|---|---|---|---|
| ribbon shadow chainsaws as it twists (73–76) | unbounded grazing bias on a `CullMode::None` sheet | **P2** | release P8 |
| square shadow on the sky sphere (78, 79) | a panel's shadow displaced by the same runaway bias | **P2** | run the discriminator below |
| terrain shadow edges sawtooth (73, 74, 77) | caster tessellation, previously hidden by a 1.47 wu penumbra now 0.82 wu | **P3** | caster tessellation is coarser than assumed — report, do not add geometry |
| checker pattern in shadows (77) | per-cell flat normal ⇒ per-cell offset jump, visible only in penumbra | **P1-B / P4** | see discriminator below |
| indoor pawn shadow at the feet | pawn lift deleted, nothing backfilled | **P5** | report |

**Sphere-square discriminator (Jean, one pass):** walk the pawn while the sphere holds
position. If the square slides across the sphere, it is the un-discarded terrain rim caster
tracking THE POINT — a square-cornered caster past a smooth round visible rim, already flagged
in UMBRA's HORIZON and still not chased. If it holds still, it is a real panel and P2 is the
whole answer.

**Checker discriminator (Jean, one glance):** is the checker equally strong on open lit ground
far from any shadow? Then it is the cell-colour aesthetic and not ours. Is it visible only
where the shadow term is partial? Then it is the offset inheriting a faceted normal, and P1-B
says whether P4 is enough or whether the escalation (receiver-plane depth bias from
`dpdx`/`dpdy` at the fragment site) is needed. Do not build the escalation until this glance
asks for it.

---

# CAMPAIGN LEDGER (fill at close)

| handoff | commit | state (landed / held / dead) | notes |
|---|---|---|---|
| P1 | | | |
| P2 | | | |
| P3 | | | |
| P4 | | | |
| P5 | | | |
| P6 | | | |
| P7 | | | |
| P8 | | HELD | releases only on Jean's word |
