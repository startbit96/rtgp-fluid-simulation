#version 330 core

layout (location = 0) in vec3 position;
layout (location = 3) in vec3 velocity;

out vec3 vertex_velocity;

uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;

void main()
{
    // Within this project we do not need a model matrix, so only use the 
    // projection matrix and view matrix.
    gl_Position = u_projection_matrix * u_view_matrix * vec4(position, 1.0);

    // Pass the velocity to the fragment shader for coloring.
    vertex_velocity = velocity;
}