#include <glad/gl.h>

#include "renderer.h"
#include "../math/mat4.h"
#include "../core/log.h"

/* ------------------------------------------------------------------ */
/* Ground tile colours (checkerboard olive/mud)                        */
/* ------------------------------------------------------------------ */

/* Dark olive  #3C4A2E */
static const Vec4 COLOR_TILE_A = { .x = 0.235f, .y = 0.290f, .z = 0.180f, .w = 1.0f };
/* Slightly lighter olive #4A5A38 */
static const Vec4 COLOR_TILE_B = { .x = 0.290f, .y = 0.353f, .z = 0.220f, .w = 1.0f };

/* Clear colour: dark olive #3C4A2E */
static const float CLEAR_R = 0.235f;
static const float CLEAR_G = 0.290f;
static const float CLEAR_B = 0.180f;

/* ------------------------------------------------------------------ */
/* Unit quad on the XZ plane (Y = 0)                                   */
/* ------------------------------------------------------------------ */

static const float quad_vertices[] = {
    /* x     y     z  */
    0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f
};

static const uint32_t quad_indices[] = {
    0, 1, 2,
    0, 2, 3
};

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

bool renderer_init(Renderer *r)
{
    /* Load tile shader from files. */
    if (!shader_load_from_file(&r->tile_shader,
                               "assets/shaders/tile.vert",
                               "assets/shaders/tile.frag")) {
        LOG_ERROR("Failed to load tile shader");
        return false;
    }

    /* Create VAO / VBO / EBO for the unit quad. */
    glGenVertexArrays(1, &r->tile_vao);
    glGenBuffers(1, &r->tile_vbo);
    glGenBuffers(1, &r->tile_ebo);

    glBindVertexArray(r->tile_vao);

    glBindBuffer(GL_ARRAY_BUFFER, r->tile_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->tile_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices), quad_indices, GL_STATIC_DRAW);

    /* layout(location = 0) in vec3 aPos */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    /* Global GL state. */
    glEnable(GL_DEPTH_TEST);
    glClearColor(CLEAR_R, CLEAR_G, CLEAR_B, 1.0f);

    LOG_INFO("Renderer initialised");
    return true;
}

void renderer_shutdown(Renderer *r)
{
    glDeleteVertexArrays(1, &r->tile_vao);
    glDeleteBuffers(1, &r->tile_vbo);
    glDeleteBuffers(1, &r->tile_ebo);
    shader_destroy(&r->tile_shader);

    LOG_INFO("Renderer shut down");
}

void renderer_begin_frame(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void renderer_end_frame(void)
{
    /* Hook point for future post-processing / swap logic. */
}

void renderer_draw_ground(Renderer *r, Camera *cam, int grid_w, int grid_h)
{
    shader_use(&r->tile_shader);
    glBindVertexArray(r->tile_vao);

    for (int z = 0; z < grid_h; z++) {
        for (int x = 0; x < grid_w; x++) {
            /* Model matrix: translate the unit quad to tile position. */
            Mat4 model = mat4_translate(vec3((float)x, 0.0f, (float)z));
            Mat4 mvp   = mat4_multiply(cam->view_projection, model);

            shader_set_mat4(&r->tile_shader, "uMVP", &mvp);

            /* Checkerboard colour. */
            Vec4 color = ((x + z) % 2 == 0) ? COLOR_TILE_A : COLOR_TILE_B;
            shader_set_vec4(&r->tile_shader, "uColor", color);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }

    glBindVertexArray(0);
}
