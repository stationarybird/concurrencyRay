#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "vec3.hpp"
#include <cmath>

class Camera {
public:
    enum Camera_Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    Vec3 position;
    Vec3 front;
    Vec3 up;
    Vec3 right;
    Vec3 world_up;
    
    // Euler angles
    double yaw;
    double pitch;
    
    // Camera options
    double movement_speed;
    double mouse_sensitivity;
    double zoom;
    double aspect_ratio;

    Camera(Vec3 position = Vec3(0.0, 0.0, 0.0))
        : position(position)
        , world_up(Vec3(0.0, 1.0, 0.0))
        , yaw(-90.0)
        , pitch(0.0)
        , movement_speed(5.0)  // Increased for better control
        , mouse_sensitivity(0.05)  // Reduced for smoother camera movement
        , zoom(45.0)
        , aspect_ratio(1.0)  // Will be set properly when window is created
    {
        update_camera_vectors();
    }

    void process_keyboard(Camera_Movement direction, double delta_time) {
        double velocity = movement_speed * delta_time;
        if (direction == FORWARD)
            position = position + front * velocity;
        if (direction == BACKWARD)
            position = position - front * velocity;
        if (direction == LEFT)
            position = position - right * velocity;
        if (direction == RIGHT)
            position = position + right * velocity;
    }

    void process_mouse(double xoffset, double yoffset, bool constrain_pitch = true) {
        xoffset *= mouse_sensitivity;
        yoffset *= mouse_sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrain_pitch) {
            if (pitch > 89.0) pitch = 89.0;
            if (pitch < -89.0) pitch = -89.0;
        }

        update_camera_vectors();
    }

    void process_scroll(double yoffset) {
        zoom -= yoffset;
        zoom = std::max(1.0, std::min(zoom, 90.0));  // Allow wider FOV range
    }

    Ray get_ray(double u, double v) const {
        // Calculate the viewport dimensions based on FOV and aspect ratio
        double theta = radians(zoom);
        double h = tan(theta/2);
        double viewport_height = 2.0 * h;
        double viewport_width = aspect_ratio * viewport_height;

        // Calculate the ray direction
        Vec3 rd = front + right * (u * viewport_width) + up * (v * viewport_height);
        return Ray(position, rd.normalize());
    }

private:
    void update_camera_vectors() {
        // Calculate the new front vector
        Vec3 new_front;
        new_front.x = cos(radians(yaw)) * cos(radians(pitch));
        new_front.y = sin(radians(pitch));
        new_front.z = sin(radians(yaw)) * cos(radians(pitch));
        front = new_front.normalize();
        
        // Re-calculate the right and up vectors
        right = front.cross(world_up).normalize();
        up = right.cross(front).normalize();
    }

    double radians(double degrees) const {
        return degrees * M_PI / 180.0;
    }
};

#endif 