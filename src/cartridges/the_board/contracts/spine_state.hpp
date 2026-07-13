#pragma once
#include <cstdint>

// ─── spine_state.hpp (CONTRACT: the spine's organ types) ─────────
// Born of REBUILD-0 m1 (services graduation; recon §2.1, stamp D3):
// history in audit/LADDER.md, decision record in audit/REBUILD0_RECON.md.
//
// The in-class trio graduates to file scope so module deps structs
// (m3) can name the types without the complete Cartridge. The
// INSTANCES (time_state_, player_, transitionPhase_) stay at the
// composition root; the residency rulings (SEAM[spine:P8],
// SEAM[spine:transitions]) are unchanged — this is a type move, not
// an ownership move. MoodState graduates separately to
// direction/mood.hpp (struct with its semantic owner, instance at
// root — the WorldState pattern, R-a).
//
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ TIME STATE ══════════════════════════════════════════════════
// Per-frame clock state used everywhere. beats/seconds advance
// monotonically; dt is the most recent frame delta.
struct TimeState {
    float beats   = 0.0f;
    float seconds = 0.0f;
    float dt      = 0.016f;
    // Musical tempo follower: beats/sec, HELD-LAST through silence
    // and stopped transport; defaults to 100 BPM (the calibration
    // anchor for the authored idle motion).
    float beat_rate   = 100.0f / 60.0f;
    float prev_beats  = 0.0f;
};

// ═══ PLAYER STATE — THE WITNESS RECORD (REBUILD-0 m5; v3 §11) ═════
//
// THE WITNESS CONTRACT, declared and census-checked (the score
// census, Direction W):
//   · readback_x/z + readback_portal_trigger — SOLE AUTHOR is the
//     spine's P5 HARVEST; the spine's only other touches are the
//     TEARDOWN reset and the portal door's consume. No module writes
//     them, ever.
//   · possessed_slot — possession is RE-ANCHORING (v3 §9 Act III:
//     the anchor is a role; the camera is what we control). The
//     writes live behind the agents door (try_possess_nearest,
//     reseed_player_body), paired with the GPU selector.
//   · aura_presence — P8, the pawn is the semantic owner (writes in
//     pawn.inl only).
//   · THE CAMERA HAS NO CPU MIRROR — it lives GPU-resident, keyed on
//     config.possessed_slot; and there is NO readback_y (the witness
//     altitude is GPU-only). Neither is to be invented.
//   · the sky trio (mode / mode_prev / yaw_eased) LEFT this record
//     at m6 per Option A — it lives in RibbonState.sky, with its
//     single CPU owner (SEAM[ribbon:sky-mode], closed player-side).
//
// SEAM[spine:P8] PlayerState commented "Future (deferred)" fields
//   are explicit latent infrastructure: aura_presence is live here;
//   the other deferred fields await the unified entity layer.
//   Pattern P8 visible in source.
struct PlayerState {
    uint32_t possessed_slot = 0;   // slot in agent_state[] that the player inhabits

    // ── Camera + readback ──
    bool    fpv_mode = false;                // first-person view toggle
    float   readback_x = 0.0f;               // GPU readback of pawn world X
    float   readback_z = 0.0f;               // GPU readback of pawn world Z
    int32_t readback_portal_trigger = -1;    // set by readback callback when pawn hits portal

    // ── Aura presence (closes SEAM[spine:P8]) ──
    float aura_presence = 0.0f;                  // pawn aura ramp (was pawn_state_.aura_presence)

    // Future (deferred):
    //   uint32_t active_couplings;         // COUPLING_* bitmask owned by player
};

// ═══ TRANSITION PHASE ════════════════════════════════════════════
// The transition machine's phase enum. The MACHINE (transitionPhase_,
// pendingDestination_ and kin) stays spine-owned orchestration
// (SEAM[spine:transitions], K4); the enum TYPE lives here so its two
// module readers (mood's request door, agents' possession guard) name
// it unqualified instead of paying the Cartridge:: tax.
enum class TransitionPhase { IDLE, FADE_OUT, TEARDOWN, FADE_IN };

} // namespace the_board
} // namespace t7
