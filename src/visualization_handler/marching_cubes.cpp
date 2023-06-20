#include "marching_cubes.h"

#include <thread>

#include "../utils/debug.h"
#include "../utils/helper.h"

Marching_Cubes_Generator::Marching_Cubes_Generator ()
{
    this->vertex_array_object = 0;
    this->cube_edge_length = -1.0f;
    this->new_cube_edge_length = this->cube_edge_length;
}


// ====================================== INITIALIZATION FUNCTIONS ======================================

void Marching_Cubes_Generator::calculate_number_of_grid_cells ()
{
    // We add one cube row before and one cube behind the simulation space to have good surfaces.
    this->number_of_cells_x = ceil((this->simulation_space->x_max - this->simulation_space->x_min) / 
        this->cube_edge_length) + 2;
    this->number_of_cells_y = ceil((this->simulation_space->y_max - this->simulation_space->y_min) / 
        this->cube_edge_length) + 2;
    this->number_of_cells_z = ceil((this->simulation_space->z_max - this->simulation_space->z_min) / 
        this->cube_edge_length) + 2;
    this->number_of_cells = this->number_of_cells_x * this->number_of_cells_y * this->number_of_cells_z;
    // Resize the mutex vector.
    // Iterate over each element in the vector and assign a newly created std::mutex using std::make_unique.
    this->mutex_spatial_grid.clear();
    this->mutex_spatial_grid.resize(this->number_of_cells);
    for (auto& mutex : this->mutex_spatial_grid) {
        mutex = std::make_unique<std::mutex>();
    }
}

void Marching_Cubes_Generator::set_simulation_space (Cuboid* simulation_space)
{
    this->simulation_space = simulation_space;
    // Reset the grid cell edge length so that the vectors have to be recalculated the next time.
    this->cube_edge_length = -1.0f;
}


// ====================================== MULTITHREADING ======================================

void Marching_Cubes_Generator::parallel_for (void (Marching_Cubes_Generator::* function)(unsigned int, unsigned int), int number_of_elements, unsigned int number_of_threads)
{
    // Create threads and execute the desired function in chunks.
    // Calculate the chunk size (it depends whether we operate on the particles vector itself or the spatial grid).
    if (number_of_threads == 1) {
        // Just execute the function if only one thread is desired.
        (this->*function)(0, number_of_elements - 1);
        return;
    }
    int chunk_size = number_of_elements / number_of_threads;
    std::vector<std::thread> threads;
    threads.reserve(number_of_threads);
    // Create the threads.
    for (int i = 0; i < number_of_threads; i++) {
        int chunk_start = i * chunk_size;
        int chunk_end = chunk_start + chunk_size - 1;
        // The last chunk goes until the end of the vector.
        if (i == number_of_threads - 1) {
            chunk_end = number_of_elements - 1;
        }
        // With the following call we not only append the thread to the vector but with 
        // creating the thread it also starts.
        threads.emplace_back(function, this, chunk_start, chunk_end);
    }
    // Wait for the threads to finish.
    for (auto& thread : threads) {
        thread.join();
    }
}

void Marching_Cubes_Generator::parallel_for_grid (void (Marching_Cubes_Generator::* function)(unsigned int, unsigned int), unsigned int number_of_threads)
{
    // The parallel_for_grid function works not on a vector of particles but on the grid. 
    // On the grid we do not know if the particles are evenly distributed over the whole simulation space
    // (they are most likely not), so we have to dynamically assign the chunks based on the number of particles
    // in the grid cells.
    // Create threads and execute the desired function in chunks.
    // Calculate the chunk size not on the number of grid cells (evenly), but on the number of particles.
    if (number_of_threads == 1) {
        // Just execute the function if only one thread is desired.
        (this->*function)(0, this->number_of_cells - 1);
        return;
    }
    int evenly_distributed_number_of_particles = this->number_of_particles / number_of_threads;
    std::vector<std::thread> threads;
    threads.reserve(number_of_threads);
    // Create the threads.
    int chunk_number_of_particles = 0;
    int already_assigned_number_of_particles = 0;
    int chunk_start = 0;
    int chunk_end;
    for (int idx_cell = 0; idx_cell < this->number_of_cells; idx_cell++) {
        chunk_number_of_particles += this->spatial_grid.at(idx_cell).size();
        if (chunk_number_of_particles >= evenly_distributed_number_of_particles) {
            chunk_end = idx_cell;
            threads.emplace_back(function, this, chunk_start, chunk_end);
            chunk_start = idx_cell + 1;
            already_assigned_number_of_particles += chunk_number_of_particles;
            chunk_number_of_particles = 0;
        }
        // Check if the number of particles is already reached (this can be if the particles are really near to each other).
        // If so, break.
        if (already_assigned_number_of_particles == this->number_of_particles) {
            break;
        }
        // Check if we are now in for the last thread. If so, just assign the task.
        // It can also be that the particles are so unevenly distributed that e.g. after 6/8 threads most of 
        // the particles are assigned to the threads and a seventh one would not be filled up completely. Check this case too.
        if ((threads.size() == (number_of_threads - 1)) || 
            ((this->number_of_particles - already_assigned_number_of_particles) < evenly_distributed_number_of_particles)) {
            threads.emplace_back(function, this, chunk_start, this->number_of_cells - 1);
            break;
        }
    }
    // Wait for the threads to finish.
    for (auto& thread : threads) {
        thread.join();
    }
}


// ====================================== MARCHING CUBE ALGORITHM ======================================

void Marching_Cubes_Generator::simulation_space_changed ()
{
    // To inform the "generate_marching_cubes" function that a resizing of the spatial grid is needed, we simply
    // set the cube_edge_length to a negative value, because this will trigger the recalculation.
    this->cube_edge_lenght = -1.0f;
}

void Marching_Cubes_Generator::generate_marching_cubes (std::vector<Particle>& particles, unsigned int number_of_threads)
{
    // Check if the new resolution is different from before. If so, also resize the mutex vector.
    if (this->new_cube_edge_length < 0.0f) {
        std::cout << "ERROR: Set the cube edge length for the marching cubes algorithm first." << std::endl;
        return;
    }
    if (floats_are_same(this->cube_edge_length, this->new_cube_edge_length, MARCHING_CUBES_CUBE_EDGE_LENGTH_STEP) == false) {
        // The cube length changed. Calculate the new number of cells. 
        // This call also resizes the mutex vector, therefore it makes sense to only do it if the values changed.
        this->cube_edge_length = this->new_cube_edge_length;
        this->calculate_number_of_grid_cells();
    }
    // Clear the previous the grid.
    this->spatial_grid.clear();
    // Create the spatial grid based on the particles in the particle system.
    // To approximate the density of a cell we will use the amount of particles within this cell.
    // Therefore the spatial grid has no particles in it but only an unsigned int value representing the 
    // number of particles that would fall into this grid.
    

}


// ====================================== RENDERING RELATED FUNCTIONS ======================================

// Draws the cubes (the triangles will be generated in the geometry shader).
// Since the marching cubes also hold the information about the position in space, we can use the same data
// structure for drawing the grid used. Therefore we will simply load another shader.
void Marching_Cubes_Generator::draw (bool unbind)
{
    GLCall( glBindVertexArray(this->vertex_array_object) );
    GLCall( glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0) );
    // In order to save a few unbind-calls, do this only if neccessary. 
    // In our case, the visualization handler will handle the unbinding, so normally we will not unbind here.
    if (unbind == true) {
        GLCall( glBindVertexArray(0) );
    }
}

void Marching_Cubes_Generator::free_gpu_resources ()
{
    if (this->vertex_array_object > 0) {
        GLCall( glDeleteVertexArrays(1, &this->vertex_array_object) );
        GLCall( glDeleteBuffers(1, &this->vertex_buffer_object) );
        GLCall( glDeleteBuffers(1, &this->index_buffer_object) );
    }
    if (this->vertex_array_object_grid > 0) {
        GLCall( glDeleteVertexArrays(1, &this->vertex_array_object_grid) );
        GLCall( glDeleteBuffers(1, &this->vertex_buffer_object_grid) );
        GLCall( glDeleteBuffers(1, &this->index_buffer_object_grid) );
    }
}