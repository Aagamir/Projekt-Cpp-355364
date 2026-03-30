/*
 * 27.03.2026
 * Zmiana w ray color pod ray marching
 *
 * dodac dokumentcje tlumaczaca dzialanie z newtonem i swarzchildem
 *
 * gdzie definiuje poczatkowy kierunek promieni
 *
 * dane geodeyzjne wyliczac
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
        double focal_length = 7.0;

        origin = point3(0, 0, 0);
        horizontal = vec3(viewport_width, 0, 0);
        vertical = vec3(0, viewport_height, 0);
        lower_left_corner = origin - horizontal/2 - vertical/2 - vec3(0, 0, focal_length);
    }


    //To potem chyba wrzuce do ray.h bo bardziej pasuje nazewnictwo
    color ray_color(const ray& r, const hittable& world) {
    // 1. Pobieranie pozycji kamery i kierunku strzału
    point3 current_pos = r.origin();
    vec3 current_dir = unit_vector(r.direction());

    // Parametry symulacji (krok całkowania numerycznego)
    double step_size = 0.05; // dt
    int max_steps = 1500;    // Zabezpieczenie, żeby pętla nie kręciła się w nieskończoność

    // Wirtualna czarna dziura (w przyszlosci moze przeniose do osoobnej klasy)
    point3 black_hole_center(0.0, 0.0, -3.0);
    double event_horizon_radius = 0.3;
    double mass = 0.1; // Odpowiednik masy

    // 2. RAY MARCHING - Główna pętla symulacji fizycznej
    for (int i = 0; i < max_steps; ++i) {

        // Obliczamy wektor od naszego fotonu do środka czarnej dziury
        vec3 to_bh = black_hole_center - current_pos;
        double distance_squared = to_bh.length_squared();

        // WARUNEK A: Wpadliśmy do środka (Przekroczono Horyzont Zdarzeń)
        if (distance_squared < event_horizon_radius * event_horizon_radius) {
            return color(0.0, 0.0, 0.0); // Zwracamy idealną czerń
        }

        // SPRAWDZAMY OBIEKTY Z PRZESTRZENI
        // Wypuszczamy krótki promień na dystans naszego kroku
        ray step_ray(current_pos, current_dir);
        hit_record rec;

        if (world.hit(step_ray, 0.001, step_size, rec)) {
            // Skoro usunęliśmy albedo, wracamy do kolorowania wektorów normalnych!
            return 0.5 * color(rec.normal.x() + 1, rec.normal.y() + 1, rec.normal.z() + 1);
        }

        // 3. INTEGRACJA NUMERYCZNA (Uproszczona metoda Eulera dla prezentacji)
        // Im bliżej dziury, tym siła jest potężniejsza (prawo odwrotnych kwadratów)
        double force = mass / distance_squared;

        // Wektor grawitacji "ciągnący" nasz foton
        vec3 gravity_pull = unit_vector(to_bh) * force;

        // Zaginamy kierunek fotonu i od razu go normalizujemy
        // (Foton w próżni musi zawsze lecieć z prędkością światła c=1)
        current_dir = unit_vector(current_dir + gravity_pull * step_size);

        // 4. Robimy fizyczny krok do przodu w nowym, zakrzywionym kierunku
        current_pos = current_pos + current_dir * step_size;

        // (W przyszłości w tym miejscu dodamy sprawdzanie zderzeń z dyskiem akrecyjnym)
    }

    // WARUNEK B: Promień uciekł grawitacji i poleciał w kosmos
    // Wyświetlamy nasze tło
    return color(0.7,0.7,0.7);
}
};

#endif


/*
#ifndef CAMERA_H
#define CAMERA_H

#include "color.h"
#include "ray.h"
#include "vec3.h"
#include "hittable.h"
#include <iostream>
#include <limits>
#include <cstdlib> // Wymagane do funkcji rand()

// Prosta funkcja zwracająca losowy ułamek od 0.0 do 1.0
inline double random_double() {
    return rand() / (RAND_MAX + 1.0);
}

class camera {
public:
    double aspect_ratio = 16.0 / 9.0;
    int    image_width  = 400;
    int    samples_per_pixel = 20; // Ile próbek na każdy piksel (antialiasing)

    void render(const hittable& world) {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        for (int j = image_height - 1; j >= 0; --j) {
            std::cerr << "\rZostalo linii: " << j << ' ' << std::flush;
            for (int i = 0; i < image_width; ++i) {

                color pixel_color(0, 0, 0);

                // Wypuszczamy kilka promieni wewnątrz jednego piksela
                for (int s = 0; s < samples_per_pixel; ++s) {
                    auto u = (double(i) + random_double()) / (image_width - 1);
                    auto v = (double(j) + random_double()) / (image_height - 1);

                    ray r(origin, lower_left_corner + u*horizontal + v*vertical - origin);

                    pixel_color = pixel_color + ray_color(r, world);
                }

                // Uśredniamy kolor ze wszystkich trafień
                auto scale = 1.0 / samples_per_pixel;
                pixel_color = color(pixel_color.x() * scale,
                                    pixel_color.y() * scale,
                                    pixel_color.z() * scale);

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

        // Parametry czarnej dziury
        point3 black_hole_center(0.0, 0.0, -3.0);
        double event_horizon_radius = 0.3;
        double mass = 0.1;

        // Parametry dysku akrecyjnego
        double disk_inner_radius = 0.7;
        double disk_outer_radius = 2.2;

        for (int i = 0; i < max_steps; ++i) {
            // --- 1. SPRAWDZAMY CZARNĄ DZIURĘ ---
            vec3 to_bh = black_hole_center - current_pos;
            double distance_squared = to_bh.length_squared();

            if (distance_squared < event_horizon_radius * event_horizon_radius) {
                return color(0, 0, 0);
            }

            // --- 2. SPRAWDZAMY OBIEKTY ZE ŚWIATA ---
            ray step_ray(current_pos, current_dir);
            hit_record rec;
            if (world.hit(step_ray, 0.001, step_size, rec)) {
                return 0.5 * color(rec.normal.x() + 1, rec.normal.y() + 1, rec.normal.z() + 1);
            }

            point3 prev_pos = current_pos;

            // --- 3. FIZYKA: ZAGINANIE FOTONU ---
            double force = mass / distance_squared;
            vec3 gravity_pull = unit_vector(to_bh) * force;

            current_dir = unit_vector(current_dir + gravity_pull * step_size);
            current_pos = current_pos + current_dir * step_size;

            // --- 4. DYSK AKRECYJNY ---
            double bh_y = black_hole_center.y();

            if ((prev_pos.y() > bh_y && current_pos.y() <= bh_y) ||
                (prev_pos.y() < bh_y && current_pos.y() >= bh_y)) {

                double fraction = (prev_pos.y() - bh_y) / (prev_pos.y() - current_pos.y());
                point3 hit_point = prev_pos + fraction * (current_pos - prev_pos);

                double dx = hit_point.x() - black_hole_center.x();
                double dz = hit_point.z() - black_hole_center.z();
                double dist_to_center = sqrt(dx*dx + dz*dz);

                if (dist_to_center >= disk_inner_radius && dist_to_center <= disk_outer_radius) {
                    double temperature = 1.0 - ((dist_to_center - disk_inner_radius) / (disk_outer_radius - disk_inner_radius));
                    return color(1.0, 0.4 + 0.6 * temperature, 0.1 + 0.9 * temperature);
                }
            }

            if (current_pos.length() > 50.0) break;
        }

        // --- 5. TŁO (NIEBO) ---
        auto t = 0.5 * (current_dir.y() + 1.0);
        return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
    }
};

#endif
 */