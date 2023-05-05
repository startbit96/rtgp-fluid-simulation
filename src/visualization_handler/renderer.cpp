#include "renderer.h"
#include <iostream>

void Renderer::clear() const
{
        GLCall(glClear( GL_COLOR_BUFFER_BIT ));
}

void Renderer::draw(
    const Vertex_Array& vertex_array, 
    const Index_Buffer& index_buffer, 
    const Shader& shader) const
{
        shader.bind();
        vertex_array.bind();
        // Vertex Array already has a binding to the index buffer so we do not need to bind the index buffer again.
        //index_buffer.bind();
        GLCall(glDrawElements(GL_TRIANGLES, index_buffer.get_count(), GL_UNSIGNED_INT, nullptr));
}