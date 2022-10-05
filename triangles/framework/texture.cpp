#include "gl.hpp"
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

void Texture2d::bind_to(GLuint unit) {
    glBindTextureUnit(unit, m_id);
}
