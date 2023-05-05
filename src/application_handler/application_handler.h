#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../input_handler/input_handler.h"
#include "../scene_handler/scene_handler.h"
#include "../simulation_handler/simulation_handler.h"
#include "../visualization_handler/visualization_handler.h"

#define DEFAULT_WINDOW_WIDTH        1024
#define DEFAULT_WINDOW_HEIGHT       768
#define DEFAULT_WINDOW_NAME         "RTGP - Fluid Simulation"


enum Application_State {
    IDLE,
    APPLICATION_INITIALIZATION,
    APPLICATION_TERMINATION,
    SIMULATION_INITIALIZATION,
    SIMULATION_RUNNING,
    SIMULATION_TERMINATION
};

class Application_Handler
{
    private:
        bool initialize_window();

    public:
        bool is_running;
        Application_State current_state;
        Application_State next_state;
        GLFWwindow* window;

        Input_Handler input_handler;
        Scene_Handler scene_handler;
        Simulation_Handler simulation_handler;
        Visualization_Handler visualization_handler;

        Application_Handler();
        
        bool initialize();
        void terminate();
};