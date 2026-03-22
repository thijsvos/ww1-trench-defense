#ifndef TD_STATE_MENU_H
#define TD_STATE_MENU_H

#include "state.h"

typedef struct MenuContext {
    StateManager *sm;
} MenuContext;

State state_menu_create(void);

#endif /* TD_STATE_MENU_H */
