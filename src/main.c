#include "core/engine.h"
#include "core/log.h"
#include "render/renderer.h"
#include "ui/ui.h"
#include "states/app_context.h"
#include "states/state_menu.h"
#include "states/state_difficulty.h"
#include "states/state_levelselect.h"
#include "states/state_play.h"
#include "states/state_pause.h"
#include "states/state_gameover.h"

#include <string.h>

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

int main(void)
{
    Engine engine;
    if (!engine_init(&engine, WINDOW_WIDTH, WINDOW_HEIGHT, "WW1: Trench Defense"))
        return 1;

    Renderer renderer;
    if (!renderer_init(&renderer)) {
        engine_shutdown(&engine);
        return 1;
    }

    UIContext ui;
    if (!ui_init(&ui)) {
        renderer_shutdown(&renderer);
        engine_shutdown(&engine);
        return 1;
    }

    /* State machine */
    StateManager sm;
    state_manager_init(&sm);
    sm.states[STATE_MENU]         = state_menu_create();
    sm.states[STATE_DIFFICULTY]   = state_difficulty_create();
    sm.states[STATE_LEVEL_SELECT] = state_levelselect_create();
    sm.states[STATE_PLAY]         = state_play_create();
    sm.states[STATE_PAUSE]        = state_pause_create();
    sm.states[STATE_GAME_OVER]    = state_gameover_create();

    /* Unified application context */
    static AppContext app;
    memset(&app, 0, sizeof(app));
    app.sm = &sm;
    app.selected_level = -1;
    app.unlocked[0] = true;

    /* Start at main menu */
    state_set(&sm, STATE_MENU, &app);

    while (!engine_should_close(&engine)) {
        engine_begin_frame(&engine);

        /* Begin UI frame (use logical window size, not framebuffer size, to match mouse coords) */
        ui_begin(&ui, engine.window.window_w, engine.window.window_h,
                 (float)engine.input.mouse_x, (float)engine.input.mouse_y,
                 input_mouse_down(&engine.input, 0),
                 input_mouse_pressed(&engine.input, 0));

        /* Update + render current state */
        state_update(&sm, &engine, &ui);
        state_render(&sm, &engine, &ui);

        /* Flush UI */
        ui_end(&ui);

        renderer_end_frame();
        window_swap_buffers(&engine.window);
    }

    /* Cleanup */
    if (app.game_initialized)
        game_shutdown(&app.game);
    ui_destroy(&ui);
    renderer_shutdown(&renderer);
    engine_shutdown(&engine);
    return 0;
}
