#include <stdio.h>
#include <vector>
#include <random>
#include "vec3.h"
#include "structures.h"
#include "FreeImage/FreeImage.h"

#define WIDTH 1000
#define HEIGHT 1000
#define BPP 24

const float acne = 1e-4;

float clamp(float v, float min, float max)
{
	if (v > max)
		v = max;
	else if (v < min)
		v = min;
	return v;
}

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
	if (intersection.distance > 0.0f)
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
			if (intersections.at(k).distance < intersection.distance)
				intersection = intersections.at(k);
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

	Vec3F rouge, vert, bleu, blanc, noir, c;

	rouge = {255.0f, 0.0f, 0.0f};
	vert = {0.0f, 255.0f, 0.0f};
	bleu = {0.0f, 0.0f, 255.0f};
	blanc = {255.0f, 255.0f, 255.0f};
	noir = {0.0f, 0.0f, 0.0f};

	Sphere s1, s2, s3, sw1, sw2, sw3, sw4, sw5;

	s1.position = {500.0f, 500.0f, 500.0f};
	s1.radius = 250.0f;
	s1.color = vert;

	s2.position = {350.0f, 350.0f, 200.0f};
	s2.radius = 150.0f;
	s2.color = rouge;

	s3.position = {650.0f, 650.0f, 800.0f};
	s3.radius = 150.0f;
	s3.color = bleu;

	sw1.color = {247, 220, 111};
	sw2.color = {128, 0, 128};
	sw3.color = {46, 204, 113};
	sw4.color = {205, 92, 92};
	sw5.color = {247, 220, 111};
	sw1.radius = sw2.radius = sw3.radius = sw4.radius = sw5.radius = 10000.0f;
	sw1.position = {500.0f, 500.0f, 10970.0f};
	sw2.position = {500.0f, 10970.0f, 500.0f};
	sw3.position = {10970.0f, 500.0f, 500.0f};
	sw4.position = {-9970.0f, 500.0f, 500.0f};
	sw5.position = {500.0f, -9970.0f, 500.0f};

	Lights lights;

	int nbLights = 50;

	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_int_distribution<std::mt19937::result_type> dist1000(0, 1000);

	Spheres spheres;

	spheres.push_back(s1);
	spheres.push_back(s2);
	spheres.push_back(s3);

	spheres.push_back(sw1);
	spheres.push_back(sw2);
	spheres.push_back(sw3);
	spheres.push_back(sw4);
	spheres.push_back(sw5);

	Ray r;

	r.direction = {0.0f, 0.0f, 1.0f};

	Intersection inter;

	Vec3F camera = {500.0f, 500.0f, -2000.0f};

	// Traitement

	for (int j = 0; j < HEIGHT; j++)
	{
		for (int i = 0; i < WIDTH; i++)
		{
			r.origin = {(float)i, (float)j, 0.0f};
			r.direction = normalize(r.origin - camera);
			c = {0.0f, 0.0f, 0.0f};
			if (intersectScene(r, spheres, inter))
			{
				colorPixel.rgbRed = colorPixel.rgbGreen = colorPixel.rgbBlue = 0.0f;
				lights.clear();
				for (int i = 0; i < nbLights; i++)
				{
					Light l;
					l.position.x = dist1000(dev);
					l.position.y = 950;
					l.position.z = -2000.0f;
					lights.push_back(l);
				}
				size_t lightCount = lights.size();
				for (unsigned int k = 0; k < lightCount; k++)
				{
					Ray ray_to_light;
					Intersection inter_light;
					inter.position = inter.position + (inter.normale * acne);
					ray_to_light.origin = inter.position;
					ray_to_light.direction = normalize(lights[k].position - inter.position);
					float cos = dot(normalize(inter.normale), ray_to_light.direction);
					if (!intersectScene(ray_to_light, spheres, inter_light))
						c = c + (inter.sphere.color * cos) / nbLights;
					colorPixel.rgbRed = clamp(c.x, 0.0f, 255.0f);
					colorPixel.rgbGreen = clamp(c.y, 0.0f, 255.0f);
					colorPixel.rgbBlue = clamp(c.z, 0.0f, 255.0f);
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

// Compiler : g++ lumiere.cpp -o lumiere.out -Wall -Wextra -pedantic -ansi -std=c++11 -lfreeimage
