#ifndef TD_ENGINE_H
#define TD_ENGINE_H

#include "window.h"
#include "input.h"
#include "td_time.h"
#include "memory.h"

#include <stdbool.h>

typedef struct Engine {
    Window window;
    Input input;
    GameClock clock;
    Arena permanent_arena;
    Arena frame_arena;
    bool running;
    bool debug_overlay;
} Engine;

bool engine_init(Engine *engine, int width, int height, const char *title);
void engine_shutdown(Engine *engine);
void engine_begin_frame(Engine *engine);
bool engine_should_close(Engine *engine);

#endif /* TD_ENGINE_H */
