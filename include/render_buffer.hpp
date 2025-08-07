#ifndef RENDER_BUFFER_HPP
#define RENDER_BUFFER_HPP

#include <vector>
#include <GLFW/glfw3.h>
#include "vec3.hpp"

class RenderBuffer
{
public:
    RenderBuffer(int width, int height);
    ~RenderBuffer();

    void resize(int width, int height);
    void set_pixel(int x, int y, const Vec3 &color);
    void update_texture();
    void bind_texture() const;
    void clear();

    int get_width() const { return width_; }
    int get_height() const { return height_; }
    const std::vector<unsigned char> &get_pixels() const { return pixels_; }

private:
    int width_;
    int height_;
    std::vector<unsigned char> pixels_;
    GLuint texture_id_;

    void setup_texture();
    unsigned char clamp_color(double value) const;
};

inline RenderBuffer::RenderBuffer(int width, int height)
    : width_(width), height_(height), pixels_(width * height * 3, 0)
{
    setup_texture();
}

inline RenderBuffer::~RenderBuffer()
{
    if (texture_id_ != 0)
    {
        glDeleteTextures(1, &texture_id_);
    }
}

inline void RenderBuffer::resize(int width, int height)
{
    width_ = width;
    height_ = height;
    pixels_.resize(width * height * 3);
    clear();

    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
}

inline void RenderBuffer::set_pixel(int x, int y, const Vec3 &color)
{
    if (x >= 0 && x < width_ && y >= 0 && y < height_)
    {
        int idx = (y * width_ + x) * 3;
        pixels_[idx] = clamp_color(color.x);
        pixels_[idx + 1] = clamp_color(color.y);
        pixels_[idx + 2] = clamp_color(color.z);
    }
}

inline void RenderBuffer::update_texture()
{
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, pixels_.data());
}

inline void RenderBuffer::bind_texture() const
{
    glBindTexture(GL_TEXTURE_2D, texture_id_);
}

inline void RenderBuffer::clear()
{
    std::fill(pixels_.begin(), pixels_.end(), 0);
}

inline void RenderBuffer::setup_texture()
{
    glGenTextures(1, &texture_id_);
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_, height_, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
}

inline unsigned char RenderBuffer::clamp_color(double value) const
{
    return static_cast<unsigned char>(255.99 * std::min(1.0, std::max(0.0, value)));
}

#endif