#include "debug_draw.h"
#include "../math/vec.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ------------------------------------------------------------------ */
/*  Y layer for debug overlays (above particles)                       */
/* ------------------------------------------------------------------ */

#define DEBUG_Y 0.06f

/* Line thickness for grid lines, path connections, range circles */
#define LINE_THICKNESS 0.02f

/* ------------------------------------------------------------------ */
/*  Init                                                               */
/* ------------------------------------------------------------------ */

void debug_draw_init(DebugDraw *dd, SpriteBatch *batch)
{
    dd->batch = batch;
}

/* ------------------------------------------------------------------ */
/*  Grid overlay                                                       */
/* ------------------------------------------------------------------ */

void debug_draw_grid(DebugDraw *dd, Camera *cam, int grid_w, int grid_h)
{
    (void)cam; /* camera transform already applied to sprite batch */

    Vec4 color = vec4(1.0f, 1.0f, 1.0f, 0.15f);

    /* vertical lines (along Z axis) */
    for (int x = 0; x <= grid_w; x++) {
        Vec3 pos = vec3((float)x, DEBUG_Y, (float)grid_h * 0.5f);
        Vec2 size = vec2(LINE_THICKNESS, (float)grid_h);
        sprite_batch_draw_quad(dd->batch, pos, size, color);
    }

    /* horizontal lines (along X axis) */
    for (int z = 0; z <= grid_h; z++) {
        Vec3 pos = vec3((float)grid_w * 0.5f, DEBUG_Y, (float)z);
        Vec2 size = vec2((float)grid_w, LINE_THICKNESS);
        sprite_batch_draw_quad(dd->batch, pos, size, color);
    }
}

/* ------------------------------------------------------------------ */
/*  Path visualization                                                 */
/* ------------------------------------------------------------------ */

/* Per-path colors so each path is visually distinct */
static const Vec4 s_path_colors[] = {
    { .x = 0.0f, .y = 1.0f, .z = 0.0f, .w = 0.7f },  /* green  */
    { .x = 0.0f, .y = 0.5f, .z = 1.0f, .w = 0.7f },  /* blue   */
    { .x = 1.0f, .y = 0.5f, .z = 0.0f, .w = 0.7f },  /* orange */
    { .x = 1.0f, .y = 0.0f, .z = 1.0f, .w = 0.7f },  /* purple */
};
#define PATH_COLOR_COUNT (int)(sizeof(s_path_colors) / sizeof(s_path_colors[0]))

void debug_draw_paths(DebugDraw *dd, Camera *cam, PathSet *paths)
{
    (void)cam;

    for (int pi = 0; pi < paths->path_count; pi++) {
        Path *path = &paths->paths[pi];
        Vec4 color = s_path_colors[pi % PATH_COLOR_COUNT];

        for (int wi = 0; wi < path->waypoint_count; wi++) {
            Vec2 wp = path->waypoints[wi];

            /* draw waypoint marker (small square) */
            Vec3 marker_pos = vec3(wp.x, DEBUG_Y, wp.y);
            Vec2 marker_size = vec2(0.12f, 0.12f);
            sprite_batch_draw_quad(dd->batch, marker_pos, marker_size, color);

            /* draw connecting line to next waypoint */
            if (wi + 1 < path->waypoint_count) {
                Vec2 next = path->waypoints[wi + 1];
                Vec2 diff = vec2_sub(next, wp);
                float len = vec2_length(diff);

                if (len > 0.001f) {
                    /* midpoint of the segment */
                    Vec2 mid = vec2_add(wp, vec2_scale(diff, 0.5f));
                    Vec3 line_pos = vec3(mid.x, DEBUG_Y, mid.y);

                    /* determine if line is more horizontal or vertical */
                    float abs_dx = fabsf(diff.x);
                    float abs_dy = fabsf(diff.y);
                    Vec2 line_size;

                    if (abs_dx >= abs_dy) {
                        /* mostly horizontal: width = length along X, height = thin */
                        line_size = vec2(len, LINE_THICKNESS);
                    } else {
                        /* mostly vertical: width = thin, height = length along Z */
                        line_size = vec2(LINE_THICKNESS, len);
                    }

                    /* slightly dimmer color for lines */
                    Vec4 line_color = vec4(color.x, color.y, color.z, color.w * 0.5f);
                    sprite_batch_draw_quad(dd->batch, line_pos, line_size, line_color);
                }
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Tower range circle (16-sided polygon outline)                      */
/* ------------------------------------------------------------------ */

#define RANGE_SEGMENTS 16

void debug_draw_tower_range(DebugDraw *dd, Camera *cam, Vec2 center, float radius)
{
    (void)cam;

    Vec4 color = vec4(1.0f, 1.0f, 1.0f, 0.25f);
    float angle_step = 2.0f * (float)M_PI / (float)RANGE_SEGMENTS;

    for (int i = 0; i < RANGE_SEGMENTS; i++) {
        float a0 = (float)i * angle_step;
        float a1 = (float)(i + 1) * angle_step;

        Vec2 p0 = vec2(center.x + cosf(a0) * radius, center.y + sinf(a0) * radius);
        Vec2 p1 = vec2(center.x + cosf(a1) * radius, center.y + sinf(a1) * radius);

        Vec2 diff = vec2_sub(p1, p0);
        float len = vec2_length(diff);
        Vec2 mid  = vec2_add(p0, vec2_scale(diff, 0.5f));

        Vec3 pos = vec3(mid.x, DEBUG_Y, mid.y);

        /* orient the thin quad along the segment direction */
        float abs_dx = fabsf(diff.x);
        float abs_dy = fabsf(diff.y);
        Vec2 size;

        if (abs_dx >= abs_dy) {
            size = vec2(len, LINE_THICKNESS);
        } else {
            size = vec2(LINE_THICKNESS, len);
        }

        sprite_batch_draw_quad(dd->batch, pos, size, color);
    }
}

/* ------------------------------------------------------------------ */
/*  World-space marker (small colored diamond)                         */
/* ------------------------------------------------------------------ */

void debug_draw_marker(DebugDraw *dd, Camera *cam, Vec2 position, Vec4 color)
{
    (void)cam;

    /* Draw as a small square (diamond appearance at isometric camera) */
    Vec3 pos = vec3(position.x, DEBUG_Y, position.y);
    Vec2 size = vec2(0.08f, 0.08f);

    sprite_batch_draw_quad(dd->batch, pos, size, color);
}
