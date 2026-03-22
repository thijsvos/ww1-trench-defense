#include "state_difficulty.h"
#include "app_context.h"
#include "../core/log.h"
#include "../ui/ui_widgets.h"

#include <string.h>

static uint32_t diff_id(const char *tag, int extra) {
    uint32_t hash = 2166136261u;
    while (*tag) {
        hash ^= (uint32_t)(unsigned char)*tag++;
        hash *= 16777619u;
    }
    hash ^= (uint32_t)extra * 2654435761u;
    return hash;
}

static void diff_enter(void *ctx) {
    AppContext *app = (AppContext *)ctx;
    app->difficulty = DIFF_REGULAR;
    LOG_INFO("Entered difficulty select");
}

static void diff_exit(void *ctx) { (void)ctx; }

static void diff_update(void *ctx, Engine *engine, UIContext *ui) {
    (void)ctx; (void)engine; (void)ui;
}

static void diff_render(void *ctx, Engine *engine, UIContext *ui) {
    AppContext *app = (AppContext *)ctx;
    (void)engine;
    float sw = (float)ui->screen_w;
    float sh = (float)ui->screen_h;

    ui_draw_rect(ui, 0, 0, sw, sh, vec4(0.08f, 0.06f, 0.05f, 1.0f));

    /* Title */
    {
        const char *title = "CHOOSE YOUR FATE";
        float scale = 2.5f;
        float char_w = 5.0f * scale + scale;
        float text_w = (float)strlen(title) * char_w;
        ui_label(ui, (sw - text_w) / 2.0f, 50.0f, title,
                 vec4(0.85f, 0.75f, 0.50f, 1.0f), scale);
    }

    static const char *diff_names[DIFF_COUNT] = {
        "Fresh Recruit", "Regular Tommy", "Trench Veteran", "Kaiserschlacht"
    };
    static const char *diff_desc[DIFF_COUNT] = {
        "New to the trenches",
        "Standard Great War",
        "Seasoned soldiers only",
        "God help you"
    };
    static const char *diff_line1[DIFF_COUNT] = {
        "+50% gold & lives",
        "Standard resources",
        "-25% gold & lives",
        "Half all resources"
    };
    static const char *diff_line2[DIFF_COUNT] = {
        "Enemies weakened",
        "Normal enemies",
        "Tougher & faster",
        "Brutal & relentless"
    };

    float card_w = 280.0f;
    float card_h = 150.0f;
    float spacing = 16.0f;
    float total_w = 2.0f * card_w + spacing;
    float start_x = (sw - total_w) / 2.0f;
    float row1_y = 110.0f;
    float row2_y = row1_y + card_h + spacing;

    Vec4 diff_colors[DIFF_COUNT] = {
        vec4(0.20f, 0.35f, 0.20f, 1.0f),
        vec4(0.30f, 0.28f, 0.20f, 1.0f),
        vec4(0.40f, 0.22f, 0.12f, 1.0f),
        vec4(0.45f, 0.10f, 0.10f, 1.0f),
    };

    for (int d = 0; d < DIFF_COUNT; d++) {
        int col = d % 2;
        int row = d / 2;
        float cx = start_x + (float)col * (card_w + spacing);
        float cy = (row == 0) ? row1_y : row2_y;

        bool selected = (app->difficulty == (Difficulty)d);
        bool hovered = (ui->mouse_x >= cx && ui->mouse_x < cx + card_w &&
                        ui->mouse_y >= cy && ui->mouse_y < cy + card_h);

        /* Selection border */
        if (selected) {
            ui_draw_rect(ui, cx - 3, cy - 3, card_w + 6, card_h + 6,
                         vec4(0.85f, 0.72f, 0.30f, 1.0f));
        }

        /* Card background */
        Vec4 bg = diff_colors[d];
        if (hovered) {
            bg.x += 0.06f; bg.y += 0.06f; bg.z += 0.06f;
        }
        ui_draw_rect(ui, cx, cy, card_w, card_h, bg);

        /* Difficulty name */
        float name_scale = 2.0f;
        float name_w = (float)strlen(diff_names[d]) * 6.0f * name_scale;
        ui_label(ui, cx + (card_w - name_w) / 2.0f, cy + 12.0f,
                 diff_names[d], vec4(1.0f, 0.95f, 0.85f, 1.0f), name_scale);

        /* Decorative line */
        ui_draw_rect(ui, cx + 20.0f, cy + 36.0f, card_w - 40.0f, 1.0f,
                     vec4(0.6f, 0.5f, 0.3f, 0.4f));

        /* Flavor text */
        float desc_scale = 1.3f;
        float desc_w = (float)strlen(diff_desc[d]) * 6.0f * desc_scale;
        ui_label(ui, cx + (card_w - desc_w) / 2.0f, cy + 45.0f,
                 diff_desc[d], vec4(0.75f, 0.70f, 0.60f, 1.0f), desc_scale);

        /* Stats detail — two short lines */
        float det_scale = 1.2f;
        float l1w = (float)strlen(diff_line1[d]) * 6.0f * det_scale;
        ui_label(ui, cx + (card_w - l1w) / 2.0f, cy + 68.0f,
                 diff_line1[d], vec4(0.55f, 0.52f, 0.45f, 1.0f), det_scale);
        float l2w = (float)strlen(diff_line2[d]) * 6.0f * det_scale;
        ui_label(ui, cx + (card_w - l2w) / 2.0f, cy + 84.0f,
                 diff_line2[d], vec4(0.55f, 0.52f, 0.45f, 1.0f), det_scale);

        /* Danger markers (1-4) */
        float mark_x = cx + (card_w - (float)(d + 1) * 16.0f) / 2.0f;
        for (int s = 0; s <= d; s++) {
            ui_label(ui, mark_x + (float)s * 16.0f, cy + card_h - 32.0f,
                     "!", vec4(0.7f, 0.15f, 0.10f, 0.8f), 2.0f);
        }

        /* Click to select */
        if (ui->mouse_pressed && hovered) {
            app->difficulty = (Difficulty)d;
        }
    }

    /* Continue button */
    float btn_w = 220.0f, btn_h = 48.0f;
    float btn_x = (sw - btn_w) / 2.0f;
    float btn_y = row2_y + card_h + 24.0f;
    Vec4 btn_text = vec4(1.0f, 1.0f, 1.0f, 1.0f);

    if (ui_button(ui, diff_id("continue", 0), btn_x, btn_y, btn_w, btn_h,
                  "CONTINUE", vec4(0.28f, 0.32f, 0.22f, 1.0f), btn_text)) {
        state_set(app->sm, STATE_LEVEL_SELECT, app);
    }

    btn_y += btn_h + 12.0f;
    if (ui_button(ui, diff_id("back", 0), btn_x, btn_y, btn_w, btn_h,
                  "BACK", vec4(0.35f, 0.18f, 0.15f, 1.0f), btn_text)) {
        state_set(app->sm, STATE_MENU, app);
    }
}

State state_difficulty_create(void) {
    State s;
    memset(&s, 0, sizeof(s));
    s.id = STATE_DIFFICULTY;
    s.enter = diff_enter;
    s.exit = diff_exit;
    s.update = diff_update;
    s.render = diff_render;
    return s;
}
