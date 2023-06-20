#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <mutex>

#include "particle_system.h"

#define MARCHING_CUBES_CUBE_EDGE_LENGTH         0.1f
#define MARCHING_CUBES_CUBE_EDGE_LENGTH_MIN     0.1f
#define MARCHING_CUBES_CUBE_EDGE_LENGTH_MAX     0.1f
#define MARCHING_CUBES_CUBE_EDGE_LENGTH_STEP    0.1f

struct Marching_Cube
{
    glm::vec3 corner_min;
    GLuint number_of_particles_within;
    GLuint vertex_values[8];
};

class Marching_Cubes_Generator
{
    private:
        // These are used for the marching cubes / surfaces itself.
        GLuint vertex_buffer_object;
        GLuint index_buffer_object;
        GLuint vertex_array_object;
        std::vector<Marching_Cube> marching_cubes;
        std::vector<GLuint> indices;

        // We need a simulation space reference to know how big our grid has to be.
        Particle_System* particle_system;

        // Parallel for loops.
        void parallel_for (void (Marching_Cubes_Generator::* function)(unsigned int, unsigned int), int number_of_elements);
        void parallel_for_grid (void (Marching_Cubes_Generator::* function)(unsigned int, unsigned int));

        // We store the particles in a spatial grid, just like in the particle system.
        int number_of_cells;
        int number_of_cells_x;
        int number_of_cells_y;
        int number_of_cells_z;
        std::vector<unsigned int> spatial_grid;
        std::vector<std::unique_ptr<std::mutex>> mutex_spatial_grid;
        // We save the last cube edge length to determine if we have to regenerate the mutex vector.
        float cube_edge_length;
        void calculate_number_of_grid_cells ();

    public:
        Marching_Cubes_Generator ();

        // This function is used to manually inform the marching cube generator that the
        // simulation space of the particle system changed. Using this we can skip that the 
        // generator has to check this everytime by himself.
        void simulation_space_changed ();

        // This function calculates the marching cubes.
        void generate_marching_cubes ();

        // A public variable that can be changed using imgui.
        float new_cube_edge_length;

        // Draws the marching cubes. Note that the shader will be selected and activated by the visualization handler.
        void draw (bool unbind = false);
        // Deletes the GPU ressources (vertex array, vertex buffer, index buffer) for both contained 
        // "objects": the marching cubes as well as the grid that can be visualized too.
        void free_gpu_resources ();
};