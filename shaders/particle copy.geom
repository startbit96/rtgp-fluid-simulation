#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

// We get the density and velocity for each particle from the vertex shader.
// Use the density in the geometry shader for the size of the circle.
// Pass the velocity to the fragment shader. It will generate the color from it.
in float vertex_density[];
in vec3 vertex_velocity[];
out vec3 geometry_velocity;
out vec2 tex_coord;

void main()
{
    for (int i = 0; i < gl_in.length(); i++) {
        vec4 position = gl_in[i].gl_Position;
        geometry_velocity = vertex_velocity[i];

        // Map the density value to a particle size range.
        float density = vertex_density[0];
        float min_density = 5.0;
        float max_density = 20.0;
        float min_size = 0.005;
        float max_size = 0.1;
        // Calculate the size based on density using linear interpolation.
        if (density < min_density) {
            density = min_density;
        }
        else if (density > max_density) {
            density = max_density;
        }
        float size = mix(min_size, max_size, (density - min_density) / (max_density - min_density));
        
        // Prevent size from being negative or exceeding maxSize
        size = max(min_size, size);
        size = min(max_size, size);

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