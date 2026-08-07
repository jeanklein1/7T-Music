# SALON_1 — SUPPLY REPORT: the anamorphic squeeze, and a second uv site

Report-first. **No edit is in this commit.** Read at `cf59e6b`;
`origin/master == master` confirmed before any anchor was derived.

COMMIT 1 asks two questions before editing. Both are answered here, and the
second one turns up a site the brief does not name.

---

## §1 — HOW THE SHOT'S ASPECT MAPS INTO THE SQUARE TARGET

**It is anamorphic.** The square texture does not hold a cropped or letterboxed
view — it holds a horizontally squeezed one, and the quad stretches it back.

`build_lookat_vp` (`world.wgsl:9436-9464`), called from
`compute_photographer_vp` (`:9522`) with `cfg.aspect_ratio`:

```wgsl
let f = 1.0 / tan(fov_rad * 0.5);
let proj = mat4x4<f32>(
    vec4(f / aspect,  0.0,  ...),
    vec4(0.0,         f,    ...),
    ...
```

Standard `f/aspect` in x, `f` in y. The render target is square
(`state.hpp:2375`, `:2385`), so NDC x and y each map to the full [0,1] of the
texture. Therefore:

```
  horizontal half-FOV = atan(aspect × tan(fov/2))
  vertical   half-FOV = atan(tan(fov/2))
```

A CINEMATIC shot at `aspect = 2.39` fits **2.39× more horizontal world** into
the same pixel width as its vertical field. The stored image is compressed by
`1/aspect` along u.

The quad undoes it exactly. `fill_slot_wall_frame` (`gallery.hpp:753-756`) sets
`scale_x = base_height × aspect_ratio`, `scale_y = base_height`, and
`compute_wall_painting_geometry` builds the corners at `±scale_x/2`, `±scale_y/2`
about `slot.position`. The frame is `aspect` times wider than tall, so the
squeezed texture is stretched by `aspect` on the way out. **Squeeze × stretch =
1.** That is why a square texture has always looked correct here.

### Why this makes the uv change safe

`SNAPSHOT_RESOLUTION / PAINTING_RESOLUTION` is a **uniform** factor applied to
both u and v. A uniform scale on a square-to-square mapping leaves the
anamorphic ratio untouched — it selects a sub-square of the same shape. The
0.5 goes on both axes and the image geometry is bit-for-bit the same picture at
half the sample density. Confirmed, as the brief expected.

### The origin matches the uv, which is not automatic

`compute_wall_painting_geometry` (`world.wgsl:10091-10098`):

```wgsl
let corners = array<vec2<f32>,4>(vec2(-hw,-hh), vec2(hw,-hh), vec2(-hw,hh), vec2(hw,hh));
let uvs     = array<vec2<f32>,4>(vec2(0.0,1.0), vec2(1.0,1.0), vec2(0.0,0.0), vec2(1.0,0.0));
out.uv = uvs[ci] * vec2(s.uv_scale_x, s.uv_scale_y);
```

Scaling by 0.5 maps the quad onto `u ∈ [0,0.5], v ∈ [0,0.5]` — the **first 512
texels in both axes**, which is exactly where
`CopyTextureToTexture` with `dst.origin = {0,0,layer}` puts a 512 snapshot.
They agree. Had the uv table run v from 1 at the top, the same scale would have
sampled empty texels and the fix would have been a v-offset, not a v-scale.

---

## §2 — NOTHING ELSE READS THE OFFSCREEN DIMENSIONS

Checked exhaustively. The brief's five sites are the whole set, **plus §3**.

| candidate | verdict |
|---|---|
| `painting_array` in WGSL | `textureSample` with normalised uv only — `world.wgsl:10015`, `:10230`. **No `textureLoad`**, no size uniform, no mip assumption. Size-agnostic. |
| viewport / scissor in the snapshot pass | `render_snapshot_pass` (`gallery.hpp:1387-1457`) sets neither, so the pass defaults to the full attachment and follows the descriptor automatically. |
| `offscreen_color_texture()` | exactly one reader, `gallery.hpp:1446`, the copy the brief names as site 3. |
| `offscreen_depth_view()` | one reader, `gallery.hpp:1414`. Must match colour — both descriptors are site 2, so they move together. |
| `upload_authored_painting` (`state.hpp:2273`) | already takes `width`/`height` as parameters and writes `authoredStagingTexture_`. Untouched, as the brief says. |
| `fill_painting_layer_solid` (`state.hpp:2307`) | **has no callers** — dead code, and it targets authored staging anyway. Left alone; noted so it is not mistaken for a site. |
| `load_authored_image_to_staging` (`gallery.hpp:1463`) | derives `rec.uv_scale_x/y = dst_w/RES` against `PAINTING_RESOLUTION`. Independent of the snapshot path and stays. |

**No size uniform is fed to any shader**, so there is nothing to keep in sync
beyond the five sites and the one below.

---

## §3 — THERE ARE TWO SNAPSHOT uv SITES, NOT ONE

The brief names *"the snapshot `fill_slot_wall_frame` call"*, singular. There
are two producers of a snapshot `uv_scale` pair, and only one is a
`fill_slot_wall_frame` call:

| site | path | current | must become |
|---|---|---|---|
| `gallery.hpp:1983` | **indoor** wall frame, `fill_slot_wall_frame(..., 1.0f, 1.0f, FRAME_SNAPSHOT, …)` | `1.0, 1.0` | `SNAP/PAINT` |
| `gallery.hpp:1320-1321` | **outdoor** terrain quad, `s.uv_scale_x = 1.0f; s.uv_scale_y = 1.0f;` written inline in `commit_gallery`'s snapshot branch | `1.0, 1.0` | `SNAP/PAINT` |

The outdoor one is written directly into the slot rather than through the
helper, which is why a search for the call finds one site and a search for the
*field* finds two.

**Changing only the indoor site would break every outdoor gallery**: the
terrain quads would sample the full 1024 layer while the snapshot occupies the
top-left quarter of it, so each outdoor painting would show its image shrunk
into one corner with three quarters of cleared texture around it. Outdoor
galleries are explicitly out of scope for *intent* changes since Amendment I —
which is exactly why they must be carried through a representation change like
this one rather than left behind.

Both sites take the same derived constant. For the record, the two authored
sites (`:1278` outdoor, `:1998` indoor) pass `img.uv_scale_x/y` and are
correctly independent — they stay on `PAINTING_RESOLUTION`.

---

## §4 — ONE OBSERVATION ON THE MEMORY TABLE

The brief's table gives snapshot staging as 67 MB → 34 MB. At 32 layers of
512² × 4 B that is 33.55 MB, so the arithmetic holds. Worth stating explicitly:
**snapshot staging is the only array that gets smaller**, and it does so while
*doubling* its layer count — the resolution split pays for the layer raise
twice over on that array.

The exhibition array stays at `PAINTING_RESOLUTION` and is the largest single
allocation after the raise (40 × 1024² × 4 B = 168 MB). If the 920M refuses the
total, that array is where the next 34 MB would come from — the brief's own
fallback, `EXHIBITION_LAYERS = 32`, still clears COMMIT 4's assert at cap 7
(32 ≥ 28).

---

## LEDGER ROW

| Stage | State | Commit | Note |
|---|---|---|---|
| SUPPLY — commit 1 pre-report | **reported** | this commit | The square offscreen target is **anamorphic**, not cropped: `build_lookat_vp` puts `f/aspect` in x against a square attachment, so the image is squeezed by `1/aspect` and the quad's `scale_x = height × aspect` stretches it back — squeeze × stretch = 1. A **uniform** `SNAP/PAINT` uv factor therefore preserves it exactly, and the uv table's v-orientation happens to put the sampled sub-square at texels [0,512) in both axes, matching `dst.origin = {0,0,layer}`. **Nothing else reads the offscreen dimensions**: `painting_array` is `textureSample`-only with no `textureLoad` and no size uniform, the snapshot pass sets no viewport, `offscreen_color_texture()` has one reader, and `fill_painting_layer_solid` is dead code. **FINDING: there are TWO snapshot uv sites, not one.** The brief names the indoor `fill_slot_wall_frame` call (`:1983`); the outdoor terrain quad writes `uv_scale_x/y = 1.0f` inline at `:1320-1321`. Changing only the indoor one would shrink every outdoor gallery painting into the top-left quarter of its layer. Both take the derived constant. |
