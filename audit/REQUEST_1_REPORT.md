# REQUEST_1 — THE MOOD DECLARES, THE MACHINE EXECUTES — report

Cartridge: `the_board` (`incubator_dual`). Master-direct. Deviations are
REPORT, never improvisation.

**Preflight.** Not shallow. Base:
`0475dd1142d099609151759a16c8b63a1178611a` (Jean's POINT and MOOD — the two
handoffs), `git rev-list --count HEAD` = 1015 at base. LF-only, no BOM, no
CR introduced. glaw1 GREEN at base and every commit. Symbols, never
FILE:LINE. Part 0 written whole before the first edit. [G:shader] is n/a —
this batch is CPU-side entirely; stated explicitly as the handoff requires.

---

## PART 0 — READ-ONLY

### [R0-a] The dual entry

**`apply_mood_anchor_ribbon`, the known text confirmed whole** (the ~70-line
placement body): gate on `MOOD_TABLE[mood].has_anchor_ribbon` → `rseed =
tile_seed(active_seed, 0, 0)` → seed-derived anchor (`world_c ± hash ×
spread + mood_offset`) → neutral-weight tier select →
`fill_ribbon_selection_geometry` → forced `RibbonPlacement` (slot 0, host
from `floor(ax/az / PATCH_EXTENT)`, no negotiation) → `commit_ribbon` →
`promote_ribbon_to_rendered`. MATCHED against the handoff's expectation.

**What the STREAMED path performs that this entry skips:**

1. **THE FOOTPRINT CLAIM — the one skipped act that matters.** The streamed
   path's claim site is `place_ribbon_from_selection` →
   `negotiate_position(…, /*grounded=*/true, sel.footprint_r, …,
   PopFamily::RIBBON, slot, tier)` — whose grounded arm calls
   `register_footprint`. The mood entry never runs place; no claim is ever
   filed. (`sel.footprint_r` is authored by `fill_ribbon_selection_geometry`
   itself — `FOOTPRINT_RADIUS = 5.0f` — so the mood path always HAD the
   radius in hand and never used it.)
2. **The immediate tip records** — `dispatch_commit_ribbon` records tips on
   currently-existing patches and sets `ref_count`. The mood entry calls
   `commit_ribbon` directly, so `ref_count` stays 0 and both registered
   flags false — **and this self-heals**, exactly as the handoff expected:
   `ribbon_register_tips_at` (called by the streaming conductor at patch
   spawn) lazily registers each tip and takes the reference as the world
   streams in. Tips need nothing from this batch.

So the fulfillment verb owes exactly ONE new act beyond the old body: the
claim, made directly at the forced position with the streamed path's own
key — `register_footprint(c, ax, az, sel.footprint_r, host_gx, host_gz,
RIBBON, 0, tier_idx)`. Forced, no negotiation — the world is empty at
fulfillment; the CLAIM is the point.

### [R0-b] The machine's door

`SEAM[patch:spawn-trigger]`'s face is the streaming conductor:
`stream_patches` (the S3-trigger calls select/place/commit via
`spawn_selected_patches`), dispatched from the spine's
`phase_stream_patches` (R3, `RPhase::StreamPatches`), which holds
`machine_ctx_`, `ribbon_deps_`, and the queue. **The once-per-transition
request check belongs at the head of `phase_stream_patches`, before
`stream_patches` runs** — literally "before patch-driven selection, after
the world exists": by the first StreamPatches of a new world, `apply_mood`
has run (the update-spine transition machine precedes the render spine),
`world_state_` is the new world, and no patch-driven selection has fired
yet. The fulfillment VERB lives ribbon-side (the owner); the CALL sits at
the conductor's cadence.

### [R0-c] Ordering law O-3

The current note, quoted from the transition machine:

```cpp
                        // ROSTER-GATE ribbon (c) — finite-mode release, owner
                        // verb. Zero effect
                        // when ribbon is off (active_count stays 0). ORDER
                        // (O-3): after apply_mood set mood_state_.active.
                        if constexpr (ROSTER.ribbon)
                            release_finite_ribbons(ribbon_state_, &ribbon_deps_, queue);
```

The transition order, verified in the tree: `teardown_ribbon` (the teardown
movement) → `apply_mood` (which will now DECLARE the request) → the two
mood-transition censuses → `release_finite_ribbons`. The adjusted law, as
the new shape needs it (reported here before it is written):

- The request flag is written by `apply_mood` AFTER `teardown_ribbon`
  wiped RibbonState's actives, so the teardown cannot eat it.
- `release_finite_ribbons` touches `active[]` / `active_count` /
  `rendered_slot` / the GPU slot — **never the request flag** — so the
  request SURVIVES the release by construction, and O-3's existing clause
  (release after `apply_mood` set `mood_state_.active`) is unchanged.
- The release's `has_anchor_ribbon` exemption still protects a standing
  FULFILLED anchor: the release reads the ACTIVE mood's profile, and in an
  anchor mood it never clears. A subtlety made explicit: at the transition
  the actives are already empty (teardown ran), so the release is a
  steady-state guard, not a transition actor — the request never races it.
- One NEW clause joins the note: the request is fulfilled at the streaming
  conductor's cadence (first StreamPatches of the new world), i.e. AFTER
  this release — the mood-transition census therefore reads ribn 0/0
  (a cleaner teardown-completeness assertion than today, where the
  apply-time spawn put ribn active=1 into the "must read 0" census).

### [R0-d] The portal verb is not this patient

`force_spawn_portal_arch` registers lawfully, confirmed from the tree: it
calls `(void)register_footprint(c, cx, cz, half_span, gx, gz,
PopFamily::ARCH, slot, DOORWAY)` with the deliberate no-check-position
paragraph ("It claims its ground; it does not ask for it… a full registry
cannot deny the portal"). The census logs' arch claimed == active in finite
rooms corroborates. **VERDICT: KEEP, untouched.**

**The prophecy, pasted** (the HOME (K4) comment on `force_spawn_portal_arch`):

```cpp
    // HOME (K4): MIGRATED here from mood's force_spawn_portal_at —
    // the door's written retirement condition ("when mood converts and
    // force-spawn becomes a request channel, this door MIGRATES INTO that
    // channel"), fulfilled.
```

The condition it names arrives WITH THIS BATCH — mood's force-spawn does
become a request channel — **for the ribbon**. The arch door deliberately
does not migrate: a portal is the transition's own machinery (the way in
and the way out), not a patient request. R3 truth-fixes the comment to
state that resolution instead of the ambiguous "fulfilled".

### [R0-e] The rider's mechanism — VERDICT: **CONFIRMED**, with the fix site sharpened

The three pastes:

- `ActivePatch::EntityRef` is `{family, slot}` — **the slot index only**,
  no generation, no identity. `record_entity` appends;
  **no unrecord exists anywhere** — a record dies only with its patch
  (`evict_patch_entities` wipes wholesale after running the evictors).
- `ribbon_register_tips_at` — as expected: per tip,
  `host.record_entity(RIBBON, r)` + registered flag + `ref_count++`.
- `evict_ribbon`'s decrement side: called per record at patch eviction;
  pinned (sky/wander rendered) → early return BEFORE the decrement;
  `ref_count > 1` → decrement only; final ref → full eviction.

**The hypothesis holds**: a predecessor's record on a still-alive tip patch
outlives the predecessor, and that patch's later eviction calls
`evict_ribbon(slot)` against the SUCCESSOR now in slot 0 — one death, one
patch-eviction early.

**Sharpened by the paste — which deaths actually leave a stale record on a
LIVE patch:**

| death path | records left on live patches? |
|---|---|
| patch-driven full eviction (`ref_count` 1 → 0) | **no** — the last record is on the very patch that is evicting, and it wipes its own records right after |
| `dispatch_commit_ribbon` REJECT | no — refs 0, nothing recorded |
| `release_finite_ribbons` | no — transition path; `reset_surface` wiped the registry first |
| **`release_sky_exit_ribbon` (E1's door)** | **YES — the sole creator.** The flown ribbon's tip patches can still be alive at exit; the release clears the body and never scrubs the records |

**And one hazard that dictates the fix site**: scrubbing inside
`evict_ribbon` would mutate `entity_refs[]` WHILE `evict_patch_entities`
iterates it by index (the evictor is called from inside that loop) — a
swap-with-last there would skip the swapped-in record's evictor. Since the
clean world's only stale-record creator is the sky-exit — which runs at
`phase_ribbon_tick`, outside any patch iteration — **R2's minimal exact
fix is: `ActivePatch::unrecord_entity` + the scrub in
`release_sky_exit_ribbon`**, release-by-owner completing for records at the
one door that leaks them. This is the handoff's own "cheaper exact fix"
clause, taken and said.

---

## ANCHOR VERIFICATION

| anchor | status |
|---|---|
| `apply_mood_anchor_ribbon` (fill → forced RibbonPlacement → commit_ribbon → promote) | **MATCHED** |
| `place_ribbon_from_selection`'s `negotiate_position(…, /*grounded=*/true, …)` claim | **MATCHED** |
| `commit_ribbon`'s "Dual entry" comment (SEAM[ribbon:dual-entry]) | **MATCHED** |
| the O-3 order note on `release_finite_ribbons` | **MATCHED** (quoted above) |
| `force_spawn_portal_arch`'s `register_footprint` + the K4 prophecy | **MATCHED** (pasted above) |
| `ribbon_register_tips_at` / `EntityRef{family,slot}` / the pin-before-decrement | **MATCHED** |
| "the 70-line placement body" | **MATCHED** in substance (the body is 57 lines; the handoff's count was approximate) |

## PART 0 DESIGN NOTES (recorded before the code)

- **The request carries no payload.** The handoff says "record the pending
  request (mood, seed context)"; the paste shows both live where the
  machine already reads them at fulfillment — `mood_state_.active` and
  `world_state_.active_seed` are organs, and a copied mood/seed on the
  request would be exactly the two-copies-of-one-fact shape this campaign
  has buried nine times (and a write-only field besides). The request is
  ONE bool on RibbonState, written every transition by `apply_mood` (true
  only for anchor moods — back-to-back transitions self-correct), consumed
  once at the conductor's cadence. Deviation from the letter, reported.
- `apply_mood_anchor_ribbon`'s signature shrinks with its body (the
  declaration's act needs `RibbonState&` and the mood only); the
  orchestrator's call line moves in the same commit.

## COMMIT TABLE

| commit | hash | glaw1 | encoding |
|---|---|---|---|
| REQUEST_1: Part 0 — the dual entry, the door, and the rider confirmed | `bf1c459` | GREEN (base) | LF, no BOM, no CR |
| REQUEST_1a: the mood declares, the machine executes (R1 + R3's seam restatements) | `c69e3e4` | **GREEN** | LF, no BOM, no CR |
| REQUEST_1b: release-by-owner completes for records (R2 + R3's prophecy resolution) | `c718f47` | **GREEN** | LF, no BOM, no CR |

Base `0475dd1`, master-direct. R3 rode R1 and R2 as the handoff preferred —
no third commit was forced. [G:shader] n/a — CPU-side, stated.

## THE ADJUSTED O-3 NOTE, quoted as landed

```cpp
                        // ROSTER-GATE ribbon (c) — finite-mode release, owner
                        // verb. Zero effect
                        // when ribbon is off (active_count stays 0). ORDER
                        // (O-3, adjusted REQUEST_1): after apply_mood set
                        // mood_state_.active — and apply_mood now only
                        // DECLARES the anchor (the request flag). This
                        // release touches actives, never the request, so
                        // the flag SURVIVES to its fulfillment at the
                        // streaming conductor's cadence (the first
                        // StreamPatches of the new world); the
                        // has_anchor_ribbon exemption still protects a
                        // standing fulfilled anchor.
```

## GATE STATUS

- **[G:glaw1]** — table above, all GREEN.
- **[G:shader]** — n/a: this batch never touches WGSL, a binding, or a
  layout. Stated explicitly.
- **[G:runtime-J]** — the three checks at a fixed seed:
  1. Mood-5 world: the steady-state census reads **ribn 1/1/0** — the last
     −1 is dead. (And the mood-transition census now reads ribn 0/0 — a
     cleaner teardown assertion; the anchor arrives claimed, at the
     conductor's cadence, right after.)
  2. The successor test: pre-R2 the successor died one patch-eviction
     early via the sky-exit's stale record; now it survives to its own
     lawful death.
  3. Portal rooms unchanged: arches claimed == active, back-portal
     functions — the portal door was KEEP, untouched but for its
     prophecy's resolution.
- **[G:census]** — prediction: all integers unchanged everywhere except
  the mood-5 ribn claimed 0→1, delta −1→0.
