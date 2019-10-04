#include <stdio.h>
#include <vector>
#include <random>
#include "vec3.h"
#include "structures.h"
#include "FreeImage/FreeImage.h"

#define WIDTH 1000
#define HEIGHT 1000
#define BPP 24

const float acne = 1e-3;

float clamp(float v, float min, float max)
{
    if (v > max)
        v = max;
    else if (v < min)
        v = min;
    return v;
}

bool intersect_sphere(Ray &ray, Sphere &sphere, Intersection &intersection)
{
    Vec3F pos = ray.origin - sphere.position;
    float a = 1.0f; /* dot2(ray.direction); */
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

bool intersect_scene(Ray &ray, Spheres &spheres, Intersection &intersection)
{
    Intersections intersections;
    size_t sphereCount = spheres.size();
    for (unsigned int k = 0; k < sphereCount; k++)
    {
        Sphere sphere = spheres.at(k);
        if (intersect_sphere(ray, sphere, intersection))
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

/* Distribution sur matériaux rugueux */

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

/* Coefficient de réflexion de Fresnel */

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

/* Fonction d'ombrage et de masquage de Smith */

float RDM_Smith(float LdotH, float LdotN, float VdotH, float VdotN, float alpha)
{
    return RDM_G1(LdotH, LdotN, alpha) * RDM_G1(VdotH, VdotN, alpha);
}

/* Specular */

Vec3F RDM_bsdf_s(float LdotH, float NdotH, float VdotH, float LdotN, float VdotN, Material &m)
{
    float d = RDM_Beckmann(NdotH, m.roughness);
    float f = RDM_Fresnel(LdotH, 1, m.IOR);
    float g = RDM_Smith(LdotH, LdotN, VdotH, VdotN, m.roughness);
    return m.specularColor * d * f * g / (4.0f * LdotN * VdotN);
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
    Vec3F s = lc * bsdf * LdotN;
    return s;
}

Vec3F trace_ray(Spheres &spheres, Lights &lights, Ray &ray)
{
    int light_size = 500;
    int nb_lights = 1000;
    int max_depth = 5;

    /* Générateur de nombre aléatoire */

    std::random_device device;
    std::mt19937 rng(device());
    std::uniform_int_distribution<std::mt19937::result_type> aleatoire(0, light_size);

    unsigned int lightsCount = lights.size();
    Vec3F color, retour;
    color = retour = {0.0f, 0.0f, 0.0f};
    Intersection intersection;
    if (intersect_scene(ray, spheres, intersection))
    {
        for (unsigned int l = 0; l < lightsCount; l++)
        {
            for (int k = 0; k < nb_lights; k++)
            {
                Light light;
                light.position.x = lights[l].position.x + aleatoire(device);
                light.position.y = lights[l].position.x + aleatoire(device);
                if (l == 0)
                    light.position.y = 950.0f;
                light.position.z = lights[l].position.z + aleatoire(device);
                if (l == 1)
                    light.position.z = 0.0f;
                light.color = lights[l].color;
                intersection.position = intersection.position + (intersection.normale * acne);
                Ray ray_shadow;
                ray_shadow.origin = intersection.position;
                ray_shadow.direction = normalize(light.position - intersection.position);
                Intersection inter_shadow;
                if (!intersect_scene(ray_shadow, spheres, inter_shadow) || inter_shadow.position.x > 1000 || inter_shadow.position.x < 0 || inter_shadow.position.y < 0 || inter_shadow.position.y > 1000 || inter_shadow.position.z < 0 || inter_shadow.position.z > 1000)
                {
                    Vec3F inv = ray.direction * -1.0f;
                    color = color + (shade(intersection.normale, inv, ray_shadow.direction, light.color, intersection.sphere.material) / ((float)nb_lights * (float)lightsCount) * 20.0f);
                }
            }
        }
        if (ray.depth < max_depth)
        {
            Ray ray_reflect;
            Vec3F dir_ray_reflect = reflect(ray.direction, intersection.normale);
            dir_ray_reflect = normalize(dir_ray_reflect);
            ray_reflect.origin = intersection.position + dir_ray_reflect * acne;
            ray_reflect.direction = dir_ray_reflect;
            ray_reflect.depth = ray.depth + 1;
            Vec3F color_reflect = trace_ray(spheres, lights, ray_reflect);
            retour = color + intersection.sphere.material.specularColor * RDM_Fresnel(dot(ray_reflect.direction, intersection.normale), 1.0f, intersection.sphere.material.IOR) * color_reflect;
        }
        else
        {
            retour = color;
        }
    }
    else
    {
        return {0.0f, 0.0f, 0.0f};
    }

    return retour;
}

int render_image(Spheres &spheres, Lights &lights)
{
    /* Initialisation de l'image */

    FreeImage_Initialise();
    FIBITMAP *bitmap = FreeImage_Allocate(WIDTH, HEIGHT, BPP);

    if (!bitmap)
        return 1;

    /* Traitement */

    Vec3F camera = {500.0f, 500.0f, -2000.0f};

    for (int j = 0; j < HEIGHT; j++)
    {
        #pragma omp parallel for
        for (int i = 0; i < WIDTH; i++)
        {
            Ray ray;
            ray.origin = {(float)i, (float)j, 0.0f};
            ray.direction = normalize(ray.origin - camera);
            ray.depth = 0;

            Vec3F color = trace_ray(spheres, lights, ray);

            RGBQUAD colorPixel;
            colorPixel.rgbRed = (int)clamp(color.x, 0.0f, 255.0f);
            colorPixel.rgbGreen = (int)clamp(color.y, 0.0f, 255.0f);
            colorPixel.rgbBlue = (int)clamp(color.z, 0.0f, 255.0f);
            FreeImage_SetPixelColor(bitmap, i, j, &colorPixel);
        }
    }

    /* Ecriture de l'image */

    if (FreeImage_Save(FIF_PNG, bitmap, "out.png", 0))
        printf("Image successfully saved!\n");
    FreeImage_DeInitialise();

    return 0;
}

void init_scene(Spheres &spheres, Lights &lights)
{
    /* Matériaux */

    Material mat_rouge, mat_vert, mat_bleu, mat_blanc, mat_nickel;

    mat_rouge.diffuseColor = {0.26f, 0.036f, 0.014f};
    mat_rouge.specularColor = {1.0f, 0.852f, 1.172f};
    mat_rouge.IOR = 1.0771f;
    mat_rouge.roughness = 0.0589f;

    mat_vert.diffuseColor = {0.016f, 0.073f, 0.04f};
    mat_vert.specularColor = {1.0f, 1.056f, 1.146f};
    mat_vert.IOR = 1.1481f;
    mat_vert.roughness = 0.0625f;

    mat_bleu.diffuseColor = {0.012f, 0.036f, 0.106f};
    mat_bleu.specularColor = {1.0f, 0.965f, 1.07f};
    mat_bleu.IOR = 1.1153f;
    mat_bleu.roughness = 0.068;

    mat_blanc.diffuseColor = {0.200, 0.200, 0.200};
    mat_blanc.specularColor = {1.0f, 0.766f, 0.762f};
    mat_blanc.IOR = 1.1022f;
    mat_blanc.roughness = 0.0579f;

    mat_nickel.diffuseColor = {0.014f, 0.012f, 0.012f};
    mat_nickel.specularColor = {1.0f, 0.882f, 0.786f};
    mat_nickel.IOR = 2.4449f;
    mat_nickel.roughness = 0.0681f;

    /* Sphères */

    Sphere sphere_blanche, sphere_nickel;

    sphere_blanche.position = {750.0f, 500.0f, 500.0f};
    sphere_blanche.radius = 175.0f;
    sphere_blanche.material = mat_blanc;

    sphere_nickel.position = {250.0f, 500.0f, 500.0f};
    sphere_nickel.radius = 175.0f;
    sphere_nickel.material = mat_nickel;

    spheres.push_back(sphere_blanche);
    spheres.push_back(sphere_nickel);

    /* Murs */

    Sphere sw1, sw2, sw3, sw4, sw5;

    sw1.material = mat_blanc;
    sw2.material = mat_blanc;
    sw3.material = mat_rouge;
    sw4.material = mat_bleu;
    sw5.material = mat_blanc;

    sw1.radius = sw2.radius = sw3.radius = sw4.radius = sw5.radius = 130000.0f;

    sw1.position = {500.0f, 500.0f, sw1.radius + 1001.0f};
    sw2.position = {500.0f, sw2.radius + 1001.0f, 500.0f};
    sw3.position = {sw3.radius + 1001.0f, 500.0f, 500.0f};
    sw4.position = {-sw4.radius - 1.0f, 500.0f, 500.0f};
    sw5.position = {500.0f, -sw5.radius - 1.0f, 500.0f};

    spheres.push_back(sw1);
    spheres.push_back(sw2);
    spheres.push_back(sw3);
    spheres.push_back(sw4);
    spheres.push_back(sw5);

    /* Lumières */

    Light light1, light2;

    light1.position = {250.0f, 950.0f, 150.0f};
    light1.color = {255.0f, 255.0f, 255.0f};

    light2.position = {250.0f, 250.0f, -250.0f};
    light2.color = {255.0f, 255.0f, 255.0f};

    lights.push_back(light1);
    // lights.push_back(light2);
}

int main()
{
    Spheres spheres;
    Lights lights;

    init_scene(spheres, lights);

    if (render_image(spheres, lights))
        return 0;

    return 1;
}
