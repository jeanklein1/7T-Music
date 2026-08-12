# PIVOT_0d + TIDY_0c — ROUND REPORT (CC → chat)

Execution record for the PIVOT_0d handoff, written for the next handoff's
author. Everything below is from the tree and from Jean's boot logs; where
a claim is a hypothesis it says so.

**HEAD open** `1b98479` **→ close** `885aecd`, pushed to `master`.
No branch — none was specified, and master is where campaign commits go.

| sha | unit |
|---|---|
| `3e8e99b` | PIVOT_0d-i: effect witnesses — selected backend + toggles Dawn enabled |
| `86aac1f` | TIDY_0c-i: web_dist DEPLOY text names the real project |
| `d81315a` | TIDY_0c-ii: meter dump skips the boot window, and says it skipped |
| `f5d4180` | PIVOT_0d-ii: toggles chain to instance descriptor; device-level chain retired |
| `885aecd` | WALLET_1revA: lighting block; entity F storage 7→4 |

7 files, +295 / −187.

---

## 1. The diagnosis was right, and the negative control proved it

**Boot 1**, plan `D3D12_Dxc`, after PIVOT_0d-i landed:

```
[Console] Compiler plan (request): DXC
[Console] Adapter selected: index=2 backend=D3D12
[Console] Toggles used (15): lazy_clear_resource_on_first_use,
  use_d3d12_residency_management, timestamp_quantization,
  d3d12_use_temp_buffer_in_depth_stencil_texture_and_buffer_copy_with_non_zero_buffer_offset,
  d3d12_use_temp_buffer_in_texture_to_texture_copy_between_different_dimensions,
  apply_clear_big_integer_color_value_with_draw,
  d3d12_use_root_signature_version_1_1, d3d12_use_64kb_alignment_msaa_texture,
  d3d12_create_not_zeroed_heap, polyfill_packed_4x8_dot_product,
  d3d12_polyfill_pack_unpack_4x8, d3d12_force_stencil_component_replicate_swizzle,
  d3d12_expand_shader_resource_state_transitions_to_copy_source,
  enable_integer_range_analysis_in_robustness, blob_cache_hash_validation
  [Pipeline] update_player_agent: 28010 ms
```

Fifteen toggles Dawn set of its own accord; `use_dxc` among **none** of
them. The gate's STOP did not fire, so PIVOT_0d-ii was released and landed.

Worth recording for the record: Dawn is not silent about toggles in
general — it reported fifteen. It had nothing to say about one chained at
the wrong stage. That asymmetry is why this cost a boot to find, and it is
the argument for keeping the effect witness permanently.

**28,010 ms** for `update_player_agent` on this run, against 20,227 (the
original) and 19,745 (the pre-fix boot). Three FXC samples spanning 8.3 s
of spread on the same kernel — consistent with L14's note that this
machine's timings vary by an order of magnitude, and a reminder that no
single-run FXC/DXC comparison would have been evidence either.

---

## 2. PIVOT_0d-i — the instrument (`3e8e99b`)

Three lines, no behavior change. All three confirmed working in Jean's
later boot.

```
[Console] Adapter selected: index=3 backend=Vulkan
[Console] Compiler plan (request): VULKAN
[Console] Toggles used (9): …
```

Census resolved five sites, no STOP:

- **C2** — `backend_name` exists and is in scope at the pick site, so the
  3-case local fallback was not needed. The scoring loop's `info` is
  loop-local, so the winner's info is re-fetched.
- **C5** — `dawn/native/DawnNative.h` was already included
  (`console.hpp:37`), so E4 was a **no-op**. The region is native-only:
  `#else` … `#endif // __EMSCRIPTEN__` encloses C1–C4 together.

Anchor discipline honored — the diff was grepped for `kCompilerPlan =` and
it does not appear.

---

## 3. PIVOT_0d-ii — the subject (`f5d4180`)

The chain moved to the instance descriptor. `deviceDesc.nextInChain`: 0
occurrences. `kDxcToggle` has exactly one reader.

**One deviation from the handoff's letter, and it is worth knowing about.**
The handoff offered either the C-typed or the C++-typed descriptor form.
I used the **C++ form**, because `wgpu::DawnTogglesDescriptor` with
`enabledToggleCount` / `enabledToggles` is *proven* against this Dawn —
PIVOT_0a compiled and ran those exact fields. The C form would have
re-asserted the struct shape and the `WGPUSType_` spelling from scratch.
Two things are new either way: `wgpu::InstanceDescriptor`, and the cast:

```cpp
instance_.emplace(reinterpret_cast<const WGPUInstanceDescriptor*>(&idesc));
```

E3's comment was replaced — its premise ("native has no
`RequestAdapterOptions`") died with the move. E4 restored the committed
default to `CompilerPlan::Vulkan`.

**STILL UNVERIFIED (P1):** that `dawn::native::Instance` takes
`const WGPUInstanceDescriptor*`. It comes from the handoff's reading of
the checkout, not from anything the repo can open. The build succeeded on
Jean's machine afterwards, which is evidence the arity is right — but the
build that succeeded ran the **Vulkan** arm, where the `if constexpr`
guard is false. The emplace call itself compiles either way, so the
constructor is confirmed; the toggle path through it is not.

---

## 4. Boot 3 — Vulkan, and what it settles

Jean's most recent boot used the committed default. **It is not Boot 2.**

```
[Console] Adapter selected: index=3 backend=Vulkan
[Console] Compiler plan (request): VULKAN
[Console] Toggles used (9): lazy_clear_resource_on_first_use,
  use_temporary_buffer_in_texture_to_texture_copy, vulkan_use_d32s8,
  use_placeholder_fragment_in_vertex_only_pipeline, timestamp_quantization,
  polyfill_packed_4x8_dot_product, enable_integer_range_analysis_in_robustness,
  blob_cache_hash_validation, decompose_uniform_buffers
```

The adapter tie-break worked as designed: discrete+Vulkan scores 3,
discrete+D3D12 scores 2, so index 3 wins.

**Pipeline compilation, Vulkan vs FXC:**

| | FXC | Vulkan |
|---|---|---|
| `update_player_agent` | 19,745 / 20,227 / 28,010 ms | **270 ms** |
| all 59 pipelines | never completed — AV'd | **6,606 ms** total |
| front-end (Tint) | 338–346 ms | 324 ms |

The program boots, runs, transitions moods, walks portals, and shuts down
clean (exit code 0).

**THE MACHINE IS GPU-BOUND, and this largely pre-answers the A/B.** From
three steady-state windows:

```
[METER] window 575f  fps 18.2   |  main_pass gpu 30.22  shadow_pass gpu 14.17
[METER] window 557f  fps 18.1   |  main_pass gpu 30.82  shadow_pass gpu 14.36
[METER] window 534f  fps 17.4   |  main_pass gpu 32.05  shadow_pass gpu 14.49
[METER] S present  mean 48.68 / 50.07 / 51.76      frame_total 53.55 / 56.07 / 58.02
[METER] U_SUM 0.26–0.56   R_SUM 1.60–3.50   residue 0.04–0.13
```

~44 ms of GPU pass time per frame against a 16.6 ms budget, while every
CPU row together is under 4 ms and the residue is ~0.1 ms — the
attribution is essentially complete. `present` blocks on the GPU and
dominates `frame_total`. Whatever backend overhead Vulkan-on-425.31
carries, it is not what is costing the frame.

The indoor window after the mood transition reads `shadow_pass gpu 30.84`
(vs ~14 outdoor) at fps 13.9 — worth a look on its own, unrelated to the
compiler.

Also visible: `[METER] R census_dumps mean 1.73–2.00 max 925–1113 ms`. The
dump's own frame costs up to a second. That is the documented cost of ~50
blocking console lines and is not new.

---

## 5. TIDY_0c-i (`86aac1f`)

`--project-name the-board` → `7t`, and the destination line now names
`everexpandingboard.com` canonical with `7t.pages.dev` as the raw
fallback.

Worth naming why this mattered more than a stale comment: it would not
have failed loudly. `wrangler pages project create the-board` **creates a
second, empty project**, and the deploy then succeeds into it — a green
run and a page nobody can find.

Left alone: `cmake --preset the-board-web`. Verified against
`CMakePresets.json` — that really is the preset's name, not a second
instance of the defect.

---

## 6. TIDY_0c-ii (`d81315a`) — the handoff's premise was wrong in both details

**The stated premise does not hold.** A `window_frames > 0` guard already
existed, and `gpu_sampled_frames > 0` guards the GPU columns. There is no
zero-frame window and no division by zero.

**The defect is real and differently shaped.** Traced:

1. `lastCensusDump_ = -999.0f` (spawn_engine.hpp) — so
   `seconds - lastCensusDump_ >= 30` is true on **frame 1**.
2. `phase_census_dumps` is `RPhase::CensusDumps` in RENDER_SPINE, and
   `render()` increments `window_frames` at its **head**.

So the guard sees **1**, and prints a one-frame table whose `wall_s` runs
from `FrameMeter`'s construction — across the entire boot. On this machine
that is an fps computed over ~20 s of pipeline compilation, with every
mean from one cold frame. Not an obviously broken table: a wrong number
wearing the right format, which is the kind that gets pasted into an A/B.

Fixed as skip-with-witness (P6), confirmed live in Boot 3:

```
[METER] first window SKIPPED — 1 frame(s), wall clock spans the boot; window starts now
```

then real windows at t=30.1, 60.2, 90.2, 120.3.

---

## 7. WALLET_1revA (`885aecd`) — SUBJECT ONLY, and the ledger is red

Landed: `struct Lighting` (848 B) at binding 320 as a uniform; 11 access
sites rewritten; three buffers → one `lightingBuffer_` (Uniform|CopyDst);
three uploads → one; layout and **both** bind groups 16 → 14.

**C3 held exactly** — 5 / 2 / 4 sites, three fragment entry points
(`entity_fs`, `patch_terrain_fs`, `ribbon_fs`), no fourth. Recomputed
independently of the ledger by walking the call graph (293 functions, 65
entry points).

**C3 rider, answered:** `calc_spot_light` and `sample_spot_shadow_pcf`
carry **no FXC clause at all** — PIVOT_0c's E8 left them untouched because
there was nothing there to update. Their prose is byte-identical after
this commit.

**C7 → E8's proceed-and-report:** three `upload_*` functions, but their
three callers sit adjacent inside **one** function, `upload_lights()` in
`direction/mood.hpp`. So the block is composed once and written whole; the
offset-write alternative at 0/48/320 was not needed.

**C6:** two bind groups on that layout, enumerated by census. The HOTFIX
class did not recur.

### THE OPEN ITEM — `0b-1` is red on master right now

```
[FAIL] 0b-1  banner says 99 declarations over 96 slots;
             census found 97 over 94
```

40 pass, that one fails. **97 over 94 is exactly what the handoff
predicted**, so the red confirms the prediction rather than contradicting
it. The expectation is a hardcoded literal:

```
tools/binding_ledger.py:928   ok = (len(decls) == 99 and len(slots) == 96 and …)
tools/binding_ledger.py:932   "banner says 99 declarations over 96 slots …"
```

Moving it is an **instrument** edit, which standing order 3 forbids from
sharing the subject commit. So two commits are still owed:

1. the `0b-1` literal, 99/96 → 97/94
2. `WALLET_1revA-close: ledger regen`

**The six predictions are therefore UNVERIFIED.** The artifact cannot
generate while a witness fails, so F storage 4 of 8, F uniform 4 of 12,
layout 14 entries, banner 97/94, Table C trio gone, and "tightest row is a
6-of-8 V row" are all unchecked. They get checked in the regen; any
deviation gets published and STOPs.

---

## 8. What is outstanding

**Jean-side**

- **Boot 2 never ran.** The DXC question is still open. The toggles line
  is the whole reading: `use_dxc` present with ~100 ms-class kernels means
  DXC lives; present with honest compile errors is a driver question;
  still absent means Dawn refused it at the adapter and this machine's
  native is Vulkan permanently.
- Visual gates still unwitnessed: **WALLET_0** (collision against columns
  and arches), **ORB_V** (orb-active mood — naga is blind to the
  attribute↔struct mapping), and now **WALLET_1revA** (point AND spot
  lights live, terrain in frame).

**CC-side**

- The two commits in §7.

**Notes for the A/B, if it still runs**

- Meter preset `the-board-full-release-meter` sets
  `{frame_meter=true, periodic_census=true}` — the pair a `static_assert`
  requires. No gotcha.
- Seed pin: `-DT7_WORLD_SEED=<n>`; the configure log prints
  `World seed: pinned to <n>`.
- Cadence: `CENSUS_DUMP_INTERVAL = 30.0f` s of `time_state_.seconds`.

```
cmake --preset the-board-full-release-meter -DT7_WORLD_SEED=42 && cmake --build --preset the-board-full-release-meter
out\build\the-board-full-release-meter\incubator_dual.exe
```

That preset has its own `binaryDir`, so it is a fresh configure and a full
rebuild. The DXC DLL copy follows it correctly (POST_BUILD into the same
directory).

---

## 9. Two things the round is owed an opinion on

**The defended-site index reads 110 → 74** (renderer.hpp 38 → 2) after
PIVOT_0c. Cause: 40 identical ROSTER-GATE tags each contained the word
`FXC`, so the detector counted one line of boilerplate as 36 extra
defended sites; the spelling sweep removed the word. All witnesses pass
and all six W4-2 controls resolve. My reading is that Table H got *more*
accurate — that tag is one lesson repeated, not forty. But 110 was a
published number in a closed campaign's artifact, and redefining it is not
mine to do.

**P1 kept paying this round.** Two of the three things the repo could not
open turned out to matter: `use_dxc`'s stage (settled by Jean's paste, not
by inference) and `dawn::native::Instance`'s arity (still an assumption).
The pattern worth carrying into the next handoff is that a Dawn-side fact
asserted from the repo is a hypothesis, and the cheapest way to settle one
is three lines pasted out of `C:/dev/dawn`.
