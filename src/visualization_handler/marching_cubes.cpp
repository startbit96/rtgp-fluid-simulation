#include "marching_cubes.h"

#include <string>
#include <thread>

#include "../utils/debug.h"
#include "../utils/helper.h"


// ====================================== MARCHING CUBES GENERATOR ======================================

Marching_Cubes_Generator::Marching_Cubes_Generator ()
{
    this->vertex_array_object = 0;
    this->cube_edge_length = -1.0f;
    this->new_cube_edge_length = MARCHING_CUBES_CUBE_EDGE_LENGTH;
    this->number_of_cells_density_estimator = 0;
    this->number_of_cells_marching_cubes = 0;
    this->dataChanged = false;
    this->isovalue = MARCHING_CUBES_ISOVALUE;
}


// ====================================== MULTITHREADING ======================================

void Marching_Cubes_Generator::parallel_for (void (Marching_Cubes_Generator::* function)(unsigned int, unsigned int), int number_of_elements)
{
    // Create threads and execute the desired function in chunks.
    // Calculate the chunk size (it depends whether we operate on the particles vector itself or the spatial grid).
    if (this->particle_system->number_of_threads == 1) {
        // Just execute the function if only one thread is desired.
        (this->*function)(0, number_of_elements - 1);
        return;
    }
    int chunk_size = number_of_elements / this->particle_system->number_of_threads;
    std::vector<std::thread> threads;
    threads.reserve(this->particle_system->number_of_threads);
    // Create the threads.
    for (int i = 0; i < this->particle_system->number_of_threads; i++) {
        int chunk_start = i * chunk_size;
        int chunk_end = chunk_start + chunk_size - 1;
        // The last chunk goes until the end of the vector.
        if (i == this->particle_system->number_of_threads - 1) {
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


// ====================================== INITIALIZATION FUNCTIONS ======================================

void Marching_Cubes_Generator::calculate_number_of_grid_cells ()
{
    // At first we calculate the number of cells for the density estimator.
    // We add one cube row before and one cube behind the simulation space to have good surfaces.
    // We need this to be sure to have density values of zero at the borders.
    this->number_of_cells_x_density_estimator = ceil((this->particle_system->simulation_space->x_max - this->particle_system->simulation_space->x_min) / 
        this->cube_edge_length) + 2;
    this->number_of_cells_y_density_estimator = ceil((this->particle_system->simulation_space->y_max - this->particle_system->simulation_space->y_min) / 
        this->cube_edge_length) + 2;
    this->number_of_cells_z_density_estimator = ceil((this->particle_system->simulation_space->z_max - this->particle_system->simulation_space->z_min) / 
        this->cube_edge_length) + 2;
    this->number_of_cells_density_estimator = this->number_of_cells_x_density_estimator * 
        this->number_of_cells_y_density_estimator * this->number_of_cells_z_density_estimator;
    // Reset and resize the density estimator spatial grid.
    this->density_estimator.clear();
    this->density_estimator.resize(this->number_of_cells_density_estimator);
    // Resize the mutex vector for the density estimator.
    // Iterate over each element in the vector and assign a newly created std::mutex using std::make_unique.
    this->mutex_density_estimator.clear();
    this->mutex_density_estimator.resize(this->number_of_cells_density_estimator);
    for (auto& mutex : this->mutex_density_estimator) {
        mutex = std::make_unique<std::mutex>();
    }

    // Now do the same for the marching cubes. The number of cells of the marching cubes in each axis is one less
    // than the number of the cells of the density estimator since it is shifted half the cubes edge length and ends
    // half the cubes edge length earlier.
    this->number_of_cells_x_marching_cubes = this->number_of_cells_x_density_estimator - 1;
    this->number_of_cells_y_marching_cubes = this->number_of_cells_y_density_estimator - 1;
    this->number_of_cells_z_marching_cubes = this->number_of_cells_z_density_estimator - 1;
    this->number_of_cells_marching_cubes = this->number_of_cells_x_marching_cubes * 
        this->number_of_cells_y_marching_cubes * this->number_of_cells_z_marching_cubes;
    // Reset and resize the spatial grid of the marching cubes.
    this->marching_cubes.clear();
    this->marching_cubes.resize(this->number_of_cells_marching_cubes);
    // For the marching cubes we will not need a mutex vector.

    // The recalculation of the grid cells / cubes also demands the resetting of the VBO, IBO and VAO since
    // the number of cubes changed.

    // Clear the buffers if there is something to clear.
    this->free_gpu_resources();

    // Generate the OpenGL buffers for the particle system.
    GLCall( glGenVertexArrays(1, &this->vertex_array_object) );
    GLCall( glGenBuffers(1, &this->vertex_buffer_object) );
    GLCall( glGenBuffers(1, &this->index_buffer_object) );

    // Make vertex array object active.  
    GLCall( glBindVertexArray(this->vertex_array_object) );

    // Copy the data into the vertex buffer object.
    GLCall( glBindBuffer(GL_ARRAY_BUFFER, this->vertex_buffer_object) );
    GLCall( glBufferData(GL_ARRAY_BUFFER, sizeof(Marching_Cube) * this->number_of_cells_marching_cubes, &this->marching_cubes.at(0), GL_STATIC_DRAW) );

    // Copy the indices into the index buffer object.
    GLCall( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->index_buffer_object) );
    this->indices.clear();
    for (unsigned int i = 0; i < this->number_of_cells_marching_cubes; i++) {
        this->indices.push_back(i);
    }
    GLCall( glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * this->number_of_cells_marching_cubes, &this->indices.at(0), GL_STATIC_DRAW) );

    // Describe the vertex buffer layout of a marching cube.
    unsigned int index = 0;
    // Position of the corner.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, sizeof(Marching_Cube), (GLvoid*)0) );
    index++;
    // Number of particles.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 1, GL_INT, GL_FALSE, sizeof(Marching_Cube), (GLvoid*)(offsetof(Marching_Cube, number_of_particles_within))) );
    index++;
    // Vertex values.
    // Vertex 0.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 1, GL_INT, GL_FALSE, sizeof(Marching_Cube), (GLvoid*)(offsetof(Marching_Cube, value_vertex_0))) );
    index++;
    // Vertex 1.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 1, GL_INT, GL_FALSE, sizeof(Marching_Cube), (GLvoid*)(offsetof(Marching_Cube, value_vertex_1))) );
    index++;
    // Vertex 2.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 1, GL_INT, GL_FALSE, sizeof(Marching_Cube), (GLvoid*)(offsetof(Marching_Cube, value_vertex_2))) );
    index++;
    // Vertex 3.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 1, GL_INT, GL_FALSE, sizeof(Marching_Cube), (GLvoid*)(offsetof(Marching_Cube, value_vertex_3))) );
    index++;
    // Vertex 4.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 1, GL_INT, GL_FALSE, sizeof(Marching_Cube), (GLvoid*)(offsetof(Marching_Cube, value_vertex_4))) );
    index++;
    // Vertex 5.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 1, GL_INT, GL_FALSE, sizeof(Marching_Cube), (GLvoid*)(offsetof(Marching_Cube, value_vertex_5))) );
    index++;
    // Vertex 6.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 1, GL_INT, GL_FALSE, sizeof(Marching_Cube), (GLvoid*)(offsetof(Marching_Cube, value_vertex_6))) );
    index++;
    // Vertex 7.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 1, GL_INT, GL_FALSE, sizeof(Marching_Cube), (GLvoid*)(offsetof(Marching_Cube, value_vertex_7))) );
    index++;

    // Unbind.
    GLCall( glBindBuffer(GL_ARRAY_BUFFER, 0) );
    GLCall( glBindVertexArray(0) );
}

inline int Marching_Cubes_Generator::discretize_value (float value)
{
    // The cells of our grid have the edge length of the marching cubes.
    return (int)floor(value / this->cube_edge_length);
}

inline int Marching_Cubes_Generator::get_grid_key_density_estimator (glm::vec3 position)
{
    // We move the particles position into positive values and also a little bit more because we have 
    // the outer cubes.
    position = position + this->particle_system->particle_offset + glm::vec3(this->cube_edge_length);
    // Check if the position is within the density estimator grids volume.
    if ((position.x < 0.0f) || (position.x > this->number_of_cells_x_density_estimator * this->cube_edge_length) ||
        (position.y < 0.0f) || (position.y > this->number_of_cells_y_density_estimator * this->cube_edge_length) ||
        (position.z < 0.0f) || (position.z > this->number_of_cells_z_density_estimator * this->cube_edge_length)) {
            return -1;
    }
    return
        (Marching_Cubes_Generator::discretize_value(position.x)) +
        (Marching_Cubes_Generator::discretize_value(position.y) * this->number_of_cells_x_density_estimator) +
        (Marching_Cubes_Generator::discretize_value(position.z) * this->number_of_cells_x_density_estimator * this->number_of_cells_y_density_estimator);
}


inline glm::vec3 Marching_Cubes_Generator::get_position_from_grid_key_marching_cube (int grid_key)
{
    // Get the position index values in respective to the x, y and z coordinates.
    int cell_index_x = grid_key % this->number_of_cells_x_marching_cubes;
    int cell_index_y = (grid_key / this->number_of_cells_x_marching_cubes) % this->number_of_cells_y_marching_cubes;
    int cell_index_z = grid_key / (this->number_of_cells_x_marching_cubes * this->number_of_cells_y_marching_cubes);
    // From this calculate the position.
    glm::vec3 position = glm::vec3(
        cell_index_x * this->cube_edge_length,
        cell_index_y * this->cube_edge_length,
        cell_index_z * this->cube_edge_length
    );
    // We need to translate the position back to the global coordinates since we moved the position to
    // only allow position values. 
    // We will not shift for the whole cube edge length but only for the half since the marching cubes are 
    // shifted halb the cube edge length in comparison to the density estimator.
    position = position - this->particle_system->particle_offset - glm::vec3(this->cube_edge_length / 2);
    return position;
}


void Marching_Cubes_Generator::set_cube_position (unsigned int index_start, unsigned int index_end) 
{
    for (int idx_cell = index_start; idx_cell <= index_end; idx_cell++) {
        this->marching_cubes.at(idx_cell).corner_min = this->get_position_from_grid_key_marching_cube(idx_cell);
    }
}

void Marching_Cubes_Generator::simulation_space_changed ()
{
    // To inform the "generate_marching_cubes" function that a resizing of the spatial grid is needed, we simply
    // set the cube_edge_length to a negative value, because this will trigger the recalculation.
    this->cube_edge_length = -1.0f;
}

// ====================================== MARCHING CUBE ALGORITHM ======================================


void Marching_Cubes_Generator::estimate_density (unsigned int index_start, unsigned int index_end)
{
    for (int i = index_start; i <= index_end; i++) {
        // Assign the particle based on its position to a grid cell. Get the index of this cell.
        int grid_key = this->get_grid_key_density_estimator(this->particle_system->particles.at(i).position);
        // Lock the spatial grid cell for the insertion of the particle.
        std::unique_lock<std::mutex> lock(*this->mutex_density_estimator.at(grid_key));
        this->density_estimator.at(grid_key)++;
        lock.unlock();
    }
}

void Marching_Cubes_Generator::calculate_vertex_values (unsigned int index_start, unsigned int index_end)
{
    // We will look into the density estimator for all the vertices of the cube. How the vertices are indexed is 
    // shown here. It is important to keep this indexing since we will use the same in the geometry shader for the
    // marching cubes algorithm.
    //
    //        4 +--------+ 5
    //         /|       /|
    //        / |      / |
    //     7 +--------+ 6|
    //       |  |     |  |
    //       |0 +-----|--+ 1
    //       | /      | /
    //       |/       |/
    //     3 +--------+ 2

    for (int idx_cell = index_start; idx_cell <= index_end; idx_cell++) {
        // Get a representive position of this cell. 
        glm::vec3 position = this->marching_cubes.at(idx_cell).corner_min;
        // Set the values.
        this->marching_cubes.at(idx_cell).value_vertex_0 = this->density_estimator.at(
            get_grid_key_density_estimator(position));
        this->marching_cubes.at(idx_cell).value_vertex_1 = this->density_estimator.at(
            get_grid_key_density_estimator(position + this->cube_edge_length * glm::vec3(1.0f, 0.0f, 0.0f)));
        this->marching_cubes.at(idx_cell).value_vertex_2 = this->density_estimator.at(
            get_grid_key_density_estimator(position + this->cube_edge_length * glm::vec3(1.0f, 0.0f, 1.0f)));
        this->marching_cubes.at(idx_cell).value_vertex_3 = this->density_estimator.at(
            get_grid_key_density_estimator(position + this->cube_edge_length * glm::vec3(0.0f, 0.0f, 1.0f)));
        this->marching_cubes.at(idx_cell).value_vertex_4 = this->density_estimator.at(
            get_grid_key_density_estimator(position + this->cube_edge_length * glm::vec3(0.0f, 1.0f, 0.0f)));
        this->marching_cubes.at(idx_cell).value_vertex_5 = this->density_estimator.at(
            get_grid_key_density_estimator(position + this->cube_edge_length * glm::vec3(1.0f, 1.0f, 0.0f)));
        this->marching_cubes.at(idx_cell).value_vertex_6 = this->density_estimator.at(
            get_grid_key_density_estimator(position + this->cube_edge_length * glm::vec3(1.0f, 1.0f, 1.0f)));
        this->marching_cubes.at(idx_cell).value_vertex_7 = this->density_estimator.at(
            get_grid_key_density_estimator(position + this->cube_edge_length * glm::vec3(0.0f, 1.0f, 1.0f)));
    }
}

void Marching_Cubes_Generator::generate_marching_cubes ()
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
        // The marching cubes vector was resized and refilled with empty marching cubes. These no longer hold
        // information about their position in space, so we need to update this once.
        this->parallel_for(&Marching_Cubes_Generator::set_cube_position, this->number_of_cells_marching_cubes);
        // Since we cleared the whole marching cubes grid we do not need to reset the information stored within these
    }
    else {
        // We can still operate on the datastructures we have but we need to reset the count of the particles to zero.
        std::fill(this->density_estimator.begin(), this->density_estimator.end(), 0);
    }
    // Calculate the number of particles within each cube. Here are mutex needed.
    this->parallel_for(&Marching_Cubes_Generator::estimate_density, this->particle_system->number_of_particles);
    // Now update the vertex values for all cubes. Here are no mutex needed.
    this->parallel_for(&Marching_Cubes_Generator::calculate_vertex_values, this->number_of_cells_marching_cubes);
    // The data changed, so inform the draw call to update the data.
    this->dataChanged = true;
}


// ====================================== RENDERING RELATED FUNCTIONS ======================================

// Draws the cubes (the triangles will be generated in the geometry shader).
// Since the marching cubes also hold the information about the position in space, we can use the same data
// structure for drawing the grid used. Therefore we will simply load another shader.
void Marching_Cubes_Generator::draw (bool unbind)
{
    // Update the marching cubes data in the vertex buffer object.
    // But only if the data changed.
    if (this->dataChanged == true) {
        GLCall( glBindBuffer(GL_ARRAY_BUFFER, this->vertex_buffer_object) );
        GLCall( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Marching_Cube) * this->number_of_cells_marching_cubes, &this->marching_cubes.at(0)) );
        GLCall( glBindBuffer(GL_ARRAY_BUFFER, 0) );
        this->dataChanged = false;
    }
    // Draw the marching cubes using the vertex array object.
    GLCall( glBindVertexArray(this->vertex_array_object) );
    GLCall( glDrawElements(GL_POINTS, this->number_of_cells_marching_cubes, GL_UNSIGNED_INT, 0) );
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
}