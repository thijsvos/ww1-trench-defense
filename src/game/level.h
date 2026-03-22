#ifndef TD_LEVEL_H
#define TD_LEVEL_H

#include <stdbool.h>
#include "map.h"
#include "path.h"

#define MAX_LEVEL_NAME 64

typedef struct LevelDef {
    char name[MAX_LEVEL_NAME];
    int map_width;
    int map_height;
    int starting_gold;
    int starting_lives;
    int num_waves;
    TileType tile_data[MAP_MAX_HEIGHT][MAP_MAX_WIDTH];
    PathSet paths;
} LevelDef;

bool level_load(LevelDef *level, const char *filepath);
void level_apply(LevelDef *level, Map *map, PathSet *paths);

#endif /* TD_LEVEL_H */
