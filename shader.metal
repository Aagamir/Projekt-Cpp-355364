#include <metal_stdlib>
using namespace metal;

struct Planet {
    float3 pos;
    float radius;
    float padding;
};

struct Uniforms {
    float3 lookfrom;
    float3 lookat;
    float3 vup;
    float3 bh_center;
    Planet planets[10];
    int num_planets;
    float Rs, disk_inner, disk_outer, focal_length, step_size;
    int max_steps, image_width, image_height, samples, current_fps;
    int show_grid, show_planets, photon_active;
    float3 photon_pos;

    int trail_length;
    float pad1, pad2, pad3;
    float3 trail[200];
};

constant uint font[21] = {
    31599, 9362, 29671, 29391, 23497, 31183, 31215, 29257, 31727, 31695,
    31140, 31716, 31183, 23213, 23186, 29351, 448, 1040, 31661, 11245, 31207
};

constant uint font_alpha[31] = {
    11245, 27566, 31015, 27502, 31207, 31140, 31087, 23533, 29847, 4719,
    23981, 18727, 24429, 31597, 31599, 31716, 31609, 31661, 31183, 29842,
    23407, 23378, 23421, 23213, 23186, 29351,
    0,
    1488,
    448,
    9362,
    29671
};

constant int inst_l0[] = {19, 0, 1, 26, 19, 17, 24, 1};
constant int inst_l1[] = {18, 15, 2, 26, 5, 14, 19, 14, 13};
constant int inst_l2[] = {17, 26, 17, 4, 18, 4, 19};
constant int inst_l3[] = {15, 26, 15, 11, 0, 13, 4, 19, 0};
constant int inst_l4[] = {27, 28, 26, 12, 0, 18, 0};
constant int inst_l5[] = {29, 26, 18, 8, 0, 19, 10, 0};
constant int inst_l6[] = {30, 26, 15, 11, 0, 13, 4, 19, 24};

bool is_pixel_in_char(int char_code, int rx, int ry) {
    if (rx < 0 || rx > 2 || ry < 0 || ry > 4 || char_code < 0 || char_code > 20) return false;
    uint bitmap = font[char_code];
    int bit_index = (4 - ry) * 3 + (2 - rx);
    return (bitmap >> bit_index) & 1;
}

float3 draw_hud(uint2 id, int fps, float3 cam_pos, float3 bh_pos, float3 screen_color, int img_width, int img_height) {
    int screen_y = img_height - 1 - int(id.y);
    int scale = 2;

    int fps_x = 20, fps_y = 20;
    if (int(id.x) >= fps_x && int(id.x) < fps_x + 7*4*scale && screen_y >= fps_y && screen_y < fps_y + 5*scale) {
        int rel_x = (int(id.x) - fps_x) / scale, rel_y = (screen_y - fps_y) / scale;
        int char_idx = rel_x / 4, px = rel_x % 4, c = -1;
        if (char_idx == 0) c = 10; else if (char_idx == 1) c = 11; else if (char_idx == 2) c = 12;
        else if (char_idx >= 4) {
            if (char_idx == 4) c = (fps / 100) % 10;
            else if (char_idx == 5) c = (fps / 10) % 10;
            else if (char_idx == 6) c = fps % 10;
        }
        if (c >= 0 && px < 3 && is_pixel_in_char(c, px, rel_y)) return float3(0.2, 1.0, 0.2);
    }

    int coord_w = 6 * 4 * scale, coord_x = img_width - 20 - coord_w, coord_y = 20;
    if (int(id.x) >= coord_x && int(id.x) < coord_x + coord_w && screen_y >= coord_y && screen_y < coord_y + 17 * scale) {
        int rel_x = (int(id.x) - coord_x) / scale, rel_y = (screen_y - coord_y) / scale;
        int line = rel_y / 6, char_y = rel_y % 6;
        if (line < 3 && char_y < 5) {
            int char_idx = rel_x / 4, px = rel_x % 4, c = -1;
            float3 rel = cam_pos - bh_pos;
            float r = length(rel);
            float theta = atan2(rel.x, rel.z) * (180.0f / 3.14159f);
            float phi = asin(rel.y / (r + 1e-6f)) * (180.0f / 3.14159f);
            int val = (line == 0) ? int(round(r)) : (line == 1) ? int(round(theta)) : int(round(phi));
            if (char_idx == 0) c = (line == 0) ? 18 : (line == 1) ? 19 : 20;
            else if (char_idx == 1) c = 17;
            else if (char_idx == 2) c = (val < 0) ? 16 : -1;
            else {
                int av = abs(val);
                if (char_idx == 3) c = (av / 100) % 10;
                else if (char_idx == 4) c = (av / 10) % 10;
                else if (char_idx == 5) c = av % 10;
            }
            if (c >= 0 && px < 3 && is_pixel_in_char(c, px, char_y)) return float3(1.0, 0.7, 0.2);
        }
    }

    int inst_x = 20, inst_y = 60, line_spacing = 8 * scale;
    if (int(id.x) >= inst_x && int(id.x) < inst_x + 9 * 4 * scale && screen_y >= inst_y && screen_y < inst_y + 7 * line_spacing) {
        int rel_x = (int(id.x) - inst_x) / scale, rel_y = (screen_y - inst_y) / scale;
        int line_idx = rel_y / 8, char_y = rel_y % 8;

        if (char_y < 5) {
            int char_idx = rel_x / 4, px = rel_x % 4;
            if (px < 3) {
                int c = -1;
                if (line_idx == 0 && char_idx < 8) c = inst_l0[char_idx];
                else if (line_idx == 1 && char_idx < 9) c = inst_l1[char_idx];
                else if (line_idx == 2 && char_idx < 7) c = inst_l2[char_idx];
                else if (line_idx == 3 && char_idx < 9) c = inst_l3[char_idx];
                else if (line_idx == 4 && char_idx < 7) c = inst_l4[char_idx];
                else if (line_idx == 5 && char_idx < 8) c = inst_l5[char_idx];
                else if (line_idx == 6 && char_idx < 9) c = inst_l6[char_idx];

                if (c >= 0) {
                    uint bitmap = font_alpha[c];
                    int bit_index = (4 - char_y) * 3 + (2 - px);
                    if ((bitmap >> bit_index) & 1) return float3(1.0, 0.8, 0.2);
                }
            }
        }
    }
    return screen_color;
}

float hash31(float3 p3) {
    p3  = fract(p3 * 0.1031f);
    p3 += dot(p3, p3.yzx + 33.33f);
    return fract((p3.x + p3.y) * p3.z);
}

float3 get_star_color(float3 dir) {
    float3 grid = floor(dir * 600.0f);
    float h = hash31(grid);
    if (h > 0.995f) {
        float brightness = (h - 0.995f) * 200.0f;
        return float3(1.0) * brightness;
    }
    return float3(0.0f);
}

kernel void render_black_hole_to_tex(
    texture2d<float, access::write> output_tex [[texture(0)]],
    constant Uniforms& params [[buffer(1)]],
    uint2 id [[thread_position_in_grid]]
) {
    if (id.x >= output_tex.get_width() || id.y >= output_tex.get_height()) return;

    float aspect = float(params.image_width) / float(params.image_height);
    float3 w = normalize(params.lookfrom - params.lookat);
    float3 u_vec = normalize(cross(params.vup, w));
    float3 v_vec = cross(w, u_vec);

    float3 horiz = (2.0f * aspect) * u_vec;
    float3 vert = 2.0f * v_vec;
    float3 llc = params.lookfrom - horiz/2.0f - vert/2.0f - w * params.focal_length;

    float u = float(id.x) / float(params.image_width);
    float v = float(id.y) / float(params.image_height);

    float3 cur_pos = params.lookfrom;
    float3 cur_dir = normalize(llc + u*horiz + v*vert - params.lookfrom);
    float3 col = 0;
    bool hit = false;
    float photon_glow = 0.0f;
    float3 trail_glow = float3(0.0f);

    for (int i = 0; i < params.max_steps; i++) {
        float3 prev = cur_pos;
        float3 to_bh = params.bh_center - cur_pos;
        float d = length(to_bh);

        if (d < params.Rs * 1.001f) { hit = true; break; }

        if (params.photon_active > 0) {
            float3 diff_ph = cur_pos - params.photon_pos;
            float dist_to_ph2 = dot(diff_ph, diff_ph);

            // Jądro fotonu
            if (dist_to_ph2 < 0.0225f) { col = float3(2.0); hit = true; break; }

            // Poświata fotonu
            photon_glow += 0.0015f / (dist_to_ph2 + 0.001f);

            // Ogon fotonu na calej dlugosci
            // Zamiast nakładania wielu kul, szukamy po prostu najmniejszego dystansu
            // od aktualnego punktu w przestrzeni do CALEGO szlaku, tworząc "rurę" światła.
            if (params.trail_length > 0) {
                float min_d2 = 100.0f;
                // Skaczemy co 2 punkty (j+=2), co drastycznie oszczędza klatki (FPS)
                // ale zachowuje doskonałą ciągłość światła i nie urywa ogona.
                for (int j = 0; j < params.trail_length; j += 2) {
                    float3 diff = cur_pos - params.trail[j];
                    min_d2 = min(min_d2, dot(diff, diff));
                }
                // Jeśli przestrzeń jest blisko ogona, zaświeci się ładnie na niebiesko
                trail_glow += float3(0.1f, 0.4f, 1.0f) * (0.0006f / (min_d2 + 0.001f));
            }
        }

        float3 r_vec = -to_bh;
        float h2 = dot(cross(r_vec, cur_dir), cross(r_vec, cur_dir));
        float3 acceleration = -1.5f * params.Rs * h2 / (pow(d, 5.0f) + 1e-7f) * r_vec;

        cur_dir = normalize(cur_dir + acceleration * params.step_size);
        cur_pos += cur_dir * params.step_size;

        if ((prev.y > params.bh_center.y && cur_pos.y <= params.bh_center.y) ||
            (prev.y < params.bh_center.y && cur_pos.y >= params.bh_center.y)) {
            float dist = length(cur_pos - params.bh_center);
            if (dist >= params.disk_inner && dist <= params.disk_outer) {
                float t = (dist - params.disk_inner) / (params.disk_outer - params.disk_inner);
                col = mix(float3(1.0, 0.4, 0.05), float3(0.05, 0, 0), t);
                hit = true; break;
            }
            else if (params.show_grid == 1 && dist > params.disk_outer && dist < 100.0f) {
                if (fract(cur_pos.x) < 0.04f || fract(cur_pos.z) < 0.04f) {
                    col = float3(0.0, 0.8, 0.3) * (1.0f - dist/100.0f);
                    hit = true; break;
                }
            }
        }

        if (params.show_planets == 1) {
            for (int p = 0; p < params.num_planets; p++) {
                float3 diff_p = cur_pos - params.planets[p].pos;
                if (dot(diff_p, diff_p) < params.planets[p].radius * params.planets[p].radius) {
                    float3 normal = normalize(diff_p);
                    col = ((p % 2 == 0) ? float3(0.3, 0.5, 1.0) : float3(1.0, 0.3, 0.3)) * (0.5f * normal.y + 0.5f);
                    hit = true; break;
                }
            }
        }
        if (hit || length(cur_pos) > 120.0f) break;
    }

    float3 final_col = hit ? col : get_star_color(cur_dir);

    // Nałożenie białej poświaty jądra oraz gładkiego, nieprzerywanego śladu
    final_col += float3(0.5f, 0.8f, 1.0f) * min(photon_glow, 1.0f);
    final_col += min(trail_glow, 1.0f);

    float3 hud_col = draw_hud(id, params.current_fps, params.lookfrom, params.bh_center, final_col, params.image_width, params.image_height);
    output_tex.write(float4(pow(hud_col, 1.0/2.2), 1.0), uint2(id.x, params.image_height - 1 - id.y));
}