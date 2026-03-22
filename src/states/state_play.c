#include "state_play.h"
#include "app_context.h"
#include "../core/log.h"
#include "../render/renderer.h"

#include <string.h>
#include <GLFW/glfw3.h>

static void play_enter(void *ctx) {
    AppContext *app = (AppContext *)ctx;
    if (!app->game_initialized) {
        if (!game_init(&app->game, app->level_path,
                       1280, 720, app->difficulty)) {
            LOG_ERROR("Failed to init game: %s", app->level_path);
            return;
        }
        app->game_initialized = true;
        LOG_INFO("Play state entered: %s", app->level_path);
    } else {
        app->needs_clock_reset = true;
        LOG_INFO("Play state resumed");
    }
}

static void play_exit(void *ctx) {
    (void)ctx;
    LOG_INFO("Play state exited");
}

static void play_update(void *ctx, Engine *engine, UIContext *ui) {
    AppContext *app = (AppContext *)ctx;
    if (!app->game_initialized) {
        state_set(app->sm, STATE_MENU, app);
        return;
    }

    app->game.camera.viewport_width  = engine->window.window_w;
    app->game.camera.viewport_height = engine->window.window_h;

    /* After resuming from pause, reset clock to discard accumulated time */
    if (app->needs_clock_reset) {
        engine->clock.accumulator = 0.0;
        engine->clock.last_time = glfwGetTime();
        app->needs_clock_reset = false;
    }

    if (input_key_pressed(&engine->input, GLFW_KEY_ESCAPE)) {
        state_set(app->sm, STATE_PAUSE, app);
        return;
    }

    if (input_key_pressed(&engine->input, GLFW_KEY_F2)) {
        engine->debug_overlay = !engine->debug_overlay;
        app->game.debug_overlay = engine->debug_overlay;
    }

    game_update(&app->game, &engine->input, &engine->clock);

    if (app->game.game_over) {
        /* Populate game over stats */
        app->go_victory = app->game.victory;
        app->go_waves_survived = app->game.waves.current_wave + 1;
        app->go_total_kills = app->game.economy.total_kills;
        app->go_score = app->game.economy.score;
        state_set(app->sm, STATE_GAME_OVER, app);
    }

    (void)ui;
}

static void play_render(void *ctx, Engine *engine, UIContext *ui) {
    AppContext *app = (AppContext *)ctx;
    if (!app->game_initialized) return;
    renderer_begin_frame();
    game_render(&app->game, ui);
    (void)engine;
}

State state_play_create(void) {
    State s;
    memset(&s, 0, sizeof(s));
    s.id = STATE_PLAY;
    s.enter = play_enter;
    s.exit = play_exit;
    s.update = play_update;
    s.render = play_render;
    return s;
}
