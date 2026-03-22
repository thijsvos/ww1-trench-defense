#include "map.h"
#include <string.h>

void map_init(Map *map, int width, int height)
{
    memset(map, 0, sizeof(*map));

    if (width > MAP_MAX_WIDTH)   width = MAP_MAX_WIDTH;
    if (height > MAP_MAX_HEIGHT) height = MAP_MAX_HEIGHT;
    if (width < 1)  width = 1;
    if (height < 1) height = 1;

    map->width  = width;
    map->height = height;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            map->tiles[y][x].type        = TILE_GRASS;
            map->tiles[y][x].occupied    = false;
            map->tiles[y][x].highlighted = false;
        }
    }
}

Tile *map_get_tile(Map *map, int x, int y)
{
    if (x < 0 || x >= map->width || y < 0 || y >= map->height) {
        return NULL;
    }
    return &map->tiles[y][x];
}

bool map_is_buildable(Map *map, int x, int y)
{
    Tile *tile = map_get_tile(map, x, y);
    if (!tile) {
        return false;
    }
    return tile->type == TILE_BUILDABLE && !tile->occupied;
}

void map_set_tile(Map *map, int x, int y, TileType type)
{
    Tile *tile = map_get_tile(map, x, y);
    if (tile) {
        tile->type = type;
    }
}
