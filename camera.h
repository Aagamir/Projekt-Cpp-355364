/*
 * 27.03.2026
 * Zmiana w ray color pod ray marching
 */

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
    int    image_width  = 1600;

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
        point3 current_pos = r.origin();
        vec3 current_dir = unit_vector(r.direction());

        double step_size = 0.05;
        int max_steps = 1500;

        // Parametry naszej czarnej dziury
        point3 black_hole_center(0.0, 0.0, -3.0);
        double event_horizon_radius = 0.3;
        double mass = 0.1;

        // Parametry dysku akrecyjnego
        double disk_inner_radius = 0.7; // Zaczyna się trochę za horyzontem
        double disk_outer_radius = 2.2; // Koniec dysku

        for (int i = 0; i < max_steps; ++i) {
            // --- 1. SPRAWDZAMY CZARNĄ DZIURĘ ---
            vec3 to_bh = black_hole_center - current_pos;
            double distance_squared = to_bh.length_squared();

            if (distance_squared < event_horizon_radius * event_horizon_radius) {
                return color(0, 0, 0); // Foton wpadł za horyzont zdarzeń
            }

            // --- 2. SPRAWDZAMY OBIEKTY ZE ŚWIATA ---
            ray step_ray(current_pos, current_dir);
            hit_record rec;
            if (world.hit(step_ray, 0.001, step_size, rec)) {
                return 0.5 * color(rec.normal.x() + 1, rec.normal.y() + 1, rec.normal.z() + 1);
            }

            // Zapisujemy pozycję PRZED zrobieniem kroku fizycznego
            point3 prev_pos = current_pos;

            // --- 3. FIZYKA: ZAGINANIE FOTONU ---
            double force = mass / distance_squared;
            vec3 gravity_pull = unit_vector(to_bh) * force;

            current_dir = unit_vector(current_dir + gravity_pull * step_size);
            current_pos = current_pos + current_dir * step_size;

            // --- 4. DYSK AKRECYJNY (Przecięcie płaszczyzny Y = 0) ---
            double bh_y = black_hole_center.y();

            // Jeśli znak Y się zmienił, to znaczy, że właśnie przebiliśmy płaszczyznę dysku!
            if ((prev_pos.y() > bh_y && current_pos.y() <= bh_y) ||
                (prev_pos.y() < bh_y && current_pos.y() >= bh_y)) {

                // Liczymy dokładny punkt przebicia za pomocą prostej proporcji
                double fraction = (prev_pos.y() - bh_y) / (prev_pos.y() - current_pos.y());
                point3 hit_point = prev_pos + fraction * (current_pos - prev_pos);

                // Sprawdzamy, jak daleko od środka czarnej dziury uderzyliśmy (Twierdzenie Pitagorasa)
                double dx = hit_point.x() - black_hole_center.x();
                double dz = hit_point.z() - black_hole_center.z();
                double dist_to_center = sqrt(dx*dx + dz*dz);

                // Jeśli uderzyliśmy w materię dysku:
                if (dist_to_center >= disk_inner_radius && dist_to_center <= disk_outer_radius) {
                    // Obliczamy "temperaturę" od 0.0 do 1.0
                    double temperature = 1.0 - ((dist_to_center - disk_inner_radius) / (disk_outer_radius - disk_inner_radius));

                    return color(1.0, 0.4 + 0.6 * temperature, 0.1 + 0.9 * temperature);
                }
            }

            // Zabezpieczenie przed ucieczką promienia w nieskończoność
            if (current_pos.length() > 50.0) break;
        }

        // --- 5. TŁO (NIEBO) ---
        auto t = 0.5 * (current_dir.y() + 1.0);
        return (1.0 - t) * color(0.0, 0.0, 0.0) + t * color(0.0, 0.0, 0.0);
    }
};

#endif