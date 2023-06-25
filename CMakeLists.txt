cmake_minimum_required(VERSION 3.12)

# In order that it works with the gcc compiler.
# (this is not needed for the clang compiler)
# https://stackoverflow.com/questions/24380456/how-can-i-make-cmake-use-gcc-instead-of-clang-on-mac-os-x
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

# To satisfy VSCode and Intellisense.
# Set this to release if necessary.
set(CMAKE_BUILD_TYPE "Debug")

project(rtgp_fluid_simulation)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_INSTALL_PREFIX ./build)

# Set compiler flags to suppress deprecated declarations warnings.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
endif()

# Find the required dependencies.
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW 2.2 REQUIRED)
find_package(glm REQUIRED)

# Check if required dependencies are found.
if(NOT OPENGL_FOUND)
    message(FATAL_ERROR "OpenGL package not found")
endif()

if(NOT glfw3_FOUND)
    message(FATAL_ERROR "glfw3 package not found")
endif()

if(NOT GLEW_FOUND)
    message(FATAL_ERROR "GLEW package not found")
endif()

if(NOT glm_FOUND)
    message(FATAL_ERROR "glm package not found")
endif()

# List the source files explicitly.
# It is not recommended to use glob.
# https://stackoverflow.com/questions/1027247/is-it-better-to-specify-source-files-with-glob-or-each-file-individually-in-cmak
set(SOURCE_FILES
    src/main.cpp
    src/application.cpp
    src/application_handler/application_handler.cpp
    src/imgui/imgui_demo.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_impl_glfw.cpp
    src/imgui/imgui_impl_opengl3.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/imgui.cpp
    src/input_handler/input_context.cpp
    src/input_handler/input_handler.cpp
    src/input_handler/input.cpp
    src/simulation_handler/scene_information.cpp
    src/simulation_handler/simulation_handler.cpp
    src/utils/cuboid.cpp
    src/utils/particle_system.cpp
    src/utils/particle.cpp
    src/visualization_handler/camera.cpp
    src/visualization_handler/marching_cubes.cpp
    src/visualization_handler/shader.cpp
    src/visualization_handler/visualization_handler.cpp
    )

# List the include files explicitly.
set(INCLUDE_FILES
    src/application.h
    src/application_handler/application_handler.h
    src/imgui/imconfig.h
    src/imgui/imgui_impl_glfw.h
    src/imgui/imgui_impl_opengl3_loader.h
    src/imgui/imgui_impl_opengl3.h
    src/imgui/imgui_internal.h
    src/imgui/imgui.h
    src/imgui/imstb_rectpack.h
    src/imgui/imstb_textedit.h
    src/imgui/imstb_truetype.h
    src/input_handler/input_context.h
    src/input_handler/input_handler.h
    src/input_handler/input.h
    src/simulation_handler/scene_information.h
    src/simulation_handler/simulation_handler.h
    src/utils/cuboid.h
    src/utils/debug.h
    src/utils/helper.h
    src/utils/particle_system.h
    src/utils/particle.h
    src/visualization_handler/camera.h
    src/visualization_handler/marching_cubes.h
    src/visualization_handler/shader.h
    src/visualization_handler/visualization_handler.h
)

# Our executable.
add_executable(rtgp_fluid_simulation ${SOURCE_FILES} ${INCLUDE_FILES})

# Specify the target-specific include directories.
target_include_directories(rtgp_fluid_simulation PRIVATE
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
)

# Link the required libraries.
target_link_libraries(rtgp_fluid_simulation PRIVATE
    ${OPENGL_LIBRARIES}
    ${GLEW_LIBRARIES}
    glfw
    glm::glm
)

# Install.
install(TARGETS rtgp_fluid_simulation
    DESTINATION ${CMAKE_INSTALL_PREFIX}
)