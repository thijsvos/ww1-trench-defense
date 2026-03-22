#ifndef TD_TIME_H
#define TD_TIME_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct GameClock {
    double current_time;
    double last_time;
    double dt;
    double fixed_dt;
    double accumulator;
    double alpha;
    uint64_t tick;
    /* FPS counter — smoothed over 0.5s */
    double fps_timer;
    int fps_frame_count;
    float fps;
} GameClock;

void clock_init(GameClock *clock);
void clock_update(GameClock *clock);
bool clock_should_tick(GameClock *clock);

#endif /* TD_TIME_H */
