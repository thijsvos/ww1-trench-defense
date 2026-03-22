#ifndef TD_SPRITE_BATCH_H
#define TD_SPRITE_BATCH_H

#include <stdbool.h>
#include <stdint.h>
#include "../math/math_types.h"
#include "shader.h"
#include "texture.h"

#define SPRITE_BATCH_MAX_QUADS    4096
#define SPRITE_BATCH_MAX_VERTICES (SPRITE_BATCH_MAX_QUADS * 4)
#define SPRITE_BATCH_MAX_INDICES  (SPRITE_BATCH_MAX_QUADS * 6)

typedef struct SpriteVertex {
    Vec3 position;
    Vec2 uv;
    Vec4 color;
} SpriteVertex;

typedef struct SpriteBatch {
    uint32_t vao, vbo, ebo;
    Shader shader;
    SpriteVertex vertices[SPRITE_BATCH_MAX_VERTICES];
    int quad_count;
    uint32_t current_texture;
} SpriteBatch;

bool sprite_batch_init(SpriteBatch *batch);
void sprite_batch_destroy(SpriteBatch *batch);
void sprite_batch_begin(SpriteBatch *batch, Mat4 *view_projection);
void sprite_batch_end(SpriteBatch *batch);  /* flush remaining quads */

/* Draw a colored quad on the XZ plane (for tiles) */
void sprite_batch_draw_quad(SpriteBatch *batch, Vec3 position, Vec2 size, Vec4 color);

/* Draw a textured quad on the XZ plane */
void sprite_batch_draw_textured(SpriteBatch *batch, Vec3 position, Vec2 size,
                                TextureRegion *region, Vec4 tint);

/* Draw a textured quad rotated around its center on the XZ plane (angle in radians) */
void sprite_batch_draw_rotated(SpriteBatch *batch, Vec3 center, Vec2 size,
                               float angle, TextureRegion *region, Vec4 tint);

/* Draw a textured billboard quad (stands upright, faces camera).
   base_pos = feet/ground position, size = (width, height) in world units,
   cam_right = camera right vector on XZ plane. */
void sprite_batch_draw_billboard(SpriteBatch *batch, Vec3 base_pos, Vec2 size,
                                 Vec3 cam_right, TextureRegion *region, Vec4 tint);

#endif /* TD_SPRITE_BATCH_H */
