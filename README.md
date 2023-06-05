# rtgp-fluid-simulation
Real-time fluid simulation made in C++ using OpenGL for the course ["Real-Time Graphics Programming"](https://www.unimi.it/en/education/degree-programme-courses/2023/real-time-graphics-programming) at the [Università degli Studi di Milano Statale](https://www.unimi.it/en).

*To do: include GIF of application here.*

## Notes To Do
- change the C++ types to OpenGL Types (where OpenGL is used)
- add enough comments
- delete shaders at the end
- input handler for zooming
- is there a way to make the relative paths more robust?
- everything private that can be private?
- save termination of the program if the shaders could no be loaded
- add keys for scene switch depending on number of registered scenes
- do we need to make the values for the hash function positive?

## Dependencies

* Graphics Library Framework GLFW Version 3 (glfw3)  
* OpenGL Extension Wrangler Library (GLEW)  
* OpenGL Mathematics (GLM)  

Install the dependencies by following the installation instructions on the respective websites (listed below) or by using vcpkg or your package-manager (e.g. homebrew or APT).

### Graphics Library Framework GLFW Version 3 (glfw3)
GLFW is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan development on the desktop. It provides a simple API for creating windows, contexts and surfaces, receiving input and events.  
Click [here](https://www.glfw.org/) for more informations.

### OpenGL Extension Wrangler Library (GLEW)
The OpenGL Extension Wrangler Library (GLEW) is a cross-platform open-source C/C++ extension loading library. GLEW provides efficient run-time mechanisms for determining which OpenGL extensions are supported on the target platform.  
Click [here](https://glew.sourceforge.net/) for more informations.

### OpenGL Mathematics (GLM)
OpenGL Mathematics (GLM) is a header only C++ mathematics library for graphics software based on the OpenGL Shading Language (GLSL) specifications.  
Click [here](https://github.com/g-truc/glm) for more informations.


## Installation

Clone the repository.  

```
$ git clone https://github.com/startbit96/rtgp-fluid-simulation.git
```

Change into the directory of the repository and install it using cmake.

```
$ cd rtgp-fluid-simulation  
$ mkdir build  
$ cd build  
$ cmake ..  
$ make  
```

If an error occurs while installing the application, make sure, that you have installed all the mentioned dependencies.

## Usage

Run the application with `./rtgp_fluid_simulation` within the build directory.

### Command Line Arguments

### Key Bindings

### Implemented Scenes


## Benchmarking
fps, different amount of particles, ...

## References
The used method for the fluid simulation is based on the paper ["Particle-based fluid simulation for interactive applications" from Mueller et al.](https://dl.acm.org/doi/10.5555/846276.846298) from 2003.  

The OpenGL related code is based on the [YouTube OpenGL-tutorial of The Cherno](https://www.youtube.com/playlist?list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2) and on the lab-lessons of the "Real-Time Graphics Programming Course".