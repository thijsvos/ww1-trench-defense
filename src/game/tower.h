#ifndef TD_TOWER_H
#define TD_TOWER_H

#include <stdbool.h>
#include "../math/math_types.h"

/* Forward declarations */
typedef struct EnemyManager EnemyManager;

#define MAX_TOWERS 128

typedef enum DamageType {
    DAMAGE_BULLET = 0,
    DAMAGE_PIERCING,
    DAMAGE_EXPLOSIVE,
    DAMAGE_HEAVY_EXPLOSIVE,
    DAMAGE_CHEMICAL,
    DAMAGE_FIRE,
    DAMAGE_TYPE_COUNT
} DamageType;

typedef enum TowerType {
    TOWER_MACHINE_GUN = 0,
    TOWER_MORTAR,
    TOWER_SNIPER,
    TOWER_BARBED_WIRE,
    TOWER_ARTILLERY,
    TOWER_GAS,
    TOWER_FLAMETHROWER,
    TOWER_OBSERVATION,
    TOWER_TYPE_COUNT
} TowerType;

typedef enum TargetPriority {
    TARGET_FIRST = 0,   /* furthest along path */
    TARGET_LAST,        /* least progress */
    TARGET_STRONGEST,   /* most HP */
    TARGET_WEAKEST,     /* least HP */
    TARGET_NEAREST,     /* closest to tower */
} TargetPriority;

typedef struct TowerDef {
    const char *name;
    int cost;
    float range;
    float fire_rate;     /* shots per second */
    float damage;
    DamageType damage_type;
    float splash_radius; /* 0 = single target */
    float min_range;     /* for mortars/artillery */
    /* Upgrade costs for 3 tiers (tier 0 = base) */
    int upgrade_cost[3];
} TowerDef;

typedef struct Tower {
    bool active;
    TowerType type;
    int upgrade_level;   /* 0, 1, 2 */
    IVec2 tile;          /* grid position */
    Vec2 position;       /* world position (center of tile) */
    float range;
    float fire_rate;
    float damage;
    DamageType damage_type;
    float splash_radius;
    float min_range;
    float fire_cooldown; /* counts down to 0, then can fire */
    TargetPriority priority;
    int target_index;    /* index into enemy array, -1 = no target */
    int total_invested;  /* for sell value calculation */

    /* Observation balloon buff state */
    float buff_range;       /* observation balloon buff radius */
    float buff_damage_mult; /* +10% per observation tower in range */
    int buff_range_bonus;   /* +1 per observation tower in range */

    /* Visual facing angle (radians, 0 = right, rotates toward target) */
    float facing_angle;

    /* Barbed wire special */
    float slow_factor;
    float bleed_dps;
} Tower;

typedef struct TowerManager {
    Tower towers[MAX_TOWERS];
    int count;
} TowerManager;

/* Get the static definition for a tower type */
const TowerDef *tower_get_def(TowerType type);

void tower_manager_init(TowerManager *tm);

/* Place a tower. Returns pointer to placed tower or NULL if full. */
Tower *tower_place(TowerManager *tm, TowerType type, int tile_x, int tile_y);

/* Update all towers: cooldowns, targeting, barbed wire aoe, observation buffs.
   Does NOT spawn projectiles -- call tower_try_fire per tower after this. */
void tower_update(TowerManager *tm, EnemyManager *em, float dt);

/* Check if tower is ready to fire. Returns true and resets cooldown. */
bool tower_try_fire(Tower *tower);

/* Upgrade a tower to the next tier. Returns true if successful. */
bool tower_upgrade(Tower *tower);

/* Remove a tower (sell). Decrements count via swap-with-last. */
void tower_remove(TowerManager *tm, int index);

/* Find best target for a tower based on its priority. Returns enemy index or -1. */
int tower_find_target(Tower *tower, EnemyManager *em);

#endif /* TD_TOWER_H */
