#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <mutex>

#include "../utils/particle_system.h"

#define MARCHING_CUBES_CUBE_EDGE_LENGTH         0.1f
#define MARCHING_CUBES_CUBE_EDGE_LENGTH_MIN     0.005f
#define MARCHING_CUBES_CUBE_EDGE_LENGTH_MAX     0.2f
#define MARCHING_CUBES_CUBE_EDGE_LENGTH_STEP    0.0001f

struct Marching_Cube
{
    glm::vec3 corner_min;
    GLuint number_of_particles_within;
    GLuint vertex_values[8];
};

class Marching_Cubes_Generator
{
    private:
        // The OpenGL related variables.
        GLuint vertex_buffer_object;
        GLuint index_buffer_object;
        GLuint vertex_array_object;
        std::vector<GLuint> indices;

        // Parallel for loops.
        void parallel_for (void (Marching_Cubes_Generator::* function)(unsigned int, unsigned int), int number_of_elements);

        // The marching cubes are basically a spatial grid. When we iterate through all the particles within the particle
        // system, we then assign the particle based on the position in the spatial grid to the respective marching cube.
        int number_of_cells;
        int number_of_cells_x;
        int number_of_cells_y;
        int number_of_cells_z;
        std::vector<Marching_Cube> marching_cubes;
        // The algorithms can use multiple threads so we need to be safe using mutex.
        std::vector<std::unique_ptr<std::mutex>> mutex_spatial_grid;
        void calculate_number_of_grid_cells ();
        int discretize_value (float value);
        int get_grid_key (glm::vec3 position);
        glm::vec3 get_min_corner_from_grid_key (int grid_key);
        void set_cube_position (unsigned int index_start, unsigned int index_end);
        void reset_cube_informations (unsigned int index_start, unsigned int index_end);
        void count_particles (unsigned int index_start, unsigned int index_end);
        void calculate_vertex_values (unsigned int index_start, unsigned int index_end);

    public:
        Marching_Cubes_Generator ();

        // We need a simulation space reference to know how big our grid has to be.
        Particle_System* particle_system;
        // This function is used to manually inform the marching cube generator that the
        // simulation space of the particle system changed. Using this we can skip that the 
        // generator has to check this everytime by himself.
        void simulation_space_changed ();

        // This function calculates the marching cubes.
        void generate_marching_cubes ();

        // We save the last cube edge length to determine if we have to regenerate the mutex vector.
        float cube_edge_length;
        // A public variable that can be changed using imgui.
        float new_cube_edge_length;

        // Draws the marching cubes. Note that the shader will be selected and activated by the visualization handler.
        void draw (bool unbind = false);
        // Deletes the GPU ressources (vertex array, vertex buffer, index buffer) for both contained 
        // "objects": the marching cubes as well as the grid that can be visualized too.
        void free_gpu_resources ();
};