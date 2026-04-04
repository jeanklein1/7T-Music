#pragma once

/**
 * INPUT EVENT - Console Input Abstraction
 * ========================================
 * 
 * Platform-agnostic input events produced by the console and consumed by cartridges.
 * 
 * The console translates platform-specific input (GLFW, SDL, etc.) into these
 * events. Cartridges receive them and decide what each input means in their
 * domain — a key might be "move forward" to a render cartridge or "play note C"
 * to an analysis cartridge.
 * 
 * DESIGN PRINCIPLE
 * ----------------
 * 
 * The console knows HOW input arrives (which key, which button).
 * The cartridge knows WHAT input means (movement, music, camera control).
 * 
 * This struct carries the "how" across the boundary.
 */

namespace t7 {

struct InputEvent {
    enum class Type {
        KeyDown,
        KeyUp,
        MouseMove,
        MouseButton,
        Scroll
    };
    
    Type type;
    
    // Key events
    int key;              // Platform key code (e.g., GLFW_KEY_A)
    char character;       // ASCII character if printable, 0 otherwise
    
    // Mouse events
    float x, y;           // Position or delta depending on event type
    int button;           // Which button (0=left, 1=right, 2=middle)
    bool pressed;         // For MouseButton: true=pressed, false=released
};

} // namespace t7
