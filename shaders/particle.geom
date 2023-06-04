#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

uniform float pointSize;

out vec2 texCoord;

void main()
{
    vec4 position = gl_in[0].gl_Position;
    float size = pointSize;

    gl_Position = position + vec4(-size, -size, 0.0, 0.0);
    texCoord = vec2(0.0, 0.0);
    EmitVertex();

    gl_Position = position + vec4(-size, size, 0.0, 0.0);
    texCoord = vec2(0.0, 1.0);
    EmitVertex();

    gl_Position = position + vec4(size, -size, 0.0, 0.0);
    texCoord = vec2(1.0, 0.0);
    EmitVertex();

    gl_Position = position + vec4(size, size, 0.0, 0.0);
    texCoord = vec2(1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}