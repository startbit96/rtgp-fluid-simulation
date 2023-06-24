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

        // This program was developed on a Mac with Retina display.
        // There it was noticed that some display (e.g. the Retina display) have 
        // a higher pixel density compared to traditional displays, which means that a 
        // single logical unit (pixel) on the screen corresponds to multiple physical pixels. 
        // To accommodate this high pixel density, macOS provides a feature called "Retina scaling" 
        // or "pixel doubling." This feature scales up the content displayed on the screen, making 
        // it appear sharper and more detailed.
        // The problem is that we need this scale factor in order to perform mouse interaction with
        // the screen. So when the application handler initializes the window, we will calculate
        // the scale factors and every time the window size changes, we pass to the visualization
        // handler not the value we get from the callback but the value divided by the scale factor.
        float scale_factor_x;
        float scale_factor_y;

        Input_Handler input_handler;
        Simulation_Handler simulation_handler;
        Visualization_Handler visualization_handler;

        Application_Handler();
        
        bool initialize();
        void terminate();
        bool is_running ();
};