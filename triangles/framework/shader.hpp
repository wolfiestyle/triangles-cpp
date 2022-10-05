#ifndef __FRAMEWORK_SHADER_HPP
#define __FRAMEWORK_SHADER_HPP

#include <initializer_list>
#include <optional>

class Shader {
private:
    GLuint m_id;

public:
    Shader(GLenum ty, char const* source);
    ~Shader();
    Shader(Shader const&) = delete;

    GLuint get_id() { return m_id; }
    bool get_status();
    std::string get_log();
    void validate();
};

class Uniform {
private:
    GLint m_loc;
    GLuint m_prog;

    Uniform(GLint loc, GLuint prog): m_loc(loc), m_prog(prog) {}

    friend class Program;

public:
    void set(uint32_t value);
    void set(int32_t value);
    void set(float value);
};

class Program {
private:
    GLuint m_id;

public:
    Program(std::initializer_list<Shader*> shaders);
    ~Program();
    Program(Program const&) = delete;

    GLuint get_id() { return m_id; }
    bool get_status();
    std::string get_log();
    void validate();

    void set_active();
    std::optional<Uniform> get_uniform(char const* name);
    std::optional<GLint> get_attrib_loc(char const* name);
};

#endif // __FRAMEWORK_SHADER_HPP
