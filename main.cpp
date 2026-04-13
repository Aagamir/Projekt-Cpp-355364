#include "camera.h"
#include "hittable_list.h"
#include "sphere.h"
#include <iostream>
#include <string>
#include <sstream>

// Pomocnicza funkcja do obsługi "opcjonalnego" wejścia
template <typename T>
void get_input(const std::string& prompt, T& value) {
    std::cerr << prompt << " [" << value << "]: ";
    std::string input;
    std::getline(std::cin, input);
    if (!input.empty()) {
        std::stringstream ss(input);
        ss >> value;
    }
}

int main() {
    hittable_list world;
    camera cam;

    std::cerr << "========================================\n";
    std::cerr << "  KONSOLOWY SYMULATOR CZARNEJ DZIURY\n";
    std::cerr << "  (Wcisnij ENTER, aby uzyc domyslnych)\n";
    std::cerr << "========================================\n\n";

    //  USTAWIENIA RENDERU
    std::cerr << "--- RENDER ---\n";
    get_input("Szerokosc obrazu (px)", cam.image_width);
    get_input("Jakosc antialiasingu (probki)", cam.samples_per_pixel);

    // USTAWIENIA FIZYKI
    std::cerr << "\n--- FIZYKA ---\n";
    get_input("Promien horyzontu zdarzen (Rs)", cam.event_horizon_radius);
    get_input("Wewnetrzny promien dysku", cam.disk_inner_radius);
    get_input("Zewnetrzny promien dysku", cam.disk_outer_radius);

    //  USTAWIENIA KAMERY
    std::cerr << "\n--- KAMERA ---\n";
    // Współrzędne wektora lookfrom (X, Y, Z)
    get_input("Pozycja kamery X", cam.lookfrom.e[0]);
    get_input("Pozycja kamery Y", cam.lookfrom.e[1]);
    get_input("Pozycja kamery Z", cam.lookfrom.e[2]);

    //  PLANETY TŁA
    std::cerr << "\n--- PLANETY --- \n";
    std::cerr << "Czy chcesz dodac planete w tle? (t/n) [n]: ";
    std::string add_p;
    std::getline(std::cin, add_p);

    if (add_p == "t" || add_p == "T") {
        double p_x = 2.0, p_y = 0.0, p_z = -6.0, p_r = 1.5;
        get_input("Pozycja planety X", p_x);
        get_input("Pozycja planety Y", p_y);
        get_input("Pozycja planety Z", p_z);
        get_input("Promien planety", p_r);

        world.add(std::make_shared<sphere>(point3(p_x, p_y, p_z), p_r));
    }

    std::cerr << "\n----------------------------------------\n";
    std::cerr << "ROZPOCZYNAM RENDEROWANIE...\n";

    cam.render(world);

    return 0;
}


/*

#include "camera.h"
#include "hittable_list.h"
#include "sphere.h"
#include "color.h"

int main() {
    // 1. Tworzymy układ
    hittable_list space;

    // 2. Wrzucamy do niego obiekty używając inteligentnych wskaźników
    // make_shared rezerwuje miejsce w pamięci komputera i automatycznie je sprząta po zakończeniu
    //space.add(make_shared<sphere>(point3(4,0,-3), 0.5));
    space.add(make_shared<sphere>(point3(0.5,0.7,-3), 0.3));
    space.add(make_shared<sphere>(point3(-4.9,2.2,-30), 3)); //(lewo prawo, góra dół, głębia)

    // 3. Ustawiamy i odpalamy kamerę, przekazując jej nasz uklad
    camera cam;
    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width  = 400;

    cam.render(space);

    return 0;
}
*/