#include "visualization_handler.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../utils/typedef.h"
#include "../utils/debug.h"

Visualization_Handler::Visualization_Handler ()
{
    this->window = nullptr;
}

void Visualization_Handler::init ()
{
    Shader shader ("../shaders/00_basic.vert", "../shaders/00_basic.frag");
    shader.use_program();
}

void Visualization_Handler::visualize ()
{
    // Poll for events and process them.
    glfwPollEvents();
    // Wireframe mode.
    GLCall( glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) );
    // "Clear" the frame and z buffer.
    GLCall( glClearColor(0.2f, 0.3f, 0.3f, 1.0f) );
    GLCall( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );
    // Visualize the simulation space.
    GLCall( glBindVertexArray(this->simulation_space->vertex_array_object) );
    GLCall( glDrawElements(GL_TRIANGLES, this->simulation_space->indices.size(), GL_UNSIGNED_INT, 0) );
    GLCall( glBindVertexArray(0) );
    // Visualize the starting positions of the fluid.
    for (int i = 0; i < this->fluid_start_positions->size(); i++) {
        GLCall( glBindVertexArray(this->fluid_start_positions->at(i).vertex_array_object) );
        GLCall( glDrawElements(GL_TRIANGLES, this->fluid_start_positions->at(i).indices.size(), GL_UNSIGNED_INT, 0) );
    }
    // Visualize the particles.
    GLCall( glPointSize(5.0f) );
    GLCall( glBindVertexArray(this->vao_particles) );
    GLCall( glDrawElements(GL_POINTS, this->number_of_particles, GL_UNSIGNED_INT, 0) );
    // Unbind.
    GLCall( glBindVertexArray(0) );
    // Swap front and back buffers.
    glfwSwapBuffers(this->window);
}