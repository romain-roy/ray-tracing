#include <random>
#include "vec3.h"
#include "structures.h"
#include "read_off.h"
#include "FreeImage/FreeImage.h"

#define WIDTH 1000
#define HEIGHT 1000
#define BPP 24

#define MAX_DEPTH 0
#define NB_LIGHTS 1

const float acne = 0.0001f;

/* Générateur de nombre aléatoire */

std::random_device device;
std::mt19937 rng(device());
std::uniform_int_distribution<std::mt19937::result_type> aleatoire(0, 500);

/* Clamp un nombre entre 0 et 255 */

unsigned char clamp_color(float v)
{
	if (v > 255.f)
		v = 255.f;
	else if (v < 0.f)
		v = 0.f;
	return (unsigned char)v;
}

bool intersect_sphere(Ray &ray, Object &object, Intersection &intersection)
{
	Vec3F pos = ray.origin - object.geom.sphere.position;
	float a = 1.f; /* dot2(ray.direction); */
	float b = 2.f * dot(ray.direction, pos);
	float c = dot2(pos) - object.geom.sphere.radius * object.geom.sphere.radius;
	float delta = b * b - 4.f * a * c;
	if (delta <= 0.f)
		return false;
	else
	{
		float t1 = (-b - sqrtf(delta)) / (2.f * a);
		float t2 = (-b + sqrtf(delta)) / (2.f * a);
		intersection.distance = t1 > 0.f ? t1 : t2 > 0.f ? t2 : -1.f;
	}
	if (intersection.distance > 0.f)
	{
		intersection.position = ray.origin + ray.direction * intersection.distance;
		intersection.normale = normalize(intersection.position - object.geom.sphere.position);
		intersection.object = object;
		return true;
	}
	return false;
}

bool intersect_triangle(Ray &ray, Object &object, Intersection &intersection)
{
	Vec3F edge1, edge2, h, s, q;
	float a, f, u, v;
	const float epsilon = 0.0000001f;
	edge1 = object.geom.triangle.v1 - object.geom.triangle.v0;
	edge2 = object.geom.triangle.v2 - object.geom.triangle.v0;
	h = cross(ray.direction, edge2);
	a = dot(edge1, h);
	if (a > -epsilon && a < epsilon)
		return false;
	f = 1.f / a;
	s = ray.origin - object.geom.triangle.v0;
	u = f * (dot(s, h));
	if (u < 0.f || u > 1.f)
		return false;
	q = cross(s, edge1);
	v = f * (dot(ray.direction, q));
	if (v < 0.f || u + v > 1.f)
		return false;
	float t = f * dot(edge2, q);
	if (t > epsilon)
	{
		intersection.distance = t;
		intersection.position = ray.origin + ray.direction * t;
		intersection.normale = normalize(cross(edge1, edge2));
		intersection.object = object;
		return true;
	}
	else
		return false;
}

bool intersect_scene(Ray &ray, Objects &objects, Intersection &intersection)
{
	Intersections intersections;
	size_t object_count = objects.size();
	for (unsigned int k = 0; k < object_count; k++)
	{
		Object obj = objects.at(k);
		switch (obj.geom.type)
		{
		case TRIANGLE:
			if (intersect_triangle(ray, obj, intersection))
				intersections.push_back(intersection);
			break;
		case SPHERE:
			if (intersect_sphere(ray, obj, intersection))
				intersections.push_back(intersection);
			break;
		}
	}
	if (!intersections.empty())
	{
		intersection = intersections.at(0);
		size_t intersections_count = intersections.size();
		for (unsigned int k = 1; k < intersections_count; k++)
			if (intersections.at(k).distance < intersection.distance)
				intersection = intersections.at(k);
		return true;
	}
	return false;
}

/* Distribution sur matériaux rugueux */

float RDM_Beckmann(float NdotH, float alpha)
{
	if (NdotH > 0.f)
	{
		float cos2 = NdotH * NdotH;
		float tan2 = (1.f - cos2) / cos2;
		float cos4 = cos2 * cos2;
		float alpha2 = alpha * alpha;
		return expf(-tan2 / alpha2) / ((float)M_PI * alpha2 * cos4);
	}
	return 0.f;
}

/* Coefficient de réflexion de Fresnel */

float RDM_Fresnel(float LdotH, float extIOR, float intIOR)
{
	float cosI = LdotH;
	float ratioIOR = extIOR / intIOR;
	float sinT2 = ratioIOR * ratioIOR * (1.f - (cosI * cosI));
	if (sinT2 > 1.f)
		return 1.f;
	float cosT = sqrtf(1.f - sinT2);
	float Rs = powf(cosI * extIOR - cosT * intIOR, 2) / powf(cosI * extIOR + cosT * intIOR, 2);
	float Rp = powf(cosT * extIOR - cosI * intIOR, 2) / powf(cosT * extIOR + cosI * intIOR, 2);
	return 0.5f * (Rs + Rp);
}

float RDM_G1(float DdotH, float DdotN, float alpha)
{
	float tan = sqrtf(1.f - DdotN * DdotN) / DdotN;
	float b = 1.f / (alpha * tan);
	float k = DdotH / DdotN;
	if (k > 0.f)
		if (b < 1.6f)
			return (3.535f * b + 2.181f * b * b) / (1.f + 2.276f * b + 2.577f * b * b);
		else
			return 1.f;
	else
		return 0.f;
}

/* Fonction d'ombrage et de masquage de Smith */

float RDM_Smith(float LdotH, float LdotN, float VdotH, float VdotN, float alpha)
{
	return RDM_G1(LdotH, LdotN, alpha) * RDM_G1(VdotH, VdotN, alpha);
}

/* Specular */

Vec3F RDM_bsdf_s(float LdotH, float NdotH, float VdotH, float LdotN, float VdotN, Material &m)
{
	float d = RDM_Beckmann(NdotH, m.roughness);
	float f = RDM_Fresnel(LdotH, 1.f, m.IOR);
	float g = RDM_Smith(LdotH, LdotN, VdotH, VdotN, m.roughness);
	return m.specularColor * d * f * g / (4.f * LdotN * VdotN);
}

/* Diffuse */

Vec3F RDM_bsdf_d(Material &m)
{
	return m.diffuseColor / (float)M_PI;
}

/* Full BSDF */

Vec3F RDM_bsdf(float LdotH, float NdotH, float VdotH, float LdotN, float VdotN, Material &m)
{
	return RDM_bsdf_d(m) + RDM_bsdf_s(LdotH, NdotH, VdotH, LdotN, VdotN, m);
}

Vec3F shade(Vec3F &n, Vec3F &v, Vec3F &l, Vec3F &lc, Material &mat)
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

Vec3F trace_ray(Objects &objects, Light &light, Ray &ray)
{
	Vec3F color, ret;
	color = ret = {0.f, 0.f, 0.f};
	Intersection intersection;
	if (intersect_scene(ray, objects, intersection))
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
			if (!intersect_scene(ray_shadow, objects, inter_shadow) || inter_shadow.position.x < 0.f || inter_shadow.position.x > 1000.f || inter_shadow.position.y < 0.f || inter_shadow.position.y > 1000.f || inter_shadow.position.z < 0.f || inter_shadow.position.z > 1000.f)
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
			Vec3F color_reflect = trace_ray(objects, light, ray_reflect);
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

	return ret;
}

int render_image(Objects &objects, Light &light)
{
	/* Initialisation de l'image */

	FreeImage_Initialise();
	FIBITMAP *bitmap = FreeImage_Allocate(WIDTH, HEIGHT, BPP);

	if (!bitmap)
		return 1;

	/* Traitement */

	Vec3F camera = {500.f, 500.f, -2000.f};

	for (int j = 0; j < HEIGHT; j++)
	{
		#pragma omp parallel for
		for (int i = 0; i < WIDTH; i++)
		{
			Ray ray;
			ray.origin = {(float)i, (float)j, 0.f};
			ray.direction = normalize(ray.origin - camera);
			ray.depth = 0;

			Vec3F color = trace_ray(objects, light, ray);

			RGBQUAD colorPixel;
			colorPixel.rgbRed = clamp_color(color.x);
			colorPixel.rgbGreen = clamp_color(color.y);
			colorPixel.rgbBlue = clamp_color(color.z);
			FreeImage_SetPixelColor(bitmap, i, j, &colorPixel);
		}
	}

	/* Ecriture de l'image */

	if (FreeImage_Save(FIF_PNG, bitmap, "out.png", 0))
		printf("Image sauvegardée !\n");
	FreeImage_DeInitialise();

	return 0;
}

void create_mesh(Objects &objects, Vertices &vertices, Facades &facades, Material material)
{
	float taille = 500.f;

	/* Centrer l'objet */

	Vec3F somme_vertices = vertices.at(0);
	size_t vertices_count = vertices.size();
	float norm_max = norm(vertices.at(0));
	for (unsigned int i = 1; i < vertices_count; i++)
	{
		somme_vertices = somme_vertices + vertices.at(i);
		if (norm(vertices.at(i)) > norm_max)
			norm_max = norm(vertices.at(i));
	}
	Vec3F centre_gravite = somme_vertices / (float)vertices_count;
	Vec3F offset = {500.f, 500.f, 500.f};
	offset = offset - centre_gravite;

	/* Normaliser sa taille */

	norm_max /= taille;
	for (unsigned int i = 0; i < vertices_count; i++)
		vertices.at(i) = vertices.at(i) / norm_max;

	/* Création des triangles */

	size_t facades_count = facades.size();
	for (unsigned int i = 0; i < facades_count; i++)
	{
		Object triangle;
		triangle.material = material;
		triangle.geom.type = TRIANGLE;
		triangle.geom.triangle.v0 = vertices.at((int)facades.at(i).x) + offset;
		triangle.geom.triangle.v1 = vertices.at((int)facades.at(i).y) + offset;
		triangle.geom.triangle.v2 = vertices.at((int)facades.at(i).z) + offset;
		objects.push_back(triangle);
	}
}

void init_scene(Objects &objects, Light &light, Vertices &vertices, Facades &facades)
{
	/* Matériaux */

	Material mat_red, mat_green, mat_blue, mat_white, mat_nickel;

	mat_red.diffuseColor = {0.26f, 0.036f, 0.014f};
	mat_red.specularColor = {1.0f, 0.852f, 1.172f};
	mat_red.IOR = 1.0771f;
	mat_red.roughness = 0.0589f;

	mat_green.diffuseColor = {0.016f, 0.073f, 0.04f};
	mat_green.specularColor = {1.0f, 1.056f, 1.146f};
	mat_green.IOR = 1.1481f;
	mat_green.roughness = 0.0625f;

	mat_blue.diffuseColor = {0.012f, 0.036f, 0.106f};
	mat_blue.specularColor = {1.0f, 0.965f, 1.07f};
	mat_blue.IOR = 1.1153f;
	mat_blue.roughness = 0.068f;

	mat_white.diffuseColor = {0.200f, 0.200f, 0.200f};
	mat_white.specularColor = {1.0f, 0.766f, 0.762f};
	mat_white.IOR = 1.1022f;
	mat_white.roughness = 0.0579f;

	mat_nickel.diffuseColor = {0.014f, 0.012f, 0.012f};
	mat_nickel.specularColor = {1.0f, 0.882f, 0.786f};
	mat_nickel.IOR = 30.0f;
	mat_nickel.roughness = 0.01f;

	/* Sphères */

	Object sphere_white, sphere_nickel;

	sphere_white.geom.type = SPHERE;
	sphere_white.geom.sphere.position = {750.f, 500.f, 500.f};
	sphere_white.geom.sphere.radius = 175.f;
	sphere_white.material = mat_white;

	sphere_nickel.geom.type = SPHERE;
	sphere_nickel.geom.sphere.position = {250.f, 700.f, 500.f};
	sphere_nickel.geom.sphere.radius = 175.f;
	sphere_nickel.material = mat_nickel;

	// objects.push_back(sphere_white);
	// objects.push_back(sphere_nickel);

	/* Triangles */

	create_mesh(objects, vertices, facades, mat_white);

	/* Murs */

	Object wall_back, wall_up, wall_right, wall_left, wall_down;

	wall_back.material = mat_white;
	wall_up.material = mat_white;
	wall_right.material = mat_red;
	wall_left.material = mat_blue;
	wall_down.material = mat_white;

	wall_back.geom.type = wall_up.geom.type = wall_right.geom.type = wall_left.geom.type = wall_down.geom.type = SPHERE;

	wall_back.geom.sphere.radius = wall_up.geom.sphere.radius = wall_right.geom.sphere.radius = wall_left.geom.sphere.radius = wall_down.geom.sphere.radius = 130000.f;

	wall_back.geom.sphere.position = {500.f, 500.f, wall_back.geom.sphere.radius + 1001.f};
	wall_up.geom.sphere.position = {500.f, wall_up.geom.sphere.radius + 1001.f, 500.f};
	wall_right.geom.sphere.position = {wall_right.geom.sphere.radius + 1001.f, 500.f, 500.f};
	wall_left.geom.sphere.position = {-wall_left.geom.sphere.radius - 1.f, 500.f, 500.f};
	wall_down.geom.sphere.position = {500.f, -wall_down.geom.sphere.radius - 1.f, 500.f};

	objects.push_back(wall_back);
	objects.push_back(wall_up);
	objects.push_back(wall_right);
	objects.push_back(wall_left);
	objects.push_back(wall_down);

	/* Lumière */

	light.position = {250.f, 950.f, 200.f};
	light.color = {255.f, 255.f, 255.f};
}

int main()
{
	Objects objects;
	Light light;
	Vertices vertices;
	Facades facades;

	if (!parse("meshs/triceratops.off", vertices, facades))
		return 1;

	init_scene(objects, light, vertices, facades);

	if (render_image(objects, light))
		return 0;

	return 1;
}