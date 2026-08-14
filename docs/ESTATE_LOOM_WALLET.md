# LOOM — the wallet

*ESTATE close-out section (LOOM campaign). Fold into `docs/ESTATE.md`.
The next campaign spends from this page. Where document and code
disagree, code is fact; the live numbers regenerate in
`audit/BINDING_LEDGER.md` Table B and the witnesses named below.*

## Per-stage slot balance (Table B, worst row per class)

| class | worst row | spent | free |
|---|---|---|---|
| uniform /12 | Update Player Agent · C | 11 | **1** |
| storage /8 | Patch Terrain (instanced) · V | 6 | **2** |
| sampled /16 | Patch Terrain (instanced) · F | 6 | **10** |
| samplers /16 | Update Player Agent · C | 3 | **13** |
| storage-tex /4 | Generate Patch Heights · C | 2 | **2** |

Bind groups: 4/4 at every pipeline (the recut's four strata — spent
by design, not spendable). Dynamic offsets: 1/8 uniform
(`shadow_slot`, FRAME_R), 0/4 storage.

## The debts and the deeds

- **AGENTS 11/12 — the debt marker.** The room family's compute rows
  stand one uniform seat from the wall. The load carries three
  demoted occupier windows: `occupier_cmg` (4,096 B) and
  `occupier_amg` (1,280 B) — demoted storage→uniform at **TETRIS
  WALLET_0** — and `field_head_poses` (6,400 B) at **C6**. Dated
  relief: those rows sit at storage 5/8, so re-promoting any one
  window buys a uniform seat for a storage seat, priced and
  reversible. Spend the last uniform seat only with the relief named
  in the same commit.
- **The `.a` reserve — 28.125 MiB, pre-paid.** The heightfield
  array's alpha channel (RGBA16Float, 225×256×256): nine consumers
  already wired through the read path, written as 0.0 everywhere.
  Deed attached at the `patch_heightfield_array_write` declaration
  (world.wgsl): *write nothing until a campaign names it (LOOM
  ruling).*
- **The vertex wallet — 2/8 buffers.** Worst pipeline (Orb Sky
  Layer) uses 2 vertex buffers of 8; six free, and
  bindGroups+vertexBuffers stands at 6/24.
  `visible_patch_indices`: **ELIGIBLE-DEFERRED** — one site,
  sequential in `instance_index`, stride 4 B; still the cheapest
  storage-seat relief on the render side, still unspent.
- **A2 demotions: CANCELLED-BY-MEASUREMENT.** The remaining
  ReadOnlyStorage→uniform demotion candidates are closed as a class;
  the one live face question (ro-alias demotion, F2) was part-forced
  by the validator at U3-C1 (`fc_vp`) and the remainder stays with
  LOOM_3's docket.
- **The feature treasury.** Adapter grants census at the A8 gate
  boot: 28 features offered, 9 toggles used, timestamp-query on,
  multi-draw-indirect absent. Governed by L20 (optional features)
  and L21 (a toggle is chained at the stage that consumes it); the
  boot log prints the toggles ACTUALLY armed — a grant unread is not
  a grant.
- **Immediates: WITNESS-PENDING.** Push-constant/immediate-data
  budgets have no census and no witness; do not spend what nothing
  measures. First campaign to want them builds the witness first.

## The laws this campaign added

- **L22 — the schema law**: `tools/binding_schema.py` is the one
  authority; mirrors are generated or checked; `--check` gates every
  recon and every landing (now S-1..S-7 + P-scope both arms + P-seq
  + S-6 commit integrity).
- **L23′ — the scope law** (superseding A7's L23): one
  synchronization scope, one writability per buffer — render pass
  whole, compute dispatch over its FULL bound groups, no visibility
  or static-use filter. Mixed-writability faces never share a layout
  and are never co-bound. Pessimism is the law: relaxation only on a
  witnessed Dawn behavior test, never a citation.
- Estate-tracked from the parallel arms, numbers held by their own
  campaigns (not in this repo's `LAWS.md`): the **instanced-state
  law**, the **attachment birth rule**, the **feature wings**.

*Balance sheet closed at LOOM_CLOSE. The witnesses keep it honest;
the wallet only says where the money is.*
