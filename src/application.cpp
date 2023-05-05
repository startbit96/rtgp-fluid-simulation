#include <iostream>

#include "application.h"
#include "utils/debug.h"

Application_Handler application_handler;

void rtgp_application()
{
    while ((application_handler.is_running == true) && (glfwWindowShouldClose(application_handler.window) == false)) {
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
                // Activate the IDLE-input-context (this is used for everything but the running simulation).
                if (application_handler.input_handler.change_input_context(INPUT_BEHAVIOR_IDLE) == false) {
                    // Something went wrong. Terminate the application.
                    std::cout << "An error occured while changing the input context." << std::endl;
                    application_handler.next_state = APPLICATION_TERMINATION;
                    continue;
                }
                // Print interaction information for the user.
                application_handler.input_handler.print_information();
                // Register the available scenes.
                application_handler.scene_handler.register_new_scene(
                    "scene 1",
                    std::vector<std::string> {"test"},
                    std::vector<std::string> {"test"}
                );
                application_handler.scene_handler.register_new_scene(
                    "scene 2",
                    std::vector<std::string> {"test"},
                    std::vector<std::string> {"test"}
                );
                application_handler.scene_handler.register_new_scene(
                    "scene 3",
                    std::vector<std::string> {"test"},
                    std::vector<std::string> {"test"}
                );
                // Announce the first scene (id = 0) as the next scene to be loaded.
                application_handler.scene_handler.next_scene_id = 0;
                // Print informations about the available scenes for the user.
                application_handler.scene_handler.print_information();
                // If all went good we arrived here. So the next step is to initialize the simulation.
                application_handler.next_state = SIMULATION_INITIALIZATION;
                break;
            case APPLICATION_TERMINATION:
                // Terminate the application.
                std::cout << "Terminate the application." << std::endl;
                application_handler.terminate();
                application_handler.is_running = false;
                break;
            case SIMULATION_INITIALIZATION:
                // This state is used for loading the new scene or reloading the current scene.
                // When a scene change is requested, the scene_handler checks if the desired scene is available 
                // but does not load it yet. It just saves the desired scene. So load it now.
                if (application_handler.scene_handler.load_scene() == false) {
                    // Something went wrong. Terminate the application.
                    std::cout << "An error occured while loading the scene." << std::endl;
                    application_handler.next_state = APPLICATION_TERMINATION;
                    continue;
                }
                // Activate the input behavior for the simulation.
                application_handler.input_handler.change_input_context(INPUT_BEHAVIOR_SIMULATION);
                // The next state will be the running simulation.
                application_handler.next_state = SIMULATION_RUNNING;
                break;
            case SIMULATION_RUNNING:
                // Calculate the next simulation step.
                application_handler.simulation_handler.calculate_next_simulation_step();
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


void reload_scene ()
{
    application_handler.next_state = SIMULATION_INITIALIZATION; 
}

void switch_scene (int scene_id) 
{
    application_handler.scene_handler.next_scene_id = scene_id; 
    application_handler.next_state = SIMULATION_INITIALIZATION;
}

void pause_resume_simulation ()
{

}

void exit_application () 
{
    application_handler.next_state = APPLICATION_TERMINATION;
}