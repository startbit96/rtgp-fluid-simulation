#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

// Particle.
struct Particle 
{
    glm::vec3 position;
};

// Particle System.
class Particle_System 
{
    private:
        GLuint vertex_array_object;
        GLuint vertex_buffer_object;
        GLuint index_buffer_object;
        unsigned int *particle_indices;

    public:
        Particle *particles;
        unsigned int number_of_particles;

        void resolve_collision (float x_min, float x_max, float y_min, float y_max, float z_min, float z_max);
        void draw ();
};