cmake_minimum_required(VERSION 3.12)
project(raytracer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Add executable
add_executable(raytracer 
    src/main.cpp
)

# Link libraries
target_link_libraries(raytracer PRIVATE
    Threads::Threads
    OpenGL::GL
    glfw
)

# Include directories
target_include_directories(raytracer PRIVATE 
    include
    ${GLFW3_INCLUDE_DIR}
) 