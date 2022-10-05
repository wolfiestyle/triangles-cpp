#include <iostream>
#include "framework/framework.hpp"
#include "shader_source.hpp"

static const struct {
    float x, y;
    float r, g, b;
} vertices[4] = {
    {0.0, 0.0, 1.0, 0.0, 0.0},
    {1.0, 0.0, 0.0, 1.0, 0.0},
    {0.0, 1.0, 0.0, 0.0, 1.0},
    {1.0, 1.0, 1.0, 1.0, 1.0},
};

int main (int argc, char* argv[])
{
    using namespace std;

    auto window = GlWindow(640, 480, "triangles");
    glEnable(GL_FRAMEBUFFER_SRGB);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

    auto buffer = Buffer();
    buffer.load(vertices, sizeof(vertices), GL_STATIC_DRAW);

    auto vert_sh = Shader(GL_VERTEX_SHADER, glsl::tex_vert);
    vert_sh.validate();
    auto frag_sh = Shader(GL_FRAGMENT_SHADER, glsl::tex_frag);
    frag_sh.validate();
    auto program = Program({&vert_sh, &frag_sh});
    program.validate();

    auto u_tex = program.get_uniform("tex").value();
    auto vpos_loc = program.get_attrib_loc("coord").value();
    //auto vcol_loc = program.get_attrib_loc("color").value();

    auto vao = VertexArray();
    vao.set_attribute(vpos_loc, buffer, GL_FLOAT, 2, sizeof(float), 0, 5);
    //vao.set_attribute(vcol_loc, buffer, GL_FLOAT, 3, sizeof(float), 2, 5);

    auto image = Image("image.jpg");
    auto tex = Texture2d(&image);

    while (!window.should_close()) {
        glClear(GL_COLOR_BUFFER_BIT);

        program.set_active();
        tex.bind_to(0);
        u_tex.set(0);
        vao.draw(GL_TRIANGLE_STRIP, 0, 4);

        window.swap_buffers();
        glfwPollEvents();
    }

    return 0;
}
