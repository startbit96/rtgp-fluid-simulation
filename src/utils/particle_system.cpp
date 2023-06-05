#include "particle_system.h"

#include <math.h>

#include "debug.h"
#include "helper.h"

Particle_System::Particle_System ()
{
    this->vertex_array_object = 0;
    this->number_of_particles = 0;
    this->number_of_particles_as_string = to_string_with_separator(this->number_of_particles);
    this->particle_initial_distance = PARTICLE_INITIAL_DISTANCE_INIT;
    this->number_of_cells = 0;
}

void Particle_System::generate_initial_particles (std::vector<Cuboid>& cuboids)
{
    // Free the memory if it was used before.
    this->particles.clear();
    // Fill all cuboids with particles.
    for (int i = 0; i < cuboids.size(); i++) {
        cuboids[i].fill_with_particles(this->particle_initial_distance, this->particles);
    }
    // Get the number of particles.
    this->number_of_particles = this->particles.size();
    this->number_of_particles_as_string = to_string_with_separator(this->number_of_particles);

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
    GLCall( glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * this->number_of_particles, &this->particles[0], GL_STATIC_DRAW) );

    // Copy the indices into the index buffer object.
    GLCall( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->index_buffer_object) );
    this->particle_indices.clear();
    for (unsigned int i = 0; i < this->number_of_particles; i++) {
        this->particle_indices.push_back(i);
    }
    GLCall( glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * this->number_of_particles, &this->particle_indices[0], GL_STATIC_DRAW) );

    // Describe the vertex buffer layout of a particle.
    describe_particle_memory_layout();

    // Unbind.
    GLCall( glBindBuffer(GL_ARRAY_BUFFER, 0) );
    GLCall( glBindVertexArray(0) );
}

void Particle_System::initialize_spatial_grid (Cuboid simulation_space)
{
    // Calculate number of cells and offset for the hash formula.
    int number_of_cells_x = ceil((simulation_space.x_max - simulation_space.x_min) / SPH_KERNEL_RADIUS);
    int number_of_cells_y = ceil((simulation_space.y_max - simulation_space.y_min) / SPH_KERNEL_RADIUS);
    int number_of_cells_z = ceil((simulation_space.z_max - simulation_space.z_min) / SPH_KERNEL_RADIUS);
    this->number_of_cells = number_of_cells_x * number_of_cells_y * number_of_cells_z;
    this->hash_offset = glm::vec3(simulation_space.x_min, simulation_space.y_min, simulation_space.z_min);
}

bool Particle_System::increase_number_of_particles ()
{
    // Note that if we want to increase the number of particles, we need to decrease the initial distance of the particles.
    float new_particle_initial_distance = this->particle_initial_distance / PARTICLE_INITIAL_DISTANCE_INC_FACTOR;
    if (new_particle_initial_distance < PARTICLE_INITIAL_DISTANCE_MIN) {
        // We can not increase the number of particles more.
        return false;
    }
    else {
        // Increasement of number of particles can be done.
        this->particle_initial_distance = new_particle_initial_distance;
        return true;
    }
}

bool Particle_System::decrease_number_of_particles ()
{
    // Note that if we want to decrease the number of particles, we need to increase the initial distance of the particles.
    float new_particle_initial_distance = this->particle_initial_distance * PARTICLE_INITIAL_DISTANCE_INC_FACTOR;
    if (new_particle_initial_distance > PARTICLE_INITIAL_DISTANCE_MAX) {
        // We can not decrease the number of particles more.
        return false;
    }
    else {
        // Decreasement of number of particles can be done.
        this->particle_initial_distance = new_particle_initial_distance;
        return true;
    }
}

inline int Particle_System::discretize_value (float value)
{
    return (int)floor(value / SPH_KERNEL_RADIUS);
}

inline int Particle_System::hash (glm::vec3 position)
{
    // Do we have to move the position to positive values? Just do it for now and maybe 
    // delete it later.
    // Link to the paper for the hash function:
    // Optimized Spatial Hashing for Collision Detection of Deformable Objects
    // https://matthias-research.github.io/pages/publications/tetraederCollision.pdf
    position = position + this->hash_offset;
    return (
        (Particle_System::discretize_value(position.x) * HASH_FUNCTION_PRIME_NUMBER_1) ^
        (Particle_System::discretize_value(position.y) * HASH_FUNCTION_PRIME_NUMBER_2) ^
        (Particle_System::discretize_value(position.z) * HASH_FUNCTION_PRIME_NUMBER_3)
    ) % this->number_of_cells;
}

void Particle_System::calculate_spatial_grid ()
{
    // Clear it first.
    this->spatial_hash_grid.clear();
    // Now insert every particle based on its position.
    for (int i = 0; i < this->number_of_particles; i++) {
        this->spatial_hash_grid.insert(std::make_pair(
            Particle_System::hash(this->particles[i].position), 
            this->particles[i]
        ));
    }
}

void Particle_System::find_neighbors ()
{
    // Clear the vector of neighbor lists and announce the size of the vector.
    this->neighbor_list.clear();
    this->neighbor_list.resize(this->number_of_particles);
    for (int i = 0; i < this->number_of_particles; i++) {
        // Look for all particles into the neighbouring 26 cells.
        for (int look_x = -1; look_x <= 1; look_x++) {
            for (int look_y = -1; look_y <= 1; look_y++) {
                for (int look_z = -1; look_z <= 1; look_z++) {
                    glm::vec3 look_position = this->particles[i].position + glm::vec3(look_x, look_y, look_z) * SPH_KERNEL_RADIUS;
                    int look_key = this->hash(look_position);
                    auto range = this->spatial_hash_grid.equal_range(look_key);
                    for (auto it = range.first; it != range.second; ++it) {
                        // If the distance to this particle is less than the kernel radius, we need this one.
                        if (glm::distance(this->particles[i].position, it->second.position) < SPH_KERNEL_RADIUS) {
                            this->neighbor_list[i].push_back(it->second);
                        }
                    }
                }
            }
        }
    } 
}

inline float Particle_System::kernel_w_poly6 (glm::vec3 distance_vector)
{
    static float coefficient = 315.0f / (64.0f * M_PI * pow(SPH_KERNEL_RADIUS, 9));
    static float kernel_radius_squared = pow(SPH_KERNEL_RADIUS, 2);
    float distance_squared = glm::dot(distance_vector, distance_vector);
    return coefficient * (float)pow(kernel_radius_squared - distance_squared, 3);
}

inline glm::vec3 Particle_System::kernel_w_poly6_gradient (glm::vec3 distance_vector)
{
    static float coefficient = -945.0f / (32.0f * M_PI * pow(SPH_KERNEL_RADIUS, 9));
    static float kernel_radius_squared = pow(SPH_KERNEL_RADIUS, 2);
    float distance_squared = glm::dot(distance_vector, distance_vector);
    return (coefficient * (float)pow(kernel_radius_squared - distance_squared, 2)) * distance_vector;
}

inline float Particle_System::kernel_w_poly6_laplacian (glm::vec3 distance_vector)
{
    static float coefficient = -945.0f / (32.0f * M_PI * pow(SPH_KERNEL_RADIUS, 9));
    static float kernel_radius_squared = pow(SPH_KERNEL_RADIUS, 2);
    float distance_squared = glm::dot(distance_vector, distance_vector);
    return coefficient * (kernel_radius_squared - distance_squared) * (3.0f * kernel_radius_squared - 7.0f * distance_squared);
}

inline glm::vec3 Particle_System::kernel_w_spiky_gradient (glm::vec3 distance_vector)
{
    static float coefficient = -45.0f / (M_PI * pow(SPH_KERNEL_RADIUS, 6));
    float distance = glm::length(distance_vector);
    return coefficient * (float)pow(SPH_KERNEL_RADIUS - distance, 2) * glm::normalize(distance_vector);
}

inline float Particle_System::kernel_w_viscosity_laplacian (glm::vec3 distance_vector)
{
    static float coefficient = 45.0f / (M_PI * pow(SPH_KERNEL_RADIUS, 6));
    float distance = glm::length(distance_vector);
    return coefficient * (SPH_KERNEL_RADIUS - distance);
}

void Particle_System::calculate_density ()
{
    for (int i = 0; i < this->number_of_particles; i++) {
        this->particles[i].density = 0;
        for (int idx_neighbor = 0; idx_neighbor < this->neighbor_list[i].size(); idx_neighbor++) {
            this->particles[i].density += SPH_PARTICLE_MASS * 
                this->kernel_w_poly6(this->neighbor_list[i][idx_neighbor].position - this->particles[i].position);
        }
    }
}

void Particle_System::simulate ()
{
    this->calculate_spatial_grid();
    this->find_neighbors();
    this->calculate_density();
}

void Particle_System::draw (bool unbind)
{
    GLCall( glBindVertexArray(this->vertex_array_object) );
    GLCall( glDrawElements(GL_POINTS, this->number_of_particles, GL_UNSIGNED_INT, 0) );
    // In order to save a few unbind-calls, do this only if neccessary. 
    // In our case, the visualization handler will handle the unbinding, so normally we will not unbind here.
    if (unbind == true) {
        GLCall( glBindVertexArray(0) );
    }
}

void Particle_System::free_gpu_resources ()
{
    if (this->vertex_array_object > 0) {
        GLCall( glDeleteVertexArrays(1, &this->vertex_array_object) );
        GLCall( glDeleteBuffers(1, &this->vertex_buffer_object) );
        GLCall( glDeleteBuffers(1, &this->index_buffer_object) );
    }
}