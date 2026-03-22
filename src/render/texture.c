#include <glad/gl.h>
#include <stb_image.h>

#include "texture.h"
#include "../core/log.h"

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

bool texture_load(Texture *tex, const char *filepath)
{
    stbi_set_flip_vertically_on_load(1);

    int width, height, channels;
    unsigned char *data = stbi_load(filepath, &width, &height, &channels, 0);
    if (!data) {
        LOG_ERROR("Failed to load texture: %s (%s)", filepath, stbi_failure_reason());
        return false;
    }

    GLenum internal_format;
    GLenum data_format;

    if (channels == 4) {
        internal_format = GL_RGBA8;
        data_format     = GL_RGBA;
    } else if (channels == 3) {
        internal_format = GL_RGB8;
        data_format     = GL_RGB;
    } else if (channels == 1) {
        internal_format = GL_R8;
        data_format     = GL_RED;
    } else {
        LOG_ERROR("Unsupported channel count %d in texture: %s", channels, filepath);
        stbi_image_free(data);
        return false;
    }

    uint32_t id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    /* Pixel-art filtering: nearest neighbour */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    /* Clamp to edge to avoid bleeding at atlas borders */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, (GLint)internal_format,
                 width, height, 0, data_format, GL_UNSIGNED_BYTE, data);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    tex->id       = id;
    tex->width    = width;
    tex->height   = height;
    tex->channels = channels;

    LOG_INFO("Loaded texture: %s (%dx%d, %d channels)", filepath, width, height, channels);
    return true;
}

void texture_bind(Texture *tex, uint32_t slot)
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, tex->id);
}

void texture_destroy(Texture *tex)
{
    if (tex->id) {
        glDeleteTextures(1, &tex->id);
        tex->id = 0;
    }
}
