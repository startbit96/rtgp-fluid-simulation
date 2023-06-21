#pragma once

//        4 +--------+ 5
//         /|       /|
//        / |      / |
//     7 +--------+ 6|
//       |  |     |  |
//       |0 +-----|--+ 1
//       | /      | /
//       |/       |/
//     3 +--------+ 2

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <mutex>

#include "../utils/particle_system.h"

// The edge length of a single marching cube. The less the edge length, the higher the resolution.
#define MARCHING_CUBES_CUBE_EDGE_LENGTH         0.1f
#define MARCHING_CUBES_CUBE_EDGE_LENGTH_MIN     0.005f
#define MARCHING_CUBES_CUBE_EDGE_LENGTH_MAX     0.2f
#define MARCHING_CUBES_CUBE_EDGE_LENGTH_STEP    0.0001f
// The isovalue used in the marching cubes algorithm to determine if a vertex is within the object
// or outside. Since we simple count the number of particles in the marching cube and apply this value
// to the neighboring cubes vertices, we use int values here.
// Note that the step is not really the step the value can take but more the sensitivity for the imgui window.
#define MARCHING_CUBES_ISOVALUE                 1
#define MARCHING_CUBES_ISOVALUE_MIN             1
#define MARCHING_CUBES_ISOVALUE_MAX             20
#define MARCHING_CUBES_ISOVALUE_STEP            0.05

struct Marching_Cube
{
    glm::vec3 corner_min;
    GLint number_of_particles_within;
    // We cannot pass a vertex_values[8] to the shader since max. 4 values are supported.
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml
    // So we make a workaround.
    GLint value_vertex_0;
    GLint value_vertex_1;
    GLint value_vertex_2;
    GLint value_vertex_3;
    GLint value_vertex_4;
    GLint value_vertex_5;
    GLint value_vertex_6;
    GLint value_vertex_7;
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

        // We may call the draw function of the marching cubes twice per frame.
        // (once for the grid and once for the generated surface)
        // To not pass the new data twice to the buffer data, we check if the data changed.
        bool dataChanged;

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
        // A value that will be passed as an uniform to the geometry shader for the marching cubes algorithm.
        int isovalue;

        // Draws the marching cubes. Note that the shader will be selected and activated by the visualization handler.
        void draw (bool unbind = false);
        // Deletes the GPU ressources (vertex array, vertex buffer, index buffer) for both contained 
        // "objects": the marching cubes as well as the grid that can be visualized too.
        void free_gpu_resources ();
};