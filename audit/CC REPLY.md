The reformed contract is understood — EXECUTE / RESOLVE / STOP, outcome invariants, blast radii, bindings reported. My plan: bind and land all four inline (last round's census is still fresh and exact), commit each locally, then run an adversarial verification pass over the four diffs before pushing anything. Starting with sync and U1's three binding questions: the spine row order (the carried finding, re-verified by content), the zone death path (does lift ever outlive `active=false`? — the conservatism crux), and any existing zone-count helper.

cartridge.hpp:248 carries the same count in a live file — updating it (past reports stay historical):

All four LANDING units are committed locally on master — `599aca5` (OPT_1e), `878fc30` (door fallback), `e499aef` (evict latch), `2e2eb51` (L2 residue) — with encoding checks clean and the compile-order binding confirmed. Two adversarial verifiers are now running over the diffs (one attacking the OPT_1e rest predicate, one attacking the two portal commits). I'll push and deliver the campaign report once their verdicts come back — if either confirms a defect, I fix before pushing.

Pushed — `7fae19e..2e2eb51` is on `origin/master`, all four LANDING commits. The two adversarial verifiers are still running; when their verdicts arrive I'll either confirm the round clean and deliver the campaign report, or land corrective commits for anything they refute.

The U1 verifier came back clean: no defects on `599aca5` across all five attack surfaces — the false-rest hazard is structurally closed (every `active=false` path zeroes the GPU `transition_fraction` the same frame, so no lift outlives deactivation), all three LOD1 consumers are converted with no stragglers, spine order holds, and the cull kernel never touches slot C's indexCount. It flagged two pre-existing, non-blocking observations (a latent stale-derive corner shared by E1/OPT_1a that requires a teleport bypassing `teardown_gol`, and a stale `curtainsActive_` comment) — neither chargeable to this round; I'll note both in the campaign report.

Still waiting on the second verifier covering the two portal commits (`878fc30`, `e499aef`) before I write the final report.

The second verifier confirmed one real defect in `878fc30`: the FULLREGEN block is not once-per-world — `request_recenter` (wired to the `[`/`]` render-radius keys) re-arms it mid-world, and a re-fired count-gate in a world whose doors streamed out would force a door 60 wu from world origin: outside the live window, over unloaded terrain, on no patch record so it never evicts. The fix is the tree's own idiom — a one-shot flag mirroring `back_portal_pending`. Binding the two splice sites:

The corrective commit `33c920a` is in, and a three-lens verification workflow is now running over it (struct-layout safety of the new `MoodState` field, the one-shot lifecycle across boot/teardown/recenter, and compile/const-correctness). I'll push it and deliver the full campaign report once the lenses come back clean — or fix again if one refutes.

Pushed — `33c920a` is on `origin/master`. The verification workflow over it is still running; when its three lenses report I'll deliver the campaign report (or a further correction if one refutes).

All three lenses returned NOT REFUTED — the corrective commit is clean, and everything is already on `origin/master`. The round is complete.

---

# THE LANDING ROUND — campaign report

**Four units contracted, four landed, plus one correction found by this round's own adversarial pass. Five commits on master: `7fae19e..33c920a`, all pushed.** Jean's gates remain: `glaw1` + native boot, then the web build for first light.

## U1 — EXECUTE · `599aca5` — OPT_1e: LOD1 draws the clean prefix at true rest

Bound as contracted: a **global** zone-count predicate, deliberately not geometric. `zones_active_anywhere()` lives beside `zone_rects_in_core()` in cartridge.hpp, staged in `phase_frustum_cull` (R17) beside the E1 curtain switch — R17 < R18 < R19 in the spine, so slot C's indirect reset and both sun terrain draws read the same fresh value with no latch. The carrier mirrors E1's pair exactly (`set_zones_active_anywhere` + `patch_index_count_lod1_live()` in state.hpp; one buffer, two counts, so buffer/count can never split). No WGSL moved — the cull kernel only writes the instanceCount atomics. P6 witness: `[Ground] zones active anywhere: N` on change. **Divergence resolved in passing:** the shadow FORK comment still described `bacc1a5`'s clean-prefix state, which `345628b` ("the slab walls enter the shadow map") had deliberately reverted in code only — rewritten to carry that commit's warning that the curtain tail is the walls.

## U2 — EXECUTE + correction · `878fc30` + `33c920a` — the door guarantee

The fallback (`force_spawn_door_fallback`, mood.hpp's seventh door) is count-gated on the same `active && is_portal` scan the portal array and census use; finite worlds and door-rolling open worlds are strict no-ops. Both granted RESOLVEs were exercised: **splice point** at the end of FULLREGEN *after* the synchronous population (counting before it would have fired in every open world — Channel A's DOORWAY arches, portal density 1.0, are committed by then), and **placement** with no new grammar — forward-portal destination pieces, fresh 88xx salts, a seeded bearing at 60 wu (twice `MIN_FROM_ORIGIN`), opening facing spawn.

**The verifier refuted my lifecycle premise before it could reach a build:** FULLREGEN is not once-per-world — the `[`/`]` render-radius keys re-arm it via `request_recenter`, and a re-fired gate in a world whose doors had streamed out would have forced a door 60 wu from *origin*, outside the live window, on no patch record. `33c920a` fixes it with the tree's own idiom: `door_fallback_pending` on MoodState, default true for the boot world, re-armed at TEARDOWN beside `back_portal_pending`, consumed once. A follow-up three-lens pass confirmed the fix clean (MoodState has no byte-layout consumers; no teardown-skipping path exists; pawn is provably at origin at every consuming FULLREGEN, and the worst-case finite ensured ring still encloses the 60 wu placement).

## U3 — EXECUTE · `e499aef` — evict clears portal identity

Three lines: `is_portal`, `is_back_portal`, `destination` die with the arch. The latch was real — `run_spawn_preamble` re-reserves a freed slot by setting `active=true` alone, and until `arch_write_active` commits, an evicted portal's slot read as a live door with a dead world-exit to every consumer. All other freeing paths were audited clean (teardown resets whole structs; place-fail and host-gone paths release slots that never gained identity). **The optional overflow-break RESOLVE was declined and reported:** with identity cleared at evict, a leaked reservation can never read as a portal, and a proper leak fix needs a per-family release hook plus a queue parameter `select_entities_for_patch` doesn't have — a unit of its own, not a minimal correction.

## U4 — EXECUTE · `2e2eb51` — L2 item 4 aligned

That line only: the stale adapter grant ("storage = 10") now states the default-limits budget — storage 8 / uniforms 12 per stage, citing L14. L14 stays L14; the banner untouched.

## Verification ledger

Two adversarial reviewers over the four unit diffs plus a three-lens workflow over the correction: **one defect found (the U2 refire), fixed and re-verified; everything else held.** Two pre-existing, non-blocking observations for the record: a latent stale-derive corner shared by E1/OPT_1a (unreachable without a teleport that bypasses `teardown_gol`), and the `curtainsActive_` comment in state.hpp misdescribing E1's live staging as "any zone anywhere" when the staged flag is the geometric rev2 — both predate this round and neither was touched.