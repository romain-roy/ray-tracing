#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include "vec3.h"
#include "FreeImage/FreeImage.h"

#define WIDTH 1000
#define HEIGHT 1000
#define BPP 24

struct Ray
{
	Vec3F pos, dir;
	float intensite;
	int depth;
};

struct Sphere
{
	Vec3F pos;
	float rayon;
	Vec3F couleur;
};

struct Intersection
{
	Sphere sphere;
	Vec3F pos;
	float t;
	Vec3F normale;
	float intensite;
};

typedef std::vector<Sphere> Spheres;

typedef std::vector<Intersection> Intersections;

template <typename T>
void permute(T &a, T &b)
{
	T c = a;
	a = b;
	b = c;
}

template <typename T>
T abs(T a)
{
	if (a < 0)
		return -a;
	return a;
}

bool intersectSphere(Ray &ray, Sphere &sphere, float &t)
{
	Vec3F pos = ray.pos - sphere.pos;
	float a = 1.0f; // dot2(ray.dir);
	float b = 2.0f * dot(ray.dir, pos);
	float c = dot2(pos) - sphere.rayon * sphere.rayon;
	float delta = b * b - 4.0f * a * c;
	if (delta <= 0.0f)
		return false;
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

bool intersectScene(Ray &ray, Spheres &spheres, Intersection &intersection)
{
	Intersections intersections;
	float t;
	for (unsigned int k = 0; k < spheres.size(); k++)
	{
		Sphere s = spheres.at(k);
		if (intersectSphere(ray, s, t))
		{
			intersection.t = t;
			intersection.pos = ray.pos + ray.dir * t;
			intersection.normale = intersection.pos - s.pos;
			intersection.normale = intersection.normale / norm(intersection.normale);
			intersection.intensite = 1.0f; // ray.intensite / (t * t) * dot(ray.pos - intersection.pos, intersection.normale);
			s.couleur.x = abs(intersection.normale.x * 0.5f + 0.5f) * 255.0f * intersection.intensite;
			s.couleur.y = abs(intersection.normale.y * 0.5f + 0.5f) * 255.0f * intersection.intensite;
			s.couleur.z = abs(intersection.normale.z * 0.5f + 0.5f) * 255.0f * intersection.intensite;
			intersection.sphere = s;
			intersections.push_back(intersection);
		}
	}
	if (!intersections.empty())
	{
		intersection = intersections.at(0);
		for (unsigned int k = 0; k < intersections.size(); k++)
			if (intersections.at(k).t < intersection.t)
				intersection = intersections.at(k);
		return true;
	}
	return false;
}

int main(int argc, char *argv[])
{
	FreeImage_Initialise();
	FIBITMAP *bitmap = FreeImage_Allocate(WIDTH, HEIGHT, BPP);
	RGBQUAD color;

	if (!bitmap)
		return 1;

	// Données

	Vec3F bleu, rouge;
	Ray r;
	Sphere s1, s2;
	Spheres spheres;
	Intersection inter;

	bleu = {0.0f, 0.0f, 1.0f};
	rouge = {1.0f, 0.0f, 0.0f};

	s1.pos = {500.0f, 500.0f, 500.0f};
	s1.rayon = 250.0f;
	s1.couleur = bleu;

	s2.pos = {380.0f, 380.0f, 200.0f};
	s2.rayon = 150.0f;
	s2.couleur = rouge;

	r.pos = {500.0f, 500.0f, 0.0f};
	r.dir = {0.0f, 0.0f, 1.0f};
	r.intensite = 100.0f;

	spheres.push_back(s1);
	spheres.push_back(s2);

	// Traitement

	for (float j = 0.0f; j < HEIGHT; j++)
		for (float i = 0.0f; i < WIDTH; i++)
		{
			r.pos = {i, j, 0.0f};
			if (intersectScene(r, spheres, inter))
			{
				color.rgbRed = inter.sphere.couleur.x;
				color.rgbGreen = inter.sphere.couleur.y;
				color.rgbBlue = inter.sphere.couleur.z;
				FreeImage_SetPixelColor(bitmap, (int)i, (int)j, &color);
			}
		}

	// Ecriture de l'image

	if (FreeImage_Save(FIF_PNG, bitmap, "out.png", 0))
		std::cout << "Image successfully saved!" << std::endl;
	FreeImage_DeInitialise();

	return 0;
}

// Compiler : g++ lumiere.cpp -o lumiere.out -Wall -std=c++11 -lfreeimage
