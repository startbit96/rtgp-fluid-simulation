#include <iostream>

#include "application.h"
#include "utils/debug.h"

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
                // Initialize the input handler.
                std::cout << "Initialize input handler." << std::endl;
                // Since we can not provide a pointer to a member function, we use a function that calls the member function.
                glfwSetKeyCallback(application_handler.window, 
                    [] (GLFWwindow* window, int key, int scancode, int action, int mods) 
                    {
                        if (action == GLFW_PRESS) application_handler.input_handler.pressed_keys.push_back(key);
                    }
                );
                // Set the different key-reactions for the different contexts. 
                // Currently it would also be enough to just use one context / drop the concept of input context but for future 
                // development we use the concept of input-contexts.
                // IDLE (used for everything but the simulation, basically only the reaction for exiting the application).
                ASSERT(application_handler.input_handler.register_input_context(INPUT_BEHAVIOR_IDLE, "INPUT BEHAVIOR IN SETUP / IDLE MODE:"));
                ASSERT(application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_IDLE, GLFW_KEY_ESCAPE, exit_application, "EXIT APPLICATION"));
                // SIMULATION_RUNNING.
                // This only contains the reaction to the pressed keyboard keys and not for the interaction with the mouse.
                // The interaction with the mouse and the resulting rotation / translation of the camera is handled within the visualization_handler.
                ASSERT(application_handler.input_handler.register_input_context(INPUT_BEHAVIOR_SIMULATION, "INPUT BEHAVIOR DURING SIMULATION:"));
                ASSERT(application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_ESCAPE, exit_application, "EXIT APPLICATION"));
                ASSERT(application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_1, [] () { switch_scene(0); }, "LOAD SCENE 1"));
                ASSERT(application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_2, [] () { switch_scene(1); }, "LOAD SCENE 2"));
                ASSERT(application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_3, [] () { switch_scene(2); }, "LOAD SCENE 3"));
                ASSERT(application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_R, reload_scene, "RELOAD SCENE"));
                ASSERT(application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_SPACE, pause_resume_simulation, "PAUSE / RESUME THE SIMULATION"));
                ASSERT(application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_S, toggle_simulation_space_visualization, "SHOW / HIDE SIMULATION SPACE"));
                ASSERT(application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_D, toggle_fluid_starting_positions_visualization, "SHOW / HIDE FLUID STARTING POSITIONS"));
                ASSERT(application_handler.input_handler.add_input_behaviour(INPUT_BEHAVIOR_SIMULATION, GLFW_KEY_D, change_fluid_visualization, "CHANGE FLUID VISUALIZATION MODE"));
                // Set the callbacks for zoom and rotation (mouse callbacks).
                glfwSetScrollCallback(application_handler.window, scroll_callback);
                glfwSetMouseButtonCallback(application_handler.window, mouse_button_callback);
                glfwSetCursorPosCallback(application_handler.window, cursor_position_callback);
                // Activate the IDLE-input-context (this is used for everything but the running simulation).
                if (application_handler.input_handler.change_input_context(INPUT_BEHAVIOR_IDLE) == false) {
                    // Something went wrong. Terminate the application.
                    std::cout << "An error occured while changing the input context." << std::endl;
                    application_handler.next_state = APPLICATION_TERMINATION;
                    continue;
                }
                // Print interaction information for the user.
                application_handler.input_handler.print_information();
                // Initialize the simulation handler.
                std::cout << "Initialize simulation handler." << std::endl;
                // Register the available scenes.
                application_handler.simulation_handler.register_new_scene(
                    "Cube in the middle.",
                    Cuboid(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f),
                    std::vector<Cuboid> {
                        Cuboid(-0.5f, 0.5f, -0.5f, 0.5f, -0.5f, 0.5f)
                    }
                );
                application_handler.simulation_handler.register_new_scene(
                    "Dam break scenario.",
                    Cuboid(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f),
                    std::vector<Cuboid> {
                        Cuboid(-1.0f, -0.5f, -1.0f, 1.0f, -1.0f, 1.0f)
                    }
                );
                application_handler.simulation_handler.register_new_scene(
                    "Double dam break scenario.",
                    Cuboid(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f),
                    std::vector<Cuboid> {
                        Cuboid(-1.0f, -0.5f, -1.0f, 1.0f, -1.0f, 1.0f),
                        Cuboid(0.5f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f)
                    }
                );
                // Announce the first scene (id = 0) as the next scene to be loaded.
                // It will be loaded in the SIMULATION_INITIALIZATION state, so we do not need to 
                // check here if the scene really exists.
                application_handler.simulation_handler.next_scene_id = 0;
                // Print informations about the available scenes for the user.
                application_handler.simulation_handler.print_scene_information();
                // If all went good we arrived here. So the next step is to initialize the simulation.
                application_handler.next_state = SIMULATION_INITIALIZATION;
                // Initialize the visualization handler.
                std::cout << "Initialize visualization handler." << std::endl;
                if (application_handler.visualization_handler.initialize_shaders() == false) {
                    // Something went wrong. Terminate the application.
                    std::cout << "An error occured while loading the shaders." << std::endl;
                    application_handler.next_state = APPLICATION_TERMINATION;
                    continue;
                }
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
                // Delete the shaders.
                // ...
                application_handler.terminate();
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
                application_handler.input_handler.change_input_context(INPUT_BEHAVIOR_SIMULATION);
                // Load the references into the visualization handler.
                application_handler.visualization_handler.simulation_space = application_handler.simulation_handler.get_pointer_to_simulation_space();
                application_handler.visualization_handler.fluid_start_positions = application_handler.simulation_handler.get_pointer_to_fluid_starting_positions();
                application_handler.visualization_handler.particle_system = application_handler.simulation_handler.get_pointer_to_particle_system();
                // Set the point of interest for the camera.
                application_handler.visualization_handler.camera.scene_center = application_handler.simulation_handler.get_current_point_of_interest();
                // The next state will be the running simulation.
                application_handler.next_state = SIMULATION_RUNNING;
                break;
            case SIMULATION_RUNNING:
                // Calculate the next simulation step.
                // ...
                // Update the visualization.
                application_handler.visualization_handler.visualize();
                break;
            case SIMULATION_TERMINATION:
                // Load the next requested simulation.
                application_handler.input_handler.change_input_context(INPUT_BEHAVIOR_IDLE);
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

}

void exit_application () 
{
    application_handler.next_state = APPLICATION_TERMINATION;
}

void toggle_simulation_space_visualization ()
{
    application_handler.visualization_handler.toggle_simulation_space_visualization();
}

void toggle_fluid_starting_positions_visualization ()
{
    application_handler.visualization_handler.toggle_fluid_starting_positions_visualization();
}

void change_fluid_visualization ()
{
    application_handler.visualization_handler.change_fluid_visualization();
}

void scroll_callback(GLFWwindow* window, double x_offset, double y_offset)
{
    application_handler.visualization_handler.camera.zoom(y_offset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        application_handler.visualization_handler.camera.rotation_enabled = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        application_handler.visualization_handler.camera.rotation_enabled = false;
    }
}

void cursor_position_callback(GLFWwindow* window, double x_pos, double y_pos)
{
    application_handler.visualization_handler.camera.rotate(x_pos, y_pos);
}