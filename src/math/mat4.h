#ifndef MAT4_H
#define MAT4_H

#include "vec.h"

Mat4 mat4_identity(void);
Mat4 mat4_multiply(Mat4 a, Mat4 b);
Mat4 mat4_translate(Vec3 translation);
Mat4 mat4_scale(Vec3 scale);
Mat4 mat4_rotate_x(float radians);
Mat4 mat4_rotate_y(float radians);
Mat4 mat4_rotate_z(float radians);
Mat4 mat4_ortho(float left, float right, float bottom, float top, float near, float far);
Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up);
Mat4 mat4_inverse(Mat4 m);
Vec4 mat4_mul_vec4(Mat4 m, Vec4 v);

#endif /* MAT4_H */
