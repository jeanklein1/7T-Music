# OPT_1 — THE REMAINDER SWEEP

LEDGER_1's two largest levers are already pulled: L-1 lives as the segmented
indirect plan (full / cap-only / LOD1 slots at arg offsets 0/20/40, zone-rect
classification), L-2 as UMBRA's density pin (sun pass draws both bands through
the LOD1 IB; terrain casts in no spot tile). This campaign sweeps what remains:
the card writer's unconditional 819K invocations, the indoor eye-side gate,
the radius decision (which also closes limits exceedance 2), and — on Jean's
explicit word — R-5, the cell-granular LOD1 his R-2 ruling already declared
canonical. Every unit carries its own visual gate; the campaign opens and
closes with machine-clean METER_1 tables so each saving is measured, not
argued.

## GIT LAW

- Trunk-based, master only (after Jean merges C6). Separate commits per unit.
- `git fetch --unshallow` if shallow. LF-only, no BOM.
- CC never builds. Jean gates `glaw1` + boot + the stated visual gate per unit;
  red bisects by commit.

## REGISTER

**Every LEDGER_1 body reference is stale by ≥67 commits (R-7 proves drift);
amendment references are at `cab1a0f`, before CUT_1/PORT_1.** Bind by content
and symbol only. Read every span before editing; STOP on contradiction with
this handoff's premises, report, await ruling.

## PHASE 0 — BASELINE (Jean, no edits)

Before any commit: build with the frame meter on (`T7_INSTRUMENTS`), run
machine-clean, capture METER_1 tables for three scenes — outdoor rest, outdoor
with a live GoL zone, indoor default mood — plus the boot banner's adapter
line (this answers LEDGER_1's F4-1 gap: which adapter the milliseconds belong
to). Repeat the same capture after OPT_1 lands; the two tables are the
campaign's ledger. If first light has run, the Chrome rows sit beside them.

## O0 — RECENSUS PREFLIGHT (CC, read-only, one message, no commit)

Witness the premises at HEAD before cutting. Report:

a. **Segment plan state**: the three-slot indirect args (full / cap-only /
   LOD1), their draw sites, and the zone-rect classification path — confirm
   L-1 is live as the amendment describes.
b. **UMBRA state**: sun pass terrain through LOD1 IB for both bands; the
   `cast_terrain` arm false for spot tiles — confirm L-2 and the spot-terrain
   kill are live.
c. **Card writer**: both dispatches still unconditional per frame? Report the
   dispatch sites and any existing gate.
d. **Pulse producers post-CUT_1**: with the empty signal socket, is the pulse
   ring ever written? Report every producer of radial pulses and its driver.
   If all drivers are couplings that now miss, the rest law's musical
   conjuncts are structurally satisfied and O1's predicate reduces to "any
   GoL zone live".
e. **`terrain_time` writers**: still exactly one boot-pin to 0.0? (F-4b —
   expected unchanged; it stays a panel-eligible rest value, machinery kept.)
f. **Indoor eye side**: is the finite-mode ring-gate bypass still present, and
   does the frustum-cull compute still skip indoors? Report the sites, and
   where the indoor walls sit relative to the resident set (which resident
   patches are provably behind walls).
g. **`has_mode_bias`**: post-CUT_1, can any of its four mode params leave
   rest? If not, E3's per-pixel recompute is unreachable — report, no edit.
h. **Radius facts**: `PATCH_PREGEN_RADIUS`, `MAX_ACTIVE_PATCHES`, the two
   289-layer texture creations, and every site that derives from the 17/289
   pair (asserts, loops, upload sizes, WGSL twins).

## OPT_1a — L-5: THE CARD WRITER REST SKIP

*Spends F-4: 819,200 compute invocations per resting frame.*

- CPU predicate per O0-d: skip both card dispatches when no GoL zone is live
  (and, if O0-d finds a live pulse path, when the pulse ring is empty and
  `terrain_time <= 0` — the full three-conjunct law, evaluated CPU-side,
  conservative: any doubt ⇒ write).
- On the transition into rest: one final write (or explicit clear) so
  consumers never read stale non-zero texels; report the chosen mechanism.
- The kernels themselves are untouched — the gate is at dispatch.

Commit: `OPT_1a: live-card writer skipped at rest (one clearing write on
entry)`
Visual gate (Jean): walk a zone boundary; toggle a zone live/dead at the card
edge; terrain must not pop or retain ghost lift.

## OPT_1b — THE RADIUS: 8 → 7

*Closes limits exceedance 2 (289 → 225 layers) and shrinks the pregen ring.*
Correction on record: the previously stated 16² target was wrong — centered
windows have odd sides; 15² = 225 is the true post-edit layer count, safely
under the 256 default.

- Per O0-h's derivation census: `PATCH_PREGEN_RADIUS` 8 → 7; every derived
  site follows the constant (asserts re-pinned, both texture arrays now 225
  layers, loops and uploads by symbol — nothing hardcoded survives).
- The draw radii are untouched: the veil ring (325) needs patches to
  nearest-edge 350 = exactly radius 7; the draw set is unchanged by
  construction. What thins is pregen hysteresis — one ring less of stream-in
  lookahead.

Commit: `OPT_1b: pregen radius 7; patch arrays 225 layers (fits default
maxTextureArrayLayers)`
Visual gate (Jean): sprint the pawn in a straight line across open terrain,
then diagonally; the leading edge must never show pop-in or a bald patch.
Memory note: both patch arrays shrink ~22%.

## OPT_1c — INDOOR EYE GATE (conditional; design bound by O0-f)

*Spends F-3's surviving half: the eye pass submits every resident patch at
full LOD0 indoors, uncled.*

Only if O0-f shows resident patches provably behind walls: gate indoor
submission to the room's floor set plus one patch of margin (or restore the
ring test with `finite_radius`-derived bounds — bind to what O0-f found).
The bypass comment ("walls define boundary, not fog") is the intent to
preserve: no visible floor may vanish. If O0-f shows every resident patch IS
visible floor, report and skip this unit — the finding dissolves.

Commit: `OPT_1c: indoor eye draw gated to the room's floor set`
Visual gate (Jean): in the largest indoor mood, stand at each wall, look down
and outward, then across the diagonals; the floor must be continuous
everywhere.

## OPT_1d — R-5: CELL-GRANULAR LOD1 (stamp-gated; do not start without
Jean's explicit word in the forwarding message)

*Executes the R-2 ruling: the slab is the truth. Kills the LOD1 taper and the
straddle division — in the eye pass and, post-UMBRA, in what the sun casts —
at −27.8% LOD1 indices (6,912 → 4,992).*

- Builder: replace the LOD1 legacy-grid emission with the two-curtain cell
  band from R-5's arithmetic (256 cap quads + 512 curtain quads on +x/+z
  planes + 64 skirt segments), each curtain spanning ground to
  `max(L_here, L_neighbour)`.
- Decode: extend `ug_decode`'s LOD1 branch for cell-owned caps and the two
  curtain planes; the card is world-addressed so the neighbour fetch crosses
  patch boundaries natively; ClampToEdge makes the rim collapse benign (R-6,
  settled).
- Cost consciously accepted: +1 `sample_live_card_gol` per curtain vertex —
  the Phase 0 / closing METER pair measures it, per R-6's own demand.
- The new branching is witnessed, not banner-costed: `glaw1` + boot is the
  FXC witness; Chrome is the second. (The L2 banner gap is flagged below.)
- Re-pin every index-count constant and assert that carried 6,912/2,304;
  report the full arithmetic in the commit body before editing.

Commit: `OPT_1d: LOD1 draws slabs (R-2); two-curtain cell band, −27.8%
indices`
Visual gate (Jean): the R-8 fixture — the three-frame zone series. Frame 1
stops tapering; the straddle division disappears, not moves. Then the same
zone at low sun angle: the cast silhouette matches the slab.

## CLOSED / DEFERRED (recorded so nothing leaks)

L-1, L-2: landed pre-campaign (O0-a/b witnesses). L-3: payoff collapsed with
UMBRA (~350K uncled shadow triangles); measurement may resurrect it later.
L-6: closed by design (samplers differ on purpose). L-7/MDI: moot — not core
WebGPU, and three indirects already coexist in one pass. E3: likely
unreachable post-CUT_1 (O0-g decides). F4 probes beyond the adapter line:
mechanisms no measurement has asked for. DPR and resolution caps: browser
knobs, SHIP-era.

## CAMPAIGN REPORT

Hashes; O0 answers in full; per-unit predicted vs. structural saving; the
Phase-0/closing METER delta table slots (Jean fills); every STOP.

## FLAG FOR JEAN (charter, not CC)

R-6 found LAWS.md L2 pointing at a world.wgsl FXC banner that does not exist
in the tree — a law with no operational home. The port era reframes it:
per-witness shader constraints (native FXC via glaw1, each browser via its
gate). Say the word and I draft the L2 amendment or the reconstructed banner
next session.
