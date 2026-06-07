
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <string>
#include <sstream>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <AppKit/AppKit.h>

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <simd/simd.h>

struct Planet { simd::float3 pos; float radius; float padding; };

struct alignas(16) Uniforms {
    simd::float3 lookfrom, lookat, vup, bh_center;
    Planet planets[10];
    int num_planets;
    float Rs, disk_inner, disk_outer, focal_length, step_size;
    int max_steps, image_width, image_height, samples, current_fps;
    int show_grid, show_planets, photon_active;
    simd::float3 photon_pos;

    int trail_length;
    float pad1, pad2, pad3;
    // Pamięć na cały długi lot fotonu (200 klatek zapisu)
    simd::float3 trail[200];
};

float camera_theta = 0.0f, camera_phi = 0.4f, camera_radius = 12.0f, target_fps = 60.0f;
simd::float3 free_pos = {0, 2, 12};
bool is_free_fly = false;

template <typename T>
void get_input(const std::string& prompt, T& value) {
    std::cout << prompt << " [" << value << "]: ";
    std::string input; std::getline(std::cin, input);
    if (!input.empty()) { std::stringstream ss(input); ss >> value; }
}

int main() {
    Uniforms params;
    params.num_planets = 0;
    params.image_width = 600;
    params.max_steps = 400;
    params.Rs = 0.15f;
    params.disk_inner = 0.8f; params.disk_outer = 2.5f; params.step_size = 0.1f;
    params.samples = 1; params.focal_length = 2.0f;
    params.bh_center = {0, 0, 0}; params.show_grid = 0; params.show_planets = 1;
    params.photon_active = 0; params.trail_length = 0;

    std::cout << "===========================================\n";
    std::cout << "   SYMULATOR CZARNEJ DZIURY (REALTIME)     \n";
    std::cout << "===========================================\n\n";

    get_input("Szerokosc okna", params.image_width);
    get_input("Jakosc (samples AA)", params.samples);
    get_input("Promien horyzontu zdarzen (Rs)", params.Rs);
    get_input("Zasieg promieni / jakosc detali (max_steps)", params.max_steps);
    get_input("Docelowe FPS", target_fps);

    int gc = 0;
    get_input("Wlaczyc siatke na starcie? (1=Tak, 0=Nie)", gc);
    params.show_grid = gc;

    while (params.num_planets < 10) {
        std::cout << "\nDodac wlasna planete " << params.num_planets+1 << "? (t/n): ";
        std::string choice; std::getline(std::cin, choice);
        if (choice != "t" && choice != "T") break;

        float px = 0.0f, py = 0.0f, pz = -6.0f;
        get_input("  Pozycja X", px);
        get_input("  Pozycja Y", py);
        get_input("  Pozycja Z", pz);

        params.planets[params.num_planets].pos = simd::make_float3(px, py, pz);

        float pr = 0.5f;
        get_input("  Promien planety", pr);
        params.planets[params.num_planets].radius = pr;

        params.num_planets++;
    }

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    params.image_height = params.image_width / (16.0/9.0);
    GLFWwindow* window = glfwCreateWindow(params.image_width, params.image_height, "Black Hole Explorer", NULL, NULL);

    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    NSWindow *nswin = (NSWindow *)glfwGetCocoaWindow(window);
    CAMetalLayer *layer = [CAMetalLayer layer];
    layer.device = (__bridge id<MTLDevice>)device;
    layer.pixelFormat = MTLPixelFormatRGBA8Unorm;
    nswin.contentView.layer = layer;
    nswin.contentView.wantsLayer = YES;

    MTL::CommandQueue* queue = device->newCommandQueue();
    NS::Error* err = nullptr;
    MTL::Library* lib = device->newLibrary(NS::String::string("shader.metallib", NS::UTF8StringEncoding), &err);
    MTL::ComputePipelineState* pipeline = device->newComputePipelineState(lib->newFunction(NS::String::string("render_black_hole_to_tex", NS::UTF8StringEncoding)), &err);

    double last_t = glfwGetTime(); int frames = 0, fps_display = 0;
    bool k1_p = false, k2_p = false, sp_p = false, tb_p = false, r_p = false, p_p = false;
    simd::float3 ph_dir = {0,0,0};

    std::cout << "\nGotowe! Sterowanie:\n"
              << "[TAB]: Przelacz Orbit/Free | [SPACE]: Strzal/Pauza fotonu | [R]: Reset Fotonu\n"
              << "[1/2]: Wl/Wyl Siatke/Planety | [P]: Postaw planete przed soba\n"
              << "[+ / -]: Zmiana masy czarnej dziury na zywo\n"
              << "[WSAD]: Ruch | [STRZALKI]: Rozgladanie (w trybie Free)\n";

    while (!glfwWindowShouldClose(window)) {
        auto t_start = std::chrono::high_resolution_clock::now();
        glfwPollEvents();
        double time = glfwGetTime(); frames++;
        if (time - last_t >= 0.5) { fps_display = (int)(frames / (time - last_t)); frames = 0; last_t = time; }
        params.current_fps = fps_display;

        // Przełączanie kamery
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tb_p) {
            is_free_fly = !is_free_fly;
            if (is_free_fly) {
                free_pos = params.lookfrom;
                camera_phi = -camera_phi;
                camera_theta += M_PI;
            } else {
                simd::float3 rel = free_pos - params.bh_center;
                camera_radius = simd::length(rel);
                camera_phi = asin(rel.y / camera_radius);
                camera_theta = atan2(rel.x, rel.z);
            }
        }
        tb_p = (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !k1_p) params.show_grid = !params.show_grid;
        k1_p = (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS);
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !k2_p) params.show_planets = !params.show_planets;
        k2_p = (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS);

        // Foton
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !sp_p) {
            if (params.photon_active == 0) {
                params.photon_active = 1;
                params.trail_length = 0;
                simd::float3 f = simd::normalize(params.lookat - params.lookfrom);
                simd::float3 r = simd::normalize(simd::cross(f, simd::make_float3(0.0f, 1.0f, 0.0f)));
                params.photon_pos = params.lookfrom + r * 2.5f;
                ph_dir = f;
            } else if (params.photon_active == 1) {
                params.photon_active = 2;
            } else if (params.photon_active == 2) {
                params.photon_active = 1;
            }
        }
        sp_p = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !r_p) {
            params.photon_active = 0;
            params.trail_length = 0;
        }
        r_p = (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);

        // Dynamiczne planety
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !p_p) {
            if (params.num_planets < 10) {
                simd::float3 f = simd::normalize(params.lookat - params.lookfrom);
                params.planets[params.num_planets].pos = params.lookfrom + f * 4.0f;
                params.planets[params.num_planets].radius = 0.5f;
                params.num_planets++;
                std::cout << "\n[LIVE] Utworzono nowa planete przed kamera! (" << params.num_planets << "/10)\n";
            }
        }
        p_p = (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS);

        // Modyfikacja Rs
        bool rs_changed = false;
        if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
            params.Rs -= 0.002f;
            if (params.Rs < 0.01f) params.Rs = 0.01f;
            rs_changed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
            params.Rs += 0.002f;
            rs_changed = true;
        }
        if (rs_changed) {
            std::cout << "\r[ZMIANA NA ZYWO] Aktualny promien Rs: " << params.Rs << "    " << std::flush;
        }

        // Ruch
        float speed = 0.15f, rot = 0.04f;
        if (!is_free_fly) {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera_phi += rot;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera_phi -= rot;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera_theta -= rot;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera_theta += rot;
            if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) camera_radius -= speed*3;
            if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) camera_radius += speed*3;
            if (camera_phi > 1.55f) camera_phi = 1.55f; if (camera_phi < -1.55f) camera_phi = -1.55f;
            params.lookfrom = {camera_radius*cos(camera_phi)*sin(camera_theta), camera_radius*sin(camera_phi), camera_radius*cos(camera_phi)*cos(camera_theta)};
            params.lookat = params.bh_center;
        } else {
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) camera_phi += rot;
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) camera_phi -= rot;
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) camera_theta -= rot;
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) camera_theta += rot;
            simd::float3 f = {cos(camera_phi)*sin(camera_theta), sin(camera_phi), cos(camera_phi)*cos(camera_theta)};
            simd::float3 r = simd::normalize(simd::cross(f, simd::make_float3(0.0f, 1.0f, 0.0f)));
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) free_pos += f * speed;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) free_pos -= f * speed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) free_pos += r * speed;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) free_pos -= r * speed;
            params.lookfrom = free_pos; params.lookat = free_pos + f;
        }
        params.vup = {-sin(camera_phi)*sin(camera_theta), cos(camera_phi), -sin(camera_phi)*cos(camera_theta)};

        // Fizyka Fotonu - CO KLATKĘ, DŁUGI OGON
        if (params.photon_active == 1) {
            for(int i=0; i<4; i++) {
                simd::float3 v = params.bh_center - params.photon_pos;
                float d = simd::length(v);
                if (d < params.Rs || d > 150.0f) { params.photon_active = 2; break; }
                float h2 = simd::length_squared(simd::cross(-v, ph_dir));
                float a = -1.5f * params.Rs * h2 / (pow(d, 5.0f) + 1e-7f) * 1.5f;
                ph_dir = simd::normalize(ph_dir + (a * -v) * params.step_size);
                params.photon_pos += ph_dir * params.step_size;
            }

            // Rejestrujemy KAŻDĄ klatkę (bez omijania). Tworzy gładki, spójny ogon na 200 klatek w tył.
            if (params.trail_length >= 200) {
                for(int i=0; i<199; i++) params.trail[i] = params.trail[i+1];
                params.trail_length = 199;
            }
            params.trail[params.trail_length++] = params.photon_pos;
        }

        NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();
        auto dr = (CA::MetalDrawable*)[(__bridge id)layer nextDrawable];
        if (dr) {
            auto cmd = queue->commandBuffer();
            auto enc = cmd->computeCommandEncoder();
            enc->setComputePipelineState(pipeline);
            enc->setTexture(dr->texture(), 0);
            auto b = device->newBuffer(&params, sizeof(Uniforms), MTL::ResourceStorageModeShared);
            enc->setBuffer(b, 0, 1);
            enc->dispatchThreads(MTL::Size(params.image_width, params.image_height, 1), MTL::Size(16, 16, 1));
            enc->endEncoding();
            cmd->presentDrawable(dr);
            cmd->commit();
            b->release();
        }
        pool->release();

        auto t_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> el = t_end - t_start;
        float sleep = (1.0f/target_fps) - el.count();
        if (sleep > 0) std::this_thread::sleep_for(std::chrono::duration<float>(sleep));
    }
    glfwTerminate(); return 0;
}