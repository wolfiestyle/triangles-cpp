#include <iostream>
#include "framework/framework.hpp"
#include "shader_source.hpp"

// helper to draw a textured quad to screen
class TexDraw {
private:
    VertexArray m_vao;
    Program m_program;

public:
    TexDraw() {
        auto vert_sh = Shader(GL_VERTEX_SHADER, glsl::tex_vert);
        vert_sh.validate();
        auto frag_sh = Shader(GL_FRAGMENT_SHADER, glsl::tex_frag);
        frag_sh.validate();
        m_program.link_shaders({&vert_sh, &frag_sh});
        m_program.validate();

        struct { float x, y; } vertices[4] {
            { 0.0, 0.0, },
            { 1.0, 0.0, },
            { 0.0, 1.0, },
            { 1.0, 1.0, },
        };
        auto coords = Buffer();
        coords.load(vertices, sizeof(vertices), GL_STATIC_DRAW);

        auto coord_loc = m_program.get_attrib_loc("coord").value();
        m_vao.set_attribute(coord_loc, &coords, GL_FLOAT, 2, sizeof(float), 0, 2);
    }

    void draw(GLuint tex_unit) {
        m_program.set_active();
        m_program.get_uniform("tex").value().set(tex_unit);
        m_vao.draw(GL_TRIANGLE_STRIP, 0, 4);
    }
};

int main (int argc, char* argv[])
{
    using namespace std;

    auto window = GlWindow(640, 480, "triangles");
    glEnable(GL_FRAMEBUFFER_SRGB);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

    auto image = Image("image.jpg");
    auto tex = Texture2d(&image);

    auto texdraw = TexDraw();

    while (!window.should_close()) {
        glClear(GL_COLOR_BUFFER_BIT);

        tex.bind_to(0);
        texdraw.draw(0);

        window.swap_buffers();
        glfwPollEvents();
    }

    return 0;
}
