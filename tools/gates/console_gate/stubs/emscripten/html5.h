// STUB — declarations only, the web boot remains the runtime witness.
//
// Emscripten's html5.h, reduced to the touch surface console.hpp claims
// (SHIP_1) plus the two result constants it compares against. Field names and
// types match upstream so a rename there fails this gate rather than the boot.
#pragma once
#include <cstdint>
#include "em_types.h"

#define EMSCRIPTEN_RESULT_SUCCESS        0
#define EMSCRIPTEN_EVENT_TARGET_DOCUMENT ((const char*)0)
#define EMSCRIPTEN_EVENT_TOUCHSTART      22
#define EMSCRIPTEN_EVENT_TOUCHEND        23
#define EMSCRIPTEN_EVENT_TOUCHMOVE       24
#define EMSCRIPTEN_EVENT_TOUCHCANCEL     25

extern "C" {
    typedef int EMSCRIPTEN_RESULT;

    typedef struct {
        long identifier;
        long screenX, screenY, clientX, clientY, pageX, pageY, targetX, targetY;
        EM_BOOL isChanged, onTarget;
    } EmscriptenTouchPoint;

    typedef struct {
        double timestamp;
        int numTouches;
        EmscriptenTouchPoint touches[32];
    } EmscriptenTouchEvent;

    typedef EM_BOOL (*em_touch_callback_func)(int, const EmscriptenTouchEvent*, void*);

    EMSCRIPTEN_RESULT emscripten_set_touchstart_callback (const char*, void*, EM_BOOL, em_touch_callback_func);
    EMSCRIPTEN_RESULT emscripten_set_touchmove_callback  (const char*, void*, EM_BOOL, em_touch_callback_func);
    EMSCRIPTEN_RESULT emscripten_set_touchend_callback   (const char*, void*, EM_BOOL, em_touch_callback_func);
    EMSCRIPTEN_RESULT emscripten_set_touchcancel_callback(const char*, void*, EM_BOOL, em_touch_callback_func);
}
