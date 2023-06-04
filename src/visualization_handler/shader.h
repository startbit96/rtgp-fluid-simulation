#pragma once

#include <string>

enum Shader_Type
{
    VERTEX_SHADER,
    GEOMETRY_SHADER,
    FRAGMENT_SHADER,
    SHADER_PROGRAM
};

inline const char* to_string (Shader_Type shader_type)
{
    switch (shader_type) {
        case VERTEX_SHADER:     return "vertex shader";
        case GEOMETRY_SHADER:   return "geometry shader";
        case FRAGMENT_SHADER:   return "fragment shader";
        case SHADER_PROGRAM:    return "shader program";
        default:                return "unknown shader type";
    }
}

class Shader
{
    private:
        static bool check_compile_errors (unsigned int shader_id, Shader_Type shader_type);
        static std::string get_code_from_file (const char* filepath);

    public:
        unsigned int program_id;
        bool is_valid;

        Shader (const char* filepath_vertex_shader, 
                const char* filepath_fragment_shader, 
                const char* filepath_geometry_shader = NULL);
        
        void use_program ();
        void delete_program ();

        
};