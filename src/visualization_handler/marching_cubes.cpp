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
    this->number_of_cells = 0;
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
    // We add one cube row before and one cube behind the simulation space to have good surfaces.
    this->number_of_cells_x = ceil((this->particle_system->simulation_space->x_max - this->particle_system->simulation_space->x_min) / 
        this->cube_edge_length) + 2;
    this->number_of_cells_y = ceil((this->particle_system->simulation_space->y_max - this->particle_system->simulation_space->y_min) / 
        this->cube_edge_length) + 2;
    this->number_of_cells_z = ceil((this->particle_system->simulation_space->z_max - this->particle_system->simulation_space->z_min) / 
        this->cube_edge_length) + 2;
    this->number_of_cells = this->number_of_cells_x * this->number_of_cells_y * this->number_of_cells_z;
    // The marching cubes information need also to be reset.
    this->marching_cubes.clear();
    this->marching_cubes.resize(this->number_of_cells);
    // Resize the mutex vector.
    // Iterate over each element in the vector and assign a newly created std::mutex using std::make_unique.
    this->mutex_spatial_grid.clear();
    this->mutex_spatial_grid.resize(this->number_of_cells);
    for (auto& mutex : this->mutex_spatial_grid) {
        mutex = std::make_unique<std::mutex>();
    }
    // The recalculation of the grid cells / cubes also demands the resetting of the vbo, ibo and vao since
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
    GLCall( glBufferData(GL_ARRAY_BUFFER, sizeof(Marching_Cube) * this->number_of_cells, &this->marching_cubes.at(0), GL_STATIC_DRAW) );

    // Copy the indices into the index buffer object.
    GLCall( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->index_buffer_object) );
    this->indices.clear();
    for (unsigned int i = 0; i < this->number_of_cells; i++) {
        this->indices.push_back(i);
    }
    GLCall( glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * this->number_of_cells, &this->indices.at(0), GL_STATIC_DRAW) );

    // Describe the vertex buffer layout of a marching cube.
    unsigned int index = 0;
    // Position of the corner.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, sizeof(Marching_Cube), (GLvoid*)0) );
    index++;
    // Number of particles.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(Marching_Cube), (GLvoid*)(offsetof(Marching_Cube, number_of_particles_within))) );
    index++;
    // Vertex values.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(Marching_Cube), (GLvoid*)(offsetof(Marching_Cube, vertex_values))) );
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

inline int Marching_Cubes_Generator::get_grid_key (glm::vec3 position)
{
    // We move the particles position into positive values and also a little bit more because we have 
    // the outer cubes.
    position = position + this->particle_system->particle_offset + glm::vec3(this->cube_edge_length);
    // Check if the position is within the grids volume.
    if ((position.x < 0.0f) || (position.x > this->number_of_cells_x * this->cube_edge_length) ||
        (position.y < 0.0f) || (position.y > this->number_of_cells_y * this->cube_edge_length) ||
        (position.z < 0.0f) || (position.z > this->number_of_cells_z * this->cube_edge_length)) {
            return -1;
    }
    return
        (Marching_Cubes_Generator::discretize_value(position.x)) +
        (Marching_Cubes_Generator::discretize_value(position.y) * this->number_of_cells_x) +
        (Marching_Cubes_Generator::discretize_value(position.z) * this->number_of_cells_x * this->number_of_cells_y);
}


inline glm::vec3 Marching_Cubes_Generator::get_min_corner_from_grid_key (int grid_key)
{
    // Get the position index values in respective to the x, y and z coordinates.
    int cell_index_x = grid_key % this->number_of_cells_x;
    int cell_index_y = (grid_key / this->number_of_cells_x) % this->number_of_cells_y;
    int cell_index_z = grid_key / (this->number_of_cells_x * this->number_of_cells_y);
    // From this calculate the position.
    glm::vec3 position = glm::vec3(
        cell_index_x * this->cube_edge_length,
        cell_index_y * this->cube_edge_length,
        cell_index_z * this->cube_edge_length
    );
    // We need to translate the position back to the global coordinates since we moved the position to
    // only allow position values.
    position = position - this->particle_system->particle_offset - glm::vec3(this->cube_edge_length);
    return position;
}


void Marching_Cubes_Generator::set_cube_position (unsigned int index_start, unsigned int index_end) 
{
    for (int idx_cell = index_start; idx_cell <= index_end; idx_cell++) {
        this->marching_cubes.at(idx_cell).corner_min = this->get_min_corner_from_grid_key(idx_cell);
    }
}

void Marching_Cubes_Generator::reset_cube_informations (unsigned int index_start, unsigned int index_end)
{
    for (int idx_cell = index_start; idx_cell <= index_end; idx_cell++) {
        // Reset the number of particles within the cube.
        this->marching_cubes.at(idx_cell).number_of_particles_within = 0;
        // Reset the vertex values.
        memset(this->marching_cubes.at(idx_cell).vertex_values, 0, sizeof(this->marching_cubes.at(idx_cell).vertex_values));
    }
}

void Marching_Cubes_Generator::simulation_space_changed ()
{
    // To inform the "generate_marching_cubes" function that a resizing of the spatial grid is needed, we simply
    // set the cube_edge_length to a negative value, because this will trigger the recalculation.
    this->cube_edge_length = -1.0f;
}

// ====================================== MARCHING CUBE ALGORITHM ======================================

void Marching_Cubes_Generator::count_particles (unsigned int index_start, unsigned int index_end)
{
    for (int i = index_start; i <= index_end; i++) {
        // Assign the particle based on its position to a grid cell. Get the index of this cell.
        int grid_key = this->get_grid_key(this->particle_system->particles.at(i).position);
        // Lock the spatial grid cell for the insertion of the particle.
        std::unique_lock<std::mutex> lock(*this->mutex_spatial_grid.at(grid_key));
        this->marching_cubes.at(grid_key).number_of_particles_within++;
        lock.unlock();
    }
}

void Marching_Cubes_Generator::calculate_vertex_values (unsigned int index_start, unsigned int index_end)
{
    // We will look into 27 cells (or less if we are at the border). This is quit a lot so we define
    // some bitmasks here that indicate depending on where I look what vertices are affected.
    // The axis are "normal": x is to the right, y to the top, z to the front.
    // The cube is indexed in this way:
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
    //
    // We keep in mind that we use in the following statements nested for loops to look into the neighboring cells.
    // This runs from look_x -1 to 1, look_y -1 to 1 and look_z -1 to 1 so the look up vertex mask needs to be somehow
    // related to this.
    static int look_up_bitmask_matrix[3][3][3] = {
        // x look to the left.
        {
            // y look down.
            {
                // z look to the back.
                //76543210
                0b00000001,
                // z look to the same z value.
                //76543210
                0b00001001,
                // z look to the front.
                //76543210
                0b00001000
            },
            // y look to the same y value.
            {
                // z look to the back.
                //76543210
                0b00010001,
                // z look to the same z value.
                //76543210
                0b10011001,
                // z look to the front.
                //76543210
                0b10001000
            },
            // y look up.
            {
                // z look to the back.
                //76543210
                0b00010000,
                // z look to the same z value.
                //76543210
                0b10010000,
                // z look to the front.
                //76543210
                0b10000000
            }
        },
        // x look to the same x value.
        {
            // y look down.
            {
                // z look to the back.
                //76543210
                0b00000011,
                // z look to the same z value.
                //76543210
                0b00001111,
                // z look to the front.
                //76543210
                0b00001100
            },
            // y look to the same y value.
            {
                // z look to the back.
                //76543210
                0b00110011,
                // z look to the same z value.
                //76543210
                0b11111111,
                // z look to the front.
                //76543210
                0b11001100
            },
            // y look up.
            {
                // z look to the back.
                //76543210
                0b00110000,
                // z look to the same z value.
                //76543210
                0b11110000,
                // z look to the front.
                //76543210
                0b11000000
            }
        },
        // x look to the right.
        {
            // y look down.
            {
                // z look to the back.
                //76543210
                0b00000010,
                // z look to the same z value.
                //76543210
                0b00000110,
                // z look to the front.
                //76543210
                0b00000100
            },
            // y look to the same y value.
            {
                // z look to the back.
                //76543210
                0b00100010,
                // z look to the same z value.
                //76543210
                0b01100110,
                // z look to the front.
                //76543210
                0b01000100
            },
            // y look up.
            {
                // z look to the back.
                //76543210
                0b00100000,
                // z look to the same z value.
                //76543210
                0b01100000,
                // z look to the front.
                //76543210
                0b01000000
            }
        }
    };

    for (int idx_cell = index_start; idx_cell <= index_end; idx_cell++) {
        // Get a representive position of this cell. Do not use the corner because it could lead the float precision erros
        // determining the cell index.
        glm::vec3 current_cell_position = this->get_min_corner_from_grid_key(idx_cell) + glm::vec3(0.5f * this->cube_edge_length);
        for (int look_x = -1; look_x <= 1; look_x++) {
            for (int look_y = -1; look_y <= 1; look_y++) {
                for (int look_z = -1; look_z <= 1; look_z++) {
                    glm::vec3 look_position = current_cell_position + glm::vec3(look_x, look_y, look_z) * this->cube_edge_length;
                    int idx_neighbor = this->get_grid_key(look_position);
                    if (idx_neighbor == -1) {
                        // This cell does not exist. So the current one seems to be at the border.
                        continue;
                    }
                    // If it exists, add the number of particles within this cell to the affected vertices.
                    for (int vertex_index = 0; vertex_index < 8; vertex_index++) {
                        if (look_up_bitmask_matrix[look_x + 1][look_y + 1][look_z + 1] & (1 << vertex_index)) {
                            this->marching_cubes.at(idx_cell).vertex_values[vertex_index] += 
                                this->marching_cubes.at(idx_neighbor).number_of_particles_within;
                        }
                    }
                }
            }
        }
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
        this->parallel_for(&Marching_Cubes_Generator::set_cube_position, this->number_of_cells);
        // Since we cleared the whole marching cubes grid we do not need to reset the information stored within these
    }
    else {
        // The marching cubes vector does not need to be resized so we can keep him.
        // The vector still holds the information about the position of each marching cube.
        // But the vector also still holds the information about the number of particles within each cube as well
        // as the vertice attributes. So we need to reset these.
        this->parallel_for(&Marching_Cubes_Generator::reset_cube_informations, this->number_of_cells);
    }
    // Calculate the number of particles within each cube. Here are mutex needed.
    this->parallel_for(&Marching_Cubes_Generator::count_particles, this->particle_system->number_of_particles);
    // Go through all cubes, look to all sides and adjust the vertex values depending on the values of the neighboring cubes.
    // Here are no mutex needed.
    this->parallel_for(&Marching_Cubes_Generator::calculate_vertex_values, this->number_of_cells);
}


// ====================================== RENDERING RELATED FUNCTIONS ======================================

// Draws the cubes (the triangles will be generated in the geometry shader).
// Since the marching cubes also hold the information about the position in space, we can use the same data
// structure for drawing the grid used. Therefore we will simply load another shader.
void Marching_Cubes_Generator::draw (bool unbind)
{
    // Update the marching cubes data in the vertex buffer object.
    GLCall( glBindBuffer(GL_ARRAY_BUFFER, this->vertex_buffer_object) );
    GLCall( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Marching_Cube) * this->number_of_cells, &this->marching_cubes.at(0)) );
    GLCall( glBindBuffer(GL_ARRAY_BUFFER, 0) );
    // Draw the marching cubes using the vertex array object.
    GLCall( glBindVertexArray(this->vertex_array_object) );
    GLCall( glDrawElements(GL_POINTS, this->number_of_cells, GL_UNSIGNED_INT, 0) );
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