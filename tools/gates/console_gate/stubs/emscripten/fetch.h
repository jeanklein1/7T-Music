// STUB — declarations only, the web boot remains the runtime witness.
//
// Emscripten's fetch.h, reduced to what gallery.hpp names (EXHIBIT_0: the
// web twin's byte source is the network). Field names and types match
// upstream so a rename there fails a gate rather than a boot.
#pragma once
#include <cstddef>
#include <cstdint>

#define EMSCRIPTEN_FETCH_LOAD_TO_MEMORY  1
#define EMSCRIPTEN_FETCH_STREAM_DATA     2
#define EMSCRIPTEN_FETCH_PERSIST_FILE    4
#define EMSCRIPTEN_FETCH_APPEND          8
#define EMSCRIPTEN_FETCH_REPLACE        16
#define EMSCRIPTEN_FETCH_NO_DOWNLOAD    32
#define EMSCRIPTEN_FETCH_SYNCHRONOUS    64
#define EMSCRIPTEN_FETCH_WAITABLE      128

extern "C" {
    typedef struct emscripten_fetch_t emscripten_fetch_t;

    typedef struct emscripten_fetch_attr_t {
        char requestMethod[32];
        void* userData;
        void (*onsuccess)(emscripten_fetch_t*);
        void (*onerror)(emscripten_fetch_t*);
        void (*onprogress)(emscripten_fetch_t*);
        void (*onreadystatechange)(emscripten_fetch_t*);
        uint32_t attributes;
        unsigned long timeoutMSecs;
        int withCredentials;
        const char* destinationPath;
        const char* userName;
        const char* password;
        const char* const* requestHeaders;
        const char* overriddenMimeType;
        const char* requestData;
        size_t requestDataSize;
    } emscripten_fetch_attr_t;

    struct emscripten_fetch_t {
        unsigned int id;
        void* userData;
        const char* url;
        const char* data;
        uint64_t numBytes;
        uint64_t dataOffset;
        uint64_t totalBytes;
        unsigned short readyState;
        unsigned short status;
        char statusText[64];
    };

    void emscripten_fetch_attr_init(emscripten_fetch_attr_t*);
    emscripten_fetch_t* emscripten_fetch(emscripten_fetch_attr_t*, const char*);
    void emscripten_fetch_close(emscripten_fetch_t*);
}
