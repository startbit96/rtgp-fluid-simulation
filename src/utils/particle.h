#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

// Particle.
struct Particle 
{
    glm::vec3 position;
    GLfloat density;
    GLfloat pressure;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    // For the velocity verlet integration we also need the old acceleration.
    glm::vec3 old_acceleration;
};

// A function that returns particle based on the given x, y, z position.
// All other values are set to their default values.
Particle get_default_particle(float x, float y, float z);

// Describe the layout for the vertex buffer object.
void describe_particle_memory_layout ();