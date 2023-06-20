#include <cmath>

#include "cuboid.h"
#include "debug.h"

Cuboid::Cuboid ()
{
    this->vertex_array_object = 0;
}

Cuboid::Cuboid (float x_min, float x_max, float y_min, float y_max, float z_min, float z_max)
:
    x_min { x_min },
    x_max { x_max },
    y_min { y_min },
    y_max { y_max },
    z_min { z_min },
    z_max { z_max }
{
    this->vertices = std::vector<glm::vec3> {
        glm::vec3 (x_min, y_min, z_min), // 0, back  left  bottom
        glm::vec3 (x_max, y_min, z_min), // 1, back  right bottom 
        glm::vec3 (x_min, y_max, z_min), // 2, back  left  top
        glm::vec3 (x_max, y_max, z_min), // 3, back  right top
        glm::vec3 (x_min, y_min, z_max), // 4, front left  bottom
        glm::vec3 (x_max, y_min, z_max), // 5, front right bottom
        glm::vec3 (x_min, y_max, z_max), // 6, front left  top
        glm::vec3 (x_max, y_max, z_max)  // 7, front right top
    };
    this->indices = std::vector<GLuint> {
        0, 1, 2,    // back face triangle 1
        2, 3, 1,    // back face triangle 2
        0, 4, 5,    // bottom face triangle 1
        5, 1, 0,    // bootom face triangle 2
        4, 5, 7,    // front face triangle 1
        7, 6, 4,    // front face triangle 2
        6, 7, 3,    // top face triangle 1
        3, 2, 6,    // top face triangle 2
        0, 2, 6,    // left face triangle 1
        6, 4, 0,    // left face triangle 2
        1, 3, 7,    // right face triangle 1
        7, 5, 1     // right face triangle 2
    };

    // Generate the OpenGL buffers for this cuboid.
    GLCall( glGenVertexArrays(1, &this->vertex_array_object) );
    GLCall( glGenBuffers(1, &this->vertex_buffer_object) );
    GLCall( glGenBuffers(1, &this->index_buffer_object) );

    // Make vertex array object active.  
    GLCall( glBindVertexArray(this->vertex_array_object) );
    // Copy the data into the vertex buffer object.
    GLCall( glBindBuffer(GL_ARRAY_BUFFER, this->vertex_buffer_object) );
    GLCall( glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * this->vertices.size(), &this->vertices[0], GL_STATIC_DRAW) );
    // Copy the indices into the index buffer object.
    GLCall( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->index_buffer_object) );
    GLCall( glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * this->indices.size(), &this->indices[0], GL_STATIC_DRAW) );
    // Describe the vertex buffer layout.
    GLCall( glEnableVertexAttribArray(0) );
    GLCall( glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0) );
    // Unbind.
    GLCall( glBindBuffer(GL_ARRAY_BUFFER, 0) );
    GLCall( glBindVertexArray(0) );
}

Cuboid::~Cuboid ()
{

}

bool Cuboid::contains (Cuboid &other)
{
    return (
        (other.x_min >= this->x_min) &&
        (other.x_max <= this->x_max) &&
        (other.y_min >= this->y_min) &&
        (other.y_max <= this->y_max) &&
        (other.z_min >= this->z_min) &&
        (other.z_max <= this->z_max)
    );
}

bool Cuboid::contains (glm::vec3 position)
{
    return (
        (position.x >= this->x_min) &&
        (position.x <= this->x_max) &&
        (position.y >= this->y_min) &&
        (position.y <= this->y_max) &&
        (position.z >= this->z_min) &&
        (position.z <= this->z_max)
    );
}

float Cuboid::get_volume ()
{
    float edge_length_x = this->x_max - this->x_min;
    float edge_length_y = this->y_max - this->y_min;
    float edge_length_z = this->z_max - this->z_min;
    return edge_length_x * edge_length_y * edge_length_z;
}

glm::vec3 Cuboid::get_point_of_interest ()
{
    return glm::vec3(
        (this->x_min + this->x_max) / 2,
        (this->y_min + this->y_max) / 2,
        (this->z_min + this->z_max) / 2
    );
}

void Cuboid::fill_with_particles (float particle_distance, std::vector<Particle>& particles)
{
    // In order to not have particles at the borders to begin with, we do not set the particles at the 
    // surface of the cuboid but only within.
    for (float x = this->x_min + particle_distance / 2; x < this->x_max; x += particle_distance) {
        for (float y = this->y_min + particle_distance / 2; y < this->y_max; y += particle_distance) {
            for (float z = this->z_min + particle_distance / 2; z < this->z_max; z += particle_distance) {
                particles.push_back(get_default_particle(x, y ,z));
            }
        }
    }
}

void Cuboid::draw (bool unbind)
{
    GLCall( glBindVertexArray(this->vertex_array_object) );
    GLCall( glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0) );
    // In order to save a few unbind-calls, do this only if neccessary. 
    // In our case, the visualization handler will handle the unbinding, so normally we will not unbind here.
    if (unbind == true) {
        GLCall( glBindVertexArray(0) );
    }
}

void Cuboid::free_gpu_resources ()
{
    if (this->vertex_array_object > 0) {
        GLCall( glDeleteVertexArrays(1, &this->vertex_array_object) );
        GLCall( glDeleteBuffers(1, &this->vertex_buffer_object) );
        GLCall( glDeleteBuffers(1, &this->index_buffer_object) );
    }
}