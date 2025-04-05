#define GL_SILENCE_DEPRECATION
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <GLFW/glfw3.h>
#include "vec3.hpp"
#include "ray.hpp"
#include "sphere.hpp"
#include "scene.hpp"
#include "camera.hpp"

// Window settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
const int NUM_THREADS = std::thread::hardware_concurrency();
const int SAMPLES_PER_PIXEL = 1;

// Progressive refinement: render at lower resolution (scale factor)
const float renderScale = 0.5f;  
int renderWidth = SCR_WIDTH * renderScale;
int renderHeight = SCR_HEIGHT * renderScale;

// Global variables for camera and timing
Camera camera(Vec3(0.0f, 0.0f, 3.0f));
float last_x = 400.0f;
float last_y = 300.0f;
bool first_mouse = true;
float delta_time = 0.0f;
float last_frame = 0.0f;

// Pixel buffer for low-resolution render
std::vector<unsigned char> pixels;

// OpenGL texture for displaying the rendered image
GLuint textureID;

// ---------------------------------------------------------------------------
// ThreadPool Implementation (improves multithreading by reusing threads)
class ThreadPool {
public:
    ThreadPool(size_t threads);
    ~ThreadPool();
    void enqueue(std::function<void()> task);
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

ThreadPool::ThreadPool(size_t threads) : stop(false) {
    for (size_t i = 0; i < threads; i++) {
        workers.emplace_back([this]() {
            for (;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this]() {
                        return this->stop || !this->tasks.empty();
                    });
                    if (this->stop && this->tasks.empty())
                        return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers)
        worker.join();
}

void ThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        tasks.emplace(task);
    }
    condition.notify_one();
}

// Global thread pool (initialized in main)
ThreadPool* pool = nullptr;

// Synchronization to wait for all render tasks to finish
std::atomic<int> tasks_remaining(0);
std::mutex tasks_mutex;
std::condition_variable tasks_cv;

// ---------------------------------------------------------------------------
// Create a simple scene with some spheres
Scene create_scene() {
    Scene scene;
    scene.add_sphere(Sphere(Vec3(0, 0, -5), 1.0, Vec3(1.0, 0.2, 0.2)));  // Red sphere
    scene.add_sphere(Sphere(Vec3(2, 0, -6), 1.0, Vec3(0.2, 1.0, 0.2)));  // Green sphere
    scene.add_sphere(Sphere(Vec3(-2, 0, -4), 1.0, Vec3(0.2, 0.2, 1.0))); // Blue sphere
    return scene;
}

// ---------------------------------------------------------------------------
// Callback functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
    camera.aspect_ratio = static_cast<double>(width) / height;
    
    // Update render resolution based on renderScale
    renderWidth = static_cast<int>(width * renderScale);
    renderHeight = static_cast<int>(height * renderScale);
    pixels.resize(renderWidth * renderHeight * 3);
    
    // Resize the texture accordingly
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, renderWidth, renderHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (first_mouse) {
        last_x = xpos;
        last_y = ypos;
        first_mouse = false;
    }
    float xoffset = xpos - last_x;
    float yoffset = last_y - ypos;
    last_x = xpos;
    last_y = ypos;
    camera.process_mouse(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.process_scroll(yoffset);
}

void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    float current_frame = glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.process_keyboard(Camera::FORWARD, delta_time);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.process_keyboard(Camera::BACKWARD, delta_time);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.process_keyboard(Camera::LEFT, delta_time);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.process_keyboard(Camera::RIGHT, delta_time);
}

// ---------------------------------------------------------------------------
// Render a chunk of the image (now using renderWidth and renderHeight)
void render_chunk(const Scene& scene, int start_row, int end_row, int renderWidth, int renderHeight) {
    for (int j = start_row; j < end_row; j++) {
        for (int i = 0; i < renderWidth; i++) {
            Vec3 pixel_color(0, 0, 0);
            double u = (2.0 * i / (renderWidth - 1.0)) - 1.0;
            double v = 1.0 - (2.0 * j / (renderHeight - 1.0));
            Ray ray = camera.get_ray(u, v);
            
            HitRecord rec;
            if (scene.hit(ray, 0.001, INFINITY, rec)) {
                // Precompute light direction outside the loop if possible; here we use it per pixel for clarity.
                Vec3 light_dir = Vec3(1, 1, -1).normalize();
                double diffuse = std::max(0.0, rec.normal.dot(light_dir));
                Vec3 ambient = Vec3(0.1, 0.1, 0.1);
                pixel_color = ambient + rec.color * diffuse;
            } else {
                double t = 0.5 * (ray.direction.y + 1.0);
                pixel_color = Vec3(1.0, 1.0, 1.0) * (1.0 - t) + Vec3(0.5, 0.7, 1.0) * t;
            }
            
            int idx = (j * renderWidth + i) * 3;
            pixels[idx]     = static_cast<unsigned char>(255.99 * std::min(1.0, pixel_color.x));
            pixels[idx + 1] = static_cast<unsigned char>(255.99 * std::min(1.0, pixel_color.y));
            pixels[idx + 2] = static_cast<unsigned char>(255.99 * std::min(1.0, pixel_color.z));
        }
    }
    
    // Signal task completion
    if (--tasks_remaining == 0) {
        std::unique_lock<std::mutex> lock(tasks_mutex);
        tasks_cv.notify_one();
    }
}

// ---------------------------------------------------------------------------
// Main
int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    
    SCR_WIDTH = mode->width;
    SCR_HEIGHT = mode->height;
    
    last_x = SCR_WIDTH / 2.0f;
    last_y = SCR_HEIGHT / 2.0f; // set initial cursor position to middle of screen
    
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Ray Tracer", primary, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    int fb_width, fb_height;
    glfwGetFramebufferSize(window, &fb_width, &fb_height);
    camera.aspect_ratio = static_cast<double>(fb_width) / fb_height;
    
    // Set up callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Initialize render resolution and pixel buffer
    renderWidth = static_cast<int>(fb_width * renderScale);
    renderHeight = static_cast<int>(fb_height * renderScale);
    pixels.resize(renderWidth * renderHeight * 3);
    
    // Create and set up the texture for displaying the render
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Allocate texture storage
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, renderWidth, renderHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    
    // Create the scene
    Scene scene = create_scene();
    
    double prevTime = glfwGetTime();
    int frame = 0;
    
    // Initialize thread pool
    pool = new ThreadPool(NUM_THREADS);
    
    while (!glfwWindowShouldClose(window)) {
        process_input(window);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Reset the task counter before rendering
        tasks_remaining = NUM_THREADS;
        int chunk_size = renderHeight / NUM_THREADS;
        
        // Enqueue tasks for each chunk using the thread pool
        for (int i = 0; i < NUM_THREADS; i++) {
            int start_row = i * chunk_size;
            int end_row = (i == NUM_THREADS - 1) ? renderHeight : (i + 1) * chunk_size;
            pool->enqueue([=, &scene]() {
                render_chunk(scene, start_row, end_row, renderWidth, renderHeight);
            });
        }
        
        // Wait for all chunks to complete
        {
            std::unique_lock<std::mutex> lock(tasks_mutex);
            tasks_cv.wait(lock, [](){ return tasks_remaining.load() == 0; });
        }
        
        // Update texture with the rendered pixel data
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, renderWidth, renderHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
        
        // Draw a fullscreen textured quad (using immediate mode for simplicity)
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f); glVertex2f(-1.0f, -1.0f);
            glTexCoord2f(1.0f, 0.0f); glVertex2f( 1.0f, -1.0f);
            glTexCoord2f(1.0f, 1.0f); glVertex2f( 1.0f,  1.0f);
            glTexCoord2f(0.0f, 1.0f); glVertex2f(-1.0f,  1.0f);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        
        double currentTime = glfwGetTime();
        frame++;
        if (currentTime - prevTime >= 1.0) {
            std::cout << "FPS: " << frame << std::endl;
            frame = 0;
            prevTime = currentTime;
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    delete pool;
    glfwTerminate();
    return 0;
}