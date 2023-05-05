#include "vertex_buffer.h"
#include "../utils/debug.h"

Vertex_Buffer::Vertex_Buffer(const void* data, unsigned int size)
{
    GLCall(glGenBuffers(1, &m_renderer_id));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id));
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

Vertex_Buffer::~Vertex_Buffer()
{
    GLCall(glDeleteBuffers(1, &m_renderer_id));
}

void Vertex_Buffer::bind() const
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_renderer_id));
}

void Vertex_Buffer::unbind() const
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}