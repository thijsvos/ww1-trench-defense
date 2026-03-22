#ifndef TD_RESOURCE_MANAGER_H
#define TD_RESOURCE_MANAGER_H

#include "../render/texture.h"
#include "../render/shader.h"

#define MAX_TEXTURES 32
#define MAX_SHADERS  16

typedef struct ResourceManager {
    Texture textures[MAX_TEXTURES];
    char texture_names[MAX_TEXTURES][128];
    int texture_count;

    Shader shaders[MAX_SHADERS];
    char shader_names[MAX_SHADERS][128];
    int shader_count;
} ResourceManager;

void resource_manager_init(ResourceManager *rm);
void resource_manager_shutdown(ResourceManager *rm);
Texture *resource_load_texture(ResourceManager *rm, const char *name, const char *filepath);
Texture *resource_get_texture(ResourceManager *rm, const char *name);
Shader *resource_load_shader(ResourceManager *rm, const char *name,
                             const char *vert_path, const char *frag_path);
Shader *resource_get_shader(ResourceManager *rm, const char *name);

#endif /* TD_RESOURCE_MANAGER_H */
