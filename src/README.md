# RTGP Fluid Simulation Code Overview

## UML Class Diagram

See the current version of the UML class diagram in `doc/uml_visualization`.

Note that this may not be the currently implemented class visualization since the UML diagram is updated irregularly.

## Classes

### Application_Handler

The class `Application_Handler` contains an object of all the following classes (`Input_Handler`, `Scene_Handler`, `Simulation_Handler`, `Visualization_Handler`).

An object of this class is used to manage all the ressources needed for the application.
It also saves the current und next application state that will be used in the `application.cpp` file.

### Input_Handler

The class `Input_Handler` manages keyboard inputs and executes the reaction-functions for each pressed key.

It consists of different `Input_Context` objects. Depending on the current application state, the `Input_Context` will be switched in order to react different on each application state.
Currently the concept of input contexts is over-the-top since it is not needed yet. But in order to be prepared for future development it is integrated now.

The `Input_Handler` does not know of the GLFW-window. The task to fill the vector of pressed keys is done by the `application.cpp`since we can not pass a member function to the GLFW event.

### Scene_Handler

The class `Scene_Handler` manages the different simulation scenes that are implemented and loads the objects needed for this scene.

A scene consists of 3D objects that represent obstacles for the fluid (e.g. a cube), the simulation space and also the starting positions of the fluid.

The starting positions of the fluid are also 3D objects that are going to be filled with particles using the Poisson Disk Sampling algorithm.

### Simulation_Handler

The class `Simulation_Handler` is responsible for the calculation of the physics part (the fluid simulation) and also manages collision handling. It updates the position and other properties of the particles.

It is possible to switch between CPU- and GPU-implementations of the algorithms.

### Visualization_Handler

The class `Visualization_Handler` manages the visualization of the results in OpenGL.

We load each obj-file using ASSIMP and create a `Model`-object that contains their Vertex_Array, Vertex_Buffer and Index_Buffer.

The used shader is not model-specific but model-type-specific (simulation space, obstacle, fluid).

The `Renderer` is a function that takes in a `Model`-object and a `Shader`-object and draws the model using the shader.

The `Visualization_Handler` has a pointer to the simulation space model, a pointer to a vector of obstacle models and a pointer to the fluid particles (all handled and loaded by the `Scene_Handler`).

The `Visualization_Handler` therefore does not load the new models but only visualizes them and switches between the different shaders depending on what the user wants to see.

## Application Loop.

The application loop is implemented in the function `rtgp_application` in the file `application.cpp`.

It consists of a while loop that executes the functions depending on the current application state.

How is the fps handled? Fixed? Adjusting fps to workload?