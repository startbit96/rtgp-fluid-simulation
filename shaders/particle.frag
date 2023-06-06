#version 330 core

layout(location = 0) out vec4 color;

in vec3 geometry_velocity;
in vec2 tex_coord;
uniform float u_aspect_ratio;

void main()
{
    // Center and radius of the circle. 
    // The tex_coords range from 0 to 1 in both dimensions.
    vec2 center = vec2(0.5, 0.5);
    float radius = 0.5;

    // Calculate the distance from the fragment to the center. 
    // Take the aspect ratio into account.
    vec2 dist_vector = (tex_coord - center) * vec2(u_aspect_ratio, 1.0);
    float distance_from_center = length(dist_vector);

    // If the distance is greater than the radius, discard the fragment
    if (distance_from_center > radius)
        discard;

    // Set the color of the fragment
    color = vec4(0.1, length(geometry_velocity) * 0.5, 1.0 - distance_from_center, 1.0);
}