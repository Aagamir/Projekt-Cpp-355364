#ifndef HITTABLE_H
#define HITTABLE_H

#include <iostream>

#include "ray.h"
#include "vec3.h"

struct hit_record {
    point3 p;        //Dokładny punkt uderzenia w przestrzeni 3D
    vec3 normal;     //Wektor normalny
    double t;        //Dystans przebyty przez promien
    bool front_face; //Czy promien uderza od zewnatrz, czy od srodka

    //Zawzse chcemy by wektor normalny, celował przeciwko kierunkowi promienia
    inline void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class hittable {
public:
    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
};

#endif