//klasa do obsługi wektorów w przestrzeni 3d i kolorów RGB
//wiedza zdobyta z wpisu na https://raytracing.github.io/books/RayTracingInOneWeekend.html

#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>

using std::sqrt;

class vec3 {
public:
    double e[3]; //tablica w której trzymamy nasze 3 wartości (x, y, z) albo (r, g, b)
    //Konstruktory
    vec3() : e{0, 0, 0} {}
    vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}

    double x() const { return e[0]; }
    double y() const { return e[1]; }
    double z() const { return e[2]; }

    //1. Przeciązenia operatorow potrzebne by moc wykonywac operacje na wektorach korzystajac za pomoca znakow a nie wywolywania funkcji

    //odwrócenie wektora (zamiast na przyklad vec3.reverse mozna napisac -vec3)
    vec3 operator-() const {
        return vec3(-e[0], -e[1], -e[2]);
    }
    //zweocenie wartosci wektora w itym wymiarze (tylko do odczytu)
    double operator[](int i) const {
        return e[i];
    }
    // zwrócenie wartosci wektora w itym wymiarze (do zapisu read/write)
    double& operator[](int i) {
        return e[i];
    }

    vec3& operator+=(const vec3 &v) {
        e[0] += v.e[0];
        e[1] += v.e[1];
        e[2] += v.e[2];
        return *this;
    }
    //odejmowania nie trzeba robic, wystarczy dodac odwrotnosc wektora
    vec3& operator*=(const double t) {
        e[0] *= t;
        e[1] *= t;
        e[2] *= t;
        return *this;
    }

    vec3& operator/=(const double t) {
        return *this *= 1 / t;
    }

    double length() const {
        return sqrt(length_squared());
    }
    double length_squared() const {
        return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
    }
};

//Aliasy
using point3 = vec3; //punkt w przestrzeni 3d
using color = vec3; //kolor

//narzędzia pomocnicze (nie sa zdefiniowane w klasie z kilku powodów, chociazby gdyby w klasie byl zdefiniowany operator *
//to dzialalby tylko kiedy napiszemy v * 3.0, a nie jak zrobimy 3.0 * v

inline std::ostream& operator<<(std::ostream &out, const vec3 &v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3 &u, const vec3 &v) {
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3 &u, const vec3 &v) {
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}
//mnozenie dwoch wektorow
inline vec3 operator*(const vec3 &u, const vec3 &v) {
    return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2]);
}

inline vec3 operator*(double t, const vec3 &v) {
    return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3 operator*(const vec3 &v, double t) {
    return t * v;
}

inline vec3 operator/(const vec3 &v, double t) {
    return (1/t) * v; //kolejnosc jest wazna, najpierw dzielenie, potem wektor jednostkowy
}
//Iloczyn skalarny
inline double dot(const vec3 &u, const vec3 &v) {
    return u.e[0] * v.e[0] + u.e[1] * v.e[1] + u.e[2] * v.e[2];
}
//Iloczyn wektorowy
inline vec3 cross(const vec3 &u, const vec3 &v) {
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                u.e[2] * v.e[0] - u.e[0] * v.e[2],
                u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}
//Tensor
inline vec3 unit_vector(vec3 v) {
    return v / v.length();
}

#endif