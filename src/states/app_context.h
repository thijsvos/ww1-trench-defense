#ifndef TD_APP_CONTEXT_H
#define TD_APP_CONTEXT_H

#include "state.h"
#include "state_levelselect.h"
#include "../game/game.h"

#include <stdbool.h>

#define APP_MAX_LEVEL_PATH 128

/*
 * Unified application context shared by all game states.
 * Passed as the single void *ctx to every state callback.
 */
typedef struct AppContext {
    StateManager *sm;

    /* Level select */
    bool unlocked[MAX_LEVELS];
    int selected_level;       /* 0-based, -1 = none */
    Difficulty difficulty;

    /* Play */
    GameState game;
    char level_path[APP_MAX_LEVEL_PATH];
    bool game_initialized;
    bool needs_clock_reset;  /* reset accumulator on first update after resume */

    /* Game over */
    bool go_victory;
    int go_waves_survived;
    int go_total_kills;
    int go_score;
} AppContext;

#endif /* TD_APP_CONTEXT_H */
