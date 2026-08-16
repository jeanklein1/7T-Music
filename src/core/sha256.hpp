#pragma once

// ─── core/sha256.hpp ─────────────────────────────────────────────
//
// PROBATE_SEAL2 — SHA-256, so the program can say what it received.
//
// WHY THIS EXISTS. The Pixel was served a `world.wgsl` whose tail was
// cut mid-token — `:13639:28 expected ')'` inside `orb_vs`, 665 bytes
// short of the end — while every gate in the tree was green. It had to
// be: the tree was clean, the packaging copies the shader byte-for-byte,
// and no witness stood anywhere between `dist/` and the device. The
// corridor from the build to the audience was the one stretch of this
// program nothing testified about, so the defect lived there for as long
// as it liked and announced itself as a syntax error in a file that has
// no syntax error.
//
// One fact, two ends: `tools/web_dist.py` hashes the shader it ships and
// bakes the digest into the page; the loader hashes the bytes it
// actually received and compares. Both ends must compute the SAME
// function over the SAME bytes or the witness is worse than none — so
// this is the plain FIPS 180-4 SHA-256 with no framing, no salt and no
// encoding step, exactly what Python's `hashlib.sha256(bytes)` computes.
// `tools/gates/sha256_gate/` is the witness of that agreement: it
// compiles this header and holds its digest of the real `world.wgsl`
// against `hashlib`'s.
//
// NOT A SECURITY BOUNDARY. This detects corruption — truncation, a
// stale package, a half-written CDN object — not tampering. Anyone who
// can rewrite the shader can rewrite the digest beside it. The threat
// model is a broken pipe, and that is the whole of it.
//
// Self-contained on purpose: no OpenSSL, no emscripten dependency, and
// nothing to link. The web twin has no crypto library and SubtleCrypto
// is async, which the synchronous boot path cannot spend.

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace t7 {

    namespace sha256_detail {

        inline constexpr uint32_t K[64] = {
            0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
            0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
            0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
            0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
            0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
            0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
            0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
            0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
            0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
            0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
            0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
            0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
            0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
            0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
            0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
            0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u,
        };

        inline uint32_t rotr(uint32_t x, int n) {
            return (x >> n) | (x << (32 - n));
        }

        inline void compress(uint32_t h[8], const unsigned char* p) {
            uint32_t w[64];
            for (int i = 0; i < 16; i++) {
                w[i] = (static_cast<uint32_t>(p[i * 4 + 0]) << 24)
                     | (static_cast<uint32_t>(p[i * 4 + 1]) << 16)
                     | (static_cast<uint32_t>(p[i * 4 + 2]) << 8)
                     | (static_cast<uint32_t>(p[i * 4 + 3]));
            }
            for (int i = 16; i < 64; i++) {
                uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
                uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }
            uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
            uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];
            for (int i = 0; i < 64; i++) {
                uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
                uint32_t ch = (e & f) ^ (~e & g);
                uint32_t t1 = hh + S1 + ch + K[i] + w[i];
                uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
                uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                uint32_t t2 = S0 + maj;
                hh = g; g = f; f = e; e = d + t1;
                d = c; c = b; b = a; a = t1 + t2;
            }
            h[0] += a; h[1] += b; h[2] += c; h[3] += d;
            h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
        }

    } // namespace sha256_detail

    // Lowercase hex digest of `bytes`, 64 characters. Byte-for-byte the
    // same string Python's hashlib.sha256(...).hexdigest() returns for
    // the same input — witnessed by tools/gates/sha256_gate/.
    inline std::string sha256_hex(std::string_view bytes) {
        using namespace sha256_detail;
        uint32_t h[8] = { 0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
                          0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u };

        const unsigned char* p = reinterpret_cast<const unsigned char*>(bytes.data());
        size_t n = bytes.size();
        size_t full = n / 64;
        for (size_t i = 0; i < full; i++) compress(h, p + i * 64);

        // The tail, then the 0x80 pad, then zeroes, then the LENGTH IN
        // BITS as a big-endian 64. A 64-bit bit length overflows a
        // size_t byte count only above 2^61 bytes, which no shader is.
        unsigned char tail[128] = {};
        size_t rem = n - full * 64;
        for (size_t i = 0; i < rem; i++) tail[i] = p[full * 64 + i];
        tail[rem] = 0x80;
        size_t tail_len = (rem < 56) ? 64 : 128;
        uint64_t bits = static_cast<uint64_t>(n) * 8u;
        for (int i = 0; i < 8; i++)
            tail[tail_len - 1 - i] = static_cast<unsigned char>((bits >> (8 * i)) & 0xffu);
        for (size_t off = 0; off < tail_len; off += 64) compress(h, tail + off);

        static const char* HEX = "0123456789abcdef";
        std::string out;
        out.reserve(64);
        for (int i = 0; i < 8; i++) {
            for (int s = 28; s >= 0; s -= 4)
                out.push_back(HEX[(h[i] >> s) & 0xfu]);
        }
        return out;
    }

    // The short form both ends print. Eight hex characters is 32 bits:
    // ample for naming a corruption, and short enough to read off a
    // phone console at arm's length, which is where this gets read.
    inline std::string sha256_short(std::string_view bytes) {
        return sha256_hex(bytes).substr(0, 8);
    }

} // namespace t7
