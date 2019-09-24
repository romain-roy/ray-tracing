#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "vec3.h"
#include "utils.h"
#include "structures.h"
#include "FreeImage/FreeImage.h"

#define WIDTH 1000
#define HEIGHT 1000
#define BPP 24

bool intersectPlane(Ray &ray, Object &object, Intersection &intersection)
{
	float dist = -((dot(ray.origin, object.geom.plane.normale) + object.geom.plane.distance) / dot(ray.direction, object.geom.plane.normale));
	if (dist > 0)
	{
		intersection.normale = normalize(object.geom.plane.normale);
		intersection.position = ray.origin + ray.direction * dist;
		intersection.distance = dist;
		object.color.x = abs(intersection.normale.x * 0.5f + 0.5f) * 255.0f;
		object.color.y = abs(intersection.normale.y * 0.5f + 0.5f) * 255.0f;
		object.color.z = abs(intersection.normale.z * 0.5f + 0.5f) * 255.0f;
		intersection.object = object;
		return true;
	}
	return false;
}

bool intersectSphere(Ray &ray, Object &object, Intersection &intersection)
{
	Vec3F pos = ray.origin - object.geom.sphere.position;
	float a = 1.0f; // dot2(ray.direction);
	float b = 2.0f * dot(ray.direction, pos);
	float c = dot2(pos) - object.geom.sphere.radius * object.geom.sphere.radius;
	float delta = b * b - 4.0f * a * c;
	if (delta <= 0.0f)
		return false;
	else
	{
		float t1 = (-b - sqrtf(delta)) / (2.0f * a);
		float t2 = (-b + sqrtf(delta)) / (2.0f * a);
		if (t1 > t2)
			permute(t1, t2);
		intersection.distance = t1;
	}
	if (intersection.distance >= 0.0f)
	{
		intersection.position = ray.origin + ray.direction * intersection.distance;
		intersection.normale = normalize(intersection.position - object.geom.sphere.position);
		object.color.x = abs(intersection.normale.x /* * 0.5f + 0.5f */) * 255.0f;
		object.color.y = abs(intersection.normale.y /* * 0.5f + 0.5f */) * 255.0f;
		object.color.z = abs(intersection.normale.z /* * 0.5f + 0.5f */) * 255.0f;
		intersection.object = object;
		return true;
	}
	return false;
}

bool intersectScene(Ray &ray, Objects &objects, Intersection &intersection)
{
	Intersections intersections;
	size_t objectCount = objects.size();
	for (unsigned int k = 0; k < objectCount; k++)
	{
		Object obj = objects.at(k);
		switch (obj.geom.type)
		{
		case PLANE:
			if (intersectPlane(ray, obj, intersection))
				intersections.push_back(intersection);
			break;
		case SPHERE:
			if (intersectSphere(ray, obj, intersection))
				intersections.push_back(intersection);
			break;
		}
	}
	if (!intersections.empty())
	{
		intersection = intersections.at(0);
		for (unsigned int k = 0; k < intersections.size(); k++)
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

	Vec3F color;
	Ray r, light;
	Object s1, s2, s3, p1, p2, p3;
	Objects objects;
	Intersection inter;

	color = {255.0f, 255.0f, 255.0f};

	s1.geom.type = SPHERE;
	s1.geom.sphere.position = {500.0f, 500.0f, 500.0f};
	s1.geom.sphere.radius = 250.0f;
	s1.color = color;

	s2.geom.type = SPHERE;
	s2.geom.sphere.position = {350.0f, 350.0f, 200.0f};
	s2.geom.sphere.radius = 150.0f;
	s2.color = color;

	s3.geom.type = SPHERE;
	s3.geom.sphere.position = {650.0f, 650.0f, 800.0f};
	s3.geom.sphere.radius = 150.0f;
	s3.color = color;

	Vec3F plane1 = {0.0f, 0.0f, -1.0f};
	p1.geom.type = PLANE;
	p1.geom.plane.normale = normalize(plane1);
	p1.geom.plane.distance = 1000.0f;
	p1.color = color;

	Vec3F plane2 = {-1.0f, 0.0f, -0.1f};
	p2.geom.type = PLANE;
	p2.geom.plane.normale = normalize(plane2);
	p2.geom.plane.distance = 1000.0f;
	p2.color = color;

	Vec3F plane3 = {1.0f, 0.0f, -0.1f};
	p3.geom.type = PLANE;
	p3.geom.plane.normale = normalize(plane3);
	p3.geom.plane.distance = 0.0f;
	p3.color = color;

	r.direction = {0.0f, 0.0f, 1.0f};

	light.origin = {500.0f, 0.0f, 500.0f};
	light.direction = {0.0f, 1.0f, 0.0f};

	objects.push_back(s1);
	objects.push_back(s2);
	objects.push_back(s3);
	// objects.push_back(p1);
	// objects.push_back(p2);
	// objects.push_back(p3);

	// Traitement

	for (int j = 0; j < HEIGHT; j++)
	{
		for (int i = 0; i < WIDTH; i++)
		{
			r.origin = {(float)i, (float)j, 0.0f};
			if (intersectScene(r, objects, inter))
			{
				colorPixel.rgbRed = inter.object.color.x;
				colorPixel.rgbGreen = inter.object.color.y;
				colorPixel.rgbBlue = inter.object.color.z;
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
