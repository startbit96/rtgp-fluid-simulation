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

    public:
        Particle *particles;
        unsigned int *particle_indices;
        unsigned int number_of_particles;
        unsigned int vertex_array_object;
        unsigned int vertex_buffer_object;
        unsigned int index_buffer_object;

        void resolve_collision (float x_min, float x_max, float y_min, float y_max, float z_min, float z_max);
};