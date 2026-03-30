/*
 * 27.03.2026
 * Implementujemy fizyke za pomoca
 * ray marchingu polaczonego z calkowaniem numerycznym
*/
#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray {
    public:
    point3 orig; //Punkt startowy kamery
    vec3 dir;    //Kierunek w strone konkretnego piksela

    ray() {}
    ray(const point3& origin, const vec3& direction)
        :orig(origin), dir(direction) {}
    point3 origin() const { return orig; }
    vec3 direction() const { return dir; }

    //Wzór raytracingu: P(t) = A + t*b
    //Zwraca nam punkt w przestrzeni 3D w którym promien znajdzie sie po czasie t
    point3 at(double t) const {
        return orig + t * dir;
    }
};

#endif

//zobaczyc czy promiene sa rownolegle czy nie
//wunkcja zwaracajaca pozycje promienia w zakrzywienu