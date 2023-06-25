#version 410 core

layout(location = 0) out vec4 color;

in vec3 geometry_velocity;
in vec2 tex_coord;
uniform float u_aspect_ratio;

// This functions interpolates the rgb color depending on the speed
// of the particle. It goes from blue (low speed) over green to red (high speed).
vec4 velocity_to_rgba (vec3 velocity)
{
    float r, g, b, a;
    float speed_min = 0.0;
    float speed_max = 2.0;
    float speed = length(velocity);
    float speed_normalized = (speed - speed_min) / (speed_max - speed_min);
    r = clamp((speed_normalized - 0.5) * 2.0, 0.0, 1.0);
    g = 1.0 - abs(speed_normalized * 2.0 - 1.0);
    b = clamp(1.0 - speed_normalized * 2.0, 0.0, 1.0);
    a = 1.0;
    return vec4(r, g, b, a);
}

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

    // Set the color of the fragment based on the velocity.
    color = velocity_to_rgba(geometry_velocity);
    // Get a little bit darker to the edge.
    color = color - distance_from_center * 0.75;
}