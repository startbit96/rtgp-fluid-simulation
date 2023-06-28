#include "visualization_handler.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <sstream>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

#include "../utils/cuboid.h"
#include "../utils/debug.h"

Visualization_Handler::Visualization_Handler ()
{
    // At the start, the pointer to the GLFW window will be set to NULL.
    // Make sure to pass the pointer to the window before calling visualize().
    this->window = nullptr;
    // Calculate the aspect ratio and the projection matrix.
    this->update_window_size(WINDOW_DEFAULT_WIDTH, WINDOW_DEFAULT_HEIGHT);
    // Set the initial draw settings.
    this->draw_simulation_space = false;
    this->draw_fluid_starting_positions = false;
    this->draw_particles = true;
    this->draw_marching_cubes_surface = false;
    this->draw_marching_cubes_grid = false;
    this->draw_marching_cubes_surface_wireframe = false;
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
    this->cursor_position = glm::vec2(0.0f);
    
    // The shaders will be initialized later, because we have to wait for 
    // the OpenGL environment to be set up.
}

void Visualization_Handler::update_window_size (int width, int height)
{
    // Save the window width and height, we need it for the mouse cursor interaction.
    this->window_width = width;
    this->window_height = height;
    // Calculate the aspect ratio and the projection matrix.
    this->aspect_ratio = (float)this->window_width / (float)this->window_height;
    this->projection_matrix = glm::perspective(
        PERSPECTIVE_FOV_DEG, 
        this->aspect_ratio,
        PERSPECTIVE_NEAR, 
        PERSPECTIVE_FAR
    );
}

bool Visualization_Handler::initialize_shaders ()
{
    // Shader for visualizing the cuboids (simulation space, fluid starting positions).
    std::cout << "Compile cuboid shader ..." << std::endl;
    this->cuboid_shader = new Shader("../shaders/cuboid.vert", "../shaders/cuboid.frag");
    if (this->cuboid_shader->is_valid == false) {
        return false;
    }
    // Shaders for visualizing the fluid.
    std::cout << "Compile fluid shader ..." << std::endl;
    this->fluid_shaders = std::vector<Shader> {
        Shader("../shaders/particle.vert", "../shaders/particle.frag", "../shaders/particle.geom")
    };
    for (Shader& shader : this->fluid_shaders) {
        if (shader.is_valid == false) {
            return false;
        }
    }
    this->current_fluid_shader = 0;
    // Shader for the grid of the marching cubes algorithm.
    std::cout << "Compile marching cube grid shader ..." << std::endl;
    this->marching_cube_grid_shader = new Shader("../shaders/marching_cube_grid.vert", "../shaders/marching_cube_grid.frag", 
        "../shaders/marching_cube_grid.geom");
    if (this->marching_cube_grid_shader->is_valid == false) {
        return false;
    }
    // Shader for the marching cubes algorithm.
    std::cout << "Compile marching cube shader ..." << std::endl;
    this->marching_cube_shader = new Shader("../shaders/marching_cube.vert", "../shaders/marching_cube.frag", 
        "../shaders/marching_cube.geom");
    if (this->marching_cube_shader->is_valid == false) {
        return false;
    }
    // If we arrive here, all shaders were read in successfully.
    return true;
}

void Visualization_Handler::delete_shaders ()
{
    // Clean up.
    std::cout << "Delete cuboid shader ..." << std::endl;
    if (this->cuboid_shader != NULL) {
        this->cuboid_shader->delete_program();  
    }
    std::cout << "Delete fluid shader ..." << std::endl;
    for (Shader& shader : this->fluid_shaders) {
        shader.delete_program();
    }
    std::cout << "Delete marching cube grid shader ..." << std::endl;
    if (this->marching_cube_grid_shader != NULL) {
        this->marching_cube_grid_shader->delete_program();
    }
    std::cout << "Delete marching cube shader ..." << std::endl;
    if (this->marching_cube_shader != NULL) {
        this->marching_cube_shader->delete_program();
    }
}

void Visualization_Handler::update_external_force_position ()
{
    // This function calculates from the cursors position the ray vector and gives it to the particle system
    // so for that for every particle the external force can be calculated.
    // Only do this if the particle system is going to use the data.
    if (this->particle_system->external_forces_active == true) {
        // The mouse cursor position are given in pixel values. 
        // We need them to be in normalized device coordinates ranging from [-1; 1].
        float normalized_device_coord_x = (2.0f * this->cursor_position.x) / this->window_width - 1.0f;
        float normalized_device_coord_y = 1.0f - (2.0f * this->cursor_position.y) / this->window_height;
        // From this we can create the position in clip space. The z-value is -1.
        glm::vec4 cursor_position_clip = glm::vec4(normalized_device_coord_x, normalized_device_coord_y, -1.0f, 1.0f);
        // Transform the clip space position to the view space using the inverse projection matrix.
        glm::mat4 projection_matrix_inverse = glm::inverse(this->projection_matrix);
        glm::vec4 cursor_position_view = projection_matrix_inverse * cursor_position_clip;
        cursor_position_view.z = -1.0f;
        cursor_position_view.w = 0.0f;
        // Transform the view space position to world space using the inverse view matrix.
        glm::mat4 view_matrix_inverse = glm::inverse(this->camera.get_view_matrix());
        glm::vec4 cursor_position_world = view_matrix_inverse * cursor_position_view;
        glm::vec3 ray_direction = glm::normalize(glm::vec3(cursor_position_world));
        // Now pass the data to the particle system.
        this->particle_system->camera_position = this->camera.get_camera_position();
        this->particle_system->ray_direction_normalized = ray_direction;
    }
}

void Visualization_Handler::show_imgui_window ()
{
    // Visual settings.
    ImGui::SetNextWindowSize(ImVec2(250, 270), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(750, 170), ImGuiCond_FirstUseEver);
    ImGui::Begin("Visual settings", NULL);
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader("General settings")) {
        ImGui::Checkbox("show simulation space", &this->draw_simulation_space);
        ImGui::Checkbox("show initial fluid position", &this->draw_fluid_starting_positions);
        ImGui::Checkbox("show particles", &this->draw_particles);
    }
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader("Marching cube settings")) {
        ImGui::Checkbox("show marching cube surface", &this->draw_marching_cubes_surface);
        ImGui::Checkbox("wireframe mode", &this->draw_marching_cubes_surface_wireframe);
        ImGui::Checkbox("show grid", &this->draw_marching_cubes_grid);
        ImGui::DragFloat("grid size", &this->marching_cube_generator.new_cube_edge_length, 
            MARCHING_CUBES_CUBE_EDGE_LENGTH_STEP, MARCHING_CUBES_CUBE_EDGE_LENGTH_MIN, MARCHING_CUBES_CUBE_EDGE_LENGTH_MAX, "%.4f");
        ImGui::DragFloat("isovalue", &this->marching_cube_generator.isovalue, 
            MARCHING_CUBES_ISOVALUE_STEP, MARCHING_CUBES_ISOVALUE_MIN, MARCHING_CUBES_ISOVALUE_MAX, "%.3f");
    }
    ImGui::End();

    // Simulation settings.
    ImGui::SetNextWindowSize(ImVec2(250, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    ImGui::Begin("Simulation settings", NULL);
    // Fluid attributes.
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader("Fluid attributes")) {
        ImGui::DragFloat("particle mass", &this->particle_system->sph_particle_mass, 
            SPH_PARTICLE_MASS_STEP, SPH_PARTICLE_MASS_MIN, SPH_PARTICLE_MASS_MAX, "%.6f");
        ImGui::DragFloat("rest density", &this->particle_system->sph_rest_density, 
            SPH_REST_DENSITY_STEP, SPH_REST_DENSITY_MIN, SPH_REST_DENSITY_MAX, "%.6f");
        ImGui::DragFloat("gas constant", &this->particle_system->sph_gas_constant, 
            SPH_GAS_CONSTANT_STEP, SPH_GAS_CONSTANT_MIN, SPH_GAS_CONSTANT_MAX, "%.9f");
        ImGui::DragFloat("viscosity", &this->particle_system->sph_viscosity, 
            SPH_VISCOSITY_STEP, SPH_VISCOSITY_MIN, SPH_VISCOSITY_MAX, "%.6f");
        if (ImGui::Button("reset fluid attributes")) {
            this->particle_system->reset_fluid_attributes();
        }
    }
    // Select the gravity mode.
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader("Gravity mode")) {
        for (int i = 0; i < static_cast<int>(Gravity_Mode::_GRAVITY_MODE_COUNT); i++) {
            if (ImGui::Selectable(to_string(static_cast<Gravity_Mode>(i)), i == this->particle_system->gravity_mode))
                this->particle_system->gravity_mode = static_cast<Gravity_Mode>(i);
        }
    }
    // External force attributes.
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader("External force settings")) {
        ImGui::Checkbox("enable cursor interaction", &this->particle_system->external_forces_active);
        ImGui::DragFloat("radius of interaction", &this->particle_system->external_force_radius, 
            SPH_EXTERNAL_FORCE_RADIUS_STEP, SPH_EXTERNAL_FORCE_RADIUS_MIN, SPH_EXTERNAL_FORCE_RADIUS_MAX, "%.3f");
        // Shall the external force be repellent or attractive?
        if (ImGui::RadioButton("repellent", this->particle_system->external_force_direction == EXTERNAL_FORCE_REPELLENT)) { 
            this->particle_system->external_force_direction = EXTERNAL_FORCE_REPELLENT; 
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("attractive", this->particle_system->external_force_direction == EXTERNAL_FORCE_ATTRACTIVE)) { 
            this->particle_system->external_force_direction = EXTERNAL_FORCE_ATTRACTIVE; 
        }
    }
    // Collision attributes.
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader("Collision attributes")) {
        ImGui::DragFloat("reflexion damping", &this->particle_system->collision_reflexion_damping, 
            SPH_COLLISION_REFLEXION_DAMPING_STEP, SPH_COLLISION_REFLEXION_DAMPING_MIN, SPH_COLLISION_REFLEXION_DAMPING_MAX, "%.3f");
        ImGui::DragFloat("force damping", &this->particle_system->collision_force_damping, 
            SPH_COLLISION_FORCE_DAMPING_STEP, SPH_COLLISION_FORCE_DAMPING_MIN, SPH_COLLISION_FORCE_DAMPING_MAX, "%.3f");
        ImGui::DragFloat("force spring const.", &this->particle_system->collision_force_spring_constant, 
            SPH_COLLISION_FORCE_SPRING_CONSTANT_STEP, SPH_COLLISION_FORCE_SPRING_CONSTANT_MIN, SPH_COLLISION_FORCE_SPRING_CONSTANT_MAX, "%.3f");
        ImGui::DragFloat("force distance tol.", &this->particle_system->collision_force_distance_tolerance, 
            SPH_COLLISION_FORCE_DISTANCE_TOLERANCE_STEP, SPH_COLLISION_FORCE_DISTANCE_TOLERANCE_MIN, SPH_COLLISION_FORCE_DISTANCE_TOLERANCE_MAX, "%.3f");
        if (ImGui::Button("Reset collision attributes")) {
            this->particle_system->reset_collision_attributes();
        }
    }
    ImGui::End();

    // Computational settings.
    ImGui::SetNextWindowSize(ImVec2(250, 140), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(750, 20), ImGuiCond_FirstUseEver);
    ImGui::Begin("Computation settings", NULL);
    // Select the computation mode.
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader("Computation mode")) {
        for (int i = 0; i < static_cast<int>(Computation_Mode::_COMPUTATION_MODE_COUNT); i++) {
            if (ImGui::Selectable(to_string(static_cast<Computation_Mode>(i)), i == this->particle_system->computation_mode)) {
                this->particle_system->change_computation_mode(static_cast<Computation_Mode>(i));
            }
        }
    }
    // Multithreading
    ImGui::SetNextItemOpen(true, ImGuiCond_Appearing);
    if (ImGui::CollapsingHeader("Multithreading")) {
        ImGui::DragInt("Number of threads", &this->particle_system->number_of_threads, 
            0.1f, SIMULATION_NUMBER_OF_THREADS_MIN, SIMULATION_NUMBER_OF_THREADS_MAX, 
            "%d", ImGuiSliderFlags_AlwaysClamp);
    }
    ImGui::End();

    // At last we print the information about the key bindings from the input handler to the screen using imgui.
    ImGui::SetNextWindowSize(ImVec2(320, 240), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(680, 450), ImGuiCond_FirstUseEver);
    ImGui::Begin("Key bindings", NULL);
    // We need some flags for the table. This was copy pasted from the imgui demo.
    static ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | 
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
    if (ImGui::BeginTable("Key binding table", 2, table_flags))
    {
        ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Reaction", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();
        for (int row = 0; row < this->key_list.size(); row++)
        {
            ImGui::TableNextRow();
            // The first column will contain the keys.
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(this->key_list.at(row).c_str());
            // The second column will contain the reaction description.
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(this->reaction_description_list.at(row).c_str());
        }
        ImGui::EndTable();
    }
    ImGui::End();
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
            << this->particle_system->number_of_particles_as_string << " particles  |  "
            << fps << " FPS  |  Computation Mode: "
            << to_string(this->particle_system->computation_mode) << " ]";
        glfwSetWindowTitle(this->window, window_title.str().c_str());
        this->last_time_stamp_fps = current_time_stamp;
        this->frame_counter = 0;
    }
    
    // Get the view matrix.
    glm::mat4 view_matrix = camera.get_view_matrix();

    // Visualize the cuboids if desired.
    if ((this->draw_simulation_space == true)|| (this->draw_fluid_starting_positions == true)) {
        // Wireframe mode.
        GLCall( glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) );
        // Activate the desired shader program.
        this->cuboid_shader->use_program();
        // Set the projection matrix and the view matrix.
        this->cuboid_shader->set_uniform_mat4fv("u_projection_matrix", this->projection_matrix);
        this->cuboid_shader->set_uniform_mat4fv("u_view_matrix", view_matrix);
    }
    // Visualize the simulation space.
    if (this->draw_simulation_space == true) {
        this->cuboid_shader->set_uniform_4fv("u_color", this->color_simulation_space);
        this->simulation_space->draw();
    }
    // Visualize the starting positions of the fluid.
    if (this->draw_fluid_starting_positions == true) {
        for (int i = 0; i < this->fluid_start_positions->size(); i++) {
            this->cuboid_shader->set_uniform_4fv("u_color", this->color_fluid_starting_positions);
            this->fluid_start_positions->at(i).draw();
        }
    }
    // Undo some things that need to be undone one of the cuboids were drawn.
    if ((this->draw_simulation_space == true)|| (this->draw_fluid_starting_positions == true)) {
        // Deactivate wireframe mode.
        GLCall( glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) );
    }

    // Visualize the particles.
    if (this->draw_particles == true) {
        // Activate the desired shader program.
        this->fluid_shaders[this->current_fluid_shader].use_program();
        // Set the projection matrix and the view matrix.
        this->fluid_shaders[this->current_fluid_shader].set_uniform_mat4fv("u_projection_matrix", this->projection_matrix);
        this->fluid_shaders[this->current_fluid_shader].set_uniform_mat4fv("u_view_matrix", view_matrix);
        // Set the aspect ratio.
        this->fluid_shaders[this->current_fluid_shader].set_uniform_1f("u_aspect_ratio", this->aspect_ratio);
        // Draw the particles.
        this->particle_system->draw();
    }

    // Determine if we need to calculate the marching cubes.
    if ((this->draw_marching_cubes_grid == true) || (this->draw_marching_cubes_surface == true)) {
        // Calculate the marching cubes.
        this->marching_cube_generator.generate_marching_cubes();
    }

    // Visualize the marching cubes grid.
    if (this->draw_marching_cubes_grid == true) {
        // Activate the desired shader program.
        this->marching_cube_grid_shader->use_program();
        // Set the projection matrix and the view matrix.
        this->marching_cube_grid_shader->set_uniform_mat4fv("u_projection_matrix", this->projection_matrix);
        this->marching_cube_grid_shader->set_uniform_mat4fv("u_view_matrix", view_matrix);
        // Set the cubes edge length.
        this->marching_cube_grid_shader->set_uniform_1f("u_cube_edge_length", this->marching_cube_generator.cube_edge_length);
        // Draw the grid.
        this->marching_cube_generator.draw();
    }

    // Visualize the surface generated by the marching cubes.
    if (this->draw_marching_cubes_surface == true) {
        // Check if we want to draw in wireframe mode.
        if (this->draw_marching_cubes_surface_wireframe == true) {
            // The marching cubes surface need to be rendered in 
            GLCall( glPolygonMode(GL_FRONT_AND_BACK, GL_LINE) );
        }
        // Activate the desired shader program.
        this->marching_cube_shader->use_program();
        // Set the projection matrix and the view matrix.
        this->marching_cube_shader->set_uniform_mat4fv("u_projection_matrix", this->projection_matrix);
        this->marching_cube_shader->set_uniform_mat4fv("u_view_matrix", view_matrix);
        // Set the cubes edge length.
        this->marching_cube_shader->set_uniform_1f("u_cube_edge_length", this->marching_cube_generator.cube_edge_length);
        // Set the isovalue to be used in the marching cubes algorithm.
        this->marching_cube_shader->set_uniform_1f("u_isovalue", this->marching_cube_generator.isovalue);
        // Draw the surface of the fluid.
        this->marching_cube_generator.draw();
        // Deactivate the wireframe mode if necessary.
        // We only need to deactivate it if the simulation space and the starting positions of the fluid
        // will not be drawn because then the next draw call goes to the particles.
        if ((this->draw_marching_cubes_surface_wireframe == true) && 
            ((this->draw_simulation_space == false) && (this->draw_fluid_starting_positions == false))) {
            GLCall( glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) );
        }
    }

    // Unbind.
    GLCall( glBindVertexArray(0) );

    // Set up and draw the imgui window.
    // Start the Dear ImGui frame.
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    this->show_imgui_window();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap front and back buffers.
    glfwSwapBuffers(this->window);
}

