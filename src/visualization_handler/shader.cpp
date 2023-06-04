#include "shader.h"

#include <filesystem>

#include "../utils/debug.h"

Shader::Shader (const char* filepath_vertex_shader, const char* filepath_fragment_shader)
{
    std::string str_vertex_shader_code;
    std::string str_fragment_shader_code;
    std::ifstream vertex_shader_file;
    std::ifstream fragment_shader_file;

    // Ensure ifstream objects can throw exceptions.
    vertex_shader_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fragment_shader_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);

    // Read in the files.
    try {
        vertex_shader_file.open(filepath_vertex_shader);
        fragment_shader_file.open(filepath_fragment_shader);
        // Read file's buffer contents into stringstreams.
        std::stringstream vertex_shader_stream, fragment_shader_stream;
        vertex_shader_stream << vertex_shader_file.rdbuf();
        fragment_shader_stream << fragment_shader_file.rdbuf();
        // Close the file handlers.
        vertex_shader_file.close();
        fragment_shader_file.close();
        // Convert the stringstream into string and then to char pointer.
        str_vertex_shader_code = vertex_shader_stream.str();
        str_fragment_shader_code = fragment_shader_stream.str();
    }
    catch (std::ifstream::failure exception) {
        std::cout << "ERROR. The shader file could not be read successfully." << std::endl;
        std::cout << "The provided filepaths are:" << std::endl;
        std::cout << "filepath vertex shader: '" << filepath_vertex_shader << "'." << std::endl;
        std::cout << "filepath fragment shader: '" << filepath_fragment_shader << "'." << std::endl;
        std::cout << "The current working directory is:" << std::endl;
        std::cout << std::filesystem::current_path() << std::endl;
        return;
    }

    // Transform the strings to const char*.
    const char* vertex_shader_code = str_vertex_shader_code.c_str();
    const char* fragment_shader_code = str_fragment_shader_code.c_str();

    // Compile the shaders.
    unsigned int vertex_shader_id, fragment_shader_id;

    // Vertex shader.
    GLCall( vertex_shader_id = glCreateShader(GL_VERTEX_SHADER) );
    GLCall( glShaderSource(vertex_shader_id, 1, &vertex_shader_code, NULL) );
    GLCall( glCompileShader(vertex_shader_id) );
    // check compilation errors
    this->check_compile_errors(vertex_shader_id, VERTEX_SHADER);

    // Fragment shader.
    GLCall( fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER) );
    GLCall( glShaderSource(fragment_shader_id, 1, &fragment_shader_code, NULL) );
    GLCall( glCompileShader(fragment_shader_id) );
    // check compilation errors
    this->check_compile_errors(fragment_shader_id, FRAGMENT_SHADER);

    // Create the shader program.
    GLCall( this->program_id = glCreateProgram() );
    GLCall( glAttachShader(this->program_id, vertex_shader_id) );
    GLCall( glAttachShader(this->program_id, fragment_shader_id) );
    GLCall( glLinkProgram(this->program_id) );
    // Check for linking errors.
    this->check_compile_errors(this->program_id, SHADER_PROGRAM);

    // Delete the shaders, they are linked to the program.
    GLCall( glDeleteShader(vertex_shader_id) );
    GLCall( glDeleteShader(fragment_shader_id) );
}

void Shader::use_program ()
{
    GLCall( glUseProgram(this->program_id) );
}

void Shader::delete_program ()
{
    GLCall (glDeleteProgram(this->program_id) );
}

void Shader::check_compile_errors (unsigned int shader_id, Shader_Type shader_type)
{
    int success;
    char info_log[1024];
    if (shader_type != SHADER_PROGRAM) {
        GLCall( glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success) );
        if(success == false) {
            GLCall( glGetShaderInfoLog(shader_id, 1024, NULL, info_log) );
            std::cout << "ERROR compiling the " << to_string(shader_type) << ":" << std::endl;
            std::cout << info_log << std::endl;
        }
    }
    else {
        GLCall( glGetProgramiv(shader_id, GL_LINK_STATUS, &success) );
        if(success == false) {
            GLCall( glGetProgramInfoLog(shader_id, 1024, NULL, info_log) );
            std::cout << "ERROR linking the shader program:" << std::endl;
            std::cout << info_log << std::endl;
        }
    }
}