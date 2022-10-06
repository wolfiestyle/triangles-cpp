#include <iostream>
#include <stdexcept>
#include "gl.hpp"
#include "types.hpp"
#include "shader.hpp"

Shader::Shader(GLenum ty, char const* source) {
    m_id = glCreateShader(ty);
    glShaderSource(m_id, 1, &source, nullptr);
    glCompileShader(m_id);
}

Shader::~Shader() {
    glDeleteShader(m_id);
}

bool Shader::get_status() {
    auto status = 0;
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &status);
    return status == GL_TRUE;
}

std::string Shader::get_log() {
    auto log_len = 0;
    glGetShaderiv(m_id, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
        auto log_buff = std::string(log_len, '\0');
        glGetShaderInfoLog(m_id, log_len, nullptr, log_buff.data());
        return log_buff;
    } else {
        return std::string();
    }
}

void Shader::validate() {
    auto log = get_log();
    if (get_status()) {
        if (!log.empty()) {
            std::cerr << "-- " << log << std::endl;
        }
    } else {
        throw std::runtime_error(log);
    }
}

Program::Program() {
    m_id = glCreateProgram();
}

Program::~Program() {
    glDeleteProgram(m_id);
}

void Program::link_shaders(std::initializer_list<Shader*> shaders) {
    for (auto sh: shaders) {
        glAttachShader(m_id, sh->get_id());
    }
    glLinkProgram(m_id);
    for (auto sh: shaders) {
        glDetachShader(m_id, sh->get_id());
    }
}

bool Program::get_status() {
    auto status = 0;
    glGetProgramiv(m_id, GL_LINK_STATUS, &status);
    return status == GL_TRUE;
}

std::string Program::get_log() {
    auto log_len = 0;
    glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &log_len);
    if (log_len > 0) {
        auto log_buff = std::string(log_len, '\0');
        glGetProgramInfoLog(m_id, log_len, nullptr, log_buff.data());
        return log_buff;
    } else {
        return std::string();
    }
}

void Program::validate() {
    auto log = get_log();
    if (get_status()) {
        if (!log.empty()) {
            std::cerr << "-- " << log << std::endl;
        }
    } else {
        throw std::runtime_error(log);
    }
}

void Program::set_active() {
    glUseProgram(m_id);
}

std::optional<Uniform> Program::get_uniform(char const* name) {
    auto id = glGetUniformLocation(m_id, name);
    if (id < 0) {
        return std::nullopt;
    } else {
        return Uniform(id, m_id);
    }
}

std::optional<GLint> Program::get_attrib_loc(char const* name) {
    auto id = glGetAttribLocation(m_id, name);
    if (id < 0) {
        return std::nullopt;
    } else {
        return id;
    }
}

Vec3<GLint> Program::get_workgroup_size() {
    Vec3<GLint> wg;
    glGetProgramiv(m_id, GL_COMPUTE_WORK_GROUP_SIZE, &wg.x);
    return wg;
}

void Uniform::set(uint32_t value) {
    glProgramUniform1ui(m_prog, m_loc, value);
}

void Uniform::set(int32_t value) {
    glProgramUniform1i(m_prog, m_loc, value);
}

void Uniform::set(float value) {
    glProgramUniform1f(m_prog, m_loc, value);
}
