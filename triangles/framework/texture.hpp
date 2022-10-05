#ifndef __FRAMEWORK_TEXTURE_HPP
#define __FRAMEWORK_TEXTURE_HPP

class Texture2d {
private:
    GLuint m_id;
    GLsizei m_width;
    GLsizei m_height;

public:
    Texture2d(GLsizei width, GLsizei height, GLenum format);
    Texture2d(Image* image);
    ~Texture2d();
    Texture2d(Texture2d const&) = delete;

    GLsizei get_width() { return m_width; }
    GLsizei get_height() { return m_height; }

    void load_data(int32_t x, int32_t y, uint32_t width, uint32_t height, GLenum format, GLenum data_ty, void const* data);
    std::vector<ColorRGBA<float>> read_data_f();

    void bind_to(GLuint unit);
    void bind_to_image(GLuint img_unit, GLenum format, GLenum access);
};

#endif // __FRAMEWORK_TEXTURE_HPP
