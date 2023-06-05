#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

#include "particle.h"

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
        GLuint vertex_buffer_object;
        GLuint index_buffer_object;
        GLuint vertex_array_object;

        Cuboid ();
        Cuboid (float x_min, float x_max, float y_min, float y_max, float z_min, float z_max);
        ~Cuboid ();
    
        // We need a function that can check if another cuboid is within the current one.
        // We need this to validate the registered scenes (make sure all fluid starting positions
        // are within the simulation space).
        bool contains (Cuboid &other);
        float get_volume ();
        // The implemented camera type is a arc ball camera. We need to specify a point of interest,
        // where the camera is always looking at. To react to different simulation spaces we set 
        // the point of interest depending on the simulation space. This may be the center of the cuboid or 
        // something else. For the implemented formula check the function.
        glm::vec3 get_point_of_interest ();
        // A function that fills a cuboid evenly with particles (for the start of the simulation).
        void fill_with_particles (float particle_distance, std::vector<Particle>& particles);
        // Draws the object. Note that the shader will be selected and activated by the visualization handler.
        void draw (bool unbind = false);
        // Deletes the GPU ressources (vertex array, vertex buffer, index buffer).
        void free_gpu_resources ();
};
