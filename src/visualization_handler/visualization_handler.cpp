#include "visualization_handler.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <sstream>

#include "../utils/cuboid.h"
#include "../utils/debug.h"

Visualization_Handler::Visualization_Handler ()
{
    // At the start, the pointer to the GLFW window will be set to NULL.
    // Make sure to pass the pointer to the window before calling visualize().
    this->window = nullptr;
    // Calculate the projection matrix once.
    this->aspect_ratio = (float)WINDOW_DEFAULT_WIDTH / (float)WINDOW_DEFAULT_HEIGHT;
    this->projection_matrix = glm::perspective(
        PERSPECTIVE_FOV_DEG, 
        this->aspect_ratio,
        PERSPECTIVE_NEAR, 
        PERSPECTIVE_FAR
    );
    this->draw_simulation_space = true;
    this->draw_fluid_starting_positions = true;
    this->color_simulation_space = glm::vec4(
        SIMULATION_SPACE_COLOR_R,
        SIMULATION_SPACE_COLOR_G,
        SIMULATION_SPACE_COLOR_B,
        SIMULATION_SPACE_COLOR_A
    );
    this->color_fluid_starting_positions = glm::vec4(
        FLUID_STARTING_POS_COLOR_R,
        FLUID_STARTING_POS_COLOR_G,
        FLUID_STARTING_POS_COLOR_B,
        FLUID_STARTING_POS_COLOR_A
    );
    this->last_time_stamp = glfwGetTime();
    this->last_time_stamp_fps = this->last_time_stamp;
    this->frame_counter = 0;
    
    // The shaders will be initialized later, because we have to wait for 
    // the OpenGL environment to be set up.
}

bool Visualization_Handler::initialize_shaders ()
{
    // Shader for visualizing the cuboids (simulation space, fluid starting positions).
    this->cuboid_shader = new Shader("./shaders/cuboid.vert", "./shaders/cuboid.frag");
    if (this->cuboid_shader->is_valid == false) {
        return false;
    }
    // Shaders for visualizing the fluid.
    this->fluid_shaders = std::vector<Shader> {
        Shader("./shaders/particle.vert", "./shaders/particle.frag", "./shaders/particle.geom")
    };
    this->current_fluid_shader = 0;
    // If we arrive here, all shaders were read in successfully.
    return true;
}

void Visualization_Handler::toggle_simulation_space_visualization ()
{
    this->draw_simulation_space = !this->draw_simulation_space;
}

void Visualization_Handler::toggle_fluid_starting_positions_visualization ()
{
    this->draw_fluid_starting_positions = !this->draw_fluid_starting_positions;
}

void Visualization_Handler::change_fluid_visualization ()
{
    // ...
}

void Visualization_Handler::visualize ()
{
    // Poll for events and process them.
    glfwPollEvents();
    // "Clear" the frame and z buffer.
    GLCall( glClearColor(BACKGROUND_COLOR_R, BACKGROUND_COLOR_G, BACKGROUND_COLOR_B, BACKGROUND_COLOR_A) );
    GLCall( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );

    // Calculate the time that has passed and update it in the camera object.
    float current_time_stamp = glfwGetTime();
    this->camera.delta_time = current_time_stamp - this->last_time_stamp;
    this->last_time_stamp = current_time_stamp;

    // Update the fps and print it if a specific time has passed.
    this->frame_counter++;
    if ((current_time_stamp - this->last_time_stamp_fps) >= FPS_UPDATE_INTERVAL) {
        float fps = float(this->frame_counter) / (current_time_stamp - this->last_time_stamp_fps);
        std::stringstream window_title;
        window_title << WINDOW_DEFAULT_NAME << "  [ " 
            << this->particle_system->number_of_particles << " particles  |  "
            << fps << " FPS ]";
        glfwSetWindowTitle(this->window, window_title.str().c_str());
        this->last_time_stamp_fps = current_time_stamp;
        this->frame_counter = 0;
    }
    
    // Visualize the cuboids if desired.
    if ((this->draw_simulation_space == true)|| (this->draw_fluid_starting_positions == true)) {
        // Wireframe mode.
        GLCall( glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) );
        // Activate the desired shader program.
        this->cuboid_shader->use_program();
        // Set the projection matrix.
        GLCall( glUniformMatrix4fv(glGetUniformLocation(
            this->cuboid_shader->program_id, "u_projection_matrix"), 1, GL_FALSE, 
            glm::value_ptr(this->projection_matrix)) );
        // Set the viewmatrix.
        glm::mat4 view_matrix = camera.get_view_matrix();
        GLCall( glUniformMatrix4fv(glGetUniformLocation(
            this->cuboid_shader->program_id, "u_view_matrix"), 1, GL_FALSE, 
            glm::value_ptr(view_matrix)) );
    }
    // Visualize the simulation space.
    if (this->draw_simulation_space == true) {
        GLCall( glUniform4fv(glGetUniformLocation(
            this->cuboid_shader->program_id, "u_color"), 1, 
            glm::value_ptr(this->color_simulation_space)) );
        this->simulation_space->draw();
    }
    // Visualize the starting positions of the fluid.
    if (this->draw_fluid_starting_positions == true) {
        for (int i = 0; i < this->fluid_start_positions->size(); i++) {
            GLCall( glUniform4fv(glGetUniformLocation(
                this->cuboid_shader->program_id, "u_color"), 1, 
                glm::value_ptr(this->color_fluid_starting_positions)) );
            this->fluid_start_positions->at(i).draw();
        }
    }

    // Visualize the particles.
    // Deactivate wireframe mode.
    GLCall( glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) );
    // Activate the desired shader program.
    this->fluid_shaders[this->current_fluid_shader].use_program();
    // Set the projection matrix.
    GLCall( glUniformMatrix4fv(glGetUniformLocation(
        this->fluid_shaders[this->current_fluid_shader].program_id, "u_projection_matrix"), 1, GL_FALSE, 
        glm::value_ptr(this->projection_matrix)) );
    // Set the viewmatrix.
    glm::mat4 view_matrix = camera.get_view_matrix();
    GLCall( glUniformMatrix4fv(glGetUniformLocation(
        this->fluid_shaders[this->current_fluid_shader].program_id, "u_view_matrix"), 1, GL_FALSE, 
        glm::value_ptr(view_matrix)) );
    // Set the aspect ratio.
    GLCall( glUniform1f(glGetUniformLocation(
            this->fluid_shaders[this->current_fluid_shader].program_id, "aspectRatio"), 
            this->aspect_ratio) );

    // Draw the particles.
    this->particle_system->draw();
    // Unbind.
    GLCall( glBindVertexArray(0) );
    // Swap front and back buffers.
    glfwSwapBuffers(this->window);
}

