#pragma once

#include <string>
#include <unordered_map>

struct Shader_Program_Source
{
    std::string vertex_source;
    std::string fragment_source;
};

class Shader
{
    private:
        unsigned int m_renderer_id;
        std::string m_filepath;
        std::unordered_map<std::string, int> m_uniform_location_cache;

    public:
        Shader(const std::string& filepath);
        ~Shader();

        void bind() const;
        void unbind() const;

        // Set uniforms.
        void set_uniform1i(const std::string& name, int value);
        void set_uniform1f(const std::string& name, float value);
        void set_uniform4f(const std::string& name, float f0, float f1, float f2, float f3);

    private:
        int get_uniform_location(const std::string& name);
        struct Shader_Program_Source parse_shader(const std::string& filepath);
        unsigned int compile_shader(unsigned int type, const std::string& source);
        unsigned int create_shader(const std::string& vertex_shader_code, const std::string& fragment_shader_code);
};
