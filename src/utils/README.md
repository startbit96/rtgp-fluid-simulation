# Rendering objects using OpenGL

## Model loading

## Vertex Specification in OpenGL

This section explains the basics of vertex specification in OpenGL using Vertex Buffer Objects (VBOs), Index Buffer Objects (IBOs) and Vertex Array Objects (VAOs). For more details see the [OpenGL Documentation](https://www.khronos.org/opengl/wiki/Vertex_Specification).

### Vertex Buffer Object (VBO)

A Vertex Buffer Object (VBO) stores the actual vertex data.  
It is a memory buffer that stores vertex data, such as position, color, normal vectors and texture coordinates. Vertex Buffer Objects allow to send large batches of vertex data to the GPU in a single transfer which reduces overhead in comparison to sending vertex data to the GPU on a per-vertex basis.

### Index Buffer Object (IBO)

An Index Buffer Object holds indices that reference vertices in a Vertex Buffer Object.  
Instead of duplicating vertex data for each triangle, this approach defines a triangle by indices of vertices in the Vertex Buffer Object that should be connected to form a triangle. This method is known as indexed rendering and can help reduce memory usage and improve rendering performance.

### Vertex Array Object (VAO)

Vertex Array Objects manage the configuration and binding of Vertex Buffer Objects and Index Buffer Objects along with other settings required for rendering.  
This object encapsulates the setup and configuration of vertex attributes. It defines the format of the vertex data stored in the VBOs and the association between vertex attributes and shader variables (Vertex Buffer Layout). For example, if the Vertex Buffer Object contains an array of self-defined Struct-objects called Vertex with position, normal, color, we have to define how this Struct is structured and on what position (stride, byte offset in the object) the informations are stored.  
A VAO keeps track of the bindings of VBOs, IBOs and other state-related settings neccessary to render objects efficiently. By binding a VAO, we can easily switch between different sets of vertex attributes and buffer during rendering without having to reconfigure them each time or binding the VBOs and IBOs each time.

## Implemented classes

The class `Mesh` holds the buffer informations of an object (VBO, IBO, VAO). We can create a `Mesh`-object by loading it using the `Model`-class that loads an *.obj-file using the ASSIMP-library.  
Within the `Mesh`-objects Vertex Array Object we will also need the informations about the vertex buffer offset and stride (Vertex Buffer Layout). Since in this project we will not only render meshes that store vertices in their VBOs but also particles with different informations, the task of providing the Vertex Buffer Layout is outsourced to the `Vertex_Buffer_Layout` class that implements a function that returns the layout for different Structs (`Vertex` and `Particle`).  
Also, the rendering of a `Mesh`-object is outsourced to the `Visualization_Handler` that holds informations about the shaders to be used and therefore organizes the binding and rendering of the objects.

## Debugging

OpenGL does not automatically throw error messages when non-valid arguments are passed. Instead, it relies on error flags that a programmer can query using the `glGetError` function to check for errors after certain OpenGL calls. When an error occurs, it sets an error flag and we can retrieve the specific error code to identify the problem.  
This is handled within the `debug.h` file. It implements a macro called `GLCall` that first deletes all previous raised errors, then calls the desired function and afterwards checks if the error flag was set. If so, the error message will be printed. The debugging code in `debug.h` is from the [OpenGL tutorial of The Cherno](https://www.youtube.com/playlist?list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2) from YouTube.
For more details check the [OpenGL Documentation](https://www.khronos.org/opengl/wiki/Debug_Output).