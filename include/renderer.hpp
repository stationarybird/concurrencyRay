#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <memory>
#include "scene.hpp"
#include "camera.hpp"
#include "render_buffer.hpp"
#include "thread_pool.hpp"
#include "config.hpp"

class Renderer
{
public:
    Renderer();
    ~Renderer() = default;

    void render(const Scene &scene, const Camera &camera, RenderBuffer &buffer);
    void render_with_fps(const Scene &scene, const Camera &camera, RenderBuffer &buffer, int fps);
    void set_thread_count(int count);

private:
    std::unique_ptr<ThreadPool> thread_pool_;
    Vec3 light_direction_;

    void render_chunk(const Scene &scene, const Camera &camera, RenderBuffer &buffer,
                      int start_row, int end_row) const;
    Vec3 trace_ray(const Ray &ray, const Scene &scene) const;
    Vec3 calculate_lighting(const Vec3 &color, const Vec3 &normal) const;
    Vec3 get_background_color(const Ray &ray) const;
};

inline Renderer::Renderer()
    : thread_pool_(std::make_unique<ThreadPool>(Config::NUM_THREADS)), light_direction_(Vec3(1, 1, -1).normalize())
{
}

inline void Renderer::render(const Scene &scene, const Camera &camera, RenderBuffer &buffer)
{
    const int width = buffer.get_width();
    const int height = buffer.get_height();
    const int chunk_size = height / Config::NUM_THREADS;

    std::vector<std::future<void>> futures;

    for (int i = 0; i < Config::NUM_THREADS; ++i)
    {
        int start_row = i * chunk_size;
        int end_row = (i == Config::NUM_THREADS - 1) ? height : (i + 1) * chunk_size;

        futures.push_back(thread_pool_->enqueue([this, &scene, &camera, &buffer, start_row, end_row]()
                                                { render_chunk(scene, camera, buffer, start_row, end_row); }));
    }

    for (auto &future : futures)
    {
        future.wait();
    }
}

inline void Renderer::render_with_fps(const Scene &scene, const Camera &camera, RenderBuffer &buffer, int fps)
{
    render(scene, camera, buffer);
}

inline void Renderer::set_thread_count(int count)
{
    thread_pool_ = std::make_unique<ThreadPool>(count);
}

inline void Renderer::render_chunk(const Scene &scene, const Camera &camera,
                                   RenderBuffer &buffer, int start_row, int end_row) const
{
    const int width = buffer.get_width();
    const int height = buffer.get_height();

    //threads each render a group of rows
    for (int j = start_row; j < end_row; ++j)
    {
        for (int i = 0; i < width; ++i)
        {
            double u = (2.0 * i / (width - 1.0)) - 1.0;
            double v = 1.0 - (2.0 * j / (height - 1.0));

            Ray ray = camera.get_ray(u, v);
            Vec3 pixel_color = trace_ray(ray, scene);

            buffer.set_pixel(i, j, pixel_color);
        }
    }
}

inline Vec3 Renderer::trace_ray(const Ray &ray, const Scene &scene) const
{
    HitRecord rec;
    if (scene.hit(ray, Config::RAY_T_MIN, Config::RAY_T_MAX, rec))
    {
        return calculate_lighting(rec.color, rec.normal);
    }
    else
    {
        return get_background_color(ray);
    }
}

inline Vec3 Renderer::calculate_lighting(const Vec3 &color, const Vec3 &normal) const
{
    double diffuse = std::max(0.0, normal.dot(light_direction_));
    Vec3 ambient = Vec3(Config::AMBIENT_STRENGTH, Config::AMBIENT_STRENGTH, Config::AMBIENT_STRENGTH);
    return ambient + color * diffuse;
}

inline Vec3 Renderer::get_background_color(const Ray &ray) const
{
    double t = 0.5 * (ray.direction.y + 1.0);
    return Vec3(1.0, 1.0, 1.0) * (1.0 - t) + Vec3(0.5, 0.7, 1.0) * t;
}

#endif