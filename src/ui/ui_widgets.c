#include "ui_widgets.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/* Built-in 5x7 pixel font                                            */
/*                                                                     */
/* Each character is 5 columns x 7 rows. Stored as uint8_t[5] where   */
/* each byte is one column, bit 0 = top row, bit 6 = bottom row.      */
/* Covers ASCII 32 (' ') through 126 ('~') = 95 characters.           */
/* ------------------------------------------------------------------ */

static const uint8_t font_data[95][5] = {
    /* 32 ' ' */ {0x00, 0x00, 0x00, 0x00, 0x00},
    /* 33 '!' */ {0x00, 0x00, 0x5F, 0x00, 0x00},
    /* 34 '"' */ {0x00, 0x07, 0x00, 0x07, 0x00},
    /* 35 '#' */ {0x14, 0x7F, 0x14, 0x7F, 0x14},
    /* 36 '$' */ {0x24, 0x2A, 0x7F, 0x2A, 0x12},
    /* 37 '%' */ {0x23, 0x13, 0x08, 0x64, 0x62},
    /* 38 '&' */ {0x36, 0x49, 0x55, 0x22, 0x50},
    /* 39 ''' */ {0x00, 0x00, 0x07, 0x00, 0x00},
    /* 40 '(' */ {0x00, 0x1C, 0x22, 0x41, 0x00},
    /* 41 ')' */ {0x00, 0x41, 0x22, 0x1C, 0x00},
    /* 42 '*' */ {0x14, 0x08, 0x3E, 0x08, 0x14},
    /* 43 '+' */ {0x08, 0x08, 0x3E, 0x08, 0x08},
    /* 44 ',' */ {0x00, 0x50, 0x30, 0x00, 0x00},
    /* 45 '-' */ {0x08, 0x08, 0x08, 0x08, 0x08},
    /* 46 '.' */ {0x00, 0x60, 0x60, 0x00, 0x00},
    /* 47 '/' */ {0x20, 0x10, 0x08, 0x04, 0x02},
    /* 48 '0' */ {0x3E, 0x51, 0x49, 0x45, 0x3E},
    /* 49 '1' */ {0x00, 0x42, 0x7F, 0x40, 0x00},
    /* 50 '2' */ {0x42, 0x61, 0x51, 0x49, 0x46},
    /* 51 '3' */ {0x21, 0x41, 0x45, 0x4B, 0x31},
    /* 52 '4' */ {0x18, 0x14, 0x12, 0x7F, 0x10},
    /* 53 '5' */ {0x27, 0x45, 0x45, 0x45, 0x39},
    /* 54 '6' */ {0x3C, 0x4A, 0x49, 0x49, 0x30},
    /* 55 '7' */ {0x01, 0x71, 0x09, 0x05, 0x03},
    /* 56 '8' */ {0x36, 0x49, 0x49, 0x49, 0x36},
    /* 57 '9' */ {0x06, 0x49, 0x49, 0x29, 0x1E},
    /* 58 ':' */ {0x00, 0x36, 0x36, 0x00, 0x00},
    /* 59 ';' */ {0x00, 0x56, 0x36, 0x00, 0x00},
    /* 60 '<' */ {0x08, 0x14, 0x22, 0x41, 0x00},
    /* 61 '=' */ {0x14, 0x14, 0x14, 0x14, 0x14},
    /* 62 '>' */ {0x00, 0x41, 0x22, 0x14, 0x08},
    /* 63 '?' */ {0x02, 0x01, 0x51, 0x09, 0x06},
    /* 64 '@' */ {0x3E, 0x41, 0x5D, 0x55, 0x1E},
    /* 65 'A' */ {0x7E, 0x11, 0x11, 0x11, 0x7E},
    /* 66 'B' */ {0x7F, 0x49, 0x49, 0x49, 0x36},
    /* 67 'C' */ {0x3E, 0x41, 0x41, 0x41, 0x22},
    /* 68 'D' */ {0x7F, 0x41, 0x41, 0x22, 0x1C},
    /* 69 'E' */ {0x7F, 0x49, 0x49, 0x49, 0x41},
    /* 70 'F' */ {0x7F, 0x09, 0x09, 0x09, 0x01},
    /* 71 'G' */ {0x3E, 0x41, 0x49, 0x49, 0x7A},
    /* 72 'H' */ {0x7F, 0x08, 0x08, 0x08, 0x7F},
    /* 73 'I' */ {0x00, 0x41, 0x7F, 0x41, 0x00},
    /* 74 'J' */ {0x20, 0x40, 0x41, 0x3F, 0x01},
    /* 75 'K' */ {0x7F, 0x08, 0x14, 0x22, 0x41},
    /* 76 'L' */ {0x7F, 0x40, 0x40, 0x40, 0x40},
    /* 77 'M' */ {0x7F, 0x02, 0x0C, 0x02, 0x7F},
    /* 78 'N' */ {0x7F, 0x04, 0x08, 0x10, 0x7F},
    /* 79 'O' */ {0x3E, 0x41, 0x41, 0x41, 0x3E},
    /* 80 'P' */ {0x7F, 0x09, 0x09, 0x09, 0x06},
    /* 81 'Q' */ {0x3E, 0x41, 0x51, 0x21, 0x5E},
    /* 82 'R' */ {0x7F, 0x09, 0x19, 0x29, 0x46},
    /* 83 'S' */ {0x46, 0x49, 0x49, 0x49, 0x31},
    /* 84 'T' */ {0x01, 0x01, 0x7F, 0x01, 0x01},
    /* 85 'U' */ {0x3F, 0x40, 0x40, 0x40, 0x3F},
    /* 86 'V' */ {0x1F, 0x20, 0x40, 0x20, 0x1F},
    /* 87 'W' */ {0x3F, 0x40, 0x38, 0x40, 0x3F},
    /* 88 'X' */ {0x63, 0x14, 0x08, 0x14, 0x63},
    /* 89 'Y' */ {0x07, 0x08, 0x70, 0x08, 0x07},
    /* 90 'Z' */ {0x61, 0x51, 0x49, 0x45, 0x43},
    /* 91 '[' */ {0x00, 0x7F, 0x41, 0x41, 0x00},
    /* 92 '\' */ {0x02, 0x04, 0x08, 0x10, 0x20},
    /* 93 ']' */ {0x00, 0x41, 0x41, 0x7F, 0x00},
    /* 94 '^' */ {0x04, 0x02, 0x01, 0x02, 0x04},
    /* 95 '_' */ {0x40, 0x40, 0x40, 0x40, 0x40},
    /* 96 '`' */ {0x00, 0x01, 0x02, 0x04, 0x00},
    /* 97 'a' */ {0x20, 0x54, 0x54, 0x54, 0x78},
    /* 98 'b' */ {0x7F, 0x48, 0x44, 0x44, 0x38},
    /* 99 'c' */ {0x38, 0x44, 0x44, 0x44, 0x20},
    /*100 'd' */ {0x38, 0x44, 0x44, 0x48, 0x7F},
    /*101 'e' */ {0x38, 0x54, 0x54, 0x54, 0x18},
    /*102 'f' */ {0x08, 0x7E, 0x09, 0x01, 0x02},
    /*103 'g' */ {0x0C, 0x52, 0x52, 0x52, 0x3E},
    /*104 'h' */ {0x7F, 0x08, 0x04, 0x04, 0x78},
    /*105 'i' */ {0x00, 0x44, 0x7D, 0x40, 0x00},
    /*106 'j' */ {0x20, 0x40, 0x44, 0x3D, 0x00},
    /*107 'k' */ {0x7F, 0x10, 0x28, 0x44, 0x00},
    /*108 'l' */ {0x00, 0x41, 0x7F, 0x40, 0x00},
    /*109 'm' */ {0x7C, 0x04, 0x18, 0x04, 0x78},
    /*110 'n' */ {0x7C, 0x08, 0x04, 0x04, 0x78},
    /*111 'o' */ {0x38, 0x44, 0x44, 0x44, 0x38},
    /*112 'p' */ {0x7C, 0x14, 0x14, 0x14, 0x08},
    /*113 'q' */ {0x08, 0x14, 0x14, 0x18, 0x7C},
    /*114 'r' */ {0x7C, 0x08, 0x04, 0x04, 0x08},
    /*115 's' */ {0x48, 0x54, 0x54, 0x54, 0x20},
    /*116 't' */ {0x04, 0x3F, 0x44, 0x40, 0x20},
    /*117 'u' */ {0x3C, 0x40, 0x40, 0x20, 0x7C},
    /*118 'v' */ {0x1C, 0x20, 0x40, 0x20, 0x1C},
    /*119 'w' */ {0x3C, 0x40, 0x30, 0x40, 0x3C},
    /*120 'x' */ {0x44, 0x28, 0x10, 0x28, 0x44},
    /*121 'y' */ {0x0C, 0x50, 0x50, 0x50, 0x3C},
    /*122 'z' */ {0x44, 0x64, 0x54, 0x4C, 0x44},
    /*123 '{' */ {0x00, 0x08, 0x36, 0x41, 0x00},
    /*124 '|' */ {0x00, 0x00, 0x7F, 0x00, 0x00},
    /*125 '}' */ {0x00, 0x41, 0x36, 0x08, 0x00},
    /*126 '~' */ {0x08, 0x04, 0x08, 0x10, 0x08},
};

/* ------------------------------------------------------------------ */
/* Font rendering helper                                               */
/* ------------------------------------------------------------------ */

/* Measure the pixel width of a string at the given scale. */
static float measure_text(const char *text, float scale)
{
    int len = (int)strlen(text);
    /* Each character is 5 pixels wide + 1 pixel spacing, last char has no trailing space */
    if (len == 0) return 0.0f;
    return (float)(len * 6 - 1) * scale;
}

/* Draw a single character using the pixel font. */
static void draw_char(UIContext *ctx, float x, float y, char ch, Vec4 color, float scale)
{
    if (ch < 32 || ch > 126) return;
    int idx = (int)ch - 32;
    const uint8_t *glyph = font_data[idx];

    for (int col = 0; col < 5; col++) {
        uint8_t column = glyph[col];
        for (int row = 0; row < 7; row++) {
            if (column & (1 << row)) {
                float px = x + (float)col * scale;
                float py = y + (float)row * scale;
                ui_draw_rect(ctx, px, py, scale, scale, color);
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Widget implementations                                              */
/* ------------------------------------------------------------------ */

void ui_panel(UIContext *ctx, float x, float y, float w, float h, Vec4 color)
{
    ui_draw_rect(ctx, x, y, w, h, color);
}

void ui_label(UIContext *ctx, float x, float y, const char *text, Vec4 color, float scale)
{
    if (!text) return;

    float cursor_x = x;
    while (*text) {
        draw_char(ctx, cursor_x, y, *text, color, scale);
        cursor_x += 6.0f * scale;  /* 5 pixel width + 1 pixel spacing */
        text++;
    }
}

bool ui_button(UIContext *ctx, uint32_t id, float x, float y, float w, float h,
               const char *label, Vec4 bg_color, Vec4 text_color)
{
    bool hovered = (ctx->mouse_x >= x && ctx->mouse_x < x + w &&
                    ctx->mouse_y >= y && ctx->mouse_y < y + h);

    if (hovered) {
        ctx->hot_id = id;
        if (ctx->mouse_down) {
            ctx->active_id = id;
        }
    }

    /* Determine button color based on state */
    Vec4 color = bg_color;
    if (hovered) {
        /* Brighten on hover */
        color.x += 0.1f;
        color.y += 0.1f;
        color.z += 0.1f;
    }
    if (ctx->active_id == id && hovered) {
        /* Darken when pressed */
        color.x -= 0.15f;
        color.y -= 0.15f;
        color.z -= 0.15f;
    }

    /* Draw button background */
    ui_draw_rect(ctx, x, y, w, h, color);

    /* Draw centered label */
    if (label) {
        float scale = 2.0f;
        float text_w = measure_text(label, scale);
        float text_h = 7.0f * scale;
        float text_x = x + (w - text_w) * 0.5f;
        float text_y = y + (h - text_h) * 0.5f;
        ui_label(ctx, text_x, text_y, label, text_color, scale);
    }

    /* Click: mouse just pressed this frame while hovering */
    return ctx->mouse_pressed && hovered;
}

void ui_progress_bar(UIContext *ctx, float x, float y, float w, float h,
                     float value, Vec4 bg_color, Vec4 fill_color)
{
    /* Clamp value to [0, 1] */
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;

    /* Background */
    ui_draw_rect(ctx, x, y, w, h, bg_color);

    /* Fill */
    if (value > 0.0f) {
        ui_draw_rect(ctx, x, y, w * value, h, fill_color);
    }
}

bool ui_icon_button(UIContext *ctx, uint32_t id, float x, float y, float size,
                    Vec4 fill_color, bool selected)
{
    bool hovered = (ctx->mouse_x >= x && ctx->mouse_x < x + size &&
                    ctx->mouse_y >= y && ctx->mouse_y < y + size);

    if (hovered) {
        ctx->hot_id = id;
        if (ctx->mouse_down) {
            ctx->active_id = id;
        }
    }

    /* Draw selection border if selected */
    if (selected) {
        float border = 2.0f;
        Vec4 border_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        ui_draw_rect(ctx, x - border, y - border,
                     size + border * 2.0f, size + border * 2.0f, border_color);
    }

    /* Adjust color for hover/press states */
    Vec4 color = fill_color;
    if (hovered) {
        color.x += 0.1f;
        color.y += 0.1f;
        color.z += 0.1f;
    }
    if (ctx->active_id == id && hovered) {
        color.x -= 0.15f;
        color.y -= 0.15f;
        color.z -= 0.15f;
    }

    /* Draw icon square */
    ui_draw_rect(ctx, x, y, size, size, color);

    /* Click: mouse just pressed this frame while hovering */
    return ctx->mouse_pressed && hovered;
}

void ui_tooltip(UIContext *ctx, float x, float y, const char *text,
                Vec4 bg_color, Vec4 text_color)
{
    if (!text) return;

    float scale = 2.0f;
    float padding = 6.0f;
    float text_w = measure_text(text, scale);
    float text_h = 7.0f * scale;
    float box_w = text_w + padding * 2.0f;
    float box_h = text_h + padding * 2.0f;

    /* Draw background panel */
    ui_draw_rect(ctx, x, y, box_w, box_h, bg_color);

    /* Draw text inside */
    ui_label(ctx, x + padding, y + padding, text, text_color, scale);
}
