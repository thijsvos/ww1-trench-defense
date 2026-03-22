#include "wave.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

/* ------------------------------------------------------------------ */
/*  Helper: add a spawn group to a wave definition                    */
/* ------------------------------------------------------------------ */

static void wave_add_group(WaveDef *wd, EnemyType type, int count,
                           int path_index, float spawn_delay)
{
    if (wd->group_count >= MAX_SPAWN_GROUPS) return;
    SpawnGroup *g = &wd->groups[wd->group_count];
    g->type        = type;
    g->count       = count;
    g->path_index  = path_index;
    g->spawn_delay = spawn_delay;
    wd->group_count++;
}

/* ------------------------------------------------------------------ */
/*  Wave generation                                                   */
/* ------------------------------------------------------------------ */

void wave_generate_waves(WaveManager *wm, int num_waves, int num_paths)
{
    if (num_waves > MAX_WAVES) num_waves = MAX_WAVES;
    if (num_paths < 1) num_paths = 1;
    memset(wm->waves, 0, sizeof(wm->waves));

    /* Use -1 for random path (distributes across all available paths).
       Use specific indices only when we want enemies to attack together. */
    int p0 = 0 % num_paths;
    int p1 = 1 % num_paths;
    int p2 = 2 % num_paths;
    int p3 = 3 % num_paths;
    (void)p2; (void)p3; /* used in later waves if paths available */

    /* ---- Phase 1: The Lull (Waves 1-5) ---- */

    /* Wave 1 – a handful of infantry */
    {
        WaveDef *w = &wm->waves[0];
        w->delay_before = 5.0f;
        w->bonus_gold   = 25;
        wave_add_group(w, ENEMY_INFANTRY, 6, -1, 1.2f);
    }

    /* Wave 2 – more infantry, split across paths */
    {
        WaveDef *w = &wm->waves[1];
        w->delay_before = 8.0f;
        w->bonus_gold   = 25;
        wave_add_group(w, ENEMY_INFANTRY, 5, p0, 1.0f);
        wave_add_group(w, ENEMY_INFANTRY, 5, p1, 1.0f);
    }

    /* Wave 3 – infantry wave, faster spawns */
    {
        WaveDef *w = &wm->waves[2];
        w->delay_before = 8.0f;
        w->bonus_gold   = 25;
        wave_add_group(w, ENEMY_INFANTRY, 8, -1, 0.8f);
        wave_add_group(w, ENEMY_INFANTRY, 4, -1, 1.0f);
    }

    /* Wave 4 – cavalry scouts appear */
    {
        WaveDef *w = &wm->waves[3];
        w->delay_before = 10.0f;
        w->bonus_gold   = 25;
        wave_add_group(w, ENEMY_INFANTRY, 6, -1, 1.0f);
        wave_add_group(w, ENEMY_CAVALRY,  4, -1, 0.7f);
    }

    /* Wave 5 – introduces medics */
    {
        WaveDef *w = &wm->waves[4];
        w->delay_before = 10.0f;
        w->bonus_gold   = 25;
        wave_add_group(w, ENEMY_INFANTRY, 8, -1, 0.9f);
        wave_add_group(w, ENEMY_MEDIC,    2, -1, 2.0f);
        wave_add_group(w, ENEMY_CAVALRY,  3, -1, 0.8f);
    }

    /* ---- Phase 2: Escalation (Waves 6-10) ---- */

    /* Wave 6 – stormtroopers */
    {
        WaveDef *w = &wm->waves[5];
        w->delay_before = 12.0f;
        w->bonus_gold   = 40;
        wave_add_group(w, ENEMY_INFANTRY,     6, -1, 0.9f);
        wave_add_group(w, ENEMY_STORMTROOPER, 3, -1, 1.5f);
    }

    /* Wave 7 – officers lead the charge */
    {
        WaveDef *w = &wm->waves[6];
        w->delay_before = 12.0f;
        w->bonus_gold   = 40;
        wave_add_group(w, ENEMY_OFFICER,  2, p0, 2.0f);
        wave_add_group(w, ENEMY_INFANTRY, 8, p0, 0.8f);
        wave_add_group(w, ENEMY_CAVALRY,  4, p1, 0.7f);
    }

    /* Wave 8 – tunnel sappers */
    {
        WaveDef *w = &wm->waves[7];
        w->delay_before = 12.0f;
        w->bonus_gold   = 40;
        wave_add_group(w, ENEMY_TUNNEL_SAPPER, 4, -1, 1.5f);
        wave_add_group(w, ENEMY_INFANTRY,      6, -1, 1.0f);
        wave_add_group(w, ENEMY_MEDIC,         2, -1, 2.0f);
    }

    /* Wave 9 – armored cars */
    {
        WaveDef *w = &wm->waves[8];
        w->delay_before = 15.0f;
        w->bonus_gold   = 40;
        wave_add_group(w, ENEMY_ARMORED_CAR,   2, -1, 3.0f);
        wave_add_group(w, ENEMY_STORMTROOPER,  4, -1, 1.2f);
        wave_add_group(w, ENEMY_INFANTRY,      6, -1, 0.8f);
    }

    /* Wave 10 – first tank */
    {
        WaveDef *w = &wm->waves[9];
        w->delay_before = 15.0f;
        w->bonus_gold   = 40;
        wave_add_group(w, ENEMY_TANK,          1, p0, 1.0f);
        wave_add_group(w, ENEMY_INFANTRY,      8, -1, 0.7f);
        wave_add_group(w, ENEMY_STORMTROOPER,  3, p1, 1.5f);
        wave_add_group(w, ENEMY_MEDIC,         2, -1, 2.0f);
    }

    /* ---- Phase 3: The Push (Waves 11-15) ---- */

    /* Wave 11 – mixed assault */
    {
        WaveDef *w = &wm->waves[10];
        w->delay_before = 15.0f;
        w->bonus_gold   = 60;
        wave_add_group(w, ENEMY_OFFICER,       2, p0, 2.5f);
        wave_add_group(w, ENEMY_STORMTROOPER,  5, -1, 1.0f);
        wave_add_group(w, ENEMY_ARMORED_CAR,   2, p1, 3.0f);
        wave_add_group(w, ENEMY_INFANTRY,      6, -1, 0.8f);
    }

    /* Wave 12 – sapper and cavalry flank */
    {
        WaveDef *w = &wm->waves[11];
        w->delay_before = 15.0f;
        w->bonus_gold   = 60;
        wave_add_group(w, ENEMY_TUNNEL_SAPPER, 5, -1, 1.2f);
        wave_add_group(w, ENEMY_CAVALRY,       8, -1, 0.5f);
        wave_add_group(w, ENEMY_MEDIC,         3, -1, 2.0f);
    }

    /* Wave 13 – heavy armor push */
    {
        WaveDef *w = &wm->waves[12];
        w->delay_before = 18.0f;
        w->bonus_gold   = 60;
        wave_add_group(w, ENEMY_TANK,          2, -1, 5.0f);
        wave_add_group(w, ENEMY_ARMORED_CAR,   3, -1, 2.0f);
        wave_add_group(w, ENEMY_STORMTROOPER,  4, -1, 1.0f);
        wave_add_group(w, ENEMY_INFANTRY,      6, -1, 0.7f);
    }

    /* Wave 14 – all-rounder */
    {
        WaveDef *w = &wm->waves[13];
        w->delay_before = 18.0f;
        w->bonus_gold   = 60;
        wave_add_group(w, ENEMY_OFFICER,       3, -1, 2.0f);
        wave_add_group(w, ENEMY_STORMTROOPER,  6, -1, 0.9f);
        wave_add_group(w, ENEMY_TUNNEL_SAPPER, 4, -1, 1.2f);
        wave_add_group(w, ENEMY_CAVALRY,       5, -1, 0.6f);
        wave_add_group(w, ENEMY_MEDIC,         3, -1, 2.0f);
    }

    /* Wave 15 – double tank escort */
    {
        WaveDef *w = &wm->waves[14];
        w->delay_before = 18.0f;
        w->bonus_gold   = 60;
        wave_add_group(w, ENEMY_TANK,          2, p0, 4.0f);
        wave_add_group(w, ENEMY_TANK,          1, p1, 1.0f);
        wave_add_group(w, ENEMY_ARMORED_CAR,   3, -1, 2.0f);
        wave_add_group(w, ENEMY_STORMTROOPER,  6, -1, 0.8f);
        wave_add_group(w, ENEMY_MEDIC,         3, -1, 2.0f);
    }

    /* ---- Phase 4: Crisis (Waves 16-19) ---- */

    /* Wave 16 – speed rush */
    {
        WaveDef *w = &wm->waves[15];
        w->delay_before = 20.0f;
        w->bonus_gold   = 80;
        wave_add_group(w, ENEMY_CAVALRY,       12, -1, 0.4f);
        wave_add_group(w, ENEMY_STORMTROOPER,   6, -1, 0.8f);
        wave_add_group(w, ENEMY_ARMORED_CAR,    3, -1, 2.0f);
    }

    /* Wave 17 – underground assault */
    {
        WaveDef *w = &wm->waves[16];
        w->delay_before = 20.0f;
        w->bonus_gold   = 80;
        wave_add_group(w, ENEMY_TUNNEL_SAPPER,  8, -1, 1.0f);
        wave_add_group(w, ENEMY_OFFICER,        3, -1, 2.0f);
        wave_add_group(w, ENEMY_INFANTRY,      10, -1, 0.6f);
        wave_add_group(w, ENEMY_TANK,           2, -1, 4.0f);
    }

    /* Wave 18 – full combined arms */
    {
        WaveDef *w = &wm->waves[17];
        w->delay_before = 20.0f;
        w->bonus_gold   = 80;
        wave_add_group(w, ENEMY_TANK,           2, -1, 4.0f);
        wave_add_group(w, ENEMY_ARMORED_CAR,    4, -1, 1.5f);
        wave_add_group(w, ENEMY_STORMTROOPER,   8, -1, 0.7f);
        wave_add_group(w, ENEMY_OFFICER,        3, -1, 2.0f);
        wave_add_group(w, ENEMY_MEDIC,          4, -1, 1.5f);
    }

    /* Wave 19 – relentless pressure */
    {
        WaveDef *w = &wm->waves[18];
        w->delay_before = 20.0f;
        w->bonus_gold   = 80;
        wave_add_group(w, ENEMY_INFANTRY,      12, -1, 0.5f);
        wave_add_group(w, ENEMY_CAVALRY,        8, -1, 0.4f);
        wave_add_group(w, ENEMY_STORMTROOPER,   6, -1, 0.8f);
        wave_add_group(w, ENEMY_TANK,           3, -1, 3.0f);
        wave_add_group(w, ENEMY_TUNNEL_SAPPER,  5, -1, 1.0f);
        wave_add_group(w, ENEMY_MEDIC,          4, -1, 1.5f);
    }

    /* ---- Wave 20: Final Assault ---- */
    {
        WaveDef *w = &wm->waves[19];
        w->delay_before = 25.0f;
        w->bonus_gold   = 80;
        wave_add_group(w, ENEMY_TANK,           4, -1, 3.0f);
        wave_add_group(w, ENEMY_ARMORED_CAR,    5, -1, 1.5f);
        wave_add_group(w, ENEMY_STORMTROOPER,  10, -1, 0.6f);
        wave_add_group(w, ENEMY_OFFICER,        4, -1, 1.5f);
        wave_add_group(w, ENEMY_CAVALRY,        8, -1, 0.4f);
        wave_add_group(w, ENEMY_TUNNEL_SAPPER,  6, -1, 1.0f);
        wave_add_group(w, ENEMY_MEDIC,          5, -1, 1.5f);
    }

    wm->wave_count = num_waves < 20 ? num_waves : 20;

    /* Double all enemy counts across all waves */
    for (int w = 0; w < wm->wave_count; w++) {
        for (int g = 0; g < wm->waves[w].group_count; g++) {
            wm->waves[w].groups[g].count *= 2;
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Init / control                                                    */
/* ------------------------------------------------------------------ */

void wave_manager_init(WaveManager *wm, int num_waves)
{
    memset(wm, 0, sizeof(*wm));
    wm->current_wave   = -1;
    wm->wave_active    = false;
    wm->hp_scale       = 1.0f;
    /* waves generated separately via wave_generate_waves() */
    wm->wave_count = num_waves < 20 ? num_waves : 20;
}

void wave_start_next(WaveManager *wm)
{
    wm->current_wave++;
    if (wm->current_wave >= wm->wave_count) {
        wm->current_wave = wm->wave_count - 1;
        return;
    }

    wm->wave_active     = true;
    wm->current_group   = 0;
    wm->spawned_in_group = 0;
    wm->spawn_timer     = 0.0f;
    wm->pre_wave_timer  = wm->waves[wm->current_wave].delay_before;
    wm->hp_scale        = powf(1.03f, (float)wm->current_wave);
}

/* ------------------------------------------------------------------ */
/*  Update: spawn enemies according to wave timing                    */
/* ------------------------------------------------------------------ */

int wave_update(WaveManager *wm, EnemyManager *em, PathSet *paths, float dt)
{
    if (!wm->wave_active) return 0;
    if (wm->current_wave < 0 || wm->current_wave >= wm->wave_count) return 0;

    int spawned = 0;

    /* pre-wave countdown */
    if (wm->pre_wave_timer > 0.0f) {
        wm->pre_wave_timer -= dt;
        return 0;
    }

    WaveDef *wd = &wm->waves[wm->current_wave];

    /* advance spawn timer */
    wm->spawn_timer -= dt;

    while (wm->spawn_timer <= 0.0f) {
        /* check if all groups are done */
        if (wm->current_group >= wd->group_count) {
            wm->wave_active = false;
            break;
        }

        SpawnGroup *grp = &wd->groups[wm->current_group];

        /* determine path index */
        int path_idx = grp->path_index;
        if (path_idx < 0 && paths->path_count > 0) {
            path_idx = rand() % paths->path_count;
        }
        /* clamp to valid range */
        if (paths->path_count > 0 && path_idx >= paths->path_count) {
            path_idx = path_idx % paths->path_count;
        }

        {
            Enemy *spawned_e = enemy_spawn(em, grp->type, path_idx,
                                            wm->hp_scale * wm->diff_hp_mult);
            if (spawned_e) {
                spawned_e->base_speed *= wm->diff_spd_mult;
                spawned_e->speed = spawned_e->base_speed;
                spawned_e->reward = (int)(spawned_e->reward * wm->diff_reward_mult);
                if (spawned_e->reward < 1) spawned_e->reward = 1;
            }
        }
        spawned++;
        wm->spawned_in_group++;

        if (wm->spawned_in_group >= grp->count) {
            /* move to next group */
            wm->current_group++;
            wm->spawned_in_group = 0;

            if (wm->current_group >= wd->group_count) {
                wm->wave_active = false;
                break;
            }
            wm->spawn_timer += wd->groups[wm->current_group].spawn_delay;
        } else {
            wm->spawn_timer += grp->spawn_delay;
        }
    }

    return spawned;
}

bool wave_is_complete(WaveManager *wm, EnemyManager *em)
{
    return !wm->wave_active && em->count == 0;
}

bool wave_all_complete(WaveManager *wm)
{
    return wm->current_wave >= wm->wave_count - 1 && !wm->wave_active;
}
