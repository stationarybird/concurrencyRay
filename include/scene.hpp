#ifndef SCENE_HPP
#define SCENE_HPP

#include <vector>
#include "sphere.hpp"

class Scene {
public:
    std::vector<Sphere> spheres;

    void add_sphere(const Sphere& sphere) {
        spheres.push_back(sphere);
    }

    bool hit(const Ray& ray, double t_min, double t_max, HitRecord& rec) const {
        HitRecord temp_rec;
        bool hit_anything = false;
        double closest_so_far = t_max;

        for (const auto& sphere : spheres) {
            if (sphere.hit(ray, t_min, closest_so_far, temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }
};

#endif 