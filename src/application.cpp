#include <iostream>

#include "application.h"
#include "utils/debug.h"
#include "utils/performance_test.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

// For more details see performance_test.h:
std::unordered_map<std::string, std::vector<long long>> execution_times;

Application_Handler application_handler;

void rtgp_application()
{
    while (application_handler.is_running() == true) {
        // Process input events.
        application_handler.input_handler.check_and_execute();
        // Set the current state. We use next and current state for debugging information.
        application_handler.current_state = application_handler.next_state;
        // What the application has to do depends on the current application state.
        switch (application_handler.current_state) {
            case IDLE:
                // Nothing to do. Wait for user input that changes the state.
                // In the best case, this is not used.
                break;
            case APPLICATION_INITIALIZATION:
                // Initialize the application (e.g. the window).
                std::cout << "Initialize the application." << std::endl;
                if (application_handler.initialize() == false) {
                    // Something went wrong. Terminate the application.
                    application_handler.next_state = APPLICATION_TERMINATION;
                    continue;
                }

                // Initialize the visualization handler.
                std::cout << "Initialize visualization handler." << std::endl;
                if (application_handler.visualization_handler.initialize_shaders() == false) {
                    // Something went wrong. Terminate the application.
                    std::cout << "An error occured while loading the shaders." << std::endl;
                    application_handler.next_state = APPLICATION_TERMINATION;
                    continue;
                }

                // Initialize the simulation handler.
                std::cout << "Initialize simulation handler." << std::endl;
                // Register the available scenes.
                // Remember to also add the scene loading to the input handler below.
                application_handler.simulation_handler.register_new_scene(
                    "cube in the middle",
                    Cuboid(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f),
                    std::vector<Cuboid> {
                        Cuboid(-0.5f, 0.5f, -0.5f, 0.5f, -0.5f, 0.5f)
                    }
                );
                application_handler.simulation_handler.register_new_scene(
                    "cube in the middle (big simulation space)",
                    Cuboid(-3.0f, 3.0f, -1.0f, 3.0f, -1.0f, 1.0f),
                    std::vector<Cuboid> {
                        Cuboid(-0.5f, 0.5f, 1.5f, 2.5f, -0.5f, 0.5f)
                    }
                );
                application_handler.simulation_handler.register_new_scene(
                    "dam break scenario",
                    Cuboid(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f),
                    std::vector<Cuboid> {
                        Cuboid(-1.0f, -0.5f, -1.0f, 1.0f, -1.0f, 1.0f)
                    }
                );
                application_handler.simulation_handler.register_new_scene(
                    "double dam break scenario",
                    Cuboid(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f),
                    std::vector<Cuboid> {
                        Cuboid(-1.0f, -0.5f, -1.0f, 1.0f, -1.0f, 1.0f),
                        Cuboid(0.5f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f)
                    }
                );
                application_handler.simulation_handler.register_new_scene(
                    "drop fall scenario",
                    Cuboid(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f),
                    std::vector<Cuboid> {
                        Cuboid(-1.0f, 1.0f, -1.0f, -0.6f, -1.0f, 1.0f),
                        Cuboid(-0.2f, 0.2f, 0.5f, 0.9f, -0.2f, 0.2f)
                    }
                );
                // Announce the first scene (id = 0) as the next scene to be loaded.
                // It will be loaded in the SIMULATION_INITIALIZATION state, so we do not need to 
                // check here if the scene really exists.
                application_handler.simulation_handler.next_scene_id = 0;

                // Initialize the input handler.
                std::cout << "Initialize input handler." << std::endl;
                // Since we can not provide a pointer to a member function, we use a function that calls the member function.
                glfwSetKeyCallback(application_handler.window, key_callback);
                // Set the callbacks for zoom and rotation (mouse callbacks).
                glfwSetScrollCallback(application_handler.window, scroll_callback);
                glfwSetMouseButtonCallback(application_handler.window, mouse_button_callback);
                glfwSetCursorPosCallback(application_handler.window, cursor_position_callback);
                // Set the callback for the window resize.
                glfwSetFramebufferSizeCallback(application_handler.window, framebuffer_size_callback);
                // Set the different key-reactions for the different contexts. 
                // Currently it would also be enough to just use one context / drop the concept of input context but for future 
                // development we use the concept of input-contexts.
                // IDLE (used for everything but the simulation, basically only the reaction for exiting the application).
                ASSERT( application_handler.input_handler.register_input_context(INPUT_BEHAVIOR_IDLE, "INPUT BEHAVIOR IN SETUP / IDLE MODE:") );
                ASSERT( application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_IDLE, GLFW_KEY_ESCAPE, exit_application, "EXIT APPLICATION") );
                // SIMULATION_RUNNING.
                // This only contains the reaction to the pressed keyboard keys and not for the interaction with the mouse.
                // The interaction with the mouse and the resulting rotation / translation of the camera is handled within the visualization_handler.
                ASSERT( application_handler.input_handler.register_input_context(INPUT_BEHAVIOR_SIMULATION, "INPUT BEHAVIOR DURING SIMULATION:") );
                ASSERT( application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_ESCAPE, exit_application, "EXIT APPLICATION") );
                // Add the loading keys for the scenes.
                ASSERT( application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_1, [] () { switch_scene(0); }, "LOAD SCENE 1") );
                ASSERT( application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_2, [] () { switch_scene(1); }, "LOAD SCENE 2") );
                ASSERT( application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_3, [] () { switch_scene(2); }, "LOAD SCENE 3") );
                ASSERT( application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_4, [] () { switch_scene(3); }, "LOAD SCENE 4") );
                ASSERT( application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_5, [] () { switch_scene(4); }, "LOAD SCENE 5") );
                ASSERT( application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_R, reload_scene, "RELOAD SCENE") );
                // Simulation related input.
                ASSERT( application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_SPACE, pause_resume_simulation, "PAUSE / RESUME THE SIMULATION") );
                ASSERT( application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_S, simulate_one_step, "SIMULATE ONE STEP (IF PAUSED)") );
                ASSERT( application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_UP, increase_number_of_particles, "INCREASE NUMBER OF PARTICLES") );
                ASSERT( application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_DOWN, decrease_number_of_particles, "DECREASE NUMBER OF PARTICLES") );
                // We now want to be able to print this information also in an imgui window. So make the visualization handler aware of the key bindings.
                // We are only interested in the simulation input behavior.
                ASSERT( application_handler.input_handler.create_key_binding_list(INPUT_BEHAVIOR_SIMULATION, 
                    application_handler.visualization_handler.key_list, application_handler.visualization_handler.reaction_description_list) );
                // Activate the IDLE-input-context (this is used for everything but the running simulation).
                if (application_handler.input_handler.change_input_context(INPUT_BEHAVIOR_IDLE) == false) {
                    // Something went wrong. Terminate the application.
                    std::cout << "An error occured while changing the input context." << std::endl;
                    application_handler.next_state = APPLICATION_TERMINATION;
                    continue;
                }
                
                // Print interaction information for the user.
                application_handler.input_handler.print_information();
                // Print informations about the available scenes for the user.
                application_handler.simulation_handler.print_scene_information();

                // If all went good we arrived here. So the next step is to initialize the simulation.
                application_handler.next_state = SIMULATION_INITIALIZATION;
                break;
            case APPLICATION_TERMINATION:
                // Terminate the application.
                std::cout << "Terminate the application." << std::endl;
                // Free GPU ressources.
                for (Scene_Information scene_information: application_handler.simulation_handler.available_scenes) {
                    scene_information.simulation_space.free_gpu_resources();
                    for (Cuboid fluid_starting_position: scene_information.fluid_starting_positions) {
                        fluid_starting_position.free_gpu_resources();
                    }
                }
                application_handler.simulation_handler.particle_system.free_gpu_resources();
                // Delete the shaders.
                application_handler.visualization_handler.delete_shaders();
                // Terminate.
                application_handler.terminate();
                // Save the performance data if wished.
                #ifdef PERFORMANCE_TEST
                save_exection_time_to_csv();
                #endif
                break;
            case SIMULATION_INITIALIZATION:
                // This state is used for loading the new scene or reloading the current scene.
                // When a scene change is requested, the scene_handler checks if the desired scene is available 
                // but does not load it yet. It just saves the desired scene. So load it now.
                if (application_handler.simulation_handler.load_scene() == false) {
                    // Something went wrong. Terminate the application.
                    std::cout << "An error occured while loading the scene." << std::endl;
                    application_handler.next_state = APPLICATION_TERMINATION;
                    continue;
                }
                // Activate the input behavior for the simulation.
                ASSERT( application_handler.input_handler.change_input_context(INPUT_BEHAVIOR_SIMULATION) );
                // Load the references into the visualization handler.
                application_handler.visualization_handler.simulation_space = application_handler.simulation_handler.get_pointer_to_simulation_space();
                application_handler.visualization_handler.fluid_start_positions = application_handler.simulation_handler.get_pointer_to_fluid_starting_positions();
                application_handler.visualization_handler.particle_system = application_handler.simulation_handler.get_pointer_to_particle_system();
                application_handler.visualization_handler.marching_cube_generator.particle_system = application_handler.simulation_handler.get_pointer_to_particle_system();
                application_handler.visualization_handler.marching_cube_generator.simulation_space_changed();
                // Set the point of interest for the camera.
                application_handler.visualization_handler.camera.scene_center = application_handler.simulation_handler.get_current_point_of_interest();
                // The next state will be the running simulation.
                application_handler.next_state = SIMULATION_RUNNING;
                break;
            case SIMULATION_RUNNING:
                // For the users interaction with the fluid, the particle system needs to know the cameras position and the ray
                // casted by the mouse cursor. Update these values.
                application_handler.visualization_handler.update_external_force_position();
                // Calculate the next simulation step.
                MEASURE_EXECUTION_TIME( application_handler.simulation_handler.simulate() );
                // Update the visualization.
                MEASURE_EXECUTION_TIME( application_handler.visualization_handler.visualize() );
                break;
            case SIMULATION_TERMINATION:
                // Load the next requested simulation.
                ASSERT( application_handler.input_handler.change_input_context(INPUT_BEHAVIOR_IDLE) );
                // This application state is only reached when a scene change or a scene reload is requested.
                // Therefore the only possible next application stage is the initialization of the simulation.
                application_handler.next_state = SIMULATION_INITIALIZATION;
                break;
            default:
                // NEVER.
                std::cout << "Error: Not implemented application state detected: " << application_handler.current_state << "." << std::endl;
                application_handler.next_state = APPLICATION_TERMINATION;
                break;
        }
    }
}


// The following functions are needed as callbacks for the GLFW input events.
// Since we can not pass a pointer to a member function, we needed a global object
// of Application_Handler and within the following functions the events are handled.

void reload_scene ()
{
    application_handler.next_state = SIMULATION_INITIALIZATION; 
}

void switch_scene (int scene_id) 
{
    application_handler.simulation_handler.next_scene_id = scene_id; 
    application_handler.next_state = SIMULATION_INITIALIZATION;
}

void pause_resume_simulation ()
{
    application_handler.simulation_handler.toggle_pause_resume_simulation();
}

void simulate_one_step ()
{
    // Only do this if the simulation is paused.
    if (application_handler.simulation_handler.is_running == false) {
        application_handler.simulation_handler.simulate_one_step = true;
    }
}

void increase_number_of_particles ()
{
    bool success = application_handler.simulation_handler.particle_system.increase_number_of_particles();
    if (success == true) {
        // We can increase the number of particles even more. Reload the scene.
        reload_scene();
    }
    else {
        std::cout << "The number of particles cannot be increased further." << std::endl;
    }
}

void decrease_number_of_particles ()
{
    bool success = application_handler.simulation_handler.particle_system.decrease_number_of_particles();
    if (success == true) {
        // We can decrease the number of particles even more. Reload the scene.
        reload_scene();
    }
    else {
        std::cout << "The number of particles cannot be decreased further." << std::endl;
    }
}

void exit_application () 
{
    application_handler.next_state = APPLICATION_TERMINATION;
}


// Callbacks for the GLFW window.

void key_callback (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Only accept the key input for the input handler if the input was not ment for imgui.
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard == true) {
        // Pass the key event to imgui.
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
        return;
    }
    if (action == GLFW_PRESS) {
        application_handler.input_handler.pressed_keys.push_back(key);
    }
}

void scroll_callback(GLFWwindow* window, double x_offset, double y_offset)
{
    // Only allow zooming when the mouse is not on an imgui window.
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse == true) {
        // Pass the scroll event to imgui.
        ImGui_ImplGlfw_ScrollCallback(window, x_offset, y_offset);
        return;
    }
    application_handler.visualization_handler.camera.zoom(y_offset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Stop the rotation in any case (if we are on the simulation window or within 
    // the imgui window... it does not matter).
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        application_handler.visualization_handler.camera.rotation_enabled = false;
    }
    // Does IMGUI want this event? If so, then return.
    auto& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        // Pass the click event to imgui.
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
        return;
    }
    // The rotation will only be activated when the mouse key is pressed and also only
    // if the mouse is NOT on the imgui window (otherwise the function did already return).
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        application_handler.visualization_handler.camera.rotation_enabled = true;
    }
}

void cursor_position_callback(GLFWwindow* window, double x_pos, double y_pos)
{
    // Here we do not return if the cursor is over the imguis window.
    // Since the rotation of the camera is only activated when the user clicks on
    // the simulation window (but not on the imgui window), we do not need to check this here.
    // This also means when an user started to rotate the scene, the rotation will still
    // continue if the mouse hovers over the imgui window.
    ImGui_ImplGlfw_CursorPosCallback(window, x_pos, y_pos);
    application_handler.visualization_handler.camera.rotate(x_pos, y_pos);
    // Needed for applying external forces using the mouse:
    application_handler.visualization_handler.cursor_position.x = x_pos;
    application_handler.visualization_handler.cursor_position.y = y_pos;
}

void framebuffer_size_callback (GLFWwindow* window, int width, int height)
{
    // We need to adapt the viewport for OpenGL.
    glViewport(0, 0, width, height);
    // Also the visualization handler needs to be notified in order to recalculate the 
    // projection matrix. These values need to be divided by the scale factor.
    // For more information see application_handler.h.
    application_handler.visualization_handler.update_window_size(   width / application_handler.scale_factor_x, 
                                                                    height / application_handler.scale_factor_y);
}