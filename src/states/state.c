#include "state.h"
#include "../core/log.h"

#include <string.h>

void state_manager_init(StateManager *sm) {
    memset(sm, 0, sizeof(*sm));
    sm->current = NULL;
    sm->next = NULL;
    sm->ctx = NULL;
    sm->should_pop = false;
    sm->prev_state = STATE_NONE;
}

void state_set(StateManager *sm, StateID id, void *ctx) {
    if (id == STATE_NONE || id > STATE_CAMPAIGN_VICTORY) {
        LOG_WARN("state_set: invalid state id %d", id);
        return;
    }
    sm->next = &sm->states[id];
    sm->ctx = ctx;
    LOG_INFO("State transition queued -> %d", id);
}

void state_update(StateManager *sm, Engine *engine, UIContext *ui) {
    /* Handle pending transition */
    if (sm->next) {
        /* Exit old state */
        if (sm->current && sm->current->exit) {
            sm->current->exit(sm->ctx);
        }

        /* Record previous state for pause/gameover return */
        if (sm->current) {
            sm->prev_state = sm->current->id;
        }

        /* Enter new state */
        sm->current = sm->next;
        sm->next = NULL;
        if (sm->current->enter) {
            sm->current->enter(sm->ctx);
        }
    }

    /* Update current state */
    if (sm->current && sm->current->update) {
        sm->current->update(sm->ctx, engine, ui);
    }
}

void state_render(StateManager *sm, Engine *engine, UIContext *ui) {
    if (sm->current && sm->current->render) {
        sm->current->render(sm->ctx, engine, ui);
    }
}
