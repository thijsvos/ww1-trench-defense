#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include "mat4.h"

Mat4 mat4_identity(void) {
    Mat4 r;
    memset(&r, 0, sizeof(Mat4));
    r.m[0][0] = 1.0f;
    r.m[1][1] = 1.0f;
    r.m[2][2] = 1.0f;
    r.m[3][3] = 1.0f;
    return r;
}

Mat4 mat4_multiply(Mat4 a, Mat4 b) {
    Mat4 r;
    int i, j, k;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            r.m[i][j] = 0.0f;
            for (k = 0; k < 4; k++) {
                r.m[i][j] += a.m[k][j] * b.m[i][k];
            }
        }
    }
    return r;
}

Mat4 mat4_translate(Vec3 t) {
    Mat4 r = mat4_identity();
    r.m[3][0] = t.x;
    r.m[3][1] = t.y;
    r.m[3][2] = t.z;
    return r;
}

Mat4 mat4_scale(Vec3 s) {
    Mat4 r = mat4_identity();
    r.m[0][0] = s.x;
    r.m[1][1] = s.y;
    r.m[2][2] = s.z;
    return r;
}

Mat4 mat4_rotate_x(float radians) {
    float c = cosf(radians);
    float s = sinf(radians);
    Mat4 r = mat4_identity();
    r.m[1][1] =  c;
    r.m[1][2] =  s;
    r.m[2][1] = -s;
    r.m[2][2] =  c;
    return r;
}

Mat4 mat4_rotate_y(float radians) {
    float c = cosf(radians);
    float s = sinf(radians);
    Mat4 r = mat4_identity();
    r.m[0][0] =  c;
    r.m[0][2] = -s;
    r.m[2][0] =  s;
    r.m[2][2] =  c;
    return r;
}

Mat4 mat4_rotate_z(float radians) {
    float c = cosf(radians);
    float s = sinf(radians);
    Mat4 r = mat4_identity();
    r.m[0][0] =  c;
    r.m[0][1] =  s;
    r.m[1][0] = -s;
    r.m[1][1] =  c;
    return r;
}

Mat4 mat4_ortho(float left, float right, float bottom, float top,
                float near, float far) {
    Mat4 r = mat4_identity();
    r.m[0][0] =  2.0f / (right - left);
    r.m[1][1] =  2.0f / (top - bottom);
    r.m[2][2] = -2.0f / (far - near);
    r.m[3][0] = -(right + left) / (right - left);
    r.m[3][1] = -(top + bottom) / (top - bottom);
    r.m[3][2] = -(far + near)   / (far - near);
    return r;
}

Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up) {
    Vec3 f = vec3_normalize(vec3_sub(center, eye));
    Vec3 s = vec3_normalize(vec3_cross(f, up));
    Vec3 u = vec3_cross(s, f);

    Mat4 r = mat4_identity();
    r.m[0][0] =  s.x;
    r.m[1][0] =  s.y;
    r.m[2][0] =  s.z;
    r.m[0][1] =  u.x;
    r.m[1][1] =  u.y;
    r.m[2][1] =  u.z;
    r.m[0][2] = -f.x;
    r.m[1][2] = -f.y;
    r.m[2][2] = -f.z;
    r.m[3][0] = -vec3_dot(s, eye);
    r.m[3][1] = -vec3_dot(u, eye);
    r.m[3][2] =  vec3_dot(f, eye);
    return r;
}

Mat4 mat4_inverse(Mat4 m) {
    Mat4 r;
    float s[6], c[6];
    float det, inv_det;

    /* 2x2 sub-determinants from first two columns */
    s[0] = m.m[0][0] * m.m[1][1] - m.m[1][0] * m.m[0][1];
    s[1] = m.m[0][0] * m.m[1][2] - m.m[1][0] * m.m[0][2];
    s[2] = m.m[0][0] * m.m[1][3] - m.m[1][0] * m.m[0][3];
    s[3] = m.m[0][1] * m.m[1][2] - m.m[1][1] * m.m[0][2];
    s[4] = m.m[0][1] * m.m[1][3] - m.m[1][1] * m.m[0][3];
    s[5] = m.m[0][2] * m.m[1][3] - m.m[1][2] * m.m[0][3];

    /* 2x2 sub-determinants from last two columns */
    c[0] = m.m[2][0] * m.m[3][1] - m.m[3][0] * m.m[2][1];
    c[1] = m.m[2][0] * m.m[3][2] - m.m[3][0] * m.m[2][2];
    c[2] = m.m[2][0] * m.m[3][3] - m.m[3][0] * m.m[2][3];
    c[3] = m.m[2][1] * m.m[3][2] - m.m[3][1] * m.m[2][2];
    c[4] = m.m[2][1] * m.m[3][3] - m.m[3][1] * m.m[2][3];
    c[5] = m.m[2][2] * m.m[3][3] - m.m[3][2] * m.m[2][3];

    det = s[0] * c[5] - s[1] * c[4] + s[2] * c[3]
        + s[3] * c[2] - s[4] * c[1] + s[5] * c[0];

    if (fabsf(det) < 1e-8f) {
        return mat4_identity();
    }

    inv_det = 1.0f / det;

    r.m[0][0] = ( m.m[1][1] * c[5] - m.m[1][2] * c[4] + m.m[1][3] * c[3]) * inv_det;
    r.m[0][1] = (-m.m[0][1] * c[5] + m.m[0][2] * c[4] - m.m[0][3] * c[3]) * inv_det;
    r.m[0][2] = ( m.m[3][1] * s[5] - m.m[3][2] * s[4] + m.m[3][3] * s[3]) * inv_det;
    r.m[0][3] = (-m.m[2][1] * s[5] + m.m[2][2] * s[4] - m.m[2][3] * s[3]) * inv_det;

    r.m[1][0] = (-m.m[1][0] * c[5] + m.m[1][2] * c[2] - m.m[1][3] * c[1]) * inv_det;
    r.m[1][1] = ( m.m[0][0] * c[5] - m.m[0][2] * c[2] + m.m[0][3] * c[1]) * inv_det;
    r.m[1][2] = (-m.m[3][0] * s[5] + m.m[3][2] * s[2] - m.m[3][3] * s[1]) * inv_det;
    r.m[1][3] = ( m.m[2][0] * s[5] - m.m[2][2] * s[2] + m.m[2][3] * s[1]) * inv_det;

    r.m[2][0] = ( m.m[1][0] * c[4] - m.m[1][1] * c[2] + m.m[1][3] * c[0]) * inv_det;
    r.m[2][1] = (-m.m[0][0] * c[4] + m.m[0][1] * c[2] - m.m[0][3] * c[0]) * inv_det;
    r.m[2][2] = ( m.m[3][0] * s[4] - m.m[3][1] * s[2] + m.m[3][3] * s[0]) * inv_det;
    r.m[2][3] = (-m.m[2][0] * s[4] + m.m[2][1] * s[2] - m.m[2][3] * s[0]) * inv_det;

    r.m[3][0] = (-m.m[1][0] * c[3] + m.m[1][1] * c[1] - m.m[1][2] * c[0]) * inv_det;
    r.m[3][1] = ( m.m[0][0] * c[3] - m.m[0][1] * c[1] + m.m[0][2] * c[0]) * inv_det;
    r.m[3][2] = (-m.m[3][0] * s[3] + m.m[3][1] * s[1] - m.m[3][2] * s[0]) * inv_det;
    r.m[3][3] = ( m.m[2][0] * s[3] - m.m[2][1] * s[1] + m.m[2][2] * s[0]) * inv_det;

    return r;
}

Vec4 mat4_mul_vec4(Mat4 m, Vec4 v) {
    Vec4 r;
    r.x = m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z + m.m[3][0] * v.w;
    r.y = m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z + m.m[3][1] * v.w;
    r.z = m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z + m.m[3][2] * v.w;
    r.w = m.m[0][3] * v.x + m.m[1][3] * v.y + m.m[2][3] * v.z + m.m[3][3] * v.w;
    return r;
}
