#version 330 core

layout(location = 0) out vec4 color;

// The normal was calculated in the geometry shader and passed to the fragment shader.
in vec3 fragment_normal;

void main()
{
    // We apply a simple shadow.
    // Normalize the fragment's normal and light direction.
    vec3 fragment_normal_normalized = normalize(fragment_normal);
    vec3 light_direction = normalize(vec3(1.0, 1.0, 1.0));

    // Calculate the diffuse component using Lambertian lighting model.
    vec3 light_color = vec3(1.0, 1.0, 1.0);
    float diffuse_intensity = max(dot(fragment_normal_normalized, light_direction), 0.0);
    vec3 diffuse = light_color * diffuse_intensity;

    // Adjust the shadow intensity
    float shadow_intensity = 0.5;
    vec3 adjusted_diffuse = diffuse * (1.0 - shadow_intensity) + vec3(shadow_intensity);

    // Apply the diffuse component to the object's color
    vec3 object_color = vec3(0.0, 0.0, 1.0);
    vec3 final_color = object_color * adjusted_diffuse;

    color = vec4(final_color, 0.2); 
}