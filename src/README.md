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

### Scene_Handler

The class `Scene_Handler` manages the different simulation scenes that are implemented and loads the objects needed for this scene.

A scene consists of 3D objects that represent obstacles for the fluid (e.g. a cube), the simulation space and also the starting positions of the fluid.

The starting positions of the fluid are also 3D objects that are going to be filled with particles using the Poisson Disk Sampling algorithm.

### Simulation_Handler

The class `Simulation_Handler` is responsible for the calculation of the physics part (the fluid simulation) and also manages collision handling. It updates the position and other properties of the particles.

It is possible to switch between CPU- and GPU-implementations of the algorithms.

### Visualization_Handler

The class `Visualization_Handler` manages the visualization of the results in OpenGL.