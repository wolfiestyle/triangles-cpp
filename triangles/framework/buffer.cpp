#include "gl.hpp"
#include "buffer.hpp"

Buffer::Buffer():
    m_id(0), m_size(0), m_usage(GL_STATIC_DRAW)
{
    glCreateBuffers(1, &m_id);
}

Buffer::~Buffer() {
    glDeleteBuffers(1, &m_id);
}

void Buffer::load(void const* data, size_t size, GLenum usage) {
    m_size = size;
    m_usage = usage;
    glNamedBufferData(m_id, size, data, usage);
}

void Buffer::bind(GLenum target) {
    glBindBuffer(target, m_id);
}

VertexArray::VertexArray():
    m_id(0)
{
    glCreateVertexArrays(1, &m_id);
}

VertexArray::~VertexArray() {
    glDeleteVertexArrays(1, &m_id);
}

void VertexArray::set_attribute(GLuint attr_loc, Buffer* vbo, GLenum ty, GLuint elem_count, GLuint elem_size, GLuint offset, GLuint stride) {
    glEnableVertexArrayAttrib(m_id, attr_loc);
    glVertexArrayVertexBuffer(m_id, attr_loc, vbo->get_id(), 0, stride * elem_size);
    glVertexArrayAttribFormat(m_id, attr_loc, elem_count, ty, GL_FALSE, offset * elem_size);
    glVertexArrayAttribBinding(m_id, attr_loc, attr_loc);
}

void VertexArray::draw(GLenum mode, GLint first, GLsizei count) {
    glBindVertexArray(m_id);
    glDrawArrays(mode, first, count);
}
