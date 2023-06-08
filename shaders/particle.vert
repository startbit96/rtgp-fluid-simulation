#version 330 core

layout (location = 0) in vec3 position;
//layout (location = 1) in float density;
layout (location = 3) in vec3 velocity;

//out float vertex_density;
out vec3 vertex_velocity;

uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;

void main()
{
    // Within this project we do not need a model matrix, so only use the 
    // projection matrix and view matrix.
    gl_Position = u_projection_matrix * u_view_matrix * vec4(position, 1.0);

    // Pass the density and velocity to the geometry / fragment shader for size / coloring.
    //vertex_density = density;
    vertex_velocity = velocity;
}