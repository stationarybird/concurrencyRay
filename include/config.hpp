#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <thread>

struct Config
{
    static constexpr unsigned int DEFAULT_WIDTH = 800;
    static constexpr unsigned int DEFAULT_HEIGHT = 600;

    static constexpr float RENDER_SCALE = 0.5f;
    static constexpr int SAMPLES_PER_PIXEL = 1;
    static constexpr double RAY_T_MIN = 0.001;
    static constexpr double RAY_T_MAX = 1000.0;

    static inline const int NUM_THREADS = std::thread::hardware_concurrency();

    static constexpr float DEFAULT_CAMERA_SPEED = 5.0f;
    static constexpr float DEFAULT_MOUSE_SENSITIVITY = 0.05f;
    static constexpr double DEFAULT_FOV = 45.0;

    static constexpr double AMBIENT_STRENGTH = 0.1;
};

#endif