#version 330 core

layout(location = 0) out vec4 color;

in vec2 texCoord;
uniform vec4 u_color;
uniform float aspectRatio;

void main()
{
    vec2 center = vec2(0.5, 0.5); // Center of the circle
    float radius = 0.5; // Radius of the circle

    // Adjust the radius based on the aspect ratio
    float adjustedRadius = radius * aspectRatio;

    // Calculate the distance from the fragment to the center, taking aspect ratio into account
    vec2 distance = (texCoord - center) * vec2(aspectRatio, 1.0);
    float dist = length(distance);

    // If the distance is greater than the radius, discard the fragment
    if (dist > radius)
        discard;

    // Set the color of the fragment
    color = u_color;
}