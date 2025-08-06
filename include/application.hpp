#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <memory>
#include <iostream>
#include <GLFW/glfw3.h>
#include "camera.hpp"
#include "scene.hpp"
#include "renderer.hpp"
#include "render_buffer.hpp"
#include "config.hpp"

class Application
{
public:
    Application();
    ~Application();

    bool initialize();
    void run();
    void cleanup();

private:
    GLFWwindow *window_;
    std::unique_ptr<Camera> camera_;
    std::unique_ptr<Scene> scene_;
    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<RenderBuffer> render_buffer_;

    // Window state
    int window_width_;
    int window_height_;
    int render_width_;
    int render_height_;

    // Input state
    float last_x_;
    float last_y_;
    bool first_mouse_;
    float delta_time_;
    float last_frame_;

    // FPS tracking
    double fps_update_time_;
    int frame_count_;
    int current_fps_;

    // Methods
    bool setup_opengl();
    void setup_callbacks();
    Scene create_default_scene() const;
    void process_input();
    void update_render_size();
    void render_frame();
    void draw_fullscreen_quad() const;
    void update_fps();

    // Static callback functions
    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    static void mouse_callback(GLFWwindow *window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

    // Helper to get Application instance from GLFW window
    static Application *get_app(GLFWwindow *window);
};

inline Application::Application()
    : window_(nullptr), window_width_(Config::DEFAULT_WIDTH), window_height_(Config::DEFAULT_HEIGHT), last_x_(Config::DEFAULT_WIDTH / 2.0f), last_y_(Config::DEFAULT_HEIGHT / 2.0f), first_mouse_(true), delta_time_(0.0f), last_frame_(0.0f), fps_update_time_(0.0), frame_count_(0), current_fps_(0)
{
}

inline Application::~Application()
{
    cleanup();
}

inline bool Application::initialize()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    GLFWmonitor *primary = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(primary);

    window_width_ = mode->width;
    window_height_ = mode->height;
    last_x_ = window_width_ / 2.0f;
    last_y_ = window_height_ / 2.0f;

    window_ = glfwCreateWindow(window_width_, window_height_, "Ray Tracer", primary, nullptr);
    if (!window_)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window_);

    int fb_width, fb_height;
    glfwGetFramebufferSize(window_, &fb_width, &fb_height);
    window_width_ = fb_width;
    window_height_ = fb_height;

    if (!setup_opengl())
    {
        return false;
    }

    setup_callbacks();

    camera_ = std::make_unique<Camera>(Vec3(0.0f, 0.0f, 3.0f));
    camera_->aspect_ratio = static_cast<double>(window_width_) / window_height_;

    scene_ = std::make_unique<Scene>(create_default_scene());
    renderer_ = std::make_unique<Renderer>();

    update_render_size();

    return true;
}

inline void Application::run()
{
    fps_update_time_ = glfwGetTime();

    while (!glfwWindowShouldClose(window_))
    {
        process_input();
        render_frame();
        update_fps();

        glfwSwapBuffers(window_);
        glfwPollEvents();
    }
}

inline void Application::cleanup()
{
    render_buffer_.reset();
    renderer_.reset();
    scene_.reset();
    camera_.reset();

    if (window_)
    {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();
}

inline bool Application::setup_opengl()
{
    glViewport(0, 0, window_width_, window_height_);
    return true;
}

inline void Application::setup_callbacks()
{
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);
    glfwSetCursorPosCallback(window_, mouse_callback);
    glfwSetScrollCallback(window_, scroll_callback);
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

inline Scene Application::create_default_scene() const
{
    Scene scene;
    scene.add_sphere(Sphere(Vec3(0, 0, -5), 1.0, Vec3(1.0, 0.2, 0.2)));  // Red
    scene.add_sphere(Sphere(Vec3(2, 0, -6), 1.0, Vec3(0.2, 1.0, 0.2)));  // Green
    scene.add_sphere(Sphere(Vec3(-2, 0, -4), 1.0, Vec3(0.2, 0.2, 1.0))); // Blue
    return scene;
}

inline void Application::process_input()
{
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window_, true);
    }

    float current_frame = glfwGetTime();
    delta_time_ = current_frame - last_frame_;
    last_frame_ = current_frame;

    if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS)
        camera_->process_keyboard(Camera::FORWARD, delta_time_);
    if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
        camera_->process_keyboard(Camera::BACKWARD, delta_time_);
    if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS)
        camera_->process_keyboard(Camera::LEFT, delta_time_);
    if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS)
        camera_->process_keyboard(Camera::RIGHT, delta_time_);
}

inline void Application::update_render_size()
{
    render_width_ = static_cast<int>(window_width_ * Config::RENDER_SCALE);
    render_height_ = static_cast<int>(window_height_ * Config::RENDER_SCALE);

    if (render_buffer_)
    {
        render_buffer_->resize(render_width_, render_height_);
    }
    else
    {
        render_buffer_ = std::make_unique<RenderBuffer>(render_width_, render_height_);
    }
}

inline void Application::render_frame()
{
    glClear(GL_COLOR_BUFFER_BIT);

    renderer_->render_with_fps(*scene_, *camera_, *render_buffer_, current_fps_);
    render_buffer_->update_texture();

    draw_fullscreen_quad();
}

inline void Application::draw_fullscreen_quad() const
{
    glEnable(GL_TEXTURE_2D);
    render_buffer_->bind_texture();

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

inline void Application::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    Application *app = get_app(window);
    app->window_width_ = width;
    app->window_height_ = height;
    glViewport(0, 0, width, height);
    app->camera_->aspect_ratio = static_cast<double>(width) / height;
    app->update_render_size();
}

inline void Application::mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    Application *app = get_app(window);

    if (app->first_mouse_)
    {
        app->last_x_ = xpos;
        app->last_y_ = ypos;
        app->first_mouse_ = false;
    }

    float xoffset = xpos - app->last_x_;
    float yoffset = app->last_y_ - ypos;
    app->last_x_ = xpos;
    app->last_y_ = ypos;

    app->camera_->process_mouse(xoffset, yoffset);
}

inline void Application::scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    Application *app = get_app(window);
    app->camera_->process_scroll(yoffset);
}

inline void Application::update_fps()
{
    frame_count_++;
    double current_time = glfwGetTime();

    if (current_time - fps_update_time_ >= 1.0)
    {
        current_fps_ = frame_count_;
        frame_count_ = 0;
        fps_update_time_ = current_time;
    }
}

inline Application *Application::get_app(GLFWwindow *window)
{
    return static_cast<Application *>(glfwGetWindowUserPointer(window));
}

#endif