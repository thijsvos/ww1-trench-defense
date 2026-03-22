#include "engine.h"
#include "log.h"

#include <string.h>

#define PERMANENT_ARENA_SIZE (16 * 1024 * 1024) /* 16 MB */
#define FRAME_ARENA_SIZE     (4 * 1024 * 1024)  /*  4 MB */

bool engine_init(Engine *engine, int width, int height, const char *title) {
    memset(engine, 0, sizeof(Engine));

    if (!window_create(&engine->window, width, height, title)) {
        LOG_ERROR("Failed to create window");
        return false;
    }

    input_init(&engine->input, engine->window.handle);
    clock_init(&engine->clock);

    engine->permanent_arena = arena_create(PERMANENT_ARENA_SIZE);
    if (!engine->permanent_arena.buf) {
        LOG_ERROR("Failed to create permanent arena");
        window_destroy(&engine->window);
        return false;
    }

    engine->frame_arena = arena_create(FRAME_ARENA_SIZE);
    if (!engine->frame_arena.buf) {
        LOG_ERROR("Failed to create frame arena");
        arena_destroy(&engine->permanent_arena);
        window_destroy(&engine->window);
        return false;
    }

    engine->running = true;
    engine->debug_overlay = false;

    LOG_INFO("Engine initialized");
    return true;
}

void engine_shutdown(Engine *engine) {
    arena_destroy(&engine->frame_arena);
    arena_destroy(&engine->permanent_arena);
    window_destroy(&engine->window);
    engine->running = false;
    LOG_INFO("Engine shut down");
}

void engine_begin_frame(Engine *engine) {
    input_update(&engine->input);   /* snapshot prev state BEFORE new events */
    window_poll_events(&engine->window);
    clock_update(&engine->clock);
    arena_reset(&engine->frame_arena);
}

bool engine_should_close(Engine *engine) {
    return engine->window.should_close || !engine->running;
}
