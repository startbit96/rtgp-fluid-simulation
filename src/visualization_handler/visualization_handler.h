#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "../utils/typedef.h"
#include "camera.h"

class Visualization_Handler 
{
    private:

    public:
        GLFWwindow* window;
        Cuboid *simulation_space;
        std::vector<Cuboid> *fluid_start_positions;
        unsigned int vao_particles;
        unsigned int number_of_particles;
        Camera camera;
        Shader* shader;

        Visualization_Handler ();

        void init ();
        void visualize ();
};