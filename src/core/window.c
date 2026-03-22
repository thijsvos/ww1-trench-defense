#include "window.h"
#include "log.h"

static void framebuffer_size_callback(GLFWwindow *glfw_window, int width, int height) {
    Window *w = (Window *)glfwGetWindowUserPointer(glfw_window);
    if (w) {
        w->width = width;
        w->height = height;
        glfwGetWindowSize(glfw_window, &w->window_w, &w->window_h);
        glViewport(0, 0, width, height);
    }
}

bool window_create(Window *w, int width, int height, const char *title) {
    if (!glfwInit()) {
        LOG_ERROR("Failed to initialize GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_TRUE);
#endif

    w->handle = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!w->handle) {
        LOG_ERROR("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(w->handle);
    glfwSwapInterval(1); /* vsync — sync to monitor refresh rate */
    glfwSetWindowUserPointer(w->handle, w);
    glfwSetFramebufferSizeCallback(w->handle, framebuffer_size_callback);

    int version = gladLoadGL(glfwGetProcAddress);
    if (!version) {
        LOG_ERROR("Failed to load OpenGL with glad");
        glfwDestroyWindow(w->handle);
        glfwTerminate();
        return false;
    }

    LOG_INFO("OpenGL %d.%d loaded", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    /* Get actual framebuffer size (may differ from window size on Retina) */
    glfwGetFramebufferSize(w->handle, &w->width, &w->height);
    glfwGetWindowSize(w->handle, &w->window_w, &w->window_h);
    glViewport(0, 0, w->width, w->height);

    w->should_close = false;

    return true;
}

void window_destroy(Window *w) {
    if (w->handle) {
        glfwDestroyWindow(w->handle);
        w->handle = NULL;
    }
    glfwTerminate();
}

void window_poll_events(Window *w) {
    glfwPollEvents();
    w->should_close = glfwWindowShouldClose(w->handle);
}

void window_swap_buffers(Window *w) {
    glfwSwapBuffers(w->handle);
}
