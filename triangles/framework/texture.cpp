#include "gl.hpp"
#include "types.hpp"
#include "image.hpp"
#include "texture.hpp"

Texture2d::Texture2d(GLsizei width, GLsizei height, GLenum format):
    m_id(0), m_width(width), m_height(height)
{
    glCreateTextures(GL_TEXTURE_2D, 1, &m_id);
    glTextureStorage2D(m_id, 1, format, width, height);
}

Texture2d::Texture2d(Image* image):
    Texture2d(image->m_width, image->m_height, GL_SRGB8_ALPHA8)
{
    load_data(0, 0, image->m_width, image->m_height, GL_RGBA, GL_UNSIGNED_BYTE, image->m_data);
}

Texture2d::~Texture2d() {
    glDeleteTextures(1, &m_id);
}

void Texture2d::load_data(int32_t x, int32_t y, uint32_t width, uint32_t height, GLenum format, GLenum data_ty, void const* data) {
    glTextureSubImage2D(m_id, 0, x, y, width, height, format, data_ty, data);
}

std::vector<ColorRGBA<float>> Texture2d::read_data_f() {
    auto n_elems = m_width * m_height * 4;
    auto size = n_elems * sizeof(float);
    auto buf = std::vector(n_elems, ColorRGBA<float>());
    glGetTextureImage(m_id, 0, GL_RGBA, GL_FLOAT, size, buf.data());
    return buf;
}

void Texture2d::bind_to(GLuint unit) {
    glBindTextureUnit(unit, m_id);
}

void Texture2d::bind_to_image(GLuint img_unit, GLenum format, GLenum access) {
    glBindImageTexture(img_unit, m_id, 0, GL_FALSE, 0, access, format);
}

Framebuffer::Framebuffer():
    m_id(0)
{
    glGenFramebuffers(1, &m_id);
}

Framebuffer::~Framebuffer() {
    glDeleteFramebuffers(1, &m_id);
}

void Framebuffer::attach_texture(GLenum attachment, Texture2d* texture) {
    bind();
    glNamedFramebufferTexture(m_id, attachment, texture->get_id(), 0);
    glNamedFramebufferDrawBuffers(m_id, 1, &attachment);
    unbind();
}

void Framebuffer::validate() {
    auto status = glCheckNamedFramebufferStatus(m_id, GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("invalid framebuffer");
    }
}

void Framebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_id);
}

void Framebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
