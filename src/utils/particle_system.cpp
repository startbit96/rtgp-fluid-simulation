#include "particle_system.h"

#include "debug.h"


void Particle_System::draw ()
{
    GLCall( glBindVertexArray(this->vertex_array_object) );
    GLCall( glDrawElements(GL_POINTS, this->number_of_particles, GL_UNSIGNED_INT, 0) );
}