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
		intersection.mat = object.mat;
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
		intersection.object = object;
		intersection.mat = object.mat;
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
	Ray r;
	Light l1, l2;
	Object s1, s2, s3;
	Objects objects;
	Intersection inter;
	Lights lights;
	Material mat;
	mat.diffuseColor = {0.014f, 0.012f, 0.012f};
	mat.specularColor = {0.7f, 0.882f, 0.786f};
	mat.IOR = 6.0f;
	mat.roughness = 0.0181f;

	rouge = {255.0f, 0.0f, 0.0f};
	vert = {0.0f, 255.0f, 0.0f};
	bleu = {0.0f, 0.0f, 255.0f};
	blanc = {255.0f, 255.0f, 255.0f};

	s1.geom.type = SPHERE;
	s1.geom.sphere.position = {500.0f, 500.0f, 500.0f};
	s1.geom.sphere.radius = 250.0f;
	s1.color = vert;
	s1.mat = mat;

	s2.geom.type = SPHERE;
	s2.geom.sphere.position = {350.0f, 350.0f, 200.0f};
	s2.geom.sphere.radius = 150.0f;
	s2.color = rouge;
	s2.mat = mat;

	s3.geom.type = SPHERE;
	s3.geom.sphere.position = {650.0f, 650.0f, 800.0f};
	s3.geom.sphere.radius = 150.0f;
	s3.color = bleu;
	s3.mat = mat;

	r.direction = {0.0f, 0.0f, 1.0f};

	l1.position = {0.0f, 1000.0f, 0.0f};
	l1.color = blanc;
	l1.intensity = 1.0f;

	l2.position = {0.0f, 0.0f, 0.0f};
	l2.color = blanc;
	l2.intensity = 1.0f;

	lights.push_back(l1);
	lights.push_back(l2);

	objects.push_back(s1);
	objects.push_back(s2);
	objects.push_back(s3);

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
				size_t lightCount = lights.size();
				for (unsigned int k = 0; k < lightCount; k++)
				{
					Ray ray_to_light;
					ray_to_light.origin = inter.position;
					ray_to_light.direction = lights[k].position - inter.position;
					Intersection inter_light;
					if (intersectScene(ray_to_light, objects, inter_light))
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
