#pragma once

#include <vector>
#include <GL/glew.h>
#include "debug.h"

struct Vertex_Buffer_Element
{
    unsigned int type;
    unsigned int count;
    unsigned char normalized;

    static unsigned int get_size_of_type(unsigned int type)
    {
        switch (type)
        {
            case GL_FLOAT         : return sizeof(GLfloat);
            case GL_UNSIGNED_INT  : return sizeof(GLuint);
            case GL_UNSIGNED_BYTE : return sizeof(GLbyte);
        }
        ASSERT(false);
        return 0;
    }
};

class Vertex_Buffer_Layout
{
    private:
        unsigned int m_stride;
        std::vector<Vertex_Buffer_Element> m_elements;

    public:
        Vertex_Buffer_Layout() :
            m_stride(0) { }

        void add_float(unsigned int count)         { push(GL_FLOAT, count, GL_FALSE);        }
        void add_unsigned_int(unsigned int count)  { push(GL_UNSIGNED_INT, count, GL_FALSE); }
        void add_unsigned_byte(unsigned int count) { push(GL_UNSIGNED_BYTE, count, GL_TRUE); }

        inline const std::vector<Vertex_Buffer_Element> get_elements() const { return m_elements; };
        inline unsigned int get_stride() const { return m_stride; };

    private:
        void push(unsigned int type, unsigned int count, unsigned char normalized)
        {
            struct Vertex_Buffer_Element vertex_buffer_element = {type, count, normalized};
            m_elements.push_back(vertex_buffer_element);
            m_stride += count * Vertex_Buffer_Element::get_size_of_type(type);
        };
};