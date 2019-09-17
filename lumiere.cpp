#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "vec3.h"
#include "FreeImage/FreeImage.h"

#define WIDTH 600
#define HEIGHT 600
#define BPP 24

struct Ray
{
	Vec3F pos, dir;
};

struct Sphere
{
	Vec3F pos;
	float rayon;
};

/* Permute deux variables */
template <typename T>
void permute(T &a, T &b)
{
	T c = a;
	a = b;
	b = c;
}

bool intersectSphere(Ray &ray, Sphere &sphere, float &t)
{
	Vec3F pos = ray.pos - sphere.pos;
	float a = 1.0f; // dot2(ray.dir);
	float b = 2.0f * dot(ray.dir, pos);
	float c = dot2(pos) - sphere.rayon * sphere.rayon;
	// float b = 2.0f * (dot(ray.pos, ray.dir) - dot(sphere.pos, ray.dir));
	// float c = dot2(ray.pos) + dot2(sphere.pos) - 2.0f * dot(sphere.pos, ray.pos) - sphere.rayon * sphere.rayon;
	float delta = b * b - 4.0f * a * c;
	if (delta < 0.0f)
		return false;
	else if (delta == 0.0f)
		t = -b / 2.0f * a;
	else
	{
		float t1 = (-b - sqrtf(delta)) / (2.0f * a);
		float t2 = (-b + sqrtf(delta)) / (2.0f * a);
		if (t1 > t2)
			permute(t1, t2);
		t = t1;
	}
	if (t >= 0.0f)
		return true;
	return false;
}

int main(int argc, char *argv[])
{
	// Préparations image

	FreeImage_Initialise();
	FIBITMAP *bitmap = FreeImage_Allocate(WIDTH, HEIGHT, BPP);
	RGBQUAD color;

	if (!bitmap)
		return 1;

	color.rgbRed = color.rgbGreen = color.rgbBlue = 255; // Pixel blanc

	for (int j = 0; j < HEIGHT; j++) // Remplis l'image de blanc
		for (int i = 0; i < WIDTH; i++)
			FreeImage_SetPixelColor(bitmap, i, j, &color);

	color.rgbRed = color.rgbGreen = color.rgbBlue = 0; // Pixel noir

	// Données

	Sphere s;
	Ray r;
	float t;

	s.pos = {300.0f, 300.0f, 300.0f};
	s.rayon = 150.0f;

	r.pos = {300.0f, 300.0f, 0.0f};
	r.dir = {0.0f, 0.0f, 1.0f};

	// Dessine l'image

	for (float j = 0.0f; j < HEIGHT; j++)
		for (float i = 0.0f; i < WIDTH; i++)
		{
			r.pos = {i, j, 0.0f};
			if (intersectSphere(r, s, t))
				FreeImage_SetPixelColor(bitmap, (int)i, (int)j, &color);
		}

	// Ecrit l'image

	if (FreeImage_Save(FIF_PNG, bitmap, "out.png", 0))
		std::cout << "Image successfully saved!" << std::endl;
	FreeImage_DeInitialise();

	return 0;
}

// Compiler : g++ lumiere.cpp -o lumiere.out -Wall -std=c++11 -lfreeimage
