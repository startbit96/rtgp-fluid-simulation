#version 330 core

layout(points) in;
layout(line_strip, max_vertices = 16) out;

uniform mat4 u_view_matrix;
uniform mat4 u_projection_matrix;

uniform float u_cube_edge_length;

void main()
{
    // Define the vertices. Keep in mind to use the projection and view matrix on them as well.
    vec4 vertices[8] = vec4[8](
        // Bottom vertices.
        gl_in[0].gl_Position,
        gl_in[0].gl_Position + u_projection_matrix * u_view_matrix * vec4(u_cube_edge_length, 0.0, 0.0, 0.0),
        gl_in[0].gl_Position + u_projection_matrix * u_view_matrix * vec4(u_cube_edge_length, 0.0, u_cube_edge_length, 0.0),
        gl_in[0].gl_Position + u_projection_matrix * u_view_matrix * vec4(0.0, 0.0, u_cube_edge_length, 0.0),
        // Top vertices.
        gl_in[0].gl_Position + u_projection_matrix * u_view_matrix * vec4(0.0, u_cube_edge_length, 0.0, 0.0),
        gl_in[0].gl_Position + u_projection_matrix * u_view_matrix * vec4(u_cube_edge_length, u_cube_edge_length, 0.0, 0.0),
        gl_in[0].gl_Position + u_projection_matrix * u_view_matrix * vec4(u_cube_edge_length, u_cube_edge_length, u_cube_edge_length, 0.0),
        gl_in[0].gl_Position + u_projection_matrix * u_view_matrix * vec4(0.0, u_cube_edge_length, u_cube_edge_length, 0.0)
    );

    // It is difficult to describe the order of indices here. Its basically running around in zick-zack moves to draw all
    // the lines.
    // The axis are "normal": x is to the right, y to the top, z to the front.
    // The cube is indexed in this way:
    //
    //        4 +--------+ 5
    //         /|       /|
    //        / |      / |
    //     7 +--------+ 6|
    //       |  |     |  |
    //       |0 +-----|--+ 1
    //       | /      | /
    //       |/       |/
    //     3 +--------+ 2
    
    int indices[16] = int[16] (
        0, 1, 5, 6, 2, 3, 7, 4, 
        0, 3, 7, 6, 2, 1, 5, 4
    );

    // Emit all vertices we need.
    for (int i = 0; i < 16; i++) {
        gl_Position = vertices[indices[i]];
        EmitVertex();
    }

    EndPrimitive();
}