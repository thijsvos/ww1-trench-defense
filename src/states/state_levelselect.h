#ifndef TD_STATE_LEVELSELECT_H
#define TD_STATE_LEVELSELECT_H

#include "state.h"

#define MAX_LEVELS 5

typedef struct LevelSelectContext {
    StateManager *sm;
    bool unlocked[MAX_LEVELS];
    int selected_level; /* 0-based, -1 = none */
} LevelSelectContext;

State state_levelselect_create(void);

/* Get the asset path for a level index (0-based). Returns NULL if invalid. */
const char *levelselect_get_path(int level_index);

#endif /* TD_STATE_LEVELSELECT_H */
