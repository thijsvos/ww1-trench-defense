#include "projectile.h"
#include "enemy.h"
#include "../math/vec.h"
#include <string.h>

/* Hit radius: projectile counts as a hit when this close to target */
#define HIT_RADIUS 0.3f

/* Default lifetime (seconds) before a projectile despawns */
#define DEFAULT_LIFETIME 5.0f

/* ------------------------------------------------------------------ */
/*  Manager init                                                      */
/* ------------------------------------------------------------------ */

void projectile_manager_init(ProjectileManager *pm)
{
    memset(pm, 0, sizeof(*pm));
}

/* ------------------------------------------------------------------ */
/*  Spawn                                                             */
/* ------------------------------------------------------------------ */

Projectile *projectile_spawn(ProjectileManager *pm, Vec2 origin, int target_index,
                              Vec2 target_pos, float speed, float damage,
                              DamageType damage_type, float splash_radius)
{
    if (pm->count >= MAX_PROJECTILES) {
        return NULL;
    }

    Projectile *p = &pm->projectiles[pm->count];
    memset(p, 0, sizeof(*p));

    p->active       = true;
    p->position     = origin;
    p->speed        = speed;
    p->damage       = damage;
    p->damage_type  = damage_type;
    p->splash_radius = splash_radius;
    p->target_index = target_index;
    p->target_pos   = target_pos;
    p->lifetime     = DEFAULT_LIFETIME;

    /* Mortar and artillery use arcing trajectory */
    if (damage_type == DAMAGE_EXPLOSIVE || damage_type == DAMAGE_HEAVY_EXPLOSIVE) {
        float dist = vec2_distance(origin, target_pos);
        p->arcing       = true;
        p->arc_origin   = origin;
        p->arc_target   = target_pos;
        p->arc_progress = 0.0f;
        p->arc_duration = dist / speed * 2.5f; /* slower arc flight */
        p->arc_height   = 1.0f + dist * 0.15f; /* higher arc for longer shots */
        p->height       = 0.0f;
        /* Still set velocity for fallback */
        Vec2 dir = vec2_normalize(vec2_sub(target_pos, origin));
        p->velocity = vec2_scale(dir, speed);
    } else {
        p->arcing = false;
        p->height = 0.0f;
        /* Initial velocity toward target */
        Vec2 dir = vec2_normalize(vec2_sub(target_pos, origin));
        p->velocity = vec2_scale(dir, speed);
    }

    pm->count++;
    return p;
}

/* ------------------------------------------------------------------ */
/*  Remove (swap with last)                                           */
/* ------------------------------------------------------------------ */

void projectile_remove(ProjectileManager *pm, int index)
{
    if (index < 0 || index >= pm->count) return;

    pm->count--;
    if (index < pm->count) {
        pm->projectiles[index] = pm->projectiles[pm->count];
    }
    memset(&pm->projectiles[pm->count], 0, sizeof(Projectile));
}

/* ------------------------------------------------------------------ */
/*  Deal damage -- single target or splash                            */
/* ------------------------------------------------------------------ */

static int deal_splash_damage(Projectile *p, EnemyManager *em,
                               Vec2 hit_pos, int *gold_earned)
{
    int kills = 0;

    for (int i = 0; i < em->count; i++) {
        Enemy *e = &em->enemies[i];
        if (!e->active) continue;

        float dist = vec2_distance(hit_pos, e->position);
        if (dist > p->splash_radius) continue;

        /* Apply status effects for special damage types */
        if (p->damage_type == DAMAGE_CHEMICAL) {
            enemy_apply_gas(e, p->damage * 0.5f, 3.0f);
        } else if (p->damage_type == DAMAGE_FIRE) {
            enemy_apply_burn(e, p->damage * 0.4f, 2.0f);
        }

        bool died = enemy_take_damage(e, p->damage, p->damage_type);
        if (died) {
            *gold_earned += e->reward;
            kills++;
            /* Note: don't remove the enemy here -- enemy_update handles that.
               The enemy is marked inactive by enemy_take_damage when hp <= 0. */
        }
    }

    return kills;
}

static int deal_single_damage(Projectile *p, Enemy *target, int *gold_earned)
{
    /* Apply status effects for special damage types */
    if (p->damage_type == DAMAGE_CHEMICAL) {
        enemy_apply_gas(target, p->damage * 0.5f, 3.0f);
    } else if (p->damage_type == DAMAGE_FIRE) {
        enemy_apply_burn(target, p->damage * 0.4f, 2.0f);
    }

    bool died = enemy_take_damage(target, p->damage, p->damage_type);
    if (died) {
        *gold_earned += target->reward;
        return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Update                                                            */
/* ------------------------------------------------------------------ */

int projectile_update(ProjectileManager *pm, EnemyManager *em, float dt,
                       int *gold_earned)
{
    int total_kills = 0;
    *gold_earned = 0;

    for (int i = 0; i < pm->count; /* incremented conditionally */) {
        Projectile *p = &pm->projectiles[i];
        if (!p->active) {
            projectile_remove(pm, i);
            continue;
        }

        /* Bullets fly in a straight line — no homing, no re-targeting.
           Velocity is set at spawn and never changes direction. */

        float dist_to_target;

        if (p->arcing) {
            /* Arc trajectory: interpolate XZ position, compute Y from parabola */
            p->arc_progress += dt / p->arc_duration;
            if (p->arc_progress > 1.0f) p->arc_progress = 1.0f;

            float t = p->arc_progress;
            p->position.x = p->arc_origin.x + (p->arc_target.x - p->arc_origin.x) * t;
            p->position.y = p->arc_origin.y + (p->arc_target.y - p->arc_origin.y) * t;
            p->height = 4.0f * p->arc_height * t * (1.0f - t);

            dist_to_target = (1.0f - t) * vec2_distance(p->arc_origin, p->arc_target);
        } else {
            /* Straight-line flight — velocity never changes after spawn */
            p->position = vec2_add(p->position, vec2_scale(p->velocity, dt));

            dist_to_target = vec2_distance(p->position, p->target_pos);
        }

        /* Check hit against any enemy the bullet passes close to */
        bool did_hit = false;

        if (p->arcing) {
            /* Arcing shells hit when they reach the ground */
            if (p->arc_progress >= 1.0f) {
                if (p->splash_radius > 0.0f)
                    total_kills += deal_splash_damage(p, em, p->position, gold_earned);
                did_hit = true;
            }
        } else {
            /* Straight-line bullets: check collision with ANY enemy nearby */
            for (int j = 0; j < em->count; j++) {
                Enemy *e = &em->enemies[j];
                if (!e->active || e->burrowed) continue;
                float d = vec2_distance(p->position, e->position);
                if (d < HIT_RADIUS) {
                    if (p->splash_radius > 0.0f) {
                        total_kills += deal_splash_damage(p, em, p->position, gold_earned);
                    } else {
                        total_kills += deal_single_damage(p, e, gold_earned);
                    }
                    did_hit = true;
                    break;
                }
            }
            /* Also despawn if bullet reached its target position (missed everyone) */
            if (!did_hit && dist_to_target < HIT_RADIUS) {
                did_hit = true; /* just despawn, no damage */
            }
        }

        if (did_hit) {
            projectile_remove(pm, i);
            continue;
        }

        /* Lifetime */
        p->lifetime -= dt;
        if (p->lifetime <= 0.0f) {
            /* Splash projectiles still explode at current position on timeout */
            if (p->splash_radius > 0.0f) {
                total_kills += deal_splash_damage(p, em, p->position, gold_earned);
            }
            projectile_remove(pm, i);
            continue;
        }

        i++;
    }

    return total_kills;
}
