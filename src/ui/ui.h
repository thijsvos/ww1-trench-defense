#ifndef TD_UI_H
#define TD_UI_H

#include <stdbool.h>
#include <stdint.h>
#include "../math/math_types.h"
#include "../render/shader.h"

#define UI_MAX_QUADS 8192

typedef struct UIVertex {
    Vec2 position;    /* screen space */
    Vec2 uv;
    Vec4 color;
} UIVertex;

typedef struct UIContext {
    /* Rendering */
    uint32_t vao, vbo, ebo;
    Shader shader;
    UIVertex vertices[UI_MAX_QUADS * 4];
    int quad_count;

    /* Input state (copied each frame) */
    float mouse_x, mouse_y;
    bool mouse_down;
    bool mouse_pressed;  /* just this frame */

    /* Screen dimensions */
    int screen_w, screen_h;

    /* Hot/Active widget tracking for interactive elements */
    uint32_t hot_id;     /* mouse is over this widget */
    uint32_t active_id;  /* mouse is pressing this widget */
} UIContext;

bool ui_init(UIContext *ctx);
void ui_destroy(UIContext *ctx);
void ui_begin(UIContext *ctx, int screen_w, int screen_h,
              float mouse_x, float mouse_y, bool mouse_down, bool mouse_pressed);
void ui_end(UIContext *ctx);  /* flush all quads */

/* Internal: add a colored quad in screen space */
void ui_draw_rect(UIContext *ctx, float x, float y, float w, float h, Vec4 color);

#endif /* TD_UI_H */
