#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "particle.h"
#include "cuboid.h"

// Particle System.
class Particle_System 
{
    private:
        GLuint vertex_array_object;
        GLuint vertex_buffer_object;
        GLuint index_buffer_object;
        std::vector<unsigned int> particle_indices;
        float particle_initial_distance;

    public:
        std::vector<Particle> particles;
        unsigned int number_of_particles;

        Particle_System ();

        // This function creates the initial particles based on the given spaces that have
        // to be filled an the particle_initial_distance.
        // It also generates the OpenGL needed buffers like the vertex array object.
        void generate_initial_particles (std::vector<Cuboid>& cuboids);
        
        //void resolve_collision (float x_min, float x_max, float y_min, float y_max, float z_min, float z_max);

        // Draws the particles. Note that the shader will be selected and activated by the visualization handler.
        void draw (bool unbind = false);
        // Deletes the GPU ressources (vertex array, vertex buffer, index buffer).
        void free_gpu_resources ();
};