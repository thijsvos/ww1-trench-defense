#include "state_gameover.h"
#include "app_context.h"
#include "state_levelselect.h"
#include "../core/log.h"
#include "../ui/ui_widgets.h"
#include "../render/renderer.h"

#include <string.h>
#include <stdio.h>

static uint32_t go_id(const char *tag) {
    uint32_t hash = 2166136261u;
    while (*tag) {
        hash ^= (uint32_t)(unsigned char)*tag++;
        hash *= 16777619u;
    }
    return hash;
}

/* Difficulty score multipliers */
static const float DIFF_SCORE_MULT[] = { 1.0f, 1.5f, 2.0f, 3.0f };
static const char *DIFF_SCORE_NAMES[] = { "x1.0", "x1.5", "x2.0", "x3.0" };

static void gameover_enter(void *ctx) {
    AppContext *app = (AppContext *)ctx;

    /* Apply difficulty score multiplier */
    app->go_score = (int)(app->go_score * DIFF_SCORE_MULT[app->difficulty]);

    /* Unlock next level on victory */
    if (app->go_victory && app->selected_level >= 0) {
        int next = app->selected_level + 1;
        if (next < MAX_LEVELS && !app->unlocked[next]) {
            app->unlocked[next] = true;
            LOG_INFO("Unlocked level %d", next + 1);
        }
    }

    LOG_INFO("Game over: %s (kills: %d, score: %d)",
             app->go_victory ? "VICTORY" : "DEFEAT",
             app->go_total_kills, app->go_score);

    /* Final level victory → campaign ending screen */
    if (app->go_victory && app->selected_level == MAX_LEVELS - 1) {
        state_set(app->sm, STATE_CAMPAIGN_VICTORY, app);
    }
}

static void gameover_exit(void *ctx) { (void)ctx; }
static void gameover_update(void *ctx, Engine *engine, UIContext *ui) {
    (void)ctx; (void)engine; (void)ui;
}

static void gameover_render(void *ctx, Engine *engine, UIContext *ui) {
    AppContext *app = (AppContext *)ctx;
    (void)engine;
    float sw = (float)ui->screen_w;
    float sh = (float)ui->screen_h;

    /* Render game world underneath */
    if (app->game_initialized) {
        renderer_begin_frame();
        game_render(&app->game, ui);
    }

    ui_draw_rect(ui, 0, 0, sw, sh, vec4(0.0f, 0.0f, 0.0f, 0.70f));

    float panel_w = 380.0f, panel_h = 300.0f;
    float px = (sw - panel_w) / 2.0f;
    float py = (sh - panel_h) / 2.0f;
    ui_panel(ui, px, py, panel_w, panel_h,
             vec4(0.12f, 0.10f, 0.08f, 0.95f));

    /* Title */
    {
        const char *title = app->go_victory ? "VICTORY!" : "DEFEAT!";
        Vec4 title_col = app->go_victory
            ? vec4(0.40f, 0.90f, 0.40f, 1.0f)
            : vec4(0.90f, 0.25f, 0.25f, 1.0f);
        float scale = 3.0f;
        float char_w = 5.0f * scale + scale;
        float text_w = (float)strlen(title) * char_w;
        ui_label(ui, px + (panel_w - text_w) / 2.0f, py + 16.0f,
                 title, title_col, scale);
    }

    /* Stats */
    float stat_y = py + 60.0f, stat_x = px + 30.0f;
    Vec4 stat_col = vec4(0.80f, 0.80f, 0.80f, 1.0f);
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "Waves survived: %d / %d",
                 app->go_waves_survived, 20);
        ui_label(ui, stat_x, stat_y, buf, stat_col, 2.0f);
        stat_y += 26.0f;

        snprintf(buf, sizeof(buf), "Enemies killed: %d", app->go_total_kills);
        ui_label(ui, stat_x, stat_y, buf, stat_col, 2.0f);
        stat_y += 26.0f;

        snprintf(buf, sizeof(buf), "Difficulty bonus: %s",
                 DIFF_SCORE_NAMES[app->difficulty]);
        ui_label(ui, stat_x, stat_y, buf,
                 vec4(0.65f, 0.60f, 0.50f, 1.0f), 2.0f);
        stat_y += 26.0f;

        snprintf(buf, sizeof(buf), "Final score: %d", app->go_score);
        ui_label(ui, stat_x, stat_y, buf,
                 vec4(1.0f, 0.85f, 0.30f, 1.0f), 2.0f);
    }

    /* Buttons */
    Vec4 btn_text = vec4(1.0f, 1.0f, 1.0f, 1.0f);

    if (app->go_victory) {
        /* Three buttons: NEXT MISSION / RETRY / MENU */
        float btn_w = 140.0f, btn_h = 36.0f;
        float btn_y = py + panel_h - btn_h - 16.0f;
        float gap = 12.0f;
        float total = 3.0f * btn_w + 2.0f * gap;
        float bx = px + (panel_w - total) / 2.0f;

        /* NEXT MISSION (only if there IS a next level) */
        bool has_next = (app->selected_level >= 0 &&
                         app->selected_level < MAX_LEVELS - 1);
        if (has_next) {
            if (ui_button(ui, go_id("next"), bx, btn_y, btn_w, btn_h,
                          "NEXT",
                          vec4(0.25f, 0.40f, 0.25f, 1.0f), btn_text)) {
                audio_play(&app->audio, SFX_UI_CLICK);
                audio_stop_ambient(&app->audio);
                game_shutdown(&app->game);
                app->game_initialized = false;
                app->selected_level++;
                snprintf(app->level_path, sizeof(app->level_path),
                         "%s", levelselect_get_path(app->selected_level));
                state_set(app->sm, STATE_BRIEFING, app);
            }
        } else {
            /* Last level — show "ALL CLEAR" greyed out */
            ui_draw_rect(ui, bx, btn_y, btn_w, btn_h,
                         vec4(0.15f, 0.15f, 0.12f, 1.0f));
            float ac_w = (float)strlen("ALL CLEAR") * 12.0f;
            ui_label(ui, bx + (btn_w - ac_w) / 2.0f,
                     btn_y + (btn_h - 14.0f) / 2.0f,
                     "ALL CLEAR", vec4(0.4f, 0.4f, 0.3f, 1.0f), 2.0f);
        }
        bx += btn_w + gap;

        if (ui_button(ui, go_id("retry"), bx, btn_y, btn_w, btn_h,
                      "RETRY", vec4(0.30f, 0.28f, 0.20f, 1.0f), btn_text)) {
            audio_play(&app->audio, SFX_UI_CLICK);
            game_shutdown(&app->game);
            app->game_initialized = false;
            state_set(app->sm, STATE_PLAY, app);
        }
        bx += btn_w + gap;

        if (ui_button(ui, go_id("menu"), bx, btn_y, btn_w, btn_h,
                      "MENU", vec4(0.40f, 0.18f, 0.15f, 1.0f), btn_text)) {
            audio_play(&app->audio, SFX_UI_CLICK);
            audio_stop_ambient(&app->audio);
            game_shutdown(&app->game);
            app->game_initialized = false;
            state_set(app->sm, STATE_MENU, app);
        }
    } else {
        /* Defeat: RETRY / MENU */
        float btn_w = 160.0f, btn_h = 38.0f;
        float btn_y = py + panel_h - btn_h - 16.0f;

        float retry_x = px + (panel_w / 2.0f - btn_w) / 2.0f;
        if (ui_button(ui, go_id("retry"), retry_x, btn_y, btn_w, btn_h,
                      "RETRY", vec4(0.25f, 0.35f, 0.25f, 1.0f), btn_text)) {
            audio_play(&app->audio, SFX_UI_CLICK);
            game_shutdown(&app->game);
            app->game_initialized = false;
            state_set(app->sm, STATE_PLAY, app);
        }

        float menu_x = px + panel_w / 2.0f + (panel_w / 2.0f - btn_w) / 2.0f;
        if (ui_button(ui, go_id("menu"), menu_x, btn_y, btn_w, btn_h,
                      "MENU", vec4(0.40f, 0.18f, 0.15f, 1.0f), btn_text)) {
            audio_play(&app->audio, SFX_UI_CLICK);
            audio_stop_ambient(&app->audio);
            game_shutdown(&app->game);
            app->game_initialized = false;
            state_set(app->sm, STATE_MENU, app);
        }
    }
}

State state_gameover_create(void) {
    State s;
    memset(&s, 0, sizeof(s));
    s.id = STATE_GAME_OVER;
    s.enter = gameover_enter;
    s.exit = gameover_exit;
    s.update = gameover_update;
    s.render = gameover_render;
    return s;
}
