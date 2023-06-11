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
    this->calculate_kernel_radius();
    this->reset_fluid_attributes();
    this->number_of_cells = 0;
    this->gravity_mode = GRAVITY_NORMAL;
    this->computation_mode = COMPUTATION_MODE_SPATIAL_GRID;
    this->collision_method = COLLISION_METHOD_REFLEXION;
    this->number_of_threads = SIMULATION_NUMBER_OF_THREADS;
}


// ====================================== INITIALIZATION FUNCTIONS ======================================

void Particle_System::generate_initial_particles (std::vector<Cuboid>& cuboids)
{
    // Free the memory if it was used before.
    this->particles.clear();
    // Fill all cuboids with particles.
    for (int i = 0; i < cuboids.size(); i++) {
        cuboids.at(i).fill_with_particles(this->particle_initial_distance, this->particles);
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
    GLCall( glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * this->number_of_particles, &this->particles.at(0), GL_STATIC_DRAW) );

    // Copy the indices into the index buffer object.
    GLCall( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->index_buffer_object) );
    this->particle_indices.clear();
    for (unsigned int i = 0; i < this->number_of_particles; i++) {
        this->particle_indices.push_back(i);
    }
    GLCall( glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * this->number_of_particles, &this->particle_indices.at(0), GL_STATIC_DRAW) );

    // Describe the vertex buffer layout of a particle.
    describe_particle_memory_layout();

    // Unbind.
    GLCall( glBindBuffer(GL_ARRAY_BUFFER, 0) );
    GLCall( glBindVertexArray(0) );

    // Reset the simulation time.
    this->simulation_step = 0;
}

void Particle_System::calculate_kernel_radius ()
{
    this->sph_kernel_radius = 4 * this->particle_initial_distance;
    // The kernels radius changed, so recalculate the coefficients and other 
    // helper variables for the kernel functions.
    this->kernel_radius_squared = pow(this->sph_kernel_radius, 2);;
    this->coefficient_kernel_w_poly6 = 315.0f / (64.0f * M_PI * pow(this->sph_kernel_radius, 9));;
    this->coefficient_kernel_w_poly6_gradient = -945.0f / (32.0f * M_PI * pow(this->sph_kernel_radius, 9));
    this->coefficient_kernel_w_poly6_laplacian = 945.0f / (8.0f * M_PI * pow(this->sph_kernel_radius, 9));
    this->coefficient_kernel_w_spiky_gradient = -45.0f / (M_PI * pow(this->sph_kernel_radius, 6));
    this->coefficient_kernel_w_viscosity_laplacian = 45.0f / (M_PI * pow(this->sph_kernel_radius, 6));
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

void Particle_System::reset_fluid_attributes ()
{
    this->sph_particle_mass = SPH_PARTICLE_MASS;
    this->sph_rest_density = SPH_REST_DENSITY;
    this->sph_gas_constant = SPH_GAS_CONSTANT;
    this->sph_viscosity = SPH_VISCOSITY;
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
        this->calculate_kernel_radius();
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
        this->calculate_kernel_radius();
        this->calculate_number_of_grid_cells();
        return true;
    }
}


// ====================================== SPH KERNEL FUNCTIONS ======================================

inline float Particle_System::kernel_w_poly6 (glm::vec3 distance_vector)
{
    float distance_squared = glm::dot(distance_vector, distance_vector);
    return this->coefficient_kernel_w_poly6 * (float)pow(this->kernel_radius_squared - distance_squared, 3);
}

inline glm::vec3 Particle_System::kernel_w_poly6_gradient (glm::vec3 distance_vector)
{
    float distance_squared = glm::dot(distance_vector, distance_vector);
    return (this->coefficient_kernel_w_poly6_gradient * 
        (float)pow(this->kernel_radius_squared - distance_squared, 2)) * distance_vector;
}

inline float Particle_System::kernel_w_poly6_laplacian (glm::vec3 distance_vector)
{
    float distance_squared = glm::dot(distance_vector, distance_vector);
    return this->coefficient_kernel_w_poly6_laplacian * (this->kernel_radius_squared - distance_squared) * 
        (distance_squared - 0.75f * (this->kernel_radius_squared - distance_squared));
}

inline glm::vec3 Particle_System::kernel_w_spiky_gradient (glm::vec3 distance_vector)
{
    float distance = glm::length(distance_vector);
    if (distance > 0.0f) {
        return this->coefficient_kernel_w_spiky_gradient * (float)pow(this->sph_kernel_radius - distance, 2) * 
            (distance_vector / distance);
    }
    else {
        return glm::vec3(0.0f);
    }
}

inline float Particle_System::kernel_w_viscosity_laplacian (glm::vec3 distance_vector)
{
    float distance = glm::length(distance_vector);
    return this->coefficient_kernel_w_viscosity_laplacian * (this->sph_kernel_radius - distance);
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

void Particle_System::resolve_collision_relfexion_method (Particle& particle)
{
    // Resolve collision and if a collision happened, reset the position, inverse the velocities component
    // and apply a collision damping.
    // x.
    if (particle.position.x < this->simulation_space->x_min) {
        particle.position.x = this->simulation_space->x_min;
        particle.velocity.x = -particle.velocity.x * SPH_COLLISION_DAMPING_REFLEXION_METHOD;
    }
    else if (particle.position.x > this->simulation_space->x_max) {
        particle.position.x = this->simulation_space->x_max;
        particle.velocity.x = -particle.velocity.x * SPH_COLLISION_DAMPING_REFLEXION_METHOD;
    }
    // y.
    if (particle.position.y < this->simulation_space->y_min) {
        particle.position.y = this->simulation_space->y_min;
        particle.velocity.y = -particle.velocity.y * SPH_COLLISION_DAMPING_REFLEXION_METHOD;
    }
    else if (particle.position.y > this->simulation_space->y_max) {
        particle.position.y = this->simulation_space->y_max;
        particle.velocity.y = -particle.velocity.y * SPH_COLLISION_DAMPING_REFLEXION_METHOD;
    }
    // z.
    if (particle.position.z < this->simulation_space->z_min) {
        particle.position.z = this->simulation_space->z_min;
        particle.velocity.z = -particle.velocity.z * SPH_COLLISION_DAMPING_REFLEXION_METHOD;
    }
    else if (particle.position.z > this->simulation_space->z_max) {
        particle.position.z = this->simulation_space->z_max;
        particle.velocity.z = -particle.velocity.z * SPH_COLLISION_DAMPING_REFLEXION_METHOD;
    }
}

glm::vec3 Particle_System::resolve_collision_force_method (Particle particle)
{
    // This method applies an force to a particle if its to near the border.
    // It is basically a spring damper system.
    // Note that this method is only allowed in the brute force mode and not in the spatial grid mode.
    // Using this collision mode, the particles can swing a little bit outside the box. With a fixed grid
    // over the simulation space, these particles would be outside the grid.
    glm::vec3 f_collision = glm::vec3(0.0f);
    float distance;
    // x-min border.
    distance = (this->simulation_space->x_min + SPH_COLLISION_DISTANCE_TOLERANCE) - particle.position.x;
    if (distance > 0.0f) {
        f_collision.x += SPH_COLLISION_WALL_SPRING_CONSTANT * distance;
        f_collision.x += SPH_COLLISION_DAMPING_FORCE_METHOD * particle.velocity.x;
    }
    // x-max border.
    distance = particle.position.x - (this->simulation_space->x_max - SPH_COLLISION_DISTANCE_TOLERANCE);
    if (distance > 0.0f) {
        f_collision.x -= SPH_COLLISION_WALL_SPRING_CONSTANT * distance;
        f_collision.x -= SPH_COLLISION_DAMPING_FORCE_METHOD * particle.velocity.x;
    }
    // y-min border.
    distance = (this->simulation_space->y_min + SPH_COLLISION_DISTANCE_TOLERANCE) - particle.position.y;
    if (distance > 0.0f) {
        f_collision.y += SPH_COLLISION_WALL_SPRING_CONSTANT * distance;
        f_collision.y += SPH_COLLISION_DAMPING_FORCE_METHOD * particle.velocity.y;
    }
    // y-max border.
    distance = particle.position.y - (this->simulation_space->y_max - SPH_COLLISION_DISTANCE_TOLERANCE);
    if (distance > 0.0f) {
        f_collision.y -= SPH_COLLISION_WALL_SPRING_CONSTANT * distance;
        f_collision.y -= SPH_COLLISION_DAMPING_FORCE_METHOD * particle.velocity.y;
    }
    // z-min border.
    distance = (this->simulation_space->z_min + SPH_COLLISION_DISTANCE_TOLERANCE) - particle.position.z;
    if (distance > 0.0f) {
        f_collision.z += SPH_COLLISION_WALL_SPRING_CONSTANT * distance;
        f_collision.z += SPH_COLLISION_DAMPING_FORCE_METHOD * particle.velocity.z;
    }
    // z-max border.
    distance = particle.position.z - (this->simulation_space->z_max - SPH_COLLISION_DISTANCE_TOLERANCE);
    if (distance > 0.0f) {
        f_collision.z -= SPH_COLLISION_WALL_SPRING_CONSTANT * distance;
        f_collision.z -= SPH_COLLISION_DAMPING_FORCE_METHOD * particle.velocity.z;
    }
    return f_collision;
}


// ====================================== MULTITHREADING ======================================

void Particle_System::parallel_for (void (Particle_System::* function)(unsigned int, unsigned int), int number_of_elements)
{
    // Create threads and execute the desired function in chunks.
    // Calculate the chunk size (it depends whether we operate on the particles vector itself or the spatial grid).
    if (this->number_of_threads == 1) {
        // Just execute the function if only one thread is desired.
        (this->*function)(0, number_of_elements - 1);
        return;
    }
    int chunk_size = number_of_elements / this->number_of_threads;
    std::vector<std::thread> threads;
    threads.reserve(this->number_of_threads);
    // Create the threads.
    for (int i = 0; i < this->number_of_threads; i++) {
        int chunk_start = i * chunk_size;
        int chunk_end = chunk_start + chunk_size - 1;
        // The last chunk goes until the end of the vector.
        if (i == this->number_of_threads - 1) {
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

void Particle_System::parallel_for_grid (void (Particle_System::* function)(unsigned int, unsigned int))
{
    // The parallel_for_grid function works not on a vector of particles but on the grid. 
    // On the grid we do not know if the particles are evenly distributed over the whole simulation space
    // (they are most likely not), so we have to dynamically assign the chunks based on the number of particles
    // in the grid cells.
    // Create threads and execute the desired function in chunks.
    // Calculate the chunk size not on the number of grid cells (evenly), but on the number of particles.
    if (this->number_of_threads == 1) {
        // Just execute the function if only one thread is desired.
        (this->*function)(0, this->number_of_cells - 1);
        return;
    }
    int evenly_distributed_number_of_particles = this->number_of_particles / this->number_of_threads;
    std::vector<std::thread> threads;
    threads.reserve(this->number_of_threads);
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
        if (threads.size() == (this->number_of_threads - 1)) {
            threads.emplace_back(function, this, chunk_start, this->number_of_cells - 1);
            break;
        }
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
        this->particles.at(i).density = 0;
        for (int j = 0; j < this->number_of_particles; j++) {
            glm::vec3 distance_vector = this->particles.at(i).position - this->particles.at(j).position;
            if (glm::length(distance_vector) < this->sph_kernel_radius) {
                this->particles.at(i).density += this->kernel_w_poly6(distance_vector);
            }
        }
        this->particles.at(i).density *= this->sph_particle_mass;
        this->particles.at(i).pressure = this->sph_gas_constant * (this->particles.at(i).density - this->sph_rest_density);
    }
}

void Particle_System::calculate_acceleration_brute_force (unsigned int index_start, unsigned int index_end)
{
    // Calculate the forces for each particle independently.
    glm::vec3 f_external = this->get_gravity_vector();
    for (int i = index_start; i <= index_end; i++) {
        glm::vec3 f_pressure(0.0f);
        glm::vec3 f_viscosity(0.0f);

        // Calculate the forces based on all particles nearby.
        for (int j = 0; j < this->number_of_particles; j++) {
            if (j == i) continue;
            glm::vec3 distance_vector = this->particles.at(i).position - this->particles.at(j).position;
            if (glm::length(distance_vector) < this->sph_kernel_radius) {
                f_pressure += ((this->particles.at(i).pressure + this->particles.at(j).pressure) / (2 * this->particles.at(j).density)) *
                            this->kernel_w_spiky_gradient(distance_vector);
                f_viscosity += (this->particles.at(j).velocity - this->particles.at(i).velocity) * 
                    this->kernel_w_viscosity_laplacian(distance_vector) / 
                    this->particles.at(j).density;
            }
        }
        f_pressure *= -this->sph_particle_mass;
        f_viscosity *= this->sph_particle_mass * this->sph_viscosity;

        // Resolve collision.
        glm::vec3 f_collision = glm::vec3(0.0f);
        if (this->collision_method == COLLISION_METHOD_FORCE) {
            f_collision = this->resolve_collision_force_method(this->particles.at(i));
        }

        // Calculate the acceleration.
        this->particles.at(i).acceleration = (f_pressure + f_viscosity + f_external + f_collision) / this->particles.at(i).density;
    }
}

void Particle_System::calculate_verlet_step_brute_force (unsigned int index_start, unsigned int index_end)
{
    // Compute the new position and new velocity using the velocity verlet integration.
    for (int i = index_start; i <= index_end; i++) {
        float time_step_squared = pow(SPH_SIMULATION_TIME_STEP, 2);
        glm::vec3 new_position = this->particles.at(i).position + 
            this->particles.at(i).velocity * SPH_SIMULATION_TIME_STEP + 
            this->particles.at(i).acceleration * time_step_squared;
        glm::vec3 new_velocity = (new_position - this->particles.at(i).position) / SPH_SIMULATION_TIME_STEP;
        this->particles.at(i).position = new_position;
        this->particles.at(i).velocity = new_velocity;

        // Resolve collision.
        if (this->collision_method == COLLISION_METHOD_REFLEXION) {
            this->resolve_collision_relfexion_method(this->particles.at(i));
        }
    }
}

void Particle_System::simulate_brute_force ()
{
    // Calculate the density and the pressure for each particle.
    this->parallel_for(&Particle_System::calculate_density_pressure_brute_force, this->number_of_particles);
    // Calculate the forces and acceleration.
    this->parallel_for(&Particle_System::calculate_acceleration_brute_force, this->number_of_particles);
    // Calculate the new positions and velocities.
    this->parallel_for(&Particle_System::calculate_verlet_step_brute_force, this->number_of_particles);
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
    // Resize the mutex vector.
    // Iterate over each element in the vector and assign a newly created std::mutex using std::make_unique.
    this->mutex_spatial_grid.clear();
    this->mutex_spatial_grid.resize(this->number_of_cells);
    for (auto& mutex : this->mutex_spatial_grid) {
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
    return
        (Particle_System::discretize_value(position.x)) +
        (Particle_System::discretize_value(position.y) * this->number_of_cells_x) +
        (Particle_System::discretize_value(position.z) * this->number_of_cells_x * this->number_of_cells_y);
}

std::vector<int> Particle_System::get_neighbor_cells_indices (glm::vec3 position)
{
    std::vector<int> neighbor_cells_indices;
    for (int look_x = -1; look_x <= 1; look_x++) {
            for (int look_y = -1; look_y <= 1; look_y++) {
                for (int look_z = -1; look_z <= 1; look_z++) {
                    // Do not include the cell of the position itself, only neighbors.
                    if ((look_x == 0) && (look_y == 0) && (look_z == 0)) {
                        continue;
                    }
                    glm::vec3 look_position = position + glm::vec3(look_x, look_y, look_z) * this->sph_kernel_radius;
                    int grid_key = this->get_grid_key(look_position);
                    // Make sure not to get wrong indices for cells that do not really exist.
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
        int grid_key = this->get_grid_key(this->particles.at(i).position);
        // Lock the spatial grid cell for the insertion of the particle.
        std::unique_lock<std::mutex> lock(*this->mutex_spatial_grid.at(grid_key));
        this->spatial_grid.at(grid_key).push_back(this->particles.at(i));
        lock.unlock();
    }
}

void Particle_System::calculate_density_pressure_spatial_grid (unsigned int index_start, unsigned int index_end)
{
    // The index does now not refer to the index in the particles vector but to a grid cell.
    for (int idx_cell = index_start; idx_cell <= index_end; idx_cell++) {
        if (this->spatial_grid.at(idx_cell).size() == 0) {
            // No particles in this cell.
            continue;
        }
        // We are now in a cell with the cell index idx_cell. Get the neighboring cells.
        std::vector<int> neighboring_cells_indices = this->get_neighbor_cells_indices(this->spatial_grid.at(idx_cell).at(0).position);
        // Push also the current cell index into this list. For the density we also need self containment so its ok that the
        // current particle is also in this list.
        neighboring_cells_indices.push_back(idx_cell);
        // For each particle in this cell.
        for (Particle& particle : this->spatial_grid.at(idx_cell)) {
            particle.density = 0.0f;
            // Look in all neighboring cells (this includes also the current cell).
            for (int idx_neighbor_cell: neighboring_cells_indices) {
                // Look at all the particles in these neighboring cells.
                for (Particle& neighbor : this->spatial_grid.at(idx_neighbor_cell)) {
                    // If they are near enough, they are used for the calculation.
                    glm::vec3 distance_vector = particle.position - neighbor.position;
                    float distance = glm::length(distance_vector);
                    if (distance < this->sph_kernel_radius) {
                        particle.density += this->kernel_w_poly6(distance_vector);
                    }
                }
            }
            particle.density *= this->sph_particle_mass;
            particle.pressure = this->sph_gas_constant * (particle.density - this->sph_rest_density);
        }
    }
}

void Particle_System::calculate_acceleration_spatial_grid (unsigned int index_start, unsigned int index_end)
{
    // The index does now not refer to the index in the particles vector but to a grid cell.
    // Calculate the forces for each particle in a cell independently.
    glm::vec3 f_external = this->get_gravity_vector();
    for (int idx_cell = index_start; idx_cell <= index_end; idx_cell++) {
        if (this->spatial_grid.at(idx_cell).size() == 0) {
            // No particles in this cell.
            continue;
        }
        // We are now in a cell with the cell index idx_cell. Get the neighboring cells.
        std::vector<int> neighboring_cells_indices = this->get_neighbor_cells_indices(this->spatial_grid.at(idx_cell).at(0).position);
        // Push also the current cell index into this list. For the density we also need self containment so its ok that the
        // current particle is also in this list.
        neighboring_cells_indices.push_back(idx_cell);
        // For each particle in this cell.
        for (Particle& particle : this->spatial_grid.at(idx_cell)) {
            glm::vec3 f_pressure(0.0f);
            glm::vec3 f_viscosity(0.0f);
            // Look in all neighboring cells (this includes also the current cell).
            for (int idx_neighbor_cell: neighboring_cells_indices) {
                // Look at all the particles in these neighboring cells.
                for (Particle& neighbor : this->spatial_grid.at(idx_neighbor_cell)) {
                    // Do not use one particle on itself.
                    if (&particle == &neighbor) continue;
                    // If they are near enough, they are used for the calculation.
                    glm::vec3 distance_vector = particle.position - neighbor.position;
                    float distance = glm::length(distance_vector);
                    if (distance < this->sph_kernel_radius) {
                        f_pressure += ((particle.pressure + neighbor.pressure) / (2 * neighbor.density)) *
                            this->kernel_w_spiky_gradient(distance_vector);
                        f_viscosity += (neighbor.velocity - particle.velocity) * 
                            this->kernel_w_viscosity_laplacian(distance_vector) / 
                            neighbor.density;
                    }
                }
            }
            f_pressure *= -this->sph_particle_mass;
            f_viscosity *= this->sph_particle_mass * this->sph_viscosity;

            // Resolve collision.
            glm::vec3 f_collision = glm::vec3(0.0f);
            if (this->collision_method == COLLISION_METHOD_FORCE) {
                f_collision = this->resolve_collision_force_method(particle);
            }

            // Calculate the acceleration.
            particle.acceleration = (f_pressure + f_viscosity + f_external + f_collision) / particle.density;
        }
    }
}

void Particle_System::calculate_verlet_step_spatial_grid (unsigned int index_start, unsigned int index_end)
{
    // The index does now not refer to the index in the particles vector but to a grid cell.
    for (int idx_cell = index_start; idx_cell <= index_end; idx_cell++) {
        // Calculate for each particle in this cell the new position and velocity and resolve collision.
        for (Particle& particle : this->spatial_grid.at(idx_cell)) {
            float time_step_squared = pow(SPH_SIMULATION_TIME_STEP, 2);
            glm::vec3 new_position = particle.position + 
                particle.velocity * SPH_SIMULATION_TIME_STEP + 
                particle.acceleration * time_step_squared;
            glm::vec3 new_velocity = (new_position - particle.position) / SPH_SIMULATION_TIME_STEP;
            particle.position = new_position;
            particle.velocity = new_velocity;

            // Resolve collision.
            if (this->collision_method == COLLISION_METHOD_REFLEXION) {
                this->resolve_collision_relfexion_method(particle);
            }
        }
    }
}

void Particle_System::update_particle_vector ()
{
    this->particles.clear();
    this->particles.reserve(this->number_of_particles);
    for (const auto& cell : this->spatial_grid) {
        this->particles.insert(particles.end(), cell.begin(), cell.end());
    }
}

void Particle_System::simulate_spatial_grid ()
{
    // Create the spatial grid.
    this->spatial_grid.clear();
    this->spatial_grid.resize(this->number_of_cells);
    this->parallel_for(&Particle_System::generate_spatial_grid, this->number_of_particles);
    // Calculate the density and the pressure for each particle using multiple threads.
    this->parallel_for_grid(&Particle_System::calculate_density_pressure_spatial_grid);
    // Calculate the forces and acceleration using multiple threads.
    this->parallel_for_grid(&Particle_System::calculate_acceleration_spatial_grid);
    // Calculate the new positions and apply collision handling using multiple threads.
    this->parallel_for_grid(&Particle_System::calculate_verlet_step_spatial_grid);
    // Get the updated particles vector. We operated until here on the grid.
    // Another solution was to update the grid itself (so iterate over all grid cells and over
    // their particles and check if a particle has to be moved to another cell after this step).
    // This was also implemented and compared to the clear-and-generate-new-method we use now it
    // had no benefit in execution time. The last commit the update-grid-method was still implemented
    // is "f1ab3e1".
    this->update_particle_vector();
}



// ====================================== GENERAL SIMULATION FUNCTION ======================================

void Particle_System::simulate ()
{
    // Next simulation step (we need this for some gravity modes).
    this->simulation_step++;
    // Simulate depending on the selected computation mode.
    if (this->computation_mode == COMPUTATION_MODE_BRUTE_FORCE) {
        this->simulate_brute_force();
    }
    else if (this->computation_mode == COMPUTATION_MODE_SPATIAL_GRID) {
        this->simulate_spatial_grid();
    }
}


// ====================================== COMPUTATTION MODE ======================================

void Particle_System::next_computation_mode ()
{
    if (this->computation_mode == COMPUTATION_MODE_BRUTE_FORCE) { 
        this->change_computation_mode(COMPUTATION_MODE_SPATIAL_GRID); 
    }
    else if (this->computation_mode == COMPUTATION_MODE_SPATIAL_GRID) { 
        this->change_computation_mode(COMPUTATION_MODE_BRUTE_FORCE); 
    }
    else {
        std::cout << "ERROR. Unimplemented computation mode." << std::endl;
    }
}

void Particle_System::change_computation_mode (Computation_Mode computation_mode)
{
    if (computation_mode == this->computation_mode) {
        return;
    }
    this->computation_mode = computation_mode;
    std::cout << "Activated computation mode '" << to_string(this->computation_mode) << "'." << std::endl;
}


// ====================================== RENDERING RELATED FUNCTIONS ======================================

void Particle_System::draw (bool unbind)
{
    // Update the particles data in the vertex buffer object.
    GLCall( glBindBuffer(GL_ARRAY_BUFFER, this->vertex_buffer_object) );
    GLCall( glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle) * this->number_of_particles, &this->particles.at(0)) );
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