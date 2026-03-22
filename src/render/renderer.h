#ifndef TD_RENDERER_H
#define TD_RENDERER_H

#include <stdbool.h>
#include <stdint.h>
#include "shader.h"
#include "camera.h"

typedef struct Renderer {
    Shader tile_shader;
    uint32_t tile_vao, tile_vbo, tile_ebo;
    /* Simple colored ground plane for Phase 1 */
} Renderer;

bool renderer_init(Renderer *r);
void renderer_shutdown(Renderer *r);
void renderer_begin_frame(void);
void renderer_end_frame(void);
void renderer_draw_ground(Renderer *r, Camera *cam, int grid_w, int grid_h);

#endif /* TD_RENDERER_H */
