# Ray Tracer

A real-time ray tracing application built in C++ using OpenGL/GLFW, featuring multi-threaded rendering and interactive camera controls.

https://github.com/user-attachments/assets/7da4012d-1259-46fb-9230-f258f0d5ff93

## Features

- Real-time ray tracing with interactive camera controls
- Multi-threaded rendering using a custom thread pool
- Basic lighting
- Interactive camera controls (WASD for movement, mouse for looking around)

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

### Build Steps
```bash
mkdir build
cd build
cmake ..
make
./raytracer
```

## Controls
- WASD: Camera movement
- Mouse: Look around
- ESC: Exit
