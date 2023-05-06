#include "index_buffer.h"
#include "debug.h"

Index_Buffer::Index_Buffer(const unsigned int* indices, unsigned int count)
  :
    m_count(count)
{
    ASSERT(sizeof(unsigned int) == sizeof(GLuint));

    GLCall(glGenBuffers(1, &m_renderer_id));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_id));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW));
}

Index_Buffer::~Index_Buffer()
{
    GLCall(glDeleteBuffers(1, &m_renderer_id));
}

void Index_Buffer::bind() const
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_renderer_id));
}

void Index_Buffer::unbind() const
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}
