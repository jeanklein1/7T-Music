#pragma once
#include <cstdint>

// ─── point.hpp (CONTRACT: the point — the parent of the player system) ─
// History: audit/LADDER.md
// Jean's correction, ratified — the point model.
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
// REALIZATION: POSITION lives in the HOST's GPU
// storage — the agent slot when the pawn hosts
// (agent_state[possessed_slot]), the camera state when the camera
// hosts — read through point_pos() (world.wgsl) on the GPU and the
// host-routed P5 harvest on the CPU, whose mirror lives HERE
// (PointState.x/z + the bubble sensor — POINT_1 moved it home from
// the pawn-era PlayerState); no separate point BUFFER was needed
// (the point readback, option A, Jean's stamp) — the GPU half of
// that ruling stands. The host flag (config.point_host) routes reads and the
// input intent channel. PRESENCE FOLLOWS THE POINT (the ratified
// rule): streaming, LOD/cull, the shadow box, the orb dome, the
// living population's existence (agent + floater spawn/evict/
// possess-reach/kite/corral), the photographer's record. EMANATION
// STAYS THE BODY'S: the walk, the aura dome, the forcefield, the
// AI-pursuit-target role — all idle in free-fly by construction.
// THE BUBBLE is real (below): its first field and first sensor
// are live (below).
// ─────────────────────────────────────────────────────────────────

namespace t7 {
namespace the_board {

// ═══ THE HOSTS ═════════════════════════════════════════════════════
// Mirrored to the GPU as config.point_host (u32) — the kernels route
// the intent channel and the camera's stance on this value.

enum class PointHost : uint32_t {
    PAWN   = 0,   // the default — the body hosts; the camera kites
    CAMERA = 1,   // free-fly — the witness hosts; input flies the point
    RIBBON = 2,   // sky-flight — the ribbon hosts; the possessed body
                  // rides the seat (the pawn kernel's mount gate), the
                  // camera kites onto it; input steers the head
                  // (RESIDUE_3: riding landed as a host — the contract
                  // above made real)
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
inline constexpr PointTerrainRule POINT_HOST_TERRAIN_RULE[3] = {
    PointTerrainRule::SNAP,       // PAWN — the body snaps; the kite follows
    PointTerrainRule::NONE,       // CAMERA — pure fly; every clamp skipped
    PointTerrainRule::SOFT_FLOOR, // RIBBON — the seat rides the head, whose
                                  // altitude is the critically damped pen
                                  // over terrain (bodies/ribbon.hpp head
                                  // control law) — a name for what already
                                  // happens, no new physics
};

// ═══ THE BUBBLE (first field + first sensor live) ══════════
// The bounded awareness region around the point (v3 §11): proximity,
// the portal trigger, the coming event source. Its FIRST FIELD is the
// radius; its FIRST SENSOR is the portal (pulled by Jean's
// altitude question): in camera-host an arch fires only when it sits
// within the bubble — the xz ellipse as ever, gated vertically by
// this radius (skim over → fire; fly high → no fire). In pawn-host
// the body's own crossing is the sensor, byte-identical to the
// pre-point era. The trigger rides the possessed slot's wire and the
// same P5 harvest — the wire is the realization, the bubble is the
// semantics.

inline constexpr float POINT_BUBBLE_RADIUS = 20.0f;   // world units; boot-pinned into config.point_bubble_radius (the WGSL side reads the config field)

struct PointBubble {
    float radius = POINT_BUBBLE_RADIUS;   // the awareness bound (the portal's vertical gate today)
};

// ═══ THE POINT ═════════════════════════════════════════════════════
// The instance (point_) lives at the composition root, beside the
// witness record (PlayerState) — spine-resident, like every organ.
// POSITION is realized host-side (see the banner); host-specific
// fields (velocity, body awareness tuning, the snapped placement)
// live with the hosts and IDLE when their host is not the one.

struct PointState {
    PointHost   host = PointHost::PAWN;   // the default host — the kite

    // ── The position mirror (POINT_1: re-homed from the pawn-era
    //    PlayerState — the point's CPU face, in the point's house) ──
    float   x = 0.0f;                     // THE POINT's world X (host-authored)
    float   z = 0.0f;                     // THE POINT's world Z (host-authored)
    // ── The bubble sensor, on the possessed slot's wire in both hosts ──
    int32_t portal_trigger = -1;

    PointBubble bubble{};                 // declared whole; first sensor above
};

} // namespace the_board
} // namespace t7
