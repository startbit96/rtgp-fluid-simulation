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
#include <memory>

#include "../utils/particle_system.h"

// The edge length of a single marching cube. The less the edge length, the higher the resolution.
#define MARCHING_CUBES_CUBE_EDGE_LENGTH         0.1f
#define MARCHING_CUBES_CUBE_EDGE_LENGTH_MIN     0.01f
#define MARCHING_CUBES_CUBE_EDGE_LENGTH_MAX     0.3f
#define MARCHING_CUBES_CUBE_EDGE_LENGTH_STEP    0.0001f
// The isovalue used in the marching cubes algorithm to determine if a vertex is within the object
// or outside. Since we simple count the number of particles in the marching cube and apply this value
// to the neighboring cubes vertices, we use int values here.
// Note that the step is not really the step the value can take but more the sensitivity for the imgui window.
#define MARCHING_CUBES_ISOVALUE                 0.5f
#define MARCHING_CUBES_ISOVALUE_MIN             0.1f
#define MARCHING_CUBES_ISOVALUE_MAX             20.0f
#define MARCHING_CUBES_ISOVALUE_STEP            0.01f

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

        // We need two spatial grids.
        // One will estimate the density of the particles. This spatial grid divides the simulation space in cubes and counts
        // the number of particles within these cubes. This spatial grid needs to overlap the simulation space by one cube in
        // every direction so we can have a density of value 0 outside of the simulation space.
        // We will call this spatial grid the density estimator.
        int number_of_cells_density_estimator;
        int number_of_cells_x_density_estimator;
        int number_of_cells_y_density_estimator;
        int number_of_cells_z_density_estimator;
        std::vector<int> density_estimator;
        // Since we will use multiple threads to estimate the density and multiple threads could add a value to the density
        // of the same grid cell we need to be thread safe using mutex.
        std::vector<std::unique_ptr<std::mutex>> mutex_density_estimator;

        // The second spatial grid is basically the vector of the marching cubes. We do not need to divide the space again since
        // this already happened with the first spatial grid. A marching cube grid has one cube less in every axis than the previous
        // spatial grid and this grid is shifted by half of the cube length so the vertices of the marching cube lay at the center
        // of a cell of the first spatial grid. The density values of the first spatial grid are then mapped to the respective 
        // vertices of a marching cubes.
        // We will call this simply marching cubes.
        int number_of_cells_marching_cubes;
        int number_of_cells_x_marching_cubes;
        int number_of_cells_y_marching_cubes;
        int number_of_cells_z_marching_cubes;
        std::vector<Marching_Cube> marching_cubes;
        
        // This function calculates the number of grid cells for both spatial grids mentioned above as well as resizes them.
        void calculate_number_of_grid_cells ();
        // This function will be used to get the grid key for the density estimator spatial grid. It simply discretizes a given float value.
        int discretize_value (float value);
        // This function returns based on the position in 3d space to what cell (grid key) in the density estimator spatial grid
        // this position belongs. We will need this for the density estimation itself (to what cell does a particle count) as well as 
        // to set the vertex values of a marching cube.
        int get_grid_key_density_estimator (glm::vec3 position);
        // This function does the reverse of the function above. Given a grid key it returns the 3d position in space (the center)
        // of the grid cell. But note that this function operates on the marching cube grid. It is used to set the position of the min
        // corner of a marching cube since we need this in the geometry shader.
        glm::vec3 get_position_from_grid_key_marching_cube (int grid_key);
        // The position of a marching cube only needs to be set when the number of grid cells change (so when the resolution changes or
        // when the simulation space changes).
        void set_cube_position (unsigned int index_start, unsigned int index_end);
        // This function estimates the density of all grid cells within the density estimator. We simply count the number of particles within
        // each grid cell.
        void estimate_density (unsigned int index_start, unsigned int index_end);
        // This function takes the marching cubes and looks for every corner / vertex of a cube what the value within the density estimator
        // grid is for this position. We do not need to reset the values for the next run since they will be overwritten in the next run.
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
        float isovalue;

        // Draws the marching cubes. Note that the shader will be selected and activated by the visualization handler.
        void draw (bool unbind = false);
        // Deletes the GPU ressources (vertex array, vertex buffer, index buffer) for both contained 
        // "objects": the marching cubes as well as the grid that can be visualized too.
        void free_gpu_resources ();
};