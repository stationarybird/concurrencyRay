# Real-Time Ray Tracer

A real-time ray tracing application built in C++ using OpenGL/GLFW, featuring multi-threaded rendering and interactive camera controls.

## Features

- Real-time ray tracing with interactive camera controls
- Multi-threaded rendering using a custom thread pool
- Progressive refinement rendering for better performance
- Basic lighting with ambient and diffuse components
- Interactive camera controls (WASD for movement, mouse for looking around)
- Fullscreen display support

## Technical Details

- Built with C++ and OpenGL/GLFW
- Implements sphere-ray intersection testing
- Uses a modular architecture with separate components for:
  - Vector math (Vec3)
  - Ray tracing (Ray)
  - Scene management (Scene)
  - Camera controls (Camera)
  - Geometry (Sphere)
- Multi-threaded rendering for optimal performance
- Progressive refinement rendering (lower resolution with scaling)

## Building the Project

### Prerequisites
- C++ compiler with C++11 support
- CMake
- OpenGL
- GLFW

### Build Steps
```bash
mkdir build
cd build
cmake ..
make
```

## Controls
- WASD: Camera movement
- Mouse: Look around
- ESC: Exit

## Project Structure
```
concurrencyRay/
├── include/
│   ├── camera.hpp
│   ├── ray.hpp
│   ├── vec3.hpp
│   ├── sphere.hpp
│   └── scene.hpp
├── src/
│   └── main.cpp
└── CMakeLists.txt
```

## License
This project is open source and available under the MIT License.
