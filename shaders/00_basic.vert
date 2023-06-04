#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const mat4 constantModel = mat4(0.1); // Constant model matrix
uniform mat4 viewMatrix;
//const mat4 viewMatrix = mat4(1.0); // Set the view matrix
//const mat4 constantProjection = mat4(0.1); // Constant projection matrix
uniform mat4 projectionMatrix;

void main()
{
    mat4 MVP = projectionMatrix * viewMatrix * constantModel;
    gl_Position = MVP * vec4(position, 1.0);
}