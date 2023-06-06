#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

#include "particle.h"
#include "cuboid.h"


// The number of initial particles depends on the fluids cuboids
// and the distance between the initial particles. This distance can
// be modified during runtime. Define here the min and max distance and
// also the factor. Make sure that the values for init, min and max are 
// within the power series of the inc factor.
// Also note that if the factor is 2 that does not mean the number of 
// particles doubles but since we are in 3D it is 2^3 = 8 times the 
// number of particles.
#define PARTICLE_INITIAL_DISTANCE_INIT          0.128f
#define PARTICLE_INITIAL_DISTANCE_MIN           0.008f
#define PARTICLE_INITIAL_DISTANCE_MAX           0.256f
#define PARTICLE_INITIAL_DISTANCE_INC_FACTOR    2
// Our hash function requires three big prime numbers. Define them here.
#define HASH_FUNCTION_PRIME_NUMBER_1            73856093
#define HASH_FUNCTION_PRIME_NUMBER_2            19349663
#define HASH_FUNCTION_PRIME_NUMBER_3            83492791
// SPH related defines.
#define SPH_KERNEL_RADIUS                       0.25f
#define SPH_PARTICLE_MASS                       0.02f
#define SPH_REST_DENSITY                        998.29f
#define SPH_GAS_CONSTANT                        0.05f
#define SPH_VISCOSITY                           0.00089f
#define SPH_SURFACE_TENSION                     0.0728f
#define SPH_SURFACE_THRESHOLD                   7.065f
// Collision handling.
#define SPH_COLLISION_DAMPING                   0.25f
// Simulation time defines.
#define SPH_SIMULATION_TIME_STEP                0.05f

// Particle System.
class Particle_System 
{
    private:
        GLuint vertex_array_object;
        GLuint vertex_buffer_object;
        GLuint index_buffer_object;
        std::vector<unsigned int> particle_indices;
        float particle_initial_distance;
        Cuboid* simulation_space;

        // Within SPH, the new values of a particle are calculated using the values
        // of the particles nearby. For the speed of this application it is necessary to 
        // have a good datastructure. For each particle we will create a list of neighbouring 
        // particles that are within the reach of the used kernel. To create these lists we will
        // use a spatial hash grid.
        glm::vec3 hash_offset;
        int number_of_cells;
        float sph_kernel_radius;
        std::unordered_multimap<int, Particle> spatial_hash_grid;
        std::vector<std::vector<Particle>> neighbor_list;
        int discretize_value (float value);
        int hash (glm::vec3 position);

        // For the SPH we use different kernels, their gradient and laplacian.
        // Note that normally the function returns 0 if the distance between the particles
        // is greater than the kernels radius. Therefore we normally would need an if statement
        // in every kernel function. But since we already do this within the find_neighbors function
        // we do not need this here and can save some execution time.
        // ATTENTION: if changes are made to the find_neighbors function, then we may need 
        // the if statement here!
        // Make sure to use some static variables for the coefficients and also to not square a 
        // vector length since this can be faster calculated using the dot product.
        float kernel_w_poly6 (glm::vec3 distance_vector);
        glm::vec3 kernel_w_poly6_gradient (glm::vec3 distance_vector);
        float kernel_w_poly6_laplacian (glm::vec3 distance_vector);
        glm::vec3 kernel_w_spiky_gradient (glm::vec3 distance_vector);
        float kernel_w_viscosity_laplacian (glm::vec3 distance_vector);

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
        void set_simulation_space (Cuboid* simulation_space);
        // Note that these functions do not call the generate_initial_particles function, this
        // has to be done by the application. It simply increases / decreases the distance between
        // initial particles so that with the next call of generate_initial_particles it will take effect.
        // The bool value that the function gives back indicates if an increasement / decreasement is
        // possible and if the call to generate_initial_particles makes sense or not.
        bool increase_number_of_particles ();
        bool decrease_number_of_particles ();

        // Simulation related functions.
        void simulate ();

        // Draws the particles. Note that the shader will be selected and activated by the visualization handler.
        void draw (bool unbind = false);
        // Deletes the GPU ressources (vertex array, vertex buffer, index buffer).
        void free_gpu_resources ();
};