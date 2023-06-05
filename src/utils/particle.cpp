#include "particle.h"
#include "debug.h"


Particle get_default_particle(float x, float y, float z)
{
    return Particle {
        glm::vec3(x, y ,z)
    };
}

void describe_particle_memory_layout ()
{
    // Describe the vertex buffer layout.
    GLCall( glEnableVertexAttribArray(0) );
    // Position.
    GLCall( glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0) );
}