#include "shader.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>

#include "../utils/debug.h"

Shader::Shader (const char* filepath_vertex_shader,
                const char* filepath_fragment_shader, 
                const char* filepath_geometry_shader)
{
    // Make sure to recognize if an error occured.
    // Even if an error occured at the beginning, we will still run the constructor
    // until the end. At the end everything will be deleted and the program will get
    // the signal to abort.
    this->is_valid = true;

    // Get the code of the vertex shader.
    std::string str_vertex_shader_code = Shader::get_code_from_file(filepath_vertex_shader);
    const GLchar* vertex_shader_code = str_vertex_shader_code.c_str();
    // Compile the vertex shader.
    GLCall( GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER) );
    GLCall( glShaderSource(vertex_shader_id, 1, &vertex_shader_code, NULL) );
    GLCall( glCompileShader(vertex_shader_id) );
    // Check for compile errors.
    this->is_valid = this->is_valid && Shader::check_compile_errors(vertex_shader_id, VERTEX_SHADER);

    // Get the code of the fragment shader.
    std::string str_fragment_shader_code = Shader::get_code_from_file(filepath_fragment_shader);
    const GLchar* fragment_shader_code = str_fragment_shader_code.c_str();
    // Compile the fragment shader.
    GLCall( GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER) );
    GLCall( glShaderSource(fragment_shader_id, 1, &fragment_shader_code, NULL) );
    GLCall( glCompileShader(fragment_shader_id) );
    // Check for compile errors.
    this->is_valid = this->is_valid && Shader::check_compile_errors(fragment_shader_id, FRAGMENT_SHADER);

    // If also a geometry shader given, load this too.
    GLuint geometry_shader_id;
    if (filepath_geometry_shader != NULL) {
        // Get the code of the vertex shader.
        std::string str_geometry_shader_code = Shader::get_code_from_file(filepath_geometry_shader);
        const GLchar* geometry_shader_code = str_geometry_shader_code.c_str();
        // Compile the vertex shader.
        GLCall( geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER) );
        GLCall( glShaderSource(geometry_shader_id, 1, &geometry_shader_code, NULL) );
        GLCall( glCompileShader(geometry_shader_id) );
        // Check for compile errors.
        this->is_valid = this->is_valid && Shader::check_compile_errors(geometry_shader_id, GEOMETRY_SHADER);
    }

    // Create the shader program.
    GLCall( this->program_id = glCreateProgram() );
    GLCall( glAttachShader(this->program_id, vertex_shader_id) );
    GLCall( glAttachShader(this->program_id, fragment_shader_id) );
    if (filepath_geometry_shader != NULL) {
        GLCall( glAttachShader(this->program_id, geometry_shader_id) );
    }
    GLCall( glLinkProgram(this->program_id) );
    // Check for linking errors.
    this->is_valid = this->is_valid && Shader::check_compile_errors(this->program_id, SHADER_PROGRAM);

    // Delete the shaders, they are linked to the program.
    GLCall( glDeleteShader(vertex_shader_id) );
    GLCall( glDeleteShader(fragment_shader_id) );
    if (filepath_geometry_shader != NULL) {
        GLCall( glDeleteShader(geometry_shader_id) );
    }
    // If an error occured, delete also the program.
    if (this->is_valid == false) {
        this->delete_program();
    }
}

void Shader::use_program ()
{
    GLCall( glUseProgram(this->program_id) );
}

void Shader::delete_program ()
{
    GLCall (glDeleteProgram(this->program_id) );
}

bool Shader::check_compile_errors (unsigned int shader_id, Shader_Type shader_type)
{
    GLint success;
    char info_log[1024];
    if (shader_type != SHADER_PROGRAM) {
        GLCall( glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success) );
        if(success == 0) {
            GLCall( glGetShaderInfoLog(shader_id, 1024, NULL, info_log) );
            std::cout << "ERROR compiling the " << to_string(shader_type) << ":" << std::endl;
            std::cout << info_log << std::endl;
        }
    }
    else {
        GLCall( glGetProgramiv(shader_id, GL_LINK_STATUS, &success) );
        if(success == 0) {
            GLCall( glGetProgramInfoLog(shader_id, 1024, NULL, info_log) );
            std::cout << "ERROR linking the shader program:" << std::endl;
            std::cout << info_log << std::endl;
        }
    }
    return success == 0 ? false : true;
}

std::string Shader::get_code_from_file (const char* filepath)
{
    std::string str_shader_code;
    std::ifstream shader_file;

    // Ensure ifstream object can throw exceptions.
    shader_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);

    // Read in the file.
    try {
        shader_file.open(filepath);
        // Read file's buffer contents into stringstreams.
        std::stringstream shader_stream;
        shader_stream << shader_file.rdbuf();
        // Close the file handlers.
        shader_file.close();
        // Convert the stringstream into string.
        str_shader_code = shader_stream.str();
    }
    catch (std::ifstream::failure exception) {
        std::cout << "ERROR. The shader file could not be read successfully." << std::endl;
        std::cout << "The provided filepath is:" << std::endl;
        std::cout << filepath << std::endl;
        std::cout << "The current working directory is:" << std::endl;
        std::cout << std::filesystem::current_path() << std::endl;
        str_shader_code = "";
    }
    return str_shader_code;
}