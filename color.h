//plik narzędziowy zajmujący się konwersją formatu
#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include <iostream>

//Funkcja przyjmuje wartosc 0.0 (brak koloru) - 1.0 (pełny kolor) i zamienia go na wartości RGB (0-255)

void write_color(std::ostream &out, color pixel_color) {
    out << static_cast<int>(255.999 * pixel_color.x()) << ' '
        << static_cast<int>(255.999 * pixel_color.y()) << ' '
        << static_cast<int>(255.999 * pixel_color.z()) << '\n';
}

#endif