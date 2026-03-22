#ifndef TD_INPUT_H
#define TD_INPUT_H

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef struct Input {
    bool keys[GLFW_KEY_LAST + 1];
    bool keys_prev[GLFW_KEY_LAST + 1];
    double mouse_x, mouse_y;
    double scroll_y;
    bool mouse_buttons[GLFW_MOUSE_BUTTON_LAST + 1];
    bool mouse_buttons_prev[GLFW_MOUSE_BUTTON_LAST + 1];
} Input;

void input_init(Input *input, GLFWwindow *window);
void input_update(Input *input);
bool input_key_pressed(Input *input, int key);
bool input_key_down(Input *input, int key);
bool input_mouse_pressed(Input *input, int button);
bool input_mouse_down(Input *input, int button);

#endif /* TD_INPUT_H */
