#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h" //zobaczycz co dziedziczy po czym
#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

class hittable_list : public hittable {
public:
    // Wektor przechowujący inteligentne wskaźniki na obiekty
    std::vector<shared_ptr<hittable>> objects;

    hittable_list() {}
    hittable_list(shared_ptr<hittable> object) { add(object); }

    void clear() { objects.clear(); }
    void add(shared_ptr<hittable> object) { objects.push_back(object); }

    // Wypuszczamy promień na całą listę i szukamy najbliższego trafienia
    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override {
        hit_record temp_rec;
        bool hit_anything = false;
        double closest_so_far = t_max;

        for (const auto& object : objects) {
            // Jeśli obiekt jest bliżej niż poprzedni rekord, aktualizujemy raport
            if (object->hit(r, t_min, closest_so_far, temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }
        return hit_anything;
    }
};

#endif