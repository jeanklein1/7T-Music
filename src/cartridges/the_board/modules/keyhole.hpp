#pragma once

// ─── keyhole.hpp (the forward-decl home) ──────────────────────────
//
// The two forward declarations every converted module header needs,
// provided ONCE. Module functions take the keyhole — Cartridge* c,
// the composition root (defined in cartridge.hpp) — and module/adapter
// signatures take wgpu::Queue by reference; both appear in the headers
// only by pointer/reference, so incomplete types suffice.
//
// LOCKSTEP INSURANCE: the Queue declaration mirrors webgpu_cpp.h's
// declaration form (`class Queue`, in namespace wgpu). If Dawn ever
// changes that form, replace this forward declaration with the
// include. (orbs.hpp forward-declares its two extra encoder handles —
// CommandEncoder / RenderPassEncoder — beside its banner under the
// same insurance.)

namespace wgpu { class Queue; }

namespace t7 {
namespace the_board {

class Cartridge;  // the keyhole — defined in cartridge.hpp

} // namespace the_board
} // namespace t7
