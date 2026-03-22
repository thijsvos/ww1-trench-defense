#include "tower.h"
#include "enemy.h"
#include "../math/vec.h"
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------ */
/*  Static tower definitions                                          */
/* ------------------------------------------------------------------ */

static const TowerDef tower_defs[TOWER_TYPE_COUNT] = {
    [TOWER_MACHINE_GUN] = {
        .name        = "Machine Gun",
        .cost        = 100,
        .range       = 5.0f,
        .fire_rate   = 8.0f,
        .damage      = 8.0f,
        .damage_type = DAMAGE_BULLET,
        .splash_radius = 0.0f,
        .min_range     = 0.0f,
        .upgrade_cost  = { 0, 75, 150 },
    },
    [TOWER_MORTAR] = {
        .name        = "Mortar",
        .cost        = 150,
        .range       = 7.0f,
        .fire_rate   = 1.0f,
        .damage      = 40.0f,
        .damage_type = DAMAGE_EXPLOSIVE,
        .splash_radius = 1.5f,
        .min_range     = 4.0f,
        .upgrade_cost  = { 0, 115, 225 },
    },
    [TOWER_SNIPER] = {
        .name        = "Sniper",
        .cost        = 125,
        .range       = 8.0f,
        .fire_rate   = 0.5f,
        .damage      = 80.0f,
        .damage_type = DAMAGE_PIERCING,
        .splash_radius = 0.0f,
        .min_range     = 0.0f,
        .upgrade_cost  = { 0, 95, 190 },
    },
    [TOWER_BARBED_WIRE] = {
        .name        = "Barbed Wire",
        .cost        = 30,
        .range       = 0.5f,
        .fire_rate   = 0.0f,
        .damage      = 2.0f,
        .damage_type = DAMAGE_BULLET,
        .splash_radius = 0.0f,
        .min_range     = 0.0f,
        .upgrade_cost  = { 0, 20, 45 },
    },
    [TOWER_ARTILLERY] = {
        .name        = "Artillery",
        .cost        = 300,
        .range       = 12.0f,
        .fire_rate   = 0.2f,
        .damage      = 200.0f,
        .damage_type = DAMAGE_HEAVY_EXPLOSIVE,
        .splash_radius = 2.0f,
        .min_range     = 8.0f,
        .upgrade_cost  = { 0, 225, 450 },
    },
    [TOWER_GAS] = {
        .name        = "Gas Dispenser",
        .cost        = 175,
        .range       = 4.0f,
        .fire_rate   = 0.5f,
        .damage      = 15.0f,
        .damage_type = DAMAGE_CHEMICAL,
        .splash_radius = 2.0f,
        .min_range     = 0.0f,
        .upgrade_cost  = { 0, 130, 265 },
    },
    [TOWER_FLAMETHROWER] = {
        .name        = "Flamethrower",
        .cost        = 200,
        .range       = 3.0f,
        .fire_rate   = 3.0f,
        .damage      = 20.0f,
        .damage_type = DAMAGE_FIRE,
        .splash_radius = 1.0f,
        .min_range     = 0.0f,
        .upgrade_cost  = { 0, 150, 300 },
    },
    [TOWER_OBSERVATION] = {
        .name        = "Observation Balloon",
        .cost        = 125,
        .range       = 4.0f,
        .fire_rate   = 0.0f,
        .damage      = 0.0f,
        .damage_type = DAMAGE_BULLET,
        .splash_radius = 0.0f,
        .min_range     = 0.0f,
        .upgrade_cost  = { 0, 95, 190 },
    },
};

/* ------------------------------------------------------------------ */
/*  Upgrade multipliers per tier (cumulative from base)               */
/*  Tier 0 = base, Tier 1 = +30%/+10%/+15%, Tier 2 = +50%/+20%/+25% */
/* ------------------------------------------------------------------ */

static const float upgrade_damage_mult[]    = { 1.0f, 1.3f, 1.5f };
static const float upgrade_range_mult[]     = { 1.0f, 1.1f, 1.2f };
static const float upgrade_fire_rate_mult[] = { 1.0f, 1.15f, 1.25f };

/* ------------------------------------------------------------------ */
/*  Public helpers                                                    */
/* ------------------------------------------------------------------ */

const TowerDef *tower_get_def(TowerType type)
{
    if (type < 0 || type >= TOWER_TYPE_COUNT) {
        return NULL;
    }
    return &tower_defs[type];
}

/* ------------------------------------------------------------------ */
/*  Manager init                                                      */
/* ------------------------------------------------------------------ */

void tower_manager_init(TowerManager *tm)
{
    memset(tm, 0, sizeof(*tm));
}

/* ------------------------------------------------------------------ */
/*  Place                                                             */
/* ------------------------------------------------------------------ */

Tower *tower_place(TowerManager *tm, TowerType type, int tile_x, int tile_y)
{
    if (tm->count >= MAX_TOWERS) {
        return NULL;
    }
    if (type < 0 || type >= TOWER_TYPE_COUNT) {
        return NULL;
    }

    const TowerDef *def = &tower_defs[type];
    Tower *t = &tm->towers[tm->count];
    memset(t, 0, sizeof(*t));

    t->active       = true;
    t->type         = type;
    t->upgrade_level = 0;
    t->tile         = ivec2(tile_x, tile_y);
    t->position     = vec2((float)tile_x + 0.5f, (float)tile_y + 0.5f);
    t->range        = def->range;
    t->fire_rate    = def->fire_rate;
    t->damage       = def->damage;
    t->damage_type  = def->damage_type;
    t->splash_radius = def->splash_radius;
    t->min_range    = def->min_range;
    t->fire_cooldown = 0.0f;
    t->priority     = TARGET_FIRST;
    t->target_index = -1;
    t->total_invested = def->cost;

    /* Barbed wire defaults */
    if (type == TOWER_BARBED_WIRE) {
        t->slow_factor = 0.5f;  /* enemies slowed to 50% speed */
        t->bleed_dps   = def->damage;
    }

    /* Observation balloon defaults */
    if (type == TOWER_OBSERVATION) {
        t->buff_range       = def->range;
        t->buff_damage_mult = 0.10f;  /* +10% damage */
        t->buff_range_bonus = 1;      /* +1 range */
    }

    tm->count++;
    return t;
}

/* ------------------------------------------------------------------ */
/*  Find target                                                       */
/* ------------------------------------------------------------------ */

int tower_find_target(Tower *tower, EnemyManager *em)
{
    int best = -1;
    float best_score = 0.0f;
    bool first_candidate = true;

    for (int i = 0; i < em->count; i++) {
        Enemy *e = &em->enemies[i];
        if (!e->active)   continue;
        if (e->burrowed)  continue;

        float dist = vec2_distance(tower->position, e->position);
        if (dist > tower->range)      continue;
        if (dist < tower->min_range)  continue;

        float score = 0.0f;
        switch (tower->priority) {
            case TARGET_FIRST:     score =  e->progress; break;
            case TARGET_LAST:      score = -e->progress; break;
            case TARGET_STRONGEST: score =  e->hp;       break;
            case TARGET_WEAKEST:   score = -e->hp;       break;
            case TARGET_NEAREST:   score = -dist;        break;
        }

        if (first_candidate || score > best_score) {
            best_score = score;
            best = i;
            first_candidate = false;
        }
    }

    return best;
}

/* ------------------------------------------------------------------ */
/*  Barbed wire: slow + bleed enemies on its tile                     */
/* ------------------------------------------------------------------ */

static void barbed_wire_update(Tower *wire, EnemyManager *em, float dt)
{
    (void)dt;

    for (int i = 0; i < em->count; i++) {
        Enemy *e = &em->enemies[i];
        if (!e->active || e->burrowed) continue;

        float dist = vec2_distance(wire->position, e->position);
        if (dist <= wire->range) {
            enemy_apply_slow(e, wire->slow_factor, 0.5f);
            /* Apply bleed as burn damage (physical bleed) */
            enemy_apply_burn(e, wire->bleed_dps, 1.0f);
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Observation balloon: buff nearby towers                           */
/* ------------------------------------------------------------------ */

static void observation_clear_buffs(TowerManager *tm)
{
    for (int i = 0; i < tm->count; i++) {
        Tower *t = &tm->towers[i];
        if (!t->active) continue;
        /* Reset buffed stats to base + upgrade level */
        const TowerDef *def = &tower_defs[t->type];
        int lvl = t->upgrade_level;
        t->range  = def->range  * upgrade_range_mult[lvl];
        t->damage = def->damage * upgrade_damage_mult[lvl];
    }
}

static void observation_apply_buffs(TowerManager *tm)
{
    for (int i = 0; i < tm->count; i++) {
        Tower *obs = &tm->towers[i];
        if (!obs->active || obs->type != TOWER_OBSERVATION) continue;

        for (int j = 0; j < tm->count; j++) {
            if (i == j) continue;
            Tower *t = &tm->towers[j];
            if (!t->active) continue;
            /* Don't buff other observation balloons */
            if (t->type == TOWER_OBSERVATION) continue;

            float dist = vec2_distance(obs->position, t->position);
            if (dist <= obs->buff_range) {
                t->range  += (float)obs->buff_range_bonus;
                t->damage *= (1.0f + obs->buff_damage_mult);
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/*  tower_try_fire                                                    */
/* ------------------------------------------------------------------ */

bool tower_try_fire(Tower *tower)
{
    if (!tower->active)         return false;
    if (tower->fire_rate <= 0.0f) return false;
    if (tower->fire_cooldown > 0.0f) return false;
    if (tower->target_index < 0) return false;

    tower->fire_cooldown = 1.0f / tower->fire_rate;
    return true;
}

/* ------------------------------------------------------------------ */
/*  Update                                                            */
/* ------------------------------------------------------------------ */

void tower_update(TowerManager *tm, EnemyManager *em, float dt)
{
    /* Pass 1: reset observation buffs so they can be recalculated cleanly */
    observation_clear_buffs(tm);

    /* Pass 2: apply observation buffs */
    observation_apply_buffs(tm);

    /* Pass 3: update each tower */
    for (int i = 0; i < tm->count; i++) {
        Tower *t = &tm->towers[i];
        if (!t->active) continue;

        /* Barbed wire has special behaviour */
        if (t->type == TOWER_BARBED_WIRE) {
            barbed_wire_update(t, em, dt);
            continue;
        }

        /* Observation balloon doesn't fire */
        if (t->type == TOWER_OBSERVATION) {
            continue;
        }

        /* Tick cooldown */
        if (t->fire_cooldown > 0.0f) {
            t->fire_cooldown -= dt;
            if (t->fire_cooldown < 0.0f) {
                t->fire_cooldown = 0.0f;
            }
        }

        /* Validate current target */
        if (t->target_index >= 0) {
            bool still_valid = false;
            if (t->target_index < em->count) {
                Enemy *e = &em->enemies[t->target_index];
                if (e->active && !e->burrowed) {
                    float dist = vec2_distance(t->position, e->position);
                    if (dist <= t->range && dist >= t->min_range) {
                        still_valid = true;
                    }
                }
            }
            if (!still_valid) {
                t->target_index = -1;
            }
        }

        /* Acquire new target if needed */
        if (t->target_index < 0) {
            t->target_index = tower_find_target(t, em);
        }

        /* Update facing angle toward target */
        if (t->target_index >= 0 && t->target_index < em->count) {
            Enemy *e = &em->enemies[t->target_index];
            float dx = e->position.x - t->position.x;
            float dy = e->position.y - t->position.y;
            t->facing_angle = atan2f(dy, dx);
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Upgrade                                                           */
/* ------------------------------------------------------------------ */

bool tower_upgrade(Tower *tower)
{
    if (!tower->active) return false;
    if (tower->upgrade_level >= 2) return false;

    const TowerDef *def = &tower_defs[tower->type];
    int next_level = tower->upgrade_level + 1;

    tower->upgrade_level = next_level;
    tower->damage    = def->damage    * upgrade_damage_mult[next_level];
    tower->range     = def->range     * upgrade_range_mult[next_level];
    tower->fire_rate = def->fire_rate * upgrade_fire_rate_mult[next_level];
    tower->total_invested += def->upgrade_cost[next_level];

    /* Scale barbed wire special values */
    if (tower->type == TOWER_BARBED_WIRE) {
        tower->slow_factor = 0.5f - 0.05f * (float)next_level; /* 0.45, 0.40 */
        tower->bleed_dps   = tower->damage;
    }

    /* Scale observation balloon buff */
    if (tower->type == TOWER_OBSERVATION) {
        tower->buff_range       = def->range * upgrade_range_mult[next_level];
        tower->buff_damage_mult = 0.10f + 0.05f * (float)next_level; /* 0.15, 0.20 */
        tower->buff_range_bonus = 1 + next_level;                     /* 2, 3 */
    }

    return true;
}

/* ------------------------------------------------------------------ */
/*  Remove (sell)                                                     */
/* ------------------------------------------------------------------ */

void tower_remove(TowerManager *tm, int index)
{
    if (index < 0 || index >= tm->count) return;

    /* Swap with last */
    tm->count--;
    if (index < tm->count) {
        tm->towers[index] = tm->towers[tm->count];
    }
    memset(&tm->towers[tm->count], 0, sizeof(Tower));
}
