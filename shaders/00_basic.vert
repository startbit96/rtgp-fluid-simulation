#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const mat4 constantModel = mat4(0.5); // Constant model matrix
const mat4 constantView = mat4(0.2); // Constant view matrix
const mat4 constantProjection = mat4(0.1); // Constant projection matrix

void main()
{
    mat4 MVP = constantProjection * constantView * constantModel;
    gl_Position = MVP * vec4(position, 1.0);
}