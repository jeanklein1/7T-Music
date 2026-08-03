# SALON_1 — E-c REPORT: the frames can cast, and there is almost nothing to cast into

Report-first. **No edit is in this commit.** Read at `a579a0f`;
`origin/master == master` confirmed before any anchor was derived.

Six questions, all answered from the tree. The result is a **STOP at (3)**, on
the criterion E-c set for itself:

> If the standoff resolves to under ~2 texels, report it and stop. Paying for
> an invisible shadow is a failure this campaign has already made once.

**Median horizontal texel offset: 1.24 at radius 1, falling to 0.42 at radius
4.** And upstream of resolution, a second and larger finding: **only 16.5–19.5 %
of painting positions lie inside any indoor spot light's cone *and* range.** Over
four fifths of the hang is lit by ambient alone and would cast nothing at any
resolution.

The blocker E-c expected — (1), reachability — **is not a blocker at all**, and
the reason is that the brief's chosen route is the wrong one.

---

## §1 — REACHABILITY: NOT VIA `shadowRenderLayout`, AND NOTHING NEEDS TO GROW

`shadowRenderLayout` (`renderer.hpp:1528-1538`) is:

```cpp
std::array<wgpu::BindGroupLayout, 2> shadowLayouts = {
    renderEntityLayout_,        // group 0
    shadowTextureLayout_        // group 1
};
```

`shadowTextureLayout_` (`state.hpp:4332-4378`) has **exactly four entries** —
`bilinear_sampler` (22), `nearest_sampler` (23), `patch_heightfield_array_read`
(28), `live_card_read` (34). **`painting_slots` (binding 50) is not there**, so
a pipeline on that layout cannot reach it.

**But the painting pipelines never used `renderEntityLayout_`/`renderTextureLayout_`
in the first place.** Both colour pipelines are built on their own pair
(`renderer.hpp:1863-1866`, `:1918-1921`):

```cpp
std::array<wgpu::BindGroupLayout, 2> galleryGroups = {
    galleryEntityLayout_, galleryTextureLayout_
};
```

- **`galleryEntityLayout_`** (group 0): `config`, `render_vp`, `render_camera`,
  `render_light` — four entries.
- **`galleryTextureLayout_`** (group 1): **`painting_slots` (50, ReadOnlyStorage,
  Vertex|Fragment)**, `painting_array`, `painting_sampler_filt`,
  `bilinear_sampler`, `live_card_read`.

That is the drawable table's header rule made concrete — *"wall_paintings /
gallery_frames: their OWN gallery bind groups"* — and it applies to the shadow
draws for exactly the same reason it applies to the colour ones.

### The light matrix is already in that pair

The one thing a shadow VS needs beyond the colour VS is the light's view-
projection. `shadow_shell_vs` (`world.wgsl:5441`) is the whole of a shadow entry
point:

```wgsl
out.clip_pos = render_vp.light_vp * vec4(in.pos, 1.0);
```

and `light_vp` is a **member of the same struct** as `m` (`world.wgsl`,
`struct VPMatrix { m: mat4x4<f32>, light_vp: mat4x4<f32> }`). `render_vp` is
entry 1 of `galleryEntityLayout_`.

**So the painting shadow pipelines need no new binding, no layout change, and no
new bind group.** They are the colour pipelines with `light_vp` instead of `m`,
depth-only, on layouts and groups that already exist. Commit B's premise — build
them through `makeShadow`, which hardcodes `desc.layout = shadowRenderLayout`
(`renderer.hpp:2208`) — is the only thing that would have forced a layout to
grow, and it should be dropped in favour of a gallery-shaped shadow builder.

**For the record, had the `shadowRenderLayout` route been taken:**
`shadowTextureLayout_` would gain `bind::g1::painting_slots`, ReadOnlyStorage,
`ShaderStage::Vertex`, and `shadowTextureBindGroup_` (`state.hpp:5343`) the
matching buffer entry — burdening all twelve pipelines on that layout to serve
two.

---

## §2 — L2.4: NO CEILING ON EITHER ROUTE

Counted per stage, by hand, from the layout constructions.

| stage | route | storage | uniform |
|---|---|---:|---:|
| shadow VS today | `renderEntityLayout_` + `shadowTextureLayout_` | **7** | **7** |
| shadow VS, if `painting_slots` were added there | — | **8** | 7 |
| **gallery VS (colour, today)** | `galleryEntityLayout_` + `galleryTextureLayout_` | **2** | **1** |
| **gallery shadow VS (proposed)** | same pair, unchanged | **2** | **1** |

Against L2's `maxStorageBuffersPerShaderStage = 10` /
`maxUniformBuffersPerShaderStage = 12` (`SALON_1.md:375`, `LEDGER_1_REPORT.md:821`).

The `shadowRenderLayout` route would sit at 8/10 — legal, two to spare. **The
gallery route adds nothing at all** and sits at 2/10. This is the one place E-c
was told it might hit a real ceiling; it does not, and on the correct route the
question does not arise.

*(One stale comment noted, not acted on: `state.hpp:4332` says "Avoids exceeding
the 8 storage buffer per-stage limit." The limit this repo records elsewhere is
10. Not E-c's to fix.)*

---

## §3 — RESOLUTION: **STOP**

### The partition

`SHADOW_MAP_SIZE = 4096` (`state.hpp:246`). The indoor pass
(`render_passes.hpp:280-321`) is a **two-texture atlas, one tile per light**:

```cpp
static constexpr uint32_t TILE_W = Dim::SHADOW_MAP_SIZE / 2;  // 2048
static constexpr uint32_t TILE_H = Dim::SHADOW_MAP_SIZE;      // 4096
bool use_sun_map = (li < 2);      // lights 0-1 -> sun map, 2-3 -> spot map
uint32_t within = li % 2;         // left half / right half
```

So each of up to four spot lights owns a **2048 × 4096** tile.

**The projection is square** — `compute_spot_light_vp` (`render_passes.hpp:513`)
builds `proj` with the same `f` in x and y and no aspect term. The same FOV
therefore covers 2048 texels horizontally and 4096 vertically: **horizontal
density is half of vertical**, and the horizontal axis is the one a wall
painting's shadow is displaced along.

### What has to be resolved

`frame_depth = 0.30`, `canvas_recess = 0.09`. The canvas stands 0.21 wu off the
wall; the frame's outer face stands **0.30**. A shadow is a *displacement*, not
a depth difference: for a light whose direction to the frame makes angle θ with
the wall's inward normal,

```
  offset = frame_depth × tan(θ)
```

### Measured over 20 000 seeds × 4 schemes × 4 walls × 7 frames

Replaying `derive_indoor_lights` in full — scheme pick, Gaussian lateral/height/
pitch/yaw, the anchor switch, both cone clamps, `MAX_OUTER_HALF` — against the
paint line at `wall_height × 0.45`:

| r | span | in range | **lit (range ∧ cone)** | tex/wu (V) | **p25 / MEDIAN / p75 texels (H)** | irradiance-wtd mean |
|---:|---:|---:|---:|---:|---:|---:|
| 1 | 150 | 73.6 % | **19.5 %** | 15.82 | 0.66 / **1.24** / 2.57 | 2.68 |
| 2 | 250 | 62.9 % | **17.1 %** | 10.22 | 0.40 / **0.77** / 1.58 | 1.83 |
| 3 | 350 | 62.3 % | **16.7 %** | 7.48 | 0.28 / **0.54** / 1.10 | 1.34 |
| 4 | 450 | 62.2 % | **16.5 %** | 5.93 | 0.22 / **0.42** / 0.85 | 1.07 |

**The median is under 2 texels at every radius, and under 1 at three of four.**
68–85 % of lit frames fall under 2; 41–79 % under 1.

The arithmetic mean is much higher (11.6–14.1) and is **not the number to use**:
it is dragged by a thin tail of near-grazing lights where `tan θ → ∞`. Those are
precisely the cases where `cos θ → 0` and the painting is barely lit — the
offset is largest exactly where the light is dimmest. Weighting by incident
irradiance collapses the mean to 1.07–2.68, and only the smallest room clears 2.

**This is E-c's own stop condition, met.**

### The finding above the resolution one

Only **16.5–19.5 %** of painting positions are inside any spot light's cone
*and* range. The cause is a dimensional mismatch in `derive_indoor_lights`
(`mood.hpp:506-507`):

```cpp
L.range = (s.anchor == LightAnchor::CEILING)
    ? wall_height + 30.0f : room_range;
```

A ceiling light's range is **a constant** — 50.0 on FLAT, 55.0 on VAULT — in a
room spanning 150 to 450 wu. A wall sconce's range is `room_size × 0.8`, so it
**can never reach the opposite wall** at any radius; 0.8 < 1.0 is exact, not
statistical.

Four fifths of the hang receives no spot light at all. Those frames would cast
nothing into any shadow map at any resolution, and E-c cannot fix that — it is
a lighting-model question, not a shadow-pipeline one.

---

## §4 — THE WALL CASTS, SO THIS WOULD BE A DROP SHADOW

`shell` is a full member of the drawable table
(`drawable_table.hpp:114`):

```cpp
{ "shell", DRAW_SHADOW | DRAW_MAIN | DRAW_SNAPSHOT, dt_shell },
```

and `draw_shadow_all` runs the table unconditionally (`render_passes.hpp:416`) —
`cast_terrain` gates **only** the terrain fork above it, never the table. So the
indoor shell casts from every spot light, in every atlas tile.

**Therefore painting shadows are a drop shadow within the standoff, not an
occlusion.** The wall behind a painting is already an occluder at effectively the
same depth; the only light a frame newly blocks is what would have landed in the
0.30 wu sliver between its face and the wall. That is what makes §3's threshold
the right test rather than a conservative one — there is no second, larger effect
to fall back on.

---

## §5 — THE OUTDOOR CASE: SAME LAYOUTS, DIFFERENT VIEW, SAME ANSWER

Outdoor moods take `render_shadow_pass`'s `else` branch
(`render_passes.hpp:322-341`): a single full-map pass into
`shadow_map_view()` with the **sun's** `light_vp`, `cast_terrain = true`.

Outdoor paintings are `FORM_TERRAIN_QUAD` slots, drawn by `gallery_frame_vs`, so
they would cast into that sun map. **It shares (1)'s layout question and (1)'s
answer**: the gallery pair reaches `painting_slots`, `render_vp.light_vp`, and —
needed by `gallery_frame_vs`'s ring gate — `config` (entry 0 of
`galleryEntityLayout_`, carrying `veil_ring` and `lod_point_x/z`). Nothing to add.

The outdoor case does **not** hit §3's stop: the sun map is 4096 × 4096 undivided,
one view, and outdoor paintings stand free of any wall, so their shadow falls on
terrain at full length rather than into a 0.30 wu standoff. **If E-c is ever
narrowed, outdoor is the half that survives the arithmetic.** Recorded, not
proposed — E-c as written is one stage covering both.

---

## §6 — CULL MODE AND THE CUT'S ARITHMETIC

All three colour pipelines are **`wgpu::CullMode::None`**:

| pipeline | line | cull | comment at the site |
|---|---|---|---|
| Gallery Frame | `renderer.hpp:1900` | `None` | — |
| Wall Painting Canvas | `renderer.hpp:1950` | `None` | *"visible from both sides (outdoor monuments)"* |
| Wall Painting Frame | `renderer.hpp:1978` | `None` | *"visible from both sides (outdoor monuments)"* |

So both shadow pipelines take `CullMode::None`. That also matters for §4: a
double-sided caster in a 0.30 wu standoff has no back face to bias away from,
which is the configuration peel/acne artifacts prefer.

**The ROSTER site, confirmed** (`renderer.hpp:1170`):

```cpp
if (!(ROSTER.gallery)) n += 4;  // was 6; shadow_gallery_frame + shadow_wall_painting cut
```

Restoring both entry points returns it to **6**, and the comment's second clause
retires with them. The two CUT markers are at `renderer.hpp:1912` (*"Shadow
Gallery Frame pipeline CUT — caller-free"*) and `renderer.hpp:1991` (*"Shadow
Wall Painting pipeline CUT — caller-free"*).

### Vertex arithmetic, corrected

E-c costs the work at 78 verts per frame. That is right for **wall paintings**
(`PAINTING_FRAME_VERTS_PER = 78`, `state.hpp:273`) — 28 frames × 78 = 2,184 per
light, 8,736 across four spots, as stated.

It is **not** right for gallery frames: `PAINTING_QUAD_VERTS = PAINTING_QUAD_N²
× 6` with `PAINTING_QUAD_N = 8` (`state.hpp:272`) = **384 verts per instance**,
drawn `slotHighWater` times. The conclusion is unchanged — both are small, and a
second geometry path would still be mechanism for its own sake — but the outdoor
number is roughly five times the indoor one per frame.

### Commit C's precondition, checked

`slot_high_water` is raised **at every claim site**: `gallery.hpp:1236`
(`commit_gallery`, outdoor) and `:1953` (`place_wall_paintings`, indoor), with
`recompute_slot_high_water` at `:1407` and `:2076` and a reset at `:2141`. Both
colour draws already size from it (`renderer.hpp:927`, `:944`). Sizing the shadow
draws from it is safe.

---

## §7 — THE RULING

E-c fails its own gate, on the criterion E-c wrote, for two independent reasons:

1. **The standoff does not resolve.** Median 1.24 → 0.42 texels across the radii,
   irradiance-weighted mean under 2 at every radius but the smallest.
2. **The lights do not reach.** 16.5–19.5 % of painting positions are lit by any
   spot at all; the rest would cast nothing at any resolution.

The work is **not landed**. Nothing was edited.

What is worth keeping from this read, for whoever picks it up:

- **(1) is free.** The gallery layout pair already reaches everything two shadow
  entry points need, including `light_vp`. E-c's expected blocker does not exist
  — the brief's `makeShadow` route invents it. Any future attempt should build
  on `galleryEntityLayout_ + galleryTextureLayout_`.
- **§5 survives.** The outdoor half faces a 4096² undivided map and casts onto
  open terrain rather than into a 0.30 wu standoff. If E-c returns, it should
  return outdoors-first.
- **The real blocker is upstream.** `L.range = wall_height + 30.0f` is a constant
  in a room that spans 150–450 wu, and `room_size × 0.8` cannot cross a room by
  construction. That is a lighting-model finding this stage turned up, and it
  bears on more than shadows — it is why most of the hang is ambient-lit. It
  belongs on the control panel, **open**, and it is not E-c's to fix.

---

## LEDGER ROW

| Stage | State | Commit | Note |
|---|---|---|---|
| E-c — the frames cast | **reported; STOPPED at (3), no work landed** | this commit | **(1) is not a blocker**: the painting pipelines never used `shadowRenderLayout`: they run on `galleryEntityLayout_ + galleryTextureLayout_`, which already carries `painting_slots` (50) AND `render_vp` — whose `VPMatrix` holds `light_vp` beside `m`. Two shadow entry points need **no new binding, layout, or bind group**; only the brief's `makeShadow` route (which hardcodes `shadowRenderLayout`) would have forced one. **(2) no ceiling**: gallery VS is 2 storage / 1 uniform against 10/12; the abandoned route would have been 8/10. **(3) STOP**: the atlas gives each spot a 2048×4096 tile from a SQUARE projection, so horizontal density is half; against `frame_depth` 0.30 the median horizontal offset is **1.24 / 0.77 / 0.54 / 0.42 texels** at radius 1–4, irradiance-weighted mean 2.68 → 1.07. The arithmetic mean (11.6–14.1) is tail-driven and misleading — the large offsets are grazing lights where the painting is barely lit. **Larger finding**: only **16.5–19.5 %** of painting positions are inside any spot's cone *and* range, because `L.range` is `wall_height + 30` — a CONSTANT — in rooms 150–450 wu across, and a sconce's `room_size × 0.8` cannot cross a room by construction. **(4)** the shell casts from every spot (`drawable_table.hpp:114`, table run unconditionally), so painting shadows are a drop shadow inside the standoff, not an occlusion. **(5)** outdoor casts into the undivided 4096² sun map and shares (1)'s answer; it does NOT hit the stop, and is where E-c should return first. **(6)** all three colour pipelines are `CullMode::None`; `renderer.hpp:1170` restores 4 → 6. Vertex arithmetic corrected: 78/frame is right for wall paintings, gallery frames are **384** (`PAINTING_QUAD_N² × 6`). `slot_high_water` is raised at both claim sites including `commit_gallery`. |
