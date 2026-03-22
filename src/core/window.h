#ifndef TD_WINDOW_H
#define TD_WINDOW_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef struct Window {
    GLFWwindow *handle;
    int width;          /* framebuffer width (for glViewport) */
    int height;         /* framebuffer height */
    int window_w;       /* logical window width (for UI/mouse) */
    int window_h;       /* logical window height */
    bool should_close;
} Window;

bool window_create(Window *w, int width, int height, const char *title);
void window_destroy(Window *w);
void window_poll_events(Window *w);
void window_swap_buffers(Window *w);

#endif /* TD_WINDOW_H */
