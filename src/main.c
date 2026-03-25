#include "core/engine.h"
#include "core/log.h"
#include "render/renderer.h"
#include "ui/ui.h"
#include "states/app_context.h"
#include "states/state_menu.h"
#include "states/state_difficulty.h"
#include "states/state_levelselect.h"
#include "states/state_briefing.h"
#include "states/state_play.h"
#include "states/state_pause.h"
#include "states/state_gameover.h"
#include "states/state_victory.h"
#include "audio/audio.h"

#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <libgen.h>
#endif

#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720

/* Set the working directory to the executable's directory so that
 * relative asset paths (assets/shaders/, assets/levels/) resolve
 * correctly regardless of where the program is launched from. */
static void set_exe_dir(const char *argv0)
{
#ifdef _WIN32
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    /* Strip the executable name to get the directory */
    char *last_sep = strrchr(path, '\\');
    if (last_sep) *last_sep = '\0';
    SetCurrentDirectoryA(path);
#elif defined(__APPLE__)
    /* On macOS, use _NSGetExecutablePath or just dirname(argv0) */
    char buf[1024];
    strncpy(buf, argv0, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    chdir(dirname(buf));
#else
    /* Linux: read /proc/self/exe */
    char buf[1024];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0) {
        buf[len] = '\0';
        chdir(dirname(buf));
    } else {
        /* Fallback to argv[0] */
        char arg[1024];
        strncpy(arg, argv0, sizeof(arg) - 1);
        arg[sizeof(arg) - 1] = '\0';
        chdir(dirname(arg));
    }
#endif
}

int main(int argc, char *argv[])
{
    (void)argc;
    set_exe_dir(argv[0]);
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
    sm.states[STATE_BRIEFING]     = state_briefing_create();
    sm.states[STATE_PLAY]         = state_play_create();
    sm.states[STATE_PAUSE]        = state_pause_create();
    sm.states[STATE_GAME_OVER]    = state_gameover_create();
    sm.states[STATE_CAMPAIGN_VICTORY] = state_victory_create();

    /* Unified application context */
    static AppContext app;
    memset(&app, 0, sizeof(app));
    app.sm = &sm;
    app.selected_level = -1;
    for (int i = 0; i < MAX_LEVELS; i++) app.unlocked[i] = true;

    /* Audio */
    if (!audio_init(&app.audio))
        LOG_WARN("Audio init failed — continuing without sound");

    /* Start at main menu */
    state_set(&sm, STATE_MENU, &app);

    while (!engine_should_close(&engine)) {
        engine_begin_frame(&engine);

        /* Begin UI frame (use logical window size, not framebuffer size, to match mouse coords) */
        ui_begin(&ui, engine.window.window_w, engine.window.window_h,
                 (float)engine.input.mouse_x, (float)engine.input.mouse_y,
                 input_mouse_down(&engine.input, 0),
                 input_mouse_pressed(&engine.input, 0));

        /* Update audio (cooldowns, ambient fades) */
        audio_update(&app.audio, (float)engine.clock.dt);

        /* Update + render current state */
        state_update(&sm, &engine, &ui);
        state_render(&sm, &engine, &ui);

        /* Flush UI */
        ui_end(&ui);

        renderer_end_frame();
        window_swap_buffers(&engine.window);
    }

    /* Cleanup */
    audio_shutdown(&app.audio);
    if (app.game_initialized)
        game_shutdown(&app.game);
    ui_destroy(&ui);
    renderer_shutdown(&renderer);
    engine_shutdown(&engine);
    return 0;
}
