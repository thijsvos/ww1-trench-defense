#ifndef TD_STATE_PLAY_H
#define TD_STATE_PLAY_H

#include "state.h"
#include "../game/game.h"

#define PLAY_MAX_LEVEL_PATH 128

typedef struct PlayContext {
    StateManager *sm;
    GameState game;
    char level_path[PLAY_MAX_LEVEL_PATH];
    bool initialized;
} PlayContext;

State state_play_create(void);

#endif /* TD_STATE_PLAY_H */
