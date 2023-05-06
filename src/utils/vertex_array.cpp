#include "vertex_array.h"
#include "debug.h"

Vertex_Array::Vertex_Array()
{
    GLCall(glGenVertexArrays(1, &m_renderer_id));
}

Vertex_Array::~Vertex_Array()
{
    GLCall(glDeleteVertexArrays(1, &m_renderer_id));
}

void Vertex_Array::add_buffer(const Vertex_Buffer& vertex_buffer, const Vertex_Buffer_Layout& layout)
{
    bind();
    vertex_buffer.bind();
    const std::vector<Vertex_Buffer_Element> elements = layout.get_elements();
    unsigned int offset = 0;
    for (unsigned int i = 0; i < elements.size() ; i++)
    {
        const Vertex_Buffer_Element element = elements[i];
        GLCall(glEnableVertexAttribArray(i));
        GLCall(glVertexAttribPointer(i, element.count, element.type, element.normalized,
                                      layout.get_stride(), INT2VOIDP(offset)));
        offset += element.count * Vertex_Buffer_Element::get_size_of_type(element.type);
    }
}

void Vertex_Array::bind() const
{
    GLCall(glBindVertexArray(m_renderer_id));
}

void Vertex_Array::unbind() const
{
    GLCall(glBindVertexArray(0));
};
