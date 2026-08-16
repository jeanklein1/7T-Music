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

extern "C" {
    void   emscripten_set_main_loop_arg(void (*)(void*), void*, int, int);
    void   emscripten_cancel_main_loop(void);
    double emscripten_get_device_pixel_ratio(void);
    double emscripten_get_now(void);
}
