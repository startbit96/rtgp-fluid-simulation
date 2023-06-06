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
    this->sph_kernel_radius = 2 * this->particle_initial_distance;
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

void Particle_System::set_simulation_space (Cuboid* simulation_space)
{
    // Calculate number of cells and offset for the hash formula.
    int number_of_cells_x = ceil((simulation_space->x_max - simulation_space->x_min) / SPH_KERNEL_RADIUS);
    int number_of_cells_y = ceil((simulation_space->y_max - simulation_space->y_min) / SPH_KERNEL_RADIUS);
    int number_of_cells_z = ceil((simulation_space->z_max - simulation_space->z_min) / SPH_KERNEL_RADIUS);
    this->number_of_cells = number_of_cells_x * number_of_cells_y * number_of_cells_z;
    this->hash_offset = glm::vec3(simulation_space->x_min, simulation_space->y_min, simulation_space->z_min);
    // We need a pointer to the simulation space for resolving the collision.
    this->simulation_space = simulation_space;
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
        this->sph_kernel_radius = 2 * this->particle_initial_distance;
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
        this->sph_kernel_radius = 2 * this->particle_initial_distance;
        return true;
    }
}

inline int Particle_System::discretize_value (float value)
{
    return (int)floor(value / this->sph_kernel_radius);
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

Particle Particle_System::resolve_collision (Particle particle)
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
    return particle;
}

void Particle_System::simulate_spatial_hash_grid_old ()
{
    // Calculate the spatial grid. Therefore clear it first.
    this->spatial_hash_grid.clear();
    // Now insert every particle based on its position.
    for (int i = 0; i < this->number_of_particles; i++) {
        this->spatial_hash_grid.insert(std::make_pair(
            Particle_System::hash(this->particles[i].position), 
            this->particles[i]
        ));
    }

    // Find the neighbors of each particle.
    // Clear the vector of neighbor lists and announce the size of the vector.
    this->neighbor_list.clear();
    this->neighbor_list.resize(this->number_of_particles);
    // Mueller et al. suggested to store the neighbouring particles on the grid structure
    // itself rather than as references, so that due to memory locality the hit cache rate 
    // will increase significantly.
    // The problem is, that if we save them directly there, the density and pressure of the 
    // particle are the old once from the last simulation step since the density is not 
    // calculated yet.
    // Therefore can not simply use the created neighbor vectors for the calculation, since these
    // are based on the grid which contains copies of the particles. These copies have not 
    // the updated density and pressure value. Therefore first calculate the density and 
    // pressure values and then iterate again over the grid and create the neighbor vectors.
    for (int i = 0; i < this->number_of_particles; i++) {
        // Look for all particles into the neighbouring 26 cells and calculate pressure and density.
        this->particles[i].density = 0;
        this->particles[i].density += this->kernel_w_poly6(glm::vec3(0.0f));
        for (int look_x = -1; look_x <= 1; look_x++) {
            for (int look_y = -1; look_y <= 1; look_y++) {
                for (int look_z = -1; look_z <= 1; look_z++) {
                    glm::vec3 look_position = this->particles[i].position + glm::vec3(look_x, look_y, look_z) * this->sph_kernel_radius;
                    int look_key = this->hash(look_position);
                    auto range = this->spatial_hash_grid.equal_range(look_key);
                    for (auto it = range.first; it != range.second; ++it) {
                        // If the distance to this particle is less than the kernel radius, we need this one.
                        if (glm::distance(this->particles[i].position, it->second.position) < this->sph_kernel_radius) {
                            this->particles[i].density += this->kernel_w_poly6(it->second.position - this->particles[i].position);
                        }
                    }
                }
            }
        }
        this->particles[i].density *= SPH_PARTICLE_MASS;
        this->particles[i].pressure = SPH_GAS_CONSTANT * (this->particles[i].density - SPH_GAS_CONSTANT);
    }

    // Calculate the spatial grid again. This time with the updated density and pressure values.
    this->spatial_hash_grid.clear();
    // Now insert every particle based on its position.
    for (int i = 0; i < this->number_of_particles; i++) {
        this->spatial_hash_grid.insert(std::make_pair(
            Particle_System::hash(this->particles[i].position), 
            this->particles[i]
        ));
    }
    for (int i = 0; i < this->number_of_particles; i++) {
        // Look for all particles into the neighbouring 26 cells.
        for (int look_x = -1; look_x <= 1; look_x++) {
            for (int look_y = -1; look_y <= 1; look_y++) {
                for (int look_z = -1; look_z <= 1; look_z++) {
                    glm::vec3 look_position = this->particles[i].position + glm::vec3(look_x, look_y, look_z) * this->sph_kernel_radius;
                    int look_key = this->hash(look_position);
                    auto range = this->spatial_hash_grid.equal_range(look_key);
                    for (auto it = range.first; it != range.second; ++it) {
                        // If the distance to this particle is less than the kernel radius, we need this one.
                        if (glm::distance(this->particles[i].position, it->second.position) < this->sph_kernel_radius) {
                            this->neighbor_list[i].push_back(it->second);
                        }
                    }
                }
            }
        }
    } 

    
    // Calculate the forces.
    for (int i = 0; i < this->number_of_particles; i++) {

        // Calculate the forces.
        glm::vec3 f_pressure(0.0f);
        glm::vec3 f_viscosity(0.0f);
        glm::vec3 f_surface(0.0f);
        glm::vec3 f_external(0.0f, -9.8f, 0.0f);
        glm::vec3 color_field_normal(0.0f);
        float color_field_laplacian = 0.0f;

        for (int idx_neighbor = 0; idx_neighbor < this->neighbor_list[i].size(); idx_neighbor++) {
            glm::vec3 distance_vector = this->particles[i].position - this->neighbor_list[i][idx_neighbor].position;
            f_pressure += (float)(this->particles[i].pressure / pow(this->particles[i].density, 2) + 
                this->neighbor_list[i][idx_neighbor].pressure / pow(this->neighbor_list[i][idx_neighbor].density, 2)) *
                this->kernel_w_spiky_gradient(distance_vector);
            f_viscosity += (this->neighbor_list[i][idx_neighbor].velocity - this->particles[i].velocity) * 
                this->kernel_w_viscosity_laplacian(distance_vector) / 
                this->neighbor_list[i][idx_neighbor].density;
            color_field_normal += this->kernel_w_poly6_gradient(distance_vector) / 
                this->neighbor_list[i][idx_neighbor].density;
            color_field_laplacian += this->kernel_w_poly6_laplacian(distance_vector) / 
                this->neighbor_list[i][idx_neighbor].density;
        }
        f_pressure *= -SPH_PARTICLE_MASS * this->particles[i].density;
        f_viscosity *= SPH_PARTICLE_MASS * SPH_VISCOSITY;
        color_field_normal *= SPH_PARTICLE_MASS;
        this->particles[i].normal = -1.0f * color_field_normal;
        color_field_laplacian *= SPH_PARTICLE_MASS;
        // surface tension force
        float color_field_normal_magnitude = glm::length(color_field_normal);
        if (color_field_normal_magnitude > SPH_SURFACE_THRESHOLD) {
            f_surface = -SPH_SURFACE_TENSION * (color_field_normal / color_field_normal_magnitude) * color_field_laplacian;
        }

        // Calculate the acceleration.
        glm::vec3 acceleration = (f_pressure + f_viscosity + f_surface + f_external) / this->particles[i].density;

        // Compute the new position and new velocity using the velocity verlet integration.
        static float time_step_squared = pow(SPH_SIMULATION_TIME_STEP, 2);
        glm::vec3 new_position = this->particles[i].position + 
            this->particles[i].velocity * SPH_SIMULATION_TIME_STEP + 
            acceleration * time_step_squared;
        glm::vec3 new_velocity = (new_position - this->particles[i].position) / SPH_SIMULATION_TIME_STEP;
        this->particles[i].position = new_position;
        this->particles[i].velocity = new_velocity;

        // Resolve collision.
        this->particles[i] = this->resolve_collision(this->particles[i]);
    }
}

void Particle_System::simulate_brute_force ()
{
    // Calculate for each particle the density and the pressure.
    for (int i = 0; i < this->number_of_particles; i++) {
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
    
    // Calculate the forces.
    for (int i = 0; i < this->number_of_particles; i++) {
        glm::vec3 f_pressure(0.0f);
        glm::vec3 f_viscosity(0.0f);
        glm::vec3 f_surface(0.0f);
        glm::vec3 f_external(0.0f, -9.8f, 0.0f);
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
        glm::vec3 acceleration = (f_pressure + f_viscosity + f_surface + f_external) / this->particles[i].density;

        // Compute the new position and new velocity using the velocity verlet integration.
        static float time_step_squared = pow(SPH_SIMULATION_TIME_STEP, 2);
        glm::vec3 new_position = this->particles[i].position + 
            this->particles[i].velocity * SPH_SIMULATION_TIME_STEP + 
            acceleration * time_step_squared;
        glm::vec3 new_velocity = (new_position - this->particles[i].position) / SPH_SIMULATION_TIME_STEP;
        this->particles[i].position = new_position;
        this->particles[i].velocity = new_velocity;

        // Resolve collision.
        this->particles[i] = this->resolve_collision(this->particles[i]);
    }
}

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