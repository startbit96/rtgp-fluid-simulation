#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "camera.h"
#include "../utils/cuboid.h"
#include "../utils/particle_system.h"

// Perspective related defines.
#define WINDOW_DEFAULT_WIDTH        1024
#define WINDOW_DEFAULT_HEIGHT       768
#define PERSPECTIVE_FOV_DEG         45.0f
#define PERSPECTIVE_NEAR            0.1f
#define PERSPECTIVE_FAR             10000.0f
// Color related defines.
// Background.
#define BACKGROUND_COLOR_R          0.91f
#define BACKGROUND_COLOR_G          0.93f
#define BACKGROUND_COLOR_B          0.93f
#define BACKGROUND_COLOR_A          1.0f
// Simulation space.
#define SIMULATION_SPACE_COLOR_R    1.0f
#define SIMULATION_SPACE_COLOR_G    0.0f
#define SIMULATION_SPACE_COLOR_B    0.0f
#define SIMULATION_SPACE_COLOR_A    1.0f
// Fluid starting positions.
#define FLUID_STARTING_POS_COLOR_R  0.0f
#define FLUID_STARTING_POS_COLOR_G  1.0f
#define FLUID_STARTING_POS_COLOR_B  0.0f
#define FLUID_STARTING_POS_COLOR_A  1.0f


class Visualization_Handler 
{
    private:
        // The shader used for the simulation space and for the 
        // starting positions of the fluid (each of them is a cuboid).
        // We will set different colors but they use the same shaders program.
        Shader* cuboid_shader;
        // For the fluid we may have different shaders which we can switch during
        // the execution of the program. Therefore save them in a vector.
        std::vector<Shader> fluid_shaders;
        unsigned int current_fluid_shader;
        // The projection matrix. It will use the values defined above in the define section.
        float aspect_ratio;
        glm::mat4 projection_matrix;
        // Some settings regarding what has to be drawn and what.
        bool draw_simulation_space;
        bool draw_fluid_starting_positions;
        // Color settings.
        glm::vec4 color_simulation_space;
        glm::vec4 color_fluid_starting_positions;

    public:
        // A reference to the GLFW window. It will be created in the application handler and
        // the reference to it will be passed down to the visualization handler.
        GLFWwindow* window;
        // A pointer to the current simulation space and also to the starting positions of the fluid. 
        // They are stored within the scene handler.
        Cuboid *simulation_space;
        std::vector<Cuboid> *fluid_start_positions;
        Particle_System *particle_system;
        unsigned int vao_particles;
        unsigned int number_of_particles;
        // Our camera that handles the calculation of the view matrix.
        // It is an arc ball camera.
        Camera camera;

        Visualization_Handler ();

        bool initialize_shaders ();
        void toggle_simulation_space_visualization ();
        void toggle_fluid_starting_positions_visualization ();
        void change_fluid_visualization ();
        void visualize ();
};