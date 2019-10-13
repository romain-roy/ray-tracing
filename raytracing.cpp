#include <ctime>
#include <random>
#include <math.h>
#include <fstream>
#include <iostream>

#include "vec3.h"
#include "utils.h"
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

const float acne = 0.0001f;

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
		if (intersect_box(ray, boxs.at(b)))
		{
			if (boxs.at(b).depth < DEPTH_BOX)
			{
				return trace_ray(light, ray, boxs.at(b).boxs);
			}
			Intersection intersection;
			if (intersect_scene(ray, boxs.at(b), intersection))
			{
				for (int k = 0; k < NB_LIGHTS; k++)
				{
					light.position.x = 250.f + (float)aleatoire(device);
					light.position.z = 250.f + (float)aleatoire(device);
					intersection.position = intersection.position + (intersection.normale * acne);
					Ray ray_shadow;
					ray_shadow.origin = intersection.position;
					ray_shadow.direction = normalize(light.position - intersection.position);
					Intersection inter_shadow;
					if (!intersect_scene(ray_shadow, boxs.at(b), inter_shadow) || inter_shadow.position.x < 0.f || inter_shadow.position.x > 1000.f || inter_shadow.position.y < 0.f || inter_shadow.position.y > 1000.f || inter_shadow.position.z < 0.f || inter_shadow.position.z > 1000.f)
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
				return {0.f, 0.f, 0.f};
			}
		}
	}
	return ret;
}

bool render_image(Boxs &boxs, Light &light)
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

int main()
{
	printf("RAY TRACING by Romain Roy\n-------------------------\n");

	std::time_t t1 = std::time(0);
	std::tm *start = std::localtime(&t1);
	printf("Start time: %d:%d:%d\n", start->tm_hour, start->tm_min, start->tm_sec);

	printf("Rendering image... 0 %%\r");

	Light light;
	Boxs boxs;

	if (!init_scene(light, boxs))
		return 1;

	if (render_image(boxs, light))
	{
		std::time_t t2 = std::time(0);
		std::tm *finish = std::localtime(&t2);
		printf("Finish time: %d:%d:%d\n", finish->tm_hour, finish->tm_min, finish->tm_sec);

		std::time_t t3 = t2 - t1;
		std::tm *compute = std::localtime(&t3);
		printf("Compute time: %d:%d:%d\n", (compute->tm_hour - 1), compute->tm_min, compute->tm_sec);
		return 0;
	}

	return 1;
}