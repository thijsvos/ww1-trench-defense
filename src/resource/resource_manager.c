#include <string.h>

#include "resource_manager.h"
#include "../core/log.h"

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

void resource_manager_init(ResourceManager *rm)
{
    memset(rm, 0, sizeof(ResourceManager));
    LOG_INFO("Resource manager initialised");
}

void resource_manager_shutdown(ResourceManager *rm)
{
    for (int i = 0; i < rm->texture_count; i++) {
        texture_destroy(&rm->textures[i]);
    }
    for (int i = 0; i < rm->shader_count; i++) {
        shader_destroy(&rm->shaders[i]);
    }

    memset(rm, 0, sizeof(ResourceManager));
    LOG_INFO("Resource manager shut down");
}

Texture *resource_load_texture(ResourceManager *rm, const char *name, const char *filepath)
{
    /* Check if already loaded */
    Texture *existing = resource_get_texture(rm, name);
    if (existing) {
        LOG_WARN("Texture '%s' already loaded, returning existing", name);
        return existing;
    }

    if (rm->texture_count >= MAX_TEXTURES) {
        LOG_ERROR("Texture storage full (%d/%d), cannot load '%s'",
                  rm->texture_count, MAX_TEXTURES, name);
        return NULL;
    }

    int idx = rm->texture_count;

    if (!texture_load(&rm->textures[idx], filepath)) {
        LOG_ERROR("Failed to load texture '%s' from '%s'", name, filepath);
        return NULL;
    }

    strncpy(rm->texture_names[idx], name, sizeof(rm->texture_names[idx]) - 1);
    rm->texture_names[idx][sizeof(rm->texture_names[idx]) - 1] = '\0';
    rm->texture_count++;

    LOG_INFO("Resource manager: loaded texture '%s'", name);
    return &rm->textures[idx];
}

Texture *resource_get_texture(ResourceManager *rm, const char *name)
{
    for (int i = 0; i < rm->texture_count; i++) {
        if (strcmp(rm->texture_names[i], name) == 0) {
            return &rm->textures[i];
        }
    }
    return NULL;
}

Shader *resource_load_shader(ResourceManager *rm, const char *name,
                             const char *vert_path, const char *frag_path)
{
    /* Check if already loaded */
    Shader *existing = resource_get_shader(rm, name);
    if (existing) {
        LOG_WARN("Shader '%s' already loaded, returning existing", name);
        return existing;
    }

    if (rm->shader_count >= MAX_SHADERS) {
        LOG_ERROR("Shader storage full (%d/%d), cannot load '%s'",
                  rm->shader_count, MAX_SHADERS, name);
        return NULL;
    }

    int idx = rm->shader_count;

    if (!shader_load_from_file(&rm->shaders[idx], vert_path, frag_path)) {
        LOG_ERROR("Failed to load shader '%s' from '%s' / '%s'", name, vert_path, frag_path);
        return NULL;
    }

    strncpy(rm->shader_names[idx], name, sizeof(rm->shader_names[idx]) - 1);
    rm->shader_names[idx][sizeof(rm->shader_names[idx]) - 1] = '\0';
    rm->shader_count++;

    LOG_INFO("Resource manager: loaded shader '%s'", name);
    return &rm->shaders[idx];
}

Shader *resource_get_shader(ResourceManager *rm, const char *name)
{
    for (int i = 0; i < rm->shader_count; i++) {
        if (strcmp(rm->shader_names[i], name) == 0) {
            return &rm->shaders[i];
        }
    }
    return NULL;
}
