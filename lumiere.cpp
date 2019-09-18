#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include "vec3.h"
#include "FreeImage/FreeImage.h"

#define WIDTH 600
#define HEIGHT 600
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

bool intersectSphere(Ray &ray, Sphere &sphere, float &t)
{
	Vec3F pos = ray.pos - sphere.pos;
	float a = 1.0f; // dot2(ray.dir);
	float b = 2.0f * dot(ray.dir, pos);
	float c = dot2(pos) - sphere.rayon * sphere.rayon;
	float delta = b * b - 4.0f * a * c;
	if (delta < 0.0f)
		return false;
	if (delta == 0.0f)
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

bool intersectScene(Ray &ray, Spheres &spheres, Intersection &intersection)
{
	Intersections intersections;
	float t;
	for (unsigned int k = 0; k < spheres.size(); k++)
	{
		Sphere s = spheres.at(k);
		if (intersectSphere(ray, s, t))
		{
			intersection.sphere = s;
			intersection.t = t;
			intersection.pos = ray.pos + ray.dir * t;
			intersection.normale = intersection.pos - s.pos;
			intersection.normale = intersection.normale / norm(intersection.normale);
			intersection.intensite = 1 / (t * t) * dot(ray.pos - intersection.pos, intersection.normale);
			// printf("%1.f\n", intersection.intensite);
			// s.couleur.x = ;
			// s.couleur.y = ;
			// s.couleur.z = ;
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

	s1.pos = {300.0f, 300.0f, 300.0f};
	s1.rayon = 150.0f;
	s1.couleur = bleu;

	s2.pos = {220.0f, 220.0f, 150.0f};
	s2.rayon = 100.0f;
	s2.couleur = rouge;

	r.pos = {300.0f, 300.0f, 0.0f};
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
