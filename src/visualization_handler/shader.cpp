#include "renderer.h"
#include "shader.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

Shader::Shader(const std::string& filepath)
    : m_filepath(filepath), m_renderer_id(0)
{
    Shader_Program_Source source = parse_shader(filepath);

    std::cout << "VERTEX" << std::endl << source.vertex_source << std::endl;
    std::cout << "FRAGMENT" << std::endl << source.fragment_source << std::endl;

    m_renderer_id = create_shader(source.vertex_source, source.fragment_source);

    GLCall(glUseProgram(m_renderer_id));
}

Shader::~Shader()
{
    GLCall(glDeleteProgram(m_renderer_id));
}

void Shader::bind() const
{
    GLCall(glUseProgram(m_renderer_id));
}

void Shader::unbind() const
{
    GLCall(glUseProgram(0));
}

int Shader::get_uniform_location(const std::string& name)
{
    if (m_uniform_location_cache.find(name) != m_uniform_location_cache.end())
        return m_uniform_location_cache[name];

    GLCall(int location = glGetUniformLocation(m_renderer_id, name.c_str()));
    if (location == -1)
        std::cout << "No active uniform variable with name " << name << " found." << std::endl;

    m_uniform_location_cache[name] = location;

    return location;
}

void Shader::set_uniform1i(const std::string& name, int value)
{
    GLCall(glUniform1i(get_uniform_location(name), value));
}

void Shader::set_uniform1f(const std::string& name, float value)
{
    GLCall(glUniform1f(get_uniform_location(name), value));
}

void Shader::set_uniform4f(const std::string& name, float f0, float f1, float f2, float f3)
{
    GLCall(glUniform4f(get_uniform_location(name), f0, f1, f2, f3));
}

enum Shader_Type
{
    NONE = -1, 
    VERTEX = 0, 
    FRAGMENT = 1
};

struct Shader_Program_Source Shader::parse_shader(const std::string& filepath)
{

    std::ifstream stream(filepath);
    std::string line;
    std::stringstream string_stream[2];
    Shader_Type type = NONE;

    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = FRAGMENT;
        }
        else
        {
            string_stream[(int)type] << line << '\n';
        }
    }

    struct Shader_Program_Source shader_program_source = { string_stream[0].str(), string_stream[1].str() };
    return shader_program_source;
}

unsigned int Shader::compile_shader(unsigned int type, const std::string& source)
{
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str();
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    // Error handling.
    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    std::cout << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader compile status: " << result << std::endl;
    if (result == GL_FALSE)
    {
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*) alloca(length * sizeof(char));
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        std::cout 
            << "Failed to compile "
            << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
            << "shader."
            << std::endl;
        std::cout << message << std::endl;
        GLCall(glDeleteShader(id));
        return 0;
    }

    return id;
}

unsigned int Shader::create_shader(const std::string& vertex_shader_code, const std::string& fragment_shader_code)
{
    unsigned int program = glCreateProgram();
    unsigned int vertex_shader_id = compile_shader(GL_VERTEX_SHADER, vertex_shader_code);
    unsigned int fragment_shader_id = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_code);

    GLCall(glAttachShader(program, vertex_shader_id));
    GLCall(glAttachShader(program, fragment_shader_id));

    GLCall(glLinkProgram(program));

    GLint program_linked;

    GLCall(glGetProgramiv(program, GL_LINK_STATUS, &program_linked));
    std::cout << "Program link status: " << program_linked << std::endl;
    if (program_linked != GL_TRUE)
    {
        GLsizei log_length = 0;
        GLchar message[1024];
        GLCall(glGetProgramInfoLog(program, 1024, &log_length, message));
        std::cout << "Failed to link program." << std::endl;
        std::cout << message << std::endl;
    }

    GLCall(glValidateProgram(program));

    GLCall(glDeleteShader(vertex_shader_id));
    GLCall(glDeleteShader(fragment_shader_id));

    return program;
}
