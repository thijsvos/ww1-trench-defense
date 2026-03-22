#include "state_levelselect.h"
#include "app_context.h"
#include "../core/log.h"
#include "../ui/ui_widgets.h"

#include <string.h>
#include <stdio.h>

static const char *LEVEL_NAMES[MAX_LEVELS] = {
    "1. No Man's Land",
    "2. Gallipoli Beach",
    "3. Verdun Meatgrinder",
    "4. Brusilov's Breakthrough",
    "5. The Kaiserschlacht",
};

static const char *LEVEL_PATHS[MAX_LEVELS] = {
    "assets/levels/level_01.dat",
    "assets/levels/level_02.dat",
    "assets/levels/level_03.dat",
    "assets/levels/level_04.dat",
    "assets/levels/level_05.dat",
};

static uint32_t ls_id(const char *tag, int extra) {
    uint32_t hash = 2166136261u;
    while (*tag) {
        hash ^= (uint32_t)(unsigned char)*tag++;
        hash *= 16777619u;
    }
    hash ^= (uint32_t)extra * 2654435761u;
    return hash;
}

static void levelselect_enter(void *ctx) {
    AppContext *app = (AppContext *)ctx;
    for (int i = 0; i < MAX_LEVELS; i++)
        app->unlocked[i] = true;
    app->selected_level = -1;
    LOG_INFO("Entered level select");
}

static void levelselect_exit(void *ctx) { (void)ctx; }

static void levelselect_update(void *ctx, Engine *engine, UIContext *ui) {
    (void)ctx; (void)engine; (void)ui;
}

static void levelselect_render(void *ctx, Engine *engine, UIContext *ui) {
    AppContext *app = (AppContext *)ctx;
    (void)engine;
    float sw = (float)ui->screen_w;
    float sh = (float)ui->screen_h;

    ui_draw_rect(ui, 0, 0, sw, sh, vec4(0.08f, 0.06f, 0.05f, 1.0f));

    {
        const char *title = "SELECT MISSION";
        float scale = 2.5f;
        float char_w = 5.0f * scale + scale;
        float text_w = (float)strlen(title) * char_w;
        ui_label(ui, (sw - text_w) / 2.0f, 60.0f, title,
                 vec4(0.85f, 0.75f, 0.50f, 1.0f), scale);
    }

    float btn_w = 340.0f, btn_h = 40.0f;
    float btn_x = (sw - btn_w) / 2.0f;
    float start_y = 110.0f;
    Vec4 btn_text = vec4(1.0f, 1.0f, 1.0f, 1.0f);

    for (int i = 0; i < MAX_LEVELS; i++) {
        float by = start_y + (float)i * (btn_h + 12.0f);
        bool unlocked = app->unlocked[i];
        Vec4 bg = unlocked ? vec4(0.25f, 0.30f, 0.22f, 1.0f)
                           : vec4(0.18f, 0.16f, 0.14f, 1.0f);
        Vec4 text_col = unlocked ? btn_text
                                 : vec4(0.40f, 0.38f, 0.35f, 1.0f);
        char label[64];
        if (unlocked)
            snprintf(label, sizeof(label), "%s", LEVEL_NAMES[i]);
        else
            snprintf(label, sizeof(label), "%s [LOCKED]", LEVEL_NAMES[i]);

        if (ui_button(ui, ls_id("level", i), btn_x, by, btn_w, btn_h,
                      label, bg, text_col)) {
            if (unlocked) {
                app->selected_level = i;
                snprintf(app->level_path, sizeof(app->level_path),
                         "%s", LEVEL_PATHS[i]);
                app->game_initialized = false; /* force fresh load */
                state_set(app->sm, STATE_PLAY, app);
                LOG_INFO("Selected level %d: %s", i + 1, LEVEL_NAMES[i]);
            }
        }
    }

    float back_y = start_y + (float)MAX_LEVELS * (btn_h + 12.0f) + 20.0f;
    if (ui_button(ui, ls_id("back", 0), btn_x, back_y, btn_w, btn_h,
                  "BACK", vec4(0.35f, 0.18f, 0.15f, 1.0f), btn_text)) {
        state_set(app->sm, STATE_DIFFICULTY, app);
    }
}

State state_levelselect_create(void) {
    State s;
    memset(&s, 0, sizeof(s));
    s.id = STATE_LEVEL_SELECT;
    s.enter = levelselect_enter;
    s.exit = levelselect_exit;
    s.update = levelselect_update;
    s.render = levelselect_render;
    return s;
}

const char *levelselect_get_path(int level_index) {
    if (level_index < 0 || level_index >= MAX_LEVELS) return NULL;
    return LEVEL_PATHS[level_index];
}
