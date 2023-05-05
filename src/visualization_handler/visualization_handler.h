#pragma once

#include <GLFW/glfw3.h>

class Visualization_Handler {
    private:

    public:
        GLFWwindow* window;

        Visualization_Handler ();

        void visualize ();
};