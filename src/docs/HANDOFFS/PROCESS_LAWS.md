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
