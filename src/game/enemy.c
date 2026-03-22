#include "enemy.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

/* ------------------------------------------------------------------ */
/*  Static enemy definitions                                          */
/* ------------------------------------------------------------------ */

static const EnemyDef s_enemy_defs[ENEMY_TYPE_COUNT] = {
    [ENEMY_INFANTRY] = {
        .name      = "Infantry",
        .max_hp    = 60.0f,
        .speed     = 1.0f,
        .armor     = ARMOR_UNARMORED,
        .reward    = 3,
        .lives_cost = 1,
        .evasion   = 0.0f,
    },
    [ENEMY_CAVALRY] = {
        .name      = "Cavalry",
        .max_hp    = 40.0f,
        .speed     = 2.0f,
        .armor     = ARMOR_UNARMORED,
        .reward    = 4,
        .lives_cost = 1,
        .evasion   = 0.0f,
    },
    [ENEMY_STORMTROOPER] = {
        .name      = "Stormtrooper",
        .max_hp    = 120.0f,
        .speed     = 1.2f,
        .armor     = ARMOR_LIGHT,
        .reward    = 8,
        .lives_cost = 2,
        .evasion   = 0.25f,
    },
    [ENEMY_TANK] = {
        .name      = "Tank",
        .max_hp    = 800.0f,
        .speed     = 0.4f,
        .armor     = ARMOR_HEAVY,
        .reward    = 25,
        .lives_cost = 5,
        .evasion   = 0.0f,
    },
    [ENEMY_MEDIC] = {
        .name      = "Medic",
        .max_hp    = 50.0f,
        .speed     = 0.9f,
        .armor     = ARMOR_UNARMORED,
        .reward    = 10,
        .lives_cost = 1,
        .evasion   = 0.0f,
    },
    [ENEMY_OFFICER] = {
        .name      = "Officer",
        .max_hp    = 100.0f,
        .speed     = 0.8f,
        .armor     = ARMOR_LIGHT,
        .reward    = 12,
        .lives_cost = 3,
        .evasion   = 0.0f,
    },
    [ENEMY_TUNNEL_SAPPER] = {
        .name      = "Tunnel Sapper",
        .max_hp    = 80.0f,
        .speed     = 0.7f,
        .armor     = ARMOR_UNARMORED,
        .reward    = 9,
        .lives_cost = 2,
        .evasion   = 0.0f,
    },
    [ENEMY_ARMORED_CAR] = {
        .name      = "Armored Car",
        .max_hp    = 250.0f,
        .speed     = 1.3f,
        .armor     = ARMOR_MEDIUM,
        .reward    = 15,
        .lives_cost = 3,
        .evasion   = 0.0f,
    },
};

/* ------------------------------------------------------------------ */
/*  Damage multiplier table  [6 damage types][4 armor types]          */
/*  Row order matches the damage_type int values expected from tower.h */
/*    0 = DAMAGE_BULLET                                               */
/*    1 = DAMAGE_PIERCING                                             */
/*    2 = DAMAGE_EXPLOSIVE                                            */
/*    3 = DAMAGE_HEAVY_EXPLOSIVE                                      */
/*    4 = DAMAGE_CHEMICAL                                             */
/*    5 = DAMAGE_FIRE                                                 */
/* ------------------------------------------------------------------ */

#define NUM_DAMAGE_TYPES 6

static const float s_armor_mult[NUM_DAMAGE_TYPES][ARMOR_TYPE_COUNT] = {
    /* DAMAGE_BULLET          */ { 1.0f, 0.7f, 0.3f, 0.1f },
    /* DAMAGE_PIERCING        */ { 1.0f, 1.0f, 0.7f, 0.4f },
    /* DAMAGE_EXPLOSIVE       */ { 1.0f, 0.9f, 0.8f, 0.6f },
    /* DAMAGE_HEAVY_EXPLOSIVE */ { 0.8f, 1.0f, 1.2f, 1.5f },
    /* DAMAGE_CHEMICAL        */ { 1.2f, 1.0f, 0.5f, 0.0f },
    /* DAMAGE_FIRE            */ { 1.5f, 1.2f, 0.6f, 0.3f },
};

/* Tunnel sapper: seconds between burrow toggles */
#define BURROW_CYCLE 3.0f

/* Medic: heals nearby allies */
#define MEDIC_HEAL_RANGE  2.0f
#define MEDIC_HEAL_RATE   8.0f  /* HP per second */

/* Officer: buffs nearby allies */
#define OFFICER_BUFF_RANGE  2.5f
#define OFFICER_SPEED_MULT  1.15f  /* +15% speed */

/* ------------------------------------------------------------------ */
/*  Public API                                                        */
/* ------------------------------------------------------------------ */

const EnemyDef *enemy_get_def(EnemyType type)
{
    if (type < 0 || type >= ENEMY_TYPE_COUNT) return &s_enemy_defs[0];
    return &s_enemy_defs[type];
}

void enemy_manager_init(EnemyManager *em)
{
    memset(em, 0, sizeof(*em));
}

Enemy *enemy_spawn(EnemyManager *em, EnemyType type, int path_index, float hp_scale)
{
    if (em->count >= MAX_ENEMIES) return NULL;

    const EnemyDef *def = enemy_get_def(type);

    Enemy *e = &em->enemies[em->count];
    memset(e, 0, sizeof(*e));

    e->active     = true;
    e->type       = type;
    e->max_hp     = def->max_hp * hp_scale;
    e->hp         = e->max_hp;
    e->base_speed = def->speed;
    e->speed      = def->speed;
    e->armor      = def->armor;
    e->path_index = path_index;
    e->progress   = 0.0f;
    e->slow_factor = 1.0f;
    e->reward     = def->reward;
    e->lives_cost = def->lives_cost;
    e->evasion    = def->evasion;
    e->burrowed   = false;
    e->burrow_timer = BURROW_CYCLE;

    em->count++;
    return e;
}

int enemy_update(EnemyManager *em, PathSet *paths, float dt)
{
    int leaked = 0;
    int i = 0;

    while (i < em->count) {
        Enemy *e = &em->enemies[i];

        /* --- apply burn DoT --- */
        if (e->burn_timer > 0.0f) {
            e->hp -= e->burn_dps * dt;
            e->burn_timer -= dt;
            if (e->burn_timer <= 0.0f) {
                e->burn_timer = 0.0f;
                e->burn_dps   = 0.0f;
            }
        }

        /* --- apply gas DoT --- */
        if (e->gas_timer > 0.0f) {
            e->hp -= e->gas_dps * dt;
            e->gas_timer -= dt;
            if (e->gas_timer <= 0.0f) {
                e->gas_timer = 0.0f;
                e->gas_dps   = 0.0f;
            }
        }

        /* --- check death from DoT --- */
        if (e->hp <= 0.0f) {
            enemy_remove(em, i);
            continue; /* don't increment i, slot has a new enemy */
        }

        /* --- update slow --- */
        if (e->slow_timer > 0.0f) {
            e->slow_timer -= dt;
            if (e->slow_timer <= 0.0f) {
                e->slow_timer  = 0.0f;
                e->slow_factor = 1.0f;
            }
        }

        /* --- tunnel sapper burrow toggle --- */
        if (e->type == ENEMY_TUNNEL_SAPPER) {
            e->burrow_timer -= dt;
            if (e->burrow_timer <= 0.0f) {
                e->burrowed = !e->burrowed;
                e->burrow_timer = BURROW_CYCLE;
            }
        }

        /* --- animation timer (speed-dependent) --- */
        e->anim_timer += dt * e->base_speed;

        /* --- movement --- */
        float effective_speed = e->base_speed * e->slow_factor;
        e->speed = effective_speed;

        if (e->path_index >= 0 && e->path_index < paths->path_count) {
            Path *path = &paths->paths[e->path_index];
            float path_len = path_total_length(path);
            if (path_len > 0.0f) {
                /* progress delta = (speed in tiles/s) * dt / path_length_in_tiles */
                float delta = (effective_speed * dt) / path_len;
                e->progress += delta;
            }

            /* update world position */
            float clamped = e->progress < 0.0f ? 0.0f : (e->progress > 1.0f ? 1.0f : e->progress);
            e->position = path_get_position(path, clamped);
        }

        /* --- check leaked --- */
        if (e->progress >= 1.0f) {
            leaked += e->lives_cost;
            enemy_remove(em, i);
            continue;
        }

        i++;
    }

    /* --- Medic healing pass (after all movement so positions are up to date) --- */
    for (int i = 0; i < em->count; i++) {
        Enemy *medic = &em->enemies[i];
        if (medic->type != ENEMY_MEDIC) continue;

        float heal = MEDIC_HEAL_RATE * dt;
        for (int j = 0; j < em->count; j++) {
            if (j == i) continue; /* don't heal self */
            Enemy *ally = &em->enemies[j];
            if (ally->hp >= ally->max_hp) continue; /* already full */

            float dx = medic->position.x - ally->position.x;
            float dy = medic->position.y - ally->position.y;
            float dist2 = dx * dx + dy * dy;
            if (dist2 <= MEDIC_HEAL_RANGE * MEDIC_HEAL_RANGE) {
                ally->hp += heal;
                if (ally->hp > ally->max_hp)
                    ally->hp = ally->max_hp;
            }
        }
    }

    /* --- Officer speed buff pass --- */
    /* Reset all speeds to base * slow_factor first, then apply officer buff */
    for (int i = 0; i < em->count; i++) {
        Enemy *e = &em->enemies[i];
        e->speed = e->base_speed * e->slow_factor;
    }
    for (int i = 0; i < em->count; i++) {
        Enemy *officer = &em->enemies[i];
        if (officer->type != ENEMY_OFFICER) continue;

        for (int j = 0; j < em->count; j++) {
            if (j == i) continue;
            Enemy *ally = &em->enemies[j];

            float dx = officer->position.x - ally->position.x;
            float dy = officer->position.y - ally->position.y;
            float dist2 = dx * dx + dy * dy;
            if (dist2 <= OFFICER_BUFF_RANGE * OFFICER_BUFF_RANGE) {
                ally->speed *= OFFICER_SPEED_MULT;
            }
        }
    }

    return leaked;
}

void enemy_remove(EnemyManager *em, int index)
{
    if (index < 0 || index >= em->count) return;

    /* swap with last */
    em->enemies[index] = em->enemies[em->count - 1];
    memset(&em->enemies[em->count - 1], 0, sizeof(Enemy));
    em->count--;
}

bool enemy_take_damage(Enemy *enemy, float damage, int damage_type)
{
    /* evasion check */
    if (enemy->evasion > 0.0f) {
        float roll = (float)rand() / (float)RAND_MAX;
        if (roll < enemy->evasion) {
            return false; /* dodged */
        }
    }

    /* burrowed sappers are immune to non-explosive damage */
    if (enemy->burrowed && damage_type < 2) {
        return false;
    }

    float mult = enemy_get_armor_multiplier(damage_type, enemy->armor);
    enemy->hp -= damage * mult;

    return enemy->hp <= 0.0f;
}

void enemy_apply_slow(Enemy *enemy, float factor, float duration)
{
    /* keep the strongest slow (lowest factor) */
    if (factor < enemy->slow_factor || duration > enemy->slow_timer) {
        enemy->slow_factor = factor;
        enemy->slow_timer  = duration;
    }
}

void enemy_apply_burn(Enemy *enemy, float dps, float duration)
{
    /* refresh: keep the stronger effect */
    if (dps >= enemy->burn_dps) {
        enemy->burn_dps   = dps;
        enemy->burn_timer = duration;
    }
}

void enemy_apply_gas(Enemy *enemy, float dps, float duration)
{
    if (dps >= enemy->gas_dps) {
        enemy->gas_dps   = dps;
        enemy->gas_timer = duration;
    }
}

float enemy_get_armor_multiplier(int damage_type, ArmorType armor)
{
    if (damage_type < 0 || damage_type >= NUM_DAMAGE_TYPES) return 1.0f;
    if (armor < 0 || armor >= ARMOR_TYPE_COUNT) return 1.0f;
    return s_armor_mult[damage_type][armor];
}
