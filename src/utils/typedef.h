#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

// Particle.
struct Particle 
{
    glm::vec3 position;
};

// Cuboid class for the definition of the simulation space
// and the starting volume(s) of the fluid.
class Cuboid
{
    private:

    public:
        float x_min;
        float x_max;
        float y_min;
        float y_max;
        float z_min;
        float z_max;
        std::vector<glm::vec3> vertices;
        std::vector<unsigned int> indices;
        unsigned int vertex_buffer_object;
        unsigned int index_buffer_object;
        unsigned int vertex_array_object;

        Cuboid ();
        Cuboid (float x_min, float x_max, float y_min, float y_max, float z_min, float z_max);
        ~Cuboid ();
    
        bool contains (Cuboid &other);
        float get_volume ();
        void fill_with_particles (unsigned int number_of_particles, Particle* particles, unsigned int offset=0);
        void free_gpu_resources ();
};
