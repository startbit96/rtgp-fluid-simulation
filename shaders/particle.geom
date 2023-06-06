#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 vertex_velocity[];
out vec3 geometry_velocity; 
out vec2 tex_coord;

void main()
{
    for (int i = 0; i < gl_in.length(); ++i)
    {
        vec4 position = gl_in[i].gl_Position;
        geometry_velocity = vertex_velocity[i];
        float size = 0.1;

        gl_Position = position + vec4(-size, -size, 0.0, 0.0);
        tex_coord = vec2(0.0, 0.0);
        EmitVertex();

        gl_Position = position + vec4(-size, size, 0.0, 0.0);
        tex_coord = vec2(0.0, 1.0);
        EmitVertex();

        gl_Position = position + vec4(size, -size, 0.0, 0.0);
        tex_coord = vec2(1.0, 0.0);
        EmitVertex();

        gl_Position = position + vec4(size, size, 0.0, 0.0);
        tex_coord = vec2(1.0, 1.0);
        EmitVertex();

        EndPrimitive();
    }
}