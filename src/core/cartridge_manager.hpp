#pragma once

// ─── cartridge_manager.hpp ───────────────────────────────────────
//
// Transition orchestration: owns all cartridges and handles transitions
// between them. Isolated from main.cpp for clean separation of concerns.
//
// Transition flow:
//   1. Cartridge detects door commitment in its update()
//   2. Cartridge sets pending_transition_ flag
//   3. main.cpp checks pending transition AFTER update completes
//   4. main.cpp calls transition_from() to execute transition
//   5. CartridgeManager switches active cartridge
//   6. CartridgeManager spawns pawn at appropriate position
//
// Spawn positioning: when transitioning TO a cartridge with a pawn, it
// spawns at a position that makes sense relative to the door used
// (currently hardcoded per source→destination pair).
//
// Re-entrancy protection: transitions are locked during execution to
// prevent multiple transitions from triggering simultaneously.
//
// Previous-room tracking: every transition saves the source cartridge
// ID, so BACKSPACE (or other commands) can return to the previous room —
// useful for rooms without exit doors.

#include "core/cartridge_ids.hpp"
#include "render/render_cartridge.hpp"

#include "cartridges/gallery_raymarch/cartridge.hpp"
#include "cartridges/gallery/cartridge.hpp"
#include "cartridges/n_dimensional_2/cartridge.hpp"
#include "cartridges/the_board/cartridge.hpp"
#include "cartridges/world_compute/cartridge.hpp"
#include "cartridges/terrain_pawn/cartridge.hpp"
#include "cartridges/playground_hybrid/cartridge.hpp"
#include "cartridges/species_studio/cartridge.hpp"
#include "cartridges/pawn_rasterize/cartridge.hpp"

#include <memory>
#include <iostream>

namespace t7 {

class CartridgeManager {
public:
    bool init(wgpu::Device device, wgpu::TextureFormat colorFormat, wgpu::TextureFormat depthFormat) {
        device_ = device;
        colorFormat_ = colorFormat;
        depthFormat_ = depthFormat;

        // ── Create All Cartridges ────────────────────────────────

        // ── Gallery Raymarch (Entry Point) ───────────────────────
        galleryRaymarch_ = std::make_unique<gallery_raymarch::Cartridge>();
        galleryRaymarch_->initialize(device);
        if (!galleryRaymarch_->init_renderer(colorFormat, depthFormat)) {
            std::cerr << "[CartridgeManager] Failed: gallery_raymarch\n";
            return false;
        }

        // ── Gallery (Annex) ──────────────────────────────────────
        gallery_ = std::make_unique<gallery::Cartridge>();
        gallery_->initialize(device);
        if (!gallery_->init_renderer(colorFormat, depthFormat)) {
            std::cerr << "[CartridgeManager] Failed: gallery\n";
            return false;
        }

        // ── N-Dimensional 2 ──────────────────────────────────────
        nDimensional2_ = std::make_unique<n_dimensional_2::Cartridge>();
        nDimensional2_->initialize(device);
        if (!nDimensional2_->init_renderer(colorFormat, depthFormat)) {
            std::cerr << "[CartridgeManager] Failed: n_dimensional_2\n";
            return false;
        }

                // ── N-Dimensional 4 ──────────────────────────────
        theBoard_ = std::make_unique<the_board::Cartridge>();
        theBoard_->initialize(device);
        if (!theBoard_->init_renderer(colorFormat, depthFormat)) {
            std::cerr << "[CartridgeManager] Failed: the_board\n";
            return false;
        }

        // ── World Compute ────────────────────────────────────────
        worldCompute_ = std::make_unique<world_compute::Cartridge>();
        worldCompute_->initialize(device);
        if (!worldCompute_->init_renderer(colorFormat, depthFormat)) {
            std::cerr << "[CartridgeManager] Failed: world_compute\n";
            return false;
        }

        // ── Terrain Pawn ─────────────────────────────────────────
        terrainPawn_ = std::make_unique<terrain_pawn::Cartridge>();
        terrainPawn_->initialize(device);
        if (!terrainPawn_->init_renderer(colorFormat, depthFormat)) {
            std::cerr << "[CartridgeManager] Failed: terrain_pawn\n";
            return false;
        }

        // ── Pawn Rasterize ───────────────────────────────────────
        pawnRasterize_ = std::make_unique<pawn_rasterize::Cartridge>();
        pawnRasterize_->initialize(device);
        if (!pawnRasterize_->init_renderer(colorFormat, depthFormat)) {
            std::cerr << "[CartridgeManager] Failed: pawn_rasterize\n";
            return false;
        }

        // ── Playground Hybrid ────────────────────────────────────
        playgroundHybrid_ = std::make_unique<playground_hybrid::Cartridge>();
        playgroundHybrid_->initialize(device);
        if (!playgroundHybrid_->init_renderer(colorFormat, depthFormat)) {
            std::cerr << "[CartridgeManager] Failed: playground_hybrid\n";
            return false;
        }

        // ── Species Studio ───────────────────────────────────────
        speciesStudio_ = std::make_unique<species_studio::Cartridge>();
        speciesStudio_->initialize(device);
        if (!speciesStudio_->init_renderer(colorFormat, depthFormat)) {
            std::cerr << "[CartridgeManager] Failed: species_studio\n";
            return false;
        }

        // ── Start In Gallery Raymarch ────────────────────────────

        active_ = galleryRaymarch_.get();
        activeId_ = CartridgeId::GALLERY_RAYMARCH;
        
        // Initialize previousId_ to starting cartridge so BACKSPACE works
        // even if user presses F-keys before entering through a door.
        // This way: F1 → World_Compute → BACKSPACE returns to Gallery_Raymarch
        previousId_ = CartridgeId::GALLERY_RAYMARCH;

        std::cout << "[CartridgeManager] Ready. Entry point: Gallery Raymarch\n";
        return true;
    }

    // ── Access ───────────────────────────────────────────────────

    RenderCartridge* active() { return active_; }
    uint32_t active_id() const { return activeId_; }

    // ── Transition — Called by cartridge callbacks ───────────────
    //
    // source: where we're coming FROM (for spawn position decisions)
    // target: where we're going TO
    //

    void transition_from(uint32_t source, uint32_t target, bool isDoorTransition = false) {

        // ── RE-ENTRANCY Protection ───────────────────────────────
        // Prevents callbacks from triggering additional transitions while
        // a transition is already in progress.
        
        if (inTransition_) {
            std::cout << "[CartridgeManager] BLOCKED re-entrant transition: "
                      << id_name(source) << " → " << id_name(target) << "\n";
            return;
        }


        if (target == activeId_ || target == CartridgeId::NONE) return;

        // Lock transitions
        inTransition_ = true;

        wgpu::Queue queue = device_.GetQueue();

        std::cout << "[CartridgeManager] " << id_name(source) 
                  << " → " << id_name(target);
        if (isDoorTransition) std::cout << " (door)";
        std::cout << "\n";

        // ── Save Previous Room — For Backspace navigation (door transitions only) ─
        // Before switching to the target, save where we're coming FROM,
        // BUT ONLY for door transitions. Keyboard shortcuts (F1-F4) don't
        // overwrite the previous room, allowing BACKSPACE to return to the
        // room you entered via a door.
        if (isDoorTransition) {
            previousId_ = activeId_;
        }

        switch (target) {

        case CartridgeId::GALLERY_RAYMARCH:
            active_ = galleryRaymarch_.get();
            activeId_ = target;
            // Spawn based on where we came from

            if (source == CartridgeId::GALLERY) {
                // Coming from gallery annex: spawn INSIDE safe zone (Z < 12), facing back
                // Safe zone for front wall door requires Z < 12, so use Z = 10
                galleryRaymarch_->spawn_pawn(0.0f, 10.0f, 3.14159f, queue);
            } else {
                galleryRaymarch_->reset_pawn(queue);
            }
            break;

        case CartridgeId::GALLERY:
            active_ = gallery_.get();
            activeId_ = target;
            // Spawn based on where we came from
            if (source == CartridgeId::GALLERY_RAYMARCH) {
                // Coming from hub: spawn INSIDE safe zone (Z > -12), facing forward
                // Safe zone for back wall door requires Z > -12, so use Z = -10
                gallery_->spawn_pawn(0.0f, -10.0f, 0.0f, queue);
            } else {
                gallery_->reset_pawn(queue);
            }
            break;

        case CartridgeId::N_DIMENSIONAL_2:
            active_ = nDimensional2_.get();
            activeId_ = target;
             break;
        
        
        case CartridgeId::THE_BOARD:
            active_ = theBoard_.get();
            activeId_ = target;
             break;



        case CartridgeId::WORLD_COMPUTE:
            active_ = worldCompute_.get();
            activeId_ = target;
            break;

        case CartridgeId::TERRAIN_PAWN:
            active_ = terrainPawn_.get();
            activeId_ = target;
            break;

        case CartridgeId::PAWN_RASTERIZE:
            active_ = pawnRasterize_.get();
            activeId_ = target;
            break;

        case CartridgeId::PLAYGROUND_HYBRID:
            active_ = playgroundHybrid_.get();
            activeId_ = target;
            break;

        case CartridgeId::SPECIES_STUDIO:
            active_ = speciesStudio_.get();
            activeId_ = target;
            break;

        default:
            std::cerr << "[CartridgeManager] Unknown target: " << target << "\n";
            break;
        }

        // Unlock transitions
        inTransition_ = false;
    }

    // ── Direct Transition — For keyboard shortcuts (no source context) ─

    void transition_to(uint32_t target) {
        transition_from(activeId_, target);
    }

    void return_to_hub(wgpu::Queue queue) {
        if (activeId_ != CartridgeId::GALLERY_RAYMARCH) {
            transition_from(activeId_, CartridgeId::GALLERY_RAYMARCH);
        }
    }

    // ── Return To Previous Room — For Backspace key ──────────────
    // 
    // Returns to the room you entered from (e.g., pressing BACKSPACE in a
    // room with no exit doors returns you through the door you entered).
    // If you haven't transitioned through a door yet, does nothing.
    //
    // This is useful for:
    //   - Rooms with no exit doors
    //   - Quick backtracking without walking back
    //   - Dead-end exploration

    void return_to_previous() {
        if (previousId_ != CartridgeId::NONE && previousId_ != activeId_) {
            std::cout << "[CartridgeManager] BACKSPACE: returning to "
                      << id_name(previousId_) << "\n";
            transition_from(activeId_, previousId_);
        } else {
            // No previous room, or we're already there
            // Do nothing (already where we came from)
        }
    }

private:
    wgpu::Device device_;
    wgpu::TextureFormat colorFormat_;
    wgpu::TextureFormat depthFormat_;

    // All cartridges
    std::unique_ptr<gallery_raymarch::Cartridge> galleryRaymarch_;
    std::unique_ptr<gallery::Cartridge> gallery_;
    std::unique_ptr<n_dimensional_2::Cartridge> nDimensional2_;
    std::unique_ptr<the_board::Cartridge> theBoard_;
    std::unique_ptr<world_compute::Cartridge> worldCompute_;
    std::unique_ptr<terrain_pawn::Cartridge> terrainPawn_;
    std::unique_ptr<pawn_rasterize::Cartridge> pawnRasterize_;
    std::unique_ptr<playground_hybrid::Cartridge> playgroundHybrid_;
    std::unique_ptr<species_studio::Cartridge> speciesStudio_;

    RenderCartridge* active_ = nullptr;
    uint32_t activeId_ = CartridgeId::NONE;
    uint32_t previousId_ = CartridgeId::NONE;  // Track room we came from (for BACKSPACE)
    
    // Re-entrancy lock - prevents callbacks from triggering transitions
    // while a transition is already in progress
    bool inTransition_ = false;

    // Helper for debug output
    static const char* id_name(uint32_t id) {
        switch (id) {
            case CartridgeId::NONE: return "NONE";
            case CartridgeId::GALLERY_RAYMARCH: return "Gallery Raymarch";
            case CartridgeId::GALLERY: return "Gallery";
            case CartridgeId::N_DIMENSIONAL_2: return "N-Dimensional 2";
            case CartridgeId::THE_BOARD: return "The Board";
            case CartridgeId::WORLD_COMPUTE: return "World Compute";
            case CartridgeId::TERRAIN_PAWN: return "Terrain Pawn";
            case CartridgeId::PAWN_RASTERIZE: return "Pawn Rasterize";
            case CartridgeId::PLAYGROUND_HYBRID: return "Playground Hybrid";
            case CartridgeId::SPECIES_STUDIO: return "Species Studio";
            default: return "Unknown";
        }
    }
};

} // namespace t7
