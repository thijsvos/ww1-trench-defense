#ifndef TD_SHADER_H
#define TD_SHADER_H

#include <stdbool.h>
#include <stdint.h>
#include "../math/math_types.h"

typedef struct Shader {
    uint32_t program;
} Shader;

bool shader_load(Shader *s, const char *vert_src, const char *frag_src);
bool shader_load_from_file(Shader *s, const char *vert_path, const char *frag_path);
void shader_use(Shader *s);
void shader_destroy(Shader *s);
void shader_set_int(Shader *s, const char *name, int value);
void shader_set_float(Shader *s, const char *name, float value);
void shader_set_vec3(Shader *s, const char *name, Vec3 value);
void shader_set_vec4(Shader *s, const char *name, Vec4 value);
void shader_set_mat4(Shader *s, const char *name, Mat4 *value);

#endif /* TD_SHADER_H */
