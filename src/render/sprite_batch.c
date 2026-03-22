#include <glad/gl.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include "sprite_batch.h"
#include "../core/log.h"

/* ------------------------------------------------------------------ */
/* Internal state                                                      */
/* ------------------------------------------------------------------ */

static Mat4 *s_view_projection = NULL;

/* ------------------------------------------------------------------ */
/* Internal helpers                                                    */
/* ------------------------------------------------------------------ */

static void flush(SpriteBatch *batch)
{
    if (batch->quad_count == 0) return;

    int vertex_count = batch->quad_count * 4;
    int index_count  = batch->quad_count * 6;

    /* Upload vertex data */
    glBindBuffer(GL_ARRAY_BUFFER, batch->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    (GLsizeiptr)(vertex_count * (int)sizeof(SpriteVertex)),
                    batch->vertices);

    /* Bind shader and set uniforms */
    shader_use(&batch->shader);
    shader_set_mat4(&batch->shader, "uVP", s_view_projection);

    if (batch->current_texture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, batch->current_texture);
        shader_set_int(&batch->shader, "uTexture", 0);
        shader_set_int(&batch->shader, "uUseTexture", 1);
    } else {
        shader_set_int(&batch->shader, "uUseTexture", 0);
    }

    /* Draw */
    glBindVertexArray(batch->vao);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    /* Reset for next batch */
    batch->quad_count = 0;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

bool sprite_batch_init(SpriteBatch *batch)
{
    memset(batch, 0, sizeof(SpriteBatch));

    /* Load sprite shader */
    if (!shader_load_from_file(&batch->shader,
                               "assets/shaders/sprite.vert",
                               "assets/shaders/sprite.frag")) {
        LOG_ERROR("Failed to load sprite batch shader");
        return false;
    }

    /* Generate VAO / VBO / EBO */
    glGenVertexArrays(1, &batch->vao);
    glGenBuffers(1, &batch->vbo);
    glGenBuffers(1, &batch->ebo);

    glBindVertexArray(batch->vao);

    /* Dynamic VBO — sized for maximum vertices */
    glBindBuffer(GL_ARRAY_BUFFER, batch->vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(SPRITE_BATCH_MAX_VERTICES * (int)sizeof(SpriteVertex)),
                 NULL, GL_DYNAMIC_DRAW);

    /* Pre-generate index buffer: 0,1,2, 0,2,3, 4,5,6, 4,6,7, ... */
    uint32_t indices[SPRITE_BATCH_MAX_INDICES];
    for (int i = 0; i < SPRITE_BATCH_MAX_QUADS; i++) {
        uint32_t base = (uint32_t)(i * 4);
        int idx       = i * 6;
        indices[idx + 0] = base + 0;
        indices[idx + 1] = base + 1;
        indices[idx + 2] = base + 2;
        indices[idx + 3] = base + 0;
        indices[idx + 4] = base + 2;
        indices[idx + 5] = base + 3;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    /* Vertex layout: position(3f) + uv(2f) + color(4f) = 9 floats */
    int stride = (int)sizeof(SpriteVertex);

    /* layout(location = 0) in vec3 aPos */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          (void *)offsetof(SpriteVertex, position));
    glEnableVertexAttribArray(0);

    /* layout(location = 1) in vec2 aUV */
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride,
                          (void *)offsetof(SpriteVertex, uv));
    glEnableVertexAttribArray(1);

    /* layout(location = 2) in vec4 aColor */
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride,
                          (void *)offsetof(SpriteVertex, color));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    LOG_INFO("Sprite batch initialised (max %d quads)", SPRITE_BATCH_MAX_QUADS);
    return true;
}

void sprite_batch_destroy(SpriteBatch *batch)
{
    glDeleteVertexArrays(1, &batch->vao);
    glDeleteBuffers(1, &batch->vbo);
    glDeleteBuffers(1, &batch->ebo);
    shader_destroy(&batch->shader);

    LOG_INFO("Sprite batch destroyed");
}

void sprite_batch_begin(SpriteBatch *batch, Mat4 *view_projection)
{
    s_view_projection      = view_projection;
    batch->quad_count      = 0;
    batch->current_texture = 0;
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void sprite_batch_end(SpriteBatch *batch)
{
    flush(batch);
    s_view_projection = NULL;
    glDisable(GL_BLEND);
}

void sprite_batch_draw_quad(SpriteBatch *batch, Vec3 position, Vec2 size, Vec4 color)
{
    /* Flush if we need to switch away from a textured batch */
    if (batch->current_texture != 0) {
        flush(batch);
        batch->current_texture = 0;
    }

    /* Auto-flush if batch is full */
    if (batch->quad_count >= SPRITE_BATCH_MAX_QUADS) {
        flush(batch);
    }

    float x = position.x;
    float y = position.y;
    float z = position.z;
    float w = size.x;
    float h = size.y;

    int base = batch->quad_count * 4;

    /* Quad on XZ plane: bottom-left, bottom-right, top-right, top-left */
    batch->vertices[base + 0].position = vec3(x,     y, z);
    batch->vertices[base + 0].uv       = vec2(0.0f, 0.0f);
    batch->vertices[base + 0].color    = color;

    batch->vertices[base + 1].position = vec3(x + w, y, z);
    batch->vertices[base + 1].uv       = vec2(1.0f, 0.0f);
    batch->vertices[base + 1].color    = color;

    batch->vertices[base + 2].position = vec3(x + w, y, z + h);
    batch->vertices[base + 2].uv       = vec2(1.0f, 1.0f);
    batch->vertices[base + 2].color    = color;

    batch->vertices[base + 3].position = vec3(x,     y, z + h);
    batch->vertices[base + 3].uv       = vec2(0.0f, 1.0f);
    batch->vertices[base + 3].color    = color;

    batch->quad_count++;
}

void sprite_batch_draw_textured(SpriteBatch *batch, Vec3 position, Vec2 size,
                                TextureRegion *region, Vec4 tint)
{
    uint32_t tex_id = region->texture->id;

    /* Flush if texture changes */
    if (batch->current_texture != 0 && batch->current_texture != tex_id) {
        flush(batch);
    }
    batch->current_texture = tex_id;

    /* Auto-flush if batch is full */
    if (batch->quad_count >= SPRITE_BATCH_MAX_QUADS) {
        flush(batch);
    }

    float x = position.x;
    float y = position.y;
    float z = position.z;
    float w = size.x;
    float h = size.y;

    int base = batch->quad_count * 4;

    /* Quad on XZ plane with atlas UVs */
    batch->vertices[base + 0].position = vec3(x,     y, z);
    batch->vertices[base + 0].uv       = vec2(region->u0, region->v0);
    batch->vertices[base + 0].color    = tint;

    batch->vertices[base + 1].position = vec3(x + w, y, z);
    batch->vertices[base + 1].uv       = vec2(region->u1, region->v0);
    batch->vertices[base + 1].color    = tint;

    batch->vertices[base + 2].position = vec3(x + w, y, z + h);
    batch->vertices[base + 2].uv       = vec2(region->u1, region->v1);
    batch->vertices[base + 2].color    = tint;

    batch->vertices[base + 3].position = vec3(x,     y, z + h);
    batch->vertices[base + 3].uv       = vec2(region->u0, region->v1);
    batch->vertices[base + 3].color    = tint;

    batch->quad_count++;
}

void sprite_batch_draw_rotated(SpriteBatch *batch, Vec3 center, Vec2 size,
                               float angle, TextureRegion *region, Vec4 tint)
{
    uint32_t tex_id = region->texture->id;

    if (batch->current_texture != 0 && batch->current_texture != tex_id) {
        flush(batch);
    }
    batch->current_texture = tex_id;

    if (batch->quad_count >= SPRITE_BATCH_MAX_QUADS) {
        flush(batch);
    }

    float hw = size.x * 0.5f;
    float hh = size.y * 0.5f;
    float c  = cosf(angle);
    float s  = sinf(angle);

    float cx = center.x;
    float y  = center.y;
    float cz = center.z;

    int base = batch->quad_count * 4;
    float lx, lz;

    /* corner 0: (-hw, -hh) */
    lx = -hw; lz = -hh;
    batch->vertices[base + 0].position = vec3(cx + lx*c - lz*s, y, cz + lx*s + lz*c);
    batch->vertices[base + 0].uv       = vec2(region->u0, region->v0);
    batch->vertices[base + 0].color    = tint;

    /* corner 1: (+hw, -hh) */
    lx = hw; lz = -hh;
    batch->vertices[base + 1].position = vec3(cx + lx*c - lz*s, y, cz + lx*s + lz*c);
    batch->vertices[base + 1].uv       = vec2(region->u1, region->v0);
    batch->vertices[base + 1].color    = tint;

    /* corner 2: (+hw, +hh) */
    lx = hw; lz = hh;
    batch->vertices[base + 2].position = vec3(cx + lx*c - lz*s, y, cz + lx*s + lz*c);
    batch->vertices[base + 2].uv       = vec2(region->u1, region->v1);
    batch->vertices[base + 2].color    = tint;

    /* corner 3: (-hw, +hh) */
    lx = -hw; lz = hh;
    batch->vertices[base + 3].position = vec3(cx + lx*c - lz*s, y, cz + lx*s + lz*c);
    batch->vertices[base + 3].uv       = vec2(region->u0, region->v1);
    batch->vertices[base + 3].color    = tint;

    batch->quad_count++;
}

void sprite_batch_draw_billboard(SpriteBatch *batch, Vec3 base_pos, Vec2 size,
                                 Vec3 cam_right, TextureRegion *region, Vec4 tint)
{
    uint32_t tex_id = region->texture->id;

    if (batch->current_texture != 0 && batch->current_texture != tex_id) {
        flush(batch);
    }
    batch->current_texture = tex_id;

    if (batch->quad_count >= SPRITE_BATCH_MAX_QUADS) {
        flush(batch);
    }

    float hw = size.x * 0.5f;
    float h  = size.y;

    /* Right offset: cam_right * half_width */
    float rx = cam_right.x * hw;
    float rz = cam_right.z * hw;

    /* Base position (feet on ground) */
    float bx = base_pos.x;
    float by = base_pos.y;
    float bz = base_pos.z;

    int base = batch->quad_count * 4;

    /* bottom-left */
    batch->vertices[base + 0].position = vec3(bx - rx, by,     bz - rz);
    batch->vertices[base + 0].uv       = vec2(region->u0, region->v1);
    batch->vertices[base + 0].color    = tint;

    /* bottom-right */
    batch->vertices[base + 1].position = vec3(bx + rx, by,     bz + rz);
    batch->vertices[base + 1].uv       = vec2(region->u1, region->v1);
    batch->vertices[base + 1].color    = tint;

    /* top-right */
    batch->vertices[base + 2].position = vec3(bx + rx, by + h, bz + rz);
    batch->vertices[base + 2].uv       = vec2(region->u1, region->v0);
    batch->vertices[base + 2].color    = tint;

    /* top-left */
    batch->vertices[base + 3].position = vec3(bx - rx, by + h, bz - rz);
    batch->vertices[base + 3].uv       = vec2(region->u0, region->v0);
    batch->vertices[base + 3].color    = tint;

    batch->quad_count++;
}
