#include <iostream>
#include <cassert>
#include <random>
#include <chrono>
#include <CLI/CLI.hpp>
#include "framework/framework.hpp"
#include "shader_source.hpp"

// rng state
std::mt19937 g_rng;

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

        auto wg_size = m_program.get_workgroup_size();
        // we do the first iteration inline in the shader, so half each wg size
        auto wg_size_x = wg_size.x * 2;
        auto wg_size_y = wg_size.y * 2;
        assert(size_x % wg_size_x == 0 && size_y % wg_size_y == 0);

        auto wg_x = size_x / wg_size_x;
        auto wg_y = size_y / wg_size_y;
        auto tex_in = tex_src;
        std::shared_ptr<Texture2d> tex_out;

        m_program.set_active();
        do {
            // do an iteration
            tex_out = run_compute(wg_x, wg_y, tex_in);

            // check if it's worth to iterate again
            if (wg_x % wg_size_x == 0 && wg_y % wg_size_y == 0) {
                wg_x /= wg_size_x;
                wg_y /= wg_size_y;
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

class TexScaler {
private:
    Program m_program;

public:
    TexScaler() {
        auto comp_sh = Shader(GL_COMPUTE_SHADER, glsl::scale_comp);
        comp_sh.validate();
        m_program.link_shaders({&comp_sh});
        m_program.validate();
        m_program.get_uniform("src").value().set(0);    // tex unit 0
        m_program.get_uniform("dest").value().set(0);   // img unit 0
    }

    Texture2d* scale_texture(Texture2d* tex_src, uint32_t size_x, uint32_t size_y) {
        auto wg_size = m_program.get_workgroup_size();
        assert(size_x % wg_size.x == 0 && size_y % wg_size.y == 0);
        auto wg_x = size_x / wg_size.x;
        auto wg_y = size_y / wg_size.y;

        // can't bind the image as sRGB, so we use RGBA16 instead
        Texture2d* tex_dest = new Texture2d(size_x, size_y, GL_RGBA16);

        m_program.set_active();
        tex_src->bind_to(0);
        tex_dest->bind_to_image(0, GL_RGBA16, GL_WRITE_ONLY);
        glDispatchCompute(wg_x, wg_y, 1);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);

        return tex_dest;
    }
};

// calcs the difference squared between two textures
class TexDsq {
private:
    Program m_program;

public:
    TexDsq() {
        auto comp_sh = Shader(GL_COMPUTE_SHADER, glsl::dsq_comp);
        comp_sh.validate();
        m_program.link_shaders({&comp_sh});
        m_program.validate();
        m_program.get_uniform("src1").value().set(0);   // tex unit 0
        m_program.get_uniform("src2").value().set(1);   // tex unit 1
        m_program.get_uniform("dest").value().set(0);   // img unit 0
    }

    void run(Texture2d* src1, Texture2d* src2, Texture2d* dest) {
        auto size_x = dest->get_width();
        auto size_y = dest->get_height();
        auto wg_size = m_program.get_workgroup_size();
        assert(size_x % wg_size.x == 0 && size_y % wg_size.y == 0);
        auto wg_x = size_x / wg_size.x;
        auto wg_y = size_y / wg_size.y;

        m_program.set_active();
        src1->bind_to(0);
        src2->bind_to(1);
        dest->bind_to_image(0, GL_RGBA32F, GL_WRITE_ONLY);

        glDispatchCompute(wg_x, wg_y, 1);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
    }
};

// calcs the mean square error between two textures
class TexMse {
public:
    TexDsq m_dsq;
    TexFold m_fold;
    Texture2d m_tex_dsq;

    TexMse(uint32_t width, uint32_t height):
        m_tex_dsq(width, height, GL_RGBA32F) {}

    float run(Texture2d* src1, Texture2d* src2) {
        // calc differences between the two images
        m_dsq.run(src1, src2, &m_tex_dsq);
        // fold the differences into a single value
        auto mse = m_fold.run(&m_tex_dsq) / (m_tex_dsq.get_width() * m_tex_dsq.get_height());
        // we sum the color components to obtain a single value
        return mse.r + mse.g + mse.b + mse.a;
    }
};

// shared GL state
struct GlState {
    Program m_program;
    Texture2d* m_tex_img;
    Texture2d* m_fb_tex;
    Framebuffer m_fbo;
    TexMse m_texmse;

    GlState(Image* image, uint32_t tex_size):
        m_texmse(tex_size, tex_size)
    {
        // program for rendering triangles
        auto vert_sh = Shader(GL_VERTEX_SHADER, glsl::color_vert);
        vert_sh.validate();
        auto frag_sh = Shader(GL_FRAGMENT_SHADER, glsl::color_frag);
        frag_sh.validate();
        m_program.link_shaders({&vert_sh, &frag_sh});
        m_program.validate();

        // prepare the reference image
        auto tex = new Texture2d(image);
        glTextureParameteri(tex->get_id(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        if (image->m_width != tex_size || image->m_height != tex_size) {
            auto texscaler = TexScaler();
            m_tex_img = texscaler.scale_texture(tex, tex_size, tex_size);
            delete tex;
        } else {
            m_tex_img = tex;
        }

        // prepare the offscreen framebuffer
        m_fb_tex = new Texture2d(tex_size, tex_size, GL_SRGB8_ALPHA8);
        m_fbo.attach_texture(GL_COLOR_ATTACHMENT0, m_fb_tex);
        m_fbo.validate();

        // set the background color to the average color of the image
        auto avg_col = m_texmse.m_fold.run(m_tex_img) / (tex_size * tex_size);
        glClearColor(avg_col.r, avg_col.g, avg_col.b, avg_col.a);
        std::cout << "average color: " << avg_col << std::endl;
    }
};

#define ELEMS_PER_VERT 6  // 2 pos + 4 color

// buffer with random triangles, used to approximate an image
class TriangleBuf {
private:
    VertexArray m_vao;
    Buffer* m_vbo;
    uint32_t m_nverts;
    std::optional<float> m_mse;

public:
    TriangleBuf(Buffer* vbo, uint32_t n_verts):
        m_vbo(vbo), m_nverts(n_verts)
    {
        // each element is a single triangle:
        // attr 0 = vec2 position
        m_vao.set_attribute(0, m_vbo, GL_FLOAT, 2, sizeof(float), 0, ELEMS_PER_VERT);
        // attr 1 = vec4 color
        m_vao.set_attribute(1, m_vbo, GL_FLOAT, 4, sizeof(float), 2, ELEMS_PER_VERT);
    }

    static TriangleBuf random(uint32_t n_tris) {
        auto n_verts = n_tris * 3;
        auto n_elems = n_verts * ELEMS_PER_VERT;
        // generate a bunch of random floats
        std::uniform_real_distribution<float> dist(0.0, 1.0);
        std::vector<float> data;
        data.reserve(n_elems);
        for (auto i = 0; i < n_elems; ++i) {
            data.push_back(dist(g_rng));
        }
        // use the random data to form triangles
        auto vbo = new Buffer();
        vbo->load(data.data(), data.size(), GL_DYNAMIC_DRAW);
        return TriangleBuf(vbo, n_verts);
    }

    void draw(GlState* gl_state) {
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        gl_state->m_program.set_active();
        m_vao.draw(GL_TRIANGLES, 0, m_nverts);
        glDisable(GL_BLEND);
    }

    float calc_mse(GlState* gl_state) {
        if (m_mse.has_value()) {
            return m_mse.value();
        } else {
            gl_state->m_fbo.bind();
            draw(gl_state);
            auto mse = gl_state->m_texmse.run(gl_state->m_tex_img, gl_state->m_fb_tex);
            gl_state->m_fbo.unbind();
            m_mse = mse;
            return mse;
        }
    }

    // mutate a single number in the array
    std::pair<size_t, float> mutate() {
        // pick a random index into the buffer
        auto buff_len = m_vbo->byte_size() / sizeof(float);
        std::uniform_int_distribution<> id_dist(0, buff_len);
        auto elem_id = id_dist(g_rng);
        // save it's current value
        auto old_elem = m_vbo->get<float>(elem_id);
        // replace it with a random value
        std::uniform_real_distribution<float> f_dist(0.0, 1.0);
        m_vbo->set(elem_id, f_dist(g_rng));
        // drop the cached mse value
        m_mse.reset();
        // return the old value
        return std::make_pair(elem_id, old_elem);
    }

    void revert(std::pair<size_t, float> old_state) {
        m_vbo->set(old_state.first, old_state.second);
        m_mse.reset();
    }
};

int main (int argc, char* argv[])
{
    using namespace std;
    using namespace std::chrono_literals;

    string image_file;
    uint32_t tex_size = 256;
    uint32_t n_tris = 100;
    uint32_t draw_interval = 1000;

    // parse command line arguments
    CLI::App app{"Approximates an image with random triangles"};
    app.add_option("image", image_file, "Input image file")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-t,--tex-size", tex_size, "Texture size used in computations");
    app.add_option("-n,--num-tris", n_tris, "Number of triangles in approximation");
    app.add_option("-d,--draw-interval", draw_interval, "Display the result after N iterations");
    CLI11_PARSE(app, argc, argv);

    cout << "image: " << image_file << endl;
    cout << "texture size: " << tex_size << endl;
    cout << "num triangles: " << n_tris << endl;
    cout << "drawing every " << draw_interval << " iters" << endl;

    // initialized the random generator
    std::random_device rd;
    g_rng.seed(rd());

    // initialize opengl context
    auto window = GlWindow(tex_size, tex_size, "triangles");
    glEnable(GL_FRAMEBUFFER_SRGB);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

    // initialize opengl state
    auto image = Image(image_file.c_str());
    auto gl_state = GlState(&image, tex_size);

    // for displaying the results
    auto texdraw = TexDraw();

    // generate a bunch of random triangles
    auto triangles = TriangleBuf::random(n_tris);
    auto best_mse = triangles.calc_mse(&gl_state);
    cerr << "initial error: " << best_mse << endl;

    uint32_t iters = 0;
    auto start_time = std::chrono::steady_clock::now();
    auto frame_time = start_time;
    uint32_t frame_count = 0;
    uint32_t frame_total = 0;

    while (!window.should_close()) {
        // do a random change and see if it improves the result
        auto old_state = triangles.mutate();
        // calculate the mean square error between the reference image and the rendered triangles
        auto mse = triangles.calc_mse(&gl_state);
        if (mse < best_mse) {
            best_mse = mse; // if it reduces the error, keep it
        } else {
            triangles.revert(old_state); // if it increased the error, revert it
        }

        // get statistics
        frame_count++;
        frame_total++;
        auto current = std::chrono::steady_clock::now();
        auto elapsed_sec = current - frame_time;
        auto elapsed_total = current - start_time;
        if (elapsed_sec >= 1s) {
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed_total);
            cerr << "\r" << frame_total << " iters in " << seconds.count() <<
                "s ("<< frame_count << " iters/s) error: " << best_mse << "        " << flush;
            frame_count = 0;
            frame_time = current;
        }

        // check if we should display progress
        if (iters > draw_interval) {
            iters = 0;

            glClear(GL_COLOR_BUFFER_BIT);
            gl_state.m_fb_tex->bind_to(0);
            texdraw.draw(0);
            window.swap_buffers();
        } else {
            iters++;
        }

        glfwPollEvents();
    }

    return 0;
}
