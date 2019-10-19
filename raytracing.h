#include <random>
#include <math.h>

#include "vec3.h"
#include "shade.h"
#include "scene.h"
#include "intersect.h"
#include "FreeImage/FreeImage.h"

/* Image settings */

#define WIDTH 1000
#define HEIGHT 1000
#define BPP 24

/* Ray tracing settings */

#define MAX_DEPTH 0 // Nombre de rebonds des rayons de lumière
#define NB_LIGHTS 1 // Nombre de rayons de lumière par lampe

const float acne = 0.01f;

/* Générateur de nombre aléatoire */

std::random_device device;
std::mt19937 rng(device());
std::uniform_int_distribution<std::mt19937::result_type> aleatoire(0, 500);

Vec3F trace_ray(Light &light, Ray &ray, Boxs &boxs)
{
    Vec3F color, ret;
    color = ret = {0.f, 0.f, 0.f};
    size_t boxs_count = boxs.size();
    for (unsigned int b = 0; b < boxs_count; b++)
    {
        Intersection inter_box, intersection;
        if (intersect_box(ray, boxs.at(b), inter_box))
        {
            if (boxs.at(b).depth < DEPTH_BOX)
            {
                Vec3F ret_temp = trace_ray(light, ray, boxs.at(b).boxs);
                Vec3F noir = {0.f, 0.f, 0.f};
                if (ret == noir) ret = ret_temp;
            } else if (inter_box.position.z < 500.f && intersect_scene(ray, boxs.at(b), intersection))
            {
                for (int k = 0; k < NB_LIGHTS; k++)
                {
                    light.position.x = 250.f + (float)aleatoire(device);
                    light.position.y = 250.f + (float)aleatoire(device);
                    intersection.position = intersection.position + (intersection.normale * acne);
                    Ray ray_shadow;
                    ray_shadow.origin = intersection.position;
                    ray_shadow.direction = normalize(light.position - intersection.position);
                    Intersection inter_shadow;
                    if (!intersect_scene(ray_shadow, boxs.at(b), inter_shadow) || coord_out_of_scene(inter_shadow.position))
                    {
                        Vec3F inv = ray.direction * -1.f;
                        color = color + (shade(intersection.normale, inv, ray_shadow.direction, light.color, intersection.object.material) / (float)NB_LIGHTS * 20.f);
                    }
                }
                if (ray.depth < MAX_DEPTH)
                {
                    Ray ray_reflect;
                    Vec3F dir_ray_reflect = reflect(ray.direction, intersection.normale);
                    dir_ray_reflect = normalize(dir_ray_reflect);
                    ray_reflect.origin = intersection.position + dir_ray_reflect * acne;
                    ray_reflect.direction = dir_ray_reflect;
                    ray_reflect.depth = ray.depth + 1;
                    Vec3F color_reflect = trace_ray(light, ray_reflect, boxs);
                    ret = color + intersection.object.material.specularColor * RDM_Fresnel(dot(ray_reflect.direction, intersection.normale), 1.f, intersection.object.material.IOR) * color_reflect;
                }
                else
                {
                    ret = color;
                }
            }
            else
            {
                return ret;
            }
        }
    }
    return ret;
}

bool render_image(Light &light, Boxs &boxs)
{
    /* Initialisation de l'image */

    FreeImage_Initialise();
    FIBITMAP *bitmap = FreeImage_Allocate(WIDTH, HEIGHT, BPP);

    if (!bitmap)
        return 1;

    /* Traitement */

    Vec3F camera = {500.f, 500.f, 3000.f};

    for (int j = 0; j < HEIGHT; j++)
    {
		#pragma omp parallel for
        for (int i = 0; i < WIDTH; i++)
        {
            Ray ray;
            ray.origin = {(float)i, (float)j, 1000.f};
            ray.direction = normalize(ray.origin - camera);
            // ray.direction = {0.f, 0.f, -1.f};
            ray.depth = 0;

            Vec3F color = trace_ray(light, ray, boxs);

            RGBQUAD colorPixel;
            colorPixel.rgbRed = clamp_color(color.x);
            colorPixel.rgbGreen = clamp_color(color.y);
            colorPixel.rgbBlue = clamp_color(color.z);
            FreeImage_SetPixelColor(bitmap, i, j, &colorPixel);
        }
        if ((j + 1) % 10 == 0)
        {
            printf("Rendering image... %d %%\r", ((j + 1) / 10));
        }
    }

    /* Écriture de l'image */

    if (FreeImage_Save(FIF_PNG, bitmap, "out.png", 0))
        printf("\nImage successfully saved!\n");
    FreeImage_DeInitialise();

    return true;
}