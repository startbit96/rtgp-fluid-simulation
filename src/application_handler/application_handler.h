#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../input_handler/input_handler.h"
#include "../simulation_handler/simulation_handler.h"
#include "../visualization_handler/visualization_handler.h"

// Note, the window width, height and title are defined in the visualization_handler.h file.
// We need them there for the projection matrix and the fps counter.

enum Application_State 
{
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
        bool _is_running;

    public:
        Application_State current_state;
        Application_State next_state;
        GLFWwindow* window;

        Input_Handler input_handler;
        Simulation_Handler simulation_handler;
        Visualization_Handler visualization_handler;

        Application_Handler();
        
        bool initialize();
        void terminate();
        bool is_running ();
};