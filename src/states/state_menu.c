#include "state_menu.h"
#include "app_context.h"
#include "../core/log.h"
#include "../ui/ui_widgets.h"

#include <string.h>
#include <math.h>
#include <GLFW/glfw3.h>

static uint32_t menu_id(const char *tag) {
    uint32_t hash = 2166136261u;
    while (*tag) {
        hash ^= (uint32_t)(unsigned char)*tag++;
        hash *= 16777619u;
    }
    return hash;
}

static void menu_enter(void *ctx) {
    (void)ctx;
    LOG_INFO("Entered main menu");
}

static void menu_exit(void *ctx) {
    (void)ctx;
}

static void menu_update(void *ctx, Engine *engine, UIContext *ui) {
    (void)ctx; (void)engine; (void)ui;
}

static void menu_render(void *ctx, Engine *engine, UIContext *ui) {
    AppContext *app = (AppContext *)ctx;
    (void)engine;
    float sw = (float)ui->screen_w;
    float sh = (float)ui->screen_h;
    float time = (float)glfwGetTime();

    /* === Background gradient === */
    ui_draw_rect(ui, 0, 0, sw, sh * 0.35f, vec4(0.10f, 0.08f, 0.07f, 1.0f));
    ui_draw_rect(ui, 0, sh * 0.35f, sw, sh * 0.1f, vec4(0.14f, 0.11f, 0.08f, 1.0f));
    ui_draw_rect(ui, 0, sh * 0.45f, sw, sh * 0.55f, vec4(0.09f, 0.07f, 0.05f, 1.0f));

    /* Mud texture stripes */
    for (float y = sh * 0.46f; y < sh; y += 12.0f) {
        float v = 0.05f + (y / sh) * 0.04f;
        ui_draw_rect(ui, 0, y, sw, 1.0f, vec4(v + 0.02f, v, v - 0.01f, 0.4f));
    }

    /* === Horizon line details === */
    float hz = sh * 0.43f;

    /* Ruined building silhouettes on the horizon */
    ui_draw_rect(ui, sw * 0.08f, hz - 25.0f, 12.0f, 25.0f, vec4(0.06f, 0.05f, 0.04f, 0.9f));
    ui_draw_rect(ui, sw * 0.08f, hz - 30.0f, 6.0f, 5.0f, vec4(0.06f, 0.05f, 0.04f, 0.9f));
    ui_draw_rect(ui, sw * 0.22f, hz - 18.0f, 20.0f, 18.0f, vec4(0.07f, 0.06f, 0.04f, 0.85f));
    ui_draw_rect(ui, sw * 0.22f, hz - 28.0f, 4.0f, 10.0f, vec4(0.07f, 0.06f, 0.04f, 0.8f));
    ui_draw_rect(ui, sw * 0.70f, hz - 22.0f, 16.0f, 22.0f, vec4(0.06f, 0.05f, 0.04f, 0.9f));
    ui_draw_rect(ui, sw * 0.70f, hz - 32.0f, 8.0f, 10.0f, vec4(0.06f, 0.05f, 0.04f, 0.85f));
    ui_draw_rect(ui, sw * 0.88f, hz - 15.0f, 14.0f, 15.0f, vec4(0.07f, 0.06f, 0.04f, 0.8f));

    /* Dead tree silhouettes */
    ui_draw_rect(ui, sw * 0.35f, hz - 35.0f, 2.0f, 35.0f, vec4(0.08f, 0.06f, 0.04f, 0.8f));
    ui_draw_rect(ui, sw * 0.35f - 5.0f, hz - 28.0f, 5.0f, 2.0f, vec4(0.08f, 0.06f, 0.04f, 0.7f));
    ui_draw_rect(ui, sw * 0.35f + 2.0f, hz - 22.0f, 6.0f, 2.0f, vec4(0.08f, 0.06f, 0.04f, 0.7f));
    ui_draw_rect(ui, sw * 0.55f, hz - 28.0f, 2.0f, 28.0f, vec4(0.07f, 0.05f, 0.04f, 0.75f));
    ui_draw_rect(ui, sw * 0.55f - 4.0f, hz - 20.0f, 4.0f, 2.0f, vec4(0.07f, 0.05f, 0.04f, 0.65f));

    /* Barbed wire across horizon */
    for (float wx = 0; wx < sw; wx += 6.0f) {
        float wobble = sinf(wx * 0.06f + 2.0f) * 3.0f;
        ui_draw_rect(ui, wx, hz + wobble, 4.0f, 1.0f, vec4(0.18f, 0.16f, 0.13f, 0.6f));
    }
    for (float px = 30.0f; px < sw; px += 65.0f) {
        ui_draw_rect(ui, px, hz - 6.0f, 2.0f, 12.0f, vec4(0.16f, 0.13f, 0.10f, 0.7f));
    }

    /* === Explosions — scattered 1-2px particles in circular bursts === */
    for (int e = 0; e < 7; e++) {
        float ex = sw * (0.07f + (float)e * 0.13f);
        float speed = 1.4f + (float)e * 0.7f;
        float offset = (float)e * 2.9f;
        float cycle = fmodf(time * speed + offset, 4.0f);

        if (cycle < 1.2f) {
            float t = cycle / 1.2f; /* 0..1 over the flash */
            float fade = t < 0.3f ? t / 0.3f : 1.0f - (t - 0.3f) / 0.7f;
            float ey = hz - 4.0f;
            float radius = 6.0f + t * 18.0f;

            /* Scatter particles in a circle */
            for (int p = 0; p < 20; p++) {
                float angle = (float)p * 0.314f + (float)e;
                float dist = (0.2f + (float)(p % 5) * 0.2f) * radius;
                float px = ex + cosf(angle) * dist;
                float py = ey + sinf(angle) * dist * 0.5f; /* squash vertically */
                float sz = 1.0f + (1.0f - (float)(p % 5) * 0.2f) * 3.0f;

                /* Color: white core → yellow → orange → red at edges */
                float d_frac = dist / radius;
                float r = fade * (1.0f - d_frac * 0.3f);
                float g = fade * (0.8f - d_frac * 0.5f);
                float b = fade * (0.3f - d_frac * 0.3f);
                if (g < 0.0f) g = 0.0f;
                if (b < 0.0f) b = 0.0f;
                float a = fade * (0.8f - d_frac * 0.5f);
                if (a < 0.0f) a = 0.0f;

                ui_draw_rect(ui, px, py, sz, sz, vec4(r, g, b, a));
            }

            /* Smoke rising from blast */
            for (int s = 0; s < 6; s++) {
                float sa = (float)s * 1.05f + (float)e * 0.5f;
                float rise = t * 25.0f * (1.0f + (float)s * 0.3f);
                float drift = sinf(sa) * 8.0f;
                float smoke_a = fade * 0.25f * (1.0f - t * 0.5f);
                float ssz = 3.0f + (float)s * 2.0f;
                ui_draw_rect(ui, ex + drift - ssz * 0.5f, ey - rise - ssz * 0.5f,
                             ssz, ssz, vec4(0.15f, 0.12f, 0.10f, smoke_a));
            }

            /* Ground illumination */
            float glow = fade * 0.2f;
            ui_draw_rect(ui, ex - 30.0f, ey + 2.0f, 60.0f, 8.0f,
                         vec4(glow * 0.8f, glow * 0.3f, glow * 0.05f, glow));
        }
    }

    /* === Drifting smoke clouds === */
    for (int s = 0; s < 8; s++) {
        float sx = fmodf(time * (5.0f + (float)s * 2.5f) + (float)s * 160.0f, sw + 120.0f) - 60.0f;
        float sy = sh * 0.36f + (float)s * 5.0f + sinf(time * 0.3f + (float)s) * 3.0f;
        float cloud_w = 40.0f + (float)(s % 3) * 25.0f;
        ui_draw_rect(ui, sx, sy, cloud_w, 3.0f + (float)(s % 2) * 2.0f,
                     vec4(0.14f, 0.12f, 0.09f, 0.12f));
    }

    /* === Searchlight beams sweeping the sky === */
    for (int b = 0; b < 2; b++) {
        float angle = sinf(time * 0.4f + (float)b * 3.14f) * 0.3f;
        float base_x = sw * (0.3f + (float)b * 0.4f);
        float base_y = hz + 5.0f;
        for (int seg = 0; seg < 15; seg++) {
            float t = (float)seg / 15.0f;
            float beam_x = base_x + angle * t * 200.0f - 1.0f;
            float beam_y = base_y - t * hz * 0.8f;
            float beam_a = 0.04f * (1.0f - t);
            ui_draw_rect(ui, beam_x, beam_y, 2.0f + t * 8.0f, 4.0f,
                         vec4(0.4f, 0.4f, 0.35f, beam_a));
        }
    }

    /* === Star shells / flares falling slowly === */
    for (int f = 0; f < 3; f++) {
        float flare_cycle = fmodf(time * 0.3f + (float)f * 1.8f, 3.0f);
        if (flare_cycle < 2.5f) {
            float fx = sw * (0.2f + (float)f * 0.3f) + sinf(time * 0.5f + (float)f) * 15.0f;
            float fy_f = sh * 0.08f + flare_cycle * sh * 0.12f;
            float flare_a = 1.0f - flare_cycle / 2.5f;
            /* Bright flare point */
            ui_draw_rect(ui, fx - 1.0f, fy_f - 1.0f, 3.0f, 3.0f,
                         vec4(1.0f, 0.95f, 0.7f, 0.7f * flare_a));
            /* Glow around flare */
            ui_draw_rect(ui, fx - 6.0f, fy_f - 4.0f, 12.0f, 8.0f,
                         vec4(0.5f, 0.45f, 0.3f, 0.12f * flare_a));
            /* Dripping sparks trail */
            for (int sp = 0; sp < 4; sp++) {
                float sy = fy_f - 3.0f - (float)sp * 4.0f;
                float spark_a = flare_a * (0.5f - (float)sp * 0.1f);
                if (spark_a > 0.0f)
                    ui_draw_rect(ui, fx + sinf((float)sp * 2.0f + time) * 3.0f, sy,
                                 1.0f, 1.0f, vec4(1.0f, 0.8f, 0.4f, spark_a));
            }
        }
    }

    /* === Red Baron's Fokker Dr.I triplane flyby === */
    {
        /* Flies across every ~25 seconds, takes ~12 seconds to cross — slow and dramatic */
        float cycle = fmodf(time, 25.0f);
        if (cycle < 12.0f) {
            float t = cycle / 12.0f;
            float px = -80.0f + t * (sw + 160.0f);
            float py = sh * 0.10f + sinf(t * 3.14f) * 12.0f; /* gentle arc */
            /* Slight banking tilt */
            float tilt = cosf(t * 6.28f) * 2.0f;

            Vec4 red      = vec4(0.72f, 0.10f, 0.06f, 1.0f);
            Vec4 lite_red  = vec4(0.85f, 0.18f, 0.10f, 1.0f);
            Vec4 dark_red  = vec4(0.50f, 0.06f, 0.03f, 1.0f);
            Vec4 shadow    = vec4(0.35f, 0.04f, 0.02f, 1.0f);
            Vec4 black     = vec4(0.10f, 0.08f, 0.06f, 1.0f);
            Vec4 strut_col = vec4(0.25f, 0.18f, 0.12f, 1.0f);
            Vec4 wire_col  = vec4(0.30f, 0.28f, 0.25f, 0.6f);

            /* === Fuselage — rounded shape, tapers front and back === */
            /* Main body */
            ui_draw_rect(ui, px + 10.0f, py + tilt, 34.0f, 8.0f, red);
            /* Top highlight (fabric sheen) */
            ui_draw_rect(ui, px + 12.0f, py + tilt, 30.0f, 2.0f, lite_red);
            /* Bottom shadow */
            ui_draw_rect(ui, px + 12.0f, py + 6.0f + tilt, 30.0f, 2.0f, dark_red);
            /* Nose — short taper into cowling */
            ui_draw_rect(ui, px + 44.0f, py + 1.0f + tilt, 6.0f, 6.0f, red);
            ui_draw_rect(ui, px + 48.0f, py + 2.0f + tilt, 3.0f, 4.0f, dark_red);
            /* Tail taper */
            ui_draw_rect(ui, px + 4.0f, py + 2.0f + tilt, 8.0f, 4.0f, dark_red);
            ui_draw_rect(ui, px + 0.0f, py + 3.0f + tilt, 5.0f, 2.0f, shadow);

            /* === Engine cowling (metal, darker) === */
            ui_draw_rect(ui, px + 50.0f, py + 1.0f + tilt, 5.0f, 6.0f, vec4(0.20f, 0.18f, 0.16f, 1.0f));
            ui_draw_rect(ui, px + 50.0f, py + 1.0f + tilt, 5.0f, 2.0f, vec4(0.28f, 0.25f, 0.22f, 1.0f));
            /* Exhaust pipes */
            ui_draw_rect(ui, px + 51.0f, py + 7.0f + tilt, 3.0f, 1.0f, black);

            /* === Propeller (fast spinning disc blur) === */
            float prop_phase = fmodf(time * 30.0f, 3.0f);
            float prop_a = 0.5f;
            if (prop_phase < 1.0f) {
                ui_draw_rect(ui, px + 55.0f, py - 6.0f + tilt, 2.0f, 20.0f,
                             vec4(0.3f, 0.25f, 0.18f, prop_a));
            } else if (prop_phase < 2.0f) {
                ui_draw_rect(ui, px + 53.0f, py + 3.0f + tilt, 6.0f, 2.0f,
                             vec4(0.3f, 0.25f, 0.18f, prop_a));
            } else {
                ui_draw_rect(ui, px + 54.0f, py - 4.0f + tilt, 2.0f, 8.0f,
                             vec4(0.3f, 0.25f, 0.18f, prop_a));
                ui_draw_rect(ui, px + 53.0f, py + 4.0f + tilt, 4.0f, 2.0f,
                             vec4(0.3f, 0.25f, 0.18f, prop_a * 0.7f));
            }
            /* Prop hub */
            ui_draw_rect(ui, px + 54.0f, py + 3.0f + tilt, 2.0f, 2.0f, black);

            /* === Three wings (TRIPLANE — the Dr.I's defining feature) === */
            /* Top wing (longest, above fuselage) */
            ui_draw_rect(ui, px + 16.0f, py - 12.0f + tilt, 32.0f, 4.0f, red);
            ui_draw_rect(ui, px + 16.0f, py - 12.0f + tilt, 32.0f, 1.0f, lite_red);
            ui_draw_rect(ui, px + 16.0f, py - 9.0f + tilt, 32.0f, 1.0f, dark_red);
            /* Middle wing (at fuselage, slightly shorter) */
            ui_draw_rect(ui, px + 18.0f, py - 2.0f + tilt, 28.0f, 4.0f, red);
            ui_draw_rect(ui, px + 18.0f, py + 1.0f + tilt, 28.0f, 1.0f, dark_red);
            /* Bottom wing (below fuselage) */
            ui_draw_rect(ui, px + 16.0f, py + 8.0f + tilt, 32.0f, 4.0f, dark_red);
            ui_draw_rect(ui, px + 16.0f, py + 11.0f + tilt, 32.0f, 1.0f, shadow);

            /* === Interplane struts (wooden, connecting all 3 wings) === */
            /* Left pair */
            ui_draw_rect(ui, px + 22.0f, py - 11.0f + tilt, 2.0f, 22.0f, strut_col);
            /* Right pair */
            ui_draw_rect(ui, px + 40.0f, py - 11.0f + tilt, 2.0f, 22.0f, strut_col);

            /* Rigging wires (thin diagonal lines between struts) */
            for (int w = 0; w < 3; w++) {
                float wy = py - 10.0f + (float)w * 8.0f + tilt;
                ui_draw_rect(ui, px + 23.0f, wy, 18.0f, 1.0f, wire_col);
            }

            /* === Tail section === */
            /* Horizontal stabilizer */
            ui_draw_rect(ui, px - 4.0f, py + 1.0f + tilt, 12.0f, 3.0f, dark_red);
            ui_draw_rect(ui, px - 4.0f, py + 1.0f + tilt, 12.0f, 1.0f, red);
            /* Vertical fin (rudder) */
            ui_draw_rect(ui, px - 2.0f, py - 6.0f + tilt, 4.0f, 8.0f, red);
            ui_draw_rect(ui, px - 2.0f, py - 6.0f + tilt, 1.0f, 8.0f, lite_red);
            /* Rudder hinge line */
            ui_draw_rect(ui, px + 0.0f, py - 5.0f + tilt, 1.0f, 7.0f, dark_red);

            /* === Iron Cross markings === */
            /* On fuselage */
            ui_draw_rect(ui, px + 28.0f, py + 2.0f + tilt, 7.0f, 1.0f, vec4(1.0f, 1.0f, 1.0f, 0.8f));
            ui_draw_rect(ui, px + 30.0f, py + 0.0f + tilt, 1.0f, 5.0f, vec4(1.0f, 1.0f, 1.0f, 0.8f));
            ui_draw_rect(ui, px + 29.0f, py + 2.0f + tilt, 5.0f, 1.0f, black);
            ui_draw_rect(ui, px + 30.0f, py + 1.0f + tilt, 1.0f, 3.0f, black);
            /* On top wing */
            ui_draw_rect(ui, px + 30.0f, py - 11.0f + tilt, 5.0f, 1.0f, vec4(1.0f, 1.0f, 1.0f, 0.6f));
            ui_draw_rect(ui, px + 32.0f, py - 12.0f + tilt, 1.0f, 3.0f, vec4(1.0f, 1.0f, 1.0f, 0.6f));

            /* === Landing gear === */
            /* V-struts from fuselage to axle */
            ui_draw_rect(ui, px + 24.0f, py + 12.0f + tilt, 1.0f, 6.0f, strut_col);
            ui_draw_rect(ui, px + 39.0f, py + 12.0f + tilt, 1.0f, 6.0f, strut_col);
            /* Cross brace */
            ui_draw_rect(ui, px + 25.0f, py + 15.0f + tilt, 14.0f, 1.0f, wire_col);
            /* Axle */
            ui_draw_rect(ui, px + 22.0f, py + 17.0f + tilt, 20.0f, 1.0f, strut_col);
            /* Wheels (spoked circles) */
            ui_draw_rect(ui, px + 22.0f, py + 16.0f + tilt, 4.0f, 4.0f, vec4(0.18f, 0.15f, 0.12f, 1.0f));
            ui_draw_rect(ui, px + 23.0f, py + 17.0f + tilt, 2.0f, 2.0f, vec4(0.25f, 0.20f, 0.15f, 1.0f));
            ui_draw_rect(ui, px + 37.0f, py + 16.0f + tilt, 4.0f, 4.0f, vec4(0.18f, 0.15f, 0.12f, 1.0f));
            ui_draw_rect(ui, px + 38.0f, py + 17.0f + tilt, 2.0f, 2.0f, vec4(0.25f, 0.20f, 0.15f, 1.0f));
            /* Tail skid */
            ui_draw_rect(ui, px + 2.0f, py + 4.0f + tilt, 1.0f, 3.0f, strut_col);
        }
    }

    /* === Vignette === */
    ui_draw_rect(ui, 0, 0, sw * 0.08f, sh, vec4(0.0f, 0.0f, 0.0f, 0.4f));
    ui_draw_rect(ui, sw * 0.92f, 0, sw * 0.08f, sh, vec4(0.0f, 0.0f, 0.0f, 0.4f));
    ui_draw_rect(ui, 0, 0, sw, sh * 0.06f, vec4(0.0f, 0.0f, 0.0f, 0.5f));
    ui_draw_rect(ui, 0, sh * 0.94f, sw, sh * 0.06f, vec4(0.0f, 0.0f, 0.0f, 0.5f));

    /* === Title panel background (semi-transparent for readability) === */
    float title_panel_y = sh * 0.18f;
    ui_draw_rect(ui, sw * 0.15f, title_panel_y - 10.0f,
                 sw * 0.7f, 70.0f,
                 vec4(0.0f, 0.0f, 0.0f, 0.45f));

    /* === Title === */
    {
        const char *title = "WW1: TRENCH DEFENSE";
        float scale = 3.0f;
        float char_w = 5.0f * scale + scale;
        float text_w = (float)strlen(title) * char_w;
        ui_label(ui, (sw - text_w) / 2.0f, title_panel_y,
                 title, vec4(0.92f, 0.80f, 0.45f, 1.0f), scale);
    }
    {
        const char *sub = "A Great War Tower Defense";
        float scale = 1.5f;
        float char_w = 5.0f * scale + scale;
        float text_w = (float)strlen(sub) * char_w;
        ui_label(ui, (sw - text_w) / 2.0f, title_panel_y + 36.0f,
                 sub, vec4(0.55f, 0.48f, 0.35f, 1.0f), scale);
    }

    /* === Decorative line under title === */
    float line_w = 200.0f;
    ui_draw_rect(ui, (sw - line_w) / 2.0f, title_panel_y + 58.0f,
                 line_w, 1.0f,
                 vec4(0.50f, 0.42f, 0.28f, 0.6f));

    /* === Buttons === */
    float btn_w = 220.0f, btn_h = 48.0f;
    float btn_x = (sw - btn_w) / 2.0f;
    float btn_y = sh * 0.52f;
    Vec4 btn_text = vec4(1.0f, 1.0f, 1.0f, 1.0f);

    if (ui_button(ui, menu_id("play"), btn_x, btn_y, btn_w, btn_h,
                  "PLAY", vec4(0.28f, 0.25f, 0.18f, 1.0f), btn_text)) {
        audio_play(&app->audio, SFX_UI_CLICK);
        state_set(app->sm, STATE_DIFFICULTY, app);
    }

    btn_y += btn_h + 16.0f;
    if (ui_button(ui, menu_id("quit"), btn_x, btn_y, btn_w, btn_h,
                  "QUIT", vec4(0.35f, 0.12f, 0.10f, 1.0f), btn_text)) {
        audio_play(&app->audio, SFX_UI_CLICK);
        engine->running = false;
    }

    /* === Bottom credits === */
    {
        const char *credit = "Built from scratch in C + OpenGL";
        float scale = 1.0f;
        float char_w = 5.0f * scale + scale;
        float text_w = (float)strlen(credit) * char_w;
        ui_label(ui, (sw - text_w) / 2.0f, sh - 20.0f,
                 credit, vec4(0.30f, 0.28f, 0.24f, 1.0f), scale);
    }
}

State state_menu_create(void) {
    State s;
    memset(&s, 0, sizeof(s));
    s.id = STATE_MENU;
    s.enter = menu_enter;
    s.exit = menu_exit;
    s.update = menu_update;
    s.render = menu_render;
    return s;
}
