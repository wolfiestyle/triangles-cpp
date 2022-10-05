#include <iostream>
#include <cassert>
#include <CLI/CLI.hpp>
#include "framework/framework.hpp"
#include "shader_source.hpp"

#define FOLD_WG_SIZE 8  // local_size_N from fold shader

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

        Point2d vertices[4] {
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

// calcs the sum if all the texels of a texture
class TexFold {
private:
    Program m_program;

public:
    TexFold() {
        auto comp_sh = Shader(GL_COMPUTE_SHADER, glsl::fold_comp);
        comp_sh.validate();
        m_program.link_shaders({&comp_sh});
        m_program.validate();
        m_program.get_uniform("src").value().set(0);
        m_program.get_uniform("dest").value().set(0);
    }

    ColorRGBA<float> run(Texture2d* tex_src) {
        auto size_x = tex_src->get_width();
        auto size_y = tex_src->get_height();

        auto wg_size2 = FOLD_WG_SIZE * 2;
        assert(size_x % wg_size2 == 0 && size_y % wg_size2 == 0);

        auto wg_x = size_x / wg_size2;
        auto wg_y = size_y / wg_size2;
        auto tex_in = tex_src;
        std::shared_ptr<Texture2d> tex_out;

        m_program.set_active();
        do {
            // do an iteration
            tex_out = run_compute(wg_x, wg_y, tex_in);

            // check if it's worth to iterate again
            if (wg_x % wg_size2 == 0 && wg_y % wg_size2 == 0) {
                wg_x /= wg_size2;
                wg_y /= wg_size2;
                tex_in = tex_out.get();
            } else {
                break;
            }
        } while(true);

        auto result_data = tex_out->read_data_f();

        // fold the (hopefully) tiny result texture into the final value
        return std::accumulate(result_data.begin(), result_data.end(), ColorRGBA<float>());
    }

private:
    std::shared_ptr<Texture2d> run_compute(uint32_t wg_x, uint32_t wg_y, Texture2d* tex_in) {
        auto tex_out = std::shared_ptr<Texture2d>(new Texture2d(wg_x, wg_y, GL_RGBA32F));
        tex_in->bind_to(0); // tex unit 0 = src
        tex_out->bind_to_image(0, GL_RGBA32F, GL_WRITE_ONLY); // img unit 0 = dest
        glDispatchCompute(wg_x, wg_y, 1);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
        return tex_out;
    }
};

int main (int argc, char* argv[])
{
    using namespace std;

    string image_file;
    CLI::App app{"Approximates an image with random triangles"};
    app.add_option("image", image_file, "Input image file")
        ->required()
        ->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);

    cout << "image: " << image_file << endl;

    auto window = GlWindow(640, 480, "triangles");
    glEnable(GL_FRAMEBUFFER_SRGB);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

    auto image = Image(image_file.c_str());
    auto tex = Texture2d(&image);

    auto texdraw = TexDraw();

    auto texfold = TexFold();
    auto avg_col = texfold.run(&tex) / (tex.get_width() * tex.get_height());
    cout << "average color: " << avg_col << endl;

    while (!window.should_close()) {
        glClear(GL_COLOR_BUFFER_BIT);

        tex.bind_to(0);
        texdraw.draw(0);

        window.swap_buffers();
        glfwPollEvents();
    }

    return 0;
}
