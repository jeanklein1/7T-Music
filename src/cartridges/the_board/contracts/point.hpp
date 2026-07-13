#pragma once
#include <cstdint>

// ─── point.hpp (CONTRACT: the point — the parent of the player system) ─
// Born of PANEL-0 p1a (Jean's correction, ratified — the point model):
// history in audit/LADDER.md, design record in audit/PANEL0_RECON.md.
//
// THE POINT IS THE PARENT. The anchor IS a point; THE POINT OWNS THE
// BUBBLE. The camera is the point's permanent witness — it renders
// from wherever the point is — but does not own it. The point is
// HOSTED, like a spirit, wherever context demands (v3 §9 Act III /
// §11 made real):
//
//   PAWN host (the default) — the pawn hosts the point; the camera
//     couples to the pawn LIKE A KITE (the damped aim-point orbit,
//     world.wgsl update_camera — preserved pixel-identical). The
//     body carries its own terrain-snap (the walker ground resolve);
//     TERRAIN RULE = SNAP.
//   CAMERA host (free-fly) — the camera hosts the point: they
//     coincide; input moves it; body-specific contributions IDLE
//     (masked, not absent — the idleness principle at the structural
//     level); TERRAIN RULE = NONE (pure fly, clips freely — the
//     revision camera).
//
// THE CHAIN extends by the same grammar: when the pawn rides the
// ribbon (sky mode), the whole chain rides — possession, free-fly,
// riding are ONE mechanism: the point migrates between hosts;
// everything else couples to it or carries it. config.possessed_slot
// was always a host pointer restricted to agent slots; this contract
// names the general form.
//
// REALIZATION (p1a): POSITION lives in the HOST's GPU storage — the
// agent slot when the pawn hosts (agent_state[possessed_slot]), the
// camera state when the camera hosts. The host flag
// (config.point_host) routes reads and the input intent channel; a
// literal point storage of its own arrives at p1b, when live sensing
// must travel with the point. THE BUBBLE is declared here as part of
// the point's shape (the structure stays whole — first-principles
// assembly); its current REALIZATION — the 64-slot resolve loop and
// the aura grid — is physically centered on the default host (the
// pawn) because the pawn is today's only sensing host. In free-fly
// the sensors are DORMANT (kept, not active — Jean's standing
// ruling): the pawn idles, so nothing fires away from it and the
// slot-0 machinery is untouched by construction. Migrating the
// machinery to follow the point generally is p1b, pulled when a demo
// needs live sensing away from the pawn.
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ THE HOSTS ═════════════════════════════════════════════════════
// Mirrored to the GPU as config.point_host (u32) — the kernels route
// the intent channel and the camera's stance on this value.

enum class PointHost : uint32_t {
    PAWN   = 0,   // the default — the body hosts; the camera kites
    CAMERA = 1,   // free-fly — the witness hosts; input flies the point
};

// ═══ THE TERRAIN RULE ══════════════════════════════════════════════
// The constraint lives on the HOST'S CAST, not on the camera and not
// on the body. All three values have existing realizations: SNAP is
// the walker ground resolve (the body's own chain); SOFT_FLOOR is the
// flyer min-clearance clamp (update_camera carries it today); NONE
// skips both (clips freely).

enum class PointTerrainRule : uint32_t {
    NONE       = 0,
    SOFT_FLOOR = 1,
    SNAP       = 2,
};

// The host table — each host's cast, compile-time.
inline constexpr PointTerrainRule POINT_HOST_TERRAIN_RULE[2] = {
    PointTerrainRule::SNAP,   // PAWN — the body snaps; the kite follows
    PointTerrainRule::NONE,   // CAMERA — pure fly; every clamp skipped
};

// ═══ THE BUBBLE (declared; sensors dormant until p1b) ══════════════
// The bounded awareness region around the point (v3 §11): proximity,
// the portal trigger, the coming event source. Declared as part of
// the point's shape so the structure is whole; it carries no fields
// yet — the realization is the pawn-centered machinery named in the
// banner, and live per-host sensing arrives at p1b by pull.

struct PointBubble {};

// ═══ THE POINT ═════════════════════════════════════════════════════
// The instance (point_) lives at the composition root, beside the
// witness record (PlayerState) — spine-resident, like every organ.
// POSITION is realized host-side (see the banner); host-specific
// fields (velocity, body awareness tuning, the snapped placement)
// live with the hosts and IDLE when their host is not the one.

struct PointState {
    PointHost   host = PointHost::PAWN;   // the default host — the kite
    PointBubble bubble{};                 // declared whole; dormant
};

} // namespace the_board
} // namespace t7
