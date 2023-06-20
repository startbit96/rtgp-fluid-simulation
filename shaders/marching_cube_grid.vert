#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;

void main()
{
    // Within this project we do not need a model matrix, so only use the 
    // projection matrix and view matrix.
    gl_Position = u_projection_matrix * u_view_matrix * vec4(position, 1.0);
}