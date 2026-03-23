#include "camera.h"
#include "hittable_list.h"
#include "sphere.h"

int main() {
    // 1. Tworzymy układ
    hittable_list space;

    // 2. Wrzucamy do niego obiekty używając inteligentnych wskaźników
    // make_shared rezerwuje miejsce w pamięci komputera i automatycznie je sprząta po zakończeniu
    space.add(make_shared<sphere>(point3(0,0,-3), 0.5));
    space.add(make_shared<sphere>(point3(0.5,0.5,-3), 0.3));
    space.add(make_shared<sphere>(point3(-0.5,0.5,-3), 0.3));

    // 3. Ustawiamy i odpalamy kamerę, przekazując jej nasz uklad
    camera cam;
    cam.aspect_ratio = 16.0 / 9.0;
    cam.image_width  = 400;

    cam.render(space);

    return 0;
}