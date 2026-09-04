# STAGE_0 round 2 + WHEEL_0 amendments — R1–R8 · report

**Landed on `master`.** Nine commits plus two ledger settles. Every gate CC
can run is green; the probe and G-LAW 1 are Jean's.

Two rulings asked for the same thing in different words — *build the
instrument before you trust the cut* (R2) and *tombstones die honest* (R8) —
and between them they turned this round from a paperwork pass into the round
that closed four defects no gate could see.

---

## §1 — RULING BY RULING

| ruling | what it ordered | what happened |
|---|---|---|
| **R1** | ring: EXCISE; cite three prior refusals as superseded | Already excised at U2. The **record** was the work: a new OPEN.md section citing 2 original rulings, 3 refusals, 3 more that qualify, 2 supporting records |
| **R2** | extend the per-field mirror witness to `GPUFloatingEntityState` | Built. **It caught a three-campaign-old defect on the day it was written** |
| **R3** | U4+U5 as one commit | Substance satisfied, form not — and the form was never load-bearing. Recorded, not rewritten |
| **R4** | `choir_slot_seed(k) = hash(CHOIR_SEED, k)` | Landed, plus a **compile-time distinctness proof** the failure never had |
| **R5** | state the spawn-law repeal, don't slip it | Stated at the site where the law was written; 3 stale statements of its other sense swept |
| **R6** | `height_gain` 0.5; promote hands-off-rest to standing law | Both. The promotion says three things its source prose did not |
| **R7** | rebirth raiser; ε-gate confirmation; shell gate presses door 5 | All three; **3 new gate rows, each proven to bite** |
| **R8** | three wrong numbers + the phantom, at true values | All three recorded — **and the phantom had a living twin still in the tree** |

---

## §2 — THE FOUR THINGS THAT WERE ACTUALLY BROKEN

The rulings were framed as re-rulings and paperwork. Executing them found
four live defects, every one of them invisible to the whole eleven-row
battery.

### 1 · The WGSL room's offsets had been wrong for three campaigns (found by R2)

`world.wgsl`'s `FloatingEntityState` carried `behavior_phase // 188`,
`plasticity // 196`, `target_x // 200`, `target_z // 204` and `} // 208
total` — the **pre-U4 numbers** — while the C++ room's static_asserts proved
192/184/188/176. The layout was always right in both rooms; only the record
lied, and **no gate in this tree reads a comment**.

Writing the `BYTE-FOR-BYTE (192 B)` marker is what surfaced it, because
writing a marker means deriving the offsets. The witness now asserts all 33
members (161 across 8 structs, was 128 across 7) and `binding_ledger`'s
`0b-4` independently reproduces the 192 from its own layout calculator.

### 2 · `spawn_population` is a phantom declaration, still in the tree (found by R8)

R8 asked for the retired `respawn_evicted_agents` overload to be "named as a
phantom". Tracing it found **the same defect, from the same commit, alive**:

| | declared | defined | called |
|---|---|---|---|
| `respawn_evicted_agents` | 4 params | 6 params | 6 args → the definition |
| **`spawn_population`** | **6 params** | **8 params** | **8 args → the definition** |

`66752a61` (HEM_1) added `box_min, box_max` to three definitions and touched
no declaration — **two phantoms in one commit**. STAGE_0 U2 killed one only
because it was retiring that function anyway, and never looked up at the
declaration three lines above the one it was deleting.

A declaration with no definition is caught **only at link**, and nothing in
this battery links the cartridge: the TU gate type-checks, the shell gate
links a different TU, the probe never reaches it. Corrected (not deleted —
`spawn_population` is live with two real callers); the rest of `agents.hpp`
swept and clean.

### 3 · A stale reader entry was costing real coverage (found by R8)

`tools/organ_readers.py` still declared `respawn_evicted_agents` as an
`AGENTS_LIVE` reader, three campaigns after it retired. **`organ_ledger.py`
silently ignores an entry naming a function that does not exist** — 240
proved / 0 suspect before and after. A stale row there is a live reader's
coverage, lost quietly, and nothing says so.

### 4 · A second three-number banner was still standing (found by R8)

STAGE_0 U2 retired the `Dim` veil block and its roster. The **same roster,
written in the same commit (`7b26911c`)**, also sits over the boot pins in
`initializeState`, and U2 never touched it: *"ring 325 / icing 40 / lod0 175;
strength staged per frame"*. By then the ring was 342 and dark,
`GRAIN_BAND_DEFAULT` and `LOD0_RADIUS_DEFAULT` did not exist, and
`veil_strength`'s setter was retired. **Four false claims in three lines.**

---

## §3 — R8's THIRD NUMBER, AND WHY IT GOT AWAY

The ruling says "the banner's three wrong numbers … recorded at their true
values". U2's tombstone recorded **two**:

| the banner said | the constant read | recorded before? |
|---|---|---|
| RING 325 | `DRAW_RING_DEFAULT` = **342** | yes, twice |
| floaters evict at 400 | WGSL const = **800** since `309ab754` | yes |
| **ICING δ 40** | `GRAIN_BAND_DEFAULT` = **42** | **nowhere** |

The third is named for the first time in this round. It got away because the
icing was already dead, so nobody re-derived it — and it is the sharpest
illustration of the class: **all three were TRUE the day the banner was
written.** One commit (`705baec7`, "the four-mood desk lands") moved
6.5f→6.84f and 40.0f→42.0f *together*, and neither prose line followed.
The numbers were not wrong. They were **left**.

That tombstone's own arithmetic was also off — it claimed three comments in
two rooms still said 400; the re-count finds two. Recorded.

---

## §4 — R4: THE SEED, AND THE PROOF IT NEVER HAD

`choir_slot_seed(k) = cpu_hash(CHOIR_SEED, k)`. Per-key distinct kills the
monochrome; `CHOIR_SEED` kills the world-seed dependence; `CHOIR_PATCH_ROW`
and the synthetic patch retire with the recompute they existed for.

**The law is untouched, as the ruling requires.** The mix
(`base + (light − base)·I`), the variance's `(1 − I)` close, the swell, and
the silence bit-exactness all stand: at `I = 0` the projector still returns
the mirror's own draw to the last bit. Only the arithmetic upstream of
`base` moved. `active_seed` leaves four functions and two call sites with it
(CHOIR_0's D3 precedent: a dead parameter at a live seam is excised).

**And the failure is now proved away rather than merely fixed.** STAGE_0
found the flat-colour bug by hand and recorded that *"nothing asserts it; it
would simply look wrong"*. With the seed a pure function of a compile-time
key, `cpu_hash` becomes `constexpr` (the arithmetic did not move a bit; only
the keyword was missing) and `choir_palette_is_distinct()` checks all 276
pairs at build time. Counted independently: **24 distinct seeds, 24 distinct
colours, minimum pairwise RGB distance 0.0385.**

**FLAGGED — the body and the palette now share one hash stream.** Writing
R4's formula literally makes the colour seed identical to the body seed, and
that collapses what STAGE_0 called "two seeds, and the second one is not
optional" into one. It is safe **by property index rather than by luck**:
colours draw at `COLOR_R/G/B` = 150/151/152, the nine tier params at
{140,142,144,145,146,147,153,154,155} with Gaussian partners at +1000, and
the behaviour picks at `0xBEEF11A0` / `0xF10A7E70` — all disjoint, verified.
Named here as a collapse rather than slipped in as a refactor.

**For Jean's desk, as the ruling asks:** each key's colour is now a permanent
identity across all worlds. That invites an **authored per-pitch-class
palette** — twelve colours, one per pc, rank as a shade. Invited, not built.

---

## §5 — R6: THE LAW THE PROSE DID NOT STATE

`height_gain` 0.8 → **0.5**, the unique gain mapping the full field range
onto the full clamp:

| field | 0 | 1 | 2 | 3 | 4 | 5 | 6 |
|---|---|---|---|---|---|---|---|
| was, `1 + 0.8·f`, clamped | 1.0 | 1.8 | 2.6 | 3.4 | **4.0** | **4.0** | **4.0** |
| now, `1 + 0.5·f` | 1.0 | 1.5 | 2.0 | 2.5 | 3.0 | 3.5 | **4.0** |

Cost named: the quiet end is gentler, field 1 lifting 1.5× where it lifted
1.8×.

**The hands-off-rest seam is promoted to a standing OPEN.md section**, and
GROUND_VOICE_0's flag 4 becomes a pointer to it (L46: one fact, one home).
Writing it as law forced three statements its source prose never made:

1. **IT IS A SWITCH, NOT A BLEND.** The dial's value is used when the gain is
   *exactly* zero; at any other gain the coupling is the sole author. At
   `height_gain = 0.5` the output is `clamp(1 + 0.5·energy)` and the dial
   appears nowhere in it. Anyone generalising from the drivers'-room doctrine
   would write `out = dial + gain·(driven − dial)` — **and be wrong**.
2. **AN UNBOUND PIPE IS ALSO HANDS OFF**, which inverts every other seam in
   the tree: elsewhere a missing binding means the REST SPEAKS; here
   authorship goes back to the dial, because the dial *is* the rest.
3. **PER TERM IS STRUCTURAL** — two guards over one setter, so one gain at
   zero leaves its dial live while the other is driven.

Plus the test for a new coupling (*can a hand turn what I am about to
write?*) and the better answer at freeze-lift (give the dial a rest home; the
doctrine then applies again and this law stops being needed for it).

The field-jump smoothing stays unbuilt and is recorded **pre-authorized**.

---

## §6 — R7: WHERE THE REBIRTH RAISER GOES, AND WHY IT IS NOT `clear_cubes`

`choir_project` clears `repaint_all` **outside** its slot loop, so it spends
the flag even when every slot is inactive. A raise in `clear_cubes` is
therefore made at the one instant in the program's life when there is no
choir to force, and survives to the rebuilt one **only by the atomicity of
`rebirth_world`** — true today, an accident of ordering rather than a
construction, lost silently the day a teardown-only frame or a deferred
rebuild exists.

It goes at the **tail of `rebirth_world`**, after `build_world`. Every key is
born and active there, and it is rebirth-**only** — `build_world` is boot's
path too, so raising it inside `birth_the_choir` would make this a *birth*
raiser and the ledger of edges would be wrong.

**Stated as belt-and-braces, not as a bug fix with no bug.** `cube_write_gpu`
already dresses each newborn through the projector's own laws and re-seeds
the poke gate. The edge exists because `clear_cubes` **silently flips the
mode** to its rest — exactly the class the poke gate cannot see — and to keep
the edge honest if the birth ever stops routing through the projector.

### The shell gate, and the limit it must state

**`reveal_zoetrope` is not in the binary.** The door's consumer is
`organ_flush`, inside `class Cartridge`, the only includer of
`cube_behaviors.hpp`; the shell TU is two console headers. So the bit is set
and never taken.

**And the gate's own banner was citing the wrong limit for doors.** It says
"it stops exactly where `bind_home` does" — that limit stops the **write**
path. The door path never gets far enough to be stopped by it; it stops one
tier earlier, because its consumer is not linked. Corrected in the gate's own
voice.

Three new rows, **each proven to bite by breaking it**:

| row | what it pins | new coverage? |
|---|---|---|
| door 5 is the wheel's, by id and by label | the roster position U3 promised would not renumber, and the label it changed | yes — nothing pinned any label |
| a press lands its bit in the mask the frame boundary drains | `take_doors_pending()` drained and asserted == bits 3 and 5; door 99 lands none | **yes — the gate previously proved only that the acknowledgement printed** |
| two presses between two boundaries are one raise | the coalesce, which is why a scripted "full cycle" is not expressible | yes |

Also fixed: the gate's failure line said `"%d of %d scene(s)"` against the
scene count while `failed` accumulated over scenes **and** repl checks **and**
the round trip — a repl-only failure reported "3 of 5 scene(s)" with no scene
failing.

---

## §7 — FLAGS

**1 · A CONFIRMED STALE-COMMENT SET, NOT SWEPT.** Auditing R3's ordering
surfaced ~15 present-tense comments asserting mechanisms STAGE_0 U4/U5
deleted. The highest-value ones, each verified with a quote:

- `world.wgsl` THE CAP LEDGER's headline row — *"the claim ships as a
  `const_assert`"* for constants U5 deleted
- `world.wgsl` `gol_carve_fade` — *"it is `fn row_cube_push`'s Test A
  verbatim"*, naming a retired function; and `ZONE_SUPPRESS_OUTER`'s half of
  the same pair, still asserting a live twin
- `POLICY_FLYER`'s consumer census, **in both rooms** — five sites named,
  "cube kite home" among them; the count is four. (The banner's own last
  clause records that this list was miscounted once before.)
- `floaters.hpp` SEAM naming `toggle_cube_kite_mode`, from a third file
  neither U4 nor U5 touched
- `cube_behaviors.hpp`'s *"Corral / kite (the anchor law)"* heading —
  *"mode switches RIDE the follow_pawn sentinels"*
- the FIELD arc's R2/R3 rulings, listing `row_cube_push` as a live row
  awaiting Jean's word

**Not swept here on purpose.** R8's named subjects are the three numbers and
the phantom; folding an open-ended sweep into it is exactly the scope growth
this tree forbids. It wants its own unit, and it is cheap — comments only.

**2 · A DEAD WRITE THAT HAS SURVIVED THREE CAMPAIGNS.** `fe.plasticity` is
written on every birth into a field with **zero GPU readers** since U5 (which
deferred it: *"flagged for the sweep rather than cut here"*). U4 re-laid the
struct and kept it. Its organ row (`CONFIG.cube_plasticity`) still reads
"live" in the generated ledger with no runtime reader, and no gate catches it
because `organ_readers.py` puts the whole CONFIG family out of scope.

**3 · `ActiveCube::live_pos` / `live_body_radius` HAVE NO READER AT ALL.**
`reconcile_cube_mirror` writes them every frame; their last reader was the
hand-back that retired at WHEEL_0 U3. A harvest nothing consumes.

**4 · THE STANDING HOLE, NAMED.** Defect 2 above is a declaration/definition
mismatch, and **nothing in this battery links the cartridge**. Until
something does, a phantom declaration is caught by a reader or not at all.
The cheapest instrument would be a link step in the TU gate's CARTRIDGE tier
— which needs Dawn, which is why it does not exist.

---

## §8 — GATES

| gate | verdict |
|---|---|
| TU gate | **PASS** — both tiers, zero diagnostics (it is what checks R2's 33 new asserts and R4's distinctness proof) |
| shell gate | **PASS** — 5 scenes, **14** repl checks (was 11), the round trip |
| G-LAW 2 | **GREEN** — 257 fn, 249 const, 55 struct, 55 binding, 34 entry points |
| score census | **GREEN** — 7 update + 15 render rows |
| WGSL gate | **PASS** — naga, raw |
| binding surface `--check` | **PASS** (S-6 wants the pushed tip) |
| organ gap `--gate` | **PASS** — 0 surviving runtime readers across 14 graduated pairs |
| organ ledger `--check` | **PASS** — 240 proved, 0 suspect |
| mirror census | **GREEN** — 0 FAIL rows |
| mirror offsets `--check` | **PASS** — **161 members across 8 structs** (was 128 / 7) |
| L33 rebuild witness | run at the tip: five `audit/` files + `mirror_offsets.gen.inc` deleted, six tools re-run, **byte-identical** |
| **the probe** | **JEAN'S** |

**No `glaw2 --record` was needed** — GREEN without one. STAGE_0's held
`--record` diff still stands unrecorded and still needs Jean's word.

---

## §9 — THE DESK LIST

1. **The choir's new palette.** 24 authored colours, the same in every world.
   Do they read as an instrument? (§4's invitation — an authored per-pc
   palette — is one token away if they do not.)
2. **The ground at 0.5.** Fields 1–5 now have distinct heights where 4–6 were
   one; the quiet end is gentler.
3. Everything on WHEEL_0's desk list still stands — the braid at a
   hand-turned `step` above all.
