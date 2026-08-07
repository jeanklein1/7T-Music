# PROBE_1 — BOOT RUNBOOK (Jean's hands)

Four boots, one optional. Boots 2 and 3 carry the measurements; boot 1
is a 60-second verdict; boot 4 is curiosity. Total session ≈ 45–55 min.

## SESSION ORDER (before any boot)

1. Copy `cc_handoff_census_1.txt` (the parent census) into
   `src/docs/HANDOFFS/OPTIMIZATION 1/` — CC's RUN 4 needs it in-tree.
2. Run CC on `cc_handoff_census_2.txt`. Review its report — the
   LAWS.md diff sits at the top for your stamp.
3. Run CC on `cc_handoff_probe_arms.txt`. Note the four branch hashes
   it reports.
4. `git pull` on the machine. Then the boots below.

## PREP (once, and it holds for every boot)

- **AC power.** Windows power plan: High performance / Best
  performance slider. On battery both GPUs throttle and no two
  windows are comparable.
- Close Ableton, browsers, Epic/Steam launchers. loopMIDI stays (it
  is part of every boot; keep it constant).
- **Do not resize or move the window.** Default startup size, same
  monitor, all boots. (Resize changes main_pass; it would poison the
  A/B.)
- Capture every boot to a file. PowerShell, from the exe's directory:

      .\incubator_dual.exe *>&1 | Tee-Object -FilePath ..\logs\bootN_<name>.log

  (cmd fallback: `incubator_dual.exe > log.txt 2>&1` — no live view.)
- Build as usual (glaw1) after each checkout. **Boot 2's build IS the
  glaw1 gate for the CENSUS_2 commits** — if it fails to compile,
  stop and send CC the error; nothing else proceeds.

## SCENE PROCEDURE (used in boots 2 and 3, identically)

Per scene: settle, **discard the first METER window**, then copy the
next **three full windows** (each ≈ 30 s). The tee has them; just
note the three `t=` values per scene.

- **S1 — outdoor resting.** From boot, hands off. Discard window 1
  after `renderer ready`, copy windows 2–4.
- **S2 — outdoor, GoL over the LOD0 core.** Watch the `[GoL]` lines;
  walk the pawn to within ~2 patches of a printed corner, stop.
  Discard one window, copy three.
- **S3 — indoor, most live spot lights.** Take a portal to that mood.
  **Note which portal — repeat the same one in boot 3.** Settle,
  discard one, copy three.

What matters from each window: `shadow_pass` and `main_pass` rows
(cpu and gpu, mean/max) + the `fps` line. Copying whole windows is
fine.

## BOOT 1 — C3, the L2.4 adjudicator (~6 min)

    git checkout claude/probe1-c3-two-indirect
    git log --oneline -3        # confirm the C3 commit title
    <build>  → run with tee → boot1_c3.log

Checklist:
- [ ] `Adapter selected: index=2` (C3 sits on the merged master)
- [ ] Watch ~60 s past `[Incubator] the_board renderer ready` for any
      `WebGPU Error` line. Also confirm the world simply renders.
- [ ] Verdict: **PERMISSIVE** (silence, world normal) or
      **REJECTED** (copy the error verbatim).

Kill the run. This verdict decides the fate of seven comment homes
and unlocks (or refutes) the LOD1/shadow indirect architecture.

## BOOT 2 — master baseline on the chosen machine (~15–18 min)

    git checkout master
    git log --oneline -8        # C1b merge + CENSUS_2 commits present
    <build>  → run with tee → boot2_baseline.log

Boot-line checklist (first ~30 s):
- [ ] `Adapter selected: index=2` — the 920M, D3D12.
- [ ] `[Console] Adapter limits:` for the 920M — **record it**. If
      `storageBuffers/stage` prints below 10 or the boot fails at
      buffer/bind-group creation, stop: that failure is data, send it.
- [ ] The features line **re-read on the 920M**:
      `timestamp-query=YES` is mandatory (if `no`, stop — the METER's
      GPU columns are dark; report). Record `multi-draw-indirect=`.
- [ ] Dawn revision line, if CC landed commit 3 — record.
- [ ] `[Cartridge] Terrain gen:` line is **gone** (CENSUS_2b witness)
      and the pipeline list is one shorter.

Then run S1 → S2 → S3 per the scene procedure. **At the moment these
tables exist, every prior METER number is retired** — different
machine.

## BOOT 3 — C2, the shadow discriminator (~15–18 min)

    git checkout claude/probe1-c2
    git log --oneline -3        # the rebased two-rooms commit
    <build>  → run with tee → boot3_c2.log

Same S1 → S2 → S3, same portal in S3 as boot 2.

**Visual gate (binding — a failed gate reverts C2 regardless of the
numbers):**
- [ ] Shadow edges at low sun across a lifted GoL zone
- [ ] Contact shadow under the pawn's feet
- [ ] Indoor spot shadows (atlas tile is now 1024×2048)
- [ ] Acne / peter-panning watch — a bias-retune *signal*; note it,
      don't fix it
- Screenshots if convenient. Verdict per item: PASS / FAIL + a line.

## BOOT 4 — C4, the Vulkan curiosity (optional, last, ~5 min)

    git checkout claude/probe1-c4-vulkan
    <build>  → run with tee → boot4_vulkan.log

Any outcome is a valid result — including failure to create the
device (2019 driver vs 2026 Dawn). If it reaches `renderer ready`:
- [ ] Record `[Renderer] Total pipelines:` — this is the FXC-bypass
      number; seconds here vs ~260 s is the whole point.
- [ ] One METER window + a glance that the world looks right.
Kill. No rulings ride on this boot; if it comes up clean, BACKEND_1
becomes its own campaign.

## SEND BACK

1. The four logs (or their METER excerpts).
2. Boot 1 verdict: PERMISSIVE / REJECTED (+ error verbatim if any).
3. The 920M lines: limits, features, Dawn revision.
4. Six scene tables — fill or just point me at the window `t=` values:

   | scene | boot | shadow gpu mean/max | shadow cpu mean/max | main gpu mean/max | main cpu mean/max | fps |
   |---|---|---|---|---|---|---|
   | S1 | 2 baseline | | | | | |
   | S1 | 3 C2       | | | | | |
   | S2 | 2 baseline | | | | | |
   | S2 | 3 C2       | | | | | |
   | S3 | 2 baseline | | | | | |
   | S3 | 3 C2       | | | | | |

5. Visual-gate verdicts (4 items).
6. Boot 4: the pipeline-total line + whether it lived.

From these: the fill/geometry verdict on the shadow pass, C2's
land-or-revert, the seven-home retirement (or the law's vindication),
and the GEOMETRY_2 ranking — all on numbers that finally describe a
chosen machine.
