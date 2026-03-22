#ifndef TD_WAVE_H
#define TD_WAVE_H

#include <stdbool.h>
#include "enemy.h"

#define MAX_WAVES 25
#define MAX_SPAWN_GROUPS 8  /* per wave */

typedef struct SpawnGroup {
    EnemyType type;
    int count;
    int path_index;     /* which path (-1 = random) */
    float spawn_delay;  /* seconds between each spawn in this group */
} SpawnGroup;

typedef struct WaveDef {
    SpawnGroup groups[MAX_SPAWN_GROUPS];
    int group_count;
    float delay_before;  /* seconds before wave starts */
    int bonus_gold;      /* gold awarded when wave completes */
} WaveDef;

typedef struct WaveManager {
    WaveDef waves[MAX_WAVES];
    int wave_count;
    int current_wave;    /* -1 = not started, 0.. = active wave */
    bool wave_active;    /* enemies still spawning */

    /* Spawning state */
    int current_group;
    int spawned_in_group;
    float spawn_timer;
    float pre_wave_timer;
    float hp_scale;      /* scales with wave number: 1.03^wave */
    float diff_hp_mult;  /* difficulty HP multiplier */
    float diff_spd_mult; /* difficulty speed multiplier */
    float diff_reward_mult; /* difficulty reward multiplier */
} WaveManager;

void wave_manager_init(WaveManager *wm, int num_waves);
void wave_generate_waves(WaveManager *wm, int num_waves, int num_paths); /* generate wave definitions */
void wave_start_next(WaveManager *wm);
/* Returns number of enemies spawned this frame */
int wave_update(WaveManager *wm, EnemyManager *em, PathSet *paths, float dt);
bool wave_is_complete(WaveManager *wm, EnemyManager *em); /* no more spawns AND no active enemies */
bool wave_all_complete(WaveManager *wm); /* all waves done */

#endif /* TD_WAVE_H */
