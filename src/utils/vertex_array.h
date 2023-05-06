#pragma once

#include "vertex_buffer.h"
#include "vertex_buffer_layout.h"

class Vertex_Array
{
    private:
        unsigned int m_renderer_id;

    public:
        Vertex_Array();
        ~Vertex_Array();

        void add_buffer(const Vertex_Buffer& vertex_buffer, const Vertex_Buffer_Layout& layout);
        void bind() const;
        void unbind() const;
};
