#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"

//(A+tb−C)⋅(A+tb−C)=r^2  ==>  at^2+bt+c=0

//Delta < 0 => promien ominal obiekt
//Delta = 0 => promien idealnie otarl sie o krawedz obiektu
//Delta > 0 => promien wszedl z jednej strony kuli i wyszedl z drugiej

class sphere : public hittable {
public:
    point3 center;
    double radius;

    sphere() {}
    sphere(point3 cen, double r) : center(cen), radius(r) {}

    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override {
        vec3 oc = r.origin() - center;

        double a = r.direction().length_squared();
        double half_b = dot(oc, r.direction());
        double c = oc.length_squared() - radius * radius;

        double discriminant = half_b * half_b - a * c;
        if (discriminant < 0) return false;

        double sqrtd = sqrt(discriminant);

        double root = (-half_b - sqrtd) / a;
        if (root < t_min || t_max < root) {
            root = (-half_b + sqrtd) / a;
            if (root < t_min || t_max < root) {
                return false;
            }
        }

        rec.t = root; //jak dlugo lecial promien
        rec.p = r.at(rec.t); //konkretne koordynaty gdize promien uderzyl
        vec3 outward_normal = (rec.p - center) / radius;  //Wektor normalny uderzenia
        rec.set_face_normal(r, outward_normal);

        return true;
    }
};

#endif