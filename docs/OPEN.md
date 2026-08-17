# OPEN — the register of open state
One line per item: what · origin (sha or doc) · what unblocks it.
This file is the ONLY home of open/parked state. When an item closes, its line dies.

- CHORD_5 terrain-fragment suspect: possible per-pixel storage read in the terrain
  fragment chain; fix designed (move entities[0].pos into frame_r as frame-cadence
  uniform); unblocked by a DPR staircase session. Origin: CHORD campaign; law home
  docs/CHORD.md.
- WIT_2b: dropped_submits witness never printed — snprintf truncation in
  cartridge.hpp near the dropped_submits format site; fix the format, re-arm the
  witness. Origin: CLOSE campaign diagnosis.
- 8-fit note: PARKED, awaiting Jean's design. The room-family compute pipelines carry
  9 storage-buffer layout entries visible to the compute stage, one over the WebGPU
  core default maxStorageBuffersPerShaderStage = 8. Origin: note.md +
  audit/past reports/CUT_1_LIMITS_FIT_NOTE.md (paths die this campaign; content in
  their last shas). NO HELD BRANCH EXISTS: `git ls-remote --heads origin` matched no
  branch for CUT_1 / 8-fit / C6 / LIMITS, so despite the note's "held branch" wording
  there is no branch to keep and none was deleted.
- PIPE_0: PARKED per Jean's 2026-08-07 directive. Origin sha of
  docs/HANDOFFS/WEB/PIPE_0_DECISION.md (dies this campaign).
- STREAM_0: PARKED per Jean's 2026-08-07 directive. Origin sha of
  docs/HANDOFFS/WEB/STREAM_0_DECISION.md (dies this campaign).

## Harvested at WINNOW-2 W2 (from files that died this campaign)

- HARVEST: OPTION-B WGSL PIN — world.wgsl @binding literals stay hand-mirrored
  (Option A, tools/wgsl_gate.py as the net); Option B, ONE authored source generating
  both sides, is still unbuilt. Verified still Option A at c43d44d.
  (from audit/past reports/LADDER.md P-4, eafd9ec)
- HARVEST: PREGEN-8 CONTINGENCY — rig-triggered storage weld (225→289 layers,
  TILE_GRID 17→19, MAX_ACTIVE_PATCHES) sleeps until the rig shows rim-pops under fast
  flight. (from audit/past reports/LADDER.md P-3, eafd9ec)
- HARVEST: TERRAIN-2 b3, the finite collapse — the radius-bounding choice (raise
  pregen / lower finite_radius_max / keep origin-pin degenerate) is undecided until b3
  lands; finite_radius_max is still live in gallery.hpp.
  (from audit/past reports/LADDER.md P-6, eafd9ec)
- HARVEST: the twins ruling — KEEP-THROUGH-ORGAN (the report's recommendation: the web
  is the product, the native twin is the bench, and the bench retires when the web gains
  WGSL hot reload AND the port is vendored) vs RETIRE-NOW. Awaiting Jean's word.
  (from audit/DOMESDAY_2_REPORT.md §11 Slot 1, 5ff5696)
- HARVEST: the one-generation law — text written and already enforced at F5-a/F5-d, but
  it has NO LAWS.md entry, and the numeral it proposed (L24) is already taken by "THE
  ROOM GROWS BY TEXTURE OR UNIFORM" at docs/LAWS.md:621. Needs a free numeral and the
  entry. (from audit/DOMESDAY_2_REPORT.md §11 Slot 2, 5ff5696)
- HARVEST: the terrain word on the pinned seed — three rounds outstanding, and the last
  open item of PROBATE_I. Wrong looks like "the whole terrain is the same hill repeated"
  or "everything outside the first patch went flat". (from GATEHOUSE_REPORT.md, a5e84bf)
- HARVEST: GF6 — whether to pay for renaming tools/gates/console_gate/ to match what it
  now is; ruled HOLD, the directory rename stays unpaid.
  (from docs/audit/PROBATE_CLOSE.md + GATEHOUSE_REPORT.md, 606924f)
- HARVEST: SPAWN_3 parked — F6 newborns seat inside render radius on move.
  (from audit/past reports/FIELD_BRIDGE.md, 4c1a804)
- HARVEST: the mood-seam crack — neighbours disagree at a shared edge; remediation
  directions recorded but NOT executed, parked per ruling.
  (from audit/past reports/INVESTIGATION_mood_seam.md, eafd9ec)
- HARVEST: TEX_C0 — parked behind the feature wallet and Jean's eye on ASTC banding
  since PROBATE_F. As originally sketched it is **impossible**, not merely expensive;
  priced and shelved with its bill attached, not refused and not scheduled. The texture
  constraint belongs beside docs/FXC_LAWS_RECORD.md.
  (from docs/audit/TEX_C0_PRICE.md, de1d5db)
- HARVEST: WIT_2b CONTRADICTION — the WIT_2b line above says the dropped_submits witness
  never prints, but docs/audit/PROBATE_CLOSE.md:67 (606924f, the newest close) records
  "WIT_2b's number — speaks on every device, and says zero." One of the two is stale;
  Jean strikes whichever. (from docs/audit/PROBATE_CLOSE.md, 606924f)
