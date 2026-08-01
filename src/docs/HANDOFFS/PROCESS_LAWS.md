# THE PROCESS LAWS

Repo home: `src/docs/HANDOFFS/`. Sibling of `src/docs/LAWS.md` — that file is
the LAWS OF PRACTICE (what breaks in the program if you don't); this one is the
LAWS OF METHOD (what breaks in the *campaign* if you don't).

The method is design-in-chat → handoff → execution → boot. Prose is the medium
of the whole triangulation, and every law below was paid for: each one is the
generalization of a specific failure this campaign produced and caught.

Rules are NUMBERED and the numbers are permanent. A retired rule is struck, not
renumbered.

---

## P1 — ASSERT-AND-GUARD

Any environment fact beyond the repo boundary — include paths, driver behavior,
Dawn internals, machine state — is asserted as **hypothesis** and **guarded in
code**. The build stays green when the hypothesis fails, and the log says so
honestly.

*Paid for by:* SWEEP_1 U4 asserted `dawn/common/Version_autogen.h` was reachable
via `${DAWN_BUILD}/gen/include`. Existence was verified; reachability was not.
MSVC C1083 on every configuration, master and both arms — the whole tree
unbuildable. HOTFIX_1's cure is the pattern: `__has_include` guard, a
`T7_DAWN_VERSION` flag, and an else-branch that prints
`"Dawn revision: unavailable"` rather than failing to compile.

The tell: a handoff sentence containing "verified on the machine" about anything
the executor cannot open. That phrase means *someone looked*, not *the compiler
agrees*.

## P2 — SYMBOLS, NOT RANGES

Handoffs name boundaries by **symbol** — "the function, signature to closing
brace"; "the emission block, its opening comment to `patchIndexCount_`". Line
numbers are hints, never authority. Counts remain governed by the count law and
are verified at edit time.

*Paid for by:* CENSUS_1's first draft gave `compute_sun_matrices` as
`:517-553`. The function ran to `:588`, and the wrong `half_extent = 350.0f`
sat at `:567` — **outside** the stated range. Executing it verbatim would have
left the lie in the tree while breaking compilation. The chain
read → report → handoff → execution has now broken at every hop; a symbol
boundary cannot break, because the executor recomputes it against the tree in
front of it.

## P3 — REFUTER FOR DELETIONS

Every Class-B deletion gets **one adversarial verification pass before its
commit**, in addition to the census mandate (one reader, one refuter).

*Paid for by:* the same `:517-553`. A census refuter caught it; a deletion
handoff executed without one would not have. Deletion is where a wrong boundary
is unrecoverable — the reviewer sees a plausible diff and a green tree, or no
tree at all.

Scope note: the census mandate governs *reports*; this governs *edits*. A
deletion that rides a census still gets its own pass, because the census
verified the finding, not the cut.

## P4 — ARCHIVAL STAMP

"Filed as such" must be visible **inside the file**. Every archival document
opens with the dated stamp; being in an `old docs/` folder is not filing, it is
shelving.

*Paid for by:* `the_board_seam_map.md` corroborated LEDGER_1's wrong sun extent
at every step. It was archival by folder and authoritative by grep — a future
auditor could find it, believe it, and be wrong, which is exactly what happened
three times over. The census's own scope test is "could a reader grep it and
believe it"; the stamp is how a file answers that question itself.

Form note: the stamp is prose, so it lands in prose. Machine-parsed files
(`.json`, `.patch`) carry no comment syntax and are **not** stamped — their
filing is their path. Scripts carry the stamp in their native comment form.

## P5 — DE-NUMBER OR WITNESS

Where a number in prose merely **restates** an adjacent constant or array,
**de-number** it. Where a count does **registry work** — the bind-group entry
counts, the binding census totals — **correct** it and keep it witnessed.

*Paid for by:* the `"Live Card (512x512…)"` label against `LIVE_CARD_SIZE = 640`
(restated, so de-numbered) standing beside the `"(5 entries…)"` bind-group
comment against a 6-entry array (registry work, so corrected). Two numbers, two
opposite dispositions, one rule that tells them apart without judgment.

## P6 — EVERY SWITCH HAS A WITNESS

A runtime switch that selects behavior **logs its transitions**. Transitions
only — never per frame. A gate cannot gate what it cannot see.

*Paid for by:* ECONOMY_1 E1's first form. The flag was correct, the invariant
was sound, and the arm was **inert** — zones are alive globally almost always,
so "any zone anywhere" never released the curtains. Nothing in the log said so;
the boot looked like a pass. A silent switch is indistinguishable from a switch
that never fires, and the difference is the entire arm.

**Corollary (paid for by the Release boot):** a transition witness must also
print its state ONCE at boot, so that silence afterwards means "no transition",
not "no witness". The draw plan's Release log carried no `[Ground]` line at
all, and the two causes — no zone reached the core, or the witness never
shipped — were indistinguishable from the log.

## P7 — STRUCTURAL CHOICES INSIDE A NAMED EDIT

When a named edit cannot be executed without a structural decision — a fact
with no home, an anchor that would have to be duplicated — **the decision is
part of the edit**. Take the MINIMAL form and report it; do not stop, and do
not duplicate.

*Paid for by:* U1. Printing the zone-rect count at boot required either a
second copy of the arithmetic or one extraction. A handoff that names a print
has, without saying so, named the home of the thing printed.

## P8 — A HANDOFF IS NOT THE TREE

A law, horizon item or scheduling entry **exists only once it is committed**.
Text that lives in a handoff or in chat may be superseded silently and leave no
trace.

*Paid for by:* U2's instruction to "replace the DAWN RELEASE BUILD horizon
entry" — an entry that had never landed. The error was upstream of the
executor, and the only reason it surfaced is that the anchor was checked before
the edit rather than after.

## P9 — FETCH BEFORE YOU CLAIM

No ancestry, topology, or divergence claim without a `git fetch` in the **same
command sequence**. A remote-tracking ref is a cached label, and this campaign
family exists because cached labels lie.

*Paid for by:* the UMBRA close. Asked to merge to `master`, the executor read
`origin/master` from the clone-time remote-tracking ref, found `60818b0`, and
reported — with a table, a file count and a diffstat — that `master` and the
campaign line had **no common ancestor**, 313 files apart, and that merging was
structurally impossible. Every number in that report was correct about
`60818b0` and irrelevant to the question. A single `git fetch origin master`
returned `0466346`: the campaign's own base. The remote had moved; the ref had
not. The real operation was a fast-forward — no conflict, no force, nothing to
decide.

The tell is precise and worth memorizing: **`git log` / `git diff` /
`git merge-base` against `origin/*` read local cache, not the remote.**
`git fetch` and `git ls-remote` are the only commands in that list that talk to
the server. `--force-with-lease` is the safety net for exactly this failure and
it did fire — the one dry-run push was rejected as `stale info` rather than
executing — but a lease that fires means the analysis upstream of it was
already wrong.

Scope note: this binds *claims*, not every command. Reading `origin/master` to
answer "what did I last see?" is fine. Reading it to answer "what IS master?"
is the violation.

It is the 640-pixel card wearing a "512×512" label (P5) one layer out: there
the label was a comment, here it was a ref. A campaign family built on *read
the descriptor, not the label* produced, in its own closing act, a report
sourced entirely from a label. The rule is written down because the instinct
did not generalize on its own.

## P10 — THE GATE LOOKS BOTH WAYS

Every observation-gate row names what the edit should FIX and what the edit
could BREAK. A gate that only lists intended improvements cannot distinguish
"the fix did not work" from "the fix worked and introduced something worse".

*Paid for by:* PENUMBRA_1 P3. The handoff widened the PCF tap spacing from 1 to
2 texels and asserted the result was gapless because the taps' support intervals
touched. They touch at their zeros: a bilinear comparison tap is a tent, not a
box, and tents spaced 2 apart sum to a comb with zero weight at every odd texel.
Every shadow in the program acquired parallel banding at a 2-texel period. The
gate had a row for the sawtooth the edit was meant to soften and no row for the
filter's own shape, so the regression arrived as a surprise from the person
holding the gate rather than as a prediction from the handoff that caused it.

The tell: a handoff whose rationale contains a derivation about how something
will LOOK. Code claims are checked by the census and the compiler; a claim about
continuous mathematics is checked by nothing in the tree. Such a claim is a
hypothesis and gets a gate row of its own, phrased as the artifact it would
produce if it were wrong.

## P11 — ABSENCE IS A CLAIM ABOUT THE WHOLE FILE

A search that verifies something is **absent** is never truncated. No `head`,
no `-m`, no first-page-of-results. A search that finds something may stop at the
first hit; a search that finds *nothing* has to have looked everywhere, or it
has found nothing about nothing.

*Paid for by:* PENUMBRA_2 N1. The commit replaced a nine-tap kernel with
sixteen and attested, in its own message, "no `1/9` divisor, no `3x3` label and
no integer-offset call left in the sun path." The verification was
`grep -n "1.0 / 9.0\|3x3\|vec2<i32>" … | head`. There were 54 matches; `head`
showed 10; the surviving `3x3` label was match 27 — three lines above the
function that had just been rewritten. Absence in a truncated list was read as
absence in the file and then written into the permanent record as a checked
fact.

Scope note: this binds the *verification*, not exploration. Piping a survey to
`head` to see what a thing looks like is fine. Piping the check that a symbol is
gone is not — and the tell is the word "no" or "zero" in the sentence the search
is about to justify.

It is P9 one more layer out. There the cached label was a git ref; here it was
the shell's own output. Both times the reasoning was sound and the input to it
was a summary that had quietly dropped the disconfirming case.

---

## SCHEDULING RECORD

**CENSUS_1b — the exhaustive walk** of the four realization giants
(`state.hpp`, `world.wgsl`, `renderer.hpp`, `cartridge.hpp`) and the ten
`bodies/**` files, line by line. CENSUS_1 covered them by detector sweep and
**declared that edge**; the refuter breached it immediately (`gol_zones.hpp`'s
"Upload all 7 slots" against a 5-slot stride — an in-class miss inside a swept
directory).

**Deferred with a dated owner:** the control-panel campaign's recon, which must
enumerate those same files regardless. The walk rides that enumeration rather
than paying for it twice. This is a dated intent with an owner, which is what
P3's sibling rule in `LAWS.md` requires of anything kept — the deferral is
itself filed, not merely postponed.

**DAWN RELEASE BUILD — DONE (2026-07-29).** Dawn/Tint built `--config Release`
in the existing multi-config tree at the pinned revision `f0bf8ab5…`; the
eighty hardcoded `Debug` path segments in `CMakeLists.txt` were parameterized
to a single derived config, so the `--config`/preset choice is the one knob.
Measured effects: pipeline compile **230 s → 55 s** (most of the old boot was
Dawn's own unoptimized work, **not** FXC); Tint's own step **2548 → 281 ms**;
the CPU frame budget **15 → ~2 ms**.

*Standing consequence:* every CPU number recorded before this date is
Debug-inflated and is **retired**.
*Live consequence:* `NDEBUG` is now defined — asserts are gone in the shipped
configuration, and the assert/witness census is the gate on that.

*The FXC reframe, beside it:* FXC's cost is now visible rather than buried —
`patch_terrain` 4.9 s, `patch_terrain_indirect` 4.8 s, `monolith` 3.7 s,
`pawn` 3.5 s of a 55 s boot. FXC's **behavior** (the `world.wgsl` banner's hang
cliff) is unchanged by build configuration; only its **price** is now
measurable.
