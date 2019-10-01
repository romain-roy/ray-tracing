#include <stdio.h>
#include <vector>
#include <random>
#include "vec3.h"
#include "structures.h"
#include "FreeImage/FreeImage.h"

#define WIDTH 1000
#define HEIGHT 1000
#define BPP 24

const float acne = 1e-2; /* 1e-4 */

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

float RDM_Beckmann(float NdotH, float alpha)
{
	if (NdotH > 0.0f)
	{
		float cos2 = NdotH * NdotH;
		float tan2 = (1.0f - cos2) / cos2;
		float cos4 = cos2 * cos2;
		float alpha2 = alpha * alpha;
		return expf(-tan2 / alpha2) / ((float)M_PI * alpha2 * cos4);
	}
	return 0.0f;
}

float RDM_Fresnel(float LdotH, float extIOR, float intIOR)
{
	float cosI = LdotH;
	float ratioIOR = extIOR / intIOR;
	float sinT2 = ratioIOR * ratioIOR * (1.0f - (cosI * cosI));
	if (sinT2 > 1.0f)
		return 1.0f;
	float cosT = sqrtf(1.0f - sinT2);
	float Rs = powf(cosI * extIOR - cosT * intIOR, 2) / powf(cosI * extIOR + cosT * intIOR, 2);
	float Rp = powf(cosT * extIOR - cosI * intIOR, 2) / powf(cosT * extIOR + cosI * intIOR, 2);
	return 0.5f * (Rs + Rp);
}

float RDM_chiplus(float c)
{
	return (c > 0.f) ? 1.f : 0.f;
}

float RDM_G1(float DdotH, float DdotN, float alpha)
{
	float tan = sqrtf(1.0f - DdotN * DdotN) / DdotN;
	float b = 1.0f / (alpha * tan);
	float k = DdotH / DdotN;
	if (k > 0.0f)
		if (b < 1.6f)
			return (3.535f * b + 2.181f * b * b) / (1.0f + 2.276f * b + 2.577f * b * b);
		else
			return 1.0f;
	else
		return 0.0f;
}

float RDM_Smith(float LdotH, float LdotN, float VdotH, float VdotN, float alpha)
{
	return RDM_G1(LdotH, LdotN, alpha) * RDM_G1(VdotH, VdotN, alpha);
}

Vec3F RDM_bsdf_s(float LdotH, float NdotH, float VdotH, float LdotN, float VdotN, Material &m)
{
	float d = RDM_Beckmann(NdotH, m.roughness);
	float f = RDM_Fresnel(LdotH, 1, m.IOR);
	float g = RDM_Smith(LdotH, LdotN, VdotH, VdotN, m.roughness);
	return m.specularColor * d * f * g / (4.0f * LdotN * VdotN);
}

Vec3F RDM_bsdf_d(Material &m)
{
	return m.diffuseColor / (float)M_PI;
}

Vec3F RDM_bsdf(float LdotH, float NdotH, float VdotH, float LdotN, float VdotN, Material &m)
{
	return RDM_bsdf_d(m) + RDM_bsdf_s(LdotH, NdotH, VdotH, LdotN, VdotN, m);
}

Vec3F shade(Vec3F &n, Vec3F v, Vec3F &l, Vec3F &lc, Material &mat)
{
	Vec3F h = normalize(v + l);
	float LdotH = dot(l, h);
	float VdotH = dot(v, h);
	float NdotH = dot(n, h);
	float LdotN = dot(l, n);
	float VdotN = dot(v, n);
	Vec3F bsdf = RDM_bsdf(LdotH, NdotH, VdotH, LdotH, VdotN, mat);
	return lc * bsdf * LdotN;
}

int main()
{
	FreeImage_Initialise();
	FIBITMAP *bitmap = FreeImage_Allocate(WIDTH, HEIGHT, BPP);
	RGBQUAD colorPixel;

	if (!bitmap)
		return 1;

	// Création des objets

	Vec3F rouge, vert, bleu, blanc, noir, c;
	rouge = {255.0f, 0.0f, 0.0f};
	vert = {0.0f, 255.0f, 0.0f};
	bleu = {0.0f, 0.0f, 255.0f};
	blanc = {255.0f, 255.0f, 255.0f};
	noir = {0.0f, 0.0f, 0.0f};

	Material m1, m2, m3, mw1, mw2, mw3, mw4, mw5;

	m1.diffuseColor = {0.26, 0.036, 0.014};
	m1.specularColor = {1.0, 0.852, 1.172};
	m1.IOR = 1.0771;
	m1.roughness = 0.0589;

	m2.diffuseColor = {0.016, 0.143, 0.04};
	m2.specularColor = {1.0, 0.739, 0.721};
	m2.IOR = 1.1051;
	m2.roughness = 0.0567;

	m3.diffuseColor = {0.012, 0.036, 0.212};
	m3.specularColor = {1.0, 0.748, 0.718};
	m3.IOR = 1.1051;
	m3.roughness = 0.0568;

	mw5.diffuseColor = {0.286, 0.235, 0.128};
	mw5.specularColor = {1.0, 0.766, 0.762};
	mw5.IOR = 1.1022;
	mw5.roughness = 0.0579;

	mw1 = mw2 = mw5;
	mw3 = m2;
	mw4 = m1;

	Spheres spheres;

	Sphere s1, s2, s3;
	s1.position = {725.0f, 600.0f, 600.0f};
	s1.radius = 150.0f;
	s1.color = rouge;
	s1.material = m1;
	s2.position = {275.0f, 500.0f, 400.0f};
	s2.radius = 200.0f;
	s2.color = vert;
	s2.material = m2;
	s3.position = {500.0f, 200.0f, 200.0f};
	s3.radius = 50.0f;
	s3.color = bleu;
	s3.material = m3;
	spheres.push_back(s1);
	spheres.push_back(s2);
	spheres.push_back(s3);

	Sphere sw1, sw2, sw3, sw4, sw5;
	sw1.color = sw2.color = sw5.color = blanc;
	sw3.color = vert;
	sw4.color = rouge;
	sw1.material = mw1;
	sw2.material = mw2;
	sw3.material = mw3;
	sw4.material = mw4;
	sw5.material = mw5;
	sw1.radius = sw2.radius = sw3.radius = sw4.radius = sw5.radius = 10000.0f;
	sw1.position = {500.0f, 500.0f, sw1.radius + 1000.0f};
	sw2.position = {500.0f, sw2.radius + 1000.0f, 500.0f};
	sw3.position = {sw3.radius + 1000.0f, 500.0f, 500.0f};
	sw4.position = {-sw4.radius, 500.0f, 500.0f};
	sw5.position = {500.0f, -sw5.radius, 500.0f};
	spheres.push_back(sw1);
	spheres.push_back(sw2);
	spheres.push_back(sw3);
	spheres.push_back(sw4);
	spheres.push_back(sw5);

	Light l1, l2, li;
	l1.position = {450.0f, 850.0f, 450.0f};
	l2.position = {450.0f, 450.0f, -50.0f};
	int nb_lights = 10;
	int light_size = 100;

	Lights lights;
	lights.push_back(l1);
	lights.push_back(l2);

	std::random_device rand;
	std::mt19937 rng(rand());
	std::uniform_int_distribution<std::mt19937::result_type> alea(0, light_size);

	Ray r;
	r.direction = {0.0f, 0.0f, 1.0f};

	Vec3F camera = {500.0f, 500.0f, -1250.0f};

	Intersection inter_sphere, inter_light;
	Ray ray_to_light;

	// Traitement

	unsigned int lightsCount = lights.size();

	for (int j = 0; j < HEIGHT; j++)
	{
		for (int i = 0; i < WIDTH; i++)
		{
			r.origin = {(float)i, (float)j, 0.0f};
			r.direction = normalize(r.origin - camera);
			c = {0.0f, 0.0f, 0.0f};
			if (intersectScene(r, spheres, inter_sphere))
			{
				for (unsigned int l = 0; l < lightsCount; l++)
				{
					for (int k = 0; k < nb_lights; k++)
					{
						li.position.x = lights[l].position.x + alea(rand);
						li.position.y = lights[l].position.y + alea(rand);
						li.position.z = lights[l].position.z + alea(rand);
						inter_sphere.position = inter_sphere.position + (inter_sphere.normale * acne);
						ray_to_light.origin = inter_sphere.position;
						ray_to_light.direction = normalize(li.position - inter_sphere.position);
						// float cos = dot(inter_sphere.normale, ray_to_light.direction);
						if (!intersectScene(ray_to_light, spheres, inter_light) || inter_light.distance > inter_sphere.distance)
						{
							Vec3F ls = normalize(li.position - inter_sphere.position);
							c = c + shade(inter_sphere.normale, r.direction * -1.0f, ls, blanc, inter_sphere.sphere.material);
							// c = c + (inter_sphere.sphere.color * cos) / nb_lights / lightsCount;
						}
					}
				}
			}
			colorPixel.rgbRed = clamp(c.x, 0.0f, 255.0f);
			colorPixel.rgbGreen = clamp(c.y, 0.0f, 255.0f);
			colorPixel.rgbBlue = clamp(c.z, 0.0f, 255.0f);
			FreeImage_SetPixelColor(bitmap, i, j, &colorPixel);
		}
	}

	// Ecriture de l'image

	if (FreeImage_Save(FIF_PNG, bitmap, "out.png", 0))
		printf("Image successfully saved!\n");
	FreeImage_DeInitialise();

	return 0;
}
