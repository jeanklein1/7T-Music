# SALON_1 — P7 REPORT: `derive_indoor_lights`' parameter

Report-first stage. **No edit is in this commit.** Read at `cd78621`.

Amendment IV §4 sets a two-way fork and asks which the code takes:

> - **Hangs lights relative to where the wall stops** → `wall_height` is right;
>   one-word rename.
> - **Positions them against the actual ceiling** → on VAULT that is `crown_h`,
>   47 to 92 against a nominal 25. The **call site** is wrong and the name is
>   the symptom, not the defect.

**Answer: the first branch. One-word rename.** But it does not fall out of the
uses — four of them take it, one looks like it takes the other, and the thing
that settles it is sitting eleven lines further down the same function.

---

## §1 — EVERY USE OF THE PARAMETER

`derive_indoor_lights`, declared `mood.hpp:197`, defined `mood.hpp:346`. Nine
occurrences of the name; five are uses that matter.

| path:line | use | what it positions |
|---|---|---|
| `mood.hpp:415` | `py = ceiling_height - 0.5f;` | `LightAnchor::CEILING` — the light's Y |
| `mood.hpp:420` | `py = ceiling_height * hfrac;` | `WALL_NORTH` sconce Y |
| `mood.hpp:425` | `py = ceiling_height * hfrac;` | `WALL_SOUTH` sconce Y |
| `mood.hpp:430` | `py = ceiling_height * hfrac;` | `WALL_EAST` sconce Y |
| `mood.hpp:435` | `py = ceiling_height * hfrac;` | `WALL_WEST` sconce Y |
| `mood.hpp:507` | `L.range = ... ? ceiling_height + 30.0f : room_range;` | CEILING light's reach |
| `mood.hpp:530` | `L.range = ceiling_height + 80.0f;` | the vault uplight's reach |

### The four `WALL_*` uses are unambiguous

`hfrac` is clamped to `[0.4, 0.85]` for walls — `mood.hpp:406`, commented
*"walls: sconce height"*. A sconce is mounted **on the vertical wall**, so
scaling by the height at which that wall stops is exactly right.
**`wall_height` is the correct name for these four**, and it is more correct
than `ceiling_height` was even before P5a: on VAULT they were already being
scaled by the spring, not by any ceiling.

### `LightAnchor::CEILING` is the one that looks like the other branch

`py = ceiling_height - 0.5f` reads as "hang it just under the ceiling." On
VAULT that puts it at **24.5**, while the actual ceiling — the crown — is
**47.25 to 92.25**. Taken alone, this is Amendment IV's second branch: a light
called a ceiling light, hanging at roughly half the room's height.

---

## §2 — WHAT SETTLES IT: THE VAULT UPLIGHT

`mood.hpp:513-534`, in the same function, gated on the same information:

```cpp
    if (ceiling_type == CeilingType::VAULT && count < MAX_SPOT_LIGHTS) {
        auto& L = c->cpuSpotLights_.lights[count];
        ...
        L.position[1] = 2.0f;           // near floor level
        L.direction[1] = 1.0f;           // straight up
        ...
        L.range = ceiling_height + 80.0f; // reach the crown with headroom
        std::cout << "[Lighting] Added vault uplight (slot " << (count - 1) << ")\n";
```

The function **already knows** the vault's ceiling is out of the spot array's
normal reach, and answers it with a dedicated floor-mounted uplight aimed
straight up, added only when `ceiling_type == VAULT`. That light exists
precisely because the ceiling-anchored spots do not reach the vault.

So `py = ceiling_height - 0.5f` at spring height is **deliberate, not a
mis-wired call site.** Under a vault, a downlight at the springing line is
also the ordinary architectural answer — the vault above it is lit by the
uplight, which is what the uplight is for.

**The fork therefore takes branch one for all five position uses.** No call
site is wrong. The rename is one word:

```
    float ceiling_height  →  float wall_height
```

at `mood.hpp:197` (declaration) and `mood.hpp:346` (definition), plus the seven
uses. It is a parameter, so the change is local to one function and its
prototype; the argument at `mood.hpp:589` already passes `m.wall_height`.

---

## §3 — ONE OBSERVATION, NOT A DEFECT

`mood.hpp:530` — `L.range = ceiling_height + 80.0f; // reach the crown with
headroom`.

The comment names the crown. The expression does not compute it: it adds a
constant to a number that is *not* the crown and relies on the constant being
large enough. Checked at every legal radius, it is:

| radius | crown | `wall_height + 80` | headroom |
|---:|---:|---:|---:|
| 1 | 47.50 | 105.0 | 57.5 |
| 2 | 62.50 | 105.0 | 42.5 |
| 3 | 77.50 | 105.0 | 27.5 |
| 4 | 92.50 | 105.0 | 12.5 |

It holds — with 12.5 wu to spare at the largest room, which is the tightest
case and still comfortable. **No edit is warranted.**

Recorded because the exact quantity the comment names is available as a
function call: `vault_crown_height(m, bmin, bmax)` (`mood.hpp:633`), which both
this function's caller and the camera clamp already use. Should the vault dials
ever move — `VAULT_RISE_FRACTION` is the one that would do it — `+ 80.0f` is
where this would silently fall short, and the crown is one call away. That is a
note for the control panel, not work for this campaign.

After the rename the line reads `wall_height + 80.0f // reach the crown`, which
is at least honest about starting from the wall rather than from the ceiling.

---

## §4 — WHY THIS IS THE SAME SHAPE AS P5

P5 found `ceiling_height` naming two different things and resolved it by
reading, not by renaming — `GPUDesignConfig::ceiling_height` kept its name
because it really is the ceiling, while `MoodProfile::ceiling_height` became
`wall_height` because it never was.

This is the third instance of the same spelling, and the same method settles
it: the four sconce uses always wanted the wall, the ceiling anchor wants the
wall *by design* with the uplight as its documented compensation, and the one
line that genuinely means the crown says so in a comment and clears it by 12.5
wu at the worst radius.

**One name, three referents, resolved three times by reading what the code does
with it.** Nothing here needed a mechanism.

---

## LEDGER ROW

| Stage | State | Commit | Note |
|---|---|---|---|
| P7 — `derive_indoor_lights` parameter | **reported, edit not authorized** | this commit | **Branch one: one-word rename**, `mood.hpp:197` + `:346` + 7 uses. No call site is wrong — the VAULT uplight (`:513-534`) is the deliberate compensation for ceiling spots sitting at spring height. One observation: `:530`'s `+ 80.0f` reaches the crown by headroom rather than by computing it; holds at every radius with 12.5 wu spare at the worst. |
