#include <glad/gl.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "../math/mat4.h"
#include "../core/log.h"

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */

static void ui_flush(UIContext *ctx)
{
    if (ctx->quad_count == 0) return;

    /* Always set up correct GL state and shader for UI rendering,
       since flush can be triggered mid-frame by auto-flush in ui_draw_rect */
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Mat4 projection = mat4_ortho(0.0f, (float)ctx->screen_w,
                                 (float)ctx->screen_h, 0.0f,
                                 -1.0f, 1.0f);
    shader_use(&ctx->shader);
    shader_set_mat4(&ctx->shader, "uProjection", &projection);

    int vertex_count = ctx->quad_count * 4;
    size_t size = (size_t)vertex_count * sizeof(UIVertex);

    glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)size, ctx->vertices);

    glBindVertexArray(ctx->vao);
    glDrawElements(GL_TRIANGLES, ctx->quad_count * 6, GL_UNSIGNED_INT, NULL);

    ctx->quad_count = 0;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

bool ui_init(UIContext *ctx)
{
    memset(ctx, 0, sizeof(*ctx));

    /* Load UI shader */
    if (!shader_load_from_file(&ctx->shader,
                               "assets/shaders/ui.vert",
                               "assets/shaders/ui.frag")) {
        LOG_ERROR("Failed to load UI shader");
        return false;
    }

    /* Pre-generate index buffer for all quads:
       Each quad has 4 vertices and 6 indices (two triangles). */
    uint32_t *indices = NULL;
    size_t idx_size = sizeof(uint32_t) * UI_MAX_QUADS * 6;
    indices = (uint32_t *)malloc(idx_size);
    if (!indices) {
        LOG_ERROR("Failed to allocate UI index buffer");
        shader_destroy(&ctx->shader);
        return false;
    }

    for (int i = 0; i < UI_MAX_QUADS; i++) {
        uint32_t base = (uint32_t)(i * 4);
        int idx = i * 6;
        indices[idx + 0] = base + 0;
        indices[idx + 1] = base + 1;
        indices[idx + 2] = base + 2;
        indices[idx + 3] = base + 2;
        indices[idx + 4] = base + 3;
        indices[idx + 5] = base + 0;
    }

    /* Create VAO, VBO, EBO */
    glGenVertexArrays(1, &ctx->vao);
    glGenBuffers(1, &ctx->vbo);
    glGenBuffers(1, &ctx->ebo);

    glBindVertexArray(ctx->vao);

    /* VBO: dynamic, sized for maximum vertices */
    glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(sizeof(UIVertex) * UI_MAX_QUADS * 4),
                 NULL, GL_DYNAMIC_DRAW);

    /* EBO: static, pre-generated indices */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr)idx_size, indices, GL_STATIC_DRAW);

    free(indices);

    /* Vertex layout: position(2f) + uv(2f) + color(4f) = 8 floats */
    size_t stride = sizeof(UIVertex);

    /* location 0: aPos (vec2) */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, (GLsizei)stride,
                          (const void *)offsetof(UIVertex, position));

    /* location 1: aUV (vec2) */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, (GLsizei)stride,
                          (const void *)offsetof(UIVertex, uv));

    /* location 2: aColor (vec4) */
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, (GLsizei)stride,
                          (const void *)offsetof(UIVertex, color));

    glBindVertexArray(0);

    LOG_INFO("UI system initialized (max %d quads)", UI_MAX_QUADS);
    return true;
}

void ui_destroy(UIContext *ctx)
{
    if (ctx->vao) { glDeleteVertexArrays(1, &ctx->vao); ctx->vao = 0; }
    if (ctx->vbo) { glDeleteBuffers(1, &ctx->vbo); ctx->vbo = 0; }
    if (ctx->ebo) { glDeleteBuffers(1, &ctx->ebo); ctx->ebo = 0; }
    shader_destroy(&ctx->shader);
}

void ui_begin(UIContext *ctx, int screen_w, int screen_h,
              float mouse_x, float mouse_y, bool mouse_down, bool mouse_pressed)
{
    ctx->screen_w = screen_w;
    ctx->screen_h = screen_h;
    ctx->mouse_x = mouse_x;
    ctx->mouse_y = mouse_y;
    ctx->mouse_down = mouse_down;
    ctx->mouse_pressed = mouse_pressed;
    ctx->quad_count = 0;
    ctx->hot_id = 0;
}

void ui_end(UIContext *ctx)
{
    /* Flush remaining quads (ui_flush handles shader/state setup) */
    ui_flush(ctx);

    /* Restore default GL state */
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
}

void ui_draw_rect(UIContext *ctx, float x, float y, float w, float h, Vec4 color)
{
    /* Auto-flush if buffer is full */
    if (ctx->quad_count >= UI_MAX_QUADS) {
        ui_flush(ctx);
    }

    int base = ctx->quad_count * 4;
    Vec2 uv = vec2(0.0f, 0.0f);

    /* top-left */
    ctx->vertices[base + 0].position = vec2(x, y);
    ctx->vertices[base + 0].uv = uv;
    ctx->vertices[base + 0].color = color;

    /* top-right */
    ctx->vertices[base + 1].position = vec2(x + w, y);
    ctx->vertices[base + 1].uv = uv;
    ctx->vertices[base + 1].color = color;

    /* bottom-right */
    ctx->vertices[base + 2].position = vec2(x + w, y + h);
    ctx->vertices[base + 2].uv = uv;
    ctx->vertices[base + 2].color = color;

    /* bottom-left */
    ctx->vertices[base + 3].position = vec2(x, y + h);
    ctx->vertices[base + 3].uv = uv;
    ctx->vertices[base + 3].color = color;

    ctx->quad_count++;
}
