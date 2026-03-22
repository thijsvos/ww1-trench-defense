#include "level.h"
#include "../core/log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static TileType char_to_tile(char c)
{
    switch (c) {
        case '.': return TILE_GRASS;
        case 'm': return TILE_MUD;
        case 't': return TILE_TRENCH;
        case 's': return TILE_STONE;
        case 'w': return TILE_WATER;
        case 'b': return TILE_BUILDABLE;
        default:  return TILE_GRASS;
    }
}

bool level_load(LevelDef *level, const char *filepath)
{
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        LOG_ERROR("Failed to open level file: %s", filepath);
        return false;
    }

    memset(level, 0, sizeof(*level));
    path_init(&level->paths);

    char line[256];
    bool reading_map  = false;
    bool reading_path = false;
    int map_row       = 0;
    int path_index    = -1;

    while (fgets(line, sizeof(line), fp)) {
        /* Strip trailing newline / carriage return */
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        /* Skip empty lines when not inside a section */
        if (len == 0 && !reading_map && !reading_path) {
            continue;
        }

        if (reading_map) {
            if (len == 0 || map_row >= level->map_height) {
                reading_map = false;
                /* Fall through to parse this line as a new command */
                if (len == 0) continue;
            } else {
                for (int x = 0; x < level->map_width && x < (int)len; x++) {
                    level->tile_data[map_row][x] = char_to_tile(line[x]);
                }
                map_row++;
                continue;
            }
        }

        if (reading_path) {
            if (strcmp(line, "END") == 0) {
                reading_path = false;
                continue;
            }
            int tx, ty;
            if (sscanf(line, "%d %d", &tx, &ty) == 2) {
                if (path_index >= 0 && path_index < MAX_PATHS) {
                    path_add_waypoint(
                        &level->paths.paths[path_index], tx, ty
                    );
                }
            }
            continue;
        }

        /* Parse header commands */
        if (strncmp(line, "NAME ", 5) == 0) {
            strncpy(level->name, line + 5, MAX_LEVEL_NAME - 1);
            level->name[MAX_LEVEL_NAME - 1] = '\0';
        } else if (strncmp(line, "SIZE ", 5) == 0) {
            sscanf(line + 5, "%d %d", &level->map_width, &level->map_height);
            if (level->map_width > MAP_MAX_WIDTH)   level->map_width  = MAP_MAX_WIDTH;
            if (level->map_height > MAP_MAX_HEIGHT)  level->map_height = MAP_MAX_HEIGHT;
        } else if (strncmp(line, "GOLD ", 5) == 0) {
            level->starting_gold = atoi(line + 5);
        } else if (strncmp(line, "LIVES ", 6) == 0) {
            level->starting_lives = atoi(line + 6);
        } else if (strncmp(line, "WAVES ", 6) == 0) {
            level->num_waves = atoi(line + 6);
        } else if (strcmp(line, "MAP") == 0) {
            reading_map = true;
            map_row     = 0;
        } else if (strcmp(line, "PATH") == 0) {
            path_index = level->paths.path_count;
            if (path_index < MAX_PATHS) {
                level->paths.paths[path_index].waypoint_count = 0;
                level->paths.path_count++;
                reading_path = true;
            }
        }
    }

    fclose(fp);

    LOG_INFO("Loaded level \"%s\" (%dx%d, %d paths)",
             level->name, level->map_width, level->map_height,
             level->paths.path_count);

    return true;
}

void level_apply(LevelDef *level, Map *map, PathSet *paths)
{
    map_init(map, level->map_width, level->map_height);

    for (int y = 0; y < level->map_height; y++) {
        for (int x = 0; x < level->map_width; x++) {
            map_set_tile(map, x, y, level->tile_data[y][x]);
        }
    }

    memcpy(paths, &level->paths, sizeof(PathSet));
}
