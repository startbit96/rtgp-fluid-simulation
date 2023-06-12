#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec3 vertex_velocity[];
in float vertex_density[];
out vec3 geometry_velocity; 
out vec2 tex_coord;

void main()
{
    vec4 position = gl_in[0].gl_Position;
    geometry_velocity = vertex_velocity[0];
    float density = vertex_density[0];

    // Get the size based on the density.
    float density_min = 1.0;
    float density_max = 20.0;
    float radius_min = 0.05;
    float radius_max = 0.1;
    density = min(max(density, density_min), density_max);
    float density_normalized = (density - density_min) / (density_max - density_min);
    float radius = mix(radius_min, radius_max, density_normalized);

    gl_Position = position + vec4(-radius, -radius, 0.0, 0.0);
    tex_coord = vec2(0.0, 0.0);
    EmitVertex();

    gl_Position = position + vec4(-radius, radius, 0.0, 0.0);
    tex_coord = vec2(0.0, 1.0);
    EmitVertex();

    gl_Position = position + vec4(radius, -radius, 0.0, 0.0);
    tex_coord = vec2(1.0, 0.0);
    EmitVertex();

    gl_Position = position + vec4(radius, radius, 0.0, 0.0);
    tex_coord = vec2(1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}