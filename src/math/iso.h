#ifndef ISO_H
#define ISO_H

#include "math_types.h"

#define TILE_WIDTH       64
#define TILE_HEIGHT      32
#define TILE_WIDTH_HALF  (TILE_WIDTH  / 2)  /* 32 */
#define TILE_HEIGHT_HALF (TILE_HEIGHT / 2)  /* 16 */

/*
 * Convert world coordinates (x, z) to isometric screen coordinates.
 * screen_x = (x - z) * TILE_WIDTH_HALF
 * screen_y = (x + z) * TILE_HEIGHT_HALF
 */
static inline Vec2 world_to_screen(float world_x, float world_z) {
    Vec2 r;
    r.x = (world_x - world_z) * TILE_WIDTH_HALF;
    r.y = (world_x + world_z) * TILE_HEIGHT_HALF;
    return r;
}

/*
 * Convert isometric screen coordinates back to world coordinates (x, z).
 * Inverse of the above:
 *   x = (screen_x / TILE_WIDTH_HALF + screen_y / TILE_HEIGHT_HALF) / 2
 *   z = (screen_y / TILE_HEIGHT_HALF - screen_x / TILE_WIDTH_HALF) / 2
 */
static inline Vec2 screen_to_world(float screen_x, float screen_y) {
    Vec2 r;
    float sx = screen_x / (float)TILE_WIDTH_HALF;
    float sy = screen_y / (float)TILE_HEIGHT_HALF;
    r.x = (sx + sy) * 0.5f;
    r.y = (sy - sx) * 0.5f; /* y component represents world z */
    return r;
}

/*
 * Convert world coordinates to the tile grid index.
 * Floors to integer tile coordinates.
 */
static inline IVec2 world_to_tile(float world_x, float world_z) {
    IVec2 r;
    r.x = (int)(world_x >= 0.0f ? world_x : world_x - 1.0f);
    r.y = (int)(world_z >= 0.0f ? world_z : world_z - 1.0f);
    return r;
}

#endif /* ISO_H */
