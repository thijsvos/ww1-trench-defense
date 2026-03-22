#include <glad/gl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "atlas_gen.h"
#include "../core/log.h"

/* ------------------------------------------------------------------ */
/* WW1 Color Palette                                                    */
/* ------------------------------------------------------------------ */
#define COL_MUD_R        92
#define COL_MUD_G        64
#define COL_MUD_B        51

#define COL_OLIVE_R      60
#define COL_OLIVE_G      74
#define COL_OLIVE_B      46

#define COL_KHAKI_R      195
#define COL_KHAKI_G      176
#define COL_KHAKI_B      145

#define COL_STEEL_R      113
#define COL_STEEL_G      121
#define COL_STEEL_B      126

#define COL_BLOOD_R      139
#define COL_BLOOD_G      0
#define COL_BLOOD_B      0

#define COL_MUSTARD_R    225
#define COL_MUSTARD_G    173
#define COL_MUSTARD_B    1

#define COL_SKY_R        135
#define COL_SKY_G        206
#define COL_SKY_B        235

#define COL_OFFWHITE_R   245
#define COL_OFFWHITE_G   240
#define COL_OFFWHITE_B   225

#define COL_CHARCOAL_R   54
#define COL_CHARCOAL_G   69
#define COL_CHARCOAL_B   79

#define COL_ORANGE_R     204
#define COL_ORANGE_G     85
#define COL_ORANGE_B     0

/* ------------------------------------------------------------------ */
/* Static pixel buffer and drawing helpers                              */
/* ------------------------------------------------------------------ */
static uint8_t *pixels;

static void put_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if (x < 0 || x >= ATLAS_SIZE || y < 0 || y >= ATLAS_SIZE) return;
    int idx = (y * ATLAS_SIZE + x) * 4;
    pixels[idx + 0] = r;
    pixels[idx + 1] = g;
    pixels[idx + 2] = b;
    pixels[idx + 3] = a;
}

static void fill_rect(int x, int y, int w, int h,
                       uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    for (int py = y; py < y + h; py++)
        for (int px = x; px < x + w; px++)
            put_pixel(px, py, r, g, b, a);
}

static void draw_circle(int cx, int cy, int radius,
                         uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    for (int py = cy - radius; py <= cy + radius; py++) {
        for (int px = cx - radius; px <= cx + radius; px++) {
            int dx = px - cx;
            int dy = py - cy;
            if (dx * dx + dy * dy <= radius * radius)
                put_pixel(px, py, r, g, b, a);
        }
    }
}

static void draw_circle_outline(int cx, int cy, int radius,
                                 uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    for (int py = cy - radius; py <= cy + radius; py++) {
        for (int px = cx - radius; px <= cx + radius; px++) {
            int dx = px - cx;
            int dy = py - cy;
            int dsq = dx * dx + dy * dy;
            int r_inner = (radius - 1) * (radius - 1);
            int r_outer = radius * radius;
            if (dsq <= r_outer && dsq >= r_inner)
                put_pixel(px, py, r, g, b, a);
        }
    }
}

static void draw_line(int x0, int y0, int x1, int y1,
                       uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    /* Bresenham's line algorithm */
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    for (;;) {
        put_pixel(x0, y0, r, g, b, a);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

/* Blend a pixel with alpha onto the buffer (simple over compositing) */
static void put_pixel_blend(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    if (x < 0 || x >= ATLAS_SIZE || y < 0 || y >= ATLAS_SIZE) return;
    if (a == 255) { put_pixel(x, y, r, g, b, a); return; }
    if (a == 0) return;

    int idx = (y * ATLAS_SIZE + x) * 4;
    uint8_t dr = pixels[idx + 0];
    uint8_t dg = pixels[idx + 1];
    uint8_t db = pixels[idx + 2];
    uint8_t da = pixels[idx + 3];

    float sa = a / 255.0f;
    float one_minus_sa = 1.0f - sa;
    pixels[idx + 0] = (uint8_t)(r * sa + dr * one_minus_sa);
    pixels[idx + 1] = (uint8_t)(g * sa + dg * one_minus_sa);
    pixels[idx + 2] = (uint8_t)(b * sa + db * one_minus_sa);
    pixels[idx + 3] = (uint8_t)(a + da * one_minus_sa);
}

/* Deterministic seeded RNG helper - returns value in [0, max) */
static int rng_range(int max)
{
    if (max <= 0) return 0;
    return rand() % max;
}

/* ------------------------------------------------------------------ */
/* Terrain tile generators (32x32 each, fully opaque)                   */
/* ------------------------------------------------------------------ */

static void gen_tile_grass(int ox, int oy)
{
    /* War-torn ground: brownish-green, mostly mud with sparse dead grass */
    /* Muddy earth base (dark brown-green) */
    fill_rect(ox, oy, 32, 32, 62, 58, 40, 255);

    /* Irregular mud patches (darker) */
    for (int i = 0; i < 8; i++) {
        int cx = ox + 2 + rng_range(28);
        int cy = oy + 2 + rng_range(28);
        int sz = 1 + rng_range(3);
        for (int dy = -sz; dy <= sz; dy++)
            for (int dx = -sz; dx <= sz; dx++)
                if (dx*dx + dy*dy <= sz*sz)
                    put_pixel(cx+dx, cy+dy, 50, 42, 32, 255);
    }

    /* Shell crater marks (small dark circles) */
    for (int i = 0; i < 3; i++) {
        int cx = ox + 4 + rng_range(24);
        int cy = oy + 4 + rng_range(24);
        int r = 1 + rng_range(2);
        draw_circle(cx, cy, r, 40, 34, 26, 255);
        /* Lighter rim */
        for (int a = 0; a < 6; a++) {
            int rx = cx - r + rng_range(r * 2 + 1);
            int ry = cy - r - 1 + rng_range(2);
            put_pixel(rx, ry, 75, 65, 50, 255);
        }
    }

    /* Sparse dead grass tufts (muted olive, not bright green) */
    for (int i = 0; i < 15; i++) {
        int px = ox + rng_range(32);
        int py = oy + rng_range(32);
        put_pixel(px, py, 70, 72, 45, 255);
    }
    /* Rare surviving grass blade (very few) */
    for (int i = 0; i < 3; i++) {
        int px = ox + rng_range(32);
        int py = oy + 1 + rng_range(30);
        put_pixel(px, py, 60, 75, 40, 255);
        put_pixel(px, py - 1, 68, 82, 48, 255);
    }

    /* Scattered debris/pebbles */
    for (int i = 0; i < 10; i++) {
        int px = ox + rng_range(32);
        int py = oy + rng_range(32);
        put_pixel(px, py, 80, 72, 58, 255);
    }
}

static void gen_tile_mud(int ox, int oy)
{
    /* Shell-cratered mud — devastated no man's land */
    fill_rect(ox, oy, 32, 32, 72, 52, 38, 255);

    /* Large crater (water-filled depression) */
    {
        int cx = ox + 10 + rng_range(12);
        int cy = oy + 10 + rng_range(12);
        int r = 3 + rng_range(3);
        /* Dark muddy water in crater */
        draw_circle(cx, cy, r, 45, 40, 35, 255);
        draw_circle(cx, cy, r - 1, 50, 52, 48, 255);
        /* Lighter rim (displaced earth) */
        for (int a = 0; a < 12; a++) {
            int rx = cx - r - 1 + rng_range(r * 2 + 3);
            int ry = cy - r - 1 + rng_range(r * 2 + 3);
            float d2 = (float)((rx-cx)*(rx-cx) + (ry-cy)*(ry-cy));
            if (d2 > (float)(r*r) && d2 < (float)((r+2)*(r+2)))
                put_pixel(rx, ry, 90, 68, 52, 255);
        }
    }

    /* Smaller craters */
    for (int i = 0; i < 2; i++) {
        int cx = ox + 3 + rng_range(26);
        int cy = oy + 3 + rng_range(26);
        draw_circle(cx, cy, 2, 55, 40, 30, 255);
    }

    /* Churned earth variation */
    for (int i = 0; i < 20; i++) {
        int px = ox + rng_range(32);
        int py = oy + rng_range(32);
        int shade = 58 + rng_range(30);
        put_pixel(px, py, (uint8_t)shade, (uint8_t)(shade - 14), (uint8_t)(shade - 22), 255);
    }

    /* Waterlogged puddle highlights */
    for (int i = 0; i < 6; i++) {
        int px = ox + rng_range(32);
        int py = oy + rng_range(32);
        put_pixel(px, py, 85, 90, 82, 255);
    }
}

static void gen_trench_variant(int ox, int oy, int mask); /* forward decl */

/* Simple fallback trench for the TILE_TRENCH base slot (horizontal) */
static void gen_tile_trench(int ox, int oy)
{
    gen_trench_variant(ox, oy, 5); /* E+W = horizontal */
}

/*
 * Generate a trench tile variant based on neighbor mask.
 * mask bits: 0=E, 1=S, 2=W, 3=N
 * Draws channels toward each connected neighbor.
 */
static void gen_trench_variant(int ox, int oy, int mask)
{
    int has_e = mask & 1;
    int has_s = (mask >> 1) & 1;
    int has_w = (mask >> 2) & 1;
    int has_n = (mask >> 3) & 1;

    /* Count connections */
    int connections = has_e + has_s + has_w + has_n;

    /* Earth base */
    fill_rect(ox, oy, 32, 32, COL_MUD_R, COL_MUD_G, COL_MUD_B, 255);

    /* Darker dirt variation */
    for (int i = 0; i < 20; i++) {
        int px = ox + rng_range(32);
        int py = oy + rng_range(32);
        put_pixel(px, py, 80, 56, 42, 255);
    }

    /* For 3+ connections (T-junctions, crosses), fill the whole tile */
    int ch, c0, c1;
    if (connections >= 3) {
        ch = 32; c0 = 0; c1 = 32;
    } else {
        ch = 20; c0 = 6; c1 = 26;
    }

    /* Dark channel floor color */
    uint8_t fr = 56, fg = 38, fb = 28;
    /* Lighter inner floor */
    uint8_t ir = 64, ig = 44, ib = 32;
    /* Shadow edge */
    uint8_t sr = 38, sg = 26, sb = 18;
    /* Duckboard */
    uint8_t dr = 100, dg = 74, db = 56;

    /* --- Draw channel segments toward each connected side --- */

    /* East channel: from center to right edge */
    if (has_e) {
        fill_rect(ox + 16, oy + c0, 16, ch, fr, fg, fb, 255);
        fill_rect(ox + 16, oy + c0 + 2, 16, ch - 4, ir, ig, ib, 255);
        fill_rect(ox + 16, oy + c0 - 1, 16, 1, sr, sg, sb, 255);
        fill_rect(ox + 16, oy + c1, 16, 1, sr, sg, sb, 255);
        /* Duckboards */
        for (int lx = ox + 17; lx < ox + 31; lx += 4)
            fill_rect(lx, oy + c0 + 4, 3, 1, dr, dg, db, 255);
    }

    /* West channel: from left edge to center */
    if (has_w) {
        fill_rect(ox, oy + c0, 16, ch, fr, fg, fb, 255);
        fill_rect(ox, oy + c0 + 2, 16, ch - 4, ir, ig, ib, 255);
        fill_rect(ox, oy + c0 - 1, 16, 1, sr, sg, sb, 255);
        fill_rect(ox, oy + c1, 16, 1, sr, sg, sb, 255);
        for (int lx = ox + 1; lx < ox + 15; lx += 4)
            fill_rect(lx, oy + c0 + 4, 3, 1, dr, dg, db, 255);
    }

    /* South channel: from center to bottom edge */
    if (has_s) {
        fill_rect(ox + c0, oy + 16, ch, 16, fr, fg, fb, 255);
        fill_rect(ox + c0 + 2, oy + 16, ch - 4, 16, ir, ig, ib, 255);
        fill_rect(ox + c0 - 1, oy + 16, 1, 16, sr, sg, sb, 255);
        fill_rect(ox + c1, oy + 16, 1, 16, sr, sg, sb, 255);
        for (int ly = oy + 17; ly < oy + 31; ly += 4)
            fill_rect(ox + c0 + 4, ly, 1, 3, dr, dg, db, 255);
    }

    /* North channel: from top edge to center */
    if (has_n) {
        fill_rect(ox + c0, oy, ch, 16, fr, fg, fb, 255);
        fill_rect(ox + c0 + 2, oy, ch - 4, 16, ir, ig, ib, 255);
        fill_rect(ox + c0 - 1, oy, 1, 16, sr, sg, sb, 255);
        fill_rect(ox + c1, oy, 1, 16, sr, sg, sb, 255);
        for (int ly = oy + 1; ly < oy + 15; ly += 4)
            fill_rect(ox + c0 + 4, ly, 1, 3, dr, dg, db, 255);
    }

    /* Center junction (always drawn if any connections) */
    if (mask) {
        fill_rect(ox + c0, oy + c0, ch, ch, fr, fg, fb, 255);
        fill_rect(ox + c0 + 2, oy + c0 + 2, ch - 4, ch - 4, ir, ig, ib, 255);
    }

    /* Isolated trench (no neighbors): draw a circular pit */
    if (mask == 0) {
        draw_circle(ox + 16, oy + 16, 8, fr, fg, fb, 255);
        draw_circle(ox + 16, oy + 16, 6, ir, ig, ib, 255);
    }
}

static void gen_tile_stone(int ox, int oy)
{
    /* Bombed-out concrete/stone ruins */
    fill_rect(ox, oy, 32, 32, 85, 82, 78, 255);

    /* Rubble chunks — irregular gray blocks */
    for (int i = 0; i < 6; i++) {
        int bx = ox + 1 + rng_range(26);
        int by = oy + 1 + rng_range(26);
        int bw = 2 + rng_range(4);
        int bh = 2 + rng_range(3);
        int shade = 75 + rng_range(40);
        fill_rect(bx, by, bw, bh, (uint8_t)shade, (uint8_t)(shade-4), (uint8_t)(shade-8), 255);
        /* Dark edge (shadow) */
        fill_rect(bx, by + bh - 1, bw, 1, (uint8_t)(shade-20), (uint8_t)(shade-24), (uint8_t)(shade-28), 255);
    }

    /* Cracks and damage lines */
    for (int i = 0; i < 4; i++) {
        int sx = ox + 2 + rng_range(28);
        int sy = oy + 2 + rng_range(28);
        int len = 3 + rng_range(6);
        for (int j = 0; j < len; j++) {
            put_pixel(sx + j, sy + (j % 3 == 0 ? 1 : 0), 55, 50, 45, 255);
        }
    }

    /* Dust/debris scatter */
    for (int i = 0; i < 15; i++) {
        int px = ox + rng_range(32);
        int py = oy + rng_range(32);
        put_pixel(px, py, 100, 95, 88, 255);
    }

    /* Rebar/metal fragments (dark) */
    for (int i = 0; i < 2; i++) {
        int sx = ox + 4 + rng_range(24);
        int sy = oy + 4 + rng_range(24);
        fill_rect(sx, sy, 1, 3 + rng_range(3), 50, 48, 52, 255);
    }
}

static void gen_tile_water(int ox, int oy)
{
    /* Flooded shell hole — murky brown-green water, not clean blue */
    fill_rect(ox, oy, 32, 32, 50, 55, 48, 255);

    /* Murky depth variation */
    for (int i = 0; i < 25; i++) {
        int px = ox + rng_range(32);
        int py = oy + rng_range(32);
        put_pixel(px, py, 42, 50, 42, 255);
    }

    /* Oily/muddy surface sheen — subtle lighter patches */
    for (int streak = 0; streak < 3; streak++) {
        int sy = oy + 5 + streak * 9 + rng_range(3);
        int sx = ox + 2 + rng_range(8);
        int len = 6 + rng_range(10);
        for (int j = 0; j < len; j++) {
            int px = sx + j;
            if (px >= ox + 32) break;
            put_pixel(px, sy, 62, 68, 58, 255);
        }
    }

    /* Debris floating on surface */
    for (int i = 0; i < 4; i++) {
        int px = ox + 2 + rng_range(28);
        int py = oy + 2 + rng_range(28);
        put_pixel(px, py, 70, 58, 42, 255);
        put_pixel(px + 1, py, 65, 55, 40, 255);
    }

    /* Muddy edges — darker rim around the tile */
    for (int x = ox; x < ox + 32; x++) {
        put_pixel(x, oy, 45, 40, 32, 255);
        put_pixel(x, oy + 31, 45, 40, 32, 255);
    }
    for (int y = oy; y < oy + 32; y++) {
        put_pixel(ox, y, 45, 40, 32, 255);
        put_pixel(ox + 31, y, 45, 40, 32, 255);
    }
}

static void gen_tile_buildable(int ox, int oy)
{
    /* Prepared defensive position — flattened earth with sandbag outlines */
    /* Slightly lighter packed earth (distinguishable from raw ground) */
    fill_rect(ox, oy, 32, 32, 70, 62, 48, 255);

    /* Tamped-down earth texture */
    for (int i = 0; i < 20; i++) {
        int px = ox + rng_range(32);
        int py = oy + rng_range(32);
        put_pixel(px, py, 78, 70, 54, 255);
    }
    for (int i = 0; i < 10; i++) {
        int px = ox + rng_range(32);
        int py = oy + rng_range(32);
        put_pixel(px, py, 55, 48, 36, 255);
    }

    /* Sandbag-style border (khaki colored dashes) */
    uint8_t br = COL_KHAKI_R, bg = COL_KHAKI_G, bb = COL_KHAKI_B;
    /* Top edge */
    for (int x = ox; x < ox + 32; x += 4) {
        put_pixel(x, oy, br, bg, bb, 255);
        put_pixel(x + 1, oy, br, bg, bb, 255);
    }
    /* Bottom edge */
    for (int x = ox; x < ox + 32; x += 4) {
        put_pixel(x, oy + 31, br, bg, bb, 255);
        put_pixel(x + 1, oy + 31, br, bg, bb, 255);
    }
    /* Left edge */
    for (int y = oy; y < oy + 32; y += 4) {
        put_pixel(ox, y, br, bg, bb, 255);
        put_pixel(ox, y + 1, br, bg, bb, 255);
    }
    /* Right edge */
    for (int y = oy; y < oy + 32; y += 4) {
        put_pixel(ox + 31, y, br, bg, bb, 255);
        put_pixel(ox + 31, y + 1, br, bg, bb, 255);
    }

    /* Small corner markers for extra clarity */
    put_pixel(ox + 1, oy + 1, br, bg, bb, 255);
    put_pixel(ox + 30, oy + 1, br, bg, bb, 255);
    put_pixel(ox + 1, oy + 30, br, bg, bb, 255);
    put_pixel(ox + 30, oy + 30, br, bg, bb, 255);
}

/* ------------------------------------------------------------------ */
/* Tower sprite generators (32x32 each, transparent background)         */
/* ------------------------------------------------------------------ */

static void gen_tower_machine_gun(int ox, int oy)
{
    /* Sandbag semicircle: 3-4 lumpy rows forming a half-circle wall */
    /* Bottom row of sandbags (wide) */
    for (int i = 0; i < 5; i++) {
        int bx = ox + 4 + i * 5;
        int by = oy + 24;
        fill_rect(bx, by, 5, 3, COL_KHAKI_R, COL_KHAKI_G, COL_KHAKI_B, 255);
        /* Sandbag shadow/outline */
        put_pixel(bx, by + 2, 160, 144, 116, 255);
        put_pixel(bx + 4, by + 2, 160, 144, 116, 255);
        /* Highlight top */
        put_pixel(bx + 2, by, 210, 195, 168, 255);
    }

    /* Middle row of sandbags (narrower, curved) */
    for (int i = 0; i < 4; i++) {
        int bx = ox + 6 + i * 5;
        int by = oy + 20;
        fill_rect(bx, by, 5, 3, COL_KHAKI_R, COL_KHAKI_G, COL_KHAKI_B, 255);
        put_pixel(bx + 2, by, 210, 195, 168, 255);
        put_pixel(bx, by + 2, 160, 144, 116, 255);
    }

    /* Top row (narrowest) */
    for (int i = 0; i < 3; i++) {
        int bx = ox + 8 + i * 5;
        int by = oy + 16;
        fill_rect(bx, by, 5, 3, COL_KHAKI_R, COL_KHAKI_G, COL_KHAKI_B, 255);
        put_pixel(bx + 2, by, 210, 195, 168, 255);
    }

    /* Gun barrel: thin horizontal dark gray line protruding from center-top */
    fill_rect(ox + 14, oy + 12, 8, 2, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    /* Barrel highlight */
    put_pixel(ox + 21, oy + 12, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
    /* Muzzle */
    put_pixel(ox + 22, oy + 12, 80, 80, 80, 255);
    put_pixel(ox + 22, oy + 13, 80, 80, 80, 255);

    /* Gun body (rear, behind barrel) */
    fill_rect(ox + 12, oy + 11, 4, 4, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    /* Ammo box */
    fill_rect(ox + 10, oy + 14, 4, 3, 80, 60, 40, 255);
}

static void gen_tower_mortar(int ox, int oy)
{
    /* Circular pit: dark brown ring on ground */
    draw_circle_outline(ox + 16, oy + 22, 8, 72, 50, 38, 255);
    draw_circle_outline(ox + 16, oy + 22, 7, 82, 58, 44, 255);
    /* Pit interior (slightly darker) */
    draw_circle(ox + 16, oy + 22, 6, 62, 42, 32, 255);

    /* Base plate (dark rectangle at bottom of mortar) */
    fill_rect(ox + 12, oy + 24, 8, 3, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);

    /* Mortar tube: angled from base plate up-right at ~60 degrees */
    /* Draw as a thick angled line */
    for (int t = 0; t < 12; t++) {
        int px = ox + 15 + t / 3;
        int py = oy + 23 - t;
        fill_rect(px, py, 3, 2, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
    }

    /* Tube opening (lighter at top) */
    put_pixel(ox + 18, oy + 11, 140, 148, 155, 255);
    put_pixel(ox + 19, oy + 11, 140, 148, 155, 255);

    /* Bipod legs */
    draw_line(ox + 14, oy + 18, ox + 10, oy + 26, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    draw_line(ox + 18, oy + 18, ox + 22, oy + 26, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
}

static void gen_tower_sniper(int ox, int oy)
{
    /* Sandbag hide / nest at the base */
    /* Back sandbag wall (semicircle of khaki lumps) */
    fill_rect(ox + 4, oy + 22, 24, 4, COL_KHAKI_R, COL_KHAKI_G, COL_KHAKI_B, 255);
    fill_rect(ox + 6, oy + 20, 20, 2, COL_KHAKI_R, COL_KHAKI_G, COL_KHAKI_B, 255);
    fill_rect(ox + 8, oy + 19, 16, 1, COL_KHAKI_R - 10, COL_KHAKI_G - 10, COL_KHAKI_B - 10, 255);
    /* Sandbag detail lines */
    for (int sx = ox + 5; sx < ox + 27; sx += 5) {
        put_pixel(sx, oy + 23, COL_KHAKI_R - 25, COL_KHAKI_G - 25, COL_KHAKI_B - 25, 255);
        put_pixel(sx, oy + 21, COL_KHAKI_R - 20, COL_KHAKI_G - 20, COL_KHAKI_B - 20, 255);
    }
    /* Front sandbag wall (lower) */
    fill_rect(ox + 3, oy + 26, 26, 3, COL_KHAKI_R - 15, COL_KHAKI_G - 15, COL_KHAKI_B - 15, 255);
    fill_rect(ox + 5, oy + 29, 22, 2, COL_KHAKI_R - 20, COL_KHAKI_G - 20, COL_KHAKI_B - 20, 255);

    /* Sniper soldier (prone/crouching behind sandbags) */
    /* Helmet */
    fill_rect(ox + 12, oy + 13, 5, 4, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    fill_rect(ox + 11, oy + 15, 7, 1, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    /* Face */
    fill_rect(ox + 13, oy + 17, 3, 2, 175, 156, 125, 255);
    /* Torso (crouching, olive drab) */
    fill_rect(ox + 11, oy + 19, 8, 3, COL_OLIVE_R + 15, COL_OLIVE_G + 15, COL_OLIVE_B + 15, 255);
    /* Arms reaching forward to rifle */
    fill_rect(ox + 18, oy + 18, 4, 2, COL_OLIVE_R + 10, COL_OLIVE_G + 10, COL_OLIVE_B + 10, 255);

    /* Sniper rifle (long barrel extending right) */
    fill_rect(ox + 20, oy + 16, 10, 1, 50, 40, 35, 255); /* barrel */
    fill_rect(ox + 19, oy + 17, 5, 2, 70, 55, 40, 255);  /* stock */
    /* Scope on top of barrel */
    fill_rect(ox + 22, oy + 15, 4, 1, 60, 60, 65, 255);
    /* Scope glint */
    put_pixel(ox + 24, oy + 14, 255, 255, 220, 255);
    put_pixel(ox + 25, oy + 14, 200, 200, 180, 180);

    /* Camouflage netting draped over position */
    for (int i = 0; i < 6; i++) {
        int nx = ox + 6 + rng_range(20);
        int ny = oy + 18 + rng_range(4);
        put_pixel(nx, ny, COL_OLIVE_R - 5, COL_OLIVE_G + 5, COL_OLIVE_B - 5, 100);
    }
}

static void gen_tower_barbed_wire(int ox, int oy)
{
    /* Low-profile X-shaped wire pattern in steel gray */
    /* Multiple X crosses across the tile */
    for (int i = 0; i < 3; i++) {
        int cx = ox + 6 + i * 10;
        int cy = oy + 16;

        /* X shape */
        draw_line(cx - 4, cy - 4, cx + 4, cy + 4, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
        draw_line(cx + 4, cy - 4, cx - 4, cy + 4, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);

        /* Barb dots at intersections and midpoints */
        put_pixel(cx, cy, 180, 185, 190, 255);
        put_pixel(cx - 2, cy - 2, 180, 185, 190, 255);
        put_pixel(cx + 2, cy - 2, 180, 185, 190, 255);
        put_pixel(cx - 2, cy + 2, 180, 185, 190, 255);
        put_pixel(cx + 2, cy + 2, 180, 185, 190, 255);

        /* Additional barbs as tiny perpendicular ticks */
        put_pixel(cx - 3, cy - 2, 180, 185, 190, 255);
        put_pixel(cx + 3, cy + 2, 180, 185, 190, 255);
        put_pixel(cx - 1, cy + 3, 180, 185, 190, 255);
        put_pixel(cx + 1, cy - 3, 180, 185, 190, 255);
    }

    /* Connecting horizontal wire between the crosses */
    for (int x = ox + 2; x < ox + 30; x++) {
        int wobble = (x % 3 == 0) ? -1 : 0;
        put_pixel(x, oy + 16 + wobble, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
    }
    /* Second parallel wire */
    for (int x = ox + 3; x < ox + 29; x++) {
        int wobble = (x % 4 == 0) ? 1 : 0;
        put_pixel(x, oy + 20 + wobble, COL_STEEL_R - 10, COL_STEEL_G - 10, COL_STEEL_B - 10, 255);
    }

    /* Ground stakes at ends */
    fill_rect(ox + 2, oy + 22, 2, 6, 90, 64, 42, 255);
    fill_rect(ox + 28, oy + 22, 2, 6, 90, 64, 42, 255);
}

static void gen_tower_artillery(int ox, int oy)
{
    /* === Dicke Bertha (Big Bertha) — M-Gerät 42cm siege howitzer ===
       Iconic WW1 super-heavy mortar. Massive stubby barrel angled upward
       on a fixed steel platform. The barrel is extremely thick and short
       compared to normal artillery — it's a mortar-like siege weapon. */

    /* === Concrete/steel firing platform === */
    /* Heavy base plate bolted to ground */
    fill_rect(ox + 1, oy + 24, 30, 7, 58, 60, 64, 255);
    fill_rect(ox + 0, oy + 25, 32, 6, 52, 54, 58, 255);
    /* Riveted steel plate texture */
    for (int rx = ox + 2; rx < ox + 30; rx += 5) {
        put_pixel(rx, oy + 25, 75, 78, 82, 255);
        put_pixel(rx, oy + 29, 75, 78, 82, 255);
    }
    /* Platform edge highlight */
    fill_rect(ox + 0, oy + 24, 32, 1, 70, 72, 78, 255);
    /* Dark underside */
    fill_rect(ox + 0, oy + 30, 32, 1, 38, 40, 44, 255);

    /* === Rotating turntable ring === */
    draw_circle_outline(ox + 16, oy + 24, 10, 65, 68, 72, 255);
    draw_circle_outline(ox + 16, oy + 24, 9, 55, 58, 62, 255);

    /* === Massive cradle / mounting === */
    fill_rect(ox + 8, oy + 18, 16, 7, 52, 55, 60, 255);
    fill_rect(ox + 8, oy + 18, 16, 1, 68, 72, 78, 255); /* top highlight */
    /* Elevation mechanism (arc gear) */
    fill_rect(ox + 22, oy + 16, 3, 8, 60, 64, 68, 255);
    put_pixel(ox + 23, oy + 17, 78, 82, 88, 255);
    put_pixel(ox + 23, oy + 19, 78, 82, 88, 255);
    put_pixel(ox + 23, oy + 21, 78, 82, 88, 255);

    /* === THE BARREL — one massive solid cannon tube angled upward === */
    for (int t = 0; t < 16; t++) {
        int bx = ox + 15 - t;
        int by = oy + 18 - t;
        int width = 10;
        fill_rect(bx - width/2, by, width, 2, 48, 50, 55, 255);
        /* Top highlight for cylindrical look */
        fill_rect(bx - width/2, by, width, 1, 62, 66, 72, 255);
        /* Bottom shadow */
        fill_rect(bx - width/2, by + 1, width, 1, 40, 42, 48, 255);
    }
    /* Dark muzzle bore at the tip */
    fill_rect(ox + 0, oy + 2, 8, 2, 28, 28, 32, 255);
}

static void gen_tower_gas(int ox, int oy)
{
    /* Cylindrical canister body in mustard yellow */
    fill_rect(ox + 10, oy + 10, 12, 18, COL_MUSTARD_R, COL_MUSTARD_G, COL_MUSTARD_B, 255);
    /* Rounded top */
    fill_rect(ox + 11, oy + 8, 10, 2, COL_MUSTARD_R, COL_MUSTARD_G, COL_MUSTARD_B, 255);
    fill_rect(ox + 12, oy + 7, 8, 1, COL_MUSTARD_R, COL_MUSTARD_G, COL_MUSTARD_B, 255);
    /* Rounded bottom */
    fill_rect(ox + 11, oy + 28, 10, 1, COL_MUSTARD_R - 30, COL_MUSTARD_G - 30, COL_MUSTARD_B, 255);

    /* Dark bands across canister */
    fill_rect(ox + 10, oy + 14, 12, 2, 160, 120, 1, 255);
    fill_rect(ox + 10, oy + 22, 12, 2, 160, 120, 1, 255);

    /* Cylinder highlight (left side lighter) */
    fill_rect(ox + 11, oy + 10, 2, 18, 240, 190, 20, 255);
    /* Right side shadow */
    fill_rect(ox + 20, oy + 10, 2, 18, 180, 138, 1, 255);

    /* Valve/nozzle at top */
    fill_rect(ox + 14, oy + 5, 4, 3, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
    fill_rect(ox + 15, oy + 4, 2, 1, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);

    /* Gas cloud puffs at top (semi-transparent yellow-green) */
    draw_circle(ox + 14, oy + 3, 3, 180, 200, 40, 140);
    draw_circle(ox + 19, oy + 2, 2, 190, 210, 50, 120);
    draw_circle(ox + 10, oy + 4, 2, 170, 190, 30, 100);
    /* Wispy top */
    put_pixel_blend(ox + 16, oy + 0, 200, 220, 60, 80);
    put_pixel_blend(ox + 18, oy + 0, 200, 220, 60, 60);
    put_pixel_blend(ox + 12, oy + 1, 190, 210, 50, 70);
}

static void gen_tower_flamethrower(int ox, int oy)
{
    /* Bunker shape: dark gray rectangle with rounded top */
    fill_rect(ox + 6, oy + 16, 16, 12, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    /* Rounded top */
    fill_rect(ox + 7, oy + 14, 14, 2, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    fill_rect(ox + 8, oy + 13, 12, 1, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    fill_rect(ox + 9, oy + 12, 10, 1, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    /* Bunker slit (dark opening) */
    fill_rect(ox + 10, oy + 17, 8, 2, 30, 30, 30, 255);

    /* Nozzle protruding from slit */
    fill_rect(ox + 20, oy + 16, 6, 3, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
    /* Nozzle tip */
    fill_rect(ox + 25, oy + 15, 2, 5, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);

    /* Flame tongues emerging from nozzle (3 flame shapes) */
    /* Main flame (center) */
    fill_rect(ox + 26, oy + 14, 2, 5, COL_ORANGE_R, COL_ORANGE_G, COL_ORANGE_B, 255);
    fill_rect(ox + 28, oy + 13, 2, 5, 230, 120, 10, 255);
    put_pixel(ox + 30, oy + 14, 255, 180, 40, 255);
    put_pixel(ox + 30, oy + 16, 255, 160, 20, 255);

    /* Upper flame tongue */
    put_pixel(ox + 27, oy + 12, COL_ORANGE_R, COL_ORANGE_G, COL_ORANGE_B, 255);
    put_pixel(ox + 28, oy + 11, 240, 140, 20, 255);
    put_pixel(ox + 29, oy + 10, 255, 200, 60, 255);
    put_pixel(ox + 29, oy + 12, 230, 110, 10, 255);

    /* Lower flame tongue */
    put_pixel(ox + 27, oy + 20, COL_ORANGE_R, COL_ORANGE_G, COL_ORANGE_B, 255);
    put_pixel(ox + 28, oy + 21, 240, 140, 20, 255);
    put_pixel(ox + 29, oy + 20, 255, 180, 40, 255);

    /* Bright core of flames */
    put_pixel(ox + 26, oy + 16, 255, 230, 100, 255);
    put_pixel(ox + 27, oy + 15, 255, 240, 120, 255);

    /* Fuel line on bunker */
    draw_line(ox + 8, oy + 20, ox + 8, oy + 27, 80, 80, 60, 255);
    /* Fuel tank below */
    fill_rect(ox + 6, oy + 27, 6, 3, 80, 60, 40, 255);
}

static void gen_tower_observation(int ox, int oy)
{
    /* WW1 Drachen (kite balloon) — side profile view
       The Ae 800 / Drachen was a sausage-shaped tethered balloon with
       a bulbous nose, tapering tail, and three stabilizer fins at the rear.
       Typically olive drab or field gray, not sky blue. */

    /* === Main envelope (horizontal sausage shape) === */
    /* The balloon is drawn sideways: nose on the left, tail on the right */

    /* Colors: authentic field gray / olive drab fabric */
    uint8_t er = 130, eg = 128, eb = 110; /* envelope base */
    uint8_t hr = 148, hg = 145, hb = 128; /* highlight */
    uint8_t sr = 105, sg = 102, sb = 88;  /* shadow */
    uint8_t dr = 85,  dg = 82,  db = 70;  /* dark underside */

    /* Bulbous nose (left side, fatter) */
    fill_rect(ox + 2, oy + 6, 4, 8, er, eg, eb, 255);
    fill_rect(ox + 1, oy + 7, 2, 6, er, eg, eb, 255);
    fill_rect(ox + 0, oy + 8, 2, 4, er - 5, eg - 5, eb - 5, 255);

    /* Main body (wide middle section) */
    fill_rect(ox + 6, oy + 5, 14, 10, er, eg, eb, 255);
    fill_rect(ox + 6, oy + 4, 12, 1, er, eg, eb, 255);
    fill_rect(ox + 6, oy + 15, 12, 1, er, eg, eb, 255);

    /* Tapering tail (right side, narrower) */
    fill_rect(ox + 20, oy + 6, 4, 8, er, eg, eb, 255);
    fill_rect(ox + 24, oy + 7, 3, 6, er - 5, eg - 5, eb - 5, 255);
    fill_rect(ox + 27, oy + 8, 2, 4, er - 10, eg - 10, eb - 10, 255);
    fill_rect(ox + 29, oy + 9, 1, 2, er - 15, eg - 15, eb - 15, 255);

    /* Top highlight (light hitting the top of the envelope) */
    fill_rect(ox + 4, oy + 5, 16, 2, hr, hg, hb, 255);
    fill_rect(ox + 6, oy + 4, 12, 1, hr + 5, hg + 5, hb + 5, 255);
    put_pixel(ox + 3, oy + 6, hr, hg, hb, 255);

    /* Bottom shadow (underside darker) */
    fill_rect(ox + 4, oy + 14, 16, 1, sr, sg, sb, 255);
    fill_rect(ox + 6, oy + 15, 12, 1, dr, dg, db, 255);
    fill_rect(ox + 20, oy + 13, 6, 1, sr, sg, sb, 255);

    /* Fabric panel seams (horizontal on side view) */
    draw_line(ox + 2, oy + 8, ox + 28, oy + 8, sr - 5, sg - 5, sb - 5, 255);
    draw_line(ox + 2, oy + 12, ox + 28, oy + 12, sr - 5, sg - 5, sb - 5, 255);

    /* Vertical rigging bands */
    for (int bx = ox + 8; bx <= ox + 22; bx += 7) {
        draw_line(bx, oy + 5, bx, oy + 15, sr - 10, sg - 10, sb - 10, 255);
    }

    /* === Stabilizer fins at tail (3 fins visible from side) === */
    /* Top fin */
    fill_rect(ox + 25, oy + 2, 4, 4, er - 8, eg - 8, eb - 8, 255);
    fill_rect(ox + 26, oy + 1, 3, 1, er - 12, eg - 12, eb - 12, 255);
    put_pixel(ox + 29, oy + 3, er - 15, eg - 15, eb - 15, 255);
    /* Bottom fin */
    fill_rect(ox + 25, oy + 14, 4, 4, er - 8, eg - 8, eb - 8, 255);
    fill_rect(ox + 26, oy + 18, 3, 1, er - 12, eg - 12, eb - 12, 255);
    put_pixel(ox + 29, oy + 16, er - 15, eg - 15, eb - 15, 255);

    /* === Rigging net (diamond pattern across envelope) === */
    for (int nx = ox + 5; nx < ox + 24; nx += 4) {
        for (int ny = oy + 6; ny < oy + 14; ny += 4) {
            put_pixel(nx, ny, sr - 15, sg - 15, sb - 15, 200);
            put_pixel(nx + 2, ny + 2, sr - 15, sg - 15, sb - 15, 200);
        }
    }

    /* === Tether/cable lines from belly down to basket === */
    draw_line(ox + 8,  oy + 16, ox + 12, oy + 23, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    draw_line(ox + 14, oy + 16, ox + 14, oy + 23, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    draw_line(ox + 20, oy + 16, ox + 16, oy + 23, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);

    /* === Observation basket === */
    fill_rect(ox + 10, oy + 24, 9, 5, 100, 72, 48, 255);
    /* Rim */
    fill_rect(ox + 9,  oy + 23, 11, 1, 118, 88, 60, 255);
    /* Bottom */
    fill_rect(ox + 10, oy + 29, 9, 1, 82, 58, 38, 255);
    /* Wicker weave */
    for (int bx = ox + 10; bx < ox + 19; bx += 2) {
        put_pixel(bx, oy + 25, 85, 60, 40, 255);
        put_pixel(bx + 1, oy + 27, 85, 60, 40, 255);
    }

    /* Observer (head + shoulders visible above basket rim) */
    fill_rect(ox + 13, oy + 21, 3, 2, COL_KHAKI_R, COL_KHAKI_G, COL_KHAKI_B, 255);
    fill_rect(ox + 13, oy + 20, 3, 1, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    /* Binoculars */
    put_pixel(ox + 16, oy + 21, 50, 50, 55, 255);

    /* Mooring cable to ground */
    draw_line(ox + 14, oy + 29, ox + 14, oy + 31, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
}

/* ------------------------------------------------------------------ */
/* Enemy sprite generators (16x16 each, transparent background)         */
/* ------------------------------------------------------------------ */

static void gen_enemy_infantry(int ox, int oy)
{
    /* Helmet (dark circle on top) */
    fill_rect(ox + 6, oy + 1, 4, 3, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    /* Helmet brim */
    fill_rect(ox + 5, oy + 3, 6, 1, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);

    /* Head/face */
    fill_rect(ox + 6, oy + 4, 4, 2, COL_KHAKI_R - 20, COL_KHAKI_G - 20, COL_KHAKI_B - 20, 255);

    /* Torso (khaki) */
    fill_rect(ox + 5, oy + 6, 6, 5, COL_KHAKI_R, COL_KHAKI_G, COL_KHAKI_B, 255);
    /* Belt */
    fill_rect(ox + 5, oy + 9, 6, 1, 100, 72, 48, 255);

    /* Legs */
    fill_rect(ox + 5, oy + 11, 3, 4, COL_KHAKI_R - 30, COL_KHAKI_G - 30, COL_KHAKI_B - 30, 255);
    fill_rect(ox + 8, oy + 11, 3, 4, COL_KHAKI_R - 30, COL_KHAKI_G - 30, COL_KHAKI_B - 30, 255);

    /* Boots */
    fill_rect(ox + 5, oy + 14, 3, 1, 60, 40, 30, 255);
    fill_rect(ox + 8, oy + 14, 3, 1, 60, 40, 30, 255);

    /* Rifle (thin line to the right) */
    fill_rect(ox + 11, oy + 4, 1, 8, 80, 60, 40, 255);
    /* Bayonet */
    put_pixel(ox + 11, oy + 3, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
}

static void gen_enemy_cavalry(int ox, int oy)
{
    /* Horse body (brown horizontal oval shape) */
    fill_rect(ox + 2, oy + 7, 12, 5, 120, 80, 50, 255);
    /* Horse head */
    fill_rect(ox + 13, oy + 5, 3, 4, 120, 80, 50, 255);
    put_pixel(ox + 14, oy + 4, 120, 80, 50, 255);
    /* Horse ear */
    put_pixel(ox + 14, oy + 3, 100, 65, 40, 255);
    /* Eye */
    put_pixel(ox + 14, oy + 5, 30, 20, 15, 255);

    /* Horse legs (4 thin lines) */
    fill_rect(ox + 3, oy + 12, 1, 3, 100, 65, 40, 255);
    fill_rect(ox + 6, oy + 12, 1, 3, 100, 65, 40, 255);
    fill_rect(ox + 9, oy + 12, 1, 3, 100, 65, 40, 255);
    fill_rect(ox + 12, oy + 12, 1, 3, 100, 65, 40, 255);

    /* Horse tail */
    put_pixel(ox + 1, oy + 7, 100, 65, 40, 255);
    put_pixel(ox + 0, oy + 8, 100, 65, 40, 255);

    /* Rider on top (khaki) */
    fill_rect(ox + 6, oy + 4, 4, 3, COL_KHAKI_R, COL_KHAKI_G, COL_KHAKI_B, 255);
    /* Rider head/helmet */
    fill_rect(ox + 7, oy + 2, 3, 2, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);

    /* Saber */
    draw_line(ox + 10, oy + 3, ox + 12, oy + 1, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
}

static void gen_enemy_stormtrooper(int ox, int oy)
{
    /* Angular helmet (more distinctive than infantry) */
    fill_rect(ox + 5, oy + 0, 6, 3, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    /* Angular visor */
    put_pixel(ox + 4, oy + 2, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    put_pixel(ox + 11, oy + 2, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);

    /* Face */
    fill_rect(ox + 6, oy + 3, 4, 2, 175, 155, 130, 255);

    /* Torso (dark olive/gray -- more military) */
    fill_rect(ox + 4, oy + 5, 8, 5, COL_OLIVE_R + 10, COL_OLIVE_G + 10, COL_OLIVE_B + 10, 255);
    /* Cross-strap/bandolier */
    draw_line(ox + 4, oy + 5, ox + 11, oy + 9, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    /* Belt */
    fill_rect(ox + 4, oy + 9, 8, 1, 80, 60, 40, 255);

    /* Grenades on belt (small dots) */
    put_pixel(ox + 5, oy + 8, COL_OLIVE_R - 20, COL_OLIVE_G - 20, COL_OLIVE_B - 20, 255);
    put_pixel(ox + 10, oy + 8, COL_OLIVE_R - 20, COL_OLIVE_G - 20, COL_OLIVE_B - 20, 255);

    /* Legs */
    fill_rect(ox + 4, oy + 10, 4, 4, COL_OLIVE_R, COL_OLIVE_G, COL_OLIVE_B, 255);
    fill_rect(ox + 8, oy + 10, 4, 4, COL_OLIVE_R, COL_OLIVE_G, COL_OLIVE_B, 255);

    /* Boots */
    fill_rect(ox + 4, oy + 14, 4, 1, 40, 30, 22, 255);
    fill_rect(ox + 8, oy + 14, 4, 1, 40, 30, 22, 255);

    /* MP18 submachine gun */
    fill_rect(ox + 12, oy + 5, 2, 1, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    fill_rect(ox + 12, oy + 6, 3, 1, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    fill_rect(ox + 12, oy + 7, 2, 1, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
}

static void gen_enemy_tank(int ox, int oy)
{
    /* Chunky body: wider than tall (~14x8), steel gray */
    fill_rect(ox + 1, oy + 3, 14, 8, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);

    /* Top hull (slightly lighter, tapered) */
    fill_rect(ox + 3, oy + 1, 10, 2, COL_STEEL_R + 10, COL_STEEL_G + 10, COL_STEEL_B + 10, 255);

    /* Gun barrel protruding from front */
    fill_rect(ox + 14, oy + 5, 2, 2, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    /* Sponson gun */
    fill_rect(ox + 13, oy + 7, 2, 2, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);

    /* Track details at bottom (darker lines) */
    fill_rect(ox + 0, oy + 11, 15, 3, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    /* Track segments */
    for (int x = ox + 1; x < ox + 15; x += 3) {
        put_pixel(x, oy + 12, 80, 90, 96, 255);
    }
    /* Track wheels (lighter dots inside track) */
    for (int x = ox + 2; x < ox + 14; x += 4) {
        put_pixel(x, oy + 11, 90, 100, 106, 255);
    }

    /* Rivet details on hull */
    put_pixel(ox + 3, oy + 4, 130, 138, 144, 255);
    put_pixel(ox + 7, oy + 4, 130, 138, 144, 255);
    put_pixel(ox + 11, oy + 4, 130, 138, 144, 255);
    put_pixel(ox + 3, oy + 9, 130, 138, 144, 255);
    put_pixel(ox + 7, oy + 9, 130, 138, 144, 255);

    /* Exhaust pipe on rear */
    fill_rect(ox + 0, oy + 4, 1, 3, COL_CHARCOAL_R + 20, COL_CHARCOAL_G + 20, COL_CHARCOAL_B + 20, 255);
}

static void gen_enemy_medic(int ox, int oy)
{
    /* Helmet */
    fill_rect(ox + 6, oy + 1, 4, 2, COL_OFFWHITE_R - 20, COL_OFFWHITE_G - 20, COL_OFFWHITE_B - 20, 255);
    fill_rect(ox + 5, oy + 2, 6, 1, COL_OFFWHITE_R - 20, COL_OFFWHITE_G - 20, COL_OFFWHITE_B - 20, 255);

    /* Head/face */
    fill_rect(ox + 6, oy + 3, 4, 2, 175, 155, 130, 255);

    /* Torso (white/off-white) */
    fill_rect(ox + 5, oy + 5, 6, 6, COL_OFFWHITE_R, COL_OFFWHITE_G, COL_OFFWHITE_B, 255);

    /* Red cross on torso (2px cross) -- very recognizable */
    /* Vertical bar of cross */
    fill_rect(ox + 7, oy + 6, 2, 4, COL_BLOOD_R, COL_BLOOD_G, COL_BLOOD_B, 255);
    /* Horizontal bar of cross */
    fill_rect(ox + 6, oy + 7, 4, 2, COL_BLOOD_R, COL_BLOOD_G, COL_BLOOD_B, 255);

    /* Legs */
    fill_rect(ox + 5, oy + 11, 3, 3, COL_KHAKI_R - 20, COL_KHAKI_G - 20, COL_KHAKI_B - 20, 255);
    fill_rect(ox + 8, oy + 11, 3, 3, COL_KHAKI_R - 20, COL_KHAKI_G - 20, COL_KHAKI_B - 20, 255);

    /* Boots */
    fill_rect(ox + 5, oy + 14, 3, 1, 60, 40, 30, 255);
    fill_rect(ox + 8, oy + 14, 3, 1, 60, 40, 30, 255);

    /* Medical bag (small rectangle at side) */
    fill_rect(ox + 11, oy + 6, 3, 3, COL_OFFWHITE_R - 40, COL_OFFWHITE_G - 40, COL_OFFWHITE_B - 40, 255);
    /* Tiny red cross on bag */
    put_pixel(ox + 12, oy + 7, COL_BLOOD_R, COL_BLOOD_G, COL_BLOOD_B, 255);
}

static void gen_enemy_officer(int ox, int oy)
{
    /* Peaked cap (distinctive - wider top) */
    fill_rect(ox + 5, oy + 0, 7, 1, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    fill_rect(ox + 6, oy + 1, 5, 2, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    /* Cap visor (extends forward) */
    fill_rect(ox + 5, oy + 3, 3, 1, COL_CHARCOAL_R - 10, COL_CHARCOAL_G - 10, COL_CHARCOAL_B - 10, 255);
    /* Cap badge (gold dot) */
    put_pixel(ox + 8, oy + 1, COL_MUSTARD_R, COL_MUSTARD_G, COL_MUSTARD_B, 255);

    /* Face */
    fill_rect(ox + 6, oy + 3, 4, 2, 175, 155, 130, 255);

    /* Officer coat (darker than regular infantry) */
    fill_rect(ox + 5, oy + 5, 6, 5, COL_CHARCOAL_R + 20, COL_CHARCOAL_G + 20, COL_CHARCOAL_B + 20, 255);
    /* Buttons (bright dots down center) */
    put_pixel(ox + 8, oy + 6, COL_MUSTARD_R, COL_MUSTARD_G, COL_MUSTARD_B, 255);
    put_pixel(ox + 8, oy + 8, COL_MUSTARD_R, COL_MUSTARD_G, COL_MUSTARD_B, 255);
    /* Belt */
    fill_rect(ox + 5, oy + 9, 6, 1, 90, 64, 42, 255);

    /* Sam Browne belt (diagonal strap) */
    draw_line(ox + 5, oy + 5, ox + 10, oy + 9, 90, 64, 42, 255);

    /* Legs (darker trousers) */
    fill_rect(ox + 5, oy + 10, 3, 4, COL_CHARCOAL_R + 10, COL_CHARCOAL_G + 10, COL_CHARCOAL_B + 10, 255);
    fill_rect(ox + 8, oy + 10, 3, 4, COL_CHARCOAL_R + 10, COL_CHARCOAL_G + 10, COL_CHARCOAL_B + 10, 255);

    /* Boots (polished - slightly shinier) */
    fill_rect(ox + 5, oy + 14, 3, 1, 50, 35, 25, 255);
    fill_rect(ox + 8, oy + 14, 3, 1, 50, 35, 25, 255);

    /* Pistol in hand */
    fill_rect(ox + 11, oy + 6, 2, 1, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    put_pixel(ox + 13, oy + 6, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
}

static void gen_enemy_tunnel_sapper(int ox, int oy)
{
    /* Hunched posture: body slightly tilted/lower */
    /* Cap (soft, no helmet) */
    fill_rect(ox + 6, oy + 2, 4, 2, COL_MUD_R, COL_MUD_G, COL_MUD_B, 255);

    /* Face (peeking under cap) */
    fill_rect(ox + 6, oy + 4, 4, 2, 175, 155, 130, 255);

    /* Hunched torso (earth-toned, leaning forward slightly) */
    fill_rect(ox + 4, oy + 6, 7, 5, COL_MUD_R + 20, COL_MUD_G + 20, COL_MUD_B + 20, 255);
    /* Darker patches (dirty) */
    put_pixel(ox + 5, oy + 7, COL_MUD_R - 10, COL_MUD_G - 10, COL_MUD_B - 10, 255);
    put_pixel(ox + 9, oy + 8, COL_MUD_R - 10, COL_MUD_G - 10, COL_MUD_B - 10, 255);
    /* Belt/harness */
    fill_rect(ox + 4, oy + 9, 7, 1, 70, 50, 36, 255);

    /* Legs (crouching) */
    fill_rect(ox + 4, oy + 11, 3, 3, COL_MUD_R, COL_MUD_G, COL_MUD_B, 255);
    fill_rect(ox + 7, oy + 11, 3, 3, COL_MUD_R, COL_MUD_G, COL_MUD_B, 255);

    /* Boots */
    fill_rect(ox + 4, oy + 14, 3, 1, 50, 35, 25, 255);
    fill_rect(ox + 7, oy + 14, 3, 1, 50, 35, 25, 255);

    /* Shovel: thin handle with wider blade end */
    /* Handle (diagonal, going from shoulder to ground) */
    draw_line(ox + 11, oy + 5, ox + 13, oy + 12, 100, 72, 48, 255);
    /* Shovel blade (wider end at bottom) */
    fill_rect(ox + 12, oy + 12, 3, 3, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
    /* Blade edge */
    fill_rect(ox + 12, oy + 14, 3, 1, 140, 148, 155, 255);
}

static void gen_enemy_armored_car(int ox, int oy)
{
    /* Armored hull: gray rectangle with rounded top */
    fill_rect(ox + 1, oy + 4, 13, 6, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
    /* Rounded top (tapered) */
    fill_rect(ox + 3, oy + 2, 9, 2, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
    fill_rect(ox + 5, oy + 1, 5, 1, COL_STEEL_R + 8, COL_STEEL_G + 8, COL_STEEL_B + 8, 255);

    /* Vision slit */
    fill_rect(ox + 5, oy + 5, 4, 1, 40, 40, 40, 255);

    /* Small turret on top */
    fill_rect(ox + 6, oy + 0, 3, 1, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    /* Machine gun barrel */
    fill_rect(ox + 12, oy + 3, 3, 1, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);

    /* Rivet details */
    put_pixel(ox + 2, oy + 5, 135, 143, 148, 255);
    put_pixel(ox + 12, oy + 5, 135, 143, 148, 255);
    put_pixel(ox + 2, oy + 8, 135, 143, 148, 255);
    put_pixel(ox + 12, oy + 8, 135, 143, 148, 255);

    /* Front bumper */
    fill_rect(ox + 13, oy + 5, 1, 4, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);

    /* Wheels (2 visible on this side + partial) */
    draw_circle(ox + 4, oy + 12, 2, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    put_pixel(ox + 4, oy + 12, 80, 80, 80, 255);  /* hub */
    draw_circle(ox + 11, oy + 12, 2, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    put_pixel(ox + 11, oy + 12, 80, 80, 80, 255);  /* hub */

    /* Mud guard / fender */
    fill_rect(ox + 2, oy + 10, 5, 1, COL_STEEL_R - 10, COL_STEEL_G - 10, COL_STEEL_B - 10, 255);
    fill_rect(ox + 9, oy + 10, 5, 1, COL_STEEL_R - 10, COL_STEEL_G - 10, COL_STEEL_B - 10, 255);
}

/* ------------------------------------------------------------------ */
/* Projectile sprite generators (8x8 each, transparent background)      */
/* ------------------------------------------------------------------ */

static void gen_proj_bullet(int ox, int oy)
{
    /* Tiny bright yellow-white elongated dot (2x4) with 1px bright trail */
    /* Trail */
    put_pixel(ox + 1, oy + 3, 255, 255, 180, 160);
    /* Bullet body */
    put_pixel(ox + 2, oy + 2, 255, 255, 200, 255);
    put_pixel(ox + 2, oy + 3, 255, 255, 200, 255);
    put_pixel(ox + 3, oy + 2, 255, 255, 240, 255);
    put_pixel(ox + 3, oy + 3, 255, 255, 240, 255);
    put_pixel(ox + 4, oy + 2, 255, 255, 255, 255);
    put_pixel(ox + 4, oy + 3, 255, 255, 255, 255);
    put_pixel(ox + 5, oy + 2, 255, 255, 220, 255);
    put_pixel(ox + 5, oy + 3, 255, 255, 220, 255);
    /* Bright tip */
    put_pixel(ox + 6, oy + 3, 255, 255, 200, 200);
}

static void gen_proj_piercing(int ox, int oy)
{
    /* Longer and brighter than regular bullet with a sharp point */
    /* Trail */
    put_pixel(ox + 0, oy + 3, 255, 240, 160, 120);
    put_pixel(ox + 1, oy + 3, 255, 250, 180, 180);
    /* Body - brighter */
    put_pixel(ox + 2, oy + 3, 255, 255, 220, 255);
    put_pixel(ox + 3, oy + 2, 255, 255, 240, 255);
    put_pixel(ox + 3, oy + 3, 255, 255, 240, 255);
    put_pixel(ox + 3, oy + 4, 255, 255, 240, 255);
    put_pixel(ox + 4, oy + 2, 255, 255, 255, 255);
    put_pixel(ox + 4, oy + 3, 255, 255, 255, 255);
    put_pixel(ox + 4, oy + 4, 255, 255, 255, 255);
    put_pixel(ox + 5, oy + 3, 255, 255, 255, 255);
    /* Sharp point at front */
    put_pixel(ox + 6, oy + 3, 255, 255, 255, 255);
    put_pixel(ox + 7, oy + 3, 255, 255, 240, 200);
}

static void gen_proj_explosive(int ox, int oy)
{
    /* Small dark circle shell shape (5x5) in dark gray/brown */
    draw_circle(ox + 3, oy + 4, 2, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    /* Highlight on top-left */
    put_pixel(ox + 2, oy + 3, 80, 90, 96, 255);
    /* Nose point */
    put_pixel(ox + 6, oy + 4, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
    /* Fuse at rear */
    put_pixel(ox + 1, oy + 4, COL_BLOOD_R, COL_BLOOD_G, COL_BLOOD_B, 255);
}

static void gen_proj_heavy_explosive(int ox, int oy)
{
    /* Larger dark circle (6x6) with a band around middle */
    draw_circle(ox + 4, oy + 4, 3, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
    /* Band around middle (lighter ring) */
    put_pixel(ox + 1, oy + 4, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
    put_pixel(ox + 7, oy + 4, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
    put_pixel(ox + 4, oy + 1, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
    put_pixel(ox + 4, oy + 7, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
    /* Highlight */
    put_pixel(ox + 3, oy + 2, 80, 90, 96, 255);
    put_pixel(ox + 2, oy + 3, 80, 90, 96, 255);
    /* Copper driving band */
    fill_rect(ox + 2, oy + 5, 5, 1, 160, 100, 40, 255);
}

static void gen_proj_chemical(int ox, int oy)
{
    /* Yellowish-green cloud puff (cluster of semi-transparent pixels) */
    put_pixel_blend(ox + 3, oy + 3, 180, 200, 40, 180);
    put_pixel_blend(ox + 4, oy + 3, 190, 210, 50, 200);
    put_pixel_blend(ox + 3, oy + 4, 190, 210, 50, 200);
    put_pixel_blend(ox + 4, oy + 4, 200, 220, 60, 220);

    put_pixel_blend(ox + 2, oy + 3, 170, 190, 30, 140);
    put_pixel_blend(ox + 5, oy + 3, 170, 190, 30, 140);
    put_pixel_blend(ox + 3, oy + 2, 170, 190, 30, 130);
    put_pixel_blend(ox + 4, oy + 5, 170, 190, 30, 130);

    /* Wispy edges */
    put_pixel_blend(ox + 2, oy + 2, 160, 180, 20, 80);
    put_pixel_blend(ox + 5, oy + 4, 160, 180, 20, 80);
    put_pixel_blend(ox + 1, oy + 4, 150, 170, 20, 60);
    put_pixel_blend(ox + 5, oy + 5, 150, 170, 20, 60);
}

static void gen_proj_fire(int ox, int oy)
{
    /* Small bright orange flame shape (3-4 px wide, flickering top) */
    /* Base (wide, bright) */
    put_pixel(ox + 3, oy + 6, COL_ORANGE_R, COL_ORANGE_G, COL_ORANGE_B, 255);
    put_pixel(ox + 4, oy + 6, COL_ORANGE_R, COL_ORANGE_G, COL_ORANGE_B, 255);
    put_pixel(ox + 5, oy + 6, COL_ORANGE_R, COL_ORANGE_G, COL_ORANGE_B, 255);

    /* Middle section */
    put_pixel(ox + 3, oy + 5, 230, 120, 10, 255);
    put_pixel(ox + 4, oy + 5, 255, 200, 40, 255);  /* hot center */
    put_pixel(ox + 5, oy + 5, 230, 120, 10, 255);

    /* Upper flames */
    put_pixel(ox + 3, oy + 4, 240, 140, 20, 255);
    put_pixel(ox + 4, oy + 4, 255, 230, 80, 255);  /* bright core */
    put_pixel(ox + 5, oy + 4, 220, 100, 5, 255);

    /* Flickering tip */
    put_pixel(ox + 4, oy + 3, 255, 200, 40, 255);
    put_pixel(ox + 3, oy + 3, 240, 140, 20, 200);

    /* Top wisp */
    put_pixel(ox + 4, oy + 2, 255, 220, 60, 160);
    put_pixel(ox + 5, oy + 3, 200, 80, 0, 140);
}

/* ------------------------------------------------------------------ */
/* Animated enemy walk frames                                           */
/*                                                                       */
/* For soldiers: legs alternate positions to create walk cycle.          */
/* For vehicles: tracks/wheels shift for rolling motion.                 */
/* Frame 0 = neutral, 1 = left stride, 2 = neutral, 3 = right stride.  */
/* ------------------------------------------------------------------ */

/* Helper: draw a soldier body (shared by infantry, stormtrooper, medic, officer, sapper) */
static void draw_soldier_body(int ox, int oy, int frame,
                               uint8_t hr, uint8_t hg, uint8_t hb,   /* helmet */
                               uint8_t tr, uint8_t tg, uint8_t tb,   /* torso */
                               uint8_t lr, uint8_t lg, uint8_t lb)   /* legs */
{
    /* Helmet */
    fill_rect(ox + 6, oy + 1, 4, 3, hr, hg, hb, 255);
    fill_rect(ox + 5, oy + 3, 6, 1, hr, hg, hb, 255);

    /* Head/face */
    fill_rect(ox + 6, oy + 4, 4, 2, COL_KHAKI_R - 20, COL_KHAKI_G - 20, COL_KHAKI_B - 20, 255);

    /* Torso */
    fill_rect(ox + 5, oy + 6, 6, 5, tr, tg, tb, 255);
    fill_rect(ox + 5, oy + 9, 6, 1, 100, 72, 48, 255); /* belt */

    /* Animated legs — shift based on frame */
    int left_y  = (frame == 1) ? -1 : (frame == 3) ?  1 : 0;
    int right_y = (frame == 1) ?  1 : (frame == 3) ? -1 : 0;

    /* Left leg */
    fill_rect(ox + 5, oy + 11 + left_y, 3, 3, lr, lg, lb, 255);
    fill_rect(ox + 5, oy + 14 + left_y, 3, 1, 60, 40, 30, 255); /* boot */

    /* Right leg */
    fill_rect(ox + 8, oy + 11 + right_y, 3, 3, lr, lg, lb, 255);
    fill_rect(ox + 8, oy + 14 + right_y, 3, 1, 60, 40, 30, 255); /* boot */

    /* Body bob — torso shifts down slightly on stride frames */
    if (frame == 1 || frame == 3) {
        /* Shoulder bounce pixel */
        put_pixel(ox + 5 + (frame == 1 ? 0 : 5), oy + 6, tr + 10, tg + 10, tb + 10, 255);
    }
}

static void gen_enemy_infantry_frame(int ox, int oy, int frame)
{
    draw_soldier_body(ox, oy, frame,
        COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B,
        COL_KHAKI_R, COL_KHAKI_G, COL_KHAKI_B,
        COL_KHAKI_R - 30, COL_KHAKI_G - 30, COL_KHAKI_B - 30);
    /* Rifle */
    int rifle_bob = (frame == 1 || frame == 3) ? 1 : 0;
    fill_rect(ox + 11, oy + 4 + rifle_bob, 1, 8, 80, 60, 40, 255);
    put_pixel(ox + 11, oy + 3 + rifle_bob, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
}

static void gen_enemy_cavalry_frame(int ox, int oy, int frame)
{
    /* Horse body — slight gallop animation */
    int bob = (frame == 1 || frame == 3) ? 1 : 0;
    fill_rect(ox + 2, oy + 7 - bob, 12, 5, 120, 80, 50, 255);
    /* Horse head */
    fill_rect(ox + 13, oy + 5 - bob, 3, 4, 120, 80, 50, 255);
    put_pixel(ox + 14, oy + 4 - bob, 120, 80, 50, 255);
    /* Eye */
    put_pixel(ox + 14, oy + 5 - bob, 40, 30, 20, 255);
    /* Horse legs — gallop cycle */
    if (frame == 0 || frame == 2) {
        fill_rect(ox + 3, oy + 12, 2, 3, 100, 65, 40, 255);
        fill_rect(ox + 7, oy + 12, 2, 3, 100, 65, 40, 255);
        fill_rect(ox + 11, oy + 12, 2, 3, 100, 65, 40, 255);
    } else if (frame == 1) {
        fill_rect(ox + 2, oy + 12, 2, 3, 100, 65, 40, 255);
        fill_rect(ox + 7, oy + 12, 2, 2, 100, 65, 40, 255);
        fill_rect(ox + 12, oy + 12, 2, 3, 100, 65, 40, 255);
    } else {
        fill_rect(ox + 4, oy + 12, 2, 3, 100, 65, 40, 255);
        fill_rect(ox + 7, oy + 12, 2, 2, 100, 65, 40, 255);
        fill_rect(ox + 10, oy + 12, 2, 3, 100, 65, 40, 255);
    }
    /* Rider */
    fill_rect(ox + 6, oy + 2 - bob, 4, 5, COL_KHAKI_R, COL_KHAKI_G, COL_KHAKI_B, 255);
    fill_rect(ox + 6, oy + 1 - bob, 4, 2, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
}

static void gen_enemy_stormtrooper_frame(int ox, int oy, int frame)
{
    draw_soldier_body(ox, oy, frame,
        60, 65, 60,    /* angular dark helmet */
        COL_OLIVE_R + 10, COL_OLIVE_G + 10, COL_OLIVE_B + 10,
        50, 55, 48);
    /* Angular helmet detail */
    put_pixel(ox + 5, oy + 2, 50, 55, 50, 255);
    put_pixel(ox + 10, oy + 2, 50, 55, 50, 255);
    /* Weapon */
    int bob = (frame == 1 || frame == 3) ? 1 : 0;
    fill_rect(ox + 11, oy + 5 + bob, 2, 6, 70, 55, 40, 255);
}

static void gen_enemy_tank_frame(int ox, int oy, int frame)
{
    /* Tank hull */
    fill_rect(ox + 1, oy + 4, 14, 7, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
    /* Turret */
    fill_rect(ox + 5, oy + 2, 6, 3, COL_STEEL_R - 10, COL_STEEL_G - 10, COL_STEEL_B - 10, 255);
    /* Gun barrel */
    fill_rect(ox + 11, oy + 3, 4, 1, 80, 82, 84, 255);
    /* Track details — rolling animation */
    int track_off = frame % 2;
    for (int x = ox + 1 + track_off; x < ox + 15; x += 3) {
        fill_rect(x, oy + 11, 2, 2, 70, 72, 74, 255);
    }
    fill_rect(ox + 1, oy + 11, 14, 1, 60, 62, 64, 255);
    /* Track top */
    fill_rect(ox + 1, oy + 13, 14, 1, 55, 57, 59, 255);
    /* Exhaust smoke on odd frames */
    if (frame == 1 || frame == 3) {
        put_pixel(ox + 1, oy + 3, 80, 80, 80, 120);
        put_pixel(ox + 0, oy + 2, 70, 70, 70, 80);
    }
}

static void gen_enemy_medic_frame(int ox, int oy, int frame)
{
    draw_soldier_body(ox, oy, frame,
        COL_OFFWHITE_R, COL_OFFWHITE_G, COL_OFFWHITE_B,
        COL_OFFWHITE_R - 10, COL_OFFWHITE_G - 10, COL_OFFWHITE_B - 10,
        COL_OFFWHITE_R - 40, COL_OFFWHITE_G - 40, COL_OFFWHITE_B - 40);
    /* Red cross on torso */
    fill_rect(ox + 7, oy + 7, 2, 1, COL_BLOOD_R, COL_BLOOD_G, COL_BLOOD_B, 255);
    fill_rect(ox + 6, oy + 7, 4, 1, COL_BLOOD_R, COL_BLOOD_G, COL_BLOOD_B, 255);
    fill_rect(ox + 7, oy + 6, 2, 3, COL_BLOOD_R, COL_BLOOD_G, COL_BLOOD_B, 255);
    /* Medical bag (swings with walk) */
    int bag_off = (frame == 1) ? -1 : (frame == 3) ? 1 : 0;
    fill_rect(ox + 3 + bag_off, oy + 8, 2, 3, COL_OFFWHITE_R - 30, COL_OFFWHITE_G - 30, COL_OFFWHITE_B - 30, 255);
}

static void gen_enemy_officer_frame(int ox, int oy, int frame)
{
    draw_soldier_body(ox, oy, frame,
        60, 50, 40,    /* peaked cap */
        80, 60, 45,    /* darker uniform */
        60, 45, 35);
    /* Peaked cap brim */
    fill_rect(ox + 4, oy + 3, 8, 1, 50, 40, 30, 255);
    put_pixel(ox + 5, oy + 1, 70, 55, 40, 255); /* cap badge */
    /* Sword/pistol */
    int bob = (frame == 1 || frame == 3) ? 1 : 0;
    fill_rect(ox + 12, oy + 6 + bob, 1, 5, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
}

static void gen_enemy_sapper_frame(int ox, int oy, int frame)
{
    draw_soldier_body(ox, oy, frame,
        COL_MUD_R, COL_MUD_G, COL_MUD_B,
        COL_MUD_R + 10, COL_MUD_G + 10, COL_MUD_B + 10,
        COL_MUD_R - 10, COL_MUD_G - 10, COL_MUD_B - 10);
    /* Shovel (swings with walk) */
    int swing = (frame == 1) ? -1 : (frame == 3) ? 1 : 0;
    fill_rect(ox + 12, oy + 4 + swing, 1, 7, 90, 70, 50, 255);
    fill_rect(ox + 11, oy + 4 + swing, 3, 2, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
}

static void gen_enemy_armored_car_frame(int ox, int oy, int frame)
{
    /* Hull */
    fill_rect(ox + 2, oy + 4, 12, 6, COL_STEEL_R - 10, COL_STEEL_G - 10, COL_STEEL_B - 10, 255);
    /* Rounded top (armored hood) */
    fill_rect(ox + 4, oy + 2, 8, 3, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
    /* Gun slit */
    fill_rect(ox + 12, oy + 5, 2, 1, 40, 40, 42, 255);
    /* Wheels — rolling animation */
    int w_off = frame % 2;
    draw_circle(ox + 4, oy + 11, 2, 50, 50, 52, 255);
    draw_circle(ox + 12, oy + 11, 2, 50, 50, 52, 255);
    /* Wheel spokes rotate */
    if (w_off) {
        put_pixel(ox + 4, oy + 10, 70, 70, 72, 255);
        put_pixel(ox + 12, oy + 12, 70, 70, 72, 255);
    } else {
        put_pixel(ox + 3, oy + 11, 70, 70, 72, 255);
        put_pixel(ox + 13, oy + 11, 70, 70, 72, 255);
    }
    /* Axle line */
    fill_rect(ox + 4, oy + 12, 8, 1, 60, 60, 62, 255);
}

/* Generate all 4 frames for one enemy type */
typedef void (*enemy_frame_fn)(int ox, int oy, int frame);

static const enemy_frame_fn s_enemy_frame_fns[ENEMY_TYPE_COUNT] = {
    gen_enemy_infantry_frame,
    gen_enemy_cavalry_frame,
    gen_enemy_stormtrooper_frame,
    gen_enemy_tank_frame,
    gen_enemy_medic_frame,
    gen_enemy_officer_frame,
    gen_enemy_sapper_frame,
    gen_enemy_armored_car_frame,
};

/* ------------------------------------------------------------------ */
/* Atlas layout constants                                               */
/* ------------------------------------------------------------------ */
#define TILE_SIZE       32
#define TOWER_SIZE      32
#define ENEMY_SIZE      16
#define PROJ_SIZE        8

#define TILE_ROW_Y       0
#define TOWER_ROW_Y     32
#define ENEMY_ROW_Y     64
#define ENEMY_ANIM_ROW_Y 128  /* 4 rows of 8 enemies × 16px = y=128..191 */
#define PROJ_ROW_Y      80
#define TRENCH_ROW_Y    96

/* ------------------------------------------------------------------ */
/* Compute a TextureRegion from pixel coordinates                       */
/* ------------------------------------------------------------------ */
static TextureRegion make_region(Texture *tex, int x, int y, int w, int h)
{
    TextureRegion r;
    r.texture = tex;
    r.u0 = (float)x / (float)ATLAS_SIZE;
    r.v0 = (float)y / (float)ATLAS_SIZE;
    r.u1 = (float)(x + w) / (float)ATLAS_SIZE;
    r.v1 = (float)(y + h) / (float)ATLAS_SIZE;
    return r;
}

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */

bool atlas_generate(GameAtlas *atlas)
{
    /* Allocate RGBA pixel buffer */
    size_t buf_size = (size_t)ATLAS_SIZE * ATLAS_SIZE * 4;
    pixels = (uint8_t *)malloc(buf_size);
    if (!pixels) {
        LOG_ERROR("Failed to allocate %zu bytes for atlas pixel buffer", buf_size);
        return false;
    }

    /* Clear to fully transparent black */
    memset(pixels, 0, buf_size);

    /* Seed RNG for deterministic procedural detail */
    srand(42);

    /* ---- Generate terrain tiles (Row 0) ---- */
    gen_tile_grass(    0 * TILE_SIZE, TILE_ROW_Y);
    gen_tile_mud(      1 * TILE_SIZE, TILE_ROW_Y);
    gen_tile_trench(   2 * TILE_SIZE, TILE_ROW_Y);
    gen_tile_stone(    3 * TILE_SIZE, TILE_ROW_Y);
    gen_tile_water(    4 * TILE_SIZE, TILE_ROW_Y);
    gen_tile_buildable(5 * TILE_SIZE, TILE_ROW_Y);

    /* Solid white 4x4 pixel for tinted colored shapes (health bars, etc.) */
    fill_rect(ATLAS_SIZE - 4, 0, 4, 4, 255, 255, 255, 255);

    /* Soft circle (16x16 radial gradient) for gas/smoke rendering */
    {
        int scx = ATLAS_SIZE - 20; /* place at (492, 0) */
        int scy = 0;
        int scr = 8; /* radius */
        for (int dy = -scr; dy < scr; dy++) {
            for (int dx = -scr; dx < scr; dx++) {
                float dist = sqrtf((float)(dx * dx + dy * dy));
                if (dist < (float)scr) {
                    float t = dist / (float)scr;
                    /* Smooth falloff: bright center to transparent edge */
                    uint8_t a = (uint8_t)(255.0f * (1.0f - t * t));
                    put_pixel(scx + scr + dx, scy + scr + dy, 255, 255, 255, a);
                }
            }
        }
    }

    /* ---- Generate tower sprites (Row 1) ---- */
    gen_tower_machine_gun(  0 * TOWER_SIZE, TOWER_ROW_Y);
    gen_tower_mortar(       1 * TOWER_SIZE, TOWER_ROW_Y);
    gen_tower_sniper(       2 * TOWER_SIZE, TOWER_ROW_Y);
    gen_tower_barbed_wire(  3 * TOWER_SIZE, TOWER_ROW_Y);
    gen_tower_artillery(    4 * TOWER_SIZE, TOWER_ROW_Y);
    gen_tower_gas(          5 * TOWER_SIZE, TOWER_ROW_Y);
    gen_tower_flamethrower( 6 * TOWER_SIZE, TOWER_ROW_Y);
    gen_tower_observation(  7 * TOWER_SIZE, TOWER_ROW_Y);

    /* ---- Generate enemy sprites (Row 2) ---- */
    gen_enemy_infantry(      0 * ENEMY_SIZE, ENEMY_ROW_Y);
    gen_enemy_cavalry(       1 * ENEMY_SIZE, ENEMY_ROW_Y);
    gen_enemy_stormtrooper(  2 * ENEMY_SIZE, ENEMY_ROW_Y);
    gen_enemy_tank(          3 * ENEMY_SIZE, ENEMY_ROW_Y);
    gen_enemy_medic(         4 * ENEMY_SIZE, ENEMY_ROW_Y);
    gen_enemy_officer(       5 * ENEMY_SIZE, ENEMY_ROW_Y);
    gen_enemy_tunnel_sapper( 6 * ENEMY_SIZE, ENEMY_ROW_Y);
    gen_enemy_armored_car(   7 * ENEMY_SIZE, ENEMY_ROW_Y);

    /* ---- Generate projectile sprites (Row 3) ---- */
    gen_proj_bullet(         0 * PROJ_SIZE, PROJ_ROW_Y);
    gen_proj_piercing(       1 * PROJ_SIZE, PROJ_ROW_Y);
    gen_proj_explosive(      2 * PROJ_SIZE, PROJ_ROW_Y);
    gen_proj_heavy_explosive(3 * PROJ_SIZE, PROJ_ROW_Y);
    gen_proj_chemical(       4 * PROJ_SIZE, PROJ_ROW_Y);
    gen_proj_fire(           5 * PROJ_SIZE, PROJ_ROW_Y);

    /* ---- Generate trench variants (Row 4, 16 variants) ---- */
    for (int m = 0; m < 16; m++) {
        gen_trench_variant(m * TILE_SIZE, TRENCH_ROW_Y, m);
    }

    /* ---- Generate animated enemy frames (4 frames × 8 types) ---- */
    for (int frame = 0; frame < ENEMY_ANIM_FRAMES; frame++) {
        for (int type = 0; type < ENEMY_TYPE_COUNT; type++) {
            int ax = type * ENEMY_SIZE;
            int ay = ENEMY_ANIM_ROW_Y + frame * ENEMY_SIZE;
            s_enemy_frame_fns[type](ax, ay, frame);
        }
    }

    /* ---- Generate no man's land scene (64x64 at y=192) ---- */
    {
        int fx = 0, fy = 192;

        /* Devastated muddy ground base */
        fill_rect(fx, fy, 64, 64, 62, 50, 38, 255);
        for (int i = 0; i < 80; i++) {
            int px = fx + rng_range(64);
            int py = fy + rng_range(64);
            put_pixel(px, py, 52 + rng_range(20), 40 + rng_range(15), 30 + rng_range(12), 255);
        }

        /* Shell craters (water-filled) scattered across */
        for (int c = 0; c < 6; c++) {
            int cx = fx + 8 + rng_range(48);
            int cy = fy + 8 + rng_range(48);
            int cr = 2 + rng_range(3);
            draw_circle(cx, cy, cr + 1, 55, 42, 32, 255); /* rim */
            draw_circle(cx, cy, cr, 45, 48, 42, 255); /* murky water */
        }

        /* === Barbed wire entanglements across the middle === */
        for (int wx = fx + 4; wx < fx + 60; wx += 3) {
            int wy = fy + 28 + rng_range(8);
            put_pixel(wx, wy, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
            put_pixel(wx + 1, wy - 1, COL_STEEL_R, COL_STEEL_G, COL_STEEL_B, 255);
            put_pixel(wx - 1, wy + 1, COL_STEEL_R - 15, COL_STEEL_G - 15, COL_STEEL_B - 15, 255);
        }
        /* Wire posts */
        for (int wp = fx + 8; wp < fx + 58; wp += 12) {
            fill_rect(wp, fy + 26, 1, 6, 70, 55, 42, 255);
        }

        /* === Dead tree stumps (blasted, no leaves) === */
        /* Stump 1 */
        fill_rect(fx + 12, fy + 10, 2, 8, 55, 40, 28, 255);
        put_pixel(fx + 11, fy + 10, 50, 38, 26, 255);
        put_pixel(fx + 14, fy + 11, 48, 35, 24, 255);
        fill_rect(fx + 10, fy + 8, 1, 4, 45, 34, 24, 255); /* broken branch */
        /* Stump 2 */
        fill_rect(fx + 48, fy + 14, 2, 6, 52, 38, 26, 255);
        put_pixel(fx + 50, fy + 14, 48, 35, 24, 255);
        fill_rect(fx + 49, fy + 12, 1, 3, 42, 32, 22, 255);

        /* === Fallen soldiers (small bodies on the ground) === */
        /* Body 1 — lying horizontal */
        fill_rect(fx + 18, fy + 38, 5, 2, COL_KHAKI_R - 30, COL_KHAKI_G - 30, COL_KHAKI_B - 30, 255);
        put_pixel(fx + 17, fy + 38, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255); /* helmet */
        /* Body 2 */
        fill_rect(fx + 40, fy + 22, 4, 2, 50, 55, 45, 255); /* olive uniform */
        put_pixel(fx + 44, fy + 22, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
        /* Body 3 */
        fill_rect(fx + 28, fy + 48, 5, 2, COL_KHAKI_R - 40, COL_KHAKI_G - 40, COL_KHAKI_B - 40, 255);
        put_pixel(fx + 28, fy + 49, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
        /* Dropped rifles */
        fill_rect(fx + 22, fy + 39, 1, 4, 60, 45, 32, 255);
        fill_rect(fx + 44, fy + 24, 4, 1, 58, 42, 30, 255);

        /* === Telegraph/communication lines === */
        /* Poles running left to right */
        for (int tp = fx + 6; tp < fx + 58; tp += 16) {
            fill_rect(tp, fy + 4, 1, 10, 65, 50, 38, 255); /* pole */
            fill_rect(tp - 1, fy + 4, 3, 1, 58, 44, 32, 255); /* crossbar */
        }
        /* Wire between poles */
        for (int wx = fx + 6; wx < fx + 54; wx++) {
            int sag = (wx % 16 < 8) ? (wx % 16) / 3 : (16 - wx % 16) / 3;
            put_pixel(wx, fy + 5 + sag, 40, 40, 45, 200);
        }

        /* === Supply line / duckboard track === */
        /* Wooden planks running top to bottom on left side */
        for (int py = fy + 2; py < fy + 62; py += 3) {
            fill_rect(fx + 3, py, 4, 2, 80, 62, 44, 255);
            put_pixel(fx + 3, py, 72, 56, 38, 255);
        }
        /* Another on right side */
        for (int py = fy + 4; py < fy + 60; py += 3) {
            fill_rect(fx + 57, py, 4, 2, 78, 60, 42, 255);
            put_pixel(fx + 57, py, 70, 54, 36, 255);
        }

        /* === Sandbag positions (small defensive posts) === */
        /* Left side trench edge */
        fill_rect(fx + 1, fy + 18, 5, 3, COL_KHAKI_R, COL_KHAKI_G, COL_KHAKI_B, 255);
        fill_rect(fx + 1, fy + 42, 5, 3, COL_KHAKI_R, COL_KHAKI_G, COL_KHAKI_B, 255);
        /* Right side */
        fill_rect(fx + 58, fy + 18, 5, 3, COL_KHAKI_R - 10, COL_KHAKI_G - 10, COL_KHAKI_B - 10, 255);
        fill_rect(fx + 58, fy + 42, 5, 3, COL_KHAKI_R - 10, COL_KHAKI_G - 10, COL_KHAKI_B - 10, 255);

        /* === Abandoned equipment === */
        /* Overturned cart */
        fill_rect(fx + 34, fy + 54, 6, 3, 70, 52, 36, 255);
        draw_circle(fx + 35, fy + 57, 2, COL_CHARCOAL_R, COL_CHARCOAL_G, COL_CHARCOAL_B, 255);
        /* Ammo boxes */
        fill_rect(fx + 50, fy + 40, 3, 2, 72, 55, 38, 255);
        fill_rect(fx + 8, fy + 52, 3, 2, 68, 52, 36, 255);

        /* === Gas cloud remnants (faint yellow patches) === */
        for (int g = 0; g < 5; g++) {
            int gx = fx + 15 + rng_range(35);
            int gy = fy + 20 + rng_range(25);
            put_pixel_blend(gx, gy, 160, 150, 40, 30);
            put_pixel_blend(gx + 1, gy, 150, 145, 35, 25);
            put_pixel_blend(gx, gy + 1, 155, 148, 38, 20);
        }

        /* === Muddy puddle reflections === */
        for (int p = 0; p < 4; p++) {
            int px = fx + 10 + rng_range(44);
            int py = fy + 10 + rng_range(44);
            put_pixel(px, py, 55, 58, 52, 255);
            put_pixel(px + 1, py, 52, 56, 50, 255);
        }
    }

    /* ---- Upload to OpenGL ---- */
    uint32_t tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    /* Pixel-art filtering: nearest neighbour */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    /* Clamp to edge to avoid bleeding at atlas borders */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                 ATLAS_SIZE, ATLAS_SIZE, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glBindTexture(GL_TEXTURE_2D, 0);

    /* Free the CPU-side pixel buffer */
    free(pixels);
    pixels = NULL;

    /* Store texture handle */
    atlas->texture.id       = tex_id;
    atlas->texture.width    = ATLAS_SIZE;
    atlas->texture.height   = ATLAS_SIZE;
    atlas->texture.channels = 4;

    /* ---- Compute texture regions ---- */

    /* Tiles */
    for (int i = 0; i < TILE_COUNT; i++) {
        atlas->tiles[i] = make_region(&atlas->texture,
                                      i * TILE_SIZE, TILE_ROW_Y,
                                      TILE_SIZE, TILE_SIZE);
    }

    /* Trench variants */
    for (int i = 0; i < TRENCH_VARIANT_COUNT; i++) {
        atlas->trench_variants[i] = make_region(&atlas->texture,
                                                 i * TILE_SIZE, TRENCH_ROW_Y,
                                                 TILE_SIZE, TILE_SIZE);
    }

    /* Towers */
    for (int i = 0; i < TOWER_TYPE_COUNT; i++) {
        atlas->towers[i] = make_region(&atlas->texture,
                                       i * TOWER_SIZE, TOWER_ROW_Y,
                                       TOWER_SIZE, TOWER_SIZE);
    }

    /* Enemies (static — frame 0) */
    for (int i = 0; i < ENEMY_TYPE_COUNT; i++) {
        atlas->enemies[i] = make_region(&atlas->texture,
                                        i * ENEMY_SIZE, ENEMY_ROW_Y,
                                        ENEMY_SIZE, ENEMY_SIZE);
    }

    /* Enemy animation frames */
    for (int type = 0; type < ENEMY_TYPE_COUNT; type++) {
        for (int frame = 0; frame < ENEMY_ANIM_FRAMES; frame++) {
            atlas->enemy_anim[type][frame] = make_region(&atlas->texture,
                type * ENEMY_SIZE,
                ENEMY_ANIM_ROW_Y + frame * ENEMY_SIZE,
                ENEMY_SIZE, ENEMY_SIZE);
        }
    }

    /* Projectiles */
    for (int i = 0; i < DAMAGE_TYPE_COUNT; i++) {
        atlas->projectiles[i] = make_region(&atlas->texture,
                                            i * PROJ_SIZE, PROJ_ROW_Y,
                                            PROJ_SIZE, PROJ_SIZE);
    }

    /* White pixel */
    atlas->white = make_region(&atlas->texture,
                               ATLAS_SIZE - 4, 0, 4, 4);

    /* Soft circle */
    atlas->soft_circle = make_region(&atlas->texture,
                                     ATLAS_SIZE - 20, 0, 16, 16);

    /* Fort (64x64 at y=192) */
    atlas->fort = make_region(&atlas->texture, 0, 192, 64, 64);

    LOG_INFO("Generated %dx%d texture atlas (id=%u): %d tiles, %d towers, %d enemies, %d projectiles",
             ATLAS_SIZE, ATLAS_SIZE, tex_id,
             TILE_COUNT, TOWER_TYPE_COUNT, ENEMY_TYPE_COUNT, DAMAGE_TYPE_COUNT);

    return true;
}

void atlas_destroy(GameAtlas *atlas)
{
    texture_destroy(&atlas->texture);
}

int atlas_trench_mask(Map *map, int x, int y)
{
    int mask = 0;
    Tile *t;
    /* East */
    t = map_get_tile(map, x + 1, y);
    if (t && t->type == TILE_TRENCH) mask |= TRENCH_E;
    /* South */
    t = map_get_tile(map, x, y + 1);
    if (t && t->type == TILE_TRENCH) mask |= TRENCH_S;
    /* West */
    t = map_get_tile(map, x - 1, y);
    if (t && t->type == TILE_TRENCH) mask |= TRENCH_W;
    /* North */
    t = map_get_tile(map, x, y - 1);
    if (t && t->type == TILE_TRENCH) mask |= TRENCH_N;
    return mask;
}
