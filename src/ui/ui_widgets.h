#ifndef TD_UI_WIDGETS_H
#define TD_UI_WIDGETS_H

#include "ui.h"

/* Simple filled rectangle */
void ui_panel(UIContext *ctx, float x, float y, float w, float h, Vec4 color);

/* Text label (rendered with built-in 5x7 pixel font) */
void ui_label(UIContext *ctx, float x, float y, const char *text, Vec4 color, float scale);

/* Clickable button. Returns true when clicked. */
bool ui_button(UIContext *ctx, uint32_t id, float x, float y, float w, float h,
               const char *label, Vec4 bg_color, Vec4 text_color);

/* Progress bar (horizontal) */
void ui_progress_bar(UIContext *ctx, float x, float y, float w, float h,
                     float value, Vec4 bg_color, Vec4 fill_color);

/* Icon-style square button with a colored fill instead of text */
bool ui_icon_button(UIContext *ctx, uint32_t id, float x, float y, float size,
                    Vec4 fill_color, bool selected);

/* Simple tooltip panel that appears at position */
void ui_tooltip(UIContext *ctx, float x, float y, const char *text,
                Vec4 bg_color, Vec4 text_color);

#endif /* TD_UI_WIDGETS_H */
