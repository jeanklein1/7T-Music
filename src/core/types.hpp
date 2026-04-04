#pragma once

/**
 * TYPES - Platform-Agnostic Math Types
 * =====================================
 * 
 * Simple POD types for vectors. No dependencies.
 * 
 * These exist so that WorldState and field math can be platform-agnostic.
 * Adapters convert to platform types (glm::vec3, etc.) at the boundary.
 */

#include <cmath>

namespace t7 {

// ═══════════════════════════════════════════════════════════════════════════════
// CONSTANTS
// ═══════════════════════════════════════════════════════════════════════════════

constexpr float PI = 3.14159265358979323846f;

// ═══════════════════════════════════════════════════════════════════════════════
// FLOAT2
// ═══════════════════════════════════════════════════════════════════════════════

struct float2 {
    float x = 0.0f;
    float y = 0.0f;
    
    float2() = default;
    constexpr float2(float x_, float y_) : x(x_), y(y_) {}
    
    float2 operator+(const float2& b) const { return {x + b.x, y + b.y}; }
    float2 operator-(const float2& b) const { return {x - b.x, y - b.y}; }
    float2 operator*(float s) const { return {x * s, y * s}; }
    float2 operator/(float s) const { return {x / s, y / s}; }
    
    float2& operator+=(const float2& b) { x += b.x; y += b.y; return *this; }
    float2& operator-=(const float2& b) { x -= b.x; y -= b.y; return *this; }
    float2& operator*=(float s) { x *= s; y *= s; return *this; }
    
    float length() const { return std::sqrt(x*x + y*y); }
    float length_squared() const { return x*x + y*y; }
    
    float2 normalized() const {
        float len = length();
        return len > 0.0f ? float2{x/len, y/len} : float2{0.0f, 0.0f};
    }
};

inline float dot(const float2& a, const float2& b) {
    return a.x * b.x + a.y * b.y;
}

inline float2 operator*(float s, const float2& v) { return v * s; }

// ═══════════════════════════════════════════════════════════════════════════════
// FLOAT3
// ═══════════════════════════════════════════════════════════════════════════════

struct float3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    
    float3() = default;
    constexpr float3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    
    float3 operator+(const float3& b) const { return {x + b.x, y + b.y, z + b.z}; }
    float3 operator-(const float3& b) const { return {x - b.x, y - b.y, z - b.z}; }
    float3 operator*(float s) const { return {x * s, y * s, z * s}; }
    float3 operator/(float s) const { return {x / s, y / s, z / s}; }
    float3 operator-() const { return {-x, -y, -z}; }
    
    float3& operator+=(const float3& b) { x += b.x; y += b.y; z += b.z; return *this; }
    float3& operator-=(const float3& b) { x -= b.x; y -= b.y; z -= b.z; return *this; }
    float3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    
    float length() const { return std::sqrt(x*x + y*y + z*z); }
    float length_squared() const { return x*x + y*y + z*z; }
    
    float3 normalized() const {
        float len = length();
        return len > 0.0f ? float3{x/len, y/len, z/len} : float3{0.0f, 0.0f, 0.0f};
    }
};

inline float dot(const float3& a, const float3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float3 cross(const float3& a, const float3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

inline float3 operator*(float s, const float3& v) { return v * s; }

// ═══════════════════════════════════════════════════════════════════════════════
// FLOAT4 (for GPU-friendly layouts)
// ═══════════════════════════════════════════════════════════════════════════════

struct float4 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 0.0f;
    
    float4() = default;
    constexpr float4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    constexpr float4(const float3& v, float w_) : x(v.x), y(v.y), z(v.z), w(w_) {}
    
    float3 xyz() const { return {x, y, z}; }
};

// ═══════════════════════════════════════════════════════════════════════════════
// UTILITY
// ═══════════════════════════════════════════════════════════════════════════════

inline float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

inline float3 lerp(const float3& a, const float3& b, float t) {
    return a + (b - a) * t;
}

inline float clamp(float x, float lo, float hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}

inline float saturate(float x) {
    return clamp(x, 0.0f, 1.0f);
}

} // namespace t7
