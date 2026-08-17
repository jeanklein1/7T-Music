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
