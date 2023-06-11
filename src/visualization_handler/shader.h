#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>
#include <unordered_map>

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
        // A cache for the location of a uniform.
        std::unordered_map<std::string, int> uniform_location_cache;
        // This function returns the location of a uniform within the shader program.
        // If the location was requested, it will be stored in a cache.
        // If the location will be requested again, the cached value will be used.
        int get_uniform_location (const std::string& name);

    public:
        GLuint program_id;
        bool is_valid;

        Shader (const char* filepath_vertex_shader, 
                const char* filepath_fragment_shader, 
                const char* filepath_geometry_shader = NULL);
        
        void use_program ();
        void delete_program ();

        // These functions can be used to set the uniforms of the shader.
        void set_uniform_1i (const std::string& name, int value);
        void set_uniform_1f (const std::string& name, float value);
        void set_uniform_3f (const std::string& name, float f0, float f1, float f2);
        void set_uniform_3fv (const std::string& name, const glm::vec3& vector);
        void set_uniform_4f (const std::string& name, float f0, float f1, float f2, float f3);
        void set_uniform_4fv (const std::string& name, const glm::vec4& vector);
        void set_uniform_mat4fv (const std::string& name, const glm::mat4& matrix);
};