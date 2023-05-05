#include <iostream>

#include "application_handler.h"

Application_Handler::Application_Handler()
{
    this->is_running = true;
    this->current_state = IDLE;
    this->next_state = APPLICATION_INITIALIZATION;
    this->window = nullptr;
    this->scene_handler = Scene_Handler();
    this->input_handler = Input_Handler();
}

bool Application_Handler::initialize_window() 
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        this->window = nullptr;
        return false;
    }
    else {
        std::cout << "Successfully initialized GLFW." << std::endl;
    }

    // Define version and compatibility settings.
    // We have to do this in order to use OpenGL 4.1.
    // Otherwise we will get OpenGL 2.1.
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context.
    this->window = glfwCreateWindow(DEFAULT_WINDOW_WIDTH, 
                                    DEFAULT_WINDOW_HEIGHT, 
                                    DEFAULT_WINDOW_NAME, 
                                    NULL, NULL);
    if (this->window == NULL) {
        std::cerr << "Failed to open GLFW window." << std::endl;
        glfwTerminate();
        this->window = nullptr;
        return false;
    }
    else {
        std::cout << "Successfully opened GLFW window." << std::endl;
    }
    glfwMakeContextCurrent(this->window);

    // Initialize GLEW in order to use Modern OpenGL.
    glewExperimental = true;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW." << std::endl;
        glfwTerminate();
        this->window = nullptr;
        return false;
    }
    else {
        std::cout << "Successfully initialized GLEW." << std::endl;
    }

    std::cout << "Using GL Version: " << glGetString(GL_VERSION) << std::endl;

    // Ensure we can capture the escape key being pressed below.
    glfwSetInputMode(this->window, GLFW_STICKY_KEYS, GL_TRUE);

    return true;
}

bool Application_Handler::initialize ()
{
    // Initialize the window. 
    if (this->initialize_window() == false) {
        return false;
    }
    // Give the visualization handler the pointer to the window.
    this->visualization_handler.window = this->window;
    return true;
}

void Application_Handler::terminate () 
{
    glfwTerminate();
}