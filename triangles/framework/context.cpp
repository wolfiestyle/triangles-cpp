#include <stdexcept>
#include <iostream>
#include "gl.hpp"
#include "context.hpp"

static uint32_t g_initCount = 0;

void error_callback(int error, char const* desc) {
    std::cerr << "Error " << error << ": " << desc << std::endl;
}

void global_init() {
    if (glfwInit()) {
        g_initCount++;
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    } else {
        throw std::runtime_error("error initializing glfw");
    }
    if (g_initCount == 1) {
        glfwSetErrorCallback(error_callback);
    }
}

void global_cleanup() {
    if (g_initCount > 0) {
        if (--g_initCount == 0) {
            glfwTerminate();
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    //FIXME: using this callback for non-trivial things requires global state
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void resize_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void debug_callback(GLenum source, GLenum ty, GLuint id, GLenum severity, GLsizei length, GLchar const* message, GLvoid const* user_param) {
    std::cerr << "debug: " << message << std::endl;
}

GlWindow::GlWindow(int width, int height, char const* title):
    m_window(nullptr)
{
    global_init();

    auto window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window) {
        std::runtime_error("error creating window");
    }
    m_window = window;

    glfwMakeContextCurrent(window);
    if (g_initCount == 1) {
        glbinding::initialize(glfwGetProcAddress);
        glDebugMessageCallback(debug_callback, nullptr);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glfwSwapInterval(1);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, resize_callback);
}

GlWindow::~GlWindow() {
    glfwDestroyWindow(m_window);
    global_cleanup();
}

bool GlWindow::should_close() {
    return glfwWindowShouldClose(m_window);
}

void GlWindow::swap_buffers() {
    glfwSwapBuffers(m_window);
}
