# PIPE_0 — DECISION BRIEF (awaiting Jean's stamp)

One question: do the 60 pipelines compile in parallel instead of in line?

## The measurement asking for it (2026-08-07)

Native: `Total pipelines: 70,459 ms` — sixty sequential FXC + driver
compiles, itemized for the first time. Top of the bill: patch_terrain 5.5 s,
update_other_agents 5.4 s, patch_terrain_indirect 5.2 s, update_player_agent
5.0 s, pawn 4.8 s, monolith 4.2 s.

Web: the same corpus, deferred by Chromium's wire, executes in-order in the
GPU process and surfaces as the ~52 s "patch" phase. One cost, two twins,
both paying it serially.

## The shape

Switch boot-time pipeline creation to the async variants
(`CreateComputePipelineAsync` / `CreateRenderPipelineAsync`), collect all
sixty futures, and gate the end of renderer init on their completion. The
boot state machine already tolerates multi-frame init; the gate is one
barrier, not a redesign.

Expected effect:
- **Web:** Chromium compiles async pipelines on a GPU-process worker pool —
  the storm divides by the core count and stops blocking the queue, so the
  patch phase's honest ~1 s cost overlaps with it. Chromium also disk-caches
  compiled pipelines per origin: revisits skip most of the storm entirely.
  First-visit boot on phones drops to the low-seconds class.
- **Native:** Dawn's async creation path runs on its platform worker pool.
  If the standalone default pool parallelizes, 70 s becomes roughly
  70/cores — Jean's own iteration loop gets minutes of its day back.

## Invariants

- No pipeline is used before its future resolves; the single gate at end of
  renderer init preserves today's ordering guarantees exactly.
- Per-pipeline timing logs survive (stamp at future resolution), so the
  descending table remains a working instrument.
- No WGSL changes, no binding changes — outside the FXC law. A shader FXC
  would hang on still hangs, on a worker thread instead of the main one; no
  new exposure for shaders that compile today.

## Blast radius

Renderer init phase only: creation calls, a futures collection, one gate.
Boot state machine gains no new states.

## RESOLVE triggers (for the eventual handoff)

- If Dawn native's default worker pool turns out serial, report the finding
  and land the web half alone — the web is where first contact lives.
- If any async creation returns validation error where sync succeeded,
  STOP: that is a Dawn behavior difference worth its own recon, not an
  improvised workaround.

## Sequencing

After SHIP_0's phone verdict, alongside or before STREAM_0. If the phone
already boots in low seconds, PIPE_0 is still worth it for the native
iteration loop and for the weakest phones; if the phone boots slow, PIPE_0
is the first lever.
