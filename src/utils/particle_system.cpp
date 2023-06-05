#include "particle_system.h"

#include "debug.h"
#include "helper.h"

Particle_System::Particle_System ()
{
    this->vertex_array_object = 0;
    this->number_of_particles = 0;
    this->number_of_particles_as_string = to_string_with_separator(this->number_of_particles);
    this->particle_initial_distance = PARTICLE_INITIAL_DISTANCE_INIT;
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