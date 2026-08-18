# THE FXC LAWS — RETIRED RECORD

> **RETIRED 2026-08-12 by PIVOT_0.** These were the laws of a compiler
> this program no longer targets. They are kept verbatim because they
> explain the SHAPE of `world.wgsl` — much of the shader still looks the
> way it does because of them — and because a retired law that leaves no
> trace gets rediscovered as a mystery.
>
> **Do not honor these as live constraints.** Nothing in the tree should
> be shaped to satisfy FXC any longer, and code already shaped that way
> is not thereby wrong — it is merely no longer *required* to be so.
>
> **Any successor cliff on the new floor must be re-witnessed.** These
> laws were paid for by measurement, not argument, and their successors
> must be too. The `[Pipeline]` per-kernel timer is the instrument.

> DRIFT NOTE (RECENSION_1, 2026-08-18): the archive directories named in the
> appendices below — `docs/past docs/`, `docs/HANDOFFS/past campaigns/`,
> `audit/past reports/`, `docs/audit/` — were themselves retired to git after
> these rulings (L30). Their mentions live in history only; the rulings stand
> as minutes (L28).

## The decision

The audience floor is raised: **WebGPU core through modern compilers** —
Tint→DXC (SM6.0+) on native and Windows Chrome, Tint→MSL, Tint→SPIR-V,
naga (Firefox). **FXC is unsupported.**

`world.wgsl` remains ONE file for both twins. Only the native compiler
toggle changes: `kCompilerPlan` in `src/console/console.hpp`, default
`D3D12_Dxc`, with `Vulkan` one line away and `D3D12_Fxc` retained for
archaeology only.

## What forced it

The boot witness of 2026-08-12. TETRIS WALLET_0 demoted the two occupier
windows (`occupier_cmg`, `occupier_amg`) from the storage address space to
the uniform one — legal, byte-identical, and exactly what the binding
ledger's Table C flagged as a free A2 candidate. Under FXC the resulting
cbuffer arrays stalled `update_player_agent` at **20,227 ms**, and
`D3DCompiler_47` then **access-violated** on the next room kernel.

Jean ruled the floor up rather than the shader down. WALLET_0 stands.

The number is worth keeping beside the laws below, because it is the same
species as the one that produced law 2: FXC's cost is not linear in the
shader's complexity, and the cliff is not visible from the source.

---

## THE RETIRED BLOCK, verbatim

Cut from the `world.wgsl` file banner, where it stood as "L2's operational
home". Reproduced here exactly as it read at commit `da8104f`, minus only
the WebGPU core-defaults budget line, which was never an FXC law and stays
live in the banner.

```
// ─── FXC BANNER (L2's operational home; L2 owns the why) ────
// Witness protocol: shader-shape changes are proven by witnesses, not argument.
//   FXC witness  — Jean's native gate (glaw1 + boot; Dawn / D3D12 / FXC).
//   Web witnesses — each browser at its own gate (Chrome first).
//   A Chromium/Tint pass is not an FXC pass; no witness substitutes for another.
// Windows D3D12 compiles through FXC. Honored BY STRUCTURE:
//  1. Hot-loop instance structs stay lean and byte-pinned
//     (exemplar: GPUSpotLightArray static_assert, state.hpp).
//  2. The collision/ground chain admits NO new runtime
//     branching. Loops bound by uniforms
//     (min(count, MAX_...)); dispatch by uniform function
//     choice, never by branch.
//  3. NO texture-array stamps in or near the collision chain.
// A violation does not fail here. It fails on Windows, at
// pipeline creation, in someone else's hands. The kernel-split
// banner (above the agent kernels) prices the inlining cliff.
```

## L2 as it stood in `docs/LAWS.md`

```
## L2 — THE FXC LAW

The Windows D3D12 backend compiles through FXC, which has hard limits the
Vulkan/Metal backends do not. The shader honors them **by structure**, so
nothing in it looks like a workaround and everything is one. The law states
the principle; the operational home of the specifics is the world.wgsl FXC
banner — the banner owns the constraints, this law owns why they bind. The
banner also states the **witness protocol**: a shader-shape change is proven by
witnesses, never by argument — the native FXC gate first, then each browser at
its own gate, and no witness substitutes for another.

1. Instance structs in hot loops stay lean and byte-pinned — the pattern's
   live exemplar is the `GPUSpotLightArray` pin (`static_assert` in
   `state.hpp`: `16 + MAX_SPOT_LIGHTS * 128`).
2. The collision/ground chain admits **no new runtime branching**. The live
   exemplar: the pyramid loop bounds itself by a uniform —
   `min(pyramid_instances.count, MAX_PYRAMID_INSTANCES)` in world.wgsl —
   and dispatch is by uniform function choice, never by branch.
3. Texture-array stamps in the collision chain **hang FXC**. Do not add one.
4. Storage buffers per stage = 8. Uniform buffers per stage = 12. The
   budget is WebGPU core defaults (L14) — no adapter grant is requested
   above them.

A violation does not fail on the developer's machine. It fails on Windows, at
pipeline creation, in someone else's hands.
```

**Item 4 was never an FXC law.** Storage 8 / uniform 12 per stage are
WebGPU **core defaults** (L14) and bind on every backend and every
compiler. It survives L2's retirement and lives on: in the `world.wgsl`
banner's budget line, in L14, and as the ledger's `gate` witness.

## What each retired law was paid for

Kept because the shapes are still in the tree and a reader will meet them.

| law | the shape it produced, still present |
|---|---|
| 1 — lean byte-pinned instance structs | `GPUSpotLightArray`'s `static_assert(16 + MAX_SPOT_LIGHTS * 128)` in `state.hpp`, and the byte-pinning discipline across the whole `GPU*` mirror family |
| 2 — no new runtime branching in the collision/ground chain | the uniform-bounded loops (`min(count, MAX_…)`), the one-entry-point-per-policy structure in the ground chain, and the agent-kernel SPLIT — `update_player_agent` / `update_other_agents` exist as two kernels because FXC inlined both switch arms for all 32 threads and produced a 48 s compile |
| 3 — no texture-array stamps near the collision chain | the absence of one; the ground chain reads heightfields through the patch path rather than stamping |

The agent-kernel split is the one to think hardest about before undoing:
it was measured at 48 s of FXC compile, and whether DXC prices it the same
way is **unmeasured**. Re-witness before merging those kernels back.

## The witness protocol, superseded

The retired protocol named the FXC gate first. The live protocol is in the
`world.wgsl` COMPILER FLOOR block: `tools/wgsl_gate.py` gates the module
per commit (naga as its pinned half, behind the immediate shim; it
witnesses the MODULE only), and the web build + boot is the witness of
record. Its shape is unchanged — a shader-shape change is proven by
witnesses, never by argument, and no witness substitutes for another.

## PROBATE appendix — the probate of 2026-08-16
Rulings of record for every site the FXC trigger matched at PROBATE_B.
RETIRE = the mention is history, moved here, tree comment dropped or
shrunk. REWRITE = the shape stays for a living reason now stated in the
tree; the FXC pricing lives here. KEEP = the tree text already speaks in
history tense or is the pivot statement itself.

| symbol / site | ruling | old text (verbatim) |
|---|---|---|
| `(file banner)` — COMPILER FLOOR block, `world.wgsl` (two mentions) | KEEP | `// Tint→MSL, Tint→SPIR-V, naga (Firefox). FXC is unsupported;` · `//   every backend and survived FXC's retirement unchanged.` — the pivot statement itself, and the L14 clause that explicitly outlived it. |
| `row_agent_flee` — the §3 profile-table block, `world.wgsl` | KEEP | `// retired FXC laws forbade (docs/FXC_LAWS_RECORD.md). The banner forbids` `// nothing here now; the structure is kept because it is good structure.` — recon per the ruling: "the banner forbids" resolves to a prohibition the tree ALREADY discharges in its own next clause. No live prohibition remains, so no rewrite is owed. |
| `contrib_pawn_aura_at_self` — the Ground Query API banner that follows it, `world.wgsl` | RETIRE | `// One entry point per policy. Each consumer declares its policy at` `// its own call site (a compile-time constant choice of function) so` `// FXC sees uniform branching and can dead-code-eliminate anything` `// outside that policy's contributor set. Runtime policy dispatch is` `// deliberately avoided — see contracts/ground_architecture.hpp (POLICIES[]).` — dead-code elimination of a uniform branch is every backend's behaviour, so the tree now says so compiler-neutrally. |
| `query_ground_walker` | REWRITE | `// but avoids a second full pass over the GoL zone loop (which` `// compounds significantly under FXC loop unrolling). See` `// contrib_gol_suppression_at for the standalone subtractive form.` — the living reason survives the compiler: one traversal of the zone set instead of two is a saving everywhere. The unrolling premium was FXC's and stays here. |
| `sample_shadow_pcf` | RETIRE | `// 4x4 PCF — sixteen taps, fully unrolled, hardware bilinear per tap` `// through the comparison sampler. FXC-clean by construction: no array,` `// no loop for it to unroll, no dynamic index.` — the construction description is kept as plain description; the "FXC-clean" blessing is dropped. |
| `agent_post_step` — the AGENT POST-STEP HELPER banner, `world.wgsl` (the liberation site) | REWRITE | `// from velocity. Pulled out of each behavior body so FXC compiles` `// the common epilogue once per kernel rather than ten times.` — the factoring is good structure on its own. This is where the probate's product sentence lands: no living law bars branching in this chain. |
| `agent_post_step` — the CONTACT_2 C2b body comment ("the old FXC sanctum") | KEEP | `// chain (the old FXC sanctum — docs/FXC_LAWS_RECORD.md; the chain` `// is still kept branchless, now by taste rather than by law).` — already a record pointer in history tense, and it already names the demotion from law to taste. |
| the kernel-split banner near `behavior_levy_flight` / `update_player_agent` | REWRITE-FINISH | `// The agent kernel is split in two for compile-time reasons.` … `// in a single switch statement. FXC inlined both branch bodies for` `// every one of 32 dispatched threads, producing a pipeline compile` `// that landed at 48s. Adding more algorithmic behaviors would` `// compound the cost.` `//` `// PIVOT_0: that 48 s was FXC's price, and FXC is retired` `// (docs/FXC_LAWS_RECORD.md). WHETHER DXC PRICES THE UNIFIED KERNEL` `// THE SAME WAY IS UNMEASURED. The split stands until someone measures` `// it — the [Pipeline] per-kernel timer is the instrument, and merging` `// these two back is a change that must carry its own witness, not an` `// inference from the floor having moved.` — **THE 48 s PRICE LIVES HERE NOW, WHOLLY.** The tree's living defence is Table E: all twelve ordered pairs among `update_player_agent` / `update_other_agents` / `update_sphere` / `update_cube` are BARRED on `agent_state`, `floating_entities` and `field_forces`, and fusing deletes the implicit inter-dispatch barrier their ordering depends on, with no device-wide barrier in WGSL to put back. That bar is mechanical and compiler-independent; the split would stand if compile time were free. |
| `orb_sample_palette` | RETIRE | `// Layout is unrolled because WGSL uniform blocks can't hold arrays of` `// structs cheaply and explicit ifs let FXC see a uniform-bounded chain.` — the shape is described without the dead compiler's blessing. |
| the orb tier accessors block (between `orb_sample_palette` and the coherent-noise seed) | RETIRE | `// gets its own 4-way dispatch. All branches are on uniform values` `// (tier_count, per-invocation tier_idx) so FXC handles them without` `// divergence penalty. The pattern is mechanical; it's kept in this` `// compact form so the obvious repetition doesn't dominate the file.` — the handoff's §4.3 keyed this row to `orb_hsv_to_rgb`; recon found the FXC token on the tier-accessor banner instead (`orb_hsv_to_rgb` carries none). Same treatment, correct anchor. |
| `orb_dynamics` — the RULE DISPATCH body comment | RETIRE | `// Uniform branch — every invocation in the workgroup takes` `// the same path, no FXC divergence penalty.` — a uniform branch does not diverge; that is the WGSL fact, and it needed no compiler's name. |
| `config_.mosaic_enable` — the MOSAIC_2 dial block, `state.hpp` | REWRITE | `// runtime gate is discharged — FXC compiled the walk. The` — the living statement is that the walk compiles on the supported floor; which compiler first proved it is history. §4.4 expected NO token in `state.hpp`; the grep found this one, at the mosaic dial rather than at SceneConstants. |
| `GPUSceneConstants` — the "Do not upgrade this block" defence, `state.hpp` | KEEP (no probate owed) | Recon per §4.4: the block defends on the LIVING storage budget — "the render VERTEX stage sits at the per-stage STORAGE cap and uniform has its own budget" (L14) — and carries **no** `FXC` token. Expectation confirmed; nothing to rule. |
| `renderer.hpp` — the PIVOT_0c ROSTER-GATE boilerplate | KEEP (no probate owed) | Recon per §4.4: `grep -c "\bFXC\b" renderer.hpp` == 0. PIVOT_0c's despelling left no remainder; the Table H rebase from 110 that it caused is the same fact seen from the index side. |
| `docs/LAWS.md` — the struck `L2 — THE FXC LAW` block (five mentions) and the PIVOT_0a narration under the toggle-stage law (one) | KEEP | Law text, and O-6 forbids adapting it. All six already speak in the required register: the L2 heading is struck-through and dated, its body says "Do not honor L2's constraints as live", and the toggle-stage law's mention is past-tense narration of the defect that paid for it (`the boot log said "Compiler plan: DXC" and FXC compiled anyway`). This is the pivot statement in law form. |
| `docs/7t_program_theory_v3.md` — the CAST demotion passage, and the "leaving Dawn" passage | REWRITE (×2) | `The FXC realization laws (proxied rect solids, bounded` `loops, no new branches) are CAST-SCOPED and stand untouched.` · `recast of L5 plus retirement of the cast-scoped FXC laws — everything` — **the live-document defect this probate exists to catch.** Both sentences asserted, in the present tense, that the FXC laws were still standing; PIVOT_0 struck them on 2026-08-12. A reader could grep this file and believe a struck law was live. The cast-scoping claim itself — that those laws never reached above the realization line — is true and survives; only the status claim was wrong, and the second passage now prices "leaving Dawn" at a recast of L5 alone. |
| `docs/reference/DAWN_REFERENCE.md` | RIDER (§4.5) | One line inserted under the title, verbatim: `Documents the ARCHIVED native build (native-sunset, PIVOT_0). Kept as record; nothing here binds the web twin.` Its three FXC mentions describe the archived native build's compiler and are history by filing (P4 — the stamp is now inside the file, not merely in its folder). No other edit. |
| the `docs/` and `audit/` archive — past campaign reports, closed handoffs, superseded ledgers | KEEP, en masse | Several hundred further mentions across `docs/past docs/`, `docs/HANDOFFS/past campaigns/` and `audit/past reports/`. Every one is a dated report or a closed handoff narrating what the compiler cost at the time — history-tense by nature, and the class-KEEP case exactly. Probating them would rewrite the record this appendix exists to keep, which is the opposite of "history keeps one home". `audit/BINDING_LEDGER.md`'s thirty are the instrument's own artifact and are rebased by regeneration at PROBATE_R, not by hand. |

## §index-history — Table H, every rebase
| when | movement | cause |
|---|---|---|
| 2026-08-12 | 110 → 74 | PIVOT_0c despelled the ROSTER-GATE boilerplate: one lesson counted forty times |
| between PIVOT_0c and 3e18c39 | 74 → 68 | six sites left the index across two campaigns without the preamble following — the hand number this section exists to retire |
| PROBATE (2026-08-16) | 68 → 68 | orb_sample_palette out (FXC was its sole trigger); update_sphere in (named by the rewritten kernel-split banner citing the Table E bar) |

## PROBATE_E appendix — the gate ruling of 2026-08-16 (E3)

| symbol / site | ruling | old text (verbatim) |
|---|---|---|
| `(file banner)` — COMPILER FLOOR block, `world.wgsl`, the supported-compilers line | AMENDED | `// Tint→MSL, Tint→SPIR-V, naga (Firefox). FXC is unsupported;` — naga 30 rejects `requires immediate_address_space;` outright, so no naga-backed browser has been witnessed compiling this module since DOMESDAY_2 F3-a introduced the directive. Firefox is marked **PENDING** rather than supported: the floor of record is the Tint trio until one Firefox boot testifies. Claiming a compiler the program has never seen accept it is the same class of error as a count with two homes. |
| `(file banner)` — COMPILER FLOOR block, `world.wgsl`, the per-commit gate clause | AMENDED | `// its retired laws live in docs/FXC_LAWS_RECORD.md. naga is` `// the per-commit gate (CC); the web build + boot is the witness of` `// record; the [Pipeline] timer prices compile time per kernel.` — the clause named a gate that had been failing open for a whole campaign: naga refused the module at `3e18c39` and at every commit back to F3-a, and nothing said so. The gate is now `tools/wgsl_gate.py`, whose transform is pinned in the tree, and the banner names the blind spot the transform creates — the immediate address space itself, witnessed by boots alone. |

## PROBATE_E appendix — struck-law citations in active voice (E4)

Subject set: active-voice citations of a STRUCK law number, outside
`docs/LAWS.md` and this record. The struck set is exactly **L2** (the
FXC law, struck 2026-08-12 by PIVOT_0) — recon confirmed one struck
heading in `LAWS.md` and no other.

| symbol / site | ruling | old text (verbatim) |
|---|---|---|
| `slope_passable` — `world.wgsl` | REWRITE | `// what the height test had — the L2 posture for the collision/ground` `// chain (no new runtime branching).` — the site PROBATE flagged as F12 and left standing because it carried no `FXC` token. It cited a law struck two campaigns earlier, in the present tense, with no strike-through and no record pointer: a reader could grep `L2`, find it struck, and be left holding an active prohibition with no living source. The branch count is still described; only its false authority is gone. |
| the possessed-pawn occupier wire (BATCH G1) — `world.wgsl`, inside `update_player_agent`'s candidate block | REWRITE | `    // A VALUE change on the existing candidate — no new branches (L2).` — same class, found by generalizing F12 from one site to the predicate. The parenthetical made a struck law the reason for a shape; the shape is now described as what it is, with the law's status stated once. |
| `update_other_agents` — the FIELD_2/FIELD_B presence-law banner, `world.wgsl` | KEEP — **flagged ambiguous** | `// (Durable home per the L2-banner precedent: the law lives where` `// it binds. FIELD_BRIDGE.md indexes it.)` — this cites L2's **banner placement** as a precedent for *where documentation lives*, not its constraints as a rule for what the code may do. A method precedent survives the striking of the law that demonstrated it. Per §E4.2 this is flagged rather than adapted: a reader who greps `L2` still lands on a struck heading, and whether a struck law may go on being cited as a siting precedent is Jean's ruling, not an executor's. |
| `SEAM[gallery:L2]` — `bodies/gallery.hpp` | KEEP — not a citation | `// SEAM[gallery:L2] this is a clean instance of pattern P3 (player` — a SEAM tag whose label happens to read `L2`. Recorded so the next `\bL2\b` sweep does not re-litigate a false positive. |
| `DOMESDAY_1_HANDOFF.md` | KEEP — not a citation | `read \`LANTERN_CENSUS.md\` §L2 in full` — a section reference in another document. Same false-positive class. |
| `docs/7t_program_theory_v3.md` (two hits) | KEEP | `L0 SUBSTRATE · L1 PRIMITIVES · L2 ENTITIES · L3 COUPLINGS` is the theory stack's own layer name, a different L2 entirely; the second hit is PROBATE_X's own struck-text (`struck as L2 by PIVOT_0, 2026-08-12`). |
| `SALON_1`, `SALON_1_E_C_REPORT` (attic) | KEEP as report — **flagged** | Nine hits treating `L2.4` as a live ceiling, at a value (`maxStorageBuffersPerShaderStage = 10`) that is wrong twice over: the clause survived L2's striking and lives in **L14**, and the core default is **8**, not 10. They are dated audit reports and history by nature, which is why they are KEEP — but they sit in `docs/audit/`, unstamped, greppable and believable (P4's own test, and F13's class one folder over). A stamp on each would settle it; §E4 authorizes no such edit, so it is flagged. |

## PROBATE_SEAL appendix — the two rulings (S5)

| symbol / site | ruling | old text (verbatim) |
|---|---|---|
| `update_other_agents` — the FIELD_2/FIELD_B presence-law banner, `world.wgsl` (EF5) | REWRITE | `// (Durable home per the L2-banner precedent: the law lives where` `// it binds. FIELD_BRIDGE.md indexes it.)` — PROBATE_E flagged this as genuinely ambiguous and left it standing: it cited L2 as a precedent for WHERE DOCUMENTATION LIVES, not as a constraint on the code, and a method precedent plausibly outlives the law that demonstrated it. **The ruling: precedent may be cited, authority may not be borrowed from the dead.** The siting rule is now the banner's own living sentence — a law lives where its consumers read, and for the presence law that is this block, which every body-class presence pair sits beneath. L2 remains named, as past-tense provenance with the record pointer, because the practice genuinely came from there. |
| `SALON_1`, `SALON_1_E_C_REPORT` (attic, EF6) | STAMPED, text untouched | Nine hits treating `L2.4` as a live ceiling at `maxStorageBuffersPerShaderStage = 10` — wrong twice over: the clause survived L2's striking into L14, and the core default is 8. **The ruling: minutes are never rewritten.** Correcting the numbers inside would destroy the only thing a dated report is for; leaving it unmarked would let a reader grep it and believe it. Both files now carry the drift ON THE STAMP, under their titles, and the rule is L28. |

## GATEHOUSE appendix — the Firefox price (G5)

| symbol / site | ruling | old text (verbatim) |
|---|---|---|
| `(file banner)` — COMPILER FLOOR block, `world.wgsl`, the Firefox clause | AMENDED | `// Tint→MSL, Tint→SPIR-V. Firefox: PENDING — naga 30 lacks` `// immediate_address_space; the floor of record is the Tint trio` `// until a Firefox boot witnesses otherwise (Jean's witness,` `// queued).` — PROBATE_E3 marked Firefox PENDING and left the cost of un-marking it unstated, which is how a PENDING quietly becomes a permanent. The clause now carries the price: a generated no-immediates module (the `wgsl_gate` transform, run as a generator rather than as a gate) **plus**, on that path only, the per-patch staging machinery PROBATE_I retired — because a module without `var<immediate>` needs its params back in a buffer. Two boot paths, doubled witnesses, for one browser. **HELD** — not refused, not scheduled. The card (PROBATE_SEAL3) is the boundary meanwhile, and a Firefox visitor meets a designed sentence rather than a defect. |
