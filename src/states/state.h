#ifndef TD_STATE_H
#define TD_STATE_H

#include <stdbool.h>
#include "../core/engine.h"
#include "../ui/ui.h"

typedef enum StateID {
    STATE_NONE = 0,
    STATE_MENU,
    STATE_DIFFICULTY,
    STATE_LEVEL_SELECT,
    STATE_BRIEFING,
    STATE_PLAY,
    STATE_PAUSE,
    STATE_GAME_OVER,
    STATE_CAMPAIGN_VICTORY,
} StateID;

typedef struct State {
    StateID id;
    void (*enter)(void *ctx);
    void (*exit)(void *ctx);
    void (*update)(void *ctx, Engine *engine, UIContext *ui);
    void (*render)(void *ctx, Engine *engine, UIContext *ui);
} State;

typedef struct StateManager {
    State *current;
    State *next;        /* pending transition */
    void *ctx;          /* state-specific context */
    bool should_pop;    /* return to previous state */

    State states[9];
    StateID prev_state; /* for returning from pause/gameover */
} StateManager;

void state_manager_init(StateManager *sm);
void state_set(StateManager *sm, StateID id, void *ctx);
void state_update(StateManager *sm, Engine *engine, UIContext *ui);
void state_render(StateManager *sm, Engine *engine, UIContext *ui);

#endif /* TD_STATE_H */
