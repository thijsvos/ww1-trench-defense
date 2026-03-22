#ifndef TD_PROJECTILE_H
#define TD_PROJECTILE_H

#include <stdbool.h>
#include "../math/math_types.h"
#include "tower.h"

/* Forward declarations */
typedef struct EnemyManager EnemyManager;

#define MAX_PROJECTILES 512

typedef struct Projectile {
    bool active;
    Vec2 position;
    Vec2 velocity;      /* direction * speed */
    float speed;
    float damage;
    DamageType damage_type;
    float splash_radius;
    int target_index;   /* enemy index, for homing */
    Vec2 target_pos;    /* last known target position (for non-homing fallback) */
    float lifetime;     /* max time before despawn */
    /* Arc trajectory (mortar/artillery) */
    bool arcing;        /* true = parabolic arc, false = straight line */
    Vec2 arc_origin;    /* launch position */
    Vec2 arc_target;    /* target position (fixed at launch) */
    float arc_progress; /* 0.0 = just launched, 1.0 = arrived */
    float arc_duration; /* total flight time */
    float arc_height;   /* peak height of parabola */
    float height;       /* current Y height (for rendering) */
} Projectile;

typedef struct ProjectileManager {
    Projectile projectiles[MAX_PROJECTILES];
    int count;
} ProjectileManager;

void projectile_manager_init(ProjectileManager *pm);

/* Spawn a projectile from tower position toward target. */
Projectile *projectile_spawn(ProjectileManager *pm, Vec2 origin, int target_index,
                              Vec2 target_pos, float speed, float damage,
                              DamageType damage_type, float splash_radius);

/* Update all projectiles: move, check collision with enemies.
   Returns number of enemies killed this frame via out parameter.
   gold_earned out parameter accumulates reward gold. */
int projectile_update(ProjectileManager *pm, EnemyManager *em, float dt,
                       int *gold_earned);

void projectile_remove(ProjectileManager *pm, int index);

#endif /* TD_PROJECTILE_H */
