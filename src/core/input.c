#include "input.h"

#include <string.h>

static Input *s_input = NULL;

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    (void)window;
    (void)scancode;
    (void)mods;
    if (!s_input || key < 0 || key > GLFW_KEY_LAST) return;

    if (action == GLFW_PRESS) {
        s_input->keys[key] = true;
    } else if (action == GLFW_RELEASE) {
        s_input->keys[key] = false;
    }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    (void)window;
    (void)mods;
    if (!s_input || button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return;

    if (action == GLFW_PRESS) {
        s_input->mouse_buttons[button] = true;
    } else if (action == GLFW_RELEASE) {
        s_input->mouse_buttons[button] = false;
    }
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
    (void)window;
    if (!s_input) return;

    s_input->mouse_x = xpos;
    s_input->mouse_y = ypos;
}

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    (void)window;
    (void)xoffset;
    if (!s_input) return;

    s_input->scroll_y += yoffset;
}

void input_init(Input *input, GLFWwindow *window) {
    memset(input, 0, sizeof(Input));
    s_input = input;

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

void input_update(Input *input) {
    memcpy(input->keys_prev, input->keys, sizeof(input->keys));
    memcpy(input->mouse_buttons_prev, input->mouse_buttons, sizeof(input->mouse_buttons));
    input->scroll_y = 0.0;
}

bool input_key_pressed(Input *input, int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return input->keys[key] && !input->keys_prev[key];
}

bool input_key_down(Input *input, int key) {
    if (key < 0 || key > GLFW_KEY_LAST) return false;
    return input->keys[key];
}

bool input_mouse_pressed(Input *input, int button) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return input->mouse_buttons[button] && !input->mouse_buttons_prev[button];
}

bool input_mouse_down(Input *input, int button) {
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return false;
    return input->mouse_buttons[button];
}
