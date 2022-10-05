#ifndef __FRAMEWORK_BUFFER_HPP
#define __FRAMEWORK_BUFFER_HPP

class Buffer {
private:
    GLuint m_id;
    size_t m_size;
    GLenum m_usage;

public:
    Buffer();
    ~Buffer();
    Buffer(Buffer const&) = delete;

    GLuint get_id() { return m_id; }
    size_t byte_size() { return m_size; }

    void load(void const* data, size_t size, GLenum usage);
    void bind(GLenum target);
};

class VertexArray {
private:
    GLuint m_id;

public:
    VertexArray();
    ~VertexArray();
    VertexArray(VertexArray const&) = delete;

    GLuint get_id() { return m_id; }

    void set_attribute(GLuint attr_loc, Buffer& vbo, GLenum ty, GLuint elem_count, GLuint elem_size, GLuint offset, GLuint stride);
    void draw(GLenum mode, GLint first, GLsizei count);
};

#endif // __FRAMEWORK_BUFFER_HPP
