#include "bvh.h"
#include "meshs.h"

void init_scene(Light &light, Vertices &vertices, Facades &facades, Boxs &boxs)
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

	sphere_nickel.geom.type = SPHERE;
	sphere_nickel.geom.sphere.position = {250.f, 200.f, 500.f};
	sphere_nickel.geom.sphere.radius = 175.f;
	sphere_nickel.material = mat_nickel;

	sphere_white.geom.type = SPHERE;
	sphere_white.geom.sphere.position = {750.f, 700.f, 500.f};
	sphere_white.geom.sphere.radius = 175.f;
	sphere_white.material = mat_white;

	/* Murs */

	Object wall_back, wall_front, wall_up, wall_down, wall_right, wall_left;

	wall_back.material = mat_white;
	wall_front.material = mat_white;
	wall_up.material = mat_white;
	wall_down.material = mat_white;
	wall_right.material = mat_red;
	wall_left.material = mat_blue;

	wall_back.geom.type = wall_up.geom.type = wall_right.geom.type = wall_left.geom.type = wall_down.geom.type = wall_front.geom.type = SPHERE;

	wall_back.geom.sphere.radius = wall_front.geom.sphere.radius = wall_up.geom.sphere.radius = wall_right.geom.sphere.radius = wall_left.geom.sphere.radius = wall_down.geom.sphere.radius = 130000.f;

	wall_back.geom.sphere.position = {500.f, 500.f, wall_back.geom.sphere.radius + 1001.f};
	wall_front.geom.sphere.position = {500.f, 500.f, -wall_front.geom.sphere.radius - 1.f};
	wall_up.geom.sphere.position = {500.f, wall_up.geom.sphere.radius + 1001.f, 500.f};
	wall_down.geom.sphere.position = {500.f, -wall_down.geom.sphere.radius - 1.f, 500.f};
	wall_right.geom.sphere.position = {wall_right.geom.sphere.radius + 1001.f, 500.f, 500.f};
	wall_left.geom.sphere.position = {-wall_left.geom.sphere.radius - 1.f, 500.f, 500.f};

	/* Lumière */

	light.position = {250.f, 950.f, 200.f};
	light.color = {255.f, 255.f, 255.f};

	/* Triangles */

	create_mesh(vertices, facades, mat_white, boxs, 500.f);

	/* Boîtes */

	create_boxs(boxs);
}