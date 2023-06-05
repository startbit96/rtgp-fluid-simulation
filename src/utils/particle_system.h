#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

#include "particle.h"
#include "cuboid.h"


// The number of initial particles depends on the fluids cuboids
// and the distance between the initial particles. This distance can
// be modified during runtime. Define here the min and max distance and
// also the factor. Make sure that the values for init, min and max are 
// within the power series of the inc factor.
#define PARTICLE_INITIAL_DISTANCE_INIT          0.064f
#define PARTICLE_INITIAL_DISTANCE_MIN           0.008f
#define PARTICLE_INITIAL_DISTANCE_MAX           0.128f
#define PARTICLE_INITIAL_DISTANCE_INC_FACTOR    2

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
        // The visualization handler will show the number of particles at the title
        // of the window. In order to make it more readable we save in this string
        // the number of particles with thousand separators, so that the visualization
        // handler can directly use this. The variable is only updated when the number of 
        // particles change.
        std::string number_of_particles_as_string;

        Particle_System ();

        // This function creates the initial particles based on the given spaces that have
        // to be filled an the particle_initial_distance.
        // It also generates the OpenGL needed buffers like the vertex array object.
        void generate_initial_particles (std::vector<Cuboid>& cuboids);
        // Note that these functions do not call the generate_initial_particles function, this
        // has to be done by the application. It simply increases / decreases the distance between
        // initial particles so that with the next call of generate_initial_particles it will take effect.
        // The bool value that the function gives back indicates if an increasement / decreasement is
        // possible and if the call to generate_initial_particles makes sense or not.
        bool increase_number_of_particles ();
        bool decrease_number_of_particles ();

        //void resolve_collision (float x_min, float x_max, float y_min, float y_max, float z_min, float z_max);

        // Draws the particles. Note that the shader will be selected and activated by the visualization handler.
        void draw (bool unbind = false);
        // Deletes the GPU ressources (vertex array, vertex buffer, index buffer).
        void free_gpu_resources ();
};