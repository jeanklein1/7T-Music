// STUB — declarations only, the web boot remains the runtime witness.
//
// Emscripten's top-level public surface, reduced to what console.hpp names.
// EM_ASM and friends are macros here rather than inline assembly: the gate
// checks that the C++ AROUND them parses and types, never that the JavaScript
// inside them is correct. Nothing here is compiled into a program.
#pragma once
#include <cstddef>
#include <cstdint>
#include "emscripten/em_types.h"

#define EM_ASM(...)        ((void)0)
#define EM_ASM_INT(...)    (0)
#define EM_ASM_DOUBLE(...) (0.0)
#define EM_JS(ret, name, args, body) ret name args;
#define EMSCRIPTEN_KEEPALIVE

// The rAF driver's own surface. `_arg` was here and the plain form was not,
// so src/the_board.cpp — which calls the plain form — could not compile
// against these stubs, which is half of why no gate ever read it.
#define EM_TIMING_SETTIMEOUT 0
#define EM_TIMING_RAF        1
#define EM_TIMING_SETIMMEDIATE 2

extern "C" {
    void   emscripten_set_main_loop(void (*)(void), int, int);
    void   emscripten_set_main_loop_arg(void (*)(void*), void*, int, int);
    int    emscripten_set_main_loop_timing(int mode, int value);
    void   emscripten_cancel_main_loop(void);
    double emscripten_get_device_pixel_ratio(void);
    double emscripten_get_now(void);
}
