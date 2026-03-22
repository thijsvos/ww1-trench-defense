#ifndef TD_ATLAS_GEN_H
#define TD_ATLAS_GEN_H

#include "texture.h"
#include "../game/map.h"
#include "../game/tower.h"
#include "../game/enemy.h"

#define ATLAS_SIZE 512

/*
 * Trench neighbor bitmask for auto-tiling:
 * bit 0 = east, bit 1 = south, bit 2 = west, bit 3 = north
 */
#define TRENCH_E  1
#define TRENCH_S  2
#define TRENCH_W  4
#define TRENCH_N  8
#define TRENCH_VARIANT_COUNT 16
#define ENEMY_ANIM_FRAMES 4

/* Region indices for quick lookup */
typedef struct GameAtlas {
    Texture texture;
    TextureRegion tiles[TILE_COUNT];
    TextureRegion trench_variants[TRENCH_VARIANT_COUNT];
    TextureRegion towers[TOWER_TYPE_COUNT];
    TextureRegion enemies[ENEMY_TYPE_COUNT];             /* static (frame 0) */
    TextureRegion enemy_anim[ENEMY_TYPE_COUNT][ENEMY_ANIM_FRAMES]; /* walk cycle */
    TextureRegion projectiles[DAMAGE_TYPE_COUNT];
    TextureRegion white;
    TextureRegion soft_circle; /* 16x16 radial gradient circle for gas/smoke */
    TextureRegion fort;        /* 64x64 decorative WW1 fort */
} GameAtlas;

/* Compute the trench neighbor mask for a tile */
int atlas_trench_mask(Map *map, int x, int y);

/* Generate and upload the full atlas. Returns false on failure. */
bool atlas_generate(GameAtlas *atlas);
void atlas_destroy(GameAtlas *atlas);

#endif /* TD_ATLAS_GEN_H */
