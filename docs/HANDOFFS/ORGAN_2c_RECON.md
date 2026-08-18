# ORGAN_2c — THE SUN MIRROR: THE EVIDENCE

Gathered read-only by ORGAN_2b U7. **No conclusions.** ORGAN_2c is cut
from this file; when it lands, this file goes (docs/HANDOFFS holds open
work orders only — absence is health).

The open bullet it serves, docs/ORGAN.md "Open for ORGAN_2":
`config.sun_direction` beside `lighting.sun.direction` — two apparent
homes for one fact, carried in from CHORD.

---

## 1 — Every consumer of `config.sun_direction` in world.wgsl

The census expected two regions. **There is one.**

`src/cartridges/the_board/realization/world.wgsl:8999`, inside
`@compute @workgroup_size(1) fn compute_vp()` (declared at 8982):

```wgsl
    // Sun VP: kite coupling — the sun orbits THE POINT at fixed
    // offset (was the pawn; the shadow box must cover
    // what the eye sees, so it follows the point's host — identical
    // when the pawn hosts, tracks the camera in free-fly).
    if (coupling_active(COUPLING_PAWN_TO_SUN_VP)) {
        vp_data.light_vp = coupling_pawn_to_sun_vp(
            point_pos(),
            config.sun_direction
        );
    }
```

The second expected region is **not** a `config` consumer.
`world.wgsl:10078`

```wgsl
    photographer_vp.light_vp = coupling_pawn_to_sun_vp(point_p, cfg.sun_direction);
```

reads `cfg`, bound at `world.wgsl:10026` as `let cfg = photographer_config;`
— a **`PhotographerConfig`**, not a `DesignConfig`:

```wgsl
struct PhotographerConfig {          // world.wgsl:9864
    sun_direction: vec3<f32>,
    azimuth: f32,
    ...
};
@group(2) @binding(160) var<uniform> photographer_config: PhotographerConfig;   // 9877
```

So the sun direction has **three** CPU-side homes, not two. The third is
filled by hand at `bodies/gallery.hpp:901-915`, from `sunDirection_` —
the same cartridge member `apply_mood_lighting` writes:

```cpp
    GPUPhotographerConfig cfg{};
    float slen = std::sqrt(c->sunDirection_[0] * c->sunDirection_[0] + ...);
    cfg.sun_direction[0] = c->sunDirection_[0] / slen;
    ...
    c->gpuState_.upload_photographer_config(queue, cfg);
```

`GPUPhotographerConfig` is `state.hpp:1914`, `sizeof == 48` (assert at 1926).

### Is `frame_r` bound, and read or read_write, in the consuming pipeline?

Declared **once**, as a uniform (read-only):

```wgsl
struct FrameR {                      // world.wgsl:6334
    lighting: Lighting,
    vp: VPMatrix,
    camera: CameraState,
    sphere_pos: vec3<f32>,
    _pad_sphere: f32,
}
@group(1) @binding(1) var<uniform> frame_r: FrameR;   // 6341
```

`compute_vp`'s pass head binds strata 0 and 1 at
`realization/render_passes.hpp:181-183`:

```cpp
    // LOOM_2 pass head: WORLD + FRAME are every pipeline's strata 0/1.
    { compute.SetBindGroup(0, c->gpuState_.world_group());
      compute.SetBindGroup(1, c->gpuState_.frame_c_group()); }
```

and `dispatch_compute_vp` (`renderer.hpp:552-560`) sets only 2 and 3
(`frame_k_state_group` / `frame_k_textures_group`).

**Stratum 1 for a compute pass is `frame_c`, not `frame_r`.** Its
contents, from the generated surface
(`realization/binding_surface.gen.inc:990-1012`, layout at 84-108):

```cpp
                // A8a: the compute face — signal + samplers only. No ro camera/vp
                // windows here, so no compute dispatch ever scopes them against their
                // rw homes (L23-prime); no dynamic seat, no offset argument.
                    entries[0].binding = bind::g1::signal;
                    entries[1].binding = bind::g1::bilinear_sampler;
                    entries[2].binding = bind::g1::nearest_sampler;
```

`frame_r` — and therefore `frame_r.lighting.sun.direction` — is
**unreachable from any compute kernel** under the A8a split.

`frame_r.lighting.sun.direction` is read at `world.wgsl:3980` and `4092`,
both in fragment-stage lighting helpers, where stratum 1 is `frame_r`.

---

## 2 — `set_sun_direction`: body and every caller

`realization/state.hpp:2953-2958`:

```cpp
            void set_sun_direction(float x, float y, float z) {
                if (config_.sun_direction[0] != x || config_.sun_direction[1] != y || config_.sun_direction[2] != z) {
                    config_.sun_direction[0] = x; config_.sun_direction[1] = y; config_.sun_direction[2] = z;
                    configDirty_ = true;
                }
            }
```

**One caller**, `direction/mood.hpp:552-562`, inside `apply_mood_lighting`:

```cpp
    // Push to GPU config so compute_vp builds the shadow VP from the correct direction.
    {
        const float len = std::sqrt(m.sun_direction[0] * m.sun_direction[0] + ...);
        c->gpuState_.set_sun_direction(m.sun_direction[0] / len,
                                    m.sun_direction[1] / len,
                                    m.sun_direction[2] / len);
    }
```

Note the comment names `compute_vp` by name as the reason the field exists.

---

## 3 — `GPUDesignConfig` around `sun_direction`

C++ (`state.hpp`, "Lighting & atmosphere" run):

```cpp
            uint32_t world_seed;              // master seed for GPU-side terrain/zone generation

            // ─── Lighting & atmosphere ──────────────────────────────
            uint32_t _pad_sun;                // WGSL aligns vec3 to 16, C++ packs
                                              // float[3] at 4 — see the GROWTH LAW
            float sun_direction[3];
            float aura_enabled;               // 0.0 = off, 1.0 = on (guards all aura sampling)
            float pawn_aura_height;
            float fog_density;                // exponential fog coefficient (default 0.003)
            uint32_t _pad_fog[2];             // ditto
            float fog_color[3];               // fog/sky color RGB
```

WGSL twin (`world.wgsl:1599-1616`):

```wgsl
struct DesignConfig {
    ...
    // (pawn_tilt_tau belongs to this Interaction run semantically — it sits in
    //  the struct's trailing pad instead; see the note at the tail. Inserting
    //  here would push sun_direction off its 16-byte boundary in THIS room only.)
    // three scalars, not array<f32,3>: core WGSL rejects stride-4 arrays in
    // uniform address space. Same 12 bytes, same offsets; CPU mirror stays
    // float[3].
    world_seed: u32,              // master seed for terrain/zone generation
    sun_direction: vec3<f32>,
    aura_enabled: f32,            // 0.0 = off, 1.0 = on (guards all aura sampling)
    pawn_aura_height: f32,
    fog_density: f32,             // exponential fog coefficient (default 0.003)
    fog_color: vec3<f32>,         // fog/sky color RGB
```

The alignment note the census asked for, in full — `world.wgsl:1706-1715`,
at `pawn_tilt_tau` in the tail:

```wgsl
    // CLOSURE_PAWN [6] — possessed body's terrain-tilt lag, seconds (0 =
    // instant). Sits at offset 556 in BOTH rooms. ... It reads as an Interaction knob and is grouped with them
    // in spirit, but it cannot sit there: this room aligns vec3<f32> to 16 and
    // the C++ room aligns float[3] to 4, so a field inserted above
    // sun_direction shifts the two mirrors by different amounts. Grow at the
    // TAIL, or pad each vec3 back onto its boundary. (state.hpp carries the
    // matching static_assert.)
```

### What removal does to both rooms' offsets

Three witnesses stand on this neighbourhood:

- `state.hpp:1689` — `static_assert(sizeof(GPUDesignConfig) == 624, ...)`.
  Its own text records the arithmetic history: `592 - 44 + 12 = 560`,
  then MOSAIC_0 `560 -> 592`, then FIELD_2b `592 -> 624`. The WGSL
  chronology matches (`world.wgsl:1753`: "sizeof 592 -> 624 (state.hpp
  carries the witness)") — the `592` figures elsewhere in world.wgsl are
  that history, not a live claim.
- `state.hpp:1705-1711` — the L4 alignment pin:
  ```cpp
        static_assert(offsetof(GPUDesignConfig, sun_direction)     % 16 == 0
                   && offsetof(GPUDesignConfig, fog_color)         % 16 == 0
                   && offsetof(GPUDesignConfig, fade_color)        % 16 == 0
                   && offsetof(GPUDesignConfig, checker_resultant) % 16 == 0,
  ```
  with the note above it: *"These four are the only offsets where the two
  rooms can disagree, and no witness here fires when they do — grow at the
  TAIL (after checker_resultant's group) or pad. The trailing 4-byte pad is
  spent, so the next knob meets this."*
- `state.hpp:2441` — `static_assert(offsetof(GPUDesignConfig, lod_point_x) == 352, ...)`,
  the targeted-write pin; `state.hpp:2433` writes `placement_patch_count` by
  `offsetof` as well.

Removing `sun_direction` removes 12 B from the C++ room and 16 B from the
WGSL room (vec3 rounds to 16), so the two rooms lose **different amounts**
— which is precisely the failure the 1706 note describes. `_pad_sun`
(4 B, C++ only) exists solely to put `sun_direction` on its boundary and
would go with it; every field after it in both rooms moves, `sizeof`
changes, and the `352` pin and all three remaining `% 16` pins are
re-derived.

---

## 4 — The generated-ledger procedure for a config-struct edit

`docs/LAWS.md` **L33 — audit/ IS THE MACHINE'S ROOM**:

> `audit/` holds **generated files ONLY**. Every tool lives in `tools/`
> (gates under `tools/gates/`). ... **everything in `audit/` is deletable
> and rebuildable by its generator**, which is the room's standing witness
> (one standing exception while ML-1 is open: `MIRROR_LEDGER.md` — see
> `docs/OPEN.md`): delete the four, run the four tools, and the tree is
> byte-identical again.

The four and their producers:

| ledger | producer |
| --- | --- |
| `audit/MANIFEST.md` | `tools/binding_gen.py` from `tools/binding_schema.py` (header: "GENERATED by tools/binding_gen.py from tools/binding_schema.py — do not hand-edit") |
| `audit/BINDING_LEDGER.md` | `tools/binding_ledger.py` |
| `audit/MIRROR_LEDGER.md` | `tools/mirror_census.py` — **frozen**, see the ML-1 note below |
| `audit/COMMAND_LEDGER.md` | `tools/command_census.py` |

`tools/binding_gen.py`'s own header states the invocations and the law:

```
#   python3 tools/binding_gen.py --bootstrap        # schema module -> stdout
#   python3 tools/binding_gen.py --check            # tree vs schema, both ways
#   python3 tools/binding_gen.py --write            # emit registry + .gen.inc
#   python3 tools/binding_gen.py --write-wgsl PATH  # renumber decls in PATH
```

> THE STANDING CHECK (L22 wiring): `binding_gen.py --check` must pass at
> every campaign's recon gate and before any commit that touches the
> binding surface. ... a nonzero exit is a STOP, and "fixing" source to
> satisfy a mistyped table is never the cure — rule on which side is wrong
> first.

> AUTHORITY DIRECTION. ... From U2 onward, per flipped mirror, the SCHEMA
> is the authority — L22, THE SCHEMA LAW, docs/LAWS.md.

**What the ML-1 freeze forbids**, `docs/OPEN.md:111-115` verbatim:

> - ML-1 / mirror_census: the span model does not recognise strataLayoutFor()
>   returning wgpu::PipelineLayout (27 sites, 57 STOPs; pre-existing at ba0e26d).
>   MIRROR_LEDGER.md is frozen at its last successful regen. Unblocked by: teach
>   the span model the strata accessor, or retire the pair — Jean's pick at the
>   next instruments sitting.

`audit/MIRROR_LEDGER.md:78` records the last successful state:
`| ML-1 | **PASS** | idiom totals sum to instance totals on every surface — R 83/83, W 98/98, L 599/599, G 781/781, G+ 0/0, P 33/33 |`

Scope note, gathered not concluded: a `DesignConfig` **member** edit changes
neither `@group`/`@binding` numbers nor pipeline layouts, so it is a
struct-mirror edit rather than a binding-surface edit; the `--check` gate is
still the campaign's recon gate by L22's wiring above.

---

## 5 — Every other writer of `config_.sun_direction`

`grep -rn "config_\.sun_direction\|config()\.sun_direction" src/` returns
exactly the two lines inside `set_sun_direction` (`state.hpp:2954-2955`).

**There is no boot seed.** `config_` is value-initialised
(`state.hpp:1973`, `GPUDesignConfig config_{};`) and the long boot-config
block does not name `sun_direction` — unlike `fog_density` / `fog_color` /
`aura_enabled`, which it does pin. Until the first `apply_mood_lighting`,
`config.sun_direction` is (0,0,0).

**`sun_direction` sits at offset 64**, named as such in the C++ room's own
prose at `state.hpp:646-657` (the `pawn_tilt_tau` note), which is the
sharpest statement of what the mirror costs:

```cpp
            // WHY IT LIVES HERE AND NOT IN ─── Interaction ───, where it
            // belongs: growth law (1) says re-use a pad, and the LAST 4 bytes
            // are that pad (the sizeof witness below names them). Appending
            // inside Interaction is NOT available — it would push
            // sun_direction from 64 to 68, and the WGSL mirror declares that
            // field vec3<f32>, whose uniform-layout alignment is 16. WGSL
            // would round 68 up to 80 while C++ sits at 68, silently
            // diverging every field from there to the end of the struct.
            // sizeof stays 592 in both rooms, so neither the sizeof witness
            // nor any compiler would have caught it. The tail pad costs
            // nothing and shifts nothing — ...
```

---

## 6 — What the ORGAN registry already says about the pair

`src/console/organ_params.inc:78` enrolls the frame_r side only:

```cpp
ORGAN_PARAM_DEF(LIGHTING, GPULighting, sun.direction, VEC3, -1.0f, 1.0f, 0.01f, "Sun", "direction", MOOD, MoodProfile, sun_direction)
```

There is no `CONFIG.sun_direction` entry, and never has been. The panel's
durable write is `MoodProfile.sun_direction`; `apply_mood_lighting` is the
one author that reads it, and that author writes **both** downstream
copies in the same function body — `c->sunDirection_` (mood.hpp:550-552,
which `gallery.hpp` later normalises into `GPUPhotographerConfig`) and
`config_.sun_direction` (mood.hpp:559 via `set_sun_direction`).

The four homes, laid out without ruling on any of them:

| # | where | written by | read by |
| --- | --- | --- | --- |
| 0 | `MoodProfile.sun_direction` (definition) | the panel, `MOOD_TABLE` seed | `apply_mood_lighting` |
| 1 | `mood_deps_.sunDirection_` (cartridge) | `apply_mood_lighting` | `upload_lights`, `gallery.hpp` |
| 2 | `GPUDesignConfig.sun_direction` | `set_sun_direction` ← `apply_mood_lighting` | `compute_vp` only (world.wgsl:8999) |
| 3 | `GPUPhotographerConfig.sun_direction` | `gallery.hpp:903-905` | `compute_photographer_vp` (world.wgsl:10078) |
| — | `GPULighting.sun.direction` (frame_r) | `upload_lights` ← 1 | fragment lighting (world.wgsl:3980, 4092) |
