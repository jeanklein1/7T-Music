# PANEL-0 p2 — THE MATRIX RECON (read-only; report; ONE STOP)

Read-only. Nothing touched. Goal (ratified): replace the hand-written
per-demo headers with ONE constexpr grid — pieces as rows, demos as
columns, cells as booleans — COMPLETE (foundational pieces shown as
locked rows) and CODE-MATRIX (the grid IS C++, no generation, entirely
visible), preserving the compile-time boolean fold the pipeline gate
depends on. First p-series cut with NO pixel gate. This report returns
the current surface, the complete row list, the table shape, the
free-ticking confirmation, the migration plan, and the terrain-only
confirmation. Jean stamps the grid shape; p2 cuts only after.

---

## R1 — THE CURRENT SURFACE (what the matrix replaces)

**DemoConfig** (`contracts/demo_config.hpp`) — three fields:
```cpp
struct DemoConfig {
    Roster   roster;      // D1 — which pieces exist (19 bools)
    uint32_t seed;        // D2 — world master seed
    uint32_t boot_mood;   // D2 — the mood the world wakes in
};
```

**Consumption — exactly three sites read the three fields:**
| field | consumer | use |
|---|---|---|
| `DEMO.roster` | `demos/demo.hpp:33` → `inline constexpr Roster ROSTER = DEMO.roster` | the 148 `ROSTER-GATE` sites read `ROSTER.<bit>` |
| `DEMO.seed` | `contracts/surface_services.hpp:47` → `active_seed = DEMO.seed` | WorldState boot master seed |
| `DEMO.boot_mood` | `contracts/spine_state.hpp:134` → `active = DEMO.boot_mood` | the mood the world wakes in |

**The hand-written headers** (`demos/full.hpp`, `demos/minimal.hpp`) —
each is a positional aggregate init of `DemoConfig`: a brace-list of 19
bools **in Roster field order** (a comment header names the order),
then seed, then boot_mood. full = all 19 `true`; minimal = all 19
`false`; both seed 42 / `MOOD_OPEN_DEFAULT`.

**The selection path** (the grid must slot in unchanged):
```
INCUBATE_DEMO=<name>   (default: full, demo.hpp:20-22)
   → demo.hpp token-pastes the header path (T7B_DEMO_HEADER)
   → #include demos/<name>.hpp        [defines inline constexpr DemoConfig DEMO]
   → demo.hpp: ROSTER = DEMO.roster + the FIRST EDGE static_assert
```
`DEMO` and `ROSTER` are namespace-scope `inline constexpr`; the D2
consumers (`spine_state.hpp`, `surface_services.hpp`) read `DEMO` by
include-cohort ordering (demo.hpp precedes them), not their own include.

**The drop-in requirement:** whatever the grid produces, it must still
emit `inline constexpr DemoConfig DEMO` at the same point in the same
namespace — then demo.hpp, ROSTER, the assert, and all 148 gate sites
are untouched.

---

## R2 — THE ROW CENSUS (COMPLETE; post-p1b)

**19 TICKABLE rows** (the roster bits — `contracts/roster.hpp`):
- **12 families** (PopFamily order): pyramid, arch, column, antenna,
  palm, cactus, blade, sphere, ribbon, cube, gol, gallery.
- **7 features**: pawn_aura, orbs, spot_lights, indoor_shell, portal,
  transitions, wanderers.

**4 LOCKED rows** (foundational, always-on, un-untickable — sourced
from the score census FOUNDATIONAL whitelist, so this list is
census-checked, not guessed):
| locked row | why always-on | census anchor |
|---|---|---|
| **THE SURFACE** | the stage; terrain streams in every sentence | `init_patch_system`, `stream_patches`, `teardown_surface` |
| **THE SUN** | the atmosphere is foundational; the sun-VP is computed every frame (mood authors direction) | `apply_mood`, `upload_lights`, the compute-VP sun coupling |
| **THE POINT** (witness/camera) | the parent — p1b: the point is always present, the camera is its permanent witness | spine (no bit; the witness contract) |
| **THE PAWN BODY** (slot-0) | slot 0 is never evicted; the body always exists (idles in free-fly) | `upload_agent_registries_to_gpu`, `seed_player_body`, `reseed_player_body` |

**Not rows — per-column scalars** (the D2 fields): `seed` and
`boot_mood`. The mood is the atmosphere the world wakes in (a column
value), not a piece; it stays a scalar, not a locked row.

**Clean census seam:** slot-0 body = LOCKED; slots 1+ NPCs = the
`wanderers` TICKABLE bit. The body and the wanderers are already
separate in the roster, so showing the body locked while wanderers is
tickable is honest and non-overlapping.

**THE BODY DISPOSITION (R2 flag — Jean rules):**
- **Option A (LEAN) — show it as a LOCKED row now**, visually distinct
  like surface/sun/point, with a one-line note: *"becomes tickable when
  a demo pulls a bodiless world (separate work; p1b left it
  foundational)."* Rationale: the grid is a COMPLETE census (Jean's
  word); the body always exists today; hiding it makes the census
  lie. Locked-and-noted is the honest form.
- **Option B — keep it off the grid entirely** until it is tickable.
  Cleaner rows, but the census is then incomplete (a thing that always
  exists is invisible).
- Recommendation: **A**. Either way, **p2 does NOT make the body
  tickable** — that is separate work.

Complete census: **23 rows** (19 tickable + 4 locked) + 2 per-column
scalars (seed, boot_mood).

---

## R3 — THE COLUMN + CELL MECHANICS (how the grid reads and compiles)

**The table shape — a 2D constexpr bool array, pieces × demos:**
```cpp
namespace Piece { enum { pyramid, arch, /* … */ wanderers, COUNT }; }  // 19 row indices
enum class DemoCol : int { full, minimal, /* terrain, … */ COUNT };    // demo column indices

// rows = pieces (labeled left), columns = demos (named in the header).
// LOCKED (always-on, every column): surface · sun · point · pawn-body
inline constexpr bool GRID[Piece::COUNT][(int)DemoCol::COUNT] = {
    //                     full  minimal
    /* pyramid       */  {  1,    0     },
    /* arch          */  {  1,    0     },
    /* … 12 families … */
    /* pawn_aura      */ {  1,    0     },
    /* orbs           */ {  1,    0     },
    /* … 7 features … */
    /* wanderers      */ {  1,    0     },
};
```
Rows are pieces (name down the left — the spreadsheet layout Jean
asked for); a column is a demo, read by scanning down. The LOCKED four
ride a comment banner above the grid (always-on, so not tickable cells)
— the census reads complete without pretending they can be turned off.

**The column read → a Roster (the constexpr fold the gate needs):**
```cpp
constexpr Roster column_to_roster(DemoCol d) {
    const int c = (int)d;
    return Roster{                       // aggregate init, Roster field order
        GRID[Piece::pyramid][c], GRID[Piece::arch][c], /* …19 in order… */
        GRID[Piece::wanderers][c],
    };
}
```
Every field is a `constexpr bool` pulled from the grid; the compiler
folds `column_to_roster(...)` whole, exactly as today's brace-list
folds. The Piece indices map 1:1 to Roster's declared field order (the
one ordering invariant the cut must hold — a static_assert can pin
`Piece::COUNT == 19`).

**Seed + boot_mood per column** (they were per-header D2 fields):
```cpp
inline constexpr uint32_t DEMO_SEED[(int)DemoCol::COUNT]      = { 42, 42 };
inline constexpr uint32_t DEMO_BOOT_MOOD[(int)DemoCol::COUNT] = { MOOD_OPEN_DEFAULT, MOOD_OPEN_DEFAULT };
```
Then the drop-in `DEMO` (DemoConfig unchanged, downstream byte-identical):
```cpp
inline constexpr DemoConfig DEMO = {
    column_to_roster(SELECTED),
    DEMO_SEED[(int)SELECTED],
    DEMO_BOOT_MOOD[(int)SELECTED],
};
```

**The selector — INCUBATE_DEMO → a column (two options, Jean rules):**
- **Primary (LEAN) — token-paste, retire the per-demo headers.** In
  demo.hpp: `#define T7B_COL2(x) DemoCol::x` / `T7B_COL(x) T7B_COL2(x)`
  then `constexpr DemoCol SELECTED = T7B_COL(INCUBATE_DEMO);`. The enum
  `DemoCol { full, minimal, terrain, … }` becomes the single list of
  valid names; `demos/full.hpp`, `demos/minimal.hpp` are deleted. A bad
  `INCUBATE_DEMO=xyz` → `DemoCol::xyz` → clean "no enumerator" compile
  error. Adding a demo = one enum value + one grid column + one seed +
  one mood, all in one file.
- **Conservative — thin per-demo column-selectors.** Keep
  `demos/<name>.hpp`, each now one line: `inline constexpr DemoConfig
  DEMO = demo_column(DemoCol::<name>);`. The `#include demos/<name>.hpp`
  path is literally unchanged; file-per-demo discoverability kept.
- Recommendation: **primary** (the fuller realization of "one grid, no
  hand-written headers"); the conservative option costs one thin file
  per demo but changes the selection path by zero bytes.

**DEPENDENCY EDGES — FREE TICKING CONFIRMED.** The census finds exactly
**one** legality edge: **transitions ⇒ portal** (the FIRST EDGE
static_assert, `demo.hpp:39`). The grid does NOT encode it; the assert
rides `ROSTER` (post-column-selection), so it fires identically whether
ROSTER came from the grid or a hand-written header — tick transitions
without portal → build fails at the assert, as today. The other
cross-bit reads the census turned up (`gallery || indoor_shell` teardown
at cartridge.hpp:717/1196; `column || antenna` shared pipelines,
renderer.hpp ×7) are **shared-resource GATES, not legality edges** —
they say "if either is on, do X," they forbid no combination. So no
grid encoding; the matrix feeds the resolver, never reimplements it.

---

## R4 — THE MIGRATION (full + minimal → the first two columns)

The two headers collapse to two columns, **byte-equivalent by
construction**:
- **full** column: 19 tickable `1`, seed 42, mood OPEN_DEFAULT.
- **minimal** column: 19 tickable `0`, seed 42, mood OPEN_DEFAULT.

Because `column_to_roster(full)` yields the same 19 bools in the same
order as `full.hpp`'s brace-list, and `DEMO` stays the same
`DemoConfig` with the same seed/mood, `ROSTER` folds to bit-identical
values — **demo=full and demo=minimal produce identical builds** (same
pipelines, same 148 gates, same behavior). The golden: `ROSTER`'s 19
fields + `DEMO.seed` + `DEMO.boot_mood` compared value-for-value against
the pre-matrix headers.

**Header files:** recommend **retire** (primary selector) — the grid is
the single source; keeping empty twins invites drift. If Jean prefers
discoverability, keep them as one-line column-selectors (conservative).
Either is byte-equivalent.

---

## R5 — THE SHAPE FOR TERRAIN-ONLY (the payoff)

**Expressible — CONFIRMED, with one finding Jean should see.** A
`terrain` column = every tickable `0`:
```
/* every piece row */ {  …,  0  }   // terrain: all 19 tickable OFF
```
The locked four (surface, sun, point, pawn-body) are always-on, so this
column boots — **it is minimal's column exactly.** It is a subset of
nothing; it equals minimal.

**THE FINDING:** because the pawn body is a LOCKED row (not tickable in
p2), *terrain-only as defined* (surface + point + sun, every tickable
off) **is identical to minimal** — the pawn is present but idle; you
free-fly the point over it. A terrain-only that **omits the pawn**
(pure terrain, no body at all) is **NOT expressible until the body
becomes tickable** (the deferred work). So for p3's terrain revision:
- **today**: revise terrain on **minimal** (or an identical `terrain`
  column) — the pawn exists but idles under free-fly; OR
- **body-first**: pull the body-tickable cut, then a `terrain` column
  that unticks the body row is the true bodiless terrain sentence — the
  ONE bit that would then distinguish `terrain` from `minimal`.

R5's literal ask — *can the grid express surface+point+sun, every
tickable off* — is **yes** (it's minimal, and minimal boots). Whether
p3 wants the pawn gone is the question the finding raises. Do NOT add
the column in p2 (it is trivial to add later; the point is the grid
CAN express it).

---

## R6 — SIZING & GATES

**Small.** Pure config authoring: no WGSL, no behavior change, no pixel
gate. The grid replaces two brace-lists with a 2D array + a column
reader + a selector; `DemoConfig` / `ROSTER` / the 148 gate sites are
untouched.

**Gates (named):**
- glaw1 GREEN full + minimal — both existing sentences build unchanged.
- **The byte-equivalence golden:** demo=full and demo=minimal — the 19
  `ROSTER` bits + `DEMO.seed` + `DEMO.boot_mood` value-identical to the
  pre-matrix headers (a small compile-time check, or a diff of the
  folded values).
- score census GREEN — the grid touches no gate site; the manifest is
  unchanged.
- sentinels 147/5 — unchanged (ROSTER identical, no gate added/removed).
- encodings clean UTF-8/LF.

**Risk:** only that a column is mis-set and changes a build. The golden
(R4) catches full/minimal; a new column (terrain) has no golden to
break (it is new), so its correctness is read off the grid directly —
which is the whole point (the sentence is visible).

---

## STOP — THE STAMP REQUEST

The report returns: the current surface (R1), the complete row list —
19 tickable + 4 locked, the body flagged (R2), the constexpr table
shape + the free-ticking confirmation + the column-read mechanism (R3),
the migration plan with the byte-equivalence golden (R4), and the
terrain-only confirmation with its finding (R5). Open for the stamp:

1. **The body row (R2)** — show it as a LOCKED row now (lean; complete
   census) or keep it off-grid until tickable? (p2 does NOT make it
   tickable either way.)
2. **The selector (R3/R4)** — token-paste + retire the per-demo headers
   (lean; one grid, one file per-add) or keep thin column-selector
   headers (selection path byte-unchanged)?
3. **Terrain-only (R5)** — for p3, revise terrain on minimal/`terrain`
   with the pawn present-but-idle (expressible today), or is the
   body-tickable cut a prerequisite so terrain-only can be truly
   bodiless? (Not a p2 blocker; it scopes p3.)

The seed/boot_mood-as-per-column-scalars approach (DemoConfig
unchanged, downstream byte-identical) is the recommended default;
flag if you'd rather they move into the grid some other way.

Nothing cut. p2 cuts only after the stamp.
