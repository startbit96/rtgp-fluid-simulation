#pragma once

#include <glm/glm.hpp>

// Particle.
struct Particle 
{
    glm::vec3 position;
    float density;
    float pressure;
    glm::vec3 velocity;
    glm::vec3 acceleration;
};

// A function that returns particle based on the given x, y, z position.
// All other values are set to their default values.
Particle get_default_particle(float x, float y, float z);

// Describe the layout for the vertex buffer object.
void describe_particle_memory_layout ();