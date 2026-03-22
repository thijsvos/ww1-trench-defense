#include "td_time.h"

void clock_init(GameClock *clock) {
    clock->current_time = glfwGetTime();
    clock->last_time = clock->current_time;
    clock->dt = 0.0;
    clock->fixed_dt = 1.0 / 60.0;
    clock->accumulator = 0.0;
    clock->alpha = 0.0;
    clock->tick = 0;
    clock->fps_timer = 0.0;
    clock->fps_frame_count = 0;
    clock->fps = 0.0f;
}

void clock_update(GameClock *clock) {
    clock->current_time = glfwGetTime();
    clock->dt = clock->current_time - clock->last_time;
    clock->last_time = clock->current_time;

    /* Clamp delta time to prevent spiral of death */
    if (clock->dt > 0.25) {
        clock->dt = 0.25;
    }

    clock->accumulator += clock->dt;

    /* FPS counter — update every 0.5 seconds */
    clock->fps_frame_count++;
    clock->fps_timer += clock->dt;
    if (clock->fps_timer >= 0.5) {
        clock->fps = (float)(clock->fps_frame_count / clock->fps_timer);
        clock->fps_frame_count = 0;
        clock->fps_timer = 0.0;
    }
}

bool clock_should_tick(GameClock *clock) {
    if (clock->accumulator >= clock->fixed_dt) {
        clock->accumulator -= clock->fixed_dt;
        clock->tick++;
        return true;
    }

    /* All ticks consumed, compute interpolation factor */
    clock->alpha = clock->accumulator / clock->fixed_dt;
    return false;
}
