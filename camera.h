#ifndef CAMERA_H
#define CAMERA_H

#include "color.h"
#include "ray.h"
#include "vec3.h"
#include "hittable.h"
#include <iostream>
#include <limits>

class camera {
public:
    double aspect_ratio = 16.0 / 9.0;
    int    image_width  = 400;

    void render(const hittable& world) {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        for (int j = image_height - 1; j >= 0; --j) {
            std::cerr << "\rZostalo linii: " << j << ' ' << std::flush;
            for (int i = 0; i < image_width; ++i) {
                auto u = double(i) / (image_width - 1);
                auto v = double(j) / (image_height - 1);
                ray r(origin, lower_left_corner + u*horizontal + v*vertical - origin);

                color pixel_color = ray_color(r, world);
                write_color(std::cout, pixel_color);
            }
        }
        std::cerr << "\nRenderowanie zakonczone sukcesem!\n";
    }

private:
    int    image_height;
    point3 origin;
    vec3   horizontal;
    vec3   vertical;
    point3 lower_left_corner;

    void initialize() {
        image_height = static_cast<int>(image_width / aspect_ratio);
        if (image_height < 1) image_height = 1;

        double viewport_height = 2.0;
        double viewport_width = aspect_ratio * viewport_height;
        double focal_length = 1.0;

        origin = point3(0, 0, 0);
        horizontal = vec3(viewport_width, 0, 0);
        vertical = vec3(0, viewport_height, 0);
        lower_left_corner = origin - horizontal/2 - vertical/2 - vec3(0, 0, focal_length);
    }

    color ray_color(const ray& r, const hittable& world) {
        hit_record rec;
        const double infinity = std::numeric_limits<double>::infinity();

        // Jeśli promień uderzy w cokolwiek w przedziale od 0.001  do nieskończoności
        if (world.hit(r, 0.0, infinity, rec)) {
            // Kolorujemy na podstawie kierunku normalnego miejsca uderzenia
            return 0.5 * color(rec.normal.x() + 1, rec.normal.y() + 1, rec.normal.z() + 1);
        }

        // Rysujemy tło nieba, jeśli nic nie trafiliśmy
        vec3 unit_direction = unit_vector(r.direction());
        auto t = 0.5 * (unit_direction.y() + 1.0);
        return color(0,0,0); //(1.0 - t) * color(0.0, 0.0, 0.1) + t * color(0.0, 0.0, 1.0);
    }
};

#endif