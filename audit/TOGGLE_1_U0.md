# TOGGLE_1 — U0, TRACE THE BREAK (read-only)

**No break is identifiable in this tree.** All four U0 questions answer
clean: the descriptor is passed, the lifetimes hold, there is one native
instance creation, and the readout reads device toggles — which
inheritance should reach. So U0 ends in the branch the handoff wrote for
it: **STOP and request the paste.**

But U0 also found something that changes what the paste is *for*. The
gate condition is not a formality — it is decisive, and it is
demonstrably unmet by any binary built before `30c9a7c`.

No `src/` file was touched. HEAD `00f5c8f`.

## THE GATE CONDITION IS LOAD-BEARING — proof from the diff

`PIVOT_0d-ii` (`f5d4180`) did two things in one commit. It moved the
chain from the device descriptor to the instance descriptor — and it
switched the compiler plan to Vulkan. Its own output, verbatim:

```
console.hpp:113   inline constexpr CompilerPlan kCompilerPlan = CompilerPlan::Vulkan;
...
console.hpp:698   static const char* const kDxcToggle[] = { "use_dxc" };
console.hpp:699   wgpu::DawnTogglesDescriptor toggles{};
console.hpp:700   wgpu::InstanceDescriptor idesc{};
console.hpp:701   if constexpr (kCompilerPlan == CompilerPlan::D3D12_Dxc) {
console.hpp:702       toggles.enabledToggleCount = 1;
console.hpp:703       toggles.enabledToggles = kDxcToggle;
console.hpp:704       idesc.nextInChain = &toggles;
console.hpp:705   }
```

`idesc.nextInChain = &toggles;` sits **inside** the `if constexpr`, and
the constant it tests was set to `Vulkan` by the same commit. So from
`f5d4180` until TOGGLE_0's `30c9a7c`, on the Vulkan plan:

- the `if constexpr` is false at compile time,
- `idesc.nextInChain` stays **`nullptr`**,
- the instance descriptor chains **nothing at all**.

**A binary built in that window chains nothing, so `Toggles used (9)`
with `disable_symbol_renaming` absent is fully explained by the control
not being in the binary.** It is not evidence about whether the chain
propagates. The control test has not run yet unless the boot's binary
contains `30c9a7c`.

This is exactly what the gate condition asks, and it is why the handoff
was right to ask it. **If the `(9)`-boot predates `30c9a7c`: rebuild at
HEAD, boot once, read the line — do not run U1.** Nothing below is
actionable until that reading exists on a confirmed-fresh binary.

## U0.1 — is the descriptor passed to the creation the program uses?

**Yes, and no second path wins.**

```
console.hpp   instance_.emplace(reinterpret_cast<const WGPUInstanceDescriptor*>(&idesc));
```

`instance_` on native is `std::optional<dawn::native::Instance>`
(`console.hpp:1845`). That `emplace` is the only native instance
construction in the program.

The other two sites are not competitors:

| site | why it is not a second path |
|---|---|
| `instance_ = wgpu::CreateInstance(nullptr);` | inside `#ifdef __EMSCRIPTEN__` — the web twin, which links no Dawn native. Its `instance_` is a different member (`wgpu::Instance`, `console.hpp:1847`). Not compiled on native. |
| `wgpu::Instance(instance_->Get()).CreateSurface(&surfaceDesc)` | a handle wrapper around the **same** instance, constructed from `instance_->Get()`. Refcount-neutral, creates nothing. |

**No break here.**

## U0.2 — lifetime

**Sound. Nothing dies before the call.**

| object | storage | outlives `emplace`? |
|---|---|---|
| `kControlToggle[]` / `kDxcAndControl[]` | `static const char* const` — static storage duration | yes, trivially |
| `toggles` (`wgpu::DawnTogglesDescriptor`) | local in `initWebGPU()` | yes — no scope ends between construction and `emplace` |
| `idesc` (`wgpu::InstanceDescriptor`) | local in `initWebGPU()` | yes — same scope |

The three are constructed, chained, and consumed within one straight-line
block of the same function body. **No break here.**

## U0.3 — how many instance creations?

**Exactly one on native, and hot reload does not add another.**

The handoff flags the console header's `Hot Reload Enabled` line. Traced:
`incubator_dual.cpp` polls a file watcher every ~30 frames and calls
`app->render.reload_shaders()`, which reaches `Renderer::reload()`:

```
bool reload() {
    if (!loadShader()) return false;
    if (!createComputePipelines()) return false;
    if (!createRenderPipelines()) return false;
    std::cout << "[Hot Reload] Shader reloaded successfully\n";
    return true;
}
```

Shader module and pipelines only. No instance, no adapter, no device.
**No break here.**

## U0.4 — where does the readout read from?

**The device.** `console.hpp`:

```
auto used = dawn::native::GetTogglesUsed(device_.Get());
std::cout << "[Console] Toggles used (" << used.size() << "):";
```

This is the one place the handoff suspected could hide a success, so it
is worth stating precisely how the visibility works out:

- The readout reports **device** toggles only.
- `deviceDesc` chains **nothing** — no `nextInChain`, no toggles
  descriptor. Verified: its only members set are `label`, the two
  callbacks, `requiredLimits`, and `requiredFeatures`/`requiredFeatureCount`.
- The adapter comes from `instance_->EnumerateAdapters()` — enumerated
  unfiltered, with no options object, so it carries no toggles of its own.
- So the device's toggle set can only contain a chained toggle by
  inheritance, instance → adapter → device.

`disable_symbol_renaming` is a **Device**-stage toggle, so the readout is
the right instrument for it *provided* inheritance carries it two hops.
An instance-stage success would be visible here only through that same
inheritance. **This does not hide a success — but the claim that it
cannot is the one link in the chain this tree cannot prove.**

## WHY U0 STOPS HERE

Every link I can check from our own source is intact. The one link I
cannot check is Dawn's: whether a `DawnTogglesDescriptor` chained on a
`WGPUInstanceDescriptor` passed to `dawn::native::Instance` actually
reaches `GetTogglesUsed` on a device two inheritance hops down, in the
Dawn revision this program links.

That is not readable here. `CMakeLists.txt` pins `DAWN_DIR` to
`C:/dev/dawn`, a Windows prebuilt; there is no Dawn source, no headers,
and no compiler in this container.

### The paste request — with one correction to the handoff

The handoff says `DAWN_REFERENCE.md` "exists, empty, waiting for exactly
this job." **It is neither empty nor at the expected path:**

```
docs/past docs/DAWN_REFERENCE.md   7,215 bytes, 163 lines
# Dawn — build reference
Dawn is a prebuilt external dependency, not a submodule. This document
records HOW the Dawn tree at C:\dev\dawn\out was produced.
```

It is a live toolchain-pin document. **I did not touch it.** If the paste
should live beside it, it wants a new file — `docs/DAWN_TOGGLES.md` —
rather than an overwrite of a document that records the build.

What would settle it, in order of usefulness:

1. `dawn/include/dawn/native/DawnNative.h` — the `Instance` class
   declaration and its constructor signature, plus `GetTogglesUsed`.
2. The toggle-stage table from `dawn/src/dawn/native/Toggles.cpp` for
   `use_dxc` and `disable_symbol_renaming` — confirming the exact name
   strings and their `ToggleStage`.
3. The `InheritFrom` calls in `Instance.cpp` and `Adapter.cpp` that the
   chain-site banner cites.

## A CHEAPER DISCRIMINATOR, PROPOSED NOT PERFORMED

If the fresh-binary reading does come back `(9)`, there is a one-line
experiment that separates "the chain is never read" from "the chain is
read and the toggle was rejected", without any Dawn source:

**Chain a deliberately invalid toggle name alongside the control** — e.g.
`"t7_not_a_toggle"`. Dawn warns on unrecognised toggle names at the stage
that parses them.

| observation | meaning |
|---|---|
| a warning about the bogus name appears | the chain **is** read; the real toggle's absence has another cause |
| no warning, and still `(9)` | the chain is never read — branch (b) confirmed at the parse, not inferred from a silence |

It is one extra array element at the same site, it is removed by
TOGGLE_0 U2 along with the control, and it converts an absence into a
positive signal. Offered for Jean's ruling; not performed, because U1 is
gated and this is not the identified break.

## STATE

| | |
|---|---|
| edits made | **none** — read-only throughout |
| U1 | **not run** — gate unconfirmed, and no break identified to fix |
| TOGGLE_0 control | **still armed** at `30c9a7c`, as TOGGLE_1 requires |
| next action | Jean's one line: was the `(9)`-boot's binary built from a tree containing `30c9a7c`? If no or uncertain — rebuild at HEAD, boot once, read the line. |
