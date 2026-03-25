#include "state_pause.h"
#include "app_context.h"
#include "../core/log.h"
#include "../ui/ui_widgets.h"
#include "../render/renderer.h"

#include <string.h>
#include <GLFW/glfw3.h>

static uint32_t pause_id(const char *tag) {
    uint32_t hash = 2166136261u;
    while (*tag) {
        hash ^= (uint32_t)(unsigned char)*tag++;
        hash *= 16777619u;
    }
    return hash;
}

static void pause_enter(void *ctx) {
    AppContext *app = (AppContext *)ctx;
    audio_play(&app->audio, SFX_PAUSE);
    LOG_INFO("Game paused");
}
static void pause_exit(void *ctx) { (void)ctx; }

static void pause_update(void *ctx, Engine *engine, UIContext *ui) {
    AppContext *app = (AppContext *)ctx;
    (void)ui;

    if (input_key_pressed(&engine->input, GLFW_KEY_ESCAPE)) {
        audio_play(&app->audio, SFX_UNPAUSE);
        state_set(app->sm, STATE_PLAY, app);
    }
}

static void pause_render(void *ctx, Engine *engine, UIContext *ui) {
    AppContext *app = (AppContext *)ctx;
    (void)engine;
    float sw = (float)ui->screen_w;
    float sh = (float)ui->screen_h;

    /* Render game world underneath */
    if (app->game_initialized) {
        renderer_begin_frame();
        game_render(&app->game, ui);
    }

    /* Dark overlay */
    ui_draw_rect(ui, 0, 0, sw, sh, vec4(0.0f, 0.0f, 0.0f, 0.65f));

    {
        const char *title = "PAUSED";
        float scale = 3.0f;
        float char_w = 5.0f * scale + scale;
        float text_w = (float)strlen(title) * char_w;
        ui_label(ui, (sw - text_w) / 2.0f, sh * 0.25f, title,
                 vec4(0.90f, 0.85f, 0.70f, 1.0f), scale);
    }

    float btn_w = 220.0f, btn_h = 40.0f;
    float btn_x = (sw - btn_w) / 2.0f;
    float btn_y = sh * 0.42f;
    Vec4 btn_text = vec4(1.0f, 1.0f, 1.0f, 1.0f);

    if (ui_button(ui, pause_id("resume"), btn_x, btn_y, btn_w, btn_h,
                  "RESUME", vec4(0.25f, 0.35f, 0.25f, 1.0f), btn_text)) {
        audio_play(&app->audio, SFX_UNPAUSE);
        state_set(app->sm, STATE_PLAY, app);
    }

    btn_y += btn_h + 12.0f;
    if (ui_button(ui, pause_id("restart"), btn_x, btn_y, btn_w, btn_h,
                  "RESTART", vec4(0.30f, 0.28f, 0.20f, 1.0f), btn_text)) {
        audio_play(&app->audio, SFX_UI_CLICK);
        game_shutdown(&app->game);
        app->game_initialized = false;
        state_set(app->sm, STATE_PLAY, app);
        LOG_INFO("Restart requested");
    }

    btn_y += btn_h + 12.0f;
    if (ui_button(ui, pause_id("quit_menu"), btn_x, btn_y, btn_w, btn_h,
                  "QUIT TO MENU", vec4(0.40f, 0.18f, 0.15f, 1.0f), btn_text)) {
        audio_play(&app->audio, SFX_UI_CLICK);
        audio_stop_ambient(&app->audio);
        game_shutdown(&app->game);
        app->game_initialized = false;
        state_set(app->sm, STATE_MENU, app);
    }
}

State state_pause_create(void) {
    State s;
    memset(&s, 0, sizeof(s));
    s.id = STATE_PAUSE;
    s.enter = pause_enter;
    s.exit = pause_exit;
    s.update = pause_update;
    s.render = pause_render;
    return s;
}
