#ifndef MATH_TYPES_H
#define MATH_TYPES_H

typedef union Vec2 {
    struct { float x, y; };
    float e[2];
} Vec2;

typedef union Vec3 {
    struct { float x, y, z; };
    float e[3];
} Vec3;

typedef union Vec4 {
    struct { float x, y, z, w; };
    float e[4];
} Vec4;

typedef union Mat4 {
    float m[4][4]; /* column-major: m[col][row] */
    float e[16];
} Mat4;

typedef struct IVec2 {
    int x, y;
} IVec2;

static inline Vec2 vec2(float x, float y) {
    Vec2 v;
    v.x = x;
    v.y = y;
    return v;
}

static inline Vec3 vec3(float x, float y, float z) {
    Vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

static inline Vec4 vec4(float x, float y, float z, float w) {
    Vec4 v;
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = w;
    return v;
}

static inline IVec2 ivec2(int x, int y) {
    IVec2 v;
    v.x = x;
    v.y = y;
    return v;
}

#endif /* MATH_TYPES_H */
