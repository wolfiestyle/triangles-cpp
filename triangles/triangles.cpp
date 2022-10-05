#include <iostream>
#include "framework/framework.hpp"

int main (int argc, char* argv[])
{
    using namespace std;
    using namespace gl;

    auto window = GlWindow(640, 480, "triangles");

    glClearColor(0.1, 0.2, 0.3, 1.0);

    while (!window.should_close()) {
        glClear(GL_COLOR_BUFFER_BIT);
        window.swap_buffers();
        glfwPollEvents();
    }

    return 0;
}
