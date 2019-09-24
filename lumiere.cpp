#include <stdio.h>
#include <vector>
#include "vec3.h"
#include "structures.h"
#include "FreeImage/FreeImage.h"

#define WIDTH 1000
#define HEIGHT 1000
#define BPP 24

bool intersectSphere(Ray &ray, Sphere &sphere, Intersection &intersection)
{
	Vec3F pos = ray.origin - sphere.position;
	float a = 1.0f; // dot2(ray.direction);
	float b = 2.0f * dot(ray.direction, pos);
	float c = dot2(pos) - sphere.radius * sphere.radius;
	float delta = b * b - 4.0f * a * c;
	if (delta <= 0.0f)
		return false;
	else
	{
		float t1 = (-b - sqrtf(delta)) / (2.0f * a);
		float t2 = (-b + sqrtf(delta)) / (2.0f * a);
		intersection.distance = t1 > 0.0f ? t1 : t2 > 0.0f ? t2 : -1.0f;
	}
	if (intersection.distance >= 0.0f)
	{
		intersection.position = ray.origin + ray.direction * intersection.distance;
		intersection.normale = normalize(intersection.position - sphere.position);
		intersection.sphere = sphere;
		return true;
	}
	return false;
}

bool intersectScene(Ray &ray, Spheres &spheres, Intersection &intersection)
{
	Intersections intersections;
	size_t sphereCount = spheres.size();
	for (unsigned int k = 0; k < sphereCount; k++)
	{
		Sphere sphere = spheres.at(k);
		if (intersectSphere(ray, sphere, intersection))
			intersections.push_back(intersection);
	}
	if (!intersections.empty())
	{
		intersection = intersections.at(0);
		for (unsigned int k = 1; k < intersections.size(); k++)
		{
			if (intersections.at(k).distance < intersection.distance)
			{
				intersection = intersections.at(k);
			}
		}
		return true;
	}
	return false;
}

int main()
{
	FreeImage_Initialise();
	FIBITMAP *bitmap = FreeImage_Allocate(WIDTH, HEIGHT, BPP);
	RGBQUAD colorPixel;

	if (!bitmap)
		return 1;

	// Données

	Vec3F rouge, vert, bleu, blanc;

	rouge = {255.0f, 0.0f, 0.0f};
	vert = {0.0f, 255.0f, 0.0f};
	bleu = {0.0f, 0.0f, 255.0f};
	blanc = {255.0f, 255.0f, 255.0f};

	Sphere s1, s2, s3;

	s1.position = {500.0f, 500.0f, 500.0f};
	s1.radius = 250.0f;
	s1.color = vert;

	s2.position = {350.0f, 350.0f, 200.0f};
	s2.radius = 150.0f;
	s2.color = rouge;

	s3.position = {650.0f, 650.0f, 800.0f};
	s3.radius = 150.0f;
	s3.color = bleu;

	Light l1, l2;

	l1.position = {0.0f, 1000.0f, 0.0f};
	l1.color = blanc;
	l1.intensity = 1.0f;

	l2.position = {0.0f, 0.0f, 0.0f};
	l2.color = blanc;
	l2.intensity = 1.0f;

	Lights lights;

	lights.push_back(l1);
	lights.push_back(l2);

	Spheres spheres;

	spheres.push_back(s1);
	spheres.push_back(s2);
	spheres.push_back(s3);

	Ray r;

	r.direction = {0.0f, 0.0f, 1.0f};

	Intersection inter;

	// Traitement

	for (int j = 0; j < HEIGHT; j++)
	{
		for (int i = 0; i < WIDTH; i++)
		{
			r.origin = {(float)i, (float)j, 0.0f};
			if (intersectScene(r, spheres, inter))
			{
				colorPixel.rgbRed = inter.sphere.color.x;
				colorPixel.rgbGreen = inter.sphere.color.y;
				colorPixel.rgbBlue = inter.sphere.color.z;
				size_t lightCount = lights.size();
				for (unsigned int k = 0; k < lightCount; k++)
				{
					Ray ray_to_light;
					ray_to_light.origin = inter.position;
					ray_to_light.direction = normalize(lights[k].position - inter.position);
					Intersection inter_light;
					if (intersectScene(ray_to_light, spheres, inter_light))
					{
						colorPixel.rgbRed *= 0.75f;
						colorPixel.rgbGreen *= 0.75f;
						colorPixel.rgbBlue *= 0.75f;
					}
				}
				FreeImage_SetPixelColor(bitmap, i, j, &colorPixel);
			}
		}
	}

	// Ecriture de l'image

	if (FreeImage_Save(FIF_PNG, bitmap, "out.png", 0))
		printf("Image successfully saved!\n");
	FreeImage_DeInitialise();

	return 0;
}

// Compiler : g++ lumiere.cpp -o lumiere.out -Wall -pedantic -Wextra -ansi -std=c++11 -lfreeimage
