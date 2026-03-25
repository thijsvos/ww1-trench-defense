#include "state_briefing.h"
#include "app_context.h"
#include "../core/log.h"
#include "../ui/ui_widgets.h"

#include <string.h>
#include <math.h>
#include <GLFW/glfw3.h>

/* ------------------------------------------------------------------ */
/*  Historical briefing data for each mission                          */
/* ------------------------------------------------------------------ */

/* Flag IDs for drawing */
#define FLAG_FRANCE    0
#define FLAG_GERMANY   1
#define FLAG_BRITAIN   2
#define FLAG_OTTOMAN   3
#define FLAG_RUSSIA    4
#define FLAG_AUSTRIA   5
#define FLAG_ANZAC     6

typedef struct MissionBriefing {
    const char *title;
    const char *location;
    const char *date;
    const char *lines[12];     /* up to 12 lines of history text */
    int line_count;
    const char *left_stats[3];  /* defender side stats */
    int left_stat_count;
    const char *right_stats[3]; /* attacker side stats */
    int right_stat_count;
    const char *center_stat;    /* shared stat (duration, etc.) */
    const char *objective;
    int left_flags[3];         /* allied/defender side flags (-1 = none) */
    int left_flag_count;
    int right_flags[3];        /* enemy/attacker side flags (-1 = none) */
    int right_flag_count;
} MissionBriefing;

static const MissionBriefing s_briefings[5] = {
    /* Level 1: No Man's Land */
    {
        .title    = "NO MAN'S LAND",
        .location = "Ypres, Belgium - Western Front",
        .date     = "October 1914 - November 1918",
        .lines    = {
            "The Ypres Salient was one of the most",
            "bitterly contested sectors of the entire",
            "war. Three major battles were fought here.",
            "",
            "Between the trenches lay No Man's Land -",
            "a devastated wasteland of shell craters,",
            "mud, barbed wire, and the unburied dead.",
            "",
            "Soldiers on both sides lived in flooded",
            "trenches, enduring constant shelling and",
            "the ever-present threat of gas attacks.",
        },
        .line_count = 11,
        .left_stats = { "British: 475,000+", "French: 105,000+", "Troops: 1.5 million" },
        .left_stat_count = 3,
        .right_stats = { "German: 270,000+", "Gas first used 1915", "Troops: 1 million" },
        .right_stat_count = 3,
        .center_stat = "4 years  |  10 miles of front",
        .objective = "Hold the trench line against the assault.",
        .left_flags = { FLAG_BRITAIN, FLAG_FRANCE, -1 },
        .left_flag_count = 2,
        .right_flags = { FLAG_GERMANY, -1, -1 },
        .right_flag_count = 1,
    },
    /* Level 2: Gallipoli Beach */
    {
        .title    = "GALLIPOLI BEACH",
        .location = "Gallipoli Peninsula, Ottoman Empire",
        .date     = "April 1915 - January 1916",
        .lines    = {
            "Winston Churchill devised a plan to knock",
            "the Ottoman Empire out of the war by",
            "seizing the Dardanelles strait.",
            "",
            "Allied troops landed on exposed beaches",
            "under withering fire from the cliffs above.",
            "The Turks, led by Mustafa Kemal, held the",
            "high ground and repelled every advance.",
            "",
            "After 8 months of brutal stalemate, the",
            "Allies evacuated - one of the war's most",
            "costly failures.",
        },
        .line_count = 12,
        .left_stats = { "Allied: 187,000+", "Ships lost: 6", "8 months fighting" },
        .left_stat_count = 3,
        .right_stats = { "Ottoman: 161,000+", "Held the cliffs", "Kemal's defence" },
        .right_stat_count = 3,
        .center_stat = "April 1915 - January 1916",
        .objective = "Defend three beach landing zones.",
        .left_flags = { FLAG_BRITAIN, FLAG_ANZAC, FLAG_FRANCE },
        .left_flag_count = 3,
        .right_flags = { FLAG_OTTOMAN, -1, -1 },
        .right_flag_count = 1,
    },
    /* Level 3: Verdun Meatgrinder */
    {
        .title    = "VERDUN MEATGRINDER",
        .location = "Verdun, France",
        .date     = "February - December 1916",
        .lines    = {
            "Germany's plan was simple but horrifying:",
            "bleed France white. General Falkenhayn",
            "chose Verdun because France would defend",
            "it to the last man.",
            "",
            "For 303 days, both sides fed men into the",
            "furnace. The French rallied behind the cry",
            "\"Ils ne passeront pas!\" - They shall not",
            "pass!",
            "",
            "The battle consumed an entire generation",
            "and changed nothing on the map.",
        },
        .line_count = 12,
        .left_stats = { "French: 377,000+", "\"They shall not pass\"", "Held the fortress" },
        .left_stat_count = 3,
        .right_stats = { "German: 337,000+", "60 million shells", "Bleed France white" },
        .right_stat_count = 3,
        .center_stat = "303 days of continuous battle",
        .objective = "Survive the endless grind. One path, no mercy.",
        .left_flags = { FLAG_FRANCE, -1, -1 },
        .left_flag_count = 1,
        .right_flags = { FLAG_GERMANY, -1, -1 },
        .right_flag_count = 1,
    },
    /* Level 4: Brusilov's Breakthrough */
    {
        .title    = "BRUSILOV'S BREAKTHROUGH",
        .location = "Eastern Front, Galicia",
        .date     = "June - September 1916",
        .lines    = {
            "General Aleksei Brusilov launched the most",
            "successful Allied offensive of the war.",
            "His innovative tactics shattered the",
            "Austro-Hungarian lines across a 300-mile",
            "front.",
            "",
            "Instead of one massive push, Brusilov",
            "attacked at multiple points simultaneously,",
            "preventing the enemy from concentrating",
            "reserves.",
            "",
            "Austria-Hungary never recovered.",
        },
        .line_count = 12,
        .left_stats = { "Russian: 500,000+", "4 armies attacked", "Brilliant tactics" },
        .left_stat_count = 3,
        .right_stats = { "A-H: 1.5 million", "Front collapsed", "Never recovered" },
        .right_stat_count = 3,
        .center_stat = "300-mile front  |  June-Sept 1916",
        .objective = "Defend against attacks from all four directions.",
        .left_flags = { FLAG_RUSSIA, -1, -1 },
        .left_flag_count = 1,
        .right_flags = { FLAG_AUSTRIA, FLAG_GERMANY, -1 },
        .right_flag_count = 2,
    },
    /* Level 5: The Kaiserschlacht */
    {
        .title    = "THE KAISERSCHLACHT",
        .location = "Western Front, France",
        .date     = "March - July 1918",
        .lines    = {
            "Germany's last gamble. With Russia out of",
            "the war and American troops arriving,",
            "Ludendorff launched Operation Michael -",
            "the Spring Offensive.",
            "",
            "Using elite Stormtrooper units and new",
            "infiltration tactics, the Germans broke",
            "through Allied lines for the first time",
            "since 1914.",
            "",
            "They advanced 40 miles in days. But they",
            "could not sustain it. This was the end.",
        },
        .line_count = 12,
        .left_stats = { "Allied: 863,000+", "Held the line", "Americans arrive" },
        .left_stat_count = 3,
        .right_stats = { "German: 688,000+", "40 miles gained", "Last gamble lost" },
        .right_stat_count = 3,
        .center_stat = "March - July 1918  |  The end",
        .objective = "The final battle. Enemies from every direction.",
        .left_flags = { FLAG_BRITAIN, FLAG_FRANCE, -1 },
        .left_flag_count = 2,
        .right_flags = { FLAG_GERMANY, -1, -1 },
        .right_flag_count = 1,
    },
};

/* ------------------------------------------------------------------ */
/*  State callbacks                                                    */
/* ------------------------------------------------------------------ */

/* Draw a simplified pixel-art national flag at screen position */
static void draw_flag(UIContext *ui, float x, float y, float w, float h, int flag_id)
{
    float s3 = w / 3.0f; /* third width */
    float h3 = h / 3.0f; /* third height */

    switch (flag_id) {
    case FLAG_FRANCE: /* Blue | White | Red (vertical) */
        ui_draw_rect(ui, x, y, s3, h, vec4(0.0f, 0.14f, 0.58f, 1.0f));
        ui_draw_rect(ui, x + s3, y, s3, h, vec4(1.0f, 1.0f, 1.0f, 1.0f));
        ui_draw_rect(ui, x + s3 * 2, y, s3, h, vec4(0.93f, 0.16f, 0.22f, 1.0f));
        break;
    case FLAG_GERMANY: /* Black / White / Red (horizontal, Imperial) */
        ui_draw_rect(ui, x, y, w, h3, vec4(0.1f, 0.1f, 0.1f, 1.0f));
        ui_draw_rect(ui, x, y + h3, w, h3, vec4(1.0f, 1.0f, 1.0f, 1.0f));
        ui_draw_rect(ui, x, y + h3 * 2, w, h3, vec4(0.85f, 0.1f, 0.1f, 1.0f));
        break;
    case FLAG_BRITAIN: /* Union Jack — blue bg + white/red diagonals + white/red cross */
        ui_draw_rect(ui, x, y, w, h, vec4(0.0f, 0.14f, 0.40f, 1.0f)); /* blue */
        /* White diagonal stripes (St Andrew's + St Patrick's) */
        { int steps = (int)w;
          for (int s = 0; s < steps; s++) {
            float t = (float)s / (float)steps;
            /* Top-left to bottom-right */
            ui_draw_rect(ui, x + t * w, y + t * h - 1.0f, 2.0f, 3.0f,
                         vec4(1.0f, 1.0f, 1.0f, 0.9f));
            /* Top-right to bottom-left */
            ui_draw_rect(ui, x + (1.0f - t) * w - 2.0f, y + t * h - 1.0f, 2.0f, 3.0f,
                         vec4(1.0f, 1.0f, 1.0f, 0.9f));
          }
        }
        /* Red diagonal stripes (thinner, on top of white) */
        { int steps = (int)w;
          for (int s = 0; s < steps; s++) {
            float t = (float)s / (float)steps;
            ui_draw_rect(ui, x + t * w, y + t * h, 1.0f, 1.0f,
                         vec4(0.8f, 0.1f, 0.1f, 0.9f));
            ui_draw_rect(ui, x + (1.0f - t) * w - 1.0f, y + t * h, 1.0f, 1.0f,
                         vec4(0.8f, 0.1f, 0.1f, 0.9f));
          }
        }
        /* White cross (St George's — on top of diagonals) */
        ui_draw_rect(ui, x + w * 0.38f, y, w * 0.24f, h, vec4(1.0f, 1.0f, 1.0f, 1.0f));
        ui_draw_rect(ui, x, y + h * 0.35f, w, h * 0.3f, vec4(1.0f, 1.0f, 1.0f, 1.0f));
        /* Red cross (on top of white) */
        ui_draw_rect(ui, x + w * 0.42f, y, w * 0.16f, h, vec4(0.8f, 0.1f, 0.1f, 1.0f));
        ui_draw_rect(ui, x, y + h * 0.40f, w, h * 0.2f, vec4(0.8f, 0.1f, 0.1f, 1.0f));
        break;
    case FLAG_OTTOMAN: /* Red background with white crescent and star */
        ui_draw_rect(ui, x, y, w, h, vec4(0.9f, 0.1f, 0.1f, 1.0f));
        {
            Vec4 white = vec4(1.0f, 1.0f, 1.0f, 1.0f);
            float cx = x + w * 0.38f;
            float cy = y + h * 0.5f;
            float r_outer = h * 0.38f;
            float r_inner = h * 0.28f;
            float off_x = w * 0.06f; /* inner circle offset to form crescent */
            float ps = (h > 16.0f) ? 2.0f : 1.0f; /* pixel size */
            /* Draw white outer circle, cut out red inner circle = crescent */
            for (float py = -r_outer; py <= r_outer; py += ps) {
                for (float px = -r_outer; px <= r_outer; px += ps) {
                    float d_outer = px * px + py * py;
                    if (d_outer <= r_outer * r_outer) {
                        float dx_i = px - off_x;
                        float d_inner = dx_i * dx_i + py * py;
                        if (d_inner > r_inner * r_inner) {
                            ui_draw_rect(ui, cx + px, cy + py, ps, ps, white);
                        }
                    }
                }
            }
            /* Five-pointed star to the right of crescent */
            float sx = x + w * 0.58f;
            float sy = cy;
            float sr = h * 0.12f;
            for (int p = 0; p < 5; p++) {
                float a1 = -1.5708f + (float)p * 1.2566f;       /* outer point */
                float a2 = -1.5708f + ((float)p + 0.5f) * 1.2566f; /* inner point */
                float ox = sx + cosf(a1) * sr;
                float oy = sy + sinf(a1) * sr;
                float ix = sx + cosf(a2) * sr * 0.4f;
                float iy = sy + sinf(a2) * sr * 0.4f;
                /* Draw line from center to outer point */
                for (float t = 0.0f; t <= 1.0f; t += 0.08f) {
                    ui_draw_rect(ui, sx + (ox - sx) * t, sy + (oy - sy) * t,
                                 ps, ps, white);
                }
                /* Draw line from outer point to next inner point */
                for (float t = 0.0f; t <= 1.0f; t += 0.08f) {
                    ui_draw_rect(ui, ox + (ix - ox) * t, oy + (iy - oy) * t,
                                 ps, ps, white);
                }
            }
        }
        break;
    case FLAG_RUSSIA: /* White / Blue / Red (horizontal, Imperial) */
        ui_draw_rect(ui, x, y, w, h3, vec4(1.0f, 1.0f, 1.0f, 1.0f));
        ui_draw_rect(ui, x, y + h3, w, h3, vec4(0.0f, 0.22f, 0.55f, 1.0f));
        ui_draw_rect(ui, x, y + h3 * 2, w, h3, vec4(0.85f, 0.1f, 0.1f, 1.0f));
        break;
    case FLAG_AUSTRIA: /* Red / White / Red (horizontal) */
        ui_draw_rect(ui, x, y, w, h3, vec4(0.93f, 0.16f, 0.22f, 1.0f));
        ui_draw_rect(ui, x, y + h3, w, h3, vec4(1.0f, 1.0f, 1.0f, 1.0f));
        ui_draw_rect(ui, x, y + h3 * 2, w, h3, vec4(0.93f, 0.16f, 0.22f, 1.0f));
        break;
    case FLAG_ANZAC: /* Australian flag — blue bg, Union Jack canton, Southern Cross */
        ui_draw_rect(ui, x, y, w, h, vec4(0.0f, 0.14f, 0.40f, 1.0f));
        {
            Vec4 wht = vec4(1.0f, 1.0f, 1.0f, 1.0f);
            Vec4 red = vec4(0.8f, 0.1f, 0.1f, 1.0f);
            float cw = w * 0.45f, ch = h * 0.5f; /* canton size */
            float ps = (h > 20.0f) ? 2.0f : 1.0f;
            /* Union Jack canton background */
            ui_draw_rect(ui, x, y, cw, ch, vec4(0.0f, 0.12f, 0.38f, 1.0f));
            /* White diagonals */
            { int steps = (int)cw;
              for (int s = 0; s < steps; s++) {
                float t = (float)s / (float)steps;
                ui_draw_rect(ui, x + t * cw, y + t * ch, ps, ps, wht);
                ui_draw_rect(ui, x + (1.0f - t) * cw - ps, y + t * ch, ps, ps, wht);
              }
            }
            /* White cross */
            ui_draw_rect(ui, x + cw * 0.38f, y, cw * 0.24f, ch, wht);
            ui_draw_rect(ui, x, y + ch * 0.35f, cw, ch * 0.3f, wht);
            /* Red diagonals (thinner, on top) */
            { int steps = (int)cw;
              for (int s = 0; s < steps; s++) {
                float t = (float)s / (float)steps;
                ui_draw_rect(ui, x + t * cw + ps * 0.5f, y + t * ch + ps * 0.5f,
                             1.0f, 1.0f, red);
                ui_draw_rect(ui, x + (1.0f - t) * cw - ps * 0.5f - 1.0f,
                             y + t * ch + ps * 0.5f, 1.0f, 1.0f, red);
              }
            }
            /* Red cross (on top of white) */
            ui_draw_rect(ui, x + cw * 0.42f, y, cw * 0.16f, ch, red);
            ui_draw_rect(ui, x, y + ch * 0.40f, cw, ch * 0.2f, red);
            /* Commonwealth Star (7-pointed, below canton) */
            {
                float csx = x + cw * 0.5f;
                float csy = y + h * 0.75f;
                float sr = h * 0.14f;
                for (int p = 0; p < 7; p++) {
                    float a = -1.5708f + (float)p * (6.2832f / 7.0f);
                    float ox = csx + cosf(a) * sr;
                    float oy = csy + sinf(a) * sr;
                    for (float t = 0.0f; t <= 1.0f; t += 0.06f) {
                        ui_draw_rect(ui, csx + (ox - csx) * t,
                                     csy + (oy - csy) * t, ps, ps, wht);
                    }
                }
            }
            /* Southern Cross — 4 large stars + 1 small star on right half */
            {
                float star_sz = h * 0.06f; /* large star radius */
                float sm_sz = h * 0.04f;   /* small star radius */
                /* Approximate Southern Cross positions */
                struct { float fx, fy; float r; } sc[5] = {
                    { 0.77f, 0.25f, star_sz }, /* Alpha (top) */
                    { 0.90f, 0.45f, star_sz }, /* Beta (right) */
                    { 0.82f, 0.75f, star_sz }, /* Gamma (bottom-right) */
                    { 0.65f, 0.60f, star_sz }, /* Delta (left) */
                    { 0.80f, 0.50f, sm_sz   }, /* Epsilon (center, small) */
                };
                for (int s = 0; s < 5; s++) {
                    float ssx = x + w * sc[s].fx;
                    float ssy = y + h * sc[s].fy;
                    float sr = sc[s].r;
                    /* Draw as a small + shape */
                    ui_draw_rect(ui, ssx - sr, ssy - ps * 0.5f, sr * 2.0f, ps, wht);
                    ui_draw_rect(ui, ssx - ps * 0.5f, ssy - sr, ps, sr * 2.0f, wht);
                    /* Diagonal arms */
                    ui_draw_rect(ui, ssx - sr * 0.7f, ssy - sr * 0.7f, ps, ps, wht);
                    ui_draw_rect(ui, ssx + sr * 0.7f - ps, ssy - sr * 0.7f, ps, ps, wht);
                    ui_draw_rect(ui, ssx - sr * 0.7f, ssy + sr * 0.7f - ps, ps, ps, wht);
                    ui_draw_rect(ui, ssx + sr * 0.7f - ps, ssy + sr * 0.7f - ps, ps, ps, wht);
                }
            }
        }
        break;
    }

    /* Thin dark border around flag */
    ui_draw_rect(ui, x, y, w, 1.0f, vec4(0.2f, 0.2f, 0.2f, 0.6f));
    ui_draw_rect(ui, x, y + h - 1.0f, w, 1.0f, vec4(0.2f, 0.2f, 0.2f, 0.6f));
    ui_draw_rect(ui, x, y, 1.0f, h, vec4(0.2f, 0.2f, 0.2f, 0.6f));
    ui_draw_rect(ui, x + w - 1.0f, y, 1.0f, h, vec4(0.2f, 0.2f, 0.2f, 0.6f));
}

static uint32_t brief_id(const char *tag) {
    uint32_t hash = 2166136261u;
    while (*tag) {
        hash ^= (uint32_t)(unsigned char)*tag++;
        hash *= 16777619u;
    }
    return hash;
}

static void briefing_enter(void *ctx) {
    (void)ctx;
    LOG_INFO("Entered mission briefing");
}

static void briefing_exit(void *ctx) { (void)ctx; }

static void briefing_update(void *ctx, Engine *engine, UIContext *ui) {
    (void)ctx; (void)engine; (void)ui;
}

static void briefing_render(void *ctx, Engine *engine, UIContext *ui) {
    AppContext *app = (AppContext *)ctx;
    (void)engine;
    float sw = (float)ui->screen_w;
    float sh = (float)ui->screen_h;
    float time = (float)glfwGetTime();

    int level = app->selected_level;
    if (level < 0 || level >= 5) level = 0;
    const MissionBriefing *b = &s_briefings[level];

    /* Dark atmospheric background */
    ui_draw_rect(ui, 0, 0, sw, sh, vec4(0.06f, 0.05f, 0.04f, 1.0f));

    /* Subtle horizontal lines for military document feel */
    for (float y = 0; y < sh; y += 24.0f) {
        ui_draw_rect(ui, 0, y, sw, 1.0f, vec4(0.08f, 0.07f, 0.06f, 0.5f));
    }

    /* Top bar — CLASSIFIED / MISSION BRIEFING */
    ui_draw_rect(ui, 0, 0, sw, 32.0f, vec4(0.12f, 0.04f, 0.03f, 1.0f));
    {
        const char *cls = "CLASSIFIED - MISSION BRIEFING";
        float cs = 1.8f;
        float cw = (float)strlen(cls) * 6.0f * cs;
        ui_label(ui, (sw - cw) / 2.0f, 8.0f, cls,
                 vec4(0.9f, 0.7f, 0.5f, 1.0f), cs);
    }

    /* === Mission title === */
    {
        float ts = 3.0f;
        float tw = (float)strlen(b->title) * 6.0f * ts;
        ui_label(ui, (sw - tw) / 2.0f, 50.0f, b->title,
                 vec4(0.92f, 0.80f, 0.45f, 1.0f), ts);
    }

    /* Location and date */
    {
        float ls = 1.5f;
        float lw = (float)strlen(b->location) * 6.0f * ls;
        ui_label(ui, (sw - lw) / 2.0f, 80.0f, b->location,
                 vec4(0.60f, 0.55f, 0.45f, 1.0f), ls);

        float dw = (float)strlen(b->date) * 6.0f * ls;
        ui_label(ui, (sw - dw) / 2.0f, 96.0f, b->date,
                 vec4(0.55f, 0.50f, 0.40f, 1.0f), ls);
    }

    /* Decorative line */
    ui_draw_rect(ui, sw * 0.1f, 116.0f, sw * 0.8f, 1.0f,
                 vec4(0.40f, 0.32f, 0.20f, 0.6f));

    /* === Historical text — centered, full width === */
    float text_y = 130.0f;
    float text_scale = 2.0f;
    for (int i = 0; i < b->line_count; i++) {
        if (b->lines[i][0] == '\0') {
            text_y += 12.0f;
        } else {
            float tw = (float)strlen(b->lines[i]) * 6.0f * text_scale;
            ui_label(ui, (sw - tw) / 2.0f, text_y, b->lines[i],
                     vec4(0.75f, 0.72f, 0.65f, 1.0f), text_scale);
            text_y += 20.0f;
        }
    }

    /* === Stats — centered row below text === */
    text_y += 10.0f;
    ui_draw_rect(ui, sw * 0.1f, text_y, sw * 0.8f, 1.0f,
                 vec4(0.35f, 0.28f, 0.18f, 0.5f));
    text_y += 8.0f;

    /* === Flags, VS, and stats — all aligned on center line (sw/2) === */
    {
        float half = sw * 0.5f;
        float flag_w = 52.0f, flag_h = 32.0f;
        float flag_gap = 6.0f;
        float margin = 24.0f; /* gap between flags and center line */
        Vec4 div_col = vec4(0.40f, 0.32f, 0.20f, 0.6f);

        /* Left flags (right-aligned to center) */
        float left_total = (float)b->left_flag_count * (flag_w + flag_gap) - flag_gap;
        float lx = half - margin - left_total;
        for (int f = 0; f < b->left_flag_count; f++) {
            draw_flag(ui, lx + (float)f * (flag_w + flag_gap), text_y, flag_w, flag_h,
                      b->left_flags[f]);
        }

        /* Right flags (left-aligned from center) */
        float rx = half + margin;
        for (int f = 0; f < b->right_flag_count; f++) {
            draw_flag(ui, rx + (float)f * (flag_w + flag_gap), text_y, flag_w, flag_h,
                      b->right_flags[f]);
        }

        /* "VS" text centered on the divider line */
        float vs_scale = 1.8f;
        float vs_w = 2.0f * 6.0f * vs_scale;
        float vs_h = 5.0f * vs_scale; /* font is 5px tall */
        ui_label(ui, half - vs_w * 0.5f, text_y + (flag_h - vs_h) * 0.5f, "VS",
                 vec4(0.60f, 0.50f, 0.35f, 1.0f), vs_scale);

        text_y += flag_h + 12.0f;

        /* === Side-by-side casualty stats === */
        float stat_scale = 1.7f;
        float cpx = 6.0f * stat_scale;
        int max_lines = b->left_stat_count > b->right_stat_count
                      ? b->left_stat_count : b->right_stat_count;

        /* Vertical divider — extends from flags through stats */
        float div_h = (float)max_lines * 22.0f;
        ui_draw_rect(ui, half - 1.0f, text_y - 2.0f, 2.0f, div_h + 4.0f, div_col);

        /* Left side stats (right-aligned to center) */
        for (int i = 0; i < b->left_stat_count; i++) {
            float tw = (float)strlen(b->left_stats[i]) * cpx;
            ui_label(ui, half - tw - 20.0f, text_y + (float)i * 22.0f,
                     b->left_stats[i], vec4(0.70f, 0.65f, 0.55f, 1.0f), stat_scale);
        }

        /* Right side stats (left-aligned from center) */
        for (int i = 0; i < b->right_stat_count; i++) {
            ui_label(ui, half + 20.0f, text_y + (float)i * 22.0f,
                     b->right_stats[i], vec4(0.70f, 0.65f, 0.55f, 1.0f), stat_scale);
        }

        text_y += (float)max_lines * 22.0f + 8.0f;

        /* Center shared stat below divider */
        if (b->center_stat) {
            float cw = (float)strlen(b->center_stat) * cpx;
            ui_label(ui, (sw - cw) / 2.0f, text_y,
                     b->center_stat, vec4(0.55f, 0.50f, 0.40f, 1.0f), stat_scale);
        }
    }

    /* === Objective box === */
    float obj_y = sh - 120.0f;
    ui_draw_rect(ui, sw * 0.1f, obj_y, sw * 0.8f, 1.0f,
                 vec4(0.40f, 0.32f, 0.20f, 0.6f));
    {
        const char *obj_lbl = "MISSION OBJECTIVE";
        float os = 1.8f;
        float ow = (float)strlen(obj_lbl) * 6.0f * os;
        ui_label(ui, (sw - ow) / 2.0f, obj_y + 8.0f, obj_lbl,
                 vec4(0.85f, 0.70f, 0.40f, 1.0f), os);
    }
    {
        float os = 1.6f;
        float ow = (float)strlen(b->objective) * 6.0f * os;
        ui_label(ui, (sw - ow) / 2.0f, obj_y + 30.0f, b->objective,
                 vec4(0.80f, 0.78f, 0.70f, 1.0f), os);
    }

    /* === BEGIN MISSION button === */
    float btn_w = 240.0f, btn_h = 44.0f;
    float btn_x = (sw - btn_w) / 2.0f;
    float btn_y = sh - 56.0f;
    /* Pulsing border */
    float pulse = 0.7f + 0.3f * sinf(time * 2.5f);
    ui_draw_rect(ui, btn_x - 2, btn_y - 2, btn_w + 4, btn_h + 4,
                 vec4(0.85f * pulse, 0.70f * pulse, 0.30f * pulse, 1.0f));

    if (ui_button(ui, brief_id("begin"), btn_x, btn_y, btn_w, btn_h,
                  "BEGIN MISSION",
                  vec4(0.30f, 0.25f, 0.15f, 1.0f),
                  vec4(1.0f, 1.0f, 1.0f, 1.0f))) {
        audio_play(&app->audio, SFX_UI_CLICK);
        state_set(app->sm, STATE_PLAY, app);
    }
}

State state_briefing_create(void) {
    State s;
    memset(&s, 0, sizeof(s));
    s.id = STATE_BRIEFING;
    s.enter = briefing_enter;
    s.exit = briefing_exit;
    s.update = briefing_update;
    s.render = briefing_render;
    return s;
}
