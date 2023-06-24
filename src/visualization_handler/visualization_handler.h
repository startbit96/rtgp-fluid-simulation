#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "camera.h"
#include "../utils/cuboid.h"
#include "../utils/particle_system.h"
#include "marching_cubes.h"

// Project related defines.
#define WINDOW_DEFAULT_NAME         "RTGP - Fluid Simulation"
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
// FPS Visualization.
#define FPS_UPDATE_INTERVAL         1.0f


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
        // The shader for the marching cubes grid and the marching cubes generated surfaces.
        Shader* marching_cube_grid_shader;
        Shader* marching_cube_shader;
        // The projection matrix. It will use the values defined above in the define section.
        int window_width;
        int window_height;
        float aspect_ratio;
        glm::mat4 projection_matrix;
        // Some settings regarding what has to be drawn and what.
        // Note that if the drawing of the marching cubes is not wanted, they are not calculated,
        // what means, the computational costs sink and the fps will rise. Note that we do not differ between 
        // the surface drawing or the grid drawing. As long one of these are desired to be drawn, the marching
        // cubes are calculated.
        // This does not apply to the particles. If they shall not be drawn, the will simply not be drawn but they will
        // be simulated by the simulation handler. This has two reasons: First, the visualization handler
        // is not in charge of the simulation handler that calculates this simulation and also if the 
        // marching cubes shall be visualized but not the particles, the marching cubes still need the 
        // updated particles positions.
        // To pause the simulation use the function provided by the simulation handler.
        bool draw_simulation_space;
        bool draw_fluid_starting_positions;
        bool draw_particles;
        bool draw_marching_cubes_surface;
        bool draw_marching_cubes_grid;
        // Color settings.
        glm::vec4 color_simulation_space;
        glm::vec4 color_fluid_starting_positions;
        // Time and fps calculation.
        float last_time_stamp;
        float last_time_stamp_fps;
        unsigned int frame_counter;

        // Imgui window.
        void show_imgui_window ();

    public:
        // A reference to the GLFW window. It will be created in the application handler and
        // the reference to it will be passed down to the visualization handler.
        GLFWwindow* window;
        // A pointer to the current simulation space and also to the starting positions of the fluid. 
        // They are stored within the scene handler.
        Cuboid *simulation_space;
        std::vector<Cuboid> *fluid_start_positions;
        Particle_System *particle_system;
        // Our camera that handles the calculation of the view matrix.
        // It is an arc ball camera.
        Camera camera;

        // Marching cubes generator.
        Marching_Cubes_Generator marching_cube_generator;

        Visualization_Handler ();

        // For external forces we need to know where the mouse cursor is at any time so
        // we can also call the update function below at any time.
        glm::vec2 cursor_position;
        // This function does two things. First, it gives the particle system the position
        // of the camera. Second, it calculates based on the x and y positions of the cursor
        // a ray (vector) where the mouse cursor is pointing using the inverse of the view and 
        // projection matrices.
        void update_external_force_position ();

        // Shader related functions.
        bool initialize_shaders ();
        void delete_shaders ();

        // A function to update the window width and height. We need this for the projection matrix.
        void update_window_size (int width, int height);

        // Visualize everything (particles, simulation space, imgui window, ...).
        void visualize ();
};