#ifndef TD_DEBUG_DRAW_H
#define TD_DEBUG_DRAW_H

#include "sprite_batch.h"
#include "camera.h"
#include "../game/path.h"

typedef struct DebugDraw {
    SpriteBatch *batch;  /* borrows the sprite batch */
} DebugDraw;

void debug_draw_init(DebugDraw *dd, SpriteBatch *batch);

/* Draw a grid overlay on the map */
void debug_draw_grid(DebugDraw *dd, Camera *cam, int grid_w, int grid_h);

/* Draw path visualization */
void debug_draw_paths(DebugDraw *dd, Camera *cam, PathSet *paths);

/* Draw tower range circles (approximated as 16-sided polygon outline) */
void debug_draw_tower_range(DebugDraw *dd, Camera *cam, Vec2 center, float radius);

/* Draw a small colored marker at a world position */
void debug_draw_marker(DebugDraw *dd, Camera *cam, Vec2 position, Vec4 color);

#endif /* TD_DEBUG_DRAW_H */
