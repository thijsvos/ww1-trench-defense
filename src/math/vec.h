#ifndef VEC_H
#define VEC_H

#define _USE_MATH_DEFINES
#include <math.h>
#include "math_types.h"

/* ---------- Vec2 operations ---------- */

static inline Vec2 vec2_add(Vec2 a, Vec2 b) {
    return vec2(a.x + b.x, a.y + b.y);
}

static inline Vec2 vec2_sub(Vec2 a, Vec2 b) {
    return vec2(a.x - b.x, a.y - b.y);
}

static inline Vec2 vec2_scale(Vec2 v, float s) {
    return vec2(v.x * s, v.y * s);
}

static inline float vec2_dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

static inline float vec2_length(Vec2 v) {
    return sqrtf(vec2_dot(v, v));
}

static inline Vec2 vec2_normalize(Vec2 v) {
    float len = vec2_length(v);
    if (len > 0.0f) {
        float inv = 1.0f / len;
        return vec2(v.x * inv, v.y * inv);
    }
    return vec2(0.0f, 0.0f);
}

static inline float vec2_distance(Vec2 a, Vec2 b) {
    return vec2_length(vec2_sub(a, b));
}

/* ---------- Vec3 operations ---------- */

static inline Vec3 vec3_add(Vec3 a, Vec3 b) {
    return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vec3 vec3_sub(Vec3 a, Vec3 b) {
    return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline Vec3 vec3_scale(Vec3 v, float s) {
    return vec3(v.x * s, v.y * s, v.z * s);
}

static inline Vec3 vec3_cross(Vec3 a, Vec3 b) {
    return vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

static inline float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float vec3_length(Vec3 v) {
    return sqrtf(vec3_dot(v, v));
}

static inline Vec3 vec3_normalize(Vec3 v) {
    float len = vec3_length(v);
    if (len > 0.0f) {
        float inv = 1.0f / len;
        return vec3(v.x * inv, v.y * inv, v.z * inv);
    }
    return vec3(0.0f, 0.0f, 0.0f);
}

#endif /* VEC_H */
