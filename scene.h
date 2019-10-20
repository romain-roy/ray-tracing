#include "bvh.h"
#include "meshs.h"

bool init_scene(Light &light, Boxs &boxs, Objects &objects)
{
    /* Matériaux */

    Material mat_red, mat_green, mat_blue, mat_white, mat_nickel;

    mat_red.diffuseColor = {0.26f, 0.036f, 0.014f};
    mat_red.specularColor = {1.f, 0.852f, 1.172f};
    mat_red.IOR = 1.0771f;
    mat_red.roughness = 0.0589f;

    mat_green.diffuseColor = {0.016f, 0.073f, 0.04f};
    mat_green.specularColor = {1.f, 1.056f, 1.146f};
    mat_green.IOR = 1.1481f;
    mat_green.roughness = 0.0625f;

    mat_blue.diffuseColor = {0.012f, 0.036f, 0.106f};
    mat_blue.specularColor = {1.f, 0.965f, 1.07f};
    mat_blue.IOR = 1.1153f;
    mat_blue.roughness = 0.068f;

    mat_white.diffuseColor = {0.2f, 0.2f, 0.2f};
    mat_white.specularColor = {1.f, 0.766f, 0.762f};
    mat_white.IOR = 1.1022f;
    mat_white.roughness = 0.0579f;

    mat_nickel.diffuseColor = {0.014f, 0.012f, 0.012f};
    mat_nickel.specularColor = {1.f, 0.882f, 0.786f};
    mat_nickel.IOR = 30.f;
    mat_nickel.roughness = 0.01f;

    /* Sphères */

    Object sphere_white, sphere_nickel;

    sphere_nickel.type = SPHERE;
    sphere_nickel.sphere.position = {200.f, 700.f, 500.f};
    sphere_nickel.sphere.radius = 175.f;
    sphere_nickel.material = mat_green;

    sphere_white.type = SPHERE;
    sphere_white.sphere.position = {350.f, 475.f, 300.f};
    sphere_white.sphere.radius = 100.f;
    sphere_white.material = mat_white;

    Box box_sphere_nickel;
    box_sphere_nickel.rt = {sphere_nickel.sphere.position.x + sphere_nickel.sphere.radius, sphere_nickel.sphere.position.y + sphere_nickel.sphere.radius, sphere_nickel.sphere.position.z + sphere_nickel.sphere.radius};
    box_sphere_nickel.lb = {sphere_nickel.sphere.position.x - sphere_nickel.sphere.radius, sphere_nickel.sphere.position.y - sphere_nickel.sphere.radius, sphere_nickel.sphere.position.z - sphere_nickel.sphere.radius};
    box_sphere_nickel.depth = DEPTH_BOX; // pour ne pas qu'elle soit redécoupée
    box_sphere_nickel.objects.push_back(sphere_nickel);
    boxs.push_back(box_sphere_nickel);

	Box box_sphere_white;
    box_sphere_white.rt = {sphere_white.sphere.position.x + sphere_white.sphere.radius, sphere_white.sphere.position.y + sphere_white.sphere.radius, sphere_white.sphere.position.z + sphere_white.sphere.radius};
    box_sphere_white.lb = {sphere_white.sphere.position.x - sphere_white.sphere.radius, sphere_white.sphere.position.y - sphere_white.sphere.radius, sphere_white.sphere.position.z - sphere_white.sphere.radius};
    box_sphere_white.depth = DEPTH_BOX; // pour ne pas qu'elle soit redécoupée
    box_sphere_white.objects.push_back(sphere_white);
    boxs.push_back(box_sphere_white);

    /* Murs de la Cornell Box */

    Object wall_back, wall_front, wall_up, wall_down, wall_right, wall_left;

    wall_back.material = mat_white;
    wall_front.material = mat_white;
    wall_up.material = mat_white;
    wall_down.material = mat_white;
    wall_right.material = mat_red;
    wall_left.material = mat_blue;

    wall_back.type = wall_up.type = wall_right.type = wall_left.type = wall_down.type = wall_front.type = SPHERE;

    wall_back.sphere.radius = wall_front.sphere.radius = wall_up.sphere.radius = wall_right.sphere.radius = wall_left.sphere.radius = wall_down.sphere.radius = 130000.f;

    wall_back.sphere.position = {500.f, 500.f, wall_back.sphere.radius + 1001.f};
    wall_front.sphere.position = {500.f, 500.f, -wall_front.sphere.radius - 1.f};
    wall_up.sphere.position = {500.f, wall_up.sphere.radius + 1001.f, 500.f};
    wall_down.sphere.position = {500.f, -wall_down.sphere.radius - 1.f, 500.f};
    wall_right.sphere.position = {wall_right.sphere.radius + 1001.f, 500.f, 500.f};
    wall_left.sphere.position = {-wall_left.sphere.radius - 1.f, 500.f, 500.f};

	objects.push_back(wall_front);
	objects.push_back(wall_up);
	objects.push_back(wall_down);
	objects.push_back(wall_right);
	objects.push_back(wall_left);

    /* Lumière */

    light.position = {250.f, 950.f, 250.f};
    light.color = {255.f, 255.f, 255.f};

    /* Meshs */

    Mesh bunny, dino;

    bunny.position = {675.f, 650.f, 500.f};
    bunny.taille = 500.f;
    bunny.material = mat_red;

    dino.position = {625.f, 200.f, 650.f};
    dino.taille = 500.f;
    dino.material = mat_blue;

    if (!parse("meshs/bunny.off", bunny))
        return false;
    create_mesh(bunny, boxs);

    if (!parse("meshs/dino.off", dino))
        return false;
    create_mesh(dino, boxs);

    /* Boîtes */

    create_boxs(boxs);

    return true;
}