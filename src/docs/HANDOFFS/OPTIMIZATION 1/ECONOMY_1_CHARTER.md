# ECONOMY_1 — THE CHARTER
### The resource-logistics campaign. Same picture, fewer instructions.

**Supersedes the name and mindset of "GEOMETRY_2".** This campaign does
not optimize for a machine. It optimizes the ratio between work done
and work needed — a dimensionless quantity that ships to the web
unchanged, where the adapter is chosen by the audience. Jean's machine
is the campaign's *verifier of identity*, not its definition of
success. Milliseconds are recorded as footnotes, not pursued as goals.

**THE ECONOMY LAW this campaign installs:**
- every field is sized to its hungriest reader;
- every pass is redrawn at the rate its inputs change;
- every emission that cannot reach the screen is deleted at the source;
- every constant that is secretly a ratio is written as the ratio.

**IDENTITY CLASSES** — every arm declares one, and the gate verifies
identity first, speed second:
- **Class I — bit-identical.** Provable by construction (e.g. removing
  geometry that is degenerate). The gate is a formality.
- **Class II — sub-resolvable.** The difference exists but falls below
  what any target can express (e.g. resolution vs Nyquist of the
  consumer). The argument is stated in the arm; the gate confirms.
- **Class III — gate-checked.** The difference could be visible;
  Jean's eyes rule, and rule absolutely.

---

## THE LEDGER
Entry format: `{ the work | the invariant ratio | the mechanism |
class | gate/audit }`. Ranked by waste ratio × frequency.

**E1 — THE CURTAIN SWITCH.** The LOD0 curtain band is 8,192 of 16,896
triangles per patch (48.5% of the index stream) and its base band is
~4,096 of ~10,750 unique VS verts (38%), and it emits ZERO area
whenever no zone lift is active — the dominant regime of a two-hour
recording — paid in the main pass, the shadow pass, and the snapshot
pass. Mechanism: a second, cap+skirt-only index buffer; a conservative
CPU flag (any lift-writer active → full IB) selects per frame. Class I
at rest by construction (degenerate quads draw nothing today; we stop
asking); Class III only at zone spawn/despawn instants. FXC exposure:
zero (no shader change). Cost: one shared IB (~100 KB).

**E2 — SHADOW AT THE HALF MESH.** The shadow pass runs the eye's full
decode (~10,750 verts, 16,896 tris per LOD0 patch) against a target
that resolves ≥2.67× coarser than even the half-density mesh — ≥7×
the vertex work its output can express, every frame. Mechanism: the
shadow pass draws the LOD0 band with the existing LOD1 index buffer
(the decode is patch-agnostic; the buffer already exists). Class II by
argument (0.293 wu texels vs 1.56 wu mesh over a band-limited height
field), Class III binding at the gate: low-sun silhouettes over a
lifted zone, pawn contact shadows. FXC exposure: zero, pending one
recon (does the legacy-band shadow decode apply cell lift — lifted
zones must still cast).

**E3 — LOD1 THROUGH THE CULL.** ~105 LOD1 patches draw unconditionally;
a forward camera sees roughly half. The frustum kernel already
computes the LOD1 verdict and discards it — a chain to a law adjudicated
false (Boot 1, PERMISSIVE). Mechanism: second visible-list + second
indirect-args slot; two plain DrawIndexedIndirect calls, now known
legal. Class I (culling is exact under the stated margin). FXC: one
kernel recompile, no collision-chain motion. Scope note E3b, same
machinery: the shadow pass may cull patches against the sun's ortho
box (SUN_HALF_EXTENT 300 < ring extent 350 — geometry exists that can
never cast into the box); ruled when E3's plumbing is in.

**E4 — THE INDOOR LIGHT ECONOMY.** Indoors, the caster list is encoded
N_lights times on the CPU (measured 14–18 ms mean as witness; the
invariant is N full-list walks where one culled walk suffices), the
terrain draws into every spot tile, and a FLAT floor — which cannot
cast — is in every caster list. Mechanisms: (a) receiver-only floor
excluded from spot tiles when the shell is FLAT (Class II/III, gate
per indoor mood); (b) per-light caster culling by range/cone before
encoding (Class I). The CPU encode shrinks with the lists.

**E5 — MARGIN AS RATIO.** The frustum cull tests every patch as a
150 wu box with a −50..200 Y range — 100% planar margin. The correct
margin is the maximum displacement the program already knows (max
zone lift + wave amplitude + aura lift). Mechanism: derive the margin
and Y bounds from those constants; Class I given a correct bound —
the bound derivation is the arm's recon. Small, honest, every frame.

**E6 — CONSTANTS THAT WERE RATIOS.** Shadow bias per texel (lands in
SWEEP_1 with C2 — the acne's root); this ledger entry exists to name
the class so future numbers are born as ratios.

**E7 — HEIGHTFIELD AT READER RATE (audit-gated by A1).** The patch
heightfield array is 151.5 MB (289 × 256² × RGBA16F); every traced
consumer samples at vertex rate (0.78 wu) or point rate — the bake
carries ~16× the texels its readers can express. Candidate: ~10 MB at
mesh-aligned resolution, silhouette-identical by construction. GATED:
A1 must first settle the rendered-normal provenance and the full
reader set. Prize: −140 MB resident, and consumers standing on the
same field the viewer sees.

**E8 — SHADOW AT CHANGE RATE (parked, re-ranked after E2).** The
shadow map's inputs change at event rate (snap crossings, zone ticks,
caster spawns, mover motion); it is rebuilt at frame rate. Mechanism:
static-caster depth cached, movers redrawn per frame; the invalidation
set is CENSUS_1 seed-10's enumeration, verbatim. Economy honesty: if
E2 takes the pass to ~2–3 ms, a per-frame 16 MB depth copy plus
invalidation machinery may cost more than it saves. Re-ranked on
post-E2 numbers.

**E9 — THE RING HORIZON (out of scope, named).** Stream bursts
(60–77 ms spikes) are event cost delivered lumpy; the one-ground ring
amortizes to entering-texel rate and unifies field homes. It remains
the horizon design, gated behind this campaign's re-baseline.

**E10 — PAINTING RESIDENCY (audit-gated; ranked above E7).** The live
painting system holds 264 MiB resident — Snapshot Staging 16 +
Authored Staging 16 + Exhibition 32 layers at 1024², 4 B/texel, plus
the offscreen pair — the largest single memory holder in the program
(census A2 ledger). The audit: how many layers per array ever hold a
distinct image at once across a session; then the levers (staging
lifecycle, exhibition sizing, format/mip choices). Every lever here
is **Class III ABSOLUTE**: the paintings are the artwork; no byte is
saved at their expense without Jean's eyes.

---

## THE AUDITS

**A1 — NORMAL PATH & HEIGHTFIELD READERS** (gates E7; read-only).
Enumerate every consumer of the patch heightfield array — VS bands,
shadow VS, sample_terrain_y_at, entity placement, anything else — with
each read's rate; settle, per band, where rendered normals come from
(card gradients vs heightfield texels). The one reader that could see
a resolution change decides E7's fate.

**A2 — PAINTING LOGISTICS & THE src/render SCOPE GAP** (rides the
parent census via its scope amendment). `src/render/painting_system.hpp`
declares a second painting manager at a 2048 canvas (16 MB/layer);
its liveness is unknown, and `src/render/**` has sat outside every
census scope. The parent census now covers it; the resident-memory
ledger of the three-array system + offscreen pair is reported there. The
resident ledger is delivered (264 MiB); the layers-in-use question is
E10's opening audit.

---

## SEQUENCE & GATES

1. SWEEP_1 lands (the owed commits: C2 + bias-as-ratio, the L2.4
   adjudication sweep, the C1b label, the Dawn revision line, this
   charter, RUN 4 released).
2. ECONOMY_1a: E1 + E2 as held branches cut from post-sweep master.
   Gate: one boot per arm — identity first, one METER window for the
   record. Merge on Jean's word.
3. ECONOMY_1b: E3 + E4 + E5, same shape.
4. A1 runs read-only in parallel; E7 ruled on its report.
5. E8 re-ranked on post-E2 numbers; E3b ruled on E3's plumbing.
6. Campaign ledger closes (landed / held / dead); re-baseline once,
   for the record; the aesthetics chapter opens.

**VERIFICATION LAW:** an arm's boot verifies its identity class before
anything else. Machine-clean discipline holds for recorded numbers;
where a true A/B is ever needed, arms alternate (A,B,A,B) so thermal
drift cancels. A gate that fails on identity ends the arm regardless
of any number.

---

## THE CLOSING LEDGER
*Closed 2026-07-29. Every entry ruled; nothing left open but the record.*

| Entry | Verdict | Reason |
|---|---|---|
| E1 curtain switch | LANDED (rev2, `3dde888`), then SUPERSEDED into the draw plan | global flag -> per-patch selection; snapshot keeps the flag |
| E2 shadow at half mesh | LANDED (`8804f0c`) | shadow GPU 6.37 -> 3.40 at rest; identity held |
| E3 LOD1 through the cull | LANDED (as the draw plan, `370ee94`) | first frustum test LOD1 ever had |
| E3b shadow vs sun box | DEAD | post-E2 the whole pass is ~3.4 ms; the machinery exceeds the prize |
| E4 indoor light economy | PARKED | CPU witness was Debug-inflated; re-rank after the Release record, owner: a future indoor pass |
| E5 margin as ratio | LANDED (inside the plan, `370ee94`) | the planar margin guarded XZ motion that does not exist (R1) |
| E6 ratios | LANDED (SWEEP_1) | bias constants born again as ratios |
| E7 heightfield at reader rate | MOVED | to the memory pass, still gated by audit A1 |
| E8 shadow at change rate | DEAD | 16 MB/frame copy vs a 3.4 ms pass; economy honesty |
| E9 the ring | HORIZON | unchanged; gated behind this record |
| E10 painting residency | MOVED | to the memory pass; Class III absolute stands |

**The aesthetics chapter's opening pieces**, held and untouched by this
campaign: **SHADOW_QUALITY_1** (`b3648f3` — settled bias pinned as ratios,
normal-offset sampling on the sun path, texel-aligned snap; gate binding on
Jean's eyes) and **the Vulkan fast-boot** (`b09885e` — the C4 backend
curiosity, the 44-second tuning loop; final verdicts stay on D3D12).

**Diagnostics remain as-is** by Jean's ruling. For whoever quiets them later,
the cadence's home is named here and NOT touched:
`CENSUS_DUMP_INTERVAL = 30.0f` (`machine/spawn_engine.hpp:65`) — the periodic
entity census dump, and the FRAME METER window rides the same cadence
(`cartridge.hpp:1245`). Its sibling is `AGENT_CENSUS_INTERVAL = 30.0f`
(`bodies/agents.hpp:127`).

### THE RECORD
*Awaiting the single Release boot on final master (`370ee94` or later). To be
filled from that log: the platform block verbatim (adapter, Dawn revision,
Build: Release); outdoor rest fps + main/shadow GPU mean; one traveling
window; one indoor window. Beside them, the campaign's opening numbers for
contrast — HD 5500, Debug, unchosen adapter: **19.2 fps, main 22.3, shadow
10.3**.*
