#version 410 core

layout (location = 0) in vec3 position;

// We need the vertex values in the geometry shader.
// Input.
layout (location = 2) in int value_vertex_0;
layout (location = 3) in int value_vertex_1;
layout (location = 4) in int value_vertex_2;
layout (location = 5) in int value_vertex_3;
layout (location = 6) in int value_vertex_4;
layout (location = 7) in int value_vertex_5;
layout (location = 8) in int value_vertex_6;
layout (location = 9) in int value_vertex_7;
// Output.
out int value_vertex_0_geom;
out int value_vertex_1_geom;
out int value_vertex_2_geom;
out int value_vertex_3_geom;
out int value_vertex_4_geom;
out int value_vertex_5_geom;
out int value_vertex_6_geom;
out int value_vertex_7_geom;

// The view and the projection matrix are set using uniforms.
uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;

void main()
{
    // Pass the vertex values to the geometry shader.
    value_vertex_0_geom = value_vertex_0;
    value_vertex_1_geom = value_vertex_1;
    value_vertex_2_geom = value_vertex_2;
    value_vertex_3_geom = value_vertex_3;
    value_vertex_4_geom = value_vertex_4;
    value_vertex_5_geom = value_vertex_5;
    value_vertex_6_geom = value_vertex_6;
    value_vertex_7_geom = value_vertex_7;
    // Within this project we do not need a model matrix, so only use the 
    // projection matrix and view matrix.
    gl_Position = vec4(position, 1.0);
}