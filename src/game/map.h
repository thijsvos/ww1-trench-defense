#ifndef TD_MAP_H
#define TD_MAP_H

#include <stdbool.h>

#define MAP_MAX_WIDTH  32
#define MAP_MAX_HEIGHT 32

typedef enum TileType {
    TILE_GRASS = 0,
    TILE_MUD,
    TILE_TRENCH,     /* enemy path */
    TILE_STONE,
    TILE_WATER,
    TILE_BUILDABLE,  /* where towers can be placed */
    TILE_COUNT
} TileType;

typedef struct Tile {
    TileType type;
    bool occupied;     /* tower placed here */
    bool highlighted;  /* mouse hover */
} Tile;

typedef struct Map {
    Tile tiles[MAP_MAX_HEIGHT][MAP_MAX_WIDTH];
    int width;
    int height;
} Map;

void map_init(Map *map, int width, int height);
Tile *map_get_tile(Map *map, int x, int y);
bool map_is_buildable(Map *map, int x, int y);
void map_set_tile(Map *map, int x, int y, TileType type);

#endif /* TD_MAP_H */
