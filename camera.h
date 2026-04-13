/* * 27.03.2026
 * Zmiana w ray color pod ray marching
 * * dodac dokumentcje tlumaczaca dzialanie z newtonem i swarzchildem
 * * gdzie definiuje poczatkowy kierunek promieni?
 * * dane geodeyzjne wyliczac (jak i po co?)
 * *
 * * zobaczyc czy promiene sa rownolegle czy nie
 * * wunkcja zwaracajaca pozycje promienia w zakrzywienu
 */

#ifndef CAMERA_H
#define CAMERA_H

#include "color.h"
#include "ray.h"
#include "vec3.h"
#include "hittable.h"
#include <iostream>
#include <cstdlib>
#include <limits>

inline double random_double() {
    // Zwraca losową liczbę rzeczywistą z przedziału [0, 1)
    return rand() / (RAND_MAX + 1.0);
}

class camera {
public:
    //wartosci bazowe, jak w konsoli przekaze sie null, to zczyta z tych tutaj
    double aspect_ratio = 16.0 / 9.0;
    int image_width = 400;
    int samples_per_pixel = 1; //jakosc antyaliasingu, multisamplingu / dla testow ustawic 1

    point3 lookfrom = point3(0, 1.0, 4.0); // Kamera wyżej i dalej
    point3 lookat = point3(0, 0, -3.0); // Patrzymy w środek czarnej dziury
    vec3 vup = vec3(0, 1, 0); // "Góra" dla kamery to oś Y (czubek glowy)
    double focal_length = 2; // Normalna ogniskowa

    //  ZMIENNE WYCIĄGNIĘTE DLA INTERFEJSU UŻYTKOWNIKA
    point3 black_hole_center = point3(0.0, 0.0, -2.0);
    double event_horizon_radius = 0.1;
    double disk_inner_radius = 0.7; // Początek dysku (poza ISCO)
    double disk_outer_radius = 2.2; // Koniec dysku

    void render(const hittable& world) {
        initialize();

        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        for (int j = image_height - 1; j >= 0; --j) {
            std::cerr << "\rZostalo linii: " << j << ' ' << std::flush;
            for (int i = 0; i < image_width; ++i) {

                color pixel_color(0, 0, 0);

                for (int s = 0; s < samples_per_pixel; ++s) {
                    // Dodajemy losowe przesunięcie (jitter) do współrzędnych i oraz j
                    auto u = (double(i) + random_double()) / (image_width - 1);
                    auto v = (double(j) + random_double()) / (image_height - 1);

                    ray r(origin, lower_left_corner + u*horizontal + v*vertical - origin);
                    pixel_color = pixel_color + ray_color(r, world);
                }

                // Dzielimy sumę kolorów przez liczbę próbek, żeby wyciągnąć średnią
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
    int image_height;
    point3 origin;
    vec3 horizontal;
    vec3 vertical;
    point3 lower_left_corner;

    void initialize() {
        image_height = static_cast<int>(image_width / aspect_ratio);
        if (image_height < 1) image_height = 1;

        double viewport_height = 2.0;
        double viewport_width = aspect_ratio * viewport_height;

        // Matematyka zaawansowanej kamery (Wektory bazowe)
        origin = lookfrom; // Kamera stoi tam, gdzie lookfrom

        // Obliczamy wektory osi kamery w przestrzeni 3D
        vec3 w = unit_vector(lookfrom - lookat); // Kierunek przeciwny do patrzenia
        vec3 u = unit_vector(cross(vup, w)); // Wektor "w prawo" dla kamery
        vec3 v = cross(w, u); // Wektor "w górę" dla kamery

        // Definiujemy wektory horyzontalny i wertykalny ekranu
        horizontal = viewport_width * u;
        vertical = viewport_height * v;

        // Wyliczamy nowy lewy dolny róg, uwzględniając pozycję i obrót
        lower_left_corner = origin - horizontal/2 - vertical/2 - w*focal_length;
    }


    color ray_color(const ray& r, const hittable& world) {
        point3 current_pos = r.origin();
        vec3 current_dir = unit_vector(r.direction());

        double step_size = 0.05;
        int max_steps = 1500;

        // Wysokość, na której leży dysk (korzysta ze zmiennej w public)
        double bh_y = black_hole_center.y();

        for (int i = 0; i < max_steps; ++i) {
            // Zapamiętujemy poprzednią pozycję przed zrobieniem kroku
            point3 prev_pos = current_pos;

            // 1. SPRAWDZAMY CZARNĄ DZIURĘ
            vec3 to_bh = black_hole_center - current_pos;
            double distance_squared = to_bh.length_squared();

            if (distance_squared < event_horizon_radius * event_horizon_radius) {
                return color(0, 0, 0);
            }

            // 2. SPRAWDZAMY OBIEKTY ZE ŚWIATA (np. kule)
            ray step_ray(current_pos, current_dir);
            hit_record rec;
            if (world.hit(step_ray, 0.001, step_size, rec)) {
                return 0.5 * color(rec.normal.x() + 1, rec.normal.y() + 1, rec.normal.z() + 1);
            }
            //#########################################################################################################
            // 3. FIZYKA: RELATYWISTYCZNE ZAGINANIE (METRYKA SCHWARZSCHILDA)
            /**
             * Krok całkowania numerycznego dla trajektorii fotonu.
             * * Oblicza "pozorne przyspieszenie" grawitacyjne działające na bezmasowy foton.
             * Zamiast rozwiązywać złożone równania różniczkowe tensora metrycznego,
             * wykorzystano zredukowane wektorowe równanie geodezyjne i metodę Eulera.
             * * Wzór: a = - (1.5 * Rs * h^2 / r^5) * r_vec
             */

            // KROK 1: Wektor promienia (Odległość od centrum osobliwości)
            // Odwracamy wektor 'to_bh', aby uzyskać wektor skierowany 'od' czarnej dziury do fotonu.
            vec3 r_vec = -to_bh;
            double r_dist = sqrt(distance_squared);
            // Wyliczamy r^5 z góry dla optymalizacji (potrzebne w mianowniku).
            double r5 = distance_squared * distance_squared * r_dist;

            // KROK 2: Moment Pędu (Stopień omijania czarnej dziury)
            // Jeśli foton leci prosto w środek, iloczyn wektorowy wynosi 0 (światło nie skręci).
            // Im mocniej foton mija dziurę bokiem, tym 'h' jest większe (silniejsze soczewkowanie).
            vec3 h_vec = cross(r_vec, current_dir);
            double h2 = h_vec.length_squared();

            // KROK 3: Równanie Geodezyjne Einsteina
            // Zredukowana forma metryki. Wartość jest ujemna, bo grawitacja zawsze "ciągnie" foton.
            vec3 acceleration = -1.5 * event_horizon_radius * h2 / r5 * r_vec;

            // KROK 4: Numeryczne Całkowanie (Ray Marching)
            // Zaginamy kierunek promienia, ale natychmiast go normalizujemy (unit_vector).
            // Jest to kluczowe, bo grawitacja zmienia tor lotu, ale nie prędkość światła (c).
            current_dir = unit_vector(current_dir + acceleration * step_size);
            // Wykonujemy właściwy ruch w przestrzeni po nowej, zakrzywionej trajektorii.
            current_pos = current_pos + current_dir * step_size;
            // ########################################################################################################

            // DETEKCJA DYSKU AKRECYJNEGO
            // Sprawdzamy, czy w tym kroku foton "przebił" płaszczyznę dysku (bh_y)
            if ((prev_pos.y() > bh_y && current_pos.y() <= bh_y) ||
                (prev_pos.y() < bh_y && current_pos.y() >= bh_y)) {

                // Wyliczamy punkt przecięcia na płaszczyźnie Y
                double t = (bh_y - prev_pos.y()) / (current_pos.y() - prev_pos.y());
                point3 hit_point = prev_pos + t * (current_pos - prev_pos);

                // Sprawdzamy odległość punktu uderzenia od środka czarnej dziury
                double dx = hit_point.x() - black_hole_center.x();
                double dz = hit_point.z() - black_hole_center.z();
                double dist_to_center = sqrt(dx*dx + dz*dz);

                // Czy uderzyliśmy w pierścień dysku?
                if (dist_to_center >= disk_inner_radius && dist_to_center <= disk_outer_radius) {
                    // Prosty model temperatury: im bliżej środka, tym jaśniej/cieplej
                    double temp = 1.0 - ((dist_to_center - disk_inner_radius) / (disk_outer_radius - disk_inner_radius));
                    return color(1.0, 0.3 + 0.7 * temp, 0.1 + 0.5 * temp); // Ognisty pomarańczowy
                }
            }

            if (current_pos.length() > 50.0) break;
        }

        return color(0.0, 0.0, 0.0); // Tło: kosmiczna czerń
    }
};

#endif