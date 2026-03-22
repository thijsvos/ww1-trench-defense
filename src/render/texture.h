#ifndef TD_TEXTURE_H
#define TD_TEXTURE_H

#include <stdbool.h>
#include <stdint.h>

typedef struct Texture {
    uint32_t id;
    int width;
    int height;
    int channels;
} Texture;

bool texture_load(Texture *tex, const char *filepath);
void texture_bind(Texture *tex, uint32_t slot);
void texture_destroy(Texture *tex);

/* Sub-region of a texture atlas */
typedef struct TextureRegion {
    Texture *texture;
    float u0, v0;  /* top-left UV */
    float u1, v1;  /* bottom-right UV */
} TextureRegion;

#endif /* TD_TEXTURE_H */
