#ifndef TD_STATE_PAUSE_H
#define TD_STATE_PAUSE_H

#include "state.h"

typedef struct PauseContext {
    StateManager *sm;
} PauseContext;

State state_pause_create(void);

#endif /* TD_STATE_PAUSE_H */
