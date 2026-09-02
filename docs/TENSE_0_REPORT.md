# TENSE_0 — THE TREE IN THE PRESENT TENSE · REPORT

> **Name provisional (F1-class — Jean's gate).**
> Executed on `master` (the standing instruction of this session), not on a
> `claude/tense-0-<unit>` branch. Flagged here rather than assumed.

**THE CAMPAIGN'S OWN THESIS HELD, AND HELD HARDER THAN IT CLAIMED.** The
handoff's census found the CODE nearly clean and the decay in PROSE. Every
unit confirmed that, and **five of the seven units found MORE stale prose
than the handoff enumerated** — never less. The two counts that came in
under the claim were both the same shape: a symbol the handoff believed was
inert turned out to be enrolled on the panel.

| unit | asserted | the tree actually held | verdict |
|---|---|---|---|
| U0 | two greps, both "nothing else" | **both returned beyond the expected set**, in two different ways | one item struck, one edited against the true set |
| U1 | 3 WGSL orphans, 1 hit each | exactly as claimed | struck; census fell by exactly 3 |
| U2 | 1 organ row lies | **2 organ rows lie** | both retired |
| U3 | 5 stale sites | **7** | all repaired |
| U4 | 6 comments | **6 of 12 hits** — the other 6 are correct history | 6 repaired, 6 left |
| U5 | 2 edits | exactly as claimed | both made |
| U6 | keep the row, fix the prose | as claimed; the setter has a caller | prose fixed, setter struck |
| U7 | 8 fields, "count is exactly one" | **the count is 22 for one of them** | claim CONFIRMED by a better test |

---

## U0 — VERIFY

Both greps were run tree-wide and both returned beyond the expected set.
They are **two different kinds of delta**, and the handoff rules them
differently — which is why one item was struck and the other was executed.

### U0.1 — `mute_signal`

Expected: the WGSL declaration, `signal_active`'s body, the C++ field
declaration, the boot pin. **Nothing else.** Six sites returned:

```
src/console/organ_params.inc:731   ORGAN_PARAM(CONFIG, GPUDesignConfig,
                                     mute_signal, BOOL, 0.0f, 1.0f, 1.0f,
                                     "Debug", "mute signal")
mirror_offsets.gen.inc:76-77       (generated per-field witness)
state.hpp:567                      uint32_t mute_signal;          ← expected
state.hpp:4504                     config_.mute_signal = 0;       ← expected
world.wgsl:1673                    mute_signal: u32,              ← expected
world.wgsl:2854                    return config.mute_signal == 0u; ← expected
```

**U2b's instruction was "confirm … and record the empty result". The result
was not empty: `mute_signal` IS an organ row.**

**RULED, and the handoff rules it — under its SECOND clause, not its first.**
The strike-from-unit ruling fires on "a CALLER OR READER beyond the expected
set", because a caller or reader is what would make a strike unsafe. An
enrollment line is a WRITE surface; a generated offset assert is a
mechanical dependent. Neither keeps the symbol alive. What did fire is the
other clause: *"if a grep returns a count different from the one asserted
here, record the true count, **do the edit against the true set**, and flag
the delta."* The true set includes the enrollment row, so U2b removed it.

Doing otherwise would have left the tree **strictly worse than before the
campaign**: U1 removes `mute_signal`'s last reader, so a struck U2b leaves a
Debug dial that mutes nothing — created by this campaign's own hand. The
treatment applied is the one ruled one section above, for the identical
shape.

### U0.2 — `set_config_dynamic`

Expected: the setter, the member, one read in the drain. **"No caller of the
setter anywhere."** There is a caller, and it is live code:

```
src/cartridges/the_board/surface/patch_system.hpp:97
    // New world decides its own upload frequency policy
    c->gpuState_.set_config_dynamic(false);
```

It sits at the end of `reset_surface` — the one place a world begins again.
Not a disabled branch. **`set_config_dynamic` is STRUCK from U6 and flagged;
the rest of U6 ran.** Nothing about `configDynamic_` is a falsehood: it is a
real policy switch with a real writer.

---

## U1 — THE WGSL STRIKE

**Per-strike witness, recorded before each deletion:**

```
$ rg -n '\bseg_closest\b' src/
world.wgsl:5497:fn seg_closest(p: vec3<f32>, a: vec3<f32>, b: vec3<f32>) -> vec3<f32> {

$ rg -n '\brender_point_pos\b' src/
world.wgsl:6349:fn render_point_pos() -> vec3<f32> {

$ rg -n '\bsignal_active\b' src/
world.wgsl:2853:fn signal_active() -> bool {
```

One hit each — the definition, and nothing else. No comment mentions, no
generated entries, no callers. **The handoff's claim was exact.**

**THE ORPHANING CHECK, run before striking.** Deleting `render_point_pos`
could have orphaned what it calls; it did not:

| symbol | hits | verdict |
|---|---|---|
| `render_pawn_pos` | 8 | 6 live callers besides its definition and the one inside `render_point_pos` |
| `render_pawn_vel_xz` | 2 | definition + a live call at 4685 |
| `point_camera_hosted` | 8 | many callers |

**THE CENSUS WITNESS:**

```
BEFORE  257 fn, 256 const, 55 struct, 55 binding, 34 entry points
AFTER   254 fn, 256 const, 55 struct, 55 binding, 34 entry points
```

The function count falls by **exactly 3**; **no other census row moves.**

`symbols retired cleanly` moves 15 → 18, and that is the gate working
rather than a number to chase. G-LAW 2's witness 5 is *"nothing references a
symbol the baseline had and this tree lost"*, so `baseline.json` **is** the
retirement ledger and the three names STAY in it. **`--record` was
deliberately NOT run** — it would delete the very evidence that they retired
cleanly.

---

## U2 — THE TWO PADS

Both retired **in place** to named pads; neither deleted. `sizeof` did not
move and no `offsetof` assert changed its number.

### U2a — `field_occupier_gain`, the one defect that reached the organ

The shader had said it outright since ONE_WORLD-I, in `field_sum`'s banner:
*"config.field_occupier_gain has nothing to scale"*. Its shaft emitters left
with COLUMN/ANTENNA at PRUNE_2 U4 and the arch legs with their family at
ONE_WORLD-I U3.

**It was nonetheless a live organ row with a 0…4 slider.** The REPL would
turn it; a scene file could set it; it scaled nothing. That is the
enrollment list's own law failing inside the file that states it.

Five edits, as ruled — and **a fourth prose site the handoff did not name**,
found by reading: `control_panel.hpp` said, in the present tense, *"The arch
legs are the standing family that remains, weighed through
`config.field_occupier_gain` alone."* The arch legs left three campaigns
before the reading that found it.

### U2b — `mute_signal`

See U0.1. Same treatment, executed against the true set.

### The witnesses

- `sizeof(GPUDesignConfig) == 672` — **unchanged**. The handoff said 688; that
  is a **drifted number, not a drifted requirement** — the struct went
  688 → 672 at THE_PANEL I U5a when the mosaic retired.
- `mirror_offsets.gen.inc` regenerated; **the diff is names only**:
  ```
  -  offsetof(GPUDesignConfig, mute_signal)         == 4
  +  offsetof(GPUDesignConfig, _pad_mute_signal_retired)   == 4
  -  offsetof(GPUDesignConfig, field_occupier_gain) == 544
  +  offsetof(GPUDesignConfig, _pad_field_occupier_gain_retired) == 544
  ```
  Both offsets unchanged; still 128 members across 7 structs. **One sub-delta
  the handoff did not predict**: the assert *message* also carries the WGSL
  type spelling, so `field_occupier_gain`'s reads `(u32)` instead of `(f32)`
  — the pad takes `u32` on the `_pad_arch_slack_retired` precedent. A
  spelling in a message, not a number.
- **The per-field witness earned its keep, one campaign after it was built.**
  Before regenerating, the TU gate failed with exactly two errors — *"no
  member named 'mute_signal'"*, *"no member named 'field_occupier_gain'"* —
  catching a two-room rename the day it was asked to.
- `audit/ORGAN.md` entries **326 → 324**, falling by **two**. The handoff
  predicted exactly 1; the second is U2b's true-set delta.

---

## U3 — THE AGENT ROOM

The truth, unchanged by this unit: `sizeof(GPUAgentRoomConstants) == 512`,
`behaviors` at 0, `tier_gains` at 320. **Two members. Nothing else.**

**Seven places** described the pre-ONE_WORLD block in the present tense —
the handoff named five, and reading found two more.

### U3a — the schema and the two files it stamps

Edited at the source (`tools/binding_schema.py`), never by hand in the
generated files: the REGISTRY trailing comment and text (the ruled
replacement, verbatim), the `agentsStateLayout_` prose, the
`ribbonStateLayout_` sky-rule paragraph (which lost the occupier window and
**kept the tier registry**, which is true), and both trailing binding
comments 2864 B → 512 B.

**ONE STUMBLE, RECORDED.** The ruled replacement contains an apostrophe
(*"the arches' occupier window"*) and the schema stores these as
single-quoted Python strings, so the first write broke the module.
`binding_gen` refused to import it, which is the failure mode a schema edit
should be watched for. Repaired by double-quoting the entry;
`ast.parse` now proves the file parses.

**THE WITNESS, PROVED RATHER THAN ASSERTED.** Every changed line in the two
generated files, stripped of its trailing comment, code halves compared:

```
changed: 10 removed, 13 added
NON-COMMENT code, removed side:
    inline constexpr uint32_t agent_room = 1;
    entries[1].binding = bind::g2::agent_room;
    entries[4].binding = bind::g2::agent_room;
NON-COMMENT code, added side:   IDENTICAL, all three
VERDICT: PROSE ONLY
```

No number, no seat, no visibility operand moved.

### U3b — the hand-written banners

`state.hpp`'s `agentRoomStage_` banner; `world.wgsl`'s **"THE OCCUPIER ROWS
(BATCH F-B)"** banner, which asserted in the present that *"Arch legs push
walkers as PRESENCE bodies"* three campaigns after the arch left; and the
offset line's `portals 0`. **The code below the occupier banner is
untouched**, and the banner now says why: `row_occupier` and
`occupier_contact` are a documented hold.

### Two sites the handoff did not list

- **`state.hpp` ~3740**: *"CHORD_1: one 2864 B uniform block where five
  buffers stood"* — **a stale number standing directly beside
  `sizeof(GPUAgentRoomConstants)`, which is 512 and was always right.** The
  comment and the code it annotates disagreed by a factor of five.
- **`state.hpp` ~1939** was checked and **left alone** — already past tense.
  Correct history, not decay.

---

## U4 — THE CONDUCTOR'S NAME

**The true census is 12 hits, not 6** — and the delta is the good kind:
**6 are present-tense falsehoods** (exactly the six named, at drifted line
numbers, located by symbol) and **6 are correct history**.

| repaired | left alone (past tense, correct) |
|---|---|
| `cartridge.hpp:1458`, `:2811` | `patch_system.hpp:595`, `:603` — the obituary, ruled untouchable |
| `state.hpp:669`, `:2559`, `:2568`, `:2650` | `surface_services.hpp:348` — **already honest, and not on the handoff's list** |
| | `docs/OPEN.md:162`, `:354`; `docs/reference/ATTIC.md:78` |

**Each repair names the true caller, traced rather than assumed:**
`upload_placement_patch_count` and `upload_cull_point` are both called from
`band_patches` (`patch_system.hpp:375`, `:378`, inside its body at 333–379);
`upload_patch_params` is called from **`generate_patch_batch`** (`:143`) —
not `band_patches`, and not the conductor.

**The first was doubly false, as the handoff said**: there was never a spine
row named `stream_patches` — the conductor was *called by* one — and the
real row is `RPhase::SurfaceVisibility`.

**The second half of `state.hpp:669` had also ended**: it said the CPU bands
*"into LOD0/LOD1"*, and the LOD1 band left at ONE_SURFACE-I U5. One
conductor and one band gone in the same sentence.

**The measurement is kept and re-attributed**, as ruled: 1323.04 and 1198.06
were real numbers WEB_METER_0 captured, on the row that *was*
`stream_patches` when it measured them.

---

## U5 — THE TABLE

- The PIXEL-SAFETY banner listed **six** drawables over a table of **four**.
  `shell` struck, with a tombstone: a pixel-safety claim over a drawable that
  is not in the table proves nothing about the table. Terrain's fork note is
  untouched — it is true and lives in the pass function.
- `dt_monolith` → `dt_monolith<Enc>`. One grammar; zero behaviour change;
  `drawable_table_encoder_witness` still forces both instantiations.

---

## U6 — THE HOLLOW ROW

**The row is kept and the enum untouched, as ruled.** What was wrong was that
`phase_dispatch_compute` carried both of these at once:

> *"R8's decision rides in as an argument — the card is this pass's first
> dispatch WHEN THE REST LAW ASKS FOR IT (SPINE_2 B)."*
> *"There is no verdict: the card is this pass's first dispatch, EVERY
> FRAME."*

Two campaigns' prose in one block. Collapsed to the true sentence, with the
old claim quoted in place. `phase_live_card_write` gains the banner it
lacked, naming the three reasons removing it would be a **spine edit**: the
tables are dense by `static_assert`, the score census asserts the row by
name, and it is the tombstone-holder for GROUND_CARD_1's two revival
conditions.

**And one of those conditions now has a date.** The handoff's own PROTECT
LIST records that **Jean has ruled the pulse ring reactivated** as the
coupling campaign's first voice. The banner says so: the row is held against
something scheduled, not a hypothetical.

`set_config_dynamic` — **struck by U0.2 and flagged.** See U0.

---

## U7 — `CellIdentity` — EVIDENCE ONLY, NOT EXECUTED

**The struct was not touched.** This section is the census Jean rules on.

`world.wgsl:9409`. **22 fields, 34 scalars.** The eight marked
*"authority-internal since P2"*:

**THE HANDOFF'S STATED TEST DOES NOT WORK, AND WOULD HAVE MISLED THE
RULING.** It asks for `rg -n '<field>' src/` and *"confirm the count is
exactly one"*. The true counts:

| field | `rg` hits | what the extra hits are |
|---|---|---|
| `parity` | **22** | local variables in `discrete_cell_color_at_tier`, orb code, ribbon tick-parity, prose — **a broadly colliding name** |
| `bw_roll` | **5** | 3 are a LOCAL `bw_roll` in the tier-colour function, from `hash_property(cell_seed, 830u)` |
| `color_noise` | 2 | declaration + one comment |
| `region_mean` | 1 | declaration only ✓ |
| `region_variance` | 1 | declaration only ✓ |
| `region_wander` | 2 | declaration + a comment about a deleted resolver |
| `chess_color_a` | 2 | declaration + one comment |
| `chess_color_b` | 1 | declaration only ✓ |

Run literally, the test returns **22** for `parity` and reads as a
refutation. **The claim is nonetheless true**, and the honest test proves it
decisively — member access, which is the only way a struct field can be
read or written:

```
$ rg -n '\.<field>\b' src/      # for each of the eight
parity            0        region_wander     0
bw_roll           0        chess_color_a     0
color_noise       0        chess_color_b     0
region_mean       0        region_variance   0
```

**ZERO member reads or writes, for all eight.** Not one is assigned in
`evaluate_cell_fields` (which assigns `smooth_color`, `mode`, `style`,
`sparse`, `cell_roll`, `sparse_roll`, `archetype`, `chess_eff`, `mono_eff`,
`tier`, `blend_t`, `scatter_survival` … and none of the eight), and not one
is read by `discrete_visibility_doors`, `composite_cell_color` or
`tag_cell_behavior`.

**THE FREIGHT — and it is larger than the handoff stated.**

| | |
|---|---|
| unfilled fields | 8 of 22 |
| unfilled scalars | **18 of 34 — 52.9% of the struct** |

`var id: CellIdentity;` zero-initialises, and the struct is passed **by
value** into three consumers, so the freight crosses four function
boundaries per cell on the densest per-cell path in the program. **This is
the only finding in the whole census with an ungated per-frame cost.**

**It is not CC's and not the handoff author's.** The file states *"the field
SET and order are law (ruling 3)"* (`world.wgsl:9411`) and *"Slimming the
shape is a Phase 4 ruling"* (`:9414`, restated at `:9490`). The edit is
bit-exact by construction — a field neither written nor read cannot change
an output — which is what makes it a one-word go rather than a design round.

---

## U8 — SWEEP, GATES

**The sweep.** Every identifier named in prose this campaign wrote was
checked to resolve in code, and every symbol it removed was checked for
dangling references:

```
PROXIMITY_AFFINITY · row_occupier · occupier_contact · band_patches ·
generate_patch_batch · _pad_arch_slack_retired · _pad_veil_strength_retired ·
render_pawn_pos · render_pawn_vel_xz · drawable_table_encoder_witness ·
dt_monolith · GROUND_CARD_1 · SurfaceVisibility · phase_dispatch_compute
        → all resolve

FIELD_OCCUPIER_GAIN · field_occupier_gain · mute_signal ·
seg_closest · render_point_pos · signal_active
        → 0 non-comment references each
```

**No new orphan was created.**

**The battery**, at the closing commit:

| gate | verdict |
|---|---|
| TU gate | **PASS / PASS** — two tiers, zero diagnostics |
| G-LAW 2 | **GREEN** — 254 fn, 256 const, 55 struct, 55 binding, 34 entry points; 18 retired cleanly |
| score census | **GREEN** — 7 update + 16 render rows, bijection both directions |
| WGSL gate | **PASS** — naga parses, scopes and validates, raw |
| binding surface | **PASS** — all relations agree, all witnesses |
| organ gap | **PASS** — 39 absent across the enrolled homes |
| organ ledger | **NO SUSPECTS** — 240 proved |
| mirror census | **GREEN** — 32 witnesses |
| command census | **PASS** — 9 witnesses |
| mirror offsets | **PASS** — 128 members, 7 structs |
| shell gate | **PASS** — 17 claims |

**G-LAW 1 and the probe are Jean's** and are not claimed here.

**naga was present**; no install was needed and no environment gap was
recorded.

---

## U9 — THE RECORD, THE LEDGERS, AND A MERGE THAT ARRIVED MID-CLOSE

**The record.** This file, and one section in `docs/LAWS.md` — TENSE_0 as a
landed campaign, placed beside PRUNE_1 and directly above L30. That
adjacency is not decoration: L30's own sentence reads *"A document that
describes a subsystem as it stood three campaigns ago reads exactly like a
document that describes it now — same confident tone, same file extension,
same grep hit"*, which is this campaign's thesis, written before the campaign
existed. **Nothing else in `docs/LAWS.md` was
amended**, per the handoff.

**The ledgers.** The six tools were run in the rebuild order. Four came back
byte-identical: the campaign moved prose, and prose is not a ledger input.
Two moved their provenance stamp, and they settled in **two** commits rather
than one, because `audit/BINDING_LEDGER.md` is itself an input to the mirror
census (witness ML-0) — so the binding ledger's own landing is what the
mirror ledger's next stamp has to name.

**THE MERGE.** The push was rejected: `master` had moved five commits while
TENSE_0 was executing. Two other campaigns had landed — **EMBER_0** (the FXC
arm's compiler declaration, E3's honest post-build step, `tools/ember_route_a.py`,
and **L49**) and **THE BATON** (`docs/PROCESS_LAWS.md`, a new sibling of
`docs/LAWS.md` holding the LAWS OF METHOD; and the week entered in
`docs/OPEN.md`).

`git merge origin/master` resolved with **zero conflicts**, and the reason is
worth recording rather than being relieved about: the two lines of work touch
disjoint rooms. EMBER_0 works the toolchain and `src/console/console.hpp`'s
compiler plan; TENSE_0 works the realization's comments, two `DesignConfig`
fields and three WGSL functions. `docs/LAWS.md` is the single file both wrote,
and they wrote **different sections** of it — L49 at the tail, TENSE_0 beside
PRUNE_1 — so even the one collision was a textual near-miss, not a semantic
one.

**One ledger did follow the merge.** `src/console/console.hpp` is a
command-census input and EMBER_0 grew it by 61 lines above the surface sites,
so `audit/COMMAND_LEDGER.md`'s two configure rows and its verbatim resize
branch all shifted. Content identical — same two sites, same enclosing
functions, same debounce evidence — line numbers and hash moved. That is a
third audit-only commit, and it is exactly the class of thing a hand-edited
ledger would have gotten wrong (L28).

**L33's rebuild ritual, run at the closing commit.** The five files in
`audit/` and `mirror_offsets.gen.inc` were deleted, the six tools re-run, and
`git status --porcelain` came back **empty** — the room rebuilds
byte-identically over a tree that now carries three campaigns' work.

**S-6 is green at the pushed tip:** `HEAD cb463c2 == pushed tip`, working
tree clean. The battery table above was taken before the merge; **it was
re-run in full afterwards and every row returned the same verdict**, with
S-6 additionally going from STOP to PASS.

---

## FLAGS, COLLECTED

1. **Branch.** Executed on `master`, per this session's standing
   instruction, not `claude/tense-0-<unit>`.
2. **U0.1** — `mute_signal` IS an organ row. The handoff's premise was
   wrong; the edit was made against the true set under the handoff's own
   count-delta clause. **This is the one place a reader should check my
   reasoning**, because the alternative reading strikes U2b.
3. **U0.2** — `set_config_dynamic` HAS a caller. Struck from U6.
4. **`sizeof(GPUDesignConfig)` is 672, not 688** — the handoff's witness
   number was two campaigns stale. The requirement held exactly.
5. **U2's enrollment count fell by 2, not 1.**
6. **U3, U4, U2a each found sites the handoff did not enumerate** — 2, 0
   (but 6 correctly-historical hits it did not mention), and 1 respectively.
7. **U7's stated test is unusable as written** and would have shown a
   ruling-relevant "22". The claim is confirmed by member-access census.
8. **U7's freight is 18 of 34 scalars — 52.9%**, and the struct crosses four
   function boundaries per cell by value.
9. **A schema edit broke `binding_schema.py`'s import** on first write
   (an apostrophe inside a single-quoted string). Caught by the generator,
   repaired, and `ast.parse` now witnesses it.
10. **`master` moved five commits under the campaign** — EMBER_0 and THE
   BATON landed while TENSE_0 was executing. Merged with zero conflicts;
   one audit commit (3/3) follows the merge because `console.hpp` is a
   command-census input. Flagged because the battery table above was taken
   *before* the merge and re-run after it.

## WHAT WAS NOT TOUCHED

Every item on the PROTECT LIST, verified by name: the radial pulse ring and
its five consts, `translator` and the PGA block, `row_occupier` /
`occupier_contact` / the three `OCCUPIER_*` consts (**prose only**, as
ruled), the mode trio and OrbConfig's driver-ready inlets, `draw_mask` /
`shadow_mask`, the five unreferenced WGSL consts,
`GPUPyramidInstance::half_size`, and every dead-by-declaration ABI field.

Also untouched, as ruled: the Ableton seam, the CC map, the pulse ring's
revival, the GoL vocabulary renames (priced in `docs/OPEN.md`), and
`docs/OPEN.md`'s own open rows.

## JEAN'S GATES

1. **The name.** TENSE_0 is provisional.
2. **U7** — the `CellIdentity` slimming, on the census above.
3. **The merge, and the probe.** The campaign is subtractive and prose-only,
   so L48's bar is not tripped — but **U1 and U2 change the shader's token
   stream**, so a `--probe=N` run at the tip is the acceptance of record.
4. **Deferred, named so it is not lost:** whether the occupier limb keeps
   standing for a future family or retires. U3b made its prose honest either
   way.
