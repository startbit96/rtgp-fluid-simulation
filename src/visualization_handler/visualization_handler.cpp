#include "visualization_handler.h"

Visualization_Handler::Visualization_Handler ()
{
    this->window = nullptr;
}

void Visualization_Handler::visualize ()
{
    // Poll for events and process them.
    glfwPollEvents();
    // "Clear" the frame and z buffer.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Swap front and back buffers.
    glfwSwapBuffers(this->window);
}