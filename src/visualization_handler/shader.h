#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

enum Shader_Type
{
    VERTEX_SHADER,
    FRAGMENT_SHADER,
    SHADER_PROGRAM
};

inline const char* to_string (Shader_Type shader_type)
{
    switch (shader_type) {
        case VERTEX_SHADER:     return "vertex shader";
        case FRAGMENT_SHADER:   return "fragment shader";
        case SHADER_PROGRAM:    return "shader program";
        default:                return "unknown shader type";
    }
}

class Shader
{
    private:
        void check_compile_errors (unsigned int shader_id, Shader_Type shader_type);

    public:
        unsigned int program_id;

        Shader (const char* filepath_vertex_shader, const char* filepath_fragment_shader);
        
        void use_program ();
        void delete_program ();
};