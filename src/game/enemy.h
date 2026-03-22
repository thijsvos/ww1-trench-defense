#ifndef TD_ENEMY_H
#define TD_ENEMY_H

#include <stdbool.h>
#include "../math/math_types.h"
#include "path.h"

#define MAX_ENEMIES 256

typedef enum EnemyType {
    ENEMY_INFANTRY = 0,
    ENEMY_CAVALRY,
    ENEMY_STORMTROOPER,
    ENEMY_TANK,
    ENEMY_MEDIC,
    ENEMY_OFFICER,
    ENEMY_TUNNEL_SAPPER,
    ENEMY_ARMORED_CAR,
    ENEMY_TYPE_COUNT
} EnemyType;

typedef enum ArmorType {
    ARMOR_UNARMORED = 0,
    ARMOR_LIGHT,
    ARMOR_MEDIUM,
    ARMOR_HEAVY,
    ARMOR_TYPE_COUNT
} ArmorType;

typedef struct EnemyDef {
    const char *name;
    float max_hp;
    float speed;       /* tiles per second (1.0 = base speed) */
    ArmorType armor;
    int reward;        /* gold on kill */
    int lives_cost;    /* lives lost if leaked */
    float evasion;     /* 0.0 - 1.0, chance to dodge */
} EnemyDef;

typedef struct Enemy {
    bool active;
    EnemyType type;
    float hp;
    float max_hp;
    float speed;        /* current speed (can be modified by slow effects) */
    float base_speed;
    ArmorType armor;
    int path_index;     /* which path this enemy follows */
    float progress;     /* 0.0 = start, 1.0 = end of path */
    Vec2 position;      /* world position */
    float slow_timer;   /* remaining slow duration */
    float slow_factor;  /* 0.0-1.0, multiplied with speed */
    float burn_timer;   /* fire damage over time */
    float burn_dps;
    float gas_timer;    /* gas damage over time */
    float gas_dps;
    bool burrowed;      /* tunnel sapper underground */
    float burrow_timer;
    int reward;
    int lives_cost;
    float evasion;
    float anim_timer;   /* animation frame timer */
} Enemy;

typedef struct EnemyManager {
    Enemy enemies[MAX_ENEMIES];
    int count;          /* number of active enemies (dense array) */
} EnemyManager;

/* Get the static definition for an enemy type */
const EnemyDef *enemy_get_def(EnemyType type);

void enemy_manager_init(EnemyManager *em);

/* Spawn a new enemy on a given path. Returns pointer to spawned enemy or NULL if full.
   hp_scale is a multiplier for enemy HP (for wave scaling). */
Enemy *enemy_spawn(EnemyManager *em, EnemyType type, int path_index, float hp_scale);

/* Update all enemies: move along paths, apply DoT effects, etc.
   Returns number of enemies that reached the end (leaked). */
int enemy_update(EnemyManager *em, PathSet *paths, float dt);

/* Remove a dead enemy (swap-with-last). */
void enemy_remove(EnemyManager *em, int index);

/* Apply damage to an enemy. Returns true if the enemy died.
   damage_type is used with armor for multiplier lookup. */
/* (damage_type is defined in tower.h, so just use int here) */
bool enemy_take_damage(Enemy *enemy, float damage, int damage_type);

/* Apply slow effect */
void enemy_apply_slow(Enemy *enemy, float factor, float duration);

/* Apply burn effect */
void enemy_apply_burn(Enemy *enemy, float dps, float duration);

/* Apply gas effect */
void enemy_apply_gas(Enemy *enemy, float dps, float duration);

/* Armor multiplier table: damage_multiplier[damage_type][armor_type] */
float enemy_get_armor_multiplier(int damage_type, ArmorType armor);

#endif /* TD_ENEMY_H */
