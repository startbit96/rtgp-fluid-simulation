#version 410 core

layout(location = 0) out vec4 color;

// The normal was calculated in the geometry shader and passed to the fragment shader.
in vec3 fragment_normal;

void main()
{
    // We apply a simple shadow using the Lambert model.
    // The light will come from this direction, use global coordinates here. The calculated normal
    // from the geometry shader is also in global coordinates.
    vec3 light_direction = normalize(vec3(1.0, 10.0, 1.0));

    // Calculate the diffuse component using Lambertian illumination model.
    vec3 light_color = vec3(1.0, 1.0, 1.0);
    float diffuse_intensity = max(dot(fragment_normal, light_direction), 0.0);
    // The diffuse is max when the fragment normal has the same direction as the light.
    vec3 diffuse = light_color * diffuse_intensity;

    // Adjust the shadow intensity. A intensity factor of 0.0 means there will be no shadow,
    // a factor of 1.0 means, the full shadow will be completely black.
    float shadow_intensity = 0.6;
    vec3 adjusted_diffuse = diffuse * shadow_intensity + vec3(1.0 - shadow_intensity);

    // Apply the diffuse component to the object's color.
    vec3 object_color = vec3(0.0, 0.0, 1.0);
    vec3 final_color = object_color * adjusted_diffuse;

    // Output the color of the fragment.
    color = vec4(final_color, 1.0); 
}