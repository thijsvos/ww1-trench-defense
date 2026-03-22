#ifndef TD_STATE_GAMEOVER_H
#define TD_STATE_GAMEOVER_H

#include "state.h"

typedef struct GameOverContext {
    StateManager *sm;
    bool victory;
    int waves_survived;
    int total_kills;
    int score;
} GameOverContext;

State state_gameover_create(void);

#endif /* TD_STATE_GAMEOVER_H */
