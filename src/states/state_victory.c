#include "state_victory.h"
#include "app_context.h"
#include "../core/log.h"
#include "../ui/ui_widgets.h"
#include "../render/renderer.h"

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <GLFW/glfw3.h>

static float enter_time;

static const char *DIFF_DISPLAY_NAMES[] = {
    "Fresh Recruit", "Regular Tommy", "Trench Veteran", "Kaiserschlacht"
};

static uint32_t vic_id(const char *tag) {
    uint32_t hash = 2166136261u;
    while (*tag) {
        hash ^= (uint32_t)(unsigned char)*tag++;
        hash *= 16777619u;
    }
    return hash;
}

static void victory_enter(void *ctx) {
    (void)ctx;
    enter_time = (float)glfwGetTime();
    LOG_INFO("Entered campaign victory screen");
}

static void victory_exit(void *ctx) { (void)ctx; }

static void victory_update(void *ctx, Engine *engine, UIContext *ui) {
    (void)ctx; (void)engine; (void)ui;
}

/* Fade-in helper: returns 0..1 based on elapsed time since enter */
static float fade_in(float delay, float duration, float elapsed) {
    float t = (elapsed - delay) / duration;
    if (t < 0.0f) return 0.0f;
    if (t > 1.0f) return 1.0f;
    return t;
}

static void victory_render(void *ctx, Engine *engine, UIContext *ui) {
    AppContext *app = (AppContext *)ctx;
    (void)engine;
    float sw = (float)ui->screen_w;
    float sh = (float)ui->screen_h;
    float time = (float)glfwGetTime();
    float elapsed = time - enter_time;

    /* === Dark atmospheric background === */
    ui_draw_rect(ui, 0, 0, sw, sh, vec4(0.05f, 0.04f, 0.03f, 1.0f));

    /* Subtle horizontal document lines */
    for (float y = 0; y < sh; y += 20.0f) {
        ui_draw_rect(ui, 0, y, sw, 1.0f, vec4(0.07f, 0.06f, 0.05f, 0.4f));
    }

    /* === Horizon silhouette (quiet, no explosions) === */
    float hz = sh * 0.35f;
    ui_draw_rect(ui, 0, hz, sw, sh - hz, vec4(0.06f, 0.05f, 0.04f, 1.0f));

    /* Ruined buildings on horizon — the war is over, everything is still */
    ui_draw_rect(ui, sw * 0.10f, hz - 20.0f, 10.0f, 20.0f, vec4(0.08f, 0.07f, 0.05f, 0.8f));
    ui_draw_rect(ui, sw * 0.10f, hz - 26.0f, 4.0f, 6.0f, vec4(0.08f, 0.07f, 0.05f, 0.7f));
    ui_draw_rect(ui, sw * 0.25f, hz - 14.0f, 16.0f, 14.0f, vec4(0.07f, 0.06f, 0.04f, 0.75f));
    ui_draw_rect(ui, sw * 0.65f, hz - 18.0f, 12.0f, 18.0f, vec4(0.08f, 0.07f, 0.05f, 0.8f));
    ui_draw_rect(ui, sw * 0.85f, hz - 12.0f, 10.0f, 12.0f, vec4(0.07f, 0.06f, 0.04f, 0.7f));

    /* Dead trees */
    ui_draw_rect(ui, sw * 0.40f, hz - 30.0f, 2.0f, 30.0f, vec4(0.08f, 0.06f, 0.04f, 0.7f));
    ui_draw_rect(ui, sw * 0.40f - 4.0f, hz - 24.0f, 4.0f, 2.0f, vec4(0.08f, 0.06f, 0.04f, 0.6f));
    ui_draw_rect(ui, sw * 0.55f, hz - 22.0f, 2.0f, 22.0f, vec4(0.07f, 0.05f, 0.04f, 0.65f));

    /* === Ceasefire: explosions that fade out and stop === */
    /* During the first few seconds, distant flashes die away to silence */
    if (elapsed < 4.0f) {
        float fade_out = 1.0f - elapsed / 4.0f;
        for (int e = 0; e < 5; e++) {
            float ex = sw * (0.1f + (float)e * 0.2f);
            float speed = 2.0f + (float)e * 0.8f;
            float offset = (float)e * 2.3f;
            float cycle = fmodf(time * speed + offset, 3.0f);

            if (cycle < 0.8f) {
                float t = cycle / 0.8f;
                float flash = t < 0.2f ? t / 0.2f : 1.0f - (t - 0.2f) / 0.8f;
                float a = flash * fade_out * 0.5f;
                float ey = hz - 2.0f;
                float radius = 4.0f + t * 10.0f;

                for (int p = 0; p < 10; p++) {
                    float angle = (float)p * 0.628f + (float)e;
                    float dist = (0.3f + (float)(p % 4) * 0.2f) * radius;
                    float px = ex + cosf(angle) * dist;
                    float py = ey + sinf(angle) * dist * 0.4f;
                    float r = a * (0.9f - dist / radius * 0.3f);
                    float g = a * (0.5f - dist / radius * 0.3f);
                    if (g < 0.0f) g = 0.0f;
                    ui_draw_rect(ui, px, py, 2.0f, 2.0f, vec4(r, g, 0.0f, a * 0.8f));
                }
            }
        }
    }

    /* === Slowly rising smoke — the battlefield falls silent === */
    for (int s = 0; s < 6; s++) {
        float sx = sw * (0.12f + (float)s * 0.15f);
        float rise_speed = 3.0f + (float)s * 1.5f;
        float sy = hz - fmodf(elapsed * rise_speed + (float)s * 20.0f, hz * 0.6f);
        float smoke_a = 0.06f * (1.0f - sy / hz);
        if (smoke_a < 0.0f) smoke_a = 0.0f;
        float cloud_w = 20.0f + (float)(s % 3) * 15.0f;
        ui_draw_rect(ui, sx, sy, cloud_w, 3.0f, vec4(0.12f, 0.10f, 0.08f, smoke_a));
    }

    /* === Vignette === */
    ui_draw_rect(ui, 0, 0, sw * 0.06f, sh, vec4(0.0f, 0.0f, 0.0f, 0.5f));
    ui_draw_rect(ui, sw * 0.94f, 0, sw * 0.06f, sh, vec4(0.0f, 0.0f, 0.0f, 0.5f));
    ui_draw_rect(ui, 0, 0, sw, sh * 0.04f, vec4(0.0f, 0.0f, 0.0f, 0.6f));
    ui_draw_rect(ui, 0, sh * 0.96f, sw, sh * 0.04f, vec4(0.0f, 0.0f, 0.0f, 0.6f));

    /* ================================================================ */
    /*  Text content — fades in sequentially                            */
    /* ================================================================ */

    /* "THE WAR IS OVER" — gold title */
    {
        float a = fade_in(1.0f, 2.0f, elapsed);
        const char *title = "THE WAR IS OVER";
        float scale = 3.5f;
        float char_w = 6.0f * scale;
        float tw = (float)strlen(title) * char_w;
        ui_label(ui, (sw - tw) / 2.0f, sh * 0.08f, title,
                 vec4(0.92f * a, 0.80f * a, 0.35f * a, a), scale);
    }

    /* "ARMISTICE" subtitle */
    {
        float a = fade_in(2.5f, 1.5f, elapsed);
        const char *sub = "ARMISTICE";
        float scale = 2.5f;
        float char_w = 6.0f * scale;
        float tw = (float)strlen(sub) * char_w;
        ui_label(ui, (sw - tw) / 2.0f, sh * 0.08f + 40.0f, sub,
                 vec4(0.75f * a, 0.68f * a, 0.45f * a, a), scale);
    }

    /* Date */
    {
        float a = fade_in(3.5f, 1.5f, elapsed);
        const char *date = "11th November 1918, 11:00 AM";
        float scale = 1.8f;
        float char_w = 6.0f * scale;
        float tw = (float)strlen(date) * char_w;
        ui_label(ui, (sw - tw) / 2.0f, sh * 0.08f + 72.0f, date,
                 vec4(0.55f * a, 0.50f * a, 0.40f * a, a), scale);
    }

    /* Decorative line */
    {
        float a = fade_in(4.0f, 1.0f, elapsed);
        float line_w = sw * 0.6f;
        ui_draw_rect(ui, (sw - line_w) / 2.0f, sh * 0.08f + 96.0f,
                     line_w, 1.0f, vec4(0.40f * a, 0.32f * a, 0.20f * a, 0.6f * a));
    }

    /* === Armistice text === */
    {
        float a = fade_in(5.0f, 2.0f, elapsed);
        Vec4 tc = vec4(0.70f * a, 0.67f * a, 0.60f * a, a);
        float scale = 1.8f;
        float cpx = 6.0f * scale;
        float base_y = sh * 0.08f + 110.0f;

        static const char *lines[] = {
            "At the eleventh hour of the eleventh day",
            "of the eleventh month, the guns fell silent.",
            "",
            "After four years of unprecedented carnage,",
            "the Great War was over. The world would",
            "never be the same.",
        };
        int line_count = 6;

        for (int i = 0; i < line_count; i++) {
            if (lines[i][0] == '\0') {
                base_y += 10.0f;
            } else {
                float tw = (float)strlen(lines[i]) * cpx;
                ui_label(ui, (sw - tw) / 2.0f, base_y, lines[i], tc, scale);
                base_y += 20.0f;
            }
        }
    }

    /* === Decorative divider === */
    {
        float a = fade_in(6.5f, 1.0f, elapsed);
        float line_w = sw * 0.5f;
        ui_draw_rect(ui, (sw - line_w) / 2.0f, sh * 0.52f,
                     line_w, 1.0f, vec4(0.35f * a, 0.28f * a, 0.18f * a, 0.5f * a));
    }

    /* === Campaign stats === */
    {
        float a = fade_in(7.0f, 1.5f, elapsed);
        Vec4 header_col = vec4(0.85f * a, 0.75f * a, 0.45f * a, a);
        Vec4 stat_col = vec4(0.70f * a, 0.68f * a, 0.60f * a, a);
        float stat_scale = 1.8f;
        float cpx = 6.0f * stat_scale;
        float base_y = sh * 0.55f;

        {
            const char *hdr = "YOUR CAMPAIGN";
            float hw = (float)strlen(hdr) * cpx;
            ui_label(ui, (sw - hw) / 2.0f, base_y, hdr, header_col, stat_scale);
            base_y += 26.0f;
        }

        char buf[64];
        snprintf(buf, sizeof(buf), "Difficulty: %s",
                 DIFF_DISPLAY_NAMES[app->difficulty]);
        {
            float tw = (float)strlen(buf) * cpx;
            ui_label(ui, (sw - tw) / 2.0f, base_y, buf, stat_col, stat_scale);
            base_y += 22.0f;
        }

        snprintf(buf, sizeof(buf), "Enemies defeated: %d", app->go_total_kills);
        {
            float tw = (float)strlen(buf) * cpx;
            ui_label(ui, (sw - tw) / 2.0f, base_y, buf, stat_col, stat_scale);
            base_y += 22.0f;
        }

        snprintf(buf, sizeof(buf), "Final score: %d", app->go_score);
        {
            float tw = (float)strlen(buf) * cpx;
            ui_label(ui, (sw - tw) / 2.0f, base_y, buf,
                     vec4(1.0f * a, 0.85f * a, 0.30f * a, a), stat_scale);
        }
    }

    /* === Historical WW1 closing stats === */
    {
        float a = fade_in(8.5f, 1.5f, elapsed);
        Vec4 header_col = vec4(0.75f * a, 0.65f * a, 0.40f * a, a);
        Vec4 fact_col = vec4(0.55f * a, 0.52f * a, 0.45f * a, a);
        float scale = 1.6f;
        float cpx = 6.0f * scale;
        float base_y = sh * 0.73f;

        {
            const char *hdr = "THE GREAT WAR: 1914-1918";
            float hw = (float)strlen(hdr) * cpx;
            ui_label(ui, (sw - hw) / 2.0f, base_y, hdr, header_col, scale);
            base_y += 22.0f;
        }

        static const char *facts[] = {
            "Total casualties: 40 million",
            "Deaths: 20 million",
            "Duration: 4 years, 3 months",
            "Nations involved: 30+",
        };

        for (int i = 0; i < 4; i++) {
            float tw = (float)strlen(facts[i]) * cpx;
            ui_label(ui, (sw - tw) / 2.0f, base_y, facts[i], fact_col, scale);
            base_y += 20.0f;
        }
    }

    /* === RETURN TO MENU button — appears after text === */
    {
        float a = fade_in(10.0f, 1.5f, elapsed);
        if (a > 0.01f) {
            float btn_w = 260.0f, btn_h = 44.0f;
            float btn_x = (sw - btn_w) / 2.0f;
            float btn_y = sh - 60.0f;

            /* Subtle pulsing border */
            float pulse = 0.6f + 0.4f * sinf(time * 2.0f);
            ui_draw_rect(ui, btn_x - 2, btn_y - 2, btn_w + 4, btn_h + 4,
                         vec4(0.50f * pulse * a, 0.42f * pulse * a,
                              0.25f * pulse * a, a));

            if (ui_button(ui, vic_id("menu"), btn_x, btn_y, btn_w, btn_h,
                          "RETURN TO MENU",
                          vec4(0.25f * a, 0.20f * a, 0.12f * a, a),
                          vec4(1.0f * a, 1.0f * a, 1.0f * a, a))) {
                game_shutdown(&app->game);
                app->game_initialized = false;
                state_set(app->sm, STATE_MENU, app);
            }
        }
    }
}

State state_victory_create(void) {
    State s;
    memset(&s, 0, sizeof(s));
    s.id = STATE_CAMPAIGN_VICTORY;
    s.enter = victory_enter;
    s.exit = victory_exit;
    s.update = victory_update;
    s.render = victory_render;
    return s;
}
