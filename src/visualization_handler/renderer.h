#pragma once

#include <GL/glew.h>

#include "../utils/debug.h"
#include "vertex_array.h"
#include "index_buffer.h"
#include "shader.h"

class Renderer
{
    public:
        void clear() const;
        void draw(  const Vertex_Array& vertex_array, 
                    const Index_Buffer& index_buffer, 
                    const Shader& shader) const;
};