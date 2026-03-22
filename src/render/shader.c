#include <glad/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shader.h"
#include "../core/log.h"

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */

static uint32_t compile_shader(uint32_t type, const char *source)
{
    uint32_t shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, sizeof(info), NULL, info);
        const char *type_str = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
        LOG_ERROR("Shader compile error (%s): %s", type_str, info);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static char *read_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        LOG_ERROR("Failed to open file: %s", path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = (char *)malloc((size_t)length + 1);
    if (!buf) {
        LOG_ERROR("Failed to allocate %ld bytes for file: %s", length, path);
        fclose(f);
        return NULL;
    }

    size_t read_bytes = fread(buf, 1, (size_t)length, f);
    fclose(f);

    buf[read_bytes] = '\0';
    return buf;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

bool shader_load(Shader *s, const char *vert_src, const char *frag_src)
{
    uint32_t vert = compile_shader(GL_VERTEX_SHADER, vert_src);
    if (!vert) return false;

    uint32_t frag = compile_shader(GL_FRAGMENT_SHADER, frag_src);
    if (!frag) {
        glDeleteShader(vert);
        return false;
    }

    uint32_t program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info[512];
        glGetProgramInfoLog(program, sizeof(info), NULL, info);
        LOG_ERROR("Shader link error: %s", info);
        glDeleteProgram(program);
        glDeleteShader(vert);
        glDeleteShader(frag);
        return false;
    }

    /* Shaders are linked into the program; no longer needed individually. */
    glDeleteShader(vert);
    glDeleteShader(frag);

    s->program = program;
    return true;
}

bool shader_load_from_file(Shader *s, const char *vert_path, const char *frag_path)
{
    char *vert_src = read_file(vert_path);
    if (!vert_src) return false;

    char *frag_src = read_file(frag_path);
    if (!frag_src) {
        free(vert_src);
        return false;
    }

    bool ok = shader_load(s, vert_src, frag_src);

    free(vert_src);
    free(frag_src);
    return ok;
}

void shader_use(Shader *s)
{
    glUseProgram(s->program);
}

void shader_destroy(Shader *s)
{
    if (s->program) {
        glDeleteProgram(s->program);
        s->program = 0;
    }
}

void shader_set_int(Shader *s, const char *name, int value)
{
    glUniform1i(glGetUniformLocation(s->program, name), value);
}

void shader_set_float(Shader *s, const char *name, float value)
{
    glUniform1f(glGetUniformLocation(s->program, name), value);
}

void shader_set_vec3(Shader *s, const char *name, Vec3 value)
{
    glUniform3f(glGetUniformLocation(s->program, name), value.x, value.y, value.z);
}

void shader_set_vec4(Shader *s, const char *name, Vec4 value)
{
    glUniform4f(glGetUniformLocation(s->program, name),
                value.x, value.y, value.z, value.w);
}

void shader_set_mat4(Shader *s, const char *name, Mat4 *value)
{
    glUniformMatrix4fv(glGetUniformLocation(s->program, name),
                       1, GL_FALSE, value->e);
}
