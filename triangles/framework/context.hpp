#ifndef __FRAMEWORK_CONTEXT_HPP
#define __FRAMEWORK_CONTEXT_HPP

class GlWindow {
public:
    GLFWwindow* m_window;

    GlWindow(int width, int height, char const* title);
    ~GlWindow();

    bool should_close();
    void swap_buffers();
};

#endif // __FRAMEWORK_CONTEXT_HPP
