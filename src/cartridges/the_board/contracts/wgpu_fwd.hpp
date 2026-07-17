#pragma once

// ─── wgpu_fwd.hpp (the wgpu handle forwards — one lockstep-insurance home) ─
//
// The five Dawn handle types the_board names in pre-cohort signatures,
// forward-declared ONCE in webgpu_cpp.h's exact form (`class X`, in
// namespace wgpu). If Dawn ever changes that form, replace these with
// the include. Complete types arrive with state.hpp later in the cohort.

namespace wgpu {
class Queue;
class Device;
class CommandEncoder;
class ComputePassEncoder;
class RenderPassEncoder;
}
