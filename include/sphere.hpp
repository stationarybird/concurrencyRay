#ifndef SPHERE_HPP
#define SPHERE_HPP

#include "vec3.hpp"
#include "ray.hpp"

struct HitRecord {
    double t;
    Vec3 point;
    Vec3 normal;
    Vec3 color;
};

class Sphere {
public:
    Vec3 center;
    double radius;
    Vec3 color;

    Sphere(const Vec3& center, double radius, const Vec3& color)
        : center(center), radius(radius), color(color) {}

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& rec) const {
        Vec3 oc = ray.origin - center;
        double a = ray.direction.dot(ray.direction);
        double b = 2.0 * oc.dot(ray.direction);
        double c = oc.dot(oc) - radius * radius;
        double discriminant = b*b - 4*a*c;

        if (discriminant > 0) {
            double temp = (-b - sqrt(discriminant)) / (2.0*a);
            if (temp < t_max && temp > t_min) {
                rec.t = temp;
                rec.point = ray.point_at(rec.t);
                rec.normal = (rec.point - center) / radius;
                rec.color = color;
                return true;
            }
            temp = (-b + sqrt(discriminant)) / (2.0*a);
            if (temp < t_max && temp > t_min) {
                rec.t = temp;
                rec.point = ray.point_at(rec.t);
                rec.normal = (rec.point - center) / radius;
                rec.color = color;
                return true;
            }
        }
        return false;
    }
};

#endif 