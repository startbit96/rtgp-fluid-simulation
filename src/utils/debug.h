#pragma once

#include <GL/glew.h>

#include <iostream>
#include <string>
#include <stdint.h>
#include <assert.h>

// ====================================== OPENGL RELATED DEBUG ======================================

// This helps finding errors in the code. If a called function returns false, 
// this raises an error. With this we also get the exact line in the code.
#ifdef OPENGL_DEBUG
#define ASSERT(x) if (!(x)) assert(false)
#else
#define ASSERT(x) x
#endif

// This macro is needed e.g. in the Vertex_Array class to 
// cast an int pointer to an void pointer.
#define INT2VOIDP(i) (void*)(uintptr_t)(i)

// This function clears the OpenGL Error buffer.
void inline GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

// This function creates error messages for the different OpenGL errors.
bool inline GLCheckError()
{
    while (GLenum error = glGetError())
    {
        
        std::cout << "[OpenGL Error] ";
          switch(error) {
              case GL_INVALID_ENUM :
                  std::cout << "GL_INVALID_ENUM : An unacceptable value is specified for an enumerated argument.";
                  break;
              case GL_INVALID_VALUE :
                  std::cout << "GL_INVALID_OPERATION : A numeric argument is out of range.";
                  break;
              case GL_INVALID_OPERATION :
                  std::cout << "GL_INVALID_OPERATION : The specified operation is not allowed in the current state.";
                  break;
              case GL_INVALID_FRAMEBUFFER_OPERATION :
                  std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION : The framebuffer object is not complete.";
                  break;
              case GL_OUT_OF_MEMORY :
                  std::cout << "GL_OUT_OF_MEMORY : There is not enough memory left to execute the command.";
                  break;
              case GL_STACK_UNDERFLOW :
                  std::cout << "GL_STACK_UNDERFLOW : An attempt has been made to perform an operation that would cause an internal stack to underflow.";
                  break;
              case GL_STACK_OVERFLOW :
                  std::cout << "GL_STACK_OVERFLOW : An attempt has been made to perform an operation that would cause an internal stack to overflow.";
                  break;
              default :
                  std::cout << "Unrecognized error" << error;
          }
          std::cout << std::endl;
          return false;
    }
    return true;
}

// This macro / function clears first the error buffer, then calls the desired function
// and then checks if an OpenGL error occured and if so it prints the error message.
#ifdef OPENGL_DEBUG
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLCheckError())
#else
#define GLCall(x) x
#endif