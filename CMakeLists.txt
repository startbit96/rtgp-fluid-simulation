cmake_minimum_required(VERSION 3.4)

project(rtgp_fluid_simulation)

set(CMAKE_CXX_FLAGS "-std=c++20")

add_definitions(-Wno-deprecated-declarations)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW 2.2 REQUIRED)
find_package(glm REQUIRED)
find_package(ASSIMP REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

set(INCLUDE_DIRS ${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIRS} ${ASSIMP_INCLUDE_DIRS})
set(LIBRARIES ${OPENGL_LIBRARIES} ${GLEW_LIBRARIES} ${ASSIMP_LIBRARIES})

include_directories(${INCLUDE_DIRS})

file(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.cpp)
file(GLOB INCLUDE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/**/*.h)

add_executable(rtgp_fluid_simulation ${SOURCE_FILES} ${INCLUDE_FILES})

target_link_libraries(rtgp_fluid_simulation ${LIBRARIES} glfw glm::glm nlohmann_json::nlohmann_json)