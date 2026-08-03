# SALON_1 — B4 REPORT: what the outdoor pool must reserve

Report-first stage. **No edit is in this commit.** Read at `7e1b7f8`.

Amendment I asks two questions before the reservation number is chosen:
the paintings-per-gallery count `commit_gallery` produces, and the worst-case
indoor gallery count.

**Headline: the 32-slot placeholder in B3's arithmetic does not hold. The
geometric ceiling is 48.**

---

## §1 — PAINTINGS PER GALLERY

Three numbers in sequence; only the last is what `commit_gallery` lays out.

### 1.1 — The seed count (`select_gallery_for_patch`, `gallery.hpp:821-827`)

```cpp
    float count_raw = GalleryConfig::PAINTINGS_MEAN
        + (cpu_hash_f(seed, R1) + cpu_hash_f(seed, R2) + cpu_hash_f(seed, R3) - 1.5f)
          * GalleryConfig::PAINTINGS_SIGMA;
    uint32_t painting_count = (uint32_t)std::max((float)GalleryConfig::PAINTINGS_MIN,
        std::min((float)GalleryConfig::PAINTINGS_MAX_BY_ARCHETYPE[archetype], std::round(count_raw)));
```

Constants (`gallery.hpp:173-176`): `PAINTINGS_MEAN 5.0`, `PAINTINGS_SIGMA 2.0`,
`PAINTINGS_MIN 2`, `PAINTINGS_MAX_BY_ARCHETYPE {8, 10, 12, 12}`.

Three uniforms summed and centred gives a range of `[-1.5, +1.5]`, so
`count_raw ∈ [2, 8]` with mean 5 — a triangular-ish distribution, not a true
Gaussian. **The archetype cap only ever binds on archetype 0 (mountain, cap 8),
and only at the very top of the range.** Caps 10 and 12 are unreachable from
this expression. Recorded because the table reads as if it discriminates and
in practice it almost never does.

### 1.2 — The reservation (`place_gallery_from_selection`, `gallery.hpp:905-907`)

```cpp
    const uint32_t avail = gallery_available_staging(gs, sel.site_type);
    const uint32_t reserved = sel.painting_count < avail ? sel.painting_count : avail;
    if (reserved == 0) return false;
```

`gallery_available_staging` (`gallery.hpp:888-896`) counts unconsumed snapshot
records plus `authored_staged_count`, minus `staging_reserved`. Both staging
arrays are `Dim::STAGING_LAYERS = 16`, so **`avail ≤ 32` at any single moment**.

### 1.3 — What is actually laid out (`commit_gallery`, `gallery.hpp:1044`)

`painting_count = plan.reserved_count`, then capped again by `max_available`
(mono-tier curation and the authored load — the two things `place` cannot see).
Both only reduce.

**So: per gallery, `[2, 8]`, mean 5, and no gallery can exceed 8.**

---

## §2 — WORST-CASE INDOOR GALLERY COUNT

Galleries spawn *inside* indoor rooms — `MOOD_SPAWN_MULT` rests at identity
(`population_themes.hpp:38-43`) and gallery is `IndoorBounds::FULL`
(`indoor_module.hpp:76`). Three constraints bound how many fit.

### 2.1 — The room must contain the whole fan

`indoor_bounds_clamp` (`spawn_engine.hpp:314-345`) with `FULL` clamps
**`containment_r`**, and — unlike `MARGIN` — a collapsed box **skips the spawn**
rather than falling back to centre:

```cpp
    if (lo > hi) {
        if (bounds == IndoorBounds::FULL) {
            std::cout << "[DIAG:INDOOR-SKIP] " << family_short_name(family) ...
            return false;
```

`INDOOR_ENTITY_WALL_MARGIN = 20.0` (`indoor_module.hpp:50`). Centres are confined
to a box of side `span − 2·(20 + fan_r(n))`.

`gallery_fan_radius` (`gallery.hpp:874-880`):

| n | 1 | 2 | 3 | 4 | 5 | 6 | 8 | 10 | 12 |
|---|---|---|---|---|---|---|---|---|---|
| r | 24.17 | 28.28 | 35.32 | 43.38 | 51.85 | 60.51 | 78.10 | 95.86 | 113.71 |

### 2.2 — The spacing is NOT `MIN_GALLERY_DISTANCE`

Two separate checks apply, and the stricter one is not the obvious one.

- `select_gallery_for_patch:794-799` — centre-to-centre ≥
  `GalleryConfig::MIN_GALLERY_DISTANCE = 110.0`.
- `check_position` (`spawn_engine.hpp:580-598`) — centre-to-centre ≥
  `placing_radius + existing_radius + MIN_SEPARATION[placing][existing]`.
  `MIN_SEPARATION[Gallery][Gallery] = 30.0` (`spawn_services.hpp:112`).
  No proximity relief: `PROXIMITY_GAP_REDUCTION[Gallery] = 0.0`
  (`spawn_engine.hpp:106`), consistent with gallery's affinity row being zero.

**Effective spacing = `max(110, 2·fan_r(n) + 30)`.** For anything above a
4-painting fan the radius-sum term dominates, so the 110 in `GalleryConfig` is
not the binding rule people would assume it is — it binds only for the smallest
fans.

### 2.3 — The result

Packing centres at that spacing into that box:

| radius | span | best `n` | `fan_r` | spacing | box side | galleries | **slots** |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 1 | 150 | 5 | 51.85 | 133.7 | 6.3 | 1 | 5 |
| 2 | 250 | 4 | 43.38 | 116.8 | 123.2 | 4 | 16 |
| 3 | 350 | 7 | 69.27 | 168.5 | 171.5 | 4 | 28 |
| 4 | 450 | 3 | 35.32 | 110.0 | 339.4 | 16 | **48** |

**Peak simultaneous indoor outdoor-gallery slot demand = 48**, at radius 4:
sixteen small three-painting galleries, not a few large ones. The optimum is
*many small* because `fan_r` grows with `n` and appears twice — once shrinking
the box, once widening the spacing.

Neither global cap binds: max simultaneous galleries indoors is 16 against
`MAX_GALLERIES = 48` (`gallery.hpp:445`), and 16 footprints against
`MAX_FOOTPRINTS = 128` (`spawn_engine.hpp:64`).

### 2.4 — How reachable is 48 in practice

The bound is geometric; three things gate reaching it, none of which lowers the
ceiling:

- **Spawn chance** `{0.12, 0.24, 0.70, 0.85}` by archetype (`gallery.hpp:169`)
  — a room over basin/pool terrain approaches it, mountain rarely.
- **Content.** 48 paintings need 48 staging records over the room's life.
  Snapshots refresh (the circular buffer resets `consumed` on overwrite,
  `gallery.hpp:703`), so this accrues at ~2 captures per 50 wu walked. It is a
  matter of time spent in the room, not a hard cap.
- **`SPAWN_BUDGET_PER_FRAME = 4`** (`surface_services.hpp:132`) — a rate limit
  on patches processed per frame, not a population limit.

**Treat 48 as the number to reserve — but read it as a GEOMETRIC BOUND, not as
an expected occupancy.** Reaching it needs sixteen galleries all rolling `n = 3`
against a distribution with mean 5, landing on a near-perfect 4×4 grid at
exactly the exclusion distance, under a spawner that places randomly with
rejection. Random sequential placement reaches roughly half of grid density, so
the **realistic peak is 25–35**.

Reserve 48 regardless: a floor should be a bound, and sixteen slots is nothing
against 288. But the distinction has to travel with the number. The next
campaign to need a gallery figure will find this one in the ledger, and if it
reads "ordinary steady state" it will size a draw list to it — where the
overstatement is not cheap. A number carrying an unearned claim is the
stale-label failure mode with better grammar.

*(Corrected by Amendment III §3. The first version of this paragraph called 48
"the ordinary steady state of a large basin room"; it is not.)*

---

## §3 — WHAT THIS DOES TO B3

B3's stated derivation was:

```
  4 walls × 48 frames_per_wall          = 192   fill tier
  4 walls ×  8 max centre-band          =  32
  outdoor reservation                   =  32   <- placeholder
                                          ---
                                          256
```

With the reservation at its measured value the sum is **272**, and 256 is
**16 slots short**. Two ways to close it, both arithmetic, neither a
re-architecture:

| option | slots | fill tier | note |
|---|---:|---:|---|
| **A — raise the constant** | **288** | 48/wall | 192 + 32 + 48 = 272, rounded up for headroom. Buffer 36,864 B. |
| **B — keep 256, trim the dial** | 256 | **44/wall** | 176 + 32 + 48 = 256 exactly. |

**Recommendation: A.** The reservation is a measured floor and should not be
paid for by the dial R5 was just written around; and §5 of Amendment II already
establishes that at radius 1 the dial is area-bound rather than count-bound, so
trimming 48→44 would cost nothing where the room is small and cost real density
where it is large. 288 costs 4 KiB more of a buffer that is already noise
against the 264 MiB the subsystem holds in textures.

**Neither option is chosen here.** The number is Jean's, and it moves with
R1′/D2′ if those change the fill tier.

---

## §4 — THE RESERVATION'S SHAPE

Amendment I asks for *"a floor the wall path cannot cross, in the shape of the
`staging_reserved` pattern already in the file."* Reading that pattern
(`gallery.hpp:479`, `895`, `929`, `969-970`, `1904`), it is a counter
incremented at place, decremented at commit, and zeroed at teardown —
a **claim against a shared pool**, which is the right shape.

One asymmetry worth naming before the mechanism is written: `staging_reserved`
guards *staging records*, which both paths consume; the B4 reservation would
guard *slots*, which only `find_free_painting_slot` (`gallery.hpp:563-567`)
hands out. A floor on slots is therefore naturally expressed at that one
function rather than as a second counter — it has a single call site pair
(`commit_gallery:1067`, `place_wall_paintings:1658`) and already returns
`UINT32_MAX` on exhaustion, which both callers now handle with `break` after
Stage C.

**Reported, not chosen.** Whether the floor lives in the allocator or in a
counter beside it is a mechanism call, and mechanism calls are Jean's.

---

## LEDGER ROW

| Stage | State | Commit | Note |
|---|---|---|---|
| B4-report | **reported** | this commit | per-gallery `[2,8]` mean 5; **peak indoor demand 48, not 32**; B3 is 16 short — option A (288) recommended, not chosen |
