#include "particle_system.h"

#include <math.h>
#include <thread>
#include <algorithm>

#include "debug.h"
#include "helper.h"

Particle_System::Particle_System ()
{
    this->vertex_array_object = 0;
    this->number_of_particles = 0;
    this->number_of_particles_as_string = to_string_with_separator(this->number_of_particles);
    this->particle_initial_distance = PARTICLE_INITIAL_DISTANCE_INIT;
    this->sph_kernel_radius = this->calculate_kernel_radius();
    this->number_of_cells = 0;
    this->gravity_mode = GRAVITY_NORMAL;
    this->computation_mode = COMPUTATION_MODE_SPATIAL_GRID_CLEAR_MODE;
}


// ====================================== INITIALIZATION FUNCTIONS ======================================

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

    // Reset the simulation time.
    this->simulation_step = 0;
    // Clear the spatial grid so that it has to be refilled.
    this->spatial_grid.clear();
}

inline float Particle_System::calculate_kernel_radius ()
{
    return sqrt(2) * this->particle_initial_distance;
}

void Particle_System::set_simulation_space (Cuboid* simulation_space)
{
    // We need a pointer to the simulation space for resolving the collision
    // and performing operations needed for the spatial grid.
    this->simulation_space = simulation_space;
    // Calculate the offset for each particle later used in calculating the 
    // grid cells index of the spatial grid.
    this->particle_offset = glm::vec3(
        -simulation_space->x_min, 
        -simulation_space->y_min, 
        -simulation_space->z_min);
    // Calculate number of cells.
    this->calculate_number_of_grid_cells();
}


// ====================================== CHANGE NUMBER OF PARTICLES ======================================

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
        this->sph_kernel_radius = this->calculate_kernel_radius();
        this->calculate_number_of_grid_cells();
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
        this->sph_kernel_radius = this->calculate_kernel_radius();
        this->calculate_number_of_grid_cells();
        return true;
    }
}


// ====================================== SPH KERNEL FUNCTIONS ======================================

inline float Particle_System::kernel_w_poly6 (glm::vec3 distance_vector)
{
    float coefficient = 315.0f / (64.0f * M_PI * pow(this->sph_kernel_radius, 9));
    float kernel_radius_squared = pow(this->sph_kernel_radius, 2);
    float distance_squared = glm::dot(distance_vector, distance_vector);
    return coefficient * (float)pow(kernel_radius_squared - distance_squared, 3);
}

inline glm::vec3 Particle_System::kernel_w_poly6_gradient (glm::vec3 distance_vector)
{
    float coefficient = -945.0f / (32.0f * M_PI * pow(this->sph_kernel_radius, 9));
    float kernel_radius_squared = pow(this->sph_kernel_radius, 2);
    float distance_squared = glm::dot(distance_vector, distance_vector);
    return (coefficient * (float)pow(kernel_radius_squared - distance_squared, 2)) * distance_vector;
}

inline float Particle_System::kernel_w_poly6_laplacian (glm::vec3 distance_vector)
{
    float coefficient = -945.0f / (32.0f * M_PI * pow(this->sph_kernel_radius, 9));
    float kernel_radius_squared = pow(this->sph_kernel_radius, 2);
    float distance_squared = glm::dot(distance_vector, distance_vector);
    return coefficient * (kernel_radius_squared - distance_squared) * (3.0f * kernel_radius_squared - 7.0f * distance_squared);
}

inline glm::vec3 Particle_System::kernel_w_spiky_gradient (glm::vec3 distance_vector)
{
    float coefficient = -45.0f / (M_PI * pow(this->sph_kernel_radius, 6));
    float distance = glm::length(distance_vector);
    if (distance > 0.0f) {
        return coefficient * (float)pow(this->sph_kernel_radius - distance, 2) * (distance_vector / distance);
    }
    else {
        return glm::vec3(0.0f);
    }
}

inline float Particle_System::kernel_w_viscosity_laplacian (glm::vec3 distance_vector)
{
    float coefficient = 45.0f / (M_PI * pow(this->sph_kernel_radius, 6));
    float distance = glm::length(distance_vector);
    return coefficient * (this->sph_kernel_radius - distance);
}


// ====================================== GRAVITY ======================================

glm::vec3 Particle_System::get_gravity_vector ()
{
    if (this->gravity_mode == GRAVITY_OFF) {
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
    else if (this->gravity_mode == GRAVITY_NORMAL) {
        return glm::vec3(0.0f, -SPH_GRAVITY_MAGNITUDE, 0.0f);
    }
    else if (this->gravity_mode == GRAVITY_ROT_90) {
        if (((this->simulation_step / GRAVITY_MODE_ROT_SWITCH_TIME) % 2) == 0) {
            return glm::vec3(0.0f, -SPH_GRAVITY_MAGNITUDE, 0.0f);
        }
        else {
            return glm::vec3(-SPH_GRAVITY_MAGNITUDE, 0.0f, 0.0f);
        }
    }
    else if (this->gravity_mode == GRAVITY_WAVE) {
        return glm::vec3(
            sin(this->simulation_step * M_PI / 180.0f) * SPH_GRAVITY_MAGNITUDE, 
            -abs(cos(this->simulation_step * M_PI / 180.0f)) * SPH_GRAVITY_MAGNITUDE, 
            0.0f);
    }
    else {
        std::cout << "ERROR. Unimplemented gravity mode." << std::endl;
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
}

void Particle_System::next_gravity_mode ()
{
    if (this->gravity_mode == GRAVITY_OFF) { this->change_gravity_mode(GRAVITY_NORMAL); }
    else if (this->gravity_mode == GRAVITY_NORMAL) { this->change_gravity_mode(GRAVITY_ROT_90); }
    else if (this->gravity_mode == GRAVITY_ROT_90) { this->change_gravity_mode(GRAVITY_WAVE); }
    else if (this->gravity_mode == GRAVITY_WAVE) { this->change_gravity_mode(GRAVITY_OFF); }
    else {
        std::cout << "ERROR. Unimplemented gravity mode." << std::endl;
    }
}

void Particle_System::change_gravity_mode (Gravity_Mode gravity_mode)
{
    this->gravity_mode = gravity_mode;
    std::cout << "Activated gravity mode '" << to_string(this->gravity_mode) << "'." << std::endl;
}


// ====================================== COLLISION HANDLING ======================================

void Particle_System::resolve_collision (Particle& particle)
{
    // Resolve collision and if a collision happened, reset the position, inverse the velocities component
    // and apply a collision damping.
    // x.
    if (particle.position.x < this->simulation_space->x_min) {
        particle.position.x = this->simulation_space->x_min;
        particle.velocity.x = -particle.velocity.x * SPH_COLLISION_DAMPING;
    }
    else if (particle.position.x > this->simulation_space->x_max) {
        particle.position.x = this->simulation_space->x_max;
        particle.velocity.x = -particle.velocity.x * SPH_COLLISION_DAMPING;
    }
    // y.
    if (particle.position.y < this->simulation_space->y_min) {
        particle.position.y = this->simulation_space->y_min;
        particle.velocity.y = -particle.velocity.y * SPH_COLLISION_DAMPING;
    }
    else if (particle.position.y > this->simulation_space->y_max) {
        particle.position.y = this->simulation_space->y_max;
        particle.velocity.y = -particle.velocity.y * SPH_COLLISION_DAMPING;
    }
    // z.
    if (particle.position.z < this->simulation_space->z_min) {
        particle.position.z = this->simulation_space->z_min;
        particle.velocity.z = -particle.velocity.z * SPH_COLLISION_DAMPING;
    }
    else if (particle.position.z > this->simulation_space->z_max) {
        particle.position.z = this->simulation_space->z_max;
        particle.velocity.z = -particle.velocity.z * SPH_COLLISION_DAMPING;
    }
}


// ====================================== MULTITHREADING ======================================

void Particle_System::parallel_for (void (Particle_System::* function)(unsigned int, unsigned int), int number_of_elements)
{
    // Create threads and execute the desired function in chunks.
    // Calculate the chunk size (it depends whether we operate on the particles vector itself or the spatial grid).
    int chunk_size = number_of_elements / SIMULATION_NUMBER_OF_THREADS;
    std::vector<std::thread> threads;
    threads.reserve(SIMULATION_NUMBER_OF_THREADS);
    // Create the threads.
    for (int i = 0; i < SIMULATION_NUMBER_OF_THREADS; i++) {
        int chunk_start = i * chunk_size;
        int chunk_end = chunk_start + chunk_size - 1;
        // The last chunk goes until the end of the vector.
        if (i == SIMULATION_NUMBER_OF_THREADS - 1) {
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

// ===================================== SPH BRUTE FORCE IMPLEMENTATION ===================================

void Particle_System::calculate_density_pressure_brute_force (unsigned int index_start, unsigned int index_end)
{
    // Calculate the density and the pressure using the SPH method.
    for (int i = index_start; i <= index_end; i++) {
        this->particles[i].density = 0;
        for (int j = 0; j < this->number_of_particles; j++) {
            glm::vec3 distance_vector = this->particles[i].position - this->particles[j].position;
            if (glm::length(distance_vector) < this->sph_kernel_radius) {
                this->particles[i].density += this->kernel_w_poly6(distance_vector);
            }
        }
        this->particles[i].density *= SPH_PARTICLE_MASS;
        this->particles[i].pressure = SPH_GAS_CONSTANT * (this->particles[i].density - SPH_GAS_CONSTANT);
    }
}

void Particle_System::calculate_acceleration_brute_force (unsigned int index_start, unsigned int index_end)
{
    // Calculate the forces for each particle independently.
    glm::vec3 f_external = this->get_gravity_vector();
    for (int i = index_start; i <= index_end; i++) {
        glm::vec3 f_pressure(0.0f);
        glm::vec3 f_viscosity(0.0f);
        glm::vec3 f_surface(0.0f);
        glm::vec3 color_field_normal(0.0f);
        float color_field_laplacian = 0.0f;

        // Calculate the forces based on all particles nearby.
        for (int j = 0; j < this->number_of_particles; j++) {
            if (j == i) continue;
            glm::vec3 distance_vector = this->particles[i].position - this->particles[j].position;
            if (glm::length(distance_vector) < this->sph_kernel_radius) {
                f_pressure += (float)(this->particles[i].pressure / pow(this->particles[i].density, 2) + 
                    this->particles[j].pressure / pow(this->particles[j].density, 2)) *
                    this->kernel_w_spiky_gradient(distance_vector);
                f_viscosity += (this->particles[j].velocity - this->particles[i].velocity) * 
                    this->kernel_w_viscosity_laplacian(distance_vector) / 
                    this->particles[j].density;
                color_field_normal += this->kernel_w_poly6_gradient(distance_vector) / 
                    this->particles[j].density;
                color_field_laplacian += this->kernel_w_poly6_laplacian(distance_vector) / 
                    this->particles[j].density;
            }
        }
        f_pressure *= -SPH_PARTICLE_MASS * this->particles[i].density;
        f_viscosity *= SPH_PARTICLE_MASS * SPH_VISCOSITY;
        color_field_normal *= SPH_PARTICLE_MASS;
        this->particles[i].normal = -1.0f * color_field_normal;
        color_field_laplacian *= SPH_PARTICLE_MASS;
        float color_field_normal_magnitude = glm::length(color_field_normal);
        if (color_field_normal_magnitude > SPH_SURFACE_THRESHOLD) {
            f_surface = -SPH_SURFACE_TENSION * (color_field_normal / color_field_normal_magnitude) * color_field_laplacian;
        }

        // Calculate the acceleration.
        this->particles[i].acceleration = (f_pressure + f_viscosity + f_surface + f_external) / this->particles[i].density;
    }
}

void Particle_System::calculate_verlet_step_brute_force (unsigned int index_start, unsigned int index_end)
{
    // Compute the new position and new velocity using the velocity verlet integration.
    for (int i = index_start; i <= index_end; i++) {
        static float time_step_squared = pow(SPH_SIMULATION_TIME_STEP, 2);
        glm::vec3 new_position = this->particles[i].position + 
            this->particles[i].velocity * SPH_SIMULATION_TIME_STEP + 
            this->particles[i].acceleration * time_step_squared;
        glm::vec3 new_velocity = (new_position - this->particles[i].position) / SPH_SIMULATION_TIME_STEP;
        this->particles[i].position = new_position;
        this->particles[i].velocity = new_velocity;

        // Resolve collision.
        this->resolve_collision(this->particles[i]);
    }
}

void Particle_System::simulate_brute_force ()
{
    // Do it either in sequential mode or in parallel.
    if (this->computation_mode == COMPUTATION_MODE_BRUTE_FORCE) {
        // Calculate the density and the pressure for each particle.
        this->calculate_density_pressure_brute_force(0, this->number_of_particles - 1);
        // Calculate the forces and acceleration.
        this->calculate_acceleration_brute_force(0, this->number_of_particles - 1);
        // Calculate the new positions and apply collision handling.
        this->calculate_verlet_step_brute_force(0, this->number_of_particles - 1);
    }
    else if (this->computation_mode == COMPUTATION_MODE_BRUTE_FORCE_MULTITHREADING) {
        // Calculate the density and the pressure for each particle using multiple threads.
        this->parallel_for(&Particle_System::calculate_density_pressure_brute_force, this->number_of_particles);
        // Calculate the forces and acceleration using multiple threads.
        this->parallel_for(&Particle_System::calculate_acceleration_brute_force, this->number_of_particles);
        // Calculate the new positions and apply collision handling using multiple threads.
        this->parallel_for(&Particle_System::calculate_verlet_step_brute_force, this->number_of_particles);
    }
    else {
        std::cout << "ERROR: Unsupported computation_mode in simulate_brute_force detected." << std::endl;
    }
}


// ====================================== SPH SPATIAL GRID IMPLEMENTATION ======================================

void Particle_System::calculate_number_of_grid_cells ()
{
    this->number_of_cells_x = ceil((this->simulation_space->x_max - this->simulation_space->x_min) / 
        this->sph_kernel_radius);
    this->number_of_cells_y = ceil((this->simulation_space->y_max - this->simulation_space->y_min) / 
        this->sph_kernel_radius);
    this->number_of_cells_z = ceil((this->simulation_space->z_max - this->simulation_space->z_min) / 
        this->sph_kernel_radius);
    this->number_of_cells = this->number_of_cells_x * this->number_of_cells_y * this->number_of_cells_z;
    // Just to make sure that the application does not run a simulation after the change of the
    // simulation space or the kernel radius, clear the spatial grid here.
    this->spatial_grid.clear();
    // Resize the vector for the particles that will be moved.
    this->particles_to_be_moved.clear();
    this->particles_to_be_moved.resize(this->number_of_cells);
    // Resize the mutex vector.
    // Iterate over each element in the vector and assign a newly created std::mutex using std::make_unique.
    this->mutex_spatial_grid.clear();
    this->mutex_spatial_grid.resize(this->number_of_cells);
    for (auto& mutex : this->mutex_spatial_grid) {
        mutex = std::make_unique<std::mutex>();
    }
    this->mutex_to_be_moved.clear();
    this->mutex_to_be_moved.resize(this->number_of_cells);
    for (auto& mutex : this->mutex_to_be_moved) {
        mutex = std::make_unique<std::mutex>();
    }
}

inline int Particle_System::discretize_value (float value)
{
    // The cells of our grid have the edge length of the kernels radius.
    return (int)floor(value / this->sph_kernel_radius);
}

inline int Particle_System::get_grid_key (glm::vec3 position)
{
    // We move the particles position into positive values.
    position = position + this->particle_offset;
    // Check if the position is within the grids volume.
    if ((position.x < 0.0f) || (position.x > this->number_of_cells_x * this->sph_kernel_radius) ||
        (position.y < 0.0f) || (position.y > this->number_of_cells_y * this->sph_kernel_radius) ||
        (position.z < 0.0f) || (position.z > this->number_of_cells_z * this->sph_kernel_radius)) {
            return -1;
    }
    // The spatial grid will also be calculated and manipulated using parallel for loops.
    // In order that all threads have nearly the same amount of workload, the right choice for 
    // the generation of the key is mandatory.
    // For this we assume, that the most common scene is that most of the particles are "on the ground"
    // so all x and z values are present but only a few y values (only the ones near the ground).
    // So we want grids with the same x and z coordinates but different y coordinates next to eachother
    // so that the whole grid is in general evenly distributed.
    return
        (Particle_System::discretize_value(position.y)) +
        (Particle_System::discretize_value(position.x) * this->number_of_cells_y) +
        (Particle_System::discretize_value(position.z) * this->number_of_cells_x * this->number_of_cells_y);
}

std::vector<int> Particle_System::get_neighbor_cells_indices (glm::vec3 position)
{
    std::vector<int> neighbor_cells_indices;
    for (int look_x = -1; look_x <= 1; look_x++) {
            for (int look_y = -1; look_y <= 1; look_y++) {
                for (int look_z = -1; look_z <= 1; look_z++) {
                    // Make sure not to get wrong indices for cells that do not really exist.
                    glm::vec3 look_position = position + glm::vec3(look_x, look_y, look_z) * this->sph_kernel_radius;
                    int grid_key = this->get_grid_key(look_position);
                    if (grid_key >= 0) {
                        neighbor_cells_indices.push_back(grid_key);
                    }
                }
            }
    }
    return neighbor_cells_indices;
}

void Particle_System::generate_spatial_grid (unsigned int index_start, unsigned int index_end)
{
    for (int i = index_start; i <= index_end; i++) {
        // Assign the particle based on its position to a grid cell. Get the index of this cell.
        int grid_key = this->get_grid_key(this->particles[i].position);
        // Lock the spatial grid cell for the insertion of the particle.
        std::unique_lock<std::mutex> lock(*this->mutex_spatial_grid[grid_key]);
        this->spatial_grid[grid_key].push_back(this->particles[i]);
        lock.unlock();
    }
}

void Particle_System::calculate_density_pressure_spatial_grid (unsigned int index_start, unsigned int index_end)
{
    // The index does now not refer to the index in the particles vector but to a grid cell.
    for (int idx_cell = index_start; idx_cell <= index_end; idx_cell++) {
        if (this->spatial_grid[idx_cell].size() == 0) {
            // No particles in this cell.
            continue;
        }
        // We are now in a cell with the cell index idx_cell. Get the neighboring cells.
        std::vector<int> neighboring_cells_indices = this->get_neighbor_cells_indices(this->spatial_grid[idx_cell][0].position);
        // Push also the current cell index into this list. For the density we also need self containment so its ok that the
        // current particle is also in this list.
        neighboring_cells_indices.push_back(idx_cell);
        // For each particle in this cell.
        for (Particle& particle : this->spatial_grid[idx_cell]) {
            particle.density = 0;
            // Look in all neighboring cells (this includes also the current cell).
            for (int idx_neighbor_cell: neighboring_cells_indices) {
                // Look at all the particles in these neighboring cells.
                for (Particle& neighbor : this->spatial_grid[idx_neighbor_cell]) {
                    // If they are near enough, they are used for the calculation.
                    glm::vec3 distance_vector = particle.position - neighbor.position;
                    float distance = glm::length(distance_vector);
                    if (distance < this->sph_kernel_radius) {
                        particle.density += this->kernel_w_poly6(distance_vector);
                    }
                }
            }
            particle.density *= SPH_PARTICLE_MASS;
            particle.pressure = SPH_GAS_CONSTANT * (particle.density - SPH_GAS_CONSTANT);
        }
    }
}

void Particle_System::calculate_acceleration_spatial_grid (unsigned int index_start, unsigned int index_end)
{
    // The index does now not refer to the index in the particles vector but to a grid cell.
    // Calculate the forces for each particle in a cell independently.
    glm::vec3 f_external = this->get_gravity_vector();
    for (int idx_cell = index_start; idx_cell <= index_end; idx_cell++) {
        if (this->spatial_grid[idx_cell].size() == 0) {
            // No particles in this cell.
            continue;
        }
        // We are now in a cell with the cell index idx_cell. Get the neighboring cells.
        std::vector<int> neighboring_cells_indices = this->get_neighbor_cells_indices(this->spatial_grid[idx_cell][0].position);
        // Push also the current cell index into this list. For the density we also need self containment so its ok that the
        // current particle is also in this list.
        neighboring_cells_indices.push_back(idx_cell);
        // For each particle in this cell.
        for (Particle& particle : this->spatial_grid[idx_cell]) {
            glm::vec3 f_pressure(0.0f);
            glm::vec3 f_viscosity(0.0f);
            glm::vec3 f_surface(0.0f);
            glm::vec3 color_field_normal(0.0f);
            float color_field_laplacian = 0.0f;
            // Look in all neighboring cells (this includes also the current cell).
            for (int idx_neighbor_cell: neighboring_cells_indices) {
                // Look at all the particles in these neighboring cells.
                for (Particle& neighbor : this->spatial_grid[idx_neighbor_cell]) {
                    // Do not use one particle on itself.
                    if (&particle == &neighbor) continue;
                    // If they are near enough, they are used for the calculation.
                    glm::vec3 distance_vector = particle.position - neighbor.position;
                    float distance = glm::length(distance_vector);
                    if (distance < this->sph_kernel_radius) {
                        f_pressure += (float)(particle.pressure / pow(particle.density, 2) + 
                            neighbor.pressure / pow(neighbor.density, 2)) *
                            this->kernel_w_spiky_gradient(distance_vector);
                        f_viscosity += (neighbor.velocity - particle.velocity) * 
                            this->kernel_w_viscosity_laplacian(distance_vector) / 
                            neighbor.density;
                        color_field_normal += this->kernel_w_poly6_gradient(distance_vector) / 
                            neighbor.density;
                        color_field_laplacian += this->kernel_w_poly6_laplacian(distance_vector) / 
                            neighbor.density;
                    }
                }
            }
            f_pressure *= -SPH_PARTICLE_MASS * particle.density;
            f_viscosity *= SPH_PARTICLE_MASS * SPH_VISCOSITY;
            color_field_normal *= SPH_PARTICLE_MASS;
            particle.normal = -1.0f * color_field_normal;
            color_field_laplacian *= SPH_PARTICLE_MASS;
            float color_field_normal_magnitude = glm::length(color_field_normal);
            if (color_field_normal_magnitude > SPH_SURFACE_THRESHOLD) {
                f_surface = -SPH_SURFACE_TENSION * (color_field_normal / color_field_normal_magnitude) * color_field_laplacian;
            }

            // Calculate the acceleration.
            particle.acceleration = (f_pressure + f_viscosity + f_surface + f_external) / particle.density;
        }
    }
}

void Particle_System::calculate_verlet_step_spatial_grid (unsigned int index_start, unsigned int index_end)
{
    // The index does now not refer to the index in the particles vector but to a grid cell.
    for (int idx_cell = index_start; idx_cell <= index_end; idx_cell++) {
        // Calculate for each particle in this cell the new position and velocity and resolve collision.
        for (Particle& particle : this->spatial_grid[idx_cell]) {
            static float time_step_squared = pow(SPH_SIMULATION_TIME_STEP, 2);
            glm::vec3 new_position = particle.position + 
                particle.velocity * SPH_SIMULATION_TIME_STEP + 
                particle.acceleration * time_step_squared;
            glm::vec3 new_velocity = (new_position - particle.position) / SPH_SIMULATION_TIME_STEP;
            particle.position = new_position;
            particle.velocity = new_velocity;

            // Resolve collision.
            this->resolve_collision(particle);
        }
    }
}

void Particle_System::get_updated_cell_index (unsigned int index_start, unsigned int index_end)
{
    // Go through the cells and then through their particles. Calculate for each particle the grid cell index
    // based on its (now updated) position. If it is still the same then keep it in this cell, if it is another
    // cell index, then move it into a temporary container and remove it from the previous cell.
    for (int idx_cell = index_start; idx_cell <= index_end; idx_cell++) {
        std::vector<Particle>& cell_particles = this->spatial_grid[idx_cell];
        for (auto it = cell_particles.begin(); it != cell_particles.end(); ) {
            Particle& particle = *it;
            int new_cell_index = this->get_grid_key(particle.position);
            if (new_cell_index != idx_cell) {
                // We need to move the particle. Put it into the temporary container 
                // and remove it from the cell.
                // Make sure to lock it (maybe another thread also wants to move particles there).
                std::unique_lock<std::mutex> lock(*this->mutex_to_be_moved[new_cell_index]);
                this->particles_to_be_moved[new_cell_index].push_back(particle);
                lock.unlock();
                // Remove it and get the new iterator (points to the following element).
                it = cell_particles.erase(it);
            } else {
                // The particle will be kept in this cell, so look at the next particle.
                it++;
            }
        }
    }
}

void Particle_System::apply_updated_cell_index (unsigned int index_start, unsigned int index_end)
{
    // The function get_updated_cell_index moved particles that will no longer be part of their original cell
    // into a temporary container (a container that exists for all cells). Now combine the cell with its corresponding
    // container and empty the container.
    for (int idx_cell = index_start; idx_cell <= index_end; idx_cell++) {
        this->spatial_grid[idx_cell].insert(
            this->spatial_grid[idx_cell].end(), 
            std::make_move_iterator(this->particles_to_be_moved[idx_cell].begin()), 
            std::make_move_iterator(this->particles_to_be_moved[idx_cell].end())
        );
        this->particles_to_be_moved[idx_cell].clear();
    }
}

void Particle_System::update_particle_vector ()
{
    this->particles.clear();
    for (const auto& cell : this->spatial_grid) {
        this->particles.insert(particles.end(), cell.begin(), cell.end());
    }
}

void Particle_System::simulate_spatial_grid ()
{
    // Create the spatial grid if it does not exist already.
    if (this->spatial_grid.size() == 0) {
        this->spatial_grid.resize(this->number_of_cells);
        this->parallel_for(&Particle_System::generate_spatial_grid, this->number_of_particles);
    }
    // Calculate the density and the pressure for each particle using multiple threads.
    this->parallel_for(&Particle_System::calculate_density_pressure_spatial_grid, this->number_of_cells);
    // Calculate the forces and acceleration using multiple threads.
    this->parallel_for(&Particle_System::calculate_acceleration_spatial_grid, this->number_of_cells);
    // Calculate the new positions and apply collision handling using multiple threads.
    this->parallel_for(&Particle_System::calculate_verlet_step_spatial_grid, this->number_of_cells);
    // Get the updated particles vector.
    // NOTE: Maybe also try to apply the glBufferSubData call from the different cells using the offset of 
    // the previous ones?
    this->update_particle_vector();
    // Either clear the whole spatial grid in order to be regenerated in the next step or update it.
    if (this->computation_mode == COMPUTATION_MODE_SPATIAL_GRID_CLEAR_MODE) {
        this->spatial_grid.clear();
    }
    else if (this->computation_mode == COMPUTATION_MODE_SPATIAL_GRID_UPDATE_MODE) {
        // Iterate over all cells and check if the contained particles should still be in this cell.
        // If not get them out and save them for now in another container.
        this->parallel_for(&Particle_System::get_updated_cell_index, this->number_of_cells);
        // Move the created container to the corresponding grid cells.
        this->parallel_for(&Particle_System::apply_updated_cell_index, this->number_of_cells);
    }
    else {
        std::cout << "ERROR: Unsupported computation_mode in simulate_spatial_grid detected." << std::endl;
    }

}



// ====================================== GENERAL SIMULATION FUNCTION ======================================

void Particle_System::simulate ()
{
    this->simulation_step++;
    if ((this->computation_mode == COMPUTATION_MODE_BRUTE_FORCE) || 
        (this->computation_mode == COMPUTATION_MODE_BRUTE_FORCE_MULTITHREADING)) {
        this->simulate_brute_force();
    }
    else if ((this->computation_mode == COMPUTATION_MODE_SPATIAL_GRID_CLEAR_MODE) ||
        (this->computation_mode == COMPUTATION_MODE_SPATIAL_GRID_UPDATE_MODE)) {
        this->simulate_spatial_grid();
    }
}


// ====================================== COMPUTATTION MODE ======================================

void Particle_System::next_computation_mode ()
{
    if (this->computation_mode == COMPUTATION_MODE_BRUTE_FORCE) { 
        this->change_computation_mode(COMPUTATION_MODE_BRUTE_FORCE_MULTITHREADING); 
    }
    else if (this->computation_mode == COMPUTATION_MODE_BRUTE_FORCE_MULTITHREADING) { 
        this->change_computation_mode(COMPUTATION_MODE_SPATIAL_GRID_CLEAR_MODE); 
    }
    else if (this->computation_mode == COMPUTATION_MODE_SPATIAL_GRID_CLEAR_MODE) { 
        this->change_computation_mode(COMPUTATION_MODE_SPATIAL_GRID_UPDATE_MODE); 
    }
    else if (this->computation_mode == COMPUTATION_MODE_SPATIAL_GRID_UPDATE_MODE) { 
        this->change_computation_mode(COMPUTATION_MODE_BRUTE_FORCE); 
    }
    else {
        std::cout << "ERROR. Unimplemented computation mode." << std::endl;
    }
}

void Particle_System::change_computation_mode (Computation_Mode computation_mode)
{
    // If the previous computation mode was the "UPDATE" version of the spatial grid, 
    // also clear the spatial grid so that if the "UPDATE" mode gets activated again,
    // the spatial grid will be regenerated with the up-to-date data.
    if (this->computation_mode == COMPUTATION_MODE_SPATIAL_GRID_UPDATE_MODE) {
        this->spatial_grid.clear();
    }
    this->computation_mode = computation_mode;
    std::cout << "Activated computation mode '" << to_string(this->computation_mode) << "'." << std::endl;
}


// ====================================== RENDERING RELATED FUNCTIONS ======================================

void Particle_System::draw (bool unbind)
{
    // Update the particles data in the vertex buffer object.
    GLCall( glBindBuffer(GL_ARRAY_BUFFER, this->vertex_buffer_object) );
    GLCall( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle) * this->number_of_particles, &this->particles[0]) );
    GLCall( glBindBuffer(GL_ARRAY_BUFFER, 0) );
    // Draw the particles using the vertex array object.
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