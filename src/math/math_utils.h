#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static inline float to_radians(float degrees) {
    return degrees * ((float)M_PI / 180.0f);
}

static inline float to_degrees(float radians) {
    return radians * (180.0f / (float)M_PI);
}

static inline float clampf(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

static inline float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

static inline float minf(float a, float b) {
    return a < b ? a : b;
}

static inline float maxf(float a, float b) {
    return a > b ? a : b;
}

#endif /* MATH_UTILS_H */
