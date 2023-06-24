#include <iostream>

#include "application_handler.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

Application_Handler::Application_Handler()
{
    this->_is_running = true;
    this->current_state = IDLE;
    this->next_state = APPLICATION_INITIALIZATION;
    this->window = nullptr;
    this->simulation_handler = Simulation_Handler();
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
    // Allow the window to resize.
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // Create a windowed mode window and its OpenGL context.
    this->window = glfwCreateWindow(WINDOW_DEFAULT_WIDTH, 
                                    WINDOW_DEFAULT_HEIGHT, 
                                    WINDOW_DEFAULT_NAME, 
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

    // Define the viewport dimensions.
    int width, height;
    glfwGetFramebufferSize(this->window, &width, &height);
    glViewport(0, 0, width, height);

    // From these values we can calculate the scale factors.
    this->scale_factor_x = float(width) / float(WINDOW_DEFAULT_WIDTH);
    this->scale_factor_y = float(height) / float(WINDOW_DEFAULT_HEIGHT);

    // Enable z-test.
    glEnable(GL_DEPTH_TEST);

    // Initialize imgui.
    ImGui::CreateContext();
    // Setup Dear ImGui style.
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(this->window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    return true;
}

bool Application_Handler::initialize ()
{
    // Initialize the window. 
    if (this->initialize_window() == false) {
        return false;
    }
    // Pass the pointer to the GLFW window down to the visualization handler.
    this->visualization_handler.window = this->window;
    return true;
}

void Application_Handler::terminate () 
{
    // Stop the application.
    this->_is_running = false;
    // Cleanup imgui.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // Terminate GLFW.
    glfwDestroyWindow(this->window);
    glfwTerminate();
}

bool Application_Handler::is_running ()
{
    // Check if the window was closed by the user.
    // If the window was closed, switch the application stage to APPLICATION_TERMINATION
    // so that the terminate() function is going to be called.
    // Do not set the _is_running value on false yet. This is done by the terminate() function.
    if (glfwWindowShouldClose(this->window) == true) {
        this->next_state = APPLICATION_TERMINATION;
    }
    return this->_is_running;
}