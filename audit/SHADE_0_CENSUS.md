# SHADE_0 — FOUNDING CENSUS (the derivable half)

SHADE RIDER rides the FORMAT_1 gate as **observations only, zero edits**.
The observations need eyes on pixels and cannot be produced here. What
**can** be produced here is everything the walk should not have to
rediscover: the arithmetic that decides how dark a shadow is allowed to
get, and the per-mood numbers already in the tree.

So this file is half-written on purpose. The tables have their derived
columns filled and their observed columns blank. Fill the blanks during
the FORMAT_1 walk; the campaign then starts from a census rather than
from a blank page.

Read-only. Nothing in `src/` was touched to produce it.

## THE MECHANISM — how dark a shadow can get, exactly

Three lines of `world.wgsl`, verbatim, decide the whole question.

The composition:

```
let ambient = base_color * render_lighting.sun.ambient;
let lit = ambient + sun + points + spot;
```

The sun term:

```
fn calc_directional_light(world_pos, normal, geo_normal) -> vec3<f32> {
    let ndotl = max(dot(normal, light_dir), 0.0);
    var shadow = 1.0;
    if (render_lighting.spots.count == 0u) {
        shadow = sample_shadow_pcf(world_pos, geo_normal);
    }
    return render_lighting.sun.color * render_lighting.sun.intensity * ndotl * shadow;
}
```

> **Note added by CLOSE_0 A3.** The quoted block elides the guard's
> comment, so no stale text is reproduced here — but the comment that
> stood beside it when this census was written has since been replaced.
> It read *"light_vp is being used for spot atlas tiles, so the
> directional PCF would sample wrong"*, which described a mechanism
> ATLAS_1revB retired: `light_vp` is no longer overwritten, because the
> per-tile copy is gone. **The guard is still correct, for a different
> reason** — indoors the sun map's *content* is spot atlas tiles 0–1, so
> a sun-matrix sample would read spot depths. The live rationale is in
> `calc_directional_light`; the arithmetic below is unaffected either way.

**Two facts follow, and they are the whole census.**

**1. `shadow` multiplies the sun term ONLY.** `ambient` is outside it.
So a fully shadowed surface never falls below `base_color × sun.ambient`.
Ambient is the floor, and the floor is not a shadow parameter — it is a
lighting one. **Shadow contrast is a ratio, not a shadow setting:**

```
darkest / lit  =  ambient / (ambient + intensity · ndotl)
```

**2. Indoors, the sun casts no shadow at all.** `spots.count != 0u`
forces `shadow = 1.0`, so the sun term is *always* fully lit indoors.
Only the spot term can be shadowed there — and it too sits on top of the
same ambient floor, plus an unshadowable sun term.

## THE NUMBERS ALREADY IN THE TREE

`MOOD_TABLE`, `contracts/spine_state.hpp` — `sun_intensity` and
`sun_ambient` are columns 7 and 8 of each row:

| mood | `sun_intensity` | `sun_ambient` | sun shadows? | darkest/lit at `ndotl = 1` | contrast |
|---|---|---|---|---|---|
| `MOOD_OPEN_SUNSET` | 0.90 | 0.20 | **yes** (no spots) | 0.20 / 1.10 = **18%** | strong — 82% drop |
| `MOOD_FINITE_OUTDOOR` | 0.80 | 0.25 | **yes** (no spots) | 0.25 / 1.05 = **24%** | strong — 76% drop |
| `MOOD_INDOOR_FLAT` | 0.35 | 0.35 | **no** — guarded off | see below | weak |
| `MOOD_INDOOR_VAULT` | 0.35 | 0.35 | **no** — guarded off | see below | weak |

### The indoor floor, worked

Indoors the sun term is unshadowable and ambient is half the pair. On a
floor with the indoor sun direction `(0.20, −0.90, 0.00)` — mostly
straight down, so `ndotl ≈ 0.9` on flat ground:

```
unshadowable light  =  ambient        +  sun·ndotl
                    =  0.35           +  0.35 × 0.9   =  0.665
```

A spot light adds on top of that, and a spot **shadow** can remove only
what the spot added. **Roughly two-thirds of an indoor floor's light is
beyond the reach of any shadow the program can cast.** That is the
mechanical reason indoor shadows read faint, and it is a *lighting*
fact, not a shadow-map fact — no resolution, format, bias or PCF change
touches it.

## THE KNOBS, PRICED

The handoff's estimate — approximately zero GPU — is confirmed by the
arithmetic: every candidate is a CPU-side float already uploaded once
per mood change inside the 848-byte lighting buffer. No new binding, no
new buffer, no shader change, no per-frame cost.

| knob | where | what it does | GPU cost |
|---|---|---|---|
| `sun_ambient` ↓ | `MOOD_TABLE` column 8 | lowers the floor — deepens **every** shadow, and darkens unlit surfaces with it | zero |
| `sun_intensity` ↑ | `MOOD_TABLE` column 7 | raises lit without raising the floor — widens the ratio | zero |
| the `spots.count == 0u` guard | `calc_directional_light` | restoring indoor sun shadow. **Not free and not a knob**: indoors the sun map holds spot tiles 0–1, so a live indoor sun PCF would sample spot depths under a sun matrix. Needs its own content — a dedicated tile or arm. | a tile's worth |

The first two are one-line-per-mood edits with an immediate visual
answer. The third is a campaign.

## THE WALK SHEET — fill during the FORMAT_1 gate

Observations only. The question in every row is **faint or present**,
and where the answer is "faint", whether the floor above explains it.

| # | condition | shadows read | does the ambient floor explain it? | note |
|---|---|---|---|---|
| 1 | outdoor sunset, midday-facing wall | | | |
| 2 | outdoor sunset, ground under the pawn | | | |
| 3 | outdoor sunset, ground under an arch/column | | | |
| 4 | finite outdoor, same three | | | |
| 5 | indoor 2 lights (Gallery), floor under the pawn | | | |
| 6 | indoor 2 lights, wall behind a monolith | | | |
| 7 | indoor 4 lights (Cathedral/Quartet), floor under the pawn | | | |
| 8 | indoor 4 lights, do overlapping spot shadows read at all | | | |
| 9 | **dark pawn on light floor** — contact read | | | |
| 10 | **light pawn on dark floor** — contact read | | | |
| 11 | any surface where a shadow is *missing* that should be there | | | |

Two extra columns worth a word each, since FORMAT_1 is in the same
binary and its own gate asks for them:

| # | condition | acne? (dark moiré on lit faces) | peter-panning? (shadow off the feet) |
|---|---|---|---|
| A | outdoor sunset | | |
| B | finite outdoor | | |
| C | indoor 2 | **slight** — see below | |
| D | indoor 4 | | |

### The acne row, answered — and declined as a detour

Jean, on the `9489b8d` Vulkan boot (the FORMAT_1 gate sitting):

> *"there is a little bit of acne in the indoor room, it's not much, and
> it isn't a fruit of this session."*

Recorded as **observed, pre-existing, and not pursued.** Three reasons
it is the right call and not a shrug:

1. **It is not a FORMAT_1 regression.** `depth16unorm` halves depth
   precision, which is the change most likely to *manufacture* acne — so
   pre-existing acne that did not worsen is the format gate passing on
   its hardest question, not failing on a soft one.
2. **The bias is already priced.** `renderer.hpp` carries the PENUMBRA_1
   P2 comment with the unorm inversion worked out (~0.00915 wu per bias
   unit). Whoever picks this up starts from arithmetic, not from a knob.
3. **It is the wrong lever for the visible complaint.** SHADE_0's whole
   finding is that indoor shadows read faint because ~two-thirds of an
   indoor floor's light is unshadowable ambient plus unshadowable sun.
   Bias tuning cannot touch that. Deepening the shadows first would also
   change how visible the acne is, which means tuning bias *before*
   SHADE_0 is tuning against a floor that is about to move.

**So the order is: SHADE_0's four floats first, then re-sit this row.**
If the acne is still worth an hour after the floor moves, it is a bias
pass with a known conversion factor and a one-line edit.

### The indoor sun row, answered by mechanism rather than by eye

Jean, same sitting:

> *"about sunlight causing shadows indoors, never happened, since we
> began to work on this project."*

**Expected, and confirmed by the code above rather than contradicted by
it.** `calc_directional_light` forces `shadow = 1.0` whenever
`spots.count != 0u`, which is every indoor mood. There has never been an
indoor sun shadow to lose, and nothing this session was scoped to change
that: the guard's *rationale* was corrected by CLOSE_0 A3, its *logic*
was left byte-identical. Restoring indoor sun shadow is the third row of
THE KNOBS, PRICED — a tile's worth of content and a campaign of its own,
not a knob.

## WHAT SHADE_0 WILL ASK, ONCE THE BLANKS ARE FILLED

Not "make shadows darker" — the census already says how, and that it is
free. The real question the numbers pose:

**Is the indoor pair `0.35 / 0.35` a choice or an inheritance?** A 1:1
intensity-to-ambient ratio is a flat-light look, and it is the *same*
pair on both indoor moods while the two outdoor moods differ from each
other. If it was chosen, SHADE_0 is a tuning pass. If it was copied, it
is a design question that has never been asked.

FORMAT_1 changes no texel on purpose. SHADE_0 will change them on
purpose — and thanks to the arithmetic above, it will know the price
before it starts: zero GPU, one gate sitting, four floats.
