# SALON_1 — P4 REPORT: the `clear_wall_paintings` fork

Report-first stage. **No edit is in this commit.** The amendment sets two
mutually exclusive outcomes and makes the choice turn on one question of
fact; this report answers it and stops.

Read at HEAD (`88c3046`), after P1, P2, C and P3.

---

## THE QUESTION

The §A6 finding reasons *backwards*: `evict_paintings_for_patch` carries a
`form_type == WALL_FRAME` branch keyed on `patch_gx/gz`; both indoor fill sites
pass `INT32_MAX`; therefore the branch is reachable only for slots
`commit_gallery` made; therefore the outdoor path makes `WALL_FRAME` slots.

Sound, but an inference. The amendment asks for the direct evidence:

> **Report the outdoor site that sets the `WALL_FRAME` form-type value,
> verbatim, with `path:line`.** CC confirms the enum's qualified spelling at
> source.

---

## THE ANSWER: **case (a). The site exists.**

### The enum, confirmed at source

`src/cartridges/the_board/realization/state.hpp:1616`:

```cpp
        namespace FormType { constexpr uint32_t TERRAIN_QUAD = 0; constexpr uint32_t WALL_FRAME = 1; }
```

Qualified spelling is `FormType::WALL_FRAME`; enclosing namespaces
`t7::the_board::FormType`. It is a `constexpr uint32_t` in a namespace, **not**
an enum class — so the field it lands in (`GPUPaintingSlot::form_type`, a plain
`uint32_t`, `state.hpp:1624`) carries no type-level protection. Worth saying
plainly, because it is why a form-type mix-up cannot be caught by the compiler.

### The only writer of `WALL_FRAME`

`src/cartridges/the_board/bodies/gallery.hpp:604`, inside `fill_slot_wall_frame`
(definition opens at `:590`):

```cpp
    s.form_type = FormType::WALL_FRAME;
```

Complete census of `form_type` writes and tests in `gallery.hpp` — four, and
only two are writes:

| path:line | kind | text |
|---|---|---|
| `gallery.hpp:604` | **WRITE** `WALL_FRAME` | `s.form_type = FormType::WALL_FRAME;` (in `fill_slot_wall_frame`) |
| `gallery.hpp:1171` | **WRITE** `TERRAIN_QUAD` | `s.form_type = FormType::TERRAIN_QUAD;` (in `commit_gallery`, snapshot branch) |
| `gallery.hpp:1227` | TEST | `if (gs.painting_slots[i].form_type == FormType::WALL_FRAME) {` (in `evict_paintings_for_patch`) |
| `gallery.hpp:1818` | TEST | `gs.painting_slots[i].form_type == FormType::WALL_FRAME) {` (in `clear_wall_paintings`) |

There is no third writer. `fill_slot_wall_frame` sets `WALL_FRAME`
**unconditionally** — it is not a parameter, and no caller overrides it
afterwards.

### The outdoor caller, and what it passes

`fill_slot_wall_frame` has three call sites. The patch arguments are the
discriminator:

| path:line | caller | path | last two args (`gx`, `gz`) |
|---|---|---|---|
| `gallery.hpp:1127` | `commit_gallery` | **OUTDOOR**, authored branch | **`gx, gz`** — real patch coords |
| `gallery.hpp:1734` | `place_wall_paintings` | indoor, snapshot branch | `INT32_MAX, INT32_MAX` |
| `gallery.hpp:1787` | `place_wall_paintings` | indoor, authored branch | `INT32_MAX, INT32_MAX` |

The outdoor call verbatim, `gallery.hpp:1127-1133`:

```cpp
                fill_slot_wall_frame(s,
                    paint_x, 0.0f, paint_z,
                    face_x, 0.0f, face_z,
                    img.aspect_ratio, height,
                    exh, ContentSource::AUTHORED,
                    img.uv_scale_x, img.uv_scale_y,
                    FRAME_AUTHORED, gx, gz);
```

**So outdoor AUTHORED gallery paintings are `WALL_FRAME` slots carrying real
patch coordinates.** Outdoor *snapshot* paintings are not — they take the
separate inline fill at `gallery.hpp:1161-1178` which sets `TERRAIN_QUAD`
(`:1171`). The split is by content, not by indoor/outdoor.

The WGSL room already knows this. `world.wgsl:9571-9572`:

```wgsl
    // Y-correct all outdoor paintings (terrain quads + wall frame monuments).
    // Indoor wall frames use sentinel patch coords (0x7FFFFFFF) and are skipped.
```

`0x7FFFFFFF` is `INT32_MAX`. The shader distinguishes indoor from outdoor wall
frames **by the patch sentinel, not by form type** — which is exactly the
discriminator `clear_wall_paintings` lacks.

### The defect, stated exactly

`clear_wall_paintings` (`gallery.hpp:1815-1829`) filters on `is_active` and
`form_type == WALL_FRAME` and **nothing else**. It therefore matches outdoor
authored monuments. Running it would, for each:

- free `texture_layer` in `exhibition_occupied` — a layer the outdoor slot
  still logically owns;
- set `is_active = 0` and deactivate the GPU slot;
- **not** decrement `active_painting_count` (`evict_paintings_for_patch:1233`
  does; this function has no such line), leaving that counter high;
- **not** clear the owning `gallery_centers[]` entry or unregister the
  footprint, leaving ground claimed for a gallery whose paintings are gone.

`evict_paintings_for_patch` is the correct shape for comparison: it filters on
`patch_gx/gz` **first** (`:1218-1219`) and uses `form_type` only to decide
which counter to decrement (`:1227`).

### Reachability, unchanged from §A6

Still **none today**. Both `clear_wall_paintings` call sites are downstream of
`apply_mood`; boot resolves to `MOOD_OPEN_SUNSET` and takes the other arm; the
transition path runs `teardown_gallery` (`cartridge.hpp:987`) before
`apply_mood` (`:1021`), so the slot array is empty when either fires. The
defect is armed, not firing.

Three things arm it: R1 makes the wall-frame population large and the
`active_painting_count` leak proportional; D makes `texture_layer` shared, so
the free becomes wrong for a second, independent reason; and any future
`apply_mood` that is not preceded by a teardown fires it immediately.

---

## THE FORK RESOLVES TO (a)

> **(a) It exists** → `clear_wall_paintings` gains the patch filter,
> matching the `INT32_MAX` marker both indoor fill sites already write.

Branch (b) is dead: `evict_paintings_for_patch`'s `WALL_FRAME` branch is
**live**, not dead code, because `commit_gallery:1127` produces exactly the
slots it exists to count. Do not delete it.

**The edit this authorizes, when stamped** — one predicate, in the shape the
sibling function already uses, no new mechanism:

```cpp
    for (uint32_t i = 0; i < Dim::PAINTING_MAX_SLOTS; i++) {
        if (gs.painting_slots[i].is_active != 0 &&
            gs.painting_slots[i].form_type == FormType::WALL_FRAME &&
            gs.painting_slots[i].patch_gx == INT32_MAX) {
```

One line. `patch_gz` need not be tested — both indoor fill sites write the pair
together and nothing writes them apart, so `patch_gx` alone is the marker.
Testing both is equally correct and arguably clearer; **that is a taste call,
not a correctness one, and it is Jean's.**

### One thing the edit does not fix, named so it is not assumed

The `active_painting_count` asymmetry is **independent of this filter** and
survives it. `place_wall_paintings` increments `wall_frame_count` only
(`:1749`, `:1802`), never `active_painting_count`; `clear_wall_paintings`
decrements neither in a way that would matter, since it zeroes
`wall_frame_count` wholesale at `:1827`. Indoor frames therefore never enter
`active_painting_count` at all — self-consistent, and correct today.

The leak only appears when `clear_wall_paintings` sweeps an **outdoor** slot,
which `active_painting_count` *does* include (`commit_gallery:1187`). Adding
the patch filter removes the only path by which that can happen. So the filter
fixes the counter leak as a side effect, and no counter edit is needed.
Recorded because "add the patch filter" and "fix the counter" would otherwise
look like two changes.

---

## LEDGER ROW

| Stage | State | Commit | Note |
|---|---|---|---|
| P4 — `clear_wall_paintings` fork | **reported, edit held** | this commit | resolves to **(a)**; `evict`'s branch is live, do not delete; one-line predicate proposed, `patch_gz` optional — Jean's taste |

**The edit waits for a stamp.** Per the REGISTER: where a stage says *report
first*, the report lands and is stamped before the edit is written.
