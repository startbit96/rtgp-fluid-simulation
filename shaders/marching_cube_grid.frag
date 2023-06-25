#version 410 core

layout(location = 0) out vec4 color;

in vec3 cube_vertex;

void main()
{
    color = vec4(0.0, 0.0, 1.0, 0.1);  // Set the color of the cube lines to white
}