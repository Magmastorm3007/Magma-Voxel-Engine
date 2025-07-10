cmake_minimum_required(VERSION 3.28.3)
project(magma-voxel)

set(CMAKE_CXX_STANDARD 20)

# Find GLFW and GLM from vcpkg
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# Include glad headers manually
include_directories(
        external/glad/include
        ${GLFW_INCLUDE_DIRS}
        ${GLM_INCLUDE_DIRS}
)

# Add GLAD source manually
set(GLAD_SOURCES external/glad/src/gl.c)

# Add your executable with GLAD source included
add_executable(magma-voxel
        ${GLAD_SOURCES}
        src/main.cpp
        src/shader.cpp
        src/shader.h
        src/cube_renderer.cpp
        src/cube_renderer.h
)

# Link to libraries
target_link_libraries(magma-voxel
        glfw
        ${CMAKE_DL_LIBS}
)
