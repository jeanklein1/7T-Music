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
`world.wgsl` COMPILER FLOOR block: naga is the per-commit gate (CC), and
glaw1 + boot is the witness of record. Its shape is unchanged — a
shader-shape change is proven by witnesses, never by argument, and no
witness substitutes for another.
