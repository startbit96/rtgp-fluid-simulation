#include "particle.h"
#include "debug.h"


Particle get_default_particle(float x, float y, float z)
{
    return Particle {
        glm::vec3(x, y ,z),
        0.0f,
        glm::vec3(0.0f, -2.0f, 0.0f)
    };
}

void describe_particle_memory_layout ()
{
    // Describe the vertex buffer layout.
    // Position
    GLCall( glEnableVertexAttribArray(0) );
    GLCall( glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)0) );
    // Density.
    GLCall( glEnableVertexAttribArray(1) );
    GLCall( glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)(offsetof(Particle, density))) );
    // Velocity.
    GLCall( glEnableVertexAttribArray(2) );
    GLCall( glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)(offsetof(Particle, velocity))) );
}