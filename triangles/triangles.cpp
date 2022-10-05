#include <iostream>
#include "framework/framework.hpp"
#include "shader_source.hpp"

static const struct {
    float x, y;
    float r, g, b;
} vertices[3] = {
    {0.0, 0.0, 1.0, 0.0, 0.0},
    {1.0, 0.0, 0.0, 1.0, 0.0},
    {0.0, 1.0, 0.0, 0.0, 1.0},
};

int main (int argc, char* argv[])
{
    using namespace std;

    auto window = GlWindow(640, 480, "triangles");

    auto buffer = Buffer();
    buffer.load(vertices, sizeof(vertices), GL_STATIC_DRAW);

    auto vert_sh = Shader(GL_VERTEX_SHADER, glsl::color_vert);
    vert_sh.validate();
    auto frag_sh = Shader(GL_FRAGMENT_SHADER, glsl::color_frag);
    frag_sh.validate();
    auto program = Program({&vert_sh, &frag_sh});
    program.validate();

    auto un_test = program.get_uniform("test").value();
    auto vpos_loc = program.get_attrib_loc("position").value();
    auto vcol_loc = program.get_attrib_loc("color").value();

    auto vao = VertexArray();
    vao.set_attribute(vpos_loc, buffer, GL_FLOAT, 2, sizeof(float), 0, 5);
    vao.set_attribute(vcol_loc, buffer, GL_FLOAT, 3, sizeof(float), 2, 5);

    while (!window.should_close()) {
        glClear(GL_COLOR_BUFFER_BIT);

        program.set_active();
        un_test.set(0.5f);
        vao.draw(GL_TRIANGLES, 0, 3);

        window.swap_buffers();
        glfwPollEvents();
    }

    return 0;
}
