#include "particle.h"
#include "debug.h"


Particle get_default_particle(float x, float y, float z)
{
    return Particle {
        glm::vec3(x, y ,z),
        1000.0f,
        1000.0f,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f)
    };
}

void describe_particle_memory_layout ()
{
    // Describe the vertex buffer layout.
    unsigned int index = 0;
    // Position.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)0) );
    index++;
    // Density.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)(offsetof(Particle, density))) );
    index++;
    // Pressure.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)(offsetof(Particle, pressure))) );
    index++;
    // Velocity.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)(offsetof(Particle, velocity))) );
    index++;
    // Acceleration.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)(offsetof(Particle, acceleration))) );
    index++;
    // Normal.
    GLCall( glEnableVertexAttribArray(index) );
    GLCall( glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)(offsetof(Particle, normal))) );
    index++;
}